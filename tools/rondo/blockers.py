#!/usr/bin/env python3
"""blockers.py -- the routines without C that hold up the most other code.

    python3 tools/rondo/blockers.py [PREFIX ...]     the top 25 (only locations starting with a PREFIX)

Bottom up, a routine can be certified only once everything it calls or jumps to has C
(worklist.py). This ranks each routine without C by the bytes of the routines without C that
wait on it, directly or through others, and lists what it waits on itself: file the top first.
Reads build/rondo/routines.csv (catalog.py).
"""
import collections
import sys

import catalog


def main():
    cat = catalog.load()
    done = {k for k, r in cat.items() if r["name"]}
    calls = catalog.callees(cat, catalog.edges(), catalog.CALLS + (catalog.JMP,) + catalog.TABLES)
    need = {k: calls[k] - done - {k} for k in cat if k not in done}
    waiters = collections.defaultdict(set)
    for r, cs in need.items():
        for c in cs:
            waiters[c].add(r)

    def closure(c):
        seen, stack = set(), [c]
        while stack:
            for w in waiters.get(stack.pop(), ()):
                if w not in seen:
                    seen.add(w)
                    stack.append(w)
        return seen

    rank = sorted(((sum(cat[w]["bytes"] for w in closure(c)), len(closure(c)), c) for c in need), reverse=True)
    want = sys.argv[1:]
    shown = 0
    for b, k, c in rank:
        where = cat[c]["where"]
        if want and not any(where.startswith(w) for w in want):
            continue
        waits = " ".join(cat[x]["where"] for x in sorted(need[c]))
        print(f"{c[0]} {where:10} {cat[c]['bytes']:5} B own, blocks {b:6} B in {k:3} routines; waits on {waits[:80]}")
        shown += 1
        if shown >= 25:
            break
    return 0


if __name__ == "__main__":
    sys.exit(main())
