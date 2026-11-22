/* Lot L7: the stage enemies of four stages and their helpers, as typed C. */
#ifndef RONDO_STAGE_ENEMIES_L7_H
#define RONDO_STAGE_ENEMIES_L7_H

#include "machine.h"
#include "enemy_objects.h"

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

#endif
