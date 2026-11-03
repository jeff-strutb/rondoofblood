# Rondo of Blood decomp: standard, tools, layout

## Standard

| | |
|---|---|
| T1 | traced; in the raw intake (address labels); reassembles exact |
| T2 | C written and filed, assembly named and filed; not yet certified |
| T3 | the C is certified behaviour exact: every T3 gate passes |
| T4 | T3, and the routine's assembly is symbolic and byte-identical in place in the whole-image gate |
| EXCLUDED | beside the tiers: code the retail game provably never runs, proof in the routine header |
| M1 | every game routine T3 |
| M2 | every game routine T4, and the data track rebuilt byte exact |

Gates are defined in [PLAN.md](PLAN.md). Query the tree for counts; do not
trust a number written in a file.

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
| `src/rondo/<area>/` | the C companion |
| `src/rondo/include/` | C headers: machine model, RAM maps, structs |
| `asm/rondo/<area>/` | the decompilation, same areas as the C |
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
| `setup.sh` | install hooks, check and split the disc, run the image gate |
| `tools/rondo/disc.py` | check every reference file's SHA-1 and every data sector; write track user data and `tracks.csv` |
| `tools/rondo/sector.py` | MODE1 sector encoding: sync, header, EDC, P and Q parity |
| `tools/rondo/image.py` | image gate: both data tracks rebuilt from user data, 0 differing bytes or exit 1; `--plant` proves it fails |
| `tools/rondo/huc.py` | HuC6280 decoder and linear listing, for reading |
| `tools/rondo/precommit.py` | filing rules on staged files |

```bash
python3 tools/rondo/disc.py
python3 tools/rondo/image.py
python3 tools/rondo/huc.py build/rondo/disc/track02.bin --org 0x3000 --offset 0x1000 --len 0x130
```
