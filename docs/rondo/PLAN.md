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
| C | the behaviour-exact decompilation: proper C, each routine's inputs to outputs exactly the original's: the RAM it leaves, the registers and flags it leaves visible, and its hardware accesses in order (A5 + A7). The C makes every work RAM read, write, push and pull where the assembly makes it, in its order and number. No value is kept in a C variable across a point where the original reads memory again (decided 2026-11-21; it replaces "the order and count of plain RAM accesses are the ASM's to keep") |
| Enhanced | later, beside the C, never in place of it |

**Proper C** is the Aladdin standard, no stricter (the maintainer's ruling of
2026-11-26, which replaces the stricter wording that stood here): readable,
documented C per routine, which may keep the register file, flag helpers and
the pushed frames its proof needs, in its body or its `abi_` wrapper, as
Aladdin's certified C does. It may not run original code: code not yet C is
not decompiled, reported with its count (`ledger.py`, proper_c_faults), and A7
counts it against the script there. Where ASM and C disagree, the ASM and
the original disc are right.

**Shaped for the native target** (decided 2026-11-21, from Aladdin's
second step, where C shaped only by the proof had to be rewritten whole):
RAM is reached through typed fields laid over the original layout, a
struct member at the original address, not through bare addresses; records
(an object slot, a sound channel, a save file) have their own types; disc
tables are read through one typed data layer, each table's bank settled
from traces, never from a comment. A call the original makes with JSR or
BSR pushes the original's return address on the emulated stack at the
original's place, through one helper. Each routine's header says whether
it can wait, and how deep.

**The typed access layer** (decided 2026-11-21). The decomp's C is an
intermediary to a later port, and this layer is what lets a routine's body
survive that move:

- One macro per access, taking a named field: `RD8(OBJ->vx_hi[slot])`,
  `WR8(ZP->temp10, v)`. Each expands to exactly one checked access of the
  member's original address, so order and count stay visible to the
  gates. The struct is laid at the original address and is never
  dereferenced. For the port only the macros' bodies change.
- A field wider than a byte is read and written through a macro that names
  the order (`RD16_LH`, `RD16_HL` and the like). There is none without an
  order.
- Records (object slots, sound channels, save files, zero page) are
  described in a file of their own, not in `symbols.csv`, and their
  structs are written by the generator, which adds a compile-time check
  that every member sits at its original address. The assembly's include does not change.
- Disc tables are read through generated readers from a table list whose
  bank column a tool fills from the traces. A table read under two banks
  has a row per overlay; a table no script reads has no reader until a
  script reaches it.
- It is required for T3: a routine's body names no bare address. Every
  filed routine is rewritten for access order anyway, and this is done in
  the same rewrite, not a second one.
- The lists of names and records have one owner. A writer that needs a new
  name or field sends it to that owner; no two writers edit those files.

Cycle exact is not the standard (playbook section 15): the C carries no
instruction timing.

**The gates.**

| Gate | Proves |
|---|---|
| 0 | the routine's bytes in the built image equal the disc's, and its header is complete |
| C | every byte of the routine was run by a script, or is exempt by proof |
| A5 | each call of the C, alone, gives the original's outputs, with its accesses in the original's order |
| A7 | (not a gate for T3 until the game is done: the ruling below) whole scripts in lockstep, the C running in place for every call: calls an interrupt lands in, calls that cross a frame, calls into original code and waits included. A routine passes when every script that reaches it is identical and measured |

**Exempt by proof** (decided 2026-11-21): only bytes that no input, no
poke of work RAM and no unit call at the routine's own entry can run
sanely (a path that would run data as code, a branch its own code makes
impossible). The proof and the exact byte range go in the routine's
header. Dead code is not exempt for being dead: it gets a unit call at its
own entry or a poke, and certified C. "The checker cannot follow it" is a
limit of the checker, not a proof.

**The denominator** is every code byte: run by a script, or found by a
static census that follows calls, jumps, tables and handler pointers. The
census is audited as hard as the gates: tables read to their full length,
no block thrown out for an opcode the game uses, and a second, relaxed
census whose every disagreeing byte is explained.

**Duplicates and unreached code** (the maintainer's ruling, 2026-11-26):
"If it can be proven to be duplicate code (for disc read speed purposes?)
then we should record it as such and leave it out. If it's actual code
(not data) we should keep it until we prove otherwise. Unreachable code
still needs to be decomp'd even if we can't reach it, we just do so
optimistically since we can't prove it until something reaches it and
document it as such." Applied:

1. A disc area proven a byte-exact copy of code counted elsewhere goes in
   a recorded list of duplicates (its range, the copy it duplicates, the
   proof) and out of the denominator.
2. Anything that decodes as code and is not proven data or a proven
   duplicate stays in the denominator, loadable or not.
3. Code nothing reaches stays in, and is filed with C marked unreached.
   Amended by the maintainer the same day: "If we prove it unreachable,
   it counts in our progress until we prove otherwise yes." So code
   proven unreachable (no load path, input, poke or unit call can run it),
   once filed as proper C, counts in M1. Its header records the proof and
   marks the C optimistic, not gate-proven. If a script ever reaches it,
   it is judged by the gates like any other routine, and it leaves the
   count if it fails. Code that is only unreached, with no proof, does
   not count until a script runs it and the gates pass it. The README
   shows code no script runs as a line of its own.

| Tier | Meaning |
|---|---|
| T1 | traced; in the raw intake with address labels; reassembles exact |
| T2 | C written and filed, assembly named and filed; not yet certified |
| T3 | the C is proper C and certified behaviour exact: gates 0, C and A5 pass (A7 not required: the ruling below) |
| T4 | T3, and its assembly is symbolic and byte identical in place in the whole image |

**A5 alone certifies** (the maintainer's ruling of 2026-11-27, verbatim: "A5 it is, solely, until we
think the game is done. Then and only then we discuss more."). A routine is certified on its own
calls: each call's real inputs from the record, run through the C, every output the original's
(gate A5), with every byte of it run (gate C). Lockstep (A7) is not run by the runner and certifies
nothing until the game is done; the ledger still shows its column.

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
| Not the game's | The System Card 3.0 BIOS (a HuCard ROM, entered through the $E000 jump table) is in scope as its own project, in its own top-level folder `syscard/`, kept separate so it can move to its own repo; the game reaches it through one interface header. M1 and M2 count the game's bytes only; the card has its own measure (Jeff's decisions in chat, 2026-11-05). The card has both decompilations, as the game does: bit-exact assembly under `syscard/asm/` against its own image, and behaviour-exact proper C under `syscard/src/`, a routine filed as `@card BB:AAAA name`. Its C is written before the game's callers stop calling it by address: A5 is not narrowed (Jeff's decisions in chat, 2026-11-06) |
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
- Beetle PCE (Mednafen's accurate PC Engine, `b96c11e0`) built in the
  repo with two local patches: bundled zlib for the macOS SDK, and
  read-only inspection exports (RAM banks, MPRs, PC).
- Both System Card 3.0 images pinned: Japan (the target) and USA (a
  witness).
- `pcebox`, the headless runner, with every emulation option answered
  explicitly. First boot: the load table's blocks land exactly where the
  hand reading predicted, the kernel loads on, the Konami logo fades in;
  overlays reuse RAM within four seconds. The USA card loads identical
  Super and CD RAM.
- Tracing (core patch 0003, `trace.c`, `trace.py`): every executed and
  read byte folded onto the disc through each RAM byte's disc origin,
  consistent with the disc at every dump. Scripts reach the menus,
  Stage 0 and Stage 1: 44,847 B of disc code traced. Estimated total
  about 196 KB of distinct code (131 to 260 KB; `estimate.py`).

Status 2026-11-05, from [the ledger](ledger.csv), which `tools/rondo/ledger.py` works out:
404 routines (13,112 B) have assembly and C, and the image gate passes with them in place
(0 differing). 71 are at T4 and 1 at T3; 46 of those are one-line jump vectors, leaving 26
routines of real logic, 526 B. The ledger's proper-C check is a pattern check, so those 26
are a ceiling until reviewed. 209 routines (11,072 B) fail it: register-shaped C, a
stepping stone, not decompiled, being redone bottom up. A5 judged 13,747,708 calls
identical and 0 divergent, A7 119,770 frames identical, swapping 111 routines; 293 were not
swapped ([log](decomp/log/oracles-2026-11-05.md)). The denominator is still the estimate.

## 4. Phases

| Phase | | State |
|---|---|---|
| 0 | Layout, hooks, provenance; disc split and image gate; assembler staged and controlled; core built headless with trace hooks, accuracy settings | done |
| 1 | A dozen scripts from power-on through every mode; trace; fold RAM execution onto the disc; T1 raw intake; image gate from source | trace and fold done; 6 scripts (boot, attract, menus, Stage 0-1); the other stages, the menus' other paths, Maria and the ending not yet reached; T1 intake next |
| 2 | A5 and A7 proven with negative controls; catalog | A5 and A7 built and proven on a planted bug; A7 cannot yet swap a routine that runs original code (153 of 404); catalog not started |
| 3 | Routines bottom up, smallest first, committed per verified batch | 404 routines filed, 209 of them failing the proper-C check and being redone; T1 intake and the catalog were skipped and are still owed |
| 4 | Coverage closure and the static census | not started |

## 5. Toolchain and open decisions

Decided 2026-11-03 (fetch approved by the maintainer):

- **Assembler:** ca65 and ld65 from cc65, pinned (`.setcpu "huc6280"`;
  `z:` and `a:` force operand sizes, as `.b` and `.w` did under asar),
  accepted on the control above.
- **Core:** Mednafen's accurate `pce` emulation (the libretro Beetle PCE
  core, not Beetle PCE Fast), pinned, to be patched for the trace, check
  and lockstep hooks: the role bsnes played for the SNES.

- **System Card:** the Japanese Super CD-ROM² System 3.0 is the target
  (the card the game shipped against); the USA card is a witness.

Nothing is open.

## 7. Plan from 2026-11-21

The method from here is Aladdin's, which reached M1 and M2 100%
(`aladdinsvhc`, `docs/aladdin/ASM console decom strategy.md`). Where it and
this project's earlier practice disagree, Aladdin's wins (the maintainer's
decision, 2026-11-21).

### What was measured (main at 5692076, and the scripts branch at f3fb945)

| | |
|---|---|
| Filed | 603 game routines, about 20 KB, and 135 card routines, each with assembly and C |
| M1, M2 | 2.36%, 2.27%, last measured at 00d4c3c against the 196 KB estimate; the ledger was not rerun for the eight C commits after it |
| A5, 11 scripts (8a1b865) | 456 identical, 34 unverified, 246 uncovered, 2 divergent |
| A5, 32 scripts (f3fb945) | 476 identical, 32 unverified, 184 uncovered, 46 divergent |
| A7 | identical on every script, both times |
| Traced code, 32 scripts | 210,017 B, more than the 196 KB estimate used as the denominator |

**A7 as built does not test access order.** A call runs the original, not
the C, when the original took an interrupt inside it, when it crossed a
frame, or when its C reaches original code or a wait (`check.c`, the
hybrid's entry test). A5 counts the same calls unverified. Neither
compares the stack or the order of work RAM accesses. So the A7 results
above are true and do not yet meet gate A7 as section 1 now defines it,
and every T3 and T4 counted so far is unproven until the new checker
measures it.

**Missing on main:** a runner that tests only what changed, a landing
script, a written procedure for writers, a worklist that names the failing
gate, an audited census, and a unit call for scripts.

### Roles

- **The master** thinks, briefs and certifies. It writes no game C and no
  tools.
- **Writers** decompile and write coverage scripts, each in its own
  worktree, split by file so no two touch the same files. They do not run
  gates.
- **One runner** certifies: it tests only the routines whose compiled C
  changed and the scripts that reach them, reports failures, holds
  nothing, and commits the ledgers itself.
- **Landing** goes through one script under one lock.

### Order of work

| Step | Work | Done when |
|---|---|---|
| 1 | **State.** Gate records for f3fb945 committed; all scripts traced again; catalog, ledger and progress rebuilt; the census audited (section 1) and put in the denominator | the README's numbers are measured at one named commit with every script having a result |
| 2 | **Checker.** Lockstep A7 as section 1 defines it: interrupts delivered inside C calls at the original's place, C calling original code, waits, return frames on the emulated stack; A5 judging work RAM order; a unit call and a poke for scripts | planted bugs of each kind (a read moved, a read doubled, a frame left out) fail it, and the original against itself passes |
| 3 | **Pipeline.** The landing script (rebase, assembly byte exact, link clean, fast forward, one lock); the runner; a worklist by script that prints each routine short of the bar, the gate that fails and the reason; a what-if that predicts the bars for a fix | a change to one routine is certified without the whole game being rerun |
| 4 | **Procedure.** One rules file every writer reads, amended with a dated section each time a new cause is found | written before any writer starts |
| 5 | **Measure main** with the new checker, then clear reds by cause, largest predicted gain first | every filed routine has a verdict from the new gates |
| 6 | **File everything**: the traced code with no C yet, many writers by disc range, bottom up | every traced routine has assembly and C |
| 7 | **Coverage** for the unreached: a script with real input, then pokes, then a unit call at the routine's own entry, then exempt by proof | gate C passes or is exempt for every byte in the denominator |
| 8 | **Symbolic assembly** last | M2 |

Steps 1 to 3 run at once, on separate files. Step 4 is the master's.

### Order changed, 2026-11-21: draft and prove first, proper C second

Measured with the new checker, about 410 of the 738 filed routines fail:
their C was written as typed C first (values read up front, exit flags
worked out afterwards by reading memory again, calls restructured, the
stack unused). Only about a fifth could be patched; most would be
rewritten beside the assembly, and with that C in place the boot runs with
over a million access defects, so no routine can be judged in a clean
lockstep. Both finished projects went the other way, and this one now
does (the maintainer's decision, 2026-11-21):

1. **Draft.** A tool writes, for every traced routine, C that follows its
   assembly instruction for instruction on the register file, so every
   access, push, pull and call frame is where the original makes it by
   construction. One function per distinct routine content, serving every
   disc location that holds it. This is register-shaped C: a stepping
   stone, never counted as decompiled, reported with its count.
2. **Prove.** The drafts are judged by the new A5 and the lockstep A7
   with C in place for every routine, until every script is identical.
3. **Lift.** Each routine is then written as proper C over the typed
   access layer, filed by responsibility, and judged by the same gates in
   a hybrid where every other routine is proven C. The 603 filed routines'
   C is the starting text for their lift.

Two bars are reported, never one for the other: behaviour proven (a
routine whose draft or filed C passes the gates) and M1 (proper C at T3).

Steps 5 to 7 of the table above follow this order: the drafter comes
before any writer, and writers lift.

### Rules carried from Aladdin

- Before a fix, state the expected result in numbers. A miss is explained
  from evidence before anything else is done.
- A red script is rerun alone before it is believed. No report is
  published while a script has no result.
- A new script's trace can add catalog entries inside code that already
  has C: rebuild the catalog with it and give every new entry its own tag
  and C in the same landing.
- A coverage script that pokes RAM is checked against the original alone
  first.
- A checker change may only make the judgement more correct. Its landing
  lists every routine whose verdict changed, with the evidence.
- Concurrent runs never share a checker binary or an output folder.
- A session's report is not proof: the diff is read or the gate run
  before its claim is repeated.

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
