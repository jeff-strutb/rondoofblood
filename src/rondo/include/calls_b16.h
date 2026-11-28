/* The routines of object/calls_b16.c. */
#ifndef RONDO_CALLS_B16_H
#define RONDO_CALLS_B16_H

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

void CountThenStartRow0E0B(uint8_t slot);
void abi_CountThenStartRow0E0B(void);
void LandThenHittable12DE(uint8_t slot);
void abi_LandThenHittable12DE(void);
void LandThenHittable12DC(uint8_t slot);
void abi_LandThenHittable12DC(void);
void PointCallThenCount0EC0(uint8_t slot);
void abi_PointCallThenCount0EC0(void);
void FallListThenWait12DC(uint8_t slot);
void abi_FallListThenWait12DC(void);
void FallListThenWait12DE(uint8_t slot);
void abi_FallListThenWait12DE(void);
void PatrolList0_117E(uint8_t slot);
void abi_PatrolList0_117E(void);
void PatrolList3_117E(uint8_t slot);
void abi_PatrolList3_117E(void);
void TurnToAttack117E(uint8_t slot);
void abi_TurnToAttack117E(void);
void FaceThenSubstate18DB(uint8_t slot);
void abi_FaceThenSubstate18DB(void);
void FallWithoutPrevious0EBC(uint8_t slot);
void abi_FallWithoutPrevious0EBC(void);
void ListSpawnTwice101C(uint8_t slot);
void abi_ListSpawnTwice101C(void);
void LeadTrail101D(uint8_t slot);
void abi_LeadTrail101D(void);
void LeadTrail12DC(uint8_t slot);
void abi_LeadTrail12DC(void);
void ChannelByFrame18DB(uint8_t slot);
void abi_ChannelByFrame18DB(void);
void FallOrRun088A(uint8_t slot);
void abi_FallOrRun088A(void);
void FaceHopStateTen0F6C(uint8_t slot);
void abi_FaceHopStateTen0F6C(void);
void OrbitByTrail101E(uint8_t slot);
void abi_OrbitByTrail101E(void);
void TurnBackAtRandom0EBF(uint8_t slot);
void abi_TurnBackAtRandom0EBF(void);
void RunThenNewCourse0EBF(uint8_t slot);
void abi_RunThenNewCourse0EBF(void);
void DoorRouteByHit0EBD(uint8_t slot);
void abi_DoorRouteByHit0EBD(void);
void StateSixteenWhenHit088A(uint8_t slot);
void abi_StateSixteenWhenHit088A(void);
void AnimateThenLeapAcross0EBE(uint8_t slot);
void abi_AnimateThenLeapAcross0EBE(void);
void AnimateThenLeapAcross161C(uint8_t slot);
void abi_AnimateThenLeapAcross161C(void);
void CloseDoorOnScreen7_101D(uint8_t slot);
void abi_CloseDoorOnScreen7_101D(void);
void TouchThenTurn117B(uint8_t slot);
void abi_TouchThenTurn117B(void);
void AnimateByWorkDThenState12DE(uint8_t slot);
void abi_AnimateByWorkDThenState12DE(void);

#endif
