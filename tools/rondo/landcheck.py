#!/usr/bin/env python3
"""landcheck.py -- the two checks land.sh makes on a worktree before it fast-forwards main.

    python3 tools/rondo/landcheck.py image OUTDIR     filed assembly in place, whole image, game and card
    python3 tools/rondo/landcheck.py link OUTDIR      the hybrid built from this tree's C links clean

Run from the tree being checked (its own tools/rondo/landcheck.py), so every output lands in that
tree's own build/rondo and OUTDIR; nothing is shared with another tree. One line per refusal, exit 1.
Heavy (assembler, compiler): run through cpu-run (land.sh does).
"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import cpuguard  # noqa: E402

cpuguard.require("landcheck.py")
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


def main():
    if len(sys.argv) == 3 and sys.argv[1] in ("image", "link"):
        return {"image": image, "link": link}[sys.argv[1]](sys.argv[2])
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
