/* The stage overlay's object set-up: the enemies cleared, the stream placed, the frame started. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "enemy_objects.h"
#include "stage_map.h"

enum {
    MPR5 = 0x20, MPR6 = 0x40, OBJECT_BANK_OFFSET = 8, EVENT_FLAG_BYTES = 32, SECTION_RECORD_SHIFT = 2,
    CLEAR_RETURN = 0x8A62, MAP_RETURN = 0x8A65, WALK_RETURN = 0x8AA1
};

Ended MapOverlayBank7(void);
void ClearEnemySlots(uint8_t entry_x);
void StreamFullWalk(uint8_t x);

/*
 * Clears the enemy slots, maps bank 7, copies the stage section's four-byte record into the
 * level state (the stream's position and the axis), clears the event flags when the record's
 * fourth byte is set, maps the object banks (the second pair at the offset the overlay keeps),
 * walks the whole stream, and finishes with the frame set-up's end.
 */
static void set_up_stage_objects(uint16_t record_table, uint16_t second_offset_at, uint8_t x)
{
    uint8_t y;
    uint8_t value;
    uint8_t clear_flags;
    Sum sum;

    jsr_push(CLEAR_RETURN);
    ClearEnemySlots(x);
    jsr_pop();
    jsr_push(MAP_RETURN);
    MapOverlayBank7();
    jsr_pop();
    y = (uint8_t)(RD8(ZP->field83) << SECTION_RECORD_SHIFT);
    value = rd8((uint16_t)(record_table + y));
    WR8(LEVEL->level_stream.lo, value);
    value = rd8((uint16_t)(record_table + 1 + y));
    WR8(LEVEL->level_stream.hi, value);
    value = rd8((uint16_t)(record_table + 2 + y));
    WR8(LEVEL->level_axis, value);
    clear_flags = rd8((uint16_t)(record_table + 3 + y));
    if (clear_flags != 0) {
        for (x = EVENT_FLAG_BYTES - 1; (int8_t)x >= 0; x--) {
            WR8(LEVEL->event_flags[x], 0);
        }
    }
    sum = add_bytes(OBJECT_BANK_OFFSET, RD8(ZP->ram_base_bank), false);
    tam(MPR5, sum.value);
    tam(MPR6, (uint8_t)(sum.value + 1));
    value = rd8(second_offset_at);
    sum = add_bytes(value, RD8(ZP->ram_base_bank), false);
    tam(MPR5, sum.value);
    tam(MPR6, (uint8_t)(sum.value + 1));
    jsr_push(WALK_RETURN);
    StreamFullWalk(x);
    jsr_pop();
    StepScrollAndRunObjects(x);
}

/*
 * @routine 024E:25F SetUpStageObjects
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjects(uint8_t x)
{
    set_up_stage_objects(0x9BD2, 0xAC95, x);
}

void abi_SetUpStageObjects(void)
{
    SetUpStageObjects(cpu.x);
}

/*
 * @routine 050E:25F SetUpStageObjectsB
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9D01 and its second bank offset is the byte at $ACCA.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsB(uint8_t x)
{
    set_up_stage_objects(0x9D01, 0xACCA, x);
}

void abi_SetUpStageObjectsB(void)
{
    SetUpStageObjectsB(cpu.x);
}

/*
 * @routine 05BE:25F SetUpStageObjectsC
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9EBD and its second bank offset is the byte at $AC0E.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsC(uint8_t x)
{
    set_up_stage_objects(0x9EBD, 0xAC0E, x);
}

void abi_SetUpStageObjectsC(void)
{
    SetUpStageObjectsC(cpu.x);
}

/*
 * @routine 071E:25F SetUpStageObjectsD
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9F90 and its second bank offset is the byte at $AC16.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsD(uint8_t x)
{
    set_up_stage_objects(0x9F90, 0xAC16, x);
}

void abi_SetUpStageObjectsD(void)
{
    SetUpStageObjectsD(cpu.x);
}

/*
 * @routine 07CE:25F SetUpStageObjectsE
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9E19 and its second bank offset is the byte at $ABDC.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsE(uint8_t x)
{
    set_up_stage_objects(0x9E19, 0xABDC, x);
}

void abi_SetUpStageObjectsE(void)
{
    SetUpStageObjectsE(cpu.x);
}

/*
 * @routine 087E:25F SetUpStageObjectsF
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9E90 and its second bank offset is the byte at $AB03.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsF(uint8_t x)
{
    set_up_stage_objects(0x9E90, 0xAB03, x);
}

void abi_SetUpStageObjectsF(void)
{
    SetUpStageObjectsF(cpu.x);
}

/*
 * @routine 092E:25F SetUpStageObjectsG
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9E00 and its second bank offset is the byte at $AC8E.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsG(uint8_t x)
{
    set_up_stage_objects(0x9E00, 0xAC8E, x);
}

void abi_SetUpStageObjectsG(void)
{
    SetUpStageObjectsG(cpu.x);
}

/*
 * @routine 09DE:25F SetUpStageObjectsH
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9ED6 and its second bank offset is the byte at $AC5E.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsH(uint8_t x)
{
    set_up_stage_objects(0x9ED6, 0xAC5E, x);
}

void abi_SetUpStageObjectsH(void)
{
    SetUpStageObjectsH(cpu.x);
}

/*
 * @routine 0A8E:25F SetUpStageObjectsI
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9E30 and its second bank offset is the byte at $ACE3.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsI(uint8_t x)
{
    set_up_stage_objects(0x9E30, 0xACE3, x);
}

void abi_SetUpStageObjectsI(void)
{
    SetUpStageObjectsI(cpu.x);
}

/*
 * @routine 0B3E:25F SetUpStageObjectsJ
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9EA1 and its second bank offset is the byte at $AB27.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsJ(uint8_t x)
{
    set_up_stage_objects(0x9EA1, 0xAB27, x);
}

void abi_SetUpStageObjectsJ(void)
{
    SetUpStageObjectsJ(cpu.x);
}

/*
 * @routine 0BEE:25F SetUpStageObjectsK
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9E05 and its second bank offset is the byte at $ACA7.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsK(uint8_t x)
{
    set_up_stage_objects(0x9E05, 0xACA7, x);
}

void abi_SetUpStageObjectsK(void)
{
    SetUpStageObjectsK(cpu.x);
}

/*
 * @routine 0C9E:25F SetUpStageObjectsL
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9EA4 and its second bank offset is the byte at $AC8B.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsL(uint8_t x)
{
    set_up_stage_objects(0x9EA4, 0xAC8B, x);
}

void abi_SetUpStageObjectsL(void)
{
    SetUpStageObjectsL(cpu.x);
}

/*
 * @routine 0D4E:25F SetUpStageObjectsM
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9EA1 and its second bank offset is the byte at $AD91.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsM(uint8_t x)
{
    set_up_stage_objects(0x9EA1, 0xAD91, x);
}

void abi_SetUpStageObjectsM(void)
{
    SetUpStageObjectsM(cpu.x);
}

/*
 * @routine 0DFE:25F SetUpStageObjectsN
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9ED3 and its second bank offset is the byte at $AB70.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsN(uint8_t x)
{
    set_up_stage_objects(0x9ED3, 0xAB70, x);
}

void abi_SetUpStageObjectsN(void)
{
    SetUpStageObjectsN(cpu.x);
}

/*
 * @routine 0EAE:25F SetUpStageObjectsO
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9F36 and its second bank offset is the byte at $AD3F.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsO(uint8_t x)
{
    set_up_stage_objects(0x9F36, 0xAD3F, x);
}

void abi_SetUpStageObjectsO(void)
{
    SetUpStageObjectsO(cpu.x);
}

/*
 * @routine 0F5E:25F SetUpStageObjectsP
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9F47 and its second bank offset is the byte at $ABA7.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsP(uint8_t x)
{
    set_up_stage_objects(0x9F47, 0xABA7, x);
}

void abi_SetUpStageObjectsP(void)
{
    SetUpStageObjectsP(cpu.x);
}

/*
 * @routine 100E:25F SetUpStageObjectsQ
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9D8A and its second bank offset is the byte at $AD1B.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsQ(uint8_t x)
{
    set_up_stage_objects(0x9D8A, 0xAD1B, x);
}

void abi_SetUpStageObjectsQ(void)
{
    SetUpStageObjectsQ(cpu.x);
}

/*
 * @routine 10BE:25F SetUpStageObjectsR
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9F4A and its second bank offset is the byte at $ACC9.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsR(uint8_t x)
{
    set_up_stage_objects(0x9F4A, 0xACC9, x);
}

void abi_SetUpStageObjectsR(void)
{
    SetUpStageObjectsR(cpu.x);
}

/*
 * @routine 116E:25F SetUpStageObjectsS
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9D54 and its second bank offset is the byte at $AD36.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsS(uint8_t x)
{
    set_up_stage_objects(0x9D54, 0xAD36, x);
}

void abi_SetUpStageObjectsS(void)
{
    SetUpStageObjectsS(cpu.x);
}

/*
 * @routine 12CE:25F SetUpStageObjectsT
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9F7E and its second bank offset is the byte at $ACF7.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsT(uint8_t x)
{
    set_up_stage_objects(0x9F7E, 0xACF7, x);
}

void abi_SetUpStageObjectsT(void)
{
    SetUpStageObjectsT(cpu.x);
}

/*
 * @routine 160E:25F SetUpStageObjectsU
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9E26 and its second bank offset is the byte at $AD31.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsU(uint8_t x)
{
    set_up_stage_objects(0x9E26, 0xAD31, x);
}

void abi_SetUpStageObjectsU(void)
{
    SetUpStageObjectsU(cpu.x);
}

/*
 * @routine 16BE:25F SetUpStageObjectsV
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9F6D and its second bank offset is the byte at $AC65.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsV(uint8_t x)
{
    set_up_stage_objects(0x9F6D, 0xAC65, x);
}

void abi_SetUpStageObjectsV(void)
{
    SetUpStageObjectsV(cpu.x);
}

/*
 * @routine 18CE:25F SetUpStageObjectsW
 * @run 6B:8A5F
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
 * the axis from the section's record, clears the event flags when the record says to, maps the
 * object banks, walks the whole stream once, and runs the first frame of objects.
 * HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
 * clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
 * then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
 * The same code as SetUpStageObjects; this overlay's section records are at $9F31 and its second bank offset is the byte at $AC49.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void SetUpStageObjectsW(uint8_t x)
{
    set_up_stage_objects(0x9F31, 0xAC49, x);
}

void abi_SetUpStageObjectsW(void)
{
    SetUpStageObjectsW(cpu.x);
}
