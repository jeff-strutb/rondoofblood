#!/usr/bin/env python3
"""progress.py -- put T3/T4 and M1/M2 progress from a ledger into the README and its SVG.

    python3 tools/rondo/progress.py [LEDGER]     default docs/rondo/ledger.csv

Writes docs/rondo/progress.svg and the block between the markers in README.md, below-standard
first. M1 is every routine T3 and M2 every routine T4, so each is shown in code bytes at that
tier against the estimate of the disc's code (estimate.py), which is still an estimate: no
census has been made.
"""
import csv
import os
import sys

TOP = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
README = os.path.join(TOP, "README.md")
SVG = os.path.join(TOP, "docs", "rondo", "progress.svg")
ESTIMATE_KB = 196
ESTIMATE = ESTIMATE_KB * 1024
START = "<!-- progress: written by tools/rondo/progress.py -->"
END = "<!-- /progress -->"
WIDTH = 720
BAR = 640


def tally(rows, test):
    chosen = [row for row in rows if test(row)]
    return len(chosen), sum(int(row["bytes"]) for row in chosen)


def percent(count):
    return f"{100 * count / ESTIMATE:.2f}%"


def bar(y, label, parts):
    """A labelled bar of (bytes, colour) against the estimate, with a minimum visible width."""
    out = [f'<text x="40" y="{y - 6}" class="label">{label}</text>',
           f'<rect x="40" y="{y}" width="{BAR}" height="16" class="track"/>']
    x = 40.0
    for count, colour in parts:
        width = BAR * count / ESTIMATE
        if count:
            width = max(width, 2)
        out.append(f'<rect x="{x:.1f}" y="{y}" width="{width:.1f}" height="16" fill="{colour}"/>')
        x += width
    return out


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(TOP, "docs", "rondo", "ledger.csv")
    rows = list(csv.DictReader(open(path, newline="")))
    t4 = tally(rows, lambda r: r["tier"] == "T4")
    t3 = tally(rows, lambda r: r["tier"] == "T3")
    t2 = tally(rows, lambda r: r["tier"] == "T2")
    bad = tally(rows, lambda r: r["proper_c"] == "0")
    vectors = tally(rows, lambda r: r["tier"] in ("T3", "T4") and int(r["bytes"]) == 3)
    t3_up = (t3[0] + t4[0], t3[1] + t4[1])
    real = (t3_up[0] - vectors[0], t3_up[1] - vectors[1])
    filed = (len(rows), sum(int(r["bytes"]) for r in rows))
    style = ('<style>.label{font:13px sans-serif;fill:#57606a}.title{font:bold 15px sans-serif;fill:#57606a}'
             '.track{fill:#8c959f;fill-opacity:.2}</style>')
    svg = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{WIDTH}" height="176" '
           f'viewBox="0 0 {WIDTH} 176" role="img" aria-label="Decompilation progress">', style,
           f'<text x="40" y="24" class="title">Code bytes by tier, against about {ESTIMATE_KB} KB '
           'estimated on the disc</text>']
    svg += bar(52, f"M1, every routine T3: {t3_up[1]:,} B, {percent(t3_up[1])}", [(t4[1], "#2da44e"), (t3[1], "#54aeff")])
    svg += bar(92, f"M2, every routine T4: {t4[1]:,} B, {percent(t4[1])}", [(t4[1], "#2da44e")])
    svg += bar(132, f"Filed, not yet certified (T2): {t2[1]:,} B, {percent(t2[1])}", [(t2[1], "#bf8700")])
    svg.append("</svg>")
    with open(SVG, "w", encoding="ascii", newline="\n") as f:
        f.write("\n".join(svg) + "\n")
    block = [
        START,
        f"**{t2[0]} of {filed[0]} routines ({t2[1]:,} B) are below T3, and {bad[0]} of those "
        f"({bad[1]:,} B) fail the proper-C check: register-shaped C, a stepping stone, not "
        "decompiled.**",
        "",
        f"![Progress by tier](docs/rondo/progress.svg)",
        "",
        "| | Routines | Code bytes | Of about " + f"{ESTIMATE_KB} KB |",
        "|---|---:|---:|---:|",
        f"| T4 | {t4[0]} | {t4[1]:,} | {percent(t4[1])} |",
        f"| T3 | {t3[0]} | {t3[1]:,} | {percent(t3[1])} |",
        f"| T2 | {t2[0]} | {t2[1]:,} | {percent(t2[1])} |",
        f"| M1 (every routine T3 or better) | {t3_up[0]} | {t3_up[1]:,} | {percent(t3_up[1])} |",
        f"| M2 (every routine T4) | {t4[0]} | {t4[1]:,} | {percent(t4[1])} |",
        "",
        f"{vectors[0]} of the {t3_up[0]} routines at T3 or T4 are one-line jump vectors, leaving "
        f"{real[0]} routines of real logic, {real[1]:,} B. The proper-C check is a pattern check, so "
        "T3 and T4 are a ceiling until reviewed. The denominator is an estimate, not a census, and "
        "only routines the scripts reach can be certified. From [the ledger](docs/rondo/ledger.csv).",
        END,
    ]
    text = open(README, encoding="utf-8", newline="").read()
    if START in text:
        head = text[:text.index(START)]
        tail = text[text.index(END) + len(END):]
    else:
        at = text.index("**[The ledger](docs/rondo/ledger.csv)")
        finish = text.index("catalog to classify.", at) + len("catalog to classify.")
        head, tail = text[:at], text[finish:]
    with open(README, "w", encoding="utf-8", newline="") as f:
        f.write(head + "\n".join(block) + tail)
    print(f"T4 {t4[0]} ({t4[1]} B), T3 {t3[0]} ({t3[1]} B), T2 {t2[0]} ({t2[1]} B); "
          f"M1 {percent(t3_up[1])}, M2 {percent(t4[1])}; {bad[0]} fail proper C")


if __name__ == "__main__":
    main()
