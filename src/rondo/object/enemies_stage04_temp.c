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

/*
 * @routine 07DB:384 LoadRecordFacingAndRandomFrame
 * @run 70:B384
 * WHAT IT DOES: Starts the object from its record, makes its facing bit 0 of its next_kind, and
 * gives its animation step a random value from 0 to 15.
 * HOW: LoadObjectRecord, NextRandom masked to a nibble.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the random nibble, N, Z from it; C and V from NextRandom's
 * last add; Y 11; X kept.
 */
Ended LoadRecordFacingAndRandomFrame(uint8_t slot)
{
    uint8_t nz, last, kind, bits;
    bool carry;
    RandomResult random;
    Ended e;

    jsr_push(0xB387);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    kind = RD8(OBJ->next_kind[slot]);
    WR8(OBJ->facing_left[slot], (uint8_t)(kind & 1));
    jsr_push(0xB392);
    random = NextRandom();
    jsr_pop();
    bits = (uint8_t)(random.value & 0x0F);
    WR8(OBJ->anim_step[slot], bits);
    return ended_then(ended_then(e, ended_of_random(random)), ended_load(bits));
}

void abi_LoadRecordFacingAndRandomFrame(void)
{
    abi_ended(LoadRecordFacingAndRandomFrame(cpu.x));
}

/*
 * @routine 07DB:4E7 PickSpeedAndTurnFromTableB
 * @run 70:B4E7
 * WHAT IT DOES: PickSpeedAndTurnFromTable in this stage's overlay: sets the object's X and Y
 * speeds from a table of four byte entries (at overlay address $B513) by its f3670, adds one to
 * f3510, and reverses its X speed when the player is on its left.
 * HOW: SideOfPlayer; NegateVelocityX as a tail jump.
 * WAITS: no.
 * Entry, exit: as PickSpeedAndTurnFromTable.
 */
Ended PickSpeedAndTurnFromTableB(uint8_t slot)
{
    return pick_speed_and_turn(slot, 0xB513, 0xB50B);
}

void abi_PickSpeedAndTurnFromTableB(void)
{
    abi_ended(PickSpeedAndTurnFromTableB(cpu.x));
}

/*
 * @routine 07DB:336 SpawnSixPiecesA
 * @run 70:B336
 * WHAT IT DOES: Starts up to six objects of type $21 at the object's place, each from a free slot
 * from $30, giving each its life_timer, f3510 and speeds from a four byte entry of a table at
 * overlay address $B36C.
 * HOW: A loop over pieces 0 to 5 (kept in zHitSlot, the object in zRichterObject while the
 * table is read): FindFreeSlotFrom30, StartObjectAtPlace, the four stores; it stops when no slot
 * is free.
 * WAITS: no.
 * Entry: X object, D clear. Exit: after six pieces A the last speed byte, N, Z, C from the compare
 * of Y with 6, Y 6; when no slot was free as FindFreeSlotFrom30 left it. X kept. Clobbers
 * zHitSlot, zRichterObject, zTemp10-11 (record reads), and the stage's cleared slots.
 */
Ended SpawnSixPiecesA(uint8_t slot)
{
    enum { TABLE = 0xB36C, TYPE = 0x21 };
    uint8_t piece = 0, found_slot, last, row, value;
    bool found;
    Ended e = ended_with_y(ended_kept(), 0);

    do {
        WR8(ZP->hit_slot, piece);
        jsr_push(0xB33C);
        found_slot = FindFreeSlotFrom30(&last, &found);
        jsr_pop();
        e = ended_then(e, ended_of_find_slot(found_slot, last, found));
        if (!found) {
            return e;
        }
        jsr_push(0xB343);
        StartObjectAtPlace(TYPE, slot, found_slot);
        jsr_pop();
        WR8(ZP->richter_object, slot);
        row = (uint8_t)(RD8(ZP->hit_slot) << 2);
        value = rd8((uint16_t)(TABLE + row));
        WR8(OBJ->life_timer[found_slot], value);
        value = rd8((uint16_t)(TABLE + 1 + row));
        WR8(OBJ->f3510[found_slot], value);
        value = rd8((uint16_t)(TABLE + 2 + row));
        WR8(OBJ->vx_hi[found_slot], value);
        value = rd8((uint16_t)(TABLE + 3 + row));
        WR8(OBJ->vy_hi[found_slot], value);
        slot = RD8(ZP->richter_object);
        piece = RD8(ZP->hit_slot);
        piece++;
        e = ended_with_y(ended_with_carry(ended_with_a(ended_nz((uint8_t)(piece - 6)), value), piece >= 6), piece);
    } while (piece < 6);
    return e;
}

void abi_SpawnSixPiecesA(void)
{
    abi_ended(SpawnSixPiecesA(cpu.x));
}

/*
 * @routine 07DB:73B OrbitStepBack
 * @run 70:B73B
 * WHAT IT DOES: Moves the object on a circle: its velocity from the angle in angle_hi at speed
 * 1.0 (SetVelocityFromAngle), moves it, and turns the angle back by 4; when the angle reaches
 * $E0 it is set to $40 and the object moves on to its next state.
 * HOW: zTemp00-01 = $0100, SetVelocityFromAngle, ApplyVelocity, a subtract.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C from the compare with $E0, or A $40 and N, Z from the
 * new state; V from the subtract; X, Y kept (Y 0 from the scale).
 */
Ended OrbitStepBack(uint8_t slot)
{
    uint8_t angle, state;
    Sum sum;
    Ended e;

    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 1);
    angle = RD8(OBJ->angle_hi[slot]);
    jsr_push(0xB747);
    SetVelocityFromAngle(angle, slot);
    jsr_pop();
    jsr_push(0xB74A);
    ApplyVelocity(slot);
    jsr_pop();
    angle = RD8(OBJ->angle_hi[slot]);
    sum = add_bytes(angle, 0xFB, true);
    WR8(OBJ->angle_hi[slot], sum.value);
    e = ended_with_y(ended_with_overflow(ended_compare(sum.value, 0xE0), sum.overflow), 0);
    if (sum.value != 0xE0) {
        return e;
    }
    WR8(OBJ->angle_hi[slot], 0x40);
    state = INC8(OBJ->state[slot]);
    return ended_then(e, ended_with_a(ended_nz(state), 0x40));
}

void abi_OrbitStepBack(void)
{
    abi_ended(OrbitStepBack(cpu.x));
}

/*
 * @routine 07DB:760 OrbitStepForward
 * @run 70:B760
 * WHAT IT DOES: OrbitStepBack the other way: the angle turns on by 4; at $A0 it is set to $40
 * and the object goes back one state.
 * HOW: As OrbitStepBack with an add and a decrement.
 * WAITS: no.
 * Entry, exit: as OrbitStepBack, the compare being with $A0.
 */
Ended OrbitStepForward(uint8_t slot)
{
    uint8_t angle, state;
    Sum sum;
    Ended e;

    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 1);
    angle = RD8(OBJ->angle_hi[slot]);
    jsr_push(0xB76C);
    SetVelocityFromAngle(angle, slot);
    jsr_pop();
    jsr_push(0xB76F);
    ApplyVelocity(slot);
    jsr_pop();
    angle = RD8(OBJ->angle_hi[slot]);
    sum = add_bytes(angle, 4, false);
    WR8(OBJ->angle_hi[slot], sum.value);
    e = ended_with_y(ended_with_overflow(ended_compare(sum.value, 0xA0), sum.overflow), 0);
    if (sum.value != 0xA0) {
        return e;
    }
    WR8(OBJ->angle_hi[slot], 0x40);
    state = DEC8(OBJ->state[slot]);
    return ended_then(e, ended_with_a(ended_nz(state), 0x40));
}

void abi_OrbitStepForward(void)
{
    abi_ended(OrbitStepForward(cpu.x));
}

/*
 * @routine 07DF:331 StartTypeAndLaunchUp
 * @run 71:D331
 * WHAT IT DOES: Starts a new object of type $9C where this one is (the new slot is in Y), gives
 * it a Y speed of -$10 high byte ($F0) and moves it once by it.
 * HOW: StartObjectAtPlace, a store through Y, then ApplyVelocityY on the new object (X and Y
 * swapped around it), and its Y speed high byte cleared again.
 * WAITS: no.
 * Entry: X object, Y new slot, D clear. Exit: A, N, Z, C, V from ApplyVelocityY; X and Y kept.
 */
Ended StartTypeAndLaunchUp(uint8_t slot, uint8_t fresh)
{
    AxisMove move;

    jsr_push(0xD336);
    StartObjectAtPlace(0x9C, slot, fresh);
    jsr_pop();
    WR8(OBJ->vy_hi[fresh], 0xF0);
    jsr_push(0xD33F);
    move = ApplyVelocityY(fresh);
    jsr_pop();
    WR8(OBJ->vy_hi[fresh], 0);
    return ended_of_axis_move(move);
}

void abi_StartTypeAndLaunchUp(void)
{
    abi_ended(StartTypeAndLaunchUp(cpu.x, cpu.y));
}

/*
 * @routine 07DC:0C8 AnimateThenFireOrWait
 * @run 70:B8C8
 * WHAT IT DOES: Animates the object facing the player (list 2). When the list has ended it
 * stops the animation, enters state 3, counts f3568 up and sets its life_timer from a four byte
 * table at overlay address $B806 by f3568's low two bits. When the frame changed to $32 it starts
 * an object of type $23 at its place in a free slot from $30.
 * HOW: StepFramesFacingPlayerAlways; FindFreeSlotFrom30; StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as the last step that ran (the animation step, a compare, the
 * slot search or the start); Y 2, the table index, or the slot.
 */
Ended AnimateThenFireOrWait(uint8_t slot)
{
    enum { TABLE = 0xB806 };
    FrameStepEnd frames;
    uint8_t frame, count, y, value, found_slot, last;
    bool found;
    Ended e;

    jsr_push(0xB8CD);
    frames = StepFramesFacingPlayerAlways(slot, 2);
    jsr_pop();
    e = ended_then(ended_with_y(ended_kept(), 2), frames.end);
    if (frames.step == FRAME_LIST_ENDED) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 3);
        INC8(OBJ->f3568[slot]);
        count = RD8(OBJ->f3568[slot]);
        y = (uint8_t)(count & 3);
        value = rd8((uint16_t)(TABLE + y));
        WR8(OBJ->life_timer[slot], value);
        return ended_then(e, ended_with_y(ended_load(value), y));
    }
    if (frames.step != FRAME_CHANGED) {
        return e;
    }
    frame = RD8(OBJ->frame[slot]);
    e = ended_then(e, ended_compare(frame, 0x32));
    if (frame != 0x32) {
        return e;
    }
    jsr_push(0xB8DB);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = ended_then(e, ended_of_find_slot(found_slot, last, found));
    if (!found) {
        return e;
    }
    jsr_push(0xB8E2);
    value = StartObjectAtPlace(0x23, slot, found_slot);
    jsr_pop();
    return ended_then(e, ended_load(value));
}

void abi_AnimateThenFireOrWait(void)
{
    abi_ended(AnimateThenFireOrWait(cpu.x));
}

/*
 * @routine 07DC:16B BurstIntoSixPieces
 * @run 70:B96B
 * WHAT IT DOES: Replaces the object by up to six objects of type $24 started at its place in free
 * slots from $30, each facing as this one does and with its life_timer, f3510 and both speed
 * bytes pairs from an eight byte entry of a table at overlay address $B9B6; then removes the
 * object.
 * HOW: A loop over pieces 0 to 5 as SpawnSixPiecesA; ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as the last compare of the piece count (C 1, Z 1, A the last
 * speed byte), or as FindFreeSlotFrom30 left it when no slot was free; Y 6 or $40; X kept.
 */
Ended BurstIntoSixPieces(uint8_t slot)
{
    enum { TABLE = 0xB9B6, TYPE = 0x24 };
    uint8_t piece = 0, found_slot, last, row, value;
    bool found;
    Ended e = ended_with_y(ended_kept(), 0);

    do {
        WR8(ZP->hit_slot, piece);
        jsr_push(0xB971);
        found_slot = FindFreeSlotFrom30(&last, &found);
        jsr_pop();
        e = ended_then(e, ended_of_find_slot(found_slot, last, found));
        if (!found) {
            ClearObject(slot);
            return e;
        }
        jsr_push(0xB978);
        StartObjectAtPlace(TYPE, slot, found_slot);
        jsr_pop();
        value = RD8(OBJ->facing_left[slot]);
        WR8(OBJ->facing_left[found_slot], value);
        WR8(ZP->richter_object, slot);
        row = (uint8_t)(RD8(ZP->hit_slot) << 3);
        value = rd8((uint16_t)(TABLE + row));
        WR8(OBJ->life_timer[found_slot], value);
        value = rd8((uint16_t)(TABLE + 1 + row));
        WR8(OBJ->f3510[found_slot], value);
        value = rd8((uint16_t)(TABLE + 2 + row));
        WR8(OBJ->vx_hi[found_slot], value);
        value = rd8((uint16_t)(TABLE + 3 + row));
        WR8(OBJ->vx_lo[found_slot], value);
        value = rd8((uint16_t)(TABLE + 4 + row));
        WR8(OBJ->vy_hi[found_slot], value);
        value = rd8((uint16_t)(TABLE + 5 + row));
        WR8(OBJ->vy_lo[found_slot], value);
        slot = RD8(ZP->richter_object);
        piece = RD8(ZP->hit_slot);
        piece++;
        e = ended_with_y(ended_with_carry(ended_with_a(ended_nz((uint8_t)(piece - 6)), value), piece >= 6), piece);
    } while (piece < 6);
    ClearObject(slot);
    return e;
}

void abi_BurstIntoSixPieces(void)
{
    abi_ended(BurstIntoSixPieces(cpu.x));
}

/*
 * @routine 07DB:632 SoundEveryFourthFrameB
 * @run 70:B632
 * WHAT IT DOES: Plays sound $22, on the first screen only, on the frames when the low two bits of
 * the object's f3618 are zero.
 * HOW: One test and SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the masked bits and N, Z from them when no sound; else as
 * SoundIfOnFirstScreen leaves it.
 */
ScreenSoundEnd SoundEveryFourthFrameB(uint8_t slot)
{
    ScreenSoundEnd result = { ended_kept(), ended_kept(), false, { 0 } };
    uint8_t bits = RD8(OBJ->f3618[slot]);

    bits = (uint8_t)(bits & 3);
    if (bits != 0) {
        result.before = ended_load(bits);
        return result;
    }
    result.before = ended_load(0x22);
    jsr_push(0xB63E);
    result.sound = SoundIfOnFirstScreen(0x22, slot, 0);   /* Y: not worked out yet */
    jsr_pop();
    result.called = true;
    return result;
}

void abi_SoundEveryFourthFrameB(void)
{
    abi_screen_sound_end(SoundEveryFourthFrameB(cpu.x));
}

/*
 * @routine 07DB:311 BurstWhenFlaggedHit
 * @run 70:B311
 * WHAT IT DOES: When the object is not yet in state 6 and bit 7 of its status flags is set, plays
 * sound $8B (first screen only), spawns its six pieces (SpawnSixPiecesA), stops its f3250 and its
 * animation and enters state 6.
 * HOW: Two tests, SoundIfOnFirstScreen, SpawnSixPiecesA.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A 6 and N, Z clear when it burst, C and V as the piece spawn left
 * them; otherwise A, N, Z, C from the failing test; X kept; Y as the spawn left it.
 */
ScreenSoundEnd BurstWhenFlaggedHit(uint8_t slot)
{
    ScreenSoundEnd result = { ended_kept(), ended_kept(), false, { 0 } };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t flags, bit;
    Ended e;

    result.before = ended_compare(state, 6);
    if (state == 6) {
        return result;
    }
    flags = RD8(OBJ->status_flags[slot]);
    bit = (uint8_t)(flags & 0x80);
    result.before = ended_then(result.before, ended_load(bit));
    if (bit == 0) {
        return result;
    }
    result.before = ended_then(result.before, ended_load(0x8B));
    jsr_push(0xB324);
    result.sound = SoundIfOnFirstScreen(0x8B, slot, 0);   /* Y: not worked out yet */
    jsr_pop();
    result.called = true;
    jsr_push(0xB327);
    e = SpawnSixPiecesA(slot);
    jsr_pop();
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 6);
    result.end = ended_then(e, ended_load(6));
    return result;
}

void abi_BurstWhenFlaggedHit(void)
{
    abi_screen_sound_end(BurstWhenFlaggedHit(cpu.x));
}

/*
 * @routine 07DB:46E PickStateByPlayerDistance
 * @run 70:B46E
 * WHAT IT DOES: Clears the step byte, then: an object off the first screen across goes to state
 * 3; one off it down goes to state 4; one on it takes a state from a table at overlay address
 * $B494 indexed by the low nibble of PlayerDistanceSum.
 * HOW: Two tests of the position's high bytes, then PlayerDistanceSum.
 * WAITS: no.
 * Entry: X object, C in (kept to the sum), D clear. Exit: A the state stored, N, Z from it; C and V
 * from PlayerDistanceSum on the last path; Y the table index there. The second branch ran in no
 * script: the draft stops there, so liftcmp does not judge it.
 */
Ended PickStateByPlayerDistance(uint8_t slot, bool carry_in)
{
    return pick_state_by_distance(slot, carry_in, 0xB494, 0xB48A);
}

void abi_PickStateByPlayerDistance(void)
{
    abi_ended(PickStateByPlayerDistance(cpu.x, (cpu.p & P_C) != 0));
}

/*
 * @routine 07DB:568 AnimateMoveThenSetUpNextStep
 * @run 70:B568
 * WHAT IT DOES: Animates the object facing the player (list 4), moves it, and counts life_timer
 * down; at zero it plays sound $8B (first screen only), picks a speed and turn
 * (PickSpeedAndTurnFromTableB), sets f36c8 to $58 plus six bits of PlayerDistanceSum, stops its
 * animation and advances its step.
 * HOW: StepFramesFacingPlayer, ApplyVelocity, a decrement, then the calls named.
 * WAITS: no.
 * Entry: X object, C in (kept to the sum), D clear. Exit: while counting, N, Z from the timer, C and
 * V from ApplyVelocity, Y as the animation step left it; at zero A the timer value, N, Z from the
 * new step, C and V from the add of $58, Y from the speed pick.
 */
Ended AnimateMoveThenSetUpNextStep(uint8_t slot, bool carry_in)
{
    FrameStepEnd frames;
    AxisMove move;
    uint8_t left, step;
    Sum sum;
    Ended e;

    jsr_push(0xB56D);
    frames = StepFramesFacingPlayer(slot, 4);
    jsr_pop();
    e = ended_then(ended_with_y(ended_kept(), 4), frames.end);
    jsr_push(0xB570);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = ended_then(e, ended_of_axis_move(move));
    left = RD8(OBJ->life_timer[slot]);
    left = (uint8_t)(left - 1);
    WR8(OBJ->life_timer[slot], left);
    e = ended_then(e, ended_nz(left));
    if (left != 0) {
        return e;
    }
    jsr_push(0xB57A);
    SoundIfOnFirstScreen(0x8B, slot, 0);   /* Y: not worked out yet */
    jsr_pop();
    jsr_push(0xB57D);
    e = ended_then(e, PickSpeedAndTurnFromTableB(slot));
    jsr_pop();
    jsr_push(0xB580);
    sum = PlayerDistanceSum(slot, ended_carry_or(e, carry_in));
    jsr_pop();
    sum = add_bytes((uint8_t)(sum.value & 0x3F), 0x58, false);
    WR8(OBJ->f36c8[slot], sum.value);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    step = INC8(OBJ->f2800[slot]);
    return ended_then(e, ended_with_a(ended_with_overflow(ended_with_carry(ended_nz(step), sum.carry),
                                                          sum.overflow), sum.value));
}

void abi_AnimateMoveThenSetUpNextStep(void)
{
    abi_ended(AnimateMoveThenSetUpNextStep(cpu.x, (cpu.p & P_C) != 0));
}

/*
 * @routine 07DB:398 ChargeWhenPlayerNear
 * @run 70:B398
 * WHAT IT DOES: Animates the object facing the player (list 0). When its next_kind bit 7 is
 * clear and the player is under $68 pixels away along X, it stops its animation, keeps bit 7 of
 * its flags, takes an X speed from a table by its facing (at overlay address $B3E7) and Y speed
 * from a table by its next_kind (at $B3E9), sets f36c8 to $48 plus five bits of PlayerDistanceSum,
 * enters state 4 with step 2.
 * HOW: StepFramesFacingPlayer, DistanceXToPlayer, PlayerDistanceSum.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A 2 and N, Z clear, C and V from the add of $48 when it
 * charged; else as the animation step, the next_kind test or the distance compare left them.
 */
Ended ChargeWhenPlayerNear(uint8_t slot)
{
    enum { X_SPEEDS = 0xB3E7, Y_SPEEDS = 0xB3E9 };
    FrameStepEnd frames;
    PlayerDistance distance;
    uint8_t kind, flags, facing, y, value;
    Sum sum;
    Ended e;

    jsr_push(0xB39C);
    frames = StepFramesFacingPlayer(slot, 0);
    jsr_pop();
    e = ended_then(ended_with_y(ended_kept(), 0), frames.end);
    kind = RD8(OBJ->next_kind[slot]);
    e = ended_then(e, ended_load(kind));
    if ((kind & 0x80) != 0) {
        return e;
    }
    jsr_push(0xB3A4);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = ended_then(ended_then(e, ended_of_distance(distance)), ended_compare(distance.distance, 0x68));
    if (distance.distance >= 0x68) {
        return e;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    flags = RD8(OBJ->flags[slot]);
    WR8(OBJ->flags[slot], (uint8_t)(flags & 0x80));
    facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(X_SPEEDS + facing)));
    kind = RD8(OBJ->next_kind[slot]);
    y = (uint8_t)((kind >> 1) << 1);
    WR8(OBJ->vy_lo[slot], rd8((uint16_t)(Y_SPEEDS + y)));
    value = rd8((uint16_t)(Y_SPEEDS + 1 + y));
    WR8(OBJ->vy_hi[slot], value);
    jsr_push(0xB3D4);
    sum = PlayerDistanceSum(slot, false);
    jsr_pop();
    sum = add_bytes((uint8_t)(sum.value & 0x1F), 0x48, false);
    WR8(OBJ->f36c8[slot], sum.value);
    WR8(OBJ->state[slot], 4);
    WR8(OBJ->f2800[slot], 2);
    e = ended_then(e, ended_with_y(ended_with_overflow(ended_with_carry(ended_load(2), sum.carry),
                                                       sum.overflow), y));
    return e;
}

void abi_ChargeWhenPlayerNear(void)
{
    abi_ended(ChargeWhenPlayerNear(cpu.x));
}

/*
 * @routine 07DB:67B TurnOnRandomBeatWalkA
 * @run 70:B67B
 * @stop B6AC
 * WHAT IT DOES: A walk that turns on a random beat: at its first step it turns to the player's side,
 * takes an X speed from a table at overlay address $B6AC (two bytes by side), picks life_timer
 * at random from eight bytes at $B6B0, stops the animation and moves on; after that it runs
 * TurnOnRandomBeatWalkFollowA.
 * HOW: The shared body; the two tables lie between the two parts and stay data.
 * WAITS: no.
 * Entry: X object, D clear. Exit: first step: A the timer, N, Z from the new step, C and V from
 * NextRandom, Y its index; later as TurnOnRandomBeatWalkFollowA.
 */
Ended TurnOnRandomBeatWalkA(uint8_t slot)
{
    uint8_t phase = RD8(OBJ->f2800[slot]);

    if (phase != 0) {
        return TurnOnRandomBeatWalkFollowA(slot);
    }
    return walk_on_random_beat_first(slot, 0xB67B, 0xB6AC, 0xB6B0);
}

void abi_TurnOnRandomBeatWalkA(void)
{
    abi_ended(TurnOnRandomBeatWalkA(cpu.x));
}

/*
 * @routine 07DB:6B8 TurnOnRandomBeatWalkFollowA
 * @run 70:B6B8
 * WHAT IT DOES: The later steps of TurnOnRandomBeatWalkA: animates the object facing the player (list 2),
 * counts life_timer down and moves it along X; at zero it enters state 5 with step 0.
 * HOW: StepFramesFacingPlayer, a decrement, ApplyVelocityX as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as the animation step, the decrement and ApplyVelocityX leave them
 * (A 5 and N, Z clear at zero).
 */
Ended TurnOnRandomBeatWalkFollowA(uint8_t slot)
{
    return walk_on_random_beat_follow(slot, 0xB6BD, StepFramesFacingPlayer);
}

void abi_TurnOnRandomBeatWalkFollowA(void)
{
    abi_ended(TurnOnRandomBeatWalkFollowA(cpu.x));
}
