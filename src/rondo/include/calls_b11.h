/* The routines of object/calls_b11.c. */
#ifndef RONDO_CALLS_B11_H
#define RONDO_CALLS_B11_H

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

void HitInStates5To7_18DB(uint8_t slot);
void abi_HitInStates5To7_18DB(void);
void FireWhenFacingPlayer101D(uint8_t slot);
void abi_FireWhenFacingPlayer101D(void);
void ListSpiralOut16CC(uint8_t slot);
void abi_ListSpiralOut16CC(void);
void ChecksListThenNextState18DB(uint8_t slot);
void abi_ChecksListThenNextState18DB(void);
void AndVramWithMask088C(void);
void abi_AndVramWithMask088C(void);
void MoveStopAtWall0B4C(uint8_t slot);
void abi_MoveStopAtWall0B4C(void);
void StartAtCountWithSound0CAC(uint8_t slot);
void abi_StartAtCountWithSound0CAC(void);
void LoadRiser0EBF(uint8_t slot);
void abi_LoadRiser0EBF(void);
void WalkAndTurnEvery101B(uint8_t slot);
void abi_WalkAndTurnEvery101B(void);
void SoundScriptAndPose10CC(uint8_t slot);
void abi_SoundScriptAndPose10CC(void);
void ListFlyThenFall117E(uint8_t slot);
void abi_ListFlyThenFall117E(void);
void DripWhenLow12DB(uint8_t slot);
void abi_DripWhenLow12DB(void);
void WaitForSlot18ThenList12DD(uint8_t slot);
void abi_WaitForSlot18ThenList12DD(void);
void HomeOnAngle12DE(uint8_t slot);
void abi_HomeOnAngle12DE(void);
void LoadAimThenList161D(uint8_t slot);
void abi_LoadAimThenList161D(void);
void ListThenLaunch27_18DC(uint8_t slot);
void abi_ListThenLaunch27_18DC(void);
void ThrowOnTurn0F6C(uint8_t slot);
void abi_ThrowOnTurn0F6C(void);
void FollowPreviousWithSlot16_101E(uint8_t slot);
void abi_FollowPreviousWithSlot16_101E(void);
void RaiseSlot67_117F(uint8_t slot);
void abi_RaiseSlot67_117F(void);
void PieceChannelByFrame10CB(uint8_t slot);
void abi_PieceChannelByFrame10CB(void);

#endif
