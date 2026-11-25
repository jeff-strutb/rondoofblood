/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 05CB (enemy kinds not yet identified), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "front_end.h"
#include "stage_enemies_l8.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/*
 * @routine 05CB:0E3 LoadRecordThenRecordEventAtFifth
 * @run 70:B0E3
 * WHAT IT DOES: In state 0 loads the record and sets sprite set 7, frame $12 and the hit height $14. Later it steps the animation list at $B136; once the object has taken 5 hits it records its event, takes the kind of OBJ_3040, sets OBJ_3A90 $10, goes to state 0 and starts an object of type $9C from a free slot of $50 to $53 with the Y speed -$0600, moved once.
 * HOW: LoadObjectRecord, StepObjectFrames, RecordObjectEvent, FindFreeSlotFrom50, StartObjectAtPlace, ApplyVelocityY with X and Y swapped.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended LoadRecordThenRecordEventAtFifth(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, hits, stored, event, byte, kind, found_slot, last, placed;
    FrameStep step;
    bool marked, found;
    AxisMove move;
    uint8_t first;
    Ended e = l8_load_if_new(slot, 0xB0EB, &first);

    if (first == 0) {
        WR8(OBJ->sprite_set[slot], 7);
        WR8(OBJ->frame[slot], 0x12);
        WR8(OBJ->hitbox_half_h[slot], 0x14);
        return then(e, ended_load(0x14));
    }
    e = l8_step_list(slot, 0xB136, 0xB106, true, &y, &step);
    hits = RD8(OBJ->hit_taken[slot]);
    e = then(e, ended_compare(hits, 5));
    if (hits < 5) {
        return e;
    }
    jsr_push(0xB110);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    jsr_pop();
    e = then(e, l8_of_event(marked, stored, event, byte));
    kind = RD8(OBJ->f3040[slot]);
    e = then(e, ended_load(kind));
    WR8(OBJ->kind[slot], kind);
    WR8(OBJ->timer[slot], 0x10);
    e = then(e, ended_load(0x10));
    WR8(OBJ->state[slot], 0);
    jsr_push(0xB121);
    found_slot = FindFreeSlotFrom50(&last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, last, found));
    if (!found) {
        return e;
    }
    jsr_push(0xB128);
    placed = StartObjectAtPlace(0x9C, slot, found_slot);
    jsr_pop();
    e = then(e, ended_load(placed));
    WR8(OBJ->vy_hi[found_slot], 0xFA);
    e = then(e, ended_load(0xFA));
    jsr_push(0xB131);
    move = ApplyVelocityY(found_slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    WR8(OBJ->vy_hi[found_slot], 0);
    return then(e, ended_with_y(ended_kept(), found_slot));
}

void abi_LoadRecordThenRecordEventAtFifth(void)
{
    abi_ended(LoadRecordThenRecordEventAtFifth(cpu.x, cpu.y));
}

/*
 * @routine 05CB:1A9 FlagThenHitsCompare
 * @run 70:B1A9
 * WHAT IT DOES: Tests bit 7 of OBJ_31A0 and, when it is clear, compares the hits taken with 5; nothing is stored.
 * HOW: An AND and a compare.
 * WAITS: no.
 * Entry: X object. Exit: the AND, or the compare.
 */
Ended FlagThenHitsCompare(uint8_t slot)
{
    uint8_t flags, hits;
    Ended e;

    flags = RD8(OBJ->status_flags[slot]);
    e = ended_load((uint8_t)(flags & 0x80));
    if ((flags & 0x80) != 0) {
        return e;
    }
    hits = RD8(OBJ->hit_taken[slot]);
    return then(e, ended_compare(hits, 5));
}

void abi_FlagThenHitsCompare(void)
{
    abi_ended(FlagThenHitsCompare(cpu.x));
}

/*
 * @routine 05CB:1B8 EnterState5WhenFlagged
 * @run 70:B1B8
 * WHAT IT DOES: When bit 7 of OBJ_31A0 is set and the state is below 5, sets the state 5 and OBJ_2800 0.
 * HOW: Two tests, two stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. The stores ran in no script; the draft stops there.
 */
Ended EnterState5WhenFlagged(uint8_t slot)
{
    uint8_t flags, state;
    Ended e;

    flags = RD8(OBJ->status_flags[slot]);
    e = ended_load((uint8_t)(flags & 0x80));
    if ((flags & 0x80) == 0) {
        return e;
    }
    state = RD8(OBJ->state[slot]);
    e = then(e, ended_compare(state, 5));
    if (state >= 5) {
        return e;
    }
    WR8(OBJ->state[slot], 5);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_load(5));
}

void abi_EnterState5WhenFlagged(void)
{
    abi_ended(EnterState5WhenFlagged(cpu.x));
}

/*
 * @routine 05CB:1ED LoadRecordStartTwoPieces
 * @run 70:B1ED
 * WHAT IT DOES: Loads the object's record, clears slots $11 and $12 and starts an object of type $26 in each at the object's place; OBJ_3510 $40.
 * HOW: LoadObjectRecord, ClearObjectInY and StartObjectAtPlace twice, a store.
 * WAITS: no.
 * Entry: X object. Exit: A $40, Y $12, C from the record.
 */
Ended LoadRecordStartTwoPieces(uint8_t slot)
{
    uint8_t last, nz, placed;
    bool carry;
    Ended e;

    jsr_push(0xB1F0);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    jsr_push(0xB1F5);
    ClearObjectInY(0x11);
    jsr_pop();
    jsr_push(0xB1FA);
    placed = StartObjectAtPlace(0x26, slot, 0x11);
    jsr_pop();
    e = then(then(e, ended_with_y(ended_nz(0x11), 0x11)), ended_load(placed));
    jsr_push(0xB1FE);
    ClearObjectInY(0x12);
    jsr_pop();
    jsr_push(0xB203);
    placed = StartObjectAtPlace(0x26, slot, 0x12);
    jsr_pop();
    e = then(then(e, ended_with_y(ended_nz(0x12), 0x12)), ended_load(placed));
    WR8(OBJ->f3510[slot], 0x40);
    return then(e, ended_load(0x40));
}

void abi_LoadRecordStartTwoPieces(void)
{
    abi_ended(LoadRecordStartTwoPieces(cpu.x));
}

/*
 * @routine 05CB:232 SetUpEntryFromLeft
 * @run 70:B232
 * WHAT IT DOES: Places the object at X $FFF0, Y $0030 with the speed X $00C0 and goes to the next OBJ_2800.
 * HOW: Stores and an INC.
 * WAITS: no.
 * Entry: X object. Exit: A $C0, N and Z from the new OBJ_2800.
 */
Ended SetUpEntryFromLeft(uint8_t slot)
{
    WR8(OBJ->x_hi[slot], 0xFF);
    WR8(OBJ->x_lo[slot], 0xF0);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0x30);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0xC0);
    return then(ended_load(0xC0), ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_SetUpEntryFromLeft(void)
{
    abi_ended(SetUpEntryFromLeft(cpu.x));
}

/*
 * @routine 05CB:250 StepListMoveXThenTurnUp
 * @run 70:B250
 * WHAT IT DOES: Steps the animation list at $B289 and moves the object along X; once it is at X $0120 or more it takes frame 0, the speed X -$0400 and Y $0500, clears the animation and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, ApplyVelocityX, two compares, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListMoveXThenTurnUp(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, high, low;
    FrameStep step;
    AxisMove move;
    Ended e = l8_step_list(slot, 0xB289, 0xB25B, false, &y, &step);

    jsr_push(0xB25E);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    high = RD8(OBJ->x_hi[slot]);
    e = then(e, ended_compare(high, 1));
    if (high != 1) {
        return e;
    }
    low = RD8(OBJ->x_lo[slot]);
    e = then(e, ended_compare(low, 0x20));
    if (low < 0x20) {
        return e;
    }
    WR8(OBJ->frame[slot], 0);
    WR8(OBJ->vx_hi[slot], 0xFC);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 5);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    e = then(e, ended_load(5));
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_StepListMoveXThenTurnUp(void)
{
    abi_ended(StepListMoveXThenTurnUp(cpu.x, cpu.y));
}

/*
 * @routine 05CB:296 WaitForScreenByte2
 * @run 70:B296
 * WHAT IT DOES: When the scroll byte ScrollXHiScreen is 2 sets OBJ_3510 $60 and goes to the next OBJ_2800.
 * HOW: A compare, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the new OBJ_2800's N and Z (A $60).
 */
Ended WaitForScreenByte2(uint8_t slot)
{
    uint8_t screen;
    Ended e;

    screen = RD8(SCROLLE->x_hi_screen[0]);
    e = ended_compare(screen, 2);
    if (screen != 2) {
        return e;
    }
    WR8(OBJ->f3510[slot], 0x60);
    e = then(e, ended_load(0x60));
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_WaitForScreenByte2(void)
{
    abi_ended(WaitForScreenByte2(cpu.x));
}

/*
 * @routine 05CB:2A6 CountDownThenSound84AndAppear
 * @run 70:B2A6
 * WHAT IT DOES: Counts OBJ_3510 down; at zero plays sound $84, places the object at X $0110, Y $0010 and goes to the next OBJ_2800.
 * HOW: A memory decrement, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
SoundEnd CountDownThenSound84AndAppear(uint8_t slot, uint8_t y_entry)
{
    uint8_t timer;
    SoundEnd r = l8_sound_none(ended_kept());

    timer = DEC8(OBJ->f3510[slot]);
    if (timer != 0) {
        r.end = ended_nz(timer);
        return r;
    }
    r.sounded = true;
    r.command = 0x84;
    r.before = then(ended_nz(timer), ended_load(0x84));
    jsr_push(0xB2B0);
    r.call = SoundCommand(0x84, slot, y_entry);
    jsr_pop();
    WR8(OBJ->x_hi[slot], 1);
    WR8(OBJ->x_lo[slot], 0x10);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0x10);
    r.end = then(ended_load(0x10), ended_nz(INC8(OBJ->f2800[slot])));
    return r;
}

void abi_CountDownThenSound84AndAppear(void)
{
    abi_sound_end(CountDownThenSound84AndAppear(cpu.x, cpu.y));
}

/*
 * @routine 05CB:2C6 StepListFallAndSwing
 * @run 70:B2C6
 * WHAT IT DOES: Steps the animation list at $B30B and moves the object; its X speed gains $0010 and its Y speed loses $0020 a frame; once its Y high byte is not 0 and its Y low byte is below $C0 it sets OBJ_3510 $40 and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, ApplyVelocity, two 16-bit adds in memory, two tests.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListFallAndSwing(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, value, high, low;
    FrameStep step;
    AxisMove move;
    Sum sum;
    Ended e = l8_step_list(slot, 0xB30B, 0xB2D1, false, &y, &step);

    jsr_push(0xB2D4);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    value = RD8(OBJ->vx_lo[slot]);
    sum = add_bytes(value, 0x10, false);
    WR8(OBJ->vx_lo[slot], sum.value);
    value = RD8(OBJ->vx_hi[slot]);
    sum = add_bytes(value, 0, sum.carry);
    WR8(OBJ->vx_hi[slot], sum.value);
    value = RD8(OBJ->vy_lo[slot]);
    sum = sub_bytes(value, 0x20, true);
    WR8(OBJ->vy_lo[slot], sum.value);
    value = RD8(OBJ->vy_hi[slot]);
    sum = sub_bytes(value, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    e = then(e, ended_of_sum(sum));
    high = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_load(high));
    if (high == 0) {
        return e;
    }
    low = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(low, 0xC0));
    if (low >= 0xC0) {
        return e;
    }
    WR8(OBJ->f3510[slot], 0x40);
    e = then(e, ended_load(0x40));
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_StepListFallAndSwing(void)
{
    abi_ended(StepListFallAndSwing(cpu.x, cpu.y));
}

/*
 * @routine 05CB:316 CountDownThenLeaveRight
 * @run 70:B316
 * WHAT IT DOES: Counts OBJ_3510 down; at zero sets OBJ_3250 2, places the object at X $00F0, gives it the speed X -$00E0 and Y $0200, clears the animation and goes to the next OBJ_2800.
 * HOW: A memory decrement, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended CountDownThenLeaveRight(uint8_t slot)
{
    uint8_t timer;
    Ended e;

    timer = DEC8(OBJ->f3510[slot]);
    e = ended_nz(timer);
    if (timer != 0) {
        return e;
    }
    WR8(OBJ->hitbox_on[slot], 2);
    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->x_lo[slot], 0xF0);
    WR8(OBJ->vx_hi[slot], 0xFF);
    WR8(OBJ->vx_lo[slot], 0x20);
    WR8(OBJ->vy_hi[slot], 2);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    e = then(e, ended_load(2));
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_CountDownThenLeaveRight(void)
{
    abi_ended(CountDownThenLeaveRight(cpu.x));
}

/*
 * @routine 05CB:3CA InsideCenterBand
 * @run 70:B3CA
 * WHAT IT DOES: Says whether the object's X is inside the band from $0038 to $00C7 on the first screen: C 0 when it is, C 1 when not.
 * HOW: A test of the high byte and two compares.
 * WAITS: no.
 * Entry: X object. Exit: C 0 inside, 1 outside; A and N, Z from the last load or compare.
 */
Ended InsideCenterBand(uint8_t slot)
{
    uint8_t high, low;
    Ended e;

    high = RD8(OBJ->x_hi[slot]);
    e = ended_load(high);
    if (high != 0) {
        return ended_with_carry(e, true);
    }
    low = RD8(OBJ->x_lo[slot]);
    e = ended_compare(low, 0x38);
    if (low < 0x38) {
        return ended_with_carry(e, true);
    }
    e = ended_compare(low, 0xC8);
    return ended_with_carry(e, low >= 0xC8);
}

void abi_InsideCenterBand(void)
{
    abi_ended(InsideCenterBand(cpu.x));
}

/*
 * @routine 05CB:3DE FacingByX
 * @run 70:B3DE
 * WHAT IT DOES: A 1 when the object's X (first screen) is below $80, a 0 when it is $80 or more; off the first screen it gives the side of the player's bit 0 instead.
 * HOW: Two ROLs and an EOR on the X low byte, or SideOfPlayer and an AND.
 * WAITS: no.
 * Entry: X object, C kept into the first ROL. Exit: A the answer; C bit 6 of the X low byte on the first screen.
 */
Ended FacingByX(uint8_t slot)
{
    uint8_t high, low, answer;
    SideResult side;
    Ended e;

    high = RD8(OBJ->x_hi[slot]);
    e = ended_load(high);
    if (high != 0) {
        jsr_push(0xB3F0);
        side = SideOfPlayer(slot);
        jsr_pop();
        e = then(e, ended_of_side(side));
        return then(e, ended_load((uint8_t)(side.sides & 0x01)));
    }
    low = RD8(OBJ->x_lo[slot]);
    e = ended_load(low);
    answer = (uint8_t)(((low >> 7) & 0x01) ^ 0x01);
    return then(ended_with_carry(e, ((low >> 6) & 0x01) != 0), ended_load(answer));
}

void abi_FacingByX(void)
{
    abi_ended(FacingByX(cpu.x));
}

/*
 * @routine 05CB:3F3 StepListThenTurnAround
 * @run 70:B3F3
 * WHAT IT DOES: Steps the animation list at $B411; when it ends sets frame 5, turns the object round and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, a store, an EOR, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListThenTurnAround(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, facing;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xB411, 0xB3FE, false, &y, &step);

    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->frame[slot], 5);
    e = then(e, ended_load(5));
    facing = RD8(OBJ->facing_left[slot]);
    facing = (uint8_t)(facing ^ 0x01);
    WR8(OBJ->facing_left[slot], facing);
    e = then(e, ended_load(facing));
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_StepListThenTurnAround(void)
{
    abi_ended(StepListThenTurnAround(cpu.x, cpu.y));
}

/*
 * @routine 05CB:49D StepListThenCountDownToState4
 * @run 70:B49D
 * WHAT IT DOES: Steps the animation list at $B375; when it ends counts OBJ_3510 down and at zero goes to state 4 with OBJ_2800 0.
 * HOW: StepObjectFrames, a memory decrement, two stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListThenCountDownToState4(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, timer;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xB375, 0xB4A8, false, &y, &step);

    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    timer = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(timer));
    if (timer != 0) {
        return e;
    }
    WR8(OBJ->state[slot], 4);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_load(4));
}

void abi_StepListThenCountDownToState4(void)
{
    abi_ended(StepListThenCountDownToState4(cpu.x, cpu.y));
}

/*
 * @routine 05CB:4D6 PickNextPhaseFromTable
 * @run 70:B4D6
 * WHAT IT DOES: Clears the animation and sets OBJ_2800 from the table at $B4EC by the low three bits of OBJ_3568, then advances OBJ_3568.
 * HOW: Stores, a table read, an INC.
 * WAITS: no.
 * Entry: X object. Exit: A the table byte, Y the index, N and Z from the new OBJ_3568.
 */
Ended PickNextPhaseFromTable(uint8_t slot)
{
    uint8_t pick, value;

    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    pick = (uint8_t)(RD8(OBJ->f3568[slot]) & 0x07);
    value = rd8((uint16_t)(0xB4EC + pick));
    WR8(OBJ->f2800[slot], value);
    return then(ended_with_y(ended_load(value), pick), ended_nz(INC8(OBJ->f3568[slot])));
}

void abi_PickNextPhaseFromTable(void)
{
    abi_ended(PickNextPhaseFromTable(cpu.x));
}

/*
 * @routine 05CB:4F4 StepListThenLeapWithSound
 * @run 70:B4F4
 * WHAT IT DOES: Steps the animation list at $B525; when it ends plays sound $85 and gives the object a leap: the X speed from the table at $B532 by its facing and the Y speed $0600; goes to the next OBJ_2800.
 * HOW: StepObjectFrames, SoundCommand, table reads, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
SoundEnd StepListThenLeapWithSound(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, facing, value;
    FrameStep step;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e = l8_step_list(slot, 0xB525, 0xB4FF, false, &y, &step);

    if (step != FRAME_LIST_ENDED) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0x85;
    r.before = then(e, ended_load(0x85));
    jsr_push(0xB506);
    r.call = SoundCommand(0x85, slot, y);
    jsr_pop();
    facing = RD8(OBJ->facing_left[slot]);
    y = (uint8_t)(facing << 1);
    value = rd8((uint16_t)(0xB533 + y));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xB532 + y));
    WR8(OBJ->vx_lo[slot], value);
    WR8(OBJ->vy_hi[slot], 6);
    WR8(OBJ->vy_lo[slot], 0);
    r.end = then(ended_with_y(ended_with_carry(ended_load(0), facing >= 0x80), y), ended_nz(INC8(OBJ->f2800[slot])));
    return r;
}

void abi_StepListThenLeapWithSound(void)
{
    abi_sound_end(StepListThenLeapWithSound(cpu.x, cpu.y));
}

/*
 * @routine 05CB:536 FlyAndSagWithTwoLists
 * @run 70:B536
 * WHAT IT DOES: Clears OBJ_3358 and steps one of two animation lists (at $B575, by the sign of the Y speed) while moving; the Y speed loses $0030 a frame; when it is negative and the Y low byte is below $78 sets OBJ_3358 2 and goes to the next OBJ_2800.
 * HOW: A store, two ROLs, a table read, StepObjectFrames, ApplyVelocity, a 16-bit subtract in memory, a compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended FlyAndSagWithTwoLists(uint8_t slot)
{
    uint8_t y, speed, lo, hi, value, low;
    FrameStep step;
    AxisMove move;
    Sum sum;
    Ended e;

    WR8(OBJ->f3358[slot], 0);
    speed = RD8(OBJ->vy_hi[slot]);
    y = (uint8_t)(((speed >> 7) & 0x01) << 1);
    e = ended_with_y(ended_with_carry(ended_load(y), false), y);
    lo = rd8((uint16_t)(0xB575 + y));
    WR8(ZP->temp10, lo);
    hi = rd8((uint16_t)(0xB576 + y));
    WR8(ZP->temp11, hi);
    e = then(e, ended_load(hi));
    jsr_push(0xB54F);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
    jsr_push(0xB552);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    value = RD8(OBJ->vy_lo[slot]);
    sum = sub_bytes(value, 0x30, true);
    WR8(OBJ->vy_lo[slot], sum.value);
    value = RD8(OBJ->vy_hi[slot]);
    sum = sub_bytes(value, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    e = then(e, ended_of_sum(sum));
    if ((sum.value & 0x80) == 0) {
        return e;
    }
    low = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(low, 0x78));
    if (low >= 0x78) {
        return e;
    }
    WR8(OBJ->f3358[slot], 2);
    e = then(e, ended_load(2));
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_FlyAndSagWithTwoLists(void)
{
    abi_ended(FlyAndSagWithTwoLists(cpu.x));
}

/*
 * @routine 05CB:58D SoundEvery32Frames
 * @run 70:B58D
 * WHAT IT DOES: When the low five bits of OBJ_34B8 are 0 plays sound $29.
 * HOW: An AND, SoundCommand.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: the AND, or the sound call's.
 */
SoundEnd SoundEvery32Frames(uint8_t slot, uint8_t y_entry)
{
    uint8_t bits;
    SoundEnd r = l8_sound_none(ended_kept());

    bits = (uint8_t)(RD8(OBJ->f34b8[slot]) & 0x1F);
    if (bits != 0) {
        r.end = ended_load(bits);
        return r;
    }
    r.sounded = true;
    r.command = 0x29;
    r.before = ended_load(0x29);
    jsr_push(0xB599);
    r.call = SoundCommand(0x29, slot, y_entry);
    jsr_pop();
    return r;
}

void abi_SoundEvery32Frames(void)
{
    abi_sound_end(SoundEvery32Frames(cpu.x, cpu.y));
}

/*
 * @routine 05CB:5C4 PickRandomThenState3
 * @run 70:B5C4
 * WHAT IT DOES: Takes a random number (kept in Y as 0 to 3), clears OBJ_2800 and the animation and goes to state 3.
 * HOW: NextRandom, an AND, stores.
 * WAITS: no.
 * Entry: X object. Exit: A 3, Y the random number masked, C and V from NextRandom.
 */
Ended PickRandomThenState3(uint8_t slot)
{
    uint8_t pick;
    RandomResult random;
    Ended e;

    jsr_push(0xB5C7);
    random = NextRandom();
    jsr_pop();
    pick = random.value & 0x03;
    e = then(ended_of_random(random), ended_with_y(ended_load(pick), pick));
    WR8(OBJ->f2800[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 3);
    return then(e, ended_load(3));
}

void abi_PickRandomThenState3(void)
{
    abi_ended(PickRandomThenState3(cpu.x));
}

/*
 * @routine 05CB:59A StepListSlowDownThenPickRandom
 * @run 70:B59A
 * WHAT IT DOES: Steps the animation list at $B580 and moves the object; every eighth frame of OBJ_34B8 halves its speed and, once both speed bytes are 0 or both $FF, goes on to PickRandomThenState3.
 * HOW: StepObjectFrames, ApplyVelocity, HalveObjectVelocity, tests of OBJ_VY_HI and OBJ_VY_LO.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListSlowDownThenPickRandom(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, bits, high, low;
    FrameStep step;
    AxisMove move;
    ArithExit halved;
    Ended e = l8_step_list(slot, 0xB580, 0xB5A5, false, &y, &step);

    jsr_push(0xB5A8);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    bits = (uint8_t)(RD8(OBJ->f34b8[slot]) & 0x07);
    e = then(e, ended_load(bits));
    if (bits != 0) {
        return e;
    }
    jsr_push(0xB5B2);
    halved = HalveObjectVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_arith(halved));
    high = RD8(OBJ->vy_hi[slot]);
    low = RD8(OBJ->vy_lo[slot]);
    e = then(e, ended_load((uint8_t)(high | low)));
    if ((high | low) != 0) {
        high = RD8(OBJ->vy_hi[slot]);
        low = RD8(OBJ->vy_lo[slot]);
        e = then(e, ended_compare((uint8_t)(high & low), 0xFF));
        if ((high & low) != 0xFF) {
            return e;
        }
    }
    return then(e, PickRandomThenState3(slot));
}

void abi_StepListSlowDownThenPickRandom(void)
{
    abi_ended(StepListSlowDownThenPickRandom(cpu.x, cpu.y));
}

/*
 * @routine 05CB:465 EdgeCodeByDirection
 * @run 70:B465
 * WHAT IT DOES: Gives 2 when the object is inside the center band (InsideCenterBand says C 0), else 1 when OBJ_3618 is 0 and 0 when it is not.
 * HOW: A load and a call to InsideCenterBand, a load of the answer.
 * WAITS: no.
 * Entry: X object. Exit: A the answer, N and Z as the load. The arm for OBJ_3618 not 0 outside the band ran in no script.
 */
Ended EdgeCodeByDirection(uint8_t slot)
{
    uint8_t direction;
    Ended e, band;

    direction = RD8(OBJ->f3618[slot]);
    e = ended_load(direction);
    jsr_push(direction != 0 ? 0xB46D : 0xB477);
    band = InsideCenterBand(slot);
    jsr_pop();
    e = then(e, band);
    if (band.c) {
        if (direction != 0) {
            return then(e, ended_with_a(ended_kept(), 0));
        }
        return then(e, ended_load(1));
    }
    return then(e, ended_load(2));
}

void abi_EdgeCodeByDirection(void)
{
    abi_ended(EdgeCodeByDirection(cpu.x));
}

/*
 * @routine 05CB:3A1 TurnTowardCenterOrWait
 * @run 70:B3A1
 * WHAT IT DOES: Clears the animation, goes to the next OBJ_2800, sets OBJ_3510 to 2 to 5 at random and OBJ_3618 to 0 or 1 by whether it is outside the center band; when the object's facing already matches FacingByX goes on once more.
 * HOW: NextRandom, InsideCenterBand, FacingByX, a ROL, a compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended TurnTowardCenterOrWait(uint8_t slot)
{
    uint8_t pick, facing;
    RandomResult random;
    Sum sum;
    Ended e, band, side;

    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    e = ended_nz(INC8(OBJ->f2800[slot]));
    jsr_push(0xB3AD);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = random.value & 0x03;
    e = then(e, ended_load(pick));
    sum = add_bytes(pick, 2, false);
    WR8(OBJ->f3510[slot], sum.value);
    e = then(e, ended_of_sum(sum));
    jsr_push(0xB3B8);
    band = InsideCenterBand(slot);
    jsr_pop();
    e = then(e, band);
    pick = (uint8_t)((uint8_t)(band.a << 1) | (band.c ? 1 : 0));
    e = then(e, ended_with_carry(ended_load(pick), (band.a & 0x80) != 0));
    pick = pick & 0x01;
    e = then(e, ended_load(pick));
    WR8(OBJ->f3618[slot], pick);
    jsr_push(0xB3C1);
    side = FacingByX(slot);
    jsr_pop();
    e = then(e, side);
    facing = RD8(OBJ->facing_left[slot]);
    e = then(e, ended_compare(side.a, facing));
    if (side.a != facing) {
        return e;
    }
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_TurnTowardCenterOrWait(void)
{
    abi_ended(TurnTowardCenterOrWait(cpu.x));
}

/*
 * @routine 05CB:41C StepSideToSideWithCenterCheck
 * @run 70:B41C
 * WHAT IT DOES: Sets the X speed from a table at $B462 by the object's facing and moves along X while it steps the animation list at $B375; when the list ends it asks EdgeCodeByDirection: 0 leaves it, 1 sets OBJ_3510 1 and advances OBJ_2800 (and once more when the side of the player matches the facing), 2 counts OBJ_3510 down first.
 * HOW: Table reads, ApplyVelocityX, StepObjectFrames, EdgeCodeByDirection, SideOfPlayer.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepSideToSideWithCenterCheck(uint8_t slot, uint8_t y_entry)
{
    uint8_t facing, y, value, code, timer, bits, current;
    FrameStep step;
    AxisMove move;
    SideResult side;
    Ended e, edge;

    facing = RD8(OBJ->facing_left[slot]);
    y = (uint8_t)(facing << 1);
    e = ended_with_y(ended_with_carry(ended_load(y), facing >= 0x80), y);
    value = rd8((uint16_t)(0xB462 + y));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xB462 + y));
    WR8(OBJ->vx_lo[slot], value);
    e = then(e, ended_load(value));
    jsr_push(0xB430);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    e = then(e, l8_step_list(slot, 0xB375, 0xB43B, false, &y, &step));
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    jsr_push(0xB440);
    edge = EdgeCodeByDirection(slot);
    jsr_pop();
    e = then(e, edge);
    code = edge.a;
    if (code == 0) {
        return e;
    }
    e = then(e, ended_compare(code, 1));
    if (code != 1) {
        timer = DEC8(OBJ->f3510[slot]);
        e = then(e, ended_nz(timer));
        if (timer != 0) {
            return e;
        }
    }
    WR8(OBJ->f3510[slot], 1);
    e = then(e, ended_load(1));
    e = then(e, ended_nz(INC8(OBJ->f2800[slot])));
    jsr_push(0xB456);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_side(side));
    bits = side.sides & 0x01;
    e = then(e, ended_load(bits));
    current = RD8(OBJ->facing_left[slot]);
    e = then(e, ended_compare(bits, current));
    if (bits != current) {
        return e;
    }
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_StepSideToSideWithCenterCheck(void)
{
    abi_ended(StepSideToSideWithCenterCheck(cpu.x, cpu.y));
}

/* StepThenCdAgain's result: the sound the first call may play, and whether the music was restarted. */
typedef struct {
    SoundEnd sound;
    bool played;
    TrackPlay track;
    Ended post;
} FallEnd;

/*
 * @routine 05CB:344 StepListFallThenMusicAgain
 * @run 70:B344
 * WHAT IT DOES: Plays the sound every 32 frames (SoundEvery32Frames), steps the animation list at $B375 and moves; once the Y high byte is 0 and the Y low byte is $40 or more it clears bits 3 and 4 of zField8E, plays the track again (CdPlayAgain), sets bit 2 of zField8E, goes to the next state and clears OBJ_2800.
 * HOW: SoundEvery32Frames, StepObjectFrames, ApplyVelocity, RMW of zField8E, CdPlayAgain.
 * WAITS: no. (CdPlayAgain starts a disc command; it does not wait.)
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
static FallEnd step_list_fall_then_music_again(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, high, low, flags, state;
    FrameStep step;
    AxisMove move;
    FallEnd r;
    Ended e;

    r.played = false;
    r.post = ended_kept();
    jsr_push(0xB347);
    r.sound = SoundEvery32Frames(slot, y);
    jsr_pop();
    e = l8_step_list(slot, 0xB375, 0xB352, false, &y, &step);
    jsr_push(0xB355);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    high = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_load(high));
    if (high != 0) {
        r.sound = sound_end_then(r.sound, e);
        return r;
    }
    low = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(low, 0x40));
    if (low < 0x40) {
        r.sound = sound_end_then(r.sound, e);
        return r;
    }
    RMW8(ZP->field8e, old, old & 0xF7);
    RMW8(ZP->field8e, old, old & 0xEF);
    r.sound = sound_end_then(r.sound, e);
    jsr_push(0xB368);
    r.track = CdPlayAgain(slot, y);
    jsr_pop();
    r.played = true;
    flags = RD8(ZP->field8e);
    flags = (uint8_t)(flags | 0x04);
    WR8(ZP->field8e, flags);
    state = INC8(OBJ->state[slot]);
    WR8(OBJ->f2800[slot], 0);
    r.post = then(ended_load(flags), ended_nz(state));
    return r;
}

void abi_StepListFallThenMusicAgain(void)
{
    FallEnd r = step_list_fall_then_music_again(cpu.x, cpu.y);

    abi_sound_end(r.sound);
    if (r.played) {
        play_track_exit_shim(r.track);
        abi_ended(r.post);
    }
}

/*
 * @routine 05CB:5EE StepListThenSoundAndSpawn
 * @run 70:B5EE
 * WHAT IT DOES: Steps the animation list at $B62B; when it ends goes to PickRandomThenState3; when the graphic changes at animation step $0A plays sound $2A; from step $0A on, every fourth frame of OBJ_34B8, starts an object of type $25 in a free slot of $20 to $3F facing as this one.
 * HOW: StepObjectFrames, SoundCommand, FindFreeSlotFrom20, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
SoundEnd StepListThenSoundAndSpawn(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, anim, bits, found_slot, last, placed, facing;
    FrameStep step;
    bool found;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e = l8_step_list(slot, 0xB62B, 0xB5F9, false, &y, &step);

    if (step == FRAME_LIST_ENDED) {
        r.end = then(e, PickRandomThenState3(slot));
        return r;
    }
    if (step == FRAME_CHANGED) {
        anim = RD8(OBJ->anim_step[slot]);
        e = then(e, ended_compare(anim, 0x0A));
        if (anim == 0x0A) {
            r.sounded = true;
            r.command = 0x2A;
            r.before = then(e, ended_load(0x2A));
            jsr_push(0xB60C);
            r.call = SoundCommand(0x2A, slot, y);
            jsr_pop();
            e = ended_kept();
        }
    }
    anim = RD8(OBJ->anim_step[slot]);
    e = then(e, ended_compare(anim, 0x0A));
    if (anim < 0x0A) {
        r.end = e;
        return r;
    }
    bits = (uint8_t)(RD8(OBJ->f34b8[slot]) & 0x03);
    e = then(e, ended_load(bits));
    if (bits != 0) {
        r.end = e;
        return r;
    }
    jsr_push(0xB61D);
    found_slot = FindFreeSlotFrom20(&last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, last, found));
    if (!found) {
        r.end = e;
        return r;
    }
    jsr_push(0xB624);
    placed = StartObjectAtPlace(0x25, slot, found_slot);
    jsr_pop();
    e = then(e, ended_load(placed));
    facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[found_slot], facing);
    r.end = then(e, ended_load(facing));
    return r;
}

void abi_StepListThenSoundAndSpawn(void)
{
    abi_sound_end(StepListThenSoundAndSpawn(cpu.x, cpu.y));
}

/*
 * @routine 05CB:648 StepListThenBurstAtFrame14
 * @run 70:B648
 * WHAT IT DOES: Steps the animation list at $B675; when it ends goes to PickRandomThenState3; when the graphic changes and the frame is $14 plays sound $86 and starts an object of type $28 in slot $13 facing as this one.
 * HOW: StepObjectFrames, SoundCommand, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
SoundEnd StepListThenBurstAtFrame14(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, frame, placed, facing;
    FrameStep step;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e = l8_step_list(slot, 0xB675, 0xB653, false, &y, &step);

    if (step == FRAME_LIST_ENDED) {
        r.end = then(e, PickRandomThenState3(slot));
        return r;
    }
    if (step != FRAME_CHANGED) {
        r.end = e;
        return r;
    }
    frame = RD8(OBJ->frame[slot]);
    e = then(e, ended_compare(frame, 0x14));
    if (frame != 0x14) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0x86;
    r.before = then(e, ended_load(0x86));
    jsr_push(0xB663);
    r.call = SoundCommand(0x86, slot, y);
    jsr_pop();
    jsr_push(0xB66A);
    placed = StartObjectAtPlace(0x28, slot, 0x13);
    jsr_pop();
    facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[0x13], facing);
    r.end = then(then(ended_with_y(ended_nz(0x13), 0x13), ended_load(placed)), ended_load(facing));
    return r;
}

void abi_StepListThenBurstAtFrame14(void)
{
    abi_sound_end(StepListThenBurstAtFrame14(cpu.x, cpu.y));
}
