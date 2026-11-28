/* Lot L1 (the front end: boot, logo, title, attract, file and name menus, game over, demo scenes).
 * Declared here, not in routines.h. */
#ifndef RONDO_FRONT_END_H
#define RONDO_FRONT_END_H

#include <stdbool.h>
#include <stdint.h>
#include "machine.h"
#include "wait_regs.h"

/* Defined elsewhere, typed, and not declared in routines.h. */
void ClearObject(uint8_t object);                    /* object/lifecycle.c */

/* What a routine leaves in the registers and flags, for its abi_ wrapper: the registers it sets, the
 * value N and Z come from, C, V when it sets V, and a flag for each register it leaves as it was. */
typedef struct {
    uint8_t a, x, y, nz;
    bool c, v, v_set, a_kept, x_kept, y_kept, c_kept, from_cpu;
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
    if (!e.c_kept) {
        abi_flag(P_C, e.c);
    }
    if (e.v_set) {
        abi_flag(P_V, e.v);
    }
}

/* What an arithmetic routine leaves: A, the value N and Z come from, C, and V when it sets one. */
typedef struct {
    uint8_t a, nz;
    bool carry, overflow, overflow_set;
} ArithExit;

static inline void arith_exit_shim(ArithExit e)
{
    cpu.a = e.a;
    abi_nz(e.nz);
    abi_flag(P_C, e.carry);
    if (e.overflow_set) {
        abi_flag(P_V, e.overflow);
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

/* What a cursor glide leaves, and the move whose V it ends with. */
typedef struct { Exit exit; AxisMove move; bool moved; } GlideExit;
/* What CursorMoveByPad leaves; moved says X (1 or the slot's offset) was set. */
typedef struct { Exit exit; uint8_t x; bool moved; } PadMoveExit;
#define CURSOR_FRAMES 0x9D55
#define CURSOR_FIRST_ITEMS 0x9CBE
#define CURSOR_LAST_ITEMS 0x9CC4
#define MENU_BLOCKED 0x7DDB

/* menu/file_select.c */
GlideExit GlideCursorObject(uint8_t object);
GlideExit GlideCursorObjects(void);
GlideExit JitterCursorObject(uint8_t object);
GlideExit JitterCursorObjects(void);
PadMoveExit CursorMoveByPad(uint8_t menu);
uint8_t StartScriptSlotKind6B(uint8_t script, uint8_t slot);
uint8_t StartScriptSlotKind7B(uint8_t script, uint8_t slot);
void abi_GlideCursorObject(void);
void abi_GlideCursorObjects(void);
void abi_JitterCursorObject(void);
void abi_JitterCursorObjects(void);
void abi_CursorMoveByPad(void);
void abi_StartScriptSlotKind6B(void);
void abi_StartScriptSlotKind7B(void);
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
ArithExit DrawSelectedFileDigit(void);
ArithExit DrawCursorRowDigit(void);
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
ArithExit HalveObjectVelocity(uint8_t object);
CursorTarget SetCursorTarget(uint8_t object);
CursorTarget SetCursorTargets(void);
CursorTarget SetCursorValue(uint8_t value);
CursorTarget PlaceCursorObject(uint8_t value, uint8_t object);
CursorTarget PlaceCursorObjects(uint8_t value);
ArithExit ShiftObjectsByScrollChange(void);
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

/* Defined elsewhere, typed, and not declared in routines.h. */
void ClearScreenState(WaitRegs entry);   /* system/video.c: entry the registers the caller holds at the JSR */                          /* system/video.c */

/* Placeholders for the scene script (menu/attract.c): its tables of command lengths and handlers
 * and the title's "save read" byte. */
#define SCRIPT_LENGTHS 0x87F0
#define SCRIPT_COMMANDS 0x8827
#define TITLE_SAVE_READ 0x3F83

/* Block moves with the registers the CPU pushes: TAI and TIA push Y, A and X and pull them back. */
static inline void block_move_tai(uint16_t src, uint16_t dst, uint16_t len, uint8_t a, uint8_t x, uint8_t y)
{
    push8(y);
    push8(a);
    push8(x);
    for (uint16_t i = 0; i < len; i++) {
        uint8_t byte = rd8((uint16_t)(src + (i & 1)));

        wr8((uint16_t)(dst + i), byte);
    }
    (void)pull8();
    (void)pull8();
    (void)pull8();
}

static inline void block_move_tia(uint16_t src, uint16_t dst, uint16_t len, uint8_t a, uint8_t x, uint8_t y)
{
    push8(y);
    push8(a);
    push8(x);
    for (uint16_t i = 0; i < len; i++) {
        uint8_t byte = rd8((uint16_t)(src + i));

        wr8((uint16_t)(dst + (i & 1)), byte);
    }
    (void)pull8();
    (void)pull8();
    (void)pull8();
}

/* menu/title.c */
Exit TitlePaletteRotate(uint8_t which, uint8_t kept_y);
Exit TitleTileWrite(void);
Exit TitleTileRows(void);
Exit TitleScrollIn(void);
Exit TitleBlink(uint8_t kept_y);
Exit TitleWipe(void);
Exit TitleLogoFade(uint8_t kept_y);
Exit TitleCycle(void);
void abi_TitlePaletteRotate(void);
void abi_TitleTileWrite(void);
void abi_TitleTileRows(void);
void abi_TitleScrollIn(void);
void abi_TitleBlink(void);
void abi_TitleWipe(void);
void abi_TitleLogoFade(void);
void abi_TitleCycle(void);

/* What a scene object handler leaves: the registers, and the move whose C and V it may end with. */
typedef struct { Exit exit; AxisMove move; bool moved; } ObjExit;
#define FRAME_LISTS 0x8EED
#define DEBRIS_TABLE 0x8BFE

/* menu/attract.c */
ObjExit ObjectFrameList(uint8_t list, uint8_t object);
ObjExit ObjectFrameListAt(uint8_t list, uint8_t object, uint16_t table);   /* with the list table's address, for the copies in other overlays */
ObjExit PlayFrameListOnce(uint8_t object);
ObjExit SlideForCount(uint8_t object);
ObjExit AnimateAndCueSeven(uint8_t object);
ObjExit AnimateList1AndCueSeven(uint8_t object);
ObjExit AnimateAndCueNine(uint8_t object);
ObjExit AnimateList9AndCueNine(uint8_t object);
ObjExit BlinkScrollFlag(uint8_t object);
ObjExit MirrorVramBlock(uint8_t object);
ObjExit NudgeScrollIn(uint8_t object);
void abi_AnimateAndCueNine(void);
void abi_AnimateList9AndCueNine(void);
void abi_BlinkScrollFlag(void);
void abi_MirrorVramBlock(void);
void abi_NudgeScrollIn(void);
ObjExit DropAndShakeScreen(uint8_t object);
ObjExit ScatterDebris(uint8_t kept_x);
void abi_DropAndShakeScreen(void);
void abi_ScatterDebris(void);
ObjExit BreakApartOnFrame(uint8_t object);
ObjExit FallAndClear(uint8_t object);
ObjExit IntroSceneObject(uint8_t object);
void abi_BreakApartOnFrame(void);
void abi_FallAndClear(void);
void abi_IntroSceneObject(void);
ObjExit RiseAndFallObject(uint8_t object);
ObjExit DriftAndClearObject(uint8_t object);
ObjExit SplitScreenObject(uint8_t object);
ObjExit FlingObjectRight(uint8_t object);
ObjExit FlingObjectLeft(uint8_t object);
void abi_RiseAndFallObject(void);
void abi_DriftAndClearObject(void);
void abi_SplitScreenObject(void);
void abi_FlingObjectRight(void);
void abi_FlingObjectLeft(void);
void abi_ObjectFrameList(void);
void abi_PlayFrameListOnce(void);
void abi_SlideForCount(void);
void abi_AnimateAndCueSeven(void);
void abi_AnimateList1AndCueSeven(void);
Exit ScriptAdvance(void);
Exit ScriptStep(void);
Exit ScriptSaveReadAndState(void);
Exit ScriptSound(uint8_t doubled_command);
Exit ScriptStartListAndClear(void);
Exit ScriptStartList(void);
Exit ScriptStartSlotKind1(void);
Exit ScriptStartSlotKind2(void);
Exit ScriptStartSlotKind3(void);
Exit ScriptPlayAdpcm(void);
Exit ScriptPlayTrack(uint8_t doubled_command);
Exit ScriptPlaceObject(uint8_t object, uint8_t y);
Exit ScriptPlaceObjectState(void);
Exit ScriptPlaceObjectKind2(void);
Exit ScriptPlaceObjectFrame(void);
Exit ScriptPlaceObjectTimed(void);
Exit ScriptPlaceTitleObjectA(void);
Exit ScriptPlaceTitleObjectB(void);
Exit AttractHoldBeforeEnd(void);
void abi_ScriptAdvance(void);
void abi_ScriptStep(void);
void abi_ScriptSaveReadAndState(void);
void abi_ScriptSound(void);
void abi_ScriptStartListAndClear(void);
void abi_ScriptStartList(void);
void abi_ScriptStartSlotKind1(void);
void abi_ScriptStartSlotKind2(void);
void abi_ScriptStartSlotKind3(void);
void abi_ScriptPlayAdpcm(void);
void abi_ScriptPlayTrack(void);
void abi_ScriptPlaceObject(void);
void abi_ScriptPlaceObjectState(void);
void abi_ScriptPlaceObjectKind2(void);
void abi_ScriptPlaceObjectFrame(void);
void abi_ScriptPlaceObjectTimed(void);
void abi_ScriptPlaceTitleObjectA(void);
void abi_ScriptPlaceTitleObjectB(void);
void abi_AttractHoldBeforeEnd(void);
uint8_t AttractBegin(uint8_t kept_x, uint8_t kept_y);
Exit AttractRestart(uint8_t kept_x, uint8_t kept_y);
uint8_t ModeDoneAdd(void);
void abi_AttractBegin(void);
void abi_AttractRestart(void);
void abi_ModeDoneAdd(void);
uint8_t AttractFrameTick(void);
/* What AttractEndCountdown ends on: the byte its flags come from, and whether it reset the game state. */
typedef struct {
    uint8_t value;
    bool reset;
} AttractEnd;
AttractEnd AttractEndCountdown(uint8_t kept_x, uint8_t kept_y);
void ClearAllObjects(void);

void abi_AttractFrameTick(void);
void abi_AttractEndCountdown(void);
void abi_ClearAllObjects(void);


/* The title overlay's sub-mode handlers (menu/title_steps.c). A handler that ends in a sound command
 * returns the call with what comes after it. */
typedef struct { CommandCall call; uint8_t done; } TitleSound;
typedef struct { Exit exit; CommandCall call; bool sounded; uint8_t x, y; } AttractStart;
Exit TitleSetUp(uint8_t kept_x, uint8_t kept_y);
Exit TitleScreenBegin(uint8_t kept_x, uint8_t kept_y);
TitleSound TitleSoundBegin(uint8_t kept_x, uint8_t kept_y);
/* The stage select: a pick (the sound given and the mode done) or the pad's move. */
typedef struct { Exit exit; TitleSound picked; bool started; uint8_t sound, x, y; } StageSelect;
StageSelect StageSelectStep(uint8_t kept_x, uint8_t kept_y);
void stage_select_exit_shim(StageSelect r);
Exit StageSelectMove(uint8_t kept_x, uint8_t kept_y);
Exit StageSelectToggleCharacter(uint8_t kept_x, uint8_t kept_y);
Exit StageSelectStage(uint8_t pad, uint8_t kept_x, uint8_t kept_y);
void abi_StageSelectStep(void);
void abi_StageSelectMove(void);
void abi_StageSelectToggleCharacter(void);
void abi_StageSelectStage(void);
void title_sound_exit_shim(TitleSound sound);
Exit TitleStep(uint8_t kept_y);
Exit TitleEntry(uint8_t kept_y);
Exit TitleScreenStep(uint8_t kept_y);
Exit AttractWaitStart(void);
AttractStart AttractStartCheck(uint8_t index, uint8_t kept_y);
void attract_start_exit_shim(AttractStart start);
Exit AttractBlink(void);
Exit AttractStep(uint8_t kept_y);
void abi_TitleSetUp(void);
void abi_TitleScreenBegin(void);
void abi_TitleSoundBegin(void);
void abi_TitleStep(void);
void abi_TitleEntry(void);
void abi_TitleScreenStep(void);
void abi_AttractWaitStart(void);
void abi_AttractStartCheck(void);
void abi_AttractBlink(void);
void abi_AttractStep(void);


/* The menus' glyph text (menu/glyph_text.c). */
Exit WriteGlyphMapRows(void);
Exit DrawGlyphPair(void);
Exit DrawGlyphText(void);
void abi_WriteGlyphMapRows(void);
void abi_DrawGlyphPair(void);
void abi_DrawGlyphText(void);


/* The ending scene (menu/ending.c). A handler converted to typed C returns what it ended on, not registers: the arm, the values it read or
 * made, a flag, the move it made, and an Exit a callee returned; its exit shim builds the registers and flags from them. */
typedef struct {
    int arm;
    uint8_t first, second, third, index;
    bool flag;
    AxisMove move;
    Exit exit;
} EndingArm;

uint8_t EndingFrameTick(void);
uint8_t EndingModeDoneBack(void);
void EndingNothing(void);
uint8_t EndingHoldTimer(uint8_t object);
Exit EndingScrollBack8(void);
Exit EndingScrollSlots(void);
ObjExit EndingFrameList(uint8_t list, uint8_t object);
ObjExit EndingClearAfterScrollX(uint8_t object);
ObjExit EndingClearAfterScrollY(uint8_t object);
ObjExit EndingScrollRunUp(uint8_t object);
ObjExit EndingScrollPull(uint8_t object);
ObjExit EndingPlayList10(uint8_t object);
ObjExit EndingPlayList0F(uint8_t object);
ObjExit EndingPlayList0EThenClear(uint8_t object);
ObjExit EndingRiseToY40(uint8_t object);
ObjExit EndingScrollWindowA(uint8_t object);
ObjExit EndingScrollWindowB(uint8_t object);
ObjExit EndingScrollWindowHold(uint8_t object);
ObjExit EndingScrollWindowSteps(uint8_t object);
ObjExit EndingStartList3(uint8_t object, uint8_t entry_a);
ObjExit EndingStartScript18(uint8_t object, uint8_t entry_a);
ObjExit EndingScrollSlide(uint8_t object);
ObjExit EndingSplitScrolls(uint8_t object, uint8_t entry_a);
ObjExit EndingScrollsOn(uint8_t object, uint8_t entry_a);
ObjExit EndingRiseFrame31(uint8_t object);
ObjExit EndingRiseFrame32A(uint8_t object);
ObjExit EndingRiseFrame32B(uint8_t object);
ObjExit EndingRiseWithList(uint8_t object);
ObjExit EndingScrollRight6(uint8_t object);
ObjExit EndingScrollUp(uint8_t object);
ObjExit EndingScrollDown(uint8_t object);
EndingArm EndingScrollSteps(uint8_t object);
ObjExit EndingHoldThenScript19(uint8_t object);
ObjExit EndingLinkAnimationA(uint8_t object);
ObjExit EndingLinkAnimationB(uint8_t object);
ObjExit EndingAimAtPlace(uint8_t object);
EndingArm EndingScrollSqueeze(uint8_t object);
ObjExit EndingWaitForList(uint8_t object);
ObjExit EndingFlyOff(uint8_t object);
EndingArm EndingDropAndBounce(uint8_t object);
EndingArm EndingCreditScrollStart(uint8_t object);
EndingArm EndingFallingPiece(uint8_t object);
ObjExit EndingPieceSpawn(uint8_t object);
void EndingClearObjects(void);
Exit EndingScriptAdvance(void);
Exit EndingScriptStep(void);
Exit EndingBegin(uint8_t kept_x, uint8_t kept_y);
Exit EndingBegin1E61(uint8_t kept_x, uint8_t kept_y);
ObjExit EndingDispatchKind(uint8_t kind, uint8_t object);
Exit EndingObjectsStep(void);
Exit EndingFrame(void);
Exit EndingStep(uint8_t kept_y);
Exit EndingEffectList(uint8_t slot);
Exit EndingEffectFade(uint8_t slot);
Exit EndingEffectDispatch(uint8_t type, uint8_t slot);
Exit EndingEffectStep(void);
Exit EndingEffectEntry(void);
Exit EndingPlaceObject(uint8_t object, uint8_t y);
Exit EndingCommandStartList(void);
Exit EndingCommandStartScript1(void);
Exit EndingCommandStartScript2(void);
Exit EndingCommandPlaceKind(void);
Exit EndingCommandPlaceNext(void);
Exit EndingCommandPlaceFrame(void);
Exit EndingCommandPlaceTimed(void);
Exit EndingCommandLoadBanks(void);
Exit EndingCommandCdPlay(uint8_t index);


/* The scene objects' loop and the overlay's entry stubs (menu/attract_objects.c). */
ObjExit DispatchSceneObject(uint8_t kind, uint8_t object);
Exit StepSceneObjects(void);
Exit TitleScreenEntry(uint8_t kept_y);
Exit AttractEntry(uint8_t kept_y);
Exit SceneObjectsEntry(void);
void abi_DispatchSceneObject(void);
void abi_StepSceneObjects(void);
void abi_TitleScreenEntry(void);
void abi_AttractEntry(void);
void abi_SceneObjectsEntry(void);

void abi_EndingCommandSubModeTwo(void);
void abi_EndingCommandStartListOnly(void);
void abi_EndingCommandStartScript3(void);
void abi_EndingCommandSkip(void);
void abi_EndingCommandPlaceTitleObjectA(void);
void abi_EndingCommandPlaceTitleObjectB(void);
void abi_EndingScrollByState(void);
Exit WaitThenStartGlide(uint8_t object);
void abi_WaitThenStartGlide(void);
Exit SetVramAddressXYAt(uint8_t lo, uint8_t hi, uint16_t select_return);

Exit ClearVramRowsAt(uint8_t rows, uint8_t lo, uint8_t hi, uint16_t address_return, uint16_t select_return);

#endif
