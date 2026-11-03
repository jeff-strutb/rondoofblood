#!/bin/sh
# Stage the project inside the repo. Nothing is installed on the host.
#
#   ./setup.sh            hooks, then check and split the reference disc
#
# The pinned assembler and emulator core join this script once their pins
# are chosen (docs/rondo/PLAN.md, section 5); bumping a pin means
# re-verifying every gate, so pins change deliberately and nowhere else.
set -eu
top="$(cd "$(dirname "$0")" && pwd)"
python3 "$top/tools/common/install_hooks.py"
python3 -I "$top/tools/rondo/disc.py"
python3 -I "$top/tools/rondo/image.py"
