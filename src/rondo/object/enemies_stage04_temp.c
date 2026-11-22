/*
 * TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
 * together here until each kind has a file of its own.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"

/*
 * @routine 07DB:5FD ClampSpeedXToTwo
 * @run 70:B5FD
 * WHAT IT DOES: Keeps an object's X speed from passing 2 pixels a frame either way: a speed over
 * +2 becomes +2.00 and one under -2 becomes -2.00.
 * HOW: Reads the speed's high byte (twice, as the sign test and the compare each load it) and,
 * past the limit, stores the limit in the high byte and zero in the low byte.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z, C from the last compare or load (C the compare's, also when the
 * speed is clamped); X and Y kept.
 */
Ended ClampSpeedXToTwo(uint8_t slot)
{
    uint8_t speed = RD8(OBJ->vx_hi[slot]);

    if ((speed & 0x80) != 0) {
        speed = RD8(OBJ->vx_hi[slot]);
        if (speed >= 0xFE) {
            return ended_compare(speed, 0xFE);
        }
        WR8(OBJ->vx_hi[slot], 0xFE);
        WR8(OBJ->vx_lo[slot], 0);
        return ended_with_carry(ended_load(0xFE), false);
    }
    speed = RD8(OBJ->vx_hi[slot]);
    if (speed < 2) {
        return ended_compare(speed, 2);
    }
    WR8(OBJ->vx_hi[slot], 2);
    WR8(OBJ->vx_lo[slot], 0);
    return ended_with_carry(ended_load(2), true);
}

void abi_ClampSpeedXToTwo(void)
{
    abi_ended(ClampSpeedXToTwo(cpu.x));
}

/*
 * @routine 07DB:622 ThrowUpward
 * @run 70:B622
 * WHAT IT DOES: Throws an object upward at 3 pixels a frame, unless it is already rising slowly
 * (its Y speed between -2.00 and 0 counted as a high byte of $FE or $FF).
 * HOW: Reads the Y speed's high byte; below $FE it stores $FD there and zero in the low byte.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z, C from the last compare or load; X and Y kept.
 */
Ended ThrowUpward(uint8_t slot)
{
    uint8_t speed = RD8(OBJ->vy_hi[slot]);

    if (speed >= 0xFE) {
        return ended_compare(speed, 0xFE);
    }
    WR8(OBJ->vy_hi[slot], 0xFD);
    WR8(OBJ->vy_lo[slot], 0);
    return ended_with_carry(ended_load(0xFD), false);
}

void abi_ThrowUpward(void)
{
    abi_ended(ThrowUpward(cpu.x));
}

/*
 * @routine 07DB:71B CountdownThenState4
 * @run 70:B71B
 * WHAT IT DOES: Counts the object's f3510 timer down and, when it reaches zero, sends the
 * object to state 4.
 * HOW: One decrement of the timer; the state byte is written only at zero.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the timer, or A 4 at zero; X, Y and C kept.
 */
Ended CountdownThenState4(uint8_t slot)
{
    return countdown_then_state(slot, COL(OBJ->f3510), 4);
}

void abi_CountdownThenState4(void)
{
    abi_ended(CountdownThenState4(cpu.x));
}

/*
 * @routine 07DB:785 ChangeToType9B
 * @run 70:B785
 * WHAT IT DOES: Turns the object into one of type $9B, in state 0.
 * HOW: Stores $9B in its type byte (OBJ_2750) and zero in its state.
 * WAITS: no.
 * Entry: X object. Exit: A $9B, N set, Z clear; X, Y and C kept.
 */
Ended ChangeToType9B(uint8_t slot)
{
    return become_type(slot, 0x9B);
}

void abi_ChangeToType9B(void)
{
    abi_ended(ChangeToType9B(cpu.x));
}

/*
 * @routine 07DB:7B1 EnterState6IfFlagged
 * @run 70:B7B1
 * WHAT IT DOES: Sends the object to state 6 when bit 7 of its f31a0 is set.
 * HOW: Reads f31a0 once and masks bit 7; writes the state only when it is set.
 * WAITS: no.
 * Entry: X object. Exit: A the masked bit, or 6 when entered; N, Z from it; X, Y and C kept.
 */
Ended EnterState6IfFlagged(uint8_t slot)
{
    return enter_state_if_flagged(slot, 6);
}

void abi_EnterState6IfFlagged(void)
{
    abi_ended(EnterState6IfFlagged(cpu.x));
}

/*
 * @routine 07DC:09D CountdownStopThenState4
 * @run 70:B89D
 * WHAT IT DOES: Counts the object's f34b8 timer down and, when it reaches zero, clears its f3408
 * and f3460 and sends it to state 4.
 * HOW: One decrement of the timer; the rest only at zero.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the timer, or A 4 at zero; X, Y and C kept.
 */
Ended CountdownStopThenState4(uint8_t slot)
{
    return countdown_stop_then_state(slot, COL(OBJ->f34b8), 4);
}

void abi_CountdownStopThenState4(void)
{
    abi_ended(CountdownStopThenState4(cpu.x));
}

/*
 * @routine 07DF:349 RightOfObject4DWithin
 * @run 71:D349
 * WHAT IT DOES: Says, in the carry, whether the object stands to the right of the object in slot
 * $4D by less than the distance in A, when slot $4D holds an object of type $AC.
 * HOW: Stores A as the limit in zTemp00; checks slot $4D's type; subtracts the two X positions
 * (low bytes into zTemp01, then high bytes); the answer is yes only when the high bytes are
 * equal and the low difference is under the limit.
 * WAITS: no.
 * Entry: X object, A limit. Exit: C set for yes; A the last value compared; N, Z from it. Clobbers
 * zTemp00-01. The arms after the type check ran in no script, so the draft stops there and
 * liftcmp judges only the type check; the rest waits for coverage (PROCEDURE section 6).
 */
Ended RightOfObject4DWithin(uint8_t slot, uint8_t limit)
{
    enum { OTHER = 0x4D, OTHER_TYPE = 0xAC };
    uint8_t type, low, other_low, high, other_high, diff_low, diff_high, kept;
    bool borrow, overflow;

    WR8(ZP->temp00, limit);
    type = RD8(OBJ->kind[OTHER]);
    if (type != OTHER_TYPE) {
        return ended_with_carry(ended_compare(type, OTHER_TYPE), false);
    }
    low = RD8(OBJ->x_lo[slot]);
    other_low = RD8(OBJ->x_lo[OTHER]);
    diff_low = (uint8_t)(low - other_low);
    borrow = low < other_low;
    WR8(ZP->temp01, diff_low);
    high = RD8(OBJ->x_hi[slot]);
    other_high = RD8(OBJ->x_hi[OTHER]);
    diff_high = (uint8_t)(high - other_high - (borrow ? 1 : 0));
    overflow = (((high ^ other_high) & (high ^ diff_high)) & 0x80) != 0;
    if (diff_high != 0) {
        return ended_with_overflow(ended_with_carry(ended_load(diff_high), false), overflow);
    }
    kept = RD8(ZP->temp01);
    limit = RD8(ZP->temp00);
    if (kept >= limit) {
        return ended_with_overflow(ended_with_carry(ended_compare(kept, limit), false), overflow);
    }
    return ended_with_overflow(ended_with_carry(ended_compare(kept, limit), true), overflow);
}

void abi_RightOfObject4DWithin(void)
{
    abi_ended(RightOfObject4DWithin(cpu.x, cpu.a));
}

/*
 * @routine 07DB:726 LoadRecordStartAngle40
 * @run 70:B726
 * WHAT IT DOES: Starts an object from its record, moves it by its velocity, and sets its angle's
 * high byte to $40.
 * HOW: LoadObjectRecord, ApplyVelocity, one store.
 * WAITS: no.
 * Entry: X object. Exit: A $40, N and Z clear; C and V from ApplyVelocity's add; Y 11; X kept.
 */
Ended LoadRecordStartAngle40(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;
    AxisMove move;
    Ended e;

    jsr_push(0xB729);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    jsr_push(0xB72C);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = ended_then(e, ended_of_axis_move(move));
    WR8(OBJ->angle_hi[slot], 0x40);
    return ended_then(e, ended_load(0x40));
}

void abi_LoadRecordStartAngle40(void)
{
    abi_ended(LoadRecordStartAngle40(cpu.x));
}

/*
 * @routine 07DB:7BE LoadRecordTimer50
 * @run 70:B7BE
 * WHAT IT DOES: Starts an object from its record and sets its f34b8 timer to $50.
 * HOW: LoadObjectRecord, one store.
 * WAITS: no.
 * Entry: X object. Exit: A $50, N and Z clear; C from the record; Y 11; X and V kept.
 */
Ended LoadRecordTimer50(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;
    Ended e;

    jsr_push(0xB7C1);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->life_timer[slot], 0x50);
    return ended_then(e, ended_load(0x50));
}

void abi_LoadRecordTimer50(void)
{
    abi_ended(LoadRecordTimer50(cpu.x));
}

/*
 * @routine 07DB:3ED StepFramesFacingPlayer
 * @run 70:B3ED
 * WHAT IT DOES: Steps the object's animation by one frame from the list whose pointer is the
 * pair of bytes at overlay address $B409 plus Y, first turning the object to face the player
 * (the facing byte takes bit 0 of the side the player is on) once the object is past state 1.
 * HOW: SideOfPlayer, then the list pointer into zTemp10-11, then StepObjectFrames as a tail jump.
 * WAITS: no.
 * Entry: X object, Y the list's offset in the table. Exit: as StepObjectFrames (A $FF and C 1
 * when the frame changed; A 0 and C 0 when the list ended; A $FF and C 0 when held; Y changed
 * only in the first two cases); V and X kept. Clobbers zTemp00, zTemp10-11.
 */
FrameStepEnd StepFramesFacingPlayer(uint8_t slot, uint8_t index)
{
    enum { LISTS = 0xB409 };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t sides, exit_y = index;
    FrameStepEnd result;

    if (state >= 2) {
        jsr_push(0xB3F7);
        sides = SideOfPlayer(slot).sides;
        jsr_pop();
        WR8(OBJ->facing_left[slot], (uint8_t)(sides & 1));
    }
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    result.step = StepObjectFrames(slot, &exit_y);
    result.end = ended_of_frame_step(result.step, result.step != FRAME_HELD, exit_y);
    return result;
}

void abi_StepFramesFacingPlayer(void)
{
    abi_ended(StepFramesFacingPlayer(cpu.x, cpu.y).end);
}

/*
 * @routine 07DC:017 StepFramesFacingPlayerAlways
 * @run 70:B817
 * WHAT IT DOES: StepFramesFacingPlayer without the state test: the object always turns to face
 * the player, and the animation list is the pair of bytes at overlay address $B82C plus Y.
 * HOW: SideOfPlayer, the list pointer into zTemp10-11, then StepObjectFrames as a tail jump.
 * WAITS: no.
 * Entry, exit: as StepFramesFacingPlayer.
 */
FrameStepEnd StepFramesFacingPlayerAlways(uint8_t slot, uint8_t index)
{
    enum { LISTS = 0xB82C };
    uint8_t sides, exit_y = index;
    FrameStepEnd result;

    jsr_push(0xB81A);
    sides = SideOfPlayer(slot).sides;
    jsr_pop();
    WR8(OBJ->facing_left[slot], (uint8_t)(sides & 1));
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    result.step = StepObjectFrames(slot, &exit_y);
    result.end = ended_of_frame_step(result.step, result.step != FRAME_HELD, exit_y);
    return result;
}

void abi_StepFramesFacingPlayerAlways(void)
{
    abi_ended(StepFramesFacingPlayerAlways(cpu.x, cpu.y).end);
}

/*
 * @routine 07DB:7F1 WalkWhileFrameChanges
 * @run 70:B7F1
 * WHAT IT DOES: Animates the object facing the player and, on the frames where the picture
 * changed, gives it an X speed from a two byte table by its f3510 and moves it along X.
 * HOW: StepFramesFacingPlayerAlways with Y 0; when it says the frame changed, the speed byte
 * at overlay address $B804 plus f3510 goes to the X speed's high byte and ApplyVelocityX runs.
 * WAITS: no.
 * Entry: X object. Exit: after a change, as ApplyVelocityX; otherwise as the animation step
 * (A $FF, C 0, or A 0, C 0). Clobbers zTemp00, zTemp10-11, zTemp-bytes of the move.
 */
Ended WalkWhileFrameChanges(uint8_t slot)
{
    enum { SPEEDS = 0xB804 };
    FrameStepEnd step;
    uint8_t phase;
    AxisMove move;

    jsr_push(0xB7F5);
    step = StepFramesFacingPlayerAlways(slot, 0);
    jsr_pop();
    if (step.step != FRAME_CHANGED) {
        return ended_then(ended_with_y(ended_kept(), 0), step.end);
    }
    phase = RD8(OBJ->f3510[slot]);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(SPEEDS + phase)));
    jsr_push(0xB803);
    move = ApplyVelocityX(slot);
    jsr_pop();
    return ended_then(ended_then(step.end, ended_with_y(ended_kept(), phase)), ended_of_axis_move(move));
}

void abi_WalkWhileFrameChanges(void)
{
    abi_ended(WalkWhileFrameChanges(cpu.x));
}

/*
 * @routine 07DB:6CE WaitThenState2
 * @run 70:B6CE
 * WHAT IT DOES: A two-step wait. First time through (step byte 0) it sets the object's f34b8
 * timer to $1C and the step to 1. After that it animates the object facing the player with the
 * list at index 2 and counts the timer down; at zero the object enters state 2.
 * HOW: StepFramesFacingPlayer with Y 2, then a decrement of the timer.
 * WAITS: no.
 * Entry: X object. Exit: N, Z from the timer or A 2 at zero; C and Y as the animation step left
 * them (Y 2 if it did not change it).
 */
Ended WaitThenState2(uint8_t slot)
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
    jsr_push(0xB6E1);
    frames = StepFramesFacingPlayer(slot, 2);
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

void abi_WaitThenState2(void)
{
    abi_ended(WaitThenState2(cpu.x));
}

/*
 * @routine 07DB:6EC BecomeType9CWhenAnimationEnds
 * @run 70:B6EC
 * WHAT IT DOES: Animates the object (list 8, facing the player) and, when the animation list has
 * ended, turns it into an object of type $9C in state 0.
 * HOW: StepFramesFacingPlayer with Y 8; Z from it says the list ended.
 * WAITS: no.
 * Entry: X object. Exit: as the animation step, or A $9C and N set, Z clear (C and Y from the step).
 */
Ended BecomeType9CWhenAnimationEnds(uint8_t slot)
{
    FrameStepEnd frames;
    Ended e;

    jsr_push(0xB6F1);
    frames = StepFramesFacingPlayer(slot, 8);
    jsr_pop();
    e = ended_then(ended_with_y(ended_kept(), 8), frames.end);
    if (frames.step != FRAME_LIST_ENDED) {
        return e;
    }
    return ended_then(e, become_type(slot, 0x9C));
}

void abi_BecomeType9CWhenAnimationEnds(void)
{
    abi_ended(BecomeType9CWhenAnimationEnds(cpu.x));
}
