# Rondo of Blood: bit-exact decompilation

**Started November 3, 2026. Phase 0 (bootstrap): no routine is decompiled
yet.** The reference disc is checked and pinned. Its sector layer is
proven: both data tracks rebuild byte for byte from their user data. The
boot program and its loader are read. The assembler and the emulator core
are chosen but not yet staged ([Current Status](#current-status)).

> **Bit exact, behaviour exact.** The decompilation is symbolic, commented
> HuC6280 assembly that rebuilds the disc's data track byte for byte. Its
> companion is C for every routine, proven to behave exactly like the
> running original. The C states no instruction timing: instruction counts
> are an emulator's bookkeeping and were never part of the original
> source.

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

The work that is my own (the tools and the test harness) is dedicated to
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
CD-ROM², 1993), known in the West as *Rondo of Blood*. Every routine is to
be rewritten as readable C, documented with what it does and how, filed by
what it is for, and proven to behave exactly like the original. The whole
data track is to be rebuilt byte for byte from symbolic assembly. Progress
is measured only by those two proofs; playability is a consequence, never
a reason to reorder the work.

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
games of the time were written in assembly. That is confirmed from traced
code, not assumed.

## How a CD game differs from a cartridge

On a HuCard, or a SNES cartridge, the code sits in ROM at fixed
addresses. Here there is no ROM. The System Card's boot reads one sector
of the disc to $3000 and runs it. That program reads the game in through
the card's BIOS, into Super System Card RAM, and the game goes on reading
pieces of itself into the same RAM as it moves from stage to stage. So:

- **The image is the disc's data track:** track 02's 10,149 user sectors
  (20,785,152 B), code and data together. The image gate re-encodes them
  into the raw BIN (sync, header, EDC, P and Q parity) and compares
  against the original. Track 22 is the disc's backup copy of track 02's
  first 9,999 sectors and is rebuilt from the same bytes.
- **A routine is named by where it lies on the disc** (`@routine
  0043:000`: sector $43, offset 0, counted as the game's loader counts),
  with `@run` giving the bank and address it runs at, since pieces loaded
  at different times share run addresses.
- **Code bytes are found by folding RAM execution back onto the disc**
  sectors that loaded it, as the SNES decomps fold the sound CPU's RAM
  onto the ROM bytes uploaded there.
- **The System Card's BIOS is not the game's code.** It is called through
  its $E000 jump table and fenced out, as a C runtime is in a compiled
  game.

## Two lanes, one game

No compiler produced these bytes, so no C can rebuild them. Each routine
therefore has two sources, as in the Stunt Race FX and Aladdin decomps:

- **Assembly, the decompilation.** Symbolic, commented HuC6280 assembly
  that rebuilds the data track byte for byte (T4). It is the record of the
  game, and the reading the C is written from.
- **C, the companion.** Readable C per routine, certified behaviour exact
  against the running original (T3): at every call the scripts make, and
  with every routine swapped in at once, in lockstep with the original,
  frame for frame.

## Current Status

Phase 0, bootstrap. **0 routines at any tier.** The code-byte denominator
is unknown until the first traces. The bars and the progress map will be
generated by a progress tool once there is a catalog to classify.

| | State |
|---|---|
| Reference set | 23 files pinned by SHA-1; every MODE1 sector of both data tracks checked (sync, header address, mode, EDC): 0 bad |
| Sector layer | both data tracks rebuilt from user data: 0 differing bytes, pinned SHA-1s; a planted byte fails the gate. **This is the disc's encoding, not decompilation progress:** the user data is still taken whole from the disc |
| Boot | boot block, boot program and load table read by hand ([handbook](docs/rondo/HANDBOOK.md)): the game's resident kernel starts at sector $43 with a table of entry vectors |
| Decoder | HuC6280, all 234 defined opcodes, for reading; encodings not yet proven against an assembler |
| Hooks | provenance check and filing rules (`@routine`, `@run`, `WHAT IT DOES:`, `HOW:`) on every commit |
| Assembler | **not staged:** ca65 (cc65) proposed, to be pinned and controlled on every opcode form |
| Emulator core | **not staged:** Mednafen's accurate PC Engine (Beetle PCE), to be pinned and patched for trace, check and lockstep hooks |
| System Card 3.0 image | **not supplied:** the core cannot boot the game without it |
| T1 raw intake, A5, A7, catalog, census | not started |

Query the tree. Do not trust a number in this file.

```bash
python3 tools/rondo/disc.py      # pins, every data sector checked, user data split out
python3 tools/rondo/image.py     # image gate: both data tracks rebuilt, 0 differing bytes
python3 tools/rondo/image.py --plant 4100   # must fail
python3 tools/rondo/huc.py build/rondo/disc/track02.bin --org 0x3000 --offset 0x1000 --len 0x130
```

T1 = traced, raw intake only; T2 = C or assembly in the tree, not
certified; T3 = the C is certified behaviour exact; T4 = T3, and the
routine's assembly is symbolic, documented and byte-identical in place.
M1 = every routine T3; M2 = every routine T4 and the data track rebuilt
byte exact.

## More

- [docs/rondo/PLAN.md](docs/rondo/PLAN.md): the plan, the standard, what
  differs from the SNES decomps, open decisions and known limits.
- [docs/rondo/HANDBOOK.md](docs/rondo/HANDBOOK.md): what the work has
  learned, one rule per finding.
- [docs/rondo/DECOMP.md](docs/rondo/DECOMP.md): standard, layout and tools
  in one page.

### Architecture

    src/rondo/              the C companion, by area (not started)
    asm/rondo/              the decompilation, same areas as the C (not started)
    config/rondo/           load map, symbols, verdict ledgers (not started)
    tools/rondo/            disc, sector encoding, image gate, decoder, filing rules
    tools/common/           git hooks, provenance check
    docs/rondo/             plan, standard, handbook, dated notes
    references/             the disc and the System Card image (not tracked)
    build/                  everything generated

### Building & Setup

- **`./setup.sh`**: installs the hooks, checks and splits the disc, and
  runs the image gate. The pinned assembler and core join it once chosen.

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
