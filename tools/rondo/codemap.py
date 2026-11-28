#!/usr/bin/env python3
"""codemap.py -- the static code map of the disc: what loads where, what is code, and the copies.

Nothing is run. The map is worked out from the disc alone:

  Loads. The card reads user sectors 0 and 1 to $2800 and calls $2B26, which reads the boot program
  (sector 2) to $3000. The boot program reads the blocks of its own table ($312F) and jumps to the
  kernel at $4000; with a card below 3.0 it reads the old-card program (20 sectors from $23) to $4000
  instead. Every later load is a scene (LoadSceneScript, disc_load.c): SceneRecords ($74D1 in the
  kernel), five bytes each, give a scene's disc address and its list of ops, which map MPR4-6 (op 0),
  read sectors at an offset from the scene's address to SceneReadDestinations[op] (ops 1-$7F), go to
  another list ($C0), call the scene overlay ($80-$83), or end. The boot's table uses the same ops,
  every read to $8000. Replaying a scene's ops over the kernel gives that scene's memory: which disc
  byte each physical bank holds.

  Code. In each memory the code is followed from its roots (the card's call of $2B26, the boot
  program, the kernel's vector table, the scene overlay's entry): every branch, JSR and JMP, and
  JMP (abs,X) and JMP (abs) tables. The follower keeps what is known of A, X, Y, the carry, a small
  stack and every MPR, so a call into a window goes to the bank the code mapped there
  (`LDA #k : CLC : ADC zRamBaseBank : TAM`) and a JSR continues with the MPRs the callee leaves.

    python3 tools/rondo/codemap.py            the map, into build/rondo/codemap/
    python3 tools/rondo/codemap.py --check    the map against every filed @routine and @run
    python3 tools/rondo/codemap.py --progress M1 and M2 by disc area: docs/rondo/progress.svg and the README
    python3 tools/rondo/codemap.py --todo     every routine found, each body once, and whether it is done
"""
import array
import collections
import csv
import glob
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import huc  # noqa: E402

TRACK = os.path.join(TOP, "build", "rondo", "disc", "track02.bin")
OUT = os.path.join(TOP, "build", "rondo", "codemap")
SECTOR = 0x800
BANK = 0x2000
BASE = 0x68                     # the first Super System Card RAM bank, which ex_memopen ($E0DE) returns on a 3.0 card
TRACK_START = 0xF32             # the BIOS address of track 02's first user sector
ZP_BASE = 0x20DB                # zRamBaseBank, where the boot program keeps BASE
SCENE_RECORDS, SCENE_DESTINATIONS, SCENE_CALLS = 0x74D1, 0x70F8, 0x72C8
KERNEL_SECTOR = 0x43
STAGE_SCENES, STAGE_COUNT = 0x74B1, 17                  # up to StageScenesSecret
STAGE_SCENES_SECRET, STAGE_SECRET_COUNT = 0x74C2, 15    # up to SceneRecords
BOOT_TABLE = 0x312F
WORK = 0xF8                     # work RAM, logical $2000-$3FFF
CARD_ROM = 0x00                 # the card's bank 0 at $E000
STACK_MAX = 8
RET = (("ret", 1), ("ret", 0))  # a routine's stack on entry: the return address JSR pushed, high byte first
RAM_MAX = 16
ALL = "all"                     # a write set: every address (an indirect store)
CARD_BYTES = {0xFFF4: BASE, 0xFFF5: 0x80}       # the 3.0 card's ROM: first Super RAM bank, first CD RAM bank
ENTRY_LOOK = 64                 # instructions a table entry must decode through to a return, cleanly
TABLE_MAX = 128


def hx(v, n=4):
    return "-" if v is None else f"{v:0{n}X}"


# ----------------------------------------------------------------------------------------- memory

class Mem:
    """Which disc byte each byte of each physical bank holds (-1: none from the disc)."""

    def __init__(self, other=None):
        self.b = {} if other is None else {k: array.array("l", v) for k, v in other.b.items()}

    def load(self, sector, count, logical, mpr):
        for i in range(count * SECTOR):
            la = (logical + i) & 0xFFFF
            pb = mpr[la >> 13]
            if pb is None:
                continue
            a = self.b.get(pb)
            if a is None:
                a = self.b[pb] = array.array("l", [-1]) * BANK
            a[la & (BANK - 1)] = sector * SECTOR + i

    def at(self, pb, off):
        a = self.b.get(pb)
        return -1 if a is None else a[off]


class Disc:
    def __init__(self):
        self.buf = open(TRACK, "rb").read()
        self._dec = {}

    def decode(self, d, pc):
        k = (d, pc)
        v = self._dec.get(k)
        if v is None:
            v = self._dec[k] = huc.decode(self.buf, d, pc)
        return v


def kernel_disc(run):
    """The kernel: run $4000-$7FFF is sector $43 on (MPR2-3 = BASE+1, BASE+2)."""
    return KERNEL_SECTOR * SECTOR + run - 0x4000 if 0x4000 <= run < 0x8000 else None


def ops_list(read, head, sector, fixed_dest=None, limit=64):
    """A list of load ops from head: [("banks", (b4, b5, b6)) | ("read", sector, count, dest) | ("call", op)].
    read(addr) gives a byte of the list's memory; None once the list leaves it. Returns (ops, ended)."""
    ops, p = [], head
    for _ in range(limit):
        op = read(p)
        if op is None:
            return ops, False
        if op == 0:
            bs = tuple(read(p + i) for i in (1, 2, 3))
            if None in bs:
                return ops, False
            ops.append(("banks", bs))
            p += 4
        elif op < 0x80:
            off, count = read(p + 1), read(p + 2)
            if fixed_dest is not None:
                dest = fixed_dest
            else:
                lo, hi = read(SCENE_DESTINATIONS + 2 * op), read(SCENE_DESTINATIONS + 2 * op + 1)
                dest = None if lo is None or hi is None else lo | hi << 8
            if None in (off, count, dest):
                return ops, False
            ops.append(("read", sector + off, count, dest))
            p += 3
        elif op == 0xC0 and fixed_dest is None:
            lo, hi = read(p + 1), read(p + 2)
            if lo is None or hi is None:
                return ops, False
            p = lo | hi << 8
        elif op in (0x80, 0x81, 0x82, 0x83) and fixed_dest is None:
            ops.append(("call", op))
            p += 1
        else:
            return ops, True
    return ops, False


def replay(mem, ops, mpr):
    mpr = list(mpr)
    for op in ops:
        if op[0] == "banks":
            mpr[4:7] = [BASE + b for b in op[1]]
        elif op[0] == "read":
            mem.load(op[1], op[2], op[3], mpr)
    return mpr


class Memories:
    """Every memory the code can run in: the boot's, the kernel's, and one per scene."""

    def __init__(self, disc):
        self.disc = disc
        buf = disc.buf
        boot = Mem()
        boot.load(0, 2, 0x2800, [None, WORK] + [None] * 6)
        boot.load(2, 1, 0x3000, [None, WORK] + [None] * 6)
        # the old-card program: MPR2-5 = $80-$83 from $FFF5, 20 sectors to $4000
        boot.load(0x23, 20, 0x4000, [None, WORK, 0x80, 0x81, 0x82, 0x83, None, None])
        brd = lambda a: buf[2 * SECTOR + a - 0x3000] if 0x3000 <= a < 0x3800 else None
        sector = (brd(BOOT_TABLE) | brd(BOOT_TABLE + 1) << 8 | brd(BOOT_TABLE + 2) << 16) - TRACK_START
        head = brd(BOOT_TABLE + 3) | brd(BOOT_TABLE + 4) << 8
        self.boot_ops, ok = ops_list(brd, head, sector, fixed_dest=0x8000)
        assert ok, "the boot table's list does not end"
        kernel = Mem()
        self.kernel_mpr = replay(kernel, self.boot_ops, [None, WORK, BASE + 1, BASE + 2, None, None, None, CARD_ROM])
        self.kernel_mpr[2:4] = [BASE + 1, BASE + 2]
        for pb, a in kernel.b.items():
            boot.b.setdefault(pb, a)
        self.boot, self.kernel = boot, kernel
        krd = lambda a: buf[kernel_disc(a)] if kernel_disc(a) is not None else None
        self.scenes = []
        for n in range(256):
            r = SCENE_RECORDS + 5 * n
            if kernel_disc(r + 4) is None:
                break
            sector = (krd(r) | krd(r + 1) << 8 | krd(r + 2) << 16) - TRACK_START
            head = krd(r + 3) | krd(r + 4) << 8
            if not 0 <= sector < len(buf) // SECTOR or kernel_disc(head) is None:
                break
            ops, ended = ops_list(krd, head, sector)
            if not ended:
                break
            self.scenes.append({"n": n, "sector": sector, "list": head, "ops": ops})
        self.layered = self.layers(krd)


    def stack(self, scenes):
        """The kernel's memory after these scenes are loaded, in this order: a later read replaces an earlier."""
        mem = Mem(self.kernel)
        by = {s["n"]: s for s in self.scenes}
        for n in scenes:
            if n in by:
                replay(mem, by[n]["ops"], self.kernel_mpr)
        return mem

    def layers(self, krd):
        """Every memory the game builds, from the order it loads scenes in (game_mode.s, stage_load.s, interrupt.s):
        the boot's scenes $04 and $06 first; a stage is the character's scene ($00 Richter, $01 Maria), then
        StageScenes[stage] + character, then StageScenes[stage] + 2 (StageLoad); the other loads are the fixed
        sequences of the mode code, each on the boot's scenes alone and with either character loaded."""
        out = []
        boot = [0x04, 0x06]
        out.append(("boot04-06", boot))
        n = len(self.scenes)
        for table, count, tag in ((STAGE_SCENES, STAGE_COUNT, "stage"), (STAGE_SCENES_SECRET, STAGE_SECRET_COUNT, "secret")):
            for k in range(count):
                t = krd(table + k)
                if not t or t + 2 >= n:
                    continue
                for c in (0, 1):
                    out.append((f"{tag}{k:02X}-{'RM'[c]}", boot + [c, t + c, t + 2]))
        seqs = [[0x05], [0x07, 0x08, 0x09], [0x5C, 0x5D], [0x5E], [0x0E, 0x0F], [0x0C, 0x0D], [0x0A, 0x0B],
                [0x02, 0x03], [0x58, 0x59], [0x5A, 0x5B]]
        for q in seqs:
            name = "seq" + "-".join(f"{x:02X}" for x in q)
            out.append((name, boot + q))
            for c in (0, 1):
                out.append((f"{name}-{'RM'[c]}", boot + [c] + q))
        covered = {x for _, sc in out for x in sc}
        for s_ in self.scenes:
            if s_["n"] not in covered:
                out.append((f"scene{s_['n']:02X}", boot + [s_["n"]]))
        return out

    def all(self):
        if not hasattr(self, "_all"):
            self._all = [("boot", self.boot), ("kernel", self.kernel)] + [
                (name, self.stack(scenes)) for name, scenes in self.layered]
        return self._all


# ---------------------------------------------------------------------------------------- the state

class St:
    """What is known at an instruction: MPRs, A, X, Y, carry, a stack of pushed values, and the values
    stored at fixed RAM addresses (ram: sorted (address, value) pairs). None: unknown."""

    __slots__ = ("mpr", "a", "x", "y", "c", "stk", "ram")

    def __init__(self, mpr, a=None, x=None, y=None, c=None, stk=(), ram=()):
        self.mpr, self.a, self.x, self.y, self.c, self.stk, self.ram = tuple(mpr), a, x, y, c, stk, ram

    def key(self):
        return (self.mpr, self.a, self.x, self.y, self.c, self.stk, tuple(sorted(self.ram, key=lambda kv: kv[0])))

    def copy(self, **kw):
        s = St(self.mpr, self.a, self.x, self.y, self.c, self.stk, self.ram)
        for k, v in kw.items():
            setattr(s, k, v)
        return s

    def join(self, o):
        j = lambda p, q: p if p == q else None
        stk = self.stk if self.stk == o.stk else None
        both = set(o.ram)
        ram = tuple(kv for kv in self.ram if kv in both)
        return St(tuple(j(p, q) for p, q in zip(self.mpr, o.mpr)), j(self.a, o.a), j(self.x, o.x), j(self.y, o.y),
                  j(self.c, o.c), stk, ram)

    def read(self, addr):
        for k, v in self.ram:
            if k == addr:
                return v
        return None

    def write(self, addr, v):
        """Most recent last; past RAM_MAX the oldest is forgotten."""
        r = [(k, w) for k, w in self.ram if k != addr]
        if v is not None:
            r.append((addr, v))
        self.ram = tuple(r[-RAM_MAX:])

    def kill(self, writes):
        if writes is ALL:
            self.ram = ()
        elif writes:
            self.ram = tuple((k, v) for k, v in self.ram if k not in writes)


def add8(a, b, c):
    if a is None or b is None or c is None:
        return None, None
    s = a + b + c
    return s & 0xFF, s >> 8


def sub8(a, b, c):
    if a is None or b is None or c is None:
        return None, None
    s = a - b - (1 - c)
    return s & 0xFF, 0 if s < 0 else 1


def bios(target, st):
    """The card's entries the boot and kernel rely on, as their effect on what is known."""
    s = st.copy(a=None, x=None, y=None, c=None)
    if target == 0xE0DE:            # ex_memopen: A the first Super RAM bank, carry clear (3.0)
        s.a, s.c = BASE, 0
    return s


# ------------------------------------------------------------------------------------- the follower

class Map:
    """The code found in one memory."""

    def __init__(self, disc, mem, name):
        self.disc, self.mem, self.name = disc, mem, name
        self.alts = {}
        self.gstores = collections.defaultdict(set)   # fixed address -> the values stored there (None: unknown)
        self.gconst = {}                              # fixed address -> its one value, from a first pass
        self.starts = {}            # disc offset of an instruction start -> set of run addresses
        self.entries = {}           # (disc offset, run address) -> how it was entered
        self.bad = []               # (entry, place, why) where following met what code cannot be
        self.unplaced = collections.Counter()   # why a target could not be placed in this memory
        self.fns = {}               # (disc offset, run, entry MPRs) -> exit MPRs (None: no return found)
        self.bodies = {}            # (disc offset, run, MPRs) of an entry -> {(instruction start, run): length}
        self.graph = collections.defaultdict(set)   # entry key -> the entry keys it calls or tail-jumps to
        self.rootkeys = set()
        self.badkeys = set()
        self.caller = None
        self.curkey = None
        self.consumed = set()       # entry keys that pull their caller's return address
        self.inline = set()         # entry keys that jump through a table of words right after their call
        self.roots = []             # (run, MPRs, how): handlers the code gives the card, followed after
        self.rooted = set()
        self.busy = set()

    def badkey(self, key):
        self.badkeys.add(key)

    def kept(self):
        """The entries reachable from the roots without passing through one in the wrong context: in a memory a
        scene built, an entry in a bank other than the kernel's whose own path runs into what cannot be code
        (the memory is not one that routine runs in). Returns {instruction start: {run}} of the kept bodies."""
        home = (BASE + 1, BASE + 2) if self.name != "boot" else None
        def wrong(k):
            if home is None or k not in self.badkeys:
                return False
            return k[2][k[1] >> 13] not in home
        keep, work = set(), [k for k in self.rootkeys if not wrong(k)]
        while work:
            k = work.pop()
            if k in keep:
                continue
            keep.add(k)
            work += [c for c in self.graph.get(k, ()) if c not in keep and not wrong(c)]
        out = {}
        for k in keep:
            for (d, pc) in self.bodies.get(k, ()):
                out.setdefault(d, set()).add(pc)
        self.keep = keep
        return out

    def drain(self):
        """Follow the handlers given to the card, until no new one appears."""
        while self.roots:
            run, mpr, how = self.roots.pop()
            if (run, mpr) in self.rooted:
                continue
            self.rooted.add((run, mpr))
            d, why = self.place(run, mpr)
            if d is None:
                self.unplaced[why] += 1
                continue
            self.call(d, run, St(mpr), how)

    def place(self, pc, mpr):
        """The disc offset of the byte at pc under these MPRs, or None, with why: only what this memory loaded."""
        pb = mpr[pc >> 13]
        if pb is None:
            return None, "window unmapped"
        if pb in (CARD_ROM, 0xFF):
            return None, "card or I/O"
        d = self.mem.at(pb, pc & (BANK - 1))
        return (d, None) if d >= 0 else (None, "bank holds no disc bytes there")

    def places(self, pc, mpr):
        """Every disc offset the byte at pc can be: this memory's, or, where it loaded nothing into that bank, what
        the other memories load there (RAM keeps an earlier scene's reads). [], with why, when none."""
        d, why = self.place(pc, mpr)
        if d is not None:
            return [d], None
        if why != "bank holds no disc bytes there":
            return [], why
        # What an earlier scene left in this bank is not used yet: it needs the order the game loads its scenes in
        # (the kernel's stage tables). Following every scene's version of the bank is too broad: it reaches banks
        # that hold a stage's data in other scenes.
        return [], why

    def call(self, d, pc, st, how):
        """Follow a routine entered at disc offset d, run address pc, with state st; the MPRs it returns with."""
        key = (d, pc, st.mpr)
        self.entries.setdefault((d, pc), how)
        if self.caller is None:
            self.rootkeys.add(key)
        else:
            self.graph[self.caller].add(key)
        if key in self.fns:
            return self.fns[key]
        if key in self.busy:
            return None, ALL, ()
        self.busy.add(key)
        outer, self.caller = self.caller, key
        ex = self.body(d, pc, st)
        self.caller = outer
        self.busy.discard(key)
        self.fns[key] = ex
        return ex

    def body(self, d0, pc0, st0):
        disc, buf = self.disc, self.disc.buf
        seen = {}
        work = [(d0, pc0, st0)]
        exits = None
        exram = None                # the fixed-address values every return leaves
        writes = set()
        key0 = (d0, pc0, st0.mpr)
        mine = self.bodies.setdefault(key0, {})
        while work:
            d, pc, st = work.pop()
            k = (d, pc, st.mpr)             # paths that mapped different banks are followed apart
            old = seen.get(k)
            if old is not None:
                j = old.join(st)
                if j.key() == old.key():
                    continue
                st = j
            seen[k] = st
            if d + 7 > len(buf):
                self.badkey(key0); self.bad.append(((d0, pc0), d, "runs off the disc"))
                continue
            n, name, mode, ops, target = disc.decode(d, pc)
            if name is None:
                self.badkey(key0); self.bad.append(((d0, pc0), d, "undefined opcode"))
                continue
            if name == "brk":
                self.badkey(key0); self.bad.append(((d0, pc0), d, "BRK"))
                continue
            if (pc & (BANK - 1)) + n > BANK:
                # the bytes past the window's end are the next window's: fine when they are the next disc bytes
                k = BANK - (pc & (BANK - 1))
                nd, _ = self.place((pc + k) & 0xFFFF, st.mpr)
                if nd != d + k:
                    self.badkey(key0); self.bad.append(((d0, pc0), d, "instruction crosses a bank"))
                    continue
            nxt_d = None
            pn = (pc + n) & 0xFFFF
            if pn >> 13 == pc >> 13:
                nxt_d = d + n
            else:
                nxt_d, _ = self.place(pn, st.mpr)
            self.starts.setdefault(d, set()).add(pc)
            mine[(d, pc)] = n
            self.curkey = key0
            s = self.effect(name, mode, ops, st)
            if self.stored is not None and writes is not ALL:
                writes |= self.stored
            fall = name not in huc.ENDS
            if name == "rts":
                pushed = self.rts_targets(st, pc)
                if pushed:                          # an address pushed and returned to: a jump, not a return
                    self.curkey = key0
                    for t, how in pushed:
                        for td in self.places(t, s.mpr)[0]:
                            self.call(td, t, St(s.mpr), how)
                    continue
            if name in ("rts", "rti"):
                exits = s.mpr if exits is None else tuple(p if p == q else None for p, q in zip(exits, s.mpr))
                exram = set(s.ram) if exram is None else exram & set(s.ram)
                continue
            if (mode == "rel" or mode == "zrel") and name != "bsr":
                taken = self.branch(name, st)
                if taken is not False:
                    self.go(work, target, s, (d0, pc0), d)
                if taken is True:
                    fall = False
            if name == "jmp" and mode == "abs":
                if target >> 13 == pc >> 13:
                    self.go(work, target, s, (d0, pc0), d)
                else:                               # a tail jump into another window: a routine of its own
                    tds, why = self.places(target, s.mpr)
                    if not tds:
                        self.unplaced[why] += 1
                    for td in tds:
                        ex, w, er = self.call(td, target, s.copy(), "jmp")
                        if ex is not None:
                            exits = ex if exits is None else tuple(p if p == q else None for p, q in zip(exits, ex))
                            exram = set(er) if exram is None else exram & set(er)
                        if writes is not ALL:
                            writes = ALL if w is ALL else writes | w
            if name == "jsr" or name == "bsr":
                if target >= 0xE000:
                    if target == 0xE05D and s.x is not None and s.y is not None:
                        self.roots.append((s.y << 8 | s.x, s.mpr, f"ex_setvec vector {hx(s.a, 2)}"))
                    s = bios(target, s)
                else:
                    tds, why = self.places(target, s.mpr)
                    if not tds:
                        self.unplaced[why] += 1
                        s = s.copy(a=None, x=None, y=None, c=None, ram=())
                        writes = ALL
                    else:
                        exm, wall, erj = None, set(), None
                        for td in tds:
                            ex, w, er = self.call(td, target, s.copy(stk=RET), "jsr")
                            erj = set(er) if erj is None else erj & set(er)
                            ck = (td, target, s.mpr)
                            if ck in self.consumed:
                                fall = False            # the callee pulls its return address: it does not come back
                            if ck in self.inline:
                                for t in self.words((pc + 3) & 0xFFFF, 0, None, s.mpr):
                                    for td2 in self.places(t, s.mpr)[0]:
                                        self.call(td2, t, St(s.mpr), "inline table")
                            ex = ex if ex is not None else s.mpr
                            exm = ex if exm is None else tuple(p if p == q else None for p, q in zip(exm, ex))
                            wall = ALL if (w is ALL or wall is ALL) else wall | w
                        s = s.copy(mpr=exm, a=None, x=None, y=None, c=None)
                        s.kill(wall)
                        for k_, v_ in sorted(erj or (), key=lambda kv: kv[0]):
                            if wall is ALL or k_ in wall:
                                s.write(k_, v_)         # what the callee leaves at an address it writes
                        if writes is not ALL:
                            writes = ALL if wall is ALL else writes | wall
            if name == "jmp" and mode in ("ind", "iax"):
                self.curkey = key0
                for t in self.table(ops[0], mode, s, d, pc):
                    for td in self.places(t, s.mpr)[0]:
                        self.call(td, t, s.copy(a=None, x=None, y=None, c=None), "table")
            if fall:
                if nxt_d is None:
                    self.badkey(key0); self.bad.append(((d0, pc0), d, "falls into an unmapped window"))
                else:
                    work.append((nxt_d, pn, s))
        if exram is not None and writes is not ALL:
            exram = {kv for kv in exram if kv[0] in writes}
        return exits, writes, tuple(exram or ())

    def rts_targets(self, st, pc):
        """What an RTS goes to when the code pushed it: the top two bytes of the stack a known address (high pushed
        first), or a word of a table (each entry of it, read as a jump table), plus one; and a known address pushed
        beneath it, where those return to. [(run, how)]; [] for a plain return."""
        stk = st.stk
        if not stk or len(stk) < 2:
            return []
        lo, hi = stk[-1], stk[-2]
        out = []
        if isinstance(lo, int) and isinstance(hi, int):
            out.append(((hi << 8 | lo) + 1 & 0xFFFF, "pushed address"))
        elif isinstance(lo, tuple) and isinstance(hi, tuple) and lo[0] == "tbl" and hi == ("tbl", lo[1] + 1):
            for w in self.words(lo[1], 0, None, st.mpr, plus=1):
                out.append((w, "pushed from a table"))
        else:
            return []
        if len(stk) >= 4 and isinstance(stk[-3], int) and isinstance(stk[-4], int):
            out.append(((stk[-4] << 8 | stk[-3]) + 1 & 0xFFFF, "pushed return address"))
        return out

    def go(self, work, target, st, entry, at):
        tds, why = self.places(target, st.mpr)
        if not tds:
            self.unplaced[why] += 1
        for td in tds:
            work.append((td, target, st))

    def branch(self, name, st):
        """True: always taken; False: never; None: either."""
        if name == "bcs" and st.c is not None:
            return st.c == 1
        if name == "bcc" and st.c is not None:
            return st.c == 0
        if name == "bra":
            return True
        return None

    def table(self, addr, mode, st, d, pc):
        """The targets of JMP (abs,X) or JMP (abs): the words of the table, from the start (abs,X with X
        unknown) or the one X points at, while each is a place this memory holds."""
        out = []
        if mode == "ind":
            lo, hi = st.read(addr), st.read(addr + 1)
            if isinstance(lo, tuple) and isinstance(hi, tuple) and hi[1] == lo[1] + 1:
                return self.words(lo[1], 0, None, st.mpr)           # a pointer loaded from a table of words
            if isinstance(lo, int) and isinstance(hi, int):
                return [hi << 8 | lo]
            if addr < 0x4000:
                self.unplaced["pointer in RAM, value not known"] += 1
                return []
            w = self.word(addr, st.mpr)
            return [] if w is None else [w]
        lo, hi = st.read(addr), st.read((addr + 1) & 0xFFFF)
        if lo == ("ret", 0) and hi == ("ret", 1):
            self.inline.add(self.curkey)            # the table is the bytes after the call: the caller reads it
            return []
        x = st.x if isinstance(st.x, int) else None
        return self.words(addr, x or 0, x, st.mpr)

    def words(self, addr, first, one, mpr, plus=0):
        """The words of a table of code addresses from entry first: one entry when the index is known, else
        every entry while it is code (decodes cleanly to a return within ENTRY_LOOK instructions); the words
        $0000 and $FFFF are empty slots, passed over."""
        out = []
        for k in range(first, first + 2 * TABLE_MAX, 2):
            w = self.word((addr + k) & 0xFFFF, mpr)
            if w is None:
                break
            if w in (0x0000, 0xFFFF) and one is None:
                continue                            # an empty slot: the table goes on past it
            w = (w + plus) & 0xFFFF
            if w < 0x4000 or w >= 0xE000:
                break
            td, why = self.place(w, mpr)
            if td is None:
                if one is not None:
                    break
                continue                            # an address in a window not known here: not a table's end
            if not self.clean(td, w, mpr):
                break
            out.append(w)
            if one is not None:
                break
        return out

    def clean(self, d, pc, mpr):
        """The bytes at d decode, straight on from pc, to a return or jump with no undefined opcode or BRK, or to a
        JSR into a routine already known to jump through the words after its call (what follows is a table, not
        code)."""
        buf = self.disc.buf
        for _ in range(ENTRY_LOOK):
            if d + 7 > len(buf):
                return False
            n, name, mode, ops, target = self.disc.decode(d, pc)
            if name is None or name == "brk":
                return False
            if name in huc.ENDS:
                return True
            if name == "jsr" and self.inline:
                td, _ = self.place(target, mpr)
                if td is not None and any(k[0] == td and k[1] == target for k in self.inline):
                    return True
            d, pc = d + n, (pc + n) & 0xFFFF
        return False

    def word(self, addr, mpr):
        lo, _ = self.place(addr, mpr)
        hi, _ = self.place((addr + 1) & 0xFFFF, mpr)
        if lo is None or hi is None:
            return None
        return self.disc.buf[lo] | self.disc.buf[hi] << 8

    STORES = {"sta": "a", "stx": "x", "sty": "y", "stz": None}
    RMW = ("inc", "dec", "asl", "lsr", "rol", "ror", "tsb", "trb")

    def effect(self, name, mode, ops, st):
        """The state after one instruction. self.stored: the fixed addresses it writes (None: none).
        Assumed, and stated here: a store through a pointer ((zp),Y and the like) does not write a fixed
        address whose value is being tracked; indexed stores and block moves are counted over their range."""
        s = st.copy()
        ra = st.a                                   # A as stored: may be a table reference
        a, x, y, c = (v if isinstance(v, int) else None for v in (st.a, st.x, st.y, st.c))
        self.stored = None
        fixed = None
        if mode in ("zp", "abs"):
            fixed = (0x2000 | ops[0]) if mode == "zp" else ops[0]
        if name in self.STORES or (name in self.RMW and mode != "acc") or name.startswith(("rmb", "smb")):
            if fixed is not None:
                self.stored = {fixed}
                if name in self.STORES:
                    reg = self.STORES[name]
                    v = 0 if reg is None else {"a": ra, "x": x, "y": y}[reg]
                    s.write(fixed, v)
                    self.gstores[fixed].add(v if isinstance(v, int) else None)
                else:
                    s.write(fixed, None)
                    self.gstores[fixed].add(None)
            elif mode in ("abx", "aby", "zpx", "zpy"):
                base = (0x2000 | ops[0]) if mode in ("zpx", "zpy") else ops[0]
                self.stored = set(range(base, base + 0x100))
                s.kill(self.stored)
        elif name in ("tii", "tdd", "tin", "tia", "tai"):
            src_, dst, ln = ops
            lo = dst - ln if name == "tdd" else dst
            self.stored = set(range(lo, dst + (1 if name in ("tia", "tin") else ln) + 1)) if name not in (
                "tia", "tin") else {dst, dst + 1}
            s.kill(self.stored)
        imm = ops[0] if mode == "imm" else None
        zpv = BASE if mode == "zp" and ops[0] == (ZP_BASE & 0xFF) else None
        src = imm if mode == "imm" else zpv if zpv is not None else (st.read(fixed) if fixed is not None else None)
        if fixed in CARD_BYTES:
            src = CARD_BYTES[fixed]
        elif src is None and fixed is not None and fixed in self.gconst:
            src = self.gconst[fixed]
        if name == "lda" and mode in ("abx", "aby"):
            s.a = ("tbl", ops[0])                    # a byte of a table: what a pointer loaded from it points at
            return s
        src = src if isinstance(src, int) else None
        if name in self.STORES or name in self.RMW:
            return self.rest(name, mode, ops, st, s, a, x, y, c, src)
        return self.rest(name, mode, ops, st, s, a, x, y, c, src)

    def rest(self, name, mode, ops, st, s, a, x, y, c, src):
        if name == "lda":
            s.a = src
        elif name == "ldx":
            s.x = src
        elif name == "ldy":
            s.y = src
        elif name == "cla":
            s.a = 0
        elif name == "clx":
            s.x = 0
        elif name == "cly":
            s.y = 0
        elif name == "clc":
            s.c = 0
        elif name == "sec":
            s.c = 1
        elif name == "adc":
            s.a, s.c = add8(a, src, c)
        elif name == "sbc":
            s.a, s.c = sub8(a, src, c)
        elif name in ("inc", "dec") and mode == "acc":
            s.a = None if a is None else (a + (1 if name == "inc" else -1)) & 0xFF
        elif name in ("inx", "dex"):
            s.x = None if x is None else (x + (1 if name == "inx" else -1)) & 0xFF
        elif name in ("iny", "dey"):
            s.y = None if y is None else (y + (1 if name == "iny" else -1)) & 0xFF
        elif name == "tax":
            s.x = a
        elif name == "txa":
            s.a = x
        elif name == "tay":
            s.y = a
        elif name == "tya":
            s.a = y
        elif name == "sax":
            s.a, s.x = x, a
        elif name == "say":
            s.a, s.y = y, a
        elif name == "sxy":
            s.x, s.y = y, x
        elif name in ("pha", "phx", "phy"):
            v = {"pha": st.a, "phx": x, "phy": y}[name]       # A may be a table reference: an RTS dispatch
            s.stk = None if st.stk is None or len(st.stk) >= STACK_MAX else st.stk + (v,)
        elif name in ("pla", "plx", "ply"):
            if not st.stk:                      # unknown, or below this routine's frame
                v, s.stk = None, None
            else:
                v, s.stk = st.stk[-1], st.stk[:-1]
                if isinstance(v, tuple) and v[0] == "ret":
                    self.consumed.add(self.curkey)  # it pulls its caller's return address
            setattr(s, name[2], v)
        elif name in ("php", "plp"):
            if name == "php":
                s.stk = None if st.stk is None or len(st.stk) >= STACK_MAX else st.stk + (None,)
            else:
                s.stk, s.c = (st.stk[:-1] if st.stk else None), None
        elif name == "tam":
            mpr = list(st.mpr)
            for i in range(8):
                if ops[0] >> i & 1:
                    mpr[i] = a
            s.mpr = tuple(mpr)
        elif name == "tma":
            s.a = None
            for i in range(8):
                if ops[0] >> i & 1:
                    s.a = st.mpr[i]
        elif name in ("cmp", "cpx", "cpy"):
            reg = {"cmp": a, "cpx": x, "cpy": y}[name]
            s.c = None if reg is None or src is None else int(reg >= src)
        elif name in ("asl", "lsr", "rol", "ror") and mode == "acc":
            s.a, s.c = None, None
        elif name in ("ora", "and", "eor") and src is not None and a is not None:
            s.a = a | src if name == "ora" else a & src if name == "and" else a ^ src
        elif name in ("ora", "and", "eor", "bit", "tst", "tsb", "trb"):
            if name in ("ora", "and", "eor"):
                s.a = None
        elif name in ("asl", "lsr", "rol", "ror", "inc", "dec", "rmb0", "smb0", "sta", "stx", "sty", "stz",
                      "st0", "st1", "st2", "nop", "cli", "sei", "cld", "sed", "clv", "csl", "csh", "set"):
            if name in ("asl", "lsr", "rol", "ror"):
                s.c = None
        elif name in ("tii", "tdd", "tin", "tia", "tai"):
            pass
        elif name in ("tsx",):
            s.x = None
        elif name in ("txs",):
            s.stk = None
        elif name.startswith(("rmb", "smb", "bbr", "bbs")) or mode in ("rel", "zrel") or name in (
                "jmp", "jsr", "rts", "rti", "bra"):
            pass
        else:
            s.a = s.x = s.y = s.c = None
        return s


# ------------------------------------------------------------------------------------------- roots

def scene_map(disc, mem, key, kmpr, consts):
    """Follow one memory from the kernel's vectors (and the scene overlay's entry); consts: fixed addresses whose
    one value a first pass found (every store to them in this memory the same known value; stores through an
    index or by block move are not counted)."""
    m = Map(disc, mem, key)
    m.gconst = consts or {}
    for slot in range(0, 0x100, 3):          # the kernel's vector table: JMPs from $4000, empty slots zero
        run = 0x4000 + slot
        b = disc.buf[kernel_disc(run)]
        if b == 0x4C:
            m.call(kernel_disc(run), run, St(kmpr), "kernel vector")
        elif disc.buf[kernel_disc(run):kernel_disc(run) + 3] != b"\x00\x00\x00":
            break
    if key != "kernel":
        mpr = list(kmpr)
        mpr[4] = BASE + 4                    # SceneCallOverlay maps the overlay at $8000
        d, _ = m.place(0x8006, mpr)
        if d is not None:
            m.call(d, 0x8006, St(mpr), "scene overlay entry $8006")
    m.drain()
    return m


def follow_all(disc, mems):
    maps = {}
    boot = Map(disc, mems.boot, "boot")
    card = [0xFF, WORK, None, None, None, None, 0x1F, CARD_ROM]
    boot.call(0 * SECTOR + 0x326, 0x2B26, St(card), "card calls $2B26")
    boot.call(2 * SECTOR, 0x3000, St([0xFF, WORK, 0x80, 0x81, 0x82, 0x83, 0x84, CARD_ROM]), "boot block: run at $3000")
    boot.drain()
    maps["boot"] = boot
    kmpr = mems.kernel_mpr
    for key, mem in mems.all():
        if key == "boot":
            continue
        m = scene_map(disc, mem, key, kmpr, None)
        consts = {a: next(iter(v)) for a, v in m.gstores.items() if len(v) == 1 and None not in v}
        m = scene_map(disc, mem, key, kmpr, consts)
        maps[key] = m
    for key, mem, mpr, targets in unloaded_programs(disc, mems):
        m = Map(disc, mem, key)
        for t in targets:
            d, _ = m.place(t, mpr)
            if d is not None:
                m.call(d, t, St(mpr), "unloaded program's jump table")
        m.drain()
        maps[key] = m
    for m in maps.values():
        m.kstarts = m.kept()
        m.kentries = {(k[0], k[1]) for k in m.keep}
    return maps


# ------------------------------------------------------------------------------------------ copies

ANCHOR = 16                     # bytes of reached code that must match to start a copy


def copies(disc, maps, code):
    """Every stretch of the disc the map did not reach that holds, byte for byte, a stretch of reached code: each
    byte of it -> the reached byte it copies. A match starts on ANCHOR equal bytes, all of them reached code, and
    runs on while the bytes stay equal and the source stays reached code. {copy offset: source offset}"""
    buf = disc.buf
    idx = {}
    srt = sorted(code)
    for i, p in enumerate(srt):
        if i + ANCHOR <= len(srt) and srt[i + ANCHOR - 1] == p + ANCHOR - 1:
            k = buf[p:p + ANCHOR]
            if len(set(k)) >= 4:
                idx.setdefault(k, p)
    out = {}
    q, end = 0, len(buf) - ANCHOR
    get = idx.get
    while q < end:
        if q in code:
            q += 1
            continue
        p = get(buf[q:q + ANCHOR])
        if p is None:
            q += 1
            continue
        while p > 0 and q > 0 and (q - 1) not in code and (p - 1) in code and buf[q - 1] == buf[p - 1] and (q - 1) not in out:
            p, q = p - 1, q - 1
        while q < len(buf) and q not in code and p in code and buf[q] == buf[p]:
            out[q] = p
            p, q = p + 1, q + 1
    return out


RANCHOR = 12                    # bytes, none of them an absolute operand, that start a relocated copy
ABS_MODES = ("abs", "abx", "aby", "ind", "iax")


def relocated(disc, maps, code, exact):
    """Stretches the map did not reach that hold reached code moved to another address: the same instructions,
    every absolute operand equal or, where it points into the moved code's own window, shifted by one constant
    (the difference of the two load addresses). {copy offset: (source offset, shift)}; the copy's run address is
    the source's plus the shift. Exact copies (shift 0) are left to copies()."""
    buf = disc.buf
    start = {}                                      # every reached instruction start -> its run address and length
    for m in maps.values():
        for d, runs in m.kstarts.items():
            if d not in start:
                pc = min(runs)
                start[d] = (pc, disc.decode(d, pc)[0])
    operand = set()                                 # bytes that are absolute operands of reached instructions
    for d, (pc, n) in start.items():
        if disc.decode(d, pc)[2] in ABS_MODES:
            operand.update((d + 1, d + 2))
    idx = {}
    srt = sorted(code)
    for i, p in enumerate(srt):
        if i + RANCHOR <= len(srt) and srt[i + RANCHOR - 1] == p + RANCHOR - 1 and not any(
                p + k in operand for k in range(RANCHOR)):
            k = buf[p:p + RANCHOR]
            if len(set(k)) >= 4:
                idx.setdefault(k, p)
    owner = {}                                      # byte of reached code -> the start of its instruction
    for d, (pc, n) in start.items():
        for k in range(n):
            owner.setdefault(d + k, d)
    out = {}
    q, end = 0, len(buf) - RANCHOR
    while q < end:
        if q in code or q in exact or q in out:
            q += 1
            continue
        p = idx.get(buf[q:q + RANCHOR])
        if p is None or p not in owner:
            q += 1
            continue
        s0 = owner[p]                               # align on the source's instruction start
        qs = q - (p - s0)
        got, shift = extend(disc, start, s0, qs, code, exact)
        if got and shift:
            for qq, pp in got:
                out.setdefault(qq, (pp, shift))
            q = max(qq for qq, _ in got) + 1
        else:
            q += 1
    return out


def extend(disc, start, s0, q0, code, exact):
    """From source instruction s0 and copy offset q0, forward while each instruction matches: the bytes paired
    [(copy, source)] and the one shift every moved operand agrees on (None: none moved)."""
    buf = disc.buf
    win = start[s0][0] >> 13
    shift, got, p, q = None, [], s0, q0
    while p in start and q + 3 < len(buf) and q not in code and q not in exact:
        pc, n = start[p]
        if buf[q] != buf[p]:
            break
        mode = disc.decode(p, pc)[2]
        ok = True
        if mode in ABS_MODES:
            wp = buf[p + 1] | buf[p + 2] << 8
            wq = buf[q + 1] | buf[q + 2] << 8
            if wp != wq:
                if wp >> 13 != win:                 # an operand outside the code's own window must not move
                    ok = False
                elif shift is None:
                    shift = (wq - wp) & 0xFFFF
                elif (wq - wp) & 0xFFFF != shift:
                    ok = False
        elif buf[q + 1:q + n] != buf[p + 1:p + n]:
            ok = False
        if not ok:
            break
        got += [(q + k, p + k) for k in range(n)]
        p, q = p + n, q + n
    if shift is not None and (shift & 0x1FFF) == 0 and len(got) >= RANCHOR:
        return got, shift                           # a move of whole banks: the copy runs in another window
    if shift is not None and len(got) >= RANCHOR:
        return got, shift
    return [], None


# ------------------------------------------------------------------------- code in regions nothing loads

PSEUDO = 0xE0                   # bank numbers for unloaded programs: above the card's RAM, never mapped by the game


def jump_table(buf, d, limit):
    """The window a jump table at disc offset d runs in: JMP abs at 0, 3, 6 ... (JMP $0000, an empty slot, allowed)
    with every target in one window and within limit bytes of the table; None when there is none."""
    targets = []
    for off in range(0, 48, 3):
        if buf[d + off] != 0x4C:
            break
        t = buf[d + off + 1] | buf[d + off + 2] << 8
        if t:
            targets.append(t)
    if len(targets) < 2:
        return None, []
    w = targets[0] >> 13
    if not 2 <= w <= 6 or any(t >> 13 not in range(w, 7) or (t - (w << 13)) >= limit for t in targets):
        return None, []
    return w, targets


def unloaded_programs(disc, mems):
    """Programs on the disc that no read loads, each from a sector that opens with its own jump table: a memory of
    the kernel with the program mapped from that table's window on, as far as the run of unloaded sectors goes.
    [(name, Mem, MPRs, [entry runs])]"""
    buf = disc.buf
    loaded = set()
    for _, mem in mems.all():
        for a in mem.b.values():
            loaded.update(x // SECTOR for x in a if x >= 0)
    nsec = len(buf) // SECTOR
    out = []
    taken = set()
    for sec in range(nsec):
        if sec in loaded or sec in taken:
            continue
        run = 0
        while sec + run < nsec and sec + run not in loaded and run < 12:
            run += 1
        w, targets = jump_table(buf, sec * SECTOR, run * SECTOR)
        if w is None:
            continue
        mem = Mem(mems.kernel)
        mpr = list(mems.kernel_mpr)
        nb = min(7 - w, (run * SECTOR + BANK - 1) // BANK)
        for k in range(nb):
            mpr[w + k] = PSEUDO + len(out) * 4 + k
        mem.load(sec, run, w << 13, mpr)
        taken.update(range(sec, sec + run))
        out.append((f"unloaded{sec:04X}", mem, tuple(mpr), targets))
    return out


# ------------------------------------------------------------------------------------------- filed

TAG = re.compile(r"@routine\s+([0-9A-F]{4}):([0-9A-F]{3})\s+(\w+)")
RUN = re.compile(r"@run\s+([0-9A-F]{2}):([0-9A-F]{4})")


def filed():
    out = []
    for p in sorted(glob.glob(os.path.join(TOP, "asm", "rondo", "**", "*.s"), recursive=True)):
        t = open(p, encoding="ascii").read()
        for m in TAG.finditer(t):
            r = RUN.search(t, m.end())
            out.append({"disc": int(m.group(1), 16) * SECTOR + int(m.group(2), 16), "bank": int(r.group(1), 16),
                        "run": int(r.group(2), 16), "name": m.group(3), "file": os.path.relpath(p, TOP)})
    return out


def check(disc, mems, maps, cps=None):
    loaded = set()
    for key, mem in mems.all():
        for a in mem.b.values():
            loaded.update(d for d in a if d >= 0)
    starts = collections.defaultdict(set)
    entries = set()
    for m in maps.values():
        for d, runs in m.kstarts.items():
            starts[d] |= runs
        entries |= m.kentries
    res = collections.Counter()
    miss = []
    starts_ = {}
    for m in maps.values():
        for d, runs in m.kstarts.items():
            starts_.setdefault(d, set()).update(runs)
    rel = getattr(check, "rel", {})
    for f in filed():
        r = rel.get(f["disc"])
        if r is not None and r[0] in starts_:
            run = (min(starts_[r[0]]) + r[1]) & 0xFFFF
            res[("in no load: " if f["disc"] not in loaded else "") + ("a relocated copy at its run address" if
                run == f["run"] else "a relocated copy at another run address")] += 1
            continue
        src = (cps or {}).get(f["disc"])
        cp = None
        if src is not None and src in starts_:
            cp = (src, f["run"] if f["run"] in starts_[src] else next(iter(starts_[src])))
        if cp and cp[1] == f["run"]:
            res[("in no load: " if f["disc"] not in loaded else "") + "an exact copy of a reached routine, same run address"] += 1
            continue
        if cp:
            res[("in no load: " if f["disc"] not in loaded else "") + "an exact copy, at another run address"] += 1
            continue
        pre = "" if f["disc"] in loaded else "in no load: "
        if (f["disc"], f["run"]) in entries:
            res[pre + "found: an entry at its run address"] += 1
            continue
        if f["run"] in starts.get(f["disc"], ()):
            res[pre + "found: an instruction at its run address, not an entry"] += 1
            continue
        if f["disc"] not in loaded:
            res["in no load, no copy"] += 1
            continue
        if (f["disc"], f["run"]) in entries:
            res["found: an entry at its run address"] += 1
        elif f["run"] in starts.get(f["disc"], ()):
            res["found: an instruction at its run address, not an entry"] += 1
        elif f["disc"] in starts:
            res["found as code at another run address"] += 1
            miss.append((f, sorted(starts[f["disc"]])))
        else:
            res["not found as code"] += 1
            miss.append((f, []))
    return res, miss


# ---------------------------------------------------------------------------------------- progress

ORIGINAL = re.compile(r"\b(call_orig|call_orig_framed|call_original_at|hand_over|leave_original|untraced)\(([^;]*)\)\s*;")


def runs_game_code(body):
    """The C calls original game code: a call_orig and the like whose target is not a System Card entry (a
    CARD_ name or an address from $E000), which the game reaches through the card's interface by design."""
    for m in ORIGINAL.finditer(body):
        target = m.group(2).split(",")[-1].strip()
        if target.startswith("CARD_"):
            continue
        if re.fullmatch(r"0x[0-9A-Fa-f]+", target) and int(target, 16) >= 0xE000:
            continue
        return True
    return False


def areas(mems):
    """Each disc sector's area, by where the game loads it: {sector: name}, in the order the areas are listed."""
    names = {}
    order = []

    def mark(sector, count, name):
        if name not in order:
            order.append(name)
        for k in range(sector, sector + count):
            names.setdefault(k, name)

    mark(0, 3, "Boot")
    for op in mems.boot_ops:
        if op[0] == "read":
            mark(op[1], op[2], "Kernel")
    mark(0x23, 20, "Old-card program")
    by = {sc["n"]: sc for sc in mems.scenes}

    def mark_scene(n, name):
        for op in by.get(n, {}).get("ops", ()):
            if op[0] == "read":
                mark(op[1], op[2], name)

    mark_scene(0x00, "Richter")
    mark_scene(0x01, "Maria")
    mark_scene(0x04, "Title and system")
    mark_scene(0x06, "Title and system")
    for name, scenes in mems.layered:
        if name.startswith(("stage", "secret")):
            k = int(name[-4:-2], 16)
            label = f"Stage {k:02X}" if name.startswith("stage") else f"Secret stage {k:02X}"
            for n in scenes[3:]:
                mark_scene(n, label)
    for name, scenes in mems.layered:
        if name.startswith("seq") and not name.endswith(("-R", "-M")):
            for n in scenes[2:]:
                mark_scene(n, "Scenes " + name[3:])
    for sc in mems.scenes:
        mark_scene(sc["n"], f"Scene {sc['n']:02X}")
    order.append("Not loaded")
    return names, order


RAW_OPERAND = re.compile(r"(?<![#\w])(?:[az]:)?\$[0-9A-Fa-f]{2,4}")
INSTRUCTION = re.compile(r"^\s+([a-z]{2,4}\d?)\b(.*)$")


def placeholders():
    """Symbols named by nothing but their own address (3 or more hex digits): OBJ_27A8, BiosE05D. Not names yet."""
    out = set()
    path = os.path.join(TOP, "config", "rondo", "symbols.csv")
    for row in csv.reader(l for l in open(path, encoding="ascii") if not l.startswith("#")):
        if len(row) > 2 and row[0] != "addr":
            a = row[0].strip().lstrip("$").upper()
            if len(a) >= 3 and row[2].upper().endswith(a):
                out.add(row[2])
    return out


def unnamed():
    """Each filed routine's disc offset -> the operands of its assembly that are not names: a raw address (an
    operand that is a number and not an immediate) or a placeholder symbol. [] when its assembly is all names."""
    ph = placeholders()
    word = re.compile(r"[A-Za-z_]\w*")
    out = {}
    for path in glob.glob(os.path.join(TOP, "asm", "rondo", "**", "*.s"), recursive=True):
        t = open(path, encoding="ascii").read()
        ms = list(TAG.finditer(t))
        for i, m in enumerate(ms):
            body = t[m.end(): ms[i + 1].start() if i + 1 < len(ms) else len(t)]
            bad = []
            for line in body.split("\n"):
                code_ = line.split(";")[0]
                im = INSTRUCTION.match(code_)
                if not im:
                    continue
                op = im.group(2)
                if im.group(1) in ("tii", "tdd", "tin", "tia", "tai"):
                    op = ",".join(op.split(",")[:2])    # the third operand is a length, not an address
                bad += RAW_OPERAND.findall(op)
                bad += [w for w in word.findall(op) if w in ph]
            out[int(m.group(1), 16) * SECTOR + int(m.group(2), 16)] = bad
    return out


def progress(disc, mems, maps, cps):
    """M1 and M2 by area, every copy counted once: {area: (code bytes, M1 bytes, M2 bytes)} and the order.
    Code: what the map reached, and every filed routine that is not a copy of it; a copy is done when its
    source is, and a finished routine filed on a copy counts on its source. M1: bytes in routines whose C is finished
    (it calls no original code); M2: those whose assembly is filed too, written in names (no raw address
    and no placeholder named by its address as an operand), and whose C file compiles (ccheck.py). A stretch the disc holds more than once
    counts once: routines with the same bytes are one, counted in the area of the first's place on the disc, done
    where any copy of it is done."""
    buf = disc.buf
    import asmbuild
    pieces = {p["disc"]: p["end"] for p in asmbuild.filed_pieces(buf)}
    import ccheck
    import tempfile
    from concurrent.futures import ThreadPoolExecutor
    paths = glob.glob(os.path.join(TOP, "src", "rondo", "**", "*.c"), recursive=True)
    with tempfile.TemporaryDirectory() as out, ThreadPoolExecutor(os.cpu_count() or 4) as pool:
        compiles = {p: rc == 0 for p, rc, _ in pool.map(lambda p: ccheck.compile_one(p, out), paths)}
    finished, built = {}, {}
    for path in paths:
        t = open(path, encoding="utf-8").read()
        ms = list(TAG.finditer(t))
        for i, m in enumerate(ms):
            body = t[m.end(): ms[i + 1].start() if i + 1 < len(ms) else len(t)]
            d = int(m.group(1), 16) * SECTOR + int(m.group(2), 16)
            finished[d] = not runs_game_code(body)
            built[d] = compiles[path]
    names_ = unnamed()
    code = set()
    for m in maps.values():
        for d, runs in m.kstarts.items():
            code.update(range(d, d + disc.decode(d, next(iter(runs)))[0]))
    done1, done2 = set(), set()
    for d, n in pieces.items():
        for q in range(d, d + n):
            src = cps.get(q)
            if src is None:
                code.add(q)                     # a filed routine is code, unless it is a copy of code counted already
            if finished.get(d):
                for x in (q,) if src is None else (src,):
                    done1.add(x)
                    if built.get(d) and not names_.get(d):   # assembly filed (pieces) and all names; C compiles
                        done2.add(x)
    # a routine the disc holds more than once counts once: every routine body (the map's, from each kept entry in
    # one piece, and every filed routine's) grouped by its exact bytes; each byte of a later member belongs to the
    # first member, unless the first's own bodies hold it; a byte is done when any copy of it is
    bodies = dict(pieces)
    for m in maps.values():
        for key in m.keep:
            ln = {dd: n for (dd, _), n in m.bodies.get(key, {}).items()}
            e = key[0]
            while e in ln:
                e += ln[e]
            if e > key[0]:
                bodies[key[0]] = max(bodies.get(key[0], 0), e - key[0])
    groups = collections.defaultdict(list)
    for d, n in bodies.items():
        groups[buf[d:d + n]].append(d)
    own = set()
    for k, ds in groups.items():
        own.update(range(min(ds), min(ds) + len(k)))
    canon = {}
    for k, ds in groups.items():
        f = min(ds)
        for d in ds:
            if d != f:
                for j in range(len(k)):
                    if d + j not in own:
                        canon.setdefault(d + j, f + j)
    srt = sorted(code)

    def root(x):
        while x in canon:
            x = canon[x]
        return x
    for done in (done1, done2):
        for x in [x for x in done if x in canon]:
            done.add(root(x))
    names, order = areas(mems)
    out = {n: [0, 0, 0] for n in order}
    for x in srt:
        if x in canon:
            continue
        row = out[names.get(x // SECTOR, "Not loaded")]
        row[0] += 1
        row[1] += x in done1
        row[2] += x in done2
    return {n: tuple(v) for n, v in out.items() if v[0]}, [n for n in order if out[n][0]]


def svg(rows, order, total):
    """The progress picture: one bar for the whole game, then one per area."""
    W, L, H, G = 760, 190, 16, 6
    y0 = 70
    h = y0 + (len(order) + 1) * (H + G) + 40
    ink, sub, track, c2, c1 = "#24292f", "#57606a", "#e6e9ed", "#1f6feb", "#8cb4f5"
    o = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{h}" viewBox="0 0 {W} {h}" '
         f'font-family="-apple-system,Segoe UI,Helvetica,Arial,sans-serif">',
         f'<rect width="{W}" height="{h}" fill="#ffffff"/>']
    t = total
    pct = lambda a, b: 100.0 * a / b if b else 0.0
    o.append(f'<text x="16" y="28" font-size="17" font-weight="600" fill="{ink}">Rondo of Blood: M1 {pct(t[1], t[0]):.1f}%'
             f'  M2 {pct(t[2], t[0]):.1f}%</text>')
    o.append(f'<text x="16" y="48" font-size="12" fill="{sub}">{t[0]:,} B of distinct code found so far, every copy '
             f'counted once. Dark: M2. Light: M1 only. Grey: to do.</text>')

    def bar(y, name, r, bold=False):
        x, w = L, W - L - 90
        o.append(f'<text x="{L - 8}" y="{y + 12}" font-size="12" text-anchor="end" fill="{ink}"'
                 f'{" font-weight=\"600\"" if bold else ""}>{name}</text>')
        o.append(f'<rect x="{x}" y="{y}" width="{w}" height="{H}" rx="3" fill="{track}"/>')
        w1, w2 = w * r[1] / r[0], w * r[2] / r[0]
        if w1:
            o.append(f'<rect x="{x}" y="{y}" width="{w1:.1f}" height="{H}" rx="3" fill="{c1}"/>')
        if w2:
            o.append(f'<rect x="{x}" y="{y}" width="{w2:.1f}" height="{H}" rx="3" fill="{c2}"/>')
        o.append(f'<text x="{x + w + 8}" y="{y + 12}" font-size="11" fill="{sub}">{pct(r[1], r[0]):.0f}% of '
                 f'{r[0] / 1024:,.1f} KB</text>')

    bar(y0, "Whole game", t, True)
    for i, n in enumerate(order):
        bar(y0 + (i + 1) * (H + G) + 6, n, rows[n])
    o.append("</svg>")
    return "\n".join(o) + "\n"


def todo(disc, mems, maps):
    """Every routine the map found, once per distinct body: [(area, place, run, bytes, status, name)], status
    "done" (filed, C finished), "C unfinished" (filed, its C calls original game code) or "to do" (not filed).
    A routine's bytes are its body's instructions from its entry on, in one piece; a body the disc holds again
    elsewhere is listed once, at its first place."""
    buf = disc.buf
    import asmbuild
    pieces = sorted((p["disc"], p["disc"] + p["end"], p["name"]) for p in asmbuild.filed_pieces(buf))
    starts_ = [a for a, _, _ in pieces]
    import bisect
    finished = {}
    for path in glob.glob(os.path.join(TOP, "src", "rondo", "**", "*.c"), recursive=True):
        t = open(path, encoding="utf-8").read()
        ms = list(TAG.finditer(t))
        for i, m in enumerate(ms):
            body = t[m.end(): ms[i + 1].start() if i + 1 < len(ms) else len(t)]
            finished[int(m.group(1), 16) * SECTOR + int(m.group(2), 16)] = not runs_game_code(body)
    names, _ = areas(mems)
    seen, rows = {}, []
    for m in maps.values():
        for key in m.keep:
            d, pc = key[0], key[1]
            if d in seen:
                continue
            body = m.bodies.get(key, {})
            ln = {dd: n for (dd, _), n in body.items()}
            end = d
            while end in ln:
                end += ln[end]
            if end == d:
                continue
            k = buf[d:end]
            if k in seen.values():
                seen[d] = k
                continue
            seen[d] = k
            i = bisect.bisect_right(starts_, d) - 1
            status, name = "to do", ""
            if i >= 0 and pieces[i][0] <= d < pieces[i][1]:
                a, _, name = pieces[i]
                status = "done" if finished.get(a) else "C unfinished" if a in finished else "assembly only"
            rows.append((names.get(d // SECTOR, "Not loaded"), d, pc, end - d, status, name))
    rows.sort(key=lambda r: r[1])
    return rows


README = os.path.join(TOP, "README.md")
PICTURE = os.path.join(TOP, "docs", "rondo", "progress.svg")


def write_progress(rows, order):
    total = tuple(sum(rows[n][i] for n in order) for i in range(3))
    with open(PICTURE, "w", encoding="utf-8", newline="\n") as f:
        f.write(svg(rows, order, total))
    pct = lambda a: 100.0 * a / total[0]
    block = ["<!-- progress:start -->", "![Progress by disc area](docs/rondo/progress.svg)", "",
             f"**M1 {pct(total[1]):.1f}%, M2 {pct(total[2]):.1f}%** of {total[0]:,} B of distinct code found so far by "
             f"the static code map (every copy counted once). The map is not finished, so the total will grow and the "
             f"percentages fall. Generated by `tools/rondo/codemap.py --progress`.", "<!-- progress:end -->"]
    t = open(README, encoding="utf-8").read()
    a, b = t.find("<!-- progress:start -->"), t.find("<!-- progress:end -->")
    if a < 0 or b < 0:
        raise SystemExit("codemap: README has no progress markers")
    t = t[:a] + "\n".join(block) + t[b + len("<!-- progress:end -->"):]
    with open(README, "w", encoding="utf-8", newline="\n") as f:
        f.write(t)
    return total


def main():
    disc = Disc()
    mems = Memories(disc)
    print(f"boot ops {len(mems.boot_ops)}; kernel MPRs {[hx(m, 2) for m in mems.kernel_mpr]}; {len(mems.scenes)} scenes")
    maps = follow_all(disc, mems)
    code = set()
    for m in maps.values():
        for d in m.kstarts:
            n = disc.decode(d, next(iter(m.kstarts[d])))[0]
            code.update(range(d, d + n))
    print(f"code bytes found: {len(code):,}")
    bad = collections.Counter(w for m in maps.values() for _, _, w in m.bad)
    kept_bad = collections.Counter(w for m in maps.values() for e, _, w in m.bad if any(
        (e[0], e[1]) == (k[0], k[1]) for k in m.keep if k in m.badkeys))
    print("met what code cannot be, on kept paths:", dict(kept_bad))
    unp = collections.Counter()
    for m in maps.values():
        unp.update(m.unplaced)
    print("met what code cannot be:", dict(bad))
    print("targets not placed:", dict(unp))
    cps = copies(disc, maps, code)
    print(f"bytes elsewhere on the disc that copy reached code: {len(cps):,}")
    rel = relocated(disc, maps, code, cps)
    print(f"bytes that copy reached code moved to another address: {len(rel):,}")
    check.rel = rel
    if "--progress" in sys.argv[1:]:
        rows, order = progress(disc, mems, maps, cps)
        total = write_progress(rows, order)
        for n in order:
            r = rows[n]
            print(f"  {n:24} {r[0]:8,} B  M1 {100 * r[1] / r[0]:5.1f}%  M2 {100 * r[2] / r[0]:5.1f}%")
        print(f"  {'whole game':24} {total[0]:8,} B  M1 {100 * total[1] / total[0]:5.1f}%  M2 {100 * total[2] / total[0]:5.1f}%")
    if "--todo" in sys.argv[1:]:
        rows = todo(disc, mems, maps)
        os.makedirs(OUT, exist_ok=True)
        path = os.path.join(OUT, "todo.csv")
        with open(path, "w", newline="") as fh:
            w = csv.writer(fh)
            w.writerow(["area", "place", "run", "bytes", "status", "name"])
            for a, d, pc, n, st_, nm in rows:
                w.writerow([a, f"{d // SECTOR:04X}:{d % SECTOR:03X}", f"{pc:04X}", n, st_, nm])
        c = collections.Counter(r[4] for r in rows)
        b = collections.Counter()
        for r in rows:
            b[r[4]] += r[3]
        print(f"to-do list: {path}")
        for k in ("done", "C unfinished", "assembly only", "to do"):
            print(f"  {k:14} {c[k]:6,} routines {b[k]:9,} B")
    if "--check" in sys.argv[1:]:
        res, miss = check(disc, mems, maps, cps)
        for k, v in res.items():
            print(f"  {v:5}  {k}")
        os.makedirs(OUT, exist_ok=True)
        with open(os.path.join(OUT, "check_miss.csv"), "w", newline="") as fh:
            w = csv.writer(fh)
            w.writerow(["place", "name", "run", "file", "found_at"])
            for f, runs in miss:
                w.writerow([f"{f['disc'] // SECTOR:04X}:{f['disc'] % SECTOR:03X}", f["name"],
                            f"{f['bank']:02X}:{f['run']:04X}", f["file"], " ".join(f"{r:04X}" for r in runs)])
    return 0


if __name__ == "__main__":
    import threading
    sys.setrecursionlimit(200000)
    threading.stack_size(1 << 30)           # the follower recurses once per call: deep call chains need the room
    rc = []
    t = threading.Thread(target=lambda: rc.append(main()))
    t.start()
    t.join()
    sys.exit(rc[0] if rc else 1)
