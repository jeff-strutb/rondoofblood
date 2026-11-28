/* The routines of object/calls_b3.c. */
#ifndef RONDO_CALLS_B3_H
#define RONDO_CALLS_B3_H

#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"

void RecordThenClear161E(uint8_t slot);
void abi_RecordThenClear161E(void);
void WhenSlot20Ready088B(uint8_t slot);
void abi_WhenSlot20Ready088B(void);
void PushByAnswer088A(uint8_t slot);
void abi_PushByAnswer088A(void);
void ToState5WhenSlot2Ready088B(uint8_t slot);
void abi_ToState5WhenSlot2Ready088B(void);
void StartWhenSlot18Ready088B(uint8_t slot);
void abi_StartWhenSlot18Ready088B(void);
void ClearFirstEmptySlot0EBD(void);
void abi_ClearFirstEmptySlot0EBD(void);
void HitStateWhenNear101E(uint8_t slot);
void abi_HitStateWhenNear101E(void);
void StepListByWorkF1020(uint8_t slot);
void abi_StepListByWorkF1020(void);
void ListThenSubstate1_10CC(uint8_t slot);
void abi_ListThenSubstate1_10CC(void);
void SpawnOddFramesAnswer13FB(uint8_t slot);
void abi_SpawnOddFramesAnswer13FB(void);
void LoadAndStartNext161D(uint8_t slot);
void abi_LoadAndStartNext161D(void);
void ListThenClearOnCount16CC(uint8_t slot);
void abi_ListThenClearOnCount16CC(void);
void StartKind2AIn32To43_12DC(uint8_t slot);
void abi_StartKind2AIn32To43_12DC(void);
void LoadFrame4OrFall0B4C(uint8_t slot);
void abi_LoadFrame4OrFall0B4C(void);
void ListFallMove0B4D(uint8_t slot);
void abi_ListFallMove0B4D(void);
void LandOnRowThenDebris117C(uint8_t slot);
void abi_LandOnRowThenDebris117C(void);

#endif
