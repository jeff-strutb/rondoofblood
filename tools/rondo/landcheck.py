#!/usr/bin/env python3
"""landcheck.py -- the two checks land.sh makes on a worktree before it fast-forwards main.

    python3 tools/rondo/landcheck.py image OUTDIR     filed assembly in place, whole image, game and card
    python3 tools/rondo/landcheck.py link OUTDIR      the hybrid built from this tree's C links clean
    python3 tools/rondo/landcheck.py scripts BASE     the scripts this tree changes against BASE that need a trace:
                                                      added or changed, or including a changed include file
    python3 tools/rondo/landcheck.py code FILE        bytes the code map FILE marks as run
    python3 tools/rondo/landcheck.py proper BASE      the ledger's proper-C check on every routine, here against BASE:
                                                      refuse a routine that was proper and is not, or is new and is not

Run from the tree being checked (its own tools/rondo/landcheck.py), so every output lands in that
tree's own build/rondo and OUTDIR; nothing is shared with another tree. One line per refusal, exit 1.
Heavy (assembler, compiler): land.sh runs it.
"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import asmfile  # noqa: E402

TRACK = os.path.join(TOP, "build", "rondo", "disc", "track02.bin")
BUILT = os.path.join(TOP, "build", "rondo", "asm", "image.bin")


def routine_at(offset):
    """The filed routine whose first byte is the last one at or before this disc offset."""
    best = None
    for r in asmfile.all_routines():
        if r["disc"] <= offset:
            best = r
    return f"{best['name']} {best['tag']}" if best else "before every filed routine"


def image(out):
    os.makedirs(out, exist_ok=True)
    if os.path.exists(BUILT):
        os.remove(BUILT)
    r = subprocess.run([sys.executable, os.path.join(HERE, "asmbuild.py")], capture_output=True, text=True,
                       cwd=TOP, timeout=600)
    if r.returncode or not os.path.exists(BUILT):
        tail = (r.stdout + r.stderr).strip().splitlines()[-4:]
        print("LAND REFUSED (assembly): the game's filed assembly does not assemble: " + " | ".join(tail))
        return 1
    want, got = open(TRACK, "rb").read(), open(BUILT, "rb").read()
    diff = [i for i in range(min(len(want), len(got))) if want[i] != got[i]]
    if diff or len(want) != len(got):
        first = diff[0] if diff else min(len(want), len(got))
        n = len(diff) + abs(len(want) - len(got))
        print(f"LAND REFUSED (image): {n} bytes of the game image differ from the disc, first at "
              f"{first // 2048:04X}:{first % 2048:03X} in {routine_at(first)} (built "
              f"{got[first]:02X} want {want[first]:02X}); fix that routine's assembly")
        return 1
    card = subprocess.run([sys.executable, os.path.join(TOP, "syscard", "tools", "image.py"),
                           "--image", os.path.join(TOP, "build", "rondo", "system", "syscard3.pce"),
                           "--cc65", os.path.join(TOP, "tools", "toolchains", "cc65"),
                           "--out", os.path.join(out, "card")], capture_output=True, text=True, timeout=600)
    last = (card.stdout + card.stderr).strip().splitlines()[-1:] or [""]
    if card.returncode:
        print(f"LAND REFUSED (card image): {last[0]}; fix the card assembly under syscard/asm")
        return 1
    print(f"image byte exact: game {len(got)} B, card: {last[0]}")
    return 0


def link(out):
    os.makedirs(out, exist_ok=True)
    env = dict(os.environ, RONDO_BOX=os.path.join(out, "pcebox"))
    r = subprocess.run(["sh", os.path.join(TOP, "setup.sh"), "pcebox"], capture_output=True, text=True,
                       cwd=TOP, env=env, timeout=900)
    text = r.stdout + r.stderr
    open(os.path.join(out, "link.log"), "w").write(text)
    if r.returncode == 0:
        undefined = re.findall(r"undefined reference to `(abi_\w+)'", text)
        if not undefined:
            print("link clean: hybrid built, no undefined abi_ symbol")
            return 0
    undefined = sorted(set(re.findall(r"undefined reference to `(\w+)'", text)))
    if undefined:
        print("LAND REFUSED (link): calls into a routine with no C, undefined: " + " ".join(undefined[:8]) +
              "; file its C with an abi_ wrapper or stop calling it")
        return 1
    missing = re.findall(r"^(\S+ \w+): no abi_\w+", text, re.M)
    if missing:
        print("LAND REFUSED (link): routines filed without an abi_ wrapper: " + ", ".join(missing[:8]))
        return 1
    errors = [ln for ln in text.splitlines() if re.search(r"error|Error", ln)][:3]
    print("LAND REFUSED (build): the hybrid does not build: " + " | ".join(errors or text.strip().splitlines()[-3:]))
    return 1


def scripts(base):
    """Names (no .txt) of the scripts to trace: those added or changed since BASE, and every script that
    includes (through any chain of include-only `_` files) a changed file."""
    folder = os.path.join(TOP, "tools", "rondo", "scripts")
    out = subprocess.run(["git", "diff", "--name-only", "--diff-filter=ACMR", base, "HEAD", "--", "tools/rondo/scripts"],
                         cwd=TOP, capture_output=True, text=True).stdout.split()
    hit = {os.path.splitext(os.path.basename(p))[0] for p in out if p.endswith(".txt")}
    includes = {}
    for f in os.listdir(folder):
        if f.endswith(".txt"):
            includes[f[:-4]] = {os.path.splitext(os.path.basename(m))[0] for m in
                                re.findall(r"^include\s+(\S+)", open(os.path.join(folder, f)).read(), re.M)}
    grew = True
    while grew:
        grew = False
        for name, inc in includes.items():
            if name not in hit and inc & hit:
                hit.add(name)
                grew = True
    print(" ".join(sorted(n for n in hit if not n.startswith("_") and n in includes)))
    return 0


def proper(base):
    """ledger.py's own proper_c_faults on every tagged routine here and in BASE (git): a routine that was proper and
    now has a fault, or is new and has one, refuses the landing; one that failed before and still fails is counted."""
    import ledger
    here = sorted(ledger.source_texts())          # same order on both sides: a name defined twice is judged alike
    listed = subprocess.run(["git", "ls-tree", "-r", "--name-only", base, "--", "src/rondo", "syscard/src"],
                            cwd=TOP, capture_output=True, text=True).stdout.split()
    before = [(p, subprocess.run(["git", "show", f"{base}:{p}"], cwd=TOP, capture_output=True, text=True).stdout)
              for p in listed if p.endswith(".c")]
    before.sort()
    f_now, f_old = ledger.functions(here), ledger.functions(before)
    names_now, names_old = ledger.tagged_names(here), ledger.tagged_names(before)
    worse, still = [], 0
    for name in sorted(names_now):
        now = ledger.proper_c_faults(name, f_now)
        if not now:
            continue
        old = ledger.proper_c_faults(name, f_old) if name in names_old else None
        if old:
            still += 1
        else:
            worse.append(f"{name}: {'new routine' if old is None else 'was proper'}, now {'; '.join(now)}")
    if worse:
        print("LAND REFUSED (proper C): " + " | ".join(worse[:6]) + (f" | +{len(worse) - 6} more" if len(worse) > 6 else "")
              + ": use typed C (no cpu., Alu, adc8, sbc8, alu helpers, call_with, call_abi, goto)")
        return 1
    print(f"proper C: no routine got worse; {still} routine(s) were failing before and still fail (allowed)")
    return 0


def code(path):
    print(sum(1 for b in open(path, "rb").read() if b & 3) if os.path.exists(path) else 0)
    return 0


def merge(trace):
    """Refuse a landing whose tree's scripts do not give the shared merged map: the merge in TRACE (main's
    build/rondo/trace) must be the OR of the traces of this tree's scripts, as intake.py and progress.py require. A
    script traced into the shared folder by anything but a landing (an unlanded script) breaks every writer."""
    names = sorted(os.path.splitext(f)[0] for f in os.listdir(os.path.join(TOP, "tools", "rondo", "scripts"))
                   if f.endswith(".txt") and not f.startswith("_"))
    for m in ("discmap.bin", "cardmap.bin"):
        mp = os.path.join(trace, m)
        if not os.path.exists(mp):
            continue
        acc, missing = 0, []
        for n in names:
            p = os.path.join(trace, n, m)
            if os.path.exists(p):
                acc |= int.from_bytes(open(p, "rb").read(), "little")
            else:
                missing.append(n)
        if missing:
            print(f"the shared trace has no {m} for: {' '.join(missing)}")
            return 1
        if acc != int.from_bytes(open(mp, "rb").read(), "little"):
            print(f"the shared {m} is not the OR of this tree's scripts' traces (a script traced into it that is not "
                  f"landed, or a script of this tree not merged): rerun trace.py from the main checkout")
            return 1
    return 0


def main():
    if len(sys.argv) == 3 and sys.argv[1] == "merge":
        return merge(sys.argv[2])
    if len(sys.argv) == 3 and sys.argv[1] in ("scripts", "code", "proper"):
        return {"scripts": scripts, "code": code, "proper": proper}[sys.argv[1]](sys.argv[2])
    if len(sys.argv) == 3 and sys.argv[1] in ("image", "link"):
        return {"image": image, "link": link}[sys.argv[1]](sys.argv[2])
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
