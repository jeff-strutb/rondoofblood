/* Lot L1 (the front end: boot, logo, title, attract, file and name menus, game over, demo scenes).
 * Declared here, not in routines.h. */
#ifndef RONDO_FRONT_END_H
#define RONDO_FRONT_END_H

#include <stdbool.h>
#include <stdint.h>
#include "machine.h"

/* Defined elsewhere, typed, and not declared in routines.h. */
void ClearObject(uint8_t object);                    /* object/lifecycle.c */

/* menu/attract.c */
uint8_t AttractFrameTick(void);
/* What AttractEndCountdown ends on: the byte its flags come from, and whether it reset the game state. */
typedef struct {
    uint8_t value;
    bool reset;
} AttractEnd;
AttractEnd AttractEndCountdown(void);
void ClearAllObjects(void);

void abi_AttractFrameTick(void);
void abi_AttractEndCountdown(void);
void abi_ClearAllObjects(void);

#endif
