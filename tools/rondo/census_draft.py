#!/usr/bin/env python3
"""census_draft.py -- drafts of census routines: code no script runs, filed with an @routine tag.

    python3 tools/rondo/census_draft.py NAME [NAME ...] [--out DIR]
    python3 tools/rondo/census_draft.py --file src/rondo/<area>/<file>.c [--out DIR]

cdraft.py drafts only the bytes a script ran: any other byte is untraced(), so a census routine's draft would end at
once and liftcmp could prove nothing. This tool drafts a filed routine from its tag alone: its bytes are decoded
from the @routine tag's disc location at its @run address to the end of its span, as asmfile.span cuts it (the next
tag, the end of flow, or @stop), and every byte of that span is an instruction. The items go to cdraft's own Draft,
which emits the same ops6280.h steps it emits for traced code: a branch or jump out of the span is a tail jump, as
Draft.jump_to makes one, and a fall-out at the span's end runs on into the next routine. cdraft.py is not changed.

The output is census drafts, never the draft cache: DIR (default build/rondo/census-filer/drafts) holds drafts.c,
drafts.csv, members.csv and support.c in cdraft's form, for `liftcmp.py --drafts DIR --callees wrappers`. No box
serves them (served.py reads only the cache). The banks are liftcmp's default map for a routine no trace has run
(MPR2-3 $69/$6A, MPR4-6 $00): what an access reads in a mapped bank is the case's memory either way.
"""
import argparse
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import asmfile  # noqa: E402
import cdraft  # noqa: E402

OUT = os.path.join(TOP, "build", "rondo", "census-filer", "drafts")


class Piece:
    def __init__(self, lo, n):
        self.lo, self.hi = lo, lo + n


class CensusGroup:
    """The fields of an intake group that Draft reads, for one tagged routine: every byte of its span an instruction."""

    def __init__(self, buf, routine, span):
        self.canon = routine["disc"]
        self.n = sum(ins[1] for ins in span)
        self.content = bytes(buf[self.canon:self.canon + self.n])
        self.items, self.istart = [], set()
        for pc, ln, name, mode, ops, _ in span:
            o = (pc - routine["run"]) & 0xFFFF
            self.items.append(("i", o, ln, name, mode, ops, False))
            self.istart.add(o)
        self.members = [Piece(self.canon, self.n)]


def drafts_for(names):
    buf = open(asmfile.IMAGE, "rb").read()
    routines = asmfile.all_routines()
    wanted = set(names)
    out = []
    cdraft.Draft.resolve = None          # no resolver: every branch out of the span is a tail jump
    cdraft.Draft.has_row = staticmethod(lambda g, base: True)
    for i, r in enumerate(routines):
        if r["name"] not in wanted:
            continue
        nxt = routines[i + 1]["disc"] if i + 1 < len(routines) else len(buf)
        g = CensusGroup(buf, r, asmfile.span(buf, r, nxt))
        d = cdraft.Draft(g, r["run"], "census_" + r["name"])
        d.filed = r["name"]
        d.rom = 0
        d.members = [("D", g.canon)]
        d.body = d.text()
        if d.stat["untraced_arm"] or d.stat["todo"]:
            raise SystemExit(f"census_draft: {r['name']}: {d.stat['untraced_arm']} arms inside the span on no instruction, "
                             f"{d.stat['todo']} instructions the drafter does not translate")
        out.append(d)
        wanted.discard(r["name"])
    if wanted:
        raise SystemExit("census_draft: no @routine tag for " + ", ".join(sorted(wanted)))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("names", nargs="*")
    ap.add_argument("--file", action="append", default=[])
    ap.add_argument("--out", default=OUT)
    a = ap.parse_args()
    names = list(a.names)
    for path in a.file:
        names += [r["name"] for r in asmfile.routines_in(os.path.abspath(path))]
    if not names:
        ap.error("name a routine or a --file")
    drafts = drafts_for(names)
    cdraft.write(drafts, 0, a.out)
    path = os.path.join(a.out, "drafts.c")
    text = open(path).read().replace(
        "/* Drafted by tools/rondo/cdraft.py from the traced assembly: register-shaped C, a stepping stone. */",
        "/* CENSUS DRAFTS by tools/rondo/census_draft.py from the filed tags' spans (code no script runs): register-shaped\n"
        " * C, a stepping stone, for liftcmp --drafts only. Not the draft cache; no box serves these. */", 1)
    open(path, "w", encoding="ascii", newline="\n").write(text)
    print(f"census drafts in {a.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
