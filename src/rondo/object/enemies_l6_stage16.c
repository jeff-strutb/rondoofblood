/*
 * TEMPORARY: the stage object routines of lot L6 for the overlays at sectors 16C9 to 16CF (enemy kinds not yet identified), by routine. Bodies return how the
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

/*
 * @routine 16CB:3EE StepListB404ThenTimer
 * @run 70:B3EE
 * WHAT IT DOES: Steps the animation list at $B404; when it ends, OBJ_34B8 becomes $48 and the object goes to the next OBJ_2800.
 * HOW: StepObjectFrames, a store, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new OBJ_2800's N and Z (A $48) when the list ended.
 */
Ended StepListB404ThenTimer(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    uint8_t y = y_entry, phase;
    Ended e;

    WR8(ZP->temp10, 0x04);
    WR8(ZP->temp11, 0xB4);
    jsr_push(0xB3F9);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->f34b8[slot], 0x48);
    phase = INC8(OBJ->f2800[slot]);
    return then(then(e, ended_load(0x48)), ended_nz(phase));
}

void abi_StepListB404ThenTimer(void)
{
    abi_ended(StepListB404ThenTimer(cpu.x, cpu.y));
}

/*
 * @routine 16CB:20E LoadThenStartRise
 * @run 70:B20E
 * WHAT IT DOES: Loads the object's record, sets its flip byte (OBJ_2FE8) to $80, its Y speed to $FE80 and OBJ_3720 to $E0.
 * HOW: LoadObjectRecord, stores.
 * WAITS: no.
 * Entry: X object. Exit: A $E0, N set, Z clear; C and Y from the record.
 */
Ended LoadThenStartRise(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;
    Ended e;

    jsr_push(0xB211);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->sprite_flip[slot], 0x80);
    WR8(OBJ->vy_hi[slot], 0xFE);
    WR8(OBJ->vy_lo[slot], 0x80);
    WR8(OBJ->f3720[slot], 0xE0);
    return then(e, ended_load(0xE0));
}

void abi_LoadThenStartRise(void)
{
    abi_ended(LoadThenStartRise(cpu.x));
}

/*
 * @routine 16CB:24C ClearSlotsWhenScrollZero
 * @run 70:B24C
 * WHAT IT DOES: When the level's X position high byte and the scroll X low are both zero, clears the objects in slots $11 to $4E, sets OBJ_34B8 to $60 and goes on to the next OBJ_2800.
 * HOW: Two reads, a loop of ClearObjectInY, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the OR, or the new OBJ_2800's N and Z.
 */
Ended ClearSlotsWhenScrollZero(uint8_t slot)
{
    uint8_t high, low, y, phase;
    Ended e;

    high = rd8(ScrollXHiScreen);
    low = rd8(ScrollXLo);
    e = ended_load((uint8_t)(high | low));
    if ((high | low) != 0) {
        return e;
    }
    for (y = 0x11; y < 0x4F; y++) {
        jsr_push(0xB259);
        ClearObjectInY(y);
        jsr_pop();
    }
    WR8(OBJ->f34b8[slot], 0x60);
    phase = INC8(OBJ->f2800[slot]);
    return then(then(e, ended_with_y(ended_with_carry(ended_nz(0), true), 0x4F)), then(ended_load(0x60), ended_nz(phase)));
}

void abi_ClearSlotsWhenScrollZero(void)
{
    abi_ended(ClearSlotsWhenScrollZero(cpu.x));
}

/*
 * @routine 16CB:2F6 StepListB36AThenCount
 * @run 70:B2F6
 * WHAT IT DOES: Steps the animation list at $B36A and moves the object along Y; counts OBJ_34B8 down and at zero clears OBJ_2FE8, sets OBJ_3510 to $40 and goes on to the next OBJ_2800.
 * HOW: StepObjectFrames, ApplyVelocityY, DEC, stores, INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the count, or the new OBJ_2800's N and Z (A $40).
 */
Ended StepListB36AThenCount(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    AxisMove move;
    uint8_t y = y_entry, left, phase;
    Ended e;

    WR8(ZP->temp10, 0x6A);
    WR8(ZP->temp11, 0xB3);
    jsr_push(0xB301);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    jsr_push(0xB304);
    move = ApplyVelocityY(slot);
    jsr_pop();
    left = DEC8(OBJ->f34b8[slot]);
    e = then(then(e, ended_of_axis_move(move)), ended_nz(left));
    if (left != 0) {
        return e;
    }
    WR8(OBJ->sprite_flip[slot], 0);
    WR8(OBJ->f3510[slot], 0x40);
    phase = INC8(OBJ->f2800[slot]);
    return then(then(e, ended_load(0x40)), ended_nz(phase));
}

void abi_StepListB36AThenCount(void)
{
    abi_ended(StepListB36AThenCount(cpu.x, cpu.y));
}

/*
 * @routine 16CB:54F StartKindFromTableAtSlot16
 * @run 70:B54F
 * WHAT IT DOES: Takes a kind from the table at $B56B by OBJ_3618, clears the object in slot $16, starts an object of that kind there at this object's place, counts OBJ_3618 up, sets OBJ_34B8 to $40 and goes on to the next OBJ_2800.
 * HOW: A table read (bank 70), ClearObjectInY, StartObjectAtPlace, two INCs.
 * WAITS: no.
 * Entry: X object. Exit: A $40, Y $16, N and Z from the new OBJ_2800.
 */
Ended StartKindFromTableAtSlot16(uint8_t slot)
{
    uint8_t index, kind, phase;
    Ended e;

    index = RD8(OBJ->f3618[slot]);
    kind = rd8((uint16_t)(0xB56B + index));
    e = ended_with_y(ended_load(kind), index);
    push8(kind);
    jsr_push(0xB55B);
    ClearObjectInY(0x16);
    jsr_pop();
    kind = pull8();
    e = then(e, ended_with_y(ended_load(kind), 0x16));
    jsr_push(0xB55F);
    StartObjectAtPlace(kind, slot, 0x16);
    jsr_pop();
    INC8(OBJ->f3618[slot]);
    WR8(OBJ->f34b8[slot], 0x40);
    phase = INC8(OBJ->f2800[slot]);
    return then(then(e, ended_load(0x40)), ended_nz(phase));
}

void abi_StartKindFromTableAtSlot16(void)
{
    abi_ended(StartKindFromTableAtSlot16(cpu.x));
}

/*
 * @routine 16CB:3C8 CountThenJumpRight
 * @run 70:B3C8
 * WHAT IT DOES: Sets the object's frame to 1 and counts OBJ_34B8 down; at zero it gives the object an X speed high of $12 and a Y speed high of $E0, moves it once, clears its animation step and timer, sets frame 3 and goes on to the next OBJ_2800.
 * HOW: DEC, stores, ApplyVelocity, INC.
 * WAITS: no.
 * Entry: X object. Exit: the count, or the new OBJ_2800's N and Z (A 3).
 */
Ended CountThenJumpRight(uint8_t slot)
{
    AxisMove move;
    uint8_t left, phase;
    Ended e;

    WR8(OBJ->frame[slot], 1);
    left = DEC8(OBJ->f34b8[slot]);
    e = then(ended_load(1), ended_nz(left));
    if (left != 0) {
        return e;
    }
    WR8(OBJ->vx_hi[slot], 0x12);
    WR8(OBJ->vy_hi[slot], 0xE0);
    e = then(e, ended_load(0xE0));
    jsr_push(0xB3DF);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->frame[slot], 3);
    phase = INC8(OBJ->f2800[slot]);
    return then(then(e, ended_load(3)), ended_nz(phase));
}

void abi_CountThenJumpRight(void)
{
    abi_ended(CountThenJumpRight(cpu.x));
}

/*
 * @routine 16CB:27E StepListB2A7ThenPlace
 * @run 70:B27E
 * WHAT IT DOES: Steps the animation list at $B2A7; while it runs the object moves along Y; when it ends the object is put at X $007C, Y $0026 relative to its screen, gets frame 0, OBJ_34B8 $30 and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, a tail jump into ApplyVelocityY, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as ApplyVelocityY, or the new OBJ_2800's N and Z (A $30).
 */
Ended StepListB2A7ThenPlace(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    AxisMove move;
    uint8_t y = y_entry, phase;
    Ended e;

    WR8(ZP->temp10, 0xA7);
    WR8(ZP->temp11, 0xB2);
    jsr_push(0xB289);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        move = ApplyVelocityY(slot);
        return then(e, ended_of_axis_move(move));
    }
    WR8(OBJ->x_lo[slot], 0x7C);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0x26);
    WR8(OBJ->frame[slot], 0);
    WR8(OBJ->f34b8[slot], 0x30);
    phase = INC8(OBJ->f2800[slot]);
    return then(then(e, ended_load(0x30)), ended_nz(phase));
}

void abi_StepListB2A7ThenPlace(void)
{
    abi_ended(StepListB2A7ThenPlace(cpu.x, cpu.y));
}

/*
 * @routine 16CB:2D4 CountThenRiseWithFlash
 * @run 70:B2D4
 * WHAT IT DOES: Counts OBJ_34B8 down; at zero it sets sprite set 4, flash flags $10, OBJ_34B8 $40 and a Y speed of $FFC0 and goes on to the next OBJ_2800.
 * HOW: DEC, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the count, or the new OBJ_2800's N and Z (A $C0).
 */
Ended CountThenRiseWithFlash(uint8_t slot)
{
    uint8_t left, phase;

    left = DEC8(OBJ->f34b8[slot]);
    if (left != 0) {
        return ended_nz(left);
    }
    WR8(OBJ->sprite_set[slot], 4);
    WR8(OBJ->flash_flags[slot], 0x10);
    WR8(OBJ->f34b8[slot], 0x40);
    WR8(OBJ->vy_hi[slot], 0xFF);
    WR8(OBJ->vy_lo[slot], 0xC0);
    phase = INC8(OBJ->f2800[slot]);
    return then(ended_load(0xC0), ended_nz(phase));
}

void abi_CountThenRiseWithFlash(void)
{
    abi_ended(CountThenRiseWithFlash(cpu.x));
}

/*
 * @routine 16CB:782 LoadThenPlaceAtLeft
 * @run 70:B782
 * WHAT IT DOES: Loads the object's record, sets the raster effect to $0B with argument $BF, puts the object at X $0010, Y $00F6 and sets OBJ_34B8 to $40.
 * HOW: LoadObjectRecord, stores.
 * WAITS: no.
 * Entry: X object. Exit: A $40.
 */
Ended LoadThenPlaceAtLeft(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;
    Ended e;

    jsr_push(0xB785);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(ZP->raster_effect, 0x0B);
    WR8(ZP->raster_arg, 0xBF);
    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->x_lo[slot], 0x10);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0xF6);
    WR8(OBJ->f34b8[slot], 0x40);
    return then(e, ended_load(0x40));
}

void abi_LoadThenPlaceAtLeft(void)
{
    abi_ended(LoadThenPlaceAtLeft(cpu.x));
}

/*
 * @routine 16CB:5A8 JumpAwayThenLand
 * @run 70:B5A8
 * WHAT IT DOES: OBJ_2800 0: clears OBJ_3250, faces the player's side (bit 0 of SideOfPlayer), takes its X speed from the table at $B619 by that side and gets a Y speed of $FC00. Later it moves, adds $0020 to its Y speed, and once it falls (Y speed high not negative) to a Y low of $94 or more it stops at $94, sets OBJ_3250 to 2, turns round, flips bit 1 of OBJ_3618, halves OBJ_3720 twice and goes to state 2 with OBJ_2800 0.
 * HOW: SideOfPlayer, table reads (bank 70), ApplyVelocity, a 16-bit add, stores.
 * WAITS: no.
 * Entry: X object. Exit: the last test or store.
 */
Ended JumpAwayThenLand(uint8_t slot)
{
    SideResult side;
    AxisMove move;
    Sum low, high;
    uint8_t phase, bits, y, position, value;
    bool carry = false;
    Ended e;

    phase = RD8(OBJ->f2800[slot]);
    e = ended_load(phase);
    if (phase == 0) {
        WR8(OBJ->hitbox_on[slot], 0);
        jsr_push(0xB5B3);
        side = SideOfPlayer(slot);
        jsr_pop();
        bits = (uint8_t)(side.sides & 0x01);
        WR8(OBJ->facing_left[slot], bits);
        y = (uint8_t)(bits << 1);
        WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB61A + y)));
        WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB619 + y)));
        WR8(OBJ->vy_hi[slot], 0xFC);
        WR8(OBJ->vy_lo[slot], 0);
        value = INC8(OBJ->f2800[slot]);
        e = then(then(e, ended_of_side(side)), ended_load(bits));
        e = then(e, ended_with_carry(ended_with_y(ended_load(y), y), false));
        return then(then(e, ended_load(0xFC)), ended_nz(value));
    }
    jsr_push(0xB5D6);
    move = ApplyVelocity(slot);
    jsr_pop();
    low = add_bytes(RD8(OBJ->vy_lo[slot]), 0x20, false);
    WR8(OBJ->vy_lo[slot], low.value);
    high = add_bytes(RD8(OBJ->vy_hi[slot]), 0, low.carry);
    WR8(OBJ->vy_hi[slot], high.value);
    e = then(then(then(e, ended_of_axis_move(move)), ended_of_sum(low)), ended_of_sum(high));
    if ((high.value & 0x80) != 0) {
        return e;
    }
    position = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(position, 0x94));
    if (position < 0x94) {
        return e;
    }
    WR8(OBJ->y_lo[slot], 0x94);
    WR8(OBJ->hitbox_on[slot], 2);
    value = (uint8_t)(RD8(OBJ->facing_left[slot]) ^ 0x01);
    WR8(OBJ->facing_left[slot], value);
    value = (uint8_t)(RD8(OBJ->f3618[slot]) ^ 0x02);
    WR8(OBJ->f3618[slot], value);
    e = then(then(e, ended_load(0x94)), ended_load(value));
    value = LSR8_VALUE(OBJ->f3720[slot], carry);
    value = LSR8_VALUE(OBJ->f3720[slot], carry);
    WR8(OBJ->state[slot], 2);
    WR8(OBJ->f2800[slot], 0);
    return then(then(e, ended_with_carry(ended_nz(value), carry)), ended_load(2));
}

void abi_JumpAwayThenLand(void)
{
    abi_ended(JumpAwayThenLand(cpu.x));
}

/*
 * @routine 16CB:6EC LoadThenScatterAlongList
 * @run 70:B6EC
 * WHAT IT DOES: In state 0 loads the object's record, sets flash flags $10, gives it a random X speed high of -8 to 7 and Y speed high of -16 to -1, moves it once, then overwrites the speeds from the two tables at $B73F and $B747 by random rows and clears its animation step and timer. Later it steps the animation list at $B763, clears the object when it ends and otherwise moves it along Y.
 * HOW: LoadObjectRecord, NextRandom four times, ApplyVelocity, table reads (bank 70), StepObjectFrames, ApplyVelocityY, ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the last instruction's registers.
 */
Ended LoadThenScatterAlongList(uint8_t slot, uint8_t y_entry)
{
    RandomResult random;
    FrameStep step;
    AxisMove move;
    Sum sum;
    uint8_t state, nz, last, y = y_entry, index, value;
    bool carry;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        WR8(ZP->temp10, 0x63);
        WR8(ZP->temp11, 0xB7);
        jsr_push(0xB75A);
        step = StepObjectFrames(slot, &y);
        jsr_pop();
        e = then(ended_load(state), ended_of_frame_step(step, step != FRAME_HELD, y));
        if (step == FRAME_LIST_ENDED) {
            ClearObject(slot);
            return e;
        }
        jsr_push(0xB762);
        move = ApplyVelocityY(slot);
        jsr_pop();
        return then(e, ended_of_axis_move(move));
    }
    jsr_push(0xB6F4);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = then(ended_load(state), ended_of_record(last, nz, carry));
    WR8(OBJ->flash_flags[slot], 0x10);
    jsr_push(0xB6FC);
    random = NextRandom();
    jsr_pop();
    sum = sub_bytes((uint8_t)(random.value & 0x0F), 0x08, true);
    WR8(OBJ->vx_hi[slot], sum.value);
    e = then(then(e, ended_load(0x10)), ended_of_random(random));
    jsr_push(0xB707);
    random = NextRandom();
    jsr_pop();
    sum = sub_bytes((uint8_t)(random.value & 0x0F), 0x10, true);
    WR8(OBJ->vy_hi[slot], sum.value);
    e = then(then(e, ended_of_random(random)), ended_of_sum(sum));
    jsr_push(0xB712);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    jsr_push(0xB715);
    random = NextRandom();
    jsr_pop();
    index = (uint8_t)((random.value & 0x03) << 1);
    e = then(then(e, ended_of_random(random)), ended_with_carry(ended_with_y(ended_load((uint8_t)(random.value & 0x03)), index), false));
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB740 + index)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB73F + index)));
    jsr_push(0xB728);
    random = NextRandom();
    jsr_pop();
    index = (uint8_t)((random.value & 0x03) << 1);
    e = then(then(e, ended_of_random(random)), ended_with_carry(ended_with_y(ended_load((uint8_t)(random.value & 0x03)), index), false));
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(0xB748 + index)));
    value = rd8((uint16_t)(0xB747 + index));
    WR8(OBJ->vy_lo[slot], value);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    return then(e, ended_load(value));
}

void abi_LoadThenScatterAlongList(void)
{
    abi_ended(LoadThenScatterAlongList(cpu.x, cpu.y));
}

/*
 * @routine 16CB:175 CountThenSpawnInState2
 * @run 70:B175
 * WHAT IT DOES: In state 2 counts OBJ_3880 down; at zero it sets it to $22 and starts a kind $28 object in a free slot from $30 at this object's place.
 * HOW: DEC, FindFreeSlotFrom30, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: the last compare, the count, or the start's registers.
 */
Ended CountThenSpawnInState2(uint8_t slot)
{
    uint8_t state, count, found_slot, last, placed;
    bool found;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = then(ended_load(state), ended_compare(state, 2));
    if (state != 2) {
        return e;
    }
    count = RD8(OBJ->f3880[slot]);
    e = then(e, ended_load(count));
    if (count != 0) {
        count = DEC8(OBJ->f3880[slot]);
        return then(e, ended_nz(count));
    }
    WR8(OBJ->f3880[slot], 0x22);
    e = then(e, ended_load(0x22));
    jsr_push(0xB18D);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, last, found));
    if (!found) {
        return e;
    }
    jsr_push(0xB194);
    placed = StartObjectAtPlace(0x28, slot, found_slot);
    jsr_pop();
    return then(then(e, ended_load(0x28)), ended_with_y(ended_load(placed), found_slot));
}

void abi_CountThenSpawnInState2(void)
{
    abi_ended(CountThenSpawnInState2(cpu.x));
}

/*
 * @routine 16CB:1F4 StateFiveOnStatusBit7
 * @run 70:B1F4
 * WHAT IT DOES: An object in a state under 5 whose OBJ_31A0 has bit 7 set goes to state 5, clears OBJ_2800 and OBJ_3250. (The part after the test ran in no script; it is written from the bytes.)
 * HOW: A test, a compare, three stores.
 * WAITS: no.
 * Entry: X object. Exit: the test, the compare, or A 5.
 */
Ended StateFiveOnStatusBit7(uint8_t slot)
{
    uint8_t status, state;
    Ended e;

    status = (uint8_t)(RD8(OBJ->status_flags[slot]) & 0x80);
    e = ended_load(status);
    if (status == 0) {
        return e;
    }
    state = RD8(OBJ->state[slot]);
    e = then(then(e, ended_load(state)), ended_compare(state, 5));
    if (state >= 5) {
        return e;
    }
    WR8(OBJ->state[slot], 5);
    WR8(OBJ->f2800[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    return then(e, ended_load(5));
}

void abi_StateFiveOnStatusBit7(void)
{
    abi_ended(StateFiveOnStatusBit7(cpu.x));
}
