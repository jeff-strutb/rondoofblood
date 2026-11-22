# Rondo of Blood decomp: writers' procedure

Every writer reads this before its first routine, with
[PLAN.md](PLAN.md) section 1 (the standard) and section 7 (the plan),
[DECOMP.md](DECOMP.md) and [HANDBOOK.md](HANDBOOK.md). A new cause found in
the work is added here as a dated section, and that section then binds
every writer. Where this file and a brief disagree, this file wins; where
it and PLAN.md disagree, stop and tell the master.

Status 2026-11-21: every section holds. The unit call of section 6 is
on main (`call`, see `tools/rondo/pcebox/REPORT_FORMAT.md`); the draft
cache will shorten step 3 of section 5 when it lands.

## 1. How you work

- In your own worktree only. Never edit the main checkout's tracked files.
- Only the files your brief names. Two writers never touch the same file.
  If your routine needs a change in a file that is not yours (a header, a
  shared helper, a tool), send the master the exact need.
- You do not run gates, the runner, or a whole-game run. You build, you
  trace the coverage script you are writing, and you land. The runner
  certifies each landing and the master sends you its failures. A failure
  on something you landed comes before new work.
- Never touch the records: `config/rondo/*.csv`, `docs/rondo/ledger.csv`,
  `syscard/ledger.csv`, `docs/rondo/progress.svg`, the README's progress
  block. The runner writes them.
- Every job that uses a core for more than a few seconds, or whose memory
  can grow, goes through the machine's slot queue with a stated slot and
  memory claim. Your own output folder and your own checker binary, always.
- Watch everything you start. Nothing is left running when you report.
- Do not stop between items. An idle writer asks the master for the next.
- A question that would change the standard, the checker or the design is
  asked of the maintainer by you, in your own session. It is not relayed.
- After each landing send the master one line: commit, routines, bytes.
  Claim nothing you did not run.

## 2. No trial and error

- Read the assembly, trace the data and the callers, and write down what
  the routine does before writing its C.
- When a check fails, get the report's first difference, find the exact
  access or value that differs, and say why from the assembly. Then change
  the C once.
- A second attempt at the same problem starts with why the first was
  wrong, from evidence.
- If analysis cannot settle it, stop and tell the master what is known and
  what is not.

## 3. The C you write

It is proper C (PLAN section 1): typed arguments and returns, locals,
loops, structs, named RAM. No register file, flag helpers or gotos in a
routine's body. Registers and flags appear only in the routine's `abi_`
wrapper and at the places the checker needs them: its exits, its waits and
its calls that can be interrupted.

It also makes every access where the assembly makes it:

- Every work RAM read, write, push and pull in the assembly's order and
  number. Hardware accesses the same, as before.
- One read per compare. A `CMP` or `BIT` reads its operand once; the C
  does not read the same byte again for the branch.
- No value kept in a C variable across a point where the original reads
  memory again. If the assembly reads a byte three times, so does the C.
- Two memory reads in one C expression are unsequenced: give each its own
  statement.
- A read through a zero page pointer reads the pointer each time the
  assembly does (HANDBOOK, Code).
- An exit shim does not read RAM the assembly did not read at that place.
- Every JSR and BSR goes through the return-frame helper, with the return
  address taken from the disc's bytes, never from counting. A tail `JMP`
  gets no frame. A routine that drops its return address says so in its
  header; its caller still frames the call.
- The pushes and pulls the assembly makes (PHA, PHX, PHY, PHP and their
  pulls) are made on the emulated stack.
- A register's value at a store decides what is written; check the width
  and the source at each one.
- Braces on every body under `if`, `else`, `for` and `while`.

### The typed layer (on main from 19be9c3)

RAM is reached through the typed layer, never by a bare address or a
`NAME + index` sum. A routine includes `access.h` and names a field of a
record (`src/rondo/include/ramtypes.h`, written from
`config/rondo/records.csv`; the card's are under `syscard/`):

- **A byte:** `RD8(OBJ->vx_hi[slot])`, `WR8(ZP->temp10, 0)`, `RDS8(...)`
  for a signed read. One macro, one access of the field's original
  address. A wide field or a whole column handed to a byte macro does not
  compile.
- **A read then a write of one byte** (INC, DEC, a shift in memory):
  `INC8(f)`, `DEC8(f)`, `RMW8(f, old, expr)`, `LSR8`, `ASL8`, `ROR8`,
  `ROL8`. Each reads once, then writes once.
- **A field of two or three bytes:** the macro names the order the
  assembly accesses the bytes in, not the order they lie in:
  `RD16_LH(ZP->multiplier)` reads the low byte, then the high; `RD16_HL`
  the other way; `WR16_LH`, `WR16_HL`, `RD24_LMH`, `RD24_HML`, and the
  writes likewise. There is none without an order. Two separate fields
  that are a pair for this routine: `RD16_LH2(lo_field, hi_field)` and its
  siblings.
- **Through a zero-page pointer:** `RD8_PTR(ZP->temp00)`, `RD8_PTR_Y(...)`,
  `WR8_PTR_Y(...)`, which read the pointer's bytes where the assembly does.
- **A body shared by two sets of columns** (X and Y): keep each column's
  base, `COL(OBJ->vx_hi)`, in a small struct, and use `RD8_IDX(base, slot)`
  and `WR8_IDX(base, slot, v)`. `ADDR_OF(field)` is the address as a value,
  for a block move or a call that takes one; it is not an access.
- **Hardware** is a record too: `WR8(VDC->data_lo, v)`.

A record is a struct laid at its original address. It is never
instantiated or dereferenced; the macros use only a field's address, and
each member is checked at compile time against its symbol.

**When a field or a record does not exist.** The lists
(`config/rondo/symbols.csv`, `config/rondo/records.csv`, and the card's)
have one owner. Do not add a bare address and do not edit either list.
Send the owner one message with all the rows a routine needs. Each row
gives: the address, with the bank under which the routine reads it; the
name proposed and what the byte holds; the record and offset (or zero
page, or a new record with its base, count and layout); the type; for a
wide field the order the assembly accesses its bytes in; and the routine
and instruction that show it. Rows land in batches. Until they land the
routine keeps its old address form and says so in its header.

**Three mistakes the macros cannot prevent:**

1. The order. `RD16_LH` against `RD16_HL` is your reading of the
   assembly; both compile.
2. A value kept across a point where the original reads again. `v =
   RD8(f)`, then using `v` where the assembly loads `f` a second time, is
   a missed read.
3. An index or field argument with an effect. A macro may evaluate its
   field more than once, and the order of two reads in one expression is
   unspecified. An index is a plain variable.

**Checking a lift without an emulator:** `tools/rondo/accesscmp.py`
compiles the old and the new form of a routine against logging stubs of
the machine and compares every access, in order, with its value, and the
results. It runs in under a second and needs no slot. A different hash is
a change in order or number: find it, do not accept it. It is a check on
the lift, not a replacement for the gates.

Disc tables go through the typed data layer when it lands. The bank a
table is read in is taken from a trace, never from a comment or from
where the routine sits.

## 4. Every routine's header

`@routine`, `@run`, `WHAT IT DOES:`, `HOW:`, the entry contract, and
`WAITS:` (DECOMP.md). `WAITS:` says no, or what the routine waits on and
how deep its stack is there. If no input reaches the routine, the header
says so and says how its script runs it.

## 5. Build, trace, land

The loop as the first writers ran it (2026-11-21; a few minutes a routine
once known). M is the main checkout, W your worktree. Run from W.

**Once, in a new worktree:** link the untracked inputs from M as
`tools/rondo/gate.py` lists them (`references`, `tools/toolchains/src`,
`cc65`, `pce`, and `build/rondo/disc`, `system`, `trace`). Then, once:

    cpu-run -j 4 -m 2G -- python3 tools/rondo/liftcmp.py --warm

**Per routine or small batch:**

1. Build the image with the raw intake written out, and read your
   routine's intake assembly there (the intake text is written only with
   `--raw`):

       cpu-run -j 1 -m 3G -- python3 tools/rondo/asmbuild.py --raw build/rondo/raw

   Read its draft too. The intake's `@run` bank may read `??`: take it
   from the trace's run map (`runmap.bin`, `mprcfg.csv`), never from a
   guess. Write down what the routine does. A piece that another routine
   falls into is one routine with it: lift the group together.
2. Write the C in your lot's file and its header; file the assembly in the
   same-named `.s`, the `@routine` tag at the routine's first disc byte.
   `asmfile.py` drafts it, and leaves a bare address where a byte has no
   symbol: name it (a row request) or write it as a named base plus an
   offset; never leave the bare address.
3. The image, a few seconds of work:

       cpu-run -j 1 -m 3G -- python3 tools/rondo/asmbuild.py --raw build/rondo/raw
       cpu-run -j 1 -m 2G -- python3 tools/rondo/image.py --user build/rondo/asm/image.bin

   Required: 0 differing bytes, and in `build/rondo/asm/coverage.csv` (it
   holds totals) `source_filed` up by the routines you filed and
   `source_filed_copy` up by their copies.
4. Fetch the drafts under the routines' new names, then compare:

       python3 tools/rondo/hybrid.py
       cpu-run -j 1 -m 3G -- python3 tools/rondo/cdraft.py --out build/rondo/drafter/gen --only rSSSS_OOO,...
       python3 tools/rondo/liftcmp.py --drafts build/rondo/drafter/gen NAME ...
       python3 tools/rondo/liftcmp.py --drafts build/rondo/drafter/gen --file src/rondo/<area>/<file>.c

   `hybrid.py` and `liftcmp` need no slot (seconds). The `--only` names are
   the intake's own labels. A draft made before the routine was filed does
   not know its name: that is the "has no draft" message, and why the
   fetch comes after filing. Required: every case matches and every line
   is reached, or the missed arm is named in the header and given a
   curated case.
5. Optional, and required when the draft is not judged identical or the
   harness cannot model the routine (banks, interrupts): gate A5 on the
   one shortest script that runs it (the intake header's "run by" line),
   in your own folders:

       RONDO_BOX=W/build/rondo/box/pcebox cpu-run -j 1 -m 3G -- ./setup.sh pcebox
       RONDO_BOX=W/build/rondo/box/pcebox RONDO_ORACLE_OUT=W/build/rondo/oracle \
           cpu-run -j 1 -m 2G -- python3 tools/rondo/oracle.py a5 SCRIPT

   Read your routines' rows in `a5.csv` there; the report's lines about
   other routines are the whole hybrid's, not yours.
6. Commit with explicit paths, then land. `land.sh` takes its own slot and
   waits in the queue itself, printing a line while it waits: do not wrap
   it in `cpu-run`. A slot wait is not a hang.

       timeout 1800 tools/rondo/land.sh W

7. One line to the master: commit, routines, distinct bytes, bytes over
   all copies.

**Calls.** A call to another routine is a call to its typed C function
between the frame helpers of `machine.h` (`jsr_push(ret)`, the call,
`jsr_pop()`), with the return address taken from the disc. Never set a
register and call an `abi_` function from a body. A callee with a typed
function on main that `routines.h` does not declare: declare it `extern`
in your lot's header, saying where it is defined. A callee with no typed
function: the caller waits, and you tell the master which callee.

**Names.** A field named by its address (`OBJ->f27a8`) is a placeholder.
When you know what it holds, send the rename with your next row request
and use the name once it lands. A routine is not symbolic while it names
a field by address.

**Diagnosing a failure the runner sends you:** gate A5 on the one shortest
script that reaches the routine, in your own folders, with the placement
diagnostics of `tools/rondo/pcebox/REPORT_FORMAT.md`. Find the exact
access before changing the C.

## 6. Coverage for code no script reaches

In this order:

1. A script with real input.
2. A poke of work RAM, at a point where the game then runs the code
   itself.
3. A unit call at the routine's own entry, with the registers and RAM its
   code expects. Never a call into the middle of a routine.
4. Exempt by proof, for the bytes only, when none of the above can run
   them sanely. Send the master the byte range and the proof first; the
   maintainer decides each kind once.

Dead code is not exempt for being dead: it gets certified C and a unit
call or a poke.

Before landing a coverage script:

- Run it against the original alone and check the original still behaves:
  no execution in bytes filed as data, no runaway loop. A script that
  crashes the game is not coverage.
- Trace it, rebuild the catalog with its trace, and check that every
  catalog entry inside code that already has C has its own tag and C. A
  new script can add such entries and turn other scripts red with no C
  change. File them in the same landing.

*(the unit call's script syntax is filled in when the checker lands.)*

## 7. Commits

- What changed and why, and the coverage you traced. Nothing else: no
  authorship or credit line of any kind.
- Explicit paths. Never `git commit -a`.
- The hooks refuse certain words and dash characters in messages and
  staged text: read `tools/common/provcheck.py` before your first commit.
- You do not push.
