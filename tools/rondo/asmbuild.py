#!/usr/bin/env python3
"""asmbuild.py -- assemble the whole data track from source: the raw intake and the filed assembly.

Every routine block under asm/rondo/<area>/ is placed at its disc location and run at its @run
address, and so is every exact copy of it that the traces found (a filed routine is filed once and
serves every location holding its bytes). Every other traced routine comes from the raw intake
(intake.py, regenerated here on every run from the traces and never tracked), so the filed
assembly always wins: the intake drops each filed range and each copy of it. Every byte no trace
ran and no routine spans is included from the disc. Writes build/rondo/asm/image.bin.
Assembling only: comparing the result with the disc is the image gate's job. Before assembling it
refuses a plan in which two pieces overlap or a byte a script ran is in no piece.

    cpu-run -j 1 -m 3G -- python3 tools/rondo/asmbuild.py [--raw DIR] [--control NAME]

--raw DIR also writes the readable intake files there. --control plants a fault, to prove the
checks catch it: alter (one intake instruction changed), omit (one intake routine left out),
unplace (one copy of a group left unplaced), dup (a filed routine's range also in the intake).
"""
import glob
import os
import re
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import cpuguard  # noqa: E402
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


def plan(filed, intake, disc, dm):
    """Every piece to place, in disc order; refuses an overlap or a run byte no piece covers."""
    pieces = []
    for f in filed:
        pieces.append({"disc": f["disc"], "end": f["disc"] + f["end"], "run": f["run"], "text": f["text"],
                       "name": f["name"], "src": "filed"})
    if intake is not None:
        for s, f in intake.filed_copies:
            pieces.append({"disc": s, "end": s + f["end"], "run": f["run"],
                           "text": ".scope\n" + f["text"] + "\n.endscope", "name": f["name"], "src": "filed copy"})
        for x in intake.pieces:
            pieces.append(dict(x))
    pieces.sort(key=lambda x: (x["disc"], x["end"]))
    last = None
    for x in pieces:
        if last and x["disc"] < last["end"]:
            raise SystemExit(f"asmbuild: pieces overlap: {last['src']} {last['name']} at {tag(last['disc'])}-"
                             f"{tag(last['end'])} and {x['src']} {x['name']} at {tag(x['disc'])}: a range is "
                             f"assembled twice (a filed routine's range must not also be in the intake)")
        last = x
    if intake is not None:
        cov = bytearray(len(dm))
        for x in pieces:
            cov[x["disc"]:x["end"]] = b"\x01" * (x["end"] - x["disc"])
        ex = intake.imask                      # every byte of an instruction a script began
        miss = (int.from_bytes(ex, "little") & ~int.from_bytes(cov, "little")).to_bytes(len(dm), "little")
        n = len(miss) - miss.count(0)
        if n:
            first = miss.find(1)
            raise SystemExit(f"asmbuild: {n} bytes of instructions a script ran are in no piece (first at {tag(first)}): "
                             f"a routine of the intake is not placed")
    return pieces


def main():
    cpuguard.require("asmbuild.py", jobs=1, mem="3G")
    argv = sys.argv[1:]
    control = argv[argv.index("--control") + 1] if "--control" in argv else None
    raw = argv[argv.index("--raw") + 1] if "--raw" in argv else None
    t0 = time.time()
    os.makedirs(OUT, exist_ok=True)
    disc = open(TRACK, "rb").read()
    size = len(disc)
    filed = filed_pieces(disc)
    intake = dm = None
    import intake as intake_mod
    data = intake_mod.load()
    if data is None:
        print("asmbuild: no trace under build/rondo/trace: filed assembly only, the rest from the disc")
    else:
        dm, rows, es = data[1], data[2], data[3]
        intake = intake_mod.build(filed, disc, dm, rows, es, control=control)
        print(intake_mod.summary(intake))
        if raw:
            sc, names = intake_mod.script_runs(intake.groups)
            intake_mod.headers(intake, es, sc, names)
            print(f"  {intake_mod.write_files(intake, raw)} intake files in {raw}")
    t1 = time.time()
    pieces = plan(filed, intake, disc, dm)
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
    kinds = {}
    for x in pieces:
        kinds[x["src"]] = kinds.get(x["src"], 0) + 1
    print(f"{len(pieces)} pieces assembled in place ({', '.join(f'{v} {k}' for k, v in sorted(kinds.items()))}): "
          f"{os.path.join(OUT, 'image.bin')}  [intake {t1 - t0:.0f} s, ca65 and ld65 {time.time() - t2:.0f} s]")
    return 0


if __name__ == "__main__":
    sys.exit(main())
