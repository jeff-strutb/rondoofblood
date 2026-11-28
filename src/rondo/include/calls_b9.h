/* The routines of object/calls_b9.c. */
#ifndef RONDO_CALLS_B9_H
#define RONDO_CALLS_B9_H

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

void LoadOrAimFromPrevious117F(uint8_t slot);
void abi_LoadOrAimFromPrevious117F(void);
void RiseWithRoute161F(uint8_t slot);
void abi_RiseWithRoute161F(void);
void WalkListThenState0EBD(uint8_t slot);
void abi_WalkListThenState0EBD(void);
void ArenaLimits101C(uint8_t slot);
void abi_ArenaLimits101C(void);
void ShootAtFrameAB_101D(uint8_t slot);
void abi_ShootAtFrameAB_101D(void);
void LandThenWake101E(uint8_t slot);
void abi_LandThenWake101E(void);
void ParryWhenHit12DC(uint8_t slot);
void abi_ParryWhenHit12DC(void);
void ListThenHitTest13FC(uint8_t slot);
void abi_ListThenHitTest13FC(void);
void ListSpawnWhileCount16CC(uint8_t slot);
void abi_ListSpawnWhileCount16CC(void);
void FallThenBecome2E_0CAC(uint8_t slot);
void abi_FallThenBecome2E_0CAC(void);
void CircleFromCentre0CAD(uint8_t slot);
void abi_CircleFromCentre0CAD(void);
void SpeedRowByWorkI0EBF(uint8_t slot);
void abi_SpeedRowByWorkI0EBF(void);
void SpiralThenSound10CD(uint8_t slot);
void abi_SpiralThenSound10CD(void);
void RiseToLine117B(uint8_t slot);
void abi_RiseToLine117B(void);
void LeapByDistance117D(uint8_t slot);
void abi_LeapByDistance117D(void);
void WaitForScreen3_12DB(uint8_t slot);
void abi_WaitForScreen3_12DB(void);
void SpawnThreeAtStep12DC(uint8_t slot);
void abi_SpawnThreeAtStep12DC(void);
void FollowSlot16Piece12DD_0B3(uint8_t slot);
void abi_FollowSlot16Piece12DD_0B3(void);
void FollowSlot16Piece12DD_14D(uint8_t slot);
void abi_FollowSlot16Piece12DD_14D(void);
void DiveThenSound18DB(uint8_t slot);
void abi_DiveThenSound18DB(void);
void CountThenErupt09EB(uint8_t slot);
void abi_CountThenErupt09EB(void);
void ScatterEightThenClearPair101B(uint8_t slot);
void abi_ScatterEightThenClearPair101B(void);
void FlyOffWhenSourceGone101C(uint8_t slot);
void abi_FlyOffWhenSourceGone101C(void);
void SpeedByDistance0F6C(uint8_t slot);
void abi_SpeedByDistance0F6C(void);
void DebrisBelow0EBB(uint8_t slot);
void abi_DebrisBelow0EBB(void);

#endif
