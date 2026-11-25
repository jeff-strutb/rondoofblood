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

/* RecordObjectEvent's exit: unmarked, A 0 and Z; else A the byte stored, Y its index, C whether the event is a multiple of 8. */
static Ended ended_of_object_event_stage(bool marked, uint8_t stored, uint8_t event, uint8_t byte)
{
    if (!marked) {
        return ended_load(0);
    }
    return ended_with_carry(ended_with_y(ended_load(stored), byte), (event & 7) == 0);
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

/* CPY: N, Z and C from the register less the operand; A kept. */
static Ended cpy(uint8_t reg, uint8_t operand)
{
    return ended_with_carry(ended_nz((uint8_t)(reg - operand)), reg >= operand);
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

/*
 * @routine 0CAB:422 AimAtPlayerThenArm
 * @run 70:B422
 * WHAT IT DOES: Takes OBJ_3510 as the angle to the player (a random one on three counts out of four, by OBJ_3670 and 3), sets the angle word from a table at $B44F by
 * OBJ_3670 and 3, and moves OBJ_3670 and OBJ_2800 on.
 * HOW: AngleToPlayer, NextRandom, table reads (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: N and Z of the new OBJ_2800; A the table's low byte, Y the table index, C 0.
 */
Ended AimAtPlayerThenArm(uint8_t slot)
{
    AngleExit angle;
    RandomResult random;
    uint8_t phase, index, byte, count, state;
    Ended e;

    jsr_push(0xB425);
    angle = AngleToPlayer(slot);
    jsr_pop();
    e = ended_of_angle(angle);
    WR8(OBJ->f3510[slot], angle.a);
    phase = RD8(OBJ->f3670[slot]);
    phase = (uint8_t)(phase & 0x03);
    e = then(e, ended_load(phase));
    if (phase != 0) {
        jsr_push(0xB432);
        random = NextRandom();
        jsr_pop();
        e = then(e, ended_of_random(random));
        WR8(OBJ->f3510[slot], random.value);
    }
    phase = RD8(OBJ->f3670[slot]);
    phase = (uint8_t)(phase & 0x03);
    index = (uint8_t)(phase << 1);
    e = then(e, ended_with_y(ended_with_carry(ended_load(index), false), index));
    byte = rd8((uint16_t)(0xB450 + index));
    WR8(OBJ->angle_hi[slot], byte);
    byte = rd8((uint16_t)(0xB44F + index));
    WR8(OBJ->angle_lo[slot], byte);
    e = then(e, ended_load(byte));
    count = INC8(OBJ->f3670[slot]);
    e = then(e, ended_nz(count));
    state = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(state));
}

void abi_AimAtPlayerThenArm(void)
{
    abi_ended(AimAtPlayerThenArm(cpu.x));
}

/*
 * @routine 0CAB:4AC InsideScreenBox
 * @run 70:B4AC
 * WHAT IT DOES: Says whether the object is inside the box X $20 to $DF, Y $20 to $BF on the first screen: C clear when inside, set when not.
 * HOW: The position's high bytes ORed, then four compares.
 * Untraced: the outside answer (B4CC).
 * Curated cases: tools/rondo/curated/inside_screen_box.txt (150 inside cases; the draft reaches the untraced outside answer on the seeded ones).
 * WAITS: no.
 * Entry: X object. Exit: C clear when inside; A and the flags as the last compare or load left them.
 */
ViewTest InsideScreenBox(uint8_t slot)
{
    ViewTest r = { false, { 0 } };
    uint8_t high, x_low, y_low;
    Ended e;

    high = RD8(OBJ->x_hi[slot]);
    high = (uint8_t)(high | RD8(OBJ->y_hi[slot]));
    e = ended_load(high);
    if (high == 0) {
        x_low = RD8(OBJ->x_lo[slot]);
        e = then(e, ended_compare(x_low, 0x20));
        if (x_low >= 0x20) {
            e = then(e, ended_compare(x_low, 0xE0));
            if (x_low < 0xE0) {
                y_low = RD8(OBJ->y_lo[slot]);
                e = then(e, ended_compare(y_low, 0x20));
                if (y_low >= 0x20) {
                    e = then(e, ended_compare(y_low, 0xC0));
                    if (y_low < 0xC0) {
                        r.in_view = true;
                        r.end = then(e, ended_with_carry(ended_kept(), false));
                        return r;
                    }
                }
            }
        }
    }
    r.end = then(e, ended_with_carry(ended_kept(), true));
    return r;
}

void abi_InsideScreenBox(void)
{
    abi_ended(InsideScreenBox(cpu.x).end);
}

/*
 * @routine 0CAB:4CE CountThenStop
 * @run 70:B4CE
 * WHAT IT DOES: Counts the object's timer down; at zero it clears OBJ_2800.
 * HOW: DEC and a store.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the count.
 */
Ended CountThenStop(uint8_t slot)
{
    uint8_t left;

    left = DEC8(OBJ->life_timer[slot]);
    if (left == 0) {
        WR8(OBJ->f2800[slot], 0);
    }
    return ended_nz(left);
}

void abi_CountThenStop(void)
{
    abi_ended(CountThenStop(cpu.x));
}

/*
 * @routine 0CAB:509 PickNextMoveBySide
 * @run 70:B509
 * WHAT IT DOES: Faces the player (bit 0 of SideOfPlayer), picks OBJ_2800 from a table at $B52C by the object's hit points halved (the high nibble) and a counter OBJ_3720 (the low
 * nibble), and counts the counter up.
 * HOW: SideOfPlayer, table read (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: N and Z of the new counter.
 */
Ended PickNextMoveBySide(uint8_t slot)
{
    SideResult side;
    uint8_t bits, hp, counter, index, byte, next;
    Sum sum;
    Ended e;

    jsr_push(0xB50C);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = ended_of_side(side);
    bits = (uint8_t)(side.sides & 0x01);
    WR8(OBJ->facing_left[slot], bits);
    e = then(e, ended_load(bits));
    hp = RD8(OBJ->f32a8[slot]);
    e = then(e, ended_with_carry(ended_load((uint8_t)(hp >> 1)), (hp & 0x01) != 0));
    hp = (uint8_t)((hp >> 1) & 0xF0);
    e = then(e, ended_load(hp));
    WR8(ZP->temp00, hp);
    counter = RD8(OBJ->f3720[slot]);
    counter = (uint8_t)(counter & 0x0F);
    e = then(e, ended_load(counter));
    index = RD8(ZP->temp00);
    sum = add_bytes(counter, index, false);
    e = then(e, of_sum(sum));
    byte = rd8((uint16_t)(0xB52C + sum.value));
    WR8(OBJ->f2800[slot], byte);
    e = then(e, ended_with_y(ended_load(byte), sum.value));
    next = INC8(OBJ->f3720[slot]);
    return then(e, ended_nz(next));
}

void abi_PickNextMoveBySide(void)
{
    abi_ended(PickNextMoveBySide(cpu.x));
}

/*
 * @routine 0CAB:55C StepThenSpawnShot
 * @run 70:B55C
 * WHAT IT DOES: Steps the animation list at $B582; when it ends, starts a kind $28 object at its place in slot $12 with a timer of 1 and its facing, and moves OBJ_2800 on.
 * HOW: StepObjectFrames, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: list running: as the step; ended: N and Z of the new OBJ_2800.
 */
Ended StepThenSpawnShot(uint8_t slot)
{
    FrameStep step;
    uint8_t y = 0, facing, state, placed;
    Ended e;

    step = step_list(slot, 0xB582, 0xB567, &y, false);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    jsr_push(0xB56E);
    ClearObjectInY(0x12);
    jsr_pop();
    jsr_push(0xB573);
    placed = StartObjectAtPlace(0x28, slot, 0x12);
    jsr_pop();
    e = then(e, ended_with_y(ended_load(placed), 0x12));
    WR8(OBJ->life_timer[0x12], 1);
    facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[0x12], facing);
    e = then(e, ended_load(facing));
    state = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(state));
}

void abi_StepThenSpawnShot(void)
{
    abi_ended(StepThenSpawnShot(cpu.x));
}

/*
 * @routine 0CAB:5A0 ResetToStateThree
 * @run 70:B5A0
 * WHAT IT DOES: Sets the object's frame to 1, its state to 3 and clears OBJ_2800.
 * HOW: Three stores.
 * WAITS: no.
 * Entry: X object. Exit: A 3, N and Z from it.
 */
Ended ResetToStateThree(uint8_t slot)
{
    WR8(OBJ->frame[slot], 1);
    WR8(OBJ->state[slot], 3);
    WR8(OBJ->f2800[slot], 0);
    return ended_load(3);
}

void abi_ResetToStateThree(void)
{
    abi_ended(ResetToStateThree(cpu.x));
}

/*
 * @routine 0CAB:5AE StartThreeShots
 * @run 70:B5AE
 * WHAT IT DOES: Starts three kind $26 objects at this object's place in slots $13 to $15, each with this object's facing and its number 0 to 2 as its timer.
 * HOW: A loop of ClearObjectInY and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: A the last number, Y $16, N, Z and C from the compare of Y with $16.
 */
Ended StartThreeShots(uint8_t slot)
{
    uint8_t to, number, facing, placed;
    Ended e;

    WR8(ZP->temp00, 0);
    e = ended_kept();
    for (to = 0x13; to < 0x16; to++) {
        jsr_push(0xB5B5);
        ClearObjectInY(to);
        jsr_pop();
        jsr_push(0xB5BA);
        placed = StartObjectAtPlace(0x26, slot, to);
        jsr_pop();
        facing = RD8(OBJ->facing_left[slot]);
        WR8(OBJ->facing_left[to], facing);
        number = RD8(ZP->temp00);
        WR8(OBJ->life_timer[to], number);
        INC8(ZP->temp00);
        e = ended_with_y(ended_with_a(ended_load(number), number), to);
        e = then(e, ended_with_y(cpy((uint8_t)(to + 1), 0x16), (uint8_t)(to + 1)));
    }
    (void)placed;
    return e;
}

void abi_StartThreeShots(void)
{
    abi_ended(StartThreeShots(cpu.x));
}

/* The step all the shot throwers share when their list ends: a kind $28 object in slot $12 at this object's place. */
static Ended spawn_shot(uint8_t slot, uint16_t ret_clear, uint16_t ret_start, int timer, bool timer_first)
{
    uint8_t placed, facing, state;
    Ended e;

    jsr_push(ret_clear);
    ClearObjectInY(0x12);
    jsr_pop();
    jsr_push(ret_start);
    placed = StartObjectAtPlace(0x28, slot, 0x12);
    jsr_pop();
    e = ended_with_y(ended_load(placed), 0x12);
    if (timer_first && timer >= 0) {
        WR8(OBJ->life_timer[0x12], (uint8_t)timer);
        e = then(e, ended_load((uint8_t)timer));
    }
    facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[0x12], facing);
    e = then(e, ended_load(facing));
    state = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(state));
}

/*
 * @routine 0CAB:666 StepThenThrowShot
 * @run 70:B666
 * WHAT IT DOES: Steps the animation list at $B582; when it ends, starts a kind $28 object at its place in slot $12 with its facing and moves OBJ_2800 on.
 * HOW: StepObjectFrames, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: list running: as the step; ended: N and Z of the new OBJ_2800.
 */
Ended StepThenThrowShot(uint8_t slot)
{
    FrameStep step;
    uint8_t y = 0;
    Ended e;

    step = step_list(slot, 0xB582, 0xB671, &y, false);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    return then(e, spawn_shot(slot, 0xB678, 0xB67D, -1, false));
}

void abi_StepThenThrowShot(void)
{
    abi_ended(StepThenThrowShot(cpu.x));
}

/*
 * @routine 0CAB:6F3 StepThenThrowSlowShot
 * @run 70:B6F3
 * WHAT IT DOES: As StepThenThrowShot, but the shot's timer is 2.
 * HOW: StepObjectFrames, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry and exit: as StepThenThrowShot.
 */
Ended StepThenThrowSlowShot(uint8_t slot)
{
    FrameStep step;
    uint8_t y = 0;
    Ended e;

    step = step_list(slot, 0xB582, 0xB6FE, &y, false);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    return then(e, spawn_shot(slot, 0xB705, 0xB70A, 2, true));
}

void abi_StepThenThrowSlowShot(void)
{
    abi_ended(StepThenThrowSlowShot(cpu.x));
}

/* The step several objects share when their list ends: frame 1, state 3, OBJ_2800 cleared. */
static Ended step_then_rest(uint8_t slot, uint16_t list, uint16_t ret, bool frame_one)
{
    FrameStep step;
    uint8_t y = 0;
    Ended e;

    step = step_list(slot, list, ret, &y, false);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->frame[slot], frame_one ? 1 : 0);
    WR8(OBJ->state[slot], 3);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_load(3));
}

/*
 * @routine 0CAB:640 StepThenRest
 * @run 70:B640
 * WHAT IT DOES: Steps the animation list at $B65B; when it ends the object shows frame 1, enters state 3 and clears OBJ_2800.
 * HOW: StepObjectFrames and three stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: A 3.
 */
Ended StepThenRest(uint8_t slot)
{
    return step_then_rest(slot, 0xB65B, 0xB64B, true);
}

void abi_StepThenRest(void)
{
    abi_ended(StepThenRest(cpu.x));
}

/*
 * @routine 0CAB:7A9 StepThenRestB
 * @run 70:B7A9
 * WHAT IT DOES: As StepThenRest with the list at $B7C4.
 * HOW: As StepThenRest.
 * WAITS: no.
 * Entry and exit: as StepThenRest.
 */
Ended StepThenRestB(uint8_t slot)
{
    return step_then_rest(slot, 0xB7C4, 0xB7B4, true);
}

void abi_StepThenRestB(void)
{
    abi_ended(StepThenRestB(cpu.x));
}

/*
 * @routine 0CAB:5CD StepThenWaitLonger
 * @run 70:B5CD
 * WHAT IT DOES: Steps the animation list at $B5E6; when it ends the object's frame is 0, its timer $30 and OBJ_2800 goes on.
 * HOW: StepObjectFrames and stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: N and Z of the new OBJ_2800.
 */
Ended StepThenWaitLonger(uint8_t slot)
{
    FrameStep step;
    uint8_t y = 0, state;
    Ended e;

    step = step_list(slot, 0xB5E6, 0xB5D8, &y, false);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->frame[slot], 0);
    WR8(OBJ->life_timer[slot], 0x30);
    state = INC8(OBJ->f2800[slot]);
    return then(e, ended_with_a(ended_nz(state), 0x30));
}

void abi_StepThenWaitLonger(void)
{
    abi_ended(StepThenWaitLonger(cpu.x));
}

/*
 * @routine 0CAB:62B StartTwoPieces
 * @run 70:B62B
 * WHAT IT DOES: Starts up to two kind $29 objects at this object's place in free slots from $20 (each only if a slot is free).
 * HOW: FindFreeSlotFrom20 and StartObjectAtPlace, twice.
 * WAITS: no.
 * Entry: X object. Exit: as the last search or start.
 */
Ended StartTwoPieces(uint8_t slot)
{
    uint8_t last, found_slot, placed, round;
    bool found;
    Ended e = ended_kept();

    for (round = 0; round < 2; round++) {
        jsr_push(round == 0 ? 0xB62E : 0xB638);
        found_slot = FindFreeSlotFrom20(&last, &found);
        jsr_pop();
        e = ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), last), found_slot), found);
        if (!found) {
            return e;
        }
        jsr_push(round == 0 ? 0xB635 : 0xB63F);
        placed = StartObjectAtPlace(0x29, slot, found_slot);
        jsr_pop();
        e = then(e, ended_load(placed));
    }
    return e;
}

void abi_StartTwoPieces(void)
{
    abi_ended(StartTwoPieces(cpu.x));
}

/*
 * @routine 0CAB:69A SinkToLimit
 * @run 70:B69A
 * WHAT IT DOES: Takes 3 off the object's Y low byte while it is $30 or more.
 * HOW: A compare and a subtract.
 * WAITS: no.
 * Entry: X object. Exit: under $30: the compare's flags; else the subtract's.
 */
Ended SinkToLimit(uint8_t slot)
{
    uint8_t low;
    Sum sum;
    Ended e;

    low = RD8(OBJ->y_lo[slot]);
    e = ended_compare(low, 0x30);
    if (low < 0x30) {
        return e;
    }
    low = RD8(OBJ->y_lo[slot]);
    sum = sub_bytes(low, 3, true);
    WR8(OBJ->y_lo[slot], sum.value);
    return then(e, of_sum(sum));
}

void abi_SinkToLimit(void)
{
    abi_ended(SinkToLimit(cpu.x));
}

/*
 * @routine 0CAB:687 SinkWhenPartnerGone
 * @run 70:B687
 * WHAT IT DOES: When slot $12's kind is not $28 the object sinks (SinkToLimit), its timer becomes $30 and OBJ_2800 moves on.
 * HOW: A compare, SinkToLimit, two stores.
 * WAITS: no.
 * Entry: X object. Exit: the compare's flags, or N and Z of the new OBJ_2800.
 */
Ended SinkWhenPartnerGone(uint8_t slot)
{
    uint8_t kind, state;
    Ended e;

    kind = RD8(OBJ->kind[0x12]);
    e = ended_compare(kind, 0x28);
    if (kind == 0x28) {
        return e;
    }
    jsr_push(0xB691);
    e = then(e, SinkToLimit(slot));
    jsr_pop();
    WR8(OBJ->life_timer[slot], 0x30);
    e = then(e, ended_load(0x30));
    state = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(state));
}

void abi_SinkWhenPartnerGone(void)
{
    abi_ended(SinkWhenPartnerGone(cpu.x));
}

/*
 * @routine 0CAB:719 SetLongWait
 * @run 70:B719
 * WHAT IT DOES: Sets the object's timer to $40 and moves OBJ_2800 on.
 * HOW: A store and an INC.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the new OBJ_2800, A $40.
 */
Ended SetLongWait(uint8_t slot)
{
    uint8_t state;

    WR8(OBJ->life_timer[slot], 0x40);
    state = INC8(OBJ->f2800[slot]);
    return ended_with_a(ended_nz(state), 0x40);
}

void abi_SetLongWait(void)
{
    abi_ended(SetLongWait(cpu.x));
}

/*
 * @routine 0CAB:738 DropShotWhenSlotFree
 * @run 70:B738
 * WHAT IT DOES: Every 16th count of OBJ_3618, searches slots $30 to $35 for a free one and starts a kind $2A object there at its place.
 * HOW: A search by kind, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: the last compare or load.
 */
Ended DropShotWhenSlotFree(uint8_t slot)
{
    uint8_t phase, to, kind, placed;
    Ended e;

    phase = RD8(OBJ->f3618[slot]);
    phase = (uint8_t)(phase & 0x0F);
    e = ended_load(phase);
    if (phase != 0) {
        return e;
    }
    to = 0x30;
    e = then(e, ended_with_y(ended_nz(to), to));
    for (;;) {
        kind = RD8(OBJ->kind[to]);
        e = then(e, ended_load(kind));
        if (kind == 0) {
            break;
        }
        to++;
        e = then(e, ended_with_y(ended_nz(to), to));
        e = then(e, cpy(to, 0x36));
        if (to >= 0x36) {
            return e;
        }
    }
    jsr_push(0xB74F);
    ClearObjectInY(to);
    jsr_pop();
    jsr_push(0xB754);
    placed = StartObjectAtPlace(0x2A, slot, to);
    jsr_pop();
    return then(e, ended_load(placed));
}

void abi_DropShotWhenSlotFree(void)
{
    abi_ended(DropShotWhenSlotFree(cpu.x));
}

/*
 * @routine 0CAB:722 SinkThenRestart
 * @run 70:B722
 * WHAT IT DOES: Drops a shot when a slot is free (DropShotWhenSlotFree), counts its timer down and at zero shows frame 1, enters state 3 and clears OBJ_2800.
 * HOW: DropShotWhenSlotFree, DEC, three stores.
 * WAITS: no.
 * Entry: X object. Exit: the count's N and Z, or A 3 at zero.
 */
Ended SinkThenRestart(uint8_t slot)
{
    uint8_t left;
    Ended e;

    jsr_push(0xB725);
    e = DropShotWhenSlotFree(slot);
    jsr_pop();
    left = DEC8(OBJ->life_timer[slot]);
    e = then(e, ended_nz(left));
    if (left != 0) {
        return e;
    }
    WR8(OBJ->frame[slot], 1);
    WR8(OBJ->state[slot], 3);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_load(3));
}

void abi_SinkThenRestart(void)
{
    abi_ended(SinkThenRestart(cpu.x));
}

/*
 * @routine 0CAB:6C4 ShootEveryFourth
 * @run 70:B6C4
 * WHAT IT DOES: Every fourth count of OBJ_3618, while Y is under $A0, starts a kind $27 object at its place in the first free slot from $20 (sound $26 first), with the
 * object's facing and its OBJ_3778 count (low three bits) as its next kind; the count goes up.
 * HOW: FindFreeSlotFrom20, SoundCommand, StartObjectAtPlace.
 * Untraced: two bytes.
 * WAITS: no.
 * Entry: X object, Y pushed by the sound driver. Exit: see the wrapper.
 */
SoundEnd ShootEveryFourth(uint8_t slot, uint8_t y)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    uint8_t phase, y_low, last, found_slot, placed, facing, count, state;
    bool found;
    Ended e;

    phase = RD8(OBJ->f3618[slot]);
    phase = (uint8_t)(phase & 0x03);
    e = ended_load(phase);
    r.end = e;
    if (phase != 0) {
        return r;
    }
    y_low = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(y_low, 0xA0));
    r.end = e;
    if (y_low >= 0xA0) {
        return r;
    }
    jsr_push(0xB6D5);
    found_slot = FindFreeSlotFrom20(&last, &found);
    jsr_pop();
    e = then(e, ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), last), found_slot), found));
    r.end = e;
    if (!found) {
        return r;
    }
    r.before = then(e, ended_load(0x26));
    r.sounded = true;
    r.command = 0x26;
    jsr_push(0xB6DC);
    r.call = SoundCommand(0x26, slot, found_slot);
    jsr_pop();
    jsr_push(0xB6E1);
    placed = StartObjectAtPlace(0x27, slot, found_slot);
    jsr_pop();
    e = ended_load(placed);
    facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[found_slot], facing);
    e = then(e, ended_load(facing));
    count = RD8(OBJ->f3778[slot]);
    count = (uint8_t)(count & 0x07);
    WR8(OBJ->next_kind[found_slot], count);
    e = then(e, ended_load(count));
    state = INC8(OBJ->f3778[slot]);
    r.end = then(e, ended_nz(state));
    return r;
}

void abi_ShootEveryFourth(void)
{
    abi_sound_end(ShootEveryFourth(cpu.x, cpu.y));
}

/*
 * @routine 0CAB:6AB SinkAndShootThenRestart
 * @run 70:B6AB
 * WHAT IT DOES: ShootEveryFourth and SinkToLimit; the timer counts down and at zero the object shows frame 1, enters state 3 and clears OBJ_2800.
 * HOW: The two calls, a DEC and three stores.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: see the wrapper.
 */
ShootTail SinkAndShootThenRestart(uint8_t slot, uint8_t y)
{
    ShootTail r;
    uint8_t left;
    Ended e;

    jsr_push(0xB6AE);
    r.shoot = ShootEveryFourth(slot, y);
    jsr_pop();
    jsr_push(0xB6B1);
    e = SinkToLimit(slot);
    jsr_pop();
    left = DEC8(OBJ->life_timer[slot]);
    e = then(e, ended_nz(left));
    if (left == 0) {
        WR8(OBJ->frame[slot], 1);
        WR8(OBJ->state[slot], 3);
        WR8(OBJ->f2800[slot], 0);
        e = then(e, ended_load(3));
    }
    r.tail = e;
    return r;
}

void abi_SinkAndShootThenRestart(void)
{
    ShootTail r = SinkAndShootThenRestart(cpu.x, cpu.y);

    abi_sound_end(r.shoot);
    abi_ended(r.tail);
}

/*
 * @routine 0CAB:5F1 FlickerAndStartPieces
 * @run 70:B5F1
 * WHAT IT DOES: Counts the object's timer down; while it runs it starts up to two pieces (StartTwoPieces) and, when the timer is $18, moves the object to a random
 * place (X and Y low bytes from a table at $B623, high bytes 0); at zero OBJ_2800 moves on.
 * HOW: DEC, StartTwoPieces, NextRandom twice, table reads (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: the last compare, load or INC.
 */
Ended FlickerAndStartPieces(uint8_t slot)
{
    RandomResult random;
    uint8_t left, timer, pick, byte, state;
    Ended e;

    left = DEC8(OBJ->life_timer[slot]);
    e = ended_nz(left);
    if (left == 0) {
        state = INC8(OBJ->f2800[slot]);
        return then(e, ended_nz(state));
    }
    jsr_push(0xB5F9);
    e = then(e, StartTwoPieces(slot));
    jsr_pop();
    timer = RD8(OBJ->life_timer[slot]);
    e = then(e, ended_compare(timer, 0x18));
    if (timer != 0x18) {
        return e;
    }
    WR8(OBJ->x_hi[slot], 0);
    jsr_push(0xB606);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = (uint8_t)(random.value & 0x07);
    e = then(e, ended_with_y(ended_load(pick), pick));
    byte = rd8((uint16_t)(0xB623 + pick));
    WR8(OBJ->x_lo[slot], byte);
    e = then(e, ended_load(byte));
    WR8(OBJ->y_hi[slot], 0);
    jsr_push(0xB615);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = (uint8_t)(random.value & 0x07);
    e = then(e, ended_with_y(ended_load(pick), pick));
    byte = rd8((uint16_t)(0xB623 + pick));
    WR8(OBJ->y_lo[slot], byte);
    return then(e, ended_load(byte));
}

void abi_FlickerAndStartPieces(void)
{
    abi_ended(FlickerAndStartPieces(cpu.x));
}

/*
 * @routine 0CAD:12A StepPartList
 * @run 71:C12A
 * WHAT IT DOES: Steps the animation list at $C196.
 * HOW: The list's address goes to zTemp10-11; a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as StepObjectFrames.
 */
Ended StepPartList(uint8_t slot)
{
    FrameStep step;
    uint8_t y = 0;

    WR8(ZP->temp10, 0x96);
    WR8(ZP->temp11, 0xC1);
    step = StepObjectFrames(slot, &y);
    return ended_of_frame_step(step, step != FRAME_HELD, y);
}

void abi_StepPartList(void)
{
    abi_ended(StepPartList(cpu.x));
}

/*
 * @routine 0CAD:15F DoNothing
 * @run 71:C15F
 * WHAT IT DOES: Returns at once (a state handler that has nothing to do).
 * HOW: RTS.
 * WAITS: no.
 * Entry: nothing. Exit: every register and flag kept.
 */
void DoNothing(void)
{
}

void abi_DoNothing(void)
{
    DoNothing();
}

/*
 * @routine 0CAD:165 WatchVramForMark
 * @run 71:C165
 * WHAT IT DOES: In state 0 loads the object's record and sets its sprite set to 6 and its frame to 7. Each frame it reads the VDC's video RAM at $05FB and, unless the
 * word is a $43 byte, records the object's event and clears the object.
 * HOW: LoadObjectRecord, SelectVramReadAddress, two data writes, SelectVramData, two reads of the VDC's data port, RecordObjectEvent, ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: see the wrapper.
 */
VramWatch WatchVramForMark(uint8_t slot)
{
    VramWatch r = { { 0 }, false, 0, 0, { 0 }, false, 0, 0 };
    uint8_t state, nz, last, low, high, stored, event, byte;
    bool carry, marked;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(0xC16D);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        WR8(OBJ->sprite_set[slot], 6);
        WR8(OBJ->frame[slot], 7);
        r.start = then(ended_of_record(last, nz, carry), ended_load(7));
    }
    jsr_push(0xC17A);
    SelectVramReadAddress();
    jsr_pop();
    st1(0xFB);
    st2(0x05);
    jsr_push(0xC181);
    SelectVramData();
    jsr_pop();
    low = RD8(VDC->data_lo);
    high = RD8(VDC->data_hi);
    r.low = low;
    r.high = high;
    r.read = then(ended_load(low), ended_with_y(ended_kept(), high));
    r.read = then(r.read, ended_compare(low, 0x43));
    if (low == 0x43) {
        return r;
    }
    jsr_push(0xC18E);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    jsr_pop();
    r.event = ended_of_object_event_stage(marked, stored, event, byte);
    ClearObject(slot);
    r.cleared = true;
    return r;
}

void abi_WatchVramForMark(void)
{
    VramWatch r = WatchVramForMark(cpu.x);

    abi_ended(r.start);
    abi_ended(r.read);
    if (r.cleared) {
        abi_ended(r.event);
    }
}

/*
 * @routine 0CAC:45C StepMoveOrClear
 * @run 70:BC5C
 * WHAT IT DOES: Steps the animation list at $BC6F; while it runs the object moves by its velocity, and when it ends the object is cleared.
 * HOW: StepObjectFrames, then a tail jump into ApplyVelocity or ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: running: as ApplyVelocity; ended: as the step.
 */
Ended StepMoveOrClear(uint8_t slot)
{
    FrameStep step;
    AxisMove move;
    uint8_t y = 0;
    Ended e;

    step = step_list(slot, 0xBC6F, 0xBC67, &y, true);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step == FRAME_LIST_ENDED) {
        ClearObject(slot);
        return e;
    }
    move = ApplyVelocity(slot);
    return then(e, ended_of_axis_move(move));
}

void abi_StepMoveOrClear(void)
{
    abi_ended(StepMoveOrClear(cpu.x));
}

/*
 * @routine 0CAC:57C StepCountClear
 * @run 70:BD7C
 * WHAT IT DOES: Steps the animation list at $BD51; counts the object's timer down and clears it at zero; until then its hitbox half height comes from a table at $BD78 by the
 * frame less $17.
 * HOW: StepObjectFrames, DEC, a table read (bank 70).
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: cleared: N and Z of the count; else A the table byte, Y the index, C and V from the subtract.
 */
Ended StepCountClear(uint8_t slot)
{
    FrameStep step;
    uint8_t y = 0, left, frame, byte;
    Sum index;
    Ended e;

    step = step_list(slot, 0xBD51, 0xBD87, &y, false);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    left = DEC8(OBJ->life_timer[slot]);
    e = then(e, ended_nz(left));
    if (left == 0) {
        ClearObject(slot);
        return e;
    }
    frame = RD8(OBJ->frame[slot]);
    index = sub_bytes(frame, 0x17, true);
    byte = rd8((uint16_t)(0xBD78 + index.value));
    WR8(OBJ->hitbox_half_h[slot], byte);
    return then(then(e, of_sum(index)), ended_with_y(ended_load(byte), index.value));
}

void abi_StepCountClear(void)
{
    abi_ended(StepCountClear(cpu.x));
}

/*
 * @routine 0CAC:5B5 StartFallingSpark
 * @run 70:BDB5
 * WHAT IT DOES: Loads the object's record, gives OBJ_3510 a random byte, its angle low byte one of four from a table at $BDE1, a random X low byte, X high 0, Y $0100 and a
 * timer of $C0.
 * HOW: LoadObjectRecord, NextRandom three times, a table read (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: A $C0, N set, Z clear; Y the table index, C and V as the last random left them.
 */
Ended StartFallingSpark(uint8_t slot)
{
    RandomResult random;
    uint8_t nz, last, pick, byte;
    bool carry;
    Ended e;

    jsr_push(0xBDB8);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    jsr_push(0xBDBB);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    WR8(OBJ->f3510[slot], random.value);
    jsr_push(0xBDC1);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = (uint8_t)(random.value & 0x03);
    e = then(e, ended_with_y(ended_load(pick), pick));
    byte = rd8((uint16_t)(0xBDE1 + pick));
    WR8(OBJ->angle_lo[slot], byte);
    WR8(OBJ->x_hi[slot], 0);
    jsr_push(0xBDD0);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    WR8(OBJ->x_lo[slot], random.value);
    WR8(OBJ->y_hi[slot], 1);
    WR8(OBJ->y_lo[slot], 0);
    WR8(OBJ->life_timer[slot], 0xC0);
    return then(e, ended_load(0xC0));
}

void abi_StartFallingSpark(void)
{
    abi_ended(StartFallingSpark(cpu.x));
}

/*
 * @routine 0CAC:6DD StartFromTable
 * @run 70:BEDD
 * WHAT IT DOES: Loads the object's record, sets its angle low byte to $80 and its angle high byte from a table at $BEF2 by its next kind, less $A0.
 * HOW: LoadObjectRecord and a subtract.
 * WAITS: no.
 * Entry: X object. Exit: A the angle high byte, Y the next kind, C and V from the subtract.
 */
Ended StartFromTable(uint8_t slot)
{
    uint8_t nz, last, kind, byte;
    bool carry;
    Sum diff;
    Ended e;

    jsr_push(0xBEE0);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->angle_lo[slot], 0x80);
    kind = RD8(OBJ->next_kind[slot]);
    e = then(e, ended_with_y(ended_nz(kind), kind));
    byte = rd8((uint16_t)(0xBEF2 + kind));
    diff = sub_bytes(byte, 0xA0, true);
    WR8(OBJ->angle_hi[slot], diff.value);
    return then(e, of_sum(diff));
}

void abi_StartFromTable(void)
{
    abi_ended(StartFromTable(cpu.x));
}

/*
 * @routine 0CAB:591 StartShotsUnlessThere
 * @run 70:B591
 * WHAT IT DOES: Unless slot $12's kind is $28, starts the three shots (StartThreeShots) and plays sound $2E; then goes on as ResetToStateThree.
 * HOW: A compare, StartThreeShots, SoundCommand, the next routine's bytes.
 * WAITS: no.
 * Entry: X object. Exit: as ResetToStateThree, or the compare's flags when the shots are already there.
 */
SoundEnd StartShotsUnlessThere(uint8_t slot)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    uint8_t kind;
    Ended e;

    kind = RD8(OBJ->kind[0x12]);
    e = ended_compare(kind, 0x28);
    r.end = e;
    if (kind == 0x28) {
        return r;
    }
    jsr_push(0xB59B);
    e = then(e, StartThreeShots(slot));
    jsr_pop();
    r.before = then(e, ended_load(0x2E));
    r.sounded = true;
    r.command = 0x2E;
    jsr_push(0xB5A0);
    r.call = SoundCommand(0x2E, slot, 0x16);
    jsr_pop();
    r.end = ResetToStateThree(slot);
    return r;
}

void abi_StartShotsUnlessThere(void)
{
    abi_sound_end(StartShotsUnlessThere(cpu.x));
}

/*
 * @routine 0CAC:282 FollowAndFacePlayer
 * @run 70:BA82
 * WHAT IT DOES: In state 0 loads the object's record; later it sets its frame to 0 and, when a table at $BAAF by slot $10's frame says so, copies slot $10's flash flag, takes
 * its place and shows the frame $29 plus the angle to the player in thirty-seconds.
 * HOW: LoadObjectRecord (a tail jump), CopyObjectPlaceBack, AngleToPlayer, table read (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: state 0: as LoadObjectRecord; else the last load or add.
 */
Ended FollowAndFacePlayer(uint8_t slot)
{
    AngleExit angle;
    uint8_t state, nz, last, lead, flag, flash, placed, frame;
    bool carry;
    Sum sum;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        return ended_of_record(last, nz, carry);
    }
    WR8(OBJ->frame[slot], 0);
    lead = RD8(OBJ->frame[0x10]);
    flag = rd8((uint16_t)(0xBAAF + lead));
    e = ended_with_y(ended_load(flag), lead);
    if (flag == 0) {
        return e;
    }
    flash = RD8(OBJ->flash_flags[0x10]);
    WR8(OBJ->flash_flags[slot], flash);
    e = then(e, ended_load(flash));
    jsr_push(0xBAA0);
    placed = CopyObjectPlaceBack(0x10, slot);
    jsr_pop();
    e = then(e, ended_with_y(ended_load(placed), 0x10));
    jsr_push(0xBAA3);
    angle = AngleToPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_angle(angle));
    frame = (uint8_t)(angle.a >> 5);
    e = then(e, ended_with_carry(ended_load(frame), (angle.a & 0x10) != 0));
    sum = add_bytes(frame, 0x29, false);
    WR8(OBJ->frame[slot], sum.value);
    return then(e, of_sum(sum));
}

void abi_FollowAndFacePlayer(void)
{
    abi_ended(FollowAndFacePlayer(cpu.x));
}

/*
 * @routine 0CAC:315 StepOrClearByKind
 * @run 70:BB15
 * WHAT IT DOES: Steps the animation list that a table at $BB2D names for the object's timer-index (OBJ_34B8 doubled); when it ends the object is cleared.
 * HOW: A table read (bank 70), StepObjectFrames, a tail jump into ClearObject. (The part of the routine at $BAEF that comes after its table.)
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step.
 */
Ended StepOrClearByKind(uint8_t slot)
{
    FrameStep step;
    uint8_t phase, index, y;

    phase = RD8(OBJ->life_timer[slot]);
    index = (uint8_t)(phase << 1);
    y = index;
    WR8(ZP->temp10, rd8((uint16_t)(0xBB2D + index)));
    WR8(ZP->temp11, rd8((uint16_t)(0xBB2E + index)));
    jsr_push(0xBB27);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    if (step == FRAME_LIST_ENDED) {
        ClearObject(slot);
    }
    return ended_of_frame_step(step, true, y);
}

void abi_StepOrClearByKind(void)
{
    abi_ended(StepOrClearByKind(cpu.x));
}

/*
 * @routine 0CAC:2EF StartSoundedPiece
 * @run 70:BAEF
 * @stop BB10
 * WHAT IT DOES: In state 0 loads the object's record, plays the sound a table at $BB12 names for its timer-index, takes an X speed from a table at $BB10 by its facing, moves
 * along X once and faces right; in any other state it goes on as StepOrClearByKind (the code after the table).
 * HOW: LoadObjectRecord, SoundCommand, ApplyVelocityX.
 * Untraced: the tables at $BB10 are data the run only read.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: see the wrapper.
 */
SoundEnd StartSoundedPiece(uint8_t slot, uint8_t y)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    AxisMove move;
    uint8_t state, nz, last, phase, command, facing, byte;
    bool carry;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        r.end = StepOrClearByKind(slot);
        return r;
    }
    jsr_push(0xBAF7);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    phase = RD8(OBJ->life_timer[slot]);
    command = rd8((uint16_t)(0xBB12 + phase));
    r.before = then(e, ended_with_y(ended_load(command), phase));
    r.sounded = true;
    r.command = command;
    jsr_push(0xBB00);
    r.call = SoundCommand(command, slot, phase);
    jsr_pop();
    facing = RD8(OBJ->facing_left[slot]);
    byte = rd8((uint16_t)(0xBB10 + facing));
    WR8(OBJ->vx_hi[slot], byte);
    jsr_push(0xBB0C);
    move = ApplyVelocityX(slot);
    jsr_pop();
    WR8(OBJ->facing_left[slot], 0);
    r.end = then(ended_with_y(ended_kept(), facing), ended_of_axis_move(move));
    return r;
}

void abi_StartSoundedPiece(void)
{
    abi_sound_end(StartSoundedPiece(cpu.x, cpu.y));
}

/*
 * @routine 0CAC:492 StartThrownPiece
 * @run 70:BC92
 * WHAT IT DOES: Loads the object's record, turns on bit 7 of its hitbox flag, gives it an X speed from a table at $BCD1 by its facing and a Y speed of $18, moves it, then picks
 * a random length from a table at $BCD3 and sets its speed from the angle a table at $BCE3 names for its next kind (SetVelocityFromAngle); a left-facing object is reversed.
 * HOW: LoadObjectRecord, ApplyVelocity, NextRandom, SetVelocityFromAngle.
 * Untraced: the reversal (BCCD).
 * Curated cases: tools/rondo/curated/start_thrown_piece.txt (right-facing objects, the traced arm).
 * WAITS: no.
 * Entry: X object. Exit: the last load; C, V and Y as the angle scale left them.
 */
Ended StartThrownPiece(uint8_t slot)
{
    RandomResult random;
    ScaleResult scale;
    AxisMove move;
    uint8_t nz, last, flags, facing, byte, pick, index, kind, angle, left;
    bool carry;
    Ended e;

    jsr_push(0xBC95);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    flags = RD8(OBJ->hitbox_on[slot]);
    flags = (uint8_t)(flags | 0x80);
    WR8(OBJ->hitbox_on[slot], flags);
    e = then(e, ended_load(flags));
    facing = RD8(OBJ->facing_left[slot]);
    byte = rd8((uint16_t)(0xBCD1 + facing));
    WR8(OBJ->vx_hi[slot], byte);
    WR8(OBJ->vy_hi[slot], 0x18);
    jsr_push(0xBCAE);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    jsr_push(0xBCB1);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = (uint8_t)(random.value & 0x07);
    index = (uint8_t)(pick << 1);
    e = then(e, ended_with_y(ended_with_carry(ended_load(index), false), index));
    byte = rd8((uint16_t)(0xBCD3 + index));
    WR8(ZP->temp00, byte);
    byte = rd8((uint16_t)(0xBCD4 + index));
    WR8(ZP->temp01, byte);
    kind = RD8(OBJ->next_kind[slot]);
    angle = rd8((uint16_t)(0xBCE3 + kind));
    jsr_push(0xBCC8);
    scale = SetVelocityFromAngle(angle, slot);
    jsr_pop();
    e = then(e, ended_of_scale(scale));
    left = RD8(OBJ->facing_left[slot]);
    e = then(e, ended_load(left));
    if (left != 0) {
        jsr_push(0xBCCD);
        e = then(e, of_sum(NegateVelocityX(slot)));
        jsr_pop();
    }
    return e;
}

void abi_StartThrownPiece(void)
{
    abi_ended(StartThrownPiece(cpu.x));
}

/* A random pick of a two-byte pair: the table's high byte at +1 and its low byte at +0 of the doubled index. */
static Ended pick_speed_pair(uint16_t table, uint16_t ret, uint16_t high_column, uint16_t low_column, uint8_t slot)
{
    RandomResult random;
    uint8_t pick, index, byte;
    Ended e;

    jsr_push(ret);
    random = NextRandom();
    jsr_pop();
    e = ended_of_random(random);
    pick = (uint8_t)(random.value & 0x07);
    index = (uint8_t)(pick << 1);
    e = then(e, ended_with_y(ended_with_carry(ended_load(index), false), index));
    byte = rd8((uint16_t)(table + 1 + index));
    WR8_IDX(high_column, slot, byte);
    byte = rd8((uint16_t)(table + index));
    WR8_IDX(low_column, slot, byte);
    return then(e, ended_load(byte));
}

/*
 * @routine 0CAC:3C1 StartScatteringPiece
 * @run 70:BBC1
 * WHAT IT DOES: In any state but 0 it goes on as StepMoveOrClear. In state 0 it loads its record, takes a random X and Y speed from a table at $BC1C (sixteen bytes: eight
 * words), moves once, then takes a second random X speed from $BC2C and Y speed from $BC3C.
 * HOW: LoadObjectRecord, NextRandom four times, ApplyVelocity, table reads (bank 70).
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: the last random's pick and table read.
 */
Ended StartScatteringPiece(uint8_t slot)
{
    AxisMove move;
    uint8_t state, nz, last;
    bool carry;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        return StepMoveOrClear(slot);
    }
    jsr_push(0xBBCC);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    e = then(e, pick_speed_pair(0xBC1C, 0xBBCF, COL(OBJ->vx_hi), COL(OBJ->vx_lo), slot));
    e = then(e, pick_speed_pair(0xBC1C, 0xBBE2, COL(OBJ->vy_hi), COL(OBJ->vy_lo), slot));
    jsr_push(0xBBF5);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    e = then(e, pick_speed_pair(0xBC2C, 0xBBF8, COL(OBJ->vx_hi), COL(OBJ->vx_lo), slot));
    return then(e, pick_speed_pair(0xBC3C, 0xBC0B, COL(OBJ->vy_hi), COL(OBJ->vy_lo), slot));
}

void abi_StartScatteringPiece(void)
{
    abi_ended(StartScatteringPiece(cpu.x));
}

/*
 * @routine 0CAC:399 StepListAndMove
 * @run 70:BB99
 * WHAT IT DOES: Steps the animation list that a table at $BBAC names for the object's timer-index (doubled) and moves the object (the part of the routine at $BB4E after its table).
 * HOW: A table read (bank 70), StepObjectFrames, a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as ApplyVelocity, Y from the step.
 */
Ended StepListAndMove(uint8_t slot)
{
    FrameStep step;
    AxisMove move;
    uint8_t phase, index, y;
    Ended e;

    phase = RD8(OBJ->life_timer[slot]);
    index = (uint8_t)(phase << 1);
    y = index;
    WR8(ZP->temp10, rd8((uint16_t)(0xBBAC + index)));
    WR8(ZP->temp11, rd8((uint16_t)(0xBBAD + index)));
    jsr_push(0xBBA9);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, true, y);
    move = ApplyVelocity(slot);
    return then(e, ended_of_axis_move(move));
}

void abi_StepListAndMove(void)
{
    abi_ended(StepListAndMove(cpu.x));
}

/*
 * @routine 0CAC:34E StartSideDrifter
 * @run 70:BB4E
 * @stop BB89
 * WHAT IT DOES: In state 0 loads the object's record, takes an X speed from a table at $BB89 by its facing, moves along X once, takes a four-byte speed row from the table at $BB8B by its
 * timer-index and reverses its X speed when it faces left; in any other state it goes on as StepListAndMove.
 * HOW: LoadObjectRecord, ApplyVelocityX, NegateVelocityX, table reads (bank 70).
 * Untraced: the tables at $BB89 and $BB8B are data the run only read.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: state 0: the last load or negate; else as StepListAndMove.
 */
Ended StartSideDrifter(uint8_t slot)
{
    AxisMove move;
    uint8_t state, nz, last, facing, byte, phase, index, left;
    bool carry;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        return StepListAndMove(slot);
    }
    jsr_push(0xBB56);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    facing = RD8(OBJ->facing_left[slot]);
    byte = rd8((uint16_t)(0xBB89 + facing));
    WR8(OBJ->vx_hi[slot], byte);
    e = then(e, ended_with_y(ended_load(byte), facing));
    jsr_push(0xBB62);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    phase = RD8(OBJ->life_timer[slot]);
    index = (uint8_t)(phase << 2);
    e = then(e, ended_with_y(ended_with_carry(ended_load(index), (phase & 0x40) != 0), index));
    byte = rd8((uint16_t)(0xBB8C + index));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xBB8B + index));
    WR8(OBJ->vx_lo[slot], byte);
    byte = rd8((uint16_t)(0xBB8E + index));
    WR8(OBJ->vy_hi[slot], byte);
    byte = rd8((uint16_t)(0xBB8D + index));
    WR8(OBJ->vy_lo[slot], byte);
    e = then(e, ended_load(byte));
    left = RD8(OBJ->facing_left[slot]);
    e = then(e, ended_load(left));
    if (left != 0) {
        jsr_push(0xBB88);
        e = then(e, of_sum(NegateVelocityX(slot)));
        jsr_pop();
    }
    return e;
}

void abi_StartSideDrifter(void)
{
    abi_ended(StartSideDrifter(cpu.x));
}
/*
 * @routine 0CAB:31C CountThenCue
 * @run 70:B31C
 * WHAT IT DOES: Counts the object's timer down; at zero it clears bits 3 and 4 of zField8E, plays the music again (CdPlayAgain), sends sound $BF, sets the angle word to
 * $8000 (OBJ_3568 $80, OBJ_35C0 0) and enters its next state.
 * HOW: DEC, two memory bit clears, CdPlayAgain, SoundCommand, INC.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
 */
CueEnd CountThenCue(uint8_t slot, uint8_t y)
{
    CueEnd r = { { 0 }, false, { 0 }, false, 0, { 0 }, { 0 } };
    uint8_t left, flags, state;

    left = DEC8(OBJ->life_timer[slot]);
    r.pre = ended_nz(left);
    if (left != 0) {
        return r;
    }
    flags = RD8(ZP->field8_e);
    flags = (uint8_t)(flags & 0xF7);
    WR8(ZP->field8_e, flags);
    flags = RD8(ZP->field8_e);
    flags = (uint8_t)(flags & 0xEF);
    WR8(ZP->field8_e, flags);
    jsr_push(0xB328);
    r.track = CdPlayAgain(slot, y);
    jsr_pop();
    r.played = true;
    r.command = 0xBF;
    jsr_push(0xB32D);
    r.call = SoundCommand(0xBF, slot, y);
    jsr_pop();
    r.sounded = true;
    WR8(OBJ->angle_hi[slot], 0x80);
    WR8(OBJ->angle_lo[slot], 0);
    state = INC8(OBJ->state[slot]);
    r.end = ended_nz(state);
    return r;
}

void abi_CountThenCue(void)
{
    CueEnd r = CountThenCue(cpu.x, cpu.y);

    abi_ended(r.pre);
    if (r.played) {
        play_track_exit_shim(r.track);
    }
    if (r.sounded) {
        sound_command_exit_shim(r.command, r.call);
    }
    abi_ended(r.end);
}


/*
 * @routine 0CAC:4EB BounceSparkOffWalls
 * @run 70:BCEB
 * WHAT IT DOES: Steps the animation list at $BD51; an object not on the first Y page is cleared; its hitbox half height comes from a table at $BD78 by the frame less $17; it moves, $0008 is added to
 * its Y speed (gravity), and it asks the terrain: a wall (request $22, height 4) halves its X speed and reverses it (the shift of the 16-bit speed through the carry, then NegateVelocityX); else a floor
 * (request 9) stops its X speed, sets its timer to $40 and moves OBJ_2800 on.
 * HOW: StepObjectFrames, ApplyVelocity, ProbeTerrain twice, table read (bank 70), a tail jump into NegateVelocityX or ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see abi_chain.
 */
ExitChain BounceSparkOffWalls(uint8_t slot, bool carry_in)
{
    ExitChain c = { { { STEP_ENDED, { 0 }, { { 0 }, { 0 }, false, 0, { 0 } }, { { 0 }, { 0 }, false, { 0 } }, { 0 } } }, 0 };
    FrameStep step;
    AxisMove move;
    Probe probe;
    uint8_t y = 0, y_high, frame, byte, low, high, hit, speed;
    bool carry;
    Sum index, sum;
    Ended e;

    step = step_list(slot, 0xBD51, 0xBCF6, &y, false);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    y_high = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_load(y_high));
    if (y_high != 0) {
        ClearObject(slot);
        chain_ended(&c, e);
        return c;
    }
    frame = RD8(OBJ->frame[slot]);
    index = sub_bytes(frame, 0x17, true);
    y = index.value;
    e = then(then(e, of_sum(index)), ended_with_y(ended_kept(), y));
    byte = rd8((uint16_t)(0xBD78 + y));
    WR8(OBJ->hitbox_half_h[slot], byte);
    e = then(e, ended_load(byte));
    jsr_push(0xBD0E);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    low = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(low, 8, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    e = then(e, of_sum(sum));
    high = RD8(OBJ->vy_hi[slot]);
    sum = add_bytes(high, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    e = then(e, of_sum(sum));
    WR8(ZP->product.mid, 4);
    carry = e.c_set ? e.c : carry_in;
    chain_ended(&c, e);
    jsr_push(0xBD28);
    probe = ProbeTerrain(0x22, slot, y, carry);
    jsr_pop();
    chain_probe(&c, probe);
    carry = probe_carry_out(probe, carry);
    hit = (uint8_t)(probe.answer & 0x02);
    e = ended_load(hit);
    if (hit != 0) {
        speed = RD8(OBJ->vx_hi[slot]);
        e = then(e, ended_with_carry(ended_load((uint8_t)(speed << 1)), (speed & 0x80) != 0));
        carry = ROR8(OBJ->vx_hi[slot], (speed & 0x80) != 0);
        e = then(e, ended_with_carry(ended_kept(), carry));
        carry = ROR8(OBJ->vx_lo[slot], carry);
        e = then(e, ended_with_carry(ended_kept(), carry));
        chain_ended(&c, e);
        chain_ended(&c, of_sum(NegateVelocityX(slot)));
        return c;
    }
    chain_ended(&c, e);
    jsr_push(0xBD3E);
    probe = ProbeTerrain(9, slot, probe.column, carry);
    jsr_pop();
    chain_probe(&c, probe);
    hit = (uint8_t)(probe.answer & 0x01);
    e = ended_load(hit);
    if (hit != 0) {
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0);
        WR8(OBJ->life_timer[slot], 0x40);
        e = then(e, ended_load(0x40));
        hit = INC8(OBJ->state[slot]);
        e = then(e, ended_nz(hit));
    }
    chain_ended(&c, e);
    return c;
}

void abi_BounceSparkOffWalls(void)
{
    ExitChain c = BounceSparkOffWalls(cpu.x, (cpu.p & P_C) != 0);

    abi_chain(&c);
}

/* One axis of 0CAC:60A: the distance from object $16 plus 8, as a 16-bit subtract and add; true when it is $10 or more. */
static bool axis_far(uint16_t lo_column, uint16_t hi_column, uint8_t slot, Ended *e)
{
    uint8_t mine, theirs, low, high;
    Sum sum;

    mine = RD8_IDX(lo_column, slot);
    theirs = RD8_IDX(lo_column, 0x16);
    sum = sub_bytes(mine, theirs, true);
    *e = then(*e, of_sum(sum));
    WR8(ZP->temp00, sum.value);
    mine = RD8_IDX(hi_column, slot);
    theirs = RD8_IDX(hi_column, 0x16);
    sum = sub_bytes(mine, theirs, sum.carry);
    *e = then(*e, of_sum(sum));
    WR8(ZP->temp01, sum.value);
    low = RD8(ZP->temp00);
    sum = add_bytes(low, 8, false);
    *e = then(*e, of_sum(sum));
    WR8(ZP->temp00, sum.value);
    high = RD8(ZP->temp01);
    sum = add_bytes(high, 0, sum.carry);
    *e = then(*e, of_sum(sum));
    if (sum.value != 0) {
        return true;
    }
    low = RD8(ZP->temp00);
    *e = then(*e, ended_compare(low, 0x10));
    return low >= 0x10;
}

/*
 * @routine 0CAC:60A TrackPointOrLand
 * @run 70:BE0A
 * WHAT IT DOES: Puts object $16 at the place of object $10, one length $30 along the object's angle OBJ_3510, and moves it. If the object has landed
 * (OBJ_3618 set), or is within 8 of object $16 on both axes, it lands: frame 2, OBJ_3300 $12, flags OBJ_2EE0 and OBJ_3618 set, and object $16
 * is placed and moved again along the angle. Otherwise it takes the angle to object $16 and moves at the speed OBJ_35C0 along it.
 * HOW: CopyObjectPlaceBack, SetVelocityFromAngle, ApplyVelocity, AngleToPoint, 16-bit subtract and add.
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocity.
 */
Ended TrackPointOrLand(uint8_t slot)
{
    AxisMove move;
    AngleExit angle;
    ScaleResult scale;
    uint8_t byte, placed;
    bool far;
    Ended e;

    byte = RD8(OBJ->f3510[slot]);
    e = ended_load(byte);
    WR8(ZP->temp02, byte);
    push8(slot);
    jsr_push(0xBE17);
    placed = CopyObjectPlaceBack(0x10, 0x16);
    jsr_pop();
    e = then(e, ended_with_y(ended_load(placed), 0x10));
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 0x30);
    byte = RD8(ZP->temp02);
    e = then(e, ended_load(byte));
    jsr_push(0xBE22);
    scale = SetVelocityFromAngle(byte, 0x16);
    jsr_pop();
    e = then(e, ended_of_scale(scale));
    jsr_push(0xBE25);
    move = ApplyVelocity(0x16);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    slot = pull8();
    e = then(e, ended_nz(slot));
    byte = RD8(OBJ->f3618[slot]);
    e = then(e, ended_load(byte));
    far = false;
    if (byte == 0) {
        far = axis_far(COL(OBJ->x_lo), COL(OBJ->x_hi), slot, &e);
        if (!far) {
            far = axis_far(COL(OBJ->y_lo), COL(OBJ->y_hi), slot, &e);
        }
    }
    if (!far) {
        WR8(OBJ->f3358[slot], 2);
        WR8(OBJ->f3300[slot], 0x12);
        WR8(OBJ->fields_2ee0[slot], 1);
        WR8(OBJ->f3618[slot], 1);
        jsr_push(0xBE8C);
        placed = CopyObjectPlaceBack(0x10, slot);
        jsr_pop();
        e = then(e, ended_with_y(ended_load(placed), 0x10));
        WR8(ZP->temp00, 0);
        WR8(ZP->temp01, 0x30);
        byte = RD8(OBJ->f3510[slot]);
        e = then(e, ended_load(byte));
        jsr_push(0xBE98);
        scale = SetVelocityFromAngle(byte, slot);
        jsr_pop();
        e = then(e, ended_of_scale(scale));
        jsr_push(0xBE9B);
        move = ApplyVelocity(slot);
        jsr_pop();
        return then(e, ended_of_axis_move(move));
    }
    byte = RD8(OBJ->x_hi[0x16]);
    WR8(ZP->temp09, byte);
    byte = RD8(OBJ->x_lo[0x16]);
    WR8(ZP->temp08, byte);
    byte = RD8(OBJ->y_hi[0x16]);
    WR8(ZP->temp0b, byte);
    byte = RD8(OBJ->y_lo[0x16]);
    WR8(ZP->temp0a, byte);
    e = then(e, ended_load(byte));
    jsr_push(0xBEB3);
    angle = AngleToPoint(slot);
    jsr_pop();
    e = then(e, ended_of_angle(angle));
    push8(angle.a);
    WR8(ZP->temp00, 0);
    byte = RD8(OBJ->angle_lo[slot]);
    WR8(ZP->temp01, byte);
    e = then(e, ended_load(byte));
    byte = pull8();
    e = then(e, ended_load(byte));
    jsr_push(0xBEBF);
    scale = SetVelocityFromAngle(byte, slot);
    jsr_pop();
    e = then(e, ended_of_scale(scale));
    jsr_push(0xBEC2);
    move = ApplyVelocity(slot);
    jsr_pop();
    return then(e, ended_of_axis_move(move));
}

void abi_TrackPointOrLand(void)
{
    abi_ended(TrackPointOrLand(cpu.x));
}

/*
 * @routine 0CAC:5E5 TrackThenAimAtPlayer
 * @run 70:BDE5
 * WHAT IT DOES: Runs TrackPointOrLand, adds the speed OBJ_35C0 to the angle OBJ_3510 and counts the timer down; at zero it takes the angle to the player and
 * the speed OBJ_35C0 along it, and goes on to the next state.
 * HOW: TrackPointOrLand, AngleToPlayer, SetVelocityFromAngle.
 * WAITS: no.
 * Entry: X object. Exit: the timer's count, or the state's count after the step.
 */
Ended TrackThenAimAtPlayer(uint8_t slot)
{
    AngleExit angle;
    ScaleResult scale;
    Sum sum;
    uint8_t byte, count;
    Ended e;

    jsr_push(0xBDE8);
    e = TrackPointOrLand(slot);
    jsr_pop();
    byte = RD8(OBJ->f3510[slot]);
    e = then(e, ended_load(byte));
    sum = add_bytes(byte, RD8(OBJ->angle_lo[slot]), false);
    WR8(OBJ->f3510[slot], sum.value);
    e = then(e, of_sum(sum));
    count = DEC8(OBJ->life_timer[slot]);
    e = then(e, ended_nz(count));
    if (count != 0) {
        return e;
    }
    jsr_push(0xBDFA);
    angle = AngleToPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_angle(angle));
    push8(angle.a);
    WR8(ZP->temp01, 0);
    byte = RD8(OBJ->angle_lo[slot]);
    WR8(ZP->temp01, byte);
    e = then(e, ended_load(byte));
    byte = pull8();
    e = then(e, ended_load(byte));
    jsr_push(0xBE06);
    scale = SetVelocityFromAngle(byte, slot);
    jsr_pop();
    e = then(e, ended_of_scale(scale));
    count = INC8(OBJ->state[slot]);
    return then(e, ended_nz(count));
}

void abi_TrackThenAimAtPlayer(void)
{
    abi_ended(TrackThenAimAtPlayer(cpu.x));
}

/*
 * @routine 0CAC:703 OrbitThenLeave
 * @run 70:BF03
 * WHAT IT DOES: Steps the animation list that a table at $BF66 names for the object's next kind, puts the object at X $0080, Y $0070 relative
 * to its screen and moves it along its angle OBJ_3568 by the length held in OBJ_3510 and the timer; then adds $80 to that 16-bit length,
 * turns the angle by one and, once the high byte of the length reaches $50, takes the speed from the turned-about angle with length $0400 and goes on to the next state.
 * HOW: A table read (bank 70), StepObjectFrames, SetVelocityFromAngle, ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: the state's count when it goes on, else the compare.
 */
Ended OrbitThenLeave(uint8_t slot)
{
    FrameStep step;
    AxisMove move;
    ScaleResult scale;
    Sum sum;
    uint8_t kind, y, byte, high, count;
    Ended e;

    kind = RD8(OBJ->next_kind[slot]);
    y = (uint8_t)(kind << 1);
    e = ended_with_carry(ended_with_y(ended_load(y), y), (kind & 0x80) != 0);
    byte = rd8((uint16_t)(0xBF66 + y));
    WR8(ZP->temp10, byte);
    byte = rd8((uint16_t)(0xBF67 + y));
    WR8(ZP->temp11, byte);
    jsr_push(0xBF15);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = then(e, ended_of_frame_step(step, true, y));
    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->x_lo[slot], 0x80);
    WR8(OBJ->y_lo[slot], 0x70);
    byte = RD8(OBJ->f3510[slot]);
    WR8(ZP->temp00, byte);
    byte = RD8(OBJ->life_timer[slot]);
    WR8(ZP->temp01, byte);
    byte = RD8(OBJ->angle_hi[slot]);
    e = then(e, ended_load(byte));
    jsr_push(0xBF35);
    scale = SetVelocityFromAngle(byte, slot);
    jsr_pop();
    e = then(e, ended_of_scale(scale));
    jsr_push(0xBF38);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    byte = RD8(OBJ->f3510[slot]);
    sum = add_bytes(byte, 0x80, false);
    WR8(OBJ->f3510[slot], sum.value);
    e = then(e, of_sum(sum));
    byte = RD8(OBJ->life_timer[slot]);
    sum = add_bytes(byte, 0, sum.carry);
    WR8(OBJ->life_timer[slot], sum.value);
    e = then(e, of_sum(sum));
    count = INC8(OBJ->angle_hi[slot]);
    e = then(e, ended_nz(count));
    high = RD8(OBJ->life_timer[slot]);
    e = then(e, ended_compare(high, 0x50));
    if (high < 0x50) {
        return e;
    }
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 4);
    byte = RD8(OBJ->angle_hi[slot]);
    sum = add_bytes(byte, 0x80, false);
    e = then(e, of_sum(sum));
    jsr_push(0xBF62);
    scale = SetVelocityFromAngle(sum.value, slot);
    jsr_pop();
    e = then(e, ended_of_scale(scale));
    count = INC8(OBJ->state[slot]);
    return then(e, ended_nz(count));
}

void abi_OrbitThenLeave(void)
{
    abi_ended(OrbitThenLeave(cpu.x));
}

/*
 * @routine 0CAB:339 SlideTurnByTable
 * @run 70:B339
 * WHAT IT DOES: Steps the animation list at $B3E0, sets the object's Y to $0070 and its Y speed to the cosine of the angle OBJ_3510 scaled by the 16-bit
 * length OBJ_3568:OBJ_35C0, moves it in Y, turns the angle half a circle, and runs the code a table at $B383 names for OBJ_2800 (SlideAngleDown,
 * SlideAngleUp, TurnAngleBack).
 * HOW: StepObjectFrames, ScaleByCosine, ApplyVelocityY, then a jump through the table.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; Y the doubled OBJ_2800.
 */
Jump SlideTurnByTable(uint8_t slot, Ended *before)
{
    FrameStep step;
    ScaleResult scale;
    AxisMove move;
    Jump jump = { { 0 }, 0, true };
    uint8_t y, byte, state, index, low, high;
    Ended e;

    step = step_list(slot, 0xB3E0, 0xB344, &y, false);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0x70);
    byte = RD8(OBJ->angle_lo[slot]);
    WR8(ZP->temp00, byte);
    byte = RD8(OBJ->angle_hi[slot]);
    WR8(ZP->temp01, byte);
    byte = RD8(OBJ->f3510[slot]);
    e = then(e, ended_load(byte));
    jsr_push(0xB35C);
    scale = ScaleByCosine(byte);
    jsr_pop();
    e = then(e, ended_of_scale(scale));
    byte = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], byte);
    byte = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], byte);
    e = then(e, ended_load(byte));
    jsr_push(0xB369);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    byte = RD8(OBJ->f3510[slot]);
    byte = (uint8_t)(byte ^ 0x80);
    e = then(e, ended_load(byte));
    WR8(OBJ->f3510[slot], byte);
    state = RD8(OBJ->f2800[slot]);
    index = (uint8_t)(state << 1);
    e = then(e, ended_with_carry(ended_with_y(ended_load(index), index), (state & 0x80) != 0));
    low = rd8((uint16_t)(0xB383 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB384 + index));
    WR8(ZP->temp01, high);
    *before = e;
    jump.end = ended_load(high);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_SlideTurnByTable(void)
{
    Ended before;
    Jump jump = SlideTurnByTable(cpu.x, &before);

    abi_ended(before);
    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0CAB:457 MoveAlongAndSlow
 * @run 70:B457
 * WHAT IT DOES: Moves the object along its angle OBJ_3510 by the 16-bit length OBJ_3568:OBJ_35C0. Outside the screen box (InsideScreenBox) it clears OBJ_3670
 * and OBJ_2800 and stops. Inside, it takes a byte of a table at $B4A0 (picked by OBJ_3670 and 7) off the length, and when the length runs out
 * it sets the timer from a table at $B4A8 picked at random and goes on to the next OBJ_2800.
 * HOW: SetVelocityFromAngle, ApplyVelocity, InsideScreenBox, a 16-bit subtract, NextRandom, table reads (bank 70).
 * Untraced: the outside arm (B46F).
 * WAITS: no.
 * Entry: X object. Exit: the last count, subtract or load.
 */
Ended MoveAlongAndSlow(uint8_t slot)
{
    ScaleResult scale;
    AxisMove move;
    ViewTest view;
    RandomResult random;
    Sum sum;
    uint8_t byte, index, count, pick;
    Ended e;

    byte = RD8(OBJ->angle_lo[slot]);
    WR8(ZP->temp00, byte);
    byte = RD8(OBJ->angle_hi[slot]);
    WR8(ZP->temp01, byte);
    byte = RD8(OBJ->f3510[slot]);
    e = ended_load(byte);
    jsr_push(0xB467);
    scale = SetVelocityFromAngle(byte, slot);
    jsr_pop();
    e = then(e, ended_of_scale(scale));
    jsr_push(0xB46A);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    jsr_push(0xB46D);
    view = InsideScreenBox(slot);
    jsr_pop();
    e = then(e, view.end);
    if (!view.in_view) {
        WR8(OBJ->f3670[slot], 0);
        WR8(OBJ->f2800[slot], 0);
        return e;
    }
    byte = RD8(OBJ->f3670[slot]);
    index = (uint8_t)(byte & 0x07);
    e = then(e, ended_with_y(ended_load(index), index));
    byte = RD8(OBJ->angle_lo[slot]);
    sum = sub_bytes(byte, rd8((uint16_t)(0xB4A0 + index)), true);
    WR8(OBJ->angle_lo[slot], sum.value);
    e = then(e, of_sum(sum));
    byte = RD8(OBJ->angle_hi[slot]);
    sum = sub_bytes(byte, 0, sum.carry);
    WR8(OBJ->angle_hi[slot], sum.value);
    e = then(e, of_sum(sum));
    if (sum.carry) {
        return e;
    }
    jsr_push(0xB493);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = (uint8_t)(random.value & 0x03);
    e = then(e, ended_with_y(ended_load(pick), pick));
    byte = rd8((uint16_t)(0xB4A8 + pick));
    WR8(OBJ->life_timer[slot], byte);
    e = then(e, ended_load(byte));
    count = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(count));
}

void abi_MoveAlongAndSlow(void)
{
    abi_ended(MoveAlongAndSlow(cpu.x));
}

/*
 * @routine 0CAB:7CD LaunchThenFall
 * @run 70:B7CD
 * WHAT IT DOES: State 0: plays sound $C0, quarters OBJ_36C8, takes the angle to the player plus $80 as the object's angle with the length $0080 and goes on to the next
 * OBJ_2800. Later: when it has been hit five times (OBJ_33B0) it sets OBJ_2EE0 to 3, its state to 4 and OBJ_2800 to 9; else it moves and steps the animation list at $B829, and when the
 * list ends it sets its frame to 1, its state to 3 and OBJ_2800 to 0.
 * HOW: SoundCommand, LSR (twice), AngleToPlayer, SetVelocityFromAngle, ApplyVelocity, StepObjectFrames.
 * Untraced: the hit arm (B7FB to B80A).
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see abi_sound_end.
 */
SoundEnd LaunchThenFall(uint8_t slot, uint8_t y_entry)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    AngleExit angle;
    ScaleResult scale;
    AxisMove move;
    FrameStep step;
    Sum sum;
    uint8_t phase, byte, hits, y, shifted, count;
    bool bit;
    Ended e;

    phase = RD8(OBJ->f2800[slot]);
    e = ended_load(phase);
    if (phase == 0) {
        r.before = then(e, ended_load(0xC0));
        r.sounded = true;
        r.command = 0xC0;
        jsr_push(0xB7D7);
        r.call = SoundCommand(0xC0, slot, y_entry);
        jsr_pop();
        shifted = LSR8_VALUE(OBJ->f36c8[slot], bit);
        shifted = LSR8_VALUE(OBJ->f36c8[slot], bit);
        e = ended_with_carry(ended_nz(shifted), bit);
        jsr_push(0xB7E0);
        angle = AngleToPlayer(slot);
        jsr_pop();
        e = then(e, ended_of_angle(angle));
        sum = add_bytes(angle.a, 0x80, false);
        e = then(e, of_sum(sum));
        push8(sum.value);
        WR8(ZP->temp00, 0x80);
        WR8(ZP->temp01, 0);
        byte = pull8();
        e = then(e, ended_load(byte));
        jsr_push(0xB7F0);
        scale = SetVelocityFromAngle(byte, slot);
        jsr_pop();
        e = then(e, ended_of_scale(scale));
        count = INC8(OBJ->f2800[slot]);
        r.end = then(e, ended_nz(count));
        return r;
    }
    hits = RD8(OBJ->hit_taken[slot]);
    e = then(e, ended_compare(hits, 5));
    if (hits >= 5) {
        WR8(OBJ->fields_2ee0[slot], 3);
        WR8(OBJ->state[slot], 4);
        WR8(OBJ->f2800[slot], 9);
        r.end = then(e, ended_load(9));
        return r;
    }
    jsr_push(0xB80E);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    step = step_list(slot, 0xB829, 0xB819, &y, false);
    e = then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
    if (step != FRAME_LIST_ENDED) {
        r.end = e;
        return r;
    }
    WR8(OBJ->frame[slot], 1);
    WR8(OBJ->state[slot], 3);
    WR8(OBJ->f2800[slot], 0);
    r.end = then(e, ended_load(3));
    return r;
}

void abi_LaunchThenFall(void)
{
    abi_sound_end(LaunchThenFall(cpu.x, cpu.y));
}

/*
 * @routine 0CAB:147 StartTileScriptByKind
 * @run 70:B147
 * WHAT IT DOES: Loads the object's record and sets its timer from a table at $B15D by its next kind; for kind 0 it draws the tile script 6 of group 0 (DrawTileScript).
 * HOW: LoadObjectRecord, a table read (bank 70), DrawTileScript.
 * WAITS: no.
 * Entry: X object. Exit: kind 0: as DrawTileScript's; else the compare with 0.
 */
Ended StartTileScriptByKind(uint8_t slot)
{
    TileRun run;
    uint8_t nz, last, kind, timer;
    bool carry;
    Ended e;

    jsr_push(0xB14A);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    kind = RD8(OBJ->next_kind[slot]);
    e = then(e, ended_with_y(ended_nz(kind), kind));
    timer = rd8((uint16_t)(0xB15D + kind));
    WR8(OBJ->life_timer[slot], timer);
    e = then(e, ended_load(timer));
    e = then(e, cpy(kind, 0));
    if (kind != 0) {
        return e;
    }
    e = then(e, ended_load(6));
    jsr_push(0xB15C);
    run = DrawTileScript(6, 0);
    jsr_pop();
    return then(e, ended_of_tile_run(run, 6));
}

void abi_StartTileScriptByKind(void)
{
    abi_ended(StartTileScriptByKind(cpu.x));
}

/*
 * @routine 0CAB:161 CountThenTileScript
 * @run 70:B161
 * WHAT IT DOES: Counts the object's timer down; at zero it draws the tile script of group 0 numbered the next kind plus 7 (DrawTileScript) and goes on to the next state.
 * HOW: DEC, DrawTileScript, INC.
 * WAITS: no.
 * Entry: X object. Exit: the count; at zero, the state's count.
 */
Ended CountThenTileScript(uint8_t slot)
{
    TileRun run;
    Sum sum;
    uint8_t left, kind, item, state;
    Ended e;

    left = DEC8(OBJ->life_timer[slot]);
    e = ended_nz(left);
    if (left != 0) {
        return e;
    }
    kind = RD8(OBJ->next_kind[slot]);
    e = then(e, ended_load(kind));
    sum = add_bytes(kind, 6, false);
    e = then(e, of_sum(sum));
    item = (uint8_t)(sum.value + 1);
    e = then(e, ended_nz(item));
    e = then(e, ended_with_y(ended_kept(), 0));
    jsr_push(0xB171);
    run = DrawTileScript(item, 0);
    jsr_pop();
    e = then(e, ended_of_tile_run(run, item));
    state = INC8(OBJ->state[slot]);
    return then(e, ended_nz(state));
}

void abi_CountThenTileScript(void)
{
    abi_ended(CountThenTileScript(cpu.x));
}
