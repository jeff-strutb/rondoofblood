#!/usr/bin/env python3
"""census.py -- the game's code that no script has run, found by following the code the traces ran.

    python3 tools/rondo/census.py                 write build/rondo/census.csv, censusmap.bin
    python3 tools/rondo/census.py --holdout       measure recall and false claims by holding scripts out

How this game's code is laid out. There is no ROM: the program is on the disc and is read into
RAM banks by scenes (LoadSceneScript, disc_load.c): SceneRecords ($74D1 in the resident kernel,
five bytes each: the scene's disc address and the address of its list of ops) and
SceneReadDestinations ($70F8). The ops map MPR4-6 (op 0), read sectors at an offset from the
scene's address to a destination (ops 1-$7F), go to another list ($C0), call the scene overlay
($80-$83), or end. Replaying every scene's ops tells which disc byte each physical bank holds
once the scene is loaded; a later read into a bank replaces the earlier. The boot's own three
reads (HANDBOOK, Code) put the kernel and its overlay banks in the same way; they are replayed
here as one more context, "boot". Code runs with a bank mapped in some window ($4000, $6000,
$8000, $A000, $C000), not always the one it was read through; a bank's windows are learned from
the traces (the run address of a traced routine whose bytes sit in it), and a bank no trace has
run takes the window its own vector table points into.

Two censuses, both from the code the scripts ran (their traces) and no other input:

  strict   follows calls, jumps and branches from every place the traces entered code, JMP
           (abs,X) and JMP (abs) tables read to their full length (repeated entries kept, "no
           entry" words skipped), and handler-pointer arrays anchored on a pointer to code that ran.
           A block that decodes into an undefined opcode, into BRK (which two bytes of zero
           decode as) or into an opcode no script ever ran, or jumps into I/O, is data and is dropped.
  relaxed  strict, plus every pointer array whether anchored or not, every lone plausible
           pointer, every untouched stretch that decodes cleanly to a return, and blocks
           with the opcodes strict refuses. Every byte it finds that strict does not is
           labelled with the rule that found it, and (in --holdout) with what the scripts that
           ran it say it is.

Recall and false claims are measured by --holdout: run both from a subset of the scripts'
traces and count how much of the code the held-out scripts alone ran is found, and how much
that was found is bytes the scripts read as data and never ran. A census is put in the
denominator only if that measurement says it can be (PLAN.md section 1; progress.py).
"""
import array
import collections
import csv
import glob
import math
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import asmfile  # noqa: E402
import huc  # noqa: E402

TRACE = os.path.join(TOP, "build", "rondo", "trace")
SCRIPTS = os.path.join(HERE, "scripts")
OUT = os.path.join(TOP, "build", "rondo", "census.csv")
MAP = os.path.join(TOP, "build", "rondo", "censusmap.bin")
SECTOR = 2048
BANK = 0x2000
USER_BYTES = 10149 * SECTOR
TRACK_START = 0xF32
SCENE_RECORDS, SCENE_DESTINATIONS = 0x74D1, 0x70F8
KERNEL_SECTOR = 0x43
OP_BANKS, OP_GOTO, OP_CALLS = 0, 0xC0, (0x80, 0x81, 0x82, 0x83)
CALL_KINDS = (0x20, 0x44, 0x4C, 0x6C, 0x7C, 0x100)
SUNK = 4 | 8 | 16 | 32            # discmap2: sent to the VDC, the VCE, the PSG or the ADPCM port: data, for certain
WINDOWS = (2, 3, 4, 5, 6)           # 8 KB windows at $4000 .. $DFFF
TABLE_MAX = 256                     # entries: X is eight bits
BOOT_READS = ((0x43, 10, 0x4000), (0x4D, 12, 0x8000), (0x59, 8, 0x8000))  # boot block's load table
BUILT_CODE = (0x22E7, 0x2286)       # code the game and the card build in work RAM
FORBID_MODES = ("blk",)             # TII, TDD and the like are fine; kept here to name the choice
EXTENT_MAX = 4096


def kernel_disc(run):
    """The resident kernel: run $4000-$7FFF is sector $43 on (MPR2-3 = $69-$6A)."""
    return KERNEL_SECTOR * SECTOR + run - 0x4000 if 0x4000 <= run < 0x8000 else None


# --------------------------------------------------------------------------------- the disc

class Disc:
    def __init__(self):
        self.buf = open(asmfile.IMAGE, "rb").read()
        self.sectors = len(self.buf) // SECTOR
        self._dec = {}

    def kb(self, run):
        return self.buf[kernel_disc(run)]

    def kw(self, run):
        return self.kb(run) | self.kb(run + 1) << 8

    def decode(self, d):
        """(length, name, mode, raw operand) at disc offset d; cached. name None: undefined opcode."""
        v = self._dec.get(d)
        if v is None:
            n, name, mode, ops, target = huc.decode(self.buf, d, 0)
            if name is None:
                v = (n, None, None, 0)
            elif mode in ("rel", "zrel"):
                b = self.buf[d + (1 if mode == "rel" else 2)]
                v = (n, name, mode, b - 256 if b & 0x80 else b)
            elif mode in ("abs", "abx", "aby", "ind", "iax"):
                v = (n, name, mode, ops[0])
            elif mode in ("zp", "zpx", "zpy", "imm", "izp", "izx", "izy"):
                v = (n, name, mode, ops[0])
            else:
                v = (n, name, mode, 0)
            self._dec[d] = v
        return v


def scenes(disc):
    """Every scene record in order, with its ops decoded; the table ends at the first record whose
    address is off the track or whose list does not decode to an end inside the kernel."""
    out = []
    for n in range(256):
        r = SCENE_RECORDS + 5 * n
        if kernel_disc(r + 4) is None:
            break
        sector = (disc.kb(r) | disc.kb(r + 1) << 8 | disc.kb(r + 2) << 16) - TRACK_START
        head = disc.kw(r + 3)
        if not 0 <= sector < disc.sectors or kernel_disc(head) is None:
            break
        ops, p, steps, ended = [], head, 0, False
        while steps < 64 and kernel_disc(p + 3) is not None:
            op, steps = disc.kb(p), steps + 1
            if op == OP_BANKS:
                ops.append(("banks", (disc.kb(p + 1), disc.kb(p + 2), disc.kb(p + 3))))
                p += 4
            elif op < 0x80:
                ops.append(("read", sector + disc.kb(p + 1), disc.kb(p + 2), disc.kw(SCENE_DESTINATIONS + 2 * op)))
                p += 3
            elif op == OP_GOTO:
                p = disc.kb(p + 1) | disc.kb(p + 2) << 8
            elif op in OP_CALLS:
                ops.append(("call", op))
                p += 1
            else:
                ended = True
                break
        if not ended:
            break
        out.append({"n": n, "sector": sector, "list": head, "ops": ops})
    return out


def replay(ops):
    """Bank (relative to the card's RAM base) -> the disc offset each of its bytes holds after the ops."""
    mpr, banks = {4: None, 5: None, 6: None}, {}
    for op in ops:
        if op[0] == "banks":
            mpr = dict(zip((4, 5, 6), op[1]))
        elif op[0] == "read":
            _, sector, count, dest = op
            for i in range(count * SECTOR):
                logical = dest + i
                bank = mpr.get(logical >> 13)
                if bank is None:
                    continue
                if bank not in banks:
                    banks[bank] = array.array("l", [-1]) * BANK
                banks[bank][logical & (BANK - 1)] = sector * SECTOR + i
    return banks


def boot_banks():
    """The boot's reads: each 8 KB of them is a bank of its own (keys 1000 and up)."""
    banks, k = {}, 1000
    for sector, count, _ in BOOT_READS:
        for part in range(0, count * SECTOR, BANK):
            a = array.array("l", [-1]) * BANK
            for i in range(part, min(part + BANK, count * SECTOR)):
                a[i - part] = sector * SECTOR + i
            banks[k] = a
            k += 1
    return banks


# ------------------------------------------------------------------------------- the traces

class Traces:
    """What a set of scripts ran: code and data flags per disc byte, and where control went."""

    def __init__(self, names):
        self.names = list(names)
        acc = 0
        for n in self.names:
            acc |= int.from_bytes(open(os.path.join(TRACE, n, "discmap.bin"), "rb").read(), "little")
        self.map = acc.to_bytes(USER_BYTES, "little")
        self.map2 = None                # what each disc byte is: trace.c's discmap2 flags (None: not traced yet)
        self.runs = None                # (disc offset, logical address, bank) of every executed opcode
        if all(os.path.exists(os.path.join(TRACE, n, "discmap2.bin")) for n in self.names):
            acc, self.runs = 0, set()
            for n in self.names:
                acc |= int.from_bytes(open(os.path.join(TRACE, n, "discmap2.bin"), "rb").read(), "little")
                mine = {}
                for line in open(os.path.join(TRACE, n, "mprcfg.csv")).read().splitlines()[1:]:
                    f = line.split(",")
                    mine[int(f[0])] = tuple(int(x, 16) for x in f[1:])
                d = open(os.path.join(TRACE, n, "runmap.bin"), "rb").read()
                self.runs.update((a, b, c, mine[g]) for a, b, c, k, g, _ in struct.iter_unpack("<IHBBHH", d) if k == 1)
            self.map2 = acc.to_bytes(USER_BYTES, "little")
        self.edges = set()
        for n in self.names:
            for r in csv.DictReader(open(os.path.join(TRACE, n, "edges.csv"), newline="")):
                if r["to_space"] == "D":
                    self.edges.add((int(r["kind"], 16), r["from_space"], int(r["from"], 16), int(r["to"], 16)))
        self.entries = {to for kind, _, _, to in self.edges}
        self.code = {i for i, v in enumerate(self.map) if v & 3}

    def ran(self, d):
        return self.map[d] & 3 != 0


def all_scripts():
    return sorted(os.path.splitext(os.path.basename(p))[0] for p in glob.glob(os.path.join(SCRIPTS, "*.txt"))
                  if not os.path.basename(p).startswith("_"))


# ------------------------------------------------------------------------------ the contexts

class Ctx:
    """One memory: which disc byte each bank holds, and which windows each bank has run in."""

    def __init__(self, key, list_key, banks):
        self.key, self.list_key, self.banks = key, list_key, banks
        self.wins = {}
        self.cfgs = {}                  # (bank, window) -> the banks in windows 2-6 when code from that bank ran there
        self.ambiguous = 0

    def place(self, run):
        """The disc offset the byte at this run address holds in this memory, or None."""
        k = kernel_disc(run)
        if k is not None:
            return k
        w = run >> 13
        hit = {self.banks[b][run & (BANK - 1)] for b, ws in self.wins.items() if w in ws} - {-1}
        if len(hit) == 1:
            return hit.pop()
        if hit:
            self.ambiguous += 1
        return None

    def resolve(self, run, bank=None, window=None):
        """Every (bank, disc offset) the byte at this run address can be in this memory. Code that runs in a window
        and calls into the same window calls its own bank, so that call has one answer; a call into another
        window has one answer per bank that has run there."""
        k = kernel_disc(run)
        if k is not None:
            return [(None, k)]
        w, off = run >> 13, run & (BANK - 1)
        if bank is not None and window == w and bank in self.banks:
            d = self.banks[bank][off]
            return [(bank, d)] if d >= 0 else []
        if bank is not None and window is not None and 2 <= w <= 6 and self.cfgs.get((bank, window)):
            seen = {c[w - 2] for c in self.cfgs[(bank, window)]}
            got = [(b, self.banks[b][off]) for b in seen if b in self.banks and self.banks[b][off] >= 0]
            if got:
                return got
        out = [(b, self.banks[b][off]) for b, ws in self.wins.items() if w in ws and self.banks[b][off] >= 0]
        if bank is not None and any(b == bank for b, _ in out) and w in self.wins.get(bank, ()):
            return [(b, d) for b, d in out if b == bank]
        return out

    def at(self):
        """Disc offset -> [(bank, offset in bank)]."""
        out = collections.defaultdict(list)
        for b, a in self.banks.items():
            for off, d in enumerate(a):
                if d >= 0:
                    out[d].append((b, off))
        return out


def vector_window(a, disc):
    """The window a bank's own vector table points into: JMP abs at 0, 3, ... with every target in
    one window and inside the bank's loaded bytes; None when it has no such table."""
    targets = []
    for off in range(0, 48, 3):
        d = a[off]
        if d < 0 or disc.buf[d] != 0x4C:
            break
        targets.append(disc.buf[d + 1] | disc.buf[d + 2] << 8)
    if len(targets) < 2 or len({t >> 13 for t in targets}) != 1:
        return None
    w = targets[0] >> 13
    return w if 4 <= w <= 6 and all(a[t & (BANK - 1)] >= 0 for t in targets) else None


def contexts(disc, tr):
    """Every scene's memory (and the boot's) with the windows the traces give each bank."""
    ctxs = [Ctx("boot", "boot", boot_banks())]
    for s in scenes(disc):
        ctxs.append(Ctx(f"scene{s['n']:02X}", s["list"], replay(s["ops"])))
    # (disc offset, run address) wherever a traced JSR or JMP abs gave both
    pairs = set()
    if tr.runs is not None:             # exact: where every executed opcode ran
        pairs = {(d, logical) for d, logical, bank, cfg in tr.runs}
    else:
        for kind, space, frm, to in tr.edges:
            if kind in (0x20, 0x4C) and space == "D":
                n, name, mode, w = disc.decode(frm)
                if name in ("jsr", "jmp") and mode == "abs":
                    pairs.add((to, w))
    by_list = collections.defaultdict(list)
    for c in ctxs:
        by_list[c.list_key].append(c)
    for lk, group in by_list.items():
        seen = collections.defaultdict(set)
        for c in group:
            at = c.at()
            for d, run in pairs:
                for b, off in at.get(d, ()):
                    if off == run & (BANK - 1) and run >> 13 in WINDOWS:
                        seen[b].add(run >> 13)
        cfgs = collections.defaultdict(set)
        if tr.runs is not None:         # the mapper as the traces saw it when code from a bank ran in a window
            for c in group:
                at = c.at()
                for d, logical, phys, cfg in tr.runs:
                    for b, off in at.get(d, ()):
                        if off == logical & (BANK - 1) and b < 1000:
                            cfgs[(b, logical >> 13)].add(tuple(x - 0x68 for x in cfg))
        for c in group:
            c.cfgs = cfgs
            for b, a in c.banks.items():
                ws = seen.get(b) or ({vector_window(a, disc)} - {None})
                if ws:
                    c.wins[b] = set(ws)
    return ctxs


# ---------------------------------------------------------------------------- the follower

def tokens(disc, d):
    """What one instruction says about being code: its opcode, and the high byte of an absolute operand."""
    n, name, mode, w = disc.decode(d)
    if name is None:
        return n, None
    t = [("op", disc.buf[d])]
    if mode in ("abs", "abx", "aby", "ind", "iax"):
        t.append(("hi", w >> 8))
    return n, t


class Model:
    """How much more likely a run of instructions is code than data: the opcodes and absolute-operand high
    bytes of the code the scripts ran against those of the sectors they read as data and never ran."""

    def __init__(self, disc, tr):
        buf = disc.buf
        code, data = collections.Counter(), collections.Counter()
        for d in tr.code:
            if tr.map[d] & 1:
                n, t = tokens(disc, d)
                if t:
                    code.update(t)
        sec_code, sec_data = collections.Counter(), collections.Counter()
        for i, v in enumerate(tr.map):
            if v & 3:
                sec_code[i // SECTOR] += 1
            elif (tr.map2[i] & SUNK if tr.map2 is not None else v == 4):
                sec_data[i // SECTOR] += 1
        self.pure = sorted(s for s, c in sec_data.items() if c >= 800 and not sec_code[s])
        for s in self.pure:
            d = s * SECTOR
            while d < (s + 1) * SECTOR - 4:
                n, t = tokens(disc, d)
                if t:
                    data.update(t)
                d += n
        tc, td = sum(code.values()), sum(data.values())
        self.llr = {k: math.log((code[k] + 0.5) / tc) - math.log((data[k] + 0.5) / td)
                    for k in set(code) | set(data)}
        self.default = math.log(0.5 / tc) - math.log(0.5 / td)
        self.ran_ops = {buf[d] for d in tr.code if tr.map[d] & 1}
        self.never = {op for op in huc.OPS if op not in self.ran_ops}

    def score(self, toks):
        return sum(self.llr.get(t, self.default) for t in toks)


class Census:
    """One census over one trace set. mode "strict" or "relaxed"."""

    TAU = {"strict": 4.0, "relaxed": 0.0}

    def __init__(self, disc, tr, model, mode):
        self.disc, self.tr, self.model, self.mode = disc, tr, model, mode
        self.strict = mode == "strict"
        self.found = {}                 # disc offset -> the rule that found it
        self.owner = {}                 # disc offset -> the offset of the extent that claimed it
        self.why = {}                   # extent start -> why it was refused, when it was
        self.stats = collections.Counter()
        self.tails = []                 # where each claimed extent ended: (disc offset, run address, bank)
        self.starts = set()             # every instruction start claimed
        self._ext = {}

    def extent(self, d, cache=True):
        """The instructions from d to its first return or jump: ([(offset, length)], ok, score). Context
        free, so cached. ok: ends properly, with no undefined opcode, nothing the scripts never ran (strict),
        no jump into I/O, and it reads as code (the model's score at least TAU)."""
        v = self._ext.get(d)
        if v is not None:
            return v
        disc, buf, p, instrs, toks, ok = self.disc, self.disc.buf, d, [], [], False
        why = "runs past the extent limit"
        while len(instrs) < EXTENT_MAX and p + 3 <= len(buf):
            n, name, mode, w = disc.decode(p)
            if name is None:
                why = "undefined opcode"
                break
            op = buf[p]
            if self.strict and op == 0:
                why = "BRK"
                break
            if self.strict and op in self.model.never:
                why = "opcode no script ran"
                break
            if op == 0 and instrs and buf[instrs[-1][0]] == 0:       # two zero bytes: data
                why = "two BRK"
                break
            if mode == "abs" and name in ("jsr", "jmp") and not self.target_class_ok(w):
                why = "jump into I/O or work RAM"
                break
            instrs.append((p, n))
            toks += tokens(disc, p)[1]
            if name in huc.ENDS:
                ok = True
                why = None
                break
            p += n
        score = self.model.score(toks)
        if ok and score < self.TAU[self.mode]:
            ok, why = False, "reads as data (score)"
        v = (instrs, ok, score)
        if cache:
            self._ext[d] = v
        if why and cache:
            self.why[d] = why
        return v

    def target_class_ok(self, w):
        if w >= 0x4000:
            return True
        return w in BUILT_CODE if self.strict else w >= 0x2000

    def pick(self, cands):
        """Of the places a target can be, those that read as code: all of them (relaxed), or the only one (strict)."""
        good = [(b, d) for b, d in cands if self.extent(d)[1]]
        if len(good) == 1 or (good and not self.strict):
            return good
        if len(cands) > 1:
            self.stats["ambiguous"] += 1
        return []

    def follow(self, ctx, seeds, rule):
        """Decode the code reachable from seeds (disc offset, run address, bank) in one context; new bytes go in
        found under rule. Returns the tables its JMP (abs,X) and JMP (abs) instructions read: (address, bank,
        window of the instruction)."""
        disc, found = self.disc, self.found
        work = list(seeds)
        seen = set()
        tables = []
        while work:
            d, pc, bank = work.pop()
            if (d, pc) in seen:
                continue
            seen.add((d, pc))
            instrs, ok, score = self.extent(d)
            if not (ok or self.tr.ran(d)) or not instrs:
                self.stats["dropped extents"] += 1
                continue
            q = pc
            for p, n in instrs:
                _, name, mode, w = disc.decode(p)
                self.starts.add(p)
                for i in range(n):
                    if p + i not in found:
                        found[p + i] = rule
                        self.owner[p + i] = d
                if mode == "abs" and name in ("jsr", "jmp"):
                    if w >= 0x4000:
                        cands = ctx.resolve(w, bank, q >> 13)
                        got = cands if len(cands) <= 1 else self.pick(cands)
                        if got:
                            for tb, t in got:
                                work.append((t, w, tb if tb is not None else bank))
                        else:
                            self.stats["unplaced"] += 1
                elif mode in ("rel", "zrel"):
                    t = (q + n + w) & 0xFFFF
                    if t >> 13 == q >> 13:
                        work.append((p + n + w, t, bank))
                elif name == "jmp" and mode in ("ind", "iax"):
                    tables.append((w, bank, q >> 13))
                    if mode == "ind" and 0x2000 <= w < 0x2100:
                        t = self.pointer_table_of(instrs, p, w - 0x2000)
                        if t is not None:
                            tables.append((t, bank, q >> 13))
                q = (q + n) & 0xFFFF
            last = instrs[-1]
            self.tails.append((last[0] + last[1], q, bank))
        return tables

    def pointer_table_of(self, instrs, at, zp):
        """JMP (zero page pointer) after `LDA table,Y` or `,X` stored into that pointer: the table's address."""
        disc = self.disc
        k = next(i for i, (p, n) in enumerate(instrs) if p == at)
        for i in range(k - 1, max(-1, k - 12), -1):
            p, n = instrs[i]
            _, name, mode, w = disc.decode(p)
            if name == "sta" and mode == "zp" and w == zp and i:
                lp, ln = instrs[i - 1]
                _, lname, lmode, lw = disc.decode(lp)
                if lname == "lda" and lmode in ("abx", "aby"):
                    return lw
        return None

    def entries_of(self, ctx, w, bank, window):
        """[(bank, disc offset)] a word can point at when what is there reads as code."""
        if w < 0x4000:
            return []
        cands = ctx.resolve(w, bank, window)
        return cands if len(cands) == 1 and self.extent(cands[0][1])[1] else self.pick(cands) if len(cands) > 1 else []

    def table_seeds(self, ctx, table):
        """Read a table to its full length: every entry that starts code, repeats kept, the words that mean "no
        entry" ($FFFF, $0000) skipped, ending at the first word that is neither."""
        t, bank, window = table
        here = ctx.resolve(t, bank, window)
        if len(here) != 1:
            return []
        d0 = here[0][1]
        out, buf = [], self.disc.buf
        for k in range(TABLE_MAX):
            d = d0 + 2 * k
            if d + 2 > len(buf):
                break
            w = buf[d] | buf[d + 1] << 8
            if w in (0xFFFF, 0x0000):
                continue
            got = self.entries_of(ctx, w, bank, window)
            if not got:
                break
            out += [(tt, w, tb if tb is not None else bank) for tb, tt in got]
        return out

    def pointer_runs(self, ctx, anchors, min_run):
        """Arrays of words in this memory that each start code: runs of at least min_run (words that mean
        "no entry" skipped), holding at least one anchor when anchors is given."""
        seeds = []
        buf = self.disc.buf
        for b, a in ctx.banks.items():
            if b not in ctx.wins:
                continue
            for par in (0, 1):
                run, hit = [], False
                for off in range(par, BANK - 1, 2):
                    d0 = a[off]
                    w = buf[d0] | buf[d0 + 1] << 8 if d0 >= 0 and a[off + 1] == d0 + 1 else None
                    got = None
                    if w is not None:
                        got = [] if w in (0xFFFF, 0) else self.entries_of(
                            ctx, w, b, w >> 13 if w >> 13 in ctx.wins[b] else None) or None
                    if got is not None:
                        run += [(t, w, tb if tb is not None else b) for tb, t in got]
                        hit = hit or (anchors is not None and w in anchors)
                        continue
                    if len(run) >= min_run and (anchors is None or hit):
                        seeds += run
                    run, hit = [], False
                if len(run) >= min_run and (anchors is None or hit):
                    seeds += run
        return seeds


COPY = 16   # bytes that must match for a place on the disc to be a copy of code that ran


def copies(disc, tr):
    """Every place on the disc that holds, byte for byte, COPY bytes of code that ran: the disc repeats its
    code (HANDBOOK), and a copy of code is code. [(copy offset, the offset it copies)], and the opcode
    offsets among the sources, which are instruction starts."""
    buf, code, keys = disc.buf, tr.map, {}
    for p in tr.code:
        if code[p] & 1 and p + COPY <= len(buf) and all(code[p + i] & 3 for i in range(COPY)):
            k = buf[p:p + COPY]
            if len(set(k)) >= 4:                  # not a run of one filler byte
                keys.setdefault(k, p)
    out, get = [], keys.get
    for q in range(len(buf) - COPY):
        src = get(buf[q:q + COPY])
        if src is not None and q != src:
            out.append((q, src))
    return out


def run_census(disc, tr, ctxs, model, mode):
    c = Census(disc, tr, model, mode)
    strict = mode == "strict"
    for d in tr.code:
        c.found[d] = "traced"
    cps = copies(disc, tr)
    for q, src in cps:
        for i in range(COPY):
            if q + i not in c.found:
                c.found[q + i] = "copy"
    starts = tr.entries | {q for q, src in cps}
    held = {d for ctx in ctxs for a in ctx.banks.values() for d in a if d >= 0}
    # copies outside every scene's memory still run on: their own branches, with no window to place calls in
    c.follow(Ctx("free", "free", {}), [(q, 0x8000 | q & 0x1FFF, None) for q, src in cps if q not in held], "copy")
    for ctx in ctxs:
        at = ctx.at()
        anchors, seeds = set(), []
        for d in starts:
            for b, off in at.get(d, ()):
                for w in ctx.wins.get(b, ()):
                    seeds.append((d, w << 13 | off, b))
                    if d in tr.entries:
                        anchors.add(w << 13 | off)
            if KERNEL_SECTOR * SECTOR <= d < KERNEL_SECTOR * SECTOR + 0x4000:
                run = 0x4000 + d - KERNEL_SECTOR * SECTOR
                seeds.append((d, run, None))
                if d in tr.entries:
                    anchors.add(run)
        for b, a in ctx.banks.items():                   # each bank's vector table
            for w in ctx.wins.get(b, ()):
                for off in range(0, 48, 3):
                    if a[off] >= 0 and disc.buf[a[off]] == 0x4C:
                        seeds.append((a[off], w << 13 | off, b))
        tables = c.follow(ctx, seeds, "entry")
        done = set()
        for _ in range(8):                               # tables, then what they reach, until nothing is new
            new = [t for t in tables if t not in done]
            if not new:
                break
            seeds = []
            for t in new:
                done.add(t)
                seeds += c.table_seeds(ctx, t)
            tables = c.follow(ctx, seeds, "table")
        for _ in range(4):                               # arrays that hold a pointer to code that ran
            before = len(c.found)
            c.follow(ctx, c.pointer_runs(ctx, anchors | {sd[1] for sd in seeds}, 2), "array")
            if len(c.found) == before:
                break
        for _ in range(8):                               # routines are laid end to end: what follows a return
            nxt = [t for t in c.tails if t[0] not in c.found and t[0] + 3 <= len(disc.buf)]
            c.tails = []
            nxt = [(d, pc, b) for d, pc, b in nxt if c.extent(d)[1] and len(c.extent(d)[0]) >= 3]
            if not nxt:
                break
            c.follow(ctx, nxt, "next")
        c.tails = []
    if not strict:
        for ctx in ctxs:                                 # arrays of three or more, anchored or not
            c.follow(ctx, c.pointer_runs(ctx, None, 3), "array3")
        free = Ctx("free", "free", {})
        gap = collections.Counter()
        for q in c.found:
            gap[q // SECTOR] += 1
        for sec in sorted(k for k, v in gap.items() if v >= 64):     # every stretch of a code sector nothing claimed
            seeds = []
            for d in range(sec * SECTOR, (sec + 1) * SECTOR - 3):
                if d not in c.found:
                    instrs, ok, score = c.extent(d, cache=False)
                    if ok and len(instrs) >= 6:
                        seeds.append((d, 0x8000 | d & 0x1FFF, None))
            c.follow(free, seeds, "sweep")
            c._ext.clear()
    return c


def folds(names):
    """The held-out groups: each stage on its own (its code is its own), and the rest in families."""
    out = [[n] for n in names if n.startswith("stage_")]
    for fam in ("attract", "play_s0"):
        out.append([n for n in names if n.startswith(fam)])
    out += [[n] for n in ("boot", "gameover", "menu_files", "sound_test") if n in names]
    return out


def holdout(only=None, out=None):
    """Hold scripts out, run both censuses from the others' traces, and score them on the code only the held-out
    scripts ran (truth), what was found that no script ran, and every byte the two censuses disagree on."""
    names = all_scripts()
    disc = Disc()
    full = Traces(names)
    pure = set(Model(disc, full).pure)
    # data for certain: sent to a sound or video port by some script and executed by none
    sunk = {i for i, v in enumerate(full.map2) if v & SUNK and not full.map[i] & 3} if full.map2 else set()
    rows, sink = [], (open(out, "w") if out else None)

    def say(text):
        print(text, flush=True)
        if sink:
            sink.write(text + "\n")
            sink.flush()

    for held in folds(names):
        if only and held[0] not in only:
            continue
        label = "+".join(held) if len(held) < 3 else f"{held[0]}..({len(held)})"
        tr = Traces([n for n in names if n not in held])
        model = Model(disc, tr)
        ctxs = contexts(disc, tr)
        truth = full.code - tr.code
        strict = run_census(disc, tr, ctxs, model, "strict")
        strict._ext.clear()
        relaxed = run_census(disc, tr, ctxs, model, "relaxed")
        relaxed._ext.clear()
        for mode, c in (("strict", strict), ("relaxed", relaxed)):
            claims = {d: r for d, r in c.found.items() if r != "traced"}
            row = {"held_out": label, "mode": mode, "truth": len(truth),
                   "found_of_truth": sum(1 for d in claims if d in truth), "claims": len(claims),
                   "claims_ran": sum(1 for d in claims if d in full.code),
                   "claims_in_unrun_loaded_sectors": sum(1 for d in claims if d // SECTOR in pure),
                   "sunk_bytes": len(sunk), "false_claims_in_sunk": sum(1 for d in claims if d in sunk)}
            row["recall"] = round(row["found_of_truth"] / max(1, row["truth"]), 3)
            for rule in ("copy", "entry", "table", "array", "next", "array3", "sweep"):
                row["by_" + rule] = sum(1 for r in claims.values() if r == rule)
                row["false_" + rule] = sum(1 for d, r in claims.items() if r == rule and d in sunk)
            # a claimed instruction that a script ran differently is wrong: it starts on an operand byte, or has
            # the opcode of another instruction inside it. Only bytes some script ran can show this.
            ran_starts = [p for p in c.starts if p not in tr.code and p in full.code]
            row["claimed_instrs_where_a_script_ran_code"] = len(ran_starts)
            bad = 0
            for p in ran_starts:
                n = disc.decode(p)[0]
                if not full.map[p] & 1 or any(full.map[p + i] & 1 for i in range(1, n)):
                    bad += 1
            row["of_them_contradicted"] = bad
            rows.append(row)
            say("FOLD " + ", ".join(f"{k} {v}" for k, v in row.items()))
        disagree = collections.defaultdict(lambda: [0, 0, 0])
        for d, r in relaxed.found.items():
            if r == "traced" or d in strict.found:
                continue
            why = strict.why.get(relaxed.owner.get(d)) or "no strict rule reaches it: " + r
            e = disagree[why]
            e[0] += 1
            e[1] += d in full.code
            e[2] += d in truth
        for why, (n, ran, tru) in sorted(disagree.items(), key=lambda kv: -kv[1][0]):
            say(f"DISAGREE {label} | {why} | bytes {n} | ran by some script {ran} | ran only by the held-out {tru}")
    if sink:
        sink.close()
    return rows


def main():
    if not os.environ.get("CPU_RUN_JOBS"):
        print("refusing to run outside the slot queue (CPU_RUN_JOBS is not set); run it as:\n"
              "  cpu-run -j N -m SIZE -- python3 tools/rondo/census.py ...")
        return 2
    if "--holdout" in sys.argv[1:]:
        holdout(out=os.path.join(TOP, "build", "rondo", "census_holdout.txt"))
        return 0
    names = all_scripts()
    disc = Disc()
    tr = Traces(names)
    model = Model(disc, tr)
    ctxs = contexts(disc, tr)
    strict = run_census(disc, tr, ctxs, model, "strict")
    strict._ext.clear()
    relaxed = run_census(disc, tr, ctxs, model, "relaxed")
    relaxed._ext.clear()
    # censusmap.bin: 1 traced, 2 strict census only, 4 relaxed census only; census.csv: bytes per rule
    m = bytearray(USER_BYTES)
    rows = collections.Counter()
    for d, rule in relaxed.found.items():
        kind = 1 if rule == "traced" else 2 if d in strict.found else 4
        m[d] = kind
        rows[(kind, rule)] += 1
    open(MAP, "wb").write(m)
    with open(OUT, "w", newline="\n", encoding="ascii") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["set", "rule", "bytes"])
        for (kind, rule), n in sorted(rows.items()):
            w.writerow([{1: "traced", 2: "strict", 4: "relaxed only"}[kind], rule, n])
    print(f"traced {len(tr.code)} B; strict census adds {sum(1 for r in strict.found.values() if r != 'traced')} B; "
          f"relaxed adds {sum(1 for r in relaxed.found.values() if r != 'traced')} B")
    print("unplaced calls:", strict.stats["unplaced"], "ambiguous:", strict.stats["ambiguous"])
    return 0


if __name__ == "__main__":
    sys.exit(main())
