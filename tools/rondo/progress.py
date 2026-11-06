#!/usr/bin/env python3
"""progress.py -- the README's status and the progress picture, from the ledger alone.

    python3 tools/rondo/progress.py

Reads docs/rondo/ledger.csv, which ledger.py works out from the gates. Writes
docs/rondo/progress.svg and the part of README.md between its progress markers. No number
here is entered by hand: to change one, change what the gates measure.

M1 is every routine T3 and M2 every routine T4, so both are measured in code bytes against
the estimate of the game's code, the only denominator until the static census replaces it.
Run it after every gate run that changes the ledger.
"""
import csv
import os

TOP = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
LEDGER = os.path.join(TOP, "docs", "rondo", "ledger.csv")
CARD_LEDGER = os.path.join(TOP, "syscard", "ledger.csv")
SVG = os.path.join(TOP, "docs", "rondo", "progress.svg")
README = os.path.join(TOP, "README.md")
START, END = "<!-- progress:start -->", "<!-- progress:end -->"
ESTIMATE = 196 * 1024  # tools/rondo/estimate.py: 131 to 260 KB
CARD_TRACED = 4649      # the card code the scripts run: a floor, no census of the card yet


def tally(rows, test):
    chosen = [r for r in rows if test(r)]
    return len(chosen), sum(int(r["bytes"]) for r in chosen)


def is_vector(row):
    return row["routine"].startswith(("Vec", "RichterVec"))


def percent(part, whole):
    return f"{100 * part / whole:.2f}%"


def bar(y, label, part, colour, whole=ESTIMATE):
    width = 560
    filled = max(1, round(width * part / whole)) if part else 0
    return (f'<text x="16" y="{y - 6}" class="t">{label}</text>'
            f'<rect x="16" y="{y}" width="{width}" height="18" class="track"/>'
            f'<rect x="16" y="{y}" width="{filled}" height="18" fill="{colour}"/>')


def picture(m1_bytes, m2_bytes, m1_text, m2_text, below_text, card):
    return "\n".join([
        '<svg xmlns="http://www.w3.org/2000/svg" width="592" height="284" viewBox="0 0 592 284"'
        ' role="img" aria-label="Decompilation progress">',
        "<style>.t{font:13px sans-serif;fill:#57606a}.h{font:bold 14px sans-serif;fill:#57606a}"
        ".track{fill:#8c959f;fill-opacity:.25}</style>",
        '<text x="16" y="22" class="h">Progress against the estimate of 196 KB of game code</text>',
        bar(56, m1_text, m1_bytes, "#2da44e"),
        bar(104, m2_text, m2_bytes, "#0969da"),
        f'<text x="16" y="146" class="t">{below_text}</text>',
        '<text x="16" y="176" class="h">The System Card, against the 4,649 B the scripts run (a floor)</text>',
        bar(206, card["m1_text"], card["m1"], "#2da44e", CARD_TRACED),
        bar(250, card["m2_text"], card["m2"], "#0969da", CARD_TRACED),
        "</svg>", ""])


def main():
    rows = list(csv.DictReader(open(LEDGER, newline="")))
    t3 = tally(rows, lambda r: r["tier"] == "T3")
    t4 = tally(rows, lambda r: r["tier"] == "T4")
    t2 = tally(rows, lambda r: r["tier"] == "T2")
    vectors = tally(rows, lambda r: r["tier"] in ("T3", "T4") and is_vector(r))
    logic = (t3[0] + t4[0] - vectors[0], t3[1] + t4[1] - vectors[1])
    below = tally(rows, lambda r: r["proper_c"] == "0")
    m1_bytes, m2_bytes = t3[1] + t4[1], t4[1]
    short = {g: tally(rows, lambda r, g=g: r[g] == "0")[0]
             for g in ("gate_0", "gate_c", "gate_a5", "gate_a7", "symbolic")}
    m1_text = f"M1, T3 or better: {percent(m1_bytes, ESTIMATE)} ({m1_bytes:,} B, {t3[0] + t4[0]} routines)"
    m2_text = f"M2, T4: {percent(m2_bytes, ESTIMATE)} ({m2_bytes:,} B, {t4[0]} routines)"
    below_text = f"Below proper C: {below[0]} routines, {below[1]:,} B, a stepping stone"
    card_rows = list(csv.DictReader(open(CARD_LEDGER, newline="")))
    card_t3 = tally(card_rows, lambda r: r["tier"] == "T3")
    card_t4 = tally(card_rows, lambda r: r["tier"] == "T4")
    card_t2 = tally(card_rows, lambda r: r["tier"] == "T2")
    card_below = tally(card_rows, lambda r: r["proper_c"] == "0")
    card_m1 = card_t3[1] + card_t4[1]
    card_short = {g: tally(card_rows, lambda r, g=g: r[g] == "0")[0]
                  for g in ("gate_0", "gate_c", "gate_a5", "gate_a7", "symbolic")}
    card = {"m1": card_m1, "m2": card_t4[1],
            "m1_text": f"Card M1, T3 or better: {percent(card_m1, CARD_TRACED)} ({card_m1:,} B, {card_t3[0] + card_t4[0]} routines)",
            "m2_text": f"Card M2, T4: {percent(card_t4[1], CARD_TRACED)} ({card_t4[1]:,} B, {card_t4[0]} routines)"}
    with open(SVG, "w", encoding="ascii", newline="\n") as f:
        f.write(picture(m1_bytes, m2_bytes, m1_text, m2_text, below_text, card))
    block = "\n".join([
        START,
        "![Progress against the estimate of the game's code](docs/rondo/progress.svg)",
        "",
        f"**{below[0]} of {len(rows)} routines ({below[1]:,} B) fail the proper-C check: "
        "register-shaped C, a stepping stone, not decompiled.** The check is a pattern check, so "
        "every count above T2 is a ceiling until reviewed.",
        "",
        "| Measure | Routines | Bytes | Of the estimate of 196 KB |",
        "|---|---:|---:|---:|",
        f"| M2: T4 | {t4[0]} | {t4[1]:,} | {percent(t4[1], ESTIMATE)} |",
        f"| M1: T3 or better | {t3[0] + t4[0]} | {m1_bytes:,} | {percent(m1_bytes, ESTIMATE)} |",
        f"| of those, real logic and not one-line vectors | {logic[0]} | {logic[1]:,} | {percent(logic[1], ESTIMATE)} |",
        f"| T2: filed, not certified | {t2[0]} | {t2[1]:,} | {percent(t2[1], ESTIMATE)} |",
        "",
        f"Routines short of a gate, overlapping: A7 swap {short['gate_a7']}, every byte run "
        f"{short['gate_c']}, A5 judged call {short['gate_a5']}, symbolic assembly "
        f"{short['symbolic']}, image {short['gate_0']}.",
        "",
        "The System Card, its own project and [ledger](syscard/ledger.csv), held to the same gates:",
        "",
        "| Measure | Routines | Bytes | Of the 4,649 B the scripts run, a floor |",
        "|---|---:|---:|---:|",
        f"| M2: T4 | {card_t4[0]} | {card_t4[1]:,} | {percent(card_t4[1], CARD_TRACED)} |",
        f"| M1: T3 or better | {card_t3[0] + card_t4[0]} | {card_m1:,} | {percent(card_m1, CARD_TRACED)} |",
        f"| T2: filed, not certified | {card_t2[0]} | {card_t2[1]:,} | {percent(card_t2[1], CARD_TRACED)} |",
        f"| below proper C | {card_below[0]} | {card_below[1]:,} | |",
        "",
        f"Card routines short of a gate, overlapping: A7 swap {card_short['gate_a7']}, every byte run "
        f"{card_short['gate_c']}, A5 judged call {card_short['gate_a5']}, symbolic assembly "
        f"{card_short['symbolic']}, image {card_short['gate_0']}.",
        "",
        "Generated by `tools/rondo/progress.py` from [the ledger](docs/rondo/ledger.csv), which "
        "`tools/rondo/ledger.py` works out from the gates.",
        END])
    text = open(README, encoding="utf-8", newline="").read()
    if text.count(START) != 1 or text.count(END) != 1:
        raise SystemExit("README.md must hold each progress marker once")
    text = text[:text.index(START)] + block + text[text.index(END) + len(END):]
    with open(README, "w", encoding="utf-8", newline="") as f:
        f.write(text)
    print(m1_text)
    print(m2_text)
    print(below_text)
    print(card["m1_text"])
    print(card["m2_text"])


if __name__ == "__main__":
    main()
