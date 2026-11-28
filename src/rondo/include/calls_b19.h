/* The routines of object/calls_b19.c. */
#ifndef RONDO_CALLS_B19_H
#define RONDO_CALLS_B19_H

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

void CountThenFire12DB(uint8_t slot);
void abi_CountThenFire12DB(void);
void MoveBounceByTable12DE(uint8_t slot);
void abi_MoveBounceByTable12DE(void);
void StrikeSoundsByStep13FB(uint8_t slot);
void abi_StrikeSoundsByStep13FB(void);
void TossThenBecome9B161C(uint8_t slot);
void abi_TossThenBecome9B161C(void);
void FallDroppingKind9C161D(uint8_t slot);
void abi_FallDroppingKind9C161D(void);
void SwingThenScript18DB(uint8_t slot);
void abi_SwingThenScript18DB(void);
void SwingThenChannel18DB(uint8_t slot);
void abi_SwingThenChannel18DB(void);
void WalkInsideScreen18DC(uint8_t slot);
void abi_WalkInsideScreen18DC(void);
void ListSpit0B4C(uint8_t slot);
void abi_ListSpit0B4C(void);
void ThrowThenWait0EBE(uint8_t slot);
void abi_ThrowThenWait0EBE(void);
void ThrowThenWait161C(uint8_t slot);
void abi_ThrowThenWait161C(void);
void LoadSpreadBySlot12DC(uint8_t slot);
void abi_LoadSpreadBySlot12DC(void);
void PopThenRise12DE(uint8_t slot);
void abi_PopThenRise12DE(void);
void FallThenGone16CB(uint8_t slot);
void abi_FallThenGone16CB(void);
void MarkInPool18DD(uint8_t slot);
void abi_MarkInPool18DD(void);
void BlinkThenWakeOther0B4A(uint8_t slot);
void abi_BlinkThenWakeOther0B4A(void);
void CircleCentre0CAC(uint8_t slot);
void abi_CircleCentre0CAC(void);
void SprinkleKind9A0EBC(uint8_t slot);
void abi_SprinkleKind9A0EBC(void);
void LoadAtRandomSide0EC0(uint8_t slot);
void abi_LoadAtRandomSide0EC0(void);
void BobWhileList10CC(uint8_t slot);
void abi_BobWhileList10CC(void);
void LandAnimateByRise117C(uint8_t slot);
void abi_LandAnimateByRise117C(void);
void RiseToTop12DB(uint8_t slot);
void abi_RiseToTop12DB(void);
void ListFallThenCrash12DE(uint8_t slot);
void abi_ListFallThenCrash12DE(void);
void DiveWhenNear0EBF(uint8_t slot);
void abi_DiveWhenNear0EBF(void);
void FlyTillLevel0EBF(uint8_t slot);
void abi_FlyTillLevel0EBF(void);
void FlyTillLevel117C(uint8_t slot);
void abi_FlyTillLevel117C(void);

#endif
