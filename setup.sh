#!/bin/sh
# Stage the toolchain inside the repo. Nothing is installed on the host.
#
#   ./setup.sh            the assembler, then check and split the disc
#   ./setup.sh cc65       one component (cc65, disc)
#
# Sources land in tools/toolchains/src/<name> at a pinned commit; build
# products in tools/toolchains/<name>/. Bumping a pin means rebuilding the
# image byte exact, so pins change deliberately and nowhere else.
set -eu
top="$(cd "$(dirname "$0")" && pwd)"
tc="$top/tools/toolchains"
mkdir -p "$tc/src"
jobs="$(sysctl -n hw.ncpu 2>/dev/null || nproc)"

CC65_URL=https://github.com/cc65/cc65
CC65_PIN=77b6484615427746e55fda2755c9cbc320a1fb51      # master, 2026-11-03

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

disc() {
    python3 -I "$top/tools/rondo/disc.py"
    python3 -I "$top/tools/rondo/image.py"
}

if [ $# -eq 0 ]; then
    build_cc65
    disc
else
    for c in "$@"; do
        case "$c" in
            cc65) build_cc65 ;;
            disc) disc ;;
            *) echo "unknown component: $c" >&2; exit 2 ;;
        esac
    done
fi
