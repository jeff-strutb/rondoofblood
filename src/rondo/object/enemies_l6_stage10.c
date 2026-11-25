/*
 * TEMPORARY: the stage object routines of lot L6 for the overlays at sectors 10C9 to 10CF (enemy kinds not yet identified), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
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

/*
 * @routine 10CD:181 StepListC192ThenClear
 * @run 71:C181
 * WHAT IT DOES: Steps the animation list at $C192; when it ends the object is cleared.
 * HOW: StepObjectFrames, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step.
 */
Ended StepListC192ThenClear(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    uint8_t y = y_entry;
    Ended e;

    WR8(ZP->temp10, 0x92);
    WR8(ZP->temp11, 0xC1);
    jsr_push(0xC18C);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step == FRAME_LIST_ENDED) {
        ClearObject(slot);
    }
    return e;
}

void abi_StepListC192ThenClear(void)
{
    abi_ended(StepListC192ThenClear(cpu.x, cpu.y));
}

/*
 * @routine 10CB:2CB ClearSlot4EUnlessCamera18
 * @run 70:B2CB
 * WHAT IT DOES: Unless the camera mode is $18, clears the object in slot $4E, sets OBJ_3510 to $40 and goes on to the next OBJ_2800.
 * HOW: A compare, ClearObjectInY, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the new OBJ_2800's N and Z (A $40).
 */
Ended ClearSlot4EUnlessCamera18(uint8_t slot)
{
    uint8_t mode, phase;
    Ended e;

    mode = rd8(CameraMode);
    e = ended_compare(mode, 0x18);
    if (mode == 0x18) {
        return e;
    }
    jsr_push(0xB2D7);
    ClearObjectInY(0x4E);
    jsr_pop();
    WR8(OBJ->f3510[slot], 0x40);
    phase = INC8(OBJ->f2800[slot]);
    return ended_then(ended_then(ended_with_y(e, 0x4E), ended_load(0x40)), ended_nz(phase));
}

void abi_ClearSlot4EUnlessCamera18(void)
{
    abi_ended(ClearSlot4EUnlessCamera18(cpu.x));
}

/*
 * @routine 10CB:680 StepListB697ThenStopY
 * @run 70:B680
 * WHAT IT DOES: Steps the animation list at $B697; when it ends, the object's Y speed is cleared and it goes to the next OBJ_2800.
 * HOW: StepObjectFrames, then stores and an increment.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new OBJ_2800's N and Z when the list ended.
 */
Ended StepListB697ThenStopY(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    uint8_t y = y_entry;
    Ended e;

    WR8(ZP->temp10, 0x97);
    WR8(ZP->temp11, 0xB6);
    jsr_push(0xB68B);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    uint8_t phase;

    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    phase = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(phase));
}

void abi_StepListB697ThenStopY(void)
{
    abi_ended(StepListB697ThenStopY(cpu.x, cpu.y));
}

/*
 * @routine 10CD:334 StartFirstObjectWhenCamera2
 * @run 71:C334
 * WHAT IT DOES: When the level's Y high byte is 2, the scroll Y low is under $30 and the first object's state and OBJ_2800 are both 0, it sets that object's OBJ_3510 to $40 and OBJ_3568 to $10 and this object goes to the next state.
 * HOW: Compares and stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the last compare or load, or the new state's N and Z (A $10).
 */
Ended StartFirstObjectWhenCamera2(uint8_t slot)
{
    uint8_t high, low, first, phase, state;
    Ended e;

    high = rd8(ScrollYHiScreen);
    e = ended_compare(high, 2);
    if (high != 2) {
        return e;
    }
    low = RD8(SCROLL->y_lo[0]);
    e = then(e, ended_compare(low, 0x30));
    if (low >= 0x30) {
        return e;
    }
    first = RD8(OBJ->state[0]);
    phase = RD8(OBJ->f2800[0]);
    e = then(e, ended_load((uint8_t)(first | phase)));
    if ((first | phase) != 0) {
        return e;
    }
    WR8(OBJ->f3510[0], 0x40);
    WR8(OBJ->f3568[0], 0x10);
    state = INC8(OBJ->state[slot]);
    return then(then(e, ended_load(0x10)), ended_nz(state));
}

void abi_StartFirstObjectWhenCamera2(void)
{
    abi_ended(StartFirstObjectWhenCamera2(cpu.x));
}

/*
 * @routine 10CB:702 StepListB728ThenRiseSlowly
 * @run 70:B702
 * WHAT IT DOES: Steps the animation list at $B728; while it runs the object moves and its Y speed loses $0010 a frame; when it ends the object goes to the next OBJ_2800.
 * HOW: StepObjectFrames, ApplyVelocity, a 16-bit subtract, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the subtract's, or the new OBJ_2800's N and Z.
 */
Ended StepListB728ThenRiseSlowly(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    AxisMove move;
    Sum low, high;
    uint8_t y = y_entry, phase;
    Ended e;

    WR8(ZP->temp10, 0x28);
    WR8(ZP->temp11, 0xB7);
    jsr_push(0xB70D);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step == FRAME_LIST_ENDED) {
        phase = INC8(OBJ->f2800[slot]);
        return then(e, ended_nz(phase));
    }
    jsr_push(0xB712);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    low = sub_bytes(RD8(OBJ->vy_lo[slot]), 0x10, true);
    WR8(OBJ->vy_lo[slot], low.value);
    high = sub_bytes(RD8(OBJ->vy_hi[slot]), 0x00, low.carry);
    WR8(OBJ->vy_hi[slot], high.value);
    return then(then(e, ended_of_sum(low)), ended_of_sum(high));
}

void abi_StepListB728ThenRiseSlowly(void)
{
    abi_ended(StepListB728ThenRiseSlowly(cpu.x, cpu.y));
}

/*
 * @routine 10CD:0C6 StepListC103ThenPickAim
 * @run 71:C0C6
 * WHAT IT DOES: Steps the animation list at $C103; when it ends the object picks OBJ_34B8: half the time the angle to the player, otherwise a random $00 to $7F plus the byte the table at $C088 gives for the player's side; it then clears OBJ_3510 and OBJ_3568, sets OBJ_3250 to 2 and goes on to the next state.
 * HOW: StepObjectFrames, NextRandom, SideOfPlayer, AngleToPlayer, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new state's N and Z (A 2) when the list ended.
 */
Ended StepListC103ThenPickAim(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    RandomResult random;
    SideResult side;
    AngleExit angle;
    Sum sum;
    uint8_t y = y_entry, bits, state;
    Ended e;

    WR8(ZP->temp10, 0x03);
    WR8(ZP->temp11, 0xC1);
    jsr_push(0xC0D1);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    jsr_push(0xC0D6);
    random = NextRandom();
    jsr_pop();
    e = then(then(e, ended_of_random(random)), ended_load((uint8_t)(random.value & 0x01)));
    if ((random.value & 0x01) != 0) {
        jsr_push(0xC0DD);
        side = SideOfPlayer(slot);
        jsr_pop();
        bits = (uint8_t)(side.sides & 0x01);
        e = then(then(e, ended_of_side(side)), ended_with_y(ended_load(bits), bits));
        jsr_push(0xC0E3);
        random = NextRandom();
        jsr_pop();
        e = then(then(e, ended_of_random(random)), ended_load((uint8_t)(random.value & 0x7F)));
        sum = add_bytes((uint8_t)(random.value & 0x7F), rd8((uint16_t)(0xC088 + bits)), false);
        WR8(OBJ->f34b8[slot], sum.value);
        e = then(e, ended_of_sum(sum));
    } else {
        jsr_push(0xC0F1);
        angle = AngleToPlayer(slot);
        jsr_pop();
        WR8(OBJ->f34b8[slot], angle.a);
        e = then(e, ended_of_angle(angle));
    }
    WR8(OBJ->f3510[slot], 0);
    WR8(OBJ->f3568[slot], 0);
    WR8(OBJ->hitbox_on[slot], 2);
    state = INC8(OBJ->state[slot]);
    return then(then(e, ended_load(2)), ended_nz(state));
}

void abi_StepListC103ThenPickAim(void)
{
    abi_ended(StepListC103ThenPickAim(cpu.x, cpu.y));
}

/*
 * @routine 10CD:126 StepListC170ThenMoveAlongAngle
 * @run 71:C126
 * WHAT IT DOES: Steps the animation list at $C170, sets the object's speed from its angle OBJ_34B8 with the length OBJ_3510:OBJ_3568, moves it and adds $000C to the length. When the object in slot $10 is in state 6 or more, or this object's OBJ_31A0 has bit 7 set, it clears its animation step and timer and goes on to the next state (and writes 0 to the byte at $0002 plus its slot).
 * HOW: StepObjectFrames, SetVelocityFromAngle, ApplyVelocity, a 16-bit add, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the last test, or the new state's N and Z.
 */
Ended StepListC170ThenMoveAlongAngle(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    ScaleResult scale;
    AxisMove move;
    Sum low, high;
    uint8_t y = y_entry, value, first, state, status;
    Ended e;

    WR8(ZP->temp10, 0x70);
    WR8(ZP->temp11, 0xC1);
    jsr_push(0xC131);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    value = RD8(OBJ->f3568[slot]);
    WR8(ZP->temp00, value);
    value = RD8(OBJ->f3510[slot]);
    WR8(ZP->temp01, value);
    value = RD8(OBJ->f34b8[slot]);
    e = then(e, ended_load(value));
    jsr_push(0xC141);
    scale = SetVelocityFromAngle(value, slot);
    jsr_pop();
    e = then(e, ended_of_scale(scale));
    jsr_push(0xC144);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    low = add_bytes(RD8(OBJ->f3568[slot]), 0x0C, false);
    WR8(OBJ->f3568[slot], low.value);
    high = add_bytes(RD8(OBJ->f3510[slot]), 0, low.carry);
    WR8(OBJ->f3510[slot], high.value);
    e = then(then(e, ended_of_sum(low)), ended_of_sum(high));
    first = RD8(OBJ->state[0x10]);
    e = then(e, ended_compare(first, 6));
    if (first < 6) {
        status = (uint8_t)(RD8(OBJ->status_flags[slot]) & 0x80);
        e = then(e, ended_load(status));
        if (status == 0) {
            return e;
        }
    }
    wr8((uint16_t)(0x0002 + slot), 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    state = INC8(OBJ->state[slot]);
    return then(e, ended_nz(state));
}

void abi_StepListC170ThenMoveAlongAngle(void)
{
    abi_ended(StepListC170ThenMoveAlongAngle(cpu.x, cpu.y));
}

/*
 * @routine 10CB:6B6 RiseThenPlaceAndDrop
 * @run 70:B6B6
 * WHAT IT DOES: Moves the object along Y by its speed and takes $0018 off its Y speed. When its Y high byte is $FE it puts the object at X $0070, Y $FFC0, gives it X speed $0030 and Y speed $0400, sets its flags to $81 and zTemp $9E to 4 and goes on to the next OBJ_2800.
 * HOW: ApplyVelocityY, a 16-bit subtract, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the new OBJ_2800's N and Z (A 4).
 */
Ended RiseThenPlaceAndDrop(uint8_t slot)
{
    AxisMove move;
    Sum low, high;
    uint8_t position, phase;
    Ended e;

    jsr_push(0xB6B9);
    move = ApplyVelocityY(slot);
    jsr_pop();
    low = sub_bytes(RD8(OBJ->vy_lo[slot]), 0x18, true);
    WR8(OBJ->vy_lo[slot], low.value);
    high = sub_bytes(RD8(OBJ->vy_hi[slot]), 0x00, low.carry);
    WR8(OBJ->vy_hi[slot], high.value);
    e = then(then(ended_of_axis_move(move), ended_of_sum(low)), ended_of_sum(high));
    position = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_compare(position, 0xFE));
    if (position != 0xFE) {
        return e;
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
    phase = INC8(OBJ->f2800[slot]);
    return then(then(e, ended_load(4)), ended_nz(phase));
}

void abi_RiseThenPlaceAndDrop(void)
{
    abi_ended(RiseThenPlaceAndDrop(cpu.x));
}

/*
 * @routine 10CB:415 RiseThenPlaceWithSound
 * @run 70:B415
 * WHAT IT DOES: Moves the object along Y by its speed and takes $0010 off its Y speed. When its Y high byte is $FE it sends sound $E7, sets its flags to $80, frame $1D, puts it at X $0000, Y $FFE0, gives it X speed $0400 and Y speed $0600 (high bytes) and goes on to the next OBJ_2800.
 * HOW: ApplyVelocityY, a 16-bit subtract, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound's push. Exit: see the wrapper.
 */
SoundEnd RiseThenPlaceWithSound(uint8_t slot, uint8_t y_entry)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    AxisMove move;
    Sum low, high;
    uint8_t position, phase;
    Ended e;

    jsr_push(0xB418);
    move = ApplyVelocityY(slot);
    jsr_pop();
    low = sub_bytes(RD8(OBJ->vy_lo[slot]), 0x10, true);
    WR8(OBJ->vy_lo[slot], low.value);
    high = sub_bytes(RD8(OBJ->vy_hi[slot]), 0x00, low.carry);
    WR8(OBJ->vy_hi[slot], high.value);
    e = then(then(ended_of_axis_move(move), ended_of_sum(low)), ended_of_sum(high));
    position = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_compare(position, 0xFE));
    if (position != 0xFE) {
        r.end = e;
        return r;
    }
    r.before = then(e, ended_load(0xE7));
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
    phase = INC8(OBJ->f2800[slot]);
    r.end = then(ended_load(4), ended_nz(phase));
    return r;
}

void abi_RiseThenPlaceWithSound(void)
{
    abi_sound_end(RiseThenPlaceWithSound(cpu.x, cpu.y));
}

/*
 * @routine 10CC:74F LoadThenFallAndFlash
 * @run 70:BF4F
 * WHAT IT DOES: In state 0 loads the object's record and sets flags 1, frame $59, X speed $00C0, Y speed $0030 and OBJ_3510 $34. Later it adds $0008 to its Y speed, moves, sets its flash flags to $10 while OBJ_3510 is under $10, counts OBJ_3510 down and clears the object at zero.
 * HOW: LoadObjectRecord, a 16-bit add, ApplyVelocity, DEC, ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: the record's registers and A $34 in state 0; else the count.
 */
Ended LoadThenFallAndFlash(uint8_t slot)
{
    AxisMove move;
    Sum low, high;
    uint8_t state, nz, last, timer, left;
    bool carry;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(0xBF57);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        WR8(OBJ->flags[slot], 1);
        WR8(OBJ->frame[slot], 0x59);
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0xC0);
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0x30);
        WR8(OBJ->f3510[slot], 0x34);
        return then(then(ended_load(state), ended_of_record(last, nz, carry)), ended_load(0x34));
    }
    low = add_bytes(RD8(OBJ->vy_lo[slot]), 0x08, false);
    WR8(OBJ->vy_lo[slot], low.value);
    high = add_bytes(RD8(OBJ->vy_hi[slot]), 0, low.carry);
    WR8(OBJ->vy_hi[slot], high.value);
    e = then(then(ended_load(state), ended_of_sum(low)), ended_of_sum(high));
    jsr_push(0xBF8F);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    timer = RD8(OBJ->f3510[slot]);
    e = then(e, ended_load(timer));
    e = then(e, ended_compare(timer, 0x10));
    if (timer < 0x10) {
        WR8(OBJ->flash_flags[slot], 0x10);
        e = then(e, ended_load(0x10));
    }
    left = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(left));
    if (left == 0) {
        ClearObject(slot);
    }
    return e;
}

void abi_LoadThenFallAndFlash(void)
{
    abi_ended(LoadThenFallAndFlash(cpu.x));
}

/*
 * @routine 10CB:2E0 CountThenSoleScriptAndSound
 * @run 70:B2E0
 * WHAT IT DOES: Counts OBJ_3510 down; at zero it starts the sole script 0 in slot 3, sends sound $E4, sets its flags to $81, OBJ_3510 to $60 and zTemp $9E to 4 and goes on to the next OBJ_2800.
 * HOW: DEC, StartSoleScriptSlot, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: see the wrapper.
 */
SoundEnd CountThenSoleScriptAndSound(uint8_t slot)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    SoleSlot sole;
    uint8_t left, phase;
    Ended e;

    left = DEC8(OBJ->f3510[slot]);
    e = ended_nz(left);
    if (left != 0) {
        r.end = e;
        return r;
    }
    push8(slot);
    e = then(e, ended_load(0));
    jsr_push(0xB2EC);
    sole = StartSoleScriptSlot(0, 3);
    jsr_pop();
    slot = pull8();
    e = then(then(e, ended_of_sole(sole)), ended_nz(slot));
    r.before = then(e, ended_load(0xE4));
    r.sounded = true;
    r.command = 0xE4;
    jsr_push(0xB2F2);
    r.call = SoundCommand(0xE4, slot, sole.script);
    jsr_pop();
    WR8(OBJ->flags[slot], 0x81);
    WR8(OBJ->f3510[slot], 0x60);
    wr8(0x209E, 4);
    phase = INC8(OBJ->f2800[slot]);
    r.end = then(ended_load(4), ended_nz(phase));
    return r;
}

void abi_CountThenSoleScriptAndSound(void)
{
    abi_sound_end(CountThenSoleScriptAndSound(cpu.x));
}

/*
 * @routine 10CB:3ED StartScriptWhenCountInTable
 * @run 70:B3ED
 * WHAT IT DOES: While OBJ_3568 is not negative it counts it up and looks its old value up in the eight-byte table at $B40D; when found at index n it starts script $98 plus n in slot 5.
 * HOW: A search loop, StartScriptSlotKind1.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the script start's registers.
 */
Ended StartScriptWhenCountInTable(uint8_t slot)
{
    uint8_t count, y, script, entry;
    Sum sum;
    Ended e;

    count = RD8(OBJ->f3568[slot]);
    e = ended_load(count);
    if ((count & 0x80) != 0) {
        return e;
    }
    INC8(OBJ->f3568[slot]);
    for (y = 0; y < 8; y++) {
        entry = rd8((uint16_t)(0xB40D + y));
        e = then(e, ended_compare(count, entry));
        if (count == entry) {
            break;
        }
        e = then(e, ended_with_y(ended_with_carry(ended_nz((uint8_t)(y + 1 - 8)), (y + 1) >= 8), (uint8_t)(y + 1)));
    }
    if (y >= 8) {
        return e;
    }
    e = then(e, ended_with_y(ended_load(y), y));
    sum = add_bytes(y, 0x98, false);
    e = then(e, ended_of_sum(sum));
    push8(slot);
    jsr_push(0xB40B);
    script = StartScriptSlotKind1(sum.value, 5);
    jsr_pop();
    slot = pull8();
    return then(then(e, ended_with_y(ended_load(script), script)), ended_nz(slot));
}

void abi_StartScriptWhenCountInTable(void)
{
    abi_ended(StartScriptWhenCountInTable(cpu.x));
}

/*
 * @routine 10CB:45F RiseThenCueMusic
 * @run 70:B45F
 * WHAT IT DOES: Moves the object along Y and takes $0030 off its Y speed. Once the speed's high byte is negative it steps the animation list at $B4AA; when that ends it sets bit 4 of zField8E, sends sound $FF, clears bits 3 and 4, plays the music again (CdPlayAgain), sets bit 2, clears zTemp $9E and OBJ_3250 to 2, clears its animation step and timer, goes on to the next state and clears OBJ_2800.
 * HOW: ApplyVelocity, a 16-bit subtract, StepObjectFrames, SoundCommand, CdPlayAgain, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
 */
SoundPlayEnd RiseThenCueMusic(uint8_t slot, uint8_t y_entry)
{
    SoundPlayEnd r = { { 0 }, false, 0, { 0 }, false, { 0 }, { 0 } };
    AxisMove move;
    FrameStep step;
    Sum low, high;
    uint8_t y = y_entry, flags, state;
    Ended e;

    jsr_push(0xB462);
    move = ApplyVelocity(slot);
    jsr_pop();
    low = sub_bytes(RD8(OBJ->vy_lo[slot]), 0x30, true);
    WR8(OBJ->vy_lo[slot], low.value);
    high = sub_bytes(RD8(OBJ->vy_hi[slot]), 0x00, low.carry);
    WR8(OBJ->vy_hi[slot], high.value);
    e = then(then(ended_of_axis_move(move), ended_of_sum(low)), ended_of_sum(high));
    if ((high.value & 0x80) == 0) {
        r.pre = e;
        return r;
    }
    WR8(ZP->temp10, 0xAA);
    WR8(ZP->temp11, 0xB4);
    jsr_push(0xB480);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = then(then(then(e, ended_load(0xB4)), ended_load(0xAA)), ended_of_frame_step(step, step != FRAME_HELD, y));
    if (step != FRAME_LIST_ENDED) {
        r.pre = e;
        return r;
    }
    flags = (uint8_t)(RD8(ZP->field8e) | 0x10);
    WR8(ZP->field8e, flags);
    r.pre = then(then(e, ended_load(0xFF)), ended_load(0xFF));
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
    state = INC8(OBJ->state[slot]);
    WR8(OBJ->f2800[slot], 0);
    r.end = then(ended_load(2), ended_nz(state));
    return r;
}

void abi_RiseThenCueMusic(void)
{
    SoundPlayEnd r = RiseThenCueMusic(cpu.x, cpu.y);

    abi_ended(r.pre);
    if (r.sounded) {
        sound_command_exit_shim(r.command, r.call);
    }
    if (r.played) {
        play_track_exit_shim(r.track);
    }
    abi_ended(r.end);
}
