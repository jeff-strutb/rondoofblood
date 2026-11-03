#!/usr/bin/env python3
"""HuC6280 instruction decoder, and a linear listing for reading code.

The HuC6280 is a 65C02 with the Rockwell bit instructions (RMB, SMB, BBR,
BBS) and Hudson's own: register swaps (SXY, SAX, SAY), clears (CLA, CLX,
CLY), VDC stores (ST0, ST1, ST2), MPR transfers (TAM, TMA), block moves
(TII, TDD, TIN, TIA, TAI), TST, BSR, CSL, CSH and SET. Its zero page is the
logical page $2000 and its stack page $2100; the listing writes zero-page
operands as <$nn.

Every opcode not in the table decodes as `.db` (the chip runs them as NOPs;
no traced code is expected to use them, and the census treats one as a bad
decode).

    python3 tools/rondo/huc.py FILE --org ADDR [--offset N] [--len N]
"""
import argparse

# mode: operand bytes, format
MODES = {
    "imp": 0, "acc": 0, "imm": 1, "zp": 1, "zpx": 1, "zpy": 1, "izp": 1,
    "izx": 1, "izy": 1, "abs": 2, "abx": 2, "aby": 2, "ind": 2, "iax": 2,
    "rel": 1, "zrel": 2, "blk": 6, "tzp": 2, "tzx": 2, "tab": 3, "tax": 3,
}

OPS = {}


def _op(code, name, mode):
    OPS[code] = (name, mode)


_ALU = [("ora", 0x00), ("and", 0x20), ("eor", 0x40), ("adc", 0x60),
        ("sta", 0x80), ("lda", 0xA0), ("cmp", 0xC0), ("sbc", 0xE0)]
for _n, _b in _ALU:
    _op(_b + 0x01, _n, "izx")
    _op(_b + 0x05, _n, "zp")
    if _n != "sta":
        _op(_b + 0x09, _n, "imm")
    _op(_b + 0x0D, _n, "abs")
    _op(_b + 0x11, _n, "izy")
    _op(_b + 0x12, _n, "izp")
    _op(_b + 0x15, _n, "zpx")
    _op(_b + 0x19, _n, "aby")
    _op(_b + 0x1D, _n, "abx")
for _i in range(8):
    _op(0x07 + 0x10 * _i, f"rmb{_i}", "zp")
    _op(0x87 + 0x10 * _i, f"smb{_i}", "zp")
    _op(0x0F + 0x10 * _i, f"bbr{_i}", "zrel")
    _op(0x8F + 0x10 * _i, f"bbs{_i}", "zrel")
for _code, _n, _m in [
    (0x00, "brk", "imp"), (0x02, "sxy", "imp"), (0x03, "st0", "imm"),
    (0x04, "tsb", "zp"), (0x06, "asl", "zp"), (0x08, "php", "imp"),
    (0x0A, "asl", "acc"), (0x0C, "tsb", "abs"), (0x0E, "asl", "abs"),
    (0x10, "bpl", "rel"), (0x13, "st1", "imm"), (0x14, "trb", "zp"),
    (0x16, "asl", "zpx"), (0x18, "clc", "imp"), (0x1A, "inc", "acc"),
    (0x1C, "trb", "abs"), (0x1E, "asl", "abx"),
    (0x20, "jsr", "abs"), (0x22, "sax", "imp"), (0x23, "st2", "imm"),
    (0x24, "bit", "zp"), (0x26, "rol", "zp"), (0x28, "plp", "imp"),
    (0x2A, "rol", "acc"), (0x2C, "bit", "abs"), (0x2E, "rol", "abs"),
    (0x30, "bmi", "rel"), (0x34, "bit", "zpx"), (0x36, "rol", "zpx"),
    (0x38, "sec", "imp"), (0x3A, "dec", "acc"), (0x3C, "bit", "abx"),
    (0x3E, "rol", "abx"),
    (0x40, "rti", "imp"), (0x42, "say", "imp"), (0x43, "tma", "imm"),
    (0x44, "bsr", "rel"), (0x46, "lsr", "zp"), (0x48, "pha", "imp"),
    (0x4A, "lsr", "acc"), (0x4C, "jmp", "abs"), (0x4E, "lsr", "abs"),
    (0x50, "bvc", "rel"), (0x53, "tam", "imm"), (0x54, "csl", "imp"),
    (0x56, "lsr", "zpx"), (0x58, "cli", "imp"), (0x5A, "phy", "imp"),
    (0x5E, "lsr", "abx"),
    (0x60, "rts", "imp"), (0x62, "cla", "imp"), (0x64, "stz", "zp"),
    (0x66, "ror", "zp"), (0x68, "pla", "imp"), (0x6A, "ror", "acc"),
    (0x6C, "jmp", "ind"), (0x6E, "ror", "abs"),
    (0x70, "bvs", "rel"), (0x73, "tii", "blk"), (0x74, "stz", "zpx"),
    (0x76, "ror", "zpx"), (0x78, "sei", "imp"), (0x7A, "ply", "imp"),
    (0x7C, "jmp", "iax"), (0x7E, "ror", "abx"),
    (0x80, "bra", "rel"), (0x82, "clx", "imp"), (0x83, "tst", "tzp"),
    (0x84, "sty", "zp"), (0x86, "stx", "zp"), (0x88, "dey", "imp"),
    (0x89, "bit", "imm"), (0x8A, "txa", "imp"), (0x8C, "sty", "abs"),
    (0x8E, "stx", "abs"),
    (0x90, "bcc", "rel"), (0x93, "tst", "tab"), (0x94, "sty", "zpx"),
    (0x96, "stx", "zpy"), (0x98, "tya", "imp"), (0x9A, "txs", "imp"),
    (0x9C, "stz", "abs"), (0x9E, "stz", "abx"),
    (0xA0, "ldy", "imm"), (0xA2, "ldx", "imm"), (0xA3, "tst", "tzx"),
    (0xA4, "ldy", "zp"), (0xA6, "ldx", "zp"), (0xA8, "tay", "imp"),
    (0xAA, "tax", "imp"), (0xAC, "ldy", "abs"), (0xAE, "ldx", "abs"),
    (0xB0, "bcs", "rel"), (0xB3, "tst", "tax"), (0xB4, "ldy", "zpx"),
    (0xB6, "ldx", "zpy"), (0xB8, "clv", "imp"), (0xBA, "tsx", "imp"),
    (0xBC, "ldy", "abx"), (0xBE, "ldx", "aby"),
    (0xC0, "cpy", "imm"), (0xC2, "cly", "imp"), (0xC3, "tdd", "blk"),
    (0xC4, "cpy", "zp"), (0xC6, "dec", "zp"), (0xC8, "iny", "imp"),
    (0xCA, "dex", "imp"), (0xCC, "cpy", "abs"), (0xCE, "dec", "abs"),
    (0xD0, "bne", "rel"), (0xD3, "tin", "blk"), (0xD4, "csh", "imp"),
    (0xD6, "dec", "zpx"), (0xD8, "cld", "imp"), (0xDA, "phx", "imp"),
    (0xDE, "dec", "abx"),
    (0xE0, "cpx", "imm"), (0xE3, "tia", "blk"), (0xE4, "cpx", "zp"),
    (0xE6, "inc", "zp"), (0xE8, "inx", "imp"), (0xEA, "nop", "imp"),
    (0xEC, "cpx", "abs"), (0xEE, "inc", "abs"),
    (0xF0, "beq", "rel"), (0xF3, "tai", "blk"), (0xF4, "set", "imp"),
    (0xF6, "inc", "zpx"), (0xF8, "sed", "imp"), (0xFA, "plx", "imp"),
    (0xFE, "inc", "abx"),
]:
    _op(_code, _n, _m)

# Instructions after which execution does not fall through.
ENDS = {"rts", "rti", "jmp", "bra", "brk"}


def length(op):
    e = OPS.get(op)
    return 1 + MODES[e[1]] if e else 1


def decode(buf, i, pc):
    """Decode at buf[i] running at pc: (length, name, mode, operands, target)."""
    op = buf[i]
    e = OPS.get(op)
    if not e:
        return 1, None, None, (op,), None
    name, mode = e
    n = MODES[mode]
    b = bytes(buf[i + 1:i + 1 + n])
    if len(b) < n:
        return 1, None, None, (op,), None
    w = lambda k: b[k] | (b[k + 1] << 8)
    rel = lambda d, at: (at + (d - 256 if d & 0x80 else d)) & 0xFFFF
    target = None
    if mode in ("imm", "zp", "zpx", "zpy", "izp", "izx", "izy"):
        ops = (b[0],)
    elif mode in ("abs", "abx", "aby", "ind", "iax"):
        ops = (w(0),)
        if name in ("jsr", "jmp") and mode == "abs":
            target = w(0)
    elif mode == "rel":
        target = rel(b[0], pc + 2)
        ops = (target,)
    elif mode == "zrel":
        target = rel(b[1], pc + 3)
        ops = (b[0], target)
    elif mode == "blk":
        ops = (w(0), w(2), w(4))
    elif mode in ("tzp", "tzx"):
        ops = (b[0], b[1])
    elif mode in ("tab", "tax"):
        ops = (b[0], w(1))
    else:
        ops = ()
    return 1 + n, name, mode, ops, target


def text(name, mode, ops):
    if name is None:
        return f".db ${ops[0]:02X}"
    f = {
        "imp": "", "acc": " a", "imm": " #${0:02X}", "zp": " <${0:02X}",
        "zpx": " <${0:02X},x", "zpy": " <${0:02X},y", "izp": " [${0:02X}]",
        "izx": " [${0:02X},x]", "izy": " [${0:02X}],y", "abs": " ${0:04X}",
        "abx": " ${0:04X},x", "aby": " ${0:04X},y", "ind": " [${0:04X}]",
        "iax": " [${0:04X},x]", "rel": " ${0:04X}", "zrel": " <${0:02X},${1:04X}",
        "blk": " ${0:04X},${1:04X},${2:04X}", "tzp": " #${0:02X},<${1:02X}",
        "tzx": " #${0:02X},<${1:02X},x", "tab": " #${0:02X},${1:04X}",
        "tax": " #${0:02X},${1:04X},x",
    }[mode]
    return name + f.format(*ops)


def ca65(name, mode, ops, label=None):
    """The instruction in ca65 syntax, every operand size explicit.

    z: forces a zero-page operand and a: an absolute one, so the bytes never
    depend on the assembler choosing the shorter form. ca65 refuses a
    constant branch target in a relocatable segment, so a branch goes to
    a label: label(target) gives its text.
    """
    if mode in ("rel", "zrel") and label is not None:
        tgt = label(ops[-1])
        return name + (f" {tgt}" if mode == "rel" else f" ${ops[0]:02X},{tgt}")
    if name is None:
        return f".byte ${ops[0]:02X}"
    # ca65 takes TMA's operand only as a single MPR bit; any other is data.
    if name == "tma" and ops[0] & (ops[0] - 1):
        return f".byte $43,${ops[0]:02X}"
    f = {
        "imp": "", "acc": " a", "imm": " #${0:02X}", "zp": " z:${0:02X}",
        "zpx": " z:${0:02X},x", "zpy": " z:${0:02X},y", "izp": " (${0:02X})",
        "izx": " (${0:02X},x)", "izy": " (${0:02X}),y", "abs": " a:${0:04X}",
        "abx": " a:${0:04X},x", "aby": " a:${0:04X},y", "ind": " (${0:04X})",
        "iax": " (${0:04X},x)", "rel": " ${0:04X}", "zrel": " ${0:02X},${1:04X}",
        "blk": " ${0:04X},${1:04X},${2:04X}", "tzp": " #${0:02X},z:${1:02X}",
        "tzx": " #${0:02X},z:${1:02X},x", "tab": " #${0:02X},a:${1:04X}",
        "tax": " #${0:02X},a:${1:04X},x",
    }[mode]
    return name + f.format(*ops)


def listing(buf, org, start=0, count=None):
    end = len(buf) if count is None else min(len(buf), start + count)
    i = start
    while i < end:
        pc = org + (i - start)
        n, name, mode, ops, _ = decode(buf, i, pc)
        raw = " ".join(f"{x:02X}" for x in buf[i:i + n])
        yield pc, raw, text(name, mode, ops), name
        i += n


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("file")
    ap.add_argument("--org", type=lambda s: int(s, 0), required=True)
    ap.add_argument("--offset", type=lambda s: int(s, 0), default=0)
    ap.add_argument("--len", type=lambda s: int(s, 0), default=None)
    a = ap.parse_args()
    buf = open(a.file, "rb").read()
    for pc, raw, t, name in listing(buf, a.org, a.offset, a.len):
        print(f"{pc:04X}  {raw:<20} {t}")


if __name__ == "__main__":
    main()
