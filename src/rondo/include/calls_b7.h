/* The routines of object/calls_b7.c. */
#ifndef RONDO_CALLS_B7_H
#define RONDO_CALLS_B7_H

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

void RasterByLevelX117A(uint8_t slot);
void abi_RasterByLevelX117A(void);
void ListSpeedProbe117D(uint8_t slot);
void abi_ListSpeedProbe117D(void);
void ListSpeedProbe161D(uint8_t slot);
void abi_ListSpeedProbe161D(void);
void WaitOnLeader117F(uint8_t slot);
void abi_WaitOnLeader117F(void);
void StartEightAtBE_12DE(void);
void abi_StartEightAtBE_12DE(void);
void FallProbeToA8_13FB(uint8_t slot);
void abi_FallProbeToA8_13FB(void);
void SwingFallToBC_13FD(uint8_t slot);
void abi_SwingFallToBC_13FD(void);
void LoadTossFromSlot17_16CC(uint8_t slot);
void abi_LoadTossFromSlot17_16CC(void);
void CountThenStart21_0B4D(uint8_t slot);
void abi_CountThenStart21_0B4D(void);
void ListAtFrame40Spawn0E0B(uint8_t slot);
void abi_ListAtFrame40Spawn0E0B(void);
void ListLandScript10CC(uint8_t slot);
void abi_ListLandScript10CC(void);
void ListCountSoundSpawn12DB(uint8_t slot);
void abi_ListCountSoundSpawn12DB(void);
void ListSoundAt48_12DD(uint8_t slot);
void abi_ListSoundAt48_12DD(void);
void ListThenShoot13_12DE(uint8_t slot);
void abi_ListThenShoot13_12DE(void);
void CountScriptSpawn16CB(uint8_t slot);
void abi_CountScriptSpawn16CB(void);
void ListThenSpawn26_18DB(uint8_t slot);
void abi_ListThenSpawn26_18DB(void);
void StartPairFacing0B4C(uint8_t slot);
void abi_StartPairFacing0B4C(void);
void ListLandSounds0E0B(uint8_t slot);
void abi_ListLandSounds0E0B(void);
void StateByPlace0EBF(uint8_t slot);
void abi_StateByPlace0EBF(void);
void ListSpawnOrEnd101C(uint8_t slot);
void abi_ListSpawnOrEnd101C(void);
void StartSix9A_101E(uint8_t slot);
void abi_StartSix9A_101E(void);
void LoadDriftOrFall161E(uint8_t slot);
void abi_LoadDriftOrFall161E(void);
void StartByAnimationStep18DB(uint8_t slot);
void abi_StartByAnimationStep18DB(void);
void ListThenTwoScripts18DC(uint8_t slot);
void abi_ListThenTwoScripts18DC(void);
void LandAtB5_088B(uint8_t slot);
void abi_LandAtB5_088B(void);
void LoadFaceOrListClear09EC(uint8_t slot);
void abi_LoadFaceOrListClear09EC(void);
void ListByFacingThenState3_0CAB(uint8_t slot);
void abi_ListByFacingThenState3_0CAB(void);
void DissolveThenBecome21_0E0B(uint8_t slot);
void abi_DissolveThenBecome21_0E0B(void);
void StartRowThenClear101C(uint8_t slot);
void abi_StartRowThenClear101C(void);
void FallAndScatter101C(uint8_t slot);
void abi_FallAndScatter101C(void);
void LoadFrameByWorkD101E(uint8_t slot);
void abi_LoadFrameByWorkD101E(void);
void LandThenStartPair10CC(uint8_t slot);
void abi_LandThenStartPair10CC(void);
void DissolveThenBecome29_10CC(uint8_t slot);
void abi_DissolveThenBecome29_10CC(void);
void LoadOrChannelWhenFrame71_10CD(uint8_t slot);
void abi_LoadOrChannelWhenFrame71_10CD(void);
void ScriptByFrameMatch10CB(uint8_t slot);
void abi_ScriptByFrameMatch10CB(void);

#endif
