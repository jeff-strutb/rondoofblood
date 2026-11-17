#!/usr/bin/env python3
"""catalog.py -- the routine catalog, built from the traces' edges.

    python3 tools/rondo/catalog.py      write build/rondo/routines.csv and print a summary

Reads every script's build/rondo/trace/<script>/edges.csv (trace.py, pcebox --trace) and the
merged maps. A routine starts wherever the traces saw code entered as a routine, folded onto
where its bytes came from (D: track 02's user data, C: the System Card):

  call       the target of a JSR or BSR
  table      the target of a JMP (abs,X) or JMP (abs): a dispatch table's entry
  interrupt  an interrupt's entry
  jump       the target of a JMP abs that lands inside another routine's code: a tail call
             (repeated until no jump crosses into the middle of another routine)

Its extent is asmfile.span's, the rule the gates use: decoded from its start until flow ends
with no branch inside still pending, or the next routine's start. Its run address is the
operand of a JSR or JMP abs that reached it, where one did. Code no trace has run is not in the
catalog: the census finds that. Each row says whether the routine has C filed, and how many of
its bytes the traces ran (gate C needs all of them).
"""
import collections
import csv
import glob
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import asmfile  # noqa: E402
import cardfile  # noqa: E402

TRACE = os.path.join(TOP, "build", "rondo", "trace")
OUT = os.path.join(TOP, "build", "rondo", "routines.csv")
CALLS = (0x20, 0x44)
TABLES = (0x6C, 0x7C)
JMP = 0x4C
INTERRUPT = 0x100
SECTOR = 2048


def images():
    return {"D": open(asmfile.IMAGE, "rb").read(), "C": open(cardfile.IMAGE, "rb").read()}


def maps():
    return {"D": open(os.path.join(TRACE, "discmap.bin"), "rb").read(),
            "C": open(os.path.join(TRACE, "cardmap.bin"), "rb").read()}


def edges():
    """Every distinct edge of every script: (kind, (space, offset), (space, offset))."""
    out = set()
    for path in glob.glob(os.path.join(TRACE, "*", "edges.csv")):
        for r in csv.DictReader(open(path, newline="")):
            out.add((int(r["kind"], 16), (r["from_space"], int(r["from"], 16)), (r["to_space"], int(r["to"], 16))))
    return out


def where(space, off, runs):
    """A disc routine as SSSS:OOO (its @routine tag); a card routine as BB:AAAA (its @card tag),
    the bank and its run address, or BB:+OOOO, the offset in the bank, when no call gave one."""
    if space == "D":
        return f"{off // SECTOR:04X}:{off % SECTOR:03X}"
    bank, rest = divmod(off, cardfile.BANK)
    return f"{bank:02X}:{min(runs):04X}" if runs else f"{bank:02X}:+{rest:04X}"


def filed():
    """(space, offset) -> name of every routine with C."""
    out = {("D", r["disc"]): r["name"] for r in asmfile.all_routines()}
    out.update({("C", r["disc"]): r["name"] for r in cardfile.routines()})
    return out


def extent(buf, start, run, nxt):
    """asmfile.span's instructions from start, bounded by the next start; [] if it decodes no code."""
    r = {"disc": start, "run": run if run is not None else 0x4000 | (start & 0x1FFF), "stop": 0, "ends": 0,
         "name": f"{start:X}"}
    try:
        return asmfile.span(buf, r, nxt)
    except SystemExit:
        return []


def build():
    bufs, mps, es = images(), maps(), edges()
    starts = {}                       # (space, offset) -> how
    runs = collections.defaultdict(set)
    for kind, src, dst in es:
        if dst[0] not in bufs:
            continue
        if kind in CALLS:
            starts.setdefault(dst, "call")
        elif kind in TABLES:
            starts.setdefault(dst, "table")
        elif kind == INTERRUPT:
            starts.setdefault(dst, "interrupt")
        if kind in (0x20, JMP) and src[0] in bufs:
            b = bufs[src[0]]
            runs[dst].add(b[src[1] + 1] | b[src[1] + 2] << 8)
    for key in filed():
        starts.setdefault(key, "filed")
    jumps = [(s, d) for k, s, d in es if k == JMP and d[0] in bufs and s[0] in bufs]

    def spans_of(order):
        out = {}
        for i, key in enumerate(order):
            nxt = order[i + 1][1] if i + 1 < len(order) and order[i + 1][0] == key[0] else len(bufs[key[0]])
            run = min(runs[key]) if runs[key] else None
            ins = extent(bufs[key[0]], key[1], run, nxt)
            out[key] = (key[1], key[1] + sum(n for _, n, *_ in ins))
        return out

    while True:
        order = sorted(starts)
        spans = spans_of(order)
        owner = {}
        for key, (lo, hi) in spans.items():
            for o in range(lo, hi):
                owner[(key[0], o)] = key
        new = {d for s, d in jumps if d not in starts and d in owner and owner.get(s) != owner[d]}
        if not new:
            break
        for d in new:
            starts[d] = "jump"
    names = filed()
    rows = []
    for key in sorted(starts):
        lo, hi = spans[key]
        mp = mps[key[0]]
        ran = sum(1 for o in range(lo, hi) if mp[o] & 3)
        rows.append({"space": key[0], "offset": f"{lo:X}", "where": where(key[0], key[1], runs[key]),
                     "run": "/".join(f"{r:04X}" for r in sorted(runs[key])), "entry": starts[key],
                     "bytes": hi - lo, "bytes_run": ran, "name": names.get(key, "")})
    with open(OUT, "w", newline="\n", encoding="ascii") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0]), lineterminator="\n")
        w.writeheader()
        w.writerows(rows)
    return rows, es


def load():
    """The catalog as written: (space, offset) -> row, with offset and bytes as ints."""
    out = {}
    for r in csv.DictReader(open(OUT, newline="")):
        r["offset"], r["bytes"], r["bytes_run"] = int(r["offset"], 16), int(r["bytes"]), int(r["bytes_run"])
        out[(r["space"], r["offset"])] = r
    return out


def callees(cat, es, kinds):
    """(space, offset) of each catalogued routine -> the catalogued routines it transfers to."""
    order = sorted(cat)
    owner = {}
    for key in order:
        for o in range(key[1], key[1] + cat[key]["bytes"]):
            owner[(key[0], o)] = key
    out = collections.defaultdict(set)
    for kind, src, dst in es:
        if kind in kinds and src in owner and dst in cat and dst != owner[src]:
            out[owner[src]].add(dst)
    return out


def main():
    rows, es = build()
    for space, label in (("D", "disc"), ("C", "System Card")):
        mine = [r for r in rows if r["space"] == space]
        if not mine:
            continue
        c = collections.Counter(r["entry"] for r in mine)
        print(f"{label}: {len(mine)} routines, {sum(r['bytes'] for r in mine)} B; "
              f"{sum(1 for r in mine if r['name'])} with C ({sum(r['bytes'] for r in mine if r['name'])} B); "
              f"{sum(1 for r in mine if r['bytes'] and r['bytes'] == r['bytes_run'])} every byte run; "
              + ", ".join(f"{k} {v}" for k, v in sorted(c.items())))
    print(f"{len(es)} distinct edges from {len(glob.glob(os.path.join(TRACE, '*', 'edges.csv')))} scripts")
    return 0


if __name__ == "__main__":
    sys.exit(main())
