/* The routines of object/calls_b22.c. */
#ifndef RONDO_CALLS_B22_H
#define RONDO_CALLS_B22_H

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

void SteerAndBob12DC(uint8_t slot);
void abi_SteerAndBob12DC(void);
void SteerAndBob101E(uint8_t slot);
void abi_SteerAndBob101E(void);
void SpiralDown12DB(uint8_t slot);
void abi_SpiralDown12DB(void);
void ChooseAttack0E0B(uint8_t slot);
void abi_ChooseAttack0E0B(void);
void PlatformCarry0EC0(uint8_t slot);
void abi_PlatformCarry0EC0(void);
void BreakableDoor161F(uint8_t slot);
void abi_BreakableDoor161F(void);
void WaterByLeader0B4D(uint8_t slot);
void abi_WaterByLeader0B4D(void);
void ListThenLeapByDistance0E0B(uint8_t slot);
void abi_ListThenLeapByDistance0E0B(void);
void FlyOverWater12DD(uint8_t slot);
void abi_FlyOverWater12DD(void);
void RiseTrailingDebris072C(uint8_t slot);
void abi_RiseTrailingDebris072C(void);
void WalkTurnOrStop0E0B(uint8_t slot);
void abi_WalkTurnOrStop0E0B(void);
void AmbushFromBehind0EBD(uint8_t slot);
void abi_AmbushFromBehind0EBD(void);
void LeapUntilLand13FB(uint8_t slot);
void abi_LeapUntilLand13FB(void);
void BreakIntoSix161C(uint8_t slot);
void abi_BreakIntoSix161C(void);
void CrushingPlatform161F(uint8_t slot);
void abi_CrushingPlatform161F(void);
void OpenArena0E0B(uint8_t slot);
void abi_OpenArena0E0B(void);
void ChooseState117E(uint8_t slot);
void abi_ChooseState117E(void);

#endif
