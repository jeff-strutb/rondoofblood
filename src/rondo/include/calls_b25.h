/* The routines of object/calls_b25.c. */
#ifndef RONDO_CALLS_B25_H
#define RONDO_CALLS_B25_H

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

void ShatterEight117D(uint8_t slot);
void abi_ShatterEight117D(void);
void ShatterEleven0EBD(uint8_t slot);
void abi_ShatterEleven0EBD(void);
void SlideWhileList161D(uint8_t slot);
void abi_SlideWhileList161D(void);
void BobbingLift161E(uint8_t slot);
void abi_BobbingLift161E(void);
void OrbitSlot1F_101D(uint8_t slot);
void abi_OrbitSlot1F_101D(void);
void RaiseColumn072C(uint8_t slot);
void abi_RaiseColumn072C(void);
void HoverThenSwoop0EC0(uint8_t slot);
void abi_HoverThenSwoop0EC0(void);
void StepOffThenWake0EC0(uint8_t slot);
void abi_StepOffThenWake0EC0(void);
void SwoopAndClimb0EBF(uint8_t slot);
void abi_SwoopAndClimb0EBF(void);
void SwoopAndClimb117B(uint8_t slot);
void abi_SwoopAndClimb117B(void);
void PlayerRidesBell0EBD(uint8_t slot);
void abi_PlayerRidesBell0EBD(void);

#endif
