#!/usr/bin/env python3
"""gate.py -- A5 and A7 on committed source, recorded with the commit they measured.

    python3 tools/rondo/gate.py [REV]    default HEAD; about 11 cores and 4 GB for its runs

Exports REV with git archive to build/rondo/gate/tree, builds pcebox from that tree,
and runs that tree's oracle.py a5 and a7 over every script. The work tree's uncommitted
changes (other sessions' work in progress) never reach a verdict. Writes the records:

  config/rondo/gate_a5.csv           every routine, game and card: A5's counts and verdict
  config/rondo/gate_a7.csv           every script: A7's verdict and its first line
  config/rondo/gate_a7_routines.csv  every routine A7 swapped in, or whose C ran inside a
                                     swapped call, with the counts

each row with the commit measured. A gate whose run of some script failed (an exit or a
timeout) writes no record of that gate: the last records stand. ledger.py credits a routine
from the records while nothing it depends on has changed since the commit measured (its C
file, the C files of every function its C reaches, and the shared paths in ledger.SHARED),
so a commit elsewhere no longer voids it.

One gate run at a time (build/rondo/gate.lock); a lock left by a run that died is taken over.
"""
import csv
import os
import re
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
WORK = os.path.join(TOP, "build", "rondo", "gate")
TREE = os.path.join(WORK, "tree")
OUT = os.path.join(WORK, "out")
CONFIG = os.path.join(TOP, "config", "rondo")
LOCK = os.path.join(TOP, "build", "rondo", "gate.lock")
# What the exported tree needs that git does not carry: the disc, the card, the
# toolchains, and the traces (gate C's coverage in oracle.py's report).
LINKS = ("references", "tools/toolchains/src", "tools/toolchains/cc65", "tools/toolchains/pce",
         "build/rondo/disc", "build/rondo/system", "build/rondo/trace")


def git(*args):
    return subprocess.run(["git", *args], capture_output=True, text=True, cwd=TOP)


def take_lock():
    if os.path.exists(LOCK):
        try:
            pid = int(open(LOCK).read().strip())
            os.kill(pid, 0)
            raise SystemExit(f"another gate run (pid {pid}) holds {LOCK}")
        except (ValueError, ProcessLookupError):
            pass
    os.makedirs(os.path.dirname(LOCK), exist_ok=True)
    open(LOCK, "w").write(str(os.getpid()))


def export(rev):
    """REV's tree at TREE, with the untracked inputs linked in from this checkout."""
    shutil.rmtree(TREE, ignore_errors=True)
    os.makedirs(TREE)
    archive = subprocess.Popen(["git", "archive", rev], stdout=subprocess.PIPE, cwd=TOP)
    subprocess.run(["tar", "-x", "-C", TREE], stdin=archive.stdout, check=True)
    if archive.wait():
        raise SystemExit(f"git archive {rev} failed")
    for rel in LINKS:
        src = os.path.realpath(os.path.join(TOP, rel))
        if not os.path.exists(src):
            raise SystemExit(f"{rel}: missing in this checkout")
        dst = os.path.join(TREE, rel)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        if os.path.lexists(dst):
            os.remove(dst) if not os.path.isdir(dst) or os.path.islink(dst) else shutil.rmtree(dst)
        os.symlink(src, dst)


def oracle(what, log):
    env = dict(os.environ, RONDO_ORACLE_OUT=OUT)
    env.pop("RONDO_BOX", None)
    r = subprocess.run([sys.executable, os.path.join(TREE, "tools", "rondo", "oracle.py"), what],
                       capture_output=True, text=True, cwd=TREE, env=env)
    open(os.path.join(WORK, log), "w").write(r.stdout + r.stderr)
    return r


def write(path, fields, rows):
    with open(path, "w", newline="\n", encoding="ascii") as f:
        w = csv.DictWriter(f, fieldnames=fields, lineterminator="\n")
        w.writeheader()
        w.writerows(rows)


def record_a5(rev):
    rows = []
    for name, card in (("a5.csv", 0), ("card_a5.csv", 1)):
        for r in csv.DictReader(open(os.path.join(OUT, name), newline="")):
            rows.append({**r, "card": card, "source": rev})
    fields = ["routine", "card", "bytes", "bytes_run", "calls", "identical", "divergent",
              "unverified", "order_flags", "verdict", "source"]
    write(os.path.join(CONFIG, "gate_a5.csv"), fields, rows)
    return rows


def record_a7(rev, stdout):
    scripts = []
    for line in stdout.splitlines():
        m = re.match(r"(\w+): (.*)$", line)
        if not m or m.group(1) in ("routine", "card"):
            continue
        detail = m.group(2)
        verdict = "IDENTICAL" if "first differing -1" in detail else "DIVERGENT"
        scripts.append({"script": m.group(1), "verdict": verdict, "detail": detail, "source": rev})
    write(os.path.join(CONFIG, "gate_a7.csv"), ["script", "verdict", "detail", "source"], scripts)
    routines = [{**r, "source": rev} for r in csv.DictReader(open(os.path.join(OUT, "a7.csv"), newline=""))]
    write(os.path.join(CONFIG, "gate_a7_routines.csv"), ["routine", "swapped", "ran_inside", "source"], routines)
    return scripts


def main():
    want = sys.argv[1] if len(sys.argv) > 1 else "HEAD"
    r = git("rev-parse", "--short=7", want)
    if r.returncode:
        raise SystemExit(f"{want}: not a commit")
    rev = r.stdout.strip()
    take_lock()
    try:
        export(rev)
        os.makedirs(OUT, exist_ok=True)
        b = subprocess.run(["sh", os.path.join(TREE, "setup.sh"), "pcebox"], capture_output=True, text=True)
        open(os.path.join(WORK, "build.log"), "w").write(b.stdout + b.stderr)
        if b.returncode:
            print(f"pcebox build of {rev} failed; see {os.path.join(WORK, 'build.log')}")
            return 1
        status = 0
        a5 = oracle("a5", "a5.log")
        failed = [ln for ln in a5.stdout.splitlines() if re.match(r"\w+: (exit|timed out)", ln)]
        if a5.returncode or failed:
            print(f"A5 on {rev}: a script run failed, no record written: {failed[:3]}")
            status = 1
        else:
            rows = record_a5(rev)
            tally = {}
            for row in rows:
                tally[row["verdict"]] = tally.get(row["verdict"], 0) + 1
            print(f"A5 on {rev}: {tally}")
        a7 = oracle("a7", "a7.log")
        if re.search(r"^\w+: (record exit|hybrid exit|timed out)", a7.stdout, re.M):
            print(f"A7 on {rev}: a script run failed, no record written")
            status = 1
        else:
            scripts = record_a7(rev, a7.stdout)
            bad = [s["script"] for s in scripts if s["verdict"] != "IDENTICAL"]
            print(f"A7 on {rev}: {len(scripts) - len(bad)} of {len(scripts)} scripts identical"
                  + (f"; not: {' '.join(bad)}" if bad else ""))
            status |= bool(bad)
        return status
    finally:
        if os.path.exists(LOCK):
            os.remove(LOCK)


if __name__ == "__main__":
    sys.exit(main())
