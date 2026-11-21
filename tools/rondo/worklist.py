#!/usr/bin/env python3
"""worklist.py -- everything short of the bar, with the gate that fails and the exact reason.

    python3 tools/rondo/worklist.py [--by-script] [--by-routine] [--gate G] [--routine NAME]
                                    [--chash FILE] [--space D|C] [--limit N] [--wave FILE] [--no-c-only]

Regenerated after every landing (the runner does it with each commit). Four parts:
  1. by routine: every filed routine short of T3 (and, with --t4, of T4), one line per failing gate:
       proper C   the faults and the first line of each (file:line)
       gate 0     the bytes of the built image that differ from the disc (needs build/rondo/asm/image.bin)
       gate C     the byte ranges no script ran (SSSS:OOO-OOO)
       A5         UNCOVERED (no script calls it), UNVERIFIED (calls, none judged), DIVERGENT (scripts,
                  counts, the first difference), order flags
       A7         the scripts reaching it that are not identical or not measured, with their first
                  difference; or that no counting script ran its C
       symbolic   (--t4) the assembly lines that still carry a bare address
  2. by script: each script's A7 state and first difference, its divergent A5 routines, and how many
     routines below T3 it reaches;
  3. the catalog check: every catalog entry that falls inside a routine that has C and has no tag of
     its own (a new script's trace can create these and turn passing scripts red; catalog.py is not
     changed by this);
  4. routines with no C yet and ready to decompile, bottom up, smallest first (the old worklist).
Reads docs/rondo/ledger.csv, the records (records.py), the traces and the catalog; compiles for the
hashes (compiled.py) unless --chash FILE. Needs the queue: run it through cpu-run.
"""
import argparse
import csv
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import cpuguard  # noqa: E402
import catalog  # noqa: E402
import ledger  # noqa: E402
import records  # noqa: E402

GATES = ("proper_c", "gate_0", "gate_c", "gate_a5", "gate_a7")


def ledger_rows():
    rows = []
    for path, card in ((os.path.join(TOP, "docs", "rondo", "ledger.csv"), 0), (os.path.join(TOP, "syscard", "ledger.csv"), 1)):
        if os.path.exists(path):
            for r in csv.DictReader(open(path, newline="")):
                r["card"] = card
                rows.append(r)
    return rows


def offset_of(row):
    """The routine's offset in the disc image (game) or the card ROM (card)."""
    a, b = row["where"].split(":")
    return int(a, 16) * 2048 + int(b, 16) if not row["card"] else int(a, 16) * 0x2000 + (int(b, 16) & 0x1FFF)


def c_lines(name, pattern_names):
    """file:line of the first line in the routine's closure that shows each proper-C fault."""
    funcs = {}
    for label, pat in ledger.BAD:
        funcs[label] = pat
    found = {}
    files, calls = ledger.function_files()
    work, seen = [name, "abi_" + name], set()
    while work:
        f = work.pop()
        if f in seen or f not in files or f.startswith("abi_") and f != "abi_" + name or f.endswith("_shim"):
            continue
        seen.add(f)
        path = os.path.join(TOP, files[f])
        text = open(path, encoding="utf-8").read()
        m = re.search(r"^[^\n;]*\b%s\s*\([^;{}]*\)\s*\n\{" % re.escape(f), text, re.M)
        if m:
            start = text[:m.start()].count("\n") + 1
            body = text[m.start():]
            end = body.find("\n}\n") + 3 if "\n}\n" in body else len(body)
            for i, line in enumerate(body[:end].split("\n")):
                for label in pattern_names:
                    if label not in found and funcs[label].search(re.sub(r"/\*.*?\*/|//.*", "", line)):
                        found[label] = f"{files[f]}:{start + i}"
        work += [c for c in calls[f] if not c.endswith("_shim")]
    return found


def ranges(offsets):
    out, lo, prev = [], None, None
    for o in offsets:
        if lo is None:
            lo = prev = o
        elif o == prev + 1:
            prev = o
        else:
            out.append((lo, prev))
            lo = prev = o
    if lo is not None:
        out.append((lo, prev))
    return out


def fmt_range(a, b, card):
    if card:
        return f"{a // 0x2000:02X}:+{a % 0x2000:04X}" + (f"-{b % 0x2000:04X}" if b != a else "")
    return f"{a // 2048:04X}:{a % 2048:03X}" + (f"-{b % 2048:03X}" if b != a else "")


def uncovered(row, maps):
    n, a = int(row["bytes"]), offset_of(row)
    m = maps[row["card"]]
    return fmt_ranges([o for o in range(a, a + n) if not (m[o] & 3)], row["card"]) if m else "no trace map"


def fmt_ranges(offs, card):
    return ", ".join(fmt_range(a, b, card) for a, b in ranges(offs)[:6]) + (" ..." if len(ranges(offs)) > 6 else "")


def image_diff(row, built, want):
    if built is None:
        return "image not built (asmbuild.py)"
    n, a = int(row["bytes"]), offset_of(row)
    return fmt_ranges([o for o in range(a, a + n) if built[o] != want[o]], row["card"]) or "no bytes differ in the span"


def a5_reason(name, st, reach_scripts):
    t = st["a5"].get(name)
    if not t:
        return "UNCOVERED: no counting script called it" + (f"; its trace is in {', '.join(reach_scripts[:4])}" if reach_scripts else "; no script's trace ran it")
    if t["divergent"]:
        return f"DIVERGENT: {t['divergent']} of {t['calls']} calls, first in {t['where']}: {t['detail']}"
    if t["identical"] == 0:
        return (f"UNVERIFIED: {t['calls']} calls in {t['scripts']} script(s), none judged "
                "(each call took an interrupt, crossed a frame or ran original code)")
    return "ok"


def a7_reason(name, st, ext):
    e = st["a7"].get(name)
    if not e:
        return "no counting script ran its C in lockstep (not swapped, not run inside a swapped call)"
    stats = records.a7_scripts(st["rec"], st["chash"], ext)
    bad = []
    for s in e[1]:
        status, verdict = stats.get(s, ("unmeasured", ""))
        row = st["rec"].scripts.get(s, {})
        bad.append(f"{s} ({status if status != 'current' else verdict}" +
                   (f": {row.get('detail', '')[:90]}" if row and status == "current" else "") + ")")
    return "scripts reaching it not passing: " + "; ".join(bad)


def part_routines(args, st, ext, maps, built, want):
    gate_order = ("proper_c", "gate_0", "gate_c", "gate_a5", "gate_a7") + (("symbolic",) if args.t4 else ())
    asm_blocks = {}
    count = 0
    print("== BY ROUTINE")
    for row in ledger_rows():
        if args.routine and row["routine"] != args.routine:
            continue
        if row["tier"] in ("T4",) or (row["tier"] == "T3" and not args.t4):
            continue
        name = row["routine"]
        lines = []
        for g in gate_order:
            if row[g] == "1" or (args.gate and g != args.gate):
                continue
            if g == "proper_c":
                faults = [f for f in row["faults"].split("; ") if f]
                where = c_lines(name, faults) if faults else {}
                lines.append("proper C: " + "; ".join(f"{f} at {where.get(f, '?')}" for f in faults))
            elif g == "gate_0":
                lines.append("gate 0: bytes differ: " + image_diff(row, built[row["card"]], want[row["card"]]))
            elif g == "gate_c":
                lines.append("gate C: not covered: " + uncovered(row, maps))
            elif g == "gate_a5":
                lines.append("A5: " + a5_reason(name, st, [s for s in records.script_names()
                                                           if records.reach(s, name, ext, st["rec"])]))
            elif g == "gate_a7":
                lines.append("A7: " + a7_reason(name, st, ext))
            elif g == "symbolic":
                if not asm_blocks:
                    asm_blocks.update(ledger.assembly_blocks())
                blocks = asm_blocks
                bad = [i + 1 for i, ln in enumerate(blocks.get(name, [])) if re.search(r"(?<!#)\$[0-9A-Fa-f]{2,4}", ln)]
                lines.append(f"symbolic: bare address in {len(bad)} line(s) of its block, first lines {bad[:5]}")
        if lines:
            count += 1
            print(f"{name} {row['where']} {row['bytes']} B {row['tier'] or 'no tier'}" + (" [card]" if row["card"] else ""))
            for ln in lines:
                print("    " + ln)
    print(f"{count} routines short of {'T4' if args.t4 else 'T3'}")


def part_scripts(st, ext):
    print("\n== BY SCRIPT")
    stats = records.a7_scripts(st["rec"], st["chash"], ext)
    below = {r["routine"] for r in ledger_rows() if r["tier"] not in ("T3", "T4") and r["tier"]}
    for s in records.script_names():
        status, verdict = stats.get(s, ("unmeasured", ""))
        row = st["rec"].scripts.get(s, {})
        div = sorted({(r["routine"], r["divergent"]) for r in st["rec"].a5 if r["script"] == s and int(r["divergent"])})
        reached = sum(1 for r in below if r in ext and records.reach(s, r, ext, st["rec"]))
        print(f"{s}: A7 {status if status != 'current' else verdict}"
              + (f" ({row['detail'][:110]})" if status == "current" and verdict != "IDENTICAL" else "")
              + (f"; A5 divergent: {', '.join(f'{r} x{n}' for r, n in div[:6])}" if div else "")
              + f"; reaches {reached} routines below T3")


def part_catalog(cat):
    print("\n== CATALOG ENTRIES INSIDE ROUTINES WITH C (no tag of their own: filing them is the fix)")
    rows = [r for r in ledger_rows() if not r["card"]]
    spans = sorted((offset_of(r), offset_of(r) + int(r["bytes"]), r["routine"]) for r in rows)
    n = 0
    for key, r in sorted(cat.items()):
        if r["name"] or key[0] != "D":
            continue
        for lo, hi, name in spans:
            if lo < key[1] < hi:
                print(f"{r['where']} ({r['entry']}, {r['bytes']} B) falls inside {name} ({lo // 2048:04X}:{lo % 2048:03X}+{hi - lo})")
                n += 1
                break
    print(f"{n} such entries")


def part_nocode(args, cat):
    print("\n== NO C YET, READY TO DECOMPILE (bottom up, smallest first)")
    done = {k for k, r in cat.items() if r["name"]}
    if args.wave:
        wave = {w.split()[0] for w in open(args.wave) if w.strip()}
        done |= {k for k, r in cat.items() if r["where"] in wave}
    calls = catalog.callees(cat, catalog.edges(), catalog.CALLS)
    ready = sorted((r["bytes"], r["where"], key[0], len(calls[key])) for key, r in cat.items()
                   if key not in done and not (args.space and key[0] != args.space) and r["bytes"]
                   and r["bytes_run"] == r["bytes"] and not (calls[key] - done))
    for n, where, space, k in ready[:args.limit]:
        print(f"{space} {where:10} {n:5} B  calls {k}")
    print(f"{len(ready)} ready, {sum(r[0] for r in ready)} B")


def main():
    if "--chash" not in sys.argv and "--catalog" not in sys.argv:           # without a stored compiled-hash file it compiles every C file
        cpuguard.require("worklist.py", jobs=4, mem="3G")
    ap = argparse.ArgumentParser()
    ap.add_argument("--by-script", action="store_true")
    ap.add_argument("--by-routine", action="store_true")
    ap.add_argument("--gate", choices=GATES + ("symbolic",))
    ap.add_argument("--routine")
    ap.add_argument("--t4", action="store_true", help="also routines at T3 short of T4")
    ap.add_argument("--chash")
    ap.add_argument("--space", choices=("D", "C"))
    ap.add_argument("--limit", type=int, default=40)
    ap.add_argument("--wave")
    ap.add_argument("--no-c-only", action="store_true")
    ap.add_argument("--catalog", action="store_true", help="only the catalog check (no records, no compile)")
    a = ap.parse_args()
    if a.catalog:
        if not os.path.exists(catalog.OUT):
            print("no build/rondo/routines.csv: run catalog.py first")
            return 1
        part_catalog(catalog.load())
        return 0
    parts = {"routine": a.by_routine, "script": a.by_script}
    everything = not (a.by_routine or a.by_script or a.no_c_only)
    ledger.load_records(a.chash)
    st = ledger.STATE
    ext = st["ext"]
    missing = records.complete(st["rec"])
    if missing:
        print(f"NOT MEASURED: no result for {len(missing)} script(s): {' '.join(missing)}; "
              "A5 and A7 lines below are incomplete\n")
    maps = {0: None, 1: None}
    for card, name in ((0, "discmap.bin"), (1, "cardmap.bin")):
        p = os.path.join(TOP, "build", "rondo", "trace", name)
        maps[card] = open(p, "rb").read() if os.path.exists(p) else None
    built, want = {0: None, 1: None}, {0: None, 1: None}
    p = os.path.join(TOP, "build", "rondo", "asm", "image.bin")
    if os.path.exists(p) and ledger.image_fresh(p):
        built[0], want[0] = open(p, "rb").read(), open(ledger.oracle.TRACK, "rb").read()
    p = os.path.join(TOP, "build", "rondo", "card", "image.bin")
    if os.path.exists(p) and ledger.card_image_fresh(p):
        built[1], want[1] = open(p, "rb").read(), open(ledger.cardfile.IMAGE, "rb").read()
    if everything or a.by_routine:
        part_routines(a, st, ext, maps, built, want)
    if everything or a.by_script:
        part_scripts(st, ext)
    cat = catalog.load() if os.path.exists(catalog.OUT) else None
    if cat is None:
        print("\n(no build/rondo/routines.csv: run catalog.py for the catalog check and the no-C list)")
        return 0
    if everything:
        part_catalog(cat)
    if everything or a.no_c_only:
        part_nocode(a, cat)
    return 0


if __name__ == "__main__":
    sys.exit(main())
