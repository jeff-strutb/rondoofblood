#!/usr/bin/env python3
"""whatif.py -- the M1 and M2 bars a fix would give, worked out from the stored records before the fix.

    python3 tools/rondo/whatif.py [--scripts a,b] [--routines R,S] [--t3 R,S] [--chash FILE] [--list]

  --scripts   assume these scripts' A7 runs identical, and their A5 divergences gone
  --routines  assume these routines' A5 divergences gone (their calls count as identical)
  --t3        assume these routines pass every gate (a proper-C rewrite plus all the rest)
Nothing is run and nothing is written: it recomputes A5 and A7 for every routine from the records
the way ledger.py does, flips what is assumed, and prints the bars before and after, in the
denominator progress.py uses, with every routine that changes tier. State the expected gain in numbers
before making a fix (PLAN.md section 7), then compare with what the runner measures.
"""
import argparse
import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import cpuguard  # noqa: E402
import ledger  # noqa: E402
import progress  # noqa: E402
import records  # noqa: E402

GATES = ("proper_c", "gate_0", "gate_c", "gate_a5", "gate_a7", "symbolic")


def tiers(rows, st, ext, fixed_scripts, fixed_routines, t3):
    rec, chash = st["rec"], st["chash"]
    a5 = records.a5_counts(rec, chash)
    if fixed_scripts or fixed_routines:
        a5 = {k: dict(v) for k, v in a5.items()}
        for r in rec.a5:
            if r["chash"] == chash.get(r["routine"]) and int(r["divergent"]) and r["routine"] in a5 and \
                    (r["script"] in fixed_scripts or r["routine"] in fixed_routines):
                a5[r["routine"]]["divergent"] -= int(r["divergent"])
                a5[r["routine"]]["identical"] += int(r["divergent"])
    stats = records.a7_scripts(rec, chash, ext)
    ran = {}
    for r in rec.a7:
        if r["chash"] == chash.get(r["routine"]) and (stats.get(r["script"], ("", ""))[0] == "current" or
                                                       r["script"] in fixed_scripts) and \
                (int(r["swapped"]) or int(r["ran_inside"])):
            ran[r["routine"]] = ran.get(r["routine"], 0) + 1
    names = records.script_names()
    out = {}
    for row in rows:
        n = row["routine"]
        if n not in ext:
            out[n] = row["tier"]
            continue
        failing = [s for s in names if records.reach(s, n, ext, rec) and s not in fixed_scripts and
                   stats.get(s) != ("current", "IDENTICAL")]
        g = {k: row[k] == "1" for k in GATES}
        t = a5.get(n, {})
        g["gate_a5"] = int(t.get("identical", 0)) > 0 and int(t.get("divergent", 1)) == 0
        g["gate_a7"] = ran.get(n, 0) > 0 and not failing
        if n in t3:
            g.update({k: True for k in GATES[:5]})
        out[n] = ledger.tier_of(g, True)
    return out


def bars(rows, tier):
    m1 = sum(int(r["bytes"]) for r in rows if tier[r["routine"]] in ("T3", "T4"))
    m2 = sum(int(r["bytes"]) for r in rows if tier[r["routine"]] == "T4")
    n1 = sum(1 for r in rows if tier[r["routine"]] in ("T3", "T4"))
    n2 = sum(1 for r in rows if tier[r["routine"]] == "T4")
    return m1, n1, m2, n2


def main():
    cpuguard.require("whatif.py", jobs=4, mem="3G")
    ap = argparse.ArgumentParser()
    ap.add_argument("--scripts", default="")
    ap.add_argument("--routines", default="")
    ap.add_argument("--t3", default="")
    ap.add_argument("--chash")
    ap.add_argument("--list", action="store_true")
    a = ap.parse_args()
    split = lambda s: {x for x in s.split(",") if x}  # noqa: E731
    ledger.load_records(a.chash)
    st = ledger.STATE
    ext = st["ext"]
    missing = records.complete(st["rec"])
    if missing:
        print(f"NOT MEASURED: no result for {len(missing)} script(s): {' '.join(missing)}: "
              "the bars below are floors, not a report")
    rows = [r for r in csv.DictReader(open(progress.LEDGER, newline=""))]
    for s in split(a.scripts):
        if s not in records.script_names():
            raise SystemExit(f"{s}: no such script")
    for r in split(a.routines) | split(a.t3):
        if r not in {x["routine"] for x in rows}:
            raise SystemExit(f"{r}: not in the ledger")
    before = tiers(rows, st, ext, set(), set(), set())
    after = tiers(rows, st, ext, split(a.scripts), split(a.routines), split(a.t3))
    est = progress.ESTIMATE
    for label, t in (("now", before), ("after", after)):
        m1, n1, m2, n2 = bars(rows, t)
        print(f"{label:5} M1 {progress.percent(m1, est)} ({m1:,} B, {n1} routines)   "
              f"M2 {progress.percent(m2, est)} ({m2:,} B, {n2} routines)")
    stored = {r["routine"]: r["tier"] for r in rows}
    drift = sum(1 for k in before if before[k] != stored[k])
    if drift:
        print(f"note: {drift} routine(s) differ between the stored ledger and the records as they stand now "
              "(the ledger predates a record or a change); 'now' is from the records")
    moved = [(k, before[k] or "-", after[k] or "-") for k in sorted(after) if before[k] != after[k]]
    print(f"{len(moved)} routine(s) change tier" + (":" if a.list and moved else ""))
    if a.list:
        for k, x, y in moved:
            print(f"  {k}: {x} -> {y}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
