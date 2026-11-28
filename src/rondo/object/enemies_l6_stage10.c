/*
 * TEMPORARY: the stage object routines of lot L6 for the overlays at sectors 10C9 to 10CF (enemy kinds not yet identified), by routine. Bodies return what they
 * ended on (ObjArm, enemy_objects.h); each routine's exit shim builds A, Y and the flags from it and its abi_ wrapper applies them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/* StartSoleScriptSlot's exit: Y the script, A the script when started else the busy kind, N and Z from A, C started. */
static Ended ended_of_sole(SoleSlot sole)
{
    uint8_t a = sole.started ? sole.script : sole.busy_kind;

    return ended_with_carry(ended_with_y(ended_load(a), sole.script), sole.started);
}

/* The exit of the angle scales (scale_exit_shim): A, N and Z, C, V when an add ran, and Y 0. */
static Ended ended_of_scale(ScaleResult r)
{
    Ended e = ended_with_y(ended_with_carry(ended_with_a(ended_nz(r.nz), r.a), r.carry), 0);

    return r.v_set ? ended_with_overflow(e, r.overflow) : e;
}

/* AngleToPlayer's exit: A, N, Z, C, V and Y. */
static Ended ended_of_angle(AngleExit r)
{
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_load(r.a), r.c), r.v), r.y);
}

/* The arms the routines below end on. */
enum { L10_CS_CAMERA, L10_CS_CLEARED };
enum { L10_B6_RUNNING, L10_B6_STOPPED };
enum { L10_SF_HIGH, L10_SF_LOW, L10_SF_BUSY, L10_SF_STARTED };
enum { L10_B7_ENDED, L10_B7_RISING };
enum { L10_C1_RUNNING, L10_C1_SIDED, L10_C1_AIMED };
enum { L10_C17_WAIT, L10_C17_GO };
enum { L10_RP_RISING, L10_RP_PLACED };
enum { L10_RS_RISING, L10_RS_PLACED };
enum { L10_FF_RECORD, L10_FF_FALLING };
enum { L10_CT_COUNTING, L10_CT_STARTED };
enum { L10_SC_HIGH_BIT, L10_SC_NO_MATCH, L10_SC_STARTED };
enum { L10_MU_RISING, L10_MU_STEPPING, L10_MU_CUED };
enum { L10_SP_WAITING, L10_SP_FULL, L10_SP_SPAWNED };
enum { L10_CS2_RUNNING, L10_CS2_ENDED };

/*
 * @routine 10CD:181 StepListC192ThenClear
 * @run 71:C181
 * WHAT IT DOES: Steps the animation list at $C192; when it ends the object is cleared.
 * HOW: StepObjectFrames, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step.
 */
ObjArm StepListC192ThenClear(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0x92);
    WR8(ZP->temp11, 0xC1);
    jsr_push(0xC18C);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    if (r.step == FRAME_LIST_ENDED) {
        ClearObject(slot);
    }
    return r;
}

void abi_StepListC192ThenClear(void)
{
    abi_ended(frame_step_exit_shim(StepListC192ThenClear(cpu.x, cpu.y)));
}

/*
 * @routine 10CB:2CB ClearSlot4EUnlessCamera18
 * @run 70:B2CB
 * WHAT IT DOES: Unless the camera mode is $18, clears the object in slot $4E, sets OBJ_3510 to $40 and goes on to the next OBJ_SUBSTATE.
 * HOW: A compare, ClearObjectInY, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the new OBJ_SUBSTATE's N and Z (A $40).
 */
ObjArm ClearSlot4EUnlessCamera18(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = rd8(CameraMode);
    r.arm = L10_CS_CAMERA;
    if (r.first == 0x18) {
        return r;
    }
    jsr_push(0xB2D7);
    ClearObjectInY(0x4E);
    jsr_pop();
    WR8(OBJ->f3510[slot], 0x40);
    r.second = INC8(OBJ->substate[slot]);
    r.arm = L10_CS_CLEARED;
    return r;
}

static Ended clear_slot4e_unless_camera18_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 0x18);

    if (r.arm == L10_CS_CAMERA) {
        return e;
    }
    return ended_then(ended_then(ended_with_y(e, 0x4E), ended_load(0x40)), ended_nz(r.second));
}

void abi_ClearSlot4EUnlessCamera18(void)
{
    abi_ended(clear_slot4e_unless_camera18_exit_shim(ClearSlot4EUnlessCamera18(cpu.x)));
}

/*
 * @routine 10CB:680 StepListB697ThenStopY
 * @run 70:B680
 * WHAT IT DOES: Steps the animation list at $B697; when it ends, the object's Y speed is cleared and it goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, then stores and an increment.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new OBJ_SUBSTATE's N and Z when the list ended.
 */
ObjArm StepListB697ThenStopY(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0x97);
    WR8(ZP->temp11, 0xB6);
    jsr_push(0xB68B);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = L10_B6_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    r.first = INC8(OBJ->substate[slot]);
    r.arm = L10_B6_STOPPED;
    return r;
}

static Ended step_list_b697_then_stop_y_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == L10_B6_RUNNING) {
        return e;
    }
    return then(e, ended_nz(r.first));
}

void abi_StepListB697ThenStopY(void)
{
    abi_ended(step_list_b697_then_stop_y_exit_shim(StepListB697ThenStopY(cpu.x, cpu.y)));
}

/*
 * @routine 10CD:334 StartFirstObjectWhenCamera2
 * @run 71:C334
 * WHAT IT DOES: When the level's Y high byte is 2, the scroll Y low is under $30 and the first object's state and OBJ_SUBSTATE are both 0, it sets that object's OBJ_3510 to $40 and OBJ_3568 to $10 and this object goes to the next state.
 * HOW: Compares and stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the last compare or load, or the new state's N and Z (A $10).
 */
ObjArm StartFirstObjectWhenCamera2(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t first, phase;

    r.first = rd8(ScrollYHiScreen);
    r.arm = L10_SF_HIGH;
    if (r.first != 2) {
        return r;
    }
    r.second = RD8(SCROLL->y_lo[0]);
    r.arm = L10_SF_LOW;
    if (r.second >= 0x30) {
        return r;
    }
    first = RD8(OBJ->state[0]);
    phase = RD8(OBJ->substate[0]);
    r.third = (uint8_t)(first | phase);
    r.arm = L10_SF_BUSY;
    if (r.third != 0) {
        return r;
    }
    WR8(OBJ->f3510[0], 0x40);
    WR8(OBJ->f3568[0], 0x10);
    r.fourth = INC8(OBJ->state[slot]);
    r.arm = L10_SF_STARTED;
    return r;
}

static Ended start_first_object_when_camera2_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 2);

    if (r.arm == L10_SF_HIGH) {
        return e;
    }
    e = then(e, ended_compare(r.second, 0x30));
    if (r.arm == L10_SF_LOW) {
        return e;
    }
    e = then(e, ended_load(r.third));
    if (r.arm == L10_SF_BUSY) {
        return e;
    }
    return then(then(e, ended_load(0x10)), ended_nz(r.fourth));
}

void abi_StartFirstObjectWhenCamera2(void)
{
    abi_ended(start_first_object_when_camera2_exit_shim(StartFirstObjectWhenCamera2(cpu.x)));
}

/*
 * @routine 10CB:702 StepListB728ThenRiseSlowly
 * @run 70:B702
 * WHAT IT DOES: Steps the animation list at $B728; while it runs the object moves and its Y speed loses $0010 a frame; when it ends the object goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, ApplyVelocity, a 16-bit subtract, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the subtract's, or the new OBJ_SUBSTATE's N and Z.
 */
ObjArm StepListB728ThenRiseSlowly(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0x28);
    WR8(ZP->temp11, 0xB7);
    jsr_push(0xB70D);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = L10_B7_ENDED;
    if (r.step == FRAME_LIST_ENDED) {
        r.first = INC8(OBJ->substate[slot]);
        return r;
    }
    jsr_push(0xB712);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.sum = sub_bytes(RD8(OBJ->vy_lo[slot]), 0x10, true);
    WR8(OBJ->vy_lo[slot], r.sum.value);
    r.sum2 = sub_bytes(RD8(OBJ->vy_hi[slot]), 0x00, r.sum.carry);
    WR8(OBJ->vy_hi[slot], r.sum2.value);
    r.arm = L10_B7_RISING;
    return r;
}

static Ended step_list_b728_then_rise_slowly_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == L10_B7_ENDED) {
        return then(e, ended_nz(r.first));
    }
    e = then(e, ended_of_axis_move(r.move));
    return then(then(e, ended_of_sum(r.sum)), ended_of_sum(r.sum2));
}

void abi_StepListB728ThenRiseSlowly(void)
{
    abi_ended(step_list_b728_then_rise_slowly_exit_shim(StepListB728ThenRiseSlowly(cpu.x, cpu.y)));
}

/*
 * @routine 10CD:0C6 StepListC103ThenPickAim
 * @run 71:C0C6
 * WHAT IT DOES: Steps the animation list at $C103; when it ends the object picks OBJ_34B8: half the time the angle to the player, otherwise a random $00 to $7F plus the byte the table at $C088 gives for the player's side; it then clears OBJ_3510 and OBJ_3568, sets OBJ_3250 to 2 and goes on to the next state.
 * HOW: StepObjectFrames, NextRandom, SideOfPlayer, AngleToPlayer, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new state's N and Z (A 2) when the list ended.
 */
ObjArm StepListC103ThenPickAim(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0x03);
    WR8(ZP->temp11, 0xC1);
    jsr_push(0xC0D1);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = L10_C1_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    jsr_push(0xC0D6);
    r.random = NextRandom();
    jsr_pop();
    if ((r.random.value & 0x01) != 0) {
        jsr_push(0xC0DD);
        r.side = SideOfPlayer(slot);
        jsr_pop();
        r.first = (uint8_t)(r.side.sides & 0x01);
        jsr_push(0xC0E3);
        r.random2 = NextRandom();
        jsr_pop();
        r.sum = add_bytes((uint8_t)(r.random2.value & 0x7F), rd8((uint16_t)(0xC088 + r.first)), false);
        WR8(OBJ->f34b8[slot], r.sum.value);
        r.arm = L10_C1_SIDED;
    } else {
        jsr_push(0xC0F1);
        r.angle = AngleToPlayer(slot);
        jsr_pop();
        WR8(OBJ->f34b8[slot], r.angle.a);
        r.arm = L10_C1_AIMED;
    }
    WR8(OBJ->f3510[slot], 0);
    WR8(OBJ->f3568[slot], 0);
    WR8(OBJ->hitbox_on[slot], 2);
    r.second = INC8(OBJ->state[slot]);
    return r;
}

static Ended step_list_c103_then_pick_aim_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == L10_C1_RUNNING) {
        return e;
    }
    e = then(then(e, ended_of_random(r.random)), ended_load((uint8_t)(r.random.value & 0x01)));
    if (r.arm == L10_C1_SIDED) {
        e = then(then(e, ended_of_side(r.side)), ended_with_y(ended_load(r.first), r.first));
        e = then(then(e, ended_of_random(r.random2)), ended_load((uint8_t)(r.random2.value & 0x7F)));
        e = then(e, ended_of_sum(r.sum));
    } else {
        e = then(e, ended_of_angle(r.angle));
    }
    return then(then(e, ended_load(2)), ended_nz(r.second));
}

void abi_StepListC103ThenPickAim(void)
{
    abi_ended(step_list_c103_then_pick_aim_exit_shim(StepListC103ThenPickAim(cpu.x, cpu.y)));
}

/*
 * @routine 10CD:126 StepListC170ThenMoveAlongAngle
 * @run 71:C126
 * WHAT IT DOES: Steps the animation list at $C170, sets the object's speed from its angle OBJ_34B8 with the length OBJ_3510:OBJ_3568, moves it and adds $000C to the length. When the object in slot $10 is in state 6 or more, or this object's OBJ_31A0 has bit 7 set, it clears its animation step and timer and goes on to the next state (and writes 0 to the byte at $0002 plus its slot).
 * HOW: StepObjectFrames, SetVelocityFromAngle, ApplyVelocity, a 16-bit add, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the last test, or the new state's N and Z.
 */
ObjArm StepListC170ThenMoveAlongAngle(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry, value;

    WR8(ZP->temp10, 0x70);
    WR8(ZP->temp11, 0xC1);
    jsr_push(0xC131);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    value = RD8(OBJ->f3568[slot]);
    WR8(ZP->temp00, value);
    value = RD8(OBJ->f3510[slot]);
    WR8(ZP->temp01, value);
    r.first = RD8(OBJ->f34b8[slot]);
    jsr_push(0xC141);
    r.scale = SetVelocityFromAngle(r.first, slot);
    jsr_pop();
    jsr_push(0xC144);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.sum = add_bytes(RD8(OBJ->f3568[slot]), 0x0C, false);
    WR8(OBJ->f3568[slot], r.sum.value);
    r.sum2 = add_bytes(RD8(OBJ->f3510[slot]), 0, r.sum.carry);
    WR8(OBJ->f3510[slot], r.sum2.value);
    r.second = RD8(OBJ->state[0x10]);
    r.arm = L10_C17_WAIT;
    if (r.second < 6) {
        r.third = (uint8_t)(RD8(OBJ->status_flags[slot]) & 0x80);
        if (r.third == 0) {
            return r;
        }
    }
    wr8((uint16_t)(0x0002 + slot), 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.fourth = INC8(OBJ->state[slot]);
    r.arm = L10_C17_GO;
    return r;
}

static Ended step_list_c170_then_move_along_angle_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    e = then(e, ended_load(r.first));
    e = then(e, ended_of_scale(r.scale));
    e = then(e, ended_of_axis_move(r.move));
    e = then(then(e, ended_of_sum(r.sum)), ended_of_sum(r.sum2));
    e = then(e, ended_compare(r.second, 6));
    if (r.second < 6) {
        e = then(e, ended_load(r.third));
    }
    if (r.arm == L10_C17_WAIT) {
        return e;
    }
    return then(e, ended_nz(r.fourth));
}

void abi_StepListC170ThenMoveAlongAngle(void)
{
    abi_ended(step_list_c170_then_move_along_angle_exit_shim(StepListC170ThenMoveAlongAngle(cpu.x, cpu.y)));
}

/*
 * @routine 10CB:6B6 RiseThenPlaceAndDrop
 * @run 70:B6B6
 * WHAT IT DOES: Moves the object along Y by its speed and takes $0018 off its Y speed. When its Y high byte is $FE it puts the object at X $0070, Y $FFC0, gives it X speed $0030 and Y speed $0400, sets its flags to $81 and zTemp $9E to 4 and goes on to the next OBJ_SUBSTATE.
 * HOW: ApplyVelocityY, a 16-bit subtract, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the new OBJ_SUBSTATE's N and Z (A 4).
 */
ObjArm RiseThenPlaceAndDrop(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB6B9);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.sum = sub_bytes(RD8(OBJ->vy_lo[slot]), 0x18, true);
    WR8(OBJ->vy_lo[slot], r.sum.value);
    r.sum2 = sub_bytes(RD8(OBJ->vy_hi[slot]), 0x00, r.sum.carry);
    WR8(OBJ->vy_hi[slot], r.sum2.value);
    r.first = RD8(OBJ->y_hi[slot]);
    r.arm = L10_RP_RISING;
    if (r.first != 0xFE) {
        return r;
    }
    WR8(OBJ->flags[slot], 0x81);
    wr8(0x209E, 4);
    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->x_lo[slot], 0x70);
    WR8(OBJ->y_hi[slot], 0xFF);
    WR8(OBJ->y_lo[slot], 0xC0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0x30);
    WR8(OBJ->vy_hi[slot], 4);
    WR8(OBJ->vy_lo[slot], 0);
    r.second = INC8(OBJ->substate[slot]);
    r.arm = L10_RP_PLACED;
    return r;
}

static Ended rise_then_place_and_drop_exit_shim(ObjArm r)
{
    Ended e = then(then(ended_of_axis_move(r.move), ended_of_sum(r.sum)), ended_of_sum(r.sum2));

    e = then(e, ended_compare(r.first, 0xFE));
    if (r.arm == L10_RP_RISING) {
        return e;
    }
    return then(then(e, ended_load(4)), ended_nz(r.second));
}

void abi_RiseThenPlaceAndDrop(void)
{
    abi_ended(rise_then_place_and_drop_exit_shim(RiseThenPlaceAndDrop(cpu.x)));
}

/*
 * @routine 10CB:415 RiseThenPlaceWithSound
 * @run 70:B415
 * WHAT IT DOES: Moves the object along Y by its speed and takes $0010 off its Y speed. When its Y high byte is $FE it sends sound $E7, sets its flags to $80, frame $1D, puts it at X $0000, Y $FFE0, gives it X speed $0400 and Y speed $0600 (high bytes) and goes on to the next OBJ_SUBSTATE.
 * HOW: ApplyVelocityY, a 16-bit subtract, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound's push. Exit: see the wrapper.
 */
ObjArm RiseThenPlaceWithSound(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };

    jsr_push(0xB418);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.sum = sub_bytes(RD8(OBJ->vy_lo[slot]), 0x10, true);
    WR8(OBJ->vy_lo[slot], r.sum.value);
    r.sum2 = sub_bytes(RD8(OBJ->vy_hi[slot]), 0x00, r.sum.carry);
    WR8(OBJ->vy_hi[slot], r.sum2.value);
    r.first = RD8(OBJ->y_hi[slot]);
    r.arm = L10_RS_RISING;
    if (r.first != 0xFE) {
        return r;
    }
    r.sounded = true;
    r.command = 0xE7;
    jsr_push(0xB435);
    r.call = SoundCommand(0xE7, slot, y_entry);
    jsr_pop();
    WR8(OBJ->flags[slot], 0x80);
    WR8(OBJ->frame[slot], 0x1D);
    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->x_lo[slot], 0);
    WR8(OBJ->y_hi[slot], 0xFF);
    WR8(OBJ->y_lo[slot], 0xE0);
    WR8(OBJ->vy_hi[slot], 6);
    WR8(OBJ->vx_hi[slot], 4);
    r.second = INC8(OBJ->substate[slot]);
    r.arm = L10_RS_PLACED;
    return r;
}

static SoundEnd rise_then_place_with_sound_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = then(then(ended_of_axis_move(r.move), ended_of_sum(r.sum)), ended_of_sum(r.sum2));

    e = then(e, ended_compare(r.first, 0xFE));
    if (r.arm == L10_RS_RISING) {
        s.end = e;
        return s;
    }
    s.before = then(e, ended_load(0xE7));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.end = then(ended_load(4), ended_nz(r.second));
    return s;
}

void abi_RiseThenPlaceWithSound(void)
{
    abi_sound_end(rise_then_place_with_sound_exit_shim(RiseThenPlaceWithSound(cpu.x, cpu.y)));
}

/*
 * @routine 10CC:74F LoadThenFallAndFlash
 * @run 70:BF4F
 * WHAT IT DOES: In state 0 loads the object's record and sets flags 1, frame $59, X speed $00C0, Y speed $0030 and OBJ_3510 $34. Later it adds $0008 to its Y speed, moves, sets its flash flags to $10 while OBJ_3510 is under $10, counts OBJ_3510 down and clears the object at zero.
 * HOW: LoadObjectRecord, a 16-bit add, ApplyVelocity, DEC, ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: the record's registers and A $34 in state 0; else the count.
 */
ObjArm LoadThenFallAndFlash(uint8_t slot)
{
    ObjArm r = { 0 };

    r.tenth = RD8(OBJ->state[slot]);
    if (r.tenth == 0) {
        r.arm = L10_FF_RECORD;
        jsr_push(0xBF57);
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        jsr_pop();
        WR8(OBJ->flags[slot], 1);
        WR8(OBJ->frame[slot], 0x59);
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0xC0);
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0x30);
        WR8(OBJ->f3510[slot], 0x34);
        return r;
    }
    r.arm = L10_FF_FALLING;
    r.sum = add_bytes(RD8(OBJ->vy_lo[slot]), 0x08, false);
    WR8(OBJ->vy_lo[slot], r.sum.value);
    r.sum2 = add_bytes(RD8(OBJ->vy_hi[slot]), 0, r.sum.carry);
    WR8(OBJ->vy_hi[slot], r.sum2.value);
    jsr_push(0xBF8F);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.third = RD8(OBJ->f3510[slot]);
    if (r.third < 0x10) {
        WR8(OBJ->flash_flags[slot], 0x10);
    }
    r.fourth = DEC8(OBJ->f3510[slot]);
    if (r.fourth == 0) {
        ClearObject(slot);
    }
    return r;
}

static Ended load_then_fall_and_flash_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == L10_FF_RECORD) {
        return then(then(ended_load(r.tenth), record_exit_shim(r)), ended_load(0x34));
    }
    e = then(then(ended_load(r.tenth), ended_of_sum(r.sum)), ended_of_sum(r.sum2));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_load(r.third));
    e = then(e, ended_compare(r.third, 0x10));
    if (r.third < 0x10) {
        e = then(e, ended_load(0x10));
    }
    return then(e, ended_nz(r.fourth));
}

void abi_LoadThenFallAndFlash(void)
{
    abi_ended(load_then_fall_and_flash_exit_shim(LoadThenFallAndFlash(cpu.x)));
}

/*
 * @routine 10CB:2E0 CountThenSoleScriptAndSound
 * @run 70:B2E0
 * WHAT IT DOES: Counts OBJ_3510 down; at zero it starts the sole script 0 in slot 3, sends sound $E4, sets its flags to $81, OBJ_3510 to $60 and zTemp $9E to 4 and goes on to the next OBJ_SUBSTATE.
 * HOW: DEC, StartSoleScriptSlot, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: see the wrapper.
 */
ObjArm CountThenSoleScriptAndSound(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = DEC8(OBJ->f3510[slot]);
    r.arm = L10_CT_COUNTING;
    if (r.first != 0) {
        return r;
    }
    push8(slot);
    jsr_push(0xB2EC);
    r.sole = StartSoleScriptSlot(0, 3);
    jsr_pop();
    slot = pull8();
    r.second = slot;
    r.sounded = true;
    r.command = 0xE4;
    jsr_push(0xB2F2);
    r.call = SoundCommand(0xE4, slot, r.sole.script);
    jsr_pop();
    WR8(OBJ->flags[slot], 0x81);
    WR8(OBJ->f3510[slot], 0x60);
    wr8(0x209E, 4);
    r.third = INC8(OBJ->substate[slot]);
    r.arm = L10_CT_STARTED;
    return r;
}

static SoundEnd count_then_sole_script_and_sound_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = ended_nz(r.first);

    if (r.arm == L10_CT_COUNTING) {
        s.end = e;
        return s;
    }
    e = then(e, ended_load(0));
    e = then(then(e, ended_of_sole(r.sole)), ended_nz(r.second));
    s.before = then(e, ended_load(0xE4));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.end = then(ended_load(4), ended_nz(r.third));
    return s;
}

void abi_CountThenSoleScriptAndSound(void)
{
    abi_sound_end(count_then_sole_script_and_sound_exit_shim(CountThenSoleScriptAndSound(cpu.x)));
}

/*
 * @routine 10CB:3ED StartScriptWhenCountInTable
 * @run 70:B3ED
 * WHAT IT DOES: While OBJ_3568 is not negative it counts it up and looks its old value up in the eight-byte table at $B40D; when found at index n it starts script $98 plus n in slot 5.
 * HOW: A search loop, StartScriptSlotKind1.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the script start's registers.
 */
ObjArm StartScriptWhenCountInTable(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y;

    r.first = RD8(OBJ->f3568[slot]);
    r.arm = L10_SC_HIGH_BIT;
    if ((r.first & 0x80) != 0) {
        return r;
    }
    INC8(OBJ->f3568[slot]);
    for (y = 0; y < 8; y++) {
        r.bytes[y] = rd8((uint16_t)(0xB40D + y));
        r.second = (uint8_t)(y + 1);
        if (r.first == r.bytes[y]) {
            break;
        }
    }
    r.arm = L10_SC_NO_MATCH;
    if (y >= 8) {
        return r;
    }
    r.third = y;
    r.sum = add_bytes(y, 0x98, false);
    push8(slot);
    jsr_push(0xB40B);
    r.fourth = StartScriptSlotKind1(r.sum.value, 5);
    jsr_pop();
    r.fifth = pull8();
    r.arm = L10_SC_STARTED;
    return r;
}

static Ended start_script_when_count_in_table_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);
    uint8_t i;

    if (r.arm == L10_SC_HIGH_BIT) {
        return e;
    }
    for (i = 0; i < r.second; i++) {
        e = then(e, ended_compare(r.first, r.bytes[i]));
        if (r.first == r.bytes[i]) {
            break;
        }
        e = then(e, ended_with_y(ended_with_carry(ended_nz((uint8_t)(i + 1 - 8)), (i + 1) >= 8), (uint8_t)(i + 1)));
    }
    if (r.arm == L10_SC_NO_MATCH) {
        return e;
    }
    e = then(e, ended_with_y(ended_load(r.third), r.third));
    e = then(e, ended_of_sum(r.sum));
    return then(then(e, ended_with_y(ended_load(r.fourth), r.fourth)), ended_nz(r.fifth));
}

void abi_StartScriptWhenCountInTable(void)
{
    abi_ended(start_script_when_count_in_table_exit_shim(StartScriptWhenCountInTable(cpu.x)));
}

/*
 * @routine 10CB:45F RiseThenCueMusic
 * @run 70:B45F
 * WHAT IT DOES: Moves the object along Y and takes $0030 off its Y speed. Once the speed's high byte is negative it steps the animation list at $B4AA; when that ends it sets bit 4 of zField8E, sends sound $FF, clears bits 3 and 4, plays the music again (CdPlayAgain), sets bit 2, clears zTemp $9E and OBJ_3250 to 2, clears its animation step and timer, goes on to the next state and clears OBJ_SUBSTATE.
 * HOW: ApplyVelocity, a 16-bit subtract, StepObjectFrames, SoundCommand, CdPlayAgain, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
 */
ObjArm RiseThenCueMusic(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry, flags;

    jsr_push(0xB462);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.sum = sub_bytes(RD8(OBJ->vy_lo[slot]), 0x30, true);
    WR8(OBJ->vy_lo[slot], r.sum.value);
    r.sum2 = sub_bytes(RD8(OBJ->vy_hi[slot]), 0x00, r.sum.carry);
    WR8(OBJ->vy_hi[slot], r.sum2.value);
    r.arm = L10_MU_RISING;
    if ((r.sum2.value & 0x80) == 0) {
        return r;
    }
    WR8(ZP->temp10, 0xAA);
    WR8(ZP->temp11, 0xB4);
    jsr_push(0xB480);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = L10_MU_STEPPING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    flags = (uint8_t)(RD8(ZP->field8e) | 0x10);
    WR8(ZP->field8e, flags);
    r.sounded = true;
    r.command = 0xFF;
    jsr_push(0xB489);
    r.call = SoundCommand(0xFF, slot, y);
    jsr_pop();
    flags = (uint8_t)(RD8(ZP->field8e) & 0xF7);
    WR8(ZP->field8e, flags);
    flags = (uint8_t)(RD8(ZP->field8e) & 0xEF);
    WR8(ZP->field8e, flags);
    jsr_push(0xB490);
    r.track = CdPlayAgain(slot, y);
    jsr_pop();
    flags = (uint8_t)(RD8(ZP->field8e) | 0x04);
    WR8(ZP->field8e, flags);
    wr8(0x209E, 0);
    WR8(OBJ->hitbox_on[slot], 2);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.first = INC8(OBJ->state[slot]);
    WR8(OBJ->substate[slot], 0);
    r.arm = L10_MU_CUED;
    return r;
}

static SoundPlayEnd rise_then_cue_music_exit_shim(ObjArm r)
{
    SoundPlayEnd s = { { 0 }, false, 0, { 0 }, false, { 0 }, { 0 } };
    Ended e = then(then(ended_of_axis_move(r.move), ended_of_sum(r.sum)), ended_of_sum(r.sum2));

    s.pre = e;
    if (r.arm == L10_MU_RISING) {
        return s;
    }
    e = then(then(then(e, ended_load(0xB4)), ended_load(0xAA)), frame_step_exit_shim(r));
    s.pre = e;
    if (r.arm == L10_MU_STEPPING) {
        return s;
    }
    s.pre = then(then(e, ended_load(0xFF)), ended_load(0xFF));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.played = true;
    s.track = r.track;
    s.end = then(ended_load(2), ended_nz(r.first));
    return s;
}

void abi_RiseThenCueMusic(void)
{
    SoundPlayEnd r = rise_then_cue_music_exit_shim(RiseThenCueMusic(cpu.x, cpu.y));

    abi_ended(r.pre);
    if (r.sounded) {
        sound_command_exit_shim(r.command, r.call);
    }
    if (r.played) {
        play_track_exit_shim(r.track);
    }
    abi_ended(r.end);
}

/*
 * @routine 10CB:28D LoadThenStartTwoScripts
 * @run 70:B28D
 * WHAT IT DOES: Loads the object's record, puts it at Y $FFC0 and starts the scripts $82 in slot 5 and $AF in slot 8.
 * HOW: LoadObjectRecord, StartScriptSlotKind1 twice.
 * WAITS: no.
 * Entry: X object. Exit: A and Y the last script, N and Z from the object index.
 */
ObjArm LoadThenStartTwoScripts(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB290);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    WR8(OBJ->y_hi[slot], 0xFF);
    WR8(OBJ->y_lo[slot], 0xC0);
    push8(slot);
    jsr_push(0xB2A2);
    r.third = StartScriptSlotKind1(0x82, 5);
    jsr_pop();
    jsr_push(0xB2A9);
    r.fourth = StartScriptSlotKind1(0xAF, 8);
    jsr_pop();
    r.fifth = pull8();
    return r;
}

static Ended load_then_start_two_scripts_exit_shim(ObjArm r)
{
    Ended e = then(record_exit_shim(r), ended_load(0xC0));

    e = then(e, ended_with_y(ended_load(r.third), r.third));
    e = then(e, ended_with_y(ended_load(r.fourth), r.fourth));
    return then(e, ended_nz(r.fifth));
}

void abi_LoadThenStartTwoScripts(void)
{
    abi_ended(load_then_start_two_scripts_exit_shim(LoadThenStartTwoScripts(cpu.x)));
}

/*
 * @routine 10CB:5AB SpawnInFreeSlotEvery30
 * @run 70:B5AB
 * WHAT IT DOES: Counts OBJ_37D0 down; at zero it sets it to $1E, looks for an empty slot from $18 to $1F, clears it and starts a kind $26 object there at this object's place.
 * HOW: DEC, a search loop, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: the count, the compare, or the start's registers.
 */
ObjArm SpawnInFreeSlotEvery30(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y;

    r.first = RD8(OBJ->f37d0[slot]);
    r.arm = L10_SP_WAITING;
    if (r.first != 0) {
        r.second = DEC8(OBJ->f37d0[slot]);
        return r;
    }
    WR8(OBJ->f37d0[slot], 0x1E);
    for (y = 0x18;; y++) {
        r.bytes[r.third] = rd8((uint16_t)(0x2750 + y));
        r.third++;
        if (r.bytes[r.third - 1] == 0) {
            break;
        }
        if ((y + 1) >= 0x20) {
            r.arm = L10_SP_FULL;
            return r;
        }
    }
    jsr_push(0xB5C9);
    ClearObjectInY(y);
    jsr_pop();
    jsr_push(0xB5CE);
    r.fourth = StartObjectAtPlace(0x26, slot, y);
    jsr_pop();
    r.fifth = y;
    r.arm = L10_SP_SPAWNED;
    return r;
}

static Ended spawn_in_free_slot_every30_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);
    uint8_t i, y;

    if (r.arm == L10_SP_WAITING) {
        return then(e, ended_nz(r.second));
    }
    e = then(e, ended_load(0x1E));
    for (i = 0; i < r.third; i++) {
        y = (uint8_t)(0x18 + i);
        e = then(then(e, ended_with_y(ended_nz(0x18), y)), ended_load(r.bytes[i]));
        if (r.bytes[i] == 0) {
            break;
        }
        e = then(e, ended_with_y(ended_with_carry(ended_nz((uint8_t)(y + 1 - 0x20)), (y + 1) >= 0x20), (uint8_t)(y + 1)));
    }
    if (r.arm == L10_SP_FULL) {
        return e;
    }
    return then(then(e, ended_load(0x26)), ended_with_y(ended_load(r.fourth), r.fifth));
}

void abi_SpawnInFreeSlotEvery30(void)
{
    abi_ended(spawn_in_free_slot_every30_exit_shim(SpawnInFreeSlotEvery30(cpu.x)));
}

/*
 * @routine 10CB:3BE CountScriptsThenStepList
 * @run 70:B3BE
 * WHAT IT DOES: Runs StartScriptWhenCountInTable, then steps the animation list at $B3D2; when it ends the object goes to the next OBJ_SUBSTATE.
 * HOW: StartScriptWhenCountInTable, StepObjectFrames, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: the step, or the new OBJ_SUBSTATE's N and Z when the list ended.
 */
ObjArm CountScriptsThenStepList(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    jsr_push(0xB3C1);
    r.sub = obj_sub(StartScriptWhenCountInTable(slot));
    jsr_pop();
    WR8(ZP->temp10, 0xD2);
    WR8(ZP->temp11, 0xB3);
    jsr_push(0xB3CC);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = L10_CS2_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    r.first = INC8(OBJ->substate[slot]);
    r.arm = L10_CS2_ENDED;
    return r;
}

static Ended count_scripts_then_step_list_exit_shim(ObjArm r)
{
    Ended e = start_script_when_count_in_table_exit_shim(obj_arm(r.sub));

    e = then(then(then(e, ended_load(0xD2)), ended_load(0xB3)), frame_step_exit_shim(r));
    if (r.arm == L10_CS2_RUNNING) {
        return e;
    }
    return then(e, ended_nz(r.first));
}

void abi_CountScriptsThenStepList(void)
{
    abi_ended(count_scripts_then_step_list_exit_shim(CountScriptsThenStepList(cpu.x, cpu.y)));
}
