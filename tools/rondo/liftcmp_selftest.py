#!/usr/bin/env python3
"""liftcmp_selftest.py -- the lift harness must fail where it should.

    python3 tools/rondo/liftcmp_selftest.py

Plants four faults one at a time in a copy of a filed routine's C (a read taken out of order, a read doubled, a
wrong flag at exit, a missing call frame), runs tools/rondo/liftcmp.py on each against the routine's draft, and
requires that each is caught (the comparison exits 1); the unchanged routine must pass. Short checks: no slot.
Exit 0 when every plant is caught and the original passes.
"""
import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))

# (routine, file, plant, text to find, text to put)
PLANTS = [
    ("WriteBandScroll", "src/rondo/system/video.c", "a read out of order",
     "    byte = RD8(BANDS->bgx_lo[band]);\n    WR8(VDC->data_lo, byte);\n    byte = RD8(BANDS->bgx_hi[band]);\n    WR8(VDC->data_hi, byte);\n",
     "    byte = RD8(BANDS->bgx_hi[band]);\n    WR8(VDC->data_hi, byte);\n    byte = RD8(BANDS->bgx_lo[band]);\n    WR8(VDC->data_lo, byte);\n"),
    ("WriteBandScroll", "src/rondo/system/video.c", "a doubled read",
     "    byte = RD8(BANDS->bgy_lo[band]);\n",
     "    byte = RD8(BANDS->bgy_lo[band]);\n    byte = RD8(BANDS->bgy_lo[band]);\n"),
    ("WriteBandScroll", "src/rondo/system/video.c", "a wrong flag at exit",
     "    cpu.a = WriteBandScroll(cpu.x);\n    abi_nz(cpu.a);\n",
     "    cpu.a = WriteBandScroll(cpu.x);\n    abi_nz((uint8_t)(cpu.a | 1));\n"),
    ("WriteBandScroll", "src/rondo/system/video.c", "a missing frame",
     "    jsr_push(0x42FE);\n    SelectVdcRegister(VDC_REG_BXR);\n    jsr_pop();\n",
     "    SelectVdcRegister(VDC_REG_BXR);\n"),
]


def liftcmp(name, lifted=None, extra=()):
    cmd = [sys.executable, "-I", os.path.join(HERE, "liftcmp.py"), name, "--no-cover", *extra]
    if lifted:
        cmd += ["--lifted", lifted]
    return subprocess.run(cmd, capture_output=True, text=True, cwd=TOP)


def main():
    bad = 0
    r = liftcmp(PLANTS[0][0])
    print(f"unchanged {PLANTS[0][0]}: {'passes' if r.returncode == 0 else 'FAILS'}  ({r.stdout.splitlines()[0] if r.stdout else r.stderr[-200:]})")
    bad += r.returncode != 0
    for name, rel, what, old, new in PLANTS:
        text = open(os.path.join(TOP, rel)).read()
        if old not in text:
            print(f"{what}: cannot plant, the text to change is not in {rel}")
            bad += 1
            continue
        with tempfile.TemporaryDirectory() as d:
            p = os.path.join(d, os.path.basename(rel))
            open(p, "w").write(text.replace(old, new, 1))
            r = liftcmp(name, p)
        caught = r.returncode == 1
        print(f"{what}: {'caught' if caught else 'NOT CAUGHT'}  ({r.stdout.splitlines()[0] if r.stdout else r.stderr[-200:]})")
        bad += not caught
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
