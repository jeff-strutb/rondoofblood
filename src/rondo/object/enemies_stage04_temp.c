/*
 * TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
 * together here until each kind has a file of its own.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"

/* The arms the converted routines end on (ObjArm, enemy_objects.h); each routine's exit shim builds the registers and flags from them. */
enum { S4_CL_NEG_KEEP, S4_CL_NEG_SET, S4_CL_POS_KEEP, S4_CL_POS_SET };
enum { S4_TU_KEEP, S4_TU_SET };
enum { S4_RO_OTHER, S4_RO_HIGH, S4_RO_GE, S4_RO_LT };
enum { S4_OB_KEEP, S4_OB_RESET };
enum { S4_OF_KEEP, S4_OF_RESET };
enum { S4_BS_DONE, S4_BS_NO_SLOT };
enum { S4_SQ_QUIET, S4_SQ_SOUNDED };
enum { S4_FB_RECORD, S4_FB_BECAME, S4_FB_FALLING };
enum { AF_ENDED, AF_HELD, AF_OTHER_FRAME, AF_NO_SLOT, AF_FIRED };
enum { AM_COUNTING, AM_DONE };
enum { CH_KIND, CH_FAR, CH_CHARGED };
enum { BF_IN_STATE, BF_CLEAR, BF_BURST };

static uint8_t animate_move_sound_y_shim(ObjArm r);
static bool animate_move_sum_carry_shim(ObjArm pick, bool carry_in);

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
ObjArm ClampSpeedXToTwo(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->vx_hi[slot]);
    if ((r.first & 0x80) != 0) {
        r.first = RD8(OBJ->vx_hi[slot]);
        r.arm = S4_CL_NEG_KEEP;
        if (r.first >= 0xFE) {
            return r;
        }
        WR8(OBJ->vx_hi[slot], 0xFE);
        WR8(OBJ->vx_lo[slot], 0);
        r.arm = S4_CL_NEG_SET;
        return r;
    }
    r.first = RD8(OBJ->vx_hi[slot]);
    r.arm = S4_CL_POS_KEEP;
    if (r.first < 2) {
        return r;
    }
    WR8(OBJ->vx_hi[slot], 2);
    WR8(OBJ->vx_lo[slot], 0);
    r.arm = S4_CL_POS_SET;
    return r;
}

static Ended clamp_speed_x_to_two_exit_shim(ObjArm r)
{
    switch (r.arm) {
    case S4_CL_NEG_KEEP:
        return ended_compare(r.first, 0xFE);
    case S4_CL_NEG_SET:
        return ended_with_carry(ended_load(0xFE), false);
    case S4_CL_POS_KEEP:
        return ended_compare(r.first, 2);
    default:
        return ended_with_carry(ended_load(2), true);
    }
}

void abi_ClampSpeedXToTwo(void)
{
    abi_ended(clamp_speed_x_to_two_exit_shim(ClampSpeedXToTwo(cpu.x)));
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
ObjArm ThrowUpward(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->vy_hi[slot]);
    r.arm = S4_TU_KEEP;
    if (r.first >= 0xFE) {
        return r;
    }
    WR8(OBJ->vy_hi[slot], 0xFD);
    WR8(OBJ->vy_lo[slot], 0);
    r.arm = S4_TU_SET;
    return r;
}

static Ended throw_upward_exit_shim(ObjArm r)
{
    if (r.arm == S4_TU_KEEP) {
        return ended_compare(r.first, 0xFE);
    }
    return ended_with_carry(ended_load(0xFD), false);
}

void abi_ThrowUpward(void)
{
    abi_ended(throw_upward_exit_shim(ThrowUpward(cpu.x)));
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
ObjArm CountdownThenState4(uint8_t slot)
{
    return countdown_then_state(slot, COL(OBJ->f3510), 4);
}

void abi_CountdownThenState4(void)
{
    abi_ended(countdown_exit_shim(CountdownThenState4(cpu.x)));
}

/*
 * @routine 07DB:785 ChangeToType9B
 * @run 70:B785
 * WHAT IT DOES: Turns the object into one of type $9B, in state 0.
 * HOW: Stores $9B in its type byte (OBJ_2750) and zero in its state.
 * WAITS: no.
 * Entry: X object. Exit: A $9B, N set, Z clear; X, Y and C kept.
 */
ObjArm ChangeToType9B(uint8_t slot)
{
    return become_type(slot, 0x9B);
}

void abi_ChangeToType9B(void)
{
    abi_ended(load_exit_shim(ChangeToType9B(cpu.x)));
}

/*
 * @routine 07DB:7B1 EnterState6IfFlagged
 * @run 70:B7B1
 * WHAT IT DOES: Sends the object to state 6 when bit 7 of its f31a0 is set.
 * HOW: Reads f31a0 once and masks bit 7; writes the state only when it is set.
 * WAITS: no.
 * Entry: X object. Exit: A the masked bit, or 6 when entered; N, Z from it; X, Y and C kept.
 */
ObjArm EnterState6IfFlagged(uint8_t slot)
{
    return enter_state_if_flagged(slot, 6);
}

void abi_EnterState6IfFlagged(void)
{
    abi_ended(load_exit_shim(EnterState6IfFlagged(cpu.x)));
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
ObjArm CountdownStopThenState4(uint8_t slot)
{
    return countdown_stop_then_state(slot, COL(OBJ->f34b8), 4);
}

void abi_CountdownStopThenState4(void)
{
    abi_ended(countdown_exit_shim(CountdownStopThenState4(cpu.x)));
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
ObjArm RightOfObject4DWithin(uint8_t slot, uint8_t limit)
{
    enum { OTHER = 0x4D, OTHER_TYPE = 0xAC };
    ObjArm r = { 0 };
    uint8_t low, other_low, high, other_high, diff_low;
    bool borrow;

    WR8(ZP->temp00, limit);
    r.first = RD8(OBJ->kind[OTHER]);
    r.arm = S4_RO_OTHER;
    if (r.first != OTHER_TYPE) {
        return r;
    }
    low = RD8(OBJ->x_lo[slot]);
    other_low = RD8(OBJ->x_lo[OTHER]);
    diff_low = (uint8_t)(low - other_low);
    borrow = low < other_low;
    WR8(ZP->temp01, diff_low);
    high = RD8(OBJ->x_hi[slot]);
    other_high = RD8(OBJ->x_hi[OTHER]);
    r.third = (uint8_t)(high - other_high - (borrow ? 1 : 0));
    r.flag = (((high ^ other_high) & (high ^ r.third)) & 0x80) != 0;
    r.arm = S4_RO_HIGH;
    if (r.third != 0) {
        return r;
    }
    r.fourth = RD8(ZP->temp01);
    r.fifth = RD8(ZP->temp00);
    r.arm = r.fourth >= r.fifth ? S4_RO_GE : S4_RO_LT;
    return r;
}

static Ended right_of_object4d_within_exit_shim(ObjArm r)
{
    switch (r.arm) {
    case S4_RO_OTHER:
        return ended_with_carry(ended_compare(r.first, 0xAC), false);
    case S4_RO_HIGH:
        return ended_with_overflow(ended_with_carry(ended_load(r.third), false), r.flag);
    case S4_RO_GE:
        return ended_with_overflow(ended_with_carry(ended_compare(r.fourth, r.fifth), false), r.flag);
    default:
        return ended_with_overflow(ended_with_carry(ended_compare(r.fourth, r.fifth), true), r.flag);
    }
}

void abi_RightOfObject4DWithin(void)
{
    abi_ended(right_of_object4d_within_exit_shim(RightOfObject4DWithin(cpu.x, cpu.a)));
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
ObjArm LoadRecordStartAngle40(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB729);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    jsr_push(0xB72C);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    WR8(OBJ->angle_hi[slot], 0x40);
    return r;
}

static Ended load_record_start_angle40_exit_shim(ObjArm r)
{
    Ended e = ended_then(record_exit_shim(r), ended_of_axis_move(r.move));

    return ended_then(e, ended_load(0x40));
}

void abi_LoadRecordStartAngle40(void)
{
    abi_ended(load_record_start_angle40_exit_shim(LoadRecordStartAngle40(cpu.x)));
}

/*
 * @routine 07DB:7BE LoadRecordTimer50
 * @run 70:B7BE
 * WHAT IT DOES: Starts an object from its record and sets its f34b8 timer to $50.
 * HOW: LoadObjectRecord, one store.
 * WAITS: no.
 * Entry: X object. Exit: A $50, N and Z clear; C from the record; Y 11; X and V kept.
 */
ObjArm LoadRecordTimer50(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB7C1);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    WR8(OBJ->life_timer[slot], 0x50);
    return r;
}

static Ended load_record_timer50_exit_shim(ObjArm r)
{
    return ended_then(record_exit_shim(r), ended_load(0x50));
}

void abi_LoadRecordTimer50(void)
{
    abi_ended(load_record_timer50_exit_shim(LoadRecordTimer50(cpu.x)));
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
ObjArm StepFramesFacingPlayer(uint8_t slot, uint8_t index)
{
    enum { LISTS = 0xB409 };
    ObjArm r = { 0 };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t sides, exit_y = index;

    if (state >= 2) {
        jsr_push(0xB3F7);
        sides = SideOfPlayer(slot).sides;
        jsr_pop();
        WR8(OBJ->facing_left[slot], (uint8_t)(sides & 1));
    }
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    r.step = StepObjectFrames(slot, &exit_y);
    r.y = exit_y;
    return r;
}

void abi_StepFramesFacingPlayer(void)
{
    abi_ended(frame_step_exit_shim(StepFramesFacingPlayer(cpu.x, cpu.y)));
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
ObjArm StepFramesFacingPlayerAlways(uint8_t slot, uint8_t index)
{
    enum { LISTS = 0xB82C };
    ObjArm r = { 0 };
    uint8_t sides, exit_y = index;

    jsr_push(0xB81A);
    sides = SideOfPlayer(slot).sides;
    jsr_pop();
    WR8(OBJ->facing_left[slot], (uint8_t)(sides & 1));
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    r.step = StepObjectFrames(slot, &exit_y);
    r.y = exit_y;
    return r;
}

void abi_StepFramesFacingPlayerAlways(void)
{
    abi_ended(frame_step_exit_shim(StepFramesFacingPlayerAlways(cpu.x, cpu.y)));
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
ObjArm WalkWhileFrameChanges(uint8_t slot)
{
    enum { SPEEDS = 0xB804 };
    ObjArm r;
    uint8_t phase;

    jsr_push(0xB7F5);
    r = StepFramesFacingPlayerAlways(slot, 0);
    jsr_pop();
    r.arm = WW_HELD;
    if (r.step != FRAME_CHANGED) {
        return r;
    }
    phase = RD8(OBJ->f3510[slot]);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(SPEEDS + phase)));
    jsr_push(0xB803);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    r.arm = WW_MOVED;
    r.first = phase;
    return r;
}

void abi_WalkWhileFrameChanges(void)
{
    abi_ended(walk_while_frame_changes_exit_shim(WalkWhileFrameChanges(cpu.x)));
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
ObjArm WaitThenState2(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t step = RD8(OBJ->f2800[slot]);
    uint8_t left;
    ObjArm frames;

    if (step == 0) {
        WR8(OBJ->life_timer[slot], 0x1C);
        step = RD8(OBJ->f2800[slot]);
        step = (uint8_t)(step + 1);
        WR8(OBJ->f2800[slot], step);
        r.arm = WT_START;
        r.first = step;
        return r;
    }
    jsr_push(0xB6E1);
    frames = StepFramesFacingPlayer(slot, 2);
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

void abi_WaitThenState2(void)
{
    abi_ended(wait_then_state2_exit_shim(WaitThenState2(cpu.x)));
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
ObjArm BecomeType9CWhenAnimationEnds(uint8_t slot)
{
    ObjArm r;

    jsr_push(0xB6F1);
    r = StepFramesFacingPlayer(slot, 8);
    jsr_pop();
    r.arm = BT_STEPPED;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    r.first = become_type(slot, 0x9C).first;
    r.arm = BT_BECAME;
    return r;
}

void abi_BecomeType9CWhenAnimationEnds(void)
{
    abi_ended(become_type_when_animation_ends_exit_shim(BecomeType9CWhenAnimationEnds(cpu.x)));
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
ObjArm LoadRecordFacingAndRandomFrame(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t kind;

    jsr_push(0xB387);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    kind = RD8(OBJ->next_kind[slot]);
    WR8(OBJ->facing_left[slot], (uint8_t)(kind & 1));
    jsr_push(0xB392);
    r.random = NextRandom();
    jsr_pop();
    r.third = (uint8_t)(r.random.value & 0x0F);
    WR8(OBJ->anim_step[slot], r.third);
    return r;
}

static Ended load_record_facing_and_random_frame_exit_shim(ObjArm r)
{
    return ended_then(ended_then(record_exit_shim(r), ended_of_random(r.random)), ended_load(r.third));
}

void abi_LoadRecordFacingAndRandomFrame(void)
{
    abi_ended(load_record_facing_and_random_frame_exit_shim(LoadRecordFacingAndRandomFrame(cpu.x)));
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
ObjArm PickSpeedAndTurnFromTableB(uint8_t slot)
{
    return pick_speed_and_turn(slot, 0xB513, 0xB50B);
}

void abi_PickSpeedAndTurnFromTableB(void)
{
    abi_ended(pick_speed_and_turn_exit_shim(PickSpeedAndTurnFromTableB(cpu.x)));
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
ObjArm SpawnSixPiecesA(uint8_t slot)
{
    return spawn_six_pieces(slot, 0x21, 0xB36C, 0xB33C, 0xB343);
}

void abi_SpawnSixPiecesA(void)
{
    abi_ended(spawn_six_pieces_exit_shim(SpawnSixPiecesA(cpu.x)));
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
ObjArm OrbitStepBack(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t angle;

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
    r.sum = add_bytes(angle, 0xFB, true);
    WR8(OBJ->angle_hi[slot], r.sum.value);
    r.arm = S4_OB_KEEP;
    if (r.sum.value != 0xE0) {
        return r;
    }
    WR8(OBJ->angle_hi[slot], 0x40);
    r.first = INC8(OBJ->state[slot]);
    r.arm = S4_OB_RESET;
    return r;
}

static Ended orbit_step_back_exit_shim(ObjArm r)
{
    Ended e = ended_with_y(ended_with_overflow(ended_compare(r.sum.value, 0xE0), r.sum.overflow), 0);

    if (r.arm == S4_OB_KEEP) {
        return e;
    }
    return ended_then(e, ended_with_a(ended_nz(r.first), 0x40));
}

void abi_OrbitStepBack(void)
{
    abi_ended(orbit_step_back_exit_shim(OrbitStepBack(cpu.x)));
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
ObjArm OrbitStepForward(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t angle;

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
    r.sum = add_bytes(angle, 4, false);
    WR8(OBJ->angle_hi[slot], r.sum.value);
    r.arm = S4_OF_KEEP;
    if (r.sum.value != 0xA0) {
        return r;
    }
    WR8(OBJ->angle_hi[slot], 0x40);
    r.first = DEC8(OBJ->state[slot]);
    r.arm = S4_OF_RESET;
    return r;
}

static Ended orbit_step_forward_exit_shim(ObjArm r)
{
    Ended e = ended_with_y(ended_with_overflow(ended_compare(r.sum.value, 0xA0), r.sum.overflow), 0);

    if (r.arm == S4_OF_KEEP) {
        return e;
    }
    return ended_then(e, ended_with_a(ended_nz(r.first), 0x40));
}

void abi_OrbitStepForward(void)
{
    abi_ended(orbit_step_forward_exit_shim(OrbitStepForward(cpu.x)));
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
ObjArm StartTypeAndLaunchUp(uint8_t slot, uint8_t fresh)
{
    ObjArm r = { 0 };

    jsr_push(0xD336);
    StartObjectAtPlace(0x9C, slot, fresh);
    jsr_pop();
    WR8(OBJ->vy_hi[fresh], 0xF0);
    jsr_push(0xD33F);
    r.move = ApplyVelocityY(fresh);
    jsr_pop();
    WR8(OBJ->vy_hi[fresh], 0);
    return r;
}

static Ended start_type_and_launch_up_exit_shim(ObjArm r)
{
    return ended_of_axis_move(r.move);
}

void abi_StartTypeAndLaunchUp(void)
{
    abi_ended(start_type_and_launch_up_exit_shim(StartTypeAndLaunchUp(cpu.x, cpu.y)));
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
ObjArm AnimateThenFireOrWait(uint8_t slot)
{
    enum { TABLE = 0xB806 };
    ObjArm r;
    uint8_t frame, count, y, value, found_slot, last;
    bool found;

    jsr_push(0xB8CD);
    r = StepFramesFacingPlayerAlways(slot, 2);
    jsr_pop();
    if (r.step == FRAME_LIST_ENDED) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 3);
        INC8(OBJ->f3568[slot]);
        count = RD8(OBJ->f3568[slot]);
        y = (uint8_t)(count & 3);
        value = rd8((uint16_t)(TABLE + y));
        WR8(OBJ->life_timer[slot], value);
        r.arm = AF_ENDED;
        r.first = value;
        r.second = y;
        return r;
    }
    r.arm = AF_HELD;
    if (r.step != FRAME_CHANGED) {
        return r;
    }
    frame = RD8(OBJ->frame[slot]);
    r.arm = AF_OTHER_FRAME;
    r.first = frame;
    if (frame != 0x32) {
        return r;
    }
    jsr_push(0xB8DB);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    r.arm = AF_NO_SLOT;
    r.second = found_slot;
    r.third = last;
    if (!found) {
        return r;
    }
    jsr_push(0xB8E2);
    r.fourth = StartObjectAtPlace(0x23, slot, found_slot);
    jsr_pop();
    r.arm = AF_FIRED;
    return r;
}

/* LDY #2 and the step; then the table's LDA and TAY, or the CMP of the frame with $32, the slot search and the start's A. */
static Ended animate_then_fire_or_wait_exit_shim(ObjArm r)
{
    Ended e = ldy_frame_step_shim(2, r);

    if (r.arm == AF_ENDED) {
        return ended_then(e, ended_with_y(ended_load(r.first), r.second));
    }
    if (r.arm == AF_HELD) {
        return e;
    }
    e = ended_then(e, ended_compare(r.first, 0x32));
    if (r.arm == AF_OTHER_FRAME) {
        return e;
    }
    e = ended_then(e, ended_of_find_slot(r.second, r.third, r.arm == AF_FIRED));
    return r.arm == AF_NO_SLOT ? e : ended_then(e, ended_load(r.fourth));
}

void abi_AnimateThenFireOrWait(void)
{
    abi_ended(animate_then_fire_or_wait_exit_shim(AnimateThenFireOrWait(cpu.x)));
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
ObjArm BurstIntoSixPieces(uint8_t slot)
{
    enum { TABLE = 0xB9B6, TYPE = 0x24 };
    ObjArm r = { 0 };
    uint8_t piece = 0, found_slot, row, value;
    bool found;

    r.arm = S4_BS_DONE;
    do {
        WR8(ZP->hit_slot, piece);
        jsr_push(0xB971);
        found_slot = FindFreeSlotFrom30(&r.fifth, &found);
        jsr_pop();
        r.fourth = found_slot;
        if (!found) {
            r.arm = S4_BS_NO_SLOT;
            ClearObject(slot);
            return r;
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
        r.first++;
        r.second = piece;
        r.third = value;
    } while (piece < 6);
    ClearObject(slot);
    return r;
}

/* Each started piece's pass replaces the exit before it; a search that found no slot follows the last pass, or LDY #0. */
static Ended burst_into_six_pieces_exit_shim(ObjArm r)
{
    Ended e = ended_with_y(ended_kept(), 0);

    if (r.first != 0) {
        e = ended_with_y(ended_with_carry(ended_with_a(ended_nz((uint8_t)(r.second - 6)), r.third), r.second >= 6), r.second);
    }
    if (r.arm == S4_BS_NO_SLOT) {
        e = ended_then(e, ended_of_find_slot(r.fourth, r.fifth, false));
    }
    return e;
}

void abi_BurstIntoSixPieces(void)
{
    abi_ended(burst_into_six_pieces_exit_shim(BurstIntoSixPieces(cpu.x)));
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
ObjArm SoundEveryFourthFrameB(uint8_t slot, uint8_t entry_y)
{
    ObjArm r = { 0 };
    uint8_t bits = RD8(OBJ->f3618[slot]);

    r.first = (uint8_t)(bits & 3);
    r.arm = S4_SQ_QUIET;
    if (r.first != 0) {
        return r;
    }
    jsr_push(0xB63E);
    r.screen = SoundIfOnFirstScreen(0x22, slot, entry_y);
    jsr_pop();
    r.screen_called = true;
    r.arm = S4_SQ_SOUNDED;
    return r;
}

static ScreenSoundEnd sound_every_fourth_frame_b_exit_shim(ObjArm r)
{
    ScreenSoundEnd s = { ended_kept(), ended_kept(), false, { 0 } };

    if (r.arm == S4_SQ_QUIET) {
        s.before = ended_load(r.first);
        return s;
    }
    s.before = ended_load(0x22);
    s.sound = r.screen;
    s.called = true;
    return s;
}

void abi_SoundEveryFourthFrameB(void)
{
    abi_screen_sound_end(sound_every_fourth_frame_b_exit_shim(SoundEveryFourthFrameB(cpu.x, cpu.y)));
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
ObjArm BurstWhenFlaggedHit(uint8_t slot, uint8_t entry_y)
{
    ObjArm r = { 0 };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t flags;

    r.first = state;
    r.arm = BF_IN_STATE;
    if (state == 6) {
        return r;
    }
    flags = RD8(OBJ->status_flags[slot]);
    r.second = (uint8_t)(flags & 0x80);
    r.arm = BF_CLEAR;
    if (r.second == 0) {
        return r;
    }
    jsr_push(0xB324);
    r.screen = SoundIfOnFirstScreen(0x8B, slot, entry_y);
    jsr_pop();
    r.screen_called = true;
    jsr_push(0xB327);
    r.sub = obj_sub(SpawnSixPiecesA(slot));
    jsr_pop();
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 6);
    r.arm = BF_BURST;
    return r;
}

/* The CMP of the state with 6, the AND's A; when it burst, LDA #$8B, the sound's exit, the spawn's and LDA #6. */
static ScreenSoundEnd burst_when_flagged_hit_exit_shim(ObjArm r)
{
    ScreenSoundEnd s = { ended_kept(), ended_kept(), false, { 0 } };

    s.before = ended_compare(r.first, 6);
    if (r.arm == BF_IN_STATE) {
        return s;
    }
    s.before = ended_then(s.before, ended_load(r.second));
    if (r.arm == BF_CLEAR) {
        return s;
    }
    s.before = ended_then(s.before, ended_load(0x8B));
    s.sound = r.screen;
    s.called = r.screen_called;
    s.end = ended_then(spawn_six_pieces_exit_shim(obj_arm(r.sub)), ended_load(6));
    return s;
}

void abi_BurstWhenFlaggedHit(void)
{
    abi_screen_sound_end(burst_when_flagged_hit_exit_shim(BurstWhenFlaggedHit(cpu.x, cpu.y)));
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
ObjArm PickStateByPlayerDistance(uint8_t slot, bool carry_in)
{
    return pick_state_by_distance(slot, carry_in, 0xB494, 0xB48A);
}

void abi_PickStateByPlayerDistance(void)
{
    abi_ended(pick_state_by_distance_exit_shim(PickStateByPlayerDistance(cpu.x, (cpu.p & P_C) != 0)));
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
ObjArm AnimateMoveThenSetUpNextStep(uint8_t slot, bool carry_in, uint8_t entry_y)
{
    ObjArm r;
    ObjArm pick;
    Sum sum;
    uint8_t left;

    (void)entry_y;
    jsr_push(0xB56D);
    r = StepFramesFacingPlayer(slot, 4);
    jsr_pop();
    jsr_push(0xB570);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    left = RD8(OBJ->life_timer[slot]);
    left = (uint8_t)(left - 1);
    WR8(OBJ->life_timer[slot], left);
    r.first = left;
    r.arm = AM_COUNTING;
    if (left != 0) {
        return r;
    }
    jsr_push(0xB57A);
    (void)SoundIfOnFirstScreen(0x8B, slot, animate_move_sound_y_shim(r));
    jsr_pop();
    jsr_push(0xB57D);
    pick = PickSpeedAndTurnFromTableB(slot);
    jsr_pop();
    r.sub = obj_sub(pick);
    jsr_push(0xB580);
    sum = PlayerDistanceSum(slot, animate_move_sum_carry_shim(pick, carry_in));
    jsr_pop();
    r.second = (uint8_t)(sum.value & 0x3F);
    WR8(OBJ->f36c8[slot], (uint8_t)(r.second + 0x58));
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.third = INC8(OBJ->f2800[slot]);
    r.arm = AM_DONE;
    return r;
}

/* LDY #4 and the step, the move, the DEC's N and Z; at zero PickSpeedAndTurnFromTableB's exit, then A the sum ($58 added with CLC), N and Z from the INC, C and V the add's. */
static Ended animate_move_then_set_up_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_then(ldy_frame_step_shim(4, r), ended_of_axis_move(r.move)), ended_nz(r.first));
    Sum sum;

    if (r.arm == AM_COUNTING) {
        return e;
    }
    sum = add_bytes(r.second, 0x58, false);
    e = ended_then(e, pick_speed_and_turn_exit_shim(obj_arm(r.sub)));
    return ended_then(e, ended_with_a(ended_with_overflow(ended_with_carry(ended_nz(r.third), sum.carry), sum.overflow), sum.value));
}

/* The Y SoundIfOnFirstScreen is entered with: the LDY #4, or the Y the step set. */
static uint8_t animate_move_sound_y_shim(ObjArm r)
{
    return ended_y_or(ldy_frame_step_shim(4, r), 4);
}

/* The C PlayerDistanceSum is entered with: what PickSpeedAndTurnFromTableB left. */
static bool animate_move_sum_carry_shim(ObjArm pick, bool carry_in)
{
    return ended_carry_or(pick_speed_and_turn_exit_shim(pick), carry_in);
}

void abi_AnimateMoveThenSetUpNextStep(void)
{
    abi_ended(animate_move_then_set_up_exit_shim(AnimateMoveThenSetUpNextStep(cpu.x, (cpu.p & P_C) != 0, cpu.y)));
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
ObjArm ChargeWhenPlayerNear(uint8_t slot)
{
    enum { X_SPEEDS = 0xB3E7, Y_SPEEDS = 0xB3E9 };
    ObjArm r;
    uint8_t kind, flags, facing, y, value;
    Sum sum;

    jsr_push(0xB39C);
    r = StepFramesFacingPlayer(slot, 0);
    jsr_pop();
    kind = RD8(OBJ->next_kind[slot]);
    r.first = kind;
    r.arm = CH_KIND;
    if ((kind & 0x80) != 0) {
        return r;
    }
    jsr_push(0xB3A4);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.arm = CH_FAR;
    if (r.distance.distance >= 0x68) {
        return r;
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
    r.third = (uint8_t)(sum.value & 0x1F);
    WR8(OBJ->f36c8[slot], (uint8_t)(r.third + 0x48));
    WR8(OBJ->state[slot], 4);
    WR8(OBJ->f2800[slot], 2);
    r.second = y;
    r.arm = CH_CHARGED;
    return r;
}

/* LDY #0 and the step, the LDA of next_kind; the distance and its CMP with $68; when it charged, A 2, C and V from the add of $48, Y the speed index. */
static Ended charge_when_player_near_exit_shim(ObjArm r)
{
    Ended e = ended_then(ldy_frame_step_shim(0, r), ended_load(r.first));
    Sum sum;

    if (r.arm == CH_KIND) {
        return e;
    }
    e = ended_then(ended_then(e, ended_of_distance(r.distance)), ended_compare(r.distance.distance, 0x68));
    if (r.arm == CH_FAR) {
        return e;
    }
    sum = add_bytes(r.third, 0x48, false);
    return ended_then(e, ended_with_y(ended_with_overflow(ended_with_carry(ended_load(2), sum.carry), sum.overflow), r.second));
}

void abi_ChargeWhenPlayerNear(void)
{
    abi_ended(charge_when_player_near_exit_shim(ChargeWhenPlayerNear(cpu.x)));
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
ObjArm TurnOnRandomBeatWalkA(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t phase = RD8(OBJ->f2800[slot]);

    if (phase != 0) {
        r.arm = TW_FOLLOW;
        r.sub = obj_sub(TurnOnRandomBeatWalkFollowA(slot));
        return r;
    }
    r.arm = TW_FIRST;
    r.sub = obj_sub(walk_on_random_beat_first(slot, 0xB67B, 0xB6AC, 0xB6B0));
    return r;
}

void abi_TurnOnRandomBeatWalkA(void)
{
    abi_ended(turn_on_random_beat_walk_exit_shim(TurnOnRandomBeatWalkA(cpu.x)));
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
ObjArm TurnOnRandomBeatWalkFollowA(uint8_t slot)
{
    return walk_on_random_beat_follow(slot, 0xB6BD, StepFramesFacingPlayer);
}

void abi_TurnOnRandomBeatWalkFollowA(void)
{
    abi_ended(walk_on_random_beat_follow_exit_shim(TurnOnRandomBeatWalkFollowA(cpu.x)));
}

/*
 * @routine 07DC:250 FallAnimatedThenBecomeKind9B
 * @run 70:BA50
 * WHAT IT DOES: In state 0 it loads its record and, when the player is on its left (bit 0 of
 * SideOfPlayer), reverses its X speed. In any other state it counts f3510 down; at zero it becomes
 * kind $9B in state 0. Until then it steps the frame list picked by life_timer from a table at
 * overlay address $BA89 (two bytes each), applies gravity without a limit and moves.
 * HOW: LoadObjectRecord, SideOfPlayer, NegateVelocityX; or a decrement; or StepObjectFrames,
 * ApplyGravityUnlimited and ApplyVelocity as a tail jump. The reversal ran in no script: the draft
 * stops there, so liftcmp does not judge it.
 * WAITS: no.
 * Entry: X object, D clear. Exit: state 0: as LoadObjectRecord, then SideOfPlayer's bit, and the
 * reversal's add when it ran; counting out: A, N, Z from the decrement (A 0); moving: A, N, Z, C
 * and V from ApplyVelocity, Y from the frame step.
 */
ObjArm FallAnimatedThenBecomeKind9B(uint8_t slot)
{
    enum { LISTS = 0xBA89 };
    ObjArm r = { 0 };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t index, exit_y;

    if (state == 0) {
        r.arm = S4_FB_RECORD;
        jsr_push(0xBA58);
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        jsr_pop();
        jsr_push(0xBA5B);
        r.side = SideOfPlayer(slot);
        jsr_pop();
        r.third = (uint8_t)(r.side.sides & 1);
        if (r.third != 0) {
            jsr_push(0xBA62);
            r.sum = NegateVelocityX(slot);
            jsr_pop();
        }
        return r;
    }
    r.fourth = (uint8_t)(RD8(OBJ->f3510[slot]) - 1);
    WR8(OBJ->f3510[slot], r.fourth);
    r.arm = S4_FB_BECAME;
    if (r.fourth == 0) {
        WR8(OBJ->kind[slot], 0x9B);
        WR8(OBJ->state[slot], 0);
        return r;
    }
    index = (uint8_t)(RD8(OBJ->life_timer[slot]) << 1);
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    exit_y = index;
    jsr_push(0xBA83);
    StepObjectFrames(slot, &exit_y);
    jsr_pop();
    jsr_push(0xBA86);
    ApplyGravityUnlimited(slot);
    jsr_pop();
    r.move = ApplyVelocity(slot);
    r.y = exit_y;
    r.arm = S4_FB_FALLING;
    return r;
}

static Ended fall_animated_then_become_kind9b_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == S4_FB_RECORD) {
        e = ended_then(ended_then(record_exit_shim(r), ended_of_side(r.side)), ended_load(r.third));
        if (r.third != 0) {
            e = ended_then(e, ended_of_sum(r.sum));
        }
        return e;
    }
    if (r.arm == S4_FB_BECAME) {
        return ended_then(ended_nz(r.fourth), ended_load(0x9B));
    }
    return ended_then(ended_with_y(ended_kept(), r.y), ended_of_axis_move(r.move));
}

void abi_FallAnimatedThenBecomeKind9B(void)
{
    abi_ended(fall_animated_then_become_kind9b_exit_shim(FallAnimatedThenBecomeKind9B(cpu.x)));
}

/*
 * @routine 07DC:1E6 StartLobTowardPlayer
 * @run 70:B9E6
 * @stop BA21
 * WHAT IT DOES: In state 0 it loads its record and throws itself on an arc toward the player: an X speed from a
 * table of word pairs at overlay address $BA21 by the player's distance along X (a pair per 32 pixels), the Y speed
 * -$0480, reversed in X when the player is on its left. In any other state it runs StepLobArc.
 * HOW: LoadObjectRecord, ApplyVelocityY, DistanceXToPlayer, SideOfPlayer, NegateVelocityX; the table lies between
 * the two parts and stays data. The reversal ran in no script: the draft stops there, so liftcmp does not judge it.
 * WAITS: no.
 * Entry: X object, Y kept to the frame step, D clear. Exit: state 0: A the Y speed's low byte or the reversal's, N, Z
 * from it, C from the table shift, V from the distance, Y the table index; later as StepLobArc.
 */
ObjArm StartLobTowardPlayer(uint8_t slot, uint8_t entry_y)
{
    static const uint16_t RETURNS[5] = { 0xB9EE, 0xB9F6, 0xB9F9, 0xBA19, 0xBA20 };
    ObjArm r = { 0 };

    if (RD8(OBJ->state[slot]) != 0) {
        r.arm = LB_ARC;
        r.sub = obj_sub(StepLobArc(slot, entry_y));
        return r;
    }
    r.arm = LB_START;
    r.sub = obj_sub(start_lob(slot, 0xBA21, RETURNS));
    return r;
}

void abi_StartLobTowardPlayer(void)
{
    abi_ended(start_lob_toward_player_exit_shim(StartLobTowardPlayer(cpu.x, cpu.y)));
}

/*
 * @routine 07DC:231 StepLobArc
 * @run 70:BA31
 * WHAT IT DOES: The later frames of the arc: steps the frame list at overlay address $B848, makes the Y speed
 * grow by $24 and moves the object.
 * HOW: StepObjectFrames, a sixteen bit add, ApplyVelocity as a tail jump.
 * WAITS: no.
 * Entry: X object, Y kept by the step, D clear. Exit: A, N, Z, C, V from ApplyVelocity; Y the step's.
 */
ObjArm StepLobArc(uint8_t slot, uint8_t entry_y)
{
    return step_lob_arc(slot, entry_y, 0xB848, 0xBA3C);
}

void abi_StepLobArc(void)
{
    abi_ended(step_lob_arc_exit_shim(StepLobArc(cpu.x, cpu.y)));
}
