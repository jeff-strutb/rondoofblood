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

/*
 * @routine 093B:0F6 StartWithFrame1A
 * @run 70:B0F6
 * WHAT IT DOES: The first time the object runs (state 0), starts it from its record and gives it
 * frame $1A; later states leave it alone.
 * HOW: Tests the state, then LoadObjectRecord and one store.
 * WAITS: no.
 * Entry: X object. Exit: A the state, or $1A after the start; N, Z from it; C and Y from the
 * record load when it ran; X and V kept.
 */
Ended StartWithFrame1A(uint8_t slot)
{
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t nz, last;
    bool carry;
    Ended e;

    if (state != 0) {
        return ended_load(state);
    }
    jsr_push(0xB0FE);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->frame[slot], 0x1A);
    return ended_then(e, ended_load(0x1A));
}

void abi_StartWithFrame1A(void)
{
    abi_ended(StartWithFrame1A(cpu.x));
}

/*
 * @routine 093C:180 ThrowByNextKind
 * @run 70:B980
 * WHAT IT DOES: Moves the object one step along a velocity built from its f3040 byte (bit 0 picks
 * the X speed from a two byte table, the low nibble times $10, negated, is the Y speed), then
 * clears both speeds again.
 * HOW: The table is at overlay address $B9A3. ApplyVelocity moves it between the two sets of
 * stores.
 * WAITS: no.
 * Entry: X object. Exit: A, N, Z, C and V from ApplyVelocity's add; Y bit 0 of f3040; X kept.
 * Clobbers zTemp10.
 */
Ended ThrowByNextKind(uint8_t slot)
{
    enum { SPEEDS = 0xB9A3 };
    uint8_t kind = RD8(OBJ->next_kind[slot]);
    uint8_t index = (uint8_t)(kind & 1);
    uint8_t shifted;
    AxisMove move;

    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(SPEEDS + index)));
    kind = RD8(OBJ->next_kind[slot]);
    shifted = (uint8_t)(kind << 4);
    WR8(OBJ->vy_hi[slot], (uint8_t)((uint8_t)(shifted ^ 0xFF) + 1));
    jsr_push(0xB99C);
    move = ApplyVelocity(slot);
    jsr_pop();
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    return ended_then(ended_with_y(ended_kept(), index), ended_of_axis_move(move));
}

void abi_ThrowByNextKind(void)
{
    abi_ended(ThrowByNextKind(cpu.x));
}

/*
 * @routine 093C:23E AnimateThenState1
 * @run 70:BA3E
 * WHAT IT DOES: Steps the object's animation along one of two lists picked by bit 0 of its f3618;
 * when the list has ended, adds one to f3618 and sends the object to state 1.
 * HOW: The two list pointers are at overlay address $BA5D. StepObjectFrames's result says whether
 * the list ended.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames (Y the list's offset when the step did not change
 * it); when the list ended, A 1, N clear, Z clear. X and V kept. Clobbers zTemp10-11.
 */
Ended AnimateThenState1(uint8_t slot)
{
    enum { LISTS = 0xBA5D };
    uint8_t count = RD8(OBJ->f3618[slot]);
    uint8_t index = (uint8_t)((count & 1) << 1);
    uint8_t exit_y = index;
    FrameStep step;
    Ended e;

    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    jsr_push(0xBA52);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    e = ended_then(ended_with_y(ended_kept(), index), ended_of_frame_step(step, step != FRAME_HELD, exit_y));
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    count = RD8(OBJ->f3618[slot]);
    count = (uint8_t)(count + 1);
    WR8(OBJ->f3618[slot], count);
    WR8(OBJ->state[slot], 1);
    return ended_then(e, ended_load(1));
}

void abi_AnimateThenState1(void)
{
    abi_ended(AnimateThenState1(cpu.x));
}

/*
 * @routine 093C:338 AnimateThenNextStep
 * @run 70:BB38
 * WHAT IT DOES: Steps the object's animation along the list that its f3670 and f36c8 pick (two
 * pointers a table row, f3670 times two plus f36c8 as the byte offset); when the list has ended
 * the step byte goes up by one.
 * HOW: The pointers are at overlay address $BB54. V is left by the offset's add.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, and when the list ended N, Z from the new step byte
 * with A 0; V from the offset's add; Y the offset when the step did not change it. Clobbers
 * zTemp10-11.
 */
Ended AnimateThenNextStep(uint8_t slot)
{
    enum { LISTS = 0xBB54 };
    uint8_t row = RD8(OBJ->f3670[slot]);
    uint8_t doubled = (uint8_t)(row << 1);
    uint8_t extra = RD8(OBJ->f36c8[slot]);
    uint8_t index = (uint8_t)(doubled + extra);
    uint8_t exit_y = index, step_byte;
    FrameStep step;
    Ended e;

    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    jsr_push(0xBB4E);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    e = ended_then(ended_with_overflow(ended_with_y(ended_kept(), index), add_overflow(doubled, extra, index)),
                   ended_of_frame_step(step, step != FRAME_HELD, exit_y));
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    step_byte = RD8(OBJ->f2800[slot]);
    step_byte = (uint8_t)(step_byte + 1);
    WR8(OBJ->f2800[slot], step_byte);
    return ended_then(e, ended_nz(step_byte));
}

void abi_AnimateThenNextStep(void)
{
    abi_ended(AnimateThenNextStep(cpu.x));
}

/*
 * @routine 093C:48F FollowParentYAndLeave
 * @run 70:BC8F
 * WHAT IT DOES: Copies the Y position's middle byte of the object whose slot is in this object's
 * f3568 (its parent) into this object's, and removes this object when the parent is in state 4.
 * HOW: ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: A the parent's state, N, Z, C from its compare with 4; Y the parent's
 * slot; X and V kept.
 */
Ended FollowParentYAndLeave(uint8_t slot)
{
    uint8_t parent = RD8(OBJ->f3568[slot]);
    uint8_t middle, state;

    middle = RD8(OBJ->y_lo[parent]);
    WR8(OBJ->y_lo[slot], middle);
    state = RD8(OBJ->state[parent]);
    if (state == 4) {
        ClearObject(slot);
    }
    return ended_with_y(ended_compare(state, 4), parent);
}

void abi_FollowParentYAndLeave(void)
{
    abi_ended(FollowParentYAndLeave(cpu.x));
}

/*
 * @routine 093D:579 AnimateThenNextStepB
 * @run 71:C579
 * WHAT IT DOES: Steps the object's animation along the list at overlay address $C590; when the list
 * has ended, restarts it and adds one to the step byte.
 * HOW: The list pointer goes into zTemp10-11; StepObjectFrames says whether the list ended.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, and at the end N, Z from the new step byte; A 0;
 * X and V kept. Clobbers zTemp10-11.
 */
Ended AnimateThenNextStepB(uint8_t slot)
{
    uint8_t exit_y = 0, step_byte;
    FrameStep step;
    Ended e;

    WR8(ZP->temp11, 0xC5);
    WR8(ZP->temp10, 0x90);
    jsr_push(0xC584);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, exit_y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    step_byte = RD8(OBJ->f2800[slot]);
    step_byte = (uint8_t)(step_byte + 1);
    WR8(OBJ->f2800[slot], step_byte);
    return ended_then(e, ended_nz(step_byte));
}

void abi_AnimateThenNextStepB(void)
{
    abi_ended(AnimateThenNextStepB(cpu.x));
}
