/* The routines of object/calls_b26.c. */
#ifndef RONDO_CALLS_B26_H
#define RONDO_CALLS_B26_H

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

void BurstIntoSix0EBE(uint8_t slot);
void abi_BurstIntoSix0EBE(void);
void SpitThenRest161B(uint8_t slot);
void abi_SpitThenRest161B(void);
void PlatformCarry12DD(uint8_t slot);
void abi_PlatformCarry12DD(void);
void LoadAtSideWithPartner0EBD(uint8_t slot);
void abi_LoadAtSideWithPartner0EBD(void);
void SinkOnWater09EB(uint8_t slot);
void abi_SinkOnWater09EB(void);
void CarrierPlatform117B(uint8_t slot);
void abi_CarrierPlatform117B(void);
void RiseSwayThenGo0F6D(uint8_t slot);
void abi_RiseSwayThenGo0F6D(void);
void SeekingOrb13FD(uint8_t slot);
void abi_SeekingOrb13FD(void);

#endif
