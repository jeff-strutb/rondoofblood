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
