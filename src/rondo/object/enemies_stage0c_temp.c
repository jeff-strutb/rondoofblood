/*
 * TEMPORARY: the stage 0C61 enemies and their helpers (disc sectors 0CA9 to 0CAD), by routine until each enemy kind is
 * identified. Bodies return how the routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the
 * abi_ wrappers show it.
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

static Ended of_sum(Sum sum)
{
    return ended_of_sum(sum);
}

/* The steps of an animation list at `list` (zTemp10 first or zTemp11 first, as the assembly stores them). */
static FrameStep step_list(uint8_t slot, uint16_t list, uint16_t ret, uint8_t *y, bool high_first)
{
    FrameStep step;

    if (high_first) {
        WR8(ZP->temp11, (uint8_t)(list >> 8));
        WR8(ZP->temp10, (uint8_t)list);
    } else {
        WR8(ZP->temp10, (uint8_t)list);
        WR8(ZP->temp11, (uint8_t)(list >> 8));
    }
    jsr_push(ret);
    step = StepObjectFrames(slot, y);
    jsr_pop();
    return step;
}

/*
 * @routine 0CAB:175 FallingWithGravity
 * @run 70:B175
 * WHAT IT DOES: Steps the animation list at $B187, applies unlimited gravity and moves the object.
 * HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as ApplyVelocity, Y as the step left it.
 */
Ended FallingWithGravity(uint8_t slot)
{
    FrameStep step;
    GravityResult gravity;
    AxisMove move;
    uint8_t y = 0;
    Ended e;

    step = step_list(slot, 0xB187, 0xB180, &y, false);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    jsr_push(0xB183);
    gravity = ApplyGravityUnlimited(slot);
    jsr_pop();
    jsr_push(0xB186);
    move = ApplyVelocity(slot);
    jsr_pop();
    (void)gravity;
    return then(e, ended_of_axis_move(move));
}

void abi_FallingWithGravity(void)
{
    abi_ended(FallingWithGravity(cpu.x));
}

/*
 * @routine 0CAB:203 StartWhenHurt
 * @run 70:B203
 * WHAT IT DOES: An object in state 3 that has taken a hit of 5 or more resets its animation, enters state 6 and clears OBJ_2800.
 * HOW: Two compares.
 * WAITS: no.
 * Entry: X object. Exit: the last compare or load.
 */
Ended StartWhenHurt(uint8_t slot)
{
    uint8_t state, hit;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_compare(state, 3);
    if (state != 3) {
        return e;
    }
    hit = RD8(OBJ->hit_taken[slot]);
    e = then(e, ended_compare(hit, 5));
    if (hit < 5) {
        return e;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 6);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_load(6));
}

void abi_StartWhenHurt(void)
{
    abi_ended(StartWhenHurt(cpu.x));
}

/*
 * @routine 0CAB:220 EnterStateSevenOnFlag
 * @run 70:B220
 * WHAT IT DOES: When bit 7 of the object's status flags is set and its state is below 7, it faces right, resets its animation, enters state 7 and clears OBJ_2800.
 * HOW: Two compares.
 * Untraced: every arm past the flag test (B227 to B23E).
 * WAITS: no.
 * Entry: X object. Exit: the last AND, compare or load.
 */
Ended EnterStateSevenOnFlag(uint8_t slot)
{
    uint8_t flags, state;
    Ended e;

    flags = RD8(OBJ->status_flags[slot]);
    flags = (uint8_t)(flags & 0x80);
    e = ended_load(flags);
    if (flags == 0) {
        return e;
    }
    state = RD8(OBJ->state[slot]);
    e = then(e, ended_compare(state, 7));
    if (state >= 7) {
        return e;
    }
    WR8(OBJ->facing_left[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 7);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_load(7));
}

void abi_EnterStateSevenOnFlag(void)
{
    abi_ended(EnterStateSevenOnFlag(cpu.x));
}

/*
 * @routine 0CAB:304 StartWithGuard
 * @run 70:B304
 * WHAT IT DOES: Loads the object's record and starts a kind $25 object at its place in slot $11; its timer ends as $80.
 * HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: A $80, Y $11, C from the record, N set.
 */
Ended StartWithGuard(uint8_t slot)
{
    uint8_t nz, last, placed;
    bool carry;
    Ended e;

    jsr_push(0xB307);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    jsr_push(0xB30C);
    ClearObjectInY(0x11);
    jsr_pop();
    jsr_push(0xB311);
    placed = StartObjectAtPlace(0x25, slot, 0x11);
    jsr_pop();
    e = then(e, ended_with_y(ended_load(placed), 0x11));
    WR8(OBJ->life_timer[slot], 0xE0);
    WR8(OBJ->life_timer[slot], 0x80);
    return then(e, ended_load(0x80));
}

void abi_StartWithGuard(void)
{
    abi_ended(StartWithGuard(cpu.x));
}

/*
 * @routine 0CAB:389 SlideAngleDown
 * @run 70:B389
 * WHAT IT DOES: Takes one off the 16-bit angle word (OBJ_3568 high, OBJ_35C0 low, in steps of $0100 less 0 on the low byte as the code subtracts $0100); when the high byte
 * reaches 0 the object's OBJ_2800 goes up one.
 * HOW: A 16-bit subtract.
 * WAITS: no.
 * Entry: X object. Exit: the last subtract or INC.
 */
Ended SlideAngleDown(uint8_t slot)
{
    uint8_t low, high, state;
    Sum sum;
    Ended e;

    low = RD8(OBJ->angle_lo[slot]);
    sum = sub_bytes(low, 0, true);
    WR8(OBJ->angle_lo[slot], sum.value);
    e = of_sum(sum);
    high = RD8(OBJ->angle_hi[slot]);
    sum = sub_bytes(high, 1, sum.carry);
    WR8(OBJ->angle_hi[slot], sum.value);
    e = then(e, of_sum(sum));
    if (sum.value != 0) {
        return e;
    }
    state = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(state));
}

void abi_SlideAngleDown(void)
{
    abi_ended(SlideAngleDown(cpu.x));
}

/*
 * @routine 0CAB:3A0 SlideAngleUp
 * @run 70:B3A0
 * WHAT IT DOES: Adds $00C0 to the 16-bit angle word (OBJ_3568 high, OBJ_35C0 low); once the high byte reaches $10 the object's OBJ_2800 goes up one.
 * HOW: A 16-bit add and a compare.
 * WAITS: no.
 * Entry: X object. Exit: the compare or the INC.
 */
Ended SlideAngleUp(uint8_t slot)
{
    uint8_t low, high, state;
    Sum sum;
    Ended e;

    low = RD8(OBJ->angle_lo[slot]);
    sum = add_bytes(low, 0xC0, false);
    WR8(OBJ->angle_lo[slot], sum.value);
    e = of_sum(sum);
    high = RD8(OBJ->angle_hi[slot]);
    sum = add_bytes(high, 0, sum.carry);
    WR8(OBJ->angle_hi[slot], sum.value);
    e = then(e, of_sum(sum));
    e = then(e, ended_compare(sum.value, 0x10));
    if (sum.value < 0x10) {
        return e;
    }
    state = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(state));
}

void abi_SlideAngleUp(void)
{
    abi_ended(SlideAngleUp(cpu.x));
}
