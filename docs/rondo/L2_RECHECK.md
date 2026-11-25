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

Still differing, in the three files (each needs its body rewritten to the assembly's own order, its call frames checked and its exit built in the body, not in a shim that reads RAM):

RichterReadPad RichterCheckTurnWindow RichterFaceByPad RichterNoteItemPress RichterCheckItemHearts RichterUseItem RichterUseThrownItem RichterLatchButtons RichterReadAbove RichterColumnOffset RichterWall RichterReadColumns RichterColumnsBlocked RichterFloorCheck RichterLowColumnsBlocked RichterFloor RichterSolidUnderColumn RichterReadUnderFeet RichterHasNoFooting RichterLadderCheck RichterJumpOrClimb RichterLedgeCheck RichterLiftByColumn RichterLandOnFloor RichterVecFrame RichterVecStart RichterVecApplyMove RichterVecC3A7 RichterVecC076 RichterVecWall RichterVecFloor RichterApplyMoveAndPush RichterApplyMoveAndPushGoTo RichterFrame RichterHitWhilePushed RichterSpikeBelow RichterStart RichterClearKeepPlace RichterStartNew RichterStartHealth RichterStartClear RichterDie RichterStateFrame RichterOutOfHealth RichterStandStep1 RichterStandStep2 RichterStandStep0 RichterStartWalk RichterAttack RichterUpWhipLeft RichterNudgeBJump RichterUpWhipRight RichterToHurtAir RichterItemCrashFive RichterToAir RichterStateWalk RichterClimbStart RichterCrouchAttack RichterCrouchStep0 RichterCrouchStep1 RichterCrouchStep3 RichterCrouchStep4 
