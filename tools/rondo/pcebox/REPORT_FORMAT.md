# Oracle report format (A5 and lockstep A7)

For the readers of `oracle.py`'s output and of `pcebox`'s reports. Written with the checker; if it changes, this file does.
**These gates changed what A5 and A7 prove** (access order, the emulated stack, interrupts inside calls, waits, frame
ends inside calls, calls into original code): old records of either gate no longer count.

## Running

`oracle.py a5 [SCRIPT...]` and `oracle.py a7 [SCRIPT...]` refuse to run unless `CPU_RUN_JOBS` is set (run them under
`cpu-run`). Pools come only from it: A5 runs `CPU_RUN_JOBS` scripts at once (one slot, about 0.9 GB each); A7 runs
`CPU_RUN_JOBS / 2` scripts at once, each a record process and a hybrid process joined by a FIFO. Output goes to
`$RONDO_ORACLE_OUT` (default `build/rondo/oracle`). `RONDO_BOX` names the pcebox binary, `RONDO_CORE` the core.
A run whose environment holds `RONDO_KEEP`, `RONDO_ONLY`, `RONDO_KEEP_WAITS`, `RONDO_PLACE`, `RONDO_SKIPS` or
`RONDO_DUMP_FRAME` is a diagnostic: it never writes the `.stamp` and its A7 lines never read identical.

## pcebox exit codes

| code | meaning |
|---|---|
| 0 | the script ran to its end |
| 1 | setup failed (a file, the core, a symbol) |
| 2 | a bad argument or script line (an unknown command; a `call` at an address that is not a routine's own entry, or whose bytes are not resident) |
| 3 | lockstep could not go on in step; the report and totals were written first (a wait with no interrupt in the record, a call into original code the record does not have, original code that does not return, the C making 200,000 accesses in a row the original did not) |
| 4 | the hybrid and the record disagree on which routine is called next; report and totals written first |
| 5 | out of memory, or an event was trimmed from the window (a checker fault) |
| 6 | a unit call ran over 2,000,000 instructions (not a valid script) |

`oracle.py` takes 3, 4 and 6 from the hybrid as a result (`STOPPED`), any other nonzero exit of either process as a failure
of that script ("record exit N", "hybrid exit N", "timed out after 1500 s"); a failed script writes no result.

## A5

`pcebox --a5 FILE` writes FILE (per script):

- Per-call lines, at most 6 defect lines and 4 verdict lines per routine per script:
  `<routine> <KIND> call <n> frame <f>[ at <script>:<line>]: <text>` with KIND one of
  `MOVED` (the C's access is matched later than the original's next counted access, with how many were skipped),
  `EXTRA` (the original made no such access before a barrier: a read twice, a value re-read, an access with no counterpart),
  `MISSING` (an access of the original the C never made; before a call into original code: the JSR's frame),
  `VALUE` (a read or write of another value than the original's), `STACK` (the call ends with another S or return target),
  `STATE` (MPRs or the CPU clock set in another order), `REGS` (A7: entry or exit registers), `CALL` (a call into original
  code with other registers, or none in the record), `INT` (an interrupt's handler that does not return in the call, more than
  256 passed). Text ends `[in the call of X]` when the access was made inside the call of another routine than the one named.
  Verdict lines: `<routine> DIVERGENT call <n>: <text>` (registers, flags, MPR, CPU clock or RAM differ at the end, or
  hardware not in the original's place, or the C ran away) and `<routine> UNVERIFIED call <n>: <text>`.
- `TAG <routine> <calls> <identical> <divergent> <unverified> <order>` then nine defect counts in the order
  MOVED EXTRA MISSING VALUE STACK STATE REGS CALL INT (every defect, so a cascade counts many), then ten cause counts:
  `left` (left by a longer route than RTS or RTI: unverified), `log` (calls whose log filled: judged as a prefix),
  `depth` (nested deeper than 512: unverified), `computed` (unused, 0), `arrive_branch` (a branch back into the routine's own
  first byte while its bytes are resident, information), `arrive_nonres` (always 0: an arrival at a tagged address while no
  row there has its bytes resident ran another overlay's code and is charged to no routine; the report's `OTHER CODE` line
  counts those, by call and by branch), `runaway`
  (the C ran past its access or write budget: counted in `divergent`), `prefix` (open at the script's end or at a full log:
  judged as far as the record goes, counted in `identical` or `order`), `noreturn` (A5: a call the C made with its return
  frame (`call_orig`) that the original's record shows did not come back inside the call: no RET at the callee's stack
  level to the return address the C stated. The C is judged up to that call and its exit is not compared, since the
  original left another way; counted in `identical` or `order`, never hidden: a callee that is not the one the original
  called is still a CALL defect, "made no such call", and a return to another address is not a return), `dispatch` (a call
  that ended by an RTS that jumps to an address the routine pushed itself: its S at the end is below its entry S, so
  the address is not its caller's. The original's call goes on in the routines that address begins (the record opens them
  as calls of their own, nested in this one's span): the C is judged up to its RTS, its exit and the rest of the span are
  not compared; in lockstep the call is finished there and the arrivals of the dispatched routines are matched one by one).
  `order` counts calls with an access defect and no other fault. `calls = identical + divergent + unverified + order`.

`oracle.py a5` writes, in `$RONDO_ORACLE_OUT`:

- `a5.csv` and `card_a5.csv`: `routine,bytes,bytes_run,calls,identical,divergent,unverified,order_flags,verdict`. Verdict:
  `UNCOVERED` (no call), `DIVERGENT` (any divergent or order call), `UNVERIFIED` (any unverified call, or none identical),
  else `IDENTICAL`. A routine with one call not judged is not IDENTICAL. (Columns are those `gate.py` reads; unchanged.)
- `a5_causes.csv`: `routine,MOVED,EXTRA,MISSING,VALUE,STACK,STATE,REGS,CALL,INT,left,log,depth,computed,arrive_branch,arrive_nonres,runaway,prefix,noreturn,dispatch`,
  summed over the scripts (and `a5_causes_card_a5.csv` for the card).
- `a5_prefix_only.csv`: `routine,calls,ended_noreturn`: the routines every call of which ended at a callee the original
  did not return from, or dispatched by RTS, so that they were judged on a prefix only (the worklist shows them).
- stdout: `<script>: done` or `<script>: exit N: ...`, then `routine verdicts`, `bytes by verdict`, `calls`, a line
  `calls not judged or set aside, by cause`, per-routine example lines, the UNVERIFIED and UNCOVERED name lists.

## Lockstep A7

`pcebox --record FILE` writes a stream (FILE should be a FIFO; a long script is gigabytes) and prints
`record: <events> events, <frames> frames`. `pcebox --hybrid FILE` reads it, writes `FILE.hybrid.txt`, and prints:

- `SWAPPED <routine> <n>` (calls run as C, by the routine entered), `UNDER <routine> <interrupts> <originals> <waits>`
  (interrupts dispatched inside its C, calls into original code, waits), `DEFECT <routine> <KIND> <n>` (**under the innermost
  tagged routine whose C made the access**, the routine to open), `RAN <routine> <n>` (its C ran inside another's swapped call),
  `STOPPED <reason>` if it stopped.
- the summary line, always last, fields in this order:
  `hybrid: <N> swapped, frames compared <C>, identical <I>, first differing <X>, access defects <D>, interrupts inside <K>,
  original calls <O>, waits <W>, frames expected <E>, parked <P>, kept <Q>, unreached <U>, unopened arrivals <A>, diagnostic <Z>`.
  `A` is the arrivals charged to routines whose bytes were resident (branch, computed); arrivals of other code at a tagged
  address are not in it (the `OTHER CODE` line).
  `X` is the first frame whose hash differs (>= 1), else a code: **-1 identical**, -2 access defects with every frame
  identical, -3 stopped in step, -4 diagnostic run, -5 not a verdict (nothing swapped, the record's frame count unknown or
  not equal to `C`, `parked` more than the calls the record left open, or calls of the record never reached).
  A script's A7 is IDENTICAL only on `-1`: frames compared = frames expected, all identical, no access defect, nothing swapped
  by name excluded, nothing left over, and not a diagnostic. `oracle.py` also needs the line to exist (otherwise "no result").
- `FILE.hybrid.txt`: the defect lines as in A5 (at most 6 a routine), `SKIP`/`PLACE` lines if asked, `stopped at frame F: <reason>`
  with the calls swapped last and the record's next calls, then `calls swapped N; access defects D; parked P, kept Q, unreached U`
  and `frames compared C of E, identical I, first differing X`.

`oracle.py a7` writes `a7.csv` (`routine,swapped,ran_inside`; columns as `gate.py` reads), `a7_defects.csv`
(`routine,kind,defects` per innermost routine), and prints `<script>: <summary line>` per script. A routine's A7 passes when
no script has a `DEFECT` for it, every script that reaches it is IDENTICAL (`-1`) and it was swapped in at least one.

## Unit calls and pokes in scripts

`call NAME|ADDR [A=hh] [X=hh] [Y=hh] [P=hh] [M=aaaa:hh,...]` runs a routine from its own first byte (any other address is
refused, exit 2), then runs frames until it returns (at most 61, else exit 2) and one frame more, so each call is a piece
with frames compared after it. `poke ADDR VALUE` writes a logical address; `poke BANK:OFFSET VALUE` a byte of a physical
bank (F7 is the backup memory, offsets 0 to 7FF; RAM banks 68-87 and F8-FB also). All between frames, in every run alike.
Known limits: a unit call of a routine that never returns (KernelStart, an idle spin) is not possible (the guards stop it);
it is judged only as the prefix of the call a script makes through the game.

## Not in the format

Frame hashes cover RAM, the stack above S, and the picture; the stack at and below S is not state. Timing is not judged
except that interrupts are dispatched where the original took them and frames end where the original's did.

## What C may ask of the machine layer besides accesses

`resident_at(run_address)` (machine.h; check.c) returns the disc location, as `DISC(sector, offset)` (sector * 2048 + offset),
of the tagged routine whose bytes are resident at that run address under the current mapping, 0 if none (and 0 for a System
Card routine). It exists so that C can name an overlay's routine: where one address holds one of several variants (024E:687 or
05BE:687 at 8E87, depending on the stage group), the C asks which is loaded and makes an ordinary typed call to that variant.
It is NOT an access of the original (the original reads nothing to know what is loaded): it is never placed, never counted, never
a defect. It uses the residency test of the routine table (`tag_pick`): in A5 against the call's rewound memory and the C's own
mapping (after its TAMs), in lockstep and the record against the live core. A port sets its answer when an overlay loads.
`RONDO_PROBE_RESIDENT=ADDR` (A5, a diagnostic): at each call of a routine at ADDR, `resident_at(ADDR)` must name the routine the
record opened; the A5 report ends with `PROBE resident_at: <calls> calls, <n> differ, variants seen <a> and <b>`.

`leave_original(addr)` (ops6280.h): the drafted C set S outright (TXS), so the call's frame is gone; in lockstep the original
runs on from addr, in A5 the C returns and the call is judged as left (unverified).

Diagnostics of the lockstep (they make the run a diagnostic: no verdict): `RONDO_FRAME_TRACE=N` prints, from frame N, each
side's registers, clock and open (record) or parked (hybrid) calls at every frame end; `RONDO_DIAG_RESUME=1` prints each time
the core reaches a parked C's resume address (its S and the one expected) and each park for a frame end; `RONDO_DIAG_CTX=1`
prints each time a C is switched in, abandoned or finished.
`RONDO_DEBUG_LEFT=1` prints each time the record's frame stack (8192 frames) overflows: the top frame, then the open frames in
runs with their entry S; `RONDO_DEBUG_LEFT=all` or a routine name prints each call the stack closes as left. `RONDO_DIAG_WIN=1` prints
the hybrid's event window at every frame end (`WIN`: base, length, scan, the lowest parked call) and a backtrace at each window growth
past 4M events (`WINGROW`).

Close kinds in the record: ok, drop, left, and end (CL_END: the script ended with the call open, the record's closing flush; the hybrid
parks the C and does not hand over to the original).

The window is bounded: a call whose returning RET (or whose end, for a C that has finished) is not within 8M events (HORIZON) is
not looked for further. The routine ends there and the original runs on; the hybrid report says `horizon hand-overs N`. Hybrid only;
A5 reads its whole log.

More diagnostics (environment variables; none changes a verdict, except RONDO_NO_RAMCALL, which switches a rule off to compare):
`RONDO_DIAG_CTX=1` also prints, in the hybrid, `IRQCORE` (an interrupt the core takes itself: its PC, S and clock against the record's next event),
`ENTERS` (a tail the drafted code decided is no call), `NOCALL` and `AFTER` (the record's events when a C calls what the original did not) and
`INTPASS` (an interrupt dispatched to a C whose S differs from the record's: the INT event, the C's S, its last placed access, the next 12 events).
`RONDO_DIAG_UNTAGGED=1` (A5) prints each CALL to an untagged target that stops placement: routine, target, whether the target's 16 bytes are in the
disc's data track, and `passed` (the target is a no-disc-origin call target from the traces, the call returns: its accesses are the routine's own) or `kept`.
`RONDO_NO_RAMCALL=1` turns that pass off. `RONDO_DIAG_AHEAD=1` prints each time the core is already past the clock the record wants it at (with the
place that asked) and each pace to a zero clock. `RONDO_DIAG_EXIT=NAME` (A5) prints, for each judged call of NAME, the C's S at its end, the S and PC
the exit compare uses, and the original's. `RONDO_DUMP_SPAN=NAME,N` (A5 and the hybrid) prints every event of the Nth call of NAME: `SPAN +index type kind
addr pc s val clock` (types: 0 R, 1 W, 2 ROMR, 3 HWR, 4 HWW, 5 RTPULL, 6 STATE, 7 OPEN, 8 CLOSE, 9 CALL, 10 RET, 11 INT, 12 IRET, 13 FRAME); in A5 the
number is the order the calls are judged in (close order for nested calls), in the hybrid the order they are swapped in.
