# Rondo of Blood: bit-exact decompilation

**Started November 3, 2026. Not finished.** The game is decompiled twice,
to one standard:

| | Decompilation | Exact when |
|---|---|---|
| **ASM** | symbolic, commented HuC6280 assembly | it reassembles to the original: the disc's data track rebuilt with 0 differing bytes |
| **C** | readable C per routine, documented, over named RAM and typed records | it does exactly what the assembly does |

It is done when every routine of the original's code is decompiled to
both. The writer knows both as it writes them, from the assembly and the
disc; nothing is run to find out ([the standard](docs/rondo/PLAN.md)).

**Maintainer:** Jeffrey Wilbur, Strut B, LLC\
**Contact:** [retro@strutb.com](mailto:retro@strutb.com)

## License, and no claim to the game

**I claim no ownership whatsoever of the decompiled source code.**
*Akumajou Dracula X: Chi no Rondo*, its code and its assets belong to its
rights holders. The assembly and C in this repository are a reconstruction
of their work, recovered so that they rebuild the original bytes. It is
not mine to own, license or give away, and nothing here says otherwise. No
game data ships in this repository: every build starts from your own copy
of the disc.

This project exists for preservation and for love of this game, so that
the Castlevania Konami made for the PC Engine CD in 1993 can still be
read, studied, built and played long after the discs, System Cards and
hardware it shipped on are gone.

The work that is my own (the tools) is dedicated to
the public domain under [CC0 1.0 Universal](LICENSE). Anyone may use it for
any purpose, with no permission or credit needed.

**Why CC0.** A preservation project should claim as little as possible.
CC0 asserts nothing over the reconstructed game, and it gives everything
that is mine to the public domain, with a fallback licence for countries
where copyright cannot be waived outright. That fallback is why CC0 was
chosen over the Unlicense, which has the same intent but no fallback.

## The Decompilation Standard

The purpose of a decompilation, as opposed to a recompilation, is
preservation: recovering the game itself, as written, in a form that
outlives the original hardware and media. Playability and portability are
welcome side effects, not the direct goal. With a bit-exact decompilation,
every function, built with the original compiler, must reproduce the
original machine code byte for byte. A bit-exact match is 100% reliability
that the source is accurate to the original, ensuring preservation for
future generations.

## Project Purpose

A decompilation of *Akumajou Dracula X: Chi no Rondo* (PC Engine Super
CD-ROM², 1993), known in the West as *Rondo of Blood*, in two forms. In
assembly, every routine symbolic and commented, and the whole data track
rebuilt from it byte for byte. In C, every routine rewritten as proper C,
documented with what it does and how, filed by what it is for, and doing
exactly what the assembly does. Playability is a consequence, never a
reason to reorder the work.

## Game background

Konami released *Akumajou Dracula X: Chi no Rondo* in Japan on October 29,
1993. It is a Super CD-ROM² title: it needs NEC's System Card 3.0, and
its boot program reads the game into the card's extra 192 KB of RAM.
Richter Belmont crosses Dracula's castle to rescue the women taken from
his village. It has branching stages, a second playable character, an
animated opening and spoken cutscenes, and most of its music is Red Book
audio played from the disc. It stayed in Japan until *Castlevania: The
Dracula X Chronicles* (PSP, 2007), which carries the original game inside
it, as the Wii's Virtual Console release and *Castlevania Requiem* (PS4,
2018) later did.

The machine is small and unusual. One processor, Hudson's HuC6280 (a
65C02 with block moves, an 8-bank memory mapper and a 6-channel wavetable
sound generator on the chip), runs everything: game logic, video and the
PSG sound driver. The HuC6270 video controller and the HuC6260 colour
encoder draw the picture. The CD unit adds 64 KB of RAM, an ADPCM voice
chip and CD-DA playback. No compiler footprint is expected: PC Engine
games of the time were written in assembly.

## How a CD game differs from a cartridge

On a HuCard, or a SNES cartridge, the code sits in ROM at fixed
addresses. Here there is no ROM. The System Card's boot reads one sector
of the disc to $3000 and runs it. That program reads the game in through
the card's BIOS, into Super System Card RAM, and the game goes on reading
pieces of itself into the same RAM as it moves from stage to stage. So:

- **The image is the disc's data track:** track 02's 10,149 user sectors
  (20,785,152 B), code and data together. `image.py` re-encodes them
  into the raw BIN (sync, header, EDC, P and Q parity) and compares
  against the original. Track 22 is the disc's backup copy of track 02's
  first 9,999 sectors and is rebuilt from the same bytes.
- **A routine is named by where it lies on the disc** (`@routine
  0043:000`: sector $43, offset 0, counted as the game's loader counts),
  with `@run` giving the bank and address it runs at, since pieces loaded
  at different times share run addresses.
- **Code is found by analysing the disc:** the loader's reads say which
  sectors land at which run address, and decoding from their entry points
  separates code from data. The same code is kept in many places on the
  disc, so each routine is decompiled once and serves every copy.
- **The System Card's BIOS is not the game's code.** It is called through
  its $E000 jump table, and is decompiled as its own project in
  `syscard/`, never counted in the game's progress.

## Two decompilations, one game

When a game was compiled from C, the right C fed to the original compiler
gives back the original bytes, so one source carries both proofs. No
compiler produced these bytes, so no C can rebuild them. Each routine
therefore has two decompilations, as in the Stunt Race FX and Aladdin
decomps:

- **ASM, byte exact.** Symbolic, commented HuC6280 assembly that
  reassembles to the data track byte for byte: every address a name, every
  routine with the same header as its C. Where the two disagree, the
  assembly and the original disc are right.
- **C, behaviour exact.** Readable, documented C per routine, held to the
  Aladdin standard, that does exactly what its assembly does: every
  access in the assembly's order and number, the registers and flags a
  caller sees, every call. It runs no original code.

## Current Status

<!-- progress:start -->
![Progress by disc area](docs/rondo/progress.svg)

**M1 31.9%, M2 12.7%** of 375,212 B of distinct code found so far by the static code map (every copy counted once). The map is not finished, so the total will grow and the percentages fall. Generated by `tools/rondo/codemap.py --progress`.
<!-- progress:end -->

Measured on this commit, 2026-11-27. Query the tree; do not trust a
number in this file.

| | |
|---|---|
| Assembly | 2,783 game routines filed; the whole data track assembles from them, the rest from the disc's own bytes, with 0 differing bytes on both data tracks |
| C | 2,783 game routines have C, every one with its assembly filed; none calls original game code (calls into the System Card are by design) |
| System Card | 198 routines with assembly and C; the card's image rebuilds with 0 differing bytes |
| Code map | `tools/rondo/codemap.py` finds the code from the disc alone: 375,212 B of distinct code so far. Its to-do list (`--todo`): 2,115 routines done (103,643 B), 3,785 to do (184,064 B), every copy counted once |

The C filed before 2026-11-27 was written under an earlier process of
gates and runners, which is kept on the branch `shelf/gates-runners` and
is no longer the standard.

### Commands

```bash
./setup.sh                        # the pinned assembler; the disc checked and split
python3 tools/rondo/asmbuild.py   # the whole data track assembled from source
python3 tools/rondo/image.py --user build/rondo/asm/image.bin   # 0 differing bytes, or exit 1
python3 tools/rondo/image.py --plant 4100   # must fail
python3 tools/rondo/asmcontrol.py           # ca65 encodes every HuC6280 instruction form exactly
python3 tools/rondo/huc.py build/rondo/disc/track02.bin --org 0x3000 --offset 0x1000 --len 0x130
```

## More

- [docs/rondo/PLAN.md](docs/rondo/PLAN.md): the standard, how code is
  found, what differs from the SNES decomps, known limits.
- [docs/rondo/PROCEDURE.md](docs/rondo/PROCEDURE.md): how a routine is
  written and merged.
- [docs/rondo/DECOMP.md](docs/rondo/DECOMP.md): standard, layout and tools
  in one page.
- [docs/rondo/HANDBOOK.md](docs/rondo/HANDBOOK.md): what the work has
  learned, one rule per finding.

### Architecture

    src/rondo/              the C decompilation, behaviour exact, by area: audio, math, menu,
                            object, system, and the headers in include/
    asm/rondo/              the ASM decompilation, bit exact, same areas
    syscard/                the System Card's own decompilation (src/, asm/, include/, tools/)
    config/rondo/           symbols, records, disc tables
    tools/rondo/            disc, image build and compare, decoder, readers, name generators
    tools/toolchains/       pinned cc65 (fetched by setup.sh)
    docs/rondo/             standard, procedure, layout, handbook
    references/             the disc and the System Card image (not tracked)
    build/                  everything generated

### Building & Setup

- **`./setup.sh`**: fetches cc65 at its pinned commit into the repo and
  builds it (nothing installed on the host), checks the disc and the
  System Cards, splits the disc and rebuilds both data tracks from it.
  `./setup.sh cc65` or `disc` runs one part.

### Reference disc (you supply; not tracked in git)

```
references/Akumajou Dracula X - Chi no Rondo (Japan)/      Redump set, 22 tracks
  ...(Japan).cue                 SHA-1  ebb1350121815493f7179556ca81b8a904a4b05f        2,565 bytes
  ...(Track 02).bin  MODE1/2352  SHA-1  c9b092fb15af2a7436f425bf29ed5d948f9c9640   24,399,648 bytes
                                 MD5    4cb1cb4858b88122fe4ea413050fd012
  ...(Track 22).bin  MODE1/2352  SHA-1  7edaa39557dcb62d58b5c9dfbcade46475963e57   24,049,200 bytes
                                 MD5    92e8492ec535caca38f5d625c2e359c7
  ...(Track 01, 03-21).bin       AUDIO  (pinned in tools/rondo/disc.py)
```

`disc.py` checks every file's SHA-1 and refuses any other set.

### System Cards (you supply; not tracked in git)

```
references/[BIOS] Super CD-ROM System (Japan) (En) (v3.0).pce         the target
    SHA-1  79f5ff55dd10187c7fd7b8daab0b3ffbd1f56a2c     262,144 bytes
    MD5    38179df8f4ac870017db21ebcbf53114
references/[BIOS] TurboGrafx CD Super System Card (USA) (v3.0).pce    witness
    SHA-1  d02611d99921986147c753df14c7349b31d71950     262,144 bytes
    MD5    0754f903b52e3b3342202bdafb13efa5
```
