#!/usr/bin/env python3
"""Frame dumps (binary PPM, as pcebox writes them) to PNG.

    python3 tools/rondo/ppm2png.py FRAME.ppm [...]    writes FRAME.png beside each
"""
import struct
import sys
import zlib


def read_ppm(path):
    d = open(path, "rb").read()
    parts = d.split(maxsplit=4)
    assert parts[0] == b"P6" and parts[3] == b"255", path
    w, h = int(parts[1]), int(parts[2])
    return w, h, parts[4][:w * h * 3]


def write_png(path, w, h, rgb):
    def chunk(t, b):
        return struct.pack(">I", len(b)) + t + b + struct.pack(">I", zlib.crc32(t + b))
    raw = b"".join(b"\0" + rgb[y * w * 3:(y + 1) * w * 3] for y in range(h))
    png = (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
           + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))
    open(path, "wb").write(png)


if __name__ == "__main__":
    for p in sys.argv[1:]:
        w, h, rgb = read_ppm(p)
        write_png(p[:-4] + ".png", w, h, rgb)
        print(f"{p[:-4]}.png: {w}x{h}")
