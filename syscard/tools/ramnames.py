#!/usr/bin/env python3
"""ramnames.py -- the card's one list of names, into its assembly include and C header.

    python3 syscard/tools/ramnames.py

Reads syscard/config/symbols.csv; writes syscard/asm/include/ram.inc and
syscard/include/card_ram.h. A zero-page name is its page offset in assembly and its logical
address, in the page at $2000, in C. Neither output is edited by hand.
"""
import csv
import os

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


if __name__ == "__main__":
    main()
