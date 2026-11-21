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
