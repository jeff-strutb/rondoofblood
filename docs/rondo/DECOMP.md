# Rondo of Blood decomp: standard, tools, layout

## Standard

| | |
|---|---|
| ASM | byte exact: the disassembled assembly reassembles to the original data track, 0 differing bytes |
| C | behaviour exact: the decompiled C does exactly what the assembly does |
| Done | every routine of the original's code decompiled to both |

The writer knows both as it writes, from the assembly and the disc
([PLAN.md](PLAN.md), section 1). No gates, runners or ledgers.

## Every routine

The header comment above each routine (C and assembly) carries:

```
@routine <sector>:<offset> <Name>
@run <bank>:<addr>
WHAT IT DOES: its purpose, in game terms
HOW: how it does it
WAITS: no, or what it waits on and how deep its stack is there
```

plus its entry contract (inputs in A, X, Y and zero page, the MPR banks it
expects, outputs, flags, what it clobbers). The sector counts from track
02's first user sector, as the game's loader counts; the bank is the
physical 8 KB bank the run address is mapped to. Routines are filed by
responsibility, never by sector or address range.

## Layout

| | |
|---|---|
| `src/rondo/<area>/` | the C decompilation (behaviour exact) |
| `src/rondo/include/` | C headers: machine model, RAM maps, structs |
| `asm/rondo/<area>/` | the ASM decompilation (byte exact), same areas as the C |
| `asm/rondo/include/` | hardware registers, RAM maps, BIOS entry points, macros |
| `config/rondo/` | symbols, records, disc tables |
| `tools/rondo/` | disc, image build and compare, decoder, readers, name generators |
| `tools/toolchains/` | the pinned assembler (fetched by `setup.sh`) |
| `docs/rondo/` | this file, the standard, the procedure, the handbook |
| `references/` | the disc and the System Card image (not tracked) |
| `build/rondo/` | everything generated |

## Tools

| | |
|---|---|
| `setup.sh` | fetch and build the pinned cc65; check the disc and System Cards, split the disc, rebuild the data tracks |
| `tools/rondo/disc.py` | check every reference file's SHA-1 and every data sector; write track user data and `tracks.csv` |
| `tools/rondo/sector.py` | MODE1 sector encoding: sync, header, EDC, P and Q parity |
| `tools/rondo/image.py` | both data tracks rebuilt from user data and compared with the disc, 0 differing bytes or exit 1; `--user` takes the assembled image; `--plant` must fail |
| `tools/rondo/asmbuild.py` | the whole data track assembled from the filed assembly in place, the rest from the disc |
| `tools/rondo/asmcontrol.py` | ca65 encodes every HuC6280 instruction form exactly; `--plant` must fail |
| `tools/rondo/huc.py` | HuC6280 decoder and linear listing |
| `tools/rondo/reach.py` | reads a piece of the disc as code from its entry points |
| `tools/rondo/codemap.py` | the static code map: what loads where, the code followed from the disc's entries, the copies; `--progress` writes the README's progress picture and its M1 and M2 by disc area |
| `tools/rondo/asmfile.py` | drafts a C file's assembly from the disc, with its header and names |
| `tools/rondo/ramnames.py` | writes `ram.h`, `ram.inc` and the record structs from `config/rondo/symbols.csv` and `records.csv` |
| `tools/rondo/typedrec.py` | the records file's format and its C structs |
| `syscard/tools/image.py` | the card's filed assembly in place against the card's image |

```bash
python3 tools/rondo/disc.py
python3 tools/rondo/asmbuild.py
python3 tools/rondo/image.py --user build/rondo/asm/image.bin
python3 tools/rondo/asmcontrol.py
python3 tools/rondo/huc.py build/rondo/disc/track02.bin --org 0x3000 --offset 0x1000 --len 0x130
```
