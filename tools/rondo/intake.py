#!/usr/bin/env python3
"""intake.py -- the raw intake (tier T1): every traced routine with no filed assembly, as labelled ca65.

    cpu-run -j 1 -m 2G -- python3 tools/rondo/intake.py [--out DIR] [--stats]

asmbuild.py calls build() on every run (the intake is regenerated, never tracked; see
build/rondo/intake-design.md for the reasons) and assembles what it returns in place beside the
filed assembly. Run alone, this writes the readable files a writer takes routines from:

    build/rondo/raw/sNNNN.s    one file per 64 sectors, one block per distinct routine content
    build/rondo/raw/index.csv  one row per location: tag, group label, size, near-copy group, file

What becomes an instruction: exactly the bytes the traces ran. pcebox's trace.c flags a byte 1
when it was fetched as an opcode and 2 when fetched as an operand, folded onto the disc byte the
RAM byte was loaded from; the merged discmap.bin ORs every script. An opcode-flagged byte starts
an instruction (huc.py gives its length); any byte no script ran stays `.byte`, each line marked
`; untraced`, so a later script that runs it turns it into an instruction when this is rerun.

A routine is a catalog.py routine (call, table, interrupt and jump targets, spans as asmfile.span
cuts them) minus the bytes filed assembly claims, plus an `orphan` for every run of executed bytes
no catalog span covers (code entered by a branch, a computed jump or a return). Routines of
identical bytes are one group: one text, placed at every disc location holding it. A routine under
16 bytes is merged only with copies at the same offset in their 8 KB bank (a bare RTS elsewhere is
a coincidence, not a copy). Every group is assembled at its own `.org` (its most common run
address), so the same text yields the same bytes at each placement.
"""
import bisect
import collections
import csv
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import huc  # noqa: E402

SECTOR = 2048
F_OPC, F_OPD = 1, 2
SMALL = 16                       # under this many bytes a copy must sit at the same offset in its bank
NEAR_ABS = ("abs", "abx", "aby", "ind", "iax", "blk", "tab", "tax")
KIND = {0x20: "call", 0x44: "call", 0x4C: "jump", 0x6C: "table", 0x7C: "table", 0x100: "interrupt"}
CAP = 8
ALLOW_MISSING = False             # set by asmbuild --allow-missing-trace, never by a landing
PER_FILE = 64                    # sectors per raw file


def tag(lo):
    return f"{lo // SECTOR:04X}:{lo % SECTOR:03X}"


def label(lo, off=0):
    base = f"r{lo // SECTOR:04X}_{lo % SECTOR:03X}"
    return base if off == 0 else f"{base}_{off:03X}"


class Group:
    """One distinct routine content, and every disc location that holds it."""

    def __init__(self, members, content):
        self.members = sorted(members, key=lambda p: p.lo)
        self.content = content
        self.n = len(content)
        self.canon = self.members[0].lo
        votes = collections.Counter(r for p in self.members for r in p.runs)
        self.run = min(votes, key=lambda r: (-votes[r], r)) if votes else None
        self.org = self.run if self.run is not None else 0x4000 | (self.canon & 0x1FFF)
        self.items = []                  # decoded: ("i", off, n, name, mode, ops, unrun) | ("d", off, n)
        self.istart = {}
        self.inside = set()
        self.needs = set()               # offsets that carry a label
        self.opc_run = 0
        self.near = None


class Piece:
    def __init__(self, lo, hi, entry, runs):
        self.lo, self.hi, self.entry, self.runs = lo, hi, entry, set(runs)
        self.group = None


def decode_group(g, dm):
    n = g.n
    opc = bytearray(n)
    run = bytearray(n)
    for p in g.members:
        seg = dm[p.lo:p.hi]
        for i, v in enumerate(seg):
            if v & F_OPC:
                opc[i] = 1
            if v & 3:
                run[i] = 1
    g.opc_run = sum(run)
    o = 0
    left = 0           # executed bytes left as data
    crossing = 0
    while o < n:
        if opc[o]:
            ln, name, mode, ops, _ = huc.decode(g.content, o, 0)
            if name is not None and o + ln <= n:
                g.items.append(("i", o, ln, name, mode, ops, not all(run[o + k] for k in range(1, ln))))
                g.istart[o] = len(g.items) - 1
                g.inside.update(range(o + 1, o + ln))
                o += ln
                continue
            crossing += 1
        e = o + 1
        while e < n and not opc[e]:
            e += 1
        if g.items and g.items[-1][0] == "d" and g.items[-1][1] + g.items[-1][2] == o:
            prev = g.items.pop()
            g.items.append(("d", prev[1], prev[2] + e - o))
        else:
            g.items.append(("d", o, e - o))
        left += sum(run[o:e])
        o = e
    return left, crossing


class Intake:
    pass


def run_banks(pieces):
    """Per piece, the physical banks its executed bytes ran in: p.banks = Counter of (bank of the piece's first byte,
    base address of the piece's first byte). runmap.bin holds, per executed disc byte, the logical address it was
    fetched at and the bank mapped there; a byte past an 8 KB window boundary ran in the next bank, so each vote is
    moved back to the bank the piece starts in. No run map: every piece has an empty vote."""
    for p in pieces:
        p.banks = collections.Counter()
    path = os.path.join(TRACE, "runmap.bin")
    if not os.path.exists(path):
        return
    rm = open(path, "rb").read()
    los = [p.lo for p in pieces]
    for i in range(len(rm) // 12):
        d, lg, bank, kind = struct.unpack_from("<IHBB", rm, i * 12)
        k = bisect.bisect_right(los, d) - 1
        if k < 0 or d >= pieces[k].hi:
            continue
        p = pieces[k]
        base = (lg - (d - p.lo)) & 0xFFFF
        p.banks[((bank - ((lg >> 13) - (base >> 13))) & 0xFF, base)] += 1


def build(filed, disc, dm, rows, es, scripts=None, control=None, log=print):
    """The intake: returns an Intake with .pieces (placements for asmbuild) and the report data.

    filed: asmbuild's filed routines (dicts with disc, end, run, text, name, labels).
    disc, dm: track 02's user data and the merged code map. rows, es: catalog.build()'s.
    """
    out = Intake()
    nd = len(disc)
    used = bytearray(nd)
    rows = [r for r in rows if r["space"] == "D"]
    for r in rows:
        r["lo"] = int(r["offset"], 16)
        r["runs"] = {int(x, 16) for x in r["run"].split("/") if x}
    starts = sorted(r["lo"] for r in rows)
    rowat = {r["lo"]: r for r in rows}

    # ---- which bytes are instructions: every opcode-flagged byte, with the operand bytes its length names
    opcm = dm.translate(bytes(v & 1 for v in range(256)))
    imask = bytearray(nd)
    operand_of = {}
    ilen = {}
    pos = opcm.find(1)
    while pos >= 0:
        ln = huc.decode(disc, pos, 0)[0]
        ilen[pos] = ln
        imask[pos:pos + ln] = b"\x01" * ln
        for k in range(1, ln):
            operand_of[pos + k] = pos
        pos = opcm.find(1, pos + 1)
    out.imask = bytes(imask)
    out.inst_bytes = imask.count(1)

    # ---- claims: filed assembly, and every exact copy of it at a catalog start
    claims = []                                    # (lo, hi, filed, is_copy)
    for k, f in enumerate(filed):
        claims.append((f["disc"], f["disc"] + f["end"], f, False))
        if control == "dup" and k == 0:
            continue                               # planted: the intake keeps this range too
        used[f["disc"]:f["disc"] + f["end"]] = b"\x01" * f["end"]
    copies = []
    for f in filed:
        n = f["end"]
        pat = bytes(disc[f["disc"]:f["disc"] + n])
        for s in starts:
            if s == f["disc"] or not disc.startswith(pat, s):
                continue
            if n < SMALL and (s & 0x1FFF) != (f["disc"] & 0x1FFF):
                continue
            if rowat[s]["lo"] + rowat[s]["bytes"] - s < n:
                continue
            if used.find(1, s, s + n) >= 0:
                continue
            used[s:s + n] = b"\x01" * n
            claims.append((s, s + n, f, True))
            copies.append((s, f))
    out.filed_copies = copies

    # ---- pieces: catalog spans minus claims, then orphans
    pieces = []
    out.extended = out.cut_inside = 0

    def carve(lo, hi, entry, runs):
        o = lo
        while o < hi:
            if used[o]:
                o += 1
                continue
            e = o
            while e < hi and not used[e]:
                e += 1
            while e < nd and operand_of.get(e, -1) >= o:      # the cut fell inside an instruction
                end = operand_of[e] + ilen[operand_of[e]]
                if used.find(1, e, end) >= 0:
                    out.cut_inside += 1
                    break
                e = end
                out.extended += 1
            pieces.append(Piece(o, e, entry if o == lo else "fragment", runs if o == lo else ()))
            used[o:e] = b"\x01" * (e - o)
            o = e

    for r in sorted(rows, key=lambda r: r["lo"]):
        carve(r["lo"], r["lo"] + r["bytes"], r["entry"], r["runs"])
    free = (int.from_bytes(out.imask, "little") & ~int.from_bytes(used, "little")).to_bytes(nd, "little")
    orphans = orphan_b = 0
    for m in re.finditer(b"\x01+", free):
        pieces.append(Piece(m.start(), m.end(), "orphan", ()))
        orphans += 1
        orphan_b += m.end() - m.start()
    used = None
    pieces.sort(key=lambda p: p.lo)
    out.orphans, out.orphan_bytes = orphans, orphan_b
    run_banks(pieces)

    # how each piece is entered, from the edges (catalog.py's single label depends on set order when a routine is
    # entered in two ways, so it is not used)
    ent = collections.defaultdict(set)
    for kind, src, dst in es:
        if dst[0] == "D" and kind in KIND:
            ent[dst[1]].add(KIND[kind])
    for p in pieces:
        p.kinds = ent.get(p.lo) or {p.entry}

    # ---- run addresses: from the JSR or JMP operand that reached a piece (the catalog's rule, applied to
    # every piece, entered at its start or inside it)
    plos = [p.lo for p in pieces]
    for kind, src, dst in es:
        if kind in (0x20, 0x4C) and src[0] == "D" and dst[0] == "D":
            k = bisect.bisect_right(plos, dst[1]) - 1
            if k >= 0 and dst[1] < pieces[k].hi:
                v = (disc[src[1] + 1] | disc[src[1] + 2] << 8) - (dst[1] - pieces[k].lo)
                if 0 <= v < 0x10000:
                    pieces[k].runs.add(v)

    # ---- groups: identical bytes; a small routine also needs the same offset in its bank
    by = collections.defaultdict(list)
    plain = collections.defaultdict(list)
    for p in pieces:
        c = bytes(disc[p.lo:p.hi])
        by[c if len(c) >= SMALL else (c, p.lo & 0x1FFF)].append(p)
        plain[c].append(p)
    groups = []
    for key, mem in by.items():
        content = key if isinstance(key, bytes) else key[0]
        g = Group(mem, content)
        for p in mem:
            p.group = g
        groups.append(g)
    groups.sort(key=lambda g: g.canon)
    out.groups, out.pieces_in = groups, pieces
    # a group no call or jump reached takes its run address from a piece that touches one of its copies on the
    # disc (a fall-through or a shared tail is loaded contiguously), and says so
    by_hi = {p.hi: p for p in pieces}
    by_lo = {p.lo: p for p in pieces}
    for g in groups:
        g.inferred = False
    for _ in range(8):
        changed = False
        for g in groups:
            if g.run is not None:
                continue
            for p in g.members:
                q = by_hi.get(p.lo)
                q2 = by_lo.get(p.hi)
                if q is not None and q.group.run is not None:
                    g.run = (q.group.run + (q.hi - q.lo)) & 0xFFFF
                elif q2 is not None and q2.group.run is not None:
                    g.run = (q2.group.run - (p.hi - p.lo)) & 0xFFFF
                if g.run is not None:
                    g.org, g.inferred, changed = g.run, True, True
                    break
        if not changed:
            break
    # the run map is the address the bytes ran at: it replaces a neighbour's inference and gives a group no call
    # reached its address (a branch to a routine placed at another address than it ran at cannot be tied)
    for g in groups:
        votes = collections.Counter()
        for p in g.members:
            for (bank, base), n in p.banks.items():
                votes[base] += n
        if votes:
            top = min(votes, key=lambda b: (-votes[b], b))
            if top != g.run:
                g.run = g.org = top
                g.inferred = False
    out.inferred = sum(1 for g in groups if g.inferred)
    out.small_split = sum(len(v) - 1 for c, v in plain.items() if len(c) < SMALL and len(v) > 1) - \
        sum(len(g.members) - 1 for g in groups if g.n < SMALL)
    out.small_merged = sum(len(g.members) - 1 for g in groups if g.n < SMALL)

    # ---- decode every group from the union of what its copies ran
    out.untraced_left = out.crossing = 0
    for g in groups:
        left, cr = decode_group(g, dm)
        out.untraced_left += left
        out.crossing += cr
    out.alone_bytes = sum(sum(1 for i in range(p.hi - p.lo) if dm[p.lo + i] & 3) for p in pieces)
    out.union_bytes = sum(g.opc_run * len(g.members) for g in groups)

    # ---- who owns a disc byte: intake group, or filed text (with its labels)
    spans = []
    for p in pieces:
        spans.append((p.lo, p.hi, "i", p.group))
    for lo, hi, f, is_copy in claims:
        if control == "dup" and f is filed[0] and not is_copy:
            continue
        spans.append((lo, hi, "f", f))
    spans.sort(key=lambda s: s[0])
    los = [s[0] for s in spans]

    def owner(d):
        k = bisect.bisect_right(los, d) - 1
        if k >= 0 and d < spans[k][1]:
            lo, hi, kind, ref = spans[k]
            return kind, ref, d - lo, lo
        return None

    out.owner = owner
    calls = collections.defaultdict(set)           # instruction disc offset -> {target disc offset}
    toC = set()
    for kind, src, dst in es:
        if src[0] != "D" or kind == 0x100:
            continue
        if dst[0] == "D":
            calls[src[1]].add(dst[1])
        else:
            toC.add(src[1])

    def named(d):
        k = owner(d)
        if not k:
            return None
        kind, ref, off, lo = k
        if kind == "i":
            if off in ref.inside:
                return None
            return label(ref.canon, off), ref.org + off, ref, off
        nm = ref["labels"].get(off)
        return (nm, ref["run"] + off, None, None) if nm else None

    # ---- operands
    st = collections.Counter()
    ambiguous = []
    unres = []
    byc = {g.canon: g for g in groups}
    for g in groups:
        g.text_ops = {}
        g.notes = {}
        for it in g.items:
            if it[0] != "i":
                continue
            _, o, ln, name, mode, ops, unrun = it
            if mode in ("rel", "zrel"):
                dispb = g.content[o + ln - 1]
                disp = dispb - 256 if dispb & 0x80 else dispb
                tgt_off = o + ln + disp
                res = set()
                why = set()
                for p in g.members:
                    if 0 <= tgt_off < g.n:
                        if tgt_off in g.inside:
                            res.add(None)
                            why.add("target is inside an instruction")
                        else:
                            res.add(("in", tgt_off))
                    else:
                        nm = named(p.lo + tgt_off)
                        if nm and nm[1] - (g.org + o + ln) == disp:
                            res.add(("x", nm[0], nm[2].canon if nm[2] else None, nm[3]))
                        else:
                            res.add(None)
                            why.add("target is in no routine or filed label" if not nm else
                                    "label's run address does not give the displacement")
                one = next(iter(res)) if len(res) == 1 else None
                if len(res) > 1:
                    why.add("copies of the routine disagree")
                if one is None:
                    g.text_ops[o] = f"*{ln + disp:+d}"
                    st["branch left as an offset from *"] += 1
                    unres.append((g, o, name, "branch: " + "; ".join(sorted(why)), ""))
                elif one[0] == "in":
                    g.needs.add(one[1])
                    g.text_ops[o] = label(g.canon, one[1])
                    st["branch to a label in its own routine"] += 1
                else:
                    if one[2] is not None:
                        byc[one[2]].needs.add(one[3])
                    g.text_ops[o] = one[1]
                    st["branch to another routine's label"] += 1
            elif mode == "abs" and name in ("jsr", "jmp"):
                T = ops[0]
                cand = set()
                card = False
                seen = False
                for p in g.members:
                    S = p.lo + o
                    card = card or S in toC
                    for d in calls.get(S, ()):
                        seen = True
                        nm = named(d)
                        if nm and nm[1] == T:
                            cand.add((nm[0], nm[2].canon if nm[2] else None, nm[3]))
                if len(cand) == 1:
                    c = next(iter(cand))
                    if c[1] is not None:
                        byc[c[1]].needs.add(c[2])
                    g.text_ops[o] = c[0]
                    st["call or jump tied to a label"] += 1
                else:
                    why = ("ambiguous: several routines resident at that address" if len(cand) > 1 else
                           "target is System Card code" if card and not seen else
                           "no trace edge for this instruction" if not seen else
                           "target's own run address differs from this operand")
                    st["call or jump left as a number: " + why] += 1
                    unres.append((g, o, name, "call or jump: " + why, f"${T:04X}"))
                    if len(cand) > 1:
                        names = sorted(c[0] for c in cand)
                        ambiguous.append((g.canon, o, names))
                        g.notes[o] = "ambiguous: " + " | ".join(names)
    out.stats = st
    out.unres = unres
    out.rows = rows

    # ---- near copies: same instruction layout and bytes, absolute operands masked
    def near_key(g):
        b = bytearray(g.content)
        lay = []
        for it in g.items:
            if it[0] == "i":
                lay.append((it[1], it[2]))
                if it[4] in NEAR_ABS:
                    for k in range(it[1] + 1, it[1] + it[2]):
                        b[k] = 0
        return bytes(b), tuple(lay)

    nk = collections.defaultdict(list)
    for g in groups:
        if any(it[0] == "i" and it[4] in NEAR_ABS for it in g.items):
            nk[near_key(g)].append(g)
    near = sorted((sorted(v, key=lambda g: g.canon) for v in nk.values() if len(v) > 1), key=lambda v: v[0].canon)
    for i, v in enumerate(near):
        for k, g in enumerate(v):
            g.near = (i, k, len(v), v[0])
    out.near = near

    # ---- placement text
    for g in groups:
        g.lines = emit(g)
    out.pieces = []
    for g in groups:
        for p in g.members:
            body = list(g.lines)
            if p is not g.members[0]:
                body = [".scope"] + body + [".endscope"]
            out.pieces.append({"disc": p.lo, "end": p.hi, "run": g.org, "text": "\n".join(body), "src": "intake",
                               "name": label(g.canon),
                               "ins": sum(it[2] for it in g.items if it[0] == "i"),
                               "dat": sum(it[2] for it in g.items if it[0] == "d")})
    out.pieces.sort(key=lambda x: x["disc"])
    out.ambiguous = ambiguous
    out.toC = toC
    out.calls = calls
    out.claims = claims
    if control == "alter":
        for x in out.pieces:
            m = re.search(r"^(    \w+ #\$)([0-9A-F]{2})(.*)$", x["text"], re.M)
            if m:
                new = f"{(int(m.group(2), 16) ^ 1):02X}"
                x["text"] = x["text"][:m.start(2)] + new + x["text"][m.end(2):]
                log(f"control alter: {x['name']} at {tag(x['disc'])}: immediate {m.group(2)} -> {new}")
                break
    elif control == "omit":
        for g in groups:
            if len(g.members) == 1 and g.opc_run >= 8:
                out.pieces = [x for x in out.pieces if x["disc"] != g.members[0].lo]
                log(f"control omit: {label(g.canon)} at {tag(g.canon)} left out ({g.opc_run} B run)")
                break
    elif control == "unplace":
        for g in groups:
            if len(g.members) >= 2:
                last = g.members[-1].lo
                out.pieces = [x for x in out.pieces if x["disc"] != last]
                log(f"control unplace: copy of {label(g.canon)} at {tag(last)} not placed")
                break
    return out


def emit(g):
    """The routine's text: labels, instructions, and `.byte` for every byte no script ran."""
    lab = {0} | set(g.needs)
    lines = []
    for it in g.items:
        if it[0] == "d":
            _, o, n = it
            cuts = sorted(x for x in lab if o < x < o + n)
            a = o
            for b in cuts + [o + n]:
                if a in lab:
                    lines.append(label(g.canon, a) + ":")
                for k in range(a, b, 8):
                    seg = g.content[k:min(k + 8, b)]
                    lines.append("    .byte " + ",".join(f"${v:02X}" for v in seg) + "  ; untraced")
                a = b
            continue
        _, o, ln, name, mode, ops, unrun = it
        if o in lab:
            lines.append(label(g.canon, o) + ":")
        t = g.text_ops.get(o)
        if mode == "rel":
            s = f"{name} {t}"
        elif mode == "zrel":
            s = f"{name} ${ops[0]:02X},{t}"
        elif mode == "abs" and t and name in ("jsr", "jmp"):
            s = f"{name} {t}"
        else:
            s = huc.ca65(name, mode, ops)
        note = g.notes.get(o, "")
        if unrun:
            note = (note + "; " if note else "") + "operand not fetched from the disc (patched at run time?)"
        lines.append("    " + s + ("  ; " + note if note else ""))
    return lines


# ---------------------------------------------------------------------------------------------
# inputs, headers, files

TRACK = os.path.join(TOP, "build", "rondo", "disc", "track02.bin")
TRACE = os.path.join(TOP, "build", "rondo", "trace")
RAW = os.path.join(TOP, "build", "rondo", "raw")


def check_inputs():
    """Refuse unless every script has a trace and the merged map is exactly the OR of those traces.

    The intake reads: track 02 (build/rondo/disc/track02.bin), the merged code map
    build/rondo/trace/discmap.bin, every script's build/rondo/trace/<script>/edges.csv, the filed
    assembly under asm/rondo and the filed C under src/rondo (catalog.py's names). Nothing else.
    """
    import glob
    names = sorted(os.path.splitext(os.path.basename(p))[0] for p in
                   glob.glob(os.path.join(HERE, "scripts", "*.txt")) if not os.path.basename(p).startswith("_"))
    missing = [n for n in names if not (os.path.exists(os.path.join(TRACE, n, "discmap.bin")) and
                                        os.path.exists(os.path.join(TRACE, n, "edges.csv")))]
    if missing and not ALLOW_MISSING:
        raise SystemExit(f"intake: scripts with no trace under build/rondo/trace: {' '.join(missing)}; "
                         f"run trace.py before building (the intake is a function of the traces)")
    merged = open(os.path.join(TRACE, "discmap.bin"), "rb").read()
    acc = 0
    for n in [n for n in names if n not in missing]:
        acc |= int.from_bytes(open(os.path.join(TRACE, n, "discmap.bin"), "rb").read(), "little")
    if acc != int.from_bytes(merged, "little"):
        raise SystemExit("intake: the merged build/rondo/trace/discmap.bin is not the OR of the scripts' traces "
                         "(older than a script's trace?); rerun trace.py so it is merged again")


def load():
    """Track 02, the merged code map, and the catalog's routines and edges; None if there is no trace."""
    import catalog
    if not os.path.exists(os.path.join(TRACE, "discmap.bin")):
        return None
    check_inputs()
    disc = open(TRACK, "rb").read()
    dm = open(os.path.join(TRACE, "discmap.bin"), "rb").read()
    rows, es = catalog.build()
    return disc, dm, rows, es


def script_runs(groups):
    """Which scripts ran each group: {group: [script names]}."""
    import glob
    tbl = bytes(1 if v & 3 else 0 for v in range(256))
    got = collections.defaultdict(list)
    names = []
    for path in sorted(glob.glob(os.path.join(TRACE, "*", "discmap.bin"))):
        name = os.path.basename(os.path.dirname(path))
        names.append(name)
        m = open(path, "rb").read()
        for g in groups:
            if any(any(m[p.lo:p.hi].translate(tbl)) for p in g.members):
                got[g].append(name)
    return got, names


def who(out, space, off):
    if space == "D":
        k = out.owner(off)
        if not k:
            return f"D:{off:X}"
        kind, ref, rel, lo = k
        return f"{tag(lo)} {ref['name']}" if kind == "f" else tag(lo)
    return f"{space}:{off:X}"


def fmt_ops(it):
    return ",".join(f"${v:04X}" if it[4] in NEAR_ABS else f"${v:02X}" for v in it[5])


def near_diff(g, first):
    out = []
    a = {it[1]: it for it in first.items if it[0] == "i"}
    for it in g.items:
        if it[0] == "i" and it[4] in NEAR_ABS and it[1] in a and it[5] != a[it[1]][5]:
            out.append(f"+{it[1]:02X} {it[3]} {fmt_ops(it)} (first {fmt_ops(a[it[1]])})")
    return out


def headers(out, es, scripts, names):
    callers = collections.defaultdict(set)
    callees = collections.defaultdict(set)
    for kind, src, dst in es:
        k = KIND.get(kind, "?")
        if dst[0] == "D":
            o = out.owner(dst[1])
            if o and o[0] == "i":
                callers[o[1]].add(f"{who(out, *src)} ({k})")
        if src[0] == "D" and kind != 0x100:
            o = out.owner(src[1])
            if o and o[0] == "i":
                callees[o[1]].add(who(out, *dst) if dst[0] == "D" else "System Card")
    out.hdr = {}
    for g in out.groups:
        ent = sorted({k for p in g.members for k in p.kinds})
        runs = collections.Counter(r for p in g.members for r in p.runs)
        other = sorted(r for r in runs if r != g.run)
        lines = [f"; ---- {label(g.canon)}: {len(g.members)} location{'s' if len(g.members) > 1 else ''}, "
                 f"{g.n} B, {g.opc_run} B run ----"]
        for i, p in enumerate(g.members):
            lines.append(f"; @routine {tag(p.lo)}" + ("" if i == 0 else "  (exact copy)"))
        bank, base, why = group_bank(g)
        out.stats[bank_key(why)] = out.stats.get(bank_key(why), 0) + 1
        out.stats["run address from the run map, not from a call"] = out.stats.get(
            "run address from the run map, not from a call", 0) + (1 if g.run is None and bank is not None else 0)
        lines.append("; @run " + (f"{bank:02X}" if bank is not None else "??") + ":" +
                     (f"{base:04X}" if base is not None else "????") +
                     (f"  (bank: {why})" if bank is None else "") +
                     ("  (run address and bank from the run map)" if g.run is None and bank is not None else
                      "  (the run map puts it at this address, not at the inferred one)" if bank is not None and base != g.run else "") +
                     ("  (inferred from the neighbouring routine on the disc)" if g.inferred else "") +
                     (f"  (also seen at {', '.join(f'{r:04X}' for r in other)})" if other else "") +
                     (f"  (assembled at ${g.org:04X})" if g.run is None else ""))
        lines.append(f"; size {g.n} B, run by a script {g.opc_run} B, untraced {g.n - g.opc_run} B; entered by "
                     + ", ".join(ent))
        for nm, bag in (("callers", callers[g]), ("callees", callees[g])):
            v = sorted(bag)
            lines.append(f"; {nm}: " + ("; ".join(v[:CAP]) + (f"; +{len(v) - CAP} more" if len(v) > CAP else "")
                                        if v else "none traced"))
        sc = scripts.get(g, [])
        lines.append(f"; run by: " + (f"all {len(names)} scripts" if len(sc) == len(names) else
                                    ", ".join(sc[:CAP]) + (f", +{len(sc) - CAP} more" if len(sc) > CAP else "")
                                    if sc else "no script"))
        if g.near:
            i, k, n, first = g.near
            if k == 0:
                lines.append(f"; near copies: N{i:04d}, first of {n} routines with the same opcodes and "
                             f"only absolute operands differing")
            else:
                d = near_diff(g, first)
                lines.append(f"; near copies: N{i:04d}, member {k + 1} of {n}, first is {label(first.canon)}; "
                             f"operands that differ from it: " + "; ".join(d[:CAP]) +
                             (f"; +{len(d) - CAP} more" if len(d) > CAP else ""))
        out.hdr[g] = lines


def group_bank(g):
    """(bank, base, why not): the most common (bank, base address) the group's copies' executed bytes started at, from
    the run map; at the group's run address when it has one. A group no call reached has no run address of its own:
    the run map's most common base is printed for it, and the assembly keeps its .org."""
    votes = collections.Counter()
    for p in g.members:
        for (bank, base), n in p.banks.items():
            votes[(bank, base)] += n
    if not votes:
        return None, g.run, "no byte of it in the run map"
    if g.run is not None:
        mine = {k: n for k, n in votes.items() if k[1] == g.run}
        if not mine:        # the run address was inferred from a neighbour and the run map disagrees: it is right
            top = min(votes, key=lambda k: (-votes[k], k))
            return top[0], top[1], ""
        return min(mine, key=lambda k: (-mine[k], k))[0], g.run, ""
    top = min(votes, key=lambda k: (-votes[k], k))
    return top[0], top[1], ""


def bank_key(why):
    return "run bank known" if not why else "run bank unknown: " + why


def write_files(out, path):
    os.makedirs(path, exist_ok=True)
    for f in os.listdir(path):
        os.remove(os.path.join(path, f))
    files = collections.defaultdict(list)
    for g in out.groups:
        files[g.canon // SECTOR // PER_FILE].append(g)
    rows = []
    for k, gs in sorted(files.items()):
        name = f"s{k * PER_FILE:04X}.s"
        with open(os.path.join(path, name), "w", encoding="ascii", newline="\n") as f:
            f.write(f"; raw intake: routines whose canonical location is in sectors "
                    f"{k * PER_FILE:04X}-{k * PER_FILE + PER_FILE - 1:04X}. Generated by tools/rondo/intake.py;\n"
                    f"; never edit. To file a routine, copy its block to asm/rondo/<area>/ (see PROCEDURE).\n"
                    f"; @run BB:AAAA: the bank is the physical bank the trace saw the routine's first byte fetched from "
                    f"(runmap.bin); ?? says why not.\n\n")
            for g in gs:
                f.write("\n".join(out.hdr[g]) + "\n" + "\n".join(g.lines) + "\n\n")
                for p in g.members:
                    near = f"N{g.near[0]:04d}" if g.near else ""
                    rows.append((tag(p.lo), label(g.canon), tag(g.canon), g.n, g.opc_run, "/".join(sorted(p.kinds)),
                                 f"{g.org:04X}" if g.run is not None else "", near, name))
    with open(os.path.join(path, "unresolved.csv"), "w", encoding="ascii", newline="\n") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["group", "location", "offset", "instruction", "cause", "operand", "candidates"])
        for g, o, name, cause, opd in sorted(out.unres, key=lambda u: (u[0].canon, u[1])):
            w.writerow([label(g.canon), tag(g.canon), f"{o:03X}", name, cause, opd, g.notes.get(o, "")])
    with open(os.path.join(path, "index.csv"), "w", encoding="ascii", newline="\n") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["where", "group", "canonical", "bytes", "bytes_run", "entry", "run", "near", "file"])
        w.writerows(sorted(rows))
    return len(files)


def summary(out, filed_n=0):
    gs = out.groups
    places = sum(len(g.members) for g in gs)
    ins = sum(1 for g in gs for it in g.items if it[0] == "i")
    insb = sum(it[2] for g in gs for it in g.items if it[0] == "i")
    datb = sum(it[2] for g in gs for it in g.items if it[0] == "d")
    lines = [
        f"intake: {len(gs)} distinct routines at {places} disc locations ({places - len(gs)} extra copies, "
        f"{sum((len(g.members) - 1) * g.n for g in gs)} B of span); {len(out.groups) and out.orphans} orphans "
        f"({out.orphan_bytes} B) among them",
        f"  instruction bytes any script began: {out.inst_bytes} B; spans cut inside an instruction and extended: "
        f"{out.extended} ({out.cut_inside} left cut); run addresses inferred from a neighbour: {out.inferred}",
        f"  decoded once per distinct routine: {ins} instructions, {insb} B; kept as bytes: {datb} B "
        f"(of which {out.untraced_left} B were run but not as an instruction start)",
        f"  executed bytes counted per location: {out.alone_bytes} B decoding each copy alone, "
        f"{out.union_bytes} B with the union over a group's copies (+{out.union_bytes - out.alone_bytes})",
        f"  small routines (under {SMALL} B) merged only at the same bank offset: {out.small_merged} merged, "
        f"{out.small_split} identical ones left apart",
        f"  filed routines with exact copies elsewhere: {len({f['disc'] for _, f in out.filed_copies})} routines, "
        f"{len(out.filed_copies)} copies placed from their filed text",
        f"  near-copy groups: {len(out.near)} ({sum(len(v) for v in out.near)} distinct routines, "
        f"{sum(len(g.members) for v in out.near for g in v)} locations); with 4+ bytes: "
        f"{sum(1 for v in out.near if v[0].n >= 4)}",
    ]
    for k, v in sorted(out.stats.items()):
        lines.append(f"  {k}: {v}")
    return "\n".join(lines)


def main():
    import cpuguard
    cpuguard.require("intake.py", jobs=1, mem="2G")
    import asmbuild
    argv = sys.argv[1:]
    path = argv[argv.index("--out") + 1] if "--out" in argv else RAW
    data = load()
    if data is None:
        print("no trace under build/rondo/trace: nothing to take in")
        return 1
    disc, dm, rows, es = data
    out = build(asmbuild.filed_pieces(disc), disc, dm, rows, es)
    scripts, names = script_runs(out.groups)
    headers(out, es, scripts, names)
    n = write_files(out, path)
    print(summary(out))
    print(f"  {n} files and index.csv in {path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
