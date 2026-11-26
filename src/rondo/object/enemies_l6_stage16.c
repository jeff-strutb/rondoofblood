/*
 * TEMPORARY: the stage object routines of lot L6 for the overlays at sectors 16C9 to 16CF (enemy kinds not yet identified), by routine. Bodies return what they
 * ended on (ObjArm, enemy_objects.h); each routine's exit shim builds A, Y and the flags from it and its abi_ wrapper applies them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

/* The arms the routines below end on. */
enum { S16_B4_RUNNING, S16_B4_ENDED };
enum { S16_CZ_BUSY, S16_CZ_CLEARED };
enum { S16_B3_COUNTING, S16_B3_NEXT };
enum { S16_CJ_COUNTING, S16_CJ_JUMPED };
enum { S16_B2_RUNNING, S16_B2_PLACED };
enum { S16_CR_COUNTING, S16_CR_RISEN };
enum { S16_JA_START, S16_JA_AIR, S16_JA_FLIGHT, S16_JA_LANDED };
enum { S16_LS_ENDED, S16_LS_MOVED, S16_LS_START };
enum { S16_CS_NOT2, S16_CS_COUNTING, S16_CS_NO_SLOT, S16_CS_SPAWNED };
enum { S16_S5_NONE, S16_S5_LATE, S16_S5_ENTERED };

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
ObjArm StepListB404ThenTimer(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0x04);
    WR8(ZP->temp11, 0xB4);
    jsr_push(0xB3F9);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = S16_B4_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->f34b8[slot], 0x48);
    r.first = INC8(OBJ->f2800[slot]);
    r.arm = S16_B4_ENDED;
    return r;
}

static Ended step_list_b404_then_timer_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == S16_B4_RUNNING) {
        return e;
    }
    return then(then(e, ended_load(0x48)), ended_nz(r.first));
}

void abi_StepListB404ThenTimer(void)
{
    abi_ended(step_list_b404_then_timer_exit_shim(StepListB404ThenTimer(cpu.x, cpu.y)));
}

/*
 * @routine 16CB:20E LoadThenStartRise
 * @run 70:B20E
 * WHAT IT DOES: Loads the object's record, sets its flip byte (OBJ_2FE8) to $80, its Y speed to $FE80 and OBJ_3720 to $E0.
 * HOW: LoadObjectRecord, stores.
 * WAITS: no.
 * Entry: X object. Exit: A $E0, N set, Z clear; C and Y from the record.
 */
ObjArm LoadThenStartRise(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB211);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    WR8(OBJ->sprite_flip[slot], 0x80);
    WR8(OBJ->vy_hi[slot], 0xFE);
    WR8(OBJ->vy_lo[slot], 0x80);
    WR8(OBJ->f3720[slot], 0xE0);
    return r;
}

static Ended load_then_start_rise_exit_shim(ObjArm r)
{
    return then(record_exit_shim(r), ended_load(0xE0));
}

void abi_LoadThenStartRise(void)
{
    abi_ended(load_then_start_rise_exit_shim(LoadThenStartRise(cpu.x)));
}

/*
 * @routine 16CB:24C ClearSlotsWhenScrollZero
 * @run 70:B24C
 * WHAT IT DOES: When the level's X position high byte and the scroll X low are both zero, clears the objects in slots $11 to $4E, sets OBJ_34B8 to $60 and goes on to the next OBJ_2800.
 * HOW: Two reads, a loop of ClearObjectInY, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the OR, or the new OBJ_2800's N and Z.
 */
ObjArm ClearSlotsWhenScrollZero(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t high, low, y;

    high = rd8(ScrollXHiScreen);
    low = rd8(ScrollXLo);
    r.first = (uint8_t)(high | low);
    r.arm = S16_CZ_BUSY;
    if (r.first != 0) {
        return r;
    }
    for (y = 0x11; y < 0x4F; y++) {
        jsr_push(0xB259);
        ClearObjectInY(y);
        jsr_pop();
    }
    WR8(OBJ->f34b8[slot], 0x60);
    r.second = INC8(OBJ->f2800[slot]);
    r.arm = S16_CZ_CLEARED;
    return r;
}

static Ended clear_slots_when_scroll_zero_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == S16_CZ_BUSY) {
        return e;
    }
    return then(then(e, ended_with_y(ended_with_carry(ended_nz(0), true), 0x4F)), then(ended_load(0x60), ended_nz(r.second)));
}

void abi_ClearSlotsWhenScrollZero(void)
{
    abi_ended(clear_slots_when_scroll_zero_exit_shim(ClearSlotsWhenScrollZero(cpu.x)));
}

/*
 * @routine 16CB:2F6 StepListB36AThenCount
 * @run 70:B2F6
 * WHAT IT DOES: Steps the animation list at $B36A and moves the object along Y; counts OBJ_34B8 down and at zero clears OBJ_2FE8, sets OBJ_3510 to $40 and goes on to the next OBJ_2800.
 * HOW: StepObjectFrames, ApplyVelocityY, DEC, stores, INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the count, or the new OBJ_2800's N and Z (A $40).
 */
ObjArm StepListB36AThenCount(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0x6A);
    WR8(ZP->temp11, 0xB3);
    jsr_push(0xB301);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    jsr_push(0xB304);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.first = DEC8(OBJ->f34b8[slot]);
    r.arm = S16_B3_COUNTING;
    if (r.first != 0) {
        return r;
    }
    WR8(OBJ->sprite_flip[slot], 0);
    WR8(OBJ->f3510[slot], 0x40);
    r.second = INC8(OBJ->f2800[slot]);
    r.arm = S16_B3_NEXT;
    return r;
}

static Ended step_list_b36a_then_count_exit_shim(ObjArm r)
{
    Ended e = then(then(frame_step_exit_shim(r), ended_of_axis_move(r.move)), ended_nz(r.first));

    if (r.arm == S16_B3_COUNTING) {
        return e;
    }
    return then(then(e, ended_load(0x40)), ended_nz(r.second));
}

void abi_StepListB36AThenCount(void)
{
    abi_ended(step_list_b36a_then_count_exit_shim(StepListB36AThenCount(cpu.x, cpu.y)));
}

/*
 * @routine 16CB:54F StartKindFromTableAtSlot16
 * @run 70:B54F
 * WHAT IT DOES: Takes a kind from the table at $B56B by OBJ_3618, clears the object in slot $16, starts an object of that kind there at this object's place, counts OBJ_3618 up, sets OBJ_34B8 to $40 and goes on to the next OBJ_2800.
 * HOW: A table read (bank 70), ClearObjectInY, StartObjectAtPlace, two INCs.
 * WAITS: no.
 * Entry: X object. Exit: A $40, Y $16, N and Z from the new OBJ_2800.
 */
ObjArm StartKindFromTableAtSlot16(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t kind;

    r.first = RD8(OBJ->f3618[slot]);
    r.second = rd8((uint16_t)(0xB56B + r.first));
    push8(r.second);
    jsr_push(0xB55B);
    ClearObjectInY(0x16);
    jsr_pop();
    kind = pull8();
    r.fourth = kind;
    jsr_push(0xB55F);
    StartObjectAtPlace(kind, slot, 0x16);
    jsr_pop();
    INC8(OBJ->f3618[slot]);
    WR8(OBJ->f34b8[slot], 0x40);
    r.third = INC8(OBJ->f2800[slot]);
    return r;
}

static Ended start_kind_from_table_at_slot16_exit_shim(ObjArm r)
{
    Ended e = ended_with_y(ended_load(r.second), r.first);

    e = then(e, ended_with_y(ended_load(r.fourth), 0x16));
    return then(then(e, ended_load(0x40)), ended_nz(r.third));
}

void abi_StartKindFromTableAtSlot16(void)
{
    abi_ended(start_kind_from_table_at_slot16_exit_shim(StartKindFromTableAtSlot16(cpu.x)));
}

/*
 * @routine 16CB:3C8 CountThenJumpRight
 * @run 70:B3C8
 * WHAT IT DOES: Sets the object's frame to 1 and counts OBJ_34B8 down; at zero it gives the object an X speed high of $12 and a Y speed high of $E0, moves it once, clears its animation step and timer, sets frame 3 and goes on to the next OBJ_2800.
 * HOW: DEC, stores, ApplyVelocity, INC.
 * WAITS: no.
 * Entry: X object. Exit: the count, or the new OBJ_2800's N and Z (A 3).
 */
ObjArm CountThenJumpRight(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(OBJ->frame[slot], 1);
    r.first = DEC8(OBJ->f34b8[slot]);
    r.arm = S16_CJ_COUNTING;
    if (r.first != 0) {
        return r;
    }
    WR8(OBJ->vx_hi[slot], 0x12);
    WR8(OBJ->vy_hi[slot], 0xE0);
    jsr_push(0xB3DF);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->frame[slot], 3);
    r.second = INC8(OBJ->f2800[slot]);
    r.arm = S16_CJ_JUMPED;
    return r;
}

static Ended count_then_jump_right_exit_shim(ObjArm r)
{
    Ended e = then(ended_load(1), ended_nz(r.first));

    if (r.arm == S16_CJ_COUNTING) {
        return e;
    }
    e = then(then(e, ended_load(0xE0)), ended_of_axis_move(r.move));
    return then(then(e, ended_load(3)), ended_nz(r.second));
}

void abi_CountThenJumpRight(void)
{
    abi_ended(count_then_jump_right_exit_shim(CountThenJumpRight(cpu.x)));
}

/*
 * @routine 16CB:27E StepListB2A7ThenPlace
 * @run 70:B27E
 * WHAT IT DOES: Steps the animation list at $B2A7; while it runs the object moves along Y; when it ends the object is put at X $007C, Y $0026 relative to its screen, gets frame 0, OBJ_34B8 $30 and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, a tail jump into ApplyVelocityY, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as ApplyVelocityY, or the new OBJ_2800's N and Z (A $30).
 */
ObjArm StepListB2A7ThenPlace(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0xA7);
    WR8(ZP->temp11, 0xB2);
    jsr_push(0xB289);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = S16_B2_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        r.move = ApplyVelocityY(slot);
        return r;
    }
    WR8(OBJ->x_lo[slot], 0x7C);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0x26);
    WR8(OBJ->frame[slot], 0);
    WR8(OBJ->f34b8[slot], 0x30);
    r.first = INC8(OBJ->f2800[slot]);
    r.arm = S16_B2_PLACED;
    return r;
}

static Ended step_list_b2a7_then_place_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == S16_B2_RUNNING) {
        return then(e, ended_of_axis_move(r.move));
    }
    return then(then(e, ended_load(0x30)), ended_nz(r.first));
}

void abi_StepListB2A7ThenPlace(void)
{
    abi_ended(step_list_b2a7_then_place_exit_shim(StepListB2A7ThenPlace(cpu.x, cpu.y)));
}

/*
 * @routine 16CB:2D4 CountThenRiseWithFlash
 * @run 70:B2D4
 * WHAT IT DOES: Counts OBJ_34B8 down; at zero it sets sprite set 4, flash flags $10, OBJ_34B8 $40 and a Y speed of $FFC0 and goes on to the next OBJ_2800.
 * HOW: DEC, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the count, or the new OBJ_2800's N and Z (A $C0).
 */
ObjArm CountThenRiseWithFlash(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = DEC8(OBJ->f34b8[slot]);
    r.arm = S16_CR_COUNTING;
    if (r.first != 0) {
        return r;
    }
    WR8(OBJ->sprite_set[slot], 4);
    WR8(OBJ->flash_flags[slot], 0x10);
    WR8(OBJ->f34b8[slot], 0x40);
    WR8(OBJ->vy_hi[slot], 0xFF);
    WR8(OBJ->vy_lo[slot], 0xC0);
    r.second = INC8(OBJ->f2800[slot]);
    r.arm = S16_CR_RISEN;
    return r;
}

static Ended count_then_rise_with_flash_exit_shim(ObjArm r)
{
    if (r.arm == S16_CR_COUNTING) {
        return ended_nz(r.first);
    }
    return then(ended_load(0xC0), ended_nz(r.second));
}

void abi_CountThenRiseWithFlash(void)
{
    abi_ended(count_then_rise_with_flash_exit_shim(CountThenRiseWithFlash(cpu.x)));
}

/*
 * @routine 16CB:782 LoadThenPlaceAtLeft
 * @run 70:B782
 * WHAT IT DOES: Loads the object's record, sets the raster effect to $0B with argument $BF, puts the object at X $0010, Y $00F6 and sets OBJ_34B8 to $40.
 * HOW: LoadObjectRecord, stores.
 * WAITS: no.
 * Entry: X object. Exit: A $40.
 */
ObjArm LoadThenPlaceAtLeft(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB785);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    WR8(ZP->raster_effect, 0x0B);
    WR8(ZP->raster_arg, 0xBF);
    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->x_lo[slot], 0x10);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0xF6);
    WR8(OBJ->f34b8[slot], 0x40);
    return r;
}

static Ended load_then_place_at_left_exit_shim(ObjArm r)
{
    return then(record_exit_shim(r), ended_load(0x40));
}

void abi_LoadThenPlaceAtLeft(void)
{
    abi_ended(load_then_place_at_left_exit_shim(LoadThenPlaceAtLeft(cpu.x)));
}

/*
 * @routine 16CB:5A8 JumpAwayThenLand
 * @run 70:B5A8
 * WHAT IT DOES: OBJ_2800 0: clears OBJ_3250, faces the player's side (bit 0 of SideOfPlayer), takes its X speed from the table at $B619 by that side and gets a Y speed of $FC00. Later it moves, adds $0020 to its Y speed, and once it falls (Y speed high not negative) to a Y low of $94 or more it stops at $94, sets OBJ_3250 to 2, turns round, flips bit 1 of OBJ_3618, halves OBJ_3720 twice and goes to state 2 with OBJ_2800 0.
 * HOW: SideOfPlayer, table reads (bank 70), ApplyVelocity, a 16-bit add, stores.
 * WAITS: no.
 * Entry: X object. Exit: the last test or store.
 */
ObjArm JumpAwayThenLand(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t value;
    bool carry = false;

    r.first = RD8(OBJ->f2800[slot]);
    if (r.first == 0) {
        r.arm = S16_JA_START;
        WR8(OBJ->hitbox_on[slot], 0);
        jsr_push(0xB5B3);
        r.side = SideOfPlayer(slot);
        jsr_pop();
        r.second = (uint8_t)(r.side.sides & 0x01);
        WR8(OBJ->facing_left[slot], r.second);
        r.third = (uint8_t)(r.second << 1);
        WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB61A + r.third)));
        WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB619 + r.third)));
        WR8(OBJ->vy_hi[slot], 0xFC);
        WR8(OBJ->vy_lo[slot], 0);
        r.fourth = INC8(OBJ->f2800[slot]);
        return r;
    }
    jsr_push(0xB5D6);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.sum = add_bytes(RD8(OBJ->vy_lo[slot]), 0x20, false);
    WR8(OBJ->vy_lo[slot], r.sum.value);
    r.sum2 = add_bytes(RD8(OBJ->vy_hi[slot]), 0, r.sum.carry);
    WR8(OBJ->vy_hi[slot], r.sum2.value);
    r.arm = S16_JA_AIR;
    if ((r.sum2.value & 0x80) != 0) {
        return r;
    }
    r.fifth = RD8(OBJ->y_lo[slot]);
    r.arm = S16_JA_FLIGHT;
    if (r.fifth < 0x94) {
        return r;
    }
    WR8(OBJ->y_lo[slot], 0x94);
    WR8(OBJ->hitbox_on[slot], 2);
    value = (uint8_t)(RD8(OBJ->facing_left[slot]) ^ 0x01);
    WR8(OBJ->facing_left[slot], value);
    r.sixth = (uint8_t)(RD8(OBJ->f3618[slot]) ^ 0x02);
    WR8(OBJ->f3618[slot], r.sixth);
    value = LSR8_VALUE(OBJ->f3720[slot], carry);
    r.seventh = LSR8_VALUE(OBJ->f3720[slot], carry);
    r.flag = carry;
    WR8(OBJ->state[slot], 2);
    WR8(OBJ->f2800[slot], 0);
    r.arm = S16_JA_LANDED;
    return r;
}

static Ended jump_away_then_land_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == S16_JA_START) {
        e = then(then(e, ended_of_side(r.side)), ended_load(r.second));
        e = then(e, ended_with_carry(ended_with_y(ended_load(r.third), r.third), false));
        return then(then(e, ended_load(0xFC)), ended_nz(r.fourth));
    }
    e = then(then(then(e, ended_of_axis_move(r.move)), ended_of_sum(r.sum)), ended_of_sum(r.sum2));
    if (r.arm == S16_JA_AIR) {
        return e;
    }
    e = then(e, ended_compare(r.fifth, 0x94));
    if (r.arm == S16_JA_FLIGHT) {
        return e;
    }
    e = then(then(e, ended_load(0x94)), ended_load(r.sixth));
    return then(then(e, ended_with_carry(ended_nz(r.seventh), r.flag)), ended_load(2));
}

void abi_JumpAwayThenLand(void)
{
    abi_ended(jump_away_then_land_exit_shim(JumpAwayThenLand(cpu.x)));
}

/*
 * @routine 16CB:6EC LoadThenScatterAlongList
 * @run 70:B6EC
 * WHAT IT DOES: In state 0 loads the object's record, sets flash flags $10, gives it a random X speed high of -8 to 7 and Y speed high of -16 to -1, moves it once, then overwrites the speeds from the two tables at $B73F and $B747 by random rows and clears its animation step and timer. Later it steps the animation list at $B763, clears the object when it ends and otherwise moves it along Y.
 * HOW: LoadObjectRecord, NextRandom four times, ApplyVelocity, table reads (bank 70), StepObjectFrames, ApplyVelocityY, ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the last instruction's registers.
 */
ObjArm LoadThenScatterAlongList(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    Sum sum;
    uint8_t y = y_entry;

    r.tenth = RD8(OBJ->state[slot]);
    if (r.tenth != 0) {
        WR8(ZP->temp10, 0x63);
        WR8(ZP->temp11, 0xB7);
        jsr_push(0xB75A);
        r.step = StepObjectFrames(slot, &y);
        jsr_pop();
        r.y = y;
        r.arm = S16_LS_ENDED;
        if (r.step == FRAME_LIST_ENDED) {
            ClearObject(slot);
            return r;
        }
        jsr_push(0xB762);
        r.move = ApplyVelocityY(slot);
        jsr_pop();
        r.arm = S16_LS_MOVED;
        return r;
    }
    r.arm = S16_LS_START;
    jsr_push(0xB6F4);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    WR8(OBJ->flash_flags[slot], 0x10);
    jsr_push(0xB6FC);
    r.random = NextRandom();
    jsr_pop();
    sum = sub_bytes((uint8_t)(r.random.value & 0x0F), 0x08, true);
    WR8(OBJ->vx_hi[slot], sum.value);
    jsr_push(0xB707);
    r.random2 = NextRandom();
    jsr_pop();
    r.sum = sub_bytes((uint8_t)(r.random2.value & 0x0F), 0x10, true);
    WR8(OBJ->vy_hi[slot], r.sum.value);
    jsr_push(0xB712);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    jsr_push(0xB715);
    r.random3 = NextRandom();
    jsr_pop();
    r.third = (uint8_t)((r.random3.value & 0x03) << 1);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB740 + r.third)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB73F + r.third)));
    jsr_push(0xB728);
    r.random4 = NextRandom();
    jsr_pop();
    r.fourth = (uint8_t)((r.random4.value & 0x03) << 1);
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(0xB748 + r.fourth)));
    r.fifth = rd8((uint16_t)(0xB747 + r.fourth));
    WR8(OBJ->vy_lo[slot], r.fifth);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    return r;
}

static Ended load_then_scatter_along_list_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm != S16_LS_START) {
        e = then(ended_load(r.tenth), frame_step_exit_shim(r));
        if (r.arm == S16_LS_ENDED) {
            return e;
        }
        return then(e, ended_of_axis_move(r.move));
    }
    e = then(ended_load(r.tenth), record_exit_shim(r));
    e = then(then(e, ended_load(0x10)), ended_of_random(r.random));
    e = then(then(e, ended_of_random(r.random2)), ended_of_sum(r.sum));
    e = then(e, ended_of_axis_move(r.move));
    e = then(then(e, ended_of_random(r.random3)), ended_with_carry(ended_with_y(ended_load((uint8_t)(r.random3.value & 0x03)), r.third), false));
    e = then(then(e, ended_of_random(r.random4)), ended_with_carry(ended_with_y(ended_load((uint8_t)(r.random4.value & 0x03)), r.fourth), false));
    return then(e, ended_load(r.fifth));
}

void abi_LoadThenScatterAlongList(void)
{
    abi_ended(load_then_scatter_along_list_exit_shim(LoadThenScatterAlongList(cpu.x, cpu.y)));
}

/*
 * @routine 16CB:175 CountThenSpawnInState2
 * @run 70:B175
 * WHAT IT DOES: In state 2 counts OBJ_3880 down; at zero it sets it to $22 and starts a kind $28 object in a free slot from $30 at this object's place.
 * HOW: DEC, FindFreeSlotFrom30, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: the last compare, the count, or the start's registers.
 */
ObjArm CountThenSpawnInState2(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t found_slot;
    bool found;

    r.first = RD8(OBJ->state[slot]);
    r.arm = S16_CS_NOT2;
    if (r.first != 2) {
        return r;
    }
    r.second = RD8(OBJ->f3880[slot]);
    r.arm = S16_CS_COUNTING;
    if (r.second != 0) {
        r.third = DEC8(OBJ->f3880[slot]);
        return r;
    }
    WR8(OBJ->f3880[slot], 0x22);
    jsr_push(0xB18D);
    found_slot = FindFreeSlotFrom30(&r.fifth, &found);
    jsr_pop();
    r.fourth = found_slot;
    r.flag = found;
    r.arm = S16_CS_NO_SLOT;
    if (!found) {
        return r;
    }
    jsr_push(0xB194);
    r.sixth = StartObjectAtPlace(0x28, slot, found_slot);
    jsr_pop();
    r.arm = S16_CS_SPAWNED;
    return r;
}

static Ended count_then_spawn_in_state2_exit_shim(ObjArm r)
{
    Ended e = then(ended_load(r.first), ended_compare(r.first, 2));

    if (r.arm == S16_CS_NOT2) {
        return e;
    }
    e = then(e, ended_load(r.second));
    if (r.arm == S16_CS_COUNTING) {
        return then(e, ended_nz(r.third));
    }
    e = then(e, ended_load(0x22));
    e = then(e, ended_of_find_slot(r.fourth, r.fifth, r.flag));
    if (r.arm == S16_CS_NO_SLOT) {
        return e;
    }
    return then(then(e, ended_load(0x28)), ended_with_y(ended_load(r.sixth), r.fourth));
}

void abi_CountThenSpawnInState2(void)
{
    abi_ended(count_then_spawn_in_state2_exit_shim(CountThenSpawnInState2(cpu.x)));
}

/*
 * @routine 16CB:1F4 StateFiveOnStatusBit7
 * @run 70:B1F4
 * WHAT IT DOES: An object in a state under 5 whose OBJ_31A0 has bit 7 set goes to state 5, clears OBJ_2800 and OBJ_3250. (The part after the test ran in no script; it is written from the bytes.)
 * HOW: A test, a compare, three stores.
 * WAITS: no.
 * Entry: X object. Exit: the test, the compare, or A 5.
 */
ObjArm StateFiveOnStatusBit7(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = (uint8_t)(RD8(OBJ->status_flags[slot]) & 0x80);
    r.arm = S16_S5_NONE;
    if (r.first == 0) {
        return r;
    }
    r.second = RD8(OBJ->state[slot]);
    r.arm = S16_S5_LATE;
    if (r.second >= 5) {
        return r;
    }
    WR8(OBJ->state[slot], 5);
    WR8(OBJ->f2800[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    r.arm = S16_S5_ENTERED;
    return r;
}

static Ended state_five_on_status_bit7_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == S16_S5_NONE) {
        return e;
    }
    e = then(then(e, ended_load(r.second)), ended_compare(r.second, 5));
    if (r.arm == S16_S5_LATE) {
        return e;
    }
    return then(e, ended_load(5));
}

void abi_StateFiveOnStatusBit7(void)
{
    abi_ended(state_five_on_status_bit7_exit_shim(StateFiveOnStatusBit7(cpu.x)));
}
