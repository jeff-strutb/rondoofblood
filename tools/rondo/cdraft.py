#!/usr/bin/env python3
"""cdraft.py -- draft C for every traced routine, instruction for instruction from its assembly.

    python3 tools/rondo/cdraft.py --only NAME[,NAME...] [--out DIR] [--as-filed]   from the cache, under a second, no slot
    cpu-run -j 1 -m 3G -- python3 tools/rondo/cdraft.py --cache                    build the cache (about a minute)
    cpu-run -j 1 -m 3G -- python3 tools/rondo/cdraft.py --fresh [--all|--only ...] generate without the cache

A draft is register-shaped C: one statement of ops6280.h per instruction a script ran, on the
register file `cpu`, so every access, push and pull is made where the assembly makes it, in its order.
It is a stepping stone, never counted as decompiled (docs/rondo/PLAN.md, section 1): the proper C is
written later, beside the draft, and checked against it.

What is emitted:
  - an instruction: its step (ops6280.h); a label where a branch lands; branches inside the routine
    are `if (...) goto L;` (labels and gotos are acceptable in a stepping stone);
  - JSR, BSR: call_orig(ret, target), the return address baked from the run address: whatever is
    resident at the target is entered by the hook, so overlays are right by construction;
  - a JMP, branch or fall-through to another routine: call_original(target); return; (a tail jump,
    no frame); JMP (abs) and JMP (abs,X): the pointer is read, then the same;
  - RTS, RTI: return; (the exit protocol reads the return target from the live stack);
  - bytes no script ran: untraced(addr), which counts, reports and ends the run (exit 3): a traced
    script never reaches one.
One C function per (distinct content, run address); the name is the intake's label of the
content's lowest disc location (r0043_100, with _RRRR appended when a content ran at several
addresses). Output (untracked, a function of the disc and the traces): DIR/drafts.c, DIR/drafts.csv
(name, run, length, disc, abi) and DIR/report.csv.

The cache. `--cache` writes every draft once into build/rondo/drafts/ of the MAIN checkout (one drafts.c, an
index.csv of each routine's label, run address, length and line range in it, members.csv of every disc or card
location and run address that holds each content, and stamp.txt). The cache is per checkout, but a worktree
shares the main checkout's: lookups try the main checkout's cache first and use it when its stamp equals the
stamp of the inputs this worktree sees (the disc image, the trace maps, cdraft.py, ops6280.h, intake.py,
huc.py: a worktree that edits cdraft.py or ops6280.h has another stamp, so it uses a cache of its own in its
own build/rondo/drafts/). The stamp hashes the sources' bytes and the big files' size and modification time.
`--only NAME` takes NAME as an intake label (r0043_100) or as a filed routine's name: a filed name is resolved
at lookup time from this tree's `@routine` and `@card` tags (disc location and run address) through
members.csv, so a routine filed a minute ago needs no regeneration. A stale cache is rebuilt by `--cache`
(a slot: run it under cpu-run); `--only` says so and exits 4 instead of taking a slot silently. In Python:
`import cdraft; text = cdraft.fetch("Name", as_filed=True)` returns the compilable unit (the includes and the
function, named abi_Name with as_filed, abi_<label> without) or raises KeyError.
"""
import argparse
import bisect
import collections
import csv
import os
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))


def intake_dir():
    """tools/rondo of this tree, or of the main checkout until the intake lands in this branch."""
    if os.path.exists(os.path.join(HERE, "intake.py")):
        return HERE
    common = subprocess.run(["git", "rev-parse", "--git-common-dir"], capture_output=True, text=True,
                            cwd=TOP).stdout.strip()
    main = os.path.dirname(os.path.abspath(os.path.join(TOP, common)))
    return os.path.join(main, "tools", "rondo")


sys.path.insert(0, HERE)
import huc  # noqa: E402


class _Lazy:
    """The intake and the assembly builder load the whole disc's traces: only a build imports them."""

    def __init__(self, name):
        self._n, self._m = name, None

    def __getattr__(self, k):
        if self._m is None:
            import importlib
            sys.path.insert(0, intake_dir())
            self._m = importlib.import_module(self._n)
        return getattr(self._m, k)


asmbuild = _Lazy("asmbuild")
intake = _Lazy("intake")

OUT = os.path.join(TOP, "build", "rondo", "drafter", "gen")

COND = {"bcc": "!(cpu.p & P_C)", "bcs": "(cpu.p & P_C)", "bne": "!(cpu.p & P_Z)", "beq": "(cpu.p & P_Z)",
        "bpl": "!(cpu.p & P_N)", "bmi": "(cpu.p & P_N)", "bvc": "!(cpu.p & P_V)", "bvs": "(cpu.p & P_V)",
        "bra": "1"}
VALUE_OPS = {"lda", "ldx", "ldy", "and", "ora", "eor", "adc", "sbc", "cmp", "cpx", "cpy", "bit"}
T_OPS = {"and", "ora", "eor", "adc"}
STORE_OPS = {"sta", "stx", "sty", "stz"}
RMW_F = {"asl": "f_asl", "lsr": "f_lsr", "rol": "f_rol", "ror": "f_ror", "inc": "f_inc", "dec": "f_dec",
         "tsb": "f_tsb", "trb": "f_trb"}
IMPLIED = {"tax", "txa", "tay", "tya", "tsx", "txs", "inx", "iny", "dex", "dey", "cla", "clx", "cly", "sax",
           "say", "sxy", "clc", "sec", "cld", "sed", "cli", "sei", "clv", "pha", "phx", "phy", "php", "pla",
           "plx", "ply", "plp", "csl", "csh"}
BLOCK = {"tii", "tdd", "tin", "tia", "tai"}


def addr_expr(mode, ops):
    """The effective address of an operand, as C; reads of zero page pointers happen in the helper."""
    v = ops[0]
    return {
        "zp": "0x%04X" % (0x2000 | v), "zpx": "ea_zpx(0x%02X)" % v, "zpy": "ea_zpy(0x%02X)" % v,
        "abs": "0x%04X" % v, "abx": "ea_abx(0x%04X)" % v, "aby": "ea_aby(0x%04X)" % v,
        "izp": "ea_ind(0x%02X)" % v, "izx": "ea_izx(0x%02X)" % v, "izy": "ea_izy(0x%02X)" % v,
    }[mode]


def value_expr(mode, ops):
    if mode == "imm":
        return "0x%02X" % ops[0]
    return "rd8(%s)" % addr_expr(mode, ops)


class Seg:
    """One piece of code in a draft: the routine itself (0), or the piece a branch or a fall-through runs on into."""

    def __init__(self, k, g, base):
        self.k, self.g, self.base = k, g, base
        self.used = set()
        self.prefix = "" if k == 0 else "S%d_" % k


class Draft:
    has_row = staticmethod(lambda g, base: True)   # does (group, run address) have a tag row of its own
    resolve = None          # (group, offset relative to its start) -> (group, offset) or None, set by build_all
    MAXSEG = 8

    def __init__(self, g, base, name):
        self.g, self.base, self.name = g, base, name
        self.segs = [Seg(0, g, base)]
        self.stat = collections.Counter()

    def label(self, seg, off):
        seg.used.add(off)
        return "%sL%03X" % (seg.prefix, off)

    def seg_for(self, g2, base2):
        for sg in self.segs:
            if sg.g is g2 and sg.base == base2:
                return sg
        if len(self.segs) >= self.MAXSEG:
            return None
        sg = Seg(len(self.segs), g2, base2)
        self.segs.append(sg)
        return sg

    def tail(self, addr, frompc):
        """A tail jump: the instruction at frompc makes it. The hybrid needs that address (the code a branch runs
        on into can be a piece inlined from another routine, outside this routine's own range)."""
        self.stat["tail"] += 1
        return "{ call_original_at(0x%04X, 0x%04X); return; }" % (frompc, addr)

    def jump_to(self, seg, addr, branch, frompc):
        """C that goes to run address addr: a label in this piece, the piece a branch runs on into (its code
        is part of this function: the C makes every access itself, no hand-off), or a tail jump to a routine
        entry (a JMP: the original's record has the entry)."""
        off = (addr - seg.base) & 0xFFFF
        if off < seg.g.n:
            if off in seg.g.istart:
                return "goto %s;" % self.label(seg, off)
            self.stat["untraced_arm"] += 1
            return "untraced(0x%04X);" % addr
        if branch and Draft.resolve:
            rel = off if off < 0x8000 else off - 0x10000
            r = Draft.resolve(seg.g, rel)
            if r:
                g2, off2 = r
                if off2 == 0 and Draft.has_row(g2, (addr - off2) & 0xFFFF):
                    return self.tail(addr, frompc)      # a routine's own start: the original enters it (a call event), its C is its own call
                if off2 in g2.istart:
                    s2 = self.seg_for(g2, (addr - off2) & 0xFFFF)
                    if s2:
                        self.stat["inlined"] += 1
                        return "goto %s;" % self.label(s2, off2)
                else:
                    self.stat["untraced_arm"] += 1
                    return "untraced(0x%04X);" % addr
        return self.tail(addr, frompc)

    def step(self, seg, idx):
        g, base = seg.g, seg.base
        _, o, ln, name, mode, ops, unrun = g.items[idx]
        pc = (base + o) & 0xFFFF
        nxt = (pc + ln) & 0xFFFF
        c = g.content
        if name in VALUE_OPS:
            f = "op_%s(%s);" % (name, value_expr(mode, ops))
            if self.tpend and name in T_OPS:
                f = "op_%s_t(%s);" % (name, value_expr(mode, ops))
            return f
        if name in STORE_OPS:
            return "op_%s(%s);" % (name, addr_expr(mode, ops))
        if name in RMW_F:
            if mode == "acc":
                return "OP_ACC(%s);" % RMW_F[name]
            return "OP_MEM(%s, %s);" % (RMW_F[name], addr_expr(mode, ops))
        if name in IMPLIED:
            return "op_%s();" % name
        if name == "nop":
            return ";"
        if name == "set":
            return "op_set();"
        if name[:3] in ("rmb", "smb"):
            return "op_%s(%s, 0x%02X);" % (name[:3], name[3], ops[0])
        if name[:3] in ("bbr", "bbs"):
            disp = c[o + ln - 1]
            disp = disp - 256 if disp & 0x80 else disp
            return "if (op_%s(%s, 0x%02X)) { %s }" % (name[:3], name[3], ops[0], self.jump_to(seg, (nxt + disp) & 0xFFFF, True, pc))
        if name in COND:
            disp = c[o + ln - 1]
            disp = disp - 256 if disp & 0x80 else disp
            if name == "bra" and ((nxt + disp) & 0xFFFF) == pc:
                self.stat["wait"] += 1
                return "for (;;) wait_interrupt();  /* a branch to itself */"
            go = self.jump_to(seg, (nxt + disp) & 0xFFFF, True, pc)
            return go if name == "bra" else "if (%s) { %s }" % (COND[name], go)
        if name in ("jsr", "bsr"):
            if name == "jsr":
                t = ops[0]
            else:
                disp = c[o + ln - 1]
                t = (nxt + (disp - 256 if disp & 0x80 else disp)) & 0xFFFF
            self.stat["call"] += 1
            return "call_orig(0x%04X, 0x%04X);" % (nxt, t)
        if name == "jmp":
            if mode == "abs":
                if ops[0] == pc:
                    self.stat["wait"] += 1
                    return "for (;;) wait_interrupt();  /* a jump to itself */"
                return self.jump_to(seg, ops[0], False, pc)
            ea = "0x%04X" % ops[0] if mode == "ind" else "ea_abx(0x%04X)" % ops[0]
            self.stat["computed"] += 1
            return "{ uint16_t t_ = op_jmp_ptr(%s); call_original(t_); return; }" % ea
        if name in ("rts", "rti"):
            return "return;"
        if name == "tst":
            imm = ops[0]
            return "op_tst(0x%02X, rd8(%s));" % (imm, addr_expr({"tzp": "zp", "tzx": "zpx", "tab": "abs",
                                                                 "tax": "abx"}[mode], (ops[1],)))
        if name in ("tam", "tma"):
            if name == "tma" and bin(ops[0]).count("1") != 1:
                self.stat["todo"] += 1
                return "untraced(0x%04X);" % pc
            return "op_%s(0x%02X);" % (name, ops[0])
        if name in ("st0", "st1", "st2"):
            return "op_%s(0x%02X);" % (name, ops[0])
        if name in BLOCK:
            if ops[2] == 0:
                self.stat["todo"] += 1
                return "untraced(0x%04X);" % pc
            return "op_%s(0x%04X, 0x%04X, 0x%04X);" % (name, ops[0], ops[1], ops[2])
        self.stat["todo"] += 1
        return "untraced(0x%04X);" % pc

    def emit_seg(self, seg, out):
        g, base = seg.g, seg.base
        body = []
        self.tpend = False
        for idx, it in enumerate(g.items):
            if it[0] == "d":
                _, o, n = it
                body.append((o, "untraced(0x%04X);" % ((base + o) & 0xFFFF), True))
                self.stat["untraced_bytes"] += n
                self.stat["untraced_items"] += 1
                continue
            _, o, ln, name, mode, ops, unrun = it
            if name == "set":
                self.tpend = True
            stmt = self.step(seg, idx)
            if name != "set" and name != "nop":
                self.tpend = False
            body.append((o, stmt, False, (base + o) & 0xFFFF, name))
            self.stat["instructions"] += 1
        last = g.items[-1]
        tail = None
        if last[0] == "i" and last[3] not in huc.ENDS:
            tail = self.jump_to(seg, (base + g.n) & 0xFFFF, True, (base + last[1]) & 0xFFFF)
            self.stat["fall_out"] += 1
        self.bodies[seg.k] = (body, tail)

    def text(self):
        self.bodies = {}
        i = 0
        while i < len(self.segs):          # emitting a piece can add the pieces it runs on into
            self.emit_seg(self.segs[i], None)
            i += 1
        out = []
        for seg in self.segs:              # labels are known only once every piece has been emitted
            body, tail = self.bodies[seg.k]
            if seg.k:
                out.append("    /* the code a branch or fall-through runs on into: %d bytes at %04X */" % (seg.g.n, seg.base))
            for b in body:
                if b[0] in seg.used:
                    out.append("%sL%03X:" % (seg.prefix, b[0]))
                if b[2]:
                    out.append("    %s  /* untraced */" % b[1])
                else:
                    out.append("    %s  /* %04X %s */" % (b[1], b[3], b[4]))
            if tail:
                out.append("    %s  /* runs on into the next routine */" % tail)
        return "\n".join(out)


def run_bases(it):
    """Per piece, the run addresses its executed bytes were fetched at (runmap.bin)."""
    rm = open(os.path.join(intake.TRACE, "runmap.bin"), "rb").read()
    los = [p.lo for p in it.pieces_in]
    out = collections.defaultdict(set)
    for i in range(len(rm) // 12):
        d, lg, bank, kind = struct.unpack_from("<IHBB", rm, i * 12)
        k = bisect.bisect_right(los, d) - 1
        if k < 0 or d >= it.pieces_in[k].hi:
            continue
        p = it.pieces_in[k]
        out[p.lo].add((lg - (d - p.lo)) & 0xFFFF)
    return out


def my_filed(disc):
    """This tree's filed routines (asmfile's tags) as the intake takes them: disc, length, run, name."""
    if os.environ.get("CDRAFT_TEST_NO_FILED"):       # the control: with no filed routine at all the drafts' bodies must not change
        return []
    sys.path.insert(0, HERE)
    import asmfile
    routines = asmfile.all_routines()
    out = []
    for i, r in enumerate(routines):
        nxt = routines[i + 1]["disc"] if i + 1 < len(routines) else len(disc)
        n = sum(ins[1] for ins in asmfile.span(disc, r, nxt))
        out.append({"disc": r["disc"], "end": n, "run": r["run"], "text": "", "name": r["name"], "labels": {},
                    "src": "filed"})
    return out


def load():
    """Every piece of traced code, grouped by content: (disc, intake). The pieces are cut from the traces and the
    catalog alone: no filed routine's span or tag takes part in the cut, so filing a routine (or filing it short)
    never changes any draft. The filed routines (this tree's @routine tags) only give names, matched by disc
    location and run address (it.filed)."""
    disc = open(asmbuild.TRACK, "rb").read()
    data = intake.load()
    dm, rows, es = data[1], data[2], data[3]
    it = intake.build([], disc, dm, rows, es, log=lambda *a: None)
    it.filed = my_filed(disc)
    it.rows_all = rows
    return disc, it


def make_resolver(it):
    """Where a relative branch lands, by disc position: the piece and offset, the same in every copy."""
    los = [p.lo for p in it.pieces_in]

    def resolve(g, rel):
        res = None
        for p in g.members:
            d = p.lo + rel
            k = bisect.bisect_right(los, d) - 1
            if k < 0 or d >= it.pieces_in[k].hi:
                return None
            q = it.pieces_in[k]
            r = (q.group, d - q.lo)
            if res is None:
                res = r
            elif res[0] is not r[0] or res[1] != r[1]:
                return None
        return res
    return resolve


def build_all(it, only=None, as_filed=False):
    """The drafts. `only`: names (label or filed name). as_filed: a draft of a filed routine's own
    location and run address takes the filed routine's name, to stand in for its hand-written C."""
    Draft.resolve = staticmethod(make_resolver(it))
    bases = run_bases(it)
    gb = {id(g): set().union(*[bases.get(p.lo, set()) for p in g.members]) for g in it.groups}
    Draft.has_row = staticmethod(lambda g, base: base in gb.get(id(g), ()))
    filed_at = {(f["disc"], f["run"]): f["name"] for f in it.filed}
    drafts = []
    norun = 0
    for g in it.groups:
        bs = sorted(set().union(*[bases.get(p.lo, set()) for p in g.members]))
        if not bs:
            norun += 1
            continue
        base_name = intake.label(g.canon)
        for b in bs:
            name = base_name if len(bs) == 1 else "%s_%04X" % (base_name, b)
            fn = next((filed_at[(p.lo, b)] for p in g.members if (p.lo, b) in filed_at), None)
            if only and name not in only and base_name not in only and fn not in only:
                continue
            d = Draft(g, b, fn if (as_filed and fn) else name)
            d.filed = fn
            d.rom = 0
            d.members = [("D", p.lo) for p in g.members]
            d.body = d.text()
            drafts.append(d)
    return drafts, norun


def card_drafts(names, rows=None):
    """Drafts of the System Card's executed code: its filed routines (cardfile.py), the catalog's card
    routines, and executed bytes in neither (bank 0 only: their run address is known), from the ROM image
    and cardmap.bin. A filed routine keeps its name; the rest are cNNNNN (ROM offset)."""
    sys.path.insert(0, HERE)
    import cardfile
    rom = open(cardfile.IMAGE, "rb").read()
    cm = open(os.path.join(intake.TRACE, "cardmap.bin"), "rb").read()
    filed_at = {} if os.environ.get("CDRAFT_TEST_NO_FILED") else \
        {(r["disc"], r["run"]): r["name"] for r in cardfile.spans()}      # names only, never a cut
    filed_names = set(filed_at.values())
    used = bytearray(len(rom))
    pieces = []                                   # (lo, hi, run, name)
    norun = 0
    for r in sorted((r for r in (rows or []) if r["space"] == "C"), key=lambda r: int(r["offset"], 16)):
        lo, n = int(r["offset"], 16), int(r["bytes"])
        if r["run"]:
            run = int(r["run"], 16)
        elif lo < 0x2000:
            run = 0xE000 + lo                  # bank 0 sits at $E000
        else:
            norun += 1
            continue
        o = lo
        while o < lo + n:
            if used[o]:
                o += 1
                continue
            e = o
            while e < lo + n and not used[e]:
                e += 1
            pieces.append((o, e, (run + o - lo) & 0xFFFF, "c%05X" % o))
            used[o:e] = b"\x01" * (e - o)
            o = e
    o = 0
    skipped = 0
    while o < len(rom):
        if (cm[o] & 3) and not used[o]:
            e = o
            while e < len(rom) and (cm[e] & 3) and not used[e]:
                e += 1
            if o < 0x2000:
                pieces.append((o, e, 0xE000 + o, "c%05X" % o))
            else:
                skipped += 1
            o = e
        else:
            o += 1
    out = []
    todo = []
    cpieces = []
    for lo, hi, run, name in sorted(pieces):
        name = filed_at.get((lo, run), name)      # a label: the filed name of the routine that starts here
        pc = intake.Piece(lo, hi, "card", ())
        g = intake.Group([pc], bytes(rom[lo:hi]))
        intake.decode_group(g, cm)
        pc.group = g
        cpieces.append(pc)
        todo.append((g, run, name))
    class CardIt:
        pieces_in = cpieces
    Draft.resolve = staticmethod(make_resolver(CardIt))
    Draft.has_row = staticmethod(lambda g, base: True)
    for g, run, name in todo:
        if names and name not in names:
            continue
        d = Draft(g, run, name)
        d.filed, d.rom = (name if name in filed_names else ""), 1
        d.members = [("C", p.lo) for p in g.members]
        if not names and name in filed_names:
            d.name = "card_" + name           # the hand-written abi_ of that name stays beside it
        d.body = d.text()
        out.append(d)
    if skipped or norun:
        print("card: %d catalog rows and %d executed runs outside bank 0 with no known run address, not drafted" % (
            norun, skipped))
    return out


SUPPORT = r'''/* drafter support: a path no script ran: counted, handed to the original. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ops6280.h"

unsigned long rondo_untraced_count;

void untraced(uint16_t addr)
{
    rondo_untraced_count++;
    hand_over(addr);
    fprintf(stderr, "UNTRACED %04X: a drafted routine reached bytes no script ran\n", addr);
    exit(3);
}
'''


def write(drafts, norun, out):
    os.makedirs(out, exist_ok=True)
    index, members = [], []
    with open(os.path.join(out, "drafts.c"), "w", encoding="ascii", newline="\n") as f:
        head = ("/* Drafted by tools/rondo/cdraft.py from the traced assembly: register-shaped C, a stepping stone. */\n"
                '#include "machine.h"\n#include "ops6280.h"\n\n')
        f.write(head)
        line = head.count("\n") + 1
        for d in drafts:
            text = ("/* %s: run %04X, %d bytes, %d instructions, content of disc %04X:%03X */\n" % (
                d.name, d.base, d.g.n, d.stat["instructions"], d.g.canon // 2048, d.g.canon % 2048) +
                "void abi_%s(void)\n{\n%s\n}\n\n" % (d.name, d.body))
            n = text.count("\n")
            f.write(text)
            index.append([d.name, "%04X" % d.base, d.g.n, d.g.canon, getattr(d, "rom", 0), line, line + n - 2])
            for sp, lo in getattr(d, "members", []):
                members.append([sp, lo, "%04X" % d.base, d.name])
            line += n
    with open(os.path.join(out, "support.c"), "w", encoding="ascii", newline="\n") as f:
        f.write(SUPPORT)
    with open(os.path.join(out, "index.csv"), "w", encoding="ascii", newline="\n") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["name", "run", "length", "disc", "rom", "first_line", "last_line"])
        w.writerows(index)
    with open(os.path.join(out, "members.csv"), "w", encoding="ascii", newline="\n") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["space", "offset", "run", "name"])
        w.writerows(members)
    with open(os.path.join(out, "drafts.csv"), "w", encoding="ascii", newline="\n") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["name", "run", "length", "disc", "abi", "filed", "rom"])
        for d in drafts:
            w.writerow([d.name, "%04X" % d.base, d.g.n, d.g.canon, "abi_" + d.name, d.filed or "", getattr(d, "rom", 0)])
    tot = collections.Counter()
    with open(os.path.join(out, "report.csv"), "w", encoding="ascii", newline="\n") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["name", "run", "bytes", "instructions", "untraced_arms", "untraced_bytes", "tails", "calls",
                    "computed", "fall_out", "todo", "inlined"])
        for d in drafts:
            s = d.stat
            w.writerow([d.name, "%04X" % d.base, d.g.n, s["instructions"], s["untraced_arm"], s["untraced_bytes"],
                        s["tail"], s["call"], s["computed"], s["fall_out"], s["todo"], s["inlined"]])
            tot.update(s)
    print("%d drafts (%d contents with no executed byte skipped); %d instructions, %d calls, %d tails, "
          "%d computed jumps, %d untraced arms, %d fall-outs, %d untranslatable (todo), %d branches run on into another piece inline" % (
              len(drafts), norun, tot["instructions"], tot["call"], tot["tail"], tot["computed"],
              tot["untraced_arm"], tot["fall_out"], tot["todo"], tot["inlined"]))


def main_root():
    common = subprocess.run(["git", "rev-parse", "--git-common-dir"], capture_output=True, text=True,
                            cwd=TOP).stdout.strip()
    return os.path.dirname(os.path.abspath(os.path.join(TOP, common)))


def _sha(path, memo):
    """sha1 of a file's bytes. The memo (path, size, mtime -> sha1) only saves time: a changed mtime with the same
    bytes gives the same hash, so the stamp itself depends on content alone."""
    import hashlib
    st = os.stat(path)
    key = "%s|%d|%d" % (os.path.realpath(path), st.st_size, st.st_mtime_ns)
    if key in memo:
        return memo[key]
    h = hashlib.sha1()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    memo[key] = h.hexdigest()
    return memo[key]


def stamp():
    """What the drafts are a function of, by content: the disc image, the card image, the merged code maps and
    every script's edges (intake.load reads them), and the sources that cut and write a draft. No path, time or
    commit takes part: a worktree whose inputs hold the same bytes has the same stamp as the main checkout."""
    import glob
    import hashlib
    main = main_root()
    memo_path = os.path.join(main, "build", "rondo", "drafts.hashes")
    memo = {}
    try:
        for ln in open(memo_path):
            k, _, v = ln.rstrip("\n").rpartition(" ")
            memo[k] = v
    except OSError:
        pass
    before = len(memo)
    h = hashlib.sha1()
    for path in (os.path.join(HERE, "cdraft.py"), os.path.join(TOP, "src", "rondo", "include", "ops6280.h"),
                 os.path.join(intake_dir(), "intake.py"), os.path.join(intake_dir(), "catalog.py"),
                 os.path.join(HERE, "huc.py")):
        h.update(os.path.basename(path).encode() + b"\0" + open(path, "rb").read())
    big = [os.path.join(main, "build", "rondo", "disc", "track02.bin"),
           os.path.join(main, "build", "rondo", "system", "syscard3.pce")]
    big += [os.path.join(main, "build", "rondo", "trace", n) for n in ("discmap.bin", "runmap.bin", "cardmap.bin")]
    big += sorted(glob.glob(os.path.join(main, "build", "rondo", "trace", "*", "edges.csv")))
    for path in big:
        h.update(os.path.relpath(path, main).encode() + b"\0" + _sha(path, memo).encode())
    if len(memo) != before:
        try:
            os.makedirs(os.path.dirname(memo_path), exist_ok=True)
            with open(memo_path + ".tmp", "w") as f:
                for k, v in sorted(memo.items()):
                    f.write("%s %s\n" % (k, v))
            os.replace(memo_path + ".tmp", memo_path)
        except OSError:
            pass
    return h.hexdigest()


def cache_dirs():
    return [os.path.join(main_root(), "build", "rondo", "drafts"), os.path.join(TOP, "build", "rondo", "drafts")]


def current_cache():
    """The first cache folder whose stamp equals the inputs' stamp, or None."""
    want = stamp()
    seen = set()
    for d in cache_dirs():
        if d in seen:
            continue
        seen.add(d)
        try:
            if open(os.path.join(d, "stamp.txt")).readline().strip() == want:
                return d
        except OSError:
            pass
    return None


def build_cache(out):
    disc, it = load()
    drafts, norun = build_all(it, None, False)
    drafts += card_drafts(None, it.rows_all)
    tmp = out + ".new"
    import shutil
    shutil.rmtree(tmp, ignore_errors=True)
    write(drafts, norun, tmp)
    with open(os.path.join(tmp, "stamp.txt"), "w") as f:
        f.write(stamp() + "\n%d drafts, built by tools/rondo/cdraft.py --cache\n" % len(drafts))
    shutil.rmtree(out, ignore_errors=True)
    os.rename(tmp, out)
    return len(drafts)


def resolve(cache, name):
    """Index rows (a list) for a label or a filed routine's name."""
    rows = {}
    with open(os.path.join(cache, "index.csv"), newline="") as f:
        for r in csv.DictReader(f):
            rows.setdefault(r["name"], r)
    if name in rows:
        return [rows[name]]
    keys = []
    import asmfile
    for r in asmfile.all_routines():
        if r["name"] == name:
            keys.append(("D", str(r["disc"]), "%04X" % r["run"]))
    try:
        import cardfile
        for r in cardfile.routines():
            if r["name"] == name:
                keys.append(("C", str(r["disc"]), "%04X" % r["run"]))
    except Exception:
        pass
    found = []
    with open(os.path.join(cache, "members.csv"), newline="") as f:
        for r in csv.DictReader(f):
            if (r["space"], r["offset"], r["run"]) in keys and r["name"] in rows:
                found.append(rows[r["name"]])
    if not found:
        raise KeyError("%s: no draft (not a label, not a filed routine with traced code at its run address)" % name)
    return found


def fetch(name, as_filed=False, cache=None):
    """The compilable unit for one routine from the cache: the includes and its function."""
    cache = cache or current_cache()
    if not cache:
        raise LookupError("the draft cache is stale or missing: cpu-run -j 1 -m 3G -- python3 tools/rondo/cdraft.py --cache")
    lines = open(os.path.join(cache, "drafts.c")).read().split("\n")
    out = ['#include "machine.h"', '#include "ops6280.h"', ""]
    for r in resolve(cache, name):
        text = "\n".join(lines[int(r["first_line"]) - 1:int(r["last_line"])])
        if as_filed:
            text = text.replace("void abi_%s(void)" % r["name"], "void abi_%s(void)" % name, 1)
        out.append(text + "\n")
    return "\n".join(out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=OUT)
    ap.add_argument("--only", default="")
    ap.add_argument("--as-filed", action="store_true")
    ap.add_argument("--all", action="store_true", help="every distinct content, disc and card, named by label")
    ap.add_argument("--cache", action="store_true", help="build the cache in the main checkout (or this tree's own)")
    ap.add_argument("--cache-dir", default="", help="with --cache: the folder to build into")
    ap.add_argument("--fresh", action="store_true", help="generate without the cache")
    a = ap.parse_args()
    only = set(x for x in a.only.split(",") if x)
    if a.cache:
        out = a.cache_dir or (cache_dirs()[0] if TOP == main_root() else cache_dirs()[1])
        print("cache: %d drafts in %s" % (build_cache(out), out))
        return 0
    if only and not a.fresh and not a.all:
        cache = current_cache()
        if not cache:
            print("cdraft: the draft cache is stale or missing; build it: cpu-run -j 1 -m 3G -- python3 tools/rondo/cdraft.py --cache",
                  file=sys.stderr)
            return 4
        os.makedirs(a.out, exist_ok=True)
        text = "".join(fetch(n, a.as_filed, cache) + "\n" for n in sorted(only))
        # one unit: the includes once
        body = [ln for ln in text.split("\n") if not ln.startswith("#include")]
        with open(os.path.join(a.out, "drafts.c"), "w") as f:
            f.write('#include "machine.h"\n#include "ops6280.h"\n' + "\n".join(body))
        import shutil
        with open(os.path.join(a.out, "support.c"), "w") as f:
            f.write(SUPPORT)
        print("cdraft: %d routines from %s into %s" % (len(only), cache, a.out))
        return 0
    disc, it = load()
    drafts, norun = build_all(it, only, a.as_filed)
    if a.as_filed and only:
        drafts += card_drafts(only)
    elif a.all:
        drafts += card_drafts(None, it.rows_all)
    write(drafts, norun, a.out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
