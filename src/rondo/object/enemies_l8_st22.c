/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 0F6B, second part (enemy kinds not yet identified), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/* The ending the two orbit routines share: the angle byte steps by $FF90 and, when it lands on $80 coming from another value, the object is placed at Y $40, X $B0 less the scroll and goes on. */
static Ended orbit_step_end(uint8_t slot, uint8_t next_phase, bool *placed)
{
    uint8_t before, low, high, now, scroll, other;
    Sum sum, diff;
    Ended e;

    before = RD8(OBJ->f37d0[slot]);
    WR8(ZP->temp00, before);
    e = ended_load(before);
    low = RD8(OBJ->f3778[slot]);
    sum = add_bytes(0x90, low, false);
    WR8(OBJ->f3778[slot], sum.value);
    high = RD8(OBJ->f37d0[slot]);
    sum = add_bytes(0xFF, high, sum.carry);
    WR8(OBJ->f37d0[slot], sum.value);
    e = then(then(e, ended_load(0xFF)), ended_of_sum(sum));
    other = RD8(ZP->temp00);
    now = sum.value;
    e = then(e, ended_compare(now, other));
    *placed = false;
    if (now == other) {
        return e;
    }
    e = then(e, ended_compare(now, 0x80));
    if (now != 0x80) {
        return e;
    }
    WR8(OBJ->y_lo[slot], 0x40);
    WR8(OBJ->y_frac[slot], 0);
    WR8(OBJ->f3778[slot], 0);
    scroll = RD8(SCROLL->x_lo[0]);
    diff = sub_bytes(0xB0, scroll, true);
    WR8(OBJ->x_lo[slot], diff.value);
    WR8(OBJ->x_frac[slot], 0);
    e = then(e, ended_of_sum(diff));
    (void)next_phase;
    *placed = true;
    return e;
}

/*
 * @routine 0F6B:52F OrbitCosineSineThenSettle
 * @run 70:B52F
 * WHAT IT DOES: Moves the object on an ellipse: X speed from the cosine of OBJ_37D0 scaled by $0120, Y speed from the sine scaled by $0050; turns OBJ_37D0 on by -$0070 a frame (a 16-bit add with OBJ_3778); when OBJ_37D0 reaches $80 it is placed at Y $0040, X $B0 less the scroll and goes to the next OBJ_2800.
 * HOW: ScaleByCosine, ScaleBySine, ApplyVelocityX and Y, a 16-bit add in memory, two compares.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended OrbitCosineSineThenSettle(uint8_t slot)
{
    uint8_t angle, value;
    ScaleResult scale;
    AxisMove move;
    bool settled;
    Ended e;

    WR8(ZP->temp00, 0x20);
    WR8(ZP->temp01, 1);
    angle = RD8(OBJ->f37d0[slot]);
    e = ended_load(angle);
    jsr_push(0xB53D);
    scale = ScaleByCosine(angle);
    jsr_pop();
    e = then(e, l8_of_scale(scale));
    value = RD8(ZP->product.hi);
    WR8(OBJ->vx_hi[slot], value);
    value = RD8(ZP->product.mid);
    WR8(OBJ->vx_lo[slot], value);
    e = then(e, ended_load(value));
    jsr_push(0xB54A);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    WR8(ZP->temp00, 0x50);
    WR8(ZP->temp01, 0);
    angle = RD8(OBJ->f37d0[slot]);
    e = then(e, ended_load(angle));
    jsr_push(0xB558);
    scale = ScaleBySine(angle);
    jsr_pop();
    e = then(e, l8_of_scale(scale));
    value = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], value);
    value = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], value);
    e = then(e, ended_load(value));
    jsr_push(0xB565);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    e = then(e, orbit_step_end(slot, 1, &settled));
    if (settled) {
        e = then(e, ended_nz(INC8(OBJ->f2800[slot])));
    }
    return e;
}

void abi_OrbitCosineSineThenSettle(void)
{
    abi_ended(OrbitCosineSineThenSettle(cpu.x));
}

/*
 * @routine 0F6B:59E ResetOrbitAndAdvance
 * @run 70:B59E
 * WHAT IT DOES: Clears OBJ_3778, sets OBJ_37D0 and OBJ_35C0 $80 and goes to the next OBJ_2800.
 * HOW: Stores and an INC.
 * WAITS: no.
 * Entry: X object. Exit: A $80, N and Z from the new OBJ_2800.
 */
Ended ResetOrbitAndAdvance(uint8_t slot)
{
    WR8(OBJ->f3778[slot], 0);
    WR8(OBJ->f37d0[slot], 0x80);
    WR8(OBJ->f35c0[slot], 0x80);
    return then(ended_load(0x80), ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_ResetOrbitAndAdvance(void)
{
    abi_ended(ResetOrbitAndAdvance(cpu.x));
}

/*
 * @routine 0F6B:5B1 OrbitWithRisingSineThenSettle
 * @run 70:B5B1
 * WHAT IT DOES: As OrbitCosineSineThenSettle, but the Y speed comes from the sine of OBJ_3828 scaled by $0100, which turns on by $01C0 a frame; at $80 the object goes to OBJ_2800 2.
 * HOW: ScaleBySine, ScaleByCosine, ApplyVelocityY and X, two 16-bit adds in memory, two compares.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended OrbitWithRisingSineThenSettle(uint8_t slot)
{
    uint8_t angle, value, low, high;
    ScaleResult scale;
    AxisMove move;
    Sum sum;
    bool settled;
    Ended e;

    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 1);
    angle = RD8(OBJ->f3828[slot]);
    e = ended_load(angle);
    jsr_push(0xB5BF);
    scale = ScaleBySine(angle);
    jsr_pop();
    e = then(e, l8_of_scale(scale));
    value = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], value);
    value = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], value);
    e = then(e, ended_load(value));
    jsr_push(0xB5CC);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    low = RD8(OBJ->f3880[slot]);
    sum = add_bytes(0xC0, low, false);
    WR8(OBJ->f3880[slot], sum.value);
    high = RD8(OBJ->f3828[slot]);
    sum = add_bytes(1, high, sum.carry);
    WR8(OBJ->f3828[slot], sum.value);
    e = then(then(e, ended_load(1)), ended_of_sum(sum));
    WR8(ZP->temp00, 0x20);
    WR8(ZP->temp01, 1);
    angle = RD8(OBJ->f37d0[slot]);
    e = then(e, ended_load(angle));
    jsr_push(0xB5EB);
    scale = ScaleByCosine(angle);
    jsr_pop();
    e = then(e, l8_of_scale(scale));
    value = RD8(ZP->product.hi);
    WR8(OBJ->vx_hi[slot], value);
    value = RD8(ZP->product.mid);
    WR8(OBJ->vx_lo[slot], value);
    e = then(e, ended_load(value));
    jsr_push(0xB5F8);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    e = then(e, orbit_step_end(slot, 2, &settled));
    if (settled) {
        WR8(OBJ->f2800[slot], 2);
        e = then(e, ended_load(2));
    }
    return e;
}

void abi_OrbitWithRisingSineThenSettle(void)
{
    abi_ended(OrbitWithRisingSineThenSettle(cpu.x));
}

/*
 * @routine 0F6B:711 StepListByNumberTable
 * @run 70:B711
 * WHAT IT DOES: Steps the animation list that the table at $BA0A names for OBJ_34B8.
 * HOW: A table read of two bytes into zTemp10-11, a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step.
 */
Ended StepListByNumberTable(uint8_t slot, uint8_t y_entry)
{
    uint8_t number, y, lo, hi;
    FrameStep step;
    Ended e;

    (void)y_entry;
    number = RD8(OBJ->f34b8[slot]);
    y = (uint8_t)(number << 1);
    e = ended_with_y(ended_with_carry(ended_load(y), number >= 0x80), y);
    lo = rd8((uint16_t)(0xBA0A + y));
    WR8(ZP->temp10, lo);
    e = then(e, ended_load(lo));
    hi = rd8((uint16_t)(0xBA0B + y));
    WR8(ZP->temp11, hi);
    e = then(e, ended_load(hi));
    step = StepObjectFrames(slot, &y);
    return then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
}

void abi_StepListByNumberTable(void)
{
    abi_ended(StepListByNumberTable(cpu.x, cpu.y));
}

/*
 * @routine 0F6B:70B ClearAnimationThenStepListByNumber
 * @run 70:B70B
 * WHAT IT DOES: Clears the animation step and timer and runs StepListByNumberTable.
 * HOW: Two stores, running into the next routine.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as StepListByNumberTable.
 */
Ended ClearAnimationThenStepListByNumber(uint8_t slot, uint8_t y_entry)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    return StepListByNumberTable(slot, y_entry);
}

void abi_ClearAnimationThenStepListByNumber(void)
{
    abi_ended(ClearAnimationThenStepListByNumber(cpu.x, cpu.y));
}

/*
 * @routine 0F6B:723 StartPieceABInFreeSlot
 * @run 70:B723
 * WHAT IT DOES: Starts an object of type $AB in a free slot of $30 to $3F at the object's place and says yes (C 1); with no free slot says no (C 0).
 * HOW: FindFreeSlotFrom30, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: C 1 started, 0 none.
 */
Ended StartPieceABInFreeSlot(uint8_t slot)
{
    uint8_t found_slot, last, placed;
    bool found;
    Ended e;

    jsr_push(0xB726);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = ended_of_find_slot(found_slot, last, found);
    if (!found) {
        return e;
    }
    jsr_push(0xB72D);
    placed = StartObjectAtPlace(0xAB, slot, found_slot);
    jsr_pop();
    return ended_with_carry(then(e, ended_load(placed)), true);
}

void abi_StartPieceABInFreeSlot(void)
{
    abi_ended(StartPieceABInFreeSlot(cpu.x));
}

/*
 * @routine 0F6B:72F LoadRecordFrameByKindThenFall
 * @run 70:B72F
 * WHAT IT DOES: In state 0 loads the record, sets sprite set 4 and takes the frame and OBJ_3510 from tables at $B76C and $B771 by OBJ_3040. Later, while OBJ_3510 counts down, kind 4 sets bit 4 of zField8E and plays sound $FE at zero; at zero it moves in Y and falls.
 * HOW: LoadObjectRecord, table reads, a memory decrement, SoundCommand, ApplyVelocityY, ApplyGravity.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
SoundEnd LoadRecordFrameByKindThenFall(uint8_t slot, uint8_t y_entry)
{
    uint8_t state, last, nz, y, value, timer, now, kind;
    bool carry;
    AxisMove move;
    GravityResult gravity;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        jsr_push(0xB737);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        e = then(e, ended_of_record(last, nz, carry));
        WR8(OBJ->sprite_set[slot], 4);
        e = then(e, ended_load(4));
        y = RD8(OBJ->f3040[slot]);
        e = then(e, ended_with_y(ended_nz(y), y));
        value = rd8((uint16_t)(0xB76C + y));
        WR8(OBJ->frame[slot], value);
        value = rd8((uint16_t)(0xB771 + y));
        WR8(OBJ->f3510[slot], value);
        r.end = then(e, ended_load(value));
        return r;
    }
    timer = RD8(OBJ->f3510[slot]);
    e = then(e, ended_load(timer));
    if (timer == 0) {
        jsr_push(0xB768);
        move = ApplyVelocityY(slot);
        jsr_pop();
        e = then(e, ended_of_axis_move(move));
        jsr_push(0xB76B);
        gravity = ApplyGravity(slot);
        jsr_pop();
        r.end = then(e, l8_of_gravity(gravity));
        return r;
    }
    now = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(now));
    if (now != 0) {
        r.end = e;
        return r;
    }
    kind = RD8(OBJ->f3040[slot]);
    e = then(e, ended_compare(kind, 4));
    if (kind != 4) {
        r.end = e;
        return r;
    }
    RMW8(ZP->field8e, old, old | 0x10);
    r.sounded = true;
    r.command = 0xFE;
    r.before = then(e, ended_load(0xFE));
    jsr_push(0xB764);
    r.call = SoundCommand(0xFE, slot, y_entry);
    jsr_pop();
    return r;
}

void abi_LoadRecordFrameByKindThenFall(void)
{
    abi_sound_end(LoadRecordFrameByKindThenFall(cpu.x, cpu.y));
}
