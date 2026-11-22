/* Lot L1 (the front end: boot, logo, title, attract, file and name menus, game over, demo scenes).
 * Declared here, not in routines.h. */
#ifndef RONDO_FRONT_END_H
#define RONDO_FRONT_END_H

#include <stdbool.h>
#include <stdint.h>
#include "machine.h"

/* Defined elsewhere, typed, and not declared in routines.h. */
void ClearObject(uint8_t object);                    /* object/lifecycle.c */

/* What a routine leaves in the registers and flags, for its abi_ wrapper: the registers it sets, the
 * value N and Z come from, C, V when it sets V, and a flag for each register it leaves as it was. */
typedef struct {
    uint8_t a, x, y, nz;
    bool c, v, v_set, a_kept, x_kept, y_kept, from_cpu;
} Exit;

static inline void exit_shim(Exit e)
{
    if (e.from_cpu) {
        return;
    }
    if (!e.a_kept) {
        cpu.a = e.a;
    }
    if (!e.x_kept) {
        cpu.x = e.x;
    }
    if (!e.y_kept) {
        cpu.y = e.y;
    }
    abi_nz(e.nz);
    abi_flag(P_C, e.c);
    if (e.v_set) {
        abi_flag(P_V, e.v);
    }
}

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
#define GLYPH_BUFFER 0x3B40
#define TEXT_ROW_ADDRESSES 0xA894
#define TEXT_PALETTES 0xA91B
#define TEXT_COLOR_ADDRESSES 0xA901
#define PERCENT_ADDRESSES 0xA845
#define GLYPH_PLACES 0xA500
#define GLYPH_NUMBERS 0xA18F
#define GLYPH_PASSES 0xA0F2
#define OBJECT_LISTS 0xA55A
#define SLOT_FACES 0x7DE3
#define FACE_TILES 0xA9BE

/* menu/name_entry.c: what the name menu leaves, and its placeholders. */
typedef struct { uint8_t a; uint8_t y; bool carry; bool overflow; bool full; } NameExit;
typedef struct { uint8_t y; bool carry; } NameCursorExit;
#define NAME_POS 0x7DE8
#define NAME_CHARS 0x7DE9
#define NAME_LENGTH 8
#define NAME_GRID 0x9645
#define NAME_ROW_PLACES 0x969E
#define NAME_COLUMN_PLACES 0x9696
NameExit ShowNameCursor(void);
NameExit NameBackspace(void);
NameExit NameTypeCharacter(void);
NameCursorExit NameCursorTarget(uint8_t object);
void abi_ShowNameCursor(void);
void abi_NameBackspace(void);
void abi_NameTypeCharacter(void);
void abi_NameCursorTarget(void);

typedef struct { uint8_t a; uint8_t x; } GlyphRows;

/* menu/file_select.c */
Exit BlankTextRow(uint8_t row, uint8_t kept_x);
Exit RecolorTextRun(uint8_t row, uint8_t palette_index);
Exit DrawSlotPercent(uint8_t row, uint8_t kept_x);
Exit OrBufferIntoVramLow(uint8_t kept_x);
Exit ShiftGlyphBufferRight(uint8_t kept_x);
Exit SetGlyphVramPlace(uint8_t glyph, uint8_t object);
Exit OverlayGlyphRows(uint8_t object);
Exit OverlayGlyphPass(uint8_t object);
Exit OverlayGlyphRowsBoth(uint8_t object);
Exit OverlayGlyphPassBoth(uint8_t object);
Exit ClearVramBlock(uint8_t object);
Exit GlideObjectUp(uint8_t object);
Exit GlideObjectDown(uint8_t object);
Exit GlyphPassByStep(uint8_t object);
Exit PlaceObjectList(uint8_t list);
Exit DrawSlotName(uint8_t row, uint8_t kept_x);
Exit DrawSlotContinuesAndFace(void);
Exit DrawSlotDetails(void);
void abi_BlankTextRow(void);
void abi_RecolorTextRun(void);
void abi_DrawSlotPercent(void);
void abi_OrBufferIntoVramLow(void);
void abi_ShiftGlyphBufferRight(void);
void abi_SetGlyphVramPlace(void);
void abi_OverlayGlyphRows(void);
void abi_OverlayGlyphPass(void);
void abi_OverlayGlyphRowsBoth(void);
void abi_OverlayGlyphPassBoth(void);
void abi_ClearVramBlock(void);
void abi_GlideObjectUp(void);
void abi_GlideObjectDown(void);
void abi_GlyphPassByStep(void);
void abi_PlaceObjectList(void);
void abi_DrawSlotName(void);
void abi_DrawSlotContinuesAndFace(void);
void abi_DrawSlotDetails(void);
void PadRepeatReset(void);
bool MenuBackOnButtonII(void);
void MenuBackToPageEight(void);
uint8_t IncrementTempWord(void);
GlyphRows WriteGlyphRows(uint8_t offset);
GlyphRows WriteGlyphRowsTwice(uint8_t offset);
void abi_PadRepeatReset(void);
void abi_MenuBackOnButtonII(void);
void abi_MenuBackToPageEight(void);
void abi_IncrementTempWord(void);
void abi_WriteGlyphRows(void);
void abi_WriteGlyphRowsTwice(void);
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
uint8_t AttractBegin(void);
uint8_t ModeDoneAdd(void);
void abi_AttractBegin(void);
void abi_ModeDoneAdd(void);
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
