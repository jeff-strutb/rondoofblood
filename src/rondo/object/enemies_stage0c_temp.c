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
