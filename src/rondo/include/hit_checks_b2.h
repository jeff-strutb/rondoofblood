/* The routines of object/hit_checks_b2.c. */
#ifndef RONDO_HIT_CHECKS_B2_H
#define RONDO_HIT_CHECKS_B2_H

#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l8.h"

void ToState6WhenHitOrAway0EBC(uint8_t slot);
void abi_ToState6WhenHitOrAway0EBC(void);
void HitBecomesKind38_0EBD(uint8_t slot);
void abi_HitBecomesKind38_0EBD(void);
void KeepNearScreen13FC(uint8_t slot);
void abi_KeepNearScreen13FC(void);
void ToState4WhenSlot16Marked13FC(uint8_t slot);
void abi_ToState4WhenSlot16Marked13FC(void);
void HitBoxByFrameRange13FC(uint8_t slot);
void abi_HitBoxByFrameRange13FC(void);
void StartByPlayerSide161D(uint8_t slot);
void abi_StartByPlayerSide161D(void);
void HitStopToState5_161D(uint8_t slot);
void abi_HitStopToState5_161D(void);
void FaceThePlayerEvery8_161D(uint8_t slot);
void abi_FaceThePlayerEvery8_161D(void);
void HalveWhenHit161D(uint8_t slot);
void abi_HalveWhenHit161D(void);
void HitStopAndClearNext161D(uint8_t slot);
void abi_HitStopAndClearNext161D(void);
void CountStateWhenHit18DB(uint8_t slot);
void abi_CountStateWhenHit18DB(void);

#endif
