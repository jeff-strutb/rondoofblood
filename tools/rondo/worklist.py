#!/usr/bin/env python3
"""worklist.py -- routines ready to decompile next, bottom up.

    python3 tools/rondo/worklist.py [--space D|C] [--limit N] [--wave FILE]

A routine is ready when it has no C yet, every byte of its extent ran in the traces (so gate C
can pass), and every catalogued routine it calls (JSR, BSR) already has C. Smallest first.
--wave FILE: the catalog locations listed in FILE (SSSS:OOO or BB:AAAA, one per line, being
filed elsewhere) count as having C, so what they unblock can go in the same batch.
Reads build/rondo/routines.csv (catalog.py).
"""
import argparse
import sys

import catalog


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--space", choices=("D", "C"))
    ap.add_argument("--limit", type=int, default=40)
    ap.add_argument("--wave")
    a = ap.parse_args()
    cat = catalog.load()
    done = {k for k, r in cat.items() if r["name"]}
    if a.wave:
        wave = {w.split()[0] for w in open(a.wave) if w.strip()}
        done |= {k for k, r in cat.items() if r["where"] in wave}
    calls = catalog.callees(cat, catalog.edges(), catalog.CALLS)
    ready = []
    for key, r in cat.items():
        if key in done or (a.space and key[0] != a.space):
            continue
        if not r["bytes"] or r["bytes_run"] != r["bytes"]:
            continue
        if calls[key] - done:
            continue
        ready.append((r["bytes"], r["where"], key[0], len(calls[key])))
    ready.sort()
    for n, where, space, k in ready[:a.limit]:
        print(f"{space} {where:10} {n:5} B  calls {k}")
    print(f"{len(ready)} ready, {sum(r[0] for r in ready)} B")
    return 0


if __name__ == "__main__":
    sys.exit(main())
