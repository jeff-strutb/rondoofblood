/* The routines of object/calls_b4.c. */
#ifndef RONDO_CALLS_B4_H
#define RONDO_CALLS_B4_H

#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"

void MoveYThenClearOffScreen12DE(uint8_t slot);
void abi_MoveYThenClearOffScreen12DE(void);
void ListThenState213FB_651(uint8_t slot);
void abi_ListThenState213FB_651(void);
void ListThenState213FB_6F0(uint8_t slot);
void abi_ListThenState213FB_6F0(void);
void ListThenState213FC_515(uint8_t slot);
void abi_ListThenState213FC_515(void);
void FollowSource161B(uint8_t slot);
void abi_FollowSource161B(void);
void ListThenWait18DB(uint8_t slot);
void abi_ListThenWait18DB(void);
void SpawnWithSoundAtRandom0E0C(uint8_t slot);
void abi_SpawnWithSoundAtRandom0E0C(void);
void ListThenSubstate117D(uint8_t slot);
void abi_ListThenSubstate117D(void);
void SoundE2AndClearNext117F(uint8_t slot);
void abi_SoundE2AndClearNext117F(void);
void FaceByBitThenNear12DC(uint8_t slot);
void abi_FaceByBitThenNear12DC(void);
void LoadThenStateOrDrop13FD(uint8_t slot);
void abi_LoadThenStateOrDrop13FD(void);
void CountThenScript97_16CB(uint8_t slot);
void abi_CountThenScript97_16CB(void);
void LoadSpawnScript18DB(uint8_t slot);
void abi_LoadSpawnScript18DB(void);
void CountThenSound91_18DB(uint8_t slot);
void abi_CountThenSound91_18DB(void);
void StartRow088B_543(uint8_t slot);
void abi_StartRow088B_543(void);
void StartRow088B_55B(uint8_t slot);
void abi_StartRow088B_55B(void);
void LoadSpeedByWorkD088C(uint8_t slot);
void abi_LoadSpeedByWorkD088C(void);
void CountThenDraw0B4D(uint8_t slot);
void abi_CountThenDraw0B4D(void);
void WaitForSlotSpawn0E0B(uint8_t slot);
void abi_WaitForSlotSpawn0E0B(void);
void StartRowAndClear101E(uint8_t slot);
void abi_StartRowAndClear101E(void);
void LandInFront117D(uint8_t slot);
void abi_LandInFront117D(void);
void AimAtPlayer088B(uint8_t slot);
void abi_AimAtPlayer088B(void);
void LoadOrListThenClear0E0C(uint8_t slot);
void abi_LoadOrListThenClear0E0C(void);
void FallListThenClear0EC0(uint8_t slot);
void abi_FallListThenClear0EC0(void);
void LoadHitBoxAndSpawn117F(uint8_t slot);
void abi_LoadHitBoxAndSpawn117F(void);
void SpawnWhileEarly12DC(uint8_t slot);
void abi_SpawnWhileEarly12DC(void);
void NextSubstateByTable13FC(uint8_t slot);
void abi_NextSubstateByTable13FC(void);
void SpawnFromCount32_16CB(uint8_t slot);
void abi_SpawnFromCount32_16CB(void);
void ListThenWhenSlot24Gone0CAC(uint8_t slot);
void abi_ListThenWhenSlot24Gone0CAC(void);
void ListAndSink0CAC(uint8_t slot);
void abi_ListAndSink0CAC(void);
void ListMoveCount0E0C_32D(uint8_t slot);
void abi_ListMoveCount0E0C_32D(void);
void ListMoveCount0E0C_4E0(uint8_t slot);
void abi_ListMoveCount0E0C_4E0(void);
void SpeedByPlayerSide101B(uint8_t slot);
void abi_SpeedByPlayerSide101B(void);
void RunThenCountClear12DE(uint8_t slot);
void abi_RunThenCountClear12DE(void);

#endif
