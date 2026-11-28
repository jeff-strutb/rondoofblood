# Rondo of Blood decomp: writers' procedure

Read this before your first routine, with [PLAN.md](PLAN.md) (the
standard), [DECOMP.md](DECOMP.md) and [HANDBOOK.md](HANDBOOK.md). Where
this file and PLAN.md disagree, PLAN.md wins: stop and tell the
maintainer.

## 1. How you work

- In your own worktree and branch. Only the files your task names; two
  writers never touch the same file.
- Watch everything you start. Nothing is left running when you report.
- A question that would change the standard or the design is asked of the
  maintainer.
- Claim nothing you did not do.

## 2. No trial and error

You know the assembly and the C are right as you write them, because you
write them to the standard (PLAN section 1). Nothing runs afterwards to
tell you.

- Read the assembly, the data it reads and its callers, and write down
  what the routine does before writing its C.
- For every instruction, know where its effect is in the C: the access it
  makes, in its order, the register or flag it leaves, the call it makes.
- If the image build shows a differing byte, find the exact byte and why
  from the listing (a wrong `@routine`, `@run` or operand size), then
  change it once.
- If analysis cannot settle something, stop and tell the maintainer what
  is known and what is not.

## 3. The C you write

It is proper C by the Aladdin standard (PLAN section 2, ruled 2026-11-26):
readable, documented C that may keep the register file, flag helpers and
the frames exactness needs, and runs no original code. Typed arguments,
locals, loops, structs and named RAM are preferred where exactness allows,
as Aladdin's strategy advises for the native step.

It also makes every access where the assembly makes it:

- Every work RAM read, write, push and pull in the assembly's order and
  number. Hardware accesses the same.
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
and instruction that show it. Until they land the routine is not finished.

**Three mistakes the macros cannot prevent:**

1. The order. `RD16_LH` against `RD16_HL` is your reading of the
   assembly; both compile.
2. A value kept across a point where the original reads again. `v =
   RD8(f)`, then using `v` where the assembly loads `f` a second time, is
   a missed read.
3. An index or field argument with an effect. A macro may evaluate its
   field more than once, and the order of two reads in one expression is
   unspecified. An index is a plain variable.

Disc tables go through the typed data layer. The bank a table is read in
is settled from the code that maps it before the read (the TAM and the
load that put it there), never from a comment or from where the routine
sits.

## 4. Every routine's header

`@routine`, `@run`, `WHAT IT DOES:`, `HOW:`, the entry contract, and
`WAITS:` (DECOMP.md). `WAITS:` says no, or what the routine waits on and
how deep its stack is there. If nothing is known to reach the routine,
the header says so; it is decompiled all the same.

## 5. Writing a routine

M is the main checkout, W your worktree. Run from W.

**Once, in a new worktree:** link the untracked inputs from M:
`references`, `tools/toolchains/cc65` and `build/rondo/disc`.

1. **Check for a finished copy first** (PLAN section 3). Search the disc
   for the routine's bytes and for its instructions with absolute operands
   moved by the load address. A finished match is reused: no new assembly,
   no new C.
2. **Read it.** List the bytes from the disc at the run address:

       python3 tools/rondo/huc.py build/rondo/disc/track02.bin --org 0xADDR --offset 0xOFFSET --len 0xN
       python3 tools/rondo/reach.py --sector 0xS --count N --org 0xADDR --seed 0xENTRY

   Write down what the routine does, its entry contract and its exits.
3. **File the assembly** in the area's `.s`, the `@routine` tag at the
   routine's first disc byte and its `@run`. `asmfile.py` drafts it from
   the disc with names; a byte with no symbol is named (a row request) or
   written as a named base plus an offset, never left bare.
4. **Write the C** in the same-named `.c`, with its header, to section 3.
5. **Build the image:**

       python3 tools/rondo/asmbuild.py
       python3 tools/rondo/image.py --user build/rondo/asm/image.bin

   0 differing bytes on both tracks. The card's, for card routines:

       python3 syscard/tools/image.py --image "references/[BIOS] Super CD-ROM System (Japan) (En) (v3.0).pce" \
           --cc65 tools/toolchains/cc65 --out build/syscard

6. **Compile the C:**

       python3 tools/rondo/ccheck.py

   Every file compiles with 0 failures. It compiles and runs nothing:
   C that does not compile cannot do what its assembly does.

7. **Merge.** When the routine meets the standard, commit with explicit
   paths and merge to main. Otherwise it waits on your branch.

**Calls.** A call to another routine is a call to its typed C function
between the frame helpers of `machine.h` (`jsr_push(ret)`, the call,
`jsr_pop()`), with the return address taken from the disc. Never set a
register and call an `abi_` function from a body. A callee with a typed
function on main that `routines.h` does not declare: declare it `extern`
in your lot's header, saying where it is defined. A callee with no typed
function: the caller is not finished until the callee has C.

**Names.** A field named by its address (`OBJ->f27a8`) is a placeholder.
When you know what it holds, send the rename with your next row request
and use the name once it lands. A routine is not symbolic while it names
a field by address.

## 6. Commits

- What changed and why. Nothing else: no authorship or credit line of any
  kind.
- Explicit paths. Never `git commit -a`.
