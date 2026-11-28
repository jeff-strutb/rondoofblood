/* The routines of object/calls_b13.c. */
#ifndef RONDO_CALLS_B13_H
#define RONDO_CALLS_B13_H

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

void FallToLineInA0EBA(uint8_t slot);
void abi_FallToLineInA0EBA(void);
void ShudderWhenHit16CB(uint8_t slot);
void abi_ShudderWhenHit16CB(void);
void SwayDownThenState0EBF(uint8_t slot);
void abi_SwayDownThenState0EBF(void);
void OrbitPrevious117F(uint8_t slot);
void abi_OrbitPrevious117F(void);
void TurnThenSubstate12DC(uint8_t slot);
void abi_TurnThenSubstate12DC(void);
void ClearOffScreenElseState0EBC(uint8_t slot);
void abi_ClearOffScreenElseState0EBC(void);
void LoadWithEightStreamers12DC(uint8_t slot);
void abi_LoadWithEightStreamers12DC(void);
void ListThenWakeWithMusic13FB(uint8_t slot);
void abi_ListThenWakeWithMusic13FB(void);
void DissolveStepWorkO12DC(uint8_t slot);
void abi_DissolveStepWorkO12DC(void);

#endif
