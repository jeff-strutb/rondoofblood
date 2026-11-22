"""cpuguard.py -- refuse to run a heavy tool outside the machine's CPU and memory queue.

    import cpuguard; cpuguard.require("ledger.py", jobs=1, mem="1G")

cpu-run sets CPU_RUN_JOBS (the slots it granted) for its command. A heavy tool called without it
prints the command to use and exits 2, so a stray run cannot take cores or memory outside the queue.
"""
import os
import sys


def cpu_run():
    """The queue command: CPU_RUN, else cpu-run on PATH, else the one in the user's tool folder."""
    import glob
    import shutil
    return (os.environ.get("CPU_RUN") or shutil.which("cpu-run") or
            next(iter(sorted(glob.glob(os.path.expanduser("~/.c*/bin/cpu-run")))), "cpu-run"))


def require(tool, jobs=1, mem="2G"):
    if os.environ.get("CPU_RUN_JOBS"):
        return
    args = " ".join(sys.argv[1:])
    print(f"{tool}: refusing to run outside the CPU queue (CPU_RUN_JOBS is not set). Run:\n"
          f"  {cpu_run()} -j {jobs} -m {mem} -- python3 tools/rondo/{tool} {args}".rstrip(),
          file=sys.stderr)
    raise SystemExit(2)


def core_problem(main):
    """None when main's core is at least as new as every patch under tools/toolchains/patches/pce, else the
    one line that says what to run. (Main's build fell behind main's source twice in one day after a patch landed;
    a measurement or a trace with an older core gives wrong values or none.) The core a run uses is RONDO_CORE
    when set, else main's own."""
    import glob
    core = os.environ.get("RONDO_CORE") or os.path.join(main, "tools", "toolchains", "pce", "mednafen_pce_libretro.dylib")
    patches = glob.glob(os.path.join(main, "tools", "toolchains", "patches", "pce", "*.patch"))
    if not os.path.exists(core):
        return f"core {core} does not exist: in {main} run: cpu-run -j 4 -m 6G -- sh setup.sh pce (then sh setup.sh pcebox)"
    if not patches:
        return None
    newest = max(patches, key=os.path.getmtime)
    if os.path.getmtime(core) < os.path.getmtime(newest):
        return (f"main's core ({core}) is older than the newest core patch ({os.path.basename(newest)}): in {main} "
                "run: cpu-run -j 4 -m 6G -- sh setup.sh pce, then sh setup.sh pcebox (a clean make; "
                "the core's Makefile does not track headers), then run this again")
    return None


if __name__ == "__main__":
    if len(sys.argv) == 3 and sys.argv[1] == "core":
        p = core_problem(sys.argv[2])
        if p:
            print(p)
        sys.exit(1 if p else 0)
