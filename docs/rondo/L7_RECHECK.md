# Lot L7 and L6 routines to recheck (writer L7b)

Routines filed by the L7b writer whose check is not a clean liftcmp through mode against a draft of the routine's own traced code. Every routine listed was compared with liftcmp and the image is byte exact for all of them; none was run through gate A5 by the writer except RunObjects (below).

## RunObjects (object_loop.c): A5 on stage_10

The loop of sector 0259 and its sixteen-plus copies. The first fix (enter_handler leaves its registers only when the dispatch has nothing to jump to) did not remove the zero-address calls; the cause was that the loop dispatched through sector 0259's table in every overlay with the same loop bytes. The 17 sectors whose dispatch code differs (07D9 0889 0A99 0B49 0CA9 0D59 0E09 0EB9 0F69 1019 10C9 1179 12D9 13F9 1619 16C9 18D9) now have their own tagged copy (RunObjects07D9 and the rest), and the loop calls the dispatch as the assembly does (JSR $A313 through the overlay's own DispatchByKind wrapper, or the original code at $A313 where that sector has none). liftcmp RunObjects: 68 of 68 comparable in both callee modes. A5 on stage_10 (pcebox built from this tree, `oracle.py a5 stage_10`): before the fix RunObjects diverged at call 1 ("C calls AEFC; the original made no such call": the typed dispatch plus a direct jump to the handler, where the original calls $A313); after it, calls 1 to 13253 are identical (RunObjects 13253 identical of 19972 calls, DispatchByKind0259 identical on 169293 calls). The first divergence is now call 13254, "C calls 0000; the original made no such call", in the frame where the stage is set up again after Richter's death (SetUpStageObjectsW call 1, StepScrollAndRunObjects calls 13254 to 13257, RunStageFrame calls 765 to 768 follow it). It is still open. The loop's dispatch reads the table in memory as the original does (accesses up to that call are equal), so the 0000 is a table entry or a handler address the original does not jump through at that point; the calling frame has to be traced from the original's side (the gdb trace of L3) to say which slot and kind. The copies RunObjectsNNNN cannot be compared with liftcmp (the calls to $A313 and $A186 in those sectors reach no C there); they stand on RunObjects' check and on A5 where a script runs that overlay. Sectors with the same dispatch bytes as 0259 (0519 05C9 0729 0939 09E9 0BF9) use RunObjects itself.

## Arms no script ran, written from the decoded bytes

The traced arms match liftcmp; the arm named is not proven.

- TilePieceObject072A, TilePieceInRange072A: the arm at A87A (high Y byte $FF) and the tail into the code at A8A8 (a tail jump, not lifted).
- ShatterOnHits (0CAD:0BC): stock 4 or more (`ldy #4`), the arm at C11F and the code at C135 (tail jumps).
- StateSevenOnStatusBit7 (0E0B:0AB), StateFiveOnStatusBit7 (16CB:1F4): the part after the status test.
- FollowBlockWhileState5 (051B:76E): the arm for a previous object not in state 5.
- LoadThenStepListBreakAtFive (051B:02F): the break arm after the compare (sound $82, event, spawn).
- CheckPlayerThenStepListIndex08 (088B:044): the state 6 arm.
- FollowSlot10WithHitBox (088B:7AF): the ClearObject jump.
- LoadThenSwingAndDrop (0E0C:045), LoadThenScatterAlongList (16CB:6EC), LoadThenBreakIntoThrownPiece (051A:7C5): tables inside the routine decode and assemble as bytes; checked on the traced arms.
- WaitForPressThenAnimate (051F:454): checked with curated cases (tools/rondo/curated/wait_for_press_then_animate.txt) for the position and press tests.

## Checked with wrapper callees only (through mode differs where a callee is a stub or the sound driver starts a voice)

- StopOnLanding users: StepListB3B8ThenLand, LandThenFaceAndRun, LandThenNextState (the terrain lookup at 800F is a stub in through mode), WalkTurnOrJumpOnLanding (MoveAndLand).
- Sound callers (SoundEnd): FallThenSoundAtPeak (7 through-mode cases differ in the voice start), RiseThenPlaceWithSound, SoundThenAimRandomLength, SpawnOnTimerInTable, SetRasterSpawnThenStepList, CountThenSoleScriptAndSound, StepListB1BAThenSound, LoadThenStepListBreakAtFive, RiseThenCueMusic (CdPlayAgain), TurnTowardPlayerAndMove.
- StepListB819 (051B:7C4): runs on into the code at $B7CF; through mode runs that code as a draft.
- Jump tables (JumpByStateTable*, JumpByPhaseTable*, about 60): wrapper callees 400 of 400; through mode differs only where the state is past the table's end and the jump lands in data (harness overlay lookup).

## Reasoned from the cases, not from the assembly

- LoadThenStartFiveFromTable (088A:699): V clear at the exit after the loop of StartObjectAtPlace calls (all 400 cases show it; the callee's own source for V was not traced).
