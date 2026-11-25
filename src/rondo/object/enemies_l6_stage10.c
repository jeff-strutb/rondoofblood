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
