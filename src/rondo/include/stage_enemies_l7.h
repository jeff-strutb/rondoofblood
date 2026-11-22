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
void StopVerticalSpeed(uint8_t slot);
Jump JumpByStageStep(void);

#endif
