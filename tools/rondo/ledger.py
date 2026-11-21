#!/usr/bin/env python3
"""ledger.py -- what tier each routine has earned, worked out, never entered by hand.

    python3 tools/rondo/ledger.py
    python3 tools/rondo/ledger.py --card    the System Card's ledger only
    python3 tools/rondo/ledger.py [--chash FILE] [--partial]

Writes docs/rondo/ledger.csv and prints the counts, below-standard first. For each routine:

  proper_c  no flag state or registers in its body or the helpers it reaches: no Alu, adc8,
            sbc8, alu_*, call_with, call_abi or cpu in them, and no goto. abi_ wrappers and
            *_shim functions may have registers, and a wrapper may not hold the logic.
            A machine check: it cannot see C that is assembly-shaped in other ways.
  gate_0    its bytes in the image built from the assembly are the disc's.
  symbolic  no bare address among its assembly's operands.
  gate_c    every byte of it ran in the traces.
  gate_a5   A5 judged calls of it and none diverged.
  gate_a7   A7 swapped it in, or its C ran inside a call A7 swapped in, in a run of every script
            with every frame identical (approved by the maintainer in chat, 2026-11-16).

A5 and A7 are read from the records runner.py writes (config/rondo/runs/, records.py). A record
counts while the routine's compiled C (compiled.py) is the one it was measured with and the script's
text is unchanged; a checker or harness change voids nothing (the exact rule is in records.py). A7
credits a routine when C ran in some counting script's run and every script that reaches the routine
has a counting, identical A7 result. With any script unmeasured the ledger is not written
(--partial writes it anyway, saying so). The compiled hashes come from --chash FILE (compiled.py's
output, as the runner passes it) or are worked out from this checkout. A waiver is a line in
config/rondo/ledger_waivers.csv (routine,gate,reason), which only the maintainer writes.

T2 C and assembly filed. T3 proper_c, gate_0, gate_c, gate_a5 and gate_a7. T4 T3 and symbolic.
It reads the gate records, the traces and the image, and runs nothing but git.
"""
import csv
import glob
import hashlib
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import asmfile  # noqa: E402
import cardfile  # noqa: E402
import compiled  # noqa: E402
import cpuguard  # noqa: E402
import oracle  # noqa: E402
import records  # noqa: E402

BUILD = os.path.join(TOP, "build", "rondo")
CONFIG = os.path.join(TOP, "config", "rondo")
OUT = os.path.join(TOP, "docs", "rondo", "ledger.csv")
CARD_OUT = os.path.join(TOP, "syscard", "ledger.csv")
WAIVERS = os.path.join(TOP, "config", "rondo", "ledger_waivers.csv")
FUNC = re.compile(r"^(?:static\s+)?(?:inline\s+)?[\w\s\*]+?\b(\w+)\s*\(([^;{}]*)\)\s*\n\{", re.M)
CALL = re.compile(r"\b(\w+)\s*\(")
BAD = (("cpu", re.compile(r"\bcpu\.")), ("alu helper", re.compile(r"\balu_\w+\s*\(")),
       ("call_with", re.compile(r"\bcall_with\s*\(")), ("call_abi", re.compile(r"\bcall_abi\s*\(")),
       ("adc8/sbc8", re.compile(r"\b(adc8|sbc8|adc8_bcd)\s*\(")), ("Alu", re.compile(r"\bAlu\b")),
       ("goto", re.compile(r"\bgoto\b")))
WRAPPER_MAX_LINES = 14


def functions():
    """Every C function: its signature and body, comments out, by name."""
    found = {}
    for path in (glob.glob(os.path.join(TOP, "src", "rondo", "**", "*.c"), recursive=True) +
                 glob.glob(os.path.join(TOP, "syscard", "src", "*.c"))):
        text = re.sub(r"/\*.*?\*/", "", open(path, encoding="utf-8").read(), flags=re.S)
        for m in FUNC.finditer(text):
            depth, i = 1, m.end()
            while depth and i < len(text):
                depth += {"{": 1, "}": -1}.get(text[i], 0)
                i += 1
            found[m.group(1)] = text[m.start():i]
    return found


def proper_c_faults(name, funcs):
    """What keeps a routine from proper C, through the plain helpers it reaches."""
    faults = set()
    wrapper = funcs.get("abi_" + name, "")
    if name not in funcs and wrapper.count("\n") > WRAPPER_MAX_LINES:
        faults.add("logic in the wrapper")
    work, seen = [name], set()
    for callee in CALL.findall(wrapper):
        if callee in funcs and not callee.startswith("abi_") and not callee.endswith("_shim"):
            work.append(callee)
    while work:
        current = work.pop()
        if current in seen or current not in funcs:
            continue
        seen.add(current)
        for label, pattern in BAD:
            if pattern.search(funcs[current]):
                faults.add(label)
        for callee in CALL.findall(funcs[current]):
            if not callee.startswith("abi_") and not callee.endswith("_shim"):
                work.append(callee)
    return sorted(faults)


def a7_passed(entry):
    """C of the routine ran in a counting script's lockstep run, and every script reaching it passes."""
    return bool(entry) and entry[0] > 0 and not entry[1]


STATE = {}


def load_records(chash_file):
    """Compiled hashes and the stored records, once: the records.py counts every gate reads."""
    if chash_file:
        chash, ext = compiled.load(chash_file)
    else:
        subprocess.run([sys.executable, "-I", os.path.join(HERE, "hybrid.py")], check=True, capture_output=True)
        chash, ext = compiled.compute(TOP)
    rec = records.Records()
    STATE.update(chash=chash, ext=ext, rec=rec, a5=records.a5_counts(rec, chash),
                 a7=records.a7_routines(rec, chash, ext))


def a5_records(card):
    return {k: v for k, v in STATE["a5"].items() if int(STATE["ext"].get(k, (0, 0, 0, 0))[3]) == card}


def a7_records():
    """{routine: (scripts it ran in, [scripts reaching it that do not pass])}."""
    return STATE["a7"]


def read_csv(path, key):
    if not os.path.exists(path):
        return {}
    return {row[key]: row for row in csv.DictReader(open(path, newline=""))}


def image_fresh(image):
    """The image must be newer than the assembly and the names it was built from."""
    if not os.path.exists(image):
        return False
    built = os.path.getmtime(image)
    sources = glob.glob(os.path.join(TOP, "asm", "rondo", "**", "*"), recursive=True)
    sources.append(os.path.join(TOP, "config", "rondo", "symbols.csv"))
    return all(os.path.getmtime(p) <= built for p in sources if os.path.isfile(p))


def assembly_blocks(pattern=("asm", "rondo", "**", "*.s")):
    """Each routine's lines of assembly, comments out."""
    blocks = {}
    for path in glob.glob(os.path.join(TOP, *pattern), recursive=True):
        name = None
        for line in open(path, encoding="ascii"):
            label = re.match(r"^(\w+):\s*$", line)
            if label and not re.search(r"_at\d+$", label.group(1)):
                name = label.group(1)
                blocks[name] = []
            elif name and not line.startswith(";"):
                blocks[name].append(line.split(";")[0])
    return blocks


def waivers():
    if not os.path.exists(WAIVERS):
        return set()
    return {(row["routine"], row["gate"]) for row in csv.DictReader(open(WAIVERS, newline=""))}


def tier_of(gates, filed):
    t3 = all(gates[g] for g in ("proper_c", "gate_0", "gate_c", "gate_a5", "gate_a7"))
    return "T4" if t3 and gates["symbolic"] else "T3" if t3 else "T2" if filed else ""


def card_image_fresh(image):
    """The card's image must be newer than its assembly and its names."""
    if not os.path.exists(image):
        return False
    built = os.path.getmtime(image)
    sources = glob.glob(os.path.join(TOP, "syscard", "asm", "**", "*"), recursive=True)
    sources.append(os.path.join(TOP, "syscard", "config", "symbols.csv"))
    return all(os.path.getmtime(p) <= built for p in sources if os.path.isfile(p))


def card_ledger(funcs, a7):
    """The System Card's routines, held to the same gates, in a ledger of its own."""
    rom = open(cardfile.IMAGE, "rb").read()
    image_path = os.path.join(BUILD, "card", "image.bin")
    image = open(image_path, "rb").read() if card_image_fresh(image_path) else None
    blocks = assembly_blocks(("syscard", "asm", "*.s"))
    a5 = a5_records(1)
    cover = cardfile.coverage()
    rows = []
    for r in cardfile.spans():
        name, n = r["name"], r["length"]
        faults = proper_c_faults(name, funcs)
        row5 = a5.get(name, {})
        gates = {
            "proper_c": not faults,
            "gate_0": image is not None and image[r["disc"]:r["disc"] + n] == rom[r["disc"]:r["disc"] + n],
            "symbolic": name in blocks and not any(
                re.search(r"(?<!#)\$[0-9A-Fa-f]{2,4}", line) for line in blocks[name]),
            "gate_c": cover.get(name, (0, 0))[0] > 0 and cover[name][0] == cover[name][1],
            "gate_a5": int(row5.get("identical", 0)) > 0 and int(row5.get("divergent", 1)) == 0,
            "gate_a7": a7_passed(a7.get(name)),
        }
        rows.append({"routine": name, "where": r["tag"], "bytes": n,
                     "tier": tier_of(gates, name in blocks), **{g: int(v) for g, v in gates.items()},
                     "unverified_calls": row5.get("unverified", ""), "faults": "; ".join(faults)})
    with open(CARD_OUT, "w", newline="\n", encoding="ascii") as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]), lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)
    for tier in ("T4", "T3", "T2", ""):
        chosen = [row for row in rows if row["tier"] == tier]
        print(f"card {tier or 'no tier'}: {len(chosen)} routines, {sum(row['bytes'] for row in chosen)} B")


def main():
    cpuguard.require("ledger.py", jobs=1, mem="3G")
    args = sys.argv[1:]
    load_records(args[args.index("--chash") + 1] if "--chash" in args else None)
    missing = records.complete(STATE["rec"])
    if missing and "--partial" not in args:
        print(f"ledger: not written, no result for {len(missing)} script(s): {' '.join(missing)}\n"
              "  run the runner (runner.py) to measure them, or --partial to write without them")
        return 1
    if missing:
        print(f"PARTIAL: no result for {len(missing)} script(s): {' '.join(missing)}")
    if "--card" in args:
        card_ledger(functions(), a7_records())
        return 0
    buf = open(oracle.TRACK, "rb").read()
    image_path = os.path.join(BUILD, "asm", "image.bin")
    image = open(image_path, "rb").read() if image_fresh(image_path) else None
    funcs = functions()
    blocks = assembly_blocks()
    a5 = a5_records(0)
    a7 = a7_records()
    cover = oracle.coverage()
    waived = waivers()
    routines = asmfile.all_routines()
    rows = []
    for i, r in enumerate(routines):
        name = r["name"]
        nxt = routines[i + 1]["disc"] if i + 1 < len(routines) else len(buf)
        n = sum(ins[1] for ins in asmfile.span(buf, r, nxt))
        faults = proper_c_faults(name, funcs)
        row5 = a5.get(name, {})
        gates = {
            "proper_c": not faults,
            "gate_0": image is not None and image[r["disc"]:r["disc"] + n] == buf[r["disc"]:r["disc"] + n],
            "symbolic": name in blocks and not any(
                re.search(r"(?<!#)\$[0-9A-Fa-f]{2,4}", line) for line in blocks[name]),
            "gate_c": cover.get(name, (0, 0))[0] > 0 and cover[name][0] == cover[name][1],
            "gate_a5": int(row5.get("identical", 0)) > 0 and int(row5.get("divergent", 1)) == 0,
            "gate_a7": a7_passed(a7.get(name)),
        }
        for gate in gates:
            if (name, gate) in waived:
                gates[gate] = True
        tier = tier_of(gates, name in blocks)
        c_text = funcs.get(name, "") + funcs.get("abi_" + name, "")
        rows.append({
            "routine": name, "where": r["tag"], "bytes": n, "tier": tier,
            **{g: int(v) for g, v in gates.items()},
            "unverified_calls": row5.get("unverified", ""),
            "faults": "; ".join(faults),
            "c_hash": hashlib.sha256(c_text.encode()).hexdigest()[:12],
        })
    with open(OUT, "w", newline="\n", encoding="ascii") as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]), lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)

    def tally(test):
        chosen = [row for row in rows if test(row)]
        return f"{len(chosen)} routines, {sum(row['bytes'] for row in chosen)} B"

    print("below standard:", tally(lambda row: not row["proper_c"]), "fail the proper-C check")
    for gate in ("gate_0", "gate_c", "gate_a5", "gate_a7", "symbolic"):
        print(f"  not {gate}:", tally(lambda row, g=gate: not row[g]))
    for tier in ("T4", "T3", "T2", ""):
        print(f"{tier or 'no tier'}:", tally(lambda row, t=tier: row["tier"] == t))
    print("A5 routines counted:", len(a5), "; A7 routines counted:", len(a7),
          "; image counted:", image is not None, "; waivers:", len(waived))
    card_ledger(funcs, a7)
    return 0


if __name__ == "__main__":
    sys.exit(main())
