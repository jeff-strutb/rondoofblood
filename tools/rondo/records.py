"""records.py -- the stored gate results, and exactly when each one still counts.

The records (config/rondo/runs/) are written by runner.py and read by ledger.py, worklist.py and
whatif.py. Nothing else writes them.

  scripts.csv  script, script_hash, verdict, detail, a5_secs, a7_secs, world, epoch, source,
               a7_code, a7_frames, a7_defects, a7_open
                 one row per script measured: its A7 verdict (IDENTICAL, DIVERGENT, DEFECTS, STOPPED or
                 NOTVERDICT, from the hybrid line's "first differing" code a7_code: -1, a frame, -2, -3, -5),
                 the hash of its text, and `world`, the hash of (routine, compiled hash) over the routines it
                 reaches when it was measured; a7_frames "compared/expected", a7_defects the access defects,
                 a7_open "parked/kept/unreached/unopened arrivals". A diagnostic run (-4) is never stored.
  a5.csv       script, routine, calls, identical, divergent, unverified, order_flags, chash, detail, epoch,
               source, defects, causes
                 A5's counts for each routine the script called (rows with no call are not kept); defects
                 "MOVED=3;EXTRA=1" (the nine access defect kinds), causes "left=1;prefix=2" (calls not judged
                 or set aside, by cause); a routine with any call not judged is not identical
  a7.csv       script, routine, swapped, ran_inside, chash, epoch, defects
                 the routines whose C ran in the script's lockstep run; defects "KIND=n;..." the access defects
                 the checker put under the routine as the innermost tagged one (a routine with any defect has
                 not passed A7)

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

The refinement (what a callee's change costs). Counting a routine's whole closure would re-measure
every script that reaches any caller of a changed routine: one edited routine re-tested 28 of 32 scripts
though 31 never ran its bytes. Where a script's stored result is a clean pass, only a routine whose OWN
compiled code changed, and that the script reaches, voids it:
  * why it is sound: the script passed, so the C made exactly the calls the original made; a routine
    the original never entered in that script never had its C run there, so changing it cannot change
    that script's result. Own code is compiled.py's OWN set: the routine's function and abi_ wrapper,
    the helpers and data only they use, its table row; an inlined callee is part of the caller's own
    bytes (the build is -O2), so inlining makes the caller changed;
  * reach is the union of two facts, never one alone: the original ran a byte of the routine in the
    script (its trace), or the hybrid ran the routine's C in it (a row of its A5 or A7 records);
  * a clean pass is: the script's A7 identical, and for a routine, no divergent A5 row of it in that
    script. A divergent routine keeps the closure rule: its whole closure is part of the script's world,
    and its row's key is the whole closure. A script whose A7 is not identical keeps the closure rule
    throughout. (An unverified row is a call A5 could not compare, not a defect found; counted as a
    defect it would put every closure back, since every script has hundreds.);
  * the key of an A5 or A7 row is, for a clean row, the hash of the routine's closure nodes that some
    routine the script reaches owns (an "e" in front); otherwise the closure hash.
Without compiled.py's .nodes file (a plain dict of hashes) every key is the closure hash, as before.
"""
import csv
import glob
import hashlib
import os

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
SCRIPTS_FIELDS = ["script", "script_hash", "verdict", "detail", "a5_secs", "a7_secs", "world", "epoch", "source",
                  "a7_code", "a7_frames", "a7_defects", "a7_open"]
A5_FIELDS = ["script", "routine", "calls", "identical", "divergent", "unverified", "order_flags", "chash",
             "detail", "epoch", "source", "defects", "causes"]
A7_FIELDS = ["script", "routine", "swapped", "ran_inside", "chash", "epoch", "defects"]


def epoch(top=TOP):
    """The gate epoch: the first number of the last line in config/rondo/runs/epoch.txt. It moves only by a commit to that
    file that says why (the gates were replaced by ones that prove more); never because a file changed."""
    path = os.path.join(runs_dir(top), "epoch.txt")
    found = [line.split()[0] for line in open(path) if line.strip() and not line.startswith("#")]
    if not found:
        raise SystemExit(f"{path}: no epoch number")
    return found[-1]               # the file lists every epoch with its reason, oldest first: the current one is the last


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
        self.__dict__.pop("_reached", None)
        self.__dict__.pop("_owned", None)

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


def reached(script, extents, rec, top=TOP):
    """The routines with an extent the script reaches (trace or rows)."""
    cache = rec.__dict__.setdefault("_reached", {})
    if script not in cache:
        cache[script] = {r for r in extents if reach(script, r, extents, rec, top)}
    return cache[script]


def strict_routines(script, rec):
    """Routines with a divergent or order-flagged A5 row in the script (both are defects found): they keep
    the closure rule. (An unverified row is a
    call A5 could not compare, not a defect found: the script's A7 run still covers it, so it stays relaxed.
    Every script has hundreds of them; counting them would put every closure back.)"""
    return {r["routine"] for r in rec.a5 if r["script"] == script and (int(r["divergent"]) or int(r["order_flags"]))}


def owned_by_reached(script, chash, extents, rec, top=TOP):
    """Node indexes some routine the script reaches owns."""
    cache = rec.__dict__.setdefault("_owned", {})
    key = (script, id(chash))
    if key not in cache:
        rs = reached(script, extents, rec, top)
        cache[key] = {i for i, owners in chash.owners.items() if owners & rs}
    return cache[key]


def refined(script, chash, rec):
    """True when the script's stored result is a clean pass and the graph is there."""
    row = rec.scripts.get(script)
    return bool(getattr(chash, "graph", False) and row and row["verdict"] == "IDENTICAL")


def world(script, chash, extents, rec, top=TOP):
    """The hash of what the script's stored result depends on: for a clean pass, the nodes owned by routines
    it reaches plus the whole closure of every divergent or unverified routine; else (routine, closure
    hash) over the routines it reaches."""
    h = hashlib.sha256()
    if refined(script, chash, rec):
        nodes = set(owned_by_reached(script, chash, extents, rec, top))
        for r in strict_routines(script, rec):
            nodes |= chash.closure.get(r, set())
        for i in sorted(nodes):
            h.update(f"{chash.nodes[i][0]} {chash.nodes[i][1]}\n".encode())
        return "e" + h.hexdigest()[:15]
    for r in sorted(extents):
        if chash.get(r, "noc") != "noc" and reach(script, r, extents, rec, top):
            h.update(f"{r} {chash[r]}\n".encode())
    return h.hexdigest()[:16]


def row_key(script, routine, chash, rec, top=TOP):
    """What an A5 or A7 row of (script, routine) must carry to still count: the routine's closure hash, or
    for a clean row of a clean-pass script the hash of the closure nodes the script's reached routines own."""
    base = chash.get(routine)
    if base is None or base == "noc" or not refined(script, chash, rec) or routine not in chash.closure \
            or routine in strict_routines(script, rec):
        return base
    mine = owned_by_reached(script, chash, chash.ext, rec, top)
    h = hashlib.sha256(chash.row_hash[routine].encode())
    for i in sorted(chash.closure[routine] & mine):
        h.update(f"{chash.nodes[i][0]} {chash.nodes[i][1]}\n".encode())
    return "e" + h.hexdigest()[:15]


def row_counts(row, chash, rec, top=TOP):
    return row["chash"] == row_key(row["script"], row["routine"], chash, rec, top)


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


def a5_verdict(t):
    """The checker's rule (oracle.py): any divergent or order-flagged call is DIVERGENT; a routine with any call
    not judged, or none identical, is UNVERIFIED, never IDENTICAL."""
    return ("UNCOVERED" if not t["calls"] else "DIVERGENT" if int(t["divergent"]) or int(t["order_flags"]) else
            "UNVERIFIED" if int(t["unverified"]) or not int(t["identical"]) else "IDENTICAL")


def a5_counts(rec, chash, top=TOP):
    """Per routine, A5's counts summed over the scripts whose row still counts, and the first detail."""
    out = {}
    for r in rec.a5:
        s = r["script"]
        row = rec.scripts.get(s)
        if not row or row["script_hash"] != script_hash(s, top) or r.get("epoch") != epoch(top) \
                or not row_counts(r, chash, rec, top):
            continue
        t = out.setdefault(r["routine"], {"calls": 0, "identical": 0, "divergent": 0, "unverified": 0,
                                          "order_flags": 0, "scripts": 0, "detail": "", "where": ""})
        for k in ("calls", "identical", "divergent", "unverified", "order_flags"):
            t[k] += int(r[k])
        t["scripts"] += 1
        if (int(r["divergent"]) or int(r["order_flags"])) and not t["detail"]:
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
        if r.get("epoch") == epoch(top) and stats.get(r["script"], ("", ""))[0] == "current" \
                and row_counts(r, chash, rec, top) and (int(r["swapped"]) or int(r["ran_inside"])):
            ran[r["routine"]] = ran.get(r["routine"], 0) + 1
    defective = {}
    for r in rec.a7:
        if r.get("epoch") == epoch(top) and r.get("defects") and stats.get(r["script"], ("", ""))[0] == "current":
            defective.setdefault(r["routine"], set()).add(r["script"])
    out = {}
    for routine, n in ran.items():
        failing = [s in stats and (stats[s] != ("current", "IDENTICAL")) and s or None
                   for s in script_names(top) if reach(s, routine, extents, rec, top)]
        out[routine] = (n, sorted({s for s in failing if s} | defective.get(routine, set()) |
                                  {s for s in script_names(top) if s not in stats and
                                   reach(s, routine, extents, rec, top)}))
    return out


def write_views(rec, chash, ext, top=TOP):
    """The three files progress.py and the README read (config/rondo/gate_a5.csv, gate_a7.csv,
    gate_a7_routines.csv), derived from the records, same columns as ever. A5 counts only the rows that
    count; a script whose result does not count shows its verdict as STALE or MISSING, never IDENTICAL."""
    counts = a5_counts(rec, chash, top)
    cfg = os.path.join(top, "config", "rondo")
    sources = sorted({r["source"] for r in rec.a5 if r["script"] in rec.scripts})
    src = sources[-1] if sources else ""
    rows = []
    for name in sorted(ext):
        run, length, disc, rom = ext[name]
        path = os.path.join(top, "build", "rondo", "trace", "cardmap.bin" if rom else "discmap.bin")
        ran = 0
        if os.path.exists(path):
            with open(path, "rb") as f:
                f.seek(disc)
                ran = sum(1 for b in f.read(length) if b & 3)
        t = counts.get(name, {"calls": 0, "identical": 0, "divergent": 0, "unverified": 0, "order_flags": 0})
        v = a5_verdict(t)
        rows.append({"routine": name, "card": rom, "bytes": length, "bytes_run": ran, "calls": t["calls"],
                     "identical": t["identical"], "divergent": t["divergent"], "unverified": t["unverified"],
                     "order_flags": t["order_flags"], "verdict": v, "source": src})
    write(os.path.join(cfg, "gate_a5.csv"), ["routine", "card", "bytes", "bytes_run", "calls", "identical",
          "divergent", "unverified", "order_flags", "verdict", "source"], rows)
    stats = a7_scripts(rec, chash, ext, top)
    srows = []
    for s in script_names(top):
        status_, verdict = stats.get(s, ("unmeasured", ""))
        row = rec.scripts.get(s, {})
        srows.append({"script": s, "verdict": verdict if status_ == "current" else
                      ("MISSING" if status_ == "unmeasured" else "STALE"),
                      "detail": row.get("detail", ""), "source": row.get("source", "")})
    write(os.path.join(cfg, "gate_a7.csv"), ["script", "verdict", "detail", "source"], srows)
    agg = {}
    for r in rec.a7:
        if r.get("epoch") == epoch(top) and stats.get(r["script"], ("", ""))[0] == "current" \
                and row_counts(r, chash, rec, top):
            t = agg.setdefault(r["routine"], [0, 0])
            t[0] += int(r["swapped"])
            t[1] += int(r["ran_inside"])
    write(os.path.join(cfg, "gate_a7_routines.csv"), ["routine", "swapped", "ran_inside", "source"],
          [{"routine": k, "swapped": v[0], "ran_inside": v[1], "source": src} for k, v in sorted(agg.items())])
