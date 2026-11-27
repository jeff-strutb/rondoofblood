#!/usr/bin/env python3
"""accesscmp.py -- do two versions of a routine make the same accesses?

    python3 tools/rondo/accesscmp.py --old-ref REF --file src/rondo/system/video.c \
        --call 'uint8_t WriteBandScroll(uint8_t)' [--call ...] [--include routines.h]

For a routine lifted or renamed in a C file, compiles the file as it stands at REF (the old) and as it
stands in the tree (the new), each against stubs of the machine that log every rd8, wr8, tai, tam and
call_orig in order with its address and value, runs each --call with every value of its one
argument from 0 to 255, and compares the hash of the whole log and of the returns. Equal hashes mean
the same accesses, in the same order and number, with the same values, and the same results, for
each of the 256 inputs against one fixed memory image; no emulator is run. The values rd8 returns
depend on the address and on how many accesses have been made, so a routine that reads in a
different order gets different values and the hash shows it.

A call is `RET NAME(ARGT)`; ARGT is one integer type or void. A struct return needs a mix: append
` :: mix(r.field); mix(r.other);` with mix(x) folding one integer into the hash, r the result. A
plain integer return is mixed whole.

It proves a pure renaming or a lift only for the inputs it runs and the memory it is given; it does
not replace the gates.
Exit 0 when every call's hashes are equal, 1 when any differ, 2 on a setup error.
"""
import argparse
import os
import re
import subprocess
import sys
import tempfile

TOP = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

STUBS = r"""
#include <stdio.h>
#include <stdint.h>
#include "machine.h"
Cpu cpu;
static uint64_t h = 1469598103934665603ULL;
static unsigned n;
static void mix(uint64_t v) { h = (h ^ v) * 1099511628211ULL; }
uint8_t rd8(uint16_t a) { uint8_t v = (uint8_t)(a * 7u + n * 13u + (a >> 8)); n++; mix(1); mix(a); mix(v); return v; }
void wr8(uint16_t a, uint8_t v) { n++; mix(2); mix(a); mix(v); }
void tai(uint16_t s, uint16_t d, uint16_t l) { mix(3); mix(s); mix(d); mix(l); }
void tam(uint8_t m, uint8_t b) { mix(4); mix(m); mix(b); }
uint8_t tma(uint8_t m) { mix(5); mix(m); return 0; }
void call_original(uint16_t a) { mix(6); mix(a); }
void call_orig(uint16_t r, uint16_t a) { mix(7); mix(r); mix(a); }
void wait_interrupt(void) { mix(8); }
void call_return_point(void) {}
void st0(uint8_t v) { mix(9); mix(v); }
void st1(uint8_t v) { mix(10); mix(v); }
void st2(uint8_t v) { mix(11); mix(v); }
void cli(void) { mix(12); }
void csl(void) { mix(13); }
void csh(void) { mix(14); }
"""


def harness(calls, includes):
    out = [STUBS] + [f'#include "{i}"' for i in includes]
    for c in calls:
        out.append(f"{c['ret']} {c['name']}({c['arg']});")
    out.append("int main(void) {")
    for k, c in enumerate(calls):
        call = f"{c['name']}(" + ("(" + c["arg"] + ")i" if c["arg"] != "void" else "") + ")"
        out.append(f"  h = 1469598103934665603ULL;")
        out.append(f"  for (unsigned i = 0; i < 256; i++) {{ n = i * 31; cpu.s = 0xF0; mix(i);")
        if c["ret"] == "void":
            out.append(f"    {call};")
        elif c["mix"]:
            out.append(f"    {c['ret']} r = {call}; {c['mix']}")
        else:
            out.append(f"    mix((uint64_t){call});")
        out.append("  }")
        out.append(f'  printf("{c["name"]} %016llx\\n", (unsigned long long)h);')
    out.append("  return 0; }")
    return "\n".join(out) + "\n"


def parse_call(text):
    body, _, mix = text.partition("::")
    m = re.match(r"^\s*(.+?)\s+(\w+)\s*\(\s*(.*?)\s*\)\s*$", body)
    if not m:
        raise SystemExit(f"accesscmp: cannot read call {text!r}")
    return dict(ret=m.group(1), name=m.group(2), arg=m.group(3) or "void", mix=mix.strip())


def run(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, **kw)


def build_and_run(label, files, work, hsrc, tree):
    exe = os.path.join(work, label + ".bin")
    r = run(["cc", "-O2", "-w", "-I", os.path.join(tree, "src", "rondo", "include"),
             "-I", os.path.join(tree, "syscard", "include"),
             "-Wl,--unresolved-symbols=ignore-all", *files, hsrc, "-o", exe], cwd=tree)
    if r.returncode:
        raise SystemExit(f"accesscmp: {label} does not compile:\n{r.stderr[-800:]}")
    r = run([exe])
    if r.returncode:
        raise SystemExit(f"accesscmp: {label} run failed ({r.returncode}):\n{r.stderr[-400:]}")
    return dict(line.split() for line in r.stdout.splitlines())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--old-ref", required=True, help="git ref holding the old version")
    ap.add_argument("--file", action="append", required=True, help="C file, path from the repository root")
    ap.add_argument("--call", action="append", required=True)
    ap.add_argument("--include", action="append", default=[])
    a = ap.parse_args()
    calls = [parse_call(c) for c in a.call]
    with tempfile.TemporaryDirectory() as work:
        hsrc = os.path.join(work, "harness.c")
        with open(hsrc, "w") as f:
            f.write(harness(calls, a.include))
        old = []
        os.makedirs(os.path.join(work, "old"))
        for rel in a.file:
            p = os.path.join(work, "old", rel.replace("/", "__"))
            r = run(["git", "-C", TOP, "show", f"{a.old_ref}:{rel}"])
            if r.returncode:
                raise SystemExit(f"accesscmp: {rel} not at {a.old_ref}")
            open(p, "w").write(r.stdout)
            old.append(p)
        new = [os.path.join(TOP, rel) for rel in a.file]
        got_old = build_and_run("old", old, work, hsrc, TOP)
        got_new = build_and_run("new", new, work, hsrc, TOP)
    bad = 0
    for c in calls:
        same = got_old.get(c["name"]) == got_new.get(c["name"])
        bad += not same
        print(f"{c['name']}: old {got_old.get(c['name'])} new {got_new.get(c['name'])} {'same' if same else 'DIFFERENT'}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
