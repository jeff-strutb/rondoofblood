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

/*
 * @routine 093D:78C CountdownThenFallOrBecomeType9B
 * @run 71:C78C
 * WHAT IT DOES: Counts the object's f3510 down; while it runs, pulls the object down with gravity
 * and moves it by its velocity; at zero it turns into an object of type $9B in state 0.
 * HOW: ApplyGravityUnlimited, then ApplyVelocity as a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocity while counting; at zero A $9B, N set, Z clear (C, V
 * kept); X and Y kept. Clobbers zTemp10.
 */
Ended CountdownThenFallOrBecomeType9B(uint8_t slot)
{
    uint8_t left = RD8(OBJ->f3510[slot]);
    AxisMove move;

    left = (uint8_t)(left - 1);
    WR8(OBJ->f3510[slot], left);
    if (left == 0) {
        return become_type(slot, 0x9B);
    }
    jsr_push(0xC79D);
    ApplyGravityUnlimited(slot);
    jsr_pop();
    move = ApplyVelocity(slot);
    return ended_of_axis_move(move);
}

void abi_CountdownThenFallOrBecomeType9B(void)
{
    abi_ended(CountdownThenFallOrBecomeType9B(cpu.x));
}

/*
 * @routine 093C:1A5 PickTimersAtRandom
 * @run 70:B9A5
 * WHAT IT DOES: Gives the object a pair of timers (f3510 and life_timer) from a table of four
 * pairs, the pair picked by a random number.
 * HOW: NextRandom, two bits of it doubled into an index into the table at overlay address $B9B9.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the second timer, N and Z from it; C 0 (the doubling); V as
 * NextRandom's last add left it; Y the table offset; X kept.
 */
RandomEnd PickTimersAtRandom(uint8_t slot)
{
    enum { TABLE = 0xB9B9 };
    RandomEnd result = { ended_kept(), true, { 0, false, false } };
    uint8_t bits, y, value;

    jsr_push(0xB9A8);
    result.random = NextRandom();
    jsr_pop();
    bits = (uint8_t)(result.random.value & 3);
    y = (uint8_t)(bits << 1);
    value = rd8((uint16_t)(TABLE + y));
    WR8(OBJ->f3510[slot], value);
    value = rd8((uint16_t)(TABLE + 1 + y));
    WR8(OBJ->life_timer[slot], value);
    result.end = ended_with_y(ended_with_carry(ended_load(value), false), y);
    return result;
}

void abi_PickTimersAtRandom(void)
{
    abi_random_end(PickTimersAtRandom(cpu.x));
}

/*
 * @routine 093C:1C1 PickAngleAtRandom
 * @run 70:B9C1
 * WHAT IT DOES: Gives the object an angle (f35c0 and f3568) from a table of four pairs, the pair
 * picked by a random number.
 * HOW: As PickTimersAtRandom, with the table at overlay address $B9D5.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as PickTimersAtRandom.
 */
RandomEnd PickAngleAtRandom(uint8_t slot)
{
    enum { TABLE = 0xB9D5 };
    RandomEnd result = { ended_kept(), true, { 0, false, false } };
    uint8_t bits, y, value;

    jsr_push(0xB9C4);
    result.random = NextRandom();
    jsr_pop();
    bits = (uint8_t)(result.random.value & 3);
    y = (uint8_t)(bits << 1);
    value = rd8((uint16_t)(TABLE + y));
    WR8(OBJ->f35c0[slot], value);
    value = rd8((uint16_t)(TABLE + 1 + y));
    WR8(OBJ->f3568[slot], value);
    result.end = ended_with_y(ended_with_carry(ended_load(value), false), y);
    return result;
}

void abi_PickAngleAtRandom(void)
{
    abi_random_end(PickAngleAtRandom(cpu.x));
}

/*
 * @routine 093C:4A3 AnimateMoveAndSag
 * @run 70:BCA3
 * WHAT IT DOES: Steps the object's animation along the list its life_timer picks (a table of
 * pointers at overlay address $BCD1), moves it by its velocity, and while life_timer is 1 adds
 * $18/256 to its Y speed.
 * HOW: StepObjectFrames, ApplyVelocity, then a 16-bit add through the Y speed bytes.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C from the compare with 1, or from the last add
 * (the Y speed's high byte); V from ApplyVelocity or that add; Y the list's offset unless the
 * animation step changed it; X kept. Clobbers zTemp10-11.
 */
Ended AnimateMoveAndSag(uint8_t slot)
{
    enum { LISTS = 0xBCD1 };
    uint8_t timer = RD8(OBJ->life_timer[slot]);
    uint8_t y = (uint8_t)(timer << 1);
    uint8_t exit_y = y, value;
    FrameStep step;
    AxisMove move;
    Ended e;
    Sum sum;

    WR8(ZP->temp10, rd8((uint16_t)(LISTS + y)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + y)));
    jsr_push(0xBCB5);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    e = ended_then(ended_with_y(ended_kept(), y), ended_of_frame_step(step, step != FRAME_HELD, exit_y));
    jsr_push(0xBCB8);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = ended_then(e, ended_of_axis_move(move));
    timer = RD8(OBJ->life_timer[slot]);
    e = ended_then(e, ended_compare(timer, 1));
    if (timer != 1) {
        return e;
    }
    value = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(value, 0x18, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    value = RD8(OBJ->vy_hi[slot]);
    sum = add_bytes(value, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    return ended_then(e, ended_with_overflow(ended_with_carry(ended_load(sum.value), sum.carry), sum.overflow));
}

void abi_AnimateMoveAndSag(void)
{
    abi_ended(AnimateMoveAndSag(cpu.x));
}

/*
 * @routine 093C:6E9 StepFramesFacingPlayerC
 * @run 70:BEE9
 * WHAT IT DOES: Turns the object to face the player and steps its animation along the list whose
 * pointer is the pair of bytes at overlay address $BEFE plus Y.
 * HOW: SideOfPlayer, the pointer into zTemp10-11, then StepObjectFrames as a tail jump.
 * WAITS: no.
 * Entry: X object, Y the list's offset, D clear. Exit: as StepObjectFrames. Clobbers zTemp00,
 * zTemp10-11.
 */
FrameStepEnd StepFramesFacingPlayerC(uint8_t slot, uint8_t index)
{
    enum { LISTS = 0xBEFE };
    uint8_t sides, exit_y = index;
    FrameStepEnd result;

    jsr_push(0xBEEC);
    sides = SideOfPlayer(slot).sides;
    jsr_pop();
    WR8(OBJ->facing_left[slot], (uint8_t)(sides & 1));
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    result.step = StepObjectFrames(slot, &exit_y);
    result.end = ended_of_frame_step(result.step, result.step != FRAME_HELD, exit_y);
    return result;
}

void abi_StepFramesFacingPlayerC(void)
{
    abi_ended(StepFramesFacingPlayerC(cpu.x, cpu.y).end);
}

/*
 * @routine 093D:282 FaceThePlayerInSomeStates
 * @run 71:C282
 * WHAT IT DOES: Turns the object to face the player when a table at overlay address $C293 says its
 * state is one that does.
 * HOW: Reads the table by the state byte; non-zero runs SideOfPlayer into the facing byte.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the table byte or the side bit; N, Z from it; C from
 * SideOfPlayer's last compare when it ran; Y the state; X kept. Clobbers zTemp00.
 */
SideEnd FaceThePlayerInSomeStates(uint8_t slot)
{
    enum { TABLE = 0xC293 };
    SideEnd result = { ended_kept(), false, { 0, false, false } };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t entry = rd8((uint16_t)(TABLE + state));
    uint8_t bit;

    result.end = ended_with_y(ended_load(entry), state);
    if (entry == 0) {
        return result;
    }
    jsr_push(0xC28D);
    result.side = SideOfPlayer(slot);
    jsr_pop();
    result.side_used = true;
    bit = (uint8_t)(result.side.sides & 1);
    WR8(OBJ->facing_left[slot], bit);
    result.end = ended_with_y(ended_load(bit), state);
    return result;
}

void abi_FaceThePlayerInSomeStates(void)
{
    abi_side_end(FaceThePlayerInSomeStates(cpu.x));
}

/*
 * @routine 093D:4AB NextPhaseAtRandom
 * @run 71:C4AB
 * WHAT IT DOES: Steps the object to its next phase: counts f3568 up, sets life_timer from a four
 * byte table by it, turns f3510 to the side the player is on, and picks a step byte of 1 or 2 at
 * random.
 * HOW: A table at overlay address $C4CE, SideOfPlayer, NextRandom.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the new step byte, N and Z from it, C and V clear; Y the table
 * offset; X kept. Clobbers zTemp00.
 */
Ended NextPhaseAtRandom(uint8_t slot)
{
    enum { TABLE = 0xC4CE };
    uint8_t phase, y, side, bits, step;
    RandomResult random;

    INC8(OBJ->f3568[slot]);
    phase = RD8(OBJ->f3568[slot]);
    y = (uint8_t)(phase & 3);
    WR8(OBJ->life_timer[slot], rd8((uint16_t)(TABLE + y)));
    jsr_push(0xC4BD);
    side = SideOfPlayer(slot).sides;
    jsr_pop();
    WR8(OBJ->f3510[slot], (uint8_t)(side & 1));
    jsr_push(0xC4C5);
    random = NextRandom();
    jsr_pop();
    bits = (uint8_t)(random.value & 1);
    step = (uint8_t)(bits + 1);
    WR8(OBJ->f2800[slot], step);
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_load(step), false), false), y);
}

void abi_NextPhaseAtRandom(void)
{
    abi_ended(NextPhaseAtRandom(cpu.x));
}

/*
 * @routine 093B:4C3 ShowOnScreenAndWrapX
 * @run 70:B4C3
 * WHAT IT DOES: The first time the object runs (state 0) starts it from its record. After that
 * it sets frame 2, hides it (frame 0) when its screen X is past the first seven screens and
 * the low sum is negative, and, while the scroll moves, wraps the object's X by $80 across the
 * page boundary at either end of the level (X high $FF with low under $E0 to high 1, or high 1
 * with low from $20 up to high $FF), by the scroll's direction.
 * HOW: Adds the scroll's X low and screen to the object's X; tests ScrollSpeedX's sign.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as LoadObjectRecord in state 0; otherwise A, N, Z, C and V
 * from the last compare, load or add; X, Y kept. Clobbers zTemp00 (in states but 0).
 */
Ended ShowOnScreenAndWrapX(uint8_t slot)
{
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t low, high, scroll_low, scroll_high, held, speed, nz, last;
    bool carry;
    Sum sum, sum2;
    Ended e;

    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        return ended_of_record(last, nz, carry);
    }
    WR8(OBJ->frame[slot], 2);
    low = RD8(OBJ->x_lo[slot]);
    scroll_low = RD8(SCROLL->x_lo[0]);
    sum = add_bytes(low, scroll_low, false);
    WR8(ZP->temp00, sum.value);
    high = RD8(OBJ->x_hi[slot]);
    scroll_high = RD8(SCROLLE->x_hi_screen[0]);
    sum2 = add_bytes(high, scroll_high, sum.carry);
    e = ended_with_overflow(ended_compare(sum2.value, 7), sum2.overflow);
    if (sum2.value >= 7) {
        held = RD8(ZP->temp00);
        e = ended_then(e, ended_load(held));
        if ((held & 0x80) != 0) {
            WR8(OBJ->frame[slot], 0);
        }
    }
    speed = RD8(SCROLLE->speed_x[0]);
    e = ended_then(e, ended_load(speed));
    if (speed == 0) {
        return e;
    }
    if ((speed & 0x80) == 0) {
        high = RD8(OBJ->x_hi[slot]);
        e = ended_then(e, ended_compare(high, 0xFF));
        if (high != 0xFF) {
            return e;
        }
        low = RD8(OBJ->x_lo[slot]);
        e = ended_then(e, ended_compare(low, 0xE0));
        if (low >= 0xE0) {
            return e;
        }
        sum = add_bytes(low, 0x80, false);
        WR8(OBJ->x_lo[slot], sum.value);
        WR8(OBJ->x_hi[slot], 1);
        return ended_then(e, ended_with_overflow(ended_with_carry(ended_load(1), sum.carry), sum.overflow));
    }
    high = RD8(OBJ->x_hi[slot]);
    e = ended_then(e, ended_compare(high, 1));
    if (high != 1) {
        return e;
    }
    low = RD8(OBJ->x_lo[slot]);
    e = ended_then(e, ended_compare(low, 0x20));
    if (low < 0x20) {
        return e;
    }
    sum = add_bytes(low, 0x80, false);
    WR8(OBJ->x_lo[slot], sum.value);
    WR8(OBJ->x_hi[slot], 0xFF);
    return ended_then(e, ended_with_overflow(ended_with_carry(ended_load(0xFF), sum.carry), sum.overflow));
}

void abi_ShowOnScreenAndWrapX(void)
{
    abi_ended(ShowOnScreenAndWrapX(cpu.x));
}

/*
 * @routine 093C:6AB StartChargeWhenPlayerNear
 * @run 70:BEAB
 * WHAT IT DOES: Animates the object facing the player; while the object is on the first screen
 * and under $78 pixels from the player along X, it stops its animation, keeps only bit 7 of its
 * flags, sets its Y speed's high byte to 3, picks a timer ($44 plus 7 random bits) in f36c8 and
 * enters state 4 with step 2.
 * HOW: StepFramesFacingPlayerC (Y 0), the high bytes ORed, DistanceXToPlayer, NextRandom.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A 2 and N, Z clear, C 0, V from the add of the timer when it
 * started; otherwise as the last compare, OR or animation step left them; Y 0 or as the step
 * left it; X kept.
 */
Ended StartChargeWhenPlayerNear(uint8_t slot)
{
    FrameStepEnd frames;
    PlayerDistance distance;
    RandomResult random;
    uint8_t high, y_high, flags, timer;
    Sum sum;
    Ended e;

    jsr_push(0xBEAF);
    frames = StepFramesFacingPlayerC(slot, 0);
    jsr_pop();
    e = ended_then(ended_with_y(ended_kept(), 0), frames.end);
    high = RD8(OBJ->x_hi[slot]);
    y_high = RD8(OBJ->y_hi[slot]);
    e = ended_then(e, ended_load((uint8_t)(high | y_high)));
    if ((high | y_high) != 0) {
        return e;
    }
    jsr_push(0xBEBA);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = ended_then(e, ended_of_distance(distance));
    e = ended_then(e, ended_compare(distance.distance, 0x78));
    if (distance.distance >= 0x78) {
        return e;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    flags = RD8(OBJ->flags[slot]);
    WR8(OBJ->flags[slot], (uint8_t)(flags & 0x80));
    WR8(OBJ->vy_hi[slot], 3);
    jsr_push(0xBED4);
    random = NextRandom();
    jsr_pop();
    e = ended_then(e, ended_of_random(random));
    timer = (uint8_t)(random.value & 0x7F);
    sum = add_bytes(timer, 0x44, false);
    WR8(OBJ->f36c8[slot], sum.value);
    WR8(OBJ->state[slot], 4);
    WR8(OBJ->f2800[slot], 2);
    e = ended_then(e, ended_with_overflow(ended_with_carry(ended_load(sum.value), false), sum.overflow));
    return ended_then(e, ended_load(2));
}

void abi_StartChargeWhenPlayerNear(void)
{
    abi_ended(StartChargeWhenPlayerNear(cpu.x));
}

/*
 * @routine 093D:191 WaitThenState2B
 * @run 71:C191
 * WHAT IT DOES: A two-step wait like WaitThenState2: the first pass sets f34b8 to $1C and the step
 * to 1; later passes animate the object facing the player (list index 2) and count the timer down;
 * at zero the object enters state 2.
 * HOW: StepFramesFacingPlayerC with Y 2, then a decrement.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as WaitThenState2.
 */
Ended WaitThenState2B(uint8_t slot)
{
    uint8_t step = RD8(OBJ->f2800[slot]);
    uint8_t left;
    FrameStepEnd frames;
    Ended e;

    if (step == 0) {
        WR8(OBJ->life_timer[slot], 0x1C);
        step = RD8(OBJ->f2800[slot]);
        step = (uint8_t)(step + 1);
        WR8(OBJ->f2800[slot], step);
        return ended_with_a(ended_nz(step), 0x1C);
    }
    jsr_push(0xC1A4);
    frames = StepFramesFacingPlayerC(slot, 2);
    jsr_pop();
    e = ended_then(ended_with_y(ended_kept(), 2), frames.end);
    left = RD8(OBJ->life_timer[slot]);
    left = (uint8_t)(left - 1);
    WR8(OBJ->life_timer[slot], left);
    if (left != 0) {
        return ended_then(e, ended_nz(left));
    }
    WR8(OBJ->state[slot], 2);
    return ended_then(e, ended_load(2));
}

void abi_WaitThenState2B(void)
{
    abi_ended(WaitThenState2B(cpu.x));
}

/*
 * @routine 093C:28D ChooseNextMoveWhenFacingAway
 * @run 70:BA8D
 * WHAT IT DOES: When the side the player is on matches the object's facing (compared with bit 0
 * of f3618), picks a phase and timer at random from two tables and advances the step; otherwise it
 * enters state 1.
 * HOW: SideOfPlayer; NextRandom twice; tables at overlay addresses $BAD5, $BAE5 and $BAF1.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the last byte loaded, N and Z from the step byte's new
 * value (when picking); C and V from the second add; Y the last table index; X kept. Clobbers
 * zTemp00, zTemp08.
 */
Ended ChooseNextMoveWhenFacingAway(uint8_t slot)
{
    enum { PHASES = 0xBAD5, TIMERS = 0xBAE5, PIECES = 0xBAF1 };
    uint8_t facing = RD8(OBJ->facing_left[slot]);
    uint8_t count = RD8(OBJ->f3618[slot]);
    uint8_t mine = (uint8_t)((facing ^ count) & 1);
    uint8_t side, theirs, y, phase, shifted, value, step;
    SideResult sides;
    RandomResult random;
    Sum sum;
    Ended e;

    WR8(ZP->temp08, mine);
    jsr_push(0xBA9A);
    sides = SideOfPlayer(slot);
    jsr_pop();
    side = sides.sides;
    theirs = (uint8_t)(side & 1);
    e = ended_then(ended_of_side(sides), ended_load(theirs));
    e = ended_then(e, ended_compare(theirs, RD8(ZP->temp08)));
    if (theirs != mine) {
        WR8(OBJ->state[slot], 1);
        return ended_then(e, ended_load(1));
    }
    jsr_push(0xBAA9);
    random = NextRandom();
    jsr_pop();
    y = (uint8_t)(random.value & 0x0F);
    phase = rd8((uint16_t)(PHASES + y));
    WR8(OBJ->f3670[slot], phase);
    shifted = (uint8_t)(phase << 2);
    WR8(ZP->temp00, shifted);
    jsr_push(0xBAB9);
    random = NextRandom();
    jsr_pop();
    value = RD8(ZP->temp00);
    sum = add_bytes((uint8_t)(random.value & 3), value, false);
    y = sum.value;
    value = rd8((uint16_t)(TIMERS + y));
    WR8(OBJ->f3720[slot], value);
    count = RD8(OBJ->f3618[slot]);
    y = (uint8_t)(count & 1);
    value = rd8((uint16_t)(PIECES + y));
    WR8(OBJ->f36c8[slot], value);
    step = INC8(OBJ->f2800[slot]);
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_with_a(ended_nz(step), value),
                                                             sum.carry), sum.overflow), y);
}

void abi_ChooseNextMoveWhenFacingAway(void)
{
    abi_ended(ChooseNextMoveWhenFacingAway(cpu.x));
}

/*
 * @routine 093C:1E4 CountTimerPairDownOrRepick
 * @run 70:B9E4
 * WHAT IT DOES: Counts the sixteen bit timer (f3510 low, life_timer high) down by one; when it is
 * already zero, picks new timers at random (PickTimersAtRandom), stops the animation and enters
 * state 2.
 * HOW: The two bytes are ORed to test for zero; the decrement is a subtract with the borrow carried.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the new high byte and N, Z from it, C and V from the borrow
 * subtract; or A 2 and N, Z clear with C, V and Y as the random pick left them.
 */
Ended CountTimerPairDownOrRepick(uint8_t slot)
{
    uint8_t high = RD8(OBJ->life_timer[slot]);
    uint8_t low = (uint8_t)(high | RD8(OBJ->f3510[slot]));
    RandomEnd picked;
    Sum sum;

    if (low == 0) {
        jsr_push(0xB9EF);
        picked = PickTimersAtRandom(slot);
        jsr_pop();
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 2);
        return ended_then(ended_then(ended_of_random(picked.random), picked.end), ended_load(2));
    }
    low = RD8(OBJ->f3510[slot]);
    sum = add_bytes(low, 0xFE, true);
    WR8(OBJ->f3510[slot], sum.value);
    high = RD8(OBJ->life_timer[slot]);
    sum = add_bytes(high, 0xFF, sum.carry);
    WR8(OBJ->life_timer[slot], sum.value);
    return ended_of_sum(sum);
}

void abi_CountTimerPairDownOrRepick(void)
{
    abi_ended(CountTimerPairDownOrRepick(cpu.x));
}

/*
 * @routine 093C:20D CountAnglePairDownOrRepick
 * @run 70:BA0D
 * WHAT IT DOES: On the first screen across only: counts the sixteen bit value f35c0 (low) and
 * f3568 (high) down by one; when it is already zero, picks a new angle at random
 * (PickAngleAtRandom), stops the animation, clears the step and enters state 3.
 * HOW: As CountTimerPairDownOrRepick.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as CountTimerPairDownOrRepick, or A the X high byte and N, Z from
 * it when off the first screen.
 */
Ended CountAnglePairDownOrRepick(uint8_t slot)
{
    uint8_t high = RD8(OBJ->x_hi[slot]);
    uint8_t value, low;
    RandomEnd picked;
    Sum sum;

    if (high != 0) {
        return ended_load(high);
    }
    value = RD8(OBJ->f3568[slot]);
    value = (uint8_t)(value | RD8(OBJ->f35c0[slot]));
    if (value == 0) {
        jsr_push(0xBA1D);
        picked = PickAngleAtRandom(slot);
        jsr_pop();
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->f2800[slot], 0);
        WR8(OBJ->state[slot], 3);
        return ended_then(ended_then(ended_of_random(picked.random), picked.end), ended_load(3));
    }
    low = RD8(OBJ->f35c0[slot]);
    sum = add_bytes(low, 0xFE, true);
    WR8(OBJ->f35c0[slot], sum.value);
    high = RD8(OBJ->f3568[slot]);
    sum = add_bytes(high, 0xFF, sum.carry);
    WR8(OBJ->f3568[slot], sum.value);
    return ended_of_sum(sum);
}

void abi_CountAnglePairDownOrRepick(void)
{
    abi_ended(CountAnglePairDownOrRepick(cpu.x));
}

/*
 * @routine 093C:54C AnimateHitboxThenExpire
 * @run 70:BD4C
 * WHAT IT DOES: Animates the object along the list at overlay address $BD63, sets its hit
 * rectangle from the frame (SetHitboxFromFrameRows), and counts f3510 down; at zero the object
 * removes itself.
 * HOW: StepObjectFrames, SetHitboxFromFrameRows, a decrement, ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: N, Z from the timer; A, C, V, Y as SetHitboxFromFrameRows left
 * them; X kept. Clobbers zTemp10-11.
 */
Ended AnimateHitboxThenExpire(uint8_t slot)
{
    uint8_t exit_y = 0, left;
    FrameStep step;
    Ended e;

    WR8(ZP->temp11, 0xBD);
    WR8(ZP->temp10, 0x63);
    jsr_push(0xBD57);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, exit_y);
    jsr_push(0xBD5A);
    e = ended_then(e, SetHitboxFromFrameRows(slot));
    jsr_pop();
    left = RD8(OBJ->f3510[slot]);
    left = (uint8_t)(left - 1);
    WR8(OBJ->f3510[slot], left);
    e = ended_then(e, ended_nz(left));
    if (left == 0) {
        ClearObject(slot);
    }
    return e;
}

void abi_AnimateHitboxThenExpire(void)
{
    abi_ended(AnimateHitboxThenExpire(cpu.x));
}

/*
 * @routine 093D:33A WalkWhileFrameChangesB
 * @run 71:C33A
 * WHAT IT DOES: Animates the object along the list whose pointer is the pair of bytes at overlay
 * address $C356 plus Y and, on the frames where the picture changed, gives it an X speed from a
 * table at $C35A by its f3510 and moves it along X.
 * HOW: StepObjectFrames, then ApplyVelocityX.
 * WAITS: no.
 * Entry: X object, Y the list's offset, D clear. Exit: after a change as ApplyVelocityX, Y f3510;
 * otherwise as StepObjectFrames; X kept. Clobbers zTemp10-11.
 */
Ended WalkWhileFrameChangesB(uint8_t slot, uint8_t index)
{
    enum { LISTS = 0xC356, SPEEDS = 0xC35A };
    uint8_t exit_y = index, phase;
    FrameStep step;
    AxisMove move;
    Ended e;

    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    jsr_push(0xC347);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, exit_y);
    if (step != FRAME_CHANGED) {
        return e;
    }
    phase = RD8(OBJ->f3510[slot]);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(SPEEDS + phase)));
    jsr_push(0xC355);
    move = ApplyVelocityX(slot);
    jsr_pop();
    return ended_then(ended_then(e, ended_with_y(ended_kept(), phase)), ended_of_axis_move(move));
}

void abi_WalkWhileFrameChangesB(void)
{
    abi_ended(WalkWhileFrameChangesB(cpu.x, cpu.y));
}

/*
 * @routine 093D:7A0 StartThenBlinkAndExpire
 * @run 71:C7A0
 * WHAT IT DOES: The first time (state 0) starts the object from its record and sets f3510 to
 * $10. After that it counts f3510 down, removing the object at zero; otherwise it shows frame 0,
 * or its life_timer as the frame on every fourth count.
 * HOW: LoadObjectRecord; a decrement; ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as the last store, load, AND or decrement left them; Y 11 after
 * the record start.
 */
Ended StartThenBlinkAndExpire(uint8_t slot)
{
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t left, bits, value, nz, last;
    bool carry;
    Ended e;

    if (state == 0) {
        jsr_push(0xC7A8);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        WR8(OBJ->f3510[slot], 0x10);
        return ended_then(ended_of_record(last, nz, carry), ended_load(0x10));
    }
    left = RD8(OBJ->f3510[slot]);
    left = (uint8_t)(left - 1);
    WR8(OBJ->f3510[slot], left);
    e = ended_then(ended_load(state), ended_nz(left));
    if (left == 0) {
        ClearObject(slot);
        return e;
    }
    WR8(OBJ->frame[slot], 0);
    value = RD8(OBJ->f3510[slot]);
    bits = (uint8_t)(value & 3);
    e = ended_then(e, ended_load(bits));
    if (bits == 0) {
        value = RD8(OBJ->life_timer[slot]);
        WR8(OBJ->frame[slot], value);
        e = ended_then(e, ended_load(value));
    }
    return e;
}

void abi_StartThenBlinkAndExpire(void)
{
    abi_ended(StartThenBlinkAndExpire(cpu.x));
}

/*
 * @routine 093D:53D LeapAwayFromPlayerTable
 * @run 71:C53D
 * WHAT IT DOES: Advances the step and launches the object: its X speed from a table at overlay
 * address $C569 by the player's distance in 32 pixel steps, Y speed -4.5 ($FB80); when its
 * f3510 is not zero the X speed is then reversed.
 * HOW: DistanceXToPlayer shifted to five bits and doubled, NegateVelocityX.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z from f3510 (when zero) or the negated speed's high byte
 * with C and V from its add; Y the table offset.
 */
Ended LeapAwayFromPlayerTable(uint8_t slot)
{
    enum { TABLE = 0xC569 };
    PlayerDistance distance;
    uint8_t y, flag;
    Ended e;

    INC8(OBJ->f2800[slot]);
    jsr_push(0xC543);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    y = (uint8_t)((distance.distance >> 5) << 1);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(TABLE + 1 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(TABLE + y)));
    WR8(OBJ->vy_hi[slot], 0xFB);
    WR8(OBJ->vy_lo[slot], 0x80);
    flag = RD8(OBJ->f3510[slot]);
    e = ended_then(ended_of_distance(distance), ended_with_y(ended_with_carry(ended_load(flag), false), y));
    if (flag == 0) {
        return e;
    }
    jsr_push(0xC568);
    e = ended_then(e, ended_of_sum(NegateVelocityX(slot)));
    jsr_pop();
    return e;
}

void abi_LeapAwayFromPlayerTable(void)
{
    abi_ended(LeapAwayFromPlayerTable(cpu.x));
}

/*
 * @routine 093C:731 PickStateByPlayerDistanceB
 * @run 70:BF31
 * WHAT IT DOES: PickStateByPlayerDistance in this stage's overlay, with its state table at
 * overlay address $BF57.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as PickStateByPlayerDistance.
 */
Ended PickStateByPlayerDistanceB(uint8_t slot, bool carry_in)
{
    return pick_state_by_distance(slot, carry_in, 0xBF57, 0xBF4D);
}

void abi_PickStateByPlayerDistanceB(void)
{
    abi_ended(PickStateByPlayerDistanceB(cpu.x, (cpu.p & P_C) != 0));
}

/*
 * @routine 093C:7AA PickSpeedAndTurnFromTableC
 * @run 70:BFAA
 * WHAT IT DOES: PickSpeedAndTurnFromTable in this stage's overlay, with its table at overlay
 * address $BFD6.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as PickSpeedAndTurnFromTable.
 */
Ended PickSpeedAndTurnFromTableC(uint8_t slot)
{
    return pick_speed_and_turn(slot, 0xBFD6, 0xBFCE);
}

void abi_PickSpeedAndTurnFromTableC(void)
{
    abi_ended(PickSpeedAndTurnFromTableC(cpu.x));
}
