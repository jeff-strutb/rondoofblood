#!/usr/bin/env python3
"""Trace every script and merge what ran into one code map of the disc.

Runs each script in tools/rondo/scripts/ (names starting with `_` are
include-only) on pcebox with --trace, in parallel, each under a timeout,
into build/rondo/trace/<script>/. Then merges the per-run maps (an OR of
the flags) into build/rondo/trace/discmap.bin, cardmap.bin and noprov.bin.

Fails if any run fails, times out, or reports a RAM byte that differs from
the disc byte its provenance names.

    python3 tools/rondo/trace.py [SCRIPT ...]     default: every script
"""
import concurrent.futures
import glob
import os
import re
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import disc  # noqa: E402

PCEBOX = os.path.join(TOP, "build", "rondo", "pcebox", "pcebox")
CORE = os.path.join(TOP, "tools", "toolchains", "pce", "mednafen_pce_libretro.dylib")
SCRIPTS = os.path.join(HERE, "scripts")
OUT = os.path.join(TOP, "build", "rondo", "trace")
TIMEOUT = 600
MAPS = (("discmap.bin", 10149 * 2048), ("cardmap.bin", 1 << 18), ("noprov.bin", 1 << 21))


def run(script):
    name = os.path.splitext(os.path.basename(script))[0]
    out = os.path.join(OUT, name)
    os.makedirs(out, exist_ok=True)
    for m, _ in MAPS:
        p = os.path.join(out, m)
        if os.path.exists(p):
            os.remove(p)
    t = time.time()
    try:
        r = subprocess.run([PCEBOX, "--core", CORE, "--cue", os.path.join(disc.REF, disc.CUE),
                            "--system", disc.SYSTEM, "--script", script, "--out", out,
                            "--trace", os.path.join(disc.OUT, "track02.bin")],
                           capture_output=True, text=True, timeout=TIMEOUT)
    except subprocess.TimeoutExpired:
        return name, f"timed out after {TIMEOUT} s", None
    if r.returncode:
        return name, f"exit {r.returncode}: {r.stderr.strip()[-300:]}", None
    wrong = [int(m) for m in re.findall(r"(\d+) differ from the disc", r.stdout)]
    if any(wrong):
        return name, f"{max(wrong)} RAM bytes differ from their disc origin", None
    summary = next((l for l in r.stdout.splitlines() if l.startswith("trace:")), "")
    return name, None, f"{time.time() - t:.0f} s, {len(wrong)} dumps consistent; {summary[7:]}"


def main():
    scripts = sys.argv[1:] or sorted(p for p in glob.glob(os.path.join(SCRIPTS, "*.txt"))
                                     if not os.path.basename(p).startswith("_"))
    for need in (PCEBOX, CORE):
        if not os.path.exists(need):
            print(f"{need}: missing; run ./setup.sh")
            return 1
    bad = 0
    with concurrent.futures.ThreadPoolExecutor(max_workers=os.cpu_count()) as ex:
        for name, err, ok in ex.map(run, scripts):
            print(f"{name}: {err or ok}")
            bad += err is not None
    merged = {m: bytearray(n) for m, n in MAPS}
    for script in scripts:
        name = os.path.splitext(os.path.basename(script))[0]
        for m, n in MAPS:
            p = os.path.join(OUT, name, m)
            if os.path.exists(p):
                d = open(p, "rb").read()
                acc = merged[m]
                acc[:] = (int.from_bytes(acc, "little") | int.from_bytes(d, "little")).to_bytes(n, "little")
    for m, n in MAPS:
        open(os.path.join(OUT, m), "wb").write(merged[m])
    d = merged["discmap.bin"]
    code = sum(1 for v in d if v & 3)
    sectors = len({i // 2048 for i, v in enumerate(d) if v & 3})
    card = sum(1 for v in merged["cardmap.bin"] if v & 3)
    noprov = sum(1 for v in merged["noprov.bin"] if v & 3)
    print(f"merged: disc code {code} B in {sectors} sectors; System Card code {card} B; "
          f"RAM code with no disc origin {noprov} B")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
