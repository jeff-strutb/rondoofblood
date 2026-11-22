#!/bin/sh
# land.sh WORKTREE: land a writer's finished batch on main, one landing at a time.
#
# Takes one cpu-run slot first, then the landing lock (build/rondo/land.lock in the main checkout:
# the runner, the state session and the master commit to main under the same lock), then:
#   1. refuses uncommitted changes in asm, src, syscard, tools (untracked files count);
#   2. rebases the worktree's branch on main (a conflict aborts the rebase and refuses);
#   3. when asm, src, syscard or tools differ from main: assembles all filed assembly in place and
#      requires the whole game image and the whole card image byte exact, and builds the hybrid from
#      the worktree's C and requires a clean link, no undefined abi_ symbol (landcheck.py);
#   4. when the landing adds or changes a file in tools/rondo/scripts, traces those scripts (and the ones
#      including a changed include file) into a private copy of the trace folder, one cpu-run slot each, up to
#      4: a failed or inconsistent trace refuses the landing here, before main is touched;
#      (before 5: a landing that changes tools/toolchains/patches/pce applies the whole patch set, in order, to a
#      clean copy of the pinned core source and is refused if any patch fails)
#   5. fast-forwards main to the branch;
#   6. then publishes the new traces and the re-merged code maps into main's build/rondo/trace, rebuilds the
#      catalog there, and prints the merged code bytes before and after and any catalog entry that now
#      falls inside a routine with C and has no tag (worklist.py --catalog).
# The trace step and the runner refuse when main's core is older than the newest core patch (cpuguard.py core).
# Every output goes to the landing worktree's own build/rondo/land-out. A refusal prints one line
# "LAND REFUSED (rule): what to do" and exits non-zero.
set -u
[ $# -eq 1 ] && [ -d "$1" ] || { echo "usage: land.sh WORKTREE"; exit 2; }
W=$(cd "$1" && pwd)
COMMON=$(git -C "$W" rev-parse --path-format=absolute --git-common-dir) || { echo "LAND REFUSED (input): $W is not a git worktree"; exit 2; }
M=$(dirname "$COMMON")
[ "$W" != "$M" ] || { echo "LAND REFUSED (input): $W is the main checkout; land from a worktree"; exit 2; }
BR=$(git -C "$W" rev-parse --abbrev-ref HEAD)
[ "$BR" != "main" ] && [ "$BR" != "HEAD" ] || { echo "LAND REFUSED (input): the worktree must be on its own branch, not $BR"; exit 2; }
# The machine's queue command: CPU_RUN, else cpu-run on PATH, else the one in the user'"'"'s tool folder.
CR=${CPU_RUN:-$(command -v cpu-run || ls "$HOME"/.c*/bin/cpu-run 2>/dev/null | head -1)}
[ -x "$CR" ] || { echo "LAND REFUSED (input): cpu-run not found; set CPU_RUN to the queue command"; exit 2; }
O=$W/build/rondo/land-out
mkdir -p "$O" "$M/build/rondo"
# A landing that touches scripts claims one slot per script to trace (up to 4, 2 GB each).
NS=$(git -C "$W" diff --name-only --diff-filter=ACMR "$(git -C "$W" merge-base main "$BR")" "$BR" -- tools/rondo/scripts | wc -l)
[ "$NS" -ge 1 ] || NS=1
[ "$NS" -le 4 ] || NS=4
MEM=$((1 + 2 * NS))
[ "$MEM" -ge 3 ] || MEM=3
export M W BR O CR
echo "land: queued for one CPU slot and the landing lock; other sessions' jobs and landings go first, so this can wait minutes though the work itself takes about 15 s (cpu-run prints its queue state; this is not a hang)"
exec "$CR" -j "$NS" -m "${MEM}G" -- flock "$M/build/rondo/land.lock" sh -c '
  cd "$W" || exit 2
  D=$(git status --porcelain --untracked-files=all -- asm src syscard tools ":(exclude)tools/toolchains")
  [ -z "$D" ] || { echo "LAND REFUSED (dirty): uncommitted changes in the worktree, commit or drop them: $(echo "$D" | head -5 | tr "\n" ";")"; exit 3; }
  if ! git rebase -q main >"$O/rebase.log" 2>&1; then
    F=$(git diff --name-only --diff-filter=U | head -5 | tr "\n" " ")
    git rebase --abort
    echo "LAND REFUSED (conflict): rebase on main conflicts in $F; run git rebase main in the worktree, resolve, commit, land again"; exit 4
  fi
  # The commit hooks do not run when a rebase replays commits, so their checks are made here, on
  # every message and file this landing adds to main.
  for c in $(git rev-list main..HEAD); do
    git log -1 --format=%B "$c" > "$O/msg.txt"
    python3 tools/common/provcheck.py --msg "$O/msg.txt" > "$O/prov.txt" 2>&1 || { echo "LAND REFUSED (provenance): commit $(echo $c | cut -c1-7) message: $(head -2 "$O/prov.txt" | tr "\n" " ")"; exit 8; }
  done
  F=$(git diff --name-only --diff-filter=ACMR main HEAD)
  [ -z "$F" ] || python3 tools/common/provcheck.py $F > "$O/prov.txt" 2>&1 || { echo "LAND REFUSED (provenance): $(head -3 "$O/prov.txt" | tr "\n" " ")"; exit 8; }
  for r in references tools/toolchains/src tools/toolchains/cc65 tools/toolchains/pce build/rondo/disc build/rondo/system; do
    [ -e "$W/$r" ] || { mkdir -p "$(dirname "$W/$r")"; ln -sfn "$M/$r" "$W/$r"; }
  done
  mkdir -p "$W/build/rondo"
  if [ -L "$W/build/rondo/trace" ] || [ ! -e "$W/build/rondo/trace" ]; then ln -sfn "$M/build/rondo/trace" "$W/build/rondo/trace"; fi
  # New or changed scripts: trace them here, in a private trace folder (main is not touched), so a failure
  # refuses the landing with main as it was, and before the checks, whose build refuses a script with no trace. The folder holds links to main existing script folders, except
  # the ones being traced, and the merged maps written into it cover all of them.
  S=$(python3 tools/rondo/landcheck.py scripts main) || exit 9
  if [ -n "$S" ]; then
    P=$(python3 tools/rondo/cpuguard.py core "$M") || { echo "LAND REFUSED (trace): $P"; exit 9; }
    [ -x "$M/build/rondo/pcebox/pcebox" ] || { echo "LAND REFUSED (trace): main has no pcebox binary; run setup.sh pcebox in the main checkout"; exit 9; }
    T=$O/trace; rm -rf "$T"; mkdir -p "$T"
    for d in "$M"/build/rondo/trace/*/; do
      n=$(basename "$d")
      case " $S " in *" $n "*) ;; *) ln -s "$M/build/rondo/trace/$n" "$T/$n" ;; esac
    done
    P=""; for n in $S; do P="$P tools/rondo/scripts/$n.txt"; done
    B=$(python3 tools/rondo/landcheck.py code "$M/build/rondo/trace/discmap.bin")
    RONDO_TRACE_OUT=$T RONDO_PCEBOX=$M/build/rondo/pcebox/pcebox python3 tools/rondo/trace.py $P > "$O/trace.log" 2>&1 \
      || { echo "LAND REFUSED (trace): tracing $S failed, main untouched: $(grep -v merged: "$O/trace.log" | tail -2 | tr "\n" " ")"; exit 9; }
    A=$(python3 tools/rondo/landcheck.py code "$T/discmap.bin")
    echo "traced $S into a private folder: $(grep "^merged:" "$O/trace.log")"
    if [ -L "$W/build/rondo/trace" ]; then ln -sfn "$T" "$W/build/rondo/trace"; fi    # the intake in the checks below sees the new traces
  fi
  # A landing that touches the core patches: the whole set must apply, in order, on a clean copy of the pinned source.
  if ! git diff --quiet main HEAD -- tools/toolchains/patches/pce; then
    PIN=$(sed -n "s/^PCE_PIN=\([0-9a-f]*\).*/\1/p" setup.sh)
    CP=$O/patchcheck; rm -rf "$CP"
    { [ -n "$PIN" ] && [ -d "$M/tools/toolchains/src/pce/.git" ] && git clone -q --no-checkout "$M/tools/toolchains/src/pce" "$CP" && git -C "$CP" checkout -q "$PIN"; } 2>"$O/patchcheck.log" \
      || { echo "LAND REFUSED (patches): could not make a clean copy of the pinned core source ($PIN) from main: $(tail -1 "$O/patchcheck.log")"; exit 11; }
    for q in tools/toolchains/patches/pce/*.patch; do
      git -C "$CP" apply "$W/$q" 2>"$O/patchcheck.log" || { echo "LAND REFUSED (patches): $(basename "$q") does not apply on the pinned core source after the patches before it: $(head -2 "$O/patchcheck.log" | tr "\n" " ")"; rm -rf "$CP"; exit 11; }
    done
    rm -rf "$CP"
    echo "core patches: all $(ls tools/toolchains/patches/pce/*.patch | wc -l) apply in order on a clean copy of the pinned source"
  fi
  if git diff --quiet main HEAD -- asm src syscard tools; then
    echo "decomp tree identical to main: image and link checks skipped"
  else
    python3 tools/rondo/landcheck.py image "$O" || exit 5
    python3 tools/rondo/landcheck.py link "$O" || exit 6
  fi
  OLDMAIN=$(git -C "$M" rev-parse HEAD)
  git -C "$M" merge -q --ff-only "$BR" || { echo "LAND REFUSED (main): main could not fast-forward to $BR (main checkout has changes at those paths, or main moved); land again"; exit 7; }
  echo "LANDED $(git -C "$M" log --oneline -1)"
  PP=$(git diff --name-only --diff-filter=ACMR "$OLDMAIN" HEAD -- tools/toolchains/patches/pce | tr "\n" " ")
  [ -z "$PP" ] || echo "WARNING: this landing adds or changes core patch(es) $PP: main core is now out of date; rebuild it before the next measurement or trace: cpu-run -j 4 -m 6G -- sh setup.sh pce, then sh setup.sh pcebox, in $M"
  if [ -n "$S" ]; then
    TM=$M/build/rondo/trace
    for n in $S; do
      rm -rf "$TM/$n.old"
      if [ -e "$TM/$n" ] || [ -L "$TM/$n" ]; then mv "$TM/$n" "$TM/$n.old"; fi
      mv "$T/$n" "$TM/$n"; rm -rf "$TM/$n.old"
    done
    for m in discmap.bin cardmap.bin noprov.bin discmap2.bin; do cp "$T/$m" "$TM/$m.new" && mv "$TM/$m.new" "$TM/$m"; done
    echo "trace published to main: $S; merged disc code bytes $B -> $A"
    (cd "$M" && python3 tools/rondo/catalog.py | tail -3 && python3 tools/rondo/worklist.py --catalog) \
      || { echo "LANDED, but the catalog step failed: run catalog.py in the main checkout"; exit 10; }
  fi
'
