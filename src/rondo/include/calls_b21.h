/* The routines of object/calls_b21.c. */
#ifndef RONDO_CALLS_B21_H
#define RONDO_CALLS_B21_H

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

void WalkByTable101E(uint8_t slot);
void abi_WalkByTable101E(void);
void WalkByTable12DC(uint8_t slot);
void abi_WalkByTable12DC(void);
void WalkByTable12DE(uint8_t slot);
void abi_WalkByTable12DE(void);
void WalkByTable13FB(uint8_t slot);
void abi_WalkByTable13FB(void);
void ThrowByFrame10CB(uint8_t slot);
void abi_ThrowByFrame10CB(void);
void StartSixSegments117F(uint8_t slot);
void abi_StartSixSegments117F(void);
void BoxByLeader12DD(uint8_t slot);
void abi_BoxByLeader12DD(void);
void LoadWithLimbs16CC(uint8_t slot);
void abi_LoadWithLimbs16CC(void);
void RunAndRecordPath088A(uint8_t slot);
void abi_RunAndRecordPath088A(void);
void SplitInTwo101B(uint8_t slot);
void abi_SplitInTwo101B(void);
void WakeBeforeLevelX101B(uint8_t slot);
void abi_WakeBeforeLevelX101B(void);
void FlyPatternUnlessHeld072C(uint8_t slot);
void abi_FlyPatternUnlessHeld072C(void);
void StartParts09EB(uint8_t slot);
void abi_StartParts09EB(void);
void ListSeedFreeSlot10CC(uint8_t slot);
void abi_ListSeedFreeSlot10CC(void);
void HoldThenShoot12DB(uint8_t slot);
void abi_HoldThenShoot12DB(void);
void LoadShotByArg12DD(uint8_t slot);
void abi_LoadShotByArg12DD(void);
void SpawnWhileWaiting161B(uint8_t slot);
void abi_SpawnWhileWaiting161B(void);
void ListLunge161D(uint8_t slot);
void abi_ListLunge161D(void);
void SeedWhileLeader10CB(uint8_t slot);
void abi_SeedWhileLeader10CB(void);

#endif
