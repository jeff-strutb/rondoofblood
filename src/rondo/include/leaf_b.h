/* The routines of object/leaf_b.c. */
#ifndef RONDO_LEAF_B_H
#define RONDO_LEAF_B_H

#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"

void HitBoxByFrameRow101B(uint8_t slot);
void abi_HitBoxByFrameRow101B(void);
void HitBoxByFrameRange161C(uint8_t slot);
void abi_HitBoxByFrameRange161C(void);
void MoveXWithScrollBand117B(uint8_t slot);
void abi_MoveXWithScrollBand117B(void);
void SumXVelocity0EBC(uint8_t slot);
void abi_SumXVelocity0EBC(void);
void StopWhenSlot16Marked0E0C(uint8_t slot);
void abi_StopWhenSlot16Marked0E0C(void);

#endif
