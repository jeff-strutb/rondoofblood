/* The routines of object/calls_b14.c. */
#ifndef RONDO_CALLS_B14_H
#define RONDO_CALLS_B14_H

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

void FaceAndAnimate0EBE(uint8_t slot);
void abi_FaceAndAnimate0EBE(void);
void FaceAndAnimate0EBF(uint8_t slot);
void abi_FaceAndAnimate0EBF(void);
void FaceAndAnimate161C(uint8_t slot);
void abi_FaceAndAnimate161C(void);
void LoadFrameByArg05CC(uint8_t slot);
void abi_LoadFrameByArg05CC(void);
void ListThenSubstate13FC(uint8_t slot);
void abi_ListThenSubstate13FC(void);
void LoadScriptInStageFiveClear088C(uint8_t slot);
void abi_LoadScriptInStageFiveClear088C(void);
void StartKind9BOnBeat101B(uint8_t slot);
void abi_StartKind9BOnBeat101B(void);
void RestartIntoStateTwo101B(uint8_t slot);
void abi_RestartIntoStateTwo101B(void);
void LandThenFrame74_101C(uint8_t slot);
void abi_LandThenFrame74_101C(void);
void WakeWhenNear101F(uint8_t slot);
void abi_WakeWhenNear101F(void);
void SoundHitOffWait10CC(uint8_t slot);
void abi_SoundHitOffWait10CC(void);
void LoadByPieceSlot16CC(uint8_t slot);
void abi_LoadByPieceSlot16CC(void);
void LoadByPieceSlot09EC(uint8_t slot);
void abi_LoadByPieceSlot09EC(void);
void ScriptByFrame0B4C(uint8_t slot);
void abi_ScriptByFrame0B4C(void);
void ListThenStateTwo101B(uint8_t slot);
void abi_ListThenStateTwo101B(void);
void CallWhenLowAndDue0EBC(uint8_t slot);
void abi_CallWhenLowAndDue0EBC(void);
void ClearUnlessScriptFive12DB(uint8_t slot);
void abi_ClearUnlessScriptFive12DB(void);
void WaitThenScriptSound16CB(uint8_t slot);
void abi_WaitThenScriptSound16CB(void);
void ListThenWait05CB(uint8_t slot);
void abi_ListThenWait05CB(void);
void FallThenLandCall088B(uint8_t slot);
void abi_FallThenLandCall088B(void);
void LoadThenRideSlot10_09EC(uint8_t slot);
void abi_LoadThenRideSlot10_09EC(void);
void StartSevenKind26_0E0C(uint8_t slot);
void abi_StartSevenKind26_0E0C(void);
void FaceCallStateTwelve0F6C(uint8_t slot);
void abi_FaceCallStateTwelve0F6C(void);
void ClearOneEmptyOf20to27_0EBF(void);
void abi_ClearOneEmptyOf20to27_0EBF(void);

#endif
