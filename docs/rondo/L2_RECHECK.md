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
