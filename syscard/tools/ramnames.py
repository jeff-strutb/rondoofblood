#!/usr/bin/env python3
"""ramnames.py -- the card's one list of names, into its assembly include and C header.

    python3 syscard/tools/ramnames.py

Reads syscard/config/symbols.csv; writes syscard/asm/include/ram.inc and
syscard/include/card_ram.h. A zero-page name is its page offset in assembly and its logical
address, in the page at $2000, in C. Neither output is edited by hand.
syscard/config/records.csv says which names are fields of a record; from it the same run writes
syscard/include/card_ramtypes.h, one struct per record, checked at compile time against the symbols.
"""
import csv
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import typedrec  # noqa: E402

TOP = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    rows = list(csv.DictReader(open(os.path.join(TOP, "config", "symbols.csv"), newline="")))
    inc = ["; Generated from syscard/config/symbols.csv by syscard/tools/ramnames.py."]
    head = ["/* Generated from syscard/config/symbols.csv by syscard/tools/ramnames.py. */",
            "#ifndef CARD_RAM_H", "#define CARD_RAM_H", ""]
    for row in rows:
        addr = int(row["addr"], 16)
        zero_page = row["kind"] == "zp"
        inc.append(f"{row['name']} = ${addr:02X}" if zero_page else f"{row['name']} = ${addr:04X}")
        logical = 0x2000 + addr if zero_page else addr
        head.append(f"#define {row['name']} 0x{logical:04X} /* {row['note']} */")
    head += ["", "#endif", ""]
    with open(os.path.join(TOP, "asm", "include", "ram.inc"), "w", newline="\n") as f:
        f.write("\n".join(inc) + "\n")
    with open(os.path.join(TOP, "include", "card_ram.h"), "w", newline="\n") as f:
        f.write("\n".join(head))
    print(f"{len(rows)} names")
    symbols = [((0x2000 + int(r["addr"], 16)) if r["kind"] == "zp" else int(r["addr"], 16), r["name"]) for r in rows]
    try:
        lines, report, claimed = typedrec.generate(os.path.join(TOP, "config", "records.csv"), symbols,
                                                   {n for _, n in symbols}, "CARD_RAMTYPES_H", "card_ram.h",
                                                   "syscard/tools/ramnames.py from syscard/config/records.csv and symbols.csv")
    except typedrec.SpecError as e:
        sys.exit(f"ramnames.py: {e}")
    with open(os.path.join(TOP, "include", "card_ramtypes.h"), "w", encoding="ascii", newline="\n") as f:
        f.write("\n".join(lines))
    print(f"{len(claimed)} of {len(rows)} names are fields of a record; {len(rows) - len(claimed)} stay plain names")
    for line in report:
        print("  " + line)


if __name__ == "__main__":
    main()
