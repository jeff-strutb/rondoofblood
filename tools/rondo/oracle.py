#!/usr/bin/env python3
"""oracle.py -- run the oracles over the scripts and report by routine.

    python3 tools/rondo/oracle.py a5 [--box PCEBOX] [SCRIPT ...]
    python3 tools/rondo/oracle.py a7 [SCRIPT ...]
    python3 tools/rondo/oracle.py plant NAME OLD NEW    a5 on a copy with one bug

a5 judges every call of every routine with C, script by script, and writes the
per-routine table to build/rondo/oracle/a5.csv with gate C (every byte of the
routine executed, from the trace's map). a7 records each script on the original,
then runs it with the C swapped in and compares every frame. plant builds a
pcebox from a copy of src/rondo with OLD replaced by NEW in file NAME, and a5 must
then report that routine divergent.

Run under WSL: the core and pcebox are Linux builds.
"""
import concurrent.futures
import glob
import hashlib
import os
import re
import shutil
import subprocess
import sys
import threading

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import asmfile  # noqa: E402
import cardfile  # noqa: E402
import disc  # noqa: E402

PCEBOX = os.path.join(TOP, "build", "rondo", "pcebox", "pcebox")
CORE = os.path.join(TOP, "tools", "toolchains", "pce", "mednafen_pce_libretro.dylib")
SCRIPTS = os.path.join(HERE, "scripts")
OUT = os.path.join(TOP, "build", "rondo", "oracle")
TRACK = os.path.join(disc.OUT, "track02.bin")
TIMEOUT = 1500


def scripts_from(args):
    names = args or sorted(os.path.splitext(os.path.basename(p))[0]
                           for p in glob.glob(os.path.join(SCRIPTS, "*.txt"))
                           if not os.path.basename(p).startswith("_"))
    return names


def box(pcebox, script, flag, path, out):
    os.makedirs(out, exist_ok=True)
    return subprocess.run([pcebox, "--core", CORE, "--cue", os.path.join(disc.REF, disc.CUE),
                           "--system", disc.SYSTEM,
                           "--script", os.path.join(SCRIPTS, script + ".txt"), "--out", out,
                           flag, path, "--user", TRACK],
                          capture_output=True, text=True, timeout=TIMEOUT)


def run_a5(pcebox, label, names):
    out = os.path.join(OUT, label)

    def one(script):
        path = os.path.join(out, script + ".a5.txt")
        try:
            r = box(pcebox, script, "--a5", path, os.path.join(out, script))
        except subprocess.TimeoutExpired:
            return script, f"timed out after {TIMEOUT} s"
        return script, (f"exit {r.returncode}: {r.stderr.strip()[-300:]}" if r.returncode else None)

    totals, details = {}, {}
    bad = 0
    with concurrent.futures.ThreadPoolExecutor(max_workers=min(len(names), os.cpu_count())) as ex:
        for script, err in ex.map(one, names):
            print(f"{script}: {err or 'done'}", flush=True)
            bad += err is not None
    for script in names:
        path = os.path.join(out, script + ".a5.txt")
        if not os.path.exists(path):
            continue
        for line in open(path, encoding="ascii", errors="replace"):
            f = line.split()
            if f and f[0] == "TAG":
                t = totals.setdefault(f[1], [0, 0, 0, 0, 0])
                for i in range(5):
                    t[i] += int(f[2 + i])
            elif f:
                d = details.setdefault(f[0], [])
                if sum(1 for ln in d if ln.split()[1:2] == f[1:2]) < 2:
                    d.append(line.rstrip())
    return totals, details, bad


def coverage():
    """Gate C by routine name: bytes in the span, bytes the trace saw executed."""
    path = os.path.join(TOP, "build", "rondo", "trace", "discmap.bin")
    if not os.path.exists(path):
        return {}
    m = open(path, "rb").read()
    buf = open(TRACK, "rb").read()
    routines = asmfile.all_routines()
    cover = {}
    for i, r in enumerate(routines):
        nxt = routines[i + 1]["disc"] if i + 1 < len(routines) else len(buf)
        n = sum(ins[1] for ins in asmfile.span(buf, r, nxt))
        cover[r["name"]] = (n, sum(1 for k in range(n) if m[r["disc"] + k] & 3))
    return cover


def tree_stamp():
    """A hash of everything a verdict depends on: any change voids it (ledger.py)."""
    digest = hashlib.sha256()
    tops = [os.path.join(TOP, "src", "rondo"), os.path.join(TOP, "tools", "rondo", "pcebox"),
            os.path.join(TOP, "config", "rondo", "symbols.csv"), os.path.join(HERE, "scripts"),
            os.path.join(TOP, "syscard", "src"), os.path.join(TOP, "syscard", "include")]
    files = []
    for top in tops:
        if os.path.isfile(top):
            files.append(top)
        for folder, _, names in os.walk(top):
            files += [os.path.join(folder, n) for n in names]
    for path in sorted(files):
        digest.update(os.path.relpath(path, TOP).replace("\\", "/").encode())
        digest.update(open(path, "rb").read())
    return digest.hexdigest()


def write_stamp(name, whole):
    """A verdict counts only from a run of every script, with no script failing."""
    path = os.path.join(OUT, name + ".stamp")
    if os.path.exists(path):
        os.remove(path)
    if whole:
        with open(path, "w", newline="\n") as f:
            f.write(tree_stamp() + "\n")


def report_card(totals, csv_name):
    """The System Card's routines, judged the same way, in a table of their own."""
    cover = cardfile.coverage()
    rows = ["routine,bytes,bytes_run,calls,identical,divergent,unverified,order_flags,verdict"]
    for r in cardfile.spans():
        name = r["name"]
        calls, same, diff, unv, order = totals.get(name, [0, 0, 0, 0, 0])
        n, ran = cover.get(name, (r["length"], 0))
        v = "UNCOVERED" if calls == 0 else "DIVERGENT" if diff else "UNVERIFIED" if same == 0 else "IDENTICAL"
        rows.append(f"{name},{n},{ran},{calls},{same},{diff},{unv},{order},{v}")
    with open(os.path.join(OUT, "card_" + csv_name), "w", newline="\n") as f:
        f.write("\n".join(rows) + "\n")
    print("card routines:", "; ".join(f"{row.split(',')[0]} {row.split(',')[-1]}" for row in rows[1:]))


def report_a5(totals, details, bad, csv_name="a5.csv"):
    cover = coverage()
    os.makedirs(OUT, exist_ok=True)
    rows = ["routine,bytes,bytes_run,calls,identical,divergent,unverified,order_flags,verdict"]
    tally = {"IDENTICAL": 0, "DIVERGENT": 0, "UNVERIFIED": 0, "UNCOVERED": 0}
    b_tally = dict(tally)
    for r in asmfile.all_routines():
        name = r["name"]
        calls, same, diff, unv, order = totals.get(name, [0, 0, 0, 0, 0])
        n, ran = cover.get(name, (0, 0))
        if calls == 0:
            v = "UNCOVERED"
        elif diff:
            v = "DIVERGENT"
        elif same == 0:
            v = "UNVERIFIED"
        else:
            v = "IDENTICAL"
        tally[v] += 1
        b_tally[v] += n
        rows.append(f"{name},{n},{ran},{calls},{same},{diff},{unv},{order},{v}")
    with open(os.path.join(OUT, csv_name), "w", newline="\n") as f:
        f.write("\n".join(rows) + "\n")
    report_card(totals, csv_name)
    print("routine verdicts:", tally)
    print("bytes by verdict:", b_tally)
    print("calls:", sum(t[0] for t in totals.values()), "identical", sum(t[1] for t in totals.values()),
          "divergent", sum(t[2] for t in totals.values()), "unverified", sum(t[3] for t in totals.values()),
          "order flags", sum(t[4] for t in totals.values()))
    print("routines fully executed (gate C):",
          sum(1 for n, ran in cover.values() if n and n == ran), "of", len(cover))
    report_gates(rows)
    for kind in ("DIVERGENT", "ORDER", "UNVERIFIED"):
        for name, lines in sorted(details.items()):
            for line in [ln for ln in lines if ln.split()[1:2] == [kind]][:2]:
                print(" ", line[:230])
    for v in ("UNVERIFIED", "UNCOVERED"):
        print(v + ":", " ".join(r.split(",")[0] for r in rows[1:] if r.endswith("," + v)))
    print("run but not every byte:", " ".join(
        f"{name}({ran}/{n})" for name, (n, ran) in sorted(cover.items()) if 0 < ran < n))
    return bad


def report_gates(rows):
    """Which routines meet every gate as measured, and what the others lack."""
    table = open(os.path.join(TOP, "build", "rondo", "pcebox", "hybrid_table.c")).read()
    swappable = {m.group(1): m.group(2) == "1"
                 for m in re.finditer(r'"(\w+)", abi_\w+, (\d),', table)}
    met, total = [0, 0], 0
    lacking = {"no call judged": 0, "not every byte run": 0, "cannot be swapped in A7": 0}
    for row in rows[1:]:
        name, n, ran, calls, same = row.split(",")[:5]
        n, ran, calls, same = int(n), int(ran), int(calls), int(same)
        total += n
        reasons = []
        if same == 0:
            reasons.append("no call judged")
        if ran < n:
            reasons.append("not every byte run")
        if not swappable.get(name):
            reasons.append("cannot be swapped in A7")
        for r in reasons:
            lacking[r] += 1
        if not reasons:
            met[0] += 1
            met[1] += n
    print(f"every gate met as measured: {met[0]} routines, {met[1]} B of {total} B; lacking: {lacking}")


def run_a7(names):
    out = os.path.join(OUT, "a7")

    def one(script):
        rec = os.path.join(out, script + ".rec")
        d = os.path.join(out, script)
        try:
            r = box(PCEBOX, script, "--record", rec, d)
            if r.returncode:
                return script, f"record exit {r.returncode}: {r.stderr.strip()[-300:]}"
            h = box(PCEBOX, script, "--hybrid", rec, d)
        except subprocess.TimeoutExpired:
            return script, f"timed out after {TIMEOUT} s"
        if h.returncode:
            return script, f"hybrid exit {h.returncode}: {h.stderr.strip()[-400:]}"
        for line in h.stdout.splitlines():
            if line.startswith("SWAPPED "):
                _, name, count = line.split()
                with swapped_lock:
                    swapped[name] = swapped.get(name, 0) + int(count)
        return script, next((ln for ln in h.stdout.splitlines() if ln.startswith("hybrid:")), "no result")

    bad = 0
    swapped = {}
    swapped_lock = threading.Lock()
    with concurrent.futures.ThreadPoolExecutor(max_workers=min(len(names), os.cpu_count())) as ex:
        for script, text in ex.map(one, names):
            print(f"{script}: {text}", flush=True)
            bad += "first differing -1" not in text
    with open(os.path.join(OUT, "a7.csv"), "w", newline="\n") as f:
        f.write("routine,swapped\n")
        for name, count in sorted(swapped.items()):
            f.write(f"{name},{count}\n")
    write_stamp("a7", not bad and names == scripts_from([]))
    return 1 if bad else 0


def plant(name, old, new):
    """a5 on a copy of the C with one bug: the routine must come back divergent."""
    card = name.startswith("syscard/")
    src = os.path.join(OUT, "plant", "src")
    shutil.rmtree(os.path.join(OUT, "plant"), ignore_errors=True)
    shutil.copytree(os.path.join(TOP, "syscard", "src") if card else os.path.join(TOP, "src", "rondo"), src)
    path = os.path.join(src, name[len("syscard/src/"):] if card else name)
    text = open(path, encoding="utf-8").read()
    if old not in text:
        print(f"{old!r} is not in {name}")
        return 1
    open(path, "w", encoding="utf-8", newline="\n").write(text.replace(old, new, 1))
    built = os.path.join(OUT, "plant", "pcebox")
    env = dict(os.environ, RONDO_BOX=built)
    env["RONDO_CARD_SRC" if card else "RONDO_SRC"] = src
    r = subprocess.run(["sh", os.path.join(TOP, "setup.sh"), "pcebox"], env=env, capture_output=True, text=True)
    if r.returncode:
        print(r.stderr[-600:])
        return 1
    totals, details, bad = run_a5(built, "plant", ["play_s0_r1"])
    report_a5(totals, details, bad, "plant.csv")
    return 0


def main():
    what = sys.argv[1] if len(sys.argv) > 1 else ""
    if what == "a5":
        names = scripts_from(sys.argv[2:])
        totals, details, bad = run_a5(PCEBOX, "a5", names)
        status = report_a5(totals, details, bad)
        write_stamp("a5", not bad and names == scripts_from([]))
        return status
    if what == "a7":
        return run_a7(scripts_from(sys.argv[2:]))
    if what == "plant" and len(sys.argv) == 5:
        return plant(*sys.argv[2:])
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
