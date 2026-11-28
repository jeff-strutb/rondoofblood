/* The routines of object/calls_b6.c. */
#ifndef RONDO_CALLS_B6_H
#define RONDO_CALLS_B6_H

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

void LoadTurnAndRise13FC(uint8_t slot);
void abi_LoadTurnAndRise13FC(void);
void ListRiseUntil13FC_275(uint8_t slot);
void abi_ListRiseUntil13FC_275(void);
void ListRiseUntil13FD_15F(uint8_t slot);
void abi_ListRiseUntil13FD_15F(void);
void AccelerateUp18DB(uint8_t slot);
void abi_AccelerateUp18DB(void);
void SoundAndClearThree18DC(uint8_t slot);
void abi_SoundAndClearThree18DC(void);
void FallThenBecome22_072C(uint8_t slot);
void abi_FallThenBecome22_072C(void);
void ToState11WhenHit088A(uint8_t slot);
void abi_ToState11WhenHit088A(void);
void ListFaceThenNear0EBE(uint8_t slot);
void abi_ListFaceThenNear0EBE(void);
void LoadAndStart24_101B(uint8_t slot);
void abi_LoadAndStart24_101B(void);
void RunThenFall12DA(uint8_t slot);
void abi_RunThenFall12DA(void);
void CountScriptAndChannel12DB(uint8_t slot);
void abi_CountScriptAndChannel12DB(void);
void LoadRiseAndScript12DB(uint8_t slot);
void abi_LoadRiseAndScript12DB(void);
void ListThenShoot12DD(uint8_t slot);
void abi_ListThenShoot12DD(void);
void SoundAndNextOfCycle12DE(uint8_t slot);
void abi_SoundAndNextOfCycle12DE(void);
void LoadSpeedByFacingAndSlot18DC(uint8_t slot);
void abi_LoadSpeedByFacingAndSlot18DC(void);
void ClearPairAndStop05CB(uint8_t slot);
void abi_ClearPairAndStop05CB(void);
void Frame44AndChannel0E0B(uint8_t slot);
void abi_Frame44AndChannel0E0B(void);
void ListThenWideBox12DB(uint8_t slot);
void abi_ListThenWideBox12DB(void);
void ListThenLeap12DE(uint8_t slot);
void abi_ListThenLeap12DE(void);
void LandAtA8_13FB(uint8_t slot);
void abi_LandAtA8_13FB(void);
void CycleAndSpawn13FB(uint8_t slot);
void abi_CycleAndSpawn13FB(void);
void FallAndDropEmbers0EBD(uint8_t slot);
void abi_FallAndDropEmbers0EBD(void);
void SpawnAtStep101C(uint8_t slot);
void abi_SpawnAtStep101C(void);
void ListSpeedThenCount117F(uint8_t slot);
void abi_ListSpeedThenCount117F(void);
void CountThenSpeed12DE(uint8_t slot);
void abi_CountThenSpeed12DE(void);
void AimWhenBothReady13FD(uint8_t slot);
void abi_AimWhenBothReady13FD(void);
void NextRandomMove117D(uint8_t slot);
void abi_NextRandomMove117D(void);
void MoveSoundThenWait12DE(uint8_t slot);
void abi_MoveSoundThenWait12DE(void);
void LoadWithCompanions12DE(uint8_t slot);
void abi_LoadWithCompanions12DE(void);
void ListThenRaster13FC(uint8_t slot);
void abi_ListThenRaster13FC(void);
void DissolveSinkThenBecome43_13FC(uint8_t slot);
void abi_DissolveSinkThenBecome43_13FC(void);
void FollowSlot16UntilLow13FC(uint8_t slot);
void abi_FollowSlot16UntilLow13FC(void);
void SpawnInRow64_18DB(uint8_t slot);
void abi_SpawnInRow64_18DB(void);
void SpawnWithRandom0B4C(uint8_t slot);
void abi_SpawnWithRandom0B4C(void);
void SpawnScattered0EBB(uint8_t slot);
void abi_SpawnScattered0EBB(void);
void SpawnInTurn101E(uint8_t slot);
void abi_SpawnInTurn101E(void);
void PlaceAtRandom0EBF(uint8_t slot);
void abi_PlaceAtRandom0EBF(void);
void LoadOrFallList10CC(uint8_t slot);
void abi_LoadOrFallList10CC(void);

#endif
