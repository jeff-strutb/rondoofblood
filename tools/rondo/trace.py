#!/usr/bin/env python3
"""Trace every script and merge what ran into one code map of the disc.

Runs each script in tools/rondo/scripts/ (names starting with `_` are
include-only) on pcebox with --trace, in parallel, each under a timeout,
into build/rondo/trace/<script>/. Then merges the per-run maps (an OR of
the flags) of every script's last trace, whichever were run now, into
build/rondo/trace/discmap.bin, cardmap.bin and noprov.bin.

Fails if any run fails, times out, or reports a RAM byte that differs from
the disc byte its provenance names.

    python3 tools/rondo/trace.py [SCRIPT ...]     default: every script
"""
import concurrent.futures
import glob
import os
import re
import struct
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import disc  # noqa: E402

PCEBOX = os.path.join(TOP, "build", "rondo", "pcebox", "pcebox")
CORE = os.environ.get("RONDO_CORE") or os.path.join(TOP, "tools", "toolchains", "pce", "mednafen_pce_libretro.dylib")
SCRIPTS = os.path.join(HERE, "scripts")
OUT = os.path.join(TOP, "build", "rondo", "trace")
TIMEOUT = 600
MAPS = (("discmap.bin", 10149 * 2048), ("cardmap.bin", 1 << 18), ("noprov.bin", 1 << 21),
        ("discmap2.bin", 10149 * 2048))   # discmap2: what each disc byte is (pcebox/trace.c)


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
    if not os.environ.get("CPU_RUN_JOBS"):
        print("refusing to run outside the slot queue (CPU_RUN_JOBS is not set); run it as:\n"
              "  cpu-run -j N -m SIZE -- python3 tools/rondo/trace.py ...")
        return 2
    scripts = sys.argv[1:] or sorted(p for p in glob.glob(os.path.join(SCRIPTS, "*.txt"))
                                     if not os.path.basename(p).startswith("_"))
    for need in (PCEBOX, CORE):
        if not os.path.exists(need):
            print(f"{need}: missing; run ./setup.sh")
            return 1
    bad = 0
    with concurrent.futures.ThreadPoolExecutor(max_workers=int(os.environ["CPU_RUN_JOBS"])) as ex:
        for name, err, ok in ex.map(run, scripts):
            print(f"{name}: {err or ok}")
            bad += err is not None
    # The merged maps cover every script's last trace, not only the ones run now.
    merged = {m: bytearray(n) for m, n in MAPS}
    every = sorted(p for p in glob.glob(os.path.join(SCRIPTS, "*.txt")) if not os.path.basename(p).startswith("_"))
    for script in every:
        name = os.path.splitext(os.path.basename(script))[0]
        for m, n in MAPS:
            p = os.path.join(OUT, name, m)
            if os.path.exists(p):
                d = open(p, "rb").read()
                acc = merged[m]
                acc[:] = (int.from_bytes(acc, "little") | int.from_bytes(d, "little")).to_bytes(n, "little")
    for m, n in MAPS:
        open(os.path.join(OUT, m), "wb").write(merged[m])
    # runmap.bin: the distinct (disc offset, logical address, bank, kind, mapper configuration) records of every
    # script, sorted, with the configurations (mprcfg.csv) numbered afresh; reads.csv: every script's read runs.
    cfg_ids, recs, rows, header = {}, set(), set(), None
    for script in every:
        name = os.path.splitext(os.path.basename(script))[0]
        p = os.path.join(OUT, name, "runmap.bin")
        if os.path.exists(p):
            mine = {}
            for line in open(os.path.join(OUT, name, "mprcfg.csv")).read().splitlines()[1:]:
                f = line.split(",")
                mine[int(f[0])] = tuple(f[1:])
            d = open(p, "rb").read()
            for i in range(0, len(d), 12):
                disc_off, logical, bank, kind, cfg, _ = struct.unpack("<IHBBHH", d[i:i + 12])
                gid = cfg_ids.setdefault(mine[cfg], len(cfg_ids))
                recs.add((disc_off, logical, bank, kind, gid))
        p = os.path.join(OUT, name, "reads.csv")
        if os.path.exists(p):
            lines = open(p).read().splitlines()
            header = lines[0]
            rows.update(lines[1:])
    with open(os.path.join(OUT, "runmap.bin"), "wb") as f:
        for r in sorted(recs):
            f.write(struct.pack("<IHBBHH", *r, 0))
    with open(os.path.join(OUT, "mprcfg.csv"), "w") as f:
        f.write("id,mpr2,mpr3,mpr4,mpr5,mpr6\n")
        for cfg, gid in sorted(cfg_ids.items(), key=lambda kv: kv[1]):
            f.write(f"{gid},{','.join(cfg)}\n")
    if header:
        with open(os.path.join(OUT, "reads.csv"), "w") as f:
            f.write(header + "\n" + "\n".join(sorted(rows)) + "\n")
    d = merged["discmap.bin"]
    code = sum(1 for v in d if v & 3)
    sectors = len({i // 2048 for i, v in enumerate(d) if v & 3})
    card = sum(1 for v in merged["cardmap.bin"] if v & 3)
    noprov = sum(1 for v in merged["noprov.bin"] if v & 3)
    m2 = merged["discmap2.bin"]
    print("merged data: " + ", ".join(f"{name} {sum(1 for v in m2 if v & bit)}" for name, bit in (
        ("loaded", 1), ("read by code", 2), ("sent to VDC", 4), ("VCE", 8), ("PSG", 16), ("ADPCM", 32),
        ("other port", 64), ("block moved", 128))) + f"; {len(recs)} run records, {len(cfg_ids)} mapper configurations")
    print(f"merged: disc code {code} B in {sectors} sectors; System Card code {card} B; "
          f"RAM code with no disc origin {noprov} B")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
