/* The routines of object/calls_b28.c. */
#ifndef RONDO_CALLS_B28_H
#define RONDO_CALLS_B28_H

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

void LungeFromList0E0B(uint8_t slot);
void abi_LungeFromList0E0B(void);
void WalkByStateTurning0E0B(uint8_t slot);
void abi_WalkByStateTurning0E0B(void);
void SpitWhileList12DC(uint8_t slot);
void abi_SpitWhileList12DC(void);
void FlickerIn12DB(uint8_t slot);
void abi_FlickerIn12DB(void);
void SparkPop0CAD(uint8_t slot);
void abi_SparkPop0CAD(void);
void BurstRow10CC(uint8_t slot);
void abi_BurstRow10CC(void);
void ListByWorkIStomp117E(uint8_t slot);
void abi_ListByWorkIStomp117E(void);

#endif
