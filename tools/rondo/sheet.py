#!/usr/bin/env python3
"""Frame dumps (PPM) laid out as one PNG contact sheet, in the order given.

    python3 tools/rondo/sheet.py OUT.png COLUMNS FRAME.ppm [...]
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ppm2png import read_ppm, write_png  # noqa: E402


def main():
    out, cols, paths = sys.argv[1], int(sys.argv[2]), sys.argv[3:]
    ims = [read_ppm(p) for p in paths]
    w = max(i[0] for i in ims)
    h = max(i[1] for i in ims)
    rows = (len(ims) + cols - 1) // cols
    sheet = bytearray(w * cols * h * rows * 3)
    for k, (iw, ih, rgb) in enumerate(ims):
        ox, oy = (k % cols) * w, (k // cols) * h
        for y in range(ih):
            at = ((oy + y) * w * cols + ox) * 3
            sheet[at:at + iw * 3] = rgb[y * iw * 3:(y + 1) * iw * 3]
    write_png(out, w * cols, h * rows, bytes(sheet))
    print(f"{out}: {len(ims)} frames, {cols}x{rows}")


if __name__ == "__main__":
    main()
