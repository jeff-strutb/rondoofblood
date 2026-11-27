#!/usr/bin/env python3
"""opcontrol.py -- the control for ops6280.h and cdraft.py's emitter.

    python3 tools/rondo/opcontrol.py [--plant NAME] [--seed N] [--core LIB]

Every instruction form the drafter emits (and SET followed by the four ALU forms that have a T
mode) is run on the core with chosen registers, flags and memory, and the same instruction as
cdraft.py writes it is run on a model bus; registers, P, S, MPRs, speed and the whole access list
must be equal (pcebox/opcontrol.c). --plant NAME makes one helper wrong on purpose, and the run
must fail:

    adc_carry   ADC ignores the carry in         tst_zero   TST swaps Z
    rol_carry   ROL's carry in is dropped        tam_mask   TAM sets one MPR fewer
    bm_order    a block move writes before reads php_t      PHP keeps T
"""
import argparse
import os
import random
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import cdraft  # noqa: E402
import huc  # noqa: E402

OUT = os.path.join(TOP, "build", "rondo", "drafter", "control")
CORE = os.path.join(TOP, "build", "rondo", "drafter", "core", "mednafen_pce_libretro.so")
CODE = {}
for _op, (_n, _m) in huc.OPS.items():
    CODE[(_n, _m)] = _op

PLANTS = {
    "adc_carry": ("unsigned l = (unsigned)a + x + (cpu.p & 1u);", "unsigned l = (unsigned)a + x;"),
    "rol_carry": ("x = (uint8_t)((x << 1) | (cpu.p & P_C));", "x = (uint8_t)(x << 1);"),
    "bm_order": ("wr8((uint16_t)(dst + i), rd8((uint16_t)(src + i)));",
                 "{ uint8_t v_ = rd8((uint16_t)(src + i)); wr8((uint16_t)(dst + i), v_); (void)v_; if (i == 0) bm_order_planted(); }"),
    "tst_zero": ("cpu.p |= (m & imm) ? 0 : P_Z;", "cpu.p |= (m & imm) ? P_Z : 0;"),
    "tam_mask": ("static inline void op_tam(uint8_t mask) { tam(mask, cpu.a); }",
                 "static inline void op_tam(uint8_t mask) { tam((uint8_t)(mask & (mask - 1)), cpu.a); }"),
    "php_t": ("cpu.p &= (uint8_t)~P_T; push8((uint8_t)(cpu.p | P_B)); }", "push8((uint8_t)(cpu.p | P_B | P_T)); }"),
}

# instruction forms: name -> modes
LOADS = ["lda", "ldx", "ldy", "and", "ora", "eor", "adc", "sbc", "cmp", "cpx", "cpy", "bit"]
FORMS = []
for _n in LOADS + ["sta", "stx", "sty", "stz", "asl", "lsr", "rol", "ror", "inc", "dec", "tsb", "trb",
                   "tax", "txa", "tay", "tya", "tsx", "txs", "inx", "iny", "dex", "dey", "cla", "clx", "cly",
                   "sax", "say", "sxy", "clc", "sec", "cld", "sed", "cli", "sei", "clv", "pha", "phx", "phy", "php",
                   "pla", "plx", "ply", "plp", "csl", "csh", "nop", "st0", "st1", "st2", "tam", "tma", "tst",
                   "tii", "tdd", "tin", "tia", "tai", "bpl", "bmi", "bvc", "bvs", "bcc", "bcs", "bne", "beq", "bra"]:
    for (_nn, _m), _o in CODE.items():
        if _nn == _n:
            FORMS.append((_n, _m, _o))
for _i in range(8):
    for _n in ("rmb", "smb", "bbr", "bbs"):
        for (_nn, _m), _o in CODE.items():
            if _nn == "%s%d" % (_n, _i):
                FORMS.append((_nn, _m, _o))

WRITES = {"sta", "stx", "sty", "stz", "asl", "lsr", "rol", "ror", "inc", "dec", "tsb", "trb"}
BIN = {"adc": "adc", "sbc": "sbc"}


class FakeGroup:
    def __init__(self, items, content):
        self.items, self.content, self.n = items, content, len(content)
        self.istart = {it[1]: i for i, it in enumerate(items)}


def emit(ins, name, mode, ops, tpend=False):
    """The C of the drafter's emitter for one instruction at $2200 (branches: the condition)."""
    ln = 1 + huc.MODES[mode]
    g = FakeGroup([("i", 0, ln, name, mode, ops, False)], bytes(ins))
    d = cdraft.Draft(g, 0x2200, "x")
    d.tpend = tpend
    if name in cdraft.COND:
        return "return %s;" % cdraft.COND[name].replace("(cpu.p", "((cpu.p") if False else "return (%s) != 0;" % cdraft.COND[name]
    if name[:3] in ("bbr", "bbs"):
        return "return op_%s(%s, 0x%02X);" % (name[:3], name[3], ops[0])
    return d.step(0)


RAM_ADDRS = [0x2010, 0x20F0, 0x2300, 0x3010, 0x3123, 0x3FF0, 0x3FFF]
ROM_ADDRS = [0x4000, 0x4123, 0x6FFF, 0x8000, 0xA55A, 0xBFFF]


def gen(seed):
    rnd = random.Random(seed)
    cases = []
    funcs = []

    def add(text, ins, a, x, y, p, s, mem, body, branch=False, n=1):
        k = len(cases)
        if branch:
            funcs.append("static int b%d(void) { %s }" % (k, body))
        else:
            funcs.append("static void f%d(void) { %s }" % (k, body))
        mm = ", ".join("{0x%04X, 0x%02X}" % m for m in mem[:12]) or "{0, 0}"
        cases.append('    { "%s", {%s}, %d, %d, %d, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, %d, {%s}, %s, %s },' % (
            text, ",".join("0x%02X" % b for b in ins), len(ins), n, 1 if branch else 0, a, x, y, p, s,
            min(len(mem), 12), mm, "0" if branch else "f%d" % k, "b%d" % k if branch else "0"))

    def regs(name):
        a, x, y = (rnd.choice([0, 1, 0x7F, 0x80, 0xFF, rnd.randrange(256)]) for _ in range(3))
        p = rnd.choice([0, 1, 2, 0x40, 0x80, 0xC3, rnd.randrange(256) & 0xCF | 0, 0x08, 0x09, 0x0B, 0x04])
        s = rnd.choice([0xF0, 0xF0, 0xF0, 0x05, 0x00, 0xFF, 0x80])
        return a, x, y, p & ~(0x20 | 0x10), s

    def one(name, mode, op, tpend=False, per=None):
        a, x, y, p, s = regs(name)
        mem = []
        ins = [op]
        ops = ()
        data = rnd.randrange(256)
        if mode in ("imm",):
            ops = (rnd.randrange(256),)
            ins += list(ops)
        elif mode == "zp":
            o = rnd.choice([0x00, 0x40, 0x7F, 0xF8, 0xFF])
            ops = (o,)
            ins += [o]
            mem.append((0x2000 | o, data))
        elif mode in ("zpx", "zpy"):
            o = rnd.choice([0x00, 0x40, 0xF8, 0xFF])
            ops = (o,)
            ins += [o]
            idx = x if mode == "zpx" else y
            mem.append((0x2000 | ((idx + o) & 0xFF), data))
        elif mode == "abs":
            ad = rnd.choice(RAM_ADDRS + ROM_ADDRS)
            ops = (ad,)
            ins += [ad & 0xFF, ad >> 8]
            if ad < 0x4000:
                mem.append((ad, data))
        elif mode in ("abx", "aby"):
            idx = x if mode == "abx" else y
            ad = rnd.choice(RAM_ADDRS + ROM_ADDRS)
            ad = min(ad, 0xFFFF - idx)
            ops = (ad,)
            ins += [ad & 0xFF, ad >> 8]
            if ad + idx < 0x4000:
                mem.append((ad + idx, data))
        elif mode in ("izp", "izx", "izy"):
            o = rnd.choice([0x30, 0x80, 0xFE, 0xFF])
            ops = (o,)
            ins += [o]
            ptr = (o + x) & 0xFF if mode == "izx" else o
            t = rnd.choice(RAM_ADDRS + ROM_ADDRS)
            if mode == "izy":
                t = min(t, 0xFFFF - y)
            mem.append((0x2000 | ptr, t & 0xFF))
            mem.append((0x2000 | ((ptr + 1) & 0xFF), t >> 8))
            ea = t + (y if mode == "izy" else 0)
            if ea < 0x4000:
                mem.append((ea, data))
        elif mode == "rel":
            d = rnd.choice([d for d in range(-20, 21) if d])
            ops = (0,)
            ins += [d & 0xFF]
            ops = (0x2200 + 2 + d,)
        elif mode == "zrel":
            o = rnd.choice([0x00, 0x40, 0xF8])
            d = rnd.choice([d for d in range(-20, 21) if d])
            ins += [o, d & 0xFF]
            ops = (o, 0x2200 + 3 + d)
            mem.append((0x2000 | o, data))
        elif mode in ("tzp", "tab"):
            imm = rnd.randrange(256)
            ad = rnd.choice([0x40, 0xF8] if mode == "tzp" else RAM_ADDRS + ROM_ADDRS)
            ops = (imm, ad)
            ins += [imm] + ([ad] if mode == "tzp" else [ad & 0xFF, ad >> 8])
            mem.append(((0x2000 | ad) if mode == "tzp" else ad, data))
            if mode == "tab" and ad >= 0x4000:
                mem.pop()
        elif mode in ("tzx", "tax"):
            imm = rnd.randrange(256)
            if mode == "tzx":
                ad = rnd.choice([0x40, 0xF8])
                ins += [imm, ad]
                mem.append((0x2000 | ((ad + x) & 0xFF), data))
            else:
                ad = min(rnd.choice(RAM_ADDRS + ROM_ADDRS), 0xFFFF - x)
                ins += [imm, ad & 0xFF, ad >> 8]
                if ad + x < 0x4000:
                    mem.append((ad + x, data))
            ops = (imm, ad)
        elif mode == "blk":
            n = rnd.choice([1, 2, 3, 5])
            if name == "tdd":
                src = rnd.choice([0x3040, 0x4040, 0xA040])
                dst = rnd.choice([0x3200, 0x3F00])
            else:
                src = rnd.choice([0x3000, 0x4000, 0xA000])
                dst = rnd.choice([0x3100, 0x3200, 0x3F00])
            ops = (src, dst, n)
            ins += [src & 0xFF, src >> 8, dst & 0xFF, dst >> 8, n, 0]
            if src < 0x4000:
                for i in range(-n, n + 1):
                    mem.append((src + i, rnd.randrange(256)))
        elif mode in ("acc", "imp"):
            ops = ()
        else:
            raise SystemExit("no generator for %s %s" % (name, mode))
        if name in ("tam", "tma"):
            ops = (rnd.choice([0x04, 0x08, 0x10, 0x20] + ([0x0C, 0x30] if name == "tam" else [])),)
            ins = [op, ops[0]]
            a = rnd.choice([1, 2, 3, 4, 5, 0xF8])
        if name[:3] in ("rmb", "smb"):
            pass
        if name in ("plp", "pla", "plx", "ply"):
            mem.append((0x2100 | ((s + 1) & 0xFF), rnd.randrange(256)))
        if name == "bit" and mode == "imm":
            pass
        body = emit(ins, name, mode, ops, tpend)
        branch = name in cdraft.COND or name[:3] in ("bbr", "bbs")
        if tpend:
            # SET, then the instruction: the T form reads zero page [X] and writes it back
            if all(m[0] != (0x2000 | x) for m in mem):
                mem.append((0x2000 | x, rnd.randrange(256)))
            body = "op_set(); " + body
            add("set;%s %s" % (name, mode), [0xF4] + ins, a, x, y, p, s, mem, body, False, 2)
        else:
            add("%s %s" % (name, mode), ins, a, x, y, p, s, mem, body, branch)

    for name, mode, op in FORMS:
        if name == "tma":
            pass
        count = 40
        if name in ("adc", "sbc"):
            count = 120
        for _ in range(count if mode not in ("imp", "acc") else 14):
            one(name, mode, op)
    for name in ("and", "ora", "eor", "adc"):
        for (nn, mm), op in CODE.items():
            if nn == name:
                for _ in range(24):
                    one(name, mm, op, tpend=True)
    # SET followed by something with no T form: T is cleared by it
    for name, mode, op in (("lda", "imm", 0xA9), ("cmp", "imm", 0xC9), ("inx", "imp", 0xE8)):
        for _ in range(6):
            a, x, y, p, s = regs(name)
            ins = [op] + ([rnd.randrange(256)] if mode == "imm" else [])
            mm = "set;%s %s (T cleared)" % (name, mode)
            add(mm, [0xF4] + ins, a, x, y, p, s, [], "op_set(); op_clear_t(); " + emit(ins, name, mode,
                                                                                    tuple(ins[1:]), False), False, 2)
    # decimal and binary ADC and SBC, many operands
    for name, op in (("adc", 0x69), ("sbc", 0xE9)):
        for _ in range(1500):
            v = rnd.randrange(256)
            ins = [op, v]
            a = rnd.randrange(256)
            p = rnd.choice([0x08, 0x09, 0x08, 0x09, 0x48, 0x49, 0, 1])
            add("%s imm D" % name, ins, a, 0, 0, p, 0xF0, [], emit(ins, name, "imm", (v,)), False)
    return cases, funcs


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--plant", default="")
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--core", default=os.environ.get("RONDO_CORE") or CORE)
    a = ap.parse_args()
    os.makedirs(OUT, exist_ok=True)
    cases, funcs = gen(a.seed)
    inc = os.path.join(OUT, "inc")
    os.makedirs(inc, exist_ok=True)
    hdr = open(os.path.join(TOP, "src", "rondo", "include", "ops6280.h")).read()
    if a.plant:
        old, new = PLANTS[a.plant]
        if old not in hdr:
            raise SystemExit("plant %s: pattern not in ops6280.h" % a.plant)
        hdr = hdr.replace(old, new)
        if a.plant == "bm_order":
            hdr = hdr.replace("static inline void bm_begin", "static inline void bm_order_planted(void) { push8(0x99); }\nstatic inline void bm_begin")
    open(os.path.join(inc, "ops6280.h"), "w").write(hdr)
    open(os.path.join(inc, "cases.h"), "w").write('''#include <stdint.h>
typedef struct { uint16_t addr; uint8_t v; } Mem;
typedef struct { const char *text; uint8_t ins[8]; uint8_t len, n, branch; uint8_t a, x, y, p, s; uint8_t nmem; Mem mem[12];
                 void (*fn)(void); int (*br)(void); } Case;
extern const Case cases[];
extern const unsigned ncases;
''')
    with open(os.path.join(OUT, "cases.c"), "w") as f:
        f.write('#include "machine.h"\n#include "ops6280.h"\n#include "cases.h"\n')
        f.write("\n".join(funcs) + "\n")
        f.write("const Case cases[] = {\n" + "\n".join(cases) + "\n};\n")
        f.write("const unsigned ncases = %d;\n" % len(cases))
    # the core's include directory for libretro.h
    lr = os.path.join(TOP, "tools", "toolchains", "src", "pce", "libretro-common", "include")
    if not os.path.isdir(lr):
        lr = os.path.join(os.path.dirname(os.path.dirname(a.core)), "core", "libretro-common", "include")
    for cand in (os.path.join(os.path.dirname(a.core), "libretro-common", "include"), lr):
        if os.path.isdir(cand):
            lr = cand
            break
    exe = os.path.join(OUT, "opcontrol" + ("_" + a.plant if a.plant else ""))
    r = subprocess.run(["cc", "-O1", "-w", "-I", inc, "-I", os.path.join(TOP, "src", "rondo", "include"), "-I", lr,
                        "-o", exe, os.path.join(HERE, "pcebox", "opcontrol.c"), os.path.join(OUT, "cases.c"), "-ldl"],
                       capture_output=True, text=True)
    if r.returncode:
        print(r.stderr[-3000:])
        return 1
    rom = os.path.join(OUT, "ops.pce")
    img = bytearray(131072)
    for b in range(16):
        for off in range(8192):
            img[b * 8192 + off] = (b * 37 + off * 13 + (off >> 8) * 7 + 5) & 0xFF
    img[0:4] = bytes([0xEA, 0x4C, 0x00, 0xE0])
    for off in range(0x1FF0, 0x2000):
        img[off] = 0
    img[0x1FFE], img[0x1FFF] = 0x00, 0xE0            # reset vector
    img[0x1FF6:0x1FFE] = bytes([0x00, 0xE0] * 4)
    open(rom, "wb").write(img)
    print("opcontrol: %d cases generated%s" % (len(cases), " (planted: %s)" % a.plant if a.plant else ""))
    p = subprocess.run([exe, "--core", a.core, "--rom", rom], capture_output=True, text=True, timeout=600)
    print(p.stdout.strip())
    if p.stderr.strip():
        print(p.stderr.strip()[-600:])
    return p.returncode


if __name__ == "__main__":
    sys.exit(main())
