#!/usr/bin/env python3
"""liftcmp.py -- the lift harness: a filed routine's C against its draft, access for access.

    python3 tools/rondo/liftcmp.py NAME [NAME ...] [--drafts DIR] [--cases N] [--seed S] [--curated FILE]
                                   [--lifted FILE] [--ignore-flags NVDIZC] [--report DIR]

NAME is a filed routine (its `abi_NAME` in src/rondo). The DRAFT is the proven side: the register-shaped C
tools/rondo/cdraft.py wrote for that routine, taken from the draft cache (cdraft.fetch: build/rondo/drafts of the main
checkout when its stamp matches, else this tree's own; a filed name is resolved from the tree's tags, so a routine filed a
minute ago needs no fetch step). --drafts DIR names a folder with drafts.c and drafts.csv instead. The C in the tree is the new side. Both are entered
the same way, through the register file with the same registers and flags, in a machine that logs everything
(tools/rondo/liftcmp_rt.c), and for each case the whole log is compared: every read and write in order with its
address and value, the VDC, MPR and clock instructions, every call the routine makes (the frame it pushes, the
callee and the registers it is entered with) and the registers and flags it leaves.

Cases. N seeded cases (default 400): entry A, X, Y, S and flags from edge-biased values; a memory image whose
unwritten bytes read as a value fixed by the seed and the address, in one of three flavours (any byte, edge values,
small values); a byte written reads back, and a second read of a byte nobody wrote gives the same value, as memory does.
Hardware is what the checker counts as hardware: an address whose MPR holds the I/O bank ($FF), normally $0000 to
$1FFF; there a second read can differ, so a port's value also depends on how often it has been read, and a write to a
port is not read back. Everything else (work RAM and the stack at $2000 to $3FFF, the banks mapped at $4000 up) is memory. The curated file adds cases a
writer wants, one a line: `a=3 x=5 y=0 s=F0 p=NZC mem:2B70=80 mem:3040+5=01` (hex; p a set of flag letters or a
hex byte; +N is an offset from the address); they run first and are named by line.

Other routines. By default (--callees through) a call is compared through: the draft side runs the callee's DRAFT, its
real accesses, and the lifted side runs the callee's C as the caller calls it (the typed function between the frame
helpers, or its abi_ wrapper where the caller calls that), and the whole sequence including the callee's is compared, so
a callee that omits or reorders an access shows at its caller. With --callees wrappers both sides run the callee's filed
abi_ wrapper: the callee is held equal and the routine is compared alone. A call into code with no C is a stub that
returns seeded A, X, Y and flags and logs the call, on both sides in either mode.
A draft that reaches bytes no script ran ends its case (UNTRACED): that case proves nothing and is counted apart.

Coverage. Both sides are built with the compiler's block probes; the report says which lines of the lifted file and
of the draft no case reached, and for each line reached by three cases or fewer, which. A line no case reached is an
arm that is not proven: add a curated case for it.

The entry contract. The cases assume what a routine's header says of its entry, not what the chip could be in: D clear
(unless the routine's own assembly sets it, then it is set in the draft too), I clear, T and B clear, S in $90 to $F8,
the banks as the trace saw the routine run (every mapper configuration it ran under; none traced: MPR2-3 $69/$6A, MPR4-6 $00; MPR0 $FF, MPR1 $F8, MPR7 $00), X below $58 in half the cases (an object or channel index), and stubs return N, V, Z and C only. `--decimal` runs decimal
cases for a routine entered with D set or calling something that returns it set; `--irq` sets I in half the cases;
the result line says which was run. Gate A5 judges the contract on real calls: a difference found here under a contract
the game never meets is not a defect, and one the gate finds under real entry is not made up by passing here.

What it does not do: it runs no emulator and no interrupt; a routine that waits is judged up to its wait; timing
is not modelled. It is a check on a lift, not a replacement for the gates.

It takes under a second per routine once the game's C is compiled (the objects are cached under
build/rondo/liftcmp, keyed by the source and the headers); the first run compiles every file, a few tens of seconds
on one core, and goes through `cpu-run -j N -m 2G -- ... --warm`. Comparing a routine needs no slot: it is a short
check of bounded memory.
Exit 0 when every compared case matches and at least one was compared; 1 when any differs; 2 on a setup error.
"""
import argparse
import concurrent.futures
import csv
import hashlib
import os
import random
import re
import struct
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
BUILD = os.path.join(TOP, "build", "rondo", "liftcmp")
CFLAGS = ["-O1", "-g1", "-w", "-fno-pie", "-fsanitize-coverage=trace-pc"]
FLAGBITS = {"N": 0x80, "V": 0x40, "T": 0x20, "B": 0x10, "D": 0x08, "I": 0x04, "Z": 0x02, "C": 0x01}


def run(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, **kw)


def main_checkout():
    r = run(["git", "-C", TOP, "rev-parse", "--path-format=absolute", "--git-common-dir"])
    return os.path.dirname(r.stdout.strip()) if r.returncode == 0 else TOP


def find_drafts(given):
    """The folders to look in: the one given (a drafts.c with drafts.csv, the older form), else None for the draft cache."""
    if given:
        if not (os.path.exists(os.path.join(given, "drafts.csv")) and os.path.exists(os.path.join(given, "drafts.c"))):
            raise SystemExit(f"liftcmp: {given} has no drafts.c and drafts.csv")
        return [given]
    return []


def find_ops(drafts_dir, given):
    inc = given or os.path.join(TOP, "src", "rondo", "include")
    if not os.path.exists(os.path.join(inc, "ops6280.h")):
        raise SystemExit("liftcmp: ops6280.h not found in %s; pass --ops DIR" % inc)
    return inc


def cache_draft(name, run_addr):
    """(row, body, count) for a filed routine from the drafter's cache (tools/rondo/cdraft.py), or None.

    A filed name the drafter's own lookup knows (an asm routine) is fetched by name. Any other, a routine only the C
    knows or a copy of a content filed at another disc location, is found through the cache's members.csv: the
    draft that serves this disc location at this run address (else at any run address, the first)."""
    sys.path.insert(0, HERE)
    try:
        import cdraft
    except Exception as e:
        raise SystemExit(f"liftcmp: cannot import cdraft.py: {e}")
    label = name
    try:
        text = cdraft.fetch(name, as_filed=True)
    except KeyError:
        disc, tag_run = routine_tag(name)
        cache = cdraft.current_cache()
        if disc is None or not cache:
            return None
        members = [r for r in csv.DictReader(open(os.path.join(cache, "members.csv"))) if r["space"] == "D" and r["offset"] == str(disc)]
        same = [r for r in members if tag_run is not None and int(r["run"], 16) == tag_run]
        pick = (same or members or [None])[0]
        if pick is None:
            return None
        label = pick["name"]
        text = cdraft.fetch(label, as_filed=True)
    except LookupError as e:
        raise SystemExit(f"liftcmp: {e}")
    chunks = re.split(r"(?m)^(?=/\* \w+: run )", text)
    chunks = [c for c in chunks if c.startswith("/* ") and ("void abi_%s(void)" % label) in c]
    if not chunks:
        return None
    pick = chunks[0]
    for c in chunks:
        m = re.match(r"/\* (\w+): run ([0-9A-F]+)", c)
        if m and run_addr is not None and int(m.group(2), 16) == run_addr:
            pick = c
    m = re.match(r"/\* (\w+): run ([0-9A-F]+)", pick)
    return dict(name=m.group(1), run=m.group(2)), pick.replace("void abi_%s(void)" % label, "void draft_entry(void)", 1), len(chunks)


def hybrid_rows():
    """The routine table of this tree (hybrid.py, 0.1 s): regenerated here when missing or older than any C file."""
    p = os.path.join(TOP, "build", "rondo", "pcebox", "hybrid_table.c")
    newest = 0
    for dp, _, fs in os.walk(os.path.join(TOP, "src")):
        newest = max([newest] + [os.path.getmtime(os.path.join(dp, f)) for f in fs if f.endswith((".c", ".h"))])
    for dp, _, fs in os.walk(os.path.join(TOP, "syscard", "src")):
        newest = max([newest] + [os.path.getmtime(os.path.join(dp, f)) for f in fs if f.endswith(".c")])
    if not os.path.exists(p) or os.path.getmtime(p) < newest:
        r = run([sys.executable, "-I", os.path.join(HERE, "hybrid.py")], cwd=TOP)
        if r.returncode:
            raise SystemExit(f"liftcmp: hybrid.py failed:\n{(r.stdout + r.stderr)[-600:]}")
    return re.findall(r'\{ 0x([0-9A-F]+), (\d+), (\d+), "(\w+)", (abi_\w+), \d+, 0x[0-9A-F]+, (\d) \},', open(p).read())


def header_hash():
    h = hashlib.sha1()
    for d in (os.path.join(TOP, "src", "rondo", "include"), os.path.join(TOP, "syscard", "include")):
        for f in sorted(os.listdir(d)):
            h.update(open(os.path.join(d, f), "rb").read())
    return h.hexdigest()


def compile_obj(path, content_override, cache, hh, extra=()):
    """The cached object of one C file (its text, or the override's), compiled with the probes."""
    text = open(content_override or path, "rb").read()
    key = hashlib.sha1(text + hh.encode() + " ".join(CFLAGS + list(extra)).encode()).hexdigest()[:16]
    obj = os.path.join(cache, os.path.relpath(path, TOP).replace(os.sep, "__") + "." + key + ".o")
    if not os.path.exists(obj):
        src = path
        if content_override:
            src = os.path.join(cache, "ovr_" + key + "_" + os.path.basename(path))
            open(src, "wb").write(text)
        r = run(["cc", *CFLAGS, *extra, "-I", os.path.join(TOP, "src", "rondo", "include"),
                 "-I", os.path.join(TOP, "syscard", "include"), "-I", os.path.dirname(path), "-c", src, "-o", obj])
        if r.returncode:
            raise SystemExit(f"liftcmp: {os.path.relpath(path, TOP)} does not compile:\n{r.stderr[-1500:]}")
    return obj


def game_sources():
    out = []
    root = os.path.join(TOP, "src", "rondo")
    for d in sorted(os.listdir(root)):
        p = os.path.join(root, d)
        if os.path.isdir(p) and d not in ("include", "drafts"):
            out += [os.path.join(p, f) for f in sorted(os.listdir(p)) if f.endswith(".c")]
    return out


def card_sources():
    root = os.path.join(TOP, "syscard", "src")
    return [os.path.join(root, f) for f in sorted(os.listdir(root)) if f.endswith(".c")] if os.path.isdir(root) else []


def find_source(name, sources):
    pat = re.compile(r"^void abi_%s\(void\)\s*$" % re.escape(name), re.M)
    for s in sources:
        if pat.search(open(s).read()):
            return s
    raise SystemExit(f"liftcmp: no `void abi_{name}(void)` in src/rondo")


def routine_tag(name):
    """(disc byte offset, run address) of a filed routine from its `@routine SSSS:OOO NAME` and `@run BB:AAAA` tags."""
    pat = re.compile(r"@routine\s+([0-9A-Fa-f]+):([0-9A-Fa-f]+)\s+%s\b[^\n]*\n(?:[^\n]*\n){0,2}?[^\n]*@run\s+[0-9A-Fa-f]+:([0-9A-Fa-f]+)" % re.escape(name))
    roots = [os.path.join(TOP, "src", "rondo"), os.path.join(TOP, "syscard", "src")]
    for root in roots:
        for dp, _, fs in os.walk(root):
            for f in fs:
                if f.endswith(".c"):
                    m = pat.search(open(os.path.join(dp, f)).read())
                    if m:
                        return int(m.group(1), 16) * 2048 + int(m.group(2), 16), int(m.group(3), 16)
    return None, None


def extract_draft(drafts_dir, name):
    allrows = list(csv.DictReader(open(os.path.join(drafts_dir, "drafts.csv"))))
    rows = [r for r in allrows if r.get("filed") == name]
    how = "by its filed name"
    if not rows:
        disc, run_addr = routine_tag(name)
        if disc is not None:
            rows = [r for r in allrows if r.get("disc") == str(disc) and not int(r.get("rom") or 0)]
            if len(rows) > 1:
                same = [r for r in rows if int(r["run"], 16) == run_addr]
                rows = same or rows
            how = "by its disc location %d (@routine tag)" % disc
    if not rows:
        return None
    row = rows[0]
    text = open(os.path.join(drafts_dir, "drafts.c")).read()
    m = re.search(r"^/\* %s: run .*?(?=^/\* \w+: run |\Z)" % re.escape(row["name"]), text, re.S | re.M)
    if not m:
        raise SystemExit(f"liftcmp: draft {row['name']} not found in drafts.c")
    body = m.group(0).replace("abi_" + row["name"], "draft_entry")
    return row, body, len(rows)


def parse_curated(path):
    cases = []
    if not path:
        return cases
    for n, line in enumerate(open(path), 1):
        line = line.split("#")[0].strip()
        if not line or line.startswith("@seed"):
            continue
        c = dict(a=0, x=0, y=0, s=0xF0, p=0, mem=[], label=f"{os.path.basename(path)}:{n}")
        for tok in line.split():
            if tok.startswith("mem:"):
                at, _, val = tok[4:].partition("=")
                base, _, off = at.partition("+")
                c["mem"].append(((int(base, 16) + (int(off, 16) if off else 0)) & 0xFFFF, int(val, 16)))
            else:
                k, _, v = tok.partition("=")
                if k == "p" and re.fullmatch(r"[NVDIZC]*", v):
                    c["p"] = sum(FLAGBITS[ch] for ch in v)
                elif k in c:
                    c[k] = int(v, 16)
                else:
                    raise SystemExit(f"liftcmp: {path}:{n}: cannot read {tok!r}")
        cases.append(c)
    return cases


def parse_seed_fixed(path):
    """The registers a curated file fixes in the seeded cases: a line '@seed y=03' (the entry contract of the routines it covers)."""
    fixed = {}
    if path:
        for line in open(path):
            line = line.split("#")[0].strip()
            if line.startswith("@seed"):
                for tok in line.split()[1:]:
                    k, _, v = tok.partition("=")
                    if k not in ("a", "x", "y"):
                        raise SystemExit(f"liftcmp: {path}: cannot fix {tok!r} in seeded cases")
                    fixed[k] = int(v, 16)
    return fixed


DEFAULT_MAP = ("69", "6A", "00", "00", "00")


_RUNMAP = {}


def load_runmap():
    """(the trace folder's {config id: (mpr2..mpr6)}, [(disc offset, logical address, config id)]), read once."""
    if "v" in _RUNMAP:
        return _RUNMAP["v"]
    for base in (TOP, main_checkout()):
        d = os.path.join(base, "build", "rondo", "trace")
        if os.path.exists(os.path.join(d, "runmap.bin")) and os.path.exists(os.path.join(d, "mprcfg.csv")):
            break
    else:
        _RUNMAP["v"] = ({}, [])
        return _RUNMAP["v"]
    cfgs = {}
    for line in open(os.path.join(d, "mprcfg.csv")).read().splitlines()[1:]:
        f = line.split(",")
        cfgs[int(f[0])] = tuple(f[1:6])
    data = open(os.path.join(d, "runmap.bin"), "rb").read()
    recs = [(disc_off, logical, cfg) for disc_off, logical, bank, kind, cfg, _ in struct.iter_unpack("<IHBBHH", data)]
    _RUNMAP["v"] = (cfgs, recs)
    return _RUNMAP["v"]


def run_configs(name, rows_full):
    """The mapper configurations the routine ran under, from the trace's run map (runmap.bin, mprcfg.csv):
    [(mpr2..mpr6 as hex strings)], or None when no script ran it."""
    row = [r for r in rows_full if r[3] == name]
    if not row:
        return None
    run_addr, length, disc = int(row[0][0], 16), int(row[0][1]), int(row[0][2])
    cfgs, recs = load_runmap()
    found = []
    for disc_off, logical, cfg in recs:
        if disc <= disc_off < disc + length and run_addr <= logical < run_addr + length and cfg in cfgs and cfgs[cfg] not in found:
            found.append(cfgs[cfg])
    return found or None


def resident_entries(configs, rows, run_set=None, near=None):
    """[(mpr tuple, run address, disc offset)]: for each configuration the run map shows, what is resident at each run
    address a call can be made to (the entry of a routine or a piece of the cache): the disc offset of the byte there.
    The mapper configuration alone does not say which overlay is loaded: the same banks hold one stage's overlay in one
    script and another's in the next, so the run map has several disc offsets for one (configuration, address). The
    callee is the one from the same overlay as the routine under test: the candidate nearest to its own disc place
    (`near`). A configuration that has other code at the address than any routine or piece we know makes the call a stub,
    never another overlay's routine at the same run address."""
    cfgs, recs = load_runmap()
    wanted = {tuple(c) for c in (configs or [DEFAULT_MAP])}
    runs = run_set if run_set is not None else {int(r[0], 16) for r in rows}
    ids = {cid for cid, t in cfgs.items() if t in wanted}
    cands = {}
    bases = {}
    for disc_off, logical, cfg in recs:
        if cfg in ids:
            # one overlay copy fills an 8 KB window from disc: its bytes share a base (disc offset less offset in the window)
            if logical in runs:
                bases.setdefault((cfg, logical >> 13), {}).setdefault(disc_off - (logical & 0x1FFF), set()).add(logical)
                cands.setdefault((cfg, logical), set()).add(disc_off)
    # The copy is chosen once for each window of a configuration, not once for each address: picking the nearest copy
    # address by address mixed overlays (the vector at $8006 from one stage's copy, the routine at $9991 from another's).
    # The copy nearest to the routine under test wins (its own overlay); the one with the most of the addresses breaks a tie.
    chosen = {}
    for key, bs in bases.items():
        rank = (lambda b: (abs(b - near), -len(bs[b]), b)) if near is not None else (lambda b: (-len(bs[b]), b))
        chosen[key] = min(bs, key=rank)
    out = []
    for (cfg, logical), ds in sorted(cands.items()):
        d = chosen[(cfg, logical >> 13)] + (logical & 0x1FFF)
        if d in ds:
            out.append((cfgs[cfg], logical, d))
        # else: the chosen copy has no code the traces ran at this address: nothing known is resident (a stub), never
        # another overlay's routine at the same run address
    return out


def make_cases(curated, n, seed, decimal=False, irq=False, configs=None, fixed=None):
    rng = random.Random(seed)
    configs = configs or [DEFAULT_MAP]
    cases = []
    for c in curated:
        for k, cfg in enumerate(configs):
            cases.append(dict(c, mpr=cfg))
    per = max(100, n // len(configs))
    edge = [0, 1, 2, 0x7F, 0x80, 0xFF, 0xFE, 0x0F, 0xF0]
    for k in range(per * len(configs)):
        cfg = configs[k % len(configs)]

        def r8():
            return rng.choice(edge) if rng.random() < 0.5 else rng.randrange(256)
        # N, V, Z, C as the entry contract has them; D and I only when asked for (see the header)
        p = rng.choice([0, 1, 2, 3, 0x40, 0x80, 0x82, 0xC3]) | (rng.getrandbits(2) & 0x03)
        if decimal and rng.random() < 0.5:
            p |= 0x08
        if irq and rng.random() < 0.5:
            p |= 0x04
        cases.append(dict(a=r8(), x=r8() if rng.random() < 0.5 else rng.randrange(0, 0x58), y=r8(), s=rng.randrange(0x90, 0xF9),
                          p=p & 0xCF, mem=[], label="seed", mpr=cfg))
        cases[-1].update(fixed or {})
    with_seed = []
    for i, c in enumerate(cases):
        c["seed"] = rng.getrandbits(48)
        c["id"] = i
        with_seed.append(c)
    return with_seed


def write_cases(cases, path):
    with open(path, "w") as f:
        for c in cases:
            f.write("%X %X %X %X %X %X %X %s %d" % (c["id"], c["seed"], c["a"], c["x"], c["y"], c["s"], c["p"],
                                                      " ".join(c["mpr"]), len(c["mem"])))
            for a, v in c["mem"]:
                f.write(" %04X=%02X" % (a, v))
            f.write("\n")


def nm_names(objs):
    """(defined, undefined) symbol names over object files."""
    defined, undefined = set(), set()
    for o in objs:
        for line in run(["nm", "-g", o]).stdout.splitlines():
            parts = line.split()
            if len(parts) == 3 and parts[1] in "TtWDdBbRrVv":
                defined.add(parts[2])
            elif len(parts) == 2 and parts[0] == "U":
                undefined.add(parts[1])
    return defined, undefined


def warm_check(game_objs, cache):
    """Links the game's C and the draft library with the runtime, every symbol required, so a helper they call that
    the runtime lacks fails `--warm` by name (not a run of every routine later)."""
    work = os.path.join(cache, "warm")
    os.makedirs(work, exist_ok=True)
    rt = os.path.join(work, "rt.o")
    r = run(["cc", "-O1", "-w", "-fno-pie", "-DENTRY=draft_entry", "-I", os.path.join(TOP, "src", "rondo", "include"),
             "-c", os.path.join(HERE, "liftcmp_rt.c"), "-o", rt])
    if r.returncode:
        raise SystemExit(f"liftcmp: the runtime does not compile:\n{r.stderr[-800:]}")
    stub = os.path.join(work, "stub.c")
    defined, undefined = nm_names(game_objs)
    libs = []
    try:
        sys.path.insert(0, HERE)
        import cdraft
        if cdraft.current_cache():
            libo, _, libsyms = draft_library([], cache)
            libs = [libo]
            d2, u2 = nm_names(libs)
            defined |= d2
            undefined |= u2
    except SystemExit:
        raise
    except Exception as e:
        print(f"liftcmp: the drafts were not checked against the runtime: {e}")
    # abi_ routines with no C, the draft entry and the dispatch tables, as a run defines them
    with open(stub, "w") as f:
        f.write("#include <stdint.h>\ntypedef struct { uint16_t run; void (*abi)(void); } Dispatch;\n"
                "const Dispatch dispatch_table[] = { { 0, 0 } }; const unsigned dispatch_count = 0;\n"
                "typedef struct { uint8_t m[5]; uint16_t run; uint32_t disc; } Resident;\n"
                "const Resident resident_tab[] = { { { 0 }, 0, 0 } }; const unsigned resident_count = 0;\n"
                "typedef struct { uint32_t disc; void (*abi)(void); } DiscDispatch;\n"
                "const DiscDispatch disc_table[] = { { 0, 0 } }; const unsigned disc_count = 0;\n"
                "typedef struct { uint32_t disc; uint16_t run; void (*abi)(void); } PieceDispatch;\n"
                "const PieceDispatch piece_table[] = { { 0, 0, 0 } }; const unsigned piece_count = 0;\nvoid draft_entry(void) {}\n")
        for n in sorted(x for x in undefined - defined if x.startswith("abi_")):
            f.write("void %s(void) {}\n" % n)
    link(os.path.join(work, "warm.bin"), game_objs + libs + [rt, stub], [], [], None)


def runtime_object(cache):
    """The runtime compiled once for its symbol list."""
    src = os.path.join(HERE, "liftcmp_rt.c")
    obj = os.path.join(cache, "rt_names." + hashlib.sha1(open(src, "rb").read() + header_hash().encode()).hexdigest()[:12] + ".o")
    if not os.path.exists(obj):
        r = run(["cc", "-O1", "-w", "-fno-pie", "-DENTRY=draft_entry", "-I", os.path.join(TOP, "src", "rondo", "include"),
                 "-c", src, "-o", obj])
        if r.returncode:
            raise SystemExit(f"liftcmp: the runtime does not compile:\n{r.stderr[-800:]}")
    return obj


def build_dispatch(path, rows, objs, through=None, libsyms=(), resident=(), pieces_src=None, pieces_unfiled_only=False):
    defined, undefined = set(), set()
    for o in objs:
        for line in run(["nm", "-g", o]).stdout.splitlines():
            parts = line.split()
            if len(parts) == 3 and parts[1] in "TtWDdBbRr":
                defined.add(parts[2])
            elif len(parts) == 2 and parts[0] == "U":
                undefined.add(parts[1])
    external = sorted(n for n in undefined - defined if n.startswith("abi_"))
    seen, items, by_disc = set(), [], {}
    for run_addr, _len, disc, name, abi, rom in rows:
        if abi not in defined:
            continue
        key = (disc, run_addr.upper().lstrip("0") or "0")
        label = None
        if through:
            label = through.get(("C",) + key if rom == "1" else key)
        sym = ("abi_" + label) if label and ("abi_" + label) in libsyms else abi
        if rom == "0" and int(disc) not in by_disc:
            by_disc[int(disc)] = sym
        if (rom, run_addr) not in seen:
            seen.add((rom, run_addr))
            items.append((run_addr, sym))
    if through:
        # the card's jump table and every other card entry that has a draft: the draft side runs it across the boundary
        have = {r for r, _ in items}
        idx = {r: k for k, (r, _) in enumerate(items)}
        for key, label in through.items():
            if key[0] == "C" and ("abi_" + label) in libsyms:
                r = key[2].rjust(4, "0")
                if r in idx:
                    items[idx[r]] = (r, "abi_" + label)
                elif r not in have:
                    items.append((r, "abi_" + label))
                    have.add(r)
    pieces = {}
    src_map = pieces_src if pieces_src is not None else through
    if src_map:
        for key, label in src_map.items():
            if key[0] != "C" and ("abi_" + label) in libsyms and not (pieces_unfiled_only and int(key[0]) in by_disc):
                pieces[(int(key[0]), int(key[1], 16))] = "abi_" + label
                r = key[1].rjust(4, "0")
                if ("0", r) not in seen and ("1", r) not in seen:
                    seen.add(("0", r))
                    items.append((r, "abi_" + label))
    syms = sorted({x for _, x in items} | set(by_disc.values()) | set(pieces.values()))
    with open(path, "w") as f:
        f.write("#include <stdint.h>\n#include <stdio.h>\n")
        f.write("typedef struct { uint16_t run; void (*abi)(void); } Dispatch;\n")
        for sym in syms:
            f.write("void %s(void);\n" % sym)
        f.write("const Dispatch dispatch_table[] = {\n")
        for run_addr, sym in items:
            f.write("    { 0x%s, %s },\n" % (run_addr, sym))
        f.write("    { 0, 0 }\n};\nconst unsigned dispatch_count = %d;\n" % len(items))
        f.write("typedef struct { uint32_t disc; void (*abi)(void); } DiscDispatch;\nconst DiscDispatch disc_table[] = {\n")
        for d, sym in sorted(by_disc.items()):
            f.write("    { %du, %s },\n" % (d, sym))
        f.write("    { 0, 0 }\n};\nconst unsigned disc_count = %d;\n" % len(by_disc))
        f.write("typedef struct { uint32_t disc; uint16_t run; void (*abi)(void); } PieceDispatch;\n"
                "const PieceDispatch piece_table[] = {\n")
        for (d, run_addr), sym in sorted(pieces.items()):
            f.write("    { %du, 0x%04X, %s },\n" % (d, run_addr, sym))
        f.write("    { 0, 0, 0 }\n};\nconst unsigned piece_count = %d;\n" % len(pieces))
        f.write("typedef struct { uint8_t m[5]; uint16_t run; uint32_t disc; } Resident;\nconst Resident resident_tab[] = {\n")
        for cfg, logical, d in resident:
            f.write("    { { %s }, 0x%04X, %du },\n" % (", ".join("0x" + c for c in cfg), logical, d))
        f.write("    { { 0 }, 0, 0 }\n};\nconst unsigned resident_count = %d;\n" % len(resident))
        # code with no C here: a call to it is logged and does nothing
        f.write("void rt_event(void);\n")
        for name in external:
            f.write('void %s(void) { rt_event(); printf("EXT %s\\n"); }\n' % (name, name))
    return len(items)


def draft_library(drafts_dirs, work_cache):
    """(object of every draft as C, {(disc, run): label}, the symbols it defines) for the callees' side of the comparison."""
    sys.path.insert(0, HERE)
    if drafts_dirs:
        d = drafts_dirs[0]
        members = {}
        for r in csv.DictReader(open(os.path.join(d, "drafts.csv"))):
            if not int(r.get("rom") or 0):
                members[(r["disc"], r["run"].upper().lstrip("0") or "0")] = r["name"]
    else:
        import cdraft
        d = cdraft.current_cache()
        if not d:
            raise SystemExit("liftcmp: the draft cache is stale or missing: cpu-run -j 1 -m 3G -- python3 tools/rondo/cdraft.py --cache")
        members = {}
        for r in csv.DictReader(open(os.path.join(d, "members.csv"))):
            key = (r["offset"], r["run"].upper().lstrip("0") or "0")
            members.setdefault(("C",) + key if r["space"] == "C" else key, r["name"])
    src = os.path.join(d, "drafts.c")
    key = hashlib.sha1(open(src, "rb").read() + header_hash().encode()).hexdigest()[:16]
    obj = os.path.join(work_cache, "drafts_lib." + key + ".o")
    if not os.path.exists(obj):
        r = run(["cc", "-O0", "-w", "-fno-pie", "-I", os.path.join(TOP, "src", "rondo", "include"), "-c", src, "-o", obj])
        if r.returncode:
            raise SystemExit(f"liftcmp: the drafts do not compile:\n{r.stderr[-800:]}")
    syms = {l.split()[2] for l in run(["nm", "-g", "--defined-only", obj]).stdout.splitlines() if len(l.split()) == 3}
    return obj, members, syms


def link(exe, objs, extra_srcs, defs, cache):
    """Links with every symbol required: a helper the C or a draft calls and the runtime lacks fails here by name
    (an abi_ routine with no C is defined as a logging stub by the dispatch file)."""
    r = run(["cc", "-O1", "-g1", "-w", "-no-pie", "-fno-pie", *defs, "-I", os.path.join(TOP, "src", "rondo", "include"),
             "-I", os.path.join(TOP, "syscard", "include"), *objs, *extra_srcs, "-o", exe])
    if r.returncode:
        missing = sorted(set(re.findall(r"undefined reference to `([^']+)'", r.stderr)))
        if missing:
            raise SystemExit(f"liftcmp: the runtime (tools/rondo/liftcmp_rt.c) lacks {', '.join(missing)}, which the C or the draft call(s); add it there")
        raise SystemExit(f"liftcmp: link failed:\n{r.stderr[-1500:]}")


def split_log(text):
    cases, cur = {}, None
    for line in text.splitlines():
        if line.startswith("CASE "):
            cur = int(line[5:], 16)
            cases[cur] = []
        elif cur is not None:
            cases[cur].append(line)
    return cases


def coverage_sites(exe):
    out = run(["objdump", "-d", "--no-show-raw-insn", exe]).stdout
    return [int(m.group(1), 16) + 5 for m in re.finditer(r"^\s*([0-9a-f]+):\s+call\s+[0-9a-f]+ <__sanitizer_cov_trace_pc>", out, re.M)]


def addr_lines(exe, addrs, base):
    """{return address: (function, line)} for the outermost frame of each probe that lies in the file `base`."""
    if not addrs:
        return {}
    inp = "\n".join("0x%x" % (a - 1) for a in addrs) + "\n"
    out = run(["addr2line", "-e", exe, "-f", "-a", "-i"], input=inp).stdout.splitlines()
    res, cur, frames = {}, None, []

    def flush():
        if cur is None:
            return
        best = None
        for fn, fl in frames:
            m = re.match(r"(.*):(\d+)", fl)
            if m and os.path.basename(m.group(1)) == base:
                best = (fn, int(m.group(2)))
        if best:
            res[cur] = best

    i = 0
    while i < len(out):
        if out[i].startswith("0x") and ":" not in out[i]:
            flush()
            cur, frames = int(out[i], 16) + 1, []
            i += 1
        else:
            frames.append((out[i], out[i + 1] if i + 1 < len(out) else ""))
            i += 2
    flush()
    return res


def coverage(label, exe, covfile, source_base, source_text, ids_ok):
    sites = coverage_sites(exe)
    where = addr_lines(exe, sites, source_base)
    mine = {a: (where[a][0], source_base, where[a][1]) for a in sites if a in where}
    reached = {}
    for line in open(covfile):
        parts = line.split()
        if not parts:
            continue
        cid = int(parts[0], 16)
        if cid not in ids_ok:
            continue
        for pc in parts[1:]:
            pc = int(pc, 16)
            if pc in mine:
                reached.setdefault(mine[pc][2], set()).add(cid)
    funcs = {}
    for a, (fn, fl, ln) in mine.items():
        funcs.setdefault(fn, set()).add(ln)
    entered = {fn for fn, lns in funcs.items() if any(l in reached for l in lns)}
    lines = sorted({ln for a, (fn, fl, ln) in mine.items() if fn in entered})
    unreached = [l for l in lines if l not in reached]
    rare = [(l, sorted(reached[l])[:3]) for l in lines if l in reached and len(reached[l]) <= 3]
    src = source_text.splitlines()
    out = [f"{label}: {len(lines) - len(unreached)} of {len(lines)} lines with probes reached, in {len(entered)} functions"]
    for l in unreached:
        out.append(f"  not reached  {l}: {src[l - 1].strip()[:100] if 0 < l <= len(src) else ''}")
    for l, ids in rare:
        out.append(f"  reached by {len(reached[l])}  {l}: {src[l - 1].strip()[:80] if 0 < l <= len(src) else ''}  cases {ids}")
    return out, len(unreached)


def fixed_bits(untraced_vals, traced_vals):
    """{bit: value a traced case has} for each bit that is the same in every untraced case and differs in some traced one."""
    fix = {}
    if not untraced_vals or not traced_vals:
        return fix
    for bit in range(8):
        u = {(v >> bit) & 1 for v in untraced_vals}
        t = {(v >> bit) & 1 for v in traced_vals}
        if len(u) == 1 and (t - u):
            fix[bit] = sorted(t - u)[0]
    return fix


def steer_cases(cases, untraced_ids, draft_logs, floor, rng_seed):
    """More cases for a draft that keeps reaching bytes no script ran: clones of the cases that did, with the byte the
    draft read last before the untraced arm, and the entry register that is the same in all of them, changed to what a
    case that stayed on traced code had."""
    rng = random.Random(rng_seed)
    by_id = {c["id"]: c for c in cases}
    last = {}
    traced_vals = {}
    for c in cases:
        log = draft_logs.get(c["id"], [])
        if any(x.startswith("UNTRACED") for x in log):
            reads = [x for x in log if x.startswith("R ")]
            if reads:
                ad, v = reads[-1].split()[1:3]
                last.setdefault(ad, set()).add(v)
        else:
            for x in log:
                if x.startswith("R "):
                    ad, v = x.split()[1:3]
                    traced_vals.setdefault(ad, set()).add(v)
    regs = {}
    tr = [c for c in cases if c["id"] not in set(untraced_ids)]
    for r in ("a", "x", "y"):
        fix = fixed_bits([by_id[i][r] for i in untraced_ids], [c[r] for c in tr])
        if fix:
            regs[r] = fix
    out = []
    for n in range(max(floor * 2, 40)):
        src = by_id[untraced_ids[n % len(untraced_ids)]]
        c = dict(src, mem=list(src["mem"]), seed=rng.getrandbits(48), label="steered")
        for r, fix in regs.items():
            for bit, val in fix.items():
                c[r] = (c[r] & ~(1 << bit) & 0xFF) | (val << bit)
        for ad, stuck in last.items():
            choices = sorted(traced_vals.get(ad, set()) - stuck) or [("%02X" % rng.choice([0, 1, 2, 0x7F, 0x80, 0xFF]))]
            v = rng.choice(choices)
            if v not in stuck or len(choices) == 1:
                c["mem"].append((int(ad, 16), int(v, 16)))
        out.append(c)
    return out


def explain_untraced(cases, untraced_ids, draft_logs, raw_draft, draft_text):
    """Lines saying what steers the draft into the bytes no script ran, for a writer's curated cases."""
    by_id = {c["id"]: c for c in cases}
    where = {}
    for i in untraced_ids:
        log = draft_logs[i]
        at = [x for x in log if x.startswith("UNTRACED")][0].split()[1]
        reads = [x for x in log if x.startswith("R ")]
        where.setdefault(at, []).append((i, reads[-1].split()[1:3] if reads else None))
    traced = [c for c in cases if c["id"] not in set(untraced_ids)]
    out = []
    dl = draft_text.splitlines()
    for at, items in sorted(where.items(), key=lambda kv: -len(kv[1])):
        out.append(f"  untraced arm ${at}: reached by {len(items)} cases, for instance {[i for i, _ in items[:4]]}")
        for k, line in enumerate(dl):
            if "untraced(0x%s)" % at in line:
                ctx = [x.strip() for x in dl[max(0, k - 3):k + 1] if x.strip() and not x.strip().startswith("L0")]
                out.append("    the draft: " + " ; ".join(ctx)[:300])
                prev = [x for x in dl[max(0, k - 2):k] if "goto" in x and "/*" in x]
                if prev:
                    m = re.search(r"/\* ([0-9A-F]{4}) (\w+) \*/", prev[-1])
                    if m:
                        out.append(f"    every traced run took the branch at ${m.group(1)} ({m.group(2)}) and went on; this arm is the way it was never taken: "
                                   "make that condition false (the flags from the instructions above it)")
                break
        lastr = {}
        for i, r in items:
            if r:
                lastr.setdefault(r[0], set()).add(r[1])
        for ad, vals in sorted(lastr.items(), key=lambda kv: kv[0])[:3]:
            t = set()
            for c in traced:
                for x in draft_logs.get(c["id"], []):
                    if x.startswith("R " + ad + " "):
                        t.add(x.split()[2])
            out.append(f"    the last read before it: ${ad} gave {sorted(vals)[:6]}{' ...' if len(vals) > 6 else ''}; cases that stayed on traced code read there: {sorted(t)[:6] or 'nothing (never read on a traced path)'}")
        for r in ("a", "x", "y"):
            fix = fixed_bits([by_id[i][r] for i, _ in items], [c[r] for c in traced])
            if fix:
                out.append(f"    entry {r.upper()}: " + ", ".join(f"bit {b} is {1 - v} in every one of them, and {v} in cases that stayed on traced code" for b, v in sorted(fix.items())))
        if items and any(r for _, r in items):
            out.append("    write a curated case: mem:ADDR=VAL for the byte above (a value a traced case read), then run it with --curated FILE")
    return out


STUBS = os.path.join(HERE, "stubs.csv")


def stub_contracts():
    """The rows of tools/rondo/stubs.csv: target (hex run address), place (SECTOR:OFFSET of the overlay copy, or *), keeps
    (letters of A X Y N V Z C), evidence (the instruction and trace that show it). Returns [(target, disc offset or None,
    keeps, evidence)]; a row with no evidence, or a letter that is not a register or flag, is refused."""
    rows = []
    if not os.path.exists(STUBS):
        return rows
    for r in csv.DictReader(l for l in open(STUBS, encoding="utf-8") if l.strip() and not l.startswith("#")):
        t, entry, place, keeps, sets, ev = (r[k].strip() for k in ("target", "entry", "place", "keeps", "sets", "evidence"))
        if not ev or not (keeps or sets) or any(c not in "AXYNVZC" for c in keeps) or not entry:
            raise SystemExit(f"liftcmp: stubs.csv row {t} {place}: it needs its entry, its keeps (letters of AXYNVZC) or sets, and its evidence")
        disc = None if place == "*" else int(place.split(":")[0], 16) * 2048 + int(place.split(":")[1], 16)
        rows.append((int(t, 16), int(entry, 16), disc, keeps or "-", sets or "-", ev))
    return rows


def write_stub_file(path):
    with open(path, "w") as f:
        for t, e, d, k, st, _ in stub_contracts():
            f.write("%04X %04X %s %s %s\n" % (t, e, "*" if d is None else "%X" % d, k, st))
    return path


def routine_ends(path, name):
    """The @ends address (hex) in the routine's tag block, or '' when it has none."""
    for block in re.findall(r"/\*.*?\*/", open(path, encoding="utf-8").read(), re.S):
        if re.search(r"@routine\s+\S+\s+%s\b" % re.escape(name), block):
            m = re.search(r"@ends\s+([0-9A-Fa-f]{4})", block)
            return m.group(1) if m else ""
    return ""


def one(name, a, ctx):
    os.makedirs(ctx["cache"], exist_ok=True)
    src = find_source(name, ctx["sources"])
    override = a.lifted
    objs = []
    for s in ctx["sources"]:
        objs.append(ctx["objs"][s] if not (override and s == src) else compile_obj(s, override, ctx["cache"], ctx["hh"]))
    found = None
    if ctx["drafts"]:
        for d in ctx["drafts"]:
            found = extract_draft(d, name)
            if found:
                break
    else:
        found = cache_draft(name, routine_tag(name)[1])
    ctx["ops"] = find_ops(None, a.ops)
    if not found:
        disc, _ = routine_tag(name)
        label = ("r%04X_%03X" % divmod(disc, 2048)) if disc is not None else None
        raise SystemExit(f"liftcmp: {name} has no draft ({'in ' + ', '.join(ctx['drafts']) if ctx['drafts'] else 'not in the draft cache'}); "
                         + (f"its disc location is {disc} ({label}); if the routine has traced code, rebuild the cache: "
                            "cpu-run -j 1 -m 3G -- python3 tools/rondo/cdraft.py --cache"
                            if label else "it has no @routine tag with a @run, so it cannot be looked up by location"))
    row, body, ndraft = found
    work = os.path.join(BUILD, "work", name)
    os.makedirs(work, exist_ok=True)
    disp = os.path.join(work, "dispatch.c")
    configs = run_configs(name, ctx["rows_full"])
    run_set = {int(r[0], 16) for r in ctx["rows"]}
    if a.callees == "through":
        mem_path = os.path.join(ctx["drafts"][0] if ctx["drafts"] else __import__("cdraft").current_cache(), "members.csv")
        if os.path.exists(mem_path):
            for r in csv.DictReader(open(mem_path)):
                if r["space"] == "D":
                    run_set.add(int(r["run"], 16))
    near = next((int(r[2]) for r in ctx["rows"] if r[3] == name), None)
    resident = resident_entries(configs, ctx["rows"], run_set, near)
    disp_d, libobj = disp, []
    if a.callees == "through":
        libo, members, libsyms = draft_library(ctx["drafts"], ctx["cache"])
        # code the C calls that has no C of its own (the original's code) runs, on the lifted side too, as its draft
        ndisp = build_dispatch(disp, ctx["rows"], objs, resident=resident, libsyms=libsyms, pieces_src=members,
                               pieces_unfiled_only=True)
        disp_d = os.path.join(work, "dispatch_draft.c")
        build_dispatch(disp_d, ctx["rows"], objs, members, libsyms, resident)
        libobj = [libo]
    else:
        ndisp = build_dispatch(disp, ctx["rows"], objs, resident=resident)
    draft_c = os.path.join(work, "draft_%s.c" % name)
    open(draft_c, "w").write('#include "machine.h"\n#include "ops6280.h"\n\n' + body)
    rt = os.path.join(HERE, "liftcmp_rt.c")
    cases = make_cases(parse_curated(a.curated), a.cases, a.seed, a.decimal, a.irq, configs, parse_seed_fixed(a.curated))
    cfile = os.path.join(work, "cases.txt")
    write_cases(cases, cfile)
    exes = {}
    ops_inc = ["-I", ctx["ops"]]
    dobj = os.path.join(work, "draft.o")
    r = run(["cc", *CFLAGS, *ops_inc, "-I", os.path.join(TOP, "src", "rondo", "include"), "-c", draft_c, "-o", dobj])
    if r.returncode:
        raise SystemExit(f"liftcmp: the draft does not compile:\n{r.stderr[-1500:]}")
    rtobj = {}
    for side, entry in (("lifted", "abi_" + name), ("draft", "draft_entry")):
        ro = os.path.join(work, "rt_%s.o" % side)
        r = run(["cc", "-O1", "-g1", "-w", "-fno-pie", "-DENTRY=" + entry, "-I", os.path.join(TOP, "src", "rondo", "include"),
                 "-c", rt, "-o", ro])
        if r.returncode:
            raise SystemExit(f"liftcmp: runtime does not compile:\n{r.stderr[-800:]}")
        rtobj[side] = ro
    dispobj = os.path.join(work, "dispatch.o")
    run(["cc", "-O1", "-w", "-fno-pie", "-c", disp, "-o", dispobj])
    dispobj_d = dispobj
    if disp_d != disp:
        dispobj_d = os.path.join(work, "dispatch_draft.o")
        run(["cc", "-O1", "-w", "-fno-pie", "-c", disp_d, "-o", dispobj_d])
    link(os.path.join(work, "lifted.bin"), objs + [rtobj["lifted"], dispobj] + libobj, [], [], None)
    link(os.path.join(work, "draft.bin"), objs + [dobj, rtobj["draft"], dispobj_d] + libobj, [], [], None)
    logs, covs = {"lifted": {}, "draft": {}}, {}
    envv = dict(os.environ, LIFTCMP_DECIMAL="1" if a.decimal else "0", LIFTCMP_ENDS=routine_ends(src, name),
                LIFTCMP_STUBS=write_stub_file(os.path.join(work, "stubs.txt")))
    for base in (TOP, main_checkout()):
        rom = os.path.join(base, "build", "rondo", "system", "syscard3.pce")
        if os.path.exists(rom):
            envv["LIFTCMP_CARDROM"] = rom
            break
    base_cases = len(cases)
    floor = a.floor if a.floor else max(1, (base_cases + 3) // 4)

    def run_round(cs, tag):
        cf = os.path.join(work, "cases_%s.txt" % tag)
        write_cases(cs, cf)
        for side in ("lifted", "draft"):
            covf = os.path.join(work, "%s_%s.cov" % (side, tag))
            r = run([os.path.join(work, side + ".bin"), cf, covf], timeout=120, env=envv)
            if r.returncode:
                raise SystemExit(f"liftcmp: the {side} side exited {r.returncode}:\n{r.stderr[-400:]}")
            logs[side].update(split_log(r.stdout))
            with open(os.path.join(work, side + ".cov"), "a" if tag != "0" else "w") as out:
                out.write(open(covf).read())
            covs[side] = os.path.join(work, side + ".cov")

    run_round(cases, "0")
    rounds = 0
    while a.steer and rounds < 2:
        un = [c["id"] for c in cases if any(x.startswith("UNTRACED") for x in logs["draft"].get(c["id"], []))]
        if len(cases) - len(un) >= floor or not un:
            break
        new_cases = steer_cases(cases, un, logs["draft"], floor, rng_seed=a.seed + 1000 + rounds)
        if not new_cases:
            break
        for c in new_cases:
            c["id"] = len(cases)
            cases.append(c)
            # (ids are positions in `cases`)
        run_round(new_cases, str(rounds + 1))
        rounds += 1
    ignore = sum(FLAGBITS[ch] for ch in a.ignore_flags)
    same, diffs, untraced, deep, crashed = [], [], [], [], []
    raw_draft = logs["draft"]
    raw_lifted = logs["lifted"]
    for side in ("draft", "lifted"):
        logs[side] = {k: [x for x in v if not x.startswith("#")] for k, v in logs[side].items()}
    names_by_run = {r[0].lstrip("0").upper() or "0": r[3] for r in ctx["rows"]}
    for c in cases:
        i = c["id"]
        dl, ll = logs["draft"].get(i, []), logs["lifted"].get(i, [])
        if any(x.startswith("UNTRACED") for x in dl):
            untraced.append(i)
            continue
        if any(x.startswith("CRASHED") for x in dl) or any(x.startswith("CRASHED") for x in ll):
            crashed.append(i)
            continue
        if any(x.startswith("DEPTHLIMIT") for x in dl) or any(x.startswith("DEPTHLIMIT") for x in ll):
            deep.append(i)
            continue
        if ignore:
            dl, ll = [mask_p(x, ignore) for x in dl], [mask_p(x, ignore) for x in ll]
        if dl == ll:
            same.append(i)
        else:
            diffs.append(i)
    comparable = len(same) + len(diffs)
    # what the case's mapper had resident where the routine asked (resident_at), and where the run stopped at a wait
    res = {}
    for i, v in raw_lifted.items():
        for x in {x for x in v if x.startswith("#RES")}:
            _, ad, d = x.split()
            res.setdefault(ad, {}).setdefault(int(d), set()).add(i)
    waits = sum(1 for i in same + diffs if any(x == "WAIT" for x in logs["draft"].get(i, [])))
    hung = sum(1 for i in same + diffs if any(x == "HUNG" for x in logs["draft"].get(i, [])))
    verdict = ("DIFFERS" if diffs else "CRASHED (%d cases ended in a crash: a seeded value drove the C or the draft out of range; not a pass)" % len(crashed) if crashed else
               "NOT CHECKED" if comparable == 0 else
               "THIN (%d of %d cases comparable, floor %d)" % (comparable, len(cases), floor) if comparable < floor else "OK")
    lines = [f"{name}: draft {row['name']} (run {row['run']}), {len(cases)} cases ({len(parse_curated(a.curated))} curated, {'decimal cases: D set in entry and stubs' if a.decimal else 'D clear'}, callees: {'the callees run as drafts against the lifted callers calls' if a.callees == 'through' else 'filed wrappers on both sides'}, {(str(len(configs)) + ' mapper configuration' + ('s' if len(configs) > 1 else '') + ' from the trace') if configs else 'no trace of it: default map'}): "
             f"{len(same)} match, {len(diffs)} differ, {len(untraced)} draft reached untraced bytes"
             + (f", {len(crashed)} crashed on a seeded value out of the game's range, on {'both sides' if any('CRASHED' in x for x in logs['draft'].get(crashed[0], [])) and any('CRASHED' in x for x in logs['lifted'].get(crashed[0], [])) else 'the lifted side' if any('CRASHED' in x for x in logs['lifted'].get(crashed[0], [])) else 'the draft'} in the first (not compared)" if crashed else "")
             + (f", {len(deep)} reached the depth limit of 512 nested calls (not compared)" if deep else "")
             + (f" ({rounds} steering round{'s' if rounds != 1 else ''} added cases)" if rounds else "") + f" -> {verdict}"]
    for ad, byd in sorted(res.items()):
        lines.append("  resident_at($%s) in the cases: " % ad + ", ".join(
            ("nothing resident" if d == 0 else "%04X:%03X" % divmod(d, 2048)) + " in %d" % len(cs) for d, cs in sorted(byd.items())))
    if waits or hung:
        lines.append(f"  {waits} compared cases ended at a wait for an interrupt (compared up to it), {hung} at a spin on hardware that never ended (compared to the time limit)")
    if untraced and verdict not in ("OK", "DIFFERS") or (untraced and a.explain):
        lines += explain_untraced(cases, untraced, logs["draft"], raw_draft, open(draft_c).read())
    for i in diffs[:a.show]:
        c = cases[i]
        dl, ll = logs["draft"][i], logs["lifted"][i]
        k = 0
        while k < min(len(dl), len(ll)) and dl[k] == ll[k]:
            k += 1
        lines.append(f"  case {i} ({c['label']}): A={c['a']:02X} X={c['x']:02X} Y={c['y']:02X} S={c['s']:02X} P={c['p']:02X} seed {c['seed']:X}"
                     f"{' mem ' + ' '.join('%04X=%02X' % m for m in c['mem']) if c['mem'] else ''}")
        inside, seen = [], 0
        for x in raw_draft[i]:
            if x.startswith("#"):
                if x.startswith("#IN"):
                    inside.append(x.split()[1])
                elif inside:
                    inside.pop()
            else:
                if seen == k:
                    break
                seen += 1
        where = ""
        if inside:
            where = " inside callee " + " > ".join("%s (%s)" % (names_by_run.get(a.lstrip("0").upper() or "0", "?"), a) for a in inside)
        lines.append(f"    the first {k} events agree; then (the draft's side{where or ' is in the routine itself'})")
        lines.append(f"    draft : {' | '.join(dl[k:k + 3]) or '(nothing)'}")
        lines.append(f"    lifted: {' | '.join(ll[k:k + 3]) or '(nothing)'}")
    if len(diffs) > a.show:
        lines.append(f"  ... and {len(diffs) - a.show} more differing cases: {diffs[a.show:a.show + 12]}")
    ok_ids = set(same) | set(diffs)
    cover_lines, miss = [], 0
    if not a.no_cover and ok_ids:
        text_l = open(override or src).read()
        cl, m1 = coverage("lifted " + os.path.relpath(src, TOP), os.path.join(work, "lifted.bin"), covs["lifted"],
                          os.path.basename(src), text_l, ok_ids)
        cd, m2 = coverage("draft " + row["name"], os.path.join(work, "draft.bin"), covs["draft"],
                          os.path.basename(draft_c), open(draft_c).read(), ok_ids)
        cover_lines = cl + cd
        miss = m1 + m2
    used = set()
    for v in list(raw_lifted.values()) + list(raw_draft.values()):
        for x in v:
            if x.startswith("#CONTRACT"):
                used.add(x.split()[1])
    assumed = []
    for t, e, d, k, st, ev in stub_contracts():
        if "%04X" % t in used:
            assumed.append(f"ASSUMPTION: the callee at ${t:04X} ({'any copy' if d is None else 'the copy of the overlay whose $%04X is at disc %X' % (e, d)}) has no C; "
                           f"its stub keeps {k}{'; sets ' + st if st != '-' else ''} ({ev})")
    lines = lines[:1] + assumed + lines[1:]
    rep = os.path.join(a.report, name + ".txt")
    os.makedirs(a.report, exist_ok=True)
    open(rep, "w").write("\n".join(lines + cover_lines) + "\n")
    if a.quiet:
        print(f"{name:32s} match {len(same):4d}  differ {len(diffs):4d}  untraced {len(untraced):4d}  unreached lines {miss:3d}  "
              f"{verdict}" + (" [assumes stub contract: " + ", ".join(sorted(used)) + "]" if used else ""))
        return verdict == "OK", dict(same=len(same), diffs=len(diffs), untraced=len(untraced), unreached=miss, ndraft=ndraft)
    print("\n".join(lines))
    if cover_lines:
        print(f"  coverage: {cover_lines[0]}; {[l for l in cover_lines if l.startswith('draft ')][0]}; {miss} lines not reached; full report {os.path.relpath(rep, TOP)}")
    return verdict == "OK", dict(same=len(same), diffs=len(diffs), untraced=len(untraced), unreached=miss, ndraft=ndraft)


def mask_p(line, ignore):
    m = re.search(r"P=([0-9A-F]{2})", line)
    if not m:
        return line
    return line[:m.start(1)] + "%02X" % (int(m.group(1), 16) & ~ignore & 0xFF) + line[m.end(1):]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("names", nargs="*")
    ap.add_argument("--file", help="a C file: every routine in it that has a draft, one line each")
    ap.add_argument("--drafts", help="a folder with drafts.c and drafts.csv, in place of the draft cache (tools/rondo/cdraft.py --cache)")
    ap.add_argument("--ops")
    ap.add_argument("--cases", type=int, default=400)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--curated")
    ap.add_argument("--lifted", help="a C file to use in place of the tree's file for the routine")
    ap.add_argument("--callees", choices=("through", "wrappers"), default="through", help="through: the callees run as their drafts on the draft side and as the typed C the lifted caller calls on the other (default); wrappers: the callees run as their filed abi_ wrappers on both sides")
    ap.add_argument("--floor", type=int, default=0, help="fewer comparable cases than this is THIN (default: a quarter of the seeded cases)")
    ap.add_argument("--no-steer", dest="steer", action="store_false", help="do not add cases that steer the draft away from bytes no script ran")
    ap.add_argument("--explain", action="store_true", help="say what steers into the untraced arms even when enough cases compare")
    ap.add_argument("--decimal", action="store_true", help="decimal cases: D set at entry in half the cases and in the P a stub returns")
    ap.add_argument("--irq", action="store_true", help="I set at entry in half the cases")
    ap.add_argument("--ignore-flags", default="", help="flag letters not compared at exit (NVDIZC)")
    ap.add_argument("--quiet", action="store_true", help=argparse.SUPPRESS)
    ap.add_argument("--no-cover", action="store_true")
    ap.add_argument("--warm", action="store_true", help="compile the game's C into the cache and stop")
    ap.add_argument("--show", type=int, default=3)
    ap.add_argument("--report", default=os.path.join(BUILD, "report"))
    a = ap.parse_args()
    os.makedirs(os.path.join(BUILD, "obj"), exist_ok=True)
    cache = os.path.join(BUILD, "obj")
    sources = game_sources() + card_sources()
    hh = header_hash()
    jobs = int(os.environ.get("CPU_RUN_JOBS") or 1)
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as ex:
        objs = dict(zip(sources, ex.map(lambda s: compile_obj(s, None, cache, hh), sources)))
    if a.warm:
        warm_check(list(objs.values()), cache)
        print(f"liftcmp: {len(sources)} files compiled into {os.path.relpath(cache, TOP)}; the runtime defines every helper they call")
        return 0
    if not a.names and not a.file:
        ap.error("name a routine or give --file")
    drafts = find_drafts(a.drafts)
    ctx = dict(cache=cache, sources=sources, objs=objs, hh=hh, drafts=drafts, ops=None, rows=hybrid_rows())
    ctx["rows_full"] = ctx["rows"]
    bad = 0
    names = list(a.names)
    if a.file:
        path = os.path.join(TOP, a.file) if not os.path.isabs(a.file) else a.file
        names += re.findall(r"^void abi_(\w+)\(void\)\s*$", open(path).read(), re.M)
        a.quiet = True
        a.show = 0
        print(f"{a.file}: {len(names)} routines with a draft")
    for n in names:
        try:
            ok, _ = one(n, a, ctx)
        except SystemExit as e:
            if not a.file or "has no draft" not in str(e):
                raise
            print(f"{n:32s} {str(e)[len('liftcmp: ' + n + ' '):][:200]}")
            continue
        bad += not ok
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
