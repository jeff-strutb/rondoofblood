/* The stage overlay's object set-up: the enemies cleared, the stream placed, the frame started. */
#include "machine.h"
#include "ops6280.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "enemy_objects.h"
#include "stage_map.h"

enum {
    MPR5 = 0x20, MPR6 = 0x40, OBJECT_BANK_OFFSET = 8, EVENT_FLAG_BYTES = 32, SECTION_RECORD_SHIFT = 2,
    CLEAR_RETURN = 0x8A62, WALK_JUMP = 0x8AA1, STEP_OBJECTS_ENTRY = 0x8A4F, MAP_RETURN = 0x8A65, WALK_RETURN = 0x8AA1
};

ObjArm MapOverlayBank7(void);
void ClearEnemySlots(uint8_t entry_x);
Sum StreamFullWalk(uint8_t x);
void lines_exit_shim(Sum count);   /* object/stream.c */


/*
 * Clears the enemy slots, maps bank 7, copies the stage section's four-byte record into the
 * level state (the stream's position and the axis), clears the event flags when the record's
 * fourth byte is set, maps the object banks (the second pair at the offset the overlay keeps),
 * walks the whole stream, and finishes with the frame set-up's end.
 */
SetUpEnd SetUpObjectsBody(uint8_t x, const SetUpLayout *layout)
{
    uint8_t y;
    uint8_t value;
    uint8_t clear_flags;
    Sum sum;
    SetUpEnd end;

    jsr_push(layout->clear_return);
    ClearEnemySlots(x);
    jsr_pop();
    jsr_push(layout->map_return);
    MapOverlayBank7();
    jsr_pop();
    y = (uint8_t)(RD8(ZP->field83) << SECTION_RECORD_SHIFT);
    value = rd8((uint16_t)(layout->record_table + y));
    WR8(LEVEL->level_stream.lo, value);
    value = rd8((uint16_t)(layout->record_table + 1 + y));
    WR8(LEVEL->level_stream.hi, value);
    value = rd8((uint16_t)(layout->record_table + 2 + y));
    WR8(LEVEL->level_axis, value);
    clear_flags = rd8((uint16_t)(layout->record_table + 3 + y));
    if (clear_flags != 0) {
        for (x = EVENT_FLAG_BYTES - 1; (int8_t)x >= 0; x--) {
            WR8(LEVEL->event_flags[x], 0);
        }
    }
    sum = add_bytes(OBJECT_BANK_OFFSET, RD8(ZP->ram_base_bank), false);
    tam(MPR5, sum.value);
    tam(MPR6, (uint8_t)(sum.value + 1));
    value = rd8(layout->second_offset_at);
    sum = add_bytes(value, RD8(ZP->ram_base_bank), false);
    tam(MPR5, sum.value);
    tam(MPR6, (uint8_t)(sum.value + 1));
    jsr_push(layout->walk_return);
    end.count = StreamFullWalk(x);
    jsr_pop();
    end.x = x;      /* the tail JMP $8A4F is made by the wrapper, the span's owner */
    return end;
}

/* 024E's layout: the return addresses of the three calls, from the overlay's record table and second bank offset. */
static SetUpEnd set_up_stage_objects(uint16_t record_table, uint16_t second_offset_at, uint8_t x)
{
    SetUpLayout layout = { record_table, second_offset_at, CLEAR_RETURN, MAP_RETURN, WALK_RETURN };

    return SetUpObjectsBody(x, &layout);
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
SetUpEnd SetUpStageObjects(uint8_t x)
{
    return set_up_stage_objects(0x9BD2, 0xAC95, x);
}

void abi_SetUpStageObjects(void)
{
    SetUpEnd end = SetUpStageObjects(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsB(uint8_t x)
{
    return set_up_stage_objects(0x9D01, 0xACCA, x);
}

void abi_SetUpStageObjectsB(void)
{
    SetUpEnd end = SetUpStageObjectsB(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsC(uint8_t x)
{
    return set_up_stage_objects(0x9EBD, 0xAC0E, x);
}

void abi_SetUpStageObjectsC(void)
{
    SetUpEnd end = SetUpStageObjectsC(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsD(uint8_t x)
{
    return set_up_stage_objects(0x9F90, 0xAC16, x);
}

void abi_SetUpStageObjectsD(void)
{
    SetUpEnd end = SetUpStageObjectsD(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsE(uint8_t x)
{
    return set_up_stage_objects(0x9E19, 0xABDC, x);
}

void abi_SetUpStageObjectsE(void)
{
    SetUpEnd end = SetUpStageObjectsE(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsF(uint8_t x)
{
    return set_up_stage_objects(0x9E90, 0xAB03, x);
}

void abi_SetUpStageObjectsF(void)
{
    SetUpEnd end = SetUpStageObjectsF(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsG(uint8_t x)
{
    return set_up_stage_objects(0x9E00, 0xAC8E, x);
}

void abi_SetUpStageObjectsG(void)
{
    SetUpEnd end = SetUpStageObjectsG(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsH(uint8_t x)
{
    return set_up_stage_objects(0x9ED6, 0xAC5E, x);
}

void abi_SetUpStageObjectsH(void)
{
    SetUpEnd end = SetUpStageObjectsH(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsI(uint8_t x)
{
    return set_up_stage_objects(0x9E30, 0xACE3, x);
}

void abi_SetUpStageObjectsI(void)
{
    SetUpEnd end = SetUpStageObjectsI(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsJ(uint8_t x)
{
    return set_up_stage_objects(0x9EA1, 0xAB27, x);
}

void abi_SetUpStageObjectsJ(void)
{
    SetUpEnd end = SetUpStageObjectsJ(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsK(uint8_t x)
{
    return set_up_stage_objects(0x9E05, 0xACA7, x);
}

void abi_SetUpStageObjectsK(void)
{
    SetUpEnd end = SetUpStageObjectsK(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsL(uint8_t x)
{
    return set_up_stage_objects(0x9EA4, 0xAC8B, x);
}

void abi_SetUpStageObjectsL(void)
{
    SetUpEnd end = SetUpStageObjectsL(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsM(uint8_t x)
{
    return set_up_stage_objects(0x9EA1, 0xAD91, x);
}

void abi_SetUpStageObjectsM(void)
{
    SetUpEnd end = SetUpStageObjectsM(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsN(uint8_t x)
{
    return set_up_stage_objects(0x9ED3, 0xAB70, x);
}

void abi_SetUpStageObjectsN(void)
{
    SetUpEnd end = SetUpStageObjectsN(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsO(uint8_t x)
{
    return set_up_stage_objects(0x9F36, 0xAD3F, x);
}

void abi_SetUpStageObjectsO(void)
{
    SetUpEnd end = SetUpStageObjectsO(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsP(uint8_t x)
{
    return set_up_stage_objects(0x9F47, 0xABA7, x);
}

void abi_SetUpStageObjectsP(void)
{
    SetUpEnd end = SetUpStageObjectsP(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsQ(uint8_t x)
{
    return set_up_stage_objects(0x9D8A, 0xAD1B, x);
}

void abi_SetUpStageObjectsQ(void)
{
    SetUpEnd end = SetUpStageObjectsQ(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsR(uint8_t x)
{
    return set_up_stage_objects(0x9F4A, 0xACC9, x);
}

void abi_SetUpStageObjectsR(void)
{
    SetUpEnd end = SetUpStageObjectsR(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsS(uint8_t x)
{
    return set_up_stage_objects(0x9D54, 0xAD36, x);
}

void abi_SetUpStageObjectsS(void)
{
    SetUpEnd end = SetUpStageObjectsS(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsT(uint8_t x)
{
    return set_up_stage_objects(0x9F7E, 0xACF7, x);
}

void abi_SetUpStageObjectsT(void)
{
    SetUpEnd end = SetUpStageObjectsT(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsU(uint8_t x)
{
    return set_up_stage_objects(0x9E26, 0xAD31, x);
}

void abi_SetUpStageObjectsU(void)
{
    SetUpEnd end = SetUpStageObjectsU(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsV(uint8_t x)
{
    return set_up_stage_objects(0x9F6D, 0xAC65, x);
}

void abi_SetUpStageObjectsV(void)
{
    SetUpEnd end = SetUpStageObjectsV(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
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
SetUpEnd SetUpStageObjectsW(uint8_t x)
{
    return set_up_stage_objects(0x9F31, 0xAC49, x);
}

void abi_SetUpStageObjectsW(void)
{
    SetUpEnd end = SetUpStageObjectsW(cpu.x);

    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    call_original_at(WALK_JUMP, STEP_OBJECTS_ENTRY);   /* the tail JMP */
}
