# L2 routines to recheck

Routines filed in lot L2 whose check is not a clean liftcmp through mode. Each is rechecked when the callee named is re-lifted.

| routine | check made | waits on |
|---|---|---|
| CountDownThenStand (player_leaves.c) | --callees wrappers, 400 of 400 | RichterStand |
| RichterReadAtOffset (player_leaves.c) | --callees wrappers, 400 of 400 | RichterReadOnScreen |
| ProbeBoxes* and ProbeBoxesFlagged* (player_probe_boxes.c) | curated pins of the handler table (tools/rondo/curated/probe_boxes.txt); the pins assign handlers cyclically, they are not a state a trace shows, so they stand only as a stand-in until the real table (disc bytes, the same in every overlay) is pinned with attributes a trace reaches | untraced overlay handlers at 9A64 to 9ADE and 9C0F to 9C31 (no C) |
| ProbeBoxesFlagged0F60 | first piece only (no whole routine in the intake) | |

## The earlier Richter C that still differs from its draft (liftcmp through mode, 2026-11-25, after call frames and the animation core)

Re-lifted and matching: every routine of richter_objects.c; the start chain (RichterStart on 12 curated cases that give the disc's handler table and the four values of zField8A, RichterStartNew, Health, Clear, Stand, ClearWork, ClearKeepPlace, AnimationSound); RichterAnimationTick, RichterSetAnimation, RichterAnimationRunning, RichterAnimationWait, the object variants (RichterObjectAnimationList, Tick, SetAnimation, Running, Wait), RichterReadAbove. The animation exits are now built in the body (AnimationStep carries A, N and Z, C and Y); the old shims read memory the assembly does not read and add accesses.

Re-lifted since (liftcmp through mode, with a stub that keeps X; see below): every routine of richter_objects.c, player_terrain.c (22 of 22 with a draft) and the pad and item routines of player_input.c (curated item and start cases: tools/rondo/curated/richter_item.txt, richter_start.txt, which give the disc's handler tables and the values the tables are indexed by).

**The stub's X.** liftcmp's stub for code with no C (the overlay's own tile read at 006 and its handlers) returns an arbitrary X. The overlay's read keeps X (its callers loop on X), so the terrain routines are compared with a local stub that keeps X (an environment switch in tools/rondo/liftcmp_rt.c, not committed: it is the runner's tool). With the stock stub RichterReadColumns, LowColumnsBlocked, Floor, HasNoFooting, Wall and LedgeCheck differ only in cases where that stub is called. The runner should offer the switch or confirm the assumption from a trace.

Still differing in richter_state.c (31), each needing its body rewritten to the assembly's own order, its call frames, the entry X and Y of its sound calls (seven call sites pass 0 and 0 and say so) and its exit built in the body: RichterVecFrame RichterVecStart RichterVecC3A7 RichterVecC076 RichterApplyMoveAndPush RichterApplyMoveAndPushGoTo RichterFrame RichterHitWhilePushed RichterSpikeBelow RichterStart RichterDie RichterStateFrame RichterOutOfHealth RichterStandStep1 RichterStandStep2 RichterStandStep0 RichterStartWalk RichterAttack RichterUpWhipLeft RichterNudgeBJump RichterUpWhipRight RichterToHurtAir RichterItemCrashFive RichterToAir RichterStateWalk RichterClimbStart RichterCrouchAttack RichterCrouchStep0 RichterCrouchStep1 RichterCrouchStep3 RichterCrouchStep4

## player_terrain.c: unchecked until the per callee stub contract lands (2026-11-25)

The 22 routines of player_terrain.c count as unchecked. Their liftcmp results were taken with a local, uncommitted switch that makes the stub for code with no C keep X (STUB_KEEP_X in tools/rondo/liftcmp_rt.c, reverted before every landing, so the switch is not in d73df27 or later). The code landed does contain what that run led to: the wall probe keeps its side in Y between reads, the floor and footing exits do not re-apply earlier shims over the state the overlay's own read left, and the read's entry state (A, X, Y, flags) is set before each call. Recheck all 22 with the stock checker once typed access gives the overlay's tile read a stub contract. Without the switch the differences are in ReadColumns, LowColumnsBlocked, Floor, HasNoFooting, Wall and LedgeCheck, only in cases where the stub is called.

## player_terrain.c rechecked (2026-11-25, after typed access landed 6b63f9f)

With tools/rondo/stubs.csv (the tile read's contracts per callee, no global keep X) and no local switch, all 22 routines of player_terrain.c that have a draft match under liftcmp through mode: ReadColumns 400, LowColumnsBlocked 400, Wall 400, Floor 377 (23 untraced), HasNoFooting 191 (209 untraced), LedgeCheck 300 (100 untraced), LadderCheck 106 (494 untraced), LandOnFloor 135 (265 untraced), the rest 400 or all comparable. The reports print an ASSUMPTION line for each contract used. The untraced counts are arms the draft steers into bytes no script ran. The contracts cover the eleven copies of the tile read at $967F; the $9991 family (ProbeTileRead9991, the box probes of player_probe_boxes.c) has none yet and stays as listed above. RichterReadColumnsHigh and RichterFloorOddScreen have no draft of their own (pieces), checked as part of ReadColumns and FloorCheck.

## State of 2026-11-25 (after 9bdc4df and the commits on it)

Checked in through mode (liftcmp, `--drafts` main's cache): RichterFrame, RichterVecFrame, RichterToAir, RichterStartWalk, RichterUpWhipLeft, RichterNudgeBJump, RichterStateFrame, RichterOutOfHealth, RichterItemCrashFive: 0 differing. RichterAttack and RichterCrouchAttack: 0 differing of 249 and 248 comparable (landed in 9bdc4df).

RichterStart and RichterVecStart: 12 of 12 curated cases (tools/rondo/curated/richter_start.txt, the table at $A116 as the disc has it) match. The random cases differ because they leave the jump table at random values and the draft jumps into code no state of the game reaches; they are not a C difference.

Through-mode only, `$8006` vector: RichterClimbStart, RichterUpWhipRight (1 of 400 cases), RichterApplyMoveAndPush, RichterStateWalk, RichterLadderCheck, RichterLedgeCheck differ only where the tile read is entered through StageTileReadVector9991 ($8006): the draft side takes it to TerrainTileRead (return 96CD), the lifted side to ProbeTileRead9991 (99DF). Validation A (L8) says the lifted side is right when the case's mapper has 05C0:191 at $9991 (the vector's JMP is the overlay's own), and the drafter's draft holds one stage's JMP. Recheck each with `--callees wrappers` and with A5 on the stage scripts before they count as done.

Still to re-lift (no call frames, bodies not in the assembly's order): RichterStandStep0, RichterStandStep1, RichterStandStep2, RichterCrouchStep0, RichterCrouchStep1, RichterCrouchStep3, RichterCrouchStep4, RichterToHurtAir, RichterVecC3A7, RichterVecC076; their 2026-11-25 sweep ran before the ClimbStart and ToAir changes, so rerun it first.

**RichterStateWalk, wrappers mode, 399 of 400: not checked.** Case 166 (seed B1DB9C777637): the 554 events before it agree on both sides; the path is the jump (RichterJumpPressed set, `jmp RichterToAir`), which ends with an RTS that pops a stack the case seeded at random, so both sides tail to $FF01. The draft arrives with X=01 and the lifted C with X=00. RichterToAir leaves X as it found it, so the X at the jump is the one the checks before it left (RichterFloorCheck, RichterJumpPressed); where it differs I have not found, and I have not explained it from the assembly. Open.
