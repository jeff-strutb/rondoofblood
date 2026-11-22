#!/usr/bin/env python3
"""served.py -- the drafts serving in place of routines whose filed C does not pass A5, and the box that does it.

A draft (cdraft.py) is register-shaped C made from the traced assembly, one per distinct content. The runner
measures A7 with drafts standing in for every routine whose filed C fails, so lockstep can be identical and the
routines whose own C is right can pass. The rule (the master's, PLAN.md section 1):

  * a filed routine whose C is A5-identical (all scripts, records.a5_verdict) keeps its C;
  * every other filed routine, and every content with no filed C, is served by its draft when the draft is itself
    A5-identical over the scripts it is called in (measured here: the box with every draft serving, A5 only);
  * a draft that is not A5-identical is left as original code, and counted.

The list the runner writes is config/rondo/serve_drafts.csv (name = the filed routine replaced). The box it builds
is a private tree with the chosen drafts compiled in and hybrid.py's own table (RONDO_DRAFTS), never this tree's
tracked files.

    python3 tools/rondo/served.py plan [--main DIR]       what the rules give from the records now (no run)
"""
import csv
import os
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)


def cache_dir(main):
    """The main checkout's draft cache when its stamp equals the inputs' (cdraft.current_cache), else None."""
    import cdraft
    d = cdraft.current_cache()
    return d


def filed_labels(cache, filed_names):
    """{filed routine name: [draft labels]} for the filed routines that have a draft (cdraft.resolve)."""
    import cdraft
    out = {}
    for name in filed_names:
        try:
            out[name] = [r["name"] for r in cdraft.resolve(cache, name)]
        except KeyError:
            pass
    return out


def read_drafts(cache):
    return list(csv.DictReader(open(os.path.join(cache, "drafts.csv"), newline="")))


def stamp(cache):
    return open(os.path.join(cache, "stamp.txt")).readline().strip()[:12]


def verdict_of(t):
    """a5 verdict of a {calls, identical, divergent, unverified, order_flags} dict."""
    import records
    return records.a5_verdict(t)


def plan(filed_a5, draft_a5, labels, drafts):
    """The serve decision.

    filed_a5   {filed name: counts dict}   the filed C's A5 over the scripts (records.a5_counts)
    draft_a5   {label: counts dict}        the drafts' A5 over the scripts
    labels     {filed name: [labels]}      which drafts stand for a filed routine
    drafts     drafts.csv rows

    Returns {"serve": {filed name: [labels]}, "free": [labels with no filed C that serve], "keep": filed names whose
    C stays, "original": [(name, why)] left as original code}.
    """
    out = {"serve": {}, "free": [], "keep": [], "original": []}
    claimed = {lab for labs in labels.values() for lab in labs}
    for name, labs in sorted(labels.items()):
        f = filed_a5.get(name)
        if f is not None and verdict_of(f) == "IDENTICAL":
            out["keep"].append(name)
            continue
        bad = [lab for lab in labs if verdict_of(draft_a5.get(lab, {"calls": 0})) != "IDENTICAL"
               and draft_a5.get(lab, {"calls": 0})["calls"]]
        if bad:
            out["original"].append((name, "draft " + ",".join(bad) + " is not A5-identical"))
        elif all(draft_a5.get(lab, {"calls": 0})["calls"] == 0 for lab in labs):
            out["original"].append((name, "its draft was never called (nothing to serve)"))
        else:
            out["serve"][name] = labs
    for r in drafts:
        lab = r["name"]
        if lab in claimed:
            continue
        t = draft_a5.get(lab)
        if not t or not t["calls"]:
            continue
        if verdict_of(t) == "IDENTICAL":
            out["free"].append(lab)
        else:
            out["original"].append((lab, "no filed C; its draft is " + verdict_of(t)))
    return out


def write_serve_list(path, serve):
    with open(path, "w", newline="\n") as f:
        f.write("name\n" + "".join(n + "\n" for n in sorted(serve)))


def make_tree(tree, dst, cache, drafts_rows, keep_labels, serve_names, filed_of):
    """A private copy of the runner's tree with the chosen drafts compiled in and the serve list; returns the
    RONDO_DRAFTS folder. keep_labels: the drafts that run (served ones and ones with no filed C); filed_of:
    {label: the filed routine it stands for in THIS tree, or ""}: the cache's own filed column is as old as the cache."""
    shutil.rmtree(dst, ignore_errors=True)
    shutil.copytree(tree, dst, symlinks=True)
    d = os.path.join(dst, "src", "rondo", "drafts")
    os.makedirs(d, exist_ok=True)
    for f in ("drafts.c", "support.c"):
        shutil.copy(os.path.join(cache, f), os.path.join(d, f))
    fields = ["name", "run", "length", "disc", "abi", "filed", "rom"]
    with open(os.path.join(d, "drafts.csv"), "w", newline="\n") as f:
        w = csv.DictWriter(f, fieldnames=fields, lineterminator="\n", extrasaction="ignore")
        w.writeheader()
        for r in drafts_rows:
            if r["name"] in keep_labels:
                w.writerow(dict(r, filed=filed_of.get(r["name"], "")))
    os.makedirs(os.path.join(dst, "config", "rondo"), exist_ok=True)
    write_serve_list(os.path.join(dst, "config", "rondo", "serve_drafts.csv"), serve_names)
    return d


def build_box(dst, draftdir, box):
    """setup.sh pcebox in the private tree with RONDO_DRAFTS: (ok, text)."""
    env = dict(os.environ, RONDO_BOX=box, RONDO_DRAFTS=draftdir)
    r = subprocess.run(["sh", os.path.join(dst, "setup.sh"), "pcebox"], env=env, capture_output=True, text=True)
    text = r.stdout + r.stderr
    if r.returncode or not os.path.exists(box):
        errs = [ln for ln in text.splitlines() if "error" in ln or "undefined" in ln]
        return False, "\n".join(errs[:20]) or text[-600:]
    return True, text[-200:]
