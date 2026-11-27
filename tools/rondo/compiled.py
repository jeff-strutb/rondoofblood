#!/usr/bin/env python3
"""compiled.py -- what the compiler produced for each routine with C, as one hash per routine.

    python3 tools/rondo/compiled.py [--tree DIR] [--out FILE] [NAME ...]

A stored verdict stays true while the routine's compiled C is the same, so the runner (runner.py)
compares these hashes, never file times or text. For each routine (the game's and the card's, from
the tree's build/rondo/pcebox/hybrid_table.c, which setup.sh pcebox writes):

  * every C file is compiled with the pcebox flags and -ffunction-sections -fdata-sections, and each
    function and data object is read back with objdump: instructions without their offsets, and the
    relocations, which name what the code calls or reads;
  * the closure runs from the routine and its abi_ wrapper over those relocations (calls, function
    pointers, tables), so a callee's change changes every routine that can reach it, and nothing else;
  * the hash covers the closure's code and data and the routine's own table row (run address,
    length, disc offset, end): the extent the oracles judge.

Each routine also gets an OWN hash: the same, over the routine's own code only (its function and abi_
wrapper, the helpers and data only they use, and its table row) and not over the code of other
routines it calls. A callee that is a routine of its own is not part of the caller's own code; an
inlined one is, because inlining changes the caller's own bytes (the hybrid is built at -O2, so the
compiler inlines small static callees and this shows as a changed OWN hash of the caller). With
--out FILE a second file FILE.nodes lists every node's hash and, per routine, its closure and own
set; records.py needs it to say which scripts a change can affect (rule in records.py).

Comments, local names, the order of functions in a file and unrelated functions change nothing. A
file's merged constants (strings, small literals) count for every function that uses any of them,
which can only re-test too much. The checker, harness, hybrid.py and setup.sh are not in the hash:
a checker fix re-measures only the routines it is told to (runner.py --routines).
Output: NAME HASH RUN LENGTH DISC ROM OWN per line (ROM 1: the card). Compiles on every core (a few seconds).
"""
import collections
import concurrent.futures
import glob
import hashlib
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

FLAGS = ["-O2", "-finstrument-functions",
         "-finstrument-functions-exclude-file-list=tools/rondo/pcebox/,build/rondo/pcebox/",
         "-ffunction-sections", "-fdata-sections", "-w", "-c"]
SECTION = re.compile(r"^Disassembly of section (\S+):$")
RELOC = re.compile(r"^\s+[0-9a-f]+:\s+R_X86_64_\w+\s+(\S+?)(?:[+-]0x[0-9a-f]+)?$")
ROW = re.compile(r'^\s*\{ (0x[0-9A-F]+), (\d+), (\d+), "(\w+)", abi_\w+, \d+, (0x[0-9A-F]+), (\d) \},$', re.M)


def tool(args, **kw):
    return subprocess.run(args, capture_output=True, text=True, **kw)


def read_object(path):
    """{section: (hash of its code or contents, [symbols it refers to])} for one object file."""
    nodes = {}
    # code: instructions without offsets, relocations inline
    cur = None
    for line in tool(["objdump", "-dr", "--no-show-raw-insn", path]).stdout.splitlines():
        m = SECTION.match(line)
        if m:
            cur = m.group(1)
            nodes[cur] = [hashlib.sha256(), []]
            continue
        if cur is None or not line.strip() or line.startswith("0000"):
            continue
        r = RELOC.match(line)
        if r:
            nodes[cur][1].append(r.group(1))
            nodes[cur][0].update(re.sub(r"^\s+[0-9a-f]+:", "", line).strip().encode() + b"\n")
            continue
        body = re.sub(r"^\s*[0-9a-f]+:\s*", "", line)
        body = re.sub(r"\s*#.*$|\s*<[^>]*>$", "", body)
        nodes[cur][0].update(body.encode() + b"\n")
    # data: contents and the relocations inside them (pointer tables)
    cur = None
    for line in tool(["objdump", "-s", path]).stdout.splitlines():
        m = re.match(r"^Contents of section (\S+):$", line)
        if m:
            cur = m.group(1)
            nodes.setdefault(cur, [hashlib.sha256(), []])
        elif cur and line.startswith(" ") and cur in nodes:
            nodes[cur][0].update(line[6:].encode() + b"\n")
    cur = None
    for line in tool(["objdump", "-r", path]).stdout.splitlines():
        m = re.match(r"^RELOCATION RECORDS FOR \[(\S+)\]:", line)
        if m:
            cur = m.group(1)
        elif cur and not cur.startswith(".text") and cur in nodes:
            r = RELOC.match(line)
            if r:
                nodes[cur][1].append(r.group(1))
                nodes[cur][0].update(line.split(None, 1)[1].encode() + b"\n")
    return {k: (v[0].hexdigest(), v[1]) for k, v in nodes.items()}


def compile_all(tree, work):
    tc = os.path.join(tree, "tools", "toolchains")
    incs = ["-I", os.path.join(tc, "src", "pce", "libretro-common", "include"),
            "-I", os.path.join(tree, "build", "rondo", "pcebox"),
            "-I", os.path.join(tree, "src", "rondo", "include"), "-I", os.path.join(tree, "syscard", "include")]
    files = sorted(glob.glob(os.path.join(tree, "src", "rondo", "*", "*.c")) +
                   glob.glob(os.path.join(tree, "syscard", "src", "*.c")))
    os.makedirs(work, exist_ok=True)

    def one(path):
        rel = os.path.relpath(path, tree)
        obj = os.path.join(work, rel.replace(os.sep, "__") + ".o")
        r = tool(["cc", *FLAGS, *incs, path, "-o", obj], cwd=tree)
        if r.returncode:
            raise SystemExit(f"compiled.py: {rel} does not compile:\n{r.stderr[-600:]}")
        return rel, read_object(obj)

    with concurrent.futures.ThreadPoolExecutor(max_workers=os.cpu_count() or 1) as ex:
        return dict(ex.map(one, files))


def routine_rows(tree):
    text = open(os.path.join(tree, "build", "rondo", "pcebox", "hybrid_table.c")).read()
    return {m.group(4): m.group(0).strip() for m in ROW.finditer(text)}


def extents(tree):
    """{routine: (run, length, disc offset, rom)}: where its bytes are, from the tree's hybrid table."""
    return {m.group(4): (int(m.group(1), 16), int(m.group(2)), int(m.group(3)), int(m.group(6)))
            for m in ROW.finditer(open(os.path.join(tree, "build", "rondo", "pcebox", "hybrid_table.c")).read())}


class Compiled(dict):
    """{routine: closure hash} (a dict, as before) that also carries the graph the refined rule needs:
    nodes [(name, hash)], closure {routine: set of node indexes}, own {routine: set of node indexes},
    own_hash and row_hash {routine: str}, owners {node index: set of routines}. graph is False when the
    object came from a file without .nodes (then records.py judges every script by closure)."""
    graph = False

    def set_graph(self, nodes, closure, own, own_hash, row_hash):
        self.graph = True
        self.nodes, self.closure, self.own, self.own_hash, self.row_hash = nodes, closure, own, own_hash, row_hash
        self.owners = {}
        for r, idx in own.items():
            for i in idx:
                self.owners.setdefault(i, set()).add(r)


def load(path):
    """(chash, extents) from a file written by main(): NAME HASH RUN LEN DISC ROM OWN per line, and the
    graph from PATH.nodes when it exists."""
    chash, ext = Compiled(), {}
    for line in open(path):
        n, h, run, length, disc, rom = line.split()[:6]
        chash[n], ext[n] = h, (int(run), int(length), int(disc), int(rom))
    nodes_path = path + ".nodes"
    if os.path.exists(nodes_path):
        nodes, closure, own, own_hash, row_hash = [], {}, {}, {}, {}
        for line in open(nodes_path):
            f = line.rstrip("\n").split("\t")
            if f[0] == "N":
                nodes.append((f[1], f[2]))
            else:
                ints = lambda x: {int(i) for i in x.split(",") if i}
                closure[f[1]], own[f[1]] = ints(f[4]), ints(f[5])
                row_hash[f[1]], own_hash[f[1]] = f[2], f[3]
        chash.set_graph(nodes, closure, own, own_hash, row_hash)
    chash.ext = ext
    return chash, ext


def compute(tree):
    """(chash, extents) for the tree: hybrid_table.c must be current (setup.sh pcebox, or hybrid.py)."""
    got, ext = hashes(tree, os.path.join(tree, "build", "rondo", "compiled")), extents(tree)
    got.ext = ext
    return got, ext


def write_nodes(got, path):
    """FILE.nodes: N <name> <hash> per node, R <routine> <row hash> <own hash> <closure> <own set> per routine."""
    with open(path, "w") as f:
        for name, h in got.nodes:
            f.write(f"N\t{name}\t{h}\n")
        for r in sorted(got.closure):
            f.write("R\t%s\t%s\t%s\t%s\t%s\n" % (r, got.row_hash[r], got.own_hash[r],
                    ",".join(map(str, sorted(got.closure[r]))), ",".join(map(str, sorted(got.own[r])))))


def hashes(tree, work, names=None):
    objs = compile_all(tree, work)
    nodes = {(f, s): v for f, secs in objs.items() for s, v in secs.items()}
    where = collections.defaultdict(list)          # global name -> [(file, section)]
    for (f, s) in nodes:
        m = re.match(r"\.(?:text|data|rodata|bss)\.(\w+)$", s)
        if m:
            where[m.group(1)].append((f, s))

    def resolve(f, sym):
        if (f, sym) in nodes:                       # a section symbol of this file
            return (f, sym)
        for kind in ("text", "data", "rodata", "bss"):
            if (f, f".{kind}.{sym}") in nodes:      # a name defined in this file
                return (f, f".{kind}.{sym}")
        found = where.get(sym)
        return found[0] if found else None

    rows = routine_rows(tree)
    root_of = {}                                     # a routine's function and wrapper nodes -> the routine
    roots_of = {}
    for name in rows:
        roots_of[name] = [n for sym in (name, "abi_" + name) for n in where.get(sym, [])
                          if n[1].startswith(".text.")]
        for n in roots_of[name]:
            root_of[n] = name

    def walk(name, private):
        """The nodes reachable from the routine's roots. private: do not enter the root of another routine
        (what is left is the routine's own code, helpers and data included)."""
        seen, stack = set(), list(roots_of[name])
        while stack:
            n = stack.pop()
            if n in seen:
                continue
            seen.add(n)
            for sym in nodes[n][1]:
                t = resolve(n[0], sym)
                if t and t not in seen and not (private and root_of.get(t, name) != name):
                    stack.append(t)
        return seen

    out = Compiled()
    closure, own, own_hash, row_hash = {}, {}, {}, {}
    for name, row in rows.items():
        if names and name not in names:
            continue
        seen = walk(name, False)
        h = hashlib.sha256(row.encode())
        for n in sorted(seen):
            h.update(n[1].encode() + b"=" + nodes[n][0].encode() + b"\n")
        out[name] = h.hexdigest()[:16] if roots_of[name] else "noc"
        if roots_of[name]:
            mine = walk(name, True)
            oh = hashlib.sha256(row.encode())
            for n in sorted(mine):
                oh.update(n[1].encode() + b"=" + nodes[n][0].encode() + b"\n")
            closure[name], own[name] = seen, mine
            own_hash[name], row_hash[name] = oh.hexdigest()[:16], hashlib.sha256(row.encode()).hexdigest()[:16]
    used = sorted({n for s_ in closure.values() for n in s_})
    index = {n: i for i, n in enumerate(used)}
    out.set_graph([(f"{n[0]}:{n[1]}", nodes[n][0][:16]) for n in used],
                  {r: {index[n] for n in s_} for r, s_ in closure.items()},
                  {r: {index[n] for n in s_} for r, s_ in own.items()}, own_hash, row_hash)
    return out


def main():
    args = sys.argv[1:]
    tree = os.getcwd()
    outfile = None
    if "--tree" in args:
        i = args.index("--tree")
        tree = os.path.abspath(args[i + 1])
        del args[i:i + 2]
    if "--out" in args:
        i = args.index("--out")
        outfile = args[i + 1]
        del args[i:i + 2]
    work = os.path.join(tree, "build", "rondo", "compiled")
    got = hashes(tree, work, set(args) or None)
    ext = extents(tree)
    text = "".join(f"{k} {v} {ext[k][0]} {ext[k][1]} {ext[k][2]} {ext[k][3]} {got.own_hash.get(k, 'noc')}\n"
                   for k, v in sorted(got.items()))
    if outfile:
        open(outfile, "w").write(text)
        write_nodes(got, outfile + ".nodes")
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
