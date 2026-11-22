#!/bin/bash
# liftloop.sh NAME... [--script SCRIPT]   (run from a writer's worktree)
#
# The checks of docs/rondo/PROCEDURE.md section 5, in order, for routines you have just filed. It stops at the
# first step that fails, printing that step's last lines. Every output goes under this worktree's build/rondo/loop/.
#
#   1  asmbuild --raw     the whole image assembled, intake text written       slot (-j 1, 3 GB)
#   2  image gate         0 differing bytes                                   slot (-j 1, 2 GB)
#   3  hybrid table       python3 tools/rondo/hybrid.py                       no slot
#   4  liftcmp NAME...    every routine against its draft, calls through      no slot
#   5  pcebox             this worktree's C, into its own folder              slot (-j 1, 3 GB)
#   6  gate A5 on SCRIPT  one script (default attract_82), your own folders   slot (-j 1, 2 GB)
#
# Steps 5 and 6 are skipped with --no-a5. Step 6 prints each NAME's row of the A5 table; a NAME the script does
# not reach reads UNCOVERED (not a failure here: pick a script from the routine's "run by" line).
# A slot wait is not a hang: cpu-run prints its queue state.
set -u
W=$(cd "$(dirname "$0")/../.." && pwd)
cd "$W" || exit 2
CR=${CPU_RUN:-$(command -v cpu-run || ls "$HOME"/.c*/bin/cpu-run 2>/dev/null | head -1)}
[ -x "$CR" ] || { echo "liftloop: cpu-run not found; set CPU_RUN"; exit 2; }
NAMES=(); SCRIPT=attract_82; A5=1
while [ $# -gt 0 ]; do
    case "$1" in
        --script) SCRIPT=$2; shift ;;
        --no-a5) A5=0 ;;
        *) NAMES+=("$1") ;;
    esac
    shift
done
[ ${#NAMES[@]} -gt 0 ] || { echo "usage: tools/rondo/liftloop.sh NAME... [--script SCRIPT] [--no-a5]"; exit 2; }
O=$W/build/rondo/loop
mkdir -p "$O" "$W/build/rondo/box" "$W/build/rondo/oracle"
step() {  # step LABEL LOGFILE CMD...
    local label=$1 log=$2; shift 2
    printf '%s %s\n' "$(date +%T)" "$label"
    if ! "$@" > "$log" 2>&1; then
        echo "liftloop: FAILED at: $label (log $log); last lines:"
        tail -12 "$log"
        exit 1
    fi
}
step "asmbuild --raw" "$O/asmbuild.log" "$CR" -j 1 -m 3G -- python3 tools/rondo/asmbuild.py --raw build/rondo/raw
grep -E "^coverage" "$O/asmbuild.log" | cut -c1-200
step "image gate" "$O/image.log" "$CR" -j 1 -m 2G -- python3 tools/rondo/image.py --user build/rondo/asm/image.bin
tail -1 "$O/image.log"
grep -q "0 differing bytes" "$O/image.log" || { echo "liftloop: the image differs"; tail -5 "$O/image.log"; exit 1; }
step "hybrid table" "$O/hybrid.log" python3 tools/rondo/hybrid.py
printf '%s liftcmp\n' "$(date +%T)"
python3 tools/rondo/liftcmp.py "${NAMES[@]}" > "$O/liftcmp.log" 2>&1; RC=$?
grep -v "^  cov" "$O/liftcmp.log" | cut -c1-260
if [ $RC -ne 0 ] || grep -q "NOT CHECKED\|THIN" "$O/liftcmp.log"; then
    echo "liftloop: liftcmp did not pass (differ, NOT CHECKED or THIN: never a pass); full report build/rondo/liftcmp/report/"
    exit 1
fi
[ $A5 -eq 1 ] || { echo "liftloop: done (A5 skipped)"; exit 0; }
step "pcebox" "$O/pcebox.log" env RONDO_BOX="$W/build/rondo/box/pcebox" "$CR" -j 1 -m 3G -- ./setup.sh pcebox
step "A5 $SCRIPT" "$O/a5.log" env RONDO_BOX="$W/build/rondo/box/pcebox" RONDO_ORACLE_OUT="$W/build/rondo/oracle" "$CR" -j 1 -m 2G -- python3 tools/rondo/oracle.py a5 "$SCRIPT"
for n in "${NAMES[@]}"; do grep "^$n," build/rondo/oracle/a5.csv || echo "$n: no A5 row"; done
echo "liftloop: done"
