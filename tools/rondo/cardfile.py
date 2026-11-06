#!/usr/bin/env python3
"""cardfile.py -- the System Card's filed routines, for the oracles.

The card is its own project under syscard/. A routine there is `@card BB:AAAA name`: the
ROM bank and the address it runs at. Its offset in the ROM takes the place of a disc offset.
"""
import glob
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import asmfile  # noqa: E402

TOP = asmfile.TOP
IMAGE = os.path.join(TOP, "build", "rondo", "system", "syscard3.pce")
TAG = re.compile(r"@card\s+([0-9A-Fa-f]{2}):([0-9A-Fa-f]{4})\s+(\w+)")
BANK = 0x2000


def routines():
    """Every card routine with C, in ROM order, in the form asmfile.span takes."""
    found = []
    for path in glob.glob(os.path.join(TOP, "syscard", "src", "*.c")):
        for block in re.findall(r"/\*.*?\*/", open(path, encoding="utf-8").read(), re.S):
            m = TAG.search(block)
            if m:
                bank, run = int(m.group(1), 16), int(m.group(2), 16)
                found.append({"disc": bank * BANK + (run & (BANK - 1)), "name": m.group(3),
                              "tag": f"{m.group(1)}:{m.group(2)}", "run": run, "path": path,
                              "ends": 0, "stop": 0})
    return sorted(found, key=lambda r: r["disc"])


def spans():
    """Each card routine with its length in bytes, decoded from the ROM."""
    buf = open(IMAGE, "rb").read()
    found = routines()
    for i, r in enumerate(found):
        nxt = found[i + 1]["disc"] if i + 1 < len(found) else len(buf)
        r["length"] = sum(ins[1] for ins in asmfile.span(buf, r, nxt))
    return found


def coverage():
    """Gate C by card routine: bytes in the span, bytes the trace saw executed."""
    path = os.path.join(TOP, "build", "rondo", "trace", "cardmap.bin")
    if not os.path.exists(path):
        return {}
    ran = open(path, "rb").read()
    return {r["name"]: (r["length"], sum(1 for k in range(r["length"]) if ran[r["disc"] + k] & 3))
            for r in spans()}
