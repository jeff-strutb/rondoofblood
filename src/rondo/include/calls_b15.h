/* The routines of object/calls_b15.c. */
#ifndef RONDO_CALLS_B15_H
#define RONDO_CALLS_B15_H

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

void AnimateThenDrift0EBE(uint8_t slot);
void abi_AnimateThenDrift0EBE(void);
void AnimateThenDrift161C(uint8_t slot);
void abi_AnimateThenDrift161C(void);
void AnimateThenStateThree0EBE(uint8_t slot);
void abi_AnimateThenStateThree0EBE(void);
void AnimateThenStateThree161C(uint8_t slot);
void abi_AnimateThenStateThree161C(void);
void CountThenRecordClear0EBD(uint8_t slot);
void abi_CountThenRecordClear0EBD(void);
void AnimateByWorkF10CD(uint8_t slot);
void abi_AnimateByWorkF10CD(void);
void ListThenHittable101D(uint8_t slot);
void abi_ListThenHittable101D(void);
void DriftThenPickBySide12DE(uint8_t slot);
void abi_DriftThenPickBySide12DE(void);
void FollowSlotZeroFor101C(uint8_t slot);
void abi_FollowSlotZeroFor101C(void);
void PointCallThenCount117B(uint8_t slot);
void abi_PointCallThenCount117B(void);
void ListCallThenSubstate13FB(uint8_t slot);
void abi_ListCallThenSubstate13FB(void);
void FrameLandCall088A(uint8_t slot);
void abi_FrameLandCall088A(void);
void TurnToAttack117F(uint8_t slot);
void abi_TurnToAttack117F(void);
void TurnToAttack161D(uint8_t slot);
void abi_TurnToAttack161D(void);
void TurnToBackOff117F(uint8_t slot);
void abi_TurnToBackOff117F(void);
void TurnToBackOff161D(uint8_t slot);
void abi_TurnToBackOff161D(void);
void KeepArgsThenClear101F(uint8_t slot);
void abi_KeepArgsThenClear101F(void);
void ChannelScriptThenClear101F(uint8_t slot);
void abi_ChannelScriptThenClear101F(void);
void ListThenSound46_10CB(uint8_t slot);
void abi_ListThenSound46_10CB(void);
void TurnOrClearSlots12to15_088B(void);
void abi_TurnOrClearSlots12to15_088B(void);
void WaitFrameListThenState0EBF(uint8_t slot);
void abi_WaitFrameListThenState0EBF(void);
void ListThenResetOther0EBD(uint8_t slot);
void abi_ListThenResetOther0EBD(void);
void FallToFloorA4_12DC(uint8_t slot);
void abi_FallToFloorA4_12DC(void);
void FallToFloor93_12DE(uint8_t slot);
void abi_FallToFloor93_12DE(void);
void StartFifteenKind2B_0E0B(uint8_t slot);
void abi_StartFifteenKind2B_0E0B(void);
void StateWhileOwner12DD(uint8_t slot);
void abi_StateWhileOwner12DD(void);
void StateWhileOwner16CC(uint8_t slot);
void abi_StateWhileOwner16CC(void);
void CountHitsThenState16CB(uint8_t slot);
void abi_CountHitsThenState16CB(void);
void KeepLastPointLandSubstate0E0B(uint8_t slot);
void abi_KeepLastPointLandSubstate0E0B(void);

#endif
