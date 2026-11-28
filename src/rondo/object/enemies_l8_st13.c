/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 025C (enemy kinds not yet identified), by routine. Bodies return
 * the arm they ended on and the values behind it (L8Arm or a struct of this file's section of stage_enemies_l8.h); each routine's exit
 * shim builds how its last instructions left A, Y and the flags (Ended, enemy_objects.h), and the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l8.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/* The arms the routines end on. */
enum { C85_COUNTING, C85_STARTED };
enum { STOP_COUNTING, STOP_DONE };
enum { B995_HELD, B995_ENDED };
enum { T2D_COUNTING, T2D_STARTED };
enum { CLEAR_COUNTING, CLEAR_DONE };
enum { T2E_HELD, T2E_STARTED };
enum { C86_COUNTING, C86_SOUNDED };
enum { SPIRAL_TURNING, SPIRAL_WRAPPED };
enum { LEFT_HIGH_ZERO, LEFT_LOW_CLEAR, LEFT_PAST };
enum { SLOT10_COUNTING, SLOT10_CLEARED };
enum { HURT_STATE_ZERO, HURT_STATE_HIGH, HURT_NOT_HURT, HURT_SOUNDED };
enum { NEAR_X_FAR, NEAR_Y_TESTED };
enum { GROUND_FALLING, GROUND_LANDED };
enum { CC_ENDED, CC_MOVED };
enum { TURN_COUNTING, TURN_DONE };
enum { CB_ENDED, CB_MOVED };
enum { FOLLOW_NEW, FOLLOW_FRAME, FOLLOW_LIST };
enum { LAUNCH_HELD, LAUNCH_DONE };
enum { T30_NOT_YET, T30_NO_SLOT, T30_SPAWNED };
enum { UNTIL_HURT_GOING, UNTIL_HURT_DONE };
enum { W2F_NONE, W2F_NO_SLOT, W2F_SPAWNED };
enum { NEXT_COUNTING, NEXT_DONE };
enum { S43_COUNTING, S43_SOUNDED };
enum { SEEK_FAR, SEEK_HIT };

static Ended l8_ring_exit_shim(L8St13Ring r);

/*
 * @routine 025C:131 StepListThenStartChannel85
 * @run 70:B931
 * WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero starts list $85 on channel 5, sets OBJ_3670 5, OBJ_3510 $7B, the byte before the frame $2F and goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, a memory decrement, StartListChannelKind2 between a PHX and a PLX, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_channel85_exit_shim).
 */
L8Arm StepListThenStartChannel85(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xB6DA, 0xB93C, false, y_entry);
    r.first = DEC8(OBJ->f3510[slot]);
    if (r.first != 0) {
        r.arm = C85_COUNTING;
        return r;
    }
    r.second = l8_channel2(slot, 0x85, 5, 0xB949);
    WR8(OBJ->f3670[slot], 5);
    WR8(OBJ->f3510[slot], 0x7B);
    WR8(OBJ->frame[slot - 1], 0x2F);
    r.third = INC8(OBJ->substate[slot]);
    r.arm = C85_STARTED;
    return r;
}

/* What StepListThenStartChannel85 leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_channel85_exit_shim(L8Arm r)
{
    Ended e = then(l8_step_ended(r.step), ended_nz(r.first));

    if (r.arm == C85_COUNTING) {
        return e;
    }
    e = then(e, l8_channel2_ended(r.second));
    e = then(e, ended_load(0x2F));
    return then(e, ended_nz(r.third));
}

void abi_StepListThenStartChannel85(void)
{
    abi_ended(l8_channel85_exit_shim(StepListThenStartChannel85(cpu.x, cpu.y)));
}

/*
 * @routine 025C:15D StepListThenStopAnimation
 * @run 70:B95D
 * WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero clears OBJ_3670 and the animation step and timer and goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, a memory decrement, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_stop_animation_exit_shim).
 */
L8Arm StepListThenStopAnimation(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xB6DA, 0xB968, false, y_entry);
    r.first = DEC8(OBJ->f3510[slot]);
    if (r.first != 0) {
        r.arm = STOP_COUNTING;
        return r;
    }
    WR8(OBJ->f3670[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.second = INC8(OBJ->substate[slot]);
    r.arm = STOP_DONE;
    return r;
}

/* What StepListThenStopAnimation leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_stop_animation_exit_shim(L8Arm r)
{
    Ended e = then(l8_step_ended(r.step), ended_nz(r.first));

    if (r.arm == STOP_COUNTING) {
        return e;
    }
    return then(e, ended_nz(r.second));
}

void abi_StepListThenStopAnimation(void)
{
    abi_ended(l8_stop_animation_exit_shim(StepListThenStopAnimation(cpu.x, cpu.y)));
}

/*
 * @routine 025C:17A StepListB995ThenWait
 * @run 70:B97A
 * WHAT IT DOES: Steps the animation list at $B995; when it ends sets OBJ_3670 6 and OBJ_3510 $3A and goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_list_b995_exit_shim).
 */
L8Arm StepListB995ThenWait(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xB995, 0xB985, false, y_entry);
    if (r.step.step != FRAME_LIST_ENDED) {
        r.arm = B995_HELD;
        return r;
    }
    WR8(OBJ->f3670[slot], 6);
    WR8(OBJ->f3510[slot], 0x3A);
    r.first = INC8(OBJ->substate[slot]);
    r.arm = B995_ENDED;
    return r;
}

/* What StepListB995ThenWait leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_list_b995_exit_shim(L8Arm r)
{
    Ended e = l8_step_ended(r.step);

    if (r.arm == B995_HELD) {
        return e;
    }
    e = then(e, ended_load(0x3A));
    return then(e, ended_nz(r.first));
}

void abi_StepListB995ThenWait(void)
{
    abi_ended(l8_list_b995_exit_shim(StepListB995ThenWait(cpu.x, cpu.y)));
}

/*
 * @routine 025C:19C StepListThenStartType2D
 * @run 70:B99C
 * WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero clears slot $50, starts an object of type $2D there at the object's place, sets OBJ_3670 7 and OBJ_3510 $83 and goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, a memory decrement, ClearObjectInY, StartObjectAtPlace, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_type2d_exit_shim).
 */
L8Arm StepListThenStartType2D(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xB6DA, 0xB9A7, false, y_entry);
    r.first = DEC8(OBJ->f3510[slot]);
    if (r.first != 0) {
        r.arm = T2D_COUNTING;
        return r;
    }
    jsr_push(0xB9B1);
    ClearObjectInY(0x50);
    jsr_pop();
    jsr_push(0xB9B6);
    r.second = StartObjectAtPlace(0x2D, slot, 0x50);
    jsr_pop();
    WR8(OBJ->f3670[slot], 7);
    WR8(OBJ->f3510[slot], 0x83);
    r.third = INC8(OBJ->substate[slot]);
    r.arm = T2D_STARTED;
    return r;
}

/* What StepListThenStartType2D leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_type2d_exit_shim(L8Arm r)
{
    Ended e = then(l8_step_ended(r.step), ended_nz(r.first));

    if (r.arm == T2D_COUNTING) {
        return e;
    }
    e = then(e, ended_with_y(ended_nz(0x50), 0x50));
    e = then(e, ended_load(r.second));
    e = then(e, ended_load(0x83));
    return then(e, ended_nz(r.third));
}

void abi_StepListThenStartType2D(void)
{
    abi_ended(l8_type2d_exit_shim(StepListThenStartType2D(cpu.x, cpu.y)));
}

/*
 * @routine 025C:1C4 StepListThenClearAnimation
 * @run 70:B9C4
 * WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero clears the animation step and timer and OBJ_3670 and goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, a memory decrement, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_clear_animation_exit_shim).
 */
L8Arm StepListThenClearAnimation(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xB6DA, 0xB9CF, false, y_entry);
    r.first = DEC8(OBJ->f3510[slot]);
    if (r.first != 0) {
        r.arm = CLEAR_COUNTING;
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f3670[slot], 0);
    r.second = INC8(OBJ->substate[slot]);
    r.arm = CLEAR_DONE;
    return r;
}

/* What StepListThenClearAnimation leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_clear_animation_exit_shim(L8Arm r)
{
    Ended e = then(l8_step_ended(r.step), ended_nz(r.first));

    if (r.arm == CLEAR_COUNTING) {
        return e;
    }
    return then(e, ended_nz(r.second));
}

void abi_StepListThenClearAnimation(void)
{
    abi_ended(l8_clear_animation_exit_shim(StepListThenClearAnimation(cpu.x, cpu.y)));
}

/*
 * @routine 025C:1E1 StepListThenStartType2E
 * @run 70:B9E1
 * WHAT IT DOES: Steps the animation list at $BA0E; when it ends clears OBJ_3720, sets OBJ_3670 8, OBJ_34B8 1 and OBJ_3510 $80, goes to the next OBJ_SUBSTATE, clears slot $30 and starts an object of type $2E there at the object's place.
 * HOW: StepObjectFrames, stores, an INC, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_type2e_exit_shim).
 */
L8Arm StepListThenStartType2E(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xBA0E, 0xB9EC, false, y_entry);
    if (r.step.step != FRAME_LIST_ENDED) {
        r.arm = T2E_HELD;
        return r;
    }
    WR8(OBJ->f3720[slot], 0);
    WR8(OBJ->f3670[slot], 8);
    WR8(OBJ->f34b8[slot], 1);
    WR8(OBJ->f3510[slot], 0x80);
    r.first = INC8(OBJ->substate[slot]);
    jsr_push(0xBA08);
    ClearObjectInY(0x30);
    jsr_pop();
    jsr_push(0xBA0D);
    r.second = StartObjectAtPlace(0x2E, slot, 0x30);
    jsr_pop();
    r.arm = T2E_STARTED;
    return r;
}

/* What StepListThenStartType2E leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_type2e_exit_shim(L8Arm r)
{
    Ended e = l8_step_ended(r.step);

    if (r.arm == T2E_HELD) {
        return e;
    }
    e = then(e, ended_load(0x80));
    e = then(e, ended_nz(r.first));
    e = then(e, ended_with_y(ended_nz(0x30), 0x30));
    return then(e, ended_load(r.second));
}

void abi_StepListThenStartType2E(void)
{
    abi_ended(l8_type2e_exit_shim(StepListThenStartType2E(cpu.x, cpu.y)));
}

/*
 * @routine 025C:215 StepListThenSoundAndStartChannel86
 * @run 70:BA15
 * WHAT IT DOES: Steps the animation list at $B6DA; when the 16-bit timer OBJ_34B8:OBJ_3510 counts down to zero starts list $86 on channel 5, plays sound $76, clears the animation, OBJ_35C0, OBJ_38D8 and OBJ_3670, sets OBJ_3880 2 and goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, a 16-bit subtract in memory, StartListChannelKind2 between a PHX and a PLX, SoundCommand, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_channel86_exit_shim).
 */
L8St13SoundArm StepListThenSoundAndStartChannel86(uint8_t slot, uint8_t y_entry)
{
    L8St13SoundArm r = { 0 };
    uint8_t low, high;
    Sum diff;

    r.step = l8_list_step(slot, 0xB6DA, 0xBA20, false, y_entry);
    low = RD8(OBJ->f3510[slot]);
    diff = sub_bytes(low, 1, true);
    WR8(OBJ->f3510[slot], diff.value);
    high = RD8(OBJ->f34b8[slot]);
    diff = sub_bytes(high, 0, diff.carry);
    WR8(OBJ->f34b8[slot], diff.value);
    r.sum = diff;
    r.first = RD8(OBJ->f3510[slot]);
    if ((uint8_t)(diff.value | r.first) != 0) {
        r.arm = C86_COUNTING;
        return r;
    }
    r.second = l8_channel2(slot, 0x86, 5, 0xBA3E);
    jsr_push(0xBA44);
    r.call = SoundCommand(0x76, slot, r.step.y);
    jsr_pop();
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f35c0[slot], 0);
    WR8(OBJ->f3880[slot], 2);
    WR8(OBJ->f38d8[slot], 0);
    WR8(OBJ->f3670[slot], 0);
    r.third = INC8(OBJ->substate[slot]);
    r.arm = C86_SOUNDED;
    return r;
}

/* What StepListThenSoundAndStartChannel86 leaves in A, Y and the flags, with its sound call, from the arm it ended on. */
static SoundEnd l8_channel86_exit_shim(L8St13SoundArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = then(then(l8_step_ended(r.step), ended_of_sum(r.sum)), ended_load((uint8_t)(r.sum.value | r.first)));

    if (r.arm == C86_COUNTING) {
        s.end = e;
        return s;
    }
    e = then(e, l8_channel2_ended(r.second));
    s.sounded = true;
    s.command = 0x76;
    s.before = then(e, ended_load(0x76));
    s.call = r.call;
    s.end = then(ended_load(2), ended_nz(r.third));
    return s;
}

void abi_StepListThenSoundAndStartChannel86(void)
{
    abi_sound_end(l8_channel86_exit_shim(StepListThenSoundAndStartChannel86(cpu.x, cpu.y)));
}

/*
 * @routine 025C:25C SpiralOutWhileStepping
 * @run 70:BA5C
 * WHAT IT DOES: Steps the animation list at $BA9A while moving the object by the speed made from OBJ_35C0 and the radius OBJ_3880:OBJ_38D8; the radius shrinks by one and OBJ_35C0 turns on by 4; when OBJ_35C0 wraps it goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, SetVelocityFromAngle, ApplyVelocity, a 16-bit subtract in memory, an add.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_spiral_out_exit_shim).
 */
L8St13SpiralArm SpiralOutWhileStepping(uint8_t slot, uint8_t y_entry)
{
    L8St13SpiralArm r = { 0 };
    uint8_t value, angle;
    Sum sum;

    r.step = l8_list_step(slot, 0xBA9A, 0xBA67, false, y_entry);
    r.radius_lo = RD8(OBJ->f38d8[slot]);
    WR8(ZP->temp00, r.radius_lo);
    r.radius_hi = RD8(OBJ->f3880[slot]);
    WR8(ZP->temp01, r.radius_hi);
    r.angle = RD8(OBJ->f35c0[slot]);
    jsr_push(0xBA77);
    r.scale = SetVelocityFromAngle(r.angle, slot);
    jsr_pop();
    jsr_push(0xBA7A);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    value = RD8(OBJ->f38d8[slot]);
    sum = sub_bytes(value, 1, true);
    WR8(OBJ->f38d8[slot], sum.value);
    value = RD8(OBJ->f3880[slot]);
    sum = sub_bytes(value, 0, sum.carry);
    WR8(OBJ->f3880[slot], sum.value);
    r.shrunk = sum;
    angle = RD8(OBJ->f35c0[slot]);
    r.turned = add_bytes(angle, 4, false);
    WR8(OBJ->f35c0[slot], r.turned.value);
    if (!r.turned.carry) {
        r.arm = SPIRAL_TURNING;
        return r;
    }
    r.counted = INC8(OBJ->substate[slot]);
    r.arm = SPIRAL_WRAPPED;
    return r;
}

/* What SpiralOutWhileStepping leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_spiral_out_exit_shim(L8St13SpiralArm r)
{
    Ended e = l8_step_ended(r.step);

    e = then(e, ended_load(r.radius_lo));
    e = then(e, ended_load(r.radius_hi));
    e = then(e, ended_load(r.angle));
    e = then(e, l8_of_scale(r.scale));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_of_sum(r.shrunk));
    e = then(e, ended_of_sum(r.turned));
    if (r.arm == SPIRAL_TURNING) {
        return e;
    }
    return then(e, ended_nz(r.counted));
}

void abi_SpiralOutWhileStepping(void)
{
    abi_ended(l8_spiral_out_exit_shim(SpiralOutWhileStepping(cpu.x, cpu.y)));
}

/*
 * @routine 025C:2A7 MoveXUntilPastLeftEdge
 * @run 70:BAA7
 * WHAT IT DOES: Moves the object along X by its speed; once its X high byte is not 0 and its low byte is below $80 sets OBJ_3510 $20 and goes to the next OBJ_SUBSTATE.
 * HOW: ApplyVelocityX, two tests, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_past_left_exit_shim).
 */
L8Arm MoveXUntilPastLeftEdge(uint8_t slot)
{
    L8Arm r = { 0 };

    jsr_push(0xBAAA);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    r.first = RD8(OBJ->x_hi[slot]);
    if (r.first == 0) {
        r.arm = LEFT_HIGH_ZERO;
        return r;
    }
    r.second = RD8(OBJ->x_lo[slot]);
    if ((r.second & 0x80) == 0) {
        r.arm = LEFT_LOW_CLEAR;
        return r;
    }
    WR8(OBJ->f3510[slot], 0x20);
    r.third = INC8(OBJ->substate[slot]);
    r.arm = LEFT_PAST;
    return r;
}

/* What MoveXUntilPastLeftEdge leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_past_left_exit_shim(L8Arm r)
{
    Ended e = then(ended_of_axis_move(r.move), ended_load(r.first));

    if (r.arm == LEFT_HIGH_ZERO) {
        return e;
    }
    e = then(e, ended_load(r.second));
    if (r.arm == LEFT_LOW_CLEAR) {
        return e;
    }
    e = then(e, ended_load(0x20));
    return then(e, ended_nz(r.third));
}

void abi_MoveXUntilPastLeftEdge(void)
{
    abi_ended(l8_past_left_exit_shim(MoveXUntilPastLeftEdge(cpu.x)));
}

/*
 * @routine 025C:2BD CountDownThenClearAndAdvanceSlot10
 * @run 70:BABD
 * WHAT IT DOES: Counts OBJ_3510 down; at zero goes to the next state of slot $10 and clears the object.
 * HOW: A memory decrement, an INC, ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_advance_slot10_exit_shim).
 */
L8Arm CountDownThenClearAndAdvanceSlot10(uint8_t slot)
{
    L8Arm r = { 0 };

    r.first = DEC8(OBJ->f3510[slot]);
    if (r.first != 0) {
        r.arm = SLOT10_COUNTING;
        return r;
    }
    r.second = INC8(OBJ->state[0x10]);
    jsr_push(0xBAC8);
    ClearObject(slot);
    jsr_pop();
    r.arm = SLOT10_CLEARED;
    return r;
}

/* What CountDownThenClearAndAdvanceSlot10 leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_advance_slot10_exit_shim(L8Arm r)
{
    Ended e = ended_nz(r.first);

    if (r.arm == SLOT10_COUNTING) {
        return e;
    }
    return then(e, ended_nz(r.second));
}

void abi_CountDownThenClearAndAdvanceSlot10(void)
{
    abi_ended(l8_advance_slot10_exit_shim(CountDownThenClearAndAdvanceSlot10(cpu.x)));
}

/*
 * @routine 025C:2EA SoundWhenHurtInStatesOneToThree
 * @run 70:BAEA
 * WHAT IT DOES: In states 1 to 3, when OBJ_3568 is 1 or 2 or bit 7 of OBJ_HIT_STATUS is set, plays sound $43 and goes to state 4 (state 5 when OBJ_3568 is 2).
 * HOW: Compares, SoundCommand, a store.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_hurt_sound_exit_shim). The OBJ_3568 = 2 arm ran in no script; the draft stops there.
 */
L8St13SoundArm SoundWhenHurtInStatesOneToThree(uint8_t slot, uint8_t y_entry)
{
    L8St13SoundArm r = { 0 };
    uint8_t state, mark, next;
    uint16_t ret;

    state = RD8(OBJ->state[slot]);
    r.first = state;
    if (state == 0) {
        r.arm = HURT_STATE_ZERO;
        return r;
    }
    if (state >= 4) {
        r.arm = HURT_STATE_HIGH;
        return r;
    }
    mark = RD8(OBJ->f3568[slot]);
    r.second = mark;
    next = 4;
    ret = 0xBB0A;
    if (mark != 1) {
        if (mark == 2) {
            next = 5;
            ret = 0xBB15;
        } else {
            r.third = RD8(OBJ->hit_status[slot]);
            if ((r.third & 0x80) == 0) {
                r.arm = HURT_NOT_HURT;
                return r;
            }
        }
    }
    jsr_push(ret);
    r.call = SoundCommand(0x43, slot, y_entry);
    jsr_pop();
    WR8(OBJ->state[slot], next);
    r.fourth = next;
    r.arm = HURT_SOUNDED;
    return r;
}

/* What SoundWhenHurtInStatesOneToThree leaves in A, Y and the flags, with its sound call, from the arm it ended on. */
static SoundEnd l8_hurt_sound_exit_shim(L8St13SoundArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_load(r.first);

    if (r.arm == HURT_STATE_ZERO) {
        s.end = e;
        return s;
    }
    e = then(e, ended_compare(r.first, 4));
    if (r.arm == HURT_STATE_HIGH) {
        s.end = e;
        return s;
    }
    e = then(e, ended_compare(r.second, 1));
    if (r.second != 1) {
        e = then(e, ended_compare(r.second, 2));
        if (r.second != 2) {
            e = then(e, ended_load((uint8_t)(r.third & 0x80)));
            if (r.arm == HURT_NOT_HURT) {
                s.end = e;
                return s;
            }
        }
    }
    s.sounded = true;
    s.command = 0x43;
    s.before = then(e, ended_load(0x43));
    s.call = r.call;
    s.end = ended_load(r.fourth);
    return s;
}

void abi_SoundWhenHurtInStatesOneToThree(void)
{
    abi_sound_end(l8_hurt_sound_exit_shim(SoundWhenHurtInStatesOneToThree(cpu.x, cpu.y)));
}

/*
 * @routine 025C:31B LoadRecordAimAtPlayer
 * @run 70:BB1B
 * WHAT IT DOES: Loads the object's record, sets X speed -$20 and moves it, finds the angle to the point $FF00:$00F0 from the player's side and sets the speed from it with a radius of $0280; OBJ_3510 $40.
 * HOW: LoadObjectRecord, ApplyVelocityX, AngleToPlayerOffset, SetVelocityFromAngle.
 * WAITS: no.
 * Entry: X object. Exit: A $40 (l8_aim_at_player_exit_shim).
 */
L8St13AimArm LoadRecordAimAtPlayer(uint8_t slot)
{
    L8St13AimArm r = { 0 };

    jsr_push(0xBB1E);
    r.record = l8_record(slot);
    jsr_pop();
    WR8(OBJ->vx_hi[slot], 0xE0);
    jsr_push(0xBB26);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    WR8(ZP->temp08, 0);
    WR8(ZP->temp09, 0);
    WR8(ZP->temp0a, 0xF0);
    WR8(ZP->temp0b, 0xFF);
    jsr_push(0xBB35);
    r.toward = AngleToPlayerOffset(slot);
    jsr_pop();
    push8(r.toward.a);
    WR8(ZP->temp00, 0x80);
    WR8(ZP->temp01, 2);
    r.angle = pull8();
    jsr_push(0xBB42);
    r.scale = SetVelocityFromAngle(r.angle, slot);
    jsr_pop();
    WR8(OBJ->f3510[slot], 0x40);
    return r;
}

/* What LoadRecordAimAtPlayer leaves in A, Y and the flags (one path). */
static Ended l8_aim_at_player_exit_shim(L8St13AimArm r)
{
    Ended e = l8_record_ended(r.record);

    e = then(e, ended_load(0xE0));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, l8_of_angle(r.toward));
    e = then(e, ended_load(r.angle));
    e = then(e, l8_of_scale(r.scale));
    return then(e, ended_load(0x40));
}

void abi_LoadRecordAimAtPlayer(void)
{
    abi_ended(l8_aim_at_player_exit_shim(LoadRecordAimAtPlayer(cpu.x)));
}

/* The absolute difference of two bytes as the assembly makes it: SEC, SBC, then EOR #$FF and INC when it borrowed. */
static uint8_t abs_difference(uint8_t a, uint8_t b, bool *overflow)
{
    Sum diff = sub_bytes(a, b, true);
    uint8_t value = diff.value;

    *overflow = diff.overflow;
    if (!diff.carry) {
        value = (uint8_t)(~value);
        value = (uint8_t)(value + 1);
    }
    return value;
}

/*
 * @routine 025C:3E9 NearObject19
 * @run 70:BBE9
 * WHAT IT DOES: Says whether the object is within 7 of the object in slot $19 on both axes (low bytes).
 * HOW: Two absolute differences and compares; C 1 when near.
 * WAITS: no.
 * Entry: X object. Exit: C 1 near, 0 not; A the last difference, N and Z from the compare, V from the last subtract (l8_near19_exit_shim).
 */
L8St13Near NearObject19(uint8_t slot)
{
    L8St13Near r = { 0 };
    uint8_t mine, other;

    mine = RD8(OBJ->x_lo[slot]);
    other = RD8(OBJ->x_lo[0x19]);
    r.diff = abs_difference(mine, other, &r.overflow);
    if (r.diff >= 8) {
        r.arm = NEAR_X_FAR;
        return r;
    }
    mine = RD8(OBJ->y_lo[slot]);
    other = RD8(OBJ->y_lo[0x19]);
    r.diff = abs_difference(mine, other, &r.overflow);
    r.near = r.diff < 8;
    r.arm = NEAR_Y_TESTED;
    return r;
}

/* What NearObject19 leaves in A and the flags, from the arm it ended on. */
static Ended l8_near19_exit_shim(L8St13Near r)
{
    Ended e = ended_with_overflow(ended_compare(r.diff, 8), r.overflow);

    if (r.arm == NEAR_X_FAR) {
        return ended_with_carry(e, false);
    }
    return ended_with_carry(e, r.diff < 8);
}

void abi_NearObject19(void)
{
    abi_ended(l8_near19_exit_shim(NearObject19(cpu.x)));
}

/*
 * @routine 025C:425 StartRiseAndSpawnRing
 * @run 70:BC25
 * WHAT IT DOES: Starts the ring of seven objects (SpawnRingOfSeven), sets OBJ_37D0 of slot $19 to 1, clears the object's OBJ_3250, animation and X speed, gives it the Y speed -$0400 and goes to the next OBJ_SUBSTATE.
 * HOW: A call, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: A $FC, N and Z from the new OBJ_SUBSTATE (l8_rise_exit_shim).
 */
L8St13RiseArm StartRiseAndSpawnRing(uint8_t slot)
{
    L8St13RiseArm r = { 0 };

    jsr_push(0xBC28);
    r.ring = SpawnRingOfSeven(slot);
    jsr_pop();
    WR8(OBJ->f37d0[0x19], 1);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0xFC);
    WR8(OBJ->vy_lo[slot], 0);
    r.counted = INC8(OBJ->substate[slot]);
    return r;
}

/* What StartRiseAndSpawnRing leaves in A, Y and the flags (one path). */
static Ended l8_rise_exit_shim(L8St13RiseArm r)
{
    Ended e = l8_ring_exit_shim(r.ring);

    e = then(e, ended_load(0xFC));
    return then(e, ended_nz(r.counted));
}

void abi_StartRiseAndSpawnRing(void)
{
    abi_ended(l8_rise_exit_shim(StartRiseAndSpawnRing(cpu.x)));
}

/*
 * @routine 025C:448 SpawnRingOfSeven
 * @run 70:BC48
 * WHAT IT DOES: Clears the slots $20 to $26 and starts an object of type $2A in each at the object's place, giving each its number 0 to 6 in OBJ_34B8.
 * HOW: A loop of ClearObjectInY, StartObjectAtPlace and a counter in zTemp00.
 * WAITS: no.
 * Entry: X object. Exit: Y $27, C 1, N and Z from the compare, A the last number (l8_ring_exit_shim).
 */
L8St13Ring SpawnRingOfSeven(uint8_t slot)
{
    L8St13Ring r = { 0 };
    uint8_t y = 0x20;

    WR8(ZP->temp00, 0);
    do {
        jsr_push(0xBC4F);
        ClearObjectInY(y);
        jsr_pop();
        jsr_push(0xBC54);
        StartObjectAtPlace(0x2A, slot, y);
        jsr_pop();
        r.count = RD8(ZP->temp00);
        WR8(OBJ->f34b8[y], r.count);
        INC8(ZP->temp00);
        y++;
    } while (y < 0x27);
    r.y = y;
    return r;
}

/* What SpawnRingOfSeven leaves in A, Y and the flags (one path). */
static Ended l8_ring_exit_shim(L8St13Ring r)
{
    return ended_with_y(ended_with_a(l8_compare_y(r.y, 0x27), r.count), r.y);
}

void abi_SpawnRingOfSeven(void)
{
    abi_ended(l8_ring_exit_shim(SpawnRingOfSeven(cpu.x)));
}

/*
 * @routine 025C:461 FallAlongListUntilGround
 * @run 70:BC61
 * WHAT IT DOES: Steps the animation list at $BC98 while the object falls (ApplyGravityUnlimited, ApplyVelocityY); once its Y low byte reaches $B8 it takes sprite set 3, frame 0, the speed X -$0400, Y -$0200, clears the animation and goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocityY, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_fall_to_ground_exit_shim).
 */
L8Arm FallAlongListUntilGround(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };
    GravityResult gravity;

    r.step = l8_list_step(slot, 0xBC98, 0xBC6C, false, y_entry);
    jsr_push(0xBC6F);
    gravity = ApplyGravityUnlimited(slot);
    jsr_pop();
    (void)gravity;
    jsr_push(0xBC72);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.first = RD8(OBJ->y_lo[slot]);
    if (r.first < 0xB8) {
        r.arm = GROUND_FALLING;
        return r;
    }
    WR8(OBJ->sprite_set[slot], 3);
    WR8(OBJ->frame[slot], 0);
    WR8(OBJ->vx_hi[slot], 0xFC);
    WR8(OBJ->vy_hi[slot], 0xFE);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.second = INC8(OBJ->substate[slot]);
    r.arm = GROUND_LANDED;
    return r;
}

/* What FallAlongListUntilGround leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_fall_to_ground_exit_shim(L8Arm r)
{
    Ended e = then(l8_step_ended(r.step), ended_of_axis_move(r.move));

    e = then(e, ended_compare(r.first, 0xB8));
    if (r.arm == GROUND_FALLING) {
        return e;
    }
    e = then(e, ended_load(0xFE));
    return then(e, ended_nz(r.second));
}

void abi_FallAlongListUntilGround(void)
{
    abi_ended(l8_fall_to_ground_exit_shim(FallAlongListUntilGround(cpu.x, cpu.y)));
}

/*
 * @routine 025C:4B9 StepListCCThenMoveOrClear
 * @run 70:BCB9
 * WHAT IT DOES: Steps the animation list at $BCCC; while it runs the object moves by its speed, when it ends the object is cleared.
 * HOW: StepObjectFrames, then a tail jump into ClearObject or ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step and then ClearObject (all kept), or as ApplyVelocity (l8_list_cc_exit_shim).
 */
L8Arm StepListCCThenMoveOrClear(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xBCCC, 0xBCC4, false, y_entry);
    if (r.step.step == FRAME_LIST_ENDED) {
        ClearObject(slot);
        r.arm = CC_ENDED;
        return r;
    }
    r.move = ApplyVelocity(slot);
    r.arm = CC_MOVED;
    return r;
}

/* What StepListCCThenMoveOrClear leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_list_cc_exit_shim(L8Arm r)
{
    if (r.arm == CC_ENDED) {
        return l8_step_ended(r.step);
    }
    return then(l8_step_ended(r.step), ended_of_axis_move(r.move));
}

void abi_StepListCCThenMoveOrClear(void)
{
    abi_ended(l8_list_cc_exit_shim(StepListCCThenMoveOrClear(cpu.x, cpu.y)));
}

/*
 * @routine 025C:534 LoadRecordThrowByNumber
 * @run 70:BD34
 * WHAT IT DOES: Loads the object's record, then gives it the speed (both axes) from a four byte entry of the table at $BD66 picked by its number OBJ_34B8, the timer OBJ_3510 from a table at $BD86 and the frame $28 plus the number.
 * HOW: LoadObjectRecord, table reads by the number times 4, an add.
 * WAITS: no.
 * Entry: X object. Exit: A the frame, C and V from the add, Y the number (l8_throw_by_number_exit_shim).
 */
L8Arm LoadRecordThrowByNumber(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t number, row, value;

    jsr_push(0xBD37);
    r.record = l8_record(slot);
    jsr_pop();
    number = RD8(OBJ->f34b8[slot]);
    row = (uint8_t)(number << 2);
    value = rd8((uint16_t)(0xBD67 + row));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xBD66 + row));
    WR8(OBJ->vx_lo[slot], value);
    value = rd8((uint16_t)(0xBD69 + row));
    WR8(OBJ->vy_hi[slot], value);
    value = rd8((uint16_t)(0xBD68 + row));
    WR8(OBJ->vy_lo[slot], value);
    r.y = RD8(OBJ->f34b8[slot]);
    r.first = rd8((uint16_t)(0xBD86 + r.y));
    WR8(OBJ->f3510[slot], r.first);
    r.sum = add_bytes(r.y, 0x28, false);
    WR8(OBJ->frame[slot], r.sum.value);
    return r;
}

/* What LoadRecordThrowByNumber leaves in A, Y and the flags (one path). */
static Ended l8_throw_by_number_exit_shim(L8Arm r)
{
    return then(then(l8_record_ended(r.record), ended_with_y(ended_load(r.first), r.y)), ended_of_sum(r.sum));
}

void abi_LoadRecordThrowByNumber(void)
{
    abi_ended(l8_throw_by_number_exit_shim(LoadRecordThrowByNumber(cpu.x)));
}

/*
 * @routine 025C:58E FallThenTurnAtTimerEnd
 * @run 70:BD8E
 * WHAT IT DOES: The object falls and moves; when OBJ_3510 counts down to zero it takes sprite set 3, frame 0, the speed X -$0400, Y -$0100, clears the animation and goes to the next state.
 * HOW: ApplyGravityUnlimited, ApplyVelocity, a memory decrement, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_fall_then_turn_exit_shim).
 */
L8Arm FallThenTurnAtTimerEnd(uint8_t slot)
{
    L8Arm r = { 0 };
    GravityResult gravity;

    jsr_push(0xBD91);
    gravity = ApplyGravityUnlimited(slot);
    jsr_pop();
    (void)gravity;
    jsr_push(0xBD94);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.first = DEC8(OBJ->f3510[slot]);
    if (r.first != 0) {
        r.arm = TURN_COUNTING;
        return r;
    }
    WR8(OBJ->sprite_set[slot], 3);
    WR8(OBJ->frame[slot], 0);
    WR8(OBJ->vx_hi[slot], 0xFC);
    WR8(OBJ->vy_hi[slot], 0xFF);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.second = INC8(OBJ->state[slot]);
    r.arm = TURN_DONE;
    return r;
}

/* What FallThenTurnAtTimerEnd leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_fall_then_turn_exit_shim(L8Arm r)
{
    Ended e = then(ended_of_axis_move(r.move), ended_nz(r.first));

    if (r.arm == TURN_COUNTING) {
        return e;
    }
    e = then(e, ended_load(0xFF));
    return then(e, ended_nz(r.second));
}

void abi_FallThenTurnAtTimerEnd(void)
{
    abi_ended(l8_fall_then_turn_exit_shim(FallThenTurnAtTimerEnd(cpu.x)));
}

/*
 * @routine 025C:5B8 StepListCBThenMoveOrClear
 * @run 70:BDB8
 * WHAT IT DOES: Steps the animation list at $BDCB; while it runs the object moves by its speed, when it ends the object is cleared.
 * HOW: StepObjectFrames, then a tail jump into ClearObject or ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step and then ClearObject (all kept), or as ApplyVelocity (l8_list_cb_exit_shim).
 */
L8Arm StepListCBThenMoveOrClear(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xBDCB, 0xBDC3, false, y_entry);
    if (r.step.step == FRAME_LIST_ENDED) {
        ClearObject(slot);
        r.arm = CB_ENDED;
        return r;
    }
    r.move = ApplyVelocity(slot);
    r.arm = CB_MOVED;
    return r;
}

/* What StepListCBThenMoveOrClear leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_list_cb_exit_shim(L8Arm r)
{
    if (r.arm == CB_ENDED) {
        return l8_step_ended(r.step);
    }
    return then(l8_step_ended(r.step), ended_of_axis_move(r.move));
}

void abi_StepListCBThenMoveOrClear(void)
{
    abi_ended(l8_list_cb_exit_shim(StepListCBThenMoveOrClear(cpu.x, cpu.y)));
}

/*
 * @routine 025C:5F8 FollowObject19ThenStepListByMode
 * @run 70:BDF8
 * WHAT IT DOES: In state 0 loads the record. Later it takes the place of slot $19, restarts its animation when OBJ_3670 of slot $19 changed, and shows frame by it: from a table at $BE33 when the entry is below $80, otherwise by stepping the list the entry names (low nibble times two, at $BE3C).
 * HOW: LoadObjectRecord, CopyObjectPlaceBack, table reads, a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_follow_by_mode_exit_shim).
 */
L8St13FollowArm FollowObject19ThenStepListByMode(uint8_t slot, uint8_t y_entry)
{
    L8St13FollowArm r = { 0 };
    uint8_t state, y, hi;

    (void)y_entry;
    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        r.record = l8_record(slot);
        r.arm = FOLLOW_NEW;
        return r;
    }
    jsr_push(0xBE05);
    r.last = CopyObjectPlaceBack(0x19, slot);
    jsr_pop();
    r.mode = RD8(OBJ->f3670[0x19]);
    r.kept = RD8(OBJ->f34b8[slot]);
    if (r.mode != r.kept) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
    }
    WR8(OBJ->f34b8[slot], r.mode);
    r.index = RD8(OBJ->f3670[0x19]);
    r.entry = rd8((uint16_t)(0xBE33 + r.index));
    if ((r.entry & 0x80) == 0) {
        WR8(OBJ->frame[slot], r.entry);
        r.arm = FOLLOW_FRAME;
        return r;
    }
    y = (uint8_t)((r.entry & 0x0F) << 1);
    r.lo = rd8((uint16_t)(0xBE3C + y));
    WR8(ZP->temp10, r.lo);
    hi = rd8((uint16_t)(0xBE3D + y));
    WR8(ZP->temp11, hi);
    r.step.stored = hi;
    r.step.y = y;
    r.step.step = StepObjectFrames(slot, &r.step.y);
    r.arm = FOLLOW_LIST;
    return r;
}

/* What FollowObject19ThenStepListByMode leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_follow_by_mode_exit_shim(L8St13FollowArm r)
{
    Ended e;
    uint8_t entry;

    if (r.arm == FOLLOW_NEW) {
        return l8_record_ended(r.record);
    }
    e = then(ended_with_y(ended_nz(0x19), 0x19), ended_load(r.last));
    e = then(e, ended_compare(r.mode, r.kept));
    e = then(e, ended_with_y(ended_nz(r.index), r.index));
    e = then(e, ended_load(r.entry));
    if (r.arm == FOLLOW_FRAME) {
        return e;
    }
    entry = (uint8_t)((r.entry & 0x0F) << 1);
    e = then(e, ended_load(entry));
    e = ended_with_y(e, entry);
    e = then(e, ended_load(r.lo));
    return then(e, l8_step_ended(r.step));
}

void abi_FollowObject19ThenStepListByMode(void)
{
    abi_ended(l8_follow_by_mode_exit_shim(FollowObject19ThenStepListByMode(cpu.x, cpu.y)));
}

/*
 * @routine 025C:6D6 StartScriptByWaveBit
 * @run 70:BED6
 * WHAT IT DOES: Starts script $8F or $90 (by bit 0 of the wave counter) as kind 1 in slot 4.
 * HOW: StartScriptSlotKind1 between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object. Exit: A and Y the script, N and Z from X, C and V from the add (l8_wave_bit_script_exit_shim).
 */
L8Arm StartScriptByWaveBit(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t wave;

    push8(slot);
    wave = RD8(ZP->wave_counter) & 0x01;
    r.sum = add_bytes(wave, 0x8F, false);
    jsr_push(0xBEE3);
    StartScriptSlotKind1(r.sum.value, 4);
    jsr_pop();
    r.first = pull8();
    return r;
}

/* What StartScriptByWaveBit leaves in A, Y and the flags (one path). */
static Ended l8_wave_bit_script_exit_shim(L8Arm r)
{
    return then(ended_of_sum(r.sum), ended_with_y(ended_with_a(ended_nz(r.first), r.sum.value), r.sum.value));
}

void abi_StartScriptByWaveBit(void)
{
    abi_ended(l8_wave_bit_script_exit_shim(StartScriptByWaveBit(cpu.x)));
}

/*
 * @routine 025C:6E5 LoadRecordWithSound1C
 * @run 70:BEE5
 * WHAT IT DOES: Loads the object's record and plays sound $1C.
 * HOW: LoadObjectRecord, SoundCommand.
 * WAITS: no.
 * Entry: X object. Exit: as the sound call (l8_sound1c_exit_shim).
 */
L8St13SoundArm LoadRecordWithSound1C(uint8_t slot)
{
    L8St13SoundArm r = { 0 };

    jsr_push(0xBEE8);
    r.record = l8_record(slot);
    jsr_pop();
    jsr_push(0xBEED);
    r.call = SoundCommand(0x1C, slot, 11);
    jsr_pop();
    return r;
}

/* What LoadRecordWithSound1C leaves in A, Y and the flags, with its sound call (one path). */
static SoundEnd l8_sound1c_exit_shim(L8St13SoundArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());

    s.sounded = true;
    s.command = 0x1C;
    s.before = then(l8_record_ended(r.record), ended_load(0x1C));
    s.call = r.call;
    return s;
}

void abi_LoadRecordWithSound1C(void)
{
    abi_sound_end(l8_sound1c_exit_shim(LoadRecordWithSound1C(cpu.x)));
}

/*
 * @routine 025C:6EE FollowObject19ThenLaunch
 * @run 70:BEEE
 * WHAT IT DOES: Takes the place of slot $19 and steps the animation list at $BF16; when it ends sets the speed from the angle $68 with the radius $0400, OBJ_3250 2 and goes to the next state.
 * HOW: CopyObjectPlaceBack, StepObjectFrames, SetVelocityFromAngle, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_follow_launch_exit_shim).
 */
L8Arm FollowObject19ThenLaunch(uint8_t slot)
{
    L8Arm r = { 0 };

    jsr_push(0xBEF3);
    r.first = CopyObjectPlaceBack(0x19, slot);
    jsr_pop();
    r.step = l8_list_step(slot, 0xBF16, 0xBEFE, false, 0x19);
    if (r.step.step != FRAME_LIST_ENDED) {
        r.arm = LAUNCH_HELD;
        return r;
    }
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 4);
    jsr_push(0xBF0D);
    r.scale = SetVelocityFromAngle(0x68, slot);
    jsr_pop();
    WR8(OBJ->hitbox_on[slot], 2);
    r.second = INC8(OBJ->state[slot]);
    r.arm = LAUNCH_DONE;
    return r;
}

/* What FollowObject19ThenLaunch leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_follow_launch_exit_shim(L8Arm r)
{
    Ended e = then(ended_with_y(ended_nz(0x19), 0x19), ended_load(r.first));

    e = then(e, l8_step_ended(r.step));
    if (r.arm == LAUNCH_HELD) {
        return e;
    }
    e = then(then(e, ended_load(0x68)), l8_of_scale(r.scale));
    e = then(e, ended_load(2));
    return then(e, ended_nz(r.second));
}

void abi_FollowObject19ThenLaunch(void)
{
    abi_ended(l8_follow_launch_exit_shim(FollowObject19ThenLaunch(cpu.x)));
}

/*
 * @routine 025C:797 SpawnType30EveryFourthFrame
 * @run 70:BF97
 * WHAT IT DOES: When the wave counter's low two bits are 0, starts an object of type $30 in a free slot of $20 to $3F at the object's place.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_type30_exit_shim).
 */
L8Arm SpawnType30EveryFourthFrame(uint8_t slot)
{
    L8Arm r = { 0 };

    r.first = RD8(ZP->wave_counter);
    if ((r.first & 0x03) != 0) {
        r.arm = T30_NOT_YET;
        return r;
    }
    jsr_push(0xBFA0);
    r.found.slot = FindFreeSlotFrom20(&r.found.last, &r.found.found);
    jsr_pop();
    if (!r.found.found) {
        r.arm = T30_NO_SLOT;
        return r;
    }
    jsr_push(0xBFA7);
    r.second = StartObjectAtPlace(0x30, slot, r.found.slot);
    jsr_pop();
    r.arm = T30_SPAWNED;
    return r;
}

/* What SpawnType30EveryFourthFrame leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_type30_exit_shim(L8Arm r)
{
    Ended e = ended_load((uint8_t)(r.first & 0x03));

    if (r.arm == T30_NOT_YET) {
        return e;
    }
    e = then(e, l8_slot_ended(r.found));
    if (r.arm == T30_NO_SLOT) {
        return e;
    }
    return then(e, ended_load(r.second));
}

void abi_SpawnType30EveryFourthFrame(void)
{
    abi_ended(l8_type30_exit_shim(SpawnType30EveryFourthFrame(cpu.x)));
}

/*
 * @routine 025C:757 StepListThenSpawnUntilHurt
 * @run 70:BF57
 * WHAT IT DOES: Steps the animation list at $BF8E, moves the object and may spawn a piece (SpawnType30EveryFourthFrame); once bit 7 of OBJ_HIT_STATUS is set it takes the speed X $00C0, Y -$0300, clears OBJ_3250 and the animation and goes to the next state.
 * HOW: StepObjectFrames, ApplyVelocity, a call, a test, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_spawn_until_hurt_exit_shim).
 */
L8St13UntilHurtArm StepListThenSpawnUntilHurt(uint8_t slot, uint8_t y_entry)
{
    L8St13UntilHurtArm r = { 0 };

    r.step = l8_list_step(slot, 0xBF8E, 0xBF62, false, y_entry);
    jsr_push(0xBF65);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    jsr_push(0xBF68);
    r.spawn = SpawnType30EveryFourthFrame(slot);
    jsr_pop();
    r.flags = RD8(OBJ->hit_status[slot]);
    if ((r.flags & 0x80) == 0) {
        r.arm = UNTIL_HURT_GOING;
        return r;
    }
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0xC0);
    WR8(OBJ->vy_hi[slot], 0xFD);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.counted = INC8(OBJ->state[slot]);
    r.arm = UNTIL_HURT_DONE;
    return r;
}

/* What StepListThenSpawnUntilHurt leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_spawn_until_hurt_exit_shim(L8St13UntilHurtArm r)
{
    Ended e = then(l8_step_ended(r.step), ended_of_axis_move(r.move));

    e = then(e, l8_type30_exit_shim(r.spawn));
    e = then(e, ended_load((uint8_t)(r.flags & 0x80)));
    if (r.arm == UNTIL_HURT_GOING) {
        return e;
    }
    e = then(e, ended_load(0xFD));
    return then(e, ended_nz(r.counted));
}

void abi_StepListThenSpawnUntilHurt(void)
{
    abi_ended(l8_spawn_until_hurt_exit_shim(StepListThenSpawnUntilHurt(cpu.x, cpu.y)));
}

/*
 * @routine 025C:7ED SpawnType2FAtWaveMarks
 * @run 70:BFED
 * WHAT IT DOES: When the wave counter (low six bits) equals one of the eight marks at $C028, starts an object of type $2F in a free slot of $20 to $3F, plays sound $18 and gives the new object the speed from the table at $C030 (two bytes a mark), then moves it once.
 * HOW: A loop over the marks, FindFreeSlotFrom20, StartObjectAtPlace, SoundCommand, ApplyVelocity with the slots swapped.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_wave_marks_exit_shim). zD9 and zDA keep their address form until they have names.
 */
L8St13WaveMarksArm SpawnType2FAtWaveMarks(uint8_t slot, uint8_t y_entry)
{
    L8St13WaveMarksArm r = { 0 };
    uint8_t wave, y = 0, index;

    (void)y_entry;
    r.wave = RD8(ZP->wave_counter);
    wave = r.wave & 0x3F;
    for (;;) {
        r.mark = rd8((uint16_t)(0xC028 + y));
        r.at = y;
        if (wave == r.mark) {
            break;
        }
        y++;
        r.next = y;
        if (y >= 8) {
            r.arm = W2F_NONE;
            return r;
        }
    }
    wr8(0x20D9, slot);
    wr8(0x20DA, y);
    jsr_push(0xC004);
    r.found.slot = FindFreeSlotFrom20(&r.found.last, &r.found.found);
    jsr_pop();
    if (!r.found.found) {
        r.arm = W2F_NO_SLOT;
        return r;
    }
    jsr_push(0xC00B);
    r.placed = StartObjectAtPlace(0x2F, slot, r.found.slot);
    jsr_pop();
    jsr_push(0xC010);
    r.call = SoundCommand(0x18, slot, r.found.slot);
    jsr_pop();
    index = (uint8_t)(rd8(0x20DA) << 1);
    WR8(OBJ->vx_hi[r.found.slot], rd8((uint16_t)(0xC030 + index)));
    WR8(OBJ->vy_hi[r.found.slot], rd8((uint16_t)(0xC031 + index)));
    jsr_push(0xC024);
    r.move = ApplyVelocity(r.found.slot);
    jsr_pop();
    r.x = rd8(0x20D9);
    r.arm = W2F_SPAWNED;
    return r;
}

/* What SpawnType2FAtWaveMarks leaves in A, Y and the flags, with its sound call, from the arm it ended on. */
static SoundEnd l8_wave_marks_exit_shim(L8St13WaveMarksArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    uint8_t wave = r.wave & 0x3F;
    Ended e = ended_load(wave);

    e = ended_with_y(ended_compare(wave, r.mark), r.at);
    if (r.arm == W2F_NONE) {
        e = then(e, ended_with_y(ended_nz(r.next), r.next));
        s.end = then(e, l8_compare_y(r.next, 8));
        return s;
    }
    e = then(e, l8_slot_ended(r.found));
    if (r.arm == W2F_NO_SLOT) {
        s.end = e;
        return s;
    }
    s.sounded = true;
    s.command = 0x18;
    s.before = then(then(e, ended_load(r.placed)), ended_load(0x18));
    s.call = r.call;
    s.end = then(ended_of_axis_move(r.move), ended_with_y(ended_nz(r.x), r.found.slot));
    return s;
}

void abi_SpawnType2FAtWaveMarks(void)
{
    abi_sound_end(l8_wave_marks_exit_shim(SpawnType2FAtWaveMarks(cpu.x, cpu.y)));
}

/*
 * @routine 025C:7A8 StepListSlowSideways
 * @run 70:BFA8
 * WHAT IT DOES: Steps the animation list at $BFDC; the X speed loses $0008 and the Y speed gains $0020 a frame; the object moves and may spawn a piece (SpawnType2FAtWaveMarks).
 * HOW: StepObjectFrames, two 16-bit adds in memory, ApplyVelocity, a call.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the call (l8_slow_sideways_exit_shim).
 */
L8St13SidewaysArm StepListSlowSideways(uint8_t slot, uint8_t y_entry)
{
    L8St13SidewaysArm r = { 0 };
    uint8_t value;
    Sum sum;

    r.step = l8_list_step(slot, 0xBFDC, 0xBFB3, false, y_entry);
    value = RD8(OBJ->vx_lo[slot]);
    sum = sub_bytes(value, 8, true);
    WR8(OBJ->vx_lo[slot], sum.value);
    value = RD8(OBJ->vx_hi[slot]);
    sum = sub_bytes(value, 0, sum.carry);
    WR8(OBJ->vx_hi[slot], sum.value);
    value = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(value, 0x20, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    value = RD8(OBJ->vy_hi[slot]);
    sum = add_bytes(value, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    r.sum = sum;
    jsr_push(0xBFD8);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    jsr_push(0xBFDB);
    r.marks = SpawnType2FAtWaveMarks(slot, r.step.y);
    jsr_pop();
    return r;
}

/* What StepListSlowSideways leaves in A, Y and the flags, with the sound call of SpawnType2FAtWaveMarks (one path). */
static SoundEnd l8_slow_sideways_exit_shim(L8St13SidewaysArm r)
{
    Ended e = then(l8_step_ended(r.step), ended_of_sum(r.sum));

    e = then(e, ended_of_axis_move(r.move));
    return sound_end_after(e, l8_wave_marks_exit_shim(r.marks));
}

void abi_StepListSlowSideways(void)
{
    abi_sound_end(l8_slow_sideways_exit_shim(StepListSlowSideways(cpu.x, cpu.y)));
}

/*
 * @routine 025C:348 StepListThenCountdownNextState
 * @run 70:BB48
 * WHAT IT DOES: Steps the animation list at $BB69, sets OBJ_3250 2, moves the object and counts OBJ_3510 down; at zero sets it $40 and goes to the next state.
 * HOW: StepObjectFrames, ApplyVelocity, a memory decrement, a store, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_countdown_next_exit_shim).
 */
L8Arm StepListThenCountdownNextState(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xBB69, 0xBB53, false, y_entry);
    WR8(OBJ->hitbox_on[slot], 2);
    jsr_push(0xBB5B);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.first = DEC8(OBJ->f3510[slot]);
    if (r.first != 0) {
        r.arm = NEXT_COUNTING;
        return r;
    }
    WR8(OBJ->f3510[slot], 0x40);
    r.second = INC8(OBJ->state[slot]);
    r.arm = NEXT_DONE;
    return r;
}

/* What StepListThenCountdownNextState leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_countdown_next_exit_shim(L8Arm r)
{
    Ended e = then(l8_step_ended(r.step), ended_load(2));

    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_nz(r.first));
    if (r.arm == NEXT_COUNTING) {
        return e;
    }
    e = then(e, ended_load(0x40));
    return then(e, ended_nz(r.second));
}

void abi_StepListThenCountdownNextState(void)
{
    abi_ended(l8_countdown_next_exit_shim(StepListThenCountdownNextState(cpu.x, cpu.y)));
}

/*
 * @routine 025C:376 StepListThenSound43AtZero
 * @run 70:BB76
 * WHAT IT DOES: Steps the animation list at $BB69; when OBJ_3510 counts down to zero plays sound $43 and goes to the next state.
 * HOW: StepObjectFrames, a memory decrement, SoundCommand, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_sound43_exit_shim).
 */
L8St13SoundArm StepListThenSound43AtZero(uint8_t slot, uint8_t y_entry)
{
    L8St13SoundArm r = { 0 };

    r.step = l8_list_step(slot, 0xBB69, 0xBB81, false, y_entry);
    r.first = DEC8(OBJ->f3510[slot]);
    if (r.first != 0) {
        r.arm = S43_COUNTING;
        return r;
    }
    jsr_push(0xBB8B);
    r.call = SoundCommand(0x43, slot, r.step.y);
    jsr_pop();
    r.second = INC8(OBJ->state[slot]);
    r.arm = S43_SOUNDED;
    return r;
}

/* What StepListThenSound43AtZero leaves in A, Y and the flags, with its sound call, from the arm it ended on. */
static SoundEnd l8_sound43_exit_shim(L8St13SoundArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = then(l8_step_ended(r.step), ended_nz(r.first));

    if (r.arm == S43_COUNTING) {
        s.end = e;
        return s;
    }
    s.sounded = true;
    s.command = 0x43;
    s.before = then(e, ended_load(0x43));
    s.call = r.call;
    s.end = ended_nz(r.second);
    return s;
}

void abi_StepListThenSound43AtZero(void)
{
    abi_sound_end(l8_sound43_exit_shim(StepListThenSound43AtZero(cpu.x, cpu.y)));
}

/*
 * @routine 025C:38F SeekObject19ThenHitAndClear
 * @run 70:BB8F
 * WHAT IT DOES: Steps the animation list at $BB69 while flying toward slot $19 with the radius $0280; when near it (NearObject19) sets the state of slot $19 to 7, its OBJ_3670 2, the frame of slot $18 1 and its animation clear, picks OBJ_36C8 of slot $19 from the table at $BBE5 by a random number and clears the object.
 * HOW: StepObjectFrames, AngleToPoint, SetVelocityFromAngle, ApplyVelocity, NearObject19, NextRandom, ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_seek19_exit_shim).
 */
L8St13SeekArm SeekObject19ThenHitAndClear(uint8_t slot, uint8_t y_entry)
{
    L8St13SeekArm r = { 0 };
    uint8_t pick;

    r.step = l8_list_step(slot, 0xBB69, 0xBB9A, false, y_entry);
    WR8(ZP->temp09, 0);
    r.x_lo = RD8(OBJ->x_lo[0x19]);
    WR8(ZP->temp08, r.x_lo);
    WR8(ZP->temp0b, 0);
    r.y_lo = RD8(OBJ->y_lo[0x19]);
    WR8(ZP->temp0a, r.y_lo);
    jsr_push(0xBBAB);
    r.toward = AngleToPoint(slot);
    jsr_pop();
    push8(r.toward.a);
    WR8(ZP->temp00, 0x80);
    WR8(ZP->temp01, 2);
    r.angle = pull8();
    jsr_push(0xBBB8);
    r.scale = SetVelocityFromAngle(r.angle, slot);
    jsr_pop();
    jsr_push(0xBBBB);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    jsr_push(0xBBBE);
    r.near = NearObject19(slot);
    jsr_pop();
    if (!r.near.near) {
        r.arm = SEEK_FAR;
        return r;
    }
    WR8(OBJ->state[0x19], 7);
    WR8(OBJ->f3670[0x19], 2);
    WR8(OBJ->frame[0x18], 1);
    WR8(OBJ->anim_step[0x18], 0);
    WR8(OBJ->anim_timer[0x18], 0);
    jsr_push(0xBBD8);
    r.random = NextRandom();
    jsr_pop();
    pick = r.random.value & 0x03;
    r.value = rd8((uint16_t)(0xBBE5 + pick));
    WR8(OBJ->f36c8[0x19], r.value);
    jsr_push(0xBBE4);
    ClearObject(slot);
    jsr_pop();
    r.arm = SEEK_HIT;
    return r;
}

/* What SeekObject19ThenHitAndClear leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_seek19_exit_shim(L8St13SeekArm r)
{
    Ended e = l8_step_ended(r.step);
    uint8_t pick;

    e = then(e, ended_load(r.x_lo));
    e = then(e, ended_load(r.y_lo));
    e = then(e, l8_of_angle(r.toward));
    e = then(e, ended_load(r.angle));
    e = then(e, l8_of_scale(r.scale));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, l8_near19_exit_shim(r.near));
    if (r.arm == SEEK_FAR) {
        return e;
    }
    e = then(e, ended_load(0));
    e = then(e, ended_of_random(r.random));
    pick = r.random.value & 0x03;
    e = then(e, ended_with_y(ended_load(pick), pick));
    return then(e, ended_load(r.value));
}

void abi_SeekObject19ThenHitAndClear(void)
{
    abi_ended(l8_seek19_exit_shim(SeekObject19ThenHitAndClear(cpu.x, cpu.y)));
}
