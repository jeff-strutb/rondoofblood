/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 05CB (enemy kinds not yet identified), by routine. Bodies return the
 * arm they ended on and the values behind it; each routine's exit shim builds how its last instructions left A, Y and the flags (Ended,
 * enemy_objects.h), and the abi_ wrappers show it.
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
 * WHAT IT DOES: In state 0 loads the record and sets sprite set 7, frame $12 and the hit height $14. Later it steps the animation list at $B136; once the object has taken 5 hits it records its event, takes the kind of OBJ_START_ARG, sets OBJ_INVULN_TIMER $10, goes to state 0 and starts an object of type $9C from a free slot of $50 to $53 with the Y speed -$0600, moved once.
 * HOW: LoadObjectRecord, StepObjectFrames, RecordObjectEvent, FindFreeSlotFrom50, StartObjectAtPlace, ApplyVelocityY with X and Y swapped.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_record_event_fifth_exit_shim).
 */
enum { ST15_FIFTH_LOADED, ST15_FIFTH_WAITING, ST15_FIFTH_NO_SLOT, ST15_FIFTH_STARTED };

L8St15Fifth LoadRecordThenRecordEventAtFifth(uint8_t slot, uint8_t y_entry)
{
    L8St15Fifth r = { 0 };

    r.load = l8_load_new(slot, 0xB0EB);
    if (r.load.state == 0) {
        WR8(OBJ->sprite_set[slot], 7);
        WR8(OBJ->frame[slot], 0x12);
        WR8(OBJ->hitbox_half_h[slot], 0x14);
        r.arm = ST15_FIFTH_LOADED;
        return r;
    }
    r.step = l8_list_step(slot, 0xB136, 0xB106, true, y_entry);
    r.hits = RD8(OBJ->hit_taken[slot]);
    if (r.hits < 5) {
        r.arm = ST15_FIFTH_WAITING;
        return r;
    }
    jsr_push(0xB110);
    r.marked = RecordObjectEvent(slot, &r.stored, &r.event, &r.byte);
    jsr_pop();
    r.kind = RD8(OBJ->f3040[slot]);
    WR8(OBJ->kind[slot], r.kind);
    WR8(OBJ->invuln_timer[slot], 0x10);
    WR8(OBJ->state[slot], 0);
    jsr_push(0xB121);
    r.found.slot = FindFreeSlotFrom50(&r.found.last, &r.found.found);
    jsr_pop();
    if (!r.found.found) {
        r.arm = ST15_FIFTH_NO_SLOT;
        return r;
    }
    jsr_push(0xB128);
    r.placed = StartObjectAtPlace(0x9C, slot, r.found.slot);
    jsr_pop();
    WR8(OBJ->vy_hi[r.found.slot], 0xFA);
    jsr_push(0xB131);
    r.move = ApplyVelocityY(r.found.slot);
    jsr_pop();
    WR8(OBJ->vy_hi[r.found.slot], 0);
    r.arm = ST15_FIFTH_STARTED;
    return r;
}

/* What LoadRecordThenRecordEventAtFifth leaves, from the arm it ended on. */
static Ended l8_record_event_fifth_exit_shim(L8St15Fifth r)
{
    Ended e;

    if (r.arm == ST15_FIFTH_LOADED) {
        return then(l8_load_ended(r.load), ended_load(0x14));
    }
    e = l8_step_ended(r.step);
    e = then(e, ended_compare(r.hits, 5));
    if (r.arm == ST15_FIFTH_WAITING) {
        return e;
    }
    e = then(e, l8_of_event(r.marked, r.stored, r.event, r.byte));
    e = then(e, ended_load(r.kind));
    e = then(e, ended_load(0x10));
    e = then(e, l8_slot_ended(r.found));
    if (r.arm == ST15_FIFTH_NO_SLOT) {
        return e;
    }
    e = then(e, ended_load(r.placed));
    e = then(e, ended_load(0xFA));
    e = then(e, ended_of_axis_move(r.move));
    return then(e, ended_with_y(ended_kept(), r.found.slot));
}

void abi_LoadRecordThenRecordEventAtFifth(void)
{
    abi_ended(l8_record_event_fifth_exit_shim(LoadRecordThenRecordEventAtFifth(cpu.x, cpu.y)));
}

/*
 * @routine 05CB:1A9 FlagThenHitsCompare
 * @run 70:B1A9
 * WHAT IT DOES: Tests bit 7 of OBJ_HIT_STATUS and, when it is clear, compares the hits taken with 5; nothing is stored.
 * HOW: An AND and a compare.
 * WAITS: no.
 * Entry: X object. Exit: the AND, or the compare (l8_flag_then_hits_exit_shim).
 */
enum { ST15_FLAG_SET, ST15_FLAG_CLEAR };

L8Arm FlagThenHitsCompare(uint8_t slot)
{
    L8Arm r = { 0 };

    r.first = RD8(OBJ->hit_status[slot]);
    if ((r.first & 0x80) != 0) {
        r.arm = ST15_FLAG_SET;
        return r;
    }
    r.second = RD8(OBJ->hit_taken[slot]);
    r.arm = ST15_FLAG_CLEAR;
    return r;
}

/* What FlagThenHitsCompare leaves, from the arm it ended on (first the flags byte, second the hits). */
Ended l8_flag_then_hits_exit_shim(L8Arm r)
{
    Ended e = ended_load((uint8_t)(r.first & 0x80));

    if (r.arm == ST15_FLAG_SET) {
        return e;
    }
    return then(e, ended_compare(r.second, 5));
}

void abi_FlagThenHitsCompare(void)
{
    abi_ended(l8_flag_then_hits_exit_shim(FlagThenHitsCompare(cpu.x)));
}

/*
 * @routine 05CB:1B8 EnterState5WhenFlagged
 * @run 70:B1B8
 * WHAT IT DOES: When bit 7 of OBJ_HIT_STATUS is set and the state is below 5, sets the state 5 and OBJ_SUBSTATE 0.
 * HOW: Two tests, two stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_enter_state5_exit_shim). The stores ran in no script; the draft stops there.
 */
enum { ST15_ENTER5_UNFLAGGED, ST15_ENTER5_STATE_HIGH, ST15_ENTER5_ENTERED };

L8Arm EnterState5WhenFlagged(uint8_t slot)
{
    L8Arm r = { 0 };

    r.first = RD8(OBJ->hit_status[slot]);
    if ((r.first & 0x80) == 0) {
        r.arm = ST15_ENTER5_UNFLAGGED;
        return r;
    }
    r.second = RD8(OBJ->state[slot]);
    if (r.second >= 5) {
        r.arm = ST15_ENTER5_STATE_HIGH;
        return r;
    }
    WR8(OBJ->state[slot], 5);
    WR8(OBJ->substate[slot], 0);
    r.arm = ST15_ENTER5_ENTERED;
    return r;
}

/* What EnterState5WhenFlagged leaves, from the arm it ended on (first the flags byte, second the state). */
Ended l8_enter_state5_exit_shim(L8Arm r)
{
    Ended e = ended_load((uint8_t)(r.first & 0x80));

    if (r.arm == ST15_ENTER5_UNFLAGGED) {
        return e;
    }
    e = then(e, ended_compare(r.second, 5));
    if (r.arm == ST15_ENTER5_STATE_HIGH) {
        return e;
    }
    return then(e, ended_load(5));
}

void abi_EnterState5WhenFlagged(void)
{
    abi_ended(l8_enter_state5_exit_shim(EnterState5WhenFlagged(cpu.x)));
}

/*
 * @routine 05CB:1ED LoadRecordStartTwoPieces
 * @run 70:B1ED
 * WHAT IT DOES: Loads the object's record, clears slots $11 and $12 and starts an object of type $26 in each at the object's place; OBJ_3510 $40.
 * HOW: LoadObjectRecord, ClearObjectInY and StartObjectAtPlace twice, a store.
 * WAITS: no.
 * Entry: X object. Exit: A $40, Y $12, C from the record (l8_two_pieces_exit_shim).
 */
L8Arm LoadRecordStartTwoPieces(uint8_t slot)
{
    L8Arm r = { 0 };

    jsr_push(0xB1F0);
    r.record = l8_record(slot);
    jsr_pop();
    jsr_push(0xB1F5);
    ClearObjectInY(0x11);
    jsr_pop();
    jsr_push(0xB1FA);
    r.first = StartObjectAtPlace(0x26, slot, 0x11);
    jsr_pop();
    jsr_push(0xB1FE);
    ClearObjectInY(0x12);
    jsr_pop();
    jsr_push(0xB203);
    r.second = StartObjectAtPlace(0x26, slot, 0x12);
    jsr_pop();
    WR8(OBJ->f3510[slot], 0x40);
    return r;
}

/* What LoadRecordStartTwoPieces leaves (first and second the two starts' results). */
static Ended l8_two_pieces_exit_shim(L8Arm r)
{
    Ended e = l8_record_ended(r.record);

    e = then(then(e, ended_with_y(ended_nz(0x11), 0x11)), ended_load(r.first));
    e = then(then(e, ended_with_y(ended_nz(0x12), 0x12)), ended_load(r.second));
    return then(e, ended_load(0x40));
}

void abi_LoadRecordStartTwoPieces(void)
{
    abi_ended(l8_two_pieces_exit_shim(LoadRecordStartTwoPieces(cpu.x)));
}

/*
 * @routine 05CB:232 SetUpEntryFromLeft
 * @run 70:B232
 * WHAT IT DOES: Places the object at X $FFF0, Y $0030 with the speed X $00C0 and goes to the next OBJ_SUBSTATE.
 * HOW: Stores and an INC.
 * WAITS: no.
 * Entry: X object. Exit: A $C0, N and Z from the new OBJ_SUBSTATE (l8_entry_from_left_exit_shim).
 */
uint8_t SetUpEntryFromLeft(uint8_t slot)
{
    WR8(OBJ->x_hi[slot], 0xFF);
    WR8(OBJ->x_lo[slot], 0xF0);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0x30);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0xC0);
    return INC8(OBJ->substate[slot]);
}

/* What SetUpEntryFromLeft leaves (count the new OBJ_SUBSTATE). */
static Ended l8_entry_from_left_exit_shim(uint8_t count)
{
    return then(ended_load(0xC0), ended_nz(count));
}

void abi_SetUpEntryFromLeft(void)
{
    abi_ended(l8_entry_from_left_exit_shim(SetUpEntryFromLeft(cpu.x)));
}

/*
 * @routine 05CB:250 StepListMoveXThenTurnUp
 * @run 70:B250
 * WHAT IT DOES: Steps the animation list at $B289 and moves the object along X; once it is at X $0120 or more it takes frame 0, the speed X -$0400 and Y $0500, clears the animation and goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, ApplyVelocityX, two compares, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_move_x_turn_up_exit_shim).
 */
enum { ST15_TURNUP_HIGH_NOT_1, ST15_TURNUP_LOW_BELOW, ST15_TURNUP_TURNED };

L8Arm StepListMoveXThenTurnUp(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xB289, 0xB25B, false, y_entry);
    jsr_push(0xB25E);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    r.first = RD8(OBJ->x_hi[slot]);
    if (r.first != 1) {
        r.arm = ST15_TURNUP_HIGH_NOT_1;
        return r;
    }
    r.second = RD8(OBJ->x_lo[slot]);
    if (r.second < 0x20) {
        r.arm = ST15_TURNUP_LOW_BELOW;
        return r;
    }
    WR8(OBJ->frame[slot], 0);
    WR8(OBJ->vx_hi[slot], 0xFC);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 5);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.third = INC8(OBJ->substate[slot]);
    r.arm = ST15_TURNUP_TURNED;
    return r;
}

/* What StepListMoveXThenTurnUp leaves (first the X high byte, second the low, third the new OBJ_SUBSTATE). */
static Ended l8_move_x_turn_up_exit_shim(L8Arm r)
{
    Ended e = l8_step_ended(r.step);

    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_compare(r.first, 1));
    if (r.arm == ST15_TURNUP_HIGH_NOT_1) {
        return e;
    }
    e = then(e, ended_compare(r.second, 0x20));
    if (r.arm == ST15_TURNUP_LOW_BELOW) {
        return e;
    }
    e = then(e, ended_load(5));
    return then(e, ended_nz(r.third));
}

void abi_StepListMoveXThenTurnUp(void)
{
    abi_ended(l8_move_x_turn_up_exit_shim(StepListMoveXThenTurnUp(cpu.x, cpu.y)));
}

/*
 * @routine 05CB:296 WaitForScreenByte2
 * @run 70:B296
 * WHAT IT DOES: When the scroll byte ScrollXHiScreen is 2 sets OBJ_3510 $60 and goes to the next OBJ_SUBSTATE.
 * HOW: A compare, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the new OBJ_SUBSTATE's N and Z (A $60) (l8_wait_screen_exit_shim).
 */
enum { ST15_SCREEN_WAITING, ST15_SCREEN_REACHED };

L8Arm WaitForScreenByte2(uint8_t slot)
{
    L8Arm r = { 0 };

    r.first = RD8(SCROLLE->x_hi_screen[0]);
    if (r.first != 2) {
        r.arm = ST15_SCREEN_WAITING;
        return r;
    }
    WR8(OBJ->f3510[slot], 0x60);
    r.second = INC8(OBJ->substate[slot]);
    r.arm = ST15_SCREEN_REACHED;
    return r;
}

/* What WaitForScreenByte2 leaves (first the screen byte, second the new OBJ_SUBSTATE). */
static Ended l8_wait_screen_exit_shim(L8Arm r)
{
    Ended e = ended_compare(r.first, 2);

    if (r.arm == ST15_SCREEN_WAITING) {
        return e;
    }
    e = then(e, ended_load(0x60));
    return then(e, ended_nz(r.second));
}

void abi_WaitForScreenByte2(void)
{
    abi_ended(l8_wait_screen_exit_shim(WaitForScreenByte2(cpu.x)));
}

/*
 * @routine 05CB:2A6 CountDownThenSound84AndAppear
 * @run 70:B2A6
 * WHAT IT DOES: Counts OBJ_3510 down; at zero plays sound $84, places the object at X $0110, Y $0010 and goes to the next OBJ_SUBSTATE.
 * HOW: A memory decrement, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_sound84_appear_exit_shim).
 */
enum { ST15_SOUND_IDLE, ST15_SOUND_PLAYED };

L8St15Sound CountDownThenSound84AndAppear(uint8_t slot, uint8_t y_entry)
{
    L8St15Sound r = { 0 };

    r.first = DEC8(OBJ->f3510[slot]);
    if (r.first != 0) {
        r.arm = ST15_SOUND_IDLE;
        return r;
    }
    jsr_push(0xB2B0);
    r.call = SoundCommand(0x84, slot, y_entry);
    jsr_pop();
    WR8(OBJ->x_hi[slot], 1);
    WR8(OBJ->x_lo[slot], 0x10);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0x10);
    r.second = INC8(OBJ->substate[slot]);
    r.arm = ST15_SOUND_PLAYED;
    return r;
}

/* What CountDownThenSound84AndAppear leaves, the sound call's included (first the count, second the new OBJ_SUBSTATE). */
static SoundEnd l8_sound84_appear_exit_shim(L8St15Sound r)
{
    SoundEnd s = l8_sound_none(ended_kept());

    if (r.arm == ST15_SOUND_IDLE) {
        s.end = ended_nz(r.first);
        return s;
    }
    s.sounded = true;
    s.command = 0x84;
    s.before = then(ended_nz(r.first), ended_load(0x84));
    s.call = r.call;
    s.end = then(ended_load(0x10), ended_nz(r.second));
    return s;
}

void abi_CountDownThenSound84AndAppear(void)
{
    abi_sound_end(l8_sound84_appear_exit_shim(CountDownThenSound84AndAppear(cpu.x, cpu.y)));
}

/*
 * @routine 05CB:2C6 StepListFallAndSwing
 * @run 70:B2C6
 * WHAT IT DOES: Steps the animation list at $B30B and moves the object; its X speed gains $0010 and its Y speed loses $0020 a frame; once its Y high byte is not 0 and its Y low byte is below $C0 it sets OBJ_3510 $40 and goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, ApplyVelocity, two 16-bit adds in memory, two tests.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_fall_swing_exit_shim).
 */
enum { ST15_SWING_HIGH_ZERO, ST15_SWING_LOW_HIGH, ST15_SWING_DONE };

L8Arm StepListFallAndSwing(uint8_t slot, uint8_t y_entry)
{
    uint8_t value;
    Sum sum;
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xB30B, 0xB2D1, false, y_entry);
    jsr_push(0xB2D4);
    r.move = ApplyVelocity(slot);
    jsr_pop();
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
    r.sum = sum;
    r.first = RD8(OBJ->y_hi[slot]);
    if (r.first == 0) {
        r.arm = ST15_SWING_HIGH_ZERO;
        return r;
    }
    r.second = RD8(OBJ->y_lo[slot]);
    if (r.second >= 0xC0) {
        r.arm = ST15_SWING_LOW_HIGH;
        return r;
    }
    WR8(OBJ->f3510[slot], 0x40);
    r.third = INC8(OBJ->substate[slot]);
    r.arm = ST15_SWING_DONE;
    return r;
}

/* What StepListFallAndSwing leaves (first the Y high byte, second the low, third the new OBJ_SUBSTATE; sum the last subtract). */
static Ended l8_fall_swing_exit_shim(L8Arm r)
{
    Ended e = l8_step_ended(r.step);

    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_of_sum(r.sum));
    e = then(e, ended_load(r.first));
    if (r.arm == ST15_SWING_HIGH_ZERO) {
        return e;
    }
    e = then(e, ended_compare(r.second, 0xC0));
    if (r.arm == ST15_SWING_LOW_HIGH) {
        return e;
    }
    e = then(e, ended_load(0x40));
    return then(e, ended_nz(r.third));
}

void abi_StepListFallAndSwing(void)
{
    abi_ended(l8_fall_swing_exit_shim(StepListFallAndSwing(cpu.x, cpu.y)));
}

/*
 * @routine 05CB:316 CountDownThenLeaveRight
 * @run 70:B316
 * WHAT IT DOES: Counts OBJ_3510 down; at zero sets OBJ_3250 2, places the object at X $00F0, gives it the speed X -$00E0 and Y $0200, clears the animation and goes to the next OBJ_SUBSTATE.
 * HOW: A memory decrement, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_leave_right_exit_shim).
 */
enum { ST15_LEAVE_COUNTING, ST15_LEAVE_GONE };

L8Arm CountDownThenLeaveRight(uint8_t slot)
{
    L8Arm r = { 0 };

    r.first = DEC8(OBJ->f3510[slot]);
    if (r.first != 0) {
        r.arm = ST15_LEAVE_COUNTING;
        return r;
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
    r.second = INC8(OBJ->substate[slot]);
    r.arm = ST15_LEAVE_GONE;
    return r;
}

/* What CountDownThenLeaveRight leaves (first the count, second the new OBJ_SUBSTATE). */
static Ended l8_leave_right_exit_shim(L8Arm r)
{
    Ended e = ended_nz(r.first);

    if (r.arm == ST15_LEAVE_COUNTING) {
        return e;
    }
    e = then(e, ended_load(2));
    return then(e, ended_nz(r.second));
}

void abi_CountDownThenLeaveRight(void)
{
    abi_ended(l8_leave_right_exit_shim(CountDownThenLeaveRight(cpu.x)));
}

/*
 * @routine 05CB:3CA InsideCenterBand
 * @run 70:B3CA
 * WHAT IT DOES: Says whether the object's X is inside the band from $0038 to $00C7 on the first screen: C 0 when it is, C 1 when not.
 * HOW: A test of the high byte and two compares.
 * WAITS: no.
 * Entry: X object. Exit: C 0 inside, 1 outside; A and N, Z from the last load or compare (l8_inside_band_exit_shim).
 */
enum { ST15_BAND_OFF_SCREEN, ST15_BAND_LEFT, ST15_BAND_RIGHT_OR_INSIDE };

L8St15Band InsideCenterBand(uint8_t slot)
{
    uint8_t high;
    L8St15Band r = { 0 };

    high = RD8(OBJ->x_hi[slot]);
    if (high != 0) {
        r.arm = ST15_BAND_OFF_SCREEN;
        r.a = high;
        r.outside = true;
        return r;
    }
    r.a = RD8(OBJ->x_lo[slot]);
    if (r.a < 0x38) {
        r.arm = ST15_BAND_LEFT;
        r.outside = true;
        return r;
    }
    r.arm = ST15_BAND_RIGHT_OR_INSIDE;
    r.outside = r.a >= 0xC8;
    return r;
}

/* What InsideCenterBand leaves (a the byte it read last, outside the carry). */
static Ended l8_inside_band_exit_shim(L8St15Band r)
{
    if (r.arm == ST15_BAND_OFF_SCREEN) {
        return ended_with_carry(ended_load(r.a), true);
    }
    if (r.arm == ST15_BAND_LEFT) {
        return ended_with_carry(ended_compare(r.a, 0x38), true);
    }
    return ended_with_carry(ended_compare(r.a, 0xC8), r.outside);
}

void abi_InsideCenterBand(void)
{
    abi_ended(l8_inside_band_exit_shim(InsideCenterBand(cpu.x)));
}

/*
 * @routine 05CB:3DE FacingByX
 * @run 70:B3DE
 * WHAT IT DOES: A 1 when the object's X (first screen) is below $80, a 0 when it is $80 or more; off the first screen it gives the side of the player's bit 0 instead.
 * HOW: Two ROLs and an EOR on the X low byte, or SideOfPlayer and an AND.
 * WAITS: no.
 * Entry: X object, C kept into the first ROL. Exit: A the answer; C bit 6 of the X low byte on the first screen (l8_facing_by_x_exit_shim).
 */
enum { ST15_FACING_OFF_SCREEN, ST15_FACING_ON_SCREEN };

L8St15Facing FacingByX(uint8_t slot, L8Layout lay)
{
    L8St15Facing r = { 0 };

    r.high = RD8(OBJ->x_hi[slot]);
    if (r.high != 0) {
        jsr_push(l8_at(lay, 0xB3F0));
        r.side = SideOfPlayer(slot);
        jsr_pop();
        r.a = (uint8_t)(r.side.sides & 0x01);
        r.arm = ST15_FACING_OFF_SCREEN;
        return r;
    }
    r.low = RD8(OBJ->x_lo[slot]);
    r.a = (uint8_t)(((r.low >> 7) & 0x01) ^ 0x01);
    r.arm = ST15_FACING_ON_SCREEN;
    return r;
}

/* What FacingByX leaves (a the answer). */
static Ended l8_facing_by_x_exit_shim(L8St15Facing r)
{
    if (r.arm == ST15_FACING_OFF_SCREEN) {
        return then(then(ended_load(r.high), ended_of_side(r.side)), ended_load(r.a));
    }
    return then(ended_with_carry(ended_load(r.low), ((r.low >> 6) & 0x01) != 0), ended_load(r.a));
}

void abi_FacingByX(void)
{
    abi_ended(l8_facing_by_x_exit_shim(FacingByX(cpu.x, L8_LOADED)));
}

/*
 * @routine 05CB:3F3 StepListThenTurnAround
 * @run 70:B3F3
 * WHAT IT DOES: Steps the animation list at $B411; when it ends sets frame 5, turns the object round and goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, a store, an EOR, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_turn_around_exit_shim).
 */
enum { ST15_TURN_STEPPED, ST15_TURN_TURNED };

L8Arm StepListThenTurnAround(uint8_t slot, uint8_t y_entry)
{
    uint8_t facing;
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xB411, 0xB3FE, false, y_entry);
    if (r.step.step != FRAME_LIST_ENDED) {
        r.arm = ST15_TURN_STEPPED;
        return r;
    }
    WR8(OBJ->frame[slot], 5);
    facing = RD8(OBJ->facing_left[slot]);
    facing = (uint8_t)(facing ^ 0x01);
    WR8(OBJ->facing_left[slot], facing);
    r.first = facing;
    r.second = INC8(OBJ->substate[slot]);
    r.arm = ST15_TURN_TURNED;
    return r;
}

/* What StepListThenTurnAround leaves (first the new facing, second the new OBJ_SUBSTATE). */
static Ended l8_turn_around_exit_shim(L8Arm r)
{
    Ended e = l8_step_ended(r.step);

    if (r.arm == ST15_TURN_STEPPED) {
        return e;
    }
    e = then(e, ended_load(5));
    e = then(e, ended_load(r.first));
    return then(e, ended_nz(r.second));
}

void abi_StepListThenTurnAround(void)
{
    abi_ended(l8_turn_around_exit_shim(StepListThenTurnAround(cpu.x, cpu.y)));
}

/*
 * @routine 05CB:49D StepListThenCountDownToState4
 * @run 70:B49D
 * WHAT IT DOES: Steps the animation list at $B375; when it ends counts OBJ_3510 down and at zero goes to state 4 with OBJ_SUBSTATE 0.
 * HOW: StepObjectFrames, a memory decrement, two stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_count_to_state4_exit_shim).
 */
enum { ST15_STATE4_STEPPED, ST15_STATE4_COUNTING, ST15_STATE4_ENTERED };

L8Arm StepListThenCountDownToState4(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xB375, 0xB4A8, false, y_entry);
    if (r.step.step != FRAME_LIST_ENDED) {
        r.arm = ST15_STATE4_STEPPED;
        return r;
    }
    r.first = DEC8(OBJ->f3510[slot]);
    if (r.first != 0) {
        r.arm = ST15_STATE4_COUNTING;
        return r;
    }
    WR8(OBJ->state[slot], 4);
    WR8(OBJ->substate[slot], 0);
    r.arm = ST15_STATE4_ENTERED;
    return r;
}

/* What StepListThenCountDownToState4 leaves (first the count). */
static Ended l8_count_to_state4_exit_shim(L8Arm r)
{
    Ended e = l8_step_ended(r.step);

    if (r.arm == ST15_STATE4_STEPPED) {
        return e;
    }
    e = then(e, ended_nz(r.first));
    if (r.arm == ST15_STATE4_COUNTING) {
        return e;
    }
    return then(e, ended_load(4));
}

void abi_StepListThenCountDownToState4(void)
{
    abi_ended(l8_count_to_state4_exit_shim(StepListThenCountDownToState4(cpu.x, cpu.y)));
}

/*
 * @routine 05CB:4D6 PickNextPhaseFromTable
 * @run 70:B4D6
 * WHAT IT DOES: Clears the animation and sets OBJ_SUBSTATE from the table at $B4EC by the low three bits of OBJ_3568, then advances OBJ_3568.
 * HOW: Stores, a table read, an INC.
 * WAITS: no.
 * Entry: X object. Exit: A the table byte, Y the index, N and Z from the new OBJ_3568 (l8_next_phase_exit_shim).
 */
L8Arm PickNextPhaseFromTable(uint8_t slot, L8Layout lay)
{
    L8Arm r = { 0 };

    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.first = (uint8_t)(RD8(OBJ->f3568[slot]) & 0x07);
    r.second = rd8((uint16_t)(l8_at(lay, 0xB4EC) + r.first));
    WR8(OBJ->substate[slot], r.second);
    r.third = INC8(OBJ->f3568[slot]);
    return r;
}

/* What PickNextPhaseFromTable leaves (first the index, second the table byte, third the new OBJ_3568). */
static Ended l8_next_phase_exit_shim(L8Arm r)
{
    return then(ended_with_y(ended_load(r.second), r.first), ended_nz(r.third));
}

void abi_PickNextPhaseFromTable(void)
{
    abi_ended(l8_next_phase_exit_shim(PickNextPhaseFromTable(cpu.x, L8_LOADED)));
}

/*
 * @routine 05CB:4F4 StepListThenLeapWithSound
 * @run 70:B4F4
 * WHAT IT DOES: Steps the animation list at $B525; when it ends plays sound $85 and gives the object a leap: the X speed from the table at $B532 by its facing and the Y speed $0600; goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, SoundCommand, table reads, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_leap_sound_exit_shim).
 */
enum { ST15_LEAP_STEPPED, ST15_LEAP_LEAPT };

L8St15Leap StepListThenLeapWithSound(uint8_t slot, uint8_t y_entry)
{
    uint8_t value;
    L8St15Leap r = { 0 };

    r.step = l8_list_step(slot, 0xB525, 0xB4FF, false, y_entry);
    if (r.step.step != FRAME_LIST_ENDED) {
        r.arm = ST15_LEAP_STEPPED;
        return r;
    }
    jsr_push(0xB506);
    r.call = SoundCommand(0x85, slot, r.step.y);
    jsr_pop();
    r.facing = RD8(OBJ->facing_left[slot]);
    r.index = (uint8_t)(r.facing << 1);
    value = rd8((uint16_t)(0xB533 + r.index));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xB532 + r.index));
    WR8(OBJ->vx_lo[slot], value);
    WR8(OBJ->vy_hi[slot], 6);
    WR8(OBJ->vy_lo[slot], 0);
    r.count = INC8(OBJ->substate[slot]);
    r.arm = ST15_LEAP_LEAPT;
    return r;
}

/* What StepListThenLeapWithSound leaves, the sound call's included. */
static SoundEnd l8_leap_sound_exit_shim(L8St15Leap r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = l8_step_ended(r.step);

    if (r.arm == ST15_LEAP_STEPPED) {
        s.end = e;
        return s;
    }
    s.sounded = true;
    s.command = 0x85;
    s.before = then(e, ended_load(0x85));
    s.call = r.call;
    s.end = then(ended_with_y(ended_with_carry(ended_load(0), r.facing >= 0x80), r.index), ended_nz(r.count));
    return s;
}

void abi_StepListThenLeapWithSound(void)
{
    abi_sound_end(l8_leap_sound_exit_shim(StepListThenLeapWithSound(cpu.x, cpu.y)));
}

/*
 * @routine 05CB:536 FlyAndSagWithTwoLists
 * @run 70:B536
 * WHAT IT DOES: Clears OBJ_3358 and steps one of two animation lists (at $B575, by the sign of the Y speed) while moving; the Y speed loses $0030 a frame; when it is negative and the Y low byte is below $78 sets OBJ_3358 2 and goes to the next OBJ_SUBSTATE.
 * HOW: A store, two ROLs, a table read, StepObjectFrames, ApplyVelocity, a 16-bit subtract in memory, a compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_fly_sag_exit_shim).
 */
enum { ST15_SAG_RISING, ST15_SAG_LOW_HIGH, ST15_SAG_DONE };

L8Arm FlyAndSagWithTwoLists(uint8_t slot, L8Layout lay)
{
    uint8_t speed, lo, value;
    Sum sum;
    L8Arm r = { 0 };

    WR8(OBJ->f3358[slot], 0);
    speed = RD8(OBJ->vy_hi[slot]);
    r.first = (uint8_t)(((speed >> 7) & 0x01) << 1);
    lo = rd8((uint16_t)(l8_at(lay, 0xB575) + r.first));
    WR8(ZP->temp10, lo);
    r.step.stored = rd8((uint16_t)(l8_at(lay, 0xB576) + r.first));
    WR8(ZP->temp11, r.step.stored);
    r.step.y = r.first;
    jsr_push(l8_at(lay, 0xB54F));
    r.step.step = StepObjectFrames(slot, &r.step.y);
    jsr_pop();
    jsr_push(l8_at(lay, 0xB552));
    r.move = ApplyVelocity(slot);
    jsr_pop();
    value = RD8(OBJ->vy_lo[slot]);
    sum = sub_bytes(value, 0x30, true);
    WR8(OBJ->vy_lo[slot], sum.value);
    value = RD8(OBJ->vy_hi[slot]);
    sum = sub_bytes(value, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    r.sum = sum;
    if ((sum.value & 0x80) == 0) {
        r.arm = ST15_SAG_RISING;
        return r;
    }
    r.second = RD8(OBJ->y_lo[slot]);
    if (r.second >= 0x78) {
        r.arm = ST15_SAG_LOW_HIGH;
        return r;
    }
    WR8(OBJ->f3358[slot], 2);
    r.third = INC8(OBJ->substate[slot]);
    r.arm = ST15_SAG_DONE;
    return r;
}

/* What FlyAndSagWithTwoLists leaves (first the list index, second the Y low byte, third the new OBJ_SUBSTATE; step the table's last byte, the step and its Y). */
static Ended l8_fly_sag_exit_shim(L8Arm r)
{
    Ended e = ended_with_y(ended_with_carry(ended_load(r.first), false), r.first);

    e = then(e, l8_step_ended(r.step));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_of_sum(r.sum));
    if (r.arm == ST15_SAG_RISING) {
        return e;
    }
    e = then(e, ended_compare(r.second, 0x78));
    if (r.arm == ST15_SAG_LOW_HIGH) {
        return e;
    }
    e = then(e, ended_load(2));
    return then(e, ended_nz(r.third));
}

void abi_FlyAndSagWithTwoLists(void)
{
    abi_ended(l8_fly_sag_exit_shim(FlyAndSagWithTwoLists(cpu.x, L8_LOADED)));
}

/*
 * @routine 05CB:58D SoundEvery32Frames
 * @run 70:B58D
 * WHAT IT DOES: When the low five bits of OBJ_34B8 are 0 plays sound $29.
 * HOW: An AND, SoundCommand.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: the AND, or the sound call's (l8_sound_every32_exit_shim).
 */
L8St15Sound SoundEvery32Frames(uint8_t slot, uint8_t y_entry)
{
    L8St15Sound r = { 0 };

    r.first = (uint8_t)(RD8(OBJ->f34b8[slot]) & 0x1F);
    if (r.first != 0) {
        r.arm = ST15_SOUND_IDLE;
        return r;
    }
    jsr_push(0xB599);
    r.call = SoundCommand(0x29, slot, y_entry);
    jsr_pop();
    r.arm = ST15_SOUND_PLAYED;
    return r;
}

/* What SoundEvery32Frames leaves, the sound call's included (first the masked count). */
static SoundEnd l8_sound_every32_exit_shim(L8St15Sound r)
{
    SoundEnd s = l8_sound_none(ended_kept());

    if (r.arm == ST15_SOUND_IDLE) {
        s.end = ended_load(r.first);
        return s;
    }
    s.sounded = true;
    s.command = 0x29;
    s.before = ended_load(0x29);
    s.call = r.call;
    return s;
}

void abi_SoundEvery32Frames(void)
{
    abi_sound_end(l8_sound_every32_exit_shim(SoundEvery32Frames(cpu.x, cpu.y)));
}

/*
 * @routine 05CB:5C4 PickRandomThenState3
 * @run 70:B5C4
 * WHAT IT DOES: Takes a random number (kept in Y as 0 to 3), clears OBJ_SUBSTATE and the animation and goes to state 3.
 * HOW: NextRandom, an AND, stores.
 * WAITS: no.
 * Entry: X object. Exit: A 3, Y the random number masked, C and V from NextRandom (l8_pick_random_exit_shim).
 */
L8Arm PickRandomThenState3(uint8_t slot, L8Layout lay)
{
    L8Arm r = { 0 };

    jsr_push(l8_at(lay, 0xB5C7));
    r.random = NextRandom();
    jsr_pop();
    r.first = r.random.value & 0x03;
    WR8(OBJ->substate[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 3);
    return r;
}

/* What PickRandomThenState3 leaves (first the random number masked). */
static Ended l8_pick_random_exit_shim(L8Arm r)
{
    Ended e = then(ended_of_random(r.random), ended_with_y(ended_load(r.first), r.first));

    return then(e, ended_load(3));
}

void abi_PickRandomThenState3(void)
{
    abi_ended(l8_pick_random_exit_shim(PickRandomThenState3(cpu.x, L8_LOADED)));
}

/*
 * @routine 05CB:59A StepListSlowDownThenPickRandom
 * @run 70:B59A
 * WHAT IT DOES: Steps the animation list at $B580 and moves the object; every eighth frame of OBJ_34B8 halves its speed and, once both speed bytes are 0 or both $FF, goes on to PickRandomThenState3.
 * HOW: StepObjectFrames, ApplyVelocity, HalveObjectVelocity, tests of OBJ_VY_HI and OBJ_VY_LO.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_slow_pick_exit_shim).
 */
enum { ST15_SLOW_BITS, ST15_SLOW_MOVING, ST15_SLOW_STOPPED_ZERO, ST15_SLOW_STOPPED_FF };

L8St15Slow StepListSlowDownThenPickRandom(uint8_t slot, uint8_t y_entry)
{
    L8St15Slow r = { 0 };

    r.step = l8_list_step(slot, 0xB580, 0xB5A5, false, y_entry);
    jsr_push(0xB5A8);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.bits = (uint8_t)(RD8(OBJ->f34b8[slot]) & 0x07);
    if (r.bits != 0) {
        r.arm = ST15_SLOW_BITS;
        return r;
    }
    jsr_push(0xB5B2);
    r.halved = HalveObjectVelocity(slot);
    jsr_pop();
    r.high = RD8(OBJ->vy_hi[slot]);
    r.low = RD8(OBJ->vy_lo[slot]);
    if ((r.high | r.low) != 0) {
        r.high2 = RD8(OBJ->vy_hi[slot]);
        r.low2 = RD8(OBJ->vy_lo[slot]);
        if ((r.high2 & r.low2) != 0xFF) {
            r.arm = ST15_SLOW_MOVING;
            return r;
        }
        r.arm = ST15_SLOW_STOPPED_FF;
    } else {
        r.arm = ST15_SLOW_STOPPED_ZERO;
    }
    r.pick = PickRandomThenState3(slot, L8_LOADED);
    return r;
}

/* What StepListSlowDownThenPickRandom leaves, the pick's included. */
static Ended l8_slow_pick_exit_shim(L8St15Slow r)
{
    Ended e = l8_step_ended(r.step);

    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_load(r.bits));
    if (r.arm == ST15_SLOW_BITS) {
        return e;
    }
    e = then(e, ended_of_arith(r.halved));
    e = then(e, ended_load((uint8_t)(r.high | r.low)));
    if (r.arm != ST15_SLOW_STOPPED_ZERO) {
        e = then(e, ended_compare((uint8_t)(r.high2 & r.low2), 0xFF));
        if (r.arm == ST15_SLOW_MOVING) {
            return e;
        }
    }
    return then(e, l8_pick_random_exit_shim(r.pick));
}

void abi_StepListSlowDownThenPickRandom(void)
{
    abi_ended(l8_slow_pick_exit_shim(StepListSlowDownThenPickRandom(cpu.x, cpu.y)));
}

/*
 * @routine 05CB:465 EdgeCodeByDirection
 * @run 70:B465
 * WHAT IT DOES: Gives 2 when the object is inside the center band (InsideCenterBand says C 0), else 1 when OBJ_3618 is 0 and 0 when it is not.
 * HOW: A load and a call to InsideCenterBand, a load of the answer.
 * WAITS: no.
 * Entry: X object. Exit: A the answer, N and Z as the load (l8_edge_code_exit_shim). The arm for OBJ_3618 not 0 outside the band ran in no script.
 */
enum { ST15_EDGE_OUT_DIRECTED, ST15_EDGE_OUT_PLAIN, ST15_EDGE_INSIDE };

L8St15Edge EdgeCodeByDirection(uint8_t slot, L8Layout lay)
{
    L8St15Edge r = { 0 };

    r.direction = RD8(OBJ->f3618[slot]);
    jsr_push(l8_at(lay, r.direction != 0 ? 0xB46D : 0xB477));
    r.band = InsideCenterBand(slot);
    jsr_pop();
    if (r.band.outside) {
        if (r.direction != 0) {
            r.arm = ST15_EDGE_OUT_DIRECTED;
            r.code = 0;
        } else {
            r.arm = ST15_EDGE_OUT_PLAIN;
            r.code = 1;
        }
    } else {
        r.arm = ST15_EDGE_INSIDE;
        r.code = 2;
    }
    return r;
}

/* What EdgeCodeByDirection leaves (code the answer in A). */
static Ended l8_edge_code_exit_shim(L8St15Edge r)
{
    Ended e = then(ended_load(r.direction), l8_inside_band_exit_shim(r.band));

    if (r.arm == ST15_EDGE_OUT_DIRECTED) {
        return then(e, ended_with_a(ended_kept(), 0));
    }
    return then(e, ended_load(r.code));
}

void abi_EdgeCodeByDirection(void)
{
    abi_ended(l8_edge_code_exit_shim(EdgeCodeByDirection(cpu.x, L8_LOADED)));
}

/*
 * @routine 05CB:3A1 TurnTowardCenterOrWait
 * @run 70:B3A1
 * WHAT IT DOES: Clears the animation, goes to the next OBJ_SUBSTATE, sets OBJ_3510 to 2 to 5 at random and OBJ_3618 to 0 or 1 by whether it is outside the center band; when the object's facing already matches FacingByX goes on once more.
 * HOW: NextRandom, InsideCenterBand, FacingByX, a ROL, a compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_turn_toward_center_exit_shim).
 */
enum { ST15_CENTER_WAITING, ST15_CENTER_MATCHED };

L8St15Turn TurnTowardCenterOrWait(uint8_t slot, L8Layout lay)
{
    L8St15Turn r = { 0 };

    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.count = INC8(OBJ->substate[slot]);
    jsr_push(l8_at(lay, 0xB3AD));
    r.random = NextRandom();
    jsr_pop();
    r.sum = add_bytes((uint8_t)(r.random.value & 0x03), 2, false);
    WR8(OBJ->f3510[slot], r.sum.value);
    jsr_push(l8_at(lay, 0xB3B8));
    r.band = InsideCenterBand(slot);
    jsr_pop();
    r.shifted = (uint8_t)((uint8_t)(r.band.a << 1) | (r.band.outside ? 1 : 0));
    r.bit = (uint8_t)(r.shifted & 0x01);
    WR8(OBJ->f3618[slot], r.bit);
    jsr_push(l8_at(lay, 0xB3C1));
    r.side = FacingByX(slot, lay);
    jsr_pop();
    r.facing = RD8(OBJ->facing_left[slot]);
    if (r.side.a != r.facing) {
        r.arm = ST15_CENTER_WAITING;
        return r;
    }
    r.count2 = INC8(OBJ->substate[slot]);
    r.arm = ST15_CENTER_MATCHED;
    return r;
}

/* What TurnTowardCenterOrWait leaves (count the first OBJ_SUBSTATE, sum the random's add, shifted the rotated byte, bit its low bit, count2 the second OBJ_SUBSTATE). */
static Ended l8_turn_toward_center_exit_shim(L8St15Turn r)
{
    Ended e = ended_nz(r.count);

    e = then(e, ended_of_random(r.random));
    e = then(e, ended_load((uint8_t)(r.random.value & 0x03)));
    e = then(e, ended_of_sum(r.sum));
    e = then(e, l8_inside_band_exit_shim(r.band));
    e = then(e, ended_with_carry(ended_load(r.shifted), (r.band.a & 0x80) != 0));
    e = then(e, ended_load(r.bit));
    e = then(e, l8_facing_by_x_exit_shim(r.side));
    e = then(e, ended_compare(r.side.a, r.facing));
    if (r.arm == ST15_CENTER_WAITING) {
        return e;
    }
    return then(e, ended_nz(r.count2));
}

void abi_TurnTowardCenterOrWait(void)
{
    abi_ended(l8_turn_toward_center_exit_shim(TurnTowardCenterOrWait(cpu.x, L8_LOADED)));
}

/*
 * @routine 05CB:41C StepSideToSideWithCenterCheck
 * @run 70:B41C
 * WHAT IT DOES: Sets the X speed from a table at $B462 by the object's facing and moves along X while it steps the animation list at $B375; when the list ends it asks EdgeCodeByDirection: 0 leaves it, 1 sets OBJ_3510 1 and advances OBJ_SUBSTATE (and once more when the side of the player matches the facing), 2 counts OBJ_3510 down first.
 * HOW: Table reads, ApplyVelocityX, StepObjectFrames, EdgeCodeByDirection, SideOfPlayer.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_side_to_side_exit_shim).
 */
enum { ST15_SIDE_STEPPED, ST15_SIDE_EDGE_NONE, ST15_SIDE_COUNTING, ST15_SIDE_TURNED, ST15_SIDE_TURNED_AGAIN };

L8St15Side StepSideToSideWithCenterCheck(uint8_t slot, uint8_t y_entry)
{
    uint8_t value;
    L8St15Side r = { 0 };

    (void)y_entry;
    r.facing = RD8(OBJ->facing_left[slot]);
    r.index = (uint8_t)(r.facing << 1);
    value = rd8((uint16_t)(0xB462 + r.index));
    WR8(OBJ->vx_hi[slot], value);
    r.value = rd8((uint16_t)(0xB462 + r.index));
    WR8(OBJ->vx_lo[slot], r.value);
    jsr_push(0xB430);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    r.step = l8_list_step(slot, 0xB375, 0xB43B, false, r.index);
    if (r.step.step != FRAME_LIST_ENDED) {
        r.arm = ST15_SIDE_STEPPED;
        return r;
    }
    jsr_push(0xB440);
    r.edge = EdgeCodeByDirection(slot, L8_LOADED);
    jsr_pop();
    if (r.edge.code == 0) {
        r.arm = ST15_SIDE_EDGE_NONE;
        return r;
    }
    if (r.edge.code != 1) {
        r.timer = DEC8(OBJ->f3510[slot]);
        if (r.timer != 0) {
            r.arm = ST15_SIDE_COUNTING;
            return r;
        }
    }
    WR8(OBJ->f3510[slot], 1);
    r.count = INC8(OBJ->substate[slot]);
    jsr_push(0xB456);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.bits = (uint8_t)(r.side.sides & 0x01);
    r.current = RD8(OBJ->facing_left[slot]);
    if (r.bits != r.current) {
        r.arm = ST15_SIDE_TURNED;
        return r;
    }
    r.count2 = INC8(OBJ->substate[slot]);
    r.arm = ST15_SIDE_TURNED_AGAIN;
    return r;
}

/* What StepSideToSideWithCenterCheck leaves (index the facing shifted, value the last table byte, timer the count, count and count2 the OBJ_2800s). */
static Ended l8_side_to_side_exit_shim(L8St15Side r)
{
    Ended e = ended_with_y(ended_with_carry(ended_load(r.index), r.facing >= 0x80), r.index);

    e = then(e, ended_load(r.value));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, l8_step_ended(r.step));
    if (r.arm == ST15_SIDE_STEPPED) {
        return e;
    }
    e = then(e, l8_edge_code_exit_shim(r.edge));
    if (r.arm == ST15_SIDE_EDGE_NONE) {
        return e;
    }
    e = then(e, ended_compare(r.edge.code, 1));
    if (r.edge.code != 1) {
        e = then(e, ended_nz(r.timer));
        if (r.arm == ST15_SIDE_COUNTING) {
            return e;
        }
    }
    e = then(e, ended_load(1));
    e = then(e, ended_nz(r.count));
    e = then(e, ended_of_side(r.side));
    e = then(e, ended_load(r.bits));
    e = then(e, ended_compare(r.bits, r.current));
    if (r.arm == ST15_SIDE_TURNED) {
        return e;
    }
    return then(e, ended_nz(r.count2));
}

void abi_StepSideToSideWithCenterCheck(void)
{
    abi_ended(l8_side_to_side_exit_shim(StepSideToSideWithCenterCheck(cpu.x, cpu.y)));
}

/* StepListFallThenMusicAgain's arms. */
enum { ST15_FALL_HIGH, ST15_FALL_LOW_SMALL, ST15_FALL_PLAYED };

/* What it ended on: the sound of the first call, the list step, the move, the Y bytes read, the track call, the flags byte and the state. */
typedef struct {
    int arm;
    L8St15Sound sound;
    L8Step step;
    AxisMove move;
    uint8_t high, low, flags, state;
    TrackPlay track;
} L8St15Fall;

/* What the exit shim of StepListFallThenMusicAgain makes: the sound, and when the music was restarted the track call and what it left. */
typedef struct {
    SoundEnd sound;
    bool played;
    TrackPlay track;
    Ended post;
} FallEnd;

/*
 * @routine 05CB:344 StepListFallThenMusicAgain
 * @run 70:B344
 * WHAT IT DOES: Plays the sound every 32 frames (SoundEvery32Frames), steps the animation list at $B375 and moves; once the Y high byte is 0 and the Y low byte is $40 or more it clears bits 3 and 4 of zField8E, plays the track again (CdPlayAgain), sets bit 2 of zField8E, goes to the next state and clears OBJ_SUBSTATE.
 * HOW: SoundEvery32Frames, StepObjectFrames, ApplyVelocity, RMW of zField8E, CdPlayAgain.
 * WAITS: no. (CdPlayAgain starts a disc command; it does not wait.)
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_fall_music_exit_shim).
 */
static L8St15Fall step_list_fall_then_music_again(uint8_t slot, uint8_t y_entry)
{
    L8St15Fall r = { 0 };

    jsr_push(0xB347);
    r.sound = SoundEvery32Frames(slot, y_entry);
    jsr_pop();
    r.step = l8_list_step(slot, 0xB375, 0xB352, false, y_entry);
    jsr_push(0xB355);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.high = RD8(OBJ->y_hi[slot]);
    if (r.high != 0) {
        r.arm = ST15_FALL_HIGH;
        return r;
    }
    r.low = RD8(OBJ->y_lo[slot]);
    if (r.low < 0x40) {
        r.arm = ST15_FALL_LOW_SMALL;
        return r;
    }
    RMW8(ZP->field8e, old, old & 0xF7);
    RMW8(ZP->field8e, old, old & 0xEF);
    jsr_push(0xB368);
    r.track = CdPlayAgain(slot, r.step.y);
    jsr_pop();
    r.flags = RD8(ZP->field8e);
    r.flags = (uint8_t)(r.flags | 0x04);
    WR8(ZP->field8e, r.flags);
    r.state = INC8(OBJ->state[slot]);
    WR8(OBJ->substate[slot], 0);
    r.arm = ST15_FALL_PLAYED;
    return r;
}

/* What StepListFallThenMusicAgain leaves, from the arm it ended on. */
static FallEnd l8_fall_music_exit_shim(L8St15Fall r)
{
    FallEnd f = { 0 };
    Ended e = l8_step_ended(r.step);

    f.sound = l8_sound_every32_exit_shim(r.sound);
    f.post = ended_kept();
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_load(r.high));
    if (r.arm != ST15_FALL_HIGH) {
        e = then(e, ended_compare(r.low, 0x40));
    }
    f.sound = sound_end_then(f.sound, e);
    if (r.arm == ST15_FALL_PLAYED) {
        f.played = true;
        f.track = r.track;
        f.post = then(ended_load(r.flags), ended_nz(r.state));
    }
    return f;
}

void abi_StepListFallThenMusicAgain(void)
{
    FallEnd r = l8_fall_music_exit_shim(step_list_fall_then_music_again(cpu.x, cpu.y));

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
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_sound_spawn_exit_shim).
 */
enum { ST15_SPAWN_ENDED, ST15_SPAWN_SHORT, ST15_SPAWN_BITS, ST15_SPAWN_NO_SLOT, ST15_SPAWN_STARTED };

L8St15Spawn StepListThenSoundAndSpawn(uint8_t slot, uint8_t y_entry)
{
    L8St15Spawn r = { 0 };

    r.step = l8_list_step(slot, 0xB62B, 0xB5F9, false, y_entry);
    if (r.step.step == FRAME_LIST_ENDED) {
        r.pick = PickRandomThenState3(slot, L8_LOADED);
        r.arm = ST15_SPAWN_ENDED;
        return r;
    }
    if (r.step.step == FRAME_CHANGED) {
        r.checked = true;
        r.anim1 = RD8(OBJ->anim_step[slot]);
        if (r.anim1 == 0x0A) {
            r.sounded = true;
            jsr_push(0xB60C);
            r.call = SoundCommand(0x2A, slot, r.step.y);
            jsr_pop();
        }
    }
    r.anim2 = RD8(OBJ->anim_step[slot]);
    if (r.anim2 < 0x0A) {
        r.arm = ST15_SPAWN_SHORT;
        return r;
    }
    r.bits = (uint8_t)(RD8(OBJ->f34b8[slot]) & 0x03);
    if (r.bits != 0) {
        r.arm = ST15_SPAWN_BITS;
        return r;
    }
    jsr_push(0xB61D);
    r.found.slot = FindFreeSlotFrom20(&r.found.last, &r.found.found);
    jsr_pop();
    if (!r.found.found) {
        r.arm = ST15_SPAWN_NO_SLOT;
        return r;
    }
    jsr_push(0xB624);
    r.placed = StartObjectAtPlace(0x25, slot, r.found.slot);
    jsr_pop();
    r.facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[r.found.slot], r.facing);
    r.arm = ST15_SPAWN_STARTED;
    return r;
}

/* What StepListThenSoundAndSpawn leaves, the sound call's included. */
static SoundEnd l8_sound_spawn_exit_shim(L8St15Spawn r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = l8_step_ended(r.step);

    if (r.arm == ST15_SPAWN_ENDED) {
        s.end = then(e, l8_pick_random_exit_shim(r.pick));
        return s;
    }
    if (r.checked) {
        e = then(e, ended_compare(r.anim1, 0x0A));
        if (r.sounded) {
            s.sounded = true;
            s.command = 0x2A;
            s.before = then(e, ended_load(0x2A));
            s.call = r.call;
            e = ended_kept();
        }
    }
    e = then(e, ended_compare(r.anim2, 0x0A));
    if (r.arm == ST15_SPAWN_SHORT) {
        s.end = e;
        return s;
    }
    e = then(e, ended_load(r.bits));
    if (r.arm == ST15_SPAWN_BITS) {
        s.end = e;
        return s;
    }
    e = then(e, l8_slot_ended(r.found));
    if (r.arm == ST15_SPAWN_NO_SLOT) {
        s.end = e;
        return s;
    }
    e = then(e, ended_load(r.placed));
    s.end = then(e, ended_load(r.facing));
    return s;
}

void abi_StepListThenSoundAndSpawn(void)
{
    abi_sound_end(l8_sound_spawn_exit_shim(StepListThenSoundAndSpawn(cpu.x, cpu.y)));
}

/*
 * @routine 05CB:648 StepListThenBurstAtFrame14
 * @run 70:B648
 * WHAT IT DOES: Steps the animation list at $B675; when it ends goes to PickRandomThenState3; when the graphic changes and the frame is $14 plays sound $86 and starts an object of type $28 in slot $13 facing as this one.
 * HOW: StepObjectFrames, SoundCommand, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_sound_burst_exit_shim).
 */
enum { ST15_BURST_ENDED, ST15_BURST_UNCHANGED, ST15_BURST_OTHER_FRAME, ST15_BURST_STARTED };

L8St15Burst StepListThenBurstAtFrame14(uint8_t slot, uint8_t y_entry)
{
    L8St15Burst r = { 0 };

    r.step = l8_list_step(slot, 0xB675, 0xB653, false, y_entry);
    if (r.step.step == FRAME_LIST_ENDED) {
        r.pick = PickRandomThenState3(slot, L8_LOADED);
        r.arm = ST15_BURST_ENDED;
        return r;
    }
    if (r.step.step != FRAME_CHANGED) {
        r.arm = ST15_BURST_UNCHANGED;
        return r;
    }
    r.frame = RD8(OBJ->frame[slot]);
    if (r.frame != 0x14) {
        r.arm = ST15_BURST_OTHER_FRAME;
        return r;
    }
    jsr_push(0xB663);
    r.call = SoundCommand(0x86, slot, r.step.y);
    jsr_pop();
    jsr_push(0xB66A);
    r.placed = StartObjectAtPlace(0x28, slot, 0x13);
    jsr_pop();
    r.facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[0x13], r.facing);
    r.arm = ST15_BURST_STARTED;
    return r;
}

/* What StepListThenBurstAtFrame14 leaves, the sound call's included. */
static SoundEnd l8_sound_burst_exit_shim(L8St15Burst r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = l8_step_ended(r.step);

    if (r.arm == ST15_BURST_ENDED) {
        s.end = then(e, l8_pick_random_exit_shim(r.pick));
        return s;
    }
    if (r.arm == ST15_BURST_UNCHANGED) {
        s.end = e;
        return s;
    }
    e = then(e, ended_compare(r.frame, 0x14));
    if (r.arm == ST15_BURST_OTHER_FRAME) {
        s.end = e;
        return s;
    }
    s.sounded = true;
    s.command = 0x86;
    s.before = then(e, ended_load(0x86));
    s.call = r.call;
    s.end = then(then(ended_with_y(ended_nz(0x13), 0x13), ended_load(r.placed)), ended_load(r.facing));
    return s;
}

void abi_StepListThenBurstAtFrame14(void)
{
    abi_sound_end(l8_sound_burst_exit_shim(StepListThenBurstAtFrame14(cpu.x, cpu.y)));
}
