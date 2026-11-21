#!/usr/bin/env python3
"""oracle_io.py -- the one place that runs the oracles for a script and reads what they report.

    python3 tools/rondo/oracle_io.py job TREE SCRIPT OUT BOX     (under cpu-run: one claim per script)

A job runs A5 and then A7 for ONE script with the tree's own oracle.py, into the private folder OUT,
with the private pcebox BOX, and writes OUT/result.json:

    {"script", "a5": {routine: {calls, identical, divergent, unverified, order_flags, detail}},
     "a7": {"verdict", "detail"}, "reach": {routine: [swapped, ran_inside]},
     "secs": {"a5", "a7"}, "errors": [text, ...]}

Nothing else in the pipeline parses oracle output: when the checker's report or verdict names change,
this file is the only one to change (the shape above is what runner.py, records.py and ledger.py use).
A run that failed (an exit, a timeout) puts its text in "errors" and leaves that gate's part empty:
the runner then writes no record for the script.
Reads the oracle as it is today: a5.csv and card_a5.csv, <script>.a5.txt detail lines, a7.csv and the
"<script>: hybrid: ... first differing -1" line.
"""
import csv
import json
import os
import re
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import cpuguard  # noqa: E402

A7_IDENTICAL = "first differing -1"


def run_oracle(tree, what, script, out, box):
    env = dict(os.environ, RONDO_ORACLE_OUT=out, RONDO_BOX=box)
    return subprocess.run([sys.executable, os.path.join(tree, "tools", "rondo", "oracle.py"), what, script],
                          capture_output=True, text=True, cwd=tree, env=env)


def read_a5(out, script, stdout):
    rows = {}
    for name in ("a5.csv", "card_a5.csv"):
        path = os.path.join(out, name)
        for r in csv.DictReader(open(path, newline="")):
            if int(r["calls"]):
                rows[r["routine"]] = {k: int(r[k]) for k in
                                      ("calls", "identical", "divergent", "unverified", "order_flags")}
                rows[r["routine"]]["detail"] = ""
    detail = os.path.join(out, "a5", script + ".a5.txt")
    if os.path.exists(detail):
        for line in open(detail, encoding="ascii", errors="replace"):
            f = line.split()
            if f and f[0] != "TAG" and f[0] in rows and not rows[f[0]]["detail"]:
                rows[f[0]]["detail"] = re.sub(r"\s+", " ", line.strip())[:200]
    return rows


def read_a7(out, script, stdout):
    line = next((ln for ln in stdout.splitlines() if ln.startswith(script + ":")), "")
    detail = line.split(":", 1)[1].strip() if line else "no result"
    reach = {}
    path = os.path.join(out, "a7.csv")
    if os.path.exists(path):
        for r in csv.DictReader(open(path, newline="")):
            reach[r["routine"]] = [int(r["swapped"]), int(r["ran_inside"])]
    return detail, reach


def job(tree, script, out, box):
    os.makedirs(out, exist_ok=True)
    result = {"script": script, "a5": {}, "a7": {}, "reach": {}, "secs": {}, "errors": []}
    t0 = time.time()
    r = run_oracle(tree, "a5", script, os.path.join(out, "a5run"), box)
    result["secs"]["a5"] = round(time.time() - t0, 1)
    bad = re.search(rf"^{script}: (exit|timed out).*$", r.stdout, re.M)
    if r.returncode or bad:
        result["errors"].append("a5: " + (bad.group(0) if bad else r.stderr.strip()[-300:] or f"exit {r.returncode}"))
    else:
        result["a5"] = read_a5(os.path.join(out, "a5run"), script, r.stdout)
    t0 = time.time()
    r = run_oracle(tree, "a7", script, os.path.join(out, "a7run"), box)
    result["secs"]["a7"] = round(time.time() - t0, 1)
    bad = re.search(rf"^{script}: (record exit|hybrid exit|timed out).*$", r.stdout, re.M)
    if r.returncode and not bad and A7_IDENTICAL not in r.stdout:
        bad = re.search(r".*", r.stderr.strip()[-300:] or f"exit {r.returncode}")
    if bad:
        result["errors"].append("a7: " + bad.group(0))
    else:
        detail, reach = read_a7(os.path.join(out, "a7run"), script, r.stdout)
        result["a7"] = {"verdict": "IDENTICAL" if A7_IDENTICAL in detail else "DIVERGENT", "detail": detail}
        result["reach"] = reach
    json.dump(result, open(os.path.join(out, "result.json"), "w"), indent=1)
    print(f"{script}: a5 {result['secs']['a5']} s, a7 {result['secs']['a7']} s, "
          f"{'errors: ' + '; '.join(result['errors']) if result['errors'] else 'ok'}")
    return 1 if result["errors"] else 0


def main():
    cpuguard.require("oracle_io.py", jobs=1, mem="2G")
    if len(sys.argv) == 6 and sys.argv[1] == "job":
        return job(os.path.abspath(sys.argv[2]), sys.argv[3], os.path.abspath(sys.argv[4]),
                   os.path.abspath(sys.argv[5]))
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
