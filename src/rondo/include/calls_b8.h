/* The routines of object/calls_b8.c. */
#ifndef RONDO_CALLS_B8_H
#define RONDO_CALLS_B8_H

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

void ListThenAim161D(uint8_t slot);
void abi_ListThenAim161D(void);
void CountThenDrop26_18DB(uint8_t slot);
void abi_CountThenDrop26_18DB(void);
void FallUntilScreen4_072C(uint8_t slot);
void abi_FallUntilScreen4_072C(void);
void ThrowThenLandRow117C(uint8_t slot);
void abi_ThrowThenLandRow117C(void);
void ListLandThen117D(uint8_t slot);
void abi_ListLandThen117D(void);
void LoadFacingWithPrevious117D(uint8_t slot);
void abi_LoadFacingWithPrevious117D(void);
void ListDashThenState117F(uint8_t slot);
void abi_ListDashThenState117F(void);
void MoveBounceOffWall12DC(uint8_t slot);
void abi_MoveBounceOffWall12DC(void);
void ListByStateThenProbe161C(uint8_t slot);
void abi_ListByStateThenProbe161C(void);
void SpawnEvery48_10CC(uint8_t slot);
void abi_SpawnEvery48_10CC(void);
void TurnAround117B(uint8_t slot);
void abi_TurnAround117B(void);
void ListKeepingPace117E(uint8_t slot);
void abi_ListKeepingPace117E(void);
void ScriptsAtCounts12DC(uint8_t slot);
void abi_ScriptsAtCounts12DC(void);
void ListWithChecksThenSpawn18DB(uint8_t slot);
void abi_ListWithChecksThenSpawn18DB(void);
void ListThenSpawn27_18DC(uint8_t slot);
void abi_ListThenSpawn27_18DC(void);
void SpawnWhenSlot24Gone18DC(uint8_t slot);
void abi_SpawnWhenSlot24Gone18DC(void);
void RunListThenState13_09EB(uint8_t slot);
void abi_RunListThenState13_09EB(void);
void ListShootAtFrame3B_0E0B(uint8_t slot);
void abi_ListShootAtFrame3B_0E0B(void);
void DripAtFrame26_0E0B(uint8_t slot);
void abi_DripAtFrame26_0E0B(void);
void ListThenWhenNear0EBD(uint8_t slot);
void abi_ListThenWhenNear0EBD(void);
void RideSourceOrState5_0EBE(uint8_t slot);
void abi_RideSourceOrState5_0EBE(void);
void ListThenHop13FB(uint8_t slot);
void abi_ListThenHop13FB(void);
void ProbeSpawnListWhenAlone13FC(uint8_t slot);
void abi_ProbeSpawnListWhenAlone13FC(void);
void ListSoundThenSpawn30_18DC(uint8_t slot);
void abi_ListSoundThenSpawn30_18DC(void);
void LoadThenListClear161E(uint8_t slot);
void abi_LoadThenListClear161E(void);
void MoveOrAimShot088B(uint8_t slot);
void abi_MoveOrAimShot088B(void);
void LoadAndStart30_117E(uint8_t slot);
void abi_LoadAndStart30_117E(void);

#endif
