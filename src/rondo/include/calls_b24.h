/* The routines of object/calls_b24.c. */
#ifndef RONDO_CALLS_B24_H
#define RONDO_CALLS_B24_H

#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"
#include "calls_b.h"
#include "calls_b3.h"

void SwingBack16CB(uint8_t slot);
void abi_SwingBack16CB(void);
void SwingForth16CB(uint8_t slot);
void abi_SwingForth16CB(void);
void PaceUntilHit161C(uint8_t slot);
void abi_PaceUntilHit161C(void);
void FollowHolder0EBD(uint8_t slot);
void abi_FollowHolder0EBD(void);
void LoadShardByArg0E0C(uint8_t slot);
void abi_LoadShardByArg0E0C(void);
void StrikeByFrame117E(uint8_t slot);
void abi_StrikeByFrame117E(void);
void DecideWhenNearOrHit13FB(uint8_t slot);
void abi_DecideWhenNearOrHit13FB(void);
void ChargeCountHits12DC(uint8_t slot);
void abi_ChargeCountHits12DC(void);
void ScatterBit09EC(uint8_t slot);
void abi_ScatterBit09EC(void);
void SpawnerPastStart101E(uint8_t slot);
void abi_SpawnerPastStart101E(void);

#endif
