#!/usr/bin/env python3
"""asmbuild.py -- assemble the filed assembly in place in the whole data track.

Every routine block under asm/rondo/<area>/ is placed at its disc location, run
at its @run address; every other byte of track 02 is included from the disc.
Writes build/rondo/asm/image.bin. Assembling only: comparing the result with the
disc is the image gate's job.

    python3 tools/rondo/asmbuild.py       (needs the Linux ca65 and ld65 under WSL)
"""
import glob
import os
import re
import subprocess

TOP = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
OUT = os.path.join(TOP, "build", "rondo", "asm")
TRACK = os.path.join(TOP, "build", "rondo", "disc", "track02.bin")
# ca65 joins every file name to a search path with "/", so an absolute name
# never opens: names are relative to the root it is given.
TRACK_REL = "build/rondo/disc/track02.bin"
SECTOR = 2048
TAG = re.compile(r"@routine\s+([0-9A-Fa-f]{4}):([0-9A-Fa-f]{3})\s+(\w+)")
RUN = re.compile(r"@run\s+[0-9A-Fa-f]{2}:([0-9A-Fa-f]{4})")
LINKER = """MEMORY { DISC: file = %O, start = 0, size = $1400000, fill = no; }
SEGMENTS { CODE: load = DISC, type = rw; }
"""


def blocks():
    """Every routine block of the filed assembly: (disc offset, run, text)."""
    found = []
    pattern = os.path.join(TOP, "asm", "rondo", "**", "*.s")
    for path in glob.glob(pattern, recursive=True):
        if os.sep + "raw" + os.sep in path:
            continue
        lines = open(path, encoding="ascii").read().split("\n")
        starts = [i for i, ln in enumerate(lines) if TAG.search(ln)]
        for k, i in enumerate(starts):
            lo = i
            while lo > 0 and lines[lo - 1].startswith(";"):
                lo -= 1
            hi = len(lines)
            if k + 1 < len(starts):
                hi = starts[k + 1]
                while lines[hi - 1].startswith(";"):
                    hi -= 1
            text = "\n".join(lines[lo:hi])
            tag = TAG.search(lines[i])
            run = RUN.search(text)
            found.append((int(tag.group(1), 16) * SECTOR + int(tag.group(2), 16),
                          int(run.group(1), 16), text))
    return sorted(found)


def wsl(path):
    drive, rest = os.path.splitdrive(os.path.abspath(path))
    return "/mnt/" + drive[0].lower() + rest.replace("\\", "/")


def main():
    os.makedirs(OUT, exist_ok=True)
    size = os.path.getsize(TRACK)
    src = ['.setcpu "huc6280"', ".segment \"CODE\"",
           '.include "asm/rondo/include/ram.inc"']
    last = 0
    pieces = blocks()
    for disc, run, text in pieces:
        if disc > last:
            src.append(f'.reloc\n.incbin "{wsl(TRACK)}", {last}, {disc - last}')
        src += [f".org ${run:04X}", text]
        # The next piece starts where this one's bytes end on the disc.
        last = disc + end_of(text, run, disc)
    image = os.path.join(OUT, "image.s")
    with open(image, "w", encoding="ascii", newline="\n") as f:
        f.write("\n".join(src) + f'\n.reloc\n.incbin "{wsl(TRACK)}", {last}, {size - last}\n')
    with open(os.path.join(OUT, "image.cfg"), "w", newline="\n") as f:
        f.write(LINKER)
    ca65, ld65 = (wsl(os.path.join(TOP, "tools", "toolchains", "cc65", t)) for t in ("ca65", "ld65"))
    base = wsl(os.path.join(OUT, "image"))
    root = wsl(TOP)
    cmd = f"{ca65} -I {root} --bin-include-dir {root} -o {base}.o {base}.s && {ld65} -C {base}.cfg -o {base}.bin {base}.o"
    subprocess.run(["wsl.exe", "-e", "sh", "-c", cmd], check=True, timeout=120)
    print(f"{len(pieces)} routines assembled in place: {os.path.join(OUT, 'image.bin')}")


def end_of(text, run, disc):
    """The routine's length on the disc, by decoding the disc at its offset."""
    import sys
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import huc
    count = sum(1 for ln in text.split("\n") if ln.startswith("    "))
    with open(TRACK, "rb") as f:
        f.seek(disc)
        buf = f.read(SECTOR * 4)
    off = 0
    for _ in range(count):
        off += huc.decode(buf, off, run + off)[0]
    return off


if __name__ == "__main__":
    main()
