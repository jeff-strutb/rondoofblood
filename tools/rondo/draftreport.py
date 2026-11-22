#!/usr/bin/env python3
"""draftreport.py -- the full A5 over the drafts, by routine: verdicts in routines and bytes, and the causes.

    python3 tools/rondo/draftreport.py A5DIR DRAFTSCSV [--out CSV]

A5DIR holds <script>.a5.txt files (oracle.py a5's per-script reports); DRAFTSCSV is cdraft.py's drafts.csv.
A routine's verdict is oracle.py's: UNCOVERED (no call), DIVERGENT (a divergent or access-order call),
UNVERIFIED (a call not judged, or none identical), else IDENTICAL; the counts are summed over the scripts.
The cause of a routine that is not identical is that of its first worst line: DIVERGENT text, else an
access defect kind, else the unverified reason, with numbers taken out so alike causes count together.
"""
import collections
import csv
import glob
import os
import re
import sys

KINDS = ["MOVED", "EXTRA", "MISSING", "VALUE", "STACK", "STATE", "REGS", "CALL", "INT"]
RANK = {"DIVERGENT": 0, "STACK": 1, "VALUE": 2, "MISSING": 3, "EXTRA": 4, "MOVED": 5, "STATE": 6, "REGS": 7,
        "CALL": 8, "INT": 9, "UNVERIFIED": 10}


def norm(text):
    text = re.sub(r"\[in the call of \w+\]", "[in a callee]", text)
    text = re.sub(r"\b[0-9A-Fa-f]{2,}\b", "N", text)
    text = re.sub(r"\d+", "N", text)
    return re.sub(r"\s+", " ", text).strip()[:90]


def main():
    a5dir, draftscsv = sys.argv[1], sys.argv[2]
    out = sys.argv[sys.argv.index("--out") + 1] if "--out" in sys.argv else None
    rows = {r["name"]: r for r in csv.DictReader(open(draftscsv, newline=""))}
    tot = collections.defaultdict(lambda: [0, 0, 0, 0, 0])      # calls identical divergent unverified order
    worst = {}
    files = sorted(glob.glob(os.path.join(a5dir, "*.a5.txt")))
    for path in files:
        for line in open(path, encoding="utf-8", errors="replace"):
            if line.startswith("TAG "):
                f = line.split()
                name, n = f[1], [int(x) for x in f[2:7]]
                t = tot[name]
                for i in range(5):
                    t[i] += n[i]
                continue
            m = re.match(r"(\S+) (DIVERGENT|UNVERIFIED|MOVED|EXTRA|MISSING|VALUE|STACK|STATE|REGS|CALL|INT) call \d+(?: frame \d+)?(?: at \S+)?: ?(.*)$", line)
            if not m:
                continue
            name, kind, text = m.groups()
            key = (RANK[kind], kind + ": " + norm(text) if kind in ("DIVERGENT", "UNVERIFIED") else
                   kind + (" in a callee" if "[in the call of" in text else ""))
            if name not in worst or key[0] < worst[name][0]:
                worst[name] = key
    verdicts = collections.Counter()
    vbytes = collections.Counter()
    causes = collections.Counter()
    cause_bytes = collections.Counter()
    table = []
    for name, r in rows.items():
        t = tot.get(name, [0, 0, 0, 0, 0])
        calls, ident, div, unv, order = t
        if calls == 0:
            v = "UNCOVERED"
        elif div or order:
            v = "DIVERGENT"
        elif unv or ident == 0:
            v = "UNVERIFIED"
        else:
            v = "IDENTICAL"
        space = "card" if r.get("rom") == "1" else "game"
        size = int(r["length"])
        verdicts[(space, v)] += 1
        vbytes[(space, v)] += size
        cause = ""
        if v in ("DIVERGENT", "UNVERIFIED"):
            cause = worst.get(name, (0, "unverified, reason not in the first lines"))[1]
            causes[cause] += 1
            cause_bytes[cause] += size
        table.append([name, r.get("filed", ""), space, size, calls, ident, div, unv, order, v, cause])
    print("scripts read: %d; drafted routines: %d (game %d, card %d)" % (
        len(files), len(rows), sum(1 for r in rows.values() if r.get("rom") != "1"),
        sum(1 for r in rows.values() if r.get("rom") == "1")))
    for space in ("game", "card"):
        print(space + ":")
        for v in ("IDENTICAL", "DIVERGENT", "UNVERIFIED", "UNCOVERED"):
            print("  %-10s %5d routines %8d bytes" % (v, verdicts[(space, v)], vbytes[(space, v)]))
    print("the commonest causes of anything not identical (routines, bytes):")
    for cause, n in causes.most_common(12):
        print("  %5d %8d  %s" % (n, cause_bytes[cause], cause))
    if out:
        with open(out, "w", newline="") as f:
            w = csv.writer(f, lineterminator="\n")
            w.writerow(["name", "filed", "space", "bytes", "calls", "identical", "divergent", "unverified", "order",
                        "verdict", "cause"])
            w.writerows(table)


if __name__ == "__main__":
    main()
