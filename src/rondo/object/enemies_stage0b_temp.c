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

/* The arms the converted routines end on (ObjArm, enemy_objects.h); each routine's exit shim builds the registers and flags from them. */
enum { BB_T8_NOT5, BB_T8_RUNNING, BB_T8_STATE8 };
enum { BB_E6_NOFLAG, BB_E6_LATE, BB_E6_MILD, BB_E6_ENTERED };
enum { BB_E7_ENTERED, BB_E7_NONE };
enum { BB_FO_FRAME, BB_FO_STEP };
enum { BB_FP_NONE, BB_FP_FACED };
enum { BB_EN_HIT, BB_EN_X_FAR, BB_EN_Y_FAR, BB_EN_NEAR };
enum { BB_SW_EARLY, BB_SW_HIGH, BB_SW_LOW, BB_SW_NEXT };
enum { BB_ST_LOW, BB_ST_NO_SLOT, BB_ST_SPAWNED };
enum { BB_P28_HIGH, BB_P28_FULL, BB_P28_STARTED };
enum { BB_LW_STEP, BB_LW_LAUNCH };
enum { BB_LP_FAR, BB_LP_ROLL, BB_LP_LEAP };
enum { BS_IN_STATE, BS_CLEAR, BS_BURST };
enum { AN_COUNTING, AN_DONE };
enum { CB_HIGH, CB_FAR, CB_CHARGED };

static uint8_t animate_move_b_sound_y_shim(ObjArm r);
static bool animate_move_b_sum_carry_shim(ObjArm pick, bool carry_in);

/*
 * @routine 117C:0EA CountdownThenState4B
 * @run 70:B8EA
 * WHAT IT DOES: The same as CountdownThenState4, at this overlay's copy of it.
 * HOW: The shared body; the f3510 timer is counted down and at zero the object enters state 4.
 * WAITS: no.
 * Entry, exit: as CountdownThenState4.
 */
ObjArm CountdownThenState4B(uint8_t slot)
{
    return countdown_then_state(slot, COL(OBJ->work_e), 4);
}

void abi_CountdownThenState4B(void)
{
    abi_ended(countdown_exit_shim(CountdownThenState4B(cpu.x)));
}

/*
 * @routine 117C:154 ChangeToType9BB
 * @run 70:B954
 * WHAT IT DOES: The same as ChangeToType9B, at this overlay's copy of it.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as ChangeToType9B.
 */
ObjArm ChangeToType9BB(uint8_t slot)
{
    return become_type(slot, 0x9B);
}

void abi_ChangeToType9BB(void)
{
    abi_ended(load_exit_shim(ChangeToType9BB(cpu.x)));
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
ObjArm SixteenBitTimerToState8(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t low, new_low, both;

    r.first = RD8(OBJ->state[slot]);
    r.arm = BB_T8_NOT5;
    if (r.first != 5) {
        return r;
    }
    low = RD8(OBJ->f35c0[slot]);
    new_low = (uint8_t)(low - 1);
    r.flag = low == 0;
    WR8(OBJ->f35c0[slot], new_low);
    r.second = RD8(OBJ->f3568[slot]);
    r.third = (uint8_t)(r.second - (r.flag ? 1 : 0));
    WR8(OBJ->f3568[slot], r.third);
    both = RD8(OBJ->f35c0[slot]);
    r.fourth = (uint8_t)(r.third | both);
    r.arm = BB_T8_RUNNING;
    if (r.fourth != 0) {
        return r;
    }
    WR8(OBJ->state[slot], 8);
    r.arm = BB_T8_STATE8;
    return r;
}

static Ended sixteen_bit_timer_to_state8_exit_shim(ObjArm r)
{
    bool carry = !(r.second == 0 && r.flag);
    bool overflow = sub_overflow(r.second, 0, r.third);
    Ended e;

    if (r.arm == BB_T8_NOT5) {
        return ended_compare(r.first, 5);
    }
    e = ended_with_overflow(ended_with_carry(ended_load(r.fourth), carry), overflow);
    if (r.arm == BB_T8_RUNNING) {
        return e;
    }
    return ended_with_overflow(ended_with_carry(ended_load(8), carry), overflow);
}

void abi_SixteenBitTimerToState8(void)
{
    abi_ended(sixteen_bit_timer_to_state8_exit_shim(SixteenBitTimerToState8(cpu.x)));
}

/*
 * @routine 117C:46E NoOpStateA
 * @run 70:BC6E
 * WHAT IT DOES: A state handler that does nothing.
 * HOW: Returns at once.
 * WAITS: no.
 * Entry: nothing. Exit: everything kept.
 */
ObjArm NoOpStateA(void)
{
    ObjArm r = { 0 };

    return r;
}

static Ended no_op_state_a_exit_shim(ObjArm r)
{
    (void)r;
    return ended_kept();
}

void abi_NoOpStateA(void)
{
    abi_ended(no_op_state_a_exit_shim(NoOpStateA()));
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
ObjArm SetXSpeedFromFacing(uint8_t slot)
{
    enum { TABLE = 0xBD0F };
    ObjArm r = { 0 };

    r.first = RD8(OBJ->facing_left[slot]);
    r.second = (uint8_t)(r.first << 1);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(TABLE + 1 + r.second)));
    r.third = rd8((uint16_t)(TABLE + r.second));
    WR8(OBJ->vx_lo[slot], r.third);
    return r;
}

static Ended set_x_speed_from_facing_exit_shim(ObjArm r)
{
    return ended_with_y(ended_with_carry(ended_load(r.third), (r.first & 0x80) != 0), r.second);
}

void abi_SetXSpeedFromFacing(void)
{
    abi_ended(set_x_speed_from_facing_exit_shim(SetXSpeedFromFacing(cpu.x)));
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
ObjArm EnterState6WhenHitHard(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->work_i[slot]);
    r.arm = BB_E6_NOFLAG;
    if (r.first == 0) {
        return r;
    }
    r.second = RD8(OBJ->state[slot]);
    r.arm = BB_E6_LATE;
    if (r.second >= 6) {
        return r;
    }
    r.third = RD8(OBJ->hit_taken[slot]);
    r.arm = BB_E6_MILD;
    if (r.third < 5) {
        return r;
    }
    WR8(OBJ->state[slot], 6);
    WR8(OBJ->substate[slot], 0);
    r.arm = BB_E6_ENTERED;
    return r;
}

static Ended enter_state6_when_hit_hard_exit_shim(ObjArm r)
{
    switch (r.arm) {
    case BB_E6_NOFLAG:
        return ended_load(0);
    case BB_E6_LATE:
        return ended_compare(r.second, 6);
    case BB_E6_MILD:
        return ended_compare(r.third, 5);
    default:
        return ended_load(6);
    }
}

void abi_EnterState6WhenHitHard(void)
{
    abi_ended(enter_state6_when_hit_hard_exit_shim(EnterState6WhenHitHard(cpu.x)));
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
ObjArm EnterState7IfYHighOrFlagged(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->y_hi[slot]);
    r.arm = BB_E7_ENTERED;
    if (r.first == 0) {
        r.second = (uint8_t)(RD8(OBJ->hit_status[slot]) & 0x80);
        r.arm = BB_E7_NONE;
        if (r.second == 0) {
            return r;
        }
        r.arm = BB_E7_ENTERED;
    }
    WR8(OBJ->state[slot], 7);
    return r;
}

static Ended enter_state7_if_y_high_or_flagged_exit_shim(ObjArm r)
{
    return r.arm == BB_E7_NONE ? ended_load(0) : ended_load(7);
}

void abi_EnterState7IfYHighOrFlagged(void)
{
    abi_ended(enter_state7_if_y_high_or_flagged_exit_shim(EnterState7IfYHighOrFlagged(cpu.x)));
}

/*
 * @routine 117C:7C5 NoOpStateB
 * @run 70:BFC5
 * WHAT IT DOES: A state handler that does nothing.
 * HOW: Returns at once.
 * WAITS: no.
 * Entry: nothing. Exit: everything kept.
 */
ObjArm NoOpStateB(void)
{
    ObjArm r = { 0 };

    return r;
}

static Ended no_op_state_b_exit_shim(ObjArm r)
{
    (void)r;
    return ended_kept();
}

void abi_NoOpStateB(void)
{
    abi_ended(no_op_state_b_exit_shim(NoOpStateB()));
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
ObjArm LaunchHopByFacing(uint8_t slot)
{
    enum { TABLE = 0xC01E };
    uint8_t facing = RD8(OBJ->facing_left[slot]);

    return hop_from_index(slot, facing, TABLE, 0x38);
}

void abi_LaunchHopByFacing(void)
{
    abi_ended(hop_from_index_exit_shim(LaunchHopByFacing(cpu.x)));
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
ObjArm CountdownStopThenState2(uint8_t slot)
{
    return countdown_stop_then_state(slot, COL(OBJ->f35c0), 2);
}

void abi_CountdownStopThenState2(void)
{
    abi_ended(countdown_exit_shim(CountdownStopThenState2(cpu.x)));
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
ObjArm StepFramesFacingPlayerD(uint8_t slot, uint8_t index)
{
    enum { LISTS = 0xB60A };
    ObjArm r = { 0 };
    uint8_t sides, exit_y = index;

    jsr_push(0xB5F8);
    sides = SideOfPlayer(slot).sides;
    jsr_pop();
    WR8(OBJ->facing_left[slot], (uint8_t)(sides & 1));
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    r.step = StepObjectFrames(slot, &exit_y);
    r.y = exit_y;
    return r;
}

void abi_StepFramesFacingPlayerD(void)
{
    abi_ended(frame_step_exit_shim(StepFramesFacingPlayerD(cpu.x, cpu.y)));
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
ObjArm WaitThenState2C(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t step = RD8(OBJ->substate[slot]);
    uint8_t left;
    ObjArm frames;

    if (step == 0) {
        WR8(OBJ->life_timer[slot], 0x1C);
        step = RD8(OBJ->substate[slot]);
        step = (uint8_t)(step + 1);
        WR8(OBJ->substate[slot], step);
        r.arm = WT_START;
        r.first = step;
        return r;
    }
    jsr_push(0xB8B0);
    frames = StepFramesFacingPlayerD(slot, 2);
    jsr_pop();
    r.step = frames.step;
    r.y = frames.y;
    left = RD8(OBJ->life_timer[slot]);
    left = (uint8_t)(left - 1);
    WR8(OBJ->life_timer[slot], left);
    r.arm = WT_COUNTING;
    r.first = left;
    if (left != 0) {
        return r;
    }
    WR8(OBJ->state[slot], 2);
    r.arm = WT_DONE;
    return r;
}

void abi_WaitThenState2C(void)
{
    abi_ended(wait_then_state2_exit_shim(WaitThenState2C(cpu.x)));
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
ObjArm BecomeType9CWhenAnimationEndsB(uint8_t slot)
{
    ObjArm r;

    jsr_push(0xB8C0);
    r = StepFramesFacingPlayerD(slot, 8);
    jsr_pop();
    r.arm = BT_STEPPED;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    r.first = become_type(slot, 0x9C).first;
    r.arm = BT_BECAME;
    return r;
}

void abi_BecomeType9CWhenAnimationEndsB(void)
{
    abi_ended(become_type_when_animation_ends_exit_shim(BecomeType9CWhenAnimationEndsB(cpu.x)));
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
ObjArm LoadRecordStartAngle40B(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB8F8);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    jsr_push(0xB8FB);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    WR8(OBJ->angle_hi[slot], 0x40);
    return r;
}

static Ended load_record_start_angle40b_exit_shim(ObjArm r)
{
    Ended e = ended_then(record_exit_shim(r), ended_of_axis_move(r.move));

    return ended_then(e, ended_load(0x40));
}

void abi_LoadRecordStartAngle40B(void)
{
    abi_ended(load_record_start_angle40b_exit_shim(LoadRecordStartAngle40B(cpu.x)));
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
ObjArm FrameOneBWhenFallingElseAnimate(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t speed = RD8(OBJ->vy_hi[slot]);
    uint8_t exit_y = 0;

    r.arm = BB_FO_FRAME;
    if ((speed & 0x80) == 0) {
        WR8(OBJ->frame[slot], 0x1B);
        return r;
    }
    WR8(ZP->temp10, 0xC7);
    WR8(ZP->temp11, 0xBB);
    r.step = StepObjectFrames(slot, &exit_y);
    r.y = exit_y;
    r.arm = BB_FO_STEP;
    return r;
}

static Ended frame_one_b_when_falling_else_animate_exit_shim(ObjArm r)
{
    if (r.arm == BB_FO_FRAME) {
        return ended_load(0x1B);
    }
    return frame_step_exit_shim(r);
}

void abi_FrameOneBWhenFallingElseAnimate(void)
{
    abi_ended(frame_one_b_when_falling_else_animate_exit_shim(FrameOneBWhenFallingElseAnimate(cpu.x)));
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
ObjArm FaceThePlayerInSomeStatesB(uint8_t slot)
{
    enum { TABLE = 0xBE95 };
    ObjArm r = { 0 };

    r.first = RD8(OBJ->state[slot]);
    r.second = rd8((uint16_t)(TABLE + r.first));
    r.arm = BB_FP_NONE;
    if (r.second == 0) {
        return r;
    }
    jsr_push(0xBE8F);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.third = (uint8_t)(r.side.sides & 1);
    WR8(OBJ->facing_left[slot], r.third);
    r.arm = BB_FP_FACED;
    return r;
}

static SideEnd face_the_player_in_some_states_b_exit_shim(ObjArm r)
{
    SideEnd s = { ended_kept(), false, { 0, false, false } };

    if (r.arm == BB_FP_NONE) {
        s.end = ended_with_y(ended_load(r.second), r.first);
        return s;
    }
    s.side = r.side;
    s.side_used = true;
    s.end = ended_with_y(ended_load(r.third), r.first);
    return s;
}

void abi_FaceThePlayerInSomeStatesB(void)
{
    abi_side_end(face_the_player_in_some_states_b_exit_shim(FaceThePlayerInSomeStatesB(cpu.x)));
}

/*
 * @routine 117D:3B6 AnimateAlongListC3C2
 * @run 71:C3B6
 * WHAT IT DOES: Steps the object's animation along the list at overlay address $C3C2.
 * HOW: The pointer into zTemp10-11, then StepObjectFrames (a call, then a return).
 * WAITS: no.
 * Entry: X object, D clear. Exit: as StepObjectFrames. Clobbers zTemp10-11.
 */
ObjArm AnimateAlongListC3C2(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t exit_y = 0;

    WR8(ZP->temp10, 0xC2);
    WR8(ZP->temp11, 0xC3);
    jsr_push(0xC3C1);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    return r;
}

void abi_AnimateAlongListC3C2(void)
{
    abi_ended(frame_step_exit_shim(AnimateAlongListC3C2(cpu.x)));
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
ObjArm SwingOnCosineB(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t angle, value;

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
    r.sum = add_bytes(angle, 2, false);
    WR8(OBJ->life_timer[slot], r.sum.value);
    return r;
}

static Ended swing_on_cosine_b_exit_shim(ObjArm r)
{
    return ended_with_y(ended_of_sum(r.sum), 0);
}

void abi_SwingOnCosineB(void)
{
    abi_ended(swing_on_cosine_b_exit_shim(SwingOnCosineB(cpu.x)));
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
ObjArm PickSpeedAndTurnFromTable(uint8_t slot)
{
    return pick_speed_and_turn(slot, 0xB6E2, 0xB6DA);
}

void abi_PickSpeedAndTurnFromTable(void)
{
    abi_ended(pick_speed_and_turn_exit_shim(PickSpeedAndTurnFromTable(cpu.x)));
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
ObjArm TurnOnRandomBeatWalkC(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t phase = RD8(OBJ->substate[slot]);

    if (phase != 0) {
        r.arm = TW_FOLLOW;
        r.sub = obj_sub(TurnOnRandomBeatWalkFollowC(slot));
        return r;
    }
    r.arm = TW_FIRST;
    r.sub = obj_sub(walk_on_random_beat_first(slot, 0xB84A, 0xB87B, 0xB87F));
    return r;
}

void abi_TurnOnRandomBeatWalkC(void)
{
    abi_ended(turn_on_random_beat_walk_exit_shim(TurnOnRandomBeatWalkC(cpu.x)));
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
ObjArm TurnOnRandomBeatWalkFollowC(uint8_t slot)
{
    return walk_on_random_beat_follow(slot, 0xB88C, StepFramesFacingPlayerD);
}

void abi_TurnOnRandomBeatWalkFollowC(void)
{
    abi_ended(walk_on_random_beat_follow_exit_shim(TurnOnRandomBeatWalkFollowC(cpu.x)));
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
ObjArm FlashFallAnimateThenRemove(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t exit_y = 0;

    WR8(OBJ->sprite_attr[slot], 0x8B);
    jsr_push(0xBE11);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    WR8(ZP->temp10, 0x22);
    WR8(ZP->temp11, 0xBE);
    jsr_push(0xBE1C);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    if (r.step == FRAME_LIST_ENDED) {
        ClearObject(slot);
    }
    return r;
}

static Ended flash_fall_animate_then_remove_exit_shim(ObjArm r)
{
    return ended_then(ended_of_axis_move(r.move), frame_step_exit_shim(r));
}

void abi_FlashFallAnimateThenRemove(void)
{
    abi_ended(flash_fall_animate_then_remove_exit_shim(FlashFallAnimateThenRemove(cpu.x)));
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
ObjArm StartAndMakeNeighbourB(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xBEF4);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    r.third = (uint8_t)(slot + 1);
    jsr_push(0xBEFA);
    ClearObjectInY(r.third);
    jsr_pop();
    jsr_push(0xBEFF);
    r.fourth = StartObjectAtPlace(0x2C, slot, r.third);
    jsr_pop();
    WR8(OBJ->life_timer[slot], 0x50);
    return r;
}

static Ended start_and_make_neighbour_b_exit_shim(ObjArm r)
{
    Ended e = ended_then(record_exit_shim(r), ended_with_y(ended_nz(r.third), r.third));

    e = ended_then(e, ended_load(r.fourth));
    return ended_then(e, ended_load(0x50));
}

void abi_StartAndMakeNeighbourB(void)
{
    abi_ended(start_and_make_neighbour_b_exit_shim(StartAndMakeNeighbourB(cpu.x)));
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
ObjArm EnterNextStateWhenHitOrPlayerNear(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->hit_taken[slot]);
    r.arm = BB_EN_HIT;
    if (r.first == 0) {
        jsr_push(0xBF0D);
        r.distance = DistanceXToPlayer(slot);
        jsr_pop();
        r.arm = BB_EN_X_FAR;
        if (r.distance.distance >= 0x68) {
            return r;
        }
        jsr_push(0xBF14);
        r.distance2 = DistanceYToPlayer(slot);
        jsr_pop();
        r.arm = BB_EN_Y_FAR;
        if (r.distance2.distance >= 0x18) {
            return r;
        }
        r.arm = BB_EN_NEAR;
    }
    WR8(OBJ->work_i[slot], 1);
    r.second = INC8(OBJ->state[slot]);
    return r;
}

static Ended enter_next_state_when_hit_or_player_near_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm != BB_EN_HIT) {
        e = ended_then(ended_then(e, ended_of_distance(r.distance)), ended_compare(r.distance.distance, 0x68));
        if (r.arm == BB_EN_X_FAR) {
            return e;
        }
        e = ended_then(ended_then(e, ended_of_distance(r.distance2)), ended_compare(r.distance2.distance, 0x18));
        if (r.arm == BB_EN_Y_FAR) {
            return e;
        }
    }
    return ended_then(e, ended_with_a(ended_nz(r.second), 1));
}

void abi_EnterNextStateWhenHitOrPlayerNear(void)
{
    abi_ended(enter_next_state_when_hit_or_player_near_exit_shim(EnterNextStateWhenHitOrPlayerNear(cpu.x)));
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
ObjArm PlaceAtRandomOnEdge(uint8_t slot)
{
    enum { PLACES = 0xBA8A, HEIGHTS = 0xBABA };
    ObjArm r = { 0 };
    uint8_t speed, group, row, y;
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
    r.random = NextRandom();
    jsr_pop();
    r.second = (uint8_t)(r.random.value & 3);
    r.first = rd8((uint16_t)(HEIGHTS + r.second));
    WR8(OBJ->y_lo[slot], r.first);
    return r;
}

static Ended place_at_random_on_edge_exit_shim(ObjArm r)
{
    return ended_with_y(ended_then(ended_of_random(r.random), ended_load(r.first)), r.second);
}

void abi_PlaceAtRandomOnEdge(void)
{
    abi_ended(place_at_random_on_edge_exit_shim(PlaceAtRandomOnEdge(cpu.x)));
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
ObjArm StartFlyerAtRandom(uint8_t slot)
{
    enum { TABLE = 0xBA49 };
    ObjArm r = { 0 };
    RandomResult random;

    jsr_push(0xBA1C);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    WR8(OBJ->sprite_attr[slot], 0x8F);
    jsr_push(0xBA24);
    r.sub = obj_sub(PlaceAtRandomOnEdge(slot));
    jsr_pop();
    jsr_push(0xBA27);
    r.random = NextRandom();
    jsr_pop();
    WR8(OBJ->work_j[slot], r.random.value);
    jsr_push(0xBA2D);
    r.random2 = NextRandom();
    jsr_pop();
    r.third = (uint8_t)(r.random2.value & 3);
    r.fourth = rd8((uint16_t)(TABLE + r.third));
    WR8(OBJ->f35c0[slot], r.fourth);
    jsr_push(0xBA39);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.fifth = (uint8_t)(r.side.sides & 1);
    WR8(OBJ->facing_left[slot], r.fifth);
    WR8(OBJ->vy_hi[slot], 0xFF);
    WR8(OBJ->vy_lo[slot], 0x40);
    (void)random;
    return r;
}

static Ended start_flyer_at_random_exit_shim(ObjArm r)
{
    Ended e = ended_then(record_exit_shim(r), ended_load(0x8F));

    e = ended_then(e, place_at_random_on_edge_exit_shim(obj_arm(r.sub)));
    e = ended_then(e, ended_of_random(r.random));
    e = ended_then(ended_then(e, ended_of_random(r.random2)), ended_with_y(ended_load(r.fourth), r.third));
    e = ended_then(ended_then(e, ended_of_side(r.side)), ended_load(r.fifth));
    return ended_then(e, ended_load(0x40));
}

void abi_StartFlyerAtRandom(void)
{
    abi_ended(start_flyer_at_random_exit_shim(StartFlyerAtRandom(cpu.x)));
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
ObjArm SwoopOnCosineC(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t angle, value;

    r.first = RD8(OBJ->frame[0x54]);
    r.arm = BB_SW_EARLY;
    if (r.first == 4) {
        return r;
    }
    WR8(ZP->temp00, 0xA0);
    WR8(ZP->temp01, 0);
    angle = RD8(OBJ->work_j[slot]);
    jsr_push(0xBAD3);
    ScaleByCosine(angle);
    jsr_pop();
    value = RD8(ZP->product.hi);
    WR8(OBJ->vx_hi[slot], value);
    value = RD8(ZP->product.mid);
    WR8(OBJ->vx_lo[slot], value);
    jsr_push(0xBAE0);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    r.second = INC8(OBJ->work_j[slot]);
    jsr_push(0xBAE6);
    r.move2 = ApplyVelocity(slot);
    jsr_pop();
    r.third = RD8(OBJ->y_hi[slot]);
    r.arm = BB_SW_HIGH;
    if (r.third != 0) {
        return r;
    }
    r.fourth = RD8(OBJ->y_lo[slot]);
    r.arm = BB_SW_LOW;
    if (r.fourth >= 0xE0) {
        return r;
    }
    r.fifth = INC8(OBJ->state[slot]);
    r.arm = BB_SW_NEXT;
    return r;
}

/* The compare of frame $54 with 4 ends the routine early; the first move's exit replaces it otherwise. */
static Ended swoop_on_cosine_c_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == BB_SW_EARLY) {
        return ended_compare(r.first, 4);
    }
    e = ended_with_y(ended_of_axis_move(r.move), 0);
    e = ended_then(e, ended_nz(r.second));
    e = ended_then(e, ended_of_axis_move(r.move2));
    e = ended_then(e, ended_load(r.third));
    if (r.arm == BB_SW_HIGH) {
        return e;
    }
    e = ended_then(e, ended_compare(r.fourth, 0xE0));
    if (r.arm == BB_SW_LOW) {
        return e;
    }
    return ended_then(e, ended_nz(r.fifth));
}

void abi_SwoopOnCosineC(void)
{
    abi_ended(swoop_on_cosine_c_exit_shim(SwoopOnCosineC(cpu.x)));
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
ObjArm SpawnSixPiecesB(uint8_t slot)
{
    return spawn_six_pieces(slot, 0x24, 0xB596, 0xB566, 0xB56D);
}

void abi_SpawnSixPiecesB(void)
{
    abi_ended(spawn_six_pieces_exit_shim(SpawnSixPiecesB(cpu.x)));
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
ObjArm BurstWhenFlaggedHitB(uint8_t slot, uint8_t entry_y)
{
    ObjArm r = { 0 };
    uint8_t state = RD8(OBJ->state[slot]);

    r.first = state;
    r.arm = BS_IN_STATE;
    if (state == 6) {
        return r;
    }
    r.second = (uint8_t)(RD8(OBJ->hit_status[slot]) & 0x80);
    r.arm = BS_CLEAR;
    if (r.second == 0) {
        return r;
    }
    r.sounded = true;
    r.command = 0x8B;
    jsr_push(0xB54E);
    r.call = SoundCommand(0x8B, slot, entry_y);
    jsr_pop();
    jsr_push(0xB551);
    r.sub = obj_sub(SpawnSixPiecesB(slot));
    jsr_pop();
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 6);
    r.arm = BS_BURST;
    return r;
}

/* The CMP of the state with 6, the AND's A; when it burst, LDA #$8B and the sound, then the spawn's exit and LDA #6. */
static SoundEnd burst_when_flagged_hit_b_exit_shim(ObjArm r)
{
    SoundEnd s = { ended_kept(), ended_kept(), false, 0, { 0 } };
    Ended before = ended_compare(r.first, 6);

    s.end = before;
    if (r.arm == BS_IN_STATE) {
        return s;
    }
    before = ended_then(before, ended_load(r.second));
    s.end = before;
    if (r.arm == BS_CLEAR) {
        return s;
    }
    s.sounded = true;
    s.command = r.command;
    s.before = ended_then(before, ended_load(0x8B));
    s.call = r.call;
    s.end = ended_then(spawn_six_pieces_exit_shim(obj_arm(r.sub)), ended_load(6));
    return s;
}

void abi_BurstWhenFlaggedHitB(void)
{
    abi_sound_end(burst_when_flagged_hit_b_exit_shim(BurstWhenFlaggedHitB(cpu.x, cpu.y)));
}

/*
 * @routine 117C:001 SoundEveryFourthFrameC
 * @run 70:B801
 * WHAT IT DOES: Plays sound $22 on the frames when the low two bits of the object's f3618 are zero.
 * HOW: One test and the call.
 * WAITS: no.
 * Entry: X object. Exit: as SoundEveryFourthFrame.
 */
ObjArm SoundEveryFourthFrameC(uint8_t slot, uint8_t entry_y)
{
    return sound_every_fourth_frame(slot, entry_y, 0xB80D);
}

void abi_SoundEveryFourthFrameC(void)
{
    abi_sound_end(sound_every_fourth_frame_exit_shim(SoundEveryFourthFrameC(cpu.x, cpu.y)));
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
ObjArm StartFromScreenEdgeByPlayer(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t player_x;
    Sum low;

    jsr_push(0xC3CE);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
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
    r.sum = sub_bytes(RD8(OBJ->y_hi[0]), 0, low.carry);
    WR8(OBJ->y_hi[slot], r.sum.value);
    return r;
}

static Ended start_from_screen_edge_by_player_exit_shim(ObjArm r)
{
    return ended_then(record_exit_shim(r), ended_of_sum(r.sum));
}

void abi_StartFromScreenEdgeByPlayer(void)
{
    abi_ended(start_from_screen_edge_by_player_exit_shim(StartFromScreenEdgeByPlayer(cpu.x)));
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
ObjArm PickStateByPlayerDistanceC(uint8_t slot, bool carry_in)
{
    return pick_state_by_distance(slot, carry_in, 0xB663, 0xB659);
}

void abi_PickStateByPlayerDistanceC(void)
{
    abi_ended(pick_state_by_distance_exit_shim(PickStateByPlayerDistanceC(cpu.x, (cpu.p & P_C) != 0)));
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
ObjArm SpawnType2AThenRestartTimer(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t found_slot;
    bool found;

    r.first = RD8(OBJ->start_arg[slot]);
    if (r.first != 4) {
        r.second = RD8(OBJ->y_lo[0]);
        r.arm = BB_ST_LOW;
        if (r.second < 0x70) {
            WR8(OBJ->work_e[slot], 0xC0);
            return r;
        }
    }
    jsr_push(0x921D);
    found_slot = FindFreeSlotFrom30(&r.fourth, &found);
    jsr_pop();
    r.third = found_slot;
    r.flag = found;
    r.arm = BB_ST_NO_SLOT;
    if (!found) {
        return r;
    }
    jsr_push(0x9224);
    r.fifth = StartObjectAtPlace(0x2A, slot, found_slot);
    jsr_pop();
    WR8(OBJ->work_e[slot], 0xC0);
    r.arm = BB_ST_SPAWNED;
    return r;
}

static Ended spawn_type2a_then_restart_timer_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 4);

    if (r.first != 4) {
        e = ended_then(e, ended_compare(r.second, 0x70));
        if (r.arm == BB_ST_LOW) {
            return ended_then(e, ended_load(0xC0));
        }
    }
    e = ended_then(e, ended_of_find_slot(r.third, r.fourth, r.flag));
    if (r.arm == BB_ST_NO_SLOT) {
        return e;
    }
    e = ended_then(e, ended_load(r.fifth));
    return ended_then(e, ended_load(0xC0));
}

void abi_SpawnType2AThenRestartTimer(void)
{
    abi_ended(spawn_type2a_then_restart_timer_exit_shim(SpawnType2AThenRestartTimer(cpu.x)));
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
ObjArm StartPieceInSlotsFrom28(uint8_t slot)
{
    enum { TYPES = 0x9204 };
    ObjArm r = { 0 };
    uint8_t y, kind, count;

    r.first = RD8(OBJ->y_lo[0]);
    r.arm = BB_P28_HIGH;
    if (r.first >= 0x70) {
        WR8(OBJ->work_e[slot], 0xA0);
        return r;
    }
    r.second = INC8(OBJ->f3568[slot]);
    y = 0x28;
    for (;;) {
        kind = RD8(OBJ->kind[y]);
        r.bytes[r.third] = kind;
        r.third++;
        if (kind == 0) {
            break;
        }
        y++;
        if (y >= 0x30) {
            r.arm = BB_P28_FULL;
            return r;
        }
    }
    jsr_push(0x91F0);
    ClearObjectInY(y);
    jsr_pop();
    push8(y);
    count = RD8(OBJ->f3568[slot]);
    r.seventh = (uint8_t)(count & 7);
    r.fifth = rd8((uint16_t)(TYPES + r.seventh));
    y = pull8();
    r.fourth = y;
    jsr_push(0x91FE);
    r.sixth = StartObjectAtPlace(r.fifth, slot, y);
    jsr_pop();
    WR8(OBJ->work_e[slot], 0xA0);
    r.arm = BB_P28_STARTED;
    return r;
}

/* Each pass of the scan loads the slot's kind; a busy slot's INY and CPY #$30 replace everything before them. */
static Ended start_piece_in_slots_from28_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 0x70);
    uint8_t i;

    if (r.arm == BB_P28_HIGH) {
        return ended_then(e, ended_load(0xA0));
    }
    e = ended_with_y(ended_then(e, ended_nz(r.second)), 0x28);
    for (i = 0; i < r.third; i++) {
        e = ended_then(e, ended_load(r.bytes[i]));
        if (r.bytes[i] == 0) {
            break;
        }
        e = ended_with_a(ended_with_y(ended_compare((uint8_t)(0x28 + i + 1), 0x30), (uint8_t)(0x28 + i + 1)), r.bytes[i]);
    }
    if (r.arm == BB_P28_FULL) {
        return e;
    }
    e = ended_then(e, ended_with_y(ended_load(r.fifth), r.seventh));
    e = ended_with_y(e, r.fourth);
    e = ended_then(e, ended_nz(r.fourth));
    e = ended_then(e, ended_load(r.sixth));
    return ended_then(e, ended_load(0xA0));
}

void abi_StartPieceInSlotsFrom28(void)
{
    abi_ended(start_piece_in_slots_from28_exit_shim(StartPieceInSlotsFrom28(cpu.x)));
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
ObjArm KnockedUpWhenHit(uint8_t slot)
{
    return knock_up_when_flagged(slot, 9, 9);
}

void abi_KnockedUpWhenHit(void)
{
    abi_ended(knock_up_when_flagged_exit_shim(KnockedUpWhenHit(cpu.x)));
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
ObjArm KnockedUpWhenHitB(uint8_t slot)
{
    return knock_up_when_flagged(slot, 3, 3);
}

void abi_KnockedUpWhenHitB(void)
{
    abi_ended(knock_up_when_flagged_exit_shim(KnockedUpWhenHitB(cpu.x)));
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
ObjArm SetHitboxFromFrameRowsC(uint8_t slot)
{
    enum { INDEX = 0xC2B9, ROWS = 0xC2D2 };
    ObjArm r = { 0 };

    r.sum = sub_bytes(RD8(OBJ->frame[slot]), 0x29, true);
    r.first = rd8((uint16_t)(INDEX + r.sum.value));
    r.second = (uint8_t)(r.first << 2);
    WR8(OBJ->hitbox_dx[slot], rd8((uint16_t)(ROWS + r.second)));
    WR8(OBJ->hitbox_dy[slot], rd8((uint16_t)(ROWS + 1 + r.second)));
    WR8(OBJ->hitbox_half_w[slot], rd8((uint16_t)(ROWS + 2 + r.second)));
    r.third = rd8((uint16_t)(ROWS + 3 + r.second));
    WR8(OBJ->hitbox_half_h[slot], r.third);
    return r;
}

static Ended set_hitbox_from_frame_rows_c_exit_shim(ObjArm r)
{
    return ended_with_overflow(ended_with_carry(ended_with_y(ended_load(r.third), r.second), (r.first & 0x40) != 0),
                               r.sum.overflow);
}

void abi_SetHitboxFromFrameRowsC(void)
{
    abi_ended(set_hitbox_from_frame_rows_c_exit_shim(SetHitboxFromFrameRowsC(cpu.x)));
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
ObjArm FollowPreviousPiece(uint8_t slot)
{
    return follow_previous_piece(slot, 0x29, 0xC2EE, 0xC285);
}

void abi_FollowPreviousPiece(void)
{
    abi_ended(follow_previous_piece_exit_shim(FollowPreviousPiece(cpu.x)));
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
ObjArm AnimateMoveThenSetUpNextStepB(uint8_t slot, bool carry_in)
{
    ObjArm r;
    ObjArm pick;
    Sum sum;
    uint8_t left;

    jsr_push(0xB73C);
    r = StepFramesFacingPlayerD(slot, 4);
    jsr_pop();
    jsr_push(0xB73F);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    left = (uint8_t)(RD8(OBJ->life_timer[slot]) - 1);
    WR8(OBJ->life_timer[slot], left);
    r.first = left;
    r.arm = AN_COUNTING;
    if (left != 0) {
        return r;
    }
    r.sounded = true;
    r.command = 0x8B;
    jsr_push(0xB749);
    r.call = SoundCommand(0x8B, slot, animate_move_b_sound_y_shim(r));
    jsr_pop();
    jsr_push(0xB74C);
    pick = PickSpeedAndTurnFromTable(slot);
    jsr_pop();
    r.sub = obj_sub(pick);
    jsr_push(0xB74F);
    sum = PlayerDistanceSum(slot, animate_move_b_sum_carry_shim(pick, carry_in));
    jsr_pop();
    r.second = (uint8_t)(sum.value & 0x3F);
    WR8(OBJ->work_j[slot], (uint8_t)(r.second + 0x58));
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.third = INC8(OBJ->substate[slot]);
    r.arm = AN_DONE;
    return r;
}

/* LDY #4 and the step, the move, the DEC's N and Z; at zero LDA #$8B and the sound, then PickSpeedAndTurnFromTable's
 * exit and A the sum ($58 added with CLC), N and Z from the INC, C and V the add's. */
static SoundEnd animate_move_b_exit_shim(ObjArm r)
{
    SoundEnd s = { ended_kept(), ended_kept(), false, 0, { 0 } };
    Ended e = ended_then(ended_then(ldy_frame_step_shim(4, r), ended_of_axis_move(r.move)), ended_nz(r.first));
    Sum sum;

    s.end = e;
    if (r.arm == AN_COUNTING) {
        return s;
    }
    s.sounded = true;
    s.command = r.command;
    s.before = ended_then(e, ended_load(0x8B));
    s.call = r.call;
    sum = add_bytes(r.second, 0x58, false);
    s.end = ended_then(pick_speed_and_turn_exit_shim(obj_arm(r.sub)),
                       ended_with_a(ended_with_overflow(ended_with_carry(ended_nz(r.third), sum.carry), sum.overflow), sum.value));
    return s;
}

/* The Y SoundCommand is entered with: the LDY #4, or the Y the step set. */
static uint8_t animate_move_b_sound_y_shim(ObjArm r)
{
    return ended_y_or(ldy_frame_step_shim(4, r), 0);
}

/* The C PlayerDistanceSum is entered with: what PickSpeedAndTurnFromTable left. */
static bool animate_move_b_sum_carry_shim(ObjArm pick, bool carry_in)
{
    return ended_carry_or(pick_speed_and_turn_exit_shim(pick), carry_in);
}

void abi_AnimateMoveThenSetUpNextStepB(void)
{
    abi_sound_end(animate_move_b_exit_shim(AnimateMoveThenSetUpNextStepB(cpu.x, (cpu.p & P_C) != 0)));
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
ObjArm LoadObjectRecordFromTable(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t byte, index, packed;

    byte = RD8(OBJ->kind[slot]);
    index = (uint8_t)((uint8_t)(byte - 1) << 1);
    WR8(ZP->temp10, rd8((uint16_t)(0xAD37 + index)));
    WR8(ZP->temp11, rd8((uint16_t)(0xAD38 + index)));
    byte = RD8_PTR_Y(ZP->temp10, 0);
    WR8(OBJ->work_c[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 1);
    WR8(OBJ->work_b[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 2);
    WR8(OBJ->health[slot], byte);
    packed = RD8_PTR_Y(ZP->temp10, 3);
    WR8(OBJ->hitbox_on[slot], (uint8_t)(packed & 0x0F));
    packed = RD8_PTR_Y(ZP->temp10, 3);
    r.flag = (packed & 0x08) != 0;
    WR8(OBJ->work_a[slot], (uint8_t)(packed >> 4));
    byte = RD8_PTR_Y(ZP->temp10, 4);
    WR8(OBJ->hitbox_half_w[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 5);
    WR8(OBJ->hitbox_half_h[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 6);
    WR8(OBJ->f2a10[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 7);
    WR8(OBJ->frame[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 8);
    WR8(OBJ->loop_flags[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 9);
    WR8(OBJ->pace[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 10);
    WR8(OBJ->hit_status[slot], byte);
    byte = RD8_PTR_Y(ZP->temp10, 11);
    r.first = byte;
    WR8(OBJ->hit_reaction[slot], byte);
    WR8(OBJ->hitbox_dx[slot], 0);
    WR8(OBJ->hitbox_dy[slot], 0);
    r.second = INC8(OBJ->state[slot]);
    return r;
}

void abi_LoadObjectRecordFromTable(void)
{
    abi_ended(record_exit_shim(LoadObjectRecordFromTable(cpu.x)));
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
ObjArm LaunchWithSoundThenDrift(uint8_t slot, uint8_t entry_y)
{
    ObjArm r = { 0 };
    uint8_t exit_y;

    r.first = RD8(OBJ->state[slot]);
    if (r.first != 0) {
        r.arm = BB_LW_STEP;
        WR8(ZP->temp10, 0x96);
        WR8(ZP->temp11, 0xB9);
        exit_y = entry_y;
        jsr_push(0xB98F);
        r.step = StepObjectFrames(slot, &exit_y);
        jsr_pop();
        r.y = exit_y;
        r.move = ApplyVelocityX(slot);
        return r;
    }
    r.arm = BB_LW_LAUNCH;
    r.sounded = true;
    r.command = 0x25;
    jsr_push(0xB967);
    r.call = SoundCommand(0x25, slot, entry_y);
    jsr_pop();
    jsr_push(0xB96A);
    r.third = LoadObjectRecord(slot, &r.fourth, &r.flag);
    jsr_pop();
    WR8(OBJ->vy_hi[slot], 0xF5);
    jsr_push(0xB972);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.fifth = RD8(OBJ->facing_left[slot]);
    r.sixth = (uint8_t)(r.fifth << 1);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB993 + r.sixth)));
    r.seventh = rd8((uint16_t)(0xB992 + r.sixth));
    WR8(OBJ->vx_lo[slot], r.seventh);
    return r;
}

static SoundEnd launch_with_sound_then_drift_exit_shim(ObjArm r)
{
    SoundEnd s = { ended_kept(), ended_kept(), false, 0, { 0 } };
    Ended e;

    if (r.arm == BB_LW_STEP) {
        s.end = ended_then(frame_step_exit_shim(r), ended_of_axis_move(r.move));
        return s;
    }
    s.sounded = true;
    s.command = r.command;
    s.before = ended_then(ended_load(r.first), ended_load(0x25));
    s.call = r.call;
    e = ended_of_record(r.third, r.fourth, r.flag);
    e = ended_then(e, ended_load(0xF5));
    e = ended_then(e, ended_of_axis_move(r.move));
    s.end = ended_then(e, ended_with_y(ended_with_carry(ended_load(r.seventh), (r.fifth & 0x80) != 0), r.sixth));
    return s;
}

void abi_LaunchWithSoundThenDrift(void)
{
    abi_sound_end(launch_with_sound_then_drift_exit_shim(LaunchWithSoundThenDrift(cpu.x, cpu.y)));
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
ObjArm SwingOnCosineThenLeapAtPlayer(uint8_t slot, uint8_t entry_y)
{
    ObjArm r = { 0 };
    uint8_t exit_y = entry_y, angle;

    WR8(ZP->temp11, 0x93);
    WR8(ZP->temp10, 0x46);
    jsr_push(0x92EB);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    WR8(ZP->temp00, 0x40);
    WR8(ZP->temp01, 0);
    angle = RD8(OBJ->life_timer[slot]);
    jsr_push(0x92F9);
    ScaleByCosine(angle);
    jsr_pop();
    r.first = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], r.first);
    r.second = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], r.second);
    jsr_push(0x9306);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.third = INC8(OBJ->life_timer[slot]);
    jsr_push(0x930C);
    r.move2 = ApplyVelocityX(slot);
    jsr_pop();
    jsr_push(0x930F);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.arm = BB_LP_FAR;
    if (r.distance.distance >= 0x60) {
        return r;
    }
    jsr_push(0x9316);
    r.random = NextRandom();
    jsr_pop();
    r.arm = BB_LP_ROLL;
    if (r.random.value >= 2) {
        return r;
    }
    jsr_push(0x931D);
    r.random2 = NextRandom();
    jsr_pop();
    r.fourth = (uint8_t)((r.random2.value & 7) << 2);
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0x934F + r.fourth)));
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0x9350 + r.fourth)));
    WR8(OBJ->vy_lo[slot], rd8((uint16_t)(0x9351 + r.fourth)));
    r.fifth = rd8((uint16_t)(0x9352 + r.fourth));
    WR8(OBJ->vy_hi[slot], r.fifth);
    r.sixth = RD8(OBJ->facing_left[slot]);
    if (r.sixth != 0) {
        jsr_push(0x9342);
        r.sum = NegateVelocityX(slot);
        jsr_pop();
    }
    r.seventh = INC8(OBJ->state[slot]);
    r.arm = BB_LP_LEAP;
    return r;
}

static Ended swing_on_cosine_then_leap_at_player_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    e = ended_then(e, ended_with_y(ended_load(r.first), 0));
    e = ended_then(e, ended_load(r.second));
    e = ended_then(e, ended_of_axis_move(r.move));
    e = ended_then(e, ended_nz(r.third));
    e = ended_then(e, ended_of_axis_move(r.move2));
    e = ended_then(ended_then(e, ended_of_distance(r.distance)), ended_compare(r.distance.distance, 0x60));
    if (r.arm == BB_LP_FAR) {
        return e;
    }
    e = ended_then(ended_then(e, ended_of_random(r.random)), ended_compare(r.random.value, 2));
    if (r.arm == BB_LP_ROLL) {
        return e;
    }
    e = ended_then(ended_then(e, ended_of_random(r.random2)), ended_with_y(ended_with_carry(ended_load(r.fourth), false), r.fourth));
    e = ended_then(e, ended_load(r.fifth));
    e = ended_then(e, ended_load(r.sixth));
    if (r.sixth != 0) {
        e = ended_then(e, ended_of_sum(r.sum));
    }
    return ended_then(e, ended_nz(r.seventh));
}

void abi_SwingOnCosineThenLeapAtPlayer(void)
{
    abi_ended(swing_on_cosine_then_leap_at_player_exit_shim(SwingOnCosineThenLeapAtPlayer(cpu.x, cpu.y)));
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
ObjArm StartAtRandomSpeedAndHeight(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y;
    RandomResult first;

    jsr_push(0x929C);
    r.sub = obj_sub(LoadObjectRecordFromTable(slot));
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
    r.random = NextRandom();
    jsr_pop();
    r.first = (uint8_t)(r.random.value & 3);
    WR8(OBJ->y_lo[slot], rd8((uint16_t)(0x92DC + r.first)));
    jsr_push(0x92CE);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.second = (uint8_t)(r.side.sides & 1);
    WR8(OBJ->facing_left[slot], r.second);
    return r;
}

static Ended start_at_random_speed_and_height_exit_shim(ObjArm r)
{
    Ended e = ended_then(record_exit_shim(obj_arm(r.sub)), ended_with_y(ended_of_random(r.random), r.first));

    return ended_then(ended_then(e, ended_of_side(r.side)), ended_load(r.second));
}

void abi_StartAtRandomSpeedAndHeight(void)
{
    abi_ended(start_at_random_speed_and_height_exit_shim(StartAtRandomSpeedAndHeight(cpu.x)));
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
ObjArm ChargeWhenPlayerNearB(uint8_t slot)
{
    ObjArm r;
    Sum sum;
    uint8_t both, flags, facing, value;

    jsr_push(0xB5B2);
    r = StepFramesFacingPlayerD(slot, 0);
    jsr_pop();
    both = RD8(OBJ->x_hi[slot]);
    both = (uint8_t)(both | RD8(OBJ->y_hi[slot]));
    r.first = both;
    r.arm = CB_HIGH;
    if (both != 0) {
        return r;
    }
    jsr_push(0xB5BD);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.arm = CB_FAR;
    if (r.distance.distance >= 0x78) {
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    flags = (uint8_t)(RD8(OBJ->scroll_band[slot]) & 0x80);
    WR8(OBJ->scroll_band[slot], flags);
    facing = RD8(OBJ->facing_left[slot]);
    value = rd8((uint16_t)(0xB5F3 + facing));
    WR8(OBJ->vx_hi[slot], value);
    WR8(OBJ->vy_hi[slot], 2);
    jsr_push(0xB5E0);
    sum = PlayerDistanceSum(slot, false);
    jsr_pop();
    r.third = (uint8_t)(sum.value & 0x1F);
    WR8(OBJ->work_j[slot], (uint8_t)(r.third + 0x48));
    WR8(OBJ->state[slot], 4);
    WR8(OBJ->substate[slot], 2);
    r.second = facing;
    r.arm = CB_CHARGED;
    return r;
}

/* LDY #0 and the step, the ORA of the high bytes; the distance and its CMP with $78; when it charged, A 2, C and V from the add of $48, Y the facing. */
static Ended charge_when_player_near_b_exit_shim(ObjArm r)
{
    Ended e = ended_then(ldy_frame_step_shim(0, r), ended_load(r.first));
    Sum sum;

    if (r.arm == CB_HIGH) {
        return e;
    }
    e = ended_then(ended_then(e, ended_of_distance(r.distance)), ended_compare(r.distance.distance, 0x78));
    if (r.arm == CB_FAR) {
        return e;
    }
    sum = add_bytes(r.third, 0x48, false);
    return ended_with_y(ended_then(e, ended_with_overflow(ended_with_carry(ended_load(2), sum.carry), sum.overflow)), r.second);
}

void abi_ChargeWhenPlayerNearB(void)
{
    abi_ended(charge_when_player_near_b_exit_shim(ChargeWhenPlayerNearB(cpu.x)));
}
