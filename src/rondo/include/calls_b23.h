/* The routines of object/calls_b23.c. */
#ifndef RONDO_CALLS_B23_H
#define RONDO_CALLS_B23_H

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

void BounceThenOpen117A(uint8_t slot);
void abi_BounceThenOpen117A(void);
void BounceThenOpen161A(uint8_t slot);
void abi_BounceThenOpen161A(void);
void LeadOrLaunch101E(uint8_t slot);
void abi_LeadOrLaunch101E(void);
void WalkByStateList12DD(uint8_t slot);
void abi_WalkByStateList12DD(void);
void DropThenScatter16CC(uint8_t slot);
void abi_DropThenScatter16CC(void);
void SwitchPlatform0EC0(uint8_t slot);
void abi_SwitchPlatform0EC0(void);
void ReleaseOnStateChange0EC0(uint8_t slot);
void abi_ReleaseOnStateChange0EC0(void);
void LoadLeapByKind117E(uint8_t slot);
void abi_LoadLeapByKind117E(void);
void LoadColumnOfFour161E(uint8_t slot);
void abi_LoadColumnOfFour161E(void);
void GateStates101E(uint8_t slot);
void abi_GateStates101E(void);
void LoadDelaysAtRandom10CD(uint8_t slot);
void abi_LoadDelaysAtRandom10CD(void);
void DriftingBackground1020(uint8_t slot);
void abi_DriftingBackground1020(void);
void SparkFromSlot10_072D(uint8_t slot);
void abi_SparkFromSlot10_072D(void);
void SlideWithCosine10CC(uint8_t slot);
void abi_SlideWithCosine10CC(void);

#endif
