"""records.py -- the stored gate results, and exactly when each one still counts.

The records (config/rondo/runs/) are written by runner.py and read by ledger.py, worklist.py and
whatif.py. Nothing else writes them.

  scripts.csv  script, script_hash, verdict, detail, a5_secs, a7_secs, world, source
                 one row per script measured: its A7 verdict, the hash of its text, and `world`, the
                 hash of (routine, compiled hash) over the routines it reaches when it was measured
  a5.csv       script, routine, calls, identical, divergent, unverified, order_flags, chash, detail, source
                 A5's counts for each routine the script called (rows with no call are not kept)
  a7.csv       script, routine, swapped, ran_inside, chash
                 the routines whose C ran in the script's lockstep run

When a record stops counting (the whole rule):
  * an A5 row counts for a routine while the routine's compiled hash (compiled.py) equals the row's
    and the script's text hash equals the one stored for the script;
  * a script's A7 result counts while its text hash is unchanged and its `world` is unchanged: the
    compiled hash of every routine it reaches (the routines of its own rows, and those whose bytes
    its trace ran) is the one it was measured with. A new routine with C that the script reaches
    changes its world, as does a changed one; an unrelated change does not;
  * every row carries the gate epoch it was measured at (epoch.txt, moved only by an explicit commit
    saying why, when the checker is replaced by gates that prove more). A row of another epoch counts
    for nothing: when the epoch moves every filed routine is unmeasured and the headline is blocked
    until the runner has measured every script again. Ordinary checker fixes do not move it;
  * a script with no trace folder reaches every routine with C (conservative);
  * a checker, harness, hybrid.py or setup.sh change changes nothing. Re-measuring after one is
    asked for by name (runner.py --routines / --scripts).
A script with no row, or a row for other text, is unmeasured: no report is written (complete()).
"""
import csv
import glob
import hashlib
import os

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
SCRIPTS_FIELDS = ["script", "script_hash", "verdict", "detail", "a5_secs", "a7_secs", "world", "epoch", "source"]
A5_FIELDS = ["script", "routine", "calls", "identical", "divergent", "unverified", "order_flags", "chash",
             "detail", "epoch", "source"]
A7_FIELDS = ["script", "routine", "swapped", "ran_inside", "chash", "epoch"]


def epoch(top=TOP):
    """The gate epoch: the first number in config/rondo/runs/epoch.txt. It moves only by a commit to that
    file that says why (the gates were replaced by ones that prove more); never because a file changed."""
    path = os.path.join(runs_dir(top), "epoch.txt")
    for line in open(path):
        if line.strip() and not line.startswith("#"):
            return line.split()[0]
    raise SystemExit(f"{path}: no epoch number")


def runs_dir(top=TOP):
    return os.path.join(top, "config", "rondo", "runs")


def script_names(top=TOP):
    """Every script the oracles run: tools/rondo/scripts/*.txt without a leading underscore."""
    return sorted(os.path.splitext(os.path.basename(p))[0]
                  for p in glob.glob(os.path.join(top, "tools", "rondo", "scripts", "*.txt"))
                  if not os.path.basename(p).startswith("_"))


def script_hash(name, top=TOP):
    return hashlib.sha256(open(os.path.join(top, "tools", "rondo", "scripts", name + ".txt"), "rb").read()).hexdigest()[:12]


def read(path):
    return list(csv.DictReader(open(path, newline=""))) if os.path.exists(path) else []


def write(path, fields, rows):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", newline="\n", encoding="ascii") as f:
        w = csv.DictWriter(f, fieldnames=fields, lineterminator="\n")
        w.writeheader()
        w.writerows(rows)


class Records:
    def __init__(self, top=TOP):
        d = runs_dir(top)
        self.top = top
        self.scripts = {r["script"]: r for r in read(os.path.join(d, "scripts.csv"))}
        self.a5 = read(os.path.join(d, "a5.csv"))
        self.a7 = read(os.path.join(d, "a7.csv"))
        self._by_script = None

    def save(self):
        d = runs_dir(self.top)
        write(os.path.join(d, "scripts.csv"), SCRIPTS_FIELDS, [self.scripts[k] for k in sorted(self.scripts)])
        write(os.path.join(d, "a5.csv"), A5_FIELDS, sorted(self.a5, key=lambda r: (r["script"], r["routine"])))
        write(os.path.join(d, "a7.csv"), A7_FIELDS, sorted(self.a7, key=lambda r: (r["script"], r["routine"])))

    def replace_script(self, script, scripts_row, a5_rows, a7_rows):
        self.scripts[script] = scripts_row
        self.a5 = [r for r in self.a5 if r["script"] != script] + a5_rows
        self.a7 = [r for r in self.a7 if r["script"] != script] + a7_rows
        self._by_script = None

    def routines_of(self, script):
        if self._by_script is None:
            self._by_script = {}
            for r in self.a5 + self.a7:
                self._by_script.setdefault(r["script"], set()).add(r["routine"])
        return self._by_script.get(script, set())


def trace_covers(script, rom, start, length, top=TOP):
    """True if the script's own trace ran a byte of [start, start+length) in the disc (rom=0) or the
    card (rom=1); None when the script has no trace folder (it then reaches everything)."""
    path = os.path.join(top, "build", "rondo", "trace", script, "cardmap.bin" if rom else "discmap.bin")
    if not os.path.exists(path):
        return None
    with open(path, "rb") as f:
        f.seek(start)
        return any(b & 3 for b in f.read(length))


_trace_cache = {}


def reach(script, routine, extents, rec, top=TOP):
    """Does this script reach the routine: it has a row for it, or its trace ran one of its bytes."""
    if routine in rec.routines_of(script):
        return True
    run, length, disc, rom = extents[routine]
    key = (script, rom, disc, length)
    if key not in _trace_cache:
        _trace_cache[key] = trace_covers(script, rom, disc, length, top)
    covered = _trace_cache[key]
    return True if covered is None else covered


def world(script, chash, extents, rec, top=TOP):
    """The hash of (routine, compiled hash) over the routines the script reaches."""
    h = hashlib.sha256()
    for r in sorted(extents):
        if chash.get(r, "noc") != "noc" and reach(script, r, extents, rec, top):
            h.update(f"{r} {chash[r]}\n".encode())
    return h.hexdigest()[:16]


def status(script, chash, extents, rec, top=TOP):
    """'unmeasured' (no row, or the script text changed), 'stale' (the world changed), or 'current'."""
    row = rec.scripts.get(script)
    if not row or row["script_hash"] != script_hash(script, top) or row.get("epoch") != epoch(top):
        return "unmeasured"
    return "current" if row["world"] == world(script, chash, extents, rec, top) else "stale"


def complete(rec, top=TOP):
    """The scripts with no result for their present text: no report may be written while any."""
    return [s for s in script_names(top)
            if s not in rec.scripts or rec.scripts[s]["script_hash"] != script_hash(s, top)
            or rec.scripts[s].get("epoch") != epoch(top)]


def a5_counts(rec, chash, top=TOP):
    """Per routine, A5's counts summed over the scripts whose row still counts, and the first detail."""
    out = {}
    for r in rec.a5:
        s = r["script"]
        row = rec.scripts.get(s)
        if r["chash"] != chash.get(r["routine"]) or not row or row["script_hash"] != script_hash(s, top) \
                or r.get("epoch") != epoch(top):
            continue
        t = out.setdefault(r["routine"], {"calls": 0, "identical": 0, "divergent": 0, "unverified": 0,
                                          "order_flags": 0, "scripts": 0, "detail": "", "where": ""})
        for k in ("calls", "identical", "divergent", "unverified", "order_flags"):
            t[k] += int(r[k])
        t["scripts"] += 1
        if int(r["divergent"]) and not t["detail"]:
            t["detail"], t["where"] = r["detail"], s
    return out


def a7_scripts(rec, chash, extents, top=TOP):
    """{script: (status, verdict)} for every script with a row."""
    return {s: (status(s, chash, extents, rec, top), row["verdict"]) for s, row in rec.scripts.items()}


def a7_routines(rec, chash, extents, top=TOP):
    """Routines A7 credits: C ran in some counting script's run, and every script reaching the routine
    has a counting, identical A7 result. Returns {routine: (n_scripts_run_in, [scripts not passing])}."""
    stats = a7_scripts(rec, chash, extents, top)
    ran = {}
    for r in rec.a7:
        if r["chash"] == chash.get(r["routine"]) and r.get("epoch") == epoch(top) \
                and stats.get(r["script"], ("", ""))[0] == "current" \
                and (int(r["swapped"]) or int(r["ran_inside"])):
            ran[r["routine"]] = ran.get(r["routine"], 0) + 1
    out = {}
    for routine, n in ran.items():
        failing = [s in stats and (stats[s] != ("current", "IDENTICAL")) and s or None
                   for s in script_names(top) if reach(s, routine, extents, rec, top)]
        out[routine] = (n, sorted({s for s in failing if s} | {s for s in script_names(top) if s not in stats and
                                                           reach(s, routine, extents, rec, top)}))
    return out
