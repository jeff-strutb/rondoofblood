# Rondo of Blood decomp: the standard

Target: *Akumajou Dracula X: Chi no Rondo* (Japan), Konami, 1993, for the
PC Engine Super CD-ROM² (System Card 3.0). Reference: the Redump set, 22
tracks, cue SHA-1 `ebb1350121815493f7179556ca81b8a904a4b05f`; every track
is pinned in `tools/rondo/disc.py`.

## 1. The standard (the maintainer's, 2026-11-27)

The game was written in assembly, so it has two decompilations.

| | Exact when |
|---|---|
| **ASM: byte exact** | the disassembled assembly reassembles to the original: `asmbuild.py` then `image.py --user build/rondo/asm/image.bin` gives 0 differing bytes on both data tracks |
| **C: behaviour exact** | the decompiled C does exactly what the assembly does |

**Done** is every routine of the original's code decompiled to both. There
is nothing else: no gates, no runners, no tiers, no ledgers, no
certification.

**No trial and error.** Nothing runs afterwards to tell a writer whether
the assembly or the C is right. The writer knows as it writes, because it
writes to the standard:

- The assembly is read from the disc's own bytes by the decoder, so its
  bytes are the original's by construction; names and comments change no
  byte. The image build shows it, and a difference there is a mistake in
  the filing (a wrong `@routine` or `@run`, an operand size), found from
  the listing, not by retrying.
- The C is written from the assembly, instruction by instruction in
  meaning: every effect the assembly has, the C has, and nothing more
  (section 2). A writer that cannot say, for each instruction, where its
  effect is in the C has not finished the routine.

Where the assembly and the C disagree, the assembly and the original disc
are right.

## 2. What behaviour exact means

The C does what the assembly does, as its callers and the hardware see it:

- Every work RAM read and write, every hardware access, every push and
  pull, and every TAM, CSL and CSH the original makes, the C makes: where
  the assembly makes it, in its order and number. No value is kept in a C
  variable across a point where the original reads memory again.
- The registers and flags a caller can see when the routine returns are
  the original's.
- Every call the original makes, the C makes, to the callee's C, with the
  original's return address on the emulated stack.
- The C runs no original code. A call into code that has no C yet means
  the caller is not finished.

**Proper C**, the Aladdin standard, no stricter (the maintainer's ruling of
2026-11-26): readable, documented C per routine, which may keep the
register file, flag helpers and the pushed frames that exactness needs, in
its body or its `abi_` wrapper. Typed arguments, locals, loops, structs
and named RAM are preferred wherever exactness allows.

**Shaped for the native target** (2026-11-21): RAM is reached through
typed fields laid over the original layout (the typed access layer,
PROCEDURE section 3); records (an object slot, a sound channel, a save
file) have their own types; disc tables are read through one typed data
layer. Each routine's header says whether it can wait, and how deep.

Cycle exact is not the standard: the C carries no instruction timing.

## 3. Finding the code

The code is found by analysing the disc, never by running the game.

- **What loads where.** The boot program and the game's loader read
  sectors into RAM banks through the System Card's `cd_read` and its
  relatives. Each load names its sector, count and destination; reading
  those calls and the tables that feed them gives every piece's run
  address.
- **What is code.** Decoding from known entries (the load's jump target,
  vectors, `JSR`, `JMP`, branch targets, jump tables read to their full
  length, handler pointers) follows the code. A byte is data when code
  reads it, sends it to a port, or no code path reaches it as an
  instruction. Opcodes the HuC6280 does not define do not occur in code.
- **Copies.** A CD game with little RAM keeps the same code in many places
  on the disc and loads whichever copy it needs. Before a routine is
  disassembled or decompiled, the disc is searched for a finished match,
  and a match is reused, never written again:
  - same bytes at the same run address: the same routine; its assembly and
    C serve the copy;
  - same bytes at a different run address: the same routine only if none
    of its absolute operands point inside itself;
  - same instructions with absolute operands moved by the load address:
    one routine filed at two origins, one C, its assembly assembled at
    each origin so both copies stay byte exact.
- **Unreached code is still code.** Every routine is decompiled whether or
  not anything is known to reach it (the maintainer's ruling of
  2026-11-26).

`codemap.py` does this for the whole disc: the loads (the boot's table and
every scene, in the order the game loads them), the code followed from the
card's call into the disc, the boot program, the kernel's vectors and the
handlers it gives the card, and the copies. Still open in it: copies loaded
at another address, and finding code in regions nothing loads.

**Progress.** M1 is the share of the game's code, every copy counted once,
in routines whose C is finished (it calls no original game code); M2 is the
share in routines that also have their assembly filed. `codemap.py
--progress` measures both by disc area for the README.

## 4. What is different from the SNES decomps

| | |
|---|---|
| Medium | No ROM. The program is on the disc and runs from RAM: the System Card's boot reads one sector to $3000, and the game's own loader reads the rest through the card's BIOS |
| The image | Track 02's user data, 10,149 sectors of 2,048 B (20,785,152 B). Track 22 is a copy of its first 9,999 sectors; both rebuild byte for byte from it (`image.py`) |
| Overlays | Pieces of the program are read into the same RAM banks at different times, so a run address does not name a routine. Routines are named by disc location (`@routine SSSS:OOO`) and say where they run (`@run BB:AAAA`) |
| Processors | One: the HuC6280 runs the game and its PSG sound driver. CD-DA (20 audio tracks) and ADPCM are streams, data, not code |
| Not the game's | The System Card 3.0 BIOS (a HuCard ROM, entered through the $E000 jump table) is its own project in `syscard/`, held to the same standard against its own image, a routine filed as `@card BB:AAAA name`. The game reaches it through one interface header |
| Memory | MMU of 8 MPRs mapping 8 KB banks of a 2 MB physical space: CD RAM $80-$87 (64 KB), Super System Card RAM $68-$7F (192 KB), work RAM $F8, I/O $FF. Zero page $2000, stack $2100 |

## 5. Merging

Work that meets the standard is merged to main. Work that does not waits
on its branch until it does. There are no hooks and no landing script.

## 6. Toolchain

- **Assembler:** ca65 and ld65 from cc65, pinned in `setup.sh`
  (`.setcpu "huc6280"`; `z:` and `a:` force operand sizes).
  `asmcontrol.py` shows ca65 encodes every HuC6280 instruction form
  exactly.
- **System Card:** the Japanese Super CD-ROM² System 3.0 is the target;
  the USA card is a witness.

## 7. Limits known before building

1. **Overlays.** C is keyed by disc location; which piece is resident at a
   run address is settled from the loader's calls (section 3).
2. **Generated code.** Code the game builds in RAM (the block-move stubs
   of HANDBOOK, Code) has no disc bytes; the code that writes it is what
   is filed.
3. **CD timing.** Reads, seeks and CD-DA cues take real drive time, which
   the game may wait on. A native build needs a drive model of its own.
4. **BIOS calls.** The C calls the BIOS through the card's interface
   header; a native build implements those entries natively.
5. **Witnesses.** The PSP's *Dracula X Chronicles* (2007) and the PS4's
   *Castlevania Requiem* (2018) carry the original game; whether either
   holds a byte-identical program is not measured.
