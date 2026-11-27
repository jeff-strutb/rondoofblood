#!/usr/bin/env python3
"""runner.py -- the one process that certifies. Writers do not run gates.

    python3 tools/rondo/runner.py once  [--main DIR] [options]    certify what main's tip needs, then stop
    python3 tools/rondo/runner.py watch [--main DIR] [options]    the same whenever the tip moves (every 60 s)
    python3 tools/rondo/runner.py plan  [--main DIR] [options]    print what would run and the time expected
    python3 tools/rondo/runner.py served [--main DIR] --scripts a [--a7]   drafts in place of failing routines (served.py)
    python3 tools/rondo/runner.py core DIR                        exit 1 and say what to run when DIR's core is older than its patches
    python3 tools/rondo/runner.py commit [--main DIR]             commit the records, views and ledgers as they stand
                                                                  in DIR (under the landing lock, explicit paths)
    options: --routines A,B   re-measure the scripts reaching these routines (after a checker change)
             --scripts a,b    re-measure these scripts
             --only           with --scripts: nothing else (a partial check; no ledger is written)
             --sweep          every script: the whole-game run, only when asked
             --commit         commit the records and ledgers to DIR's branch under the landing lock (off by
                              default until the state session has handed those files over)
             --norerun        do not rerun red scripts alone (when every script is expected red)
             --draft-a5 FILE  with `served`: per-draft A5 verdicts (name, filed, calls, identical, divergent, unverified,
                              order) measured elsewhere, instead of measuring the drafts here
             --also-defects   with --proper: also serve by draft the filed routines named in lockstep_defects.csv
             --extra A,B      with --proper: also serve these filed routines by draft; --keep A,B keeps them filed
             --iterate N      with --proper: until identical (at most N runs) add the filed routines the defects name
             --proper         with `served --a7`: the filed C where proper, the draft for every other routine (the M1 run)
             --serve-all      with `served --a7`: every filed routine and every content served by its draft (the drafter's recipe)
             --a7             with `served`: build the served box and measure A7 on --scripts alone
             --workers N      scripts measured at once (default 8)

What it does for main's tip (design: build/rondo/pipeline/DESIGN.md, the rule: records.py):
  1. exports the tip into its own private tree (git archive), links the untracked inputs, builds its own
     pcebox there, and takes compiled.py's hash of every routine with C: what the compiler produced;
  2. plans: a script is measured when it is new or its text changed, or the compiled hash of a routine it
     reaches changed (a new routine, a changed one, a callee of one). Nothing else plans anything: not
     a checker or harness change, not a commit elsewhere. The plan prints with the time it should take;
  3. measures each planned script alone: A5 then A7 (oracle_io.py), each in its own process, in a private output folder, with the private pcebox; at most --workers waiting or running;
  4. reruns every red script (A7 not identical, A5 divergent) alone, each in its own process
     claim, all reds at once within --workers, and records the second result; a flip is logged as a flake;
  5. writes the records, then the ledger (only if every script has a result), and commits them under the
     landing lock with explicit paths. It never holds or edits C: failures are reported (reds.txt, the log).
A script run that fails (exit, timeout) leaves that script's old record in place and no ledger is written.
Everything it makes is under build/rondo/runner/ in the main checkout.
"""
import concurrent.futures
import csv
import datetime
import fcntl
import glob
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
import records  # noqa: E402

LINKS = ("references", "tools/toolchains/src", "tools/toolchains/cc65", "tools/toolchains/pce",
         "build/rondo/disc", "build/rondo/system", "build/rondo/trace")
OWNED = ("config/rondo/runs", "config/rondo/gate_a5.csv", "config/rondo/gate_a7.csv",
         "config/rondo/gate_a7_routines.csv", "docs/rondo/ledger.csv", "syscard/ledger.csv", "config/rondo/drafted.csv",
         "config/rondo/lockstep_defects.csv", "config/rondo/lockstep_held.csv",
         "config/rondo/certification_queue.csv")
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


def run_job(*cmd, **kw):
    return subprocess.run(list(cmd), capture_output=True, text=True, **kw)


def core_problem(main):
    """None when main's core is at least as new as every patch under tools/toolchains/patches/pce, else the
    one line that says what to run. (Main's build fell behind main's source twice in one day after a patch landed;
    a measurement or a trace with an older core gives wrong values or none.) The core a run uses is RONDO_CORE
    when set, else main's own."""
    core = os.environ.get("RONDO_CORE") or os.path.join(main, "tools", "toolchains", "pce", "mednafen_pce_libretro.dylib")
    patches = glob.glob(os.path.join(main, "tools", "toolchains", "patches", "pce", "*.patch"))
    if not os.path.exists(core):
        return f"core {core} does not exist: in {main} run: sh setup.sh pce (then sh setup.sh pcebox)"
    if not patches:
        return None
    newest = max(patches, key=os.path.getmtime)
    if os.path.getmtime(core) < os.path.getmtime(newest):
        return (f"main's core ({core}) is older than the newest core patch ({os.path.basename(newest)}): in {main} "
                "run: sh setup.sh pce, then sh setup.sh pcebox (a clean make; the core's Makefile does not track "
                "headers), then run this again")
    return None


def export(main, rev, b, tag=""):
    tree = os.path.join(b, "tree-" + rev + tag)
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


def prepare(main, rev, b, d, tag=""):
    """The private tree, its own pcebox, and the compiled hashes: (tree, box, chash, extents). `tag` names a tree
    of its own (served runs beside a sweep of the same revision and must not re-export the sweep's tree)."""
    tree = export(main, rev, b, tag)
    box = os.path.join(b, "pcebox-" + rev)
    if not os.path.exists(box) or not os.path.exists(box + ".gen"):      # a box of this revision is reused
        r = run_job("env", "RONDO_BOX=" + box, "sh", os.path.join(tree, "setup.sh"), "pcebox")
        if r.returncode:
            raise SystemExit(f"pcebox build of {rev} failed:\n{(r.stdout + r.stderr)[-800:]}")
    gen = os.path.join(tree, "build", "rondo", "pcebox")      # setup.sh puts a private build's generated files beside the binary
    os.makedirs(gen, exist_ok=True)
    for f in ("hybrid_table.c", "opclass.h"):
        shutil.copy(os.path.join(box + ".gen", f), os.path.join(gen, f))
    out = os.path.join(tree, "chash.txt")
    r = run_job(sys.executable, os.path.join(tree, "tools", "rondo", "compiled.py"), "--tree", tree,
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


def landing_report(rec, chash, changed, rev, main):
    """The routines whose own code changed with this landing, each with its verdict and, if it has a defect, the
    first defect line, as lines to forward as they are. Also written to build/rondo/runner/landing-REV.txt."""
    counts = records.a5_counts(rec, chash, main)
    lines = [f"Landing {rev}: {len(changed)} routine(s) with changed code re-judged by A5"]
    for r in changed:
        t = counts.get(r)
        if not t:
            lines.append(f"  {r}: UNCOVERED (no counting script called it)")
            continue
        v = records.a5_verdict(t)
        text = f"  {r}: {v}, {t['identical']} of {t['calls']} calls identical in {t['scripts']} script(s)"
        if v == "DIVERGENT":
            text += f"; {t['divergent']} divergent, {t['order_flags']} order-flagged; first: [{t['where']}] {t['detail']}"
        elif v == "UNVERIFIED":
            text += f"; {t['unverified']} call(s) not judged"
        lines.append(text)
    with open(os.path.join(main, "build", "rondo", "runner", f"landing-{rev}.txt"), "w") as f:
        f.write("\n".join(lines) + "\n")
    return lines


def measure(tree, box, out, script, mode="both"):
    r = run_job(sys.executable, os.path.join(tree, "tools", "rondo", "oracle_io.py"), "job", tree,
                script, os.path.join(out, script), box, mode, timeout=SCRIPT_TIMEOUT)
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
    problem = core_problem(main)
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
    # each red script is rerun alone, in its own process and out folder slot, all reds at once (the claims queue
    # them as their own processes); a rerun's result is on disk the moment it ends
    reds_first = [s for s in sorted(results) if red(results[s]) and not opts["norerun"]]
    if reds_first:
        log(f"  {len(reds_first)} red script(s): each rerun alone, {workers} at once", b)
        with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as ex:
            for s, second in zip(reds_first, ex.map(lambda x: measure(tree, box, out, x), reds_first)):
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
    if changed:
        for line in landing_report(rec, chash, changed, rev, main):
            log(line, b)
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
            r = run_job(*cmd, cwd=tree)
            if r.returncode:
                log(f"  {name} failed: {(r.stdout + r.stderr).strip()[-300:]}", b)
                ok = False
                break
        if ok:
            for item in OWNED[4:6]:
                shutil.copy(os.path.join(tree, item), os.path.join(main, item))
            wrote = list(OWNED)
    if opts["commit"]:
        commit(main, rev, wrote, len(results) - len(failed), reds, b)
    return 1 if failed else 0


def served(main, opts):
    """Drafts serving in place of failing routines: the draft A5 on the given scripts, the serve plan from the
    stored records, and (with --a7) the served box and A7 on those scripts alone. Writes no record."""
    import served as sv
    b = os.path.join(main, "build", "rondo", "runner")
    os.makedirs(b, exist_ok=True)
    problem = core_problem(main)
    if problem:
        print("runner: REFUSED: " + problem)
        return 3
    cache = sv.cache_dir(main)
    if not cache:
        print("runner: REFUSED: the draft cache is stale or missing: python3 tools/rondo/cdraft.py --cache")
        return 3
    rev = git(main, "rev-parse", "--short=7", "HEAD").stdout.strip()
    tree, box, chash, ext = prepare(main, rev, b, b, "-served")
    rec = records.Records(main)
    scripts = opts["scripts"] or ["boot"]
    if scripts == ["all"]:
        scripts = records.script_names(main)
    st = sv.stamp(cache)
    labels = sv.filed_labels(cache, sorted(ext))
    drafts = sv.read_drafts(cache)
    filed_of = {lab: n for n, labs in labels.items() for lab in labs}
    log(f"served: {rev}, draft cache {st}: {len(drafts)} drafts, {len(labels)} of {len(ext)} filed routines have one", b)
    mode = "proper" if opts.get("proper") else "serve-all" if opts.get("serve_all") else "mix"
    out = os.path.join(b, f"out-served-{rev}-{mode}")
    allbox = os.path.join(b, f"pcebox-alldrafts-{rev}-{st}")
    # a script's draft A5 depends only on the all-drafts box: a result measured with this same box is kept
    d5dir = os.path.join(out, "d5")
    keep_d5 = os.path.exists(os.path.join(d5dir, "box.txt")) and open(os.path.join(d5dir, "box.txt")).read() == allbox
    if keep_d5:
        for name in os.listdir(out):
            if name != "d5":
                shutil.rmtree(os.path.join(out, name), ignore_errors=True) if os.path.isdir(os.path.join(out, name)) \
                    else os.remove(os.path.join(out, name))
    else:
        shutil.rmtree(out, ignore_errors=True)
    draft_a5, d5_failed = {}, []
    if opts.get("draft_a5"):
        # per-draft verdicts measured elsewhere (the drafter's full A5 with every draft serving)
        for r in csv.DictReader(open(opts["draft_a5"], newline="")):
            draft_a5[r["name"]] = {"calls": int(r["calls"]), "identical": int(r["identical"]),
                                   "divergent": int(r["divergent"]), "unverified": int(r["unverified"]),
                                   "order_flags": int(r["order"])}
        log(f"served: per-draft A5 from {opts['draft_a5']}: {len(draft_a5)} drafts", b)
    elif opts.get("serve_all") and opts.get("a7") and sorted(scripts) != sorted(records.script_names(main)):
        # every draft serves whatever A5 says, and the drafted count needs every script: a rerun of some
        # scripts measures A7 alone
        log("served: --serve-all on some scripts: no draft A5 (it decides nothing here)", b)
    else:
        if not os.path.exists(allbox):
            dst = os.path.join(b, "tree-alldrafts")
            d = sv.make_tree(tree, dst, cache, drafts, {r["name"] for r in drafts}, set(labels), filed_of)
            ok, text = build_box(dst, d, allbox)
            if not ok:
                print("runner: the all-drafts box did not build:\n" + text)
                return 1
        with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, min(opts["workers"], len(scripts)))) as ex:
            results_d5 = list(ex.map(lambda s: d5_result(tree, allbox, d5dir, s), scripts))
        with open(os.path.join(d5dir, "box.txt"), "w") as f:
            f.write(allbox)
        for s, res in zip(scripts, results_d5):
            if res.get("errors"):
                # no draft A5 for this script: it is reported, the others still count, and no drafted count is written
                log(f"served: draft A5 of {s} FAILED {res['errors']}", b)
                d5_failed.append(s)
                continue
            for lab, v in res["a5"].items():
                t = draft_a5.setdefault(lab, {"calls": 0, "identical": 0, "divergent": 0, "unverified": 0, "order_flags": 0})
                for k in t:
                    t[k] += v[k]
    if d5_failed:
        log(f"served: no drafted count written: {len(d5_failed)} script(s) without a draft A5: {' '.join(d5_failed)}", b)
    if scripts == records.script_names(main) and not opts.get("draft_a5") and not d5_failed:
        # every script measured: the drafted count the README prints (config/rondo/drafted.csv, tracked)
        import datetime
        rom = {r["name"]: r.get("rom", "0") for r in drafts}
        ident = {lab for lab, t in draft_a5.items() if sv.verdict_of(t) == "IDENTICAL"}
        with open(os.path.join(main, "config", "rondo", "drafted.csv"), "w", newline="\n") as f:
            w = csv.writer(f, lineterminator="\n")
            w.writerow(["measured_on", "drafts", "identical", "identical_game", "identical_card", "source"])
            w.writerow([datetime.date.today(), len(drafts), len(ident), sum(1 for x in ident if rom.get(x) != "1"),
                        sum(1 for x in ident if rom.get(x) == "1"),
                        f"the runner's A5 of every draft serving, over all {len(scripts)} scripts, at {rev}"])
    filed_a5 = records.a5_counts(rec, chash, main)
    plan = sv.plan(filed_a5, draft_a5, labels, drafts)
    called = {lab for lab in draft_a5 if draft_a5[lab]["calls"]}
    tally = {}
    for lab in called:
        v = sv.verdict_of(draft_a5[lab])
        tally[v] = tally.get(v, 0) + 1
    log(f"served: draft A5 on {','.join(scripts)}: {len(called)} drafts called: {tally}", b)
    log(f"served: plan: {len(plan['keep'])} filed routines keep their C, {len(plan['serve'])} filed routines served by a "
        f"draft, {len(plan['free'])} drafts with no filed C serve, {len(plan['original'])} left as original code", b)
    for name, why in plan["original"][:40]:
        if name in filed_a5 or name in draft_a5:
            log(f"  original: {name}: {why}", b)
    if not opts.get("a7"):
        return 0
    keep = {lab for labs in plan["serve"].values() for lab in labs} | set(plan["free"])
    names = set(plan["serve"]) | {n for n, _ in plan["original"] if n in labels}
    extra = set(opts.get("extra", ()))
    if opts.get("also_defects"):
        extra |= {r["routine"] for r in records.read(os.path.join(main, "config", "rondo", "lockstep_defects.csv"))
                  if r["mode"] == "proper" and r["script"] in scripts}
    extra = {n for n in extra if n in labels} - set(opts.get("keep", ()))
    import ledger
    funcs = ledger.functions() if opts.get("proper") else {}
    for it in range(max(1, opts.get("iterate", 0) or 1)):
        if opts.get("proper"):
            # the M1 recipe: the filed C where the proper-C check passes it, the draft for every other routine
            # and for the filed routines named as lockstep defects (extra)
            bad = {n for n in labels if ledger.proper_c_faults(n, funcs)} | extra
            keep = {r["name"] for r in drafts if filed_of.get(r["name"], "") not in labels or filed_of.get(r["name"]) in bad}
            names = set(bad)
            log(f"served: --proper: {len(labels) - len(bad)} filed routines keep their C, {len(bad)} served by drafts "
                f"({len(extra)} of them named by lockstep defects), {len(keep)} drafts serve in all", b)
        if opts.get("serve_all"):
            # the drafter's recipe: every filed routine and every other content served by its draft
            keep = {r["name"] for r in drafts}
            names = set(labels)
            log(f"served: --serve-all: all {len(keep)} drafts serve, {len(names)} filed routines replaced", b)
        dst = os.path.join(b, "tree-served")
        d = sv.make_tree(tree, dst, cache, drafts, keep, names, filed_of)
        os.makedirs(out, exist_ok=True)
        sv.write_serve_list(os.path.join(out, "serve_drafts.csv"), names)
        sbox = os.path.join(b, f"pcebox-served-{rev}")
        if os.path.exists(sbox):
            os.remove(sbox)
        ok, text = build_box(dst, d, sbox)
        if not ok:
            print("runner: the served box did not build:\n" + text)
            return 1
        new_extra, all_identical = set(), True
        # each script alone in its own process, --workers at once; the records below are then
        # written one script at a time
        with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, min(opts["workers"], len(scripts)))) as ex:
            runs = dict(zip(scripts, ex.map(lambda x: measure(tree, sbox, os.path.join(out, "a7"), x, "a7"), scripts)))
        table = []
        for s in scripts:
            res = runs[s]
            table.append(table_row(s, mode, rev, res, os.path.join(out, "a7", s)))
            if res.get("errors"):
                log(f"served: A7 of {s} FAILED {res['errors']}", b)
                all_identical = False
                continue
            log(f"served: A7 of {s}: {res['a7']['verdict']}: {res['a7']['detail']}", b)
            f = res["a7"]["fields"]
            log(f"served: A7 of {s} ({mode}) counts: {f['swapped']} swapped, {f['compared']} frames compared, "
                f"{f['identical']} identical, {f['defects']} access defects, parked {f['parked']}, unreached {f['unreached']}", b)
            all_identical &= res["a7"]["verdict"] == "IDENTICAL"
            path = os.path.join(main, "config", "rondo", "lockstep_defects.csv")
            old_rows = [r for r in records.read(path) if not (r["script"] == s and r["mode"] == mode)]
            # check.c charges a defect to every swapped call whose span holds the call it names: a line ending
            # "[in the call of X]" (X not the routine) was made inside X's call. The report keeps 6 lines a routine.
            own, charged = {}, {}
            rep = os.path.join(out, "a7", s, "a7run", "a7", s + ".rec.hybrid.txt")
            if os.path.exists(rep):
                for ln in open(rep, errors="replace"):
                    m = re.match(r"(\w+) (?:MOVED|EXTRA|MISSING|VALUE|STACK|STATE|REGS|CALL|INT) call \d+ frame \d+", ln)
                    if m:
                        t = re.search(r"\[in the call of (\w+)\]\s*$", ln)
                        if t and t.group(1) != m.group(1):
                            charged.setdefault(m.group(1), set()).add(t.group(1))
                        else:
                            own[m.group(1)] = own.get(m.group(1), 0) + 1
            new_rows = [{"script": s, "mode": mode, "routine": r, "kinds": pack(k), "defects": sum(k.values()), "source": rev,
                         "own_lines": own.get(r, 0), "inside_call_of": ";".join(sorted(charged.get(r, ())))}
                        for r, k in sorted(res["a7_defects"].items(), key=lambda kv: -sum(kv[1].values()))]
            records.write(path, ["script", "mode", "routine", "kinds", "defects", "source", "own_lines", "inside_call_of"],
                          old_rows + new_rows)
            for n, r, k in sorted(((sum(k.values()), r, k) for r, k in res["a7_defects"].items()), reverse=True)[:8]:
                log(f"  defects under {filed_of.get(r, r)} ({r}): {k}", b)
            new_extra |= {r for r in res["a7_defects"] if r in labels and r not in bad} if opts.get("proper") else set()
            if res["a7"]["verdict"] == "IDENTICAL" and opts.get("proper"):
                import csv as _csv
                bytes_of = {r["routine"]: r["bytes"] for r in records.read(os.path.join(main, "config", "rondo", "gate_a5.csv"))}
                held = sorted(r for r, v in res["reach"].items() if v[0] > 0 and r in labels and r not in bad)
                hp = os.path.join(main, "config", "rondo", "lockstep_held.csv")
                rows_h = [r for r in records.read(hp) if r["script"] != s]
                rows_h += [{"script": s, "routine": r, "bytes": bytes_of.get(r, ""), "source": rev,
                            "drafts_serving": len(bad)} for r in held]
                records.write(hp, ["script", "routine", "bytes", "source", "drafts_serving"], rows_h)
                log(f"served: {s}: filed C holding in lockstep: {len(held)} routines, "
                    f"{sum(int(bytes_of.get(r) or 0) for r in held)} B (with {len(bad)} filed routines served by drafts)", b)
        write_table(main, out, mode, rev, scripts, table, b)
        if all_identical or not new_extra:
            break
        extra |= new_extra
        log(f"served: iteration {it + 1}: {len(new_extra)} more filed routines named by defects go to drafts: "
            f"{', '.join(sorted(new_extra)[:12])}", b)
    return 0


def d5_result(tree, allbox, d5dir, s):
    """A script's draft A5: the stored result when it was measured with this box and has no error, else measured."""
    path = os.path.join(d5dir, s, "result.json")
    if os.path.exists(os.path.join(d5dir, "box.txt")) and os.path.exists(path):
        res = json.load(open(path))
        if not res.get("errors"):
            return res
    return measure(tree, allbox, d5dir, s, "d5")


TABLE = ["script", "mode", "verdict", "code", "frames", "defects", "first_stop", "cause", "source"]


def table_row(s, mode, rev, res, d):
    """One script's lockstep result: its verdict, the first differing frame or code, frames compared of expected,
    access defects, where it first stopped (the report's STOPPED line or its first defect line) and the cause
    the checker gave there."""
    if res.get("errors"):
        return {"script": s, "mode": mode, "verdict": "NORESULT", "code": "", "frames": "", "defects": "",
                "first_stop": "", "cause": "; ".join(res["errors"])[:300], "source": rev}
    f = res["a7"]["fields"]
    stop, cause = "", ""
    so = os.path.join(d, "a7.stdout.txt")
    if os.path.exists(so):
        stop = next((ln.strip() for ln in open(so, errors="replace") if ln.startswith("STOPPED ")), "")
    rep = os.path.join(d, "a7run", "a7", s + ".rec.hybrid.txt")
    if os.path.exists(rep):
        lines = open(rep, errors="replace").read().splitlines()
        cause = next((ln.strip() for ln in lines if ln.startswith("stopped at frame")), "")
        first = next((ln.strip() for ln in lines if re.match(
            r"\w+ (?:MOVED|EXTRA|MISSING|VALUE|STACK|STATE|REGS|CALL|INT) call \d+ frame \d+", ln)), "")
        if not stop:
            stop = first
        elif first and not cause:
            cause = first
    def text(x):        # the records are ASCII
        return x.encode("ascii", "replace").decode()[:300]
    return {"script": s, "mode": mode, "verdict": res["a7"]["verdict"], "code": f["code"],
            "frames": f"{f['compared']}/{f['expected']}", "defects": f["defects"], "first_stop": text(stop),
            "cause": text(cause), "source": rev}


def write_table(main, out, mode, rev, scripts, table, b):
    """The per-script lockstep table of one served run: in its output folder always, and as the record
    config/rondo/runs/served_a7.csv (this mode's rows replaced) when every script was measured."""
    records.write(os.path.join(out, "table.csv"), TABLE, table)
    n = sum(1 for r in table if r["verdict"] == "IDENTICAL")
    log(f"served: {mode} at {rev}: {n} of {len(table)} scripts identical; table {os.path.join(out, 'table.csv')}", b)
    path = os.path.join(main, "config", "rondo", "runs", "served_a7.csv")
    names = set(records.script_names(main))
    old = records.read(path)
    have = {r["script"] for r in old if r["mode"] == mode}
    if set(scripts) == names or names <= have | set(scripts):
        # every script has a row of this mode: a whole run replaces the mode's rows, a rerun of some scripts
        # replaces theirs (each row names the commit it was measured at); rows of removed scripts go
        done = set(scripts)
        rows = [r for r in old if r["mode"] != mode or (r["script"] not in done and r["script"] in names)] + table
        records.write(path, TABLE, sorted(rows, key=lambda r: (r["mode"], r["script"])))
    else:
        log(f"served: served_a7.csv not written: {len(names - have - set(scripts))} script(s) of mode {mode} "
            "have no row yet; run every script once", b)


def build_box(tree, draftdir, box):
    r = run_job("env", "RONDO_BOX=" + box, "RONDO_DRAFTS=" + draftdir, "sh", os.path.join(tree, "setup.sh"), "pcebox")
    if r.returncode or not os.path.exists(box):
        text = r.stdout + r.stderr
        errs = [ln for ln in text.splitlines() if "error" in ln or "undefined" in ln]
        return False, "\n".join(errs[:20]) or text[-600:]
    return True, ""


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
            "commit": False, "workers": 8, "only": False, "norerun": False, "a7": False, "draft_a5": "", "serve_all": False, "proper": False, "iterate": 0, "also_defects": False, "extra": [], "keep": []}
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
        elif a == "--draft-a5":
            opts["draft_a5"] = os.path.abspath(argv[i + 1]); i += 1
        elif a == "--iterate":
            opts["iterate"] = int(argv[i + 1]); i += 1
        elif a == "--also-defects":
            opts["also_defects"] = True
        elif a == "--extra":
            opts["extra"] = argv[i + 1].split(","); i += 1
        elif a == "--keep":
            opts["keep"] = argv[i + 1].split(","); i += 1
        elif a == "--proper":
            opts["proper"] = True
        elif a == "--serve-all":
            opts["serve_all"] = True
        elif a == "--a7":
            opts["a7"] = True
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
    if sys.argv[1:2] == ["core"] and len(sys.argv) == 3:
        problem = core_problem(sys.argv[2])     # land.sh: is DIR's core as new as its core patches
        if problem:
            print(problem)
        return 1 if problem else 0
    opts = parse(sys.argv[1:])
    if opts["mode"] == "served":
        return served(opts["main"], opts)
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
