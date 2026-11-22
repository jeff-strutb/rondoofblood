#!/usr/bin/env python3
"""draftproof.py -- build a pcebox in which chosen filed routines are served by their drafts.

    cpu-run -j 4 -m 4G -- python3 tools/rondo/draftproof.py --names A,B,C [--tree DIR] [--box FILE]

Copies this work tree to DIR (build/rondo/drafter/tree), renames each named routine's hand-written
abi_ function to abi_NAME_hand, adds the drafts (cdraft.py --as-filed) in its place, and builds
pcebox there with its own routine table, so the existing hybrid.py and the oracles judge the drafts
beside the hand C. Nothing in this tree's tracked files is changed.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
BUILD = os.path.join(TOP, "build", "rondo", "drafter")


def sync(tree):
    os.makedirs(tree, exist_ok=True)
    subprocess.run(["rsync", "-a", "--delete", "--exclude", "/build", "--exclude", "/.git", "--exclude", "/references",
                    "--exclude", "/tools/toolchains/src", "--exclude", "/tools/toolchains/cc65", TOP + "/", tree + "/"],
                   check=True)
    for rel in ("references", "tools/toolchains/src", "tools/toolchains/cc65"):
        dst = os.path.join(tree, rel)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        if not os.path.lexists(dst):
            os.symlink(os.path.realpath(os.path.join(TOP, rel)), dst)
    os.makedirs(os.path.join(tree, "build", "rondo"), exist_ok=True)
    for rel in ("disc", "system", "trace"):
        dst = os.path.join(tree, "build", "rondo", rel)
        if not os.path.lexists(dst):
            os.symlink(os.path.realpath(os.path.join(TOP, "build", "rondo", rel)), dst)


def swap(tree, names, gen):
    """Hand abi_NAME out of the way, drafts in."""
    done = set()
    roots = [os.path.join(tree, "src", "rondo"), os.path.join(tree, "syscard", "src")]
    for root in roots:
        for dp, _, fs in os.walk(root):
            for fn in fs:
                if not fn.endswith(".c"):
                    continue
                path = os.path.join(dp, fn)
                text = open(path, encoding="utf-8").read()
                new, protos = text, []
                for n in names:
                    new, k = re.subn(r"^void abi_%s\(void\)\n\{" % n, "void abi_%s_hand(void)\n{" % n, new, flags=re.M)
                    if k:
                        protos.append("void abi_%s(void);\n" % n)
                        done.add(n)
                if new != text:
                    open(path, "w", encoding="utf-8").write("".join(protos) + new)
    d = os.path.join(tree, "src", "rondo", "drafts")
    os.makedirs(d, exist_ok=True)
    for f in ("drafts.c", "support.c"):
        shutil.copy(os.path.join(gen, f), os.path.join(d, f))
    return done


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--names", default="")
    ap.add_argument("--all", action="store_true", help="every content drafted, drafts serve every routine")
    ap.add_argument("--tree", default=os.path.join(BUILD, "tree"))
    ap.add_argument("--gen", default=os.path.join(BUILD, "gen_proof"))
    ap.add_argument("--box", default=os.path.join(BUILD, "pcebox"))
    a = ap.parse_args()
    names = [n for n in a.names.split(",") if n]
    cmd = [sys.executable, os.path.join(HERE, "cdraft.py"), "--out", a.gen]
    cmd += ["--all"] if a.all else ["--fresh", "--as-filed", "--only", ",".join(names)]
    r = subprocess.run(cmd)
    if r.returncode:
        return r.returncode
    sync(a.tree)
    env = dict(os.environ, RONDO_BOX=a.box)
    if a.all:
        # every filed routine is served by its draft: the serve list lives in the private tree only
        import csv
        d = os.path.join(a.tree, "src", "rondo", "drafts")
        os.makedirs(d, exist_ok=True)
        for f in ("drafts.c", "support.c", "drafts.csv"):
            shutil.copy(os.path.join(a.gen, f), os.path.join(d, f))
        filed = sorted({row["filed"] for row in csv.DictReader(open(os.path.join(a.gen, "drafts.csv"))) if row["filed"]})
        os.makedirs(os.path.join(a.tree, "config", "rondo"), exist_ok=True)
        with open(os.path.join(a.tree, "config", "rondo", "serve_drafts.csv"), "w") as f:
            f.write("name\n" + "\n".join(filed) + "\n")
        env["RONDO_DRAFTS"] = d
        print("draftproof: %d filed routines served by their drafts" % len(filed))
    else:
        done = swap(a.tree, names, a.gen)
        missing = [n for n in names if n not in done]
        if missing:
            print("draftproof: no hand-written abi_ function for", ",".join(missing))
    b = subprocess.run(["sh", os.path.join(a.tree, "setup.sh"), "pcebox"], env=env, capture_output=True, text=True)
    text = b.stdout + b.stderr
    errs = [ln for ln in text.splitlines() if "error" in ln or "undefined" in ln]
    print("\n".join(errs[:20]) if errs else text[-600:])
    return b.returncode


if __name__ == "__main__":
    sys.exit(main())
