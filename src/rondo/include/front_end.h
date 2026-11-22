/* Lot L1 (the front end: boot, logo, title, attract, file and name menus, game over, demo scenes).
 * Declared here, not in routines.h. */
#ifndef RONDO_FRONT_END_H
#define RONDO_FRONT_END_H

#include <stdbool.h>
#include <stdint.h>
#include "machine.h"

/* Defined elsewhere, typed, and not declared in routines.h. */
void ClearObject(uint8_t object);                    /* object/lifecycle.c */

/* A value and the carry a routine leaves with it. */
typedef struct {
    uint8_t value;
    bool carry;
} ValueCarry;

/* Where a cursor goes: the table index, the table byte last read (the Y place) and the carry. */
typedef struct {
    uint8_t index;
    uint8_t value;
    bool carry;
} CursorTarget;

/* Placeholders until the names land (menu/file_select.c lists what each holds): the table of cursor
 * places by cursor value and cursor. */
#define CURSOR_PLACES 0xAD1F

/* menu/file_select.c */
void SetMenuStep(uint8_t step);
uint8_t MenuStepAdvance(void);
void SetMenuPage(uint8_t page);
uint8_t MenuPageAdvance(void);
ValueCarry ObjectFrameFromTick(uint8_t object);
Alu HalveObjectVelocity(uint8_t object);
CursorTarget SetCursorTarget(uint8_t object);
CursorTarget SetCursorTargets(void);
CursorTarget SetCursorValue(uint8_t value);
CursorTarget PlaceCursorObject(uint8_t value, uint8_t object);
CursorTarget PlaceCursorObjects(uint8_t value);
Alu ShiftObjectsByScrollChange(void);
void abi_SetMenuStep(void);
void abi_MenuStepAdvance(void);
void abi_SetMenuPage(void);
void abi_MenuPageAdvance(void);
void abi_ObjectFrameFromTick(void);
void abi_HalveObjectVelocity(void);
void abi_SetCursorTarget(void);
void abi_SetCursorTargets(void);
void abi_SetCursorValue(void);
void abi_PlaceCursorObject(void);
void abi_PlaceCursorObjects(void);
void abi_ShiftObjectsByScrollChange(void);

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
