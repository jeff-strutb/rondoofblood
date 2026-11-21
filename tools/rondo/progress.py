#!/usr/bin/env python3
"""progress.py -- the README's status and the progress picture, from the ledger alone.

    python3 tools/rondo/progress.py

Reads docs/rondo/ledger.csv, which ledger.py works out from the gates. Writes
docs/rondo/progress.svg and the part of README.md between its progress markers. No number
here is entered by hand: to change one, change what the gates measure.

M1 is every routine T3 and M2 every routine T4, so both are measured in code bytes against a
denominator that is computed, never typed in: the code the scripts ran counted by distinct whole
routines (the merged trace, rebuilt here from every script's own map; routines whose whole span
is byte for byte the same count once, with the offsets of them that any script ran unioned; code
outside every catalog routine counts per disc location), plus the code the static census finds
that no script ran once an audited census exists (CENSUS_COUNTED). Until then the denominator
is the traced floor and the picture and README say so. The card's is the card bytes the scripts
ran, likewise a floor.

    python3 tools/rondo/progress.py --print     the headline lines only, writing nothing

It refuses to publish unless every script in tools/rondo/scripts has a trace and a gate A7
result, and the tracked files that decide the ledger match HEAD. Output names the commit.
Run it after every gate run that changes the ledger.
"""
import csv
import glob
import os
import subprocess
import sys

TOP = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
LEDGER = os.path.join(TOP, "docs", "rondo", "ledger.csv")
CARD_LEDGER = os.path.join(TOP, "syscard", "ledger.csv")
SVG = os.path.join(TOP, "docs", "rondo", "progress.svg")
ROUTINES = os.path.join(TOP, "build", "rondo", "routines.csv")      # catalog.py
DISC_IMAGE = os.path.join(TOP, "build", "rondo", "disc", "track02.bin")
README = os.path.join(TOP, "README.md")
START, END = "<!-- progress:start -->", "<!-- progress:end -->"
TRACE = os.path.join(TOP, "build", "rondo", "trace")
SCRIPTS = os.path.join(TOP, "tools", "rondo", "scripts")
GATE_A5 = os.path.join(TOP, "config", "rondo", "gate_a5.csv")
GATE_A7 = os.path.join(TOP, "config", "rondo", "gate_a7.csv")
OUTPUTS = {"docs/rondo/ledger.csv", "syscard/ledger.csv", "docs/rondo/progress.svg", "README.md",
           "tools/rondo/progress.py"}
CENSUS_COUNTED = False  # set only by an audited census (PLAN.md section 1); none exists yet


def git(*args):
    return subprocess.run(["git", "-C", TOP, *args], capture_output=True, text=True, check=True).stdout.rstrip("\n")


def script_names():
    return sorted(os.path.splitext(os.path.basename(p))[0] for p in glob.glob(os.path.join(SCRIPTS, "*.txt"))
                  if not os.path.basename(p).startswith("_"))


def refuse(why):
    raise SystemExit(f"progress.py: not publishing: {why}")


def distinct_routines_floor(merged):
    """(bytes by distinct whole routines, bytes by disc location): the traced code with every group of
    byte-identical catalog routines counted once, over the offsets any copy ran, and the code outside
    every routine's span counted where it sits."""
    if not os.path.exists(ROUTINES) or os.path.getmtime(ROUTINES) < os.path.getmtime(os.path.join(TRACE, "discmap.bin")):
        refuse("build/rondo/routines.csv is missing or older than the trace: run catalog.py")
    buf = open(DISC_IMAGE, "rb").read()
    groups, covered = {}, bytearray(len(merged))
    for r in csv.DictReader(open(ROUTINES, newline="")):
        if r["space"] != "D":
            continue
        off, n = int(r["offset"], 16), int(r["bytes"])
        ran = {i for i in range(n) if merged[off + i] & 3}
        groups.setdefault(buf[off:off + n], set()).update(ran)
        covered[off:off + n] = b"\1" * n
    outside = sum(1 for i, v in enumerate(merged) if v & 3 and not covered[i])
    return sum(len(g) for g in groups.values()) + outside, sum(1 for v in merged if v & 3)


def floors():
    """(game bytes by distinct whole routines, card bytes, game bytes by disc location) the scripts ran,
    from each script's own maps OR-ed together, after checking that every script has a result and the
    tree is the commit it names."""
    names = script_names()
    missing = [n for n in names if not os.path.exists(os.path.join(TRACE, n, "discmap.bin"))]
    if missing:
        refuse("no trace for " + ", ".join(missing))
    if "--print" not in sys.argv[1:]:         # --print writes nothing, so it may look at an unfinished state
        a7 = {r["script"]: r["verdict"] for r in csv.DictReader(open(GATE_A7, newline=""))}
        nogate = [n for n in names if not a7.get(n)]
        if nogate:
            refuse("no gate A7 result for " + ", ".join(nogate))
        dirty = [l[3:] for l in git("status", "--porcelain", "--untracked-files=no").splitlines()
                 if l[3:] not in OUTPUTS]
        if dirty:
            refuse("tracked files differ from HEAD: " + ", ".join(dirty))
    out = []
    for m, n in (("discmap.bin", 10149 * 2048), ("cardmap.bin", 1 << 18)):
        acc = 0
        for name in names:
            acc |= int.from_bytes(open(os.path.join(TRACE, name, m), "rb").read(), "little")
        merged = acc.to_bytes(n, "little")
        if merged != open(os.path.join(TRACE, m), "rb").read():
            refuse(f"build/rondo/trace/{m} is not the OR of the scripts' maps: run trace.py")
        out.append(sum(1 for v in merged if v & 3))
        if m == "discmap.bin":
            out[0], by_location = distinct_routines_floor(merged)
    return out[0], out[1], by_location


def tally(rows, test):
    chosen = [r for r in rows if test(r)]
    return len(chosen), sum(int(r["bytes"]) for r in chosen)


def is_vector(row):
    return row["routine"].startswith(("Vec", "RichterVec"))


def percent(part, whole):
    return f"{100 * part / whole:.2f}%"


def bar(y, label, part, colour, whole):
    width = 560
    filled = max(1, round(width * part / whole)) if part else 0
    return (f'<text x="16" y="{y - 6}" class="t">{label}</text>'
            f'<rect x="16" y="{y}" width="{width}" height="18" class="track"/>'
            f'<rect x="16" y="{y}" width="{filled}" height="18" fill="{colour}"/>')


def picture(m1_bytes, m2_bytes, m1_text, m2_text, below_text, card, denom_text, game_whole, card_whole, card_title):
    return "\n".join([
        '<svg xmlns="http://www.w3.org/2000/svg" width="592" height="284" viewBox="0 0 592 284"'
        ' role="img" aria-label="Decompilation progress">',
        "<style>.t{font:13px sans-serif;fill:#57606a}.h{font:bold 14px sans-serif;fill:#57606a}"
        ".track{fill:#8c959f;fill-opacity:.25}</style>",
        f'<text x="16" y="22" class="h">{denom_text}</text>',
        bar(56, m1_text, m1_bytes, "#2da44e", game_whole),
        bar(104, m2_text, m2_bytes, "#0969da", game_whole),
        f'<text x="16" y="146" class="t">{below_text}</text>',
        f'<text x="16" y="176" class="h">{card_title}</text>',
        bar(206, card["m1_text"], card["m1"], "#2da44e", card_whole),
        bar(250, card["m2_text"], card["m2"], "#0969da", card_whole),
        "</svg>", ""])


def main():
    game_floor, card_floor, by_location = floors()
    census_code = 0  # bytes of code no script ran that an audited census found (CENSUS_COUNTED)
    game_whole = game_floor + (census_code if CENSUS_COUNTED else 0)
    card_whole = card_floor
    head = git("rev-parse", "--short", "HEAD").strip()
    gates = sorted({r["source"] for r in csv.DictReader(open(GATE_A5, newline=""))})
    n_scripts = len(script_names())
    if CENSUS_COUNTED:
        denom = f"{game_whole:,} B: {game_floor:,} B scripts ran + {census_code:,} B the audited census adds"
        denom_text = f"Progress against {game_whole:,} B of game code (scripts + audited census)"
    else:
        denom = (f"{game_whole:,} B, the traced floor: the code {n_scripts} scripts ran, counted by distinct whole "
                 f"routines ({by_location:,} B by disc location). The static census is not yet in it, so the "
                 "true total is larger")
        denom_text = f"Progress against the traced floor, {game_whole:,} B of game code, distinct routines (census not counted)"
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
    m1_text = f"M1, T3 or better: {percent(m1_bytes, game_whole)} ({m1_bytes:,} B, {t3[0] + t4[0]} routines)"
    m2_text = f"M2, T4: {percent(m2_bytes, game_whole)} ({m2_bytes:,} B, {t4[0]} routines)"
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
            "m1_text": f"Card M1, T3 or better: {percent(card_m1, card_whole)} ({card_m1:,} B, {card_t3[0] + card_t4[0]} routines)",
            "m2_text": f"Card M2, T4: {percent(card_t4[1], card_whole)} ({card_t4[1]:,} B, {card_t4[0]} routines)"}
    card_title = f"The System Card, against the {card_floor:,} B the scripts run (a floor)"
    if "--print" in sys.argv[1:]:
        print("denominator:", denom)
        print(m1_text)
        print(m2_text)
        return
    with open(SVG, "w", encoding="ascii", newline="\n") as f:
        f.write(picture(m1_bytes, m2_bytes, m1_text, m2_text, below_text, card, denom_text, game_whole,
                        card_whole, card_title))
    block = "\n".join([
        START,
        "![Progress against the traced floor of the game's code](docs/rondo/progress.svg)",
        "",
        f"Measured on `{head}`: gate records from {', '.join('`' + g + '`' for g in gates)}, "
        f"every one of the {n_scripts} scripts traced and with a gate A7 result.",
        "",
        f"**Denominator: {denom}.**",
        "",
        f"**{below[0]} of {len(rows)} routines ({below[1]:,} B) fail the proper-C check: "
        "register-shaped C, a stepping stone, not decompiled.** The check is a pattern check, so "
        "every count above T2 is a ceiling until reviewed.",
        "",
        f"| Measure | Routines | Bytes | Of the {game_whole:,} B |",
        "|---|---:|---:|---:|",
        f"| M2: T4 | {t4[0]} | {t4[1]:,} | {percent(t4[1], game_whole)} |",
        f"| M1: T3 or better | {t3[0] + t4[0]} | {m1_bytes:,} | {percent(m1_bytes, game_whole)} |",
        f"| of those, real logic and not one-line vectors | {logic[0]} | {logic[1]:,} | {percent(logic[1], game_whole)} |",
        f"| T2: filed, not certified | {t2[0]} | {t2[1]:,} | |",
        "",
        f"Routines short of a gate, overlapping: A7 swap {short['gate_a7']}, every byte run "
        f"{short['gate_c']}, A5 judged call {short['gate_a5']}, symbolic assembly "
        f"{short['symbolic']}, image {short['gate_0']}.",
        "",
        "The System Card, its own project and [ledger](syscard/ledger.csv), held to the same gates:",
        "",
        f"| Measure | Routines | Bytes | Of the {card_floor:,} B the scripts run, a floor |",
        "|---|---:|---:|---:|",
        f"| M2: T4 | {card_t4[0]} | {card_t4[1]:,} | {percent(card_t4[1], card_whole)} |",
        f"| M1: T3 or better | {card_t3[0] + card_t4[0]} | {card_m1:,} | {percent(card_m1, card_whole)} |",
        f"| T2: filed, not certified | {card_t2[0]} | {card_t2[1]:,} | |",
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
    print(f"measured on {head}; gates {', '.join(gates)}")
    print("denominator:", denom)
    print(m1_text)
    print(m2_text)
    print(below_text)
    print(card["m1_text"])
    print(card["m2_text"])


if __name__ == "__main__":
    main()
