#!/usr/bin/env python3
"""Assembler control: every HuC6280 instruction form round-trips through ca65.

Builds an image holding every one of the 256 opcodes: the 234 defined ones
with operands chosen to tempt the assembler (byte operands $00, $5A, $FF;
word operands $0000, $0012 and $00FF, which fit a zero page, and $1234,
$FFFF; branches at their shortest, nearest and longest reach), and the 22
undefined ones as data. It decodes the image with huc.py, writes the
listing in the text form the intake uses, assembles and links it with the
pinned ca65 and ld65, and compares the bytes.

    python3 tools/rondo/asmcontrol.py            0 differing bytes or exit 1
    python3 tools/rondo/asmcontrol.py --plant    drop one operand size; must fail
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import huc  # noqa: E402

CA65 = os.path.join(TOP, "tools", "toolchains", "cc65", "ca65")
LD65 = os.path.join(TOP, "tools", "toolchains", "cc65", "ld65")
OUT = os.path.join(TOP, "build", "rondo", "asmcontrol")
ORG = 0x4000

BYTES = (0x00, 0x5A, 0xFF)
WORDS = (0x0000, 0x0012, 0x00FF, 0x1234, 0xFFFF)
DISPS = (0x80, 0xFE, 0x00, 0x7F)   # -128, -2 (to itself), 0, +127


def image():
    out = bytearray()
    for op in range(256):
        e = huc.OPS.get(op)
        if not e:
            out.append(op)
            continue
        mode = e[1]
        n = huc.MODES[mode]
        if n == 0:
            forms = [b""]
        elif mode == "rel":
            forms = [bytes((d,)) for d in DISPS]
        elif mode == "zrel":
            forms = [bytes((z, d)) for z in BYTES for d in DISPS]
        elif e[0] == "tma":
            forms = [bytes((b,)) for b in (0x01, 0x40, 0x80, 0x00, 0x5A, 0xFF)]
        elif n == 1:
            forms = [bytes((b,)) for b in BYTES]
        elif mode in ("tzp", "tzx"):
            forms = [bytes((i, z)) for i in BYTES for z in BYTES]
        elif n == 2:
            forms = [w.to_bytes(2, "little") for w in WORDS]
        elif mode in ("tab", "tax"):
            forms = [bytes((i,)) + w.to_bytes(2, "little") for i in BYTES for w in WORDS]
        else:  # blk
            forms = [b"".join(w.to_bytes(2, "little") for w in (s, d, l))
                     for s, d, l in ((0x0000, 0x0012, 0x00FF), (0x1234, 0xFFFF, 0x0001))]
        for f in forms:
            out.append(op)
            out += f
    return bytes(out)


def main():
    plant = "--plant" in sys.argv[1:]
    img = image()
    lines = ['.setcpu "huc6280"', ".segment \"CODE\"", "img:"]
    label = lambda a: f"img+${a - ORG:04X}" if a >= ORG else f"img-${ORG - a:04X}"
    forms = 0
    planted = False
    i = 0
    while i < len(img):
        n, name, mode, ops, _ = huc.decode(img, i, ORG + i)
        t = huc.ca65(name, mode, ops, label)
        if plant and not planted and mode == "abs" and ops[0] == 0x0012:
            t = t.replace("a:", "")
            planted = True
            print(f"planted: operand size dropped from `{huc.ca65(name, mode, ops, label)}`")
        lines.append("    " + t)
        forms += name is not None
        i += n
    os.makedirs(OUT, exist_ok=True)
    src = os.path.join(OUT, "control.s")
    obj = os.path.join(OUT, "control.o")
    cfg = os.path.join(OUT, "control.cfg")
    binp = os.path.join(OUT, "control.bin")
    for p in (obj, binp):
        if os.path.exists(p):
            os.remove(p)
    open(src, "w").write("\n".join(lines) + "\n")
    open(cfg, "w").write(
        "MEMORY { M: start = $%04X, size = $%04X, file = %%O, fill = no; }\n"
        "SEGMENTS { CODE: load = M, type = ro; }\n" % (ORG, 0x10000 - ORG))
    r = subprocess.run([CA65, "-o", obj, src], capture_output=True, text=True)
    if r.returncode:
        print(r.stdout + r.stderr)
        print("asmcontrol: ca65 FAILED")
        return 1
    r = subprocess.run([LD65, "-C", cfg, "-o", binp, obj], capture_output=True, text=True)
    if r.returncode:
        print(r.stdout + r.stderr)
        print("asmcontrol: ld65 FAILED")
        return 1
    got = open(binp, "rb").read()
    diff = sum(1 for a, b in zip(got, img) if a != b) + abs(len(got) - len(img))
    first = next((k for k, (a, b) in enumerate(zip(got, img)) if a != b), None)
    ops_seen = len({img[k] for k in range(len(img))} & set(huc.OPS))
    print(f"{forms} instructions over {ops_seen} defined opcodes, {256 - len(huc.OPS)} "
          f"undefined as data; {len(img)} B expected, {len(got)} B assembled, "
          f"{diff} differing bytes" + ("" if first is None else f", first at ${ORG + first:04X}"))
    return 0 if diff == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
