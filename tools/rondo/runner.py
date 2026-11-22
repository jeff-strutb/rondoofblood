#!/usr/bin/env python3
"""runner.py -- the one process that certifies. Writers do not run gates.

    python3 tools/rondo/runner.py once  [--main DIR] [options]    certify what main's tip needs, then stop
    python3 tools/rondo/runner.py watch [--main DIR] [options]    the same whenever the tip moves (every 60 s)
    python3 tools/rondo/runner.py plan  [--main DIR] [options]    print what would run and the time expected
    python3 tools/rondo/runner.py commit [--main DIR]             commit the records, views and ledgers as they stand
                                                                  in DIR (under the landing lock, explicit paths)
    options: --routines A,B   re-measure the scripts reaching these routines (after a checker change)
             --scripts a,b    re-measure these scripts
             --only           with --scripts: nothing else (a partial check; no ledger is written)
             --sweep          every script: the whole-game run, only when asked
             --commit         commit the records and ledgers to DIR's branch under the landing lock (off by
                              default until the state session has handed those files over)
             --norerun        do not rerun red scripts alone (when every script is expected red)
             --workers N      scripts measured at once (default 8; each is one cpu-run claim)

What it does for main's tip (design: build/rondo/pipeline/DESIGN.md, the rule: records.py):
  1. exports the tip into its own private tree (git archive), links the untracked inputs, builds its own
     pcebox there, and takes compiled.py's hash of every routine with C: what the compiler produced;
  2. plans: a script is measured when it is new or its text changed, or the compiled hash of a routine it
     reaches changed (a new routine, a changed one, a callee of one). Nothing else plans anything: not
     a checker or harness change, not a commit elsewhere. The plan prints with the time it should take;
  3. measures each planned script alone: A5 then A7 (oracle_io.py), one cpu-run claim of 1 core and 2 GB
     each, in a private output folder, with the private pcebox; at most --workers waiting or running;
  4. reruns every red script (A7 not identical, A5 divergent) alone, nothing else of ours running, and
     records the second result; a flip is logged as a flake;
  5. writes the records, then the ledger (only if every script has a result), and commits them under the
     landing lock with explicit paths. It never holds or edits C: failures are reported (reds.txt, the log).
A script run that fails (exit, timeout) leaves that script's old record in place and no ledger is written.
Everything it makes is under build/rondo/runner/ in the main checkout.
"""
import concurrent.futures
import datetime
import fcntl
import json
import os
import re
import shutil
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import cpuguard  # noqa: E402
import records  # noqa: E402

LINKS = ("references", "tools/toolchains/src", "tools/toolchains/cc65", "tools/toolchains/pce",
         "build/rondo/disc", "build/rondo/system", "build/rondo/trace")
OWNED = ("config/rondo/runs", "config/rondo/gate_a5.csv", "config/rondo/gate_a7.csv",
         "config/rondo/gate_a7_routines.csv", "docs/rondo/ledger.csv", "syscard/ledger.csv")
SCRIPT_TIMEOUT = 7200
GUESS = 150.0          # seconds of A5 plus A7 for a script never measured: play_s0_r1 took 123 s and attract_82 174 s


def log(msg, d=None):
    line = f"{datetime.datetime.now():%F %T} {msg}"
    print(line, flush=True)
    if d:
        with open(os.path.join(d, "runner.log"), "a") as f:
            f.write(line + "\n")


def git(main, *a):
    return subprocess.run(["git", "-C", main, *a], capture_output=True, text=True)


def cpu_run(jobs, mem, *cmd, **kw):
    return subprocess.run([cpuguard.cpu_run(), "-j", str(jobs), "-m", mem, "--", *cmd], capture_output=True,
                          text=True, **kw)


def export(main, rev, b):
    tree = os.path.join(b, "tree-" + rev)
    shutil.rmtree(tree, ignore_errors=True)
    os.makedirs(tree)
    a = subprocess.Popen(["git", "-C", main, "archive", rev], stdout=subprocess.PIPE)
    subprocess.run(["tar", "-x", "-C", tree], stdin=a.stdout, check=True)
    a.wait()
    for rel in LINKS:
        src = os.path.realpath(os.path.join(main, rel))
        if not os.path.exists(src):
            raise SystemExit(f"{rel}: missing in {main}")
        dst = os.path.join(tree, rel)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        if os.path.lexists(dst):
            shutil.rmtree(dst) if os.path.isdir(dst) and not os.path.islink(dst) else os.remove(dst)
        os.symlink(src, dst)
    return tree


def prepare(main, rev, b, d):
    """The private tree, its own pcebox, and the compiled hashes: (tree, box, chash, extents)."""
    tree = export(main, rev, b)
    box = os.path.join(b, "pcebox-" + rev)
    r = cpu_run(1, "2G", "env", "RONDO_BOX=" + box, "sh", os.path.join(tree, "setup.sh"), "pcebox")
    if r.returncode:
        raise SystemExit(f"pcebox build of {rev} failed:\n{(r.stdout + r.stderr)[-800:]}")
    gen = os.path.join(tree, "build", "rondo", "pcebox")      # setup.sh puts a private build's generated files beside the binary
    os.makedirs(gen, exist_ok=True)
    for f in ("hybrid_table.c", "opclass.h"):
        shutil.copy(os.path.join(box + ".gen", f), os.path.join(gen, f))
    out = os.path.join(tree, "chash.txt")
    r = cpu_run(2, "3G", sys.executable, os.path.join(tree, "tools", "rondo", "compiled.py"), "--tree", tree,
                "--out", out)
    if r.returncode:
        raise SystemExit(f"compiled.py failed on {rev}:\n{(r.stdout + r.stderr)[-800:]}")
    import compiled
    chash, ext = compiled.load(out)
    return tree, box, chash, ext


def estimate(script, rec, top):
    row = rec.scripts.get(script)
    if row and row.get("a5_secs"):
        return float(row["a5_secs"]) + float(row["a7_secs"])
    return GUESS


def make_plan(rec, chash, ext, opts, top):
    """({script: the fact its run establishes}, the routines whose own code changed or None). Every planned run
    names the new fact it yields (a run that yields none is not planned)."""
    plan = {}
    names = records.script_names(top)
    changed = changed_routines(chash, top)
    for s in names:
        st = records.status(s, chash, ext, rec, top)
        if opts["sweep"]:
            plan[s] = "whole-game run asked for: every verdict re-established"
        elif st == "unmeasured":
            plan[s] = f"first result for {s}'s present text: A5 counts and the A7 verdict, none stored"
        elif st != "current":
            hit = [r for r in (changed or []) if r in records.reached(s, ext, rec, top)]
            what = ", ".join(hit[:4]) + (f" and {len(hit) - 4} more" if len(hit) > 4 else "") if hit else \
                "a routine in its stored closure"
            plan[s] = f"whether {what} still passes in {s}: its stored result no longer counts"
    if opts["only"]:
        plan = {}           # --only: just the named scripts, whatever else is stale (a partial check)
    for s in opts["scripts"]:
        if s not in names:
            raise SystemExit(f"{s}: no such script")
        plan.setdefault(s, f"named: {s} re-measured on request")
    for r in opts["routines"]:
        if r not in ext:
            raise SystemExit(f"{r}: no routine with C")
        for s in names:
            if records.reach(s, r, ext, rec, top):
                plan.setdefault(s, f"named routine {r}: its verdict in {s} re-established on request")
    return plan, changed


def changed_routines(chash, top):
    """The routines whose own compiled code differs from the last run's (own.txt), or None if not known."""
    path = os.path.join(top, "build", "rondo", "runner", "own.txt")
    if not os.path.exists(path) or not getattr(chash, "graph", False):
        return None
    old = dict(line.split() for line in open(path))
    return sorted(r for r, h in chash.own_hash.items() if old.get(r) != h)


def measure(tree, box, out, script):
    r = cpu_run(1, "2G", sys.executable, os.path.join(tree, "tools", "rondo", "oracle_io.py"), "job", tree,
                script, os.path.join(out, script), box, timeout=SCRIPT_TIMEOUT)
    path = os.path.join(out, script, "result.json")
    if not os.path.exists(path):
        return {"script": script, "errors": ["no result: " + (r.stdout + r.stderr).strip()[-300:]]}
    return json.load(open(path))


def red(res):
    """A script with a result is red when its A7 is not identical or A5 found a defect (a divergent or
    order-flagged call): the script is then run again alone."""
    return bool(res.get("errors")) is False and (
        res["a7"].get("verdict") != "IDENTICAL" or any(v["divergent"] or v["order_flags"] for v in res["a5"].values()))


def pack(d):
    """{KIND: n} as 'KIND=n;KIND=n' (only the non-zero ones)."""
    return ";".join(f"{k}={n}" for k, n in sorted(d.items()) if n)


def store(rec, res, rev, chash, ext, top):
    s = res["script"]
    ep = records.epoch(top)
    a5 = [{"script": s, "routine": r, **{k: v[k] for k in ("calls", "identical", "divergent", "unverified",
           "order_flags")}, "chash": "", "detail": v["detail"], "epoch": ep, "source": rev,
           "defects": pack(v.get("defects", {})), "causes": pack(v.get("causes", {}))}
          for r, v in sorted(res["a5"].items())]
    defects = res.get("a7_defects", {})
    a7 = [{"script": s, "routine": r, "swapped": res["reach"].get(r, [0, 0])[0],
           "ran_inside": res["reach"].get(r, [0, 0])[1], "chash": "", "epoch": ep, "defects": pack(defects.get(r, {}))}
          for r in sorted(set(res["reach"]) | set(defects))]
    f = res["a7"]["fields"]
    row = {"script": s, "script_hash": records.script_hash(s, top), "verdict": res["a7"]["verdict"],
           "detail": res["a7"]["detail"], "a5_secs": res["secs"]["a5"], "a7_secs": res["secs"]["a7"],
           "world": "", "epoch": ep, "source": rev, "a7_code": res["a7"]["code"],
           "a7_frames": f"{f['compared']}/{f['expected']}", "a7_defects": f["defects"],
           "a7_open": f"{f['parked']}/{f['kept']}/{f['unreached']}/{f['unopened']}"}
    rec.replace_script(s, row, a5, a7)
    for x in a5 + a7:                       # the key needs the script's stored rows and verdict, so after
        x["chash"] = records.row_key(s, x["routine"], chash, rec, top) or "noc"
    row["world"] = records.world(s, chash, ext, rec, top)


def cycle(main, opts):
    b = os.path.join(main, "build", "rondo", "runner")
    os.makedirs(b, exist_ok=True)
    lock = open(os.path.join(b, "runner.lock"), "w")
    try:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except OSError:
        print("runner: another runner holds " + lock.name)
        return 2
    rev = git(main, "rev-parse", "--short=7", "HEAD").stdout.strip()
    problem = cpuguard.core_problem(main)
    if problem:
        print("runner: REFUSED: " + problem)
        return 3
    tree, box, chash, ext = prepare(main, rev, b, b)
    rec = records.Records(main)
    plan, changed = make_plan(rec, chash, ext, opts, main)
    names = records.script_names(main)
    secs = {s: estimate(s, rec, main) for s in plan}
    full = sum(estimate(s, rec, main) for s in names)
    workers = max(1, min(opts["workers"], len(plan)))
    log(f"{rev}: " + ("no previous own hashes to compare; " if changed is None else
        f"{len(changed)} routine(s) with changed own code" + (": " + ", ".join(changed[:12]) if changed else "") + "; ")
        + f"{len(plan)} of {len(names)} scripts to measure", b)
    for s, why in sorted(plan.items()):
        log(f"  {s}: establishes {why}; about {secs[s]:.0f} s of one core", b)
    log(f"  expected {sum(secs.values()):.0f} core-seconds, about {sum(secs.values()) / workers:.0f} s"
        f" on {workers} at once; the whole game would be {full:.0f} core-seconds (all {len(names)} scripts)", b)
    if opts["mode"] == "plan" or not plan:
        if not plan:
            log("  nothing to measure", b)
        return 0
    out = os.path.join(b, "out-" + rev)
    shutil.rmtree(out, ignore_errors=True)
    t0 = time.time()
    results = {}
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as ex:
        for res in ex.map(lambda s: measure(tree, box, out, s), sorted(plan)):
            results[res["script"]] = res
            log(f"  measured {res['script']}: " + ("FAILED " + "; ".join(res["errors"]) if res.get("errors") else
                f"A7 {res['a7']['verdict']}, {len(res['a5'])} routines called"), b)
    for s in sorted(results):
        if red(results[s]) and not opts["norerun"]:
            log(f"  {s} is red: rerun alone", b)
            second = measure(tree, box, out, s)
            flipped = not red(second) if not second.get("errors") else False
            log(f"  {s} rerun: " + ("FAILED " + "; ".join(second["errors"]) if second.get("errors") else
                "now green (a flake)" if flipped else "red again"), b)
            if not second.get("errors"):
                results[s] = second
    failed = sorted(s for s, r in results.items() if r.get("errors"))
    for s in sorted(results):
        if s not in failed:
            store(rec, results[s], rev, chash, ext, main)
    rec.save()
    if getattr(chash, "graph", False) and not failed:
        with open(os.path.join(b, "own.txt"), "w") as f:
            f.writelines(f"{r} {h}\n" for r, h in sorted(chash.own_hash.items()))
    records.write_views(rec, chash, ext, main)
    reds = []
    for s in sorted(results):
        res = results[s]
        if s in failed:
            continue
        if res["a7"]["verdict"] != "IDENTICAL":
            reds.append(f"{rev} A7 {s}: {res['a7']['detail']}")
        reds += [f"{rev} A5 {s} {r}: {v['divergent']} divergent, {v['order_flags']} order of {v['calls']}: {v['detail']}"
                 for r, v in sorted(res["a5"].items()) if v["divergent"] or v["order_flags"]]
    with open(os.path.join(b, "reds.txt"), "a") as f:
        f.writelines(x + "\n" for x in reds)
    for x in reds:
        log("  RED " + x, b)
    log(f"{rev}: measured {len(results) - len(failed)} script(s) in {time.time() - t0:.0f} s; "
        f"{len(failed)} failed: {' '.join(failed) or '-'}; {len(reds)} red line(s)", b)
    missing = [s for s in records.script_names(main) if records.status(s, chash, ext, rec, main) != "current"]
    wrote = ["config/rondo/runs", "config/rondo/gate_a5.csv", "config/rondo/gate_a7.csv",
             "config/rondo/gate_a7_routines.csv"]
    if failed or missing:
        log(f"  no ledger: scripts without a result: {' '.join(sorted(set(failed) | set(missing)))}", b)
    else:
        for item in ("config/rondo/runs",):
            dst = os.path.join(tree, item)
            shutil.rmtree(dst, ignore_errors=True)
            shutil.copytree(os.path.join(main, item), dst)
        steps = [("asmbuild", [sys.executable, os.path.join(tree, "tools", "rondo", "asmbuild.py")]),
                 ("card image", [sys.executable, os.path.join(tree, "syscard", "tools", "image.py"), "--image",
                                 os.path.join(tree, "build", "rondo", "system", "syscard3.pce"), "--cc65",
                                 os.path.join(tree, "tools", "toolchains", "cc65"), "--out",
                                 os.path.join(tree, "build", "rondo", "card")]),
                 ("ledger", [sys.executable, os.path.join(tree, "tools", "rondo", "ledger.py"), "--chash",
                             os.path.join(tree, "chash.txt")])]
        ok = True
        for name, cmd in steps:
            r = cpu_run(1, "3G", *cmd, cwd=tree)
            if r.returncode:
                log(f"  {name} failed: {(r.stdout + r.stderr).strip()[-300:]}", b)
                ok = False
                break
        if ok:
            for item in OWNED[4:]:
                shutil.copy(os.path.join(tree, item), os.path.join(main, item))
            wrote = list(OWNED)
    if opts["commit"]:
        commit(main, rev, wrote, len(results) - len(failed), reds, b)
    return 1 if failed else 0


def commit(main, rev, paths, n, reds, b):
    dirty = git(main, "status", "--porcelain", "--", *paths).stdout
    if not dirty.strip():
        log("  records unchanged: nothing to commit", b)
        return
    lock = os.path.join(main, "build", "rondo", "land.lock")
    msg = f"Gate records at {rev}: {n} script(s) measured" + (f", {len(reds)} red line(s)" if reds else "")
    r = subprocess.run(["flock", lock, "sh", "-c",
                        "git -C \"$1\" add -- $3 && git -C \"$1\" commit -q -m \"$2\" -- $3", "sh", main, msg,
                        " ".join(paths)], capture_output=True, text=True)
    log("  committed: " + (git(main, "log", "--oneline", "-1").stdout.strip() if not r.returncode
                           else "FAILED " + r.stderr.strip()[-300:]), b)


def parse(argv):
    opts = {"mode": argv[0] if argv else "", "main": TOP, "routines": [], "scripts": [], "sweep": False,
            "commit": False, "workers": 8, "only": False, "norerun": False}
    i = 1
    while i < len(argv):
        a = argv[i]
        if a == "--main":
            opts["main"] = os.path.abspath(argv[i + 1]); i += 1
        elif a == "--routines":
            opts["routines"] = argv[i + 1].split(","); i += 1
        elif a == "--scripts":
            opts["scripts"] = argv[i + 1].split(","); i += 1
        elif a == "--workers":
            opts["workers"] = int(argv[i + 1]); i += 1
        elif a == "--norerun":
            opts["norerun"] = True
        elif a == "--only":
            opts["only"] = True
        elif a == "--sweep":
            opts["sweep"] = True
        elif a == "--commit":
            opts["commit"] = True
        else:
            raise SystemExit(f"unknown option {a}")
        i += 1
    return opts


def main():
    opts = parse(sys.argv[1:])
    if opts["mode"] == "commit":
        recs = records.Records(opts["main"]).scripts
        srcs = sorted({r["source"] for r in recs.values()})
        commit(opts["main"], srcs[-1] if srcs else "?", list(OWNED), len(recs), [],
               os.path.join(opts["main"], "build", "rondo", "runner"))
        return 0
    if opts["mode"] not in ("once", "watch", "plan"):
        print(__doc__)
        return 2
    if opts["mode"] != "watch":
        return cycle(opts["main"], opts)
    done = None
    while True:
        head = git(opts["main"], "rev-parse", "HEAD").stdout.strip()
        if head != done:
            cycle(opts["main"], dict(opts, mode="once"))
            done = git(opts["main"], "rev-parse", "HEAD").stdout.strip()
        time.sleep(60)


if __name__ == "__main__":
    sys.exit(main())
