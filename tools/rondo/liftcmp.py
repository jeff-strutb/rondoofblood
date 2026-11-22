#!/usr/bin/env python3
"""liftcmp.py -- the lift harness: a filed routine's C against its draft, access for access.

    python3 tools/rondo/liftcmp.py NAME [NAME ...] [--drafts DIR] [--cases N] [--seed S] [--curated FILE]
                                   [--lifted FILE] [--ignore-flags NVDIZC] [--report DIR]

NAME is a filed routine (its `abi_NAME` in src/rondo). The DRAFT is the proven side: the register-shaped C
tools/rondo/cdraft.py wrote for that routine (DIR/drafts.csv names it in its `filed` column; DIR is the drafter's
generated folder, found beside the checkout when not given). The C in the tree is the new side. Both are entered
the same way, through the register file with the same registers and flags, in a machine that logs everything
(tools/rondo/liftcmp_rt.c), and for each case the whole log is compared: every read and write in order with its
address and value, the VDC, MPR and clock instructions, every call the routine makes (the frame it pushes, the
callee and the registers it is entered with) and the registers and flags it leaves.

Cases. N seeded cases (default 400): entry A, X, Y, S and flags from edge-biased values; a memory image whose
unwritten bytes read as a value fixed by the seed, the address and how many times it has been read, in one of
three flavours (any byte, edge values, small values); a byte written reads back. The curated file adds cases a
writer wants, one a line: `a=3 x=5 y=0 s=F0 p=NZC mem:2B70=80 mem:3040+5=01` (hex; p a set of flag letters or a
hex byte; +N is an offset from the address); they run first and are named by line.

Other routines. A call into a routine that has C in the tree runs that C, on both sides (so the callee's accesses are
in both logs); a call into code with no C is a stub that returns seeded A, X, Y and flags and logs the call. A routine
is therefore compared with what it calls held equal, as A5 judges it, not with its callees' own correctness.
A draft that reaches bytes no script ran ends its case (UNTRACED): that case proves nothing and is counted apart.

Coverage. Both sides are built with the compiler's block probes; the report says which lines of the lifted file and
of the draft no case reached, and for each line reached by three cases or fewer, which. A line no case reached is an
arm that is not proven: add a curated case for it.

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


def find_drafts(given, ops_given=None):
    """The drafts folder: the one given, else the newest `gen_all` beside a work tree that also holds ops6280.h."""
    cands = [given] if given else []
    if not given:
        r = run(["git", "-C", TOP, "worktree", "list", "--porcelain"])
        trees = [l[9:] for l in r.stdout.splitlines() if l.startswith("worktree ")]
        for t in trees:
            for sub in ("build/rondo/drafter/gen_all", "build/rondo/drafter/gen"):
                c = os.path.join(t, sub)
                if os.path.exists(os.path.join(c, "drafts.csv")) and os.path.exists(os.path.join(t, "src", "rondo", "include", "ops6280.h")):
                    cands.append(c)
        cands.sort(key=lambda c: -os.path.getmtime(os.path.join(c, "drafts.csv")))
    for c in cands:
        if os.path.exists(os.path.join(c, "drafts.csv")) and os.path.exists(os.path.join(c, "drafts.c")):
            return c
    raise SystemExit("liftcmp: no drafts folder (drafts.c and drafts.csv) beside a tree with ops6280.h; pass --drafts DIR")


def find_ops(drafts_dir, given):
    cands = [given] if given else [os.path.join(TOP, "src", "rondo", "include")]
    if not given:
        t = drafts_dir
        for _ in range(5):
            t = os.path.dirname(t)
            cands.append(os.path.join(t, "src", "rondo", "include"))
    for c in cands:
        if os.path.exists(os.path.join(c, "ops6280.h")):
            return c
    raise SystemExit("liftcmp: ops6280.h not found; pass --ops DIR (the folder holding it)")


def hybrid_rows():
    for base in (TOP, main_checkout()):
        p = os.path.join(base, "build", "rondo", "pcebox", "hybrid_table.c")
        if os.path.exists(p):
            text = open(p).read()
            return re.findall(r'\{ 0x([0-9A-F]+), \d+, \d+, "(\w+)", (abi_\w+), \d+, 0x[0-9A-F]+, (\d) \},', text)
    raise SystemExit("liftcmp: no build/rondo/pcebox/hybrid_table.c (setup.sh pcebox writes it)")


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


def find_source(name, sources):
    pat = re.compile(r"^void abi_%s\(void\)\s*$" % re.escape(name), re.M)
    for s in sources:
        if pat.search(open(s).read()):
            return s
    raise SystemExit(f"liftcmp: no `void abi_{name}(void)` in src/rondo")


def extract_draft(drafts_dir, name):
    rows = [r for r in csv.DictReader(open(os.path.join(drafts_dir, "drafts.csv"))) if r["filed"] == name]
    if not rows:
        raise SystemExit(f"liftcmp: {name} has no draft in {drafts_dir}/drafts.csv")
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
        if not line:
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


def make_cases(curated, n, seed):
    rng = random.Random(seed)
    cases = list(curated)
    edge = [0, 1, 2, 0x7F, 0x80, 0xFF, 0xFE, 0x0F, 0xF0]
    for _ in range(n):
        def r8():
            return rng.choice(edge) if rng.random() < 0.5 else rng.randrange(256)
        p = rng.choice([0, 1, 2, 3, 0x40, 0x80, 0x82, 0xC3]) | (rng.getrandbits(6) & 0x07)
        if rng.random() < 0.9:
            p &= ~0x08
        cases.append(dict(a=r8(), x=r8() if rng.random() < 0.5 else rng.randrange(0, 0x58), y=r8(), s=rng.randrange(0x30, 0xF1),
                          p=p & 0xCF, mem=[], label="seed"))
    with_seed = []
    for i, c in enumerate(cases):
        c["seed"] = rng.getrandbits(48)
        c["id"] = i
        with_seed.append(c)
    return with_seed


def write_cases(cases, path):
    with open(path, "w") as f:
        for c in cases:
            f.write("%X %X %X %X %X %X %X %d" % (c["id"], c["seed"], c["a"], c["x"], c["y"], c["s"], c["p"], len(c["mem"])))
            for a, v in c["mem"]:
                f.write(" %04X=%02X" % (a, v))
            f.write("\n")


def build_dispatch(path, rows, objs):
    defined, undefined = set(), set()
    for o in objs:
        for line in run(["nm", "-g", o]).stdout.splitlines():
            parts = line.split()
            if len(parts) == 3 and parts[1] in "TtWDdBbRr":
                defined.add(parts[2])
            elif len(parts) == 2 and parts[0] == "U":
                undefined.add(parts[1])
    provided = {"rd8", "wr8", "st0", "st1", "st2", "tam", "tma", "tai", "cli", "csl", "csh", "wait_interrupt", "cpu",
                "call_original", "call_orig", "untraced", "hand_over", "__sanitizer_cov_trace_pc", "ENTRY"}
    external = sorted(undefined - defined - provided)
    seen, items = set(), []
    for run_addr, name, abi, rom in rows:
        if rom == "0" and abi in defined and run_addr not in seen:
            seen.add(run_addr)
            items.append((run_addr, abi))
    with open(path, "w") as f:
        f.write("#include <stdint.h>\ntypedef struct { uint16_t run; void (*abi)(void); } Dispatch;\n")
        for _, abi in items:
            f.write("void %s(void);\n" % abi)
        f.write("const Dispatch dispatch_table[] = {\n")
        for run_addr, abi in items:
            f.write("    { 0x%s, %s },\n" % (run_addr, abi))
        f.write("    { 0, 0 }\n};\nconst unsigned dispatch_count = %d;\n" % len(items))
        # code with no C here (the System Card's, for one): a call to it is logged and does nothing
        f.write("#include <stdio.h>\nvoid rt_event(void);\n")
        for name in external:
            f.write('void %s(void) { rt_event(); printf("EXT %s\\n"); }\n' % (name, name))
    return len(items)


def link(exe, objs, extra_srcs, defs, cache):
    r = run(["cc", "-O1", "-g1", "-w", "-no-pie", "-fno-pie", *defs, "-I", os.path.join(TOP, "src", "rondo", "include"),
             "-I", os.path.join(TOP, "syscard", "include"), *objs, *extra_srcs, "-o", exe,
             "-Wl,--unresolved-symbols=ignore-all"])
    if r.returncode:
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


def one(name, a, ctx):
    os.makedirs(ctx["cache"], exist_ok=True)
    src = find_source(name, ctx["sources"])
    override = a.lifted
    objs = []
    for s in ctx["sources"]:
        objs.append(ctx["objs"][s] if not (override and s == src) else compile_obj(s, override, ctx["cache"], ctx["hh"]))
    row, body, ndraft = extract_draft(ctx["drafts"], name)
    work = os.path.join(BUILD, "work", name)
    os.makedirs(work, exist_ok=True)
    disp = os.path.join(work, "dispatch.c")
    ndisp = build_dispatch(disp, ctx["rows"], objs)
    draft_c = os.path.join(work, "draft_%s.c" % name)
    open(draft_c, "w").write('#include "machine.h"\n#include "ops6280.h"\n\n' + body)
    rt = os.path.join(HERE, "liftcmp_rt.c")
    cases = make_cases(parse_curated(a.curated), a.cases, a.seed)
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
    link(os.path.join(work, "lifted.bin"), objs + [rtobj["lifted"], dispobj], [], [], None)
    link(os.path.join(work, "draft.bin"), objs + [dobj, rtobj["draft"], dispobj], [], [], None)
    logs, covs = {}, {}
    for side in ("lifted", "draft"):
        covf = os.path.join(work, side + ".cov")
        r = run([os.path.join(work, side + ".bin"), cfile, covf], timeout=60)
        if r.returncode:
            raise SystemExit(f"liftcmp: the {side} side exited {r.returncode}:\n{r.stderr[-400:]}")
        logs[side] = split_log(r.stdout)
        covs[side] = covf
    ignore = sum(FLAGBITS[ch] for ch in a.ignore_flags)
    same, diffs, untraced = [], [], []
    for c in cases:
        i = c["id"]
        dl, ll = logs["draft"].get(i, []), logs["lifted"].get(i, [])
        if any(x.startswith("UNTRACED") for x in dl):
            untraced.append(i)
            continue
        if ignore:
            dl, ll = [mask_p(x, ignore) for x in dl], [mask_p(x, ignore) for x in ll]
        if dl == ll:
            same.append(i)
        else:
            diffs.append(i)
    lines = [f"{name}: draft {row['name']} (run {row['run']}), {len(cases)} cases ({len(parse_curated(a.curated))} curated): "
             f"{len(same)} match, {len(diffs)} differ, {len(untraced)} draft reached untraced bytes"]
    for i in diffs[:a.show]:
        c = cases[i]
        dl, ll = logs["draft"][i], logs["lifted"][i]
        k = 0
        while k < min(len(dl), len(ll)) and dl[k] == ll[k]:
            k += 1
        lines.append(f"  case {i} ({c['label']}): A={c['a']:02X} X={c['x']:02X} Y={c['y']:02X} S={c['s']:02X} P={c['p']:02X} seed {c['seed']:X}"
                     f"{' mem ' + ' '.join('%04X=%02X' % m for m in c['mem']) if c['mem'] else ''}")
        lines.append(f"    the first {k} events agree; then")
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
    rep = os.path.join(a.report, name + ".txt")
    os.makedirs(a.report, exist_ok=True)
    open(rep, "w").write("\n".join(lines + cover_lines) + "\n")
    if a.quiet:
        print(f"{name:32s} match {len(same):4d}  differ {len(diffs):4d}  untraced {len(untraced):4d}  unreached lines {miss:3d}  "
              f"{'OK' if (not diffs and same) else 'DIFFERS' if diffs else 'NOTHING COMPARED'}")
        return (not diffs) and bool(same), dict(same=len(same), diffs=len(diffs), untraced=len(untraced), unreached=miss, ndraft=ndraft)
    print("\n".join(lines))
    if cover_lines:
        print(f"  coverage: {cover_lines[0]}; {[l for l in cover_lines if l.startswith('draft ')][0]}; {miss} lines not reached; full report {os.path.relpath(rep, TOP)}")
    return (not diffs) and bool(same), dict(same=len(same), diffs=len(diffs), untraced=len(untraced), unreached=miss, ndraft=ndraft)


def mask_p(line, ignore):
    m = re.search(r"P=([0-9A-F]{2})", line)
    if not m:
        return line
    return line[:m.start(1)] + "%02X" % (int(m.group(1), 16) & ~ignore & 0xFF) + line[m.end(1):]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("names", nargs="*")
    ap.add_argument("--file", help="a C file: every routine in it that has a draft, one line each")
    ap.add_argument("--drafts")
    ap.add_argument("--ops")
    ap.add_argument("--cases", type=int, default=400)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--curated")
    ap.add_argument("--lifted", help="a C file to use in place of the tree's file for the routine")
    ap.add_argument("--ignore-flags", default="", help="flag letters not compared at exit (NVDIZC)")
    ap.add_argument("--quiet", action="store_true", help=argparse.SUPPRESS)
    ap.add_argument("--no-cover", action="store_true")
    ap.add_argument("--warm", action="store_true", help="compile the game's C into the cache and stop")
    ap.add_argument("--show", type=int, default=3)
    ap.add_argument("--report", default=os.path.join(BUILD, "report"))
    a = ap.parse_args()
    os.makedirs(os.path.join(BUILD, "obj"), exist_ok=True)
    cache = os.path.join(BUILD, "obj")
    sources = game_sources()
    hh = header_hash()
    jobs = int(os.environ.get("CPU_RUN_JOBS") or 1)
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as ex:
        objs = dict(zip(sources, ex.map(lambda s: compile_obj(s, None, cache, hh), sources)))
    if a.warm:
        print(f"liftcmp: {len(sources)} files compiled into {os.path.relpath(cache, TOP)}")
        return 0
    if not a.names and not a.file:
        ap.error("name a routine or give --file")
    drafts = find_drafts(a.drafts)
    ctx = dict(cache=cache, sources=sources, objs=objs, hh=hh, drafts=drafts, ops=find_ops(drafts, a.ops), rows=hybrid_rows())
    bad = 0
    names = list(a.names)
    if a.file:
        path = os.path.join(TOP, a.file) if not os.path.isabs(a.file) else a.file
        have = {r["filed"] for r in csv.DictReader(open(os.path.join(drafts, "drafts.csv"))) if r["filed"]}
        names += [n for n in re.findall(r"^void abi_(\w+)\(void\)\s*$", open(path).read(), re.M) if n in have]
        a.quiet = True
        a.show = 0
        print(f"{a.file}: {len(names)} routines with a draft")
    for n in names:
        ok, _ = one(n, a, ctx)
        bad += not ok
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
