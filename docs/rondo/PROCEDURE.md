# Rondo of Blood decomp: writers' procedure

Every writer reads this before its first routine, with
[PLAN.md](PLAN.md) section 1 (the standard) and section 7 (the plan),
[DECOMP.md](DECOMP.md) and [HANDBOOK.md](HANDBOOK.md). A new cause found in
the work is added here as a dated section, and that section then binds
every writer. Where this file and a brief disagree, this file wins; where
it and PLAN.md disagree, stop and tell the master.

Status 2026-11-21: sections 1 to 4 and 7 hold now. Sections 5 and 6 name
tools that are being built (the checker's helpers, `land.sh`, the runner);
they are filled in when those land, and no writer starts before then.

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

RAM is reached through the typed layer, not bare addresses, and disc
tables through the typed data layer. The bank a table is read in is taken
from a trace, never from a comment or from where the routine sits.

## 4. Every routine's header

`@routine`, `@run`, `WHAT IT DOES:`, `HOW:`, the entry contract, and
`WAITS:` (DECOMP.md). `WAITS:` says no, or what the routine waits on and
how deep its stack is there. If no input reaches the routine, the header
says so and says how its script runs it.

## 5. Build, trace, land

*(filled in when the checker and `land.sh` land: the exact commands for
building the hybrid from your worktree, tracing a script, reading its
coverage, the access-order diagnostics for one routine, and landing.)*

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
