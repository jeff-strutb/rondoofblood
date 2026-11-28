/* The routines of object/calls_b27.c. */
#ifndef RONDO_CALLS_B27_H
#define RONDO_CALLS_B27_H

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

void ShardByArgArc0F6C(uint8_t slot);
void abi_ShardByArgArc0F6C(void);
void LoadFlyInByArg161B(uint8_t slot);
void abi_LoadFlyInByArg161B(void);
void RaiseArmed18DB(uint8_t slot);
void abi_RaiseArmed18DB(void);
void DebrisFallThenGround12DC(uint8_t slot);
void abi_DebrisFallThenGround12DC(void);
void WeaveThenDive0EBF(uint8_t slot);
void abi_WeaveThenDive0EBF(void);
void PieceArcThenRest0F6D(uint8_t slot);
void abi_PieceArcThenRest0F6D(void);
void LoadFromSide0EBD(uint8_t slot);
void abi_LoadFromSide0EBD(void);

#endif
