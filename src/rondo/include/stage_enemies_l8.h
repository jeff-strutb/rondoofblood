/* Lot L8: the stage object routines of the overlays at sectors 0259 to 18DD, as typed C. */
#ifndef RONDO_STAGE_ENEMIES_L8_H
#define RONDO_STAGE_ENEMIES_L8_H

#include "machine.h"
#include "enemy_objects.h"
#include "common_objects.h"

/* object/enemies_l8_st11.c */
Ended LoadRecordOrCapPlayerX(uint8_t slot);
Ended StartAtX120OrApplyVelocity(uint8_t slot);
Ended SetBossFixedFields(void);
Ended CountdownThenNextState(uint8_t slot);
Ended SetTimerThenNextState(uint8_t slot, uint8_t timer);
Ended SpawnPiece21Mask1F(uint8_t slot);
Ended SpawnPiece21UnlessMasked(uint8_t slot, uint8_t mask);
Ended CountdownThenSpawnPieces(uint8_t slot);
Ended HoldScrollStepThenNextState(uint8_t slot);

/* object/enemies_l8_st12.c */
Ended StepVerticalScrollTable(uint8_t slot);

#endif
