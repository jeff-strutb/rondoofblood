/* The routines of object/calls_b20.c. */
#ifndef RONDO_CALLS_B20_H
#define RONDO_CALLS_B20_H

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

void BoxByPreviousFrame161C(uint8_t slot);
void abi_BoxByPreviousFrame161C(void);
void FlyFromSlot10_0B4C(uint8_t slot);
void abi_FlyFromSlot10_0B4C(void);
void StartLeaderAndTrail101E(uint8_t slot);
void abi_StartLeaderAndTrail101E(void);
void HoldThenDrop117F(uint8_t slot);
void abi_HoldThenDrop117F(void);
void DriftSpitting18DC(uint8_t slot);
void abi_DriftSpitting18DC(void);
void SinkToB0_12DB(uint8_t slot);
void abi_SinkToB0_12DB(void);
void RainKind9D_09EC(uint8_t slot);
void abi_RainKind9D_09EC(void);
void BoxBesidePrevious101B(uint8_t slot);
void abi_BoxBesidePrevious101B(void);
void LoadSegmentByArg101D(uint8_t slot);
void abi_LoadSegmentByArg101D(void);
void FallingBit12DE(uint8_t slot);
void abi_FallingBit12DE(void);
void LoadWithPartner161C(uint8_t slot);
void abi_LoadWithPartner161C(void);
void ChooseByEdge18DC(uint8_t slot);
void abi_ChooseByEdge18DC(void);
void ChargeThenBrake0B4C(uint8_t slot);
void abi_ChargeThenBrake0B4C(void);
void WrapToTop10CC(uint8_t slot);
void abi_WrapToTop10CC(void);
void LandStartShield10CC(uint8_t slot);
void abi_LandStartShield10CC(void);
void ListThrow117F(uint8_t slot);
void abi_ListThrow117F(void);
void BobUntilRandom161B(uint8_t slot);
void abi_BobUntilRandom161B(void);
void ExitByDoor0EBA(uint8_t slot);
void abi_ExitByDoor0EBA(void);
void ExitByDoor161F(uint8_t slot);
void abi_ExitByDoor161F(void);

#endif
