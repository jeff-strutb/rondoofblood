# System Card 3.0

The Japanese Super CD-ROM² System 3.0 HuCard's BIOS, decompiled as its own project inside
this repo, so that it can move to a repo of its own. It is held to the game's standard
([docs/rondo/PLAN.md](../docs/rondo/PLAN.md)): assembly that reassembles to the card's image byte
for byte, and C that does exactly what the assembly does.

## Keeping it separate

- Everything of the card's lives under `syscard/`: its source, assembly, names and docs.
- Nothing here names or includes anything of the game's. Rondo is one caller among many.
- The game reaches the card only through one interface header, typed functions named as
  Hudson names the entries.
- To move it: take `syscard/` with its history, and point the game at the header.

## Filing

Symbolic assembly under `syscard/asm/`, byte exact against the card's own image
(`syscard/tools/image.py`), and proper C under `syscard/src/`, behaviour exact.

A routine is named by where it lies in the card's ROM, not on any disc: `@card BB:AAAA name`,
the ROM bank and the address it runs at, then `WHAT IT DOES:` and `HOW:`. The header is the same
above the C and above the assembly. Names are Hudson's where public notes give them, confirmed
only by what the code does.

## What is known

- 262,144 B, 32 banks of 8 KB. Bank 0 runs at $E000 and opens with a table of 81 `JMP`s.
- Decoding from the table finds 6,977 B of code in bank 0. It also jumps out of bank 0, to code
  at $C000-$CFFF and into RAM stubs. How much code the other banks hold is not measured.
- E05D reads the flags on entry (`PHP`, to restore them on the way out). Some entries leave flags
  alone: E02D (`AND`, `STA`, `RTS`) leaves C and V as they were on entry, so a caller's exit can
  show those flags.
- The CD and ADPCM entries drive the hardware at $1800 directly and may wait on it, and 44 `TAM`s
  in bank 0 map other banks in.

## The card's machine, as its C sees it

`include/card.h`: `rd8` and `wr8` on logical addresses, `tma` and `tam`, `cli`, and `csl` and
`csh` for the CPU's clock, which the backup memory's routines change and leave changed for their
caller. Registers and flags appear only in `abi_` wrappers and `_shim` functions.

## Status, 2026-11-27

- 198 routines filed as C and assembly; the card's image rebuilds with all of them in place, 0
  differing bytes.
- ex_joysns is not filed: its soft reset jumps through a vector set at run time, which C that
  runs no original code cannot do.
