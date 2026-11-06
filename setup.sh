#!/bin/sh
# Stage the toolchain inside the repo. Nothing is installed on the host.
#
#   ./setup.sh            hooks, toolchain, then check and split the disc
#   ./setup.sh cc65       one component (cc65, pce, pcebox, disc)
#
# Sources land in tools/toolchains/src/<name> at a pinned commit; build
# products in tools/toolchains/<name>/. Bumping a pin means re-verifying
# every gate, so pins change deliberately and nowhere else.
set -eu
top="$(cd "$(dirname "$0")" && pwd)"
tc="$top/tools/toolchains"
mkdir -p "$tc/src"
jobs="$(sysctl -n hw.ncpu 2>/dev/null || nproc)"

CC65_URL=https://github.com/cc65/cc65
CC65_PIN=77b6484615427746e55fda2755c9cbc320a1fb51      # master, 2026-11-03
PCE_URL=https://github.com/libretro/beetle-pce-libretro
PCE_PIN=b96c11e095b6a40a412d2da02766ae1c3f4fd539       # master, 2026-11-03

fetch() { # name url pin
    d="$tc/src/$1"
    if [ ! -d "$d/.git" ]; then
        git init -q "$d"
        git -C "$d" remote add origin "$2"
    fi
    if [ "$(git -C "$d" rev-parse -q --verify HEAD 2>/dev/null || true)" != "$3" ]; then
        git -C "$d" fetch -q --depth 1 origin "$3"
        git -C "$d" checkout -q --force "$3"
    fi
    # Local patches live in the repo and are applied on a clean checkout,
    # again whenever the patch set changes.
    pd="$tc/patches/$1"
    if [ -d "$pd" ] && ls "$pd"/*.patch >/dev/null 2>&1; then
        stamp="$(cat "$pd"/*.patch | shasum | cut -c1-40)"
        if [ "$(cat "$d/.patch-stamp" 2>/dev/null || true)" != "$stamp" ]; then
            git -C "$d" reset -q --hard "$3"
            git -C "$d" clean -qfd
            for p in "$pd"/*.patch; do git -C "$d" apply "$p"; done
            echo "$stamp" > "$d/.patch-stamp"
            touch "$d/.rebuild"
        fi
    fi
    return 0
}

build_cc65() {
    fetch cc65 "$CC65_URL" "$CC65_PIN"
    make -C "$tc/src/cc65/src" -j "$jobs" ca65 ld65 >/dev/null
    mkdir -p "$tc/cc65"
    for t in ca65 ld65; do
        cp "$tc/src/cc65/bin/$t" "$tc/cc65/.$t.new"
        mv "$tc/cc65/.$t.new" "$tc/cc65/$t"
    done
    "$tc/cc65/ca65" --version 2>&1 | head -1
}

build_pce() {
    fetch pce "$PCE_URL" "$PCE_PIN"
    mkdir -p "$top/build"
    # New patches: build from clean, so no object predates them.
    if [ -e "$tc/src/pce/.rebuild" ]; then
        make -C "$tc/src/pce" clean >/dev/null 2>&1
        rm "$tc/src/pce/.rebuild"
    fi
    # The SDK's libc++ warns on every file; the log keeps it, a failure shows it.
    make -C "$tc/src/pce" -j "$jobs" >"$top/build/pce-make.log" 2>&1 ||
        { tail -20 "$top/build/pce-make.log"; exit 1; }
    mkdir -p "$tc/pce"
    # Built into a new file and moved in: never overwrite a core a run is using.
    cp "$tc/src/pce/mednafen_pce_libretro.dylib" "$tc/pce/.core.new" 2>/dev/null ||
        cp "$tc/src/pce/mednafen_pce_libretro.so" "$tc/pce/.core.new"
    mv "$tc/pce/.core.new" "$tc/pce/mednafen_pce_libretro.dylib"
    echo "pce core: $(shasum "$tc/pce/mednafen_pce_libretro.dylib" | cut -c1-12)"
}

build_pcebox() {
    mkdir -p "$top/build/rondo/pcebox"
    python3 -I "$top/tools/rondo/huc.py" --c-opclass > "$top/build/rondo/pcebox/opclass.h"
    python3 -I "$top/tools/rondo/hybrid.py"
    # A planted bug comes in as RONDO_SRC, a copy of src/rondo with one change.
    src="${RONDO_SRC:-$top/src/rondo}"
    out="${RONDO_BOX:-$top/build/rondo/pcebox/pcebox}"
    cc -O2 -Wall -Wextra -I "$tc/src/pce/libretro-common/include" -I "$top/build/rondo/pcebox" \
        -I "$src/include" -I "$top/syscard/include" -o "$out.new" "$top/tools/rondo/pcebox/pcebox.c" \
        "$top/tools/rondo/pcebox/trace.c" "$top/tools/rondo/pcebox/check.c" \
        "$top/build/rondo/pcebox/hybrid_table.c" "$src"/*/*.c "$top/syscard/src"/*.c -ldl
    mv "$out.new" "$out"
    echo "pcebox: built"
}

disc() {
    python3 -I "$top/tools/rondo/disc.py"
    python3 -I "$top/tools/rondo/image.py"
}

if [ $# -eq 0 ]; then
    python3 "$top/tools/common/install_hooks.py"
    build_cc65
    build_pce
    build_pcebox
    disc
else
    for c in "$@"; do
        case "$c" in
            cc65) build_cc65 ;;
            pce) build_pce ;;
            pcebox) build_pcebox ;;
            disc) disc ;;
            *) echo "unknown component: $c" >&2; exit 2 ;;
        esac
    done
fi
