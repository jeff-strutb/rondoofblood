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
#   4. fast-forwards main to the branch.
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
export M W BR O CR
exec "$CR" -j 1 -m 3G -- flock "$M/build/rondo/land.lock" sh -c '
  cd "$W" || exit 2
  D=$(git status --porcelain --untracked-files=all -- asm src syscard tools ":(exclude)tools/toolchains")
  [ -z "$D" ] || { echo "LAND REFUSED (dirty): uncommitted changes in the worktree, commit or drop them: $(echo "$D" | head -5 | tr "\n" ";")"; exit 3; }
  if ! git rebase -q main >"$O/rebase.log" 2>&1; then
    F=$(git diff --name-only --diff-filter=U | head -5 | tr "\n" " ")
    git rebase --abort
    echo "LAND REFUSED (conflict): rebase on main conflicts in $F; run git rebase main in the worktree, resolve, commit, land again"; exit 4
  fi
  if git diff --quiet main HEAD -- asm src syscard tools; then
    echo "decomp tree identical to main: image and link checks skipped"
  else
    for r in references tools/toolchains/src tools/toolchains/cc65 tools/toolchains/pce build/rondo/disc build/rondo/system build/rondo/trace; do
      [ -e "$W/$r" ] || { mkdir -p "$(dirname "$W/$r")"; ln -sfn "$M/$r" "$W/$r"; }
    done
    python3 tools/rondo/landcheck.py image "$O" || exit 5
    python3 tools/rondo/landcheck.py link "$O" || exit 6
  fi
  git -C "$M" merge -q --ff-only "$BR" || { echo "LAND REFUSED (main): main could not fast-forward to $BR (main checkout has changes at those paths, or main moved); land again"; exit 7; }
  echo "LANDED $(git -C "$M" log --oneline -1)"
'
