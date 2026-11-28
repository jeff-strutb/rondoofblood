/* The routines of object/calls_b10.c. */
#ifndef RONDO_CALLS_B10_H
#define RONDO_CALLS_B10_H

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

void FallCountThenShoot10CC(uint8_t slot);
void abi_FallCountThenShoot10CC(void);
void LoadWithArms117F(uint8_t slot);
void abi_LoadWithArms117F(void);
void DripWhileEarly12DE(uint8_t slot);
void abi_DripWhileEarly12DE(void);
void ListThenLeapByFacing13FB(uint8_t slot);
void abi_ListThenLeapByFacing13FB(void);
void AfterImage13FD(uint8_t slot);
void abi_AfterImage13FD(void);
void ListRiseFasterThenClear16CC(uint8_t slot);
void abi_ListRiseFasterThenClear16CC(void);
void CountThenOutburst18DC(uint8_t slot);
void abi_CountThenOutburst18DC(void);
void ListThenWalkByFacing18DC(uint8_t slot);
void abi_ListThenWalkByFacing18DC(void);
void SinkToB8_05CB(uint8_t slot);
void abi_SinkToB8_05CB(void);
void ListThenJumpWhen0B4C(uint8_t slot);
void abi_ListThenJumpWhen0B4C(void);
void LoadAboveRightWithPartner0EBD(uint8_t slot);
void abi_LoadAboveRightWithPartner0EBD(void);
void SegmentBySlot101D(uint8_t slot);
void abi_SegmentBySlot101D(void);
void SoundAtStep9ThenState4_161C(uint8_t slot);
void abi_SoundAtStep9ThenState4_161C(void);
void ListSinkToB8Then16CC(uint8_t slot);
void abi_ListSinkToB8Then16CC(void);
void FallDissolvingThenBecome22_05CB(uint8_t slot);
void abi_FallDissolvingThenBecome22_05CB(void);
void LoadOnCircle0CAC(uint8_t slot);
void abi_LoadOnCircle0CAC(void);
void ListFallCountThen0E0C(uint8_t slot);
void abi_ListFallCountThen0E0C(void);
void PickNextSubstate101C(uint8_t slot);
void abi_PickNextSubstate101C(void);
void LoadShotByArg101C(uint8_t slot);
void abi_LoadShotByArg101C(void);
void LoadItemByArg161F(uint8_t slot);
void abi_LoadItemByArg161F(void);
void ListByAThenCheck117E(uint8_t slot);
void abi_ListByAThenCheck117E(void);
void BobThenClear088C(uint8_t slot);
void abi_BobThenClear088C(void);

#endif
