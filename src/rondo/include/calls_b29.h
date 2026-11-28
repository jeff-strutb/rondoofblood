/* The routines of object/calls_b29.c. */
#ifndef RONDO_CALLS_B29_H
#define RONDO_CALLS_B29_H

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

void ShieldOfLeader18DC(uint8_t slot);
void abi_ShieldOfLeader18DC(void);
void DissolveFall072C(uint8_t slot);
void abi_DissolveFall072C(void);
void LandAnimateByRise0EBF(uint8_t slot);
void abi_LandAnimateByRise0EBF(void);
void ChaseOrLeap0F6C(uint8_t slot);
void abi_ChaseOrLeap0F6C(void);
void GateOnScreen4_101D(uint8_t slot);
void abi_GateOnScreen4_101D(void);
void IntroList088A(uint8_t slot);
void abi_IntroList088A(void);
void ArmByLeader1180(uint8_t slot);
void abi_ArmByLeader1180(void);

#endif
