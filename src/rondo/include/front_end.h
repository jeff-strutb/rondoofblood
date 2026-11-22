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

/* A byte and the index that goes with it (Y), as a routine leaves them in A and Y. */
typedef struct {
    uint8_t value;
    uint8_t index;
} ByteIndex;

/* Placeholders: the event flags the save remembers (EventFlags, 32 bytes) and the backup file. */
#define EVENT_FLAGS 0x26D3

typedef struct { uint8_t count; bool carry; uint8_t x; } CountBits;
typedef struct { uint8_t a; bool carry; uint8_t x; uint8_t y; } SlotSum;
typedef struct { uint8_t a; uint8_t y; bool v; } SlotFlags;
typedef struct { uint8_t a; uint8_t x; uint8_t y; bool v; bool found; } SlotFound;
typedef struct { uint8_t a; uint8_t x; } SlotCopy;

/* Placeholders until the names land (menu/file_select.c lists what each holds): ROM tables, the
 * backup file in RAM and the work area the file menu copies a slot to. */
#define SLOT_OFFSETS 0x9252
#define SAVE_FILE 0x7D00
#define CLEARED_FLAGS 0x3D00
#define CLEARED_TABLE_OFFSETS 0xAFD6
#define CLEARED_TABLE_MASKS 0xAFE3
#define SLOT_WORK 0x3F40
#define PAD_REPEAT 0x7DF2
#define PAD_REPEAT_BITS 0x7DFA
#define SELECTED_FILE 0x7DF1
#define CURSOR_ROW 0x7DFF

/* menu/file_select.c */
void WriteHexByteTiles(uint8_t byte);
void WriteLowDigitTile(uint8_t value);
void WriteDigitTile(uint8_t digit);
SlotSum SlotCompletionPercent(uint8_t slot);
SlotFlags SlotClearedFlags(void);
SlotFound SlotLastClearedFlag(void);
SlotCopy SlotToWorkArea(void);
uint8_t PadRepeatUpdate(void);
Alu DrawSelectedFileDigit(void);
Alu DrawCursorRowDigit(void);
void abi_WriteHexByteTiles(void);
void abi_WriteLowDigitTile(void);
void abi_WriteDigitTile(void);
void abi_SlotCompletionPercent(void);
void abi_SlotClearedFlags(void);
void abi_SlotLastClearedFlag(void);
void abi_SlotToWorkArea(void);
void abi_PadRepeatUpdate(void);
void abi_DrawSelectedFileDigit(void);
void abi_DrawCursorRowDigit(void);
ByteIndex EventBitMask(uint8_t event);
ByteIndex EventBitClear(uint8_t event);
ByteIndex EventBitSet(uint8_t event);
ByteIndex EventBitTest(uint8_t event);
void EventFlagsClear(void);
CountBits CountSetBits(uint8_t byte, uint8_t kept_x);
void abi_EventBitMask(void);
void abi_EventBitClear(void);
void abi_EventBitSet(void);
void abi_EventBitTest(void);
void abi_EventFlagsClear(void);
void abi_CountSetBits(void);
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
