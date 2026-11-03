#!/usr/bin/env python3
"""Which disc sectors each RAM bank holds, from a pcebox RAM dump.

Every bank in the dump (Super System Card RAM $68-$7F, CD RAM $80-$87,
base RAM $F8) is cut into four 2 KB slots, and each slot is looked up
among track 02's user sectors. A slot of one repeated byte is reported as
fill, never matched (it would match every filler sector). A slot that
matches no sector whole is reported with its best sector-aligned match,
the bytes that agree, if any sector agrees on at least half.

    python3 tools/rondo/loadcheck.py DUMP.ram [DUMP.ram ...]
"""
import hashlib
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import disc  # noqa: E402

BANKS = list(range(0x68, 0x88)) + [0xF8]
SLOT = 2048


def load(path):
    d = open(path, "rb").read()
    hdr, ram = d[:16], d[16:]
    frame = int.from_bytes(hdr[0:4], "little")
    mpr = list(hdr[4:12])
    pc = hdr[12] | (hdr[13] << 8)
    banks = {b: ram[i * 8192:(i + 1) * 8192] for i, b in enumerate(BANKS)}
    return frame, mpr, pc, banks


def main():
    user = open(os.path.join(disc.OUT, "track02.bin"), "rb").read()
    n = len(user) // SLOT
    index = {}
    for s in range(n):
        index.setdefault(hashlib.sha1(user[s * SLOT:(s + 1) * SLOT]).digest(), []).append(s)
    for path in sys.argv[1:]:
        frame, mpr, pc, banks = load(path)
        print(f"{os.path.basename(path)}: frame {frame}, PC ${pc:04X}, "
              f"MPR0-7 {' '.join(f'{m:02X}' for m in mpr)}")
        for b in BANKS:
            row = []
            for k in range(4):
                slot = banks[b][k * SLOT:(k + 1) * SLOT]
                if slot.count(slot[0]) == SLOT:
                    row.append(f"fill {slot[0]:02X}")
                    continue
                hit = index.get(hashlib.sha1(slot).digest())
                if hit:
                    row.append(f"sector ${hit[0]:04X}" + ("+" if len(hit) > 1 else ""))
                    continue
                # Best partial match among sectors that share the slot's
                # first 16 bytes: a loaded sector the game has since written.
                best, bs = 0, None
                head = slot[:16]
                pos = user.find(head)
                tries = 0
                while pos >= 0 and tries < 64:
                    if pos % SLOT == 0:
                        same = sum(1 for x, y in zip(slot, user[pos:pos + SLOT]) if x == y)
                        if same > best:
                            best, bs = same, pos // SLOT
                    pos = user.find(head, pos + 1)
                    tries += 1
                row.append(f"~${bs:04X} {best}/{SLOT}" if bs is not None and best >= SLOT // 2
                           else "no match")
            if any(r != "fill 00" for r in row):
                print(f"  bank ${b:02X}: " + " | ".join(row))
    return 0


if __name__ == "__main__":
    sys.exit(main())
