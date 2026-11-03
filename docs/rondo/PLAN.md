# Rondo of Blood decomp: plan

Target: *Akumajou Dracula X: Chi no Rondo* (Japan), Konami, 1993, for the
PC Engine Super CD-ROM² (System Card 3.0). Reference: the Redump set, 22
tracks, cue SHA-1 `ebb1350121815493f7179556ca81b8a904a4b05f`; every track
is pinned in `tools/rondo/disc.py`.

The method is the SNES decompilation playbook (nativeshvc,
`docs/snes-decomp-learnings.md`), as carried through Stunt Race FX and
Aladdin, adapted to a CD system whose program lives on the disc and runs
from RAM.

## 1. Standard

The game was written in assembly, so it has two decompilations, as the
SNES decomps do:

| | |
|---|---|
| ASM | the bit-exact decompilation: symbolic source that rebuilds the disc's data track byte for byte (image gate) |
| C | the behaviour-exact decompilation: proper C, each routine's inputs to outputs exactly the original's (A5 + A7) |
| Enhanced | later, beside the C, never in place of it |

**Proper C** means typed arguments and returns, locals, loops, structs and
named RAM: written as C, not as assembly in C's syntax. No register file,
flag helpers or gotos standing in for branches in a routine's body; the
registers and flags the original leaves visible (to its caller, at a wait,
to a callee) are set only in the routine's `abi_` wrapper, the one place
that meets the assembly's calling convention. Register-shaped C is at most
a stepping stone, reported as one with its count, never as decompiled
(playbook sections 0.15 and 18). Where ASM and C disagree, the ASM and
the original disc are right.

Cycle exact is not the standard (playbook section 15): the C carries no
instruction timing.

| Tier | Meaning |
|---|---|
| T1 | traced; in the raw intake with address labels; reassembles exact |
| T2 | C written and filed, assembly named and filed; not yet certified |
| T3 | the C is proper C and certified behaviour exact: gates 0, C, A5, A7 pass |
| T4 | T3, and its assembly is symbolic and byte identical in place in the whole image |

M1 = every routine T3. M2 = every routine T4 and the data track rebuilt
byte exact. Progress is measured in code bytes on the disc, with code no
script runs (the census) in the denominator.

## 2. What is different from the SNES decomps

| | |
|---|---|
| Medium | No ROM. The program is on the disc and runs from RAM: the System Card's boot reads one sector to $3000, and the game's own loader reads the rest through the card's BIOS |
| The image | Track 02's user data, 10,149 sectors of 2,048 B (20,785,152 B). Track 22 is a copy of its first 9,999 sectors; both rebuild byte for byte from it (`image.py`) |
| Overlays | Pieces of the program are read into the same RAM banks at different times, so a run address does not name a routine. Routines are named by disc location (`@routine SSSS:OOO`) and say where they run (`@run BB:AAAA`) |
| Code bytes | Found by folding RAM execution traces back onto the disc sectors that loaded them, as the SPC700 lanes fold APU RAM onto ROM: a RAM byte counts for a disc byte only while it still holds that byte |
| Processors | One: the HuC6280 runs the game and its PSG sound driver. CD-DA (20 audio tracks) and ADPCM are streams, data, not code |
| Not the game's | The System Card 3.0 BIOS (a HuCard ROM, entered through the $E000 jump table) is fenced out like a C runtime: called, never decompiled |
| Memory | MMU of 8 MPRs mapping 8 KB banks of a 2 MB physical space: CD RAM $80-$87 (64 KB), Super System Card RAM $68-$7F (192 KB), work RAM $F8, I/O $FF. Zero page $2000, stack $2100 |

## 3. Status (2026-11-03)

Phase 0 is under way. Done and verified:

- The reference set: all 23 files hashed and pinned; every MODE1 sector of
  both data tracks checked (sync, header address, mode, EDC): 0 bad.
- The disc layer: both data tracks re-encoded from user data (sync,
  header, EDC, P and Q parity) rebuild with 0 differing bytes on their
  pinned SHA-1s; planting one user byte fails the gate (27 and 32 bytes
  differ, at the expected sector). This is the sector layer only: the
  user data is still taken whole from the disc, so it is no decompilation
  progress.
- The boot read by hand: the boot block, the boot program and its load
  table (HANDBOOK, Code).
- A HuC6280 decoder with all 234 defined opcodes (`huc.py`), which writes
  the intake's text form in ca65 syntax.
- ca65 and ld65 (cc65 at `77b64846`) built in the repo. Assembler control
  (`asmcontrol.py`): 910 instruction forms over all 234 defined opcodes,
  operands chosen to tempt a shorter form, plus the 22 undefined opcodes
  as data: 2,388 B, 0 differing bytes. Dropping one operand size fails it.
- Beetle PCE (Mednafen's accurate PC Engine, `b96c11e0`) built headless in
  the repo with one local patch: arm64, 54 libretro exports, the same
  library from a clean build. Not yet patched with hooks; not yet run, as
  it has no System Card image.

No routine is at any tier. The code-byte denominator is unknown until the
first traces.

## 4. Phases

| Phase | | State |
|---|---|---|
| 0 | Layout, hooks, provenance; disc split and image gate; assembler staged and controlled; core built headless with trace hooks, accuracy settings | done but for the core's hooks, the headless runner and a first boot, which wait on the System Card image (section 5) |
| 1 | A dozen scripts from power-on through every mode; trace; fold RAM execution onto the disc; T1 raw intake; image gate from source | not started |
| 2 | A5 and A7 proven on one tiny routine with negative controls; catalog | not started |
| 3 | Routines bottom up, smallest first, committed per verified batch | not started |
| 4 | Coverage closure and the static census | not started |

## 5. Toolchain and open decisions

Decided 2026-11-03 (fetch approved by the maintainer):

- **Assembler:** ca65 and ld65 from cc65, pinned (`.setcpu "huc6280"`;
  `z:` and `a:` force operand sizes, as `.b` and `.w` did under asar),
  accepted on the control above.
- **Core:** Mednafen's accurate `pce` emulation (the libretro Beetle PCE
  core, not Beetle PCE Fast), pinned, to be patched for the trace, check
  and lockstep hooks: the role bsnes played for the SNES.

Open, the maintainer's:

1. **The System Card 3.0 image** the core boots from. It is not in
   `references/`; nothing can run without one. It is pinned by hash once
   supplied.

## 6. Limits known before building (playbook rule 11)

1. **Overlays.** The oracles must know which piece is resident when a
   routine is entered: C is keyed by disc location, and a call is judged
   only when the RAM at its run address holds that piece's bytes.
2. **Compressed or generated code.** If any piece is decompressed or built
   in RAM rather than read straight from the disc, the fold cannot map it;
   that code would need its own lane. Unknown until traced.
3. **CD timing.** Reads, seeks and CD-DA cues take real drive time, which
   the game may wait on or race. The core must model the drive accurately;
   a native build would need a drive model of its own.
4. **BIOS calls inside game routines.** The C calls the BIOS by address. A
   native build would need those calls implemented natively; no PC Engine
   counterpart of nativeshvc exists yet.
5. **Witnesses.** The PSP's *Dracula X Chronicles* (2007) and the PS4's
   *Castlevania Requiem* (2018) carry the original game; whether either
   holds a byte-identical program, and what it would yield, is not
   measured.
