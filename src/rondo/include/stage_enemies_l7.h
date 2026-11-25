/* Lot L7: the stage enemies of four stages and their helpers, as typed C. */
#ifndef RONDO_STAGE_ENEMIES_L7_H
#define RONDO_STAGE_ENEMIES_L7_H

#include "machine.h"
#include "enemy_objects.h"
#include "common_objects.h"

/* Whether a block touches the player, and what the routine's last instructions left. */
typedef struct {
    bool contact;
    Ended end;
} Contact;

/* object/enemies_stage06_temp.c */
Ended WrapVerticalDrift(uint8_t slot);
Ended MoveAndWrapDriftA(uint8_t slot);
Ended MoveAndWrapDriftB(uint8_t slot);
Ended RisingDriftBlock(uint8_t slot);
Ended LevelDriftBlock(uint8_t slot);
Ended StartRisingPiece(uint8_t slot);
Ended StartLevelPiece(uint8_t slot);
Ended DropPieceEvery32(uint8_t slot);
Contact ContactWithPlayer(uint8_t slot);
Ended StepPieceOnParent(uint8_t slot);
Ended FollowParentUntilTouched(uint8_t slot);
Ended FallingPieceMove(uint8_t slot);
Ended FallingPieceStep(uint8_t slot);

Ended HitboxByVerticalDirection(uint8_t slot);
Ended LiftObjectsBySlowFraction(uint8_t slot);
Ended ShiftObjectsX(uint8_t slot);
Ended OrbitingObject(uint8_t slot);
Ended AccelerateAndSway(uint8_t slot);
Ended StepThenCount(uint8_t slot);
Ended StepThenStop(uint8_t slot);
Ended FallUntilLevel2(uint8_t slot);
Ended StartPieceChain(uint8_t slot);
Ended StartPieceStreamA(uint8_t slot, uint8_t y);
Ended StartPieceStreamB(uint8_t slot, uint8_t y);
Ended StartPieceStreamC(uint8_t slot);
Ended StepFallingPieceStream(uint8_t slot);
Ended StepPieceEmitter(uint8_t slot);
Ended StartOnFirstState(uint8_t slot);
Ended FallingDebris(uint8_t slot);
Ended FlashByHeight(uint8_t slot);
Ended StartWithEvent(uint8_t slot);
Ended RaiseStateOnFlag(uint8_t slot);
void StopVerticalSpeed(uint8_t slot);
Jump JumpByStageStep(void);

/* object/enemies_stage09_temp.c */
Ended FaceByCue(uint8_t slot);
Ended WalkAndTurnAtEdges(uint8_t slot);
Ended WalkSideways(uint8_t slot);
Ended PickNextAction(uint8_t slot);
Ended WalkTowardPlayer(uint8_t slot);

Ended StartCueScript(uint8_t slot, uint8_t y);
Ended PlayCueOnFrame(uint8_t slot);
SoundEnd StepFlyingCue(uint8_t slot, uint8_t y);
Ended StartTwinSpawner(uint8_t slot);

Ended StepThenQuarterWait(uint8_t slot);
Ended StartShotRing(uint8_t slot);
Ended EnterStateWhenSlotClear(uint8_t slot);
SoundEnd BurstOnLanding(uint8_t slot, uint8_t y);
Ended StartSplitterHigh(uint8_t slot, bool carry_in);
Ended StartSplitterLow(uint8_t slot, bool carry_in);
Ended MaybeShootAhead(uint8_t slot);

Ended PickWaterLevel(void);
SoundEnd WaitForPage2(uint8_t slot, uint8_t y);
Ended RiseUntilAboveGround(uint8_t slot);
Ended WaitForSlot10State4(uint8_t slot);
Ended StartNumberedPiece(uint8_t slot);
Ended FollowPlayerHitbox(uint8_t slot);

Ended RiseAndClear(uint8_t slot);
Ended RaiseStateAfterSlot10(uint8_t slot);
Ended StartChainLink(uint8_t slot);
Ended ChainFollowsPrevious(uint8_t slot);
Ended StartKickedPiece(uint8_t slot);
Ended StepThenClear(uint8_t slot);

void ClearAnimation(uint8_t slot);
Ended EnterNextAndReset(uint8_t slot);
Ended FlipSide(uint8_t slot);
Ended StepListByPhase(uint8_t slot);
Ended SetSwaySpeed(uint8_t slot);
Ended BobAndMove(uint8_t slot);

Ended ReactAtWindowEdge(uint8_t slot);
Ended SwayStep(uint8_t slot);
Ended SwayOrAdvance(uint8_t slot);
Ended SwayThenReact(uint8_t slot);
Ended SwayThenFlip(uint8_t slot);
Ended SwayThenRestart(uint8_t slot);
Jump JumpByStateTable(uint8_t slot);

/* Whether an object is inside a window, and what the routine left. */
typedef struct {
    bool in_view;
    Ended end;
} ViewTest;
ViewTest InWindowRange(uint8_t slot);

/* A memory shift right in one read and one write: the value written, and the bit shifted out in `carry_var`. */
#define LSR8_VALUE(field, carry_var) \
    RMW8(field, old_s, ((carry_var) = (old_s & 1) != 0, old_s >> 1))

#endif
