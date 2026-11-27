#!/usr/bin/env python3
"""asmbuild.py -- assemble the whole data track from source.

Every routine block under asm/rondo/<area>/ is placed at its disc location (@routine) and assembled
at its run address (@run). Every byte no filed routine spans is included from the disc. Writes
build/rondo/asm/image.bin; image.py --user build/rondo/asm/image.bin compares it with the disc.
Refuses a plan in which two filed routines overlap.

    python3 tools/rondo/asmbuild.py
"""
import glob
import os
import re
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import huc  # noqa: E402

TOP = os.path.dirname(os.path.dirname(HERE))
OUT = os.path.join(TOP, "build", "rondo", "asm")
TRACK = os.path.join(TOP, "build", "rondo", "disc", "track02.bin")
SECTOR = 2048
TAG = re.compile(r"@routine\s+([0-9A-Fa-f]{4}):([0-9A-Fa-f]{3})\s+(\w+)")
RUN = re.compile(r"@run\s+[0-9A-Fa-f]{2}:([0-9A-Fa-f]{4})")
LABEL = re.compile(r"([A-Za-z_]\w*):")
LINKER = """MEMORY { DISC: file = %O, start = 0, size = $1400000, fill = no; }
SEGMENTS { CODE: load = DISC, type = rw; }
"""


def blocks():
    """Every routine block of the filed assembly: (disc offset, run, text, name)."""
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
                          int(run.group(1), 16), text, tag.group(3)))
    return sorted(found)


def filed_pieces(disc):
    """The filed routines: disc offset, length on the disc, run address, text, name, label offsets."""
    out = []
    for d, run, text, name in blocks():
        off = 0
        labels = {}
        for ln in text.split("\n"):
            if ln.startswith("    "):
                off += huc.decode(disc, d + off, run + off)[0]
            else:
                m = LABEL.match(ln)
                if m:
                    labels.setdefault(off, m.group(1))
        out.append({"disc": d, "end": off, "run": run, "text": text, "name": name, "labels": labels,
                    "src": "filed"})
    return out


def wsl(path):
    """A Windows path as WSL sees it; on Linux or macOS the path itself."""
    if os.name != "nt":
        return os.path.abspath(path)
    drive, rest = os.path.splitdrive(os.path.abspath(path))
    return "/mnt/" + drive[0].lower() + rest.replace("\\", "/")


def tag(d):
    return f"{d // SECTOR:04X}:{d % SECTOR:03X}"


def plan(filed):
    """Every piece to place, in disc order; refuses an overlap."""
    pieces = sorted(({"disc": f["disc"], "end": f["disc"] + f["end"], "run": f["run"], "text": f["text"],
                      "name": f["name"]} for f in filed), key=lambda x: (x["disc"], x["end"]))
    last = None
    for x in pieces:
        if last and x["disc"] < last["end"]:
            raise SystemExit(f"asmbuild: filed routines overlap: {last['name']} at {tag(last['disc'])}-"
                             f"{tag(last['end'])} and {x['name']} at {tag(x['disc'])}")
        last = x
    return pieces


def main():
    os.makedirs(OUT, exist_ok=True)
    for stale in ("image.bin", "image.o"):         # a refusal must not leave an old image to be compared
        if os.path.exists(os.path.join(OUT, stale)):
            os.remove(os.path.join(OUT, stale))
    disc = open(TRACK, "rb").read()
    size = len(disc)
    pieces = plan(filed_pieces(disc))
    src = ['.setcpu "huc6280"', '.segment "CODE"', '.include "asm/rondo/include/ram.inc"']
    last = 0
    for x in pieces:
        if x["disc"] > last:
            src.append(f'.reloc\n.incbin "{wsl(TRACK)}", {last}, {x["disc"] - last}')
        src += [f".org ${x['run']:04X}", x["text"]]
        last = x["end"]
    image = os.path.join(OUT, "image.s")
    with open(image, "w", encoding="ascii", newline="\n") as f:
        f.write("\n".join(src) + f'\n.reloc\n.incbin "{wsl(TRACK)}", {last}, {size - last}\n')
    with open(os.path.join(OUT, "image.cfg"), "w", newline="\n") as f:
        f.write(LINKER)
    ca65, ld65 = (wsl(os.path.join(TOP, "tools", "toolchains", "cc65", t)) for t in ("ca65", "ld65"))
    base = wsl(os.path.join(OUT, "image"))
    root = wsl(TOP)
    cmd = f"{ca65} -I {root} --bin-include-dir {root} -o {base}.o {base}.s && {ld65} -C {base}.cfg -o {base}.bin {base}.o"
    shell = ["wsl.exe", "-e", "sh", "-c"] if os.name == "nt" else ["sh", "-c"]
    t2 = time.time()
    r = subprocess.run(shell + [cmd], capture_output=True, text=True, timeout=900)
    if r.returncode:
        print((r.stdout + r.stderr)[-1500:])
        return 1
    binp = os.path.join(OUT, "image.bin")
    if not os.path.exists(binp) or os.path.getsize(binp) != size or os.path.getmtime(binp) < t2 - 1:
        print("asmbuild: the linker produced no fresh image.bin of the disc's size")
        return 1
    print(f"{len(pieces)} filed routines assembled in place: {binp}  [ca65 and ld65 {time.time() - t2:.0f} s]")
    return 0


if __name__ == "__main__":
    sys.exit(main())
