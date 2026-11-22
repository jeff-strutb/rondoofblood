/*
 * TEMPORARY: the stage 0B overlay's object routines whose enemy kind is not yet identified, kept
 * together here until each kind has a file of its own.
 * The disc tables the routines read are named here by their overlay address until the typed data
 * layer reads them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"

/*
 * @routine 117C:0EA CountdownThenState4B
 * @run 70:B8EA
 * WHAT IT DOES: The same as CountdownThenState4, at this overlay's copy of it.
 * HOW: The shared body; the f3510 timer is counted down and at zero the object enters state 4.
 * WAITS: no.
 * Entry, exit: as CountdownThenState4.
 */
Ended CountdownThenState4B(uint8_t slot)
{
    return countdown_then_state(slot, COL(OBJ->f3510), 4);
}

void abi_CountdownThenState4B(void)
{
    abi_ended(CountdownThenState4B(cpu.x));
}

/*
 * @routine 117C:154 ChangeToType9BB
 * @run 70:B954
 * WHAT IT DOES: The same as ChangeToType9B, at this overlay's copy of it.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as ChangeToType9B.
 */
Ended ChangeToType9BB(uint8_t slot)
{
    return become_type(slot, 0x9B);
}

void abi_ChangeToType9BB(void)
{
    abi_ended(ChangeToType9BB(cpu.x));
}

/*
 * @routine 117C:1C7 SixteenBitTimerToState8
 * @run 70:B9C7
 * WHAT IT DOES: While the object is in state 5, counts its sixteen bit timer (f35c0 low, f3568
 * high) down by one a frame; when the whole timer reaches zero the object enters state 8.
 * HOW: A subtract of one from the low byte with the borrow carried into the high byte; the two
 * are then ORed (the low byte read again) to test for zero.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the last compare, OR or load; C and V from the high
 * byte's subtract when it ran; X and Y kept.
 */
Ended SixteenBitTimerToState8(uint8_t slot)
{
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t low, high, both, new_low, new_high;
    bool borrow;
    Ended e;

    if (state != 5) {
        return ended_compare(state, 5);
    }
    low = RD8(OBJ->f35c0[slot]);
    new_low = (uint8_t)(low - 1);
    borrow = low == 0;
    WR8(OBJ->f35c0[slot], new_low);
    high = RD8(OBJ->f3568[slot]);
    new_high = (uint8_t)(high - (borrow ? 1 : 0));
    WR8(OBJ->f3568[slot], new_high);
    both = RD8(OBJ->f35c0[slot]);
    both = (uint8_t)(new_high | both);
    e = ended_with_overflow(ended_with_carry(ended_load(both), !(high == 0 && borrow)),
                            sub_overflow(high, 0, new_high));
    if (both != 0) {
        return e;
    }
    WR8(OBJ->state[slot], 8);
    return ended_with_overflow(ended_with_carry(ended_load(8), e.c), e.v);
}

void abi_SixteenBitTimerToState8(void)
{
    abi_ended(SixteenBitTimerToState8(cpu.x));
}

/*
 * @routine 117C:46E NoOpStateA
 * @run 70:BC6E
 * WHAT IT DOES: A state handler that does nothing.
 * HOW: Returns at once.
 * WAITS: no.
 * Entry: nothing. Exit: everything kept.
 */
Ended NoOpStateA(void)
{
    return ended_kept();
}

void abi_NoOpStateA(void)
{
    abi_ended(NoOpStateA());
}

/*
 * @routine 117C:4ED SetXSpeedFromFacing
 * @run 70:BCED
 * WHAT IT DOES: Sets the object's X speed from a two byte entry of a table by which way it faces
 * (the facing byte times two).
 * HOW: The table is at overlay address $BD0F: the low byte first, then the high byte.
 * WAITS: no.
 * Entry: X object. Exit: A the low speed byte; N, Z from it; C bit 7 of the facing byte; Y the
 * entry's offset; X kept.
 */
Ended SetXSpeedFromFacing(uint8_t slot)
{
    enum { TABLE = 0xBD0F };
    uint8_t facing = RD8(OBJ->facing_left[slot]);
    uint8_t y = (uint8_t)(facing << 1);
    uint8_t value;

    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(TABLE + 1 + y)));
    value = rd8((uint16_t)(TABLE + y));
    WR8(OBJ->vx_lo[slot], value);
    return ended_with_y(ended_with_carry(ended_load(value), (facing & 0x80) != 0), y);
}

void abi_SetXSpeedFromFacing(void)
{
    abi_ended(SetXSpeedFromFacing(cpu.x));
}

/*
 * @routine 117C:69D EnterState6WhenHitHard
 * @run 70:BE9D
 * WHAT IT DOES: Sends the object to state 6, step 0, when its f3670 is set, its state is under 6
 * and the hit it has taken is 5 or more.
 * HOW: The three tests in that order, each stopping at the first that fails. The last arm ran in
 * no script: the draft stops there; it waits for coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z, C from the last load or compare; X and Y kept.
 */
Ended EnterState6WhenHitHard(uint8_t slot)
{
    uint8_t flag = RD8(OBJ->f3670[slot]);
    uint8_t state, hit;

    if (flag == 0) {
        return ended_load(0);
    }
    state = RD8(OBJ->state[slot]);
    if (state >= 6) {
        return ended_compare(state, 6);
    }
    hit = RD8(OBJ->hit_taken[slot]);
    if (hit < 5) {
        return ended_compare(hit, 5);
    }
    WR8(OBJ->state[slot], 6);
    WR8(OBJ->f2800[slot], 0);
    return ended_load(6);
}

void abi_EnterState6WhenHitHard(void)
{
    abi_ended(EnterState6WhenHitHard(cpu.x));
}

/*
 * @routine 117C:6B9 EnterState7IfYHighOrFlagged
 * @run 70:BEB9
 * WHAT IT DOES: Sends the object to state 7 when its Y position's high byte is not zero or bit 7
 * of its status flags is set.
 * HOW: Tests the high byte, then the flag. The arm that enters the state ran in no script: the
 * draft stops there; it waits for coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object. Exit: A the last value loaded; N, Z from it; X, Y and C kept.
 */
Ended EnterState7IfYHighOrFlagged(uint8_t slot)
{
    uint8_t high = RD8(OBJ->y_hi[slot]);
    uint8_t flags, bit;

    if (high == 0) {
        flags = RD8(OBJ->status_flags[slot]);
        bit = (uint8_t)(flags & 0x80);
        if (bit == 0) {
            return ended_load(0);
        }
    }
    WR8(OBJ->state[slot], 7);
    return ended_load(7);
}

void abi_EnterState7IfYHighOrFlagged(void)
{
    abi_ended(EnterState7IfYHighOrFlagged(cpu.x));
}

/*
 * @routine 117C:7C5 NoOpStateB
 * @run 70:BFC5
 * WHAT IT DOES: A state handler that does nothing.
 * HOW: Returns at once.
 * WAITS: no.
 * Entry: nothing. Exit: everything kept.
 */
Ended NoOpStateB(void)
{
    return ended_kept();
}

void abi_NoOpStateB(void)
{
    abi_ended(NoOpStateB());
}

/*
 * @routine 117C:7FC LaunchHopByFacing
 * @run 70:BFFC
 * WHAT IT DOES: Launches the object in a hop like LaunchHopByF3510, with frame $38, this
 * overlay's table of X speeds at $C01E, indexed by which way the object faces.
 * HOW: The shared hop body.
 * WAITS: no.
 * Entry: X object. Exit: A 8; N, Z from the step byte's new value; Y the facing byte; X and C
 * kept.
 */
Ended LaunchHopByFacing(uint8_t slot)
{
    enum { TABLE = 0xC01E };
    uint8_t facing = RD8(OBJ->facing_left[slot]);

    return hop_from_index(slot, facing, TABLE, 0x38);
}

void abi_LaunchHopByFacing(void)
{
    abi_ended(LaunchHopByFacing(cpu.x));
}

/*
 * @routine 117D:041 CountdownStopThenState2
 * @run 71:C041
 * WHAT IT DOES: Counts the object's f35c0 timer down and, when it reaches zero, stops its
 * animation and sends it to state 2.
 * HOW: One decrement of the timer; the rest only at zero.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the timer, or A 2 at zero; X, Y and C kept.
 */
Ended CountdownStopThenState2(uint8_t slot)
{
    return countdown_stop_then_state(slot, COL(OBJ->f35c0), 2);
}

void abi_CountdownStopThenState2(void)
{
    abi_ended(CountdownStopThenState2(cpu.x));
}

/*
 * @routine 117B:5F5 StepFramesFacingPlayerD
 * @run 70:B5F5
 * WHAT IT DOES: Turns the object to face the player and steps its animation along the list whose
 * pointer is the pair of bytes at overlay address $B60A plus Y (StepFramesFacingPlayerC in this
 * stage's overlay).
 * HOW: SideOfPlayer, the pointer into zTemp10-11, then StepObjectFrames as a tail jump.
 * WAITS: no.
 * Entry: X object, Y the list's offset, D clear. Exit: as StepObjectFrames. Clobbers zTemp00,
 * zTemp10-11.
 */
FrameStepEnd StepFramesFacingPlayerD(uint8_t slot, uint8_t index)
{
    enum { LISTS = 0xB60A };
    uint8_t sides, exit_y = index;
    FrameStepEnd result;

    jsr_push(0xB5F8);
    sides = SideOfPlayer(slot).sides;
    jsr_pop();
    WR8(OBJ->facing_left[slot], (uint8_t)(sides & 1));
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    result.step = StepObjectFrames(slot, &exit_y);
    result.end = ended_of_frame_step(result.step, result.step != FRAME_HELD, exit_y);
    return result;
}

void abi_StepFramesFacingPlayerD(void)
{
    abi_ended(StepFramesFacingPlayerD(cpu.x, cpu.y).end);
}

/*
 * @routine 117C:09D WaitThenState2C
 * @run 70:B89D
 * WHAT IT DOES: A two-step wait like WaitThenState2: the first pass sets f34b8 to $1C and the step
 * to 1; later passes animate the object facing the player (list index 2) and count the timer down;
 * at zero the object enters state 2.
 * HOW: StepFramesFacingPlayerD with Y 2, then a decrement.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as WaitThenState2.
 */
Ended WaitThenState2C(uint8_t slot)
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
    jsr_push(0xB8B0);
    frames = StepFramesFacingPlayerD(slot, 2);
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

void abi_WaitThenState2C(void)
{
    abi_ended(WaitThenState2C(cpu.x));
}

/*
 * @routine 117C:0BB BecomeType9CWhenAnimationEndsB
 * @run 70:B8BB
 * WHAT IT DOES: Animates the object (list 8, facing the player) and, when the animation list has
 * ended, turns it into an object of type $9C in state 0.
 * HOW: StepFramesFacingPlayerD with Y 8; the list ending decides.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as BecomeType9CWhenAnimationEnds.
 */
Ended BecomeType9CWhenAnimationEndsB(uint8_t slot)
{
    FrameStepEnd frames;
    Ended e;

    jsr_push(0xB8C0);
    frames = StepFramesFacingPlayerD(slot, 8);
    jsr_pop();
    e = ended_then(ended_with_y(ended_kept(), 8), frames.end);
    if (frames.step != FRAME_LIST_ENDED) {
        return e;
    }
    return ended_then(e, become_type(slot, 0x9C));
}

void abi_BecomeType9CWhenAnimationEndsB(void)
{
    abi_ended(BecomeType9CWhenAnimationEndsB(cpu.x));
}

/*
 * @routine 117C:0F5 LoadRecordStartAngle40B
 * @run 70:B8F5
 * WHAT IT DOES: LoadRecordStartAngle40, in this stage's overlay: starts the object from its record,
 * moves it by its velocity, and sets its angle's high byte to $40.
 * HOW: LoadObjectRecord, ApplyVelocity, one store.
 * WAITS: no.
 * Entry, exit: as LoadRecordStartAngle40.
 */
Ended LoadRecordStartAngle40B(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;
    AxisMove move;
    Ended e;

    jsr_push(0xB8F8);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    jsr_push(0xB8FB);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = ended_then(e, ended_of_axis_move(move));
    WR8(OBJ->angle_hi[slot], 0x40);
    return ended_then(e, ended_load(0x40));
}

void abi_LoadRecordStartAngle40B(void)
{
    abi_ended(LoadRecordStartAngle40B(cpu.x));
}

/*
 * @routine 117C:3B1 FrameOneBWhenFallingElseAnimate
 * @run 70:BBB1
 * WHAT IT DOES: An object that is not rising (Y speed high byte positive) takes frame $1B; one
 * that is rising animates along the list at overlay address $BBC7.
 * HOW: Tests the speed's sign; StepObjectFrames as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A $1B and N, Z from it when falling; else as StepObjectFrames.
 * Clobbers zTemp10-11.
 */
Ended FrameOneBWhenFallingElseAnimate(uint8_t slot)
{
    uint8_t speed = RD8(OBJ->vy_hi[slot]);
    uint8_t exit_y = 0;
    FrameStep step;

    if ((speed & 0x80) == 0) {
        WR8(OBJ->frame[slot], 0x1B);
        return ended_load(0x1B);
    }
    WR8(ZP->temp10, 0xC7);
    WR8(ZP->temp11, 0xBB);
    step = StepObjectFrames(slot, &exit_y);
    return ended_of_frame_step(step, step != FRAME_HELD, exit_y);
}

void abi_FrameOneBWhenFallingElseAnimate(void)
{
    abi_ended(FrameOneBWhenFallingElseAnimate(cpu.x));
}

/*
 * @routine 117C:684 FaceThePlayerInSomeStatesB
 * @run 70:BE84
 * WHAT IT DOES: FaceThePlayerInSomeStates in this stage's overlay: turns the object to face the
 * player when a table at overlay address $BE95 says its state is one that does.
 * HOW: As FaceThePlayerInSomeStates.
 * WAITS: no.
 * Entry, exit: as FaceThePlayerInSomeStates.
 */
SideEnd FaceThePlayerInSomeStatesB(uint8_t slot)
{
    enum { TABLE = 0xBE95 };
    SideEnd result = { ended_kept(), false, { 0, false, false } };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t entry = rd8((uint16_t)(TABLE + state));
    uint8_t bit;

    result.end = ended_with_y(ended_load(entry), state);
    if (entry == 0) {
        return result;
    }
    jsr_push(0xBE8F);
    result.side = SideOfPlayer(slot);
    jsr_pop();
    result.side_used = true;
    bit = (uint8_t)(result.side.sides & 1);
    WR8(OBJ->facing_left[slot], bit);
    result.end = ended_with_y(ended_load(bit), state);
    return result;
}

void abi_FaceThePlayerInSomeStatesB(void)
{
    abi_side_end(FaceThePlayerInSomeStatesB(cpu.x));
}

/*
 * @routine 117D:3B6 AnimateAlongListC3C2
 * @run 71:C3B6
 * WHAT IT DOES: Steps the object's animation along the list at overlay address $C3C2.
 * HOW: The pointer into zTemp10-11, then StepObjectFrames (a call, then a return).
 * WAITS: no.
 * Entry: X object, D clear. Exit: as StepObjectFrames. Clobbers zTemp10-11.
 */
Ended AnimateAlongListC3C2(uint8_t slot)
{
    uint8_t exit_y = 0;
    FrameStep step;

    WR8(ZP->temp10, 0xC2);
    WR8(ZP->temp11, 0xC3);
    jsr_push(0xC3C1);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    return ended_of_frame_step(step, step != FRAME_HELD, exit_y);
}

void abi_AnimateAlongListC3C2(void)
{
    abi_ended(AnimateAlongListC3C2(cpu.x));
}

/*
 * @routine 117D:41F SwingOnCosineB
 * @run 71:C41F
 * WHAT IT DOES: Gives the object a Y speed of its life_timer's angle's cosine (speed $00A0),
 * moves it, and turns life_timer on by 2.
 * HOW: ScaleByCosine with zTemp00-01 set to $00A0, ApplyVelocity.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the new life_timer, N, Z, C and V from the add of 2; Y 0.
 * Clobbers zTemp00-01, zTemp10-12, zProduct0-2, zMultiplierLo-Hi.
 */
Ended SwingOnCosineB(uint8_t slot)
{
    uint8_t angle, value;
    Sum sum;

    WR8(ZP->temp00, 0xA0);
    WR8(ZP->temp01, 0);
    angle = RD8(OBJ->life_timer[slot]);
    jsr_push(0xC42D);
    ScaleByCosine(angle);
    jsr_pop();
    value = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], value);
    value = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], value);
    jsr_push(0xC43A);
    ApplyVelocity(slot);
    jsr_pop();
    angle = RD8(OBJ->life_timer[slot]);
    sum = add_bytes(angle, 2, false);
    WR8(OBJ->life_timer[slot], sum.value);
    return ended_with_y(ended_of_sum(sum), 0);
}

void abi_SwingOnCosineB(void)
{
    abi_ended(SwingOnCosineB(cpu.x));
}

/*
 * @routine 117B:6B6 PickSpeedAndTurnFromTable
 * @run 70:B6B6
 * WHAT IT DOES: Sets the object's X and Y speeds from a table of four byte entries by its f3670,
 * adds one to f3510, and reverses its X speed when the player is on its left.
 * HOW: The table is at overlay address $B6E2 (X low, X high, Y low, Y high; loads run high byte
 * first); SideOfPlayer; NegateVelocityX as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the side bit (0) or the new X speed's high byte after the
 * negate, N, Z, C and V from it; Y the entry's offset; X kept. Clobbers zTemp00.
 */
Ended PickSpeedAndTurnFromTable(uint8_t slot)
{
    enum { ENTRIES = 0xB6E2 };
    uint8_t row = RD8(OBJ->f3670[slot]);
    uint8_t y = (uint8_t)(row << 2);
    uint8_t value, bit;
    SideResult side;
    Ended e;

    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(ENTRIES + 1 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(ENTRIES + y)));
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(ENTRIES + 3 + y)));
    value = rd8((uint16_t)(ENTRIES + 2 + y));
    WR8(OBJ->vy_lo[slot], value);
    e = ended_with_y(ended_with_carry(ended_load(value), (row & 0x40) != 0), y);
    value = INC8(OBJ->f3510[slot]);
    e = ended_then(e, ended_nz(value));
    jsr_push(0xB6DA);
    side = SideOfPlayer(slot);
    jsr_pop();
    bit = (uint8_t)(side.sides & 1);
    e = ended_then(ended_then(e, ended_of_side(side)), ended_load(bit));
    if (bit == 0) {
        return e;
    }
    return ended_then(e, ended_of_sum(NegateVelocityX(slot)));
}

void abi_PickSpeedAndTurnFromTable(void)
{
    abi_ended(PickSpeedAndTurnFromTable(cpu.x));
}
