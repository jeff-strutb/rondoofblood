#!/usr/bin/env python3
"""asmfile.py -- draft a C file's assembly decompilation from the disc.

For every @routine in src/rondo/<area>/<name>.c, decodes its bytes from track
02 and writes asm/rondo/<area>/<name>.s: the same header, then the routine with
every address a name (config/rondo/symbols.csv, the routines' own names, a
label). The draft is then source, edited by hand as names improve; the decode
makes its bytes the disc's by construction, and the whole-image gate proves it.

A routine's span runs from its tag until the next tag on the disc, or the first
instruction that ends flow with no branch inside the span still pending.

    python3 tools/rondo/asmfile.py [src/rondo/math/trig.c ...]
"""
import glob
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import huc  # noqa: E402
import ramnames  # noqa: E402

TOP = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
IMAGE = os.path.join(TOP, "build", "rondo", "disc", "track02.bin")
SECTOR = 2048
TAG = re.compile(r"@routine\s+([0-9A-Fa-f]{4}):([0-9A-Fa-f]{3})\s+(\w+)")
RUN = re.compile(r"@run\s+([0-9A-Fa-f]{2}):([0-9A-Fa-f]{4})")


def routines_in(path):
    """Each routine of a C file: disc offset, name, run address, header lines."""
    text = open(path, encoding="utf-8").read()
    found = []
    for block in re.findall(r"/\*.*?\*/", text, re.S):
        m = TAG.search(block)
        if not m:
            continue
        run = RUN.search(block)
        lines = [re.sub(r"^\s*\*? ?", "", ln).rstrip() for ln in block.split("\n")[1:-1]]
        found.append({"disc": int(m.group(1), 16) * SECTOR + int(m.group(2), 16),
                      "tag": f"{m.group(1)}:{m.group(2)}", "name": m.group(3),
                      "run": int(run.group(2), 16), "header": lines, "path": path})
    return found


def all_routines():
    found = []
    for path in glob.glob(os.path.join(TOP, "src", "rondo", "**", "*.c"), recursive=True):
        found += routines_in(path)
    return sorted(found, key=lambda r: r["disc"])


def span(buf, routine, next_disc):
    """Decode a routine: [(run address, length, name, mode, operands, target)]."""
    out = []
    pc = routine["run"]
    start = routine["disc"]
    pending = pc
    while True:
        off = start + (pc - routine["run"])
        if off >= next_disc:
            return out
        n, name, mode, ops, target = huc.decode(buf, off, pc)
        out.append((pc, n, name, mode, ops, target))
        if name is None:
            raise SystemExit(f"{routine['name']}: undefined opcode at {pc:04X}")
        if mode in ("rel", "zrel") and target is not None and target > pc:
            pending = max(pending, target)
        pc += n
        if name in huc.ENDS and pc > pending:
            return out


class Namer:
    """Names for addresses, from the list and from the routines being filed."""

    def __init__(self, routines, spans):
        self.zp, self.abs = {}, {}
        for addr, kind, name, _ in ramnames.load():
            (self.zp if kind == "zp" else self.abs)[addr] = name
        self.table = sorted(self.abs)
        self.entry = {r["run"]: r["name"] for r in routines}
        for addr, name in list(self.abs.items()):
            self.entry.setdefault(addr, name) if addr >= 0x8000 else None
        self.owner = {}
        for r in routines:
            for ins in spans[r["name"]]:
                self.owner[ins[0]] = r["name"]
        self.labels = {}
        for r in routines:
            for pc, _, _, mode, _, target in spans[r["name"]]:
                if mode in ("rel", "zrel") or target is not None:
                    if target in self.owner and target not in self.entry:
                        self.labels.setdefault(target, self._label(target))

    def _label(self, target):
        return f"{self.owner[target]}_at{sum(1 for t in self.labels if self.owner[t] == self.owner[target])}"

    def zero(self, addr):
        return self.zp.get(addr, f"${addr:02X}")

    def code(self, addr):
        if addr in self.entry:
            return self.entry[addr]
        return self.labels.get(addr) or self.abs.get(addr) or f"${addr:04X}"

    def data(self, addr):
        if addr in self.abs:
            return self.abs[addr]
        if 0x2000 <= addr < 0x2100 and addr - 0x2000 in self.zp:
            return self.zp[addr - 0x2000] + "+$2000"
        lower = [a for a in self.table if a < addr and addr - a <= 8]
        if lower:
            return f"{self.abs[lower[-1]]}+{addr - lower[-1]}"
        return f"${addr:04X}"


def operand(ins, names):
    _, _, name, mode, ops, _ = ins
    if mode in ("imp",):
        return name
    if mode == "acc":
        return f"{name} a"
    if mode == "imm":
        return f"{name} #${ops[0]:02X}"
    z = names.zero
    d = names.data
    forms = {
        "zp": lambda: f"{name} z:{z(ops[0])}",
        "zpx": lambda: f"{name} z:{z(ops[0])},x",
        "zpy": lambda: f"{name} z:{z(ops[0])},y",
        "izp": lambda: f"{name} ({z(ops[0])})",
        "izx": lambda: f"{name} ({z(ops[0])},x)",
        "izy": lambda: f"{name} ({z(ops[0])}),y",
        "abx": lambda: f"{name} a:{d(ops[0])},x",
        "aby": lambda: f"{name} a:{d(ops[0])},y",
        "ind": lambda: f"{name} ({d(ops[0])})",
        "iax": lambda: f"{name} ({d(ops[0])},x)",
        "rel": lambda: f"{name} {names.code(ops[0])}",
        "zrel": lambda: f"{name} {z(ops[0])},{names.code(ops[1])}",
        "blk": lambda: f"{name} {d(ops[0])},{d(ops[1])},${ops[2]:04X}",
        "tzp": lambda: f"{name} #${ops[0]:02X},z:{z(ops[1])}",
        "tzx": lambda: f"{name} #${ops[0]:02X},z:{z(ops[1])},x",
        "tab": lambda: f"{name} #${ops[0]:02X},a:{d(ops[1])}",
        "tax": lambda: f"{name} #${ops[0]:02X},a:{d(ops[1])},x",
    }
    if mode == "abs":
        if name in ("jsr", "jmp"):
            return f"{name} {names.code(ops[0])}"
        return f"{name} a:{d(ops[0])}"
    return forms[mode]()


def main():
    wanted = [os.path.abspath(p) for p in sys.argv[1:]]
    buf = open(IMAGE, "rb").read()
    routines = all_routines()
    spans = {}
    for i, r in enumerate(routines):
        nxt = routines[i + 1]["disc"] if i + 1 < len(routines) else len(buf)
        spans[r["name"]] = span(buf, r, nxt)
    names = Namer(routines, spans)
    by_file = {}
    for r in routines:
        by_file.setdefault(r["path"], []).append(r)
    for path, group in sorted(by_file.items()):
        if wanted and path not in wanted:
            continue
        rel = os.path.relpath(path, os.path.join(TOP, "src", "rondo"))
        out_path = os.path.join(TOP, "asm", "rondo", os.path.splitext(rel)[0] + ".s")
        os.makedirs(os.path.dirname(out_path), exist_ok=True)
        out = []
        for r in group:
            out += [";" + (" " + ln if ln else "") for ln in r["header"]]
            out.append(f"{r['name']}:")
            for ins in spans[r["name"]]:
                if ins[0] in names.labels:
                    out.append(f"{names.labels[ins[0]]}:")
                out.append("    " + operand(ins, names))
            out.append("")
        with open(out_path, "w", encoding="ascii", newline="\n") as f:
            f.write("\n".join(out))
    bytes_filed = sum(ins[1] for s in spans.values() for ins in s)
    print(f"{len(routines)} routines, {bytes_filed} B")


if __name__ == "__main__":
    main()
