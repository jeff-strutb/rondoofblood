/* The routines of object/calls_b17.c. */
#ifndef RONDO_CALLS_B17_H
#define RONDO_CALLS_B17_H

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

void FaceIfThenAttack12DC(uint8_t slot);
void abi_FaceIfThenAttack12DC(void);
void StartMarkerLeft101F(uint8_t slot);
void abi_StartMarkerLeft101F(void);
void PushWithin161E(uint8_t slot);
void abi_PushWithin161E(void);
void TakeFromPool18DD(uint8_t slot);
void abi_TakeFromPool18DD(void);
void WakeOnRoute101F(uint8_t slot);
void abi_WakeOnRoute101F(void);
void ListThenScatter0E0C(uint8_t slot);
void abi_ListThenScatter0E0C(void);
void OrbitByTrail12DD(uint8_t slot);
void abi_OrbitByTrail12DD(void);
void BoxByPreviousFrame161E(uint8_t slot);
void abi_BoxByPreviousFrame161E(void);
void TumbleThenGone0EBB(uint8_t slot);
void abi_TumbleThenGone0EBB(void);
void ChargeList117F(uint8_t slot);
void abi_ChargeList117F(void);
void StrikeList12DE(uint8_t slot);
void abi_StrikeList12DE(void);
void RiseListSlowing16CC(uint8_t slot);
void abi_RiseListSlowing16CC(void);
void SpitByStep18DB(uint8_t slot);
void abi_SpitByStep18DB(void);
void LoadSpeedThenFall088C(uint8_t slot);
void abi_LoadSpeedThenFall088C(void);
void BounceOnFloor0E0C(uint8_t slot);
void abi_BounceOnFloor0E0C(void);
void ChooseNextAct0F6C(uint8_t slot);
void abi_ChooseNextAct0F6C(void);

#endif
