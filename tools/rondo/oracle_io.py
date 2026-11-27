#!/usr/bin/env python3
"""oracle_io.py -- the one place that runs the oracles for a script and reads what they report.

    python3 tools/rondo/oracle_io.py job TREE SCRIPT OUT BOX     (one process per script)

A job runs A5 and then A7 for ONE script with the tree's own oracle.py, into the private folder OUT,
with the private pcebox BOX, and writes OUT/result.json:

    {"script", "a5": {routine: {calls, identical, divergent, unverified, order_flags, detail,
                                 defects: {KIND: n}, causes: {cause: n}}},
     "a7": {"verdict", "code", "detail", "fields": {label: n}}, "reach": {routine: [swapped, ran_inside]},
     "a7_defects": {routine: {KIND: n}}, "secs": {"a5", "a7"}, "errors": [text, ...]}

Nothing else in the pipeline parses oracle output: when the checker's report or verdict names change,
this file is the only one to change (the shape above is what runner.py, records.py and ledger.py use).
A run that failed (an exit, a timeout) puts its text in "errors" and leaves that gate's part empty:
the runner then writes no record for the script.
Reads the oracle as tools/rondo/pcebox/REPORT_FORMAT.md describes it (gate epoch 2): a5.csv and card_a5.csv,
a5_causes.csv (the nine defect kinds and the eight causes, summed over the script), <script>.a5.txt detail lines,
a7.csv, a7_defects.csv (defects under the innermost tagged routine) and the "<script>: hybrid: ..." summary line.
A7's verdict comes from the line's "first differing" code: -1 IDENTICAL (and only if the line's own counts agree),
a frame number >= 1 DIVERGENT, -2 DEFECTS (access defects, every frame identical), -3 STOPPED, -5 NOTVERDICT; -4 (a
diagnostic run) is never a verdict: it is an error, as is a line the reader cannot read. A routine with any call not
judged is not identical (the a5 counts say so: unverified > 0 or identical = 0).
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

def run_oracle(tree, what, script, out, box):
    env = dict(os.environ, RONDO_ORACLE_OUT=out, RONDO_BOX=box)
    return subprocess.run([sys.executable, os.path.join(tree, "tools", "rondo", "oracle.py"), what, script],
                          capture_output=True, text=True, cwd=tree, env=env)


KINDS = ("MOVED", "EXTRA", "MISSING", "VALUE", "STACK", "STATE", "REGS", "CALL", "INT")
CAUSES = ("left", "log", "depth", "computed", "arrive_branch", "arrive_nonres", "runaway", "prefix", "noreturn", "dispatch")
LABELS = (("swapped", r"(-?\d+) swapped"), ("compared", r"frames compared (-?\d+)"), ("identical", r"identical (-?\d+)"),
          ("code", r"first differing (-?\d+)"), ("defects", r"access defects (-?\d+)"),
          ("interrupts", r"interrupts inside (-?\d+)"), ("originals", r"original calls (-?\d+)"),
          ("waits", r"waits (-?\d+)"), ("expected", r"frames expected (-?\d+)"), ("parked", r"parked (-?\d+)"),
          ("kept", r"kept (-?\d+)"), ("unreached", r"unreached (-?\d+)"), ("unopened", r"unopened arrivals (-?\d+)"),
          ("diagnostic", r"diagnostic (-?\d+)"))


def read_a5(out, script, stdout):
    rows = {}
    for name in ("a5.csv", "card_a5.csv"):
        path = os.path.join(out, name)
        for r in csv.DictReader(open(path, newline="")):
            if int(r["calls"]):
                rows[r["routine"]] = {k: int(r[k]) for k in
                                      ("calls", "identical", "divergent", "unverified", "order_flags")}
                rows[r["routine"]].update(detail="", defects={}, causes={})
    for name in ("a5_causes.csv", "a5_causes_card_a5.csv"):
        path = os.path.join(out, name)
        if not os.path.exists(path):
            continue
        for r in csv.DictReader(open(path, newline="")):
            if r["routine"] in rows:
                rows[r["routine"]]["defects"].update({k: int(r[k]) for k in KINDS if int(r[k])})
                rows[r["routine"]]["causes"].update({k: int(r[k]) for k in CAUSES if int(r[k])})
    detail = os.path.join(out, "a5", script + ".a5.txt")
    best = {}                       # a verdict line beats a defect line
    if os.path.exists(detail):
        for line in open(detail, encoding="ascii", errors="replace"):
            f = line.split()
            if f and f[0] != "TAG" and f[0] in rows:
                rank = 0 if f[1:2] == ["DIVERGENT"] else 1 if f[1:2] == ["UNVERIFIED"] else 2
                if f[0] not in best or rank < best[f[0]][0]:
                    best[f[0]] = (rank, re.sub(r"\s+", " ", line.strip())[:200])
    for name, (_, text) in best.items():
        rows[name]["detail"] = text
    return rows


def read_tags(out, script):
    """Per tag (a drafted routine's label too) the A5 counts of the script: the TAG lines of <script>.a5.txt, for a
    box whose routines are not all in a5.csv (a5.csv lists the filed routines only)."""
    rows = {}
    path = os.path.join(out, "a5", script + ".a5.txt")
    for line in open(path, encoding="ascii", errors="replace"):
        f = line.split()
        if f and f[0] == "TAG" and int(f[2]):
            rows[f[1]] = dict(zip(("calls", "identical", "divergent", "unverified", "order_flags"), map(int, f[2:7])),
                              detail="", defects={}, causes={})
    return rows


def read_a7_defects(out):
    d = {}
    path = os.path.join(out, "a7_defects.csv")
    if os.path.exists(path):
        for r in csv.DictReader(open(path, newline="")):
            d.setdefault(r["routine"], {})[r["kind"]] = int(r["defects"])
    return d


def read_a7(out, script, stdout):
    """(verdict, code, detail, fields, reach) or raises ValueError with what is wrong with the output."""
    line = next((ln for ln in stdout.splitlines() if ln.startswith(script + ":")), "")
    if "hybrid:" not in line:
        raise ValueError("no hybrid summary line" + (": " + line[:200] if line else ""))
    f = {}
    for key, rx in LABELS:
        m = re.search(rx, line)
        if not m:
            raise ValueError(f"summary line has no '{key}' field: {line[:200]}")
        f[key] = int(m.group(1))
    code = f["code"]
    if f["diagnostic"] or code == -4:
        raise ValueError("a diagnostic run is never a verdict")
    verdict = {-1: "IDENTICAL", -2: "DEFECTS", -3: "STOPPED", -5: "NOTVERDICT"}.get(code, "DIVERGENT" if code >= 1 else None)
    if verdict is None:
        raise ValueError(f"unknown first-differing code {code}")
    if verdict == "IDENTICAL" and not (f["compared"] == f["expected"] == f["identical"] and not f["defects"]
                                       and not f["unreached"]):
        raise ValueError("code -1 but the line's own counts do not say identical: " + line[:200])
    note = f"{f['compared']} of {f['expected'] if f['expected'] >= 0 else 'unknown'} frames compared, {f['identical']} identical"
    if verdict == "DIVERGENT":
        note = f"first differing frame {code}; " + note
    detail = f"{verdict}: {note}; access defects {f['defects']}; parked {f['parked']}, kept {f['kept']}, " \
             f"unreached {f['unreached']}, unopened arrivals {f['unopened']}"
    stop = next((ln for ln in stdout.splitlines() if ln.startswith("STOPPED ")), "")
    if stop:
        detail += "; " + stop[:160]
    reach = {}
    path = os.path.join(out, "a7.csv")
    if os.path.exists(path):
        for r in csv.DictReader(open(path, newline="")):
            reach[r["routine"]] = [int(r["swapped"]), int(r["ran_inside"])]
    return verdict, code, detail, f, reach


def job(tree, script, out, box, mode="both"):
    """mode a5, a7 or both: a box with drafts serving is measured in one gate at a time."""
    os.makedirs(out, exist_ok=True)
    result = {"script": script, "a5": {}, "a7": {}, "reach": {}, "a7_defects": {}, "secs": {"a5": 0, "a7": 0},
              "errors": []}
    if mode in ("a5", "both", "d5"):
        t0 = time.time()
        r = run_oracle(tree, "a5", script, os.path.join(out, "a5run"), box)
        result["secs"]["a5"] = round(time.time() - t0, 1)
        bad = re.search(rf"^{script}: (exit|timed out).*$", r.stdout, re.M)
        if r.returncode or bad:
            result["errors"].append("a5: " + (bad.group(0) if bad else r.stderr.strip()[-300:] or f"exit {r.returncode}"))
        else:
            result["a5"] = (read_tags if mode == "d5" else read_a5)(os.path.join(out, "a5run"), script) \
                if mode == "d5" else read_a5(os.path.join(out, "a5run"), script, r.stdout)
    if mode in ("a7", "both"):
        t0 = time.time()
        r = run_oracle(tree, "a7", script, os.path.join(out, "a7run"), box)
        result["secs"]["a7"] = round(time.time() - t0, 1)
        hard = re.search(rf"^{script}: (record exit|hybrid exit|timed out).*$", r.stdout, re.M)
        open(os.path.join(out, "a7.stdout.txt"), "w").write(r.stdout)
        if hard:
            result["errors"].append("a7: " + hard.group(0))
        else:
            try:
                verdict, code, detail, fields, reach = read_a7(os.path.join(out, "a7run"), script, r.stdout)
                result["a7"] = {"verdict": verdict, "code": code, "detail": detail, "fields": fields}
                result["reach"] = reach
                result["a7_defects"] = read_a7_defects(os.path.join(out, "a7run"))
            except ValueError as e:
                result["errors"].append("a7: " + str(e) + (" " + r.stderr.strip()[-200:] if r.stderr.strip() else ""))
    json.dump(result, open(os.path.join(out, "result.json"), "w"), indent=1)
    print(f"{script}: a5 {result['secs']['a5']} s, a7 {result['secs']['a7']} s, "
          f"{'errors: ' + '; '.join(result['errors']) if result['errors'] else 'ok'}")
    return 1 if result["errors"] else 0


def main():
    if len(sys.argv) in (6, 7) and sys.argv[1] == "job":
        return job(os.path.abspath(sys.argv[2]), sys.argv[3], os.path.abspath(sys.argv[4]),
                   os.path.abspath(sys.argv[5]), sys.argv[6] if len(sys.argv) == 7 else "both")
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
