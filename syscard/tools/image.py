#!/usr/bin/env python3
"""image.py -- the card's image gate: the filed assembly, in place, against the card's image.

Every routine block under syscard/asm/ is assembled at its ROM offset and run address; every
other byte is included from the image. The result must equal the image, byte for byte.

    python3 syscard/tools/image.py --image CARD.pce --cc65 DIR --out DIR
    python3 syscard/tools/image.py ... --plant    one instruction changed: must fail
"""
import argparse
import glob
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(HERE)
TAG = re.compile(r"^; @card ([0-9A-Fa-f]{2}):([0-9A-Fa-f]{4}) (\w+)\s*$")
BANK = 0x2000
LINKER = """MEMORY { ROM: file = %O, start = 0, size = $1000000, fill = no; }
SEGMENTS { CODE: load = ROM, type = rw; }
"""


def blocks():
    """Every routine of the filed assembly, in ROM order: (offset, run, name, text)."""
    found = []
    for path in glob.glob(os.path.join(TOP, "asm", "*.s")):
        lines = open(path, encoding="ascii").read().split("\n")
        starts = [i for i, ln in enumerate(lines) if TAG.match(ln)]
        for k, i in enumerate(starts):
            tag = TAG.match(lines[i])
            bank, run = int(tag.group(1), 16), int(tag.group(2), 16)
            end = starts[k + 1] if k + 1 < len(starts) else len(lines)
            found.append((bank * BANK + (run & (BANK - 1)), run, tag.group(3),
                          "\n".join(lines[i:end])))
    return sorted(found)


def source(image, plant):
    """An assembly of the whole image: each block between includes of the bytes around it."""
    name = os.path.basename(image)
    size = os.path.getsize(image)
    src = ['.setcpu "huc6280"', '.segment "CODE"', '.include "ram.inc"']
    pieces = blocks()
    if plant:
        pieces = [(o, r, n, t.replace("and #$0F", "and #$0E", 1)) for o, r, n, t in pieces]
    last = "0"
    for i, (offset, run, routine, text) in enumerate(pieces):
        src.append(f'.reloc\n.incbin "{name}", {last}, {offset} - ({last})')
        src += [f".org ${run:04X}", f"block{i}_start:", text, f"block{i}_end:"]
        last = f"{offset} + block{i}_end - block{i}_start"
    src.append(f'.reloc\n.incbin "{name}", {last}, {size} - ({last})')
    return "\n".join(src) + "\n", pieces


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--image", required=True)
    ap.add_argument("--cc65", required=True, help="the folder with ca65 and ld65")
    ap.add_argument("--out", required=True)
    ap.add_argument("--plant", action="store_true")
    args = ap.parse_args()
    os.makedirs(args.out, exist_ok=True)
    text, pieces = source(args.image, args.plant)
    base = os.path.join(args.out, "image")
    with open(base + ".s", "w", encoding="ascii", newline="\n") as f:
        f.write(text)
    with open(base + ".cfg", "w", newline="\n") as f:
        f.write(LINKER)
    subprocess.run([os.path.join(args.cc65, "ca65"), "-I", os.path.join(TOP, "asm", "include"),
                    "--bin-include-dir", os.path.dirname(os.path.abspath(args.image)),
                    "-o", base + ".o", base + ".s"], check=True)
    subprocess.run([os.path.join(args.cc65, "ld65"), "-C", base + ".cfg", "-o", base + ".bin",
                    base + ".o"], check=True)
    want = open(args.image, "rb").read()
    got = open(base + ".bin", "rb").read()
    differing = sum(1 for a, b in zip(want, got) if a != b) + abs(len(want) - len(got))
    print(f"{len(pieces)} routines in place: {len(got)} B, {differing} differing")
    return 1 if differing else 0


if __name__ == "__main__":
    sys.exit(main())
