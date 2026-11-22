/*
 * TEMPORARY: the stage 05 overlay's object routines whose enemy kind is not yet identified, kept
 * together here until each kind has a file of its own.
 * The disc tables the routines read are named here by their overlay address until the typed data
 * layer reads them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"

/*
 * @routine 0938:7F2 SelectRasterStage1
 * @run 6F:BFF2
 * WHAT IT DOES: Sets the stage value the raster effect tests to 1.
 * HOW: One store into zRasterStage.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, N and Z clear; X, Y, C and V kept.
 */
Ended SelectRasterStage1(void)
{
    WR8(ZP->raster_stage, 1);
    return ended_load(1);
}

void abi_SelectRasterStage1(void)
{
    abi_ended(SelectRasterStage1());
}

/*
 * @routine 093C:10A SetHitboxDxFromFrame
 * @run 70:B90A
 * WHAT IT DOES: Sets the object's f3250 to 2 and its hit rectangle's X offset from a table by
 * its frame, for frames 1 to 6; any other frame clears both.
 * HOW: The table is the byte list at overlay address $B928, indexed by frame minus one. Frame 0
 * is not in range of the first test's subtract and reads the table at index $FF.
 * WAITS: no.
 * Entry: X object. Exit: A the table byte, or the frame; N, Z, C from the last compare or
 * subtract; Y the table index when read; X kept.
 */
Ended SetHitboxDxFromFrame(uint8_t slot)
{
    enum { TABLE = 0xB928 };
    uint8_t frame, index, value;

    WR8(OBJ->f3250[slot], 2);
    frame = RD8(OBJ->frame[slot]);
    if (frame >= 7) {
        WR8(OBJ->f3250[slot], 0);
        WR8(OBJ->hitbox_dx[slot], 0);
        return ended_compare(frame, 7);
    }
    index = (uint8_t)(frame - 1);
    value = rd8((uint16_t)(TABLE + index));
    WR8(OBJ->hitbox_dx[slot], value);
    return ended_with_overflow(ended_with_y(ended_with_carry(ended_load(value), frame >= 1), index),
                               sub_overflow(frame, 1, index));
}

void abi_SetHitboxDxFromFrame(void)
{
    abi_ended(SetHitboxDxFromFrame(cpu.x));
}

/*
 * @routine 093C:374 PhaseTimerToState1
 * @run 70:BB74
 * WHAT IT DOES: Counts the object's f3720 down. While it runs, the object's animation is held
 * at its start and its step byte is 1; at zero the object enters state 1 with frame 1, or
 * frame 4 when its f36c8 is set.
 * HOW: One decrement; each branch then stores its constants.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the last value loaded; X, Y, C and V kept.
 */
Ended PhaseTimerToState1(uint8_t slot)
{
    uint8_t left = RD8(OBJ->f3720[slot]);
    uint8_t flag;

    left = (uint8_t)(left - 1);
    WR8(OBJ->f3720[slot], left);
    if (left != 0) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->f2800[slot], 1);
        return ended_load(1);
    }
    WR8(OBJ->state[slot], 1);
    WR8(OBJ->frame[slot], 1);
    flag = RD8(OBJ->f36c8[slot]);
    if (flag == 0) {
        return ended_load(0);
    }
    WR8(OBJ->frame[slot], 4);
    return ended_load(4);
}

void abi_PhaseTimerToState1(void)
{
    abi_ended(PhaseTimerToState1(cpu.x));
}

/*
 * @routine 093C:586 SetHitboxFromFrameRows
 * @run 70:BD86
 * WHAT IT DOES: Sets the object's whole hit rectangle (X offset, Y offset, half width, half
 * height) from a four byte row of a table, the row chosen by the frame minus $13.
 * HOW: The table is at overlay address $BDA8.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte loaded; N, Z from it; C bit 6 of the frame minus $13
 * (the last shift); Y the row's offset; X kept.
 */
Ended SetHitboxFromFrameRows(uint8_t slot)
{
    enum { TABLE = 0xBDA8 };
    uint8_t frame = RD8(OBJ->frame[slot]);
    uint8_t row = (uint8_t)(frame - 0x13);
    uint8_t y = (uint8_t)(row << 2);
    uint8_t value;

    value = rd8((uint16_t)(TABLE + y));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(TABLE + 1 + y));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(TABLE + 2 + y));
    WR8(OBJ->hitbox_half_w[slot], value);
    value = rd8((uint16_t)(TABLE + 3 + y));
    WR8(OBJ->hitbox_half_h[slot], value);
    return ended_with_overflow(ended_with_y(ended_with_carry(ended_load(value), (row & 0x40) != 0), y),
                               sub_overflow(frame, 0x13, row));
}

void abi_SetHitboxFromFrameRows(void)
{
    abi_ended(SetHitboxFromFrameRows(cpu.x));
}

/*
 * @routine 093D:2B7 EnterState7OnFlagOrYSignChange
 * @run 71:C2B7
 * WHAT IT DOES: Sends the object to state 7 when bit 7 of its status flags is set, or when its
 * Y position's high byte is not zero and the sign of the next byte (the middle byte) is the
 * opposite of the high byte's.
 * HOW: Tests the flag, then the high byte, then the middle byte against it. The arms past a
 * non-zero high byte ran in no script: the draft stops there and liftcmp judges only the flag
 * and zero tests; the rest waits for coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the last load; X, Y, C and V kept.
 */
Ended EnterState7OnFlagOrYSignChange(uint8_t slot)
{
    uint8_t flags = RD8(OBJ->status_flags[slot]);
    uint8_t high, middle;

    if ((flags & 0x80) == 0) {
        high = RD8(OBJ->y_hi[slot]);
        if (high == 0) {
            return ended_load(0);
        }
        middle = RD8(OBJ->y_lo[slot]);
        if ((high & 0x80) != 0) {
            if ((middle & 0x80) != 0) {
                return ended_load(middle);
            }
        } else if ((middle & 0x80) == 0) {
            return ended_load(middle);
        }
    }
    WR8(OBJ->state[slot], 7);
    return ended_load(7);
}

void abi_EnterState7OnFlagOrYSignChange(void)
{
    abi_ended(EnterState7OnFlagOrYSignChange(cpu.x));
}

/*
 * @routine 093D:2D7 SetHitboxFromFrameRowsIndirect
 * @run 71:C2D7
 * WHAT IT DOES: Sets the object's whole hit rectangle from a four byte row of a table, the row
 * named by a second table indexed by the frame minus $27.
 * HOW: The index table is at overlay address $C6E6, the rows at $C6FF.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte loaded; N, Z from it; C bit 6 of the row number; Y the
 * row's offset; X kept.
 */
Ended SetHitboxFromFrameRowsIndirect(uint8_t slot)
{
    enum { ROW_NUMBERS = 0xC6E6, ROWS = 0xC6FF };
    uint8_t frame = RD8(OBJ->frame[slot]);
    uint8_t first = (uint8_t)(frame - 0x27);
    uint8_t row = rd8((uint16_t)(ROW_NUMBERS + first));
    uint8_t y = (uint8_t)(row << 2);
    uint8_t value;

    value = rd8((uint16_t)(ROWS + y));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(ROWS + 1 + y));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(ROWS + 2 + y));
    WR8(OBJ->hitbox_half_w[slot], value);
    value = rd8((uint16_t)(ROWS + 3 + y));
    WR8(OBJ->hitbox_half_h[slot], value);
    return ended_with_overflow(ended_with_y(ended_with_carry(ended_load(value), (row & 0x40) != 0), y),
                               sub_overflow(frame, 0x27, first));
}

void abi_SetHitboxFromFrameRowsIndirect(void)
{
    abi_ended(SetHitboxFromFrameRowsIndirect(cpu.x));
}

/*
 * @routine 093D:43A LaunchHopByF3510
 * @run 71:C43A
 * WHAT IT DOES: Launches the object in a hop: X speed from a two byte table by the object's f3510,
 * Y speed -3.00 (up), frame $36, f35c0 set to 8, and its step byte advanced.
 * HOW: The table is at overlay address $C45C, indexed by f3510 (0 or 1).
 * WAITS: no.
 * Entry: X object. Exit: A 8; N, Z from the step byte's new value; Y the f3510 byte; X and C
 * kept.
 */
Ended LaunchHopByF3510(uint8_t slot)
{
    enum { TABLE = 0xC45C };
    uint8_t index = RD8(OBJ->f3510[slot]);

    return hop_from_index(slot, index, TABLE, 0x36);
}

void abi_LaunchHopByF3510(void)
{
    abi_ended(LaunchHopByF3510(cpu.x));
}

/*
 * @routine 093D:5C3 CountdownStopThenState3
 * @run 71:C5C3
 * WHAT IT DOES: Counts the object's f35c0 timer down and, when it reaches zero, stops its
 * animation and sends it to state 3.
 * HOW: One decrement of the timer; the rest only at zero.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the timer, or A 3 at zero; X, Y and C kept.
 */
Ended CountdownStopThenState3(uint8_t slot)
{
    return countdown_stop_then_state(slot, COL(OBJ->f35c0), 3);
}

void abi_CountdownStopThenState3(void)
{
    abi_ended(CountdownStopThenState3(cpu.x));
}
