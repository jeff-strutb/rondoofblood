#!/usr/bin/env python3
"""tables.py -- the disc tables the C reads, with the bank each is read under settled from the traces.

    python3 tools/rondo/tables.py            fill config/rondo/tables.csv's columns from the traces, write tables.h
    python3 tools/rondo/tables.py --check    exit 1 when tables.csv or tables.h is not what the traces say

A disc table is data the original keeps in code or in a block the loader brought in: a table of handler
addresses, offsets, tile numbers. The C reads one through a generated reader, never by a bare address (docs/rondo/
PLAN.md, section 1). config/rondo/tables.csv names each one; the hand columns are

    name     the table's symbol in symbols.csv (its logical address is that symbol's: one list of names)
    len      its bytes
    elem     u8, u16le or u16be: what one element of it is
    readers  the filed routines that read it, joined by |, by name
    note     what it holds

and the tool's columns, which no person writes (`bank` least of all):

    bank     the physical bank the table was in every time one of its readers read it (hex), blank when none was
    disc     the lowest disc offset (hex) the table's first byte came from; the same bytes are often kept at
             several places on the disc (copies)
    copies   how many places
    reads    how many read runs of the traces cover it, by those readers
    status   SETTLED (one bank), OVERLAY (two or more banks: one row per overlay is needed), UNREAD (no script has
             reached it: no reader is generated until one does)

The traces' reads.csv (build/rondo/trace, the main checkout's) says which disc bytes an instruction loaded from RAM, at
which logical address and in which physical bank. The tool takes the runs whose reading instruction lies inside a named
reader routine (its disc range, from the routine table) and whose RAM bytes overlap the table, and reads the bank
and the disc place off them. It writes src/rondo/include/tables.h: for each SETTLED table
NAME_TBL (its address), NAME_TBL_BANK, NAME_TBL_LEN and NAME_TBL_ELEM, which the reader macros of access.h
(TBL8, TBL16_LH, TBL16_HL) use. A table with no such macros has no reader: using it is a compile error.
"""
import bisect
import collections
import csv
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
CSV = os.path.join(TOP, "config", "rondo", "tables.csv")
SYMBOLS = os.path.join(TOP, "config", "rondo", "symbols.csv")
HEADER = os.path.join(TOP, "src", "rondo", "include", "tables.h")
HAND = ["name", "len", "elem", "readers", "note"]
TOOL = ["bank", "disc", "copies", "reads", "status"]
ELEMS = {"u8": 1, "u16le": 2, "u16be": 2}


def main_checkout():
    r = subprocess.run(["git", "-C", TOP, "rev-parse", "--path-format=absolute", "--git-common-dir"],
                       capture_output=True, text=True)
    return os.path.dirname(r.stdout.strip()) if r.returncode == 0 else TOP


def symbols():
    out = {}
    for line in open(SYMBOLS, encoding="ascii"):
        line = line.strip()
        if not line or line.startswith("#") or line.startswith("addr,"):
            continue
        addr, kind, name, _ = line.split(",", 3)
        out[name] = int(addr, 16) + (0x2000 if kind == "zp" else 0)
    return out


def routine_ranges():
    """{routine name: (disc offset, length)} from the tree's routine table (hybrid.py writes it)."""
    p = os.path.join(TOP, "build", "rondo", "pcebox", "hybrid_table.c")
    newest = 0
    for dp, _, fs in os.walk(os.path.join(TOP, "src")):
        newest = max([newest] + [os.path.getmtime(os.path.join(dp, f)) for f in fs if f.endswith(".c")])
    if not os.path.exists(p) or os.path.getmtime(p) < newest:
        r = subprocess.run([sys.executable, os.path.join(HERE, "hybrid.py")], cwd=TOP, capture_output=True, text=True)
        if r.returncode:
            raise SystemExit("tables.py: hybrid.py failed:\n" + (r.stdout + r.stderr)[-500:])
    text = open(p).read()
    return {m.group(3): (int(m.group(2)), int(m.group(1)))
            for m in re.finditer(r'\{ 0x[0-9A-F]+, (\d+), (\d+), "(\w+)", abi_\w+, \d+, 0x[0-9A-F]+, 0 \},', text)}


def reads_file():
    for base in (TOP, main_checkout()):
        p = os.path.join(base, "build", "rondo", "trace", "reads.csv")
        if os.path.exists(p):
            return p
    raise SystemExit("tables.py: no build/rondo/trace/reads.csv (the trace writes it)")


def load_tables():
    rows = list(csv.DictReader(open(CSV, newline="", encoding="ascii")))
    for r in rows:
        for c in HAND + TOOL:
            r.setdefault(c, "")
    return rows


def measure(rows, syms, ranges):
    """Fills each row's tool columns from reads.csv."""
    wanted = []        # (reader disc start, end, table index)
    for k, r in enumerate(rows):
        if r["name"] not in syms:
            raise SystemExit(f"tables.py: {r['name']} is not a symbol in symbols.csv")
        if r["elem"] not in ELEMS:
            raise SystemExit(f"tables.py: {r['name']}: elem {r['elem']!r} is not one of {', '.join(ELEMS)}")
        for rd in [x for x in r["readers"].split("|") if x]:
            if rd not in ranges:
                raise SystemExit(f"tables.py: {r['name']}: reader {rd} is not a filed routine of the tree")
            d, n = ranges[rd]
            wanted.append((d, d + n, k))
    wanted.sort()
    starts = [w[0] for w in wanted]
    maxlen = max([w[1] - w[0] for w in wanted] + [1])
    banks = [collections.Counter() for _ in rows]
    places = [set() for _ in rows]
    nreads = [0] * len(rows)
    with open(reads_file()) as f:
        next(f)
        for line in f:
            parts = line.rstrip("\n").split(",")
            rd = parts[2]
            if not rd.startswith("D:"):
                continue
            rdisc = int(rd[2:], 16)
            i = bisect.bisect_right(starts, rdisc) - 1
            while i >= 0 and wanted[i][0] > rdisc - maxlen:
                lo, hi, k = wanted[i]
                i -= 1
                if not (lo <= rdisc < hi):
                    continue
                ds, ln, logical, bank = int(parts[0], 16), int(parts[1]), int(parts[4], 16), parts[5]
                a, n = syms[rows[k]["name"]], int(rows[k]["len"])
                if logical < a + n and a < logical + ln:
                    banks[k][bank] += 1
                    places[k].add(ds - (logical - a))
                    nreads[k] += 1
    for k, r in enumerate(rows):
        if not banks[k]:
            r.update(bank="", disc="", copies="0", reads="0", status="UNREAD")
        else:
            r.update(bank=banks[k].most_common(1)[0][0] if len(banks[k]) == 1 else "+".join(sorted(banks[k])),
                     disc="%X" % min(places[k]), copies=str(len(places[k])), reads=str(nreads[k]),
                     status="SETTLED" if len(banks[k]) == 1 else "OVERLAY")


def csv_text(rows):
    import io
    out = io.StringIO()
    w = csv.writer(out, lineterminator="\n")
    w.writerow(HAND + TOOL)
    for r in rows:
        w.writerow([r[c] for c in HAND + TOOL])
    return out.getvalue()


def header_text(rows, syms):
    out = ["/* generated by tools/rondo/tables.py from config/rondo/tables.csv and the traces; do not edit */",
           "#ifndef RONDO_TABLES_H", "#define RONDO_TABLES_H", "", '#include "ram.h"', "",
           "/* A table that has these macros was read by a script, in one bank. TBL8 and TBL16_* (access.h) use them;",
           " * a table with no macros here has no reader. */", ""]
    for r in rows:
        n = r["name"]
        if r["status"] == "SETTLED":
            out += [f"/* {n}: {r['len']} bytes of {r['elem']}, bank {r['bank']}, {r['copies']} place(s) on the disc from {r['disc']}"
                    f"{'; ' + r['note'] if r['note'] else ''} */",
                    f"#define {n}_TBL {n}", f"#define {n}_TBL_BANK 0x{r['bank']}", f"#define {n}_TBL_LEN {r['len']}",
                    f"#define {n}_TBL_ELEM {ELEMS[r['elem']]}",
                    f"_Static_assert({n} + {r['len']} <= 0x10000, \"{n} runs past the address space\");", ""]
        else:
            out += [f"/* {n}: {r['status']}: no reader until a script reads it in one bank */", ""]
    out += ["#endif", ""]
    return "\n".join(out)


def main():
    check = "--check" in sys.argv[1:]
    rows = load_tables()
    syms = symbols()
    measure(rows, syms, routine_ranges())
    ctext, htext = csv_text(rows), header_text(rows, syms)
    if check:
        same = (open(CSV, encoding="ascii").read() == ctext and os.path.exists(HEADER)
                and open(HEADER, encoding="ascii").read() == htext)
        print("tables.csv and tables.h are current" if same else "tables.csv or tables.h is not what the traces say; run tables.py")
        return 0 if same else 1
    with open(CSV, "w", encoding="ascii", newline="\n") as f:
        f.write(ctext)
    with open(HEADER, "w", encoding="ascii", newline="\n") as f:
        f.write(htext)
    c = collections.Counter(r["status"] for r in rows)
    print(f"{len(rows)} tables: " + ", ".join(f"{v} {k}" for k, v in sorted(c.items())))
    for r in rows:
        print(f"  {r['name']}: {r['status']} bank {r['bank'] or '-'} disc {r['disc'] or '-'} copies {r['copies']} reads {r['reads']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
