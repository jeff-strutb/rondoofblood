/* The routines of object/calls_b18.c. */
#ifndef RONDO_CALLS_B18_H
#define RONDO_CALLS_B18_H

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

void StopMusicOnStep18DB(uint8_t slot);
void abi_StopMusicOnStep18DB(void);
void ArcAndSpin0EBD(uint8_t slot);
void abi_ArcAndSpin0EBD(void);
void KnockedLoose117D(uint8_t slot);
void abi_KnockedLoose117D(void);
void AccelerateToSpeed117E(uint8_t slot);
void abi_AccelerateToSpeed117E(void);
void ChargeUntilListEnds117F(uint8_t slot);
void abi_ChargeUntilListEnds117F(void);
void TurnQuarterAlt12DE(uint8_t slot);
void abi_TurnQuarterAlt12DE(void);
void BobAndCount161B(uint8_t slot);
void abi_BobAndCount161B(void);
void CountDownSpawning161E(uint8_t slot);
void abi_CountDownSpawning161E(void);
void LaunchByPattern072C(uint8_t slot);
void abi_LaunchByPattern072C(void);
void LoadAtTopOrFace0EBE(uint8_t slot);
void abi_LoadAtTopOrFace0EBE(void);
void TossThenBecome9B0EBE(uint8_t slot);
void abi_TossThenBecome9B0EBE(void);
void DoorByHit117B(uint8_t slot);
void abi_DoorByHit117B(void);
void NextSubstateByHeight117D(uint8_t slot);
void abi_NextSubstateByHeight117D(void);

#endif
