#!/usr/bin/env python3
"""reach.py -- read a piece of the disc as code, from its entry points.

Decodes recursively from seeds in a window of track 02 run at a given
address: follows branches, JSR and JMP inside the window, and stops at RTS,
RTI, BRK and indirect jumps. Bytes nothing reached print as data, so a
listing shows what is known code and what is not yet. For reading only: a
byte shown as code here is reachable by decoding, not proven to run.

    python3 tools/rondo/reach.py --sector 0x43 --count 8 --org 0x4000 \\
        --seed 0x40F9 --vectors 0x4006:0x4100 > build/rondo/read/kernel.lst
"""
import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import huc  # noqa: E402

TOP = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SECTOR = 2048


def num(s):
    return int(s, 0)


def walk(buf, org, seeds):
    """Decode from every seed: instruction starts, routine entries, branch labels."""
    code, entries, labels, external = {}, set(seeds), set(), {}
    work = list(seeds)
    while work:
        pc = work.pop()
        while org <= pc < org + len(buf) and pc not in code:
            n, name, mode, ops, target = huc.decode(buf, pc - org, pc)
            if name is None:
                code[pc] = (n, None, None, ops, None)
                break
            code[pc] = (n, name, mode, ops, target)
            if target is not None:
                if org <= target < org + len(buf):
                    work.append(target)
                    (entries if name == "jsr" or name == "jmp" else labels).add(target)
                else:
                    external.setdefault(target, []).append(pc)
            if name in huc.ENDS:
                break
            pc += n
    return code, entries, labels, external


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--sector", type=num, required=True)
    ap.add_argument("--count", type=num, required=True)
    ap.add_argument("--org", type=num, required=True)
    ap.add_argument("--skip", type=num, default=0, help="bytes into the first sector")
    ap.add_argument("--seed", type=num, action="append", default=[])
    ap.add_argument("--vectors", action="append", default=[],
                    help="LO:HI, a table of JMP abs instructions; each is a seed")
    ap.add_argument("--image", default=os.path.join(TOP, "build", "rondo", "disc", "track02.bin"))
    a = ap.parse_args()
    with open(a.image, "rb") as f:
        f.seek(a.sector * SECTOR + a.skip)
        buf = f.read(a.count * SECTOR - a.skip)
    seeds = list(a.seed)
    for v in a.vectors:
        lo, hi = (num(x) for x in v.split(":"))
        seeds += list(range(lo, hi, 3))
    code, entries, labels, external = walk(buf, a.org, seeds)
    pc = a.org
    end = a.org + len(buf)
    while pc < end:
        if pc in code:
            n, name, mode, ops, _ = code[pc]
            if pc in entries:
                off = pc - a.org + a.skip
                print(f"\n;--- {a.sector + off // SECTOR:04X}:{off % SECTOR:03X}")
            raw = " ".join(f"{x:02X}" for x in buf[pc - a.org:pc - a.org + n])
            mark = "L" if pc in labels and pc not in entries else " "
            print(f"{pc:04X}{mark} {raw:<20} {huc.text(name, mode, ops)}")
            pc += n
        else:
            q = pc
            while q < end and q not in code:
                q += 1
            gap = buf[pc - a.org:q - a.org]
            print(f"\n;=== {len(gap)} B not reached at {pc:04X}")
            for i in range(0, len(gap), 32):
                print(f"{pc + i:04X}  {gap[i:i + 32].hex(' ')}")
            print()
            pc = q
    print("\n;=== outside the window")
    for t in sorted(external):
        print(f";{t:04X} from " + " ".join(f"{p:04X}" for p in external[t][:8]))
    reached = sum(c[0] for c in code.values())
    print(f";{reached} B reached of {len(buf)}, {len(entries)} entries", file=sys.stderr)


if __name__ == "__main__":
    main()
