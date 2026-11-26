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
that no script ran from config/rondo/census.csv (census.py: either census's claims less proven data and
proven duplicates, PLAN section 1). Without that file the denominator is the traced floor and the picture
and README say so; with a stale one (other traces) it refuses to publish. The card's is the card bytes the scripts
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
CENSUS = os.path.join(TOP, "config", "rondo", "census.csv")    # the counted census, written by census.py


def census_counts():
    """{key: int} from config/rondo/census.csv, the census the denominator counts on the terms of PLAN section 1
    (the maintainer's ruling of 2026-11-26): every byte either census claims, less proven data and proven
    duplicates; a byte not yet explained counts as code. None if there is no such file. Refuses when it was worked
    out from other traces than the ones present (its stamp), so the census is never stale in the denominator."""
    if not os.path.exists(CENSUS):
        return None
    rows = {r["key"]: r["value"] for r in csv.DictReader(open(CENSUS, newline=""))}
    if "--print" not in sys.argv[1:]:
        sys.path.insert(0, os.path.join(TOP, "tools", "rondo"))
        import census
        if rows.get("stamp") != census.trace_stamp(script_names()):
            refuse("config/rondo/census.csv was worked out from other traces: "
                   "cpu-run -j 1 -m 3G -- python3 tools/rondo/census.py")
    return {k: int(v) for k, v in rows.items() if k != "stamp"}


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


REWORK_SINCE = "2026-11-21"


def filed_c_counts():
    """(game routines with filed C, of those how many have any line of their comment or body last changed on or
    after REWORK_SINCE by git blame): the routines lifted or reworked since the new standard began."""
    import datetime
    import re
    cut = datetime.datetime.strptime(REWORK_SINCE, "%Y-%m-%d").timestamp()
    tag = re.compile(r"@routine\s+[0-9A-Fa-f]{4}:[0-9A-Fa-f]{3}\s+\w+")
    total = recent = 0
    root = os.path.join(TOP, "src", "rondo")
    for path in sorted(glob.glob(os.path.join(root, "**", "*.c"), recursive=True)):
        lines = open(path, encoding="utf-8").read().split("\n")
        if not any(tag.search(ln) for ln in lines):
            continue
        blame = subprocess.run(["git", "blame", "--line-porcelain", "-w", "--", path], cwd=TOP, capture_output=True,
                               text=True).stdout.split("\n")
        times = [int(ln.split()[1]) for ln in blame if ln.startswith("committer-time ")]
        if len(times) != len(lines) - (1 if lines and lines[-1] == "" else 0):
            times = times[:len(lines)]
        i = 0
        while i < len(lines):
            if tag.search(lines[i]):
                s0 = i
                while s0 > 0 and "/*" not in lines[s0]:
                    s0 -= 1
                j = i
                while j < len(lines) and "{" not in lines[j]:
                    j += 1
                depth, e = 0, j
                while e < len(lines):
                    depth += lines[e].count("{") - lines[e].count("}")
                    if depth <= 0 and "{" in "".join(lines[j:e + 1]):
                        break
                    e += 1
                total += 1
                recent += any(t >= cut for t in times[s0:e + 1])
                i = e
            i += 1
    return total, recent


def held_counts():
    """(scripts, routines, bytes, routines with every reaching script identical, their bytes) from the lockstep
    runs with the filed C in place (config/rondo/lockstep_held.csv, certification_queue.csv)."""
    held = os.path.join(TOP, "config", "rondo", "lockstep_held.csv")
    queue = os.path.join(TOP, "config", "rondo", "certification_queue.csv")
    if not os.path.exists(held):
        return None
    rows = list(csv.DictReader(open(held, newline="")))
    scripts = sorted({r["script"] for r in rows})
    names = {r["routine"] for r in rows}
    b = {}
    for r in rows:
        b[r["routine"]] = int(r["bytes"] or 0)
    cert = [r for r in csv.DictReader(open(queue, newline=""))
            if r["lockstep_condition"] == "met"] if os.path.exists(queue) else []
    return scripts, len(names), sum(b.values()), len(cert), sum(int(r["bytes"]) for r in cert), \
        sorted({r["source"] for r in rows})


def served_counts():
    """{mode: (identical, scripts, [sources])} from config/rondo/runs/served_a7.csv: the lockstep runs over every
    script with drafts serving (serve-all: every routine's draft in place; proper: the M1 recipe)."""
    path = os.path.join(TOP, "config", "rondo", "runs", "served_a7.csv")
    out = {}
    if os.path.exists(path):
        for r in csv.DictReader(open(path, newline="")):
            t = out.setdefault(r["mode"], [0, 0, set()])
            t[0] += r["verdict"] == "IDENTICAL"
            t[1] += 1
            t[2].add(r["source"])
    return {m: (t[0], t[1], sorted(t[2])) for m, t in out.items()}


def drafted_counts():
    """(drafts, identical, measured on) from config/rondo/drafted.csv, the drafter's last A5 with drafts serving."""
    path = os.path.join(TOP, "config", "rondo", "drafted.csv")
    if not os.path.exists(path):
        return None
    r = next(csv.DictReader(open(path, newline="")))
    return int(r["drafts"]), int(r["identical"]), r["measured_on"]


def tally_a5(card):
    """(identical, divergent, unverified, uncovered) routines of the game (card 0) or the card (card 1)."""
    rows = [r for r in csv.DictReader(open(GATE_A5, newline="")) if r["card"] == str(card)]
    return tuple(sum(1 for r in rows if r["verdict"] == v) for v in ("IDENTICAL", "DIVERGENT", "UNVERIFIED", "UNCOVERED"))


def main():
    game_floor, card_floor, by_location = floors()
    cen = census_counts()
    CENSUS_COUNTED = cen is not None
    census_code = cen["counted"] if cen else 0      # code no script ran that the census finds, counted as code
    game_whole = game_floor + census_code
    card_whole = card_floor
    head = git("rev-parse", "--short", "HEAD").strip()
    gates = sorted({r["source"] for r in csv.DictReader(open(GATE_A5, newline=""))})
    n_scripts = len(script_names())
    if CENSUS_COUNTED:
        denom = (f"{game_whole:,} B: {game_floor:,} B the {n_scripts} scripts ran, counted by distinct whole routines, "
                 f"+ {census_code:,} B of code no script runs that the static census finds "
                 f"({cen['counted_strict']:,} B by the strict census; {cen['counted_relaxed_only']:,} B only by the "
                 "relaxed one, counted as code until each byte is explained as code to file or data to name). "
                 f"Left out: {cen['duplicates']:,} B proven duplicates, whole runs byte for byte equal to code counted "
                 f"elsewhere (`config/rondo/census_duplicates.csv`), and {cen['proven_data']:,} B proven data (sent to "
                 "a video or sound port, read in place by code, or a decode a trace contradicts)")
        denom_text = f"Progress against {game_whole:,} B of game code (traced code + the census)"
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
    unreach = tally(rows, lambda r: r.get("unreachable") == "counted")       # proven unreachable: optimistic
    contra = tally(rows, lambda r: r.get("unreachable") == "contradicted")
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
    a5g = tally_a5(0)
    a5c = tally_a5(1)
    a7_rows = list(csv.DictReader(open(GATE_A7, newline="")))
    a7_ok = sum(1 for r in a7_rows if r["verdict"] == "IDENTICAL")
    a5_line = (f"**A5, each routine's C against the original call by call: {a5g[0] + a5c[0]} routines identical, "
               f"game {a5g[0]} and card {a5c[0]}** ({a5g[1]} and {a5c[1]} divergent, {a5g[2]} and {a5c[2]} unverified, "
               f"{a5g[3]} and {a5c[3]} uncovered). " +
               (f"M1 and M2 are zero because a routine counts only when every script that reaches it is identical in "
                f"lockstep with its C in place, and with the legacy C in place {a7_ok} of {len(a7_rows)} scripts are "
                "(the epoch-2 checker also judges access order, interrupts inside calls and frames inside calls)."
                if not a7_ok else
                f"{a7_ok} of {len(a7_rows)} scripts are identical in lockstep."))
    filed_n, reworked_n = filed_c_counts()
    dr = drafted_counts()
    filed_line = (f"**Filed C: {filed_n} game routines have filed C; {reworked_n} of them were lifted or reworked since "
                  f"{REWORK_SINCE}** (any line of the routine last changed on or after that date, by `git blame`). "
                  + (f"**Drafted C, a stepping stone and never counted as decompiled: {dr[1]:,} of {dr[0]:,} "
                     f"register-shaped drafts identical in A5** measured {dr[2]} by the runner, the A5 of every draft serving." if dr else ""))
    hc = held_counts()
    held_line = (f"**Filed C holding in lockstep, a line of its own and not part of M1: {hc[1]} routines, {hc[2]:,} B**, "
                 f"in the {len(hc[0])} script(s) that came out identical with the filed C in place and drafts serving "
                 f"for every routine that fails the proper-C check or a defect ({', '.join(hc[0])}). A routine is T3 only "
                 f"when every script that reaches it is identical with its C in place: {hc[3]} routines meet that so far "
                 f"({hc[4]:,} B); the rest wait for the scripts listed in `config/rondo/certification_queue.csv`. "
                 f"Measured at {', '.join('`' + x + '`' for x in hc[5])}.") if hc else ""
    sc = served_counts().get("serve-all")
    served_line = (f"**Drafts in lockstep, the plan's first bar (PLAN section 7) and never counted as decompiled: "
                   f"{sc[0]} of {sc[1]} scripts identical with every routine's draft in place**, measured at "
                   f"{', '.join('`' + x + '`' for x in sc[2])} (`config/rondo/runs/served_a7.csv`: each script's "
                   "first stop and its cause).") if sc else ""
    block = "\n".join([
        START,
        "![Progress against the traced floor of the game's code](docs/rondo/progress.svg)",
        "",
        f"Measured on `{head}`: gate records from {', '.join('`' + g + '`' for g in gates)}, "
        f"every one of the {n_scripts} scripts traced and with a gate A7 result.",
        "",
        f"**Denominator: {denom}.**",
        "",
        *([f"**Code no script runs: {census_code:,} B ({percent(census_code, game_whole)} of the denominator), a "
           f"line of its own** ({cen['counted_never_loaded']:,} B of it in disc areas no script loads). It is "
           "decompiled like any other code. It counts in M1 only when a script runs it and the gates pass it, or when "
           "it is proven unreachable and filed as proper C whose header records the proof (PLAN section 1, the "
           f"rulings of 2026-11-26): {unreach[0]} routines, {unreach[1]:,} B so far"
           + (f"; {contra[0]} routines marked unreachable were reached and fail a gate, and do not count" if contra[0]
              else "") + ".", ""] if CENSUS_COUNTED else []),
        f"**{below[0]} of {len(rows)} routines ({below[1]:,} B) fail the proper-C check: "
        "register-shaped C, a stepping stone, not decompiled.** The check is a pattern check, so "
        "every count above T2 is a ceiling until reviewed.",
        "",
        f"| Measure | Routines | Bytes | Of the {game_whole:,} B |",
        "|---|---:|---:|---:|",
        f"| M2: T4 | {t4[0]} | {t4[1]:,} | {percent(t4[1], game_whole)} |",
        f"| M1: T3 or better | {t3[0] + t4[0]} | {m1_bytes:,} | {percent(m1_bytes, game_whole)} |",
        f"| M1 with proven-unreachable C, optimistic (`@unreachable`, PLAN section 1) | {t3[0] + t4[0] + unreach[0]} "
        f"| {m1_bytes + unreach[1]:,} | {percent(m1_bytes + unreach[1], game_whole)} |",
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
        a5_line,
        "",
        filed_line,
        "",
        *([served_line, ""] if served_line else []),
        held_line,
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
