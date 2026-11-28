/* The routines of object/leaf_b2.c. */
#ifndef RONDO_LEAF_B2_H
#define RONDO_LEAF_B2_H

#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"

void PlaceAtLevelWhenCount117B(uint8_t slot);
void abi_PlaceAtLevelWhenCount117B(void);
void SpeedByFacingAndStep0EBE(uint8_t slot);
void abi_SpeedByFacingAndStep0EBE(void);
void SwingStep13FD(uint8_t slot);
void abi_SwingStep13FD(void);
void HitStopToState6_12DC(uint8_t slot);
void abi_HitStopToState6_12DC(void);
void SpeedXTowardTarget117B(uint8_t slot);
void abi_SpeedXTowardTarget117B(void);
void HitBoxByFrameRow117F(uint8_t slot);
void abi_HitBoxByFrameRow117F(void);
void LaunchByFacing18DB(uint8_t slot);
void abi_LaunchByFacing18DB(void);
void ScreenTileAddress0ECB(void);
void abi_ScreenTileAddress0ECB(void);

#endif
