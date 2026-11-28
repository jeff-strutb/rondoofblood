/* The routines of object/calls_b.c. */
#ifndef RONDO_CALLS_B_H
#define RONDO_CALLS_B_H

#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"

void LoadRecordTail10CD_331(uint8_t slot);
void abi_LoadRecordTail10CD_331(void);
void MoveY12DB_2A8(uint8_t slot);
void abi_MoveY12DB_2A8(void);
void MoveX12DE_074(uint8_t slot);
void abi_MoveX12DE_074(void);
void ClearAndFlag101F(uint8_t slot);
void abi_ClearAndFlag101F(void);
void SoundThenSubstate05CB_6F2(uint8_t slot);
void abi_SoundThenSubstate05CB_6F2(void);
void SoundThenSubstate16CC_1FB(uint8_t slot);
void abi_SoundThenSubstate16CC_1FB(void);
void Sound23ThenDebris0EBF(uint8_t slot);
void abi_Sound23ThenDebris0EBF(void);
void LoadThenSet0EC0_3C3(uint8_t slot);
void abi_LoadThenSet0EC0_3C3(void);
void LoadThenSet101C_307(uint8_t slot);
void abi_LoadThenSet101C_307(void);
void LoadThenSet161B_2FA(uint8_t slot);
void abi_LoadThenSet161B_2FA(void);
void LoadThenSet161B_37F(uint8_t slot);
void abi_LoadThenSet161B_37F(void);
void LoadThenSet161B_63D(uint8_t slot);
void abi_LoadThenSet161B_63D(void);
void LoadThenSet161D_080(uint8_t slot);
void abi_LoadThenSet161D_080(void);
void LoadThenFaceByArg101D(uint8_t slot);
void abi_LoadThenFaceByArg101D(void);
void LoadThenDraw5_12DA(uint8_t slot);
void abi_LoadThenDraw5_12DA(void);
void LoadClearFlagWait12DA(uint8_t slot);
void abi_LoadClearFlagWait12DA(void);
void FallAndGo072C(uint8_t slot);
void abi_FallAndGo072C(void);
void Fall161B(uint8_t slot);
void abi_Fall161B(void);
void State9AndTurnBack088B(uint8_t slot);
void abi_State9AndTurnBack088B(void);
void StateOnWhenNear117F(uint8_t slot);
void abi_StateOnWhenNear117F(void);
void StepList161F_03A(uint8_t slot);
void abi_StepList161F_03A(void);
void StepList161F_052(uint8_t slot);
void abi_StepList161F_052(void);
void ProbeHeight12Tail117C(void);
void abi_ProbeHeight12Tail117C(void);
void ProbeHeightCall117E_2E7(void);
void abi_ProbeHeightCall117E_2E7(void);
void ProbeHeightCall117F_46F(void);
void abi_ProbeHeightCall117F_46F(void);

#endif
