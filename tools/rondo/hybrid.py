#!/usr/bin/env python3
"""hybrid.py -- the table of routines with C, for the oracles.

Every @routine must have an abi_ wrapper of its name. Writes hybrid_table.c (into
$RONDO_GEN, or build/rondo/pcebox when that is unset): run address, disc location, length, name and
wrapper of each, and whether its C is free of wait_interrupt. Lockstep A7 swaps every
routine and runs original code the C calls (call_original), so the flag no longer keeps
a routine out; it names the routines that wait, which RONDO_KEEP_WAITS=1 keeps as
original code in a diagnostic run, since a wait that stands for a spin which reads
memory can not be placed.

    python3 tools/rondo/hybrid.py
"""
import csv
import glob
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import asmfile  # noqa: E402
import cardfile  # noqa: E402

TOP = asmfile.TOP
OUT = os.path.join(os.environ.get("RONDO_GEN") or os.path.join(TOP, "build", "rondo", "pcebox"), "hybrid_table.c")
FUNC = re.compile(r"^(?:static\s+)?[\w\s\*]+?\b(\w+)\s*\([^;{}]*\)\s*\n\{", re.M)
CALL = re.compile(r"\b(\w+)\s*\(")
ORIGINAL = {"wait_interrupt"}
# Drafted C (tools/rondo/cdraft.py): its folder holds drafts.c and drafts.csv. A filed routine is served
# by its draft only when config/rondo/serve_drafts.csv names it (a tracked list, written by hand).
DRAFTS = os.environ.get("RONDO_DRAFTS")
SERVE = os.path.join(TOP, "config", "rondo", "serve_drafts.csv")
CALL_ABI = re.compile(r"\bcall_abi\(\s*(\w+)")


def function_calls():
    """Each C function's name, with the names it calls."""
    calls = {}
    for path in (glob.glob(os.path.join(TOP, "src", "rondo", "**", "*.c"), recursive=True) +
                 glob.glob(os.path.join(TOP, "syscard", "src", "*.c")) +
                 (glob.glob(os.path.join(DRAFTS, "drafts.c")) if DRAFTS else [])):
        # Comments out first, as ledger.py does: prose with a parenthesis in a comment above a
        # function let the pattern run from there to the function's body, filing the body under
        # a word of the comment, so a routine that reaches original code looked swappable.
        text = re.sub(r"/\*.*?\*/", "", open(path, encoding="utf-8").read(), flags=re.S)
        for m in FUNC.finditer(text):
            depth, i = 1, m.end()
            while depth and i < len(text):
                depth += {"{": 1, "}": -1}.get(text[i], 0)
                i += 1
            body = text[m.end():i]
            calls[m.group(1)] = set(CALL.findall(body)) | set(CALL_ABI.findall(body))
    return calls


def reaches_original(name, calls, seen=None):
    seen = seen if seen is not None else set()
    if name in seen:
        return False
    seen.add(name)
    for callee in calls.get(name, ()):
        if callee in ORIGINAL or reaches_original(callee, calls, seen):
            return True
    return False


def main():
    routines = asmfile.all_routines()
    buf = open(asmfile.IMAGE, "rb").read()
    calls = function_calls()
    out = ['#include <stdint.h>', 'typedef struct { uint16_t run; uint16_t len; uint32_t disc; '
           'const char *name; void (*abi)(void); int swappable; uint16_t ends; int rom; } Tag;']
    rows = []
    serve = set()
    if DRAFTS and os.path.exists(SERVE):
        serve = {r["name"] for r in csv.DictReader(open(SERVE, newline=""))}
    bad = 0
    swappable = 0
    for i, r in enumerate(routines):
        abi = "abi_" + r["name"]
        if r["name"] in serve:
            continue
        if abi not in calls:
            print(f"{r['tag']} {r['name']}: no {abi}")
            bad += 1
            continue
        nxt = routines[i + 1]["disc"] if i + 1 < len(routines) else len(buf)
        length = sum(ins[1] for ins in asmfile.span(buf, r, nxt))
        ok = not reaches_original(abi, calls)
        swappable += ok
        out.append(f"void {abi}(void);")
        rows.append(f'    {{ 0x{r["run"]:04X}, {length}, {r["disc"]}, "{r["name"]}", {abi}, {int(ok)}, 0x{r["ends"]:04X}, 0 }},')
    for r in cardfile.spans():
        abi = "abi_" + r["name"]
        if r["name"] in serve:
            continue
        if abi not in calls:
            print(f"{r['tag']} {r['name']}: no {abi}")
            bad += 1
            continue
        ok = not reaches_original(abi, calls)
        swappable += ok
        out.append(f"void {abi}(void);")
        rows.append(f'    {{ 0x{r["run"]:04X}, {r["length"]}, {r["disc"]}, "{r["name"]}", {abi}, {int(ok)}, 0x0000, 1 }},')
    if DRAFTS:
        have = {r["name"] for r in routines if "abi_" + r["name"] in calls}
        have |= {r["name"] for r in cardfile.spans() if "abi_" + r["name"] in calls}
        taken = {m.group(1) for m in (re.search(r'"(\w+)", abi_', x) for x in rows) if m}
        added = 0
        for r in csv.DictReader(open(os.path.join(DRAFTS, "drafts.csv"), newline="")):
            if r["name"] in taken or (r["filed"] and r["filed"] in have and r["filed"] not in serve):
                continue
            abi = r["abi"]
            if abi not in calls:
                print(f"{r['name']}: no {abi}")
                bad += 1
                continue
            ok = not reaches_original(abi, calls)
            swappable += ok
            out.append(f"void {abi}(void);")
            rows.append(f'    {{ 0x{int(r["run"], 16):04X}, {r["length"]}, {r["disc"]}, "{r["name"]}", {abi}, {int(ok)}, 0x0000, {int(r.get("rom") or 0)} }},')
            added += 1
        print(f"{added} drafted routines")
    out +=["const Tag rondo_tags[] = {"] + rows + ["};",
            f"const unsigned rondo_tag_count = {len(rows)};", ""]
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="ascii", newline="\n") as f:
        f.write("\n".join(out))
    print(f"{len(rows)} routines with C, {swappable} can run on the live machine")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
