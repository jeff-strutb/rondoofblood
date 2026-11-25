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
    return pick_speed_and_turn(slot, 0xB6E2, 0xB6DA);
}

void abi_PickSpeedAndTurnFromTable(void)
{
    abi_ended(PickSpeedAndTurnFromTable(cpu.x));
}

/*
 * @routine 117C:04A TurnOnRandomBeatWalkC
 * @run 70:B84A
 * @stop B87B
 * WHAT IT DOES: A walk that turns on a random beat: at its first step it turns to the player's side,
 * takes an X speed from a table at overlay address $B87B (two bytes by side), picks life_timer
 * at random from eight bytes at $B87F, stops the animation and moves on; after that it runs
 * TurnOnRandomBeatWalkFollowC.
 * HOW: The shared body; the two tables lie between the two parts and stay data.
 * WAITS: no.
 * Entry: X object, D clear. Exit: first step: A the timer, N, Z from the new step, C and V from
 * NextRandom, Y its index; later as TurnOnRandomBeatWalkFollowC.
 */
Ended TurnOnRandomBeatWalkC(uint8_t slot)
{
    uint8_t phase = RD8(OBJ->f2800[slot]);

    if (phase != 0) {
        return TurnOnRandomBeatWalkFollowC(slot);
    }
    return walk_on_random_beat_first(slot, 0xB84A, 0xB87B, 0xB87F);
}

void abi_TurnOnRandomBeatWalkC(void)
{
    abi_ended(TurnOnRandomBeatWalkC(cpu.x));
}

/*
 * @routine 117C:087 TurnOnRandomBeatWalkFollowC
 * @run 70:B887
 * WHAT IT DOES: The later steps of TurnOnRandomBeatWalkC: animates the object facing the player (list 2),
 * counts life_timer down and moves it along X; at zero it enters state 5 with step 0.
 * HOW: StepFramesFacingPlayerD, a decrement, ApplyVelocityX as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as the animation step, the decrement and ApplyVelocityX leave them
 * (A 5 and N, Z clear at zero).
 */
Ended TurnOnRandomBeatWalkFollowC(uint8_t slot)
{
    return walk_on_random_beat_follow(slot, 0xB88C, StepFramesFacingPlayerD);
}

void abi_TurnOnRandomBeatWalkFollowC(void)
{
    abi_ended(TurnOnRandomBeatWalkFollowC(cpu.x));
}

/*
 * @routine 117C:609 FlashFallAnimateThenRemove
 * @run 70:BE09
 * WHAT IT DOES: Sets the object's flash flags to $8B, moves it along Y, and animates it along the list
 * at overlay address $BE22; when the list has ended the object removes itself.
 * HOW: ApplyVelocityY, StepObjectFrames; ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as StepObjectFrames, V from ApplyVelocityY. Clobbers zTemp10-11.
 */
Ended FlashFallAnimateThenRemove(uint8_t slot)
{
    uint8_t exit_y = 0;
    AxisMove move;
    FrameStep step;
    Ended e;

    WR8(OBJ->flash_flags[slot], 0x8B);
    jsr_push(0xBE11);
    move = ApplyVelocityY(slot);
    jsr_pop();
    WR8(ZP->temp10, 0x22);
    WR8(ZP->temp11, 0xBE);
    jsr_push(0xBE1C);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    e = ended_then(ended_of_axis_move(move), ended_of_frame_step(step, step != FRAME_HELD, exit_y));
    if (step == FRAME_LIST_ENDED) {
        ClearObject(slot);
    }
    return e;
}

void abi_FlashFallAnimateThenRemove(void)
{
    abi_ended(FlashFallAnimateThenRemove(cpu.x));
}

/*
 * @routine 117C:6F1 StartAndMakeNeighbourB
 * @run 70:BEF1
 * WHAT IT DOES: Starts the object from its record, empties the slot after it and starts an object
 * of type $2C there at its place, and sets its own life_timer to $50.
 * HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A $50 and N, Z clear; C from the record; Y the neighbour's slot.
 */
Ended StartAndMakeNeighbourB(uint8_t slot)
{
    uint8_t nz, last, neighbour, value;
    bool carry;
    Ended e;

    jsr_push(0xBEF4);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    neighbour = (uint8_t)(slot + 1);
    e = ended_then(e, ended_with_y(ended_nz(neighbour), neighbour));
    jsr_push(0xBEFA);
    ClearObjectInY(neighbour);
    jsr_pop();
    jsr_push(0xBEFF);
    value = StartObjectAtPlace(0x2C, slot, neighbour);
    jsr_pop();
    e = ended_then(e, ended_load(value));
    WR8(OBJ->life_timer[slot], 0x50);
    return ended_then(e, ended_load(0x50));
}

void abi_StartAndMakeNeighbourB(void)
{
    abi_ended(StartAndMakeNeighbourB(cpu.x));
}

/*
 * @routine 117C:705 EnterNextStateWhenHitOrPlayerNear
 * @run 70:BF05
 * WHAT IT DOES: When the object has taken a hit, or the player is within $68 pixels along X and $18
 * along Y, sets f3670 to 1 and moves the object on to its next state.
 * HOW: DistanceXToPlayer and DistanceYToPlayer compared.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C from the last test; V from the last distance;
 * or A 1 and N, Z from the new state.
 */
Ended EnterNextStateWhenHitOrPlayerNear(uint8_t slot)
{
    uint8_t hit = RD8(OBJ->hit_taken[slot]);
    PlayerDistance distance;
    uint8_t state;
    Ended e = ended_load(hit);

    if (hit == 0) {
        jsr_push(0xBF0D);
        distance = DistanceXToPlayer(slot);
        jsr_pop();
        e = ended_then(ended_then(e, ended_of_distance(distance)), ended_compare(distance.distance, 0x68));
        if (distance.distance >= 0x68) {
            return e;
        }
        jsr_push(0xBF14);
        distance = DistanceYToPlayer(slot);
        jsr_pop();
        e = ended_then(ended_then(e, ended_of_distance(distance)), ended_compare(distance.distance, 0x18));
        if (distance.distance >= 0x18) {
            return e;
        }
    }
    WR8(OBJ->f3670[slot], 1);
    state = INC8(OBJ->state[slot]);
    return ended_then(e, ended_with_a(ended_nz(state), 1));
}

void abi_EnterNextStateWhenHitOrPlayerNear(void)
{
    abi_ended(EnterNextStateWhenHitOrPlayerNear(cpu.x));
}

/*
 * @routine 117C:251 PlaceAtRandomOnEdge
 * @run 70:BA51
 * WHAT IT DOES: Puts the object at a random place chosen from a table of position pairs at overlay
 * address $BA8A: the row group depends on which way the scroll moves (zTemp00 $10, $20 moving right,
 * 0 moving left), the row on three random bits; its Y low byte comes from four bytes at $BABA by two
 * more random bits; its Y high byte is cleared.
 * HOW: NextRandom twice.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the Y byte, N, Z from it; C, V from the second NextRandom;
 * Y its index. Clobbers zTemp00.
 */
Ended PlaceAtRandomOnEdge(uint8_t slot)
{
    enum { PLACES = 0xBA8A, HEIGHTS = 0xBABA };
    uint8_t speed, group, row, y, value;
    RandomResult random;
    Sum sum;

    WR8(ZP->temp00, 0x10);
    speed = RD8(SCROLLE->speed_x[0]);
    if (speed != 0) {
        if ((speed & 0x80) != 0) {
            WR8(ZP->temp00, 0);
        } else {
            WR8(ZP->temp00, 0x20);
        }
    }
    jsr_push(0xBA67);
    random = NextRandom();
    jsr_pop();
    row = (uint8_t)((random.value & 7) << 1);
    group = RD8(ZP->temp00);
    sum = add_bytes(row, group, false);
    y = sum.value;
    WR8(OBJ->x_hi[slot], rd8((uint16_t)(PLACES + 1 + y)));
    WR8(OBJ->x_lo[slot], rd8((uint16_t)(PLACES + y)));
    WR8(OBJ->y_hi[slot], 0);
    jsr_push(0xBA80);
    random = NextRandom();
    jsr_pop();
    y = (uint8_t)(random.value & 3);
    value = rd8((uint16_t)(HEIGHTS + y));
    WR8(OBJ->y_lo[slot], value);
    return ended_with_y(ended_then(ended_of_random(random), ended_load(value)), y);
}

void abi_PlaceAtRandomOnEdge(void)
{
    abi_ended(PlaceAtRandomOnEdge(cpu.x));
}

/*
 * @routine 117C:219 StartFlyerAtRandom
 * @run 70:BA19
 * WHAT IT DOES: Starts the object from its record with flash flags $8F, places it at random
 * (PlaceAtRandomOnEdge), gives it a random f36c8 and a f35c0 from four bytes at overlay address
 * $BA49 by two random bits, turns it to face the player and gives it a Y speed of $FF40.
 * HOW: LoadObjectRecord, PlaceAtRandomOnEdge, NextRandom twice, SideOfPlayer.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A $40 and N, Z clear; C from the side compare; V from the
 * random numbers; Y the table index.
 */
Ended StartFlyerAtRandom(uint8_t slot)
{
    enum { TABLE = 0xBA49 };
    uint8_t nz, last, y, value;
    bool carry;
    RandomResult random;
    SideResult side;
    Ended e;

    jsr_push(0xBA1C);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->flash_flags[slot], 0x8F);
    e = ended_then(e, ended_load(0x8F));
    jsr_push(0xBA24);
    e = ended_then(e, PlaceAtRandomOnEdge(slot));
    jsr_pop();
    jsr_push(0xBA27);
    random = NextRandom();
    jsr_pop();
    WR8(OBJ->f36c8[slot], random.value);
    e = ended_then(e, ended_of_random(random));
    jsr_push(0xBA2D);
    random = NextRandom();
    jsr_pop();
    y = (uint8_t)(random.value & 3);
    value = rd8((uint16_t)(TABLE + y));
    WR8(OBJ->f35c0[slot], value);
    e = ended_then(ended_then(e, ended_of_random(random)), ended_with_y(ended_load(value), y));
    jsr_push(0xBA39);
    side = SideOfPlayer(slot);
    jsr_pop();
    WR8(OBJ->facing_left[slot], (uint8_t)(side.sides & 1));
    e = ended_then(ended_then(e, ended_of_side(side)), ended_load((uint8_t)(side.sides & 1)));
    WR8(OBJ->vy_hi[slot], 0xFF);
    WR8(OBJ->vy_lo[slot], 0x40);
    return ended_then(e, ended_load(0x40));
}

void abi_StartFlyerAtRandom(void)
{
    abi_ended(StartFlyerAtRandom(cpu.x));
}

/*
 * @routine 117C:2BE SwoopOnCosineC
 * @run 70:BABE
 * WHAT IT DOES: Unless the object in slot $54 shows frame 4: gives the object an X speed of
 * its f36c8's angle's cosine (speed $00A0), moves it along X, advances f36c8, moves it by its
 * velocity, and when it is on the first screen down and under $E0 moves it on a state.
 * HOW: ScaleByCosine, ApplyVelocityX, ApplyVelocity.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as the last test, add or compare left them; Y 0 after the scale.
 */
Ended SwoopOnCosineC(uint8_t slot)
{
    uint8_t frame = RD8(OBJ->frame[0x54]);
    uint8_t angle, value, high, low, state;
    AxisMove move;
    Ended e;

    e = ended_compare(frame, 4);
    if (frame == 4) {
        return e;
    }
    WR8(ZP->temp00, 0xA0);
    WR8(ZP->temp01, 0);
    angle = RD8(OBJ->f36c8[slot]);
    jsr_push(0xBAD3);
    ScaleByCosine(angle);
    jsr_pop();
    value = RD8(ZP->product.hi);
    WR8(OBJ->vx_hi[slot], value);
    value = RD8(ZP->product.mid);
    WR8(OBJ->vx_lo[slot], value);
    jsr_push(0xBAE0);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = ended_with_y(ended_of_axis_move(move), 0);
    value = INC8(OBJ->f36c8[slot]);
    e = ended_then(e, ended_nz(value));
    jsr_push(0xBAE6);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = ended_then(e, ended_of_axis_move(move));
    high = RD8(OBJ->y_hi[slot]);
    e = ended_then(e, ended_load(high));
    if (high != 0) {
        return e;
    }
    low = RD8(OBJ->y_lo[slot]);
    e = ended_then(e, ended_compare(low, 0xE0));
    if (low >= 0xE0) {
        return e;
    }
    state = INC8(OBJ->state[slot]);
    return ended_then(e, ended_nz(state));
}

void abi_SwoopOnCosineC(void)
{
    abi_ended(SwoopOnCosineC(cpu.x));
}

/*
 * @routine 117B:560 SpawnSixPiecesB
 * @run 70:B560
 * WHAT IT DOES: Starts up to six objects of type $24 at the object's place, each from a free slot
 * from $30, giving each its life_timer, f3510 and speeds from a four byte entry of a table at
 * overlay address $B596.
 * HOW: The body SpawnSixPiecesA has, with this type and table.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as SpawnSixPiecesA.
 */
Ended SpawnSixPiecesB(uint8_t slot)
{
    return spawn_six_pieces(slot, 0x24, 0xB596, 0xB566, 0xB56D);
}

void abi_SpawnSixPiecesB(void)
{
    abi_ended(SpawnSixPiecesB(cpu.x));
}

/*
 * @routine 117B:53B BurstWhenFlaggedHitB
 * @run 70:B53B
 * WHAT IT DOES: When the object is not yet in state 6 and bit 7 of its status flags is set, plays
 * sound $8B, spawns its six pieces (SpawnSixPiecesB), stops its f3250 and its animation and enters
 * state 6.
 * HOW: Two tests, SoundCommand, SpawnSixPiecesB.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A 6 and N, Z clear, C and V as the piece spawn left them when it
 * burst; otherwise A, N, Z, C from the failing test; X kept.
 */
SoundEnd BurstWhenFlaggedHitB(uint8_t slot, uint8_t entry_y)
{
    SoundEnd result = { ended_kept(), ended_kept(), false, 0, { 0 } };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t bit;
    Ended before;
    Ended e;

    result.end = ended_compare(state, 6);
    if (state == 6) {
        return result;
    }
    bit = (uint8_t)(RD8(OBJ->status_flags[slot]) & 0x80);
    before = ended_then(result.end, ended_load(bit));
    result.end = before;
    if (bit == 0) {
        return result;
    }
    result.sounded = true;
    result.command = 0x8B;
    result.before = ended_then(before, ended_load(0x8B));
    result.end = ended_kept();
    jsr_push(0xB54E);
    result.call = SoundCommand(0x8B, slot, entry_y);
    jsr_pop();
    jsr_push(0xB551);
    e = SpawnSixPiecesB(slot);
    jsr_pop();
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 6);
    result.end = ended_then(e, ended_load(6));
    return result;
}

void abi_BurstWhenFlaggedHitB(void)
{
    abi_sound_end(BurstWhenFlaggedHitB(cpu.x, cpu.y));
}

/*
 * @routine 117C:001 SoundEveryFourthFrameC
 * @run 70:B801
 * WHAT IT DOES: Plays sound $22 on the frames when the low two bits of the object's f3618 are zero.
 * HOW: One test and the call.
 * WAITS: no.
 * Entry: X object. Exit: as SoundEveryFourthFrame.
 */
SoundEnd SoundEveryFourthFrameC(uint8_t slot, uint8_t entry_y)
{
    return sound_every_fourth_frame(slot, entry_y, 0xB80D);
}

void abi_SoundEveryFourthFrameC(void)
{
    abi_sound_end(SoundEveryFourthFrameC(cpu.x, cpu.y));
}

/*
 * @routine 117D:3CB StartFromScreenEdgeByPlayer
 * @run 71:C3CB
 * WHAT IT DOES: Loads the record, then puts the object at one edge of the screen heading across it:
 * when the player's X low byte is $C0 or more, or between $40 and $BF while the player faces
 * left, on the left edge (X $FFE0, speed $0140, facing left set); otherwise on the right (X $0120,
 * speed -$0140). Its Y is the player's less 4.
 * HOW: LoadObjectRecord, two compares, stores, a 16-bit subtract.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the high byte of the new Y, N, Z, C, V from its subtract; Y 11.
 */
Ended StartFromScreenEdgeByPlayer(uint8_t slot)
{
    uint8_t last, nz, player_x;
    bool carry;
    Sum low, high;
    Ended e;

    jsr_push(0xC3CE);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    player_x = RD8(OBJ->x_lo[0]);
    if (player_x >= 0x40 && (player_x >= 0xC0 || RD8(OBJ->facing_left[0]) != 0)) {
        WR8(OBJ->facing_left[slot], 1);
        WR8(OBJ->x_hi[slot], 0xFF);
        WR8(OBJ->x_lo[slot], 0xE0);
        WR8(OBJ->vx_hi[slot], 1);
        WR8(OBJ->vx_lo[slot], 0x40);
    } else {
        WR8(OBJ->x_hi[slot], 1);
        WR8(OBJ->x_lo[slot], 0x20);
        WR8(OBJ->vx_hi[slot], 0xFE);
        WR8(OBJ->vx_lo[slot], 0xC0);
    }
    low = sub_bytes(RD8(OBJ->y_lo[0]), 4, true);
    WR8(OBJ->y_lo[slot], low.value);
    high = sub_bytes(RD8(OBJ->y_hi[0]), 0, low.carry);
    WR8(OBJ->y_hi[slot], high.value);
    return ended_then(e, ended_of_sum(high));
}

void abi_StartFromScreenEdgeByPlayer(void)
{
    abi_ended(StartFromScreenEdgeByPlayer(cpu.x));
}

/*
 * @routine 117B:63D PickStateByPlayerDistanceC
 * @run 70:B63D
 * WHAT IT DOES: PickStateByPlayerDistance in this stage's overlay, with its state table at
 * overlay address $B663.
 * HOW: The shared body. The two early arms (an object off the screen across, or down) ran in no
 * script: the draft stops there, so liftcmp does not judge them.
 * WAITS: no.
 * Entry, exit: as PickStateByPlayerDistance.
 */
Ended PickStateByPlayerDistanceC(uint8_t slot, bool carry_in)
{
    return pick_state_by_distance(slot, carry_in, 0xB663, 0xB659);
}

void abi_PickStateByPlayerDistanceC(void)
{
    abi_ended(PickStateByPlayerDistanceC(cpu.x, (cpu.p & P_C) != 0));
}

/*
 * @routine 1173:20C SpawnType2AThenRestartTimer
 * @run 6C:920C
 * WHAT IT DOES: When its next_kind is 4, or the player's Y low byte is $70 or more, it starts an
 * object of type $2A at its place from a free slot from $30 (and does nothing more when there is
 * none). Then, and when neither held, it sets its f3510 to $C0.
 * HOW: Two tests, FindFreeSlotFrom30, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A $C0 and N set, Z clear, C and Y as the last call left them;
 * with no free slot, as FindFreeSlotFrom30 left them.
 */
Ended SpawnType2AThenRestartTimer(uint8_t slot)
{
    uint8_t kind = RD8(OBJ->next_kind[slot]);
    uint8_t player_y, last, found_slot, value;
    bool found;
    Ended e = ended_compare(kind, 4);

    if (kind != 4) {
        player_y = RD8(OBJ->y_lo[0]);
        e = ended_then(e, ended_compare(player_y, 0x70));
        if (player_y < 0x70) {
            WR8(OBJ->f3510[slot], 0xC0);
            return ended_then(e, ended_load(0xC0));
        }
    }
    jsr_push(0x921D);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = ended_then(e, ended_of_find_slot(found_slot, last, found));
    if (!found) {
        return e;
    }
    jsr_push(0x9224);
    value = StartObjectAtPlace(0x2A, slot, found_slot);
    jsr_pop();
    e = ended_then(e, ended_load(value));
    WR8(OBJ->f3510[slot], 0xC0);
    return ended_then(e, ended_load(0xC0));
}

void abi_SpawnType2AThenRestartTimer(void)
{
    abi_ended(SpawnType2AThenRestartTimer(cpu.x));
}

/*
 * @routine 1173:1D6 StartPieceInSlotsFrom28
 * @run 6C:91D6
 * WHAT IT DOES: When the player's Y low byte is below $70, it advances its f3568 and starts an
 * object at its place in the first free slot of $28 to $2F, of the type a table at overlay
 * address $9204 gives by the low three bits of f3568; nothing happens when the eight slots are
 * taken. Then (and when the player is low) it sets its f3510 to $A0.
 * HOW: A search for a slot of type 0, ClearObjectInY, StartObjectAtPlace. When no slot is free the
 * routine ends at once, without the timer: that arm ran in no script, so the draft stops there and
 * liftcmp does not judge it.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A $A0, N set, Z clear, C and V as StartObjectAtPlace left them,
 * Y the slot; when the player is low, A $A0 and C, N, Z from the compare of the player's Y with $70.
 */
Ended StartPieceInSlotsFrom28(uint8_t slot)
{
    enum { TYPES = 0x9204 };
    uint8_t player_y = RD8(OBJ->y_lo[0]);
    uint8_t count, y, kind, type, value;
    Ended e = ended_compare(player_y, 0x70);

    if (player_y >= 0x70) {
        WR8(OBJ->f3510[slot], 0xA0);
        return ended_then(e, ended_load(0xA0));
    }
    count = INC8(OBJ->f3568[slot]);
    e = ended_then(e, ended_nz(count));
    y = 0x28;
    e = ended_with_y(e, y);
    for (;;) {
        kind = RD8(OBJ->kind[y]);
        e = ended_then(e, ended_load(kind));
        if (kind == 0) {
            break;
        }
        y++;
        e = ended_with_a(ended_with_y(ended_compare(y, 0x30), y), kind);
        if (y >= 0x30) {
            return e;
        }
    }
    jsr_push(0x91F0);
    ClearObjectInY(y);
    jsr_pop();
    push8(y);
    count = RD8(OBJ->f3568[slot]);
    type = rd8((uint16_t)(TYPES + (count & 7)));
    y = pull8();
    e = ended_then(e, ended_with_y(ended_load(type), (uint8_t)(count & 7)));
    e = ended_with_y(e, y);
    e = ended_then(e, ended_nz(y));
    jsr_push(0x91FE);
    value = StartObjectAtPlace(type, slot, y);
    jsr_pop();
    e = ended_then(e, ended_load(value));
    WR8(OBJ->f3510[slot], 0xA0);
    return ended_then(e, ended_load(0xA0));
}

void abi_StartPieceInSlotsFrom28(void)
{
    abi_ended(StartPieceInSlotsFrom28(cpu.x));
}

/*
 * @routine 117C:1EA KnockedUpWhenHit
 * @run 70:B9EA
 * WHAT IT DOES: When bit 7 of the object's status flags is set and its state is below 9, it stops its
 * f3250 and its animation, takes sprite set 3 and frame 0, a Y speed of -$0102 and enters state 9.
 * HOW: Two tests and the stores.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A 9 and N, Z clear when it was knocked up; otherwise A, N, Z, C from
 * the failing test. X, Y, V kept.
 */
Ended KnockedUpWhenHit(uint8_t slot)
{
    return knock_up_when_flagged(slot, 9, 9);
}

void abi_KnockedUpWhenHit(void)
{
    abi_ended(KnockedUpWhenHit(cpu.x));
}

/*
 * @routine 1173:26A KnockedUpWhenHitB
 * @run 6C:926A
 * WHAT IT DOES: KnockedUpWhenHit for a state below 3, entering state 3.
 * HOW: The shared body. Only the first test ran in a script: the draft stops after it, so liftcmp
 * does not judge the rest.
 * WAITS: no.
 * Entry, exit: as KnockedUpWhenHit.
 */
Ended KnockedUpWhenHitB(uint8_t slot)
{
    return knock_up_when_flagged(slot, 3, 3);
}

void abi_KnockedUpWhenHitB(void)
{
    abi_ended(KnockedUpWhenHitB(cpu.x));
}

/*
 * @routine 117C:6CB SetHitboxFromFrameRowsC
 * @run 70:BECB
 * WHAT IT DOES: Gives the object a hit rectangle from a table: the row is picked by its frame less $29 through a
 * byte table at overlay address $C2B9, and the row's four bytes (at $C2D2, four to a row) are its X offset, Y
 * offset, half width and half height.
 * HOW: Two table reads, four stores.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the half height, N, Z from it, C from the row's shift, V from the subtract
 * of $29, Y the row's offset.
 */
Ended SetHitboxFromFrameRowsC(uint8_t slot)
{
    enum { INDEX = 0xC2B9, ROWS = 0xC2D2 };
    Sum frame = sub_bytes(RD8(OBJ->frame[slot]), 0x29, true);
    uint8_t row = rd8((uint16_t)(INDEX + frame.value));
    uint8_t y = (uint8_t)(row << 2);
    uint8_t value;

    WR8(OBJ->hitbox_dx[slot], rd8((uint16_t)(ROWS + y)));
    WR8(OBJ->hitbox_dy[slot], rd8((uint16_t)(ROWS + 1 + y)));
    WR8(OBJ->hitbox_half_w[slot], rd8((uint16_t)(ROWS + 2 + y)));
    value = rd8((uint16_t)(ROWS + 3 + y));
    WR8(OBJ->hitbox_half_h[slot], value);
    return ended_with_overflow(ended_with_carry(ended_with_y(ended_load(value), y), (row & 0x40) != 0),
                               frame.overflow);
}

void abi_SetHitboxFromFrameRowsC(void)
{
    abi_ended(SetHitboxFromFrameRowsC(cpu.x));
}

/*
 * @routine 117D:277 FollowPreviousPiece
 * @run 71:C277
 * WHAT IT DOES: In state 0 it loads its record. Later it takes the place of the object one slot before it,
 * that object's f3250 and facing, and a hit rectangle from a table of four byte rows at overlay address $C2EE
 * picked by that object's frame less $29 (a row beyond 18 is the 18th).
 * HOW: CopyObjectPlaceBack and table reads. The row past the table's end ran in no script: the draft
 * stops there, so liftcmp does not judge it.
 * WAITS: no.
 * Entry: X object, D clear. Exit: state 0: as LoadObjectRecord; otherwise A the facing, N, Z from it, C 0, V from
 * the subtract of $29, Y the row's offset.
 */
Ended FollowPreviousPiece(uint8_t slot)
{
    return follow_previous_piece(slot, 0x29, 0xC2EE, 0xC285);
}

void abi_FollowPreviousPiece(void)
{
    abi_ended(FollowPreviousPiece(cpu.x));
}

/*
 * @routine 117B:737 AnimateMoveThenSetUpNextStepB
 * @run 70:B737
 * WHAT IT DOES: Animates the object facing the player (list 4), moves it, and counts life_timer
 * down; at zero it plays sound $8B, picks a speed and turn (PickSpeedAndTurnFromTable), sets f36c8
 * to $58 plus six bits of PlayerDistanceSum, stops its animation and advances its step.
 * HOW: StepFramesFacingPlayerD, ApplyVelocity, a decrement, then the calls named.
 * WAITS: no.
 * Entry: X object, C in (kept to the sum), D clear. Exit: as AnimateMoveThenSetUpNextStep.
 */
SoundEnd AnimateMoveThenSetUpNextStepB(uint8_t slot, bool carry_in)
{
    SoundEnd result = { ended_kept(), ended_kept(), false, 0, { 0 } };
    FrameStepEnd frames;
    AxisMove move;
    uint8_t left, step;
    Sum sum;
    Ended e;

    jsr_push(0xB73C);
    frames = StepFramesFacingPlayerD(slot, 4);
    jsr_pop();
    e = ended_then(ended_with_y(ended_kept(), 4), frames.end);
    jsr_push(0xB73F);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = ended_then(e, ended_of_axis_move(move));
    left = (uint8_t)(RD8(OBJ->life_timer[slot]) - 1);
    WR8(OBJ->life_timer[slot], left);
    e = ended_then(e, ended_nz(left));
    result.end = e;
    if (left != 0) {
        return result;
    }
    result.sounded = true;
    result.command = 0x8B;
    result.before = ended_then(e, ended_load(0x8B));
    jsr_push(0xB749);
    result.call = SoundCommand(0x8B, slot, ended_y_or(e, 0));
    jsr_pop();
    jsr_push(0xB74C);
    e = PickSpeedAndTurnFromTable(slot);
    jsr_pop();
    jsr_push(0xB74F);
    sum = PlayerDistanceSum(slot, ended_carry_or(e, carry_in));
    jsr_pop();
    sum = add_bytes((uint8_t)(sum.value & 0x3F), 0x58, false);
    WR8(OBJ->f36c8[slot], sum.value);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    step = INC8(OBJ->f2800[slot]);
    result.end = ended_then(e, ended_with_a(ended_with_overflow(ended_with_carry(ended_nz(step), sum.carry),
                                                                sum.overflow), sum.value));
    return result;
}

void abi_AnimateMoveThenSetUpNextStepB(void)
{
    abi_sound_end(AnimateMoveThenSetUpNextStepB(cpu.x, (cpu.p & P_C) != 0));
}

/*
 * @routine 1173:12C LoadObjectRecordFromTable
 * @run 6C:912C
 * WHAT IT DOES: LoadObjectRecord for the objects of this overlay: the twelve byte record of the object's kind is found
 * through a table of addresses at overlay address $AD37 (kind 1 first) and copied into the object's fields; its hit
 * rectangle offsets are cleared and its state advanced.
 * HOW: The pointer into zTemp10-11 and twelve reads through it, as LoadObjectRecord does; the table is the
 * overlay's own, not the one at the record pointer.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the record's last byte, Y 11, N and Z from the new state, C bit 3 of the
 * packed byte; X, V kept.
 */
Ended LoadObjectRecordFromTable(uint8_t slot)
{
    uint8_t byte, index, state, packed;
    bool carry;

    byte = RD8(OBJ->kind[slot]);
    index = (uint8_t)((uint8_t)(byte - 1) << 1);
    WR8(ZP->temp10, rd8((uint16_t)(0xAD37 + index)));
    WR8(ZP->temp11, rd8((uint16_t)(0xAD38 + index)));
    byte = RD8_PTR_Y(ZP->temp10, 0);
    WR8(OBJ->f3358[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 1);
    WR8(OBJ->f3300[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 2);
    WR8(OBJ->f32a8[slot], byte);
    packed = RD8_PTR_Y(ZP->temp10, 3);
    WR8(OBJ->hitbox_on[slot], (uint8_t)(packed & 0x0F));
    packed = RD8_PTR_Y(ZP->temp10, 3);
    carry = (packed & 0x08) != 0;
    WR8(OBJ->fields_2ee0[slot], (uint8_t)(packed >> 4));
    byte = RD8_PTR_Y(ZP->temp10, 4);
    WR8(OBJ->hitbox_half_w[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 5);
    WR8(OBJ->hitbox_half_h[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 6);
    WR8(OBJ->f2a10[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 7);
    WR8(OBJ->frame[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 8);
    WR8(OBJ->f30f0[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 9);
    WR8(OBJ->f3148[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 10);
    WR8(OBJ->status_flags[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 11);
    WR8(OBJ->f31f8[slot], byte);
    WR8(OBJ->hitbox_dx[slot], 0);
    WR8(OBJ->hitbox_dy[slot], 0);
    state = INC8(OBJ->state[slot]);
    return ended_of_record(byte, state, carry);
}

void abi_LoadObjectRecordFromTable(void)
{
    abi_ended(LoadObjectRecordFromTable(cpu.x));
}

/*
 * @routine 117C:15D LaunchWithSoundThenDrift
 * @run 70:B95D
 * WHAT IT DOES: In state 0 it plays sound $25, loads its record, sets its Y speed (high byte) to $F5, moves along Y
 * and takes an X speed from a table of two words at overlay address $B992 by its facing. In any other state it steps
 * the frame list at overlay address $B996 and moves along X.
 * HOW: SoundCommand, LoadObjectRecord, ApplyVelocityY; or StepObjectFrames and ApplyVelocityX as a tail jump.
 * WAITS: no.
 * Entry: X object, Y kept to the frame step, D clear. Exit: state 0: A the X speed's low byte, N, Z from it, C 0,
 * V from ApplyVelocityY, Y the facing index; later A, N, Z, C, V from ApplyVelocityX, Y the step's.
 */
SoundEnd LaunchWithSoundThenDrift(uint8_t slot, uint8_t entry_y)
{
    SoundEnd result = { ended_kept(), ended_kept(), false, 0, { 0 } };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t last, nz, y, value, exit_y, facing;
    bool carry;
    FrameStep step;
    AxisMove move;
    Ended e;

    if (state != 0) {
        WR8(ZP->temp10, 0x96);
        WR8(ZP->temp11, 0xB9);
        exit_y = entry_y;
        jsr_push(0xB98F);
        step = StepObjectFrames(slot, &exit_y);
        jsr_pop();
        e = ended_of_frame_step(step, step != FRAME_HELD, exit_y);
        move = ApplyVelocityX(slot);
        result.end = ended_then(e, ended_of_axis_move(move));
        return result;
    }
    result.sounded = true;
    result.command = 0x25;
    result.before = ended_then(ended_load(state), ended_load(0x25));
    jsr_push(0xB967);
    result.call = SoundCommand(0x25, slot, entry_y);
    jsr_pop();
    jsr_push(0xB96A);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->vy_hi[slot], 0xF5);
    e = ended_then(e, ended_load(0xF5));
    jsr_push(0xB972);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = ended_then(e, ended_of_axis_move(move));
    facing = RD8(OBJ->facing_left[slot]);
    y = (uint8_t)(facing << 1);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB993 + y)));
    value = rd8((uint16_t)(0xB992 + y));
    WR8(OBJ->vx_lo[slot], value);
    result.end = ended_then(e, ended_with_y(ended_with_carry(ended_load(value), (facing & 0x80) != 0), y));
    return result;
}

void abi_LaunchWithSoundThenDrift(void)
{
    abi_sound_end(LaunchWithSoundThenDrift(cpu.x, cpu.y));
}

/*
 * @routine 1173:2E0 SwingOnCosineThenLeapAtPlayer
 * @run 6C:92E0
 * WHAT IT DOES: Steps the frame list at overlay address $9346, gives itself a Y speed of the cosine of its life_timer
 * angle (speed $0040), moves along Y, counts the angle up and moves along X. When the player is nearer than $60
 * along X, one time in 128 (a random byte below 2) it picks a leap from a table of four byte rows at overlay address
 * $934F by three random bits (X low, X high, Y low, Y high), reverses the X speed when it faces left and moves on
 * a state.
 * HOW: StepObjectFrames, ScaleByCosine, ApplyVelocityY, ApplyVelocityX, DistanceXToPlayer, NextRandom twice,
 * NegateVelocityX.
 * WAITS: no.
 * Entry: X object, Y kept to the frame step, D clear. Exit: as the last test, add or call left them: after a leap
 * A, N, Z from the new state.
 */
Ended SwingOnCosineThenLeapAtPlayer(uint8_t slot, uint8_t entry_y)
{
    uint8_t exit_y = entry_y, angle, value, y, facing, state;
    FrameStep step;
    AxisMove move;
    PlayerDistance distance;
    RandomResult first, second;
    Ended e;

    WR8(ZP->temp11, 0x93);
    WR8(ZP->temp10, 0x46);
    jsr_push(0x92EB);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, exit_y);
    WR8(ZP->temp00, 0x40);
    WR8(ZP->temp01, 0);
    angle = RD8(OBJ->life_timer[slot]);
    jsr_push(0x92F9);
    ScaleByCosine(angle);
    jsr_pop();
    value = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], value);
    e = ended_then(e, ended_with_y(ended_load(value), 0));
    value = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], value);
    e = ended_then(e, ended_load(value));
    jsr_push(0x9306);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = ended_then(e, ended_of_axis_move(move));
    value = INC8(OBJ->life_timer[slot]);
    e = ended_then(e, ended_nz(value));
    jsr_push(0x930C);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = ended_then(e, ended_of_axis_move(move));
    jsr_push(0x930F);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = ended_then(ended_then(e, ended_of_distance(distance)), ended_compare(distance.distance, 0x60));
    if (distance.distance >= 0x60) {
        return e;
    }
    jsr_push(0x9316);
    first = NextRandom();
    jsr_pop();
    e = ended_then(ended_then(e, ended_of_random(first)), ended_compare(first.value, 2));
    if (first.value >= 2) {
        return e;
    }
    jsr_push(0x931D);
    second = NextRandom();
    jsr_pop();
    y = (uint8_t)((second.value & 7) << 2);
    e = ended_then(ended_then(e, ended_of_random(second)), ended_with_y(ended_with_carry(ended_load(y), false), y));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0x934F + y)));
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0x9350 + y)));
    WR8(OBJ->vy_lo[slot], rd8((uint16_t)(0x9351 + y)));
    value = rd8((uint16_t)(0x9352 + y));
    WR8(OBJ->vy_hi[slot], value);
    e = ended_then(e, ended_load(value));
    facing = RD8(OBJ->facing_left[slot]);
    e = ended_then(e, ended_load(facing));
    if (facing != 0) {
        jsr_push(0x9342);
        e = ended_then(e, ended_of_sum(NegateVelocityX(slot)));
        jsr_pop();
    }
    state = INC8(OBJ->state[slot]);
    return ended_then(e, ended_nz(state));
}

void abi_SwingOnCosineThenLeapAtPlayer(void)
{
    abi_ended(SwingOnCosineThenLeapAtPlayer(cpu.x, cpu.y));
}

/*
 * @routine 1173:299 StartAtRandomSpeedAndHeight
 * @run 6C:9299
 * WHAT IT DOES: Loads the record (LoadObjectRecordFromTable), takes a random life_timer, an X speed and X position
 * from a table of four byte rows at overlay address $92D4 by one random bit, a Y low byte from a table at $92DC by
 * two more random bits, and its facing from the side of the player.
 * HOW: NextRandom twice, SideOfPlayer.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the side bit and N, Z from it, C from SideOfPlayer's compare (or the second
 * random's add), V from the random; Y the table index of the Y low byte.
 */
Ended StartAtRandomSpeedAndHeight(uint8_t slot)
{
    uint8_t y, bit;
    RandomResult first, second;
    SideResult side;
    Ended e;

    jsr_push(0x929C);
    e = LoadObjectRecordFromTable(slot);
    jsr_pop();
    jsr_push(0x929F);
    first = NextRandom();
    jsr_pop();
    WR8(OBJ->life_timer[slot], first.value);
    y = (uint8_t)((first.value & 1) << 2);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0x92D5 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0x92D4 + y)));
    WR8(OBJ->x_hi[slot], rd8((uint16_t)(0x92D7 + y)));
    WR8(OBJ->x_lo[slot], rd8((uint16_t)(0x92D6 + y)));
    jsr_push(0x92C2);
    second = NextRandom();
    jsr_pop();
    y = (uint8_t)(second.value & 3);
    WR8(OBJ->y_lo[slot], rd8((uint16_t)(0x92DC + y)));
    e = ended_then(e, ended_with_y(ended_of_random(second), y));
    jsr_push(0x92CE);
    side = SideOfPlayer(slot);
    jsr_pop();
    bit = (uint8_t)(side.sides & 1);
    WR8(OBJ->facing_left[slot], bit);
    return ended_then(ended_then(e, ended_of_side(side)), ended_load(bit));
}

void abi_StartAtRandomSpeedAndHeight(void)
{
    abi_ended(StartAtRandomSpeedAndHeight(cpu.x));
}

/*
 * @routine 117B:5AE ChargeWhenPlayerNearB
 * @run 70:B5AE
 * WHAT IT DOES: Animates the object facing the player (list 0). On the first screen across and down, when the player
 * is under $78 pixels away along X it stops its animation, keeps bit 7 of its flags, takes an X speed (high byte) from
 * a table at overlay address $B5F3 by its facing, a Y speed of 2, sets f36c8 to $48 plus five bits of PlayerDistanceSum
 * and enters state 4 with step 2.
 * HOW: StepFramesFacingPlayerD, DistanceXToPlayer, PlayerDistanceSum.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A 2 and N, Z clear, C and V from the add of $48 when it charged; else as the
 * animation step, the position test or the distance compare left them.
 */
Ended ChargeWhenPlayerNearB(uint8_t slot)
{
    FrameStepEnd frames;
    PlayerDistance distance;
    Sum sum;
    uint8_t both, flags, facing, value;
    Ended e;

    jsr_push(0xB5B2);
    frames = StepFramesFacingPlayerD(slot, 0);
    jsr_pop();
    e = ended_then(ended_with_y(ended_kept(), 0), frames.end);
    both = RD8(OBJ->x_hi[slot]);
    both = (uint8_t)(both | RD8(OBJ->y_hi[slot]));
    e = ended_then(e, ended_load(both));
    if (both != 0) {
        return e;
    }
    jsr_push(0xB5BD);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = ended_then(ended_then(e, ended_of_distance(distance)), ended_compare(distance.distance, 0x78));
    if (distance.distance >= 0x78) {
        return e;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    flags = (uint8_t)(RD8(OBJ->flags[slot]) & 0x80);
    WR8(OBJ->flags[slot], flags);
    facing = RD8(OBJ->facing_left[slot]);
    value = rd8((uint16_t)(0xB5F3 + facing));
    WR8(OBJ->vx_hi[slot], value);
    WR8(OBJ->vy_hi[slot], 2);
    jsr_push(0xB5E0);
    sum = PlayerDistanceSum(slot, false);
    jsr_pop();
    sum = add_bytes((uint8_t)(sum.value & 0x1F), 0x48, false);
    WR8(OBJ->f36c8[slot], sum.value);
    WR8(OBJ->state[slot], 4);
    WR8(OBJ->f2800[slot], 2);
    return ended_with_y(ended_then(e, ended_with_overflow(ended_with_carry(ended_load(2), sum.carry), sum.overflow)), facing);
}

void abi_ChargeWhenPlayerNearB(void)
{
    abi_ended(ChargeWhenPlayerNearB(cpu.x));
}
