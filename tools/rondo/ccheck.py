#!/usr/bin/env python3
"""ccheck.py -- every C file of the game and the System Card compiles.

Each file under src/rondo/<area>/ and syscard/src/ is compiled on its own with the host's C compiler, to an
object file that is then thrown away: nothing is linked and nothing is run. C that does not compile cannot do
what its assembly does, so a routine is not finished until its file passes here.

It fails on any error, and on the warnings that mean a real fault rather than style: a function used with no
declaration, a function that can end without returning its value, an integer and a pointer mixed up, and a
variable read before anything is stored in it. The
other warnings are counted and listed, and do not fail.

    python3 tools/rondo/ccheck.py            every file; exit 1 if any fails
    python3 tools/rondo/ccheck.py FILE...    only these files
"""
import collections
import glob
import os
import re
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
CC = os.environ.get("CC", "cc")
FLAGS = ["-std=c11", "-O2", "-Wall", "-Wextra",
         "-Werror=implicit-function-declaration", "-Werror=return-type", "-Werror=int-conversion",
         "-Werror=incompatible-pointer-types", "-Werror=uninitialized", "-Werror=sometimes-uninitialized",
         "-I", os.path.join(TOP, "src", "rondo", "include"), "-I", os.path.join(TOP, "syscard", "include")]
WARNING = re.compile(r"warning: .*\[(-W[\w-]+)\]")


def files(argv):
    if argv:
        return [os.path.abspath(a) for a in argv]
    return sorted(glob.glob(os.path.join(TOP, "src", "rondo", "*", "*.c")) +
                  glob.glob(os.path.join(TOP, "syscard", "src", "*.c")))


def compile_one(path, out):
    obj = os.path.join(out, os.path.relpath(path, TOP).replace(os.sep, "_") + ".o")
    r = subprocess.run([CC] + FLAGS + ["-c", path, "-o", obj], capture_output=True, text=True, timeout=300)
    return path, r.returncode, r.stderr


def main():
    todo = files(sys.argv[1:])
    failed, warns = [], collections.Counter()
    with tempfile.TemporaryDirectory() as out, ThreadPoolExecutor(os.cpu_count() or 4) as pool:
        for path, rc, err in pool.map(lambda p: compile_one(p, out), todo):
            warns.update(WARNING.findall(err))
            if rc:
                failed.append(path)
                print(err.rstrip())
    rel = lambda p: os.path.relpath(p, TOP)
    print(f"ccheck: {len(todo)} files, {len(failed)} failed; {sum(warns.values())} other warnings"
          + (": " + ", ".join(f"{k} {v}" for k, v in warns.most_common()) if warns else ""))
    for p in failed:
        print(f"  FAILED {rel(p)}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
