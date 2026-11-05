# Rondo of Blood decomp: standard, tools, layout

## Standard

| | |
|---|---|
| T1 | traced; in the raw intake (address labels); reassembles exact |
| T2 | C written and filed, assembly named and filed; not yet certified |
| T3 | the C is proper C and certified behaviour exact: every T3 gate passes |
| T4 | T3, and the routine's assembly is symbolic and byte-identical in place in the whole-image gate |
| EXCLUDED | beside the tiers: code the retail game provably never runs, proof in the routine header |
| M1 | every game routine T3 |
| M2 | every game routine T4, and the data track rebuilt byte exact |

ASM is the bit-exact decompilation, C the behaviour-exact one; the C is
proper C, with registers and flags only in each routine's `abi_` wrapper
([PLAN.md](PLAN.md), section 1). Gates are defined there. Query the tree
for counts; do not trust a number written in a file.

## Every routine

The header comment above each routine (C and assembly) carries:

```
@routine <sector>:<offset> <Name>
@run <bank>:<addr>
WHAT IT DOES: its purpose, in game terms
HOW: how it does it
```

plus its entry contract (inputs in A, X, Y and zero page, the MPR banks it
expects, outputs, flags, what it clobbers). The sector counts from track
02's first user sector, as the game's loader counts; the bank is the
physical 8 KB bank the run address is mapped to. The pre-commit hook
refuses a new `@routine` without `@run`, `WHAT IT DOES:` and `HOW:`, and
refuses sector- or address-named files outside the raw intake. Routines
are filed by responsibility, never by sector or address range.

## Layout

| | |
|---|---|
| `src/rondo/<area>/` | the C decompilation (behaviour exact) |
| `src/rondo/include/` | C headers: machine model, RAM maps, structs |
| `asm/rondo/<area>/` | the ASM decompilation (bit exact), same areas as the C |
| `asm/rondo/include/` | hardware registers, RAM maps, BIOS entry points, macros |
| `config/rondo/` | symbols, the load map, verdict ledgers |
| `tools/rondo/` | disc, image gate, decoder, build, gates, tracer |
| `tools/common/` | hooks, provcheck |
| `tools/toolchains/` | pinned assembler and emulator core (fetched by `setup.sh`), local patches |
| `docs/rondo/` | this file, the plan, the handbook, dated notes in `decomp/` |
| `references/` | the disc and the System Card image (not tracked) |
| `build/rondo/` | everything generated |

## Tools

| | |
|---|---|
| `setup.sh` | install hooks; fetch and build pinned cc65 and Beetle PCE, apply local patches, build pcebox; check the disc and System Cards, split the disc, run the image gate |
| `tools/rondo/asmcontrol.py` | assembler control: every HuC6280 instruction form round-trips through ca65; `--plant` must fail |
| `tools/rondo/disc.py` | check every reference file's SHA-1 and every data sector; write track user data and `tracks.csv` |
| `tools/rondo/sector.py` | MODE1 sector encoding: sync, header, EDC, P and Q parity |
| `tools/rondo/image.py` | image gate: both data tracks rebuilt from user data, 0 differing bytes or exit 1; `--plant` proves it fails |
| `tools/rondo/huc.py` | HuC6280 decoder and linear listing, for reading |
| `tools/rondo/pcebox/` | the original game run headless on the pinned core from a script in `tools/rondo/scripts/`: wait, press, shot, dump |
| `tools/rondo/loadcheck.py` | which disc sectors each RAM bank of a dump holds |
| `tools/rondo/ppm2png.py` | frame dumps to PNG |
| `tools/rondo/sheet.py` | frame dumps as one contact sheet |
| `tools/rondo/pcebox/trace.c` | `--trace`: executed and read bytes folded onto the disc through each RAM byte's disc origin; `--watch` lists the instructions touching a byte |
| `tools/rondo/trace.py` | every script traced in parallel under a timeout, maps merged into `build/rondo/trace/`, disc consistency enforced |
| `tools/rondo/estimate.py` | the code-size estimate, with its measured error |
| `tools/rondo/reach.py` | reads a piece of the disc as code from its entry points: a listing, not a proof |
| `tools/rondo/ramnames.py` | writes `ram.h` and `ram.inc` from `config/rondo/symbols.csv` |
| `tools/rondo/asmfile.py` | drafts a C file's assembly from the disc, with its header and names |
| `tools/rondo/asmbuild.py` | assembles the filed assembly in place in the whole track, called from Windows through WSL |
| `tools/rondo/hybrid.py` | the table of routines with C, for the oracles |
| `tools/rondo/pcebox/check.c` | A5 (`--a5`), and A7's two runs (`--record`, `--hybrid`) |
| `tools/rondo/oracle.py` | `a5`, `a7` and `plant` over the scripts, by routine, into `build/rondo/oracle/` |
| `tools/rondo/precommit.py` | filing rules on staged files |

```bash
python3 tools/rondo/disc.py
python3 tools/rondo/image.py
python3 tools/rondo/asmcontrol.py
python3 tools/rondo/huc.py build/rondo/disc/track02.bin --org 0x3000 --offset 0x1000 --len 0x130
```
