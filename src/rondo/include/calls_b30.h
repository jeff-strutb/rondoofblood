/* The routines of object/calls_b30.c. */
#ifndef RONDO_CALLS_B30_H
#define RONDO_CALLS_B30_H

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

void BubbleAtRandom18DD(uint8_t slot);
void abi_BubbleAtRandom18DD(void);
void WeightPlatform0EBA(uint8_t slot);
void abi_WeightPlatform0EBA(void);
void SinkThenTurn18DB(uint8_t slot);
void abi_SinkThenTurn18DB(void);
void HoldAndSwallow101D(uint8_t slot);
void abi_HoldAndSwallow101D(void);
void ArmedPace10CC(uint8_t slot);
void abi_ArmedPace10CC(void);
void BellStruck0EBB(uint8_t slot);
void abi_BellStruck0EBB(void);
void OpenArenaWithRaster0E0B(uint8_t slot);
void abi_OpenArenaWithRaster0E0B(void);

#endif
