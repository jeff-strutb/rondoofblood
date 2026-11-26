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

/* The arms the converted routines end on (ObjArm, enemy_objects.h). */
enum { JA_START, JA_RISING, JA_ABOVE, JA_LANDED };
enum { SC_CLEARED, SC_MOVED, SC_STARTED };
enum { CS_OTHER_STATE, CS_COUNTING, CS_NO_SLOT, CS_SPAWNED };
enum { S5_CLEAR, S5_ABOVE, S5_SET };

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
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->f34b8[slot], 0x48);
    r.first = INC8(OBJ->f2800[slot]);
    return r;
}

void abi_StepListB404ThenTimer(void)
{
    abi_ended(step_then_load_inc_shim(StepListB404ThenTimer(cpu.x, cpu.y), 0x48));
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
    bool carry;

    jsr_push(0xB211);
    r.first = LoadObjectRecord(slot, &r.second, &carry);
    jsr_pop();
    r.flag = carry;
    WR8(OBJ->sprite_flip[slot], 0x80);
    WR8(OBJ->vy_hi[slot], 0xFE);
    WR8(OBJ->vy_lo[slot], 0x80);
    WR8(OBJ->f3720[slot], 0xE0);
    return r;
}

void abi_LoadThenStartRise(void)
{
    abi_ended(record_then_load_shim(LoadThenStartRise(cpu.x), 0xE0));
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
    return r;
}

/* The ORA's A; when zero, the loop's last INY and CPY #$4F (Y $4F, Z, C), LDA #$60 and the INC. */
static Ended clear_slots_when_scroll_zero_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.first != 0) {
        return e;
    }
    e = ended_then(e, ended_with_y(ended_with_carry(ended_nz(0), true), 0x4F));
    return ended_then(e, ended_then(ended_load(0x60), ended_nz(r.second)));
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
    r.second = DEC8(OBJ->f34b8[slot]);
    if (r.second != 0) {
        return r;
    }
    WR8(OBJ->sprite_flip[slot], 0);
    WR8(OBJ->f3510[slot], 0x40);
    r.first = INC8(OBJ->f2800[slot]);
    return r;
}

/* The step, the move, the DEC's N and Z; at zero LDA #$40 and the INC. */
static Ended step_list_b36a_then_count_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_then(frame_step_exit_shim(r), ended_of_axis_move(r.move)), ended_nz(r.second));

    return r.second != 0 ? e : ended_then(ended_then(e, ended_load(0x40)), ended_nz(r.first));
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
    uint8_t index, kind;

    index = RD8(OBJ->f3618[slot]);
    kind = rd8((uint16_t)(0xB56B + index));
    push8(kind);
    jsr_push(0xB55B);
    ClearObjectInY(0x16);
    jsr_pop();
    kind = pull8();
    jsr_push(0xB55F);
    StartObjectAtPlace(kind, slot, 0x16);
    jsr_pop();
    INC8(OBJ->f3618[slot]);
    WR8(OBJ->f34b8[slot], 0x40);
    r.first = INC8(OBJ->f2800[slot]);
    return r;
}

/* LDA #$40 (Y $16 from the LDY before the start), the INC's N and Z. */
static Ended start_kind_from_table_at_slot16_exit_shim(ObjArm r)
{
    return ended_then(ended_with_y(ended_load(0x40), 0x16), ended_nz(r.first));
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
    r.second = DEC8(OBJ->f34b8[slot]);
    if (r.second != 0) {
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
    r.first = INC8(OBJ->f2800[slot]);
    return r;
}

/* LDA #1, the DEC's N and Z; at zero LDA #$E0, the move, LDA #3 and the INC. */
static Ended count_then_jump_right_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_load(1), ended_nz(r.second));

    if (r.second != 0) {
        return e;
    }
    e = ended_then(ended_then(e, ended_load(0xE0)), ended_of_axis_move(r.move));
    return ended_then(ended_then(e, ended_load(3)), ended_nz(r.first));
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
    return r;
}

/* The step; then ApplyVelocityY's exit, or LDA #$30 and the INC when the list ended. */
static Ended step_list_b2a7_then_place_exit_shim(ObjArm r)
{
    if (r.step != FRAME_LIST_ENDED) {
        return ended_then(frame_step_exit_shim(r), ended_of_axis_move(r.move));
    }
    return step_then_load_inc_shim(r, 0x30);
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

    r.second = DEC8(OBJ->f34b8[slot]);
    if (r.second != 0) {
        return r;
    }
    WR8(OBJ->sprite_set[slot], 4);
    WR8(OBJ->flash_flags[slot], 0x10);
    WR8(OBJ->f34b8[slot], 0x40);
    WR8(OBJ->vy_hi[slot], 0xFF);
    WR8(OBJ->vy_lo[slot], 0xC0);
    r.first = INC8(OBJ->f2800[slot]);
    return r;
}

/* The DEC's N and Z; at zero LDA #$C0 and the INC. */
static Ended count_then_rise_with_flash_exit_shim(ObjArm r)
{
    return r.second != 0 ? ended_nz(r.second) : ended_then(ended_load(0xC0), ended_nz(r.first));
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
    bool carry;

    jsr_push(0xB785);
    r.first = LoadObjectRecord(slot, &r.second, &carry);
    jsr_pop();
    r.flag = carry;
    WR8(ZP->raster_effect, 0x0B);
    WR8(ZP->raster_arg, 0xBF);
    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->x_lo[slot], 0x10);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0xF6);
    WR8(OBJ->f34b8[slot], 0x40);
    return r;
}

void abi_LoadThenPlaceAtLeft(void)
{
    abi_ended(record_then_load_shim(LoadThenPlaceAtLeft(cpu.x), 0x40));
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
    Sum low;
    uint8_t bits, y, value;
    bool carry = false;

    r.first = RD8(OBJ->f2800[slot]);
    if (r.first == 0) {
        WR8(OBJ->hitbox_on[slot], 0);
        jsr_push(0xB5B3);
        r.side = SideOfPlayer(slot);
        jsr_pop();
        bits = (uint8_t)(r.side.sides & 0x01);
        WR8(OBJ->facing_left[slot], bits);
        y = (uint8_t)(bits << 1);
        WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB61A + y)));
        WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB619 + y)));
        WR8(OBJ->vy_hi[slot], 0xFC);
        WR8(OBJ->vy_lo[slot], 0);
        r.second = INC8(OBJ->f2800[slot]);
        r.third = bits;
        r.y = y;
        r.arm = JA_START;
        return r;
    }
    jsr_push(0xB5D6);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.fourth = RD8(OBJ->vy_lo[slot]);
    low = add_bytes(r.fourth, 0x20, false);
    WR8(OBJ->vy_lo[slot], low.value);
    r.fifth = RD8(OBJ->vy_hi[slot]);
    value = add_bytes(r.fifth, 0, low.carry).value;
    WR8(OBJ->vy_hi[slot], value);
    r.arm = JA_RISING;
    if ((value & 0x80) != 0) {
        return r;
    }
    r.sixth = RD8(OBJ->y_lo[slot]);
    r.arm = JA_ABOVE;
    if (r.sixth < 0x94) {
        return r;
    }
    WR8(OBJ->y_lo[slot], 0x94);
    WR8(OBJ->hitbox_on[slot], 2);
    value = (uint8_t)(RD8(OBJ->facing_left[slot]) ^ 0x01);
    WR8(OBJ->facing_left[slot], value);
    r.seventh = (uint8_t)(RD8(OBJ->f3618[slot]) ^ 0x02);
    WR8(OBJ->f3618[slot], r.seventh);
    value = LSR8_VALUE(OBJ->f3720[slot], carry);
    value = LSR8_VALUE(OBJ->f3720[slot], carry);
    r.eighth = value;
    r.flag = carry;
    WR8(OBJ->state[slot], 2);
    WR8(OBJ->f2800[slot], 0);
    r.arm = JA_LANDED;
    return r;
}

/* The phase's LDA. Start: SideOfPlayer, the AND, the ASL (C 0) and TAY, LDA #$FC, the INC. Later: the move, the two
 * adds of the speed, the CMP of Y low with $94, then LDA #$94, the EOR's A, the last LSR (N, Z, C) and LDA #2. */
static Ended jump_away_then_land_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);
    Sum low;

    if (r.arm == JA_START) {
        e = ended_then(ended_then(e, ended_of_side(r.side)), ended_load(r.third));
        e = ended_then(e, ended_with_carry(ended_with_y(ended_load(r.y), r.y), false));
        return ended_then(ended_then(e, ended_load(0xFC)), ended_nz(r.second));
    }
    low = add_bytes(r.fourth, 0x20, false);
    e = ended_then(ended_then(ended_then(e, ended_of_axis_move(r.move)), ended_of_sum(low)), ended_of_sum(add_bytes(r.fifth, 0, low.carry)));
    if (r.arm == JA_RISING) {
        return e;
    }
    e = ended_then(e, ended_compare(r.sixth, 0x94));
    if (r.arm == JA_ABOVE) {
        return e;
    }
    e = ended_then(ended_then(e, ended_load(0x94)), ended_load(r.seventh));
    return ended_then(ended_then(e, ended_with_carry(ended_nz(r.eighth), r.flag)), ended_load(2));
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
    RandomResult random;
    uint8_t nz, last, y = y_entry, index;
    bool carry;

    r.first = RD8(OBJ->state[slot]);
    if (r.first != 0) {
        WR8(ZP->temp10, 0x63);
        WR8(ZP->temp11, 0xB7);
        jsr_push(0xB75A);
        r.step = StepObjectFrames(slot, &y);
        jsr_pop();
        r.y = y;
        if (r.step == FRAME_LIST_ENDED) {
            ClearObject(slot);
            r.arm = SC_CLEARED;
            return r;
        }
        jsr_push(0xB762);
        r.move = ApplyVelocityY(slot);
        jsr_pop();
        r.arm = SC_MOVED;
        return r;
    }
    r.arm = S16_LS_START;
    jsr_push(0xB6F4);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    (void)last;
    WR8(OBJ->flash_flags[slot], 0x10);
    jsr_push(0xB6FC);
    r.random = NextRandom();
    jsr_pop();
    WR8(OBJ->vx_hi[slot], (uint8_t)((random.value & 0x0F) - 0x08));
    jsr_push(0xB707);
    r.random2 = NextRandom();
    jsr_pop();
    WR8(OBJ->vy_hi[slot], (uint8_t)((random.value & 0x0F) - 0x10));
    jsr_push(0xB712);
    (void)ApplyVelocity(slot);
    jsr_pop();
    jsr_push(0xB715);
    r.random3 = NextRandom();
    jsr_pop();
    index = (uint8_t)((random.value & 0x03) << 1);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB740 + index)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB73F + index)));
    jsr_push(0xB728);
    r.random = NextRandom();
    jsr_pop();
    index = (uint8_t)((r.random.value & 0x03) << 1);
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(0xB748 + index)));
    r.second = rd8((uint16_t)(0xB747 + index));
    WR8(OBJ->vy_lo[slot], r.second);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.y = index;
    r.arm = SC_STARTED;
    return r;
}

/* Later: the state's LDA, the step, and ApplyVelocityY's exit unless the list ended (ClearObject, a tail jump, leaves
 * the step's). State 0: the last NextRandom's exit, the AND and ASL (C 0) with TAY, the last speed byte's LDA; every
 * register and flag the record, the first three randoms, the subtract and the move left is set again by them. */
static Ended load_then_scatter_along_list_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm != SC_STARTED) {
        e = ended_then(ended_load(r.first), frame_step_exit_shim(r));
        return r.arm == SC_CLEARED ? e : ended_then(e, ended_of_axis_move(r.move));
    }
    e = ended_then(ended_of_random(r.random), ended_with_carry(ended_with_y(ended_load((uint8_t)(r.random.value & 0x03)), r.y), false));
    return ended_then(e, ended_load(r.second));
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
    uint8_t found_slot, last;
    bool found;

    r.first = RD8(OBJ->state[slot]);
    r.arm = CS_OTHER_STATE;
    if (r.first != 2) {
        return r;
    }
    r.second = RD8(OBJ->f3880[slot]);
    if (r.second != 0) {
        r.third = DEC8(OBJ->f3880[slot]);
        r.arm = CS_COUNTING;
        return r;
    }
    WR8(OBJ->f3880[slot], 0x22);
    jsr_push(0xB18D);
    found_slot = FindFreeSlotFrom30(&r.fifth, &found);
    jsr_pop();
    r.fourth = found_slot;
    r.fifth = last;
    r.arm = CS_NO_SLOT;
    if (!found) {
        return r;
    }
    jsr_push(0xB194);
    r.sixth = StartObjectAtPlace(0x28, slot, found_slot);
    jsr_pop();
    r.arm = CS_SPAWNED;
    return r;
}

/* The state's LDA and CMP #2; the count's LDA and DEC; LDA #$22, the slot search, LDA #$28 and the start's A with Y the slot. */
static Ended count_then_spawn_in_state2_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_load(r.first), ended_compare(r.first, 2));

    if (r.arm == CS_OTHER_STATE) {
        return e;
    }
    e = ended_then(e, ended_load(r.second));
    if (r.arm == CS_COUNTING) {
        return ended_then(e, ended_nz(r.third));
    }
    e = ended_then(ended_then(e, ended_load(0x22)), ended_of_find_slot(r.fourth, r.fifth, r.arm == CS_SPAWNED));
    return r.arm == CS_NO_SLOT ? e : ended_then(ended_then(e, ended_load(0x28)), ended_with_y(ended_load(r.sixth), r.fourth));
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
    r.arm = S5_CLEAR;
    if (r.first == 0) {
        return r;
    }
    r.second = RD8(OBJ->state[slot]);
    r.arm = S5_ABOVE;
    if (r.second >= 5) {
        return r;
    }
    WR8(OBJ->state[slot], 5);
    WR8(OBJ->f2800[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    r.arm = S5_SET;
    return r;
}

/* The AND's A; the state's LDA and CMP #5; LDA #5. */
static Ended state_five_on_status_bit7_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == S5_CLEAR) {
        return e;
    }
    e = ended_then(ended_then(e, ended_load(r.second)), ended_compare(r.second, 5));
    return r.arm == S5_ABOVE ? e : ended_then(e, ended_load(5));
}

void abi_StateFiveOnStatusBit7(void)
{
    abi_ended(state_five_on_status_bit7_exit_shim(StateFiveOnStatusBit7(cpu.x)));
}
