/* The routines of object/calls_b12.c. */
#ifndef RONDO_CALLS_B12_H
#define RONDO_CALLS_B12_H

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

void LoadWithArmsAndScript12DD(uint8_t slot);
void abi_LoadWithArmsAndScript12DD(void);
void SpitThenLeapByFacing0B4C(uint8_t slot);
void abi_SpitThenLeapByFacing0B4C(void);
void LoadSoundThenListClear101C(uint8_t slot);
void abi_LoadSoundThenListClear101C(void);
void FireTwiceAtStep13_13FB(uint8_t slot);
void abi_FireTwiceAtStep13_13FB(void);
void ListThenThreeShots161D(uint8_t slot);
void abi_ListThenThreeShots161D(void);
void SpitDebrisEvery8_16CB(uint8_t slot);
void abi_SpitDebrisEvery8_16CB(void);
void PaceBackAndForth117B(uint8_t slot);
void abi_PaceBackAndForth117B(void);
void ArenaStartOnScreen3_12DA(uint8_t slot);
void abi_ArenaStartOnScreen3_12DA(void);
void TurnQuarter12DE(uint8_t slot);
void abi_TurnQuarter12DE(void);
void ListStrikeThenState4_117E(uint8_t slot);
void abi_ListStrikeThenState4_117E(void);
void ListSoundThenTurnLeap0B4C(uint8_t slot);
void abi_ListSoundThenTurnLeap0B4C(void);
void FallListThenRest0B4C(uint8_t slot);
void abi_FallListThenRest0B4C(void);
void LoadHopThenWalk0EC0(uint8_t slot);
void abi_LoadHopThenWalk0EC0(void);
void StartRowByWorkK161C(uint8_t slot);
void abi_StartRowByWorkK161C(void);
void ListThenBurstSix161E(uint8_t slot);
void abi_ListThenBurstSix161E(void);
void LoadByArg12DE(uint8_t slot);
void abi_LoadByArg12DE(void);
void WaveAccumulate12DC(uint8_t slot);
void abi_WaveAccumulate12DC(void);

#endif
