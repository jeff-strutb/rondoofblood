#!/usr/bin/env python3
"""census.py -- the game's code that no script has run, read from the overlays the scenes load.

    python3 tools/rondo/census.py           write build/rondo/census.csv and censusmap.bin
    python3 tools/rondo/census.py --check   measure it on the scenes the traces ran

The game loads its overlays through scenes (LoadSceneScript, disc_load.c): SceneRecords
($74D1 in the resident kernel, five bytes each: the scene's disc address and its list of
ops) and SceneReadDestinations ($70F8). The ops map MPR4-6 (op 0), read sectors at an offset
from the scene's address to a destination (ops 1-$7F; the card's cd_read with _DH 1: _AL
sectors to the logical address _BX), go to another list ($C0), call the scene overlay
($80-$83), or end. The census replays every scene's ops, so it knows which disc byte each
physical bank holds once the scene is loaded; a later read into a bank replaces the earlier.

Code runs with a bank mapped in some window ($8000, $A000, $C000), not always the one it was
read through. The window of each bank, by the scene's list of ops, is learned from the traced
routines whose run addresses the edges give (catalog.py); a bank of a list no trace has run
takes the window its own vector table points into (consecutive JMP abs at the bank's start,
every target in one window and inside the bank). The kernel's resident code runs at
$4000-$7FFF from sector $43 (HANDBOOK.md).

From the entries (each bank's vector table, and the places traced code entered that scene's own
memory from outside its 8 KB), it decodes statically: both ways at a branch, into JSR, BSR and
JMP abs targets it can place (a window with one bank of that scene in it, or the kernel); it
stops at RTS, RTI, BRK, a JMP through a table or a pointer, a target it cannot place, and an
undefined opcode (counted, though the HuC6280 defines 234 of 256 opcodes, so data decodes
cleanly and this count cannot catch a wrong entry). It does not follow JMP (abs,X) dispatch
tables or handlers kept in data, so its recall on the scenes the traces ran is low (--check
measures it); it is a floor of the game's code, not proof it runs, and not yet a census to put
in a denominator. censusmap.bin marks what it finds (1 found, 3 also traced).
"""
import array
import collections
import csv
import glob
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import asmfile  # noqa: E402
import huc  # noqa: E402

TRACE = os.path.join(TOP, "build", "rondo", "trace")
OUT = os.path.join(TOP, "build", "rondo", "census.csv")
MAP = os.path.join(TOP, "build", "rondo", "censusmap.bin")
ROUTINES = os.path.join(TOP, "build", "rondo", "routines.csv")
SECTOR = 2048
BANK = 0x2000
TRACK_START = 0xF32
SCENE_RECORDS, SCENE_DESTINATIONS = 0x74D1, 0x70F8
KERNEL_SECTOR = 0x43
OP_BANKS, OP_GOTO, OP_CALLS = 0, 0xC0, (0x80, 0x81, 0x82, 0x83)
CALL_KINDS = (0x20, 0x44, 0x6C, 0x7C)


def kernel_disc(run):
    """The resident kernel: run $4000-$7FFF is sector $43 on (MPR2-3 = $69-$6A)."""
    return KERNEL_SECTOR * SECTOR + run - 0x4000 if 0x4000 <= run < 0x8000 else None


class Disc:
    def __init__(self):
        self.buf = open(asmfile.IMAGE, "rb").read()
        self.sectors = len(self.buf) // SECTOR

    def kb(self, run):
        return self.buf[kernel_disc(run)]

    def kw(self, run):
        return self.kb(run) | self.kb(run + 1) << 8


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


def replay(scene):
    """Bank (relative to the card's RAM base) -> the disc offset each of its bytes holds after the scene."""
    mpr, banks = {4: None, 5: None, 6: None}, {}
    for op in scene["ops"]:
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


def traced_runs():
    """(disc offset, run address) of every traced routine whose run an edge gave."""
    out = []
    if not os.path.exists(ROUTINES):
        return out
    for r in csv.DictReader(open(ROUTINES, newline="")):
        if r["space"] == "D" and r["run"]:
            out += [(int(r["offset"], 16), int(x, 16)) for x in r["run"].split("/")]
    return out


def outside_entries():
    """Disc offsets entered by a call from code outside their own 8 KB of disc: the overlays' doors."""
    out = set()
    for path in glob.glob(os.path.join(TRACE, "*", "edges.csv")):
        for r in csv.DictReader(open(path, newline="")):
            if int(r["kind"], 16) in CALL_KINDS and r["to_space"] == "D":
                t = int(r["to"], 16)
                if r["from_space"] != "D" or abs(int(r["from"], 16) - t) >= BANK:
                    out.add(t)
    return out


def where_in(banks):
    """Disc offset -> [(bank, offset in bank)] for one scene's memory."""
    out = collections.defaultdict(list)
    for b, a in banks.items():
        for off, d in enumerate(a):
            if d >= 0:
                out[d].append((b, off))
    return out


def vector_window(disc, a):
    """The window a bank's own vector table points into: JMP abs at 0, 3, ... with every target in
    one window and inside the bank's loaded bytes; None when it has no such table."""
    targets = []
    for off in range(0, 3 * 16, 3):
        d = a[off]
        if d < 0 or disc.buf[d] != 0x4C:
            break
        targets.append(disc.buf[d + 1] | disc.buf[d + 2] << 8)
    if len(targets) < 2 or len({t >> 13 for t in targets}) != 1:
        return None
    w = targets[0] >> 13
    return w if 4 <= w <= 6 and all(a[t & (BANK - 1)] >= 0 for t in targets) else None


def learn(disc, all_scenes, memories):
    """(list, bank) -> windows seen in the traces, and scene -> (bank, offset) of the doors traced into
    that scene's own memory. Doors are never lent between scenes: only the vector tables at the
    banks' starts are shared by the stages of one list; the rest of each stage's code is laid
    out its own way, so another stage's door lands mid-instruction or in data."""
    runs = traced_runs()
    doors = outside_entries()
    windows, entries = collections.defaultdict(set), collections.defaultdict(set)
    for s in all_scenes:
        at = where_in(memories[s["n"]])
        for d, run in runs:
            for b, off in at.get(d, ()):
                if off == run & (BANK - 1) and 4 <= run >> 13 <= 6:
                    windows[(s["list"], b)].add(run >> 13)
        for d in doors:
            for b, off in at.get(d, ()):
                entries[s["n"]].add((b, off))
    return windows, entries


def walk(disc, banks, wins, seeds):
    """Statically decode one scene's code from its seeds: (found offsets, undefined-opcode stops)."""
    def place(run):
        k = kernel_disc(run)
        if k is not None:
            return k
        w = run >> 13
        hits = [b for b, ws in wins.items() if w in ws and banks[b][run & (BANK - 1)] >= 0]
        return banks[hits[0]][run & (BANK - 1)] if len(hits) == 1 else None

    found, seen, stops = set(), set(), 0
    work = list(seeds)
    while work:
        d, pc = work.pop()
        while (d, pc) not in seen:
            seen.add((d, pc))
            if d + 3 > len(disc.buf):
                break
            n, name, mode, ops, target = huc.decode(disc.buf, d, pc)
            if name is None:
                stops += 1
                break
            found.update(range(d, d + n))
            if mode in ("rel", "zrel") and target is not None:
                t = d + (target - pc) if target >> 13 == pc >> 13 else place(target)
                if t is not None:
                    work.append((t, target))
            elif name in ("jsr", "jmp") and mode == "abs":
                t = place(target)
                if t is not None:
                    work.append((t, target))
            if name in huc.ENDS:
                break
            d, pc = d + n, pc + n
    return found, stops


def census(disc, check=False):
    all_scenes = scenes(disc)
    memories = {s["n"]: replay(s) for s in all_scenes}
    windows, entries = learn(disc, all_scenes, memories)
    traced = open(os.path.join(TRACE, "discmap.bin"), "rb").read()
    found_all, rows = set(), []
    for s in all_scenes:
        banks = memories[s["n"]]
        wins = {}
        for b, a in banks.items():
            ws = windows.get((s["list"], b)) or ({vector_window(disc, a)} - {None})
            if ws:
                wins[b] = ws
        seeds = []
        for b, ws in wins.items():
            a = banks[b]
            positions = {off for off in range(0, 48, 3) if a[off] >= 0 and disc.buf[a[off]] == 0x4C}
            positions |= {off for bb, off in entries.get(s["n"], ()) if bb == b}
            for off in positions:
                if a[off] >= 0:
                    seeds += [(a[off], w << 13 | off) for w in ws]
        found, stops = walk(disc, banks, wins, seeds)
        held = {d for a in banks.values() for d in a if d >= 0}
        traced_here = {d for d in held if traced[d] & 3}
        mine = found & held
        rows.append({"scene": f"{s['n']:02X}", "sector": f"{s['sector']:X}", "list": f"{s['list']:04X}",
                     "banks_placed": len(wins), "seeds": len(seeds), "found": len(mine),
                     "traced": len(traced_here), "traced_found": len(mine & traced_here),
                     "new": len(mine - traced_here), "undefined_stops": stops})
        found_all |= found
    if check:
        print("scenes with traced code: recall = traced bytes the census finds from the doors alone")
        for r in rows:
            if r["traced"]:
                print(f"  scene {r['scene']} list {r['list']}: traced {r['traced']} B, found {r['traced_found']} "
                      f"({100 * r['traced_found'] / r['traced']:.1f}%), beyond the traces {r['new']} B, "
                      f"undefined-opcode stops {r['undefined_stops']}")
    return rows, found_all, traced


def main():
    disc = Disc()
    check = "--check" in sys.argv[1:]
    rows, found, traced = census(disc, check)
    m = bytearray(len(disc.buf))
    for d in found:
        m[d] = 1 | (2 if traced[d] & 3 else 0)
    open(MAP, "wb").write(m)
    with open(OUT, "w", newline="\n", encoding="ascii") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0]), lineterminator="\n")
        w.writeheader()
        w.writerows(rows)
    code = sum(1 for v in traced if v & 3)
    new = sum(1 for d in found if not traced[d] & 3)
    print(f"{len(rows)} scenes; census finds {len(found)} B of code, {len(found) - new} B of it traced "
          f"(of {code} B the traces ran), {new} B no script has run")
    return 0


if __name__ == "__main__":
    sys.exit(main())
