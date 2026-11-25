/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 0B4C (enemy kinds not yet identified), by routine. Bodies return how the
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

/*
 * @routine 0B4C:007 CountDownOrPickNextTimer
 * @run 70:B807
 * WHAT IT DOES: Counts OBJ_34B8 down and says no (C 0). At zero it clears the animation and OBJ_3778, goes to state 7, advances OBJ_35C0, takes OBJ_34B8 from the table at $B830 by its low two bits and says yes (C 1).
 * HOW: A test, a memory decrement, stores, an INC, a table read.
 * WAITS: no.
 * Entry: X object. Exit: C 1 when the timer ran out, 0 when not; N and Z from the last load or decrement.
 */
Ended CountDownOrPickNextTimer(uint8_t slot)
{
    uint8_t timer, step, pick, value;
    Ended e;

    timer = RD8(OBJ->f34b8[slot]);
    e = ended_load(timer);
    if (timer != 0) {
        e = then(e, ended_nz(DEC8(OBJ->f34b8[slot])));
        return ended_with_carry(e, false);
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 7);
    WR8(OBJ->f3778[slot], 0);
    e = then(e, ended_load(7));
    e = then(e, ended_nz(INC8(OBJ->f35c0[slot])));
    step = RD8(OBJ->f35c0[slot]);
    pick = step & 0x03;
    e = then(e, ended_with_y(ended_load(pick), pick));
    value = rd8((uint16_t)(0xB830 + pick));
    WR8(OBJ->f34b8[slot], value);
    return ended_with_carry(then(e, ended_load(value)), true);
}

void abi_CountDownOrPickNextTimer(void)
{
    abi_ended(CountDownOrPickNextTimer(cpu.x));
}

/*
 * @routine 0B4C:051 StepListThenPickStateByLuck
 * @run 70:B851
 * WHAT IT DOES: In state 0, when the frame is 4, advances OBJ_2800; otherwise (and in later states when the animation list at $B8D2 ends) it takes frame 2, the facing of the player's side, clears the animation timer and picks the next state and OBJ_2800 from the tables at $B8A2 and $B89D by OBJ_32A8 and a random number.
 * HOW: StepObjectFrames, SideOfPlayer, NextRandom, table reads, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran. The state 0 arm ran in no script; the draft stops there.
 */
Ended StepListThenPickStateByLuck(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, state, frame, side_bit, aim, shifted, pick, index, value;
    FrameStep step;
    SideResult side;
    RandomResult random;
    Sum sum;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        frame = RD8(OBJ->frame[slot]);
        e = then(e, ended_compare(frame, 4));
        if (frame == 4) {
            return then(e, ended_nz(INC8(OBJ->f2800[slot])));
        }
    } else {
        e = then(e, l8_step_list(slot, 0xB8D2, 0xB86C, true, &y, &step));
        if (step != FRAME_LIST_ENDED) {
            return e;
        }
    }
    WR8(OBJ->frame[slot], 2);
    e = then(e, ended_load(2));
    jsr_push(0xB876);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_side(side));
    side_bit = side.sides & 0x01;
    e = then(e, ended_load(side_bit));
    WR8(OBJ->facing_left[slot], side_bit);
    WR8(OBJ->anim_timer[slot], 0);
    shifted = RD8(OBJ->f32a8[slot]);
    aim = (uint8_t)((shifted >> 1) & 0xF0);
    e = then(e, ended_with_carry(ended_load(aim), (shifted & 0x01) != 0));
    WR8(ZP->temp00, aim);
    jsr_push(0xB889);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = random.value & 0x0F;
    e = then(e, ended_load(pick));
    sum = add_bytes(pick, RD8(ZP->temp00), false);
    y = sum.value;
    e = then(e, ended_with_y(ended_of_sum(sum), y));
    index = rd8((uint16_t)(0xB8A2 + y));
    y = index;
    e = then(e, ended_with_y(ended_load(index), y));
    value = rd8((uint16_t)(0xB89D + y));
    WR8(OBJ->state[slot], value);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_load(value));
}

void abi_StepListThenPickStateByLuck(void)
{
    abi_ended(StepListThenPickStateByLuck(cpu.x, cpu.y));
}

/*
 * @routine 0B4C:0EF StepListThenLeapByDistance
 * @run 70:B8EF
 * WHAT IT DOES: Steps the animation list at $B936; when it ends plays sound $B5, clears the animation, and from the distance to the player in X (eight steps) sets the speed from the table at $B93F, goes to the next OBJ_2800 and, when facing left, reverses the X speed.
 * HOW: StepObjectFrames, SoundCommand, DistanceXToPlayer, table reads, stores, NegateVelocityX.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran. Reversing the speed ran in no script; the draft stops there.
 */
SoundEnd StepListThenLeapByDistance(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, index, value, facing;
    FrameStep step;
    PlayerDistance distance;
    Sum negated;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e = l8_step_list(slot, 0xB936, 0xB8FA, true, &y, &step);

    if (step != FRAME_LIST_ENDED) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0xB5;
    r.before = then(e, ended_load(0xB5));
    jsr_push(0xB901);
    r.call = SoundCommand(0xB5, slot, y);
    jsr_pop();
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    jsr_push(0xB90A);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = ended_of_distance(distance);
    index = (uint8_t)((distance.distance >> 5) << 2);
    e = then(e, ended_with_y(ended_with_carry(ended_load(index), false), index));
    value = rd8((uint16_t)(0xB940 + index));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xB93F + index));
    WR8(OBJ->vx_lo[slot], value);
    value = rd8((uint16_t)(0xB942 + index));
    WR8(OBJ->vy_hi[slot], value);
    value = rd8((uint16_t)(0xB941 + index));
    WR8(OBJ->vy_lo[slot], value);
    e = then(e, ended_load(value));
    e = then(e, ended_nz(INC8(OBJ->f2800[slot])));
    facing = RD8(OBJ->facing_left[slot]);
    e = then(e, ended_load(facing));
    if (facing != 0) {
        jsr_push(0xB935);
        negated = NegateVelocityX(slot);
        jsr_pop();
        e = then(e, ended_of_sum(negated));
    }
    r.end = e;
    return r;
}

void abi_StepListThenLeapByDistance(void)
{
    abi_sound_end(StepListThenLeapByDistance(cpu.x, cpu.y));
}

/*
 * @routine 0B4C:18A CountDownThenState5
 * @run 70:B98A
 * WHAT IT DOES: Counts OBJ_3510 down; at zero clears the animation and goes to state 5.
 * HOW: A memory decrement, stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended CountDownThenState5(uint8_t slot)
{
    uint8_t timer;
    Ended e;

    timer = DEC8(OBJ->f3510[slot]);
    e = ended_nz(timer);
    if (timer != 0) {
        return e;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 5);
    return then(e, ended_load(5));
}

void abi_CountDownThenState5(void)
{
    abi_ended(CountDownThenState5(cpu.x));
}

/*
 * @routine 0B4C:336 SoundAndTileWriteAtFrame24
 * @run 70:BB36
 * WHAT IT DOES: At frame $24 plays sound $19 and writes two tile words into the background map at the column the object's X names (its position plus the scroll less 2, divided by 8, low six bits).
 * HOW: SoundCommand, SelectVramWriteAddress, a 16-bit add and three shifts, VDC writes, ClearVdcControlHigh, SelectVramData.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: A $51, C from the last rotate, V from the subtract.
 */
SoundEnd SoundAndTileWriteAtFrame24(uint8_t slot, uint8_t y_entry)
{
    uint8_t frame, low, high, shifted, carry_out;
    Sum sum, diff;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    frame = RD8(OBJ->frame[slot]);
    e = ended_compare(frame, 0x24);
    if (frame != 0x24) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0x19;
    r.before = then(e, ended_load(0x19));
    jsr_push(0xBB42);
    r.call = SoundCommand(0x19, slot, y_entry);
    jsr_pop();
    jsr_push(0xBB45);
    SelectVramWriteAddress();
    jsr_pop();
    low = RD8(OBJ->x_lo[slot]);
    sum = add_bytes(low, RD8(SCROLL->x_lo[0]), false);
    WR8(ZP->temp00, sum.value);
    high = RD8(OBJ->x_hi[slot]);
    sum = add_bytes(high, RD8(SCROLLE->x_hi_screen[0]), sum.carry);
    diff = sub_bytes(sum.value, 2, true);
    WR8(ZP->temp01, diff.value);
    carry_out = LSR8(ZP->temp01);
    carry_out = ROR8(ZP->temp00, carry_out != 0);
    carry_out = LSR8(ZP->temp01);
    carry_out = ROR8(ZP->temp00, carry_out != 0);
    carry_out = LSR8(ZP->temp01);
    carry_out = ROR8(ZP->temp00, carry_out != 0);
    shifted = (uint8_t)(RD8(ZP->temp00) & 0x3F);
    WR8(VDC->data_lo, shifted);
    WR8(VDC->data_hi, 0x06);
    jsr_push(0xBB74);
    ClearVdcControlHigh();
    jsr_pop();
    jsr_push(0xBB77);
    SelectVramData();
    jsr_pop();
    WR8(VDC->data_lo, 0x8F);
    WR8(VDC->data_hi, 0x51);
    WR8(VDC->data_lo, 0x8F);
    WR8(VDC->data_hi, 0x51);
    r.end = ended_with_overflow(ended_with_carry(ended_load(0x51), carry_out != 0), diff.overflow);
    return r;
}

void abi_SoundAndTileWriteAtFrame24(void)
{
    abi_sound_end(SoundAndTileWriteAtFrame24(cpu.x, cpu.y));
}

/*
 * @routine 0B4C:38C SoundAndSpawnFiveAtFrame27
 * @run 70:BB8C
 * WHAT IT DOES: At frame $27 plays sound $B5 and starts up to five objects of type $27 in free slots of $20 to $3F at its place, each facing as this one and numbered 0 to 4 in OBJ_34B8.
 * HOW: SoundCommand, a loop of FindFreeSlotFrom20 and StartObjectAtPlace with a counter in zTemp00.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
SoundEnd SoundAndSpawnFiveAtFrame27(uint8_t slot, uint8_t y_entry)
{
    uint8_t frame, number, found_slot, last, placed, facing;
    bool found;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    frame = RD8(OBJ->frame[slot]);
    e = ended_compare(frame, 0x27);
    if (frame != 0x27) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0xB5;
    r.before = then(e, ended_load(0xB5));
    jsr_push(0xBB98);
    r.call = SoundCommand(0xB5, slot, y_entry);
    jsr_pop();
    WR8(ZP->temp00, 0);
    e = ended_kept();
    for (;;) {
        jsr_push(0xBB9D);
        found_slot = FindFreeSlotFrom20(&last, &found);
        jsr_pop();
        e = ended_of_find_slot(found_slot, last, found);
        if (!found) {
            break;
        }
        jsr_push(0xBBA4);
        placed = StartObjectAtPlace(0x27, slot, found_slot);
        jsr_pop();
        e = then(e, ended_load(placed));
        facing = RD8(OBJ->facing_left[slot]);
        WR8(OBJ->facing_left[found_slot], facing);
        number = RD8(ZP->temp00);
        WR8(OBJ->f34b8[found_slot], number);
        e = then(e, ended_compare(number, 4));
        if (number == 4) {
            break;
        }
        e = then(e, ended_nz(INC8(ZP->temp00)));
    }
    r.end = e;
    return r;
}

void abi_SoundAndSpawnFiveAtFrame27(void)
{
    abi_sound_end(SoundAndSpawnFiveAtFrame27(cpu.x, cpu.y));
}

/*
 * @routine 0B4C:409 StepListThenState5
 * @run 70:BC09
 * WHAT IT DOES: Steps the animation list at $BC22; when it ends clears the animation and goes to state 5.
 * HOW: StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListThenState5(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xBC22, 0xBC14, true, &y, &step);

    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 5);
    return then(e, ended_load(5));
}

void abi_StepListThenState5(void)
{
    abi_ended(StepListThenState5(cpu.x, cpu.y));
}

/*
 * @routine 0B4C:447 SoundThenLeapByFacing
 * @run 70:BC47
 * WHAT IT DOES: Plays sound $B8, gives the object the X speed from the table at $BC6B by its facing and the Y speed -$0340 (high $FC, low $C0) and goes to the next OBJ_2800.
 * HOW: SoundCommand, table reads, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
SoundEnd SoundThenLeapByFacing(uint8_t slot, uint8_t y_entry)
{
    uint8_t facing, y, value;
    SoundEnd r = l8_sound_none(ended_kept());

    r.sounded = true;
    r.command = 0xB8;
    r.before = ended_load(0xB8);
    jsr_push(0xBC4C);
    r.call = SoundCommand(0xB8, slot, y_entry);
    jsr_pop();
    facing = RD8(OBJ->facing_left[slot]);
    y = (uint8_t)(facing << 1);
    value = rd8((uint16_t)(0xBC6C + y));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xBC6B + y));
    WR8(OBJ->vx_lo[slot], value);
    WR8(OBJ->vy_hi[slot], 0xFC);
    WR8(OBJ->vy_lo[slot], 0xC0);
    r.end = then(ended_with_y(ended_with_carry(ended_load(0xC0), facing >= 0x80), y), ended_nz(INC8(OBJ->f2800[slot])));
    return r;
}

void abi_SoundThenLeapByFacing(void)
{
    abi_sound_end(SoundThenLeapByFacing(cpu.x, cpu.y));
}

/*
 * @routine 0B4C:5BB MaybeSound19AndSpawnPiece
 * @run 70:BDBB
 * WHAT IT DOES: When the low two bits of A are 0 plays sound $19 and starts an object of type $9A in a free slot of $30 to $3F at the object's place at Y low $BE with OBJ_34B8 1 and OBJ_3510 a random number to 7 plus the facing.
 * HOW: An AND, SoundCommand, FindFreeSlotFrom30, StartObjectAtPlace, NextRandom.
 * WAITS: no.
 * Entry: A the number, X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
SoundEnd MaybeSound19AndSpawnPiece(uint8_t slot, uint8_t number, uint8_t y_entry)
{
    uint8_t bits, found_slot, last, placed, pick, facing;
    bool found;
    RandomResult random;
    Sum sum;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    bits = number & 0x03;
    if (bits != 0) {
        r.end = ended_load(bits);
        return r;
    }
    r.sounded = true;
    r.command = 0x19;
    r.before = then(ended_load(bits), ended_load(0x19));
    jsr_push(0xBDC4);
    r.call = SoundCommand(0x19, slot, y_entry);
    jsr_pop();
    jsr_push(0xBDC7);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = ended_of_find_slot(found_slot, last, found);
    jsr_push(0xBDCE);
    placed = StartObjectAtPlace(0x9A, slot, found_slot);
    jsr_pop();
    e = then(e, ended_load(placed));
    WR8(OBJ->y_lo[found_slot], 0xBE);
    WR8(OBJ->f34b8[found_slot], 1);
    e = then(e, ended_load(1));
    jsr_push(0xBDDB);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = random.value & 0x07;
    e = then(e, ended_load(pick));
    facing = RD8(OBJ->facing_left[slot]);
    sum = add_bytes(pick, facing, false);
    WR8(OBJ->f3510[found_slot], sum.value);
    r.end = then(e, ended_of_sum(sum));
    return r;
}

void abi_MaybeSound19AndSpawnPiece(void)
{
    abi_sound_end(MaybeSound19AndSpawnPiece(cpu.x, cpu.a, cpu.y));
}
