#!/usr/bin/env python3
"""certqueue.py -- the certification queue: filed routines that hold in lockstep, and the scripts still to pass.

    python3 tools/rondo/certqueue.py            write config/rondo/certification_queue.csv, print the totals

Input: config/rondo/lockstep_held.csv (runner.py served --proper: per script that came out IDENTICAL with the
filed C in place for the routines it lists and drafts for the rest, the filed routines swapped in that run), and each
script's own trace (which scripts reach a routine: a byte of it run). By docs/rondo/PLAN.md section 1 a routine is T3
only when every script that reaches it is identical with its C in place (and A5 identical, proper C, typed access
layer: ledger.py). This file is the queue: a routine certifies the moment its last reaching script is identical
with that routine's C in place. Nothing here weakens the rule; "certified" below means only the lockstep condition
holds for every reaching script, the ledger decides T3.
"""
import csv
import glob
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import asmfile  # noqa: E402
import records  # noqa: E402

HELD = os.path.join(TOP, "config", "rondo", "lockstep_held.csv")
OUT = os.path.join(TOP, "config", "rondo", "certification_queue.csv")


def main():
    held = records.read(HELD)
    by_routine = {}
    for r in held:
        by_routine.setdefault(r["routine"], {"bytes": r["bytes"], "scripts": set()})["scripts"].add(r["script"])
    buf = open(asmfile.IMAGE, "rb").read()
    routines = asmfile.all_routines()
    info = {}
    for i, r in enumerate(routines):
        nxt = routines[i + 1]["disc"] if i + 1 < len(routines) else len(buf)
        info[r["name"]] = (r["disc"], sum(ins[1] for ins in asmfile.span(buf, r, nxt)))
    scripts = records.script_names(TOP)
    maps = {s: open(os.path.join(TOP, "build", "rondo", "trace", s, "discmap.bin"), "rb").read() for s in scripts
            if os.path.exists(os.path.join(TOP, "build", "rondo", "trace", s, "discmap.bin"))}
    done_scripts = sorted({r["script"] for r in held})
    rows, certified, cert_bytes, held_bytes = [], 0, 0, 0
    for name, v in sorted(by_routine.items()):
        disc, length = info.get(name, (None, 0))
        if disc is None:
            continue
        reaching = [s for s, m in maps.items() if any(b & 3 for b in m[disc:disc + length])]
        passed = sorted(set(reaching) & v["scripts"])
        missing = sorted(set(reaching) - v["scripts"])
        n = int(v["bytes"] or length)
        held_bytes += n
        ok = not missing
        certified += ok
        cert_bytes += n if ok else 0
        rows.append({"routine": name, "bytes": n, "scripts_reaching": len(reaching), "identical_with_its_c": len(passed),
                     "scripts_missing": " ".join(missing), "lockstep_condition": "met" if ok else "waiting"})
    with open(OUT, "w", newline="\n") as f:
        w = csv.DictWriter(f, fieldnames=["routine", "bytes", "scripts_reaching", "identical_with_its_c", "scripts_missing",
                                          "lockstep_condition"], lineterminator="\n")
        w.writeheader()
        w.writerows(rows)
    print(f"filed C holding in lockstep on {', '.join(done_scripts) or 'no script'}: {len(rows)} routines, {held_bytes} B; "
          f"every reaching script identical with its C in place: {certified} routines, {cert_bytes} B")
    missing_scripts = {}
    for r in rows:
        for s in r["scripts_missing"].split():
            missing_scripts[s] = missing_scripts.get(s, 0) + 1
    print("scripts holding most routines back:", ", ".join(f"{s} {n}" for s, n in
                                                          sorted(missing_scripts.items(), key=lambda kv: -kv[1])[:8]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
