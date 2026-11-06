# System Card 3.0

The Japanese Super CD-ROM² System 3.0 HuCard's BIOS, decompiled as its own project inside
this repo, so that it can move to a repo of its own. Nothing is decompiled here yet.

## Keeping it separate

- Everything of the card's lives under `syscard/`: its source, assembly, names and docs.
- Nothing here names or includes anything of the game's. Rondo is one caller among many.
- The game reaches the card only through one interface header, typed functions named as
  Hudson names the entries.
- To move it: take `syscard/` with its history, and point the game at the header.

## Filing

The same standard as any assembly-based decompilation: symbolic assembly under `syscard/asm/`,
bit exact against the card's own image, and proper C under `syscard/src/`, behaviour exact.

A routine is named by where it lies in the card's ROM, not on any disc: `@card BB:AAAA name`,
the ROM bank and the address it runs at, then `WHAT IT DOES:` and `HOW:`. The header is the same
above the C and above the assembly. Names are Hudson's where public notes give them, confirmed
only by what the code does.

## What is known

From reading the image and the game's traces (`build/rondo/trace/cardmap.bin`), 2026-11-05:

- 262,144 B, 32 banks of 8 KB. Bank 0 runs at $E000 and opens with a table of 81 `JMP`s.
- The traces ran 4,649 B: 3,918 in bank 0, 703 in bank 1, 28 in bank 3. The scripts call 26
  of the 81 entries, and the other stages may call more.
- Decoding from the table finds 6,977 B of code in bank 0, of which 3,625 B ran. It also
  jumps out of bank 0, to code at $C000-$CFFF and into RAM stubs. How much code the other
  banks hold is not measured.
- Of the 26 called entries, E05D reads the flags on entry (`PHP`, to restore them on the way
  out). The first instructions of the others read none, except that E01B compares A and
  E033 tests a port; none of the 26 was followed past its first seven instructions.
- Some entries leave flags alone: E02D (`AND`, `STA`, `RTS`) leaves C and V as they were on
  entry. A caller's exit can show those flags.
- The CD and ADPCM entries drive the hardware at $1800 directly and may wait on it, and
  44 `TAM`s in bank 0 map other banks in. A5 cannot judge calls that interrupts overlap, so
  some may be hard to certify.
