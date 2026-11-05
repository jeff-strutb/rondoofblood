#!/usr/bin/env python3
"""hybrid.py -- the table of routines with C, for the oracles.

Every @routine must have an abi_ wrapper of its name. Writes
build/rondo/pcebox/hybrid_table.c: run address, disc location, length, name and
wrapper of each, and whether its C can run on the live machine: not if it reaches
call_original or wait_interrupt, which only the per-call oracle can answer.

    python3 tools/rondo/hybrid.py
"""
import glob
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import asmfile  # noqa: E402

TOP = asmfile.TOP
OUT = os.path.join(TOP, "build", "rondo", "pcebox", "hybrid_table.c")
FUNC = re.compile(r"^(?:static\s+)?[\w\s\*]+?\b(\w+)\s*\([^;{}]*\)\s*\n\{", re.M)
CALL = re.compile(r"\b(\w+)\s*\(")
ORIGINAL = {"call_original", "wait_interrupt"}


def function_calls():
    """Each C function's name, with the names it calls."""
    calls = {}
    for path in glob.glob(os.path.join(TOP, "src", "rondo", "**", "*.c"), recursive=True):
        text = open(path, encoding="utf-8").read()
        for m in FUNC.finditer(text):
            depth, i = 1, m.end()
            while depth and i < len(text):
                depth += {"{": 1, "}": -1}.get(text[i], 0)
                i += 1
            calls[m.group(1)] = set(CALL.findall(text[m.end():i]))
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
           'const char *name; void (*abi)(void); int swappable; } Tag;']
    rows = []
    bad = 0
    swappable = 0
    for i, r in enumerate(routines):
        abi = "abi_" + r["name"]
        if abi not in calls:
            print(f"{r['tag']} {r['name']}: no {abi}")
            bad += 1
            continue
        nxt = routines[i + 1]["disc"] if i + 1 < len(routines) else len(buf)
        length = sum(ins[1] for ins in asmfile.span(buf, r, nxt))
        ok = not reaches_original(abi, calls)
        swappable += ok
        out.append(f"void {abi}(void);")
        rows.append(f'    {{ 0x{r["run"]:04X}, {length}, {r["disc"]}, "{r["name"]}", {abi}, {int(ok)} }},')
    out += ["const Tag rondo_tags[] = {"] + rows + ["};",
            f"const unsigned rondo_tag_count = {len(rows)};", ""]
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="ascii", newline="\n") as f:
        f.write("\n".join(out))
    print(f"{len(rows)} routines with C, {swappable} can run on the live machine")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
