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

/* The arms the converted routines end on (ObjArm, enemy_objects.h). */
enum { FO_HIGH, FO_LOW, FO_BUSY, FO_STARTED };
enum { PA_RANDOM, PA_ANGLE };
enum { MA_MOVING, MA_NEXT };
enum { ST_NEGATIVE, ST_NOT_FOUND, ST_STARTED };
enum { CM_FALLING, CM_STEPPED, CM_CUED };
enum { SF_COUNTING, SF_FULL, SF_SPAWNED };

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
 * WHAT IT DOES: Unless the camera mode is $18, clears the object in slot $4E, sets OBJ_3510 to $40 and goes on to the next OBJ_2800.
 * HOW: A compare, ClearObjectInY, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the new OBJ_2800's N and Z (A $40).
 */
ObjArm ClearSlot4EUnlessCamera18(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = rd8(CameraMode);
    if (r.first == 0x18) {
        return r;
    }
    jsr_push(0xB2D7);
    ClearObjectInY(0x4E);
    jsr_pop();
    WR8(OBJ->f3510[slot], 0x40);
    r.second = INC8(OBJ->f2800[slot]);
    return r;
}

/* The CMP of the camera mode with $18; then LDY #$4E, LDA #$40 and the INC. */
static Ended clear_slot4e_unless_camera18_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 0x18);

    return r.first == 0x18 ? e : ended_then(ended_then(ended_with_y(e, 0x4E), ended_load(0x40)), ended_nz(r.second));
}

void abi_ClearSlot4EUnlessCamera18(void)
{
    abi_ended(clear_slot4e_unless_camera18_exit_shim(ClearSlot4EUnlessCamera18(cpu.x)));
}

/*
 * @routine 10CB:680 StepListB697ThenStopY
 * @run 70:B680
 * WHAT IT DOES: Steps the animation list at $B697; when it ends, the object's Y speed is cleared and it goes to the next OBJ_2800.
 * HOW: StepObjectFrames, then stores and an increment.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new OBJ_2800's N and Z when the list ended.
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
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    r.first = INC8(OBJ->f2800[slot]);
    return r;
}

/* The step's exit; when the list ended, the INC's N and Z over it. */
static Ended step_then_inc_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    return r.step == FRAME_LIST_ENDED ? ended_then(e, ended_nz(r.first)) : e;
}

void abi_StepListB697ThenStopY(void)
{
    abi_ended(step_then_inc_exit_shim(StepListB697ThenStopY(cpu.x, cpu.y)));
}

/*
 * @routine 10CD:334 StartFirstObjectWhenCamera2
 * @run 71:C334
 * WHAT IT DOES: When the level's Y high byte is 2, the scroll Y low is under $30 and the first object's state and OBJ_2800 are both 0, it sets that object's OBJ_3510 to $40 and OBJ_3568 to $10 and this object goes to the next state.
 * HOW: Compares and stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the last compare or load, or the new state's N and Z (A $10).
 */
ObjArm StartFirstObjectWhenCamera2(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t first, phase;

    r.first = rd8(ScrollYHiScreen);
    r.arm = FO_HIGH;
    if (r.first != 2) {
        return r;
    }
    r.second = RD8(SCROLL->y_lo[0]);
    r.arm = FO_LOW;
    if (r.second >= 0x30) {
        return r;
    }
    first = RD8(OBJ->state[0]);
    phase = RD8(OBJ->f2800[0]);
    r.third = (uint8_t)(first | phase);
    r.arm = FO_BUSY;
    if (r.third != 0) {
        return r;
    }
    WR8(OBJ->f3510[0], 0x40);
    WR8(OBJ->f3568[0], 0x10);
    r.fourth = INC8(OBJ->state[slot]);
    r.arm = FO_STARTED;
    return r;
}

/* The CMP of the screen with 2, the CMP of the scroll low with $30, the ORA's A, LDA #$10 and the INC. */
static Ended start_first_object_when_camera2_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 2);

    if (r.arm == FO_HIGH) {
        return e;
    }
    e = ended_then(e, ended_compare(r.second, 0x30));
    if (r.arm == FO_LOW) {
        return e;
    }
    e = ended_then(e, ended_load(r.third));
    return r.arm == FO_BUSY ? e : ended_then(ended_then(e, ended_load(0x10)), ended_nz(r.fourth));
}

void abi_StartFirstObjectWhenCamera2(void)
{
    abi_ended(start_first_object_when_camera2_exit_shim(StartFirstObjectWhenCamera2(cpu.x)));
}

/*
 * @routine 10CB:702 StepListB728ThenRiseSlowly
 * @run 70:B702
 * WHAT IT DOES: Steps the animation list at $B728; while it runs the object moves and its Y speed loses $0010 a frame; when it ends the object goes to the next OBJ_2800.
 * HOW: StepObjectFrames, ApplyVelocity, a 16-bit subtract, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the subtract's, or the new OBJ_2800's N and Z.
 */
ObjArm StepListB728ThenRiseSlowly(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    Sum low;
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0x28);
    WR8(ZP->temp11, 0xB7);
    jsr_push(0xB70D);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    if (r.step == FRAME_LIST_ENDED) {
        r.first = INC8(OBJ->f2800[slot]);
        return r;
    }
    jsr_push(0xB712);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.second = RD8(OBJ->vy_lo[slot]);
    low = sub_bytes(r.second, 0x10, true);
    WR8(OBJ->vy_lo[slot], low.value);
    r.third = RD8(OBJ->vy_hi[slot]);
    WR8(OBJ->vy_hi[slot], sub_bytes(r.third, 0x00, low.carry).value);
    return r;
}

/* The two SBCs of a 16-bit subtract from the speed kept as low and high bytes: both exits in order. */
static Ended speed_sub_shim(uint8_t low_byte, uint8_t high_byte, uint8_t amount)
{
    Sum low = sub_bytes(low_byte, amount, true);

    return ended_then(ended_of_sum(low), ended_of_sum(sub_bytes(high_byte, 0x00, low.carry)));
}

/* The step; the INC when the list ended; else the move and the subtract. */
static Ended step_list_b728_then_rise_slowly_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.step == FRAME_LIST_ENDED) {
        return ended_then(e, ended_nz(r.first));
    }
    return ended_then(ended_then(e, ended_of_axis_move(r.move)), speed_sub_shim(r.second, r.third, 0x10));
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
    RandomResult random;
    uint8_t y = y_entry, bits;

    WR8(ZP->temp10, 0x03);
    WR8(ZP->temp11, 0xC1);
    jsr_push(0xC0D1);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    jsr_push(0xC0D6);
    random = NextRandom();
    jsr_pop();
    if ((random.value & 0x01) != 0) {
        jsr_push(0xC0DD);
        r.side = SideOfPlayer(slot);
        jsr_pop();
        bits = (uint8_t)(r.side.sides & 0x01);
        jsr_push(0xC0E3);
        r.random = NextRandom();
        jsr_pop();
        r.second = (uint8_t)(r.random.value & 0x7F);
        r.third = rd8((uint16_t)(0xC088 + bits));
        WR8(OBJ->f34b8[slot], (uint8_t)(r.second + r.third));
        r.arm = PA_RANDOM;
    } else {
        jsr_push(0xC0F1);
        r.angle = AngleToPlayer(slot);
        jsr_pop();
        WR8(OBJ->f34b8[slot], r.angle.a);
        r.arm = PA_ANGLE;
    }
    WR8(OBJ->f3510[slot], 0);
    WR8(OBJ->f3568[slot], 0);
    WR8(OBJ->hitbox_on[slot], 2);
    r.first = INC8(OBJ->state[slot]);
    return r;
}

/* The step; when the list ended either the second NextRandom and the add (every flag the first random, SideOfPlayer
 * and the loads left is set again by them) or AngleToPlayer's exit, then LDA #2 and the INC. */
static Ended step_list_c103_then_pick_aim_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.step != FRAME_LIST_ENDED) {
        return e;
    }
    if (r.arm == PA_RANDOM) {
        e = ended_then(ended_then(ended_then(e, ended_with_y(ended_load((uint8_t)(r.side.sides & 0x01)), (uint8_t)(r.side.sides & 0x01))),
                                  ended_of_random(r.random)), ended_of_sum(add_bytes(r.second, r.third, false)));
    } else {
        e = ended_then(e, ended_of_angle(r.angle));
    }
    return ended_then(ended_then(e, ended_load(2)), ended_nz(r.first));
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
    Sum low;
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
    r.second = RD8(OBJ->f3568[slot]);
    low = add_bytes(r.second, 0x0C, false);
    WR8(OBJ->f3568[slot], low.value);
    r.third = RD8(OBJ->f3510[slot]);
    WR8(OBJ->f3510[slot], add_bytes(r.third, 0, low.carry).value);
    r.fourth = RD8(OBJ->state[0x10]);
    if (r.fourth < 6) {
        r.fifth = (uint8_t)(RD8(OBJ->status_flags[slot]) & 0x80);
        r.flag = true;
        r.arm = MA_MOVING;
        if (r.fifth == 0) {
            return r;
        }
    }
    wr8((uint16_t)(0x0002 + slot), 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.sixth = INC8(OBJ->state[slot]);
    r.arm = MA_NEXT;
    return r;
}

/* The step, the angle's LDA, the scale, the move, the length's two adds, the CMP of slot $10's state with 6, the AND
 * of the status bit when under, and the INC when it moves on. */
static Ended step_list_c170_exit_shim(ObjArm r)
{
    Sum low = add_bytes(r.second, 0x0C, false);
    Ended e = ended_then(ended_then(frame_step_exit_shim(r), ended_load(r.first)), ended_of_scale(r.scale));

    e = ended_then(e, ended_of_axis_move(r.move));
    e = ended_then(ended_then(e, ended_of_sum(low)), ended_of_sum(add_bytes(r.third, 0, low.carry)));
    e = ended_then(e, ended_compare(r.fourth, 6));
    if (r.flag) {
        e = ended_then(e, ended_load(r.fifth));
    }
    return r.arm == MA_MOVING ? e : ended_then(e, ended_nz(r.sixth));
}

void abi_StepListC170ThenMoveAlongAngle(void)
{
    abi_ended(step_list_c170_exit_shim(StepListC170ThenMoveAlongAngle(cpu.x, cpu.y)));
}

/*
 * @routine 10CB:6B6 RiseThenPlaceAndDrop
 * @run 70:B6B6
 * WHAT IT DOES: Moves the object along Y by its speed and takes $0018 off its Y speed. When its Y high byte is $FE it puts the object at X $0070, Y $FFC0, gives it X speed $0030 and Y speed $0400, sets its flags to $81 and zTemp $9E to 4 and goes on to the next OBJ_2800.
 * HOW: ApplyVelocityY, a 16-bit subtract, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the new OBJ_2800's N and Z (A 4).
 */
ObjArm RiseThenPlaceAndDrop(uint8_t slot)
{
    ObjArm r = { 0 };
    Sum low;

    jsr_push(0xB6B9);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.second = RD8(OBJ->vy_lo[slot]);
    low = sub_bytes(r.second, 0x18, true);
    WR8(OBJ->vy_lo[slot], low.value);
    r.third = RD8(OBJ->vy_hi[slot]);
    WR8(OBJ->vy_hi[slot], sub_bytes(r.third, 0x00, low.carry).value);
    r.fourth = RD8(OBJ->y_hi[slot]);
    if (r.fourth != 0xFE) {
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
    r.first = INC8(OBJ->f2800[slot]);
    return r;
}

/* The move, the subtract, the CMP of Y high with $FE; there LDA #4 and the INC. */
static Ended rise_then_place_and_drop_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_of_axis_move(r.move), speed_sub_shim(r.second, r.third, 0x18));

    e = ended_then(e, ended_compare(r.fourth, 0xFE));
    return r.fourth != 0xFE ? e : ended_then(ended_then(e, ended_load(4)), ended_nz(r.first));
}

void abi_RiseThenPlaceAndDrop(void)
{
    abi_ended(rise_then_place_and_drop_exit_shim(RiseThenPlaceAndDrop(cpu.x)));
}

/*
 * @routine 10CB:415 RiseThenPlaceWithSound
 * @run 70:B415
 * WHAT IT DOES: Moves the object along Y by its speed and takes $0010 off its Y speed. When its Y high byte is $FE it sends sound $E7, sets its flags to $80, frame $1D, puts it at X $0000, Y $FFE0, gives it X speed $0400 and Y speed $0600 (high bytes) and goes on to the next OBJ_2800.
 * HOW: ApplyVelocityY, a 16-bit subtract, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound's push. Exit: see the wrapper.
 */
ObjArm RiseThenPlaceWithSound(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    Sum low;

    jsr_push(0xB418);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.second = RD8(OBJ->vy_lo[slot]);
    low = sub_bytes(r.second, 0x10, true);
    WR8(OBJ->vy_lo[slot], low.value);
    r.third = RD8(OBJ->vy_hi[slot]);
    WR8(OBJ->vy_hi[slot], sub_bytes(r.third, 0x00, low.carry).value);
    r.fourth = RD8(OBJ->y_hi[slot]);
    if (r.fourth != 0xFE) {
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
    r.first = INC8(OBJ->f2800[slot]);
    return r;
}

/* The move, the subtract, the CMP with $FE; there LDA #$E7 and the sound, then LDA #4 and the INC. */
static SoundEnd rise_then_place_with_sound_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = ended_then(ended_of_axis_move(r.move), speed_sub_shim(r.second, r.third, 0x10));

    e = ended_then(e, ended_compare(r.fourth, 0xFE));
    if (!r.sounded) {
        s.end = e;
        return s;
    }
    s.before = ended_then(e, ended_load(0xE7));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.end = ended_then(ended_load(4), ended_nz(r.first));
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
    Sum low;
    bool carry;

    r.third = RD8(OBJ->state[slot]);
    if (r.third == 0) {
        jsr_push(0xBF57);
        r.first = LoadObjectRecord(slot, &r.second, &carry);
        jsr_pop();
        r.flag = carry;
        WR8(OBJ->flags[slot], 1);
        WR8(OBJ->frame[slot], 0x59);
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0xC0);
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0x30);
        WR8(OBJ->f3510[slot], 0x34);
        return r;
    }
    r.fourth = RD8(OBJ->vy_lo[slot]);
    low = add_bytes(r.fourth, 0x08, false);
    WR8(OBJ->vy_lo[slot], low.value);
    r.fifth = RD8(OBJ->vy_hi[slot]);
    WR8(OBJ->vy_hi[slot], add_bytes(r.fifth, 0, low.carry).value);
    jsr_push(0xBF8F);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.sixth = RD8(OBJ->f3510[slot]);
    if (r.sixth < 0x10) {
        WR8(OBJ->flash_flags[slot], 0x10);
    }
    r.seventh = DEC8(OBJ->f3510[slot]);
    if (r.seventh == 0) {
        ClearObject(slot);
    }
    return r;
}

/* State 0: the state's LDA, the record, LDA #$34. Later the state's LDA, the speed's two adds, the move, the timer's LDA
 * and CMP #$10, LDA #$10 when under, the DEC (ClearObject, a tail jump, leaves it). */
static Ended load_then_fall_and_flash_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.third);
    Sum low;

    if (r.third == 0) {
        return ended_then(ended_then(e, record_exit_shim(r)), ended_load(0x34));
    }
    low = add_bytes(r.fourth, 0x08, false);
    e = ended_then(ended_then(e, ended_of_sum(low)), ended_of_sum(add_bytes(r.fifth, 0, low.carry)));
    e = ended_then(e, ended_of_axis_move(r.move));
    e = ended_then(ended_then(e, ended_load(r.sixth)), ended_compare(r.sixth, 0x10));
    if (r.sixth < 0x10) {
        e = ended_then(e, ended_load(0x10));
    }
    return ended_then(e, ended_nz(r.seventh));
}

void abi_LoadThenFallAndFlash(void)
{
    abi_ended(load_then_fall_and_flash_exit_shim(LoadThenFallAndFlash(cpu.x)));
}

/*
 * @routine 10CB:2E0 CountThenSoleScriptAndSound
 * @run 70:B2E0
 * WHAT IT DOES: Counts OBJ_3510 down; at zero it starts the sole script 0 in slot 3, sends sound $E4, sets its flags to $81, OBJ_3510 to $60 and zTemp $9E to 4 and goes on to the next OBJ_2800.
 * HOW: DEC, StartSoleScriptSlot, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: see the wrapper.
 */
ObjArm CountThenSoleScriptAndSound(uint8_t slot)
{
    ObjArm r = { 0 };
    SoleSlot sole;

    r.first = DEC8(OBJ->f3510[slot]);
    if (r.first != 0) {
        return r;
    }
    push8(slot);
    jsr_push(0xB2EC);
    sole = StartSoleScriptSlot(0, 3);
    jsr_pop();
    r.second = sole.script;
    r.third = sole.busy_kind;
    r.flag = sole.started;
    slot = pull8();
    r.fourth = slot;
    r.sounded = true;
    r.command = 0xE4;
    jsr_push(0xB2F2);
    r.call = SoundCommand(0xE4, slot, sole.script);
    jsr_pop();
    WR8(OBJ->flags[slot], 0x81);
    WR8(OBJ->f3510[slot], 0x60);
    wr8(0x209E, 4);
    r.fifth = INC8(OBJ->f2800[slot]);
    return r;
}

/* The DEC's N and Z; at zero LDA #0, StartSoleScriptSlot's exit, the PLX, LDA #$E4 and the sound; then LDA #4 and the INC. */
static SoundEnd count_then_sole_script_and_sound_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    SoleSlot sole = { r.second, r.third, r.flag };
    Ended e = ended_nz(r.first);

    if (!r.sounded) {
        s.end = e;
        return s;
    }
    e = ended_then(ended_then(ended_then(e, ended_load(0)), ended_of_sole(sole)), ended_nz(r.fourth));
    s.before = ended_then(e, ended_load(0xE4));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.end = ended_then(ended_load(4), ended_nz(r.fifth));
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
    uint8_t count, y, entry;

    count = RD8(OBJ->f3568[slot]);
    r.first = count;
    r.arm = ST_NEGATIVE;
    if ((count & 0x80) != 0) {
        return r;
    }
    INC8(OBJ->f3568[slot]);
    for (y = 0; y < 8; y++) {
        entry = rd8((uint16_t)(0xB40D + y));
        if (count == entry) {
            break;
        }
    }
    r.arm = ST_NOT_FOUND;
    if (y >= 8) {
        return r;
    }
    r.y = y;
    push8(slot);
    jsr_push(0xB40B);
    r.second = StartScriptSlotKind1((uint8_t)(y + 0x98), 5);
    jsr_pop();
    r.third = pull8();
    r.arm = ST_STARTED;
    return r;
}

/* Negative: the LDA. Not in the table: the last CMP and INY, CPY #8 (A the count, Y 8, Z and C set). Found: the TYA and
 * TAY, CLC and ADC #$98, the script's A and Y, the PLX's N and Z (they set every register and flag the search left). */
static Ended start_script_when_count_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == ST_NEGATIVE) {
        return ended_load(r.first);
    }
    if (r.arm == ST_NOT_FOUND) {
        return ended_then(ended_load(r.first), ended_with_y(ended_with_carry(ended_nz(0), true), 8));
    }
    e = ended_then(ended_with_y(ended_load(r.y), r.y), ended_of_sum(add_bytes(r.y, 0x98, false)));
    return ended_then(ended_then(e, ended_with_y(ended_load(r.second), r.second)), ended_nz(r.third));
}

void abi_StartScriptWhenCountInTable(void)
{
    abi_ended(start_script_when_count_exit_shim(StartScriptWhenCountInTable(cpu.x)));
}

/*
 * @routine 10CB:45F RiseThenCueMusic
 * @run 70:B45F
 * WHAT IT DOES: Moves the object along Y and takes $0030 off its Y speed. Once the speed's high byte is negative it steps the animation list at $B4AA; when that ends it sets bit 4 of zField8E, sends sound $FF, clears bits 3 and 4, plays the music again (CdPlayAgain), sets bit 2, clears zTemp $9E and OBJ_3250 to 2, clears its animation step and timer, goes on to the next state and clears OBJ_2800.
 * HOW: ApplyVelocity, a 16-bit subtract, StepObjectFrames, SoundCommand, CdPlayAgain, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
 */
ObjArm RiseThenCueMusic(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    Sum low;
    uint8_t y = y_entry, flags, value;

    jsr_push(0xB462);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.second = RD8(OBJ->vy_lo[slot]);
    low = sub_bytes(r.second, 0x30, true);
    WR8(OBJ->vy_lo[slot], low.value);
    r.third = RD8(OBJ->vy_hi[slot]);
    value = sub_bytes(r.third, 0x00, low.carry).value;
    WR8(OBJ->vy_hi[slot], value);
    r.arm = CM_FALLING;
    if ((value & 0x80) == 0) {
        return r;
    }
    WR8(ZP->temp10, 0xAA);
    WR8(ZP->temp11, 0xB4);
    jsr_push(0xB480);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = CM_STEPPED;
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
    r.played = true;
    flags = (uint8_t)(RD8(ZP->field8e) | 0x04);
    WR8(ZP->field8e, flags);
    wr8(0x209E, 0);
    WR8(OBJ->hitbox_on[slot], 2);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.first = INC8(OBJ->state[slot]);
    WR8(OBJ->f2800[slot], 0);
    r.arm = CM_CUED;
    return r;
}

/* The move and the subtract; once rising, LDA #$B4, LDA #$AA and the step; when the list ended LDA #$FF twice before the
 * sound, then the music, then LDA #2 and the INC. */
static SoundPlayEnd rise_then_cue_music_exit_shim(ObjArm r)
{
    SoundPlayEnd s = { { 0 }, false, 0, { 0 }, false, { 0 }, { 0 } };
    Ended e = ended_then(ended_of_axis_move(r.move), speed_sub_shim(r.second, r.third, 0x30));

    if (r.arm == CM_FALLING) {
        s.pre = e;
        return s;
    }
    e = ended_then(ended_then(ended_then(e, ended_load(0xB4)), ended_load(0xAA)), frame_step_exit_shim(r));
    if (r.arm == CM_STEPPED) {
        s.pre = e;
        return s;
    }
    s.pre = ended_then(ended_then(e, ended_load(0xFF)), ended_load(0xFF));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.played = r.played;
    s.track = r.track;
    s.end = ended_then(ended_load(2), ended_nz(r.first));
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
    bool carry;

    jsr_push(0xB290);
    r.first = LoadObjectRecord(slot, &r.second, &carry);
    jsr_pop();
    r.flag = carry;
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

/* The record, LDA #$C0, each script's A and Y, the PLX's N and Z. */
static Ended load_then_start_two_scripts_exit_shim(ObjArm r)
{
    Ended e = ended_then(record_exit_shim(r), ended_load(0xC0));

    e = ended_then(e, ended_with_y(ended_load(r.third), r.third));
    e = ended_then(e, ended_with_y(ended_load(r.fourth), r.fourth));
    return ended_then(e, ended_nz(r.fifth));
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
    uint8_t y, kind;

    r.first = RD8(OBJ->f37d0[slot]);
    if (r.first != 0) {
        r.second = DEC8(OBJ->f37d0[slot]);
        r.arm = SF_COUNTING;
        return r;
    }
    WR8(OBJ->f37d0[slot], 0x1E);
    for (y = 0x18;; y++) {
        kind = rd8((uint16_t)(0x2750 + y));
        r.y = y;
        r.third = kind;
        if (kind == 0) {
            break;
        }
        if ((y + 1) >= 0x20) {
            r.arm = SF_FULL;
            return r;
        }
    }
    jsr_push(0xB5C9);
    ClearObjectInY(y);
    jsr_pop();
    jsr_push(0xB5CE);
    r.fourth = StartObjectAtPlace(0x26, slot, y);
    jsr_pop();
    r.arm = SF_SPAWNED;
    return r;
}

/* The count's LDA and DEC; at zero LDA #$1E, then for each slot looked at the LDY's N and Z with its Y, the kind's LDA
 * (only the last one's A stands) and, when taken, INY and CPY #$20; then LDA #$26 and the start's A with Y the slot. */
static Ended spawn_in_free_slot_every30_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);
    uint8_t y;

    if (r.arm == SF_COUNTING) {
        return ended_then(e, ended_nz(r.second));
    }
    e = ended_then(e, ended_load(0x1E));
    for (y = 0x18; y < r.y; y++) {
        e = ended_then(e, ended_with_y(ended_nz(0x18), y));
        e = ended_then(e, ended_with_y(ended_with_carry(ended_nz((uint8_t)(y + 1 - 0x20)), (y + 1) >= 0x20), (uint8_t)(y + 1)));
    }
    e = ended_then(ended_then(e, ended_with_y(ended_nz(0x18), r.y)), ended_load(r.third));
    if (r.arm == SF_FULL) {
        return ended_then(e, ended_with_y(ended_with_carry(ended_nz((uint8_t)(r.y + 1 - 0x20)), true), (uint8_t)(r.y + 1)));
    }
    return ended_then(ended_then(e, ended_load(0x26)), ended_with_y(ended_load(r.fourth), r.y));
}

void abi_SpawnInFreeSlotEvery30(void)
{
    abi_ended(spawn_in_free_slot_every30_exit_shim(SpawnInFreeSlotEvery30(cpu.x)));
}

/*
 * @routine 10CB:3BE CountScriptsThenStepList
 * @run 70:B3BE
 * WHAT IT DOES: Runs StartScriptWhenCountInTable, then steps the animation list at $B3D2; when it ends the object goes to the next OBJ_2800.
 * HOW: StartScriptWhenCountInTable, StepObjectFrames, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: the step, or the new OBJ_2800's N and Z when the list ended.
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
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    r.first = INC8(OBJ->f2800[slot]);
    return r;
}

/* StartScriptWhenCountInTable's exit, LDA #$D2, LDA #$B3, the step, and the INC when the list ended. */
static Ended count_scripts_then_step_list_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_then(start_script_when_count_exit_shim(obj_arm(r.sub)), ended_load(0xD2)), ended_load(0xB3));

    return ended_then(e, step_then_inc_exit_shim(r));
}

void abi_CountScriptsThenStepList(void)
{
    abi_ended(count_scripts_then_step_list_exit_shim(CountScriptsThenStepList(cpu.x, cpu.y)));
}
