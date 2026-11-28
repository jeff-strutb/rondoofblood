/*
 * TEMPORARY: the stage object routines of lot L8 for the overlays at sectors 0F6C and 0F6D (enemy kinds not yet identified), by routine. Bodies return the
 * arm they ended on and the values behind it; each routine's exit shim builds how its last instructions left A, Y and the flags (Ended,
 * enemy_objects.h), and the abi_ wrappers show it.
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

/* The arms the routines end on (L8Arm, or the routine's own struct). */
enum { ST25_BURST_LOADED, ST25_BURST_NOT_FLAGGED, ST25_BURST_STARTED };
enum { ST25_PIECE_FALLING, ST25_PIECE_OFF_FRAME, ST25_PIECE_NONE_FREE, ST25_PIECE_STARTED };
enum { ST25_HURT_FLAGGED, ST25_HURT_UNMARKED, ST25_HURT_FEW_HITS, ST25_HURT_SOUNDED };
enum { ST25_ENTER_UNFLAGGED, ST25_ENTER_IN_0E, ST25_ENTER_ENTERED };
enum { ST25_READY_LOW_STATE, ST25_READY_AT_5C, ST25_READY_STARTED };
enum { ST25_ONCE_NOT_ONE, ST25_ONCE_AT_9A, ST25_ONCE_BUSY, ST25_ONCE_YES };
enum { ST25_WALK_STEPPED, ST25_WALK_SET, ST25_WALK_REVERSED };
enum { ST25_LEAP_NO_AIM, ST25_LEAP_FEW_HITS, ST25_LEAP_SET, ST25_LEAP_REVERSED };

/*
 * @routine 0F6D:365 StepListByNumberTableC585
 * @run 71:C365
 * WHAT IT DOES: Steps the animation list that the table at $C585 names for OBJ_34B8.
 * HOW: A table read of two bytes into zTemp10-11, a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step (l8_number_step_c585_exit_shim).
 */
L8NumberStep StepListByNumberTableC585(uint8_t slot, uint8_t y_entry)
{
    L8NumberStep r = { 0 };

    (void)y_entry;
    r.number = RD8(OBJ->f34b8[slot]);
    r.index = (uint8_t)(r.number << 1);
    r.lo = rd8((uint16_t)(0xC585 + r.index));
    WR8(ZP->temp10, r.lo);
    r.hi = rd8((uint16_t)(0xC586 + r.index));
    WR8(ZP->temp11, r.hi);
    r.y = r.index;
    r.step = StepObjectFrames(slot, &r.y);
    return r;
}

/* What StepListByNumberTableC585 leaves in A, Y and the flags: one path, built as StepListByNumberTable's. */
Ended l8_number_step_c585_exit_shim(L8NumberStep r)
{
    return l8_number_step_exit_shim(r);
}

void abi_StepListByNumberTableC585(void)
{
    abi_ended(l8_number_step_c585_exit_shim(StepListByNumberTableC585(cpu.x, cpu.y)));
}

/*
 * @routine 0F6D:35F ClearAnimationThenStepListC585
 * @run 71:C35F
 * WHAT IT DOES: Clears the animation step and timer and runs StepListByNumberTableC585.
 * HOW: Two stores, running into the next routine.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as StepListByNumberTableC585 (l8_number_step_c585_exit_shim).
 */
L8NumberStep ClearAnimationThenStepListC585(uint8_t slot, uint8_t y_entry)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    return StepListByNumberTableC585(slot, y_entry);
}

void abi_ClearAnimationThenStepListC585(void)
{
    abi_ended(l8_number_step_c585_exit_shim(ClearAnimationThenStepListC585(cpu.x, cpu.y)));
}

/*
 * @routine 0F6D:377 ToggleFacing
 * @run 71:C377
 * WHAT IT DOES: Turns the object round.
 * HOW: An EOR of bit 0 and a store.
 * WAITS: no.
 * Entry: X object. Exit: A the new facing, N and Z from it (l8_toggle_facing_exit_shim).
 */
uint8_t ToggleFacing(uint8_t slot)
{
    uint8_t facing, value;

    facing = RD8(OBJ->facing_left[slot]);
    value = (uint8_t)(facing ^ 0x01);
    value = (uint8_t)(value & 0x01);
    WR8(OBJ->facing_left[slot], value);
    return value;
}

/* What ToggleFacing leaves: one path, the facing stored. */
Ended l8_toggle_facing_exit_shim(uint8_t value)
{
    return ended_load(value);
}

void abi_ToggleFacing(void)
{
    abi_ended(l8_toggle_facing_exit_shim(ToggleFacing(cpu.x)));
}

/*
 * @routine 0F6D:3F3 LoadRecordOrBurstWhenFlagged
 * @run 71:C3F3
 * WHAT IT DOES: In state 0 loads the record. Later, when bit 7 of OBJ_HIT_STATUS is set, plays sound $16, clears slots $52 to $54 and starts objects of type $AE in $52 and $53 (the second with OBJ_START_ARG 1) and of type $9C in $54 at its place, then clears itself.
 * HOW: LoadObjectRecord, SoundCommand, ClearObjectInY and StartObjectAtPlace three times, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_burst_exit_shim). The burst ran in no script; the draft stops there.
 */
L8BurstArm LoadRecordOrBurstWhenFlagged(uint8_t slot, uint8_t y_entry)
{
    uint8_t y;
    L8BurstArm r = { 0 };

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        jsr_push(0xC3FB);
        r.record = l8_record(slot);
        jsr_pop();
        r.arm = ST25_BURST_LOADED;
        return r;
    }
    r.flags = RD8(OBJ->hit_status[slot]);
    if ((r.flags & 0x80) == 0) {
        r.arm = ST25_BURST_NOT_FLAGGED;
        return r;
    }
    jsr_push(0xC408);
    r.call = SoundCommand(0x16, slot, y_entry);
    jsr_pop();
    y = 0x52;
    jsr_push(0xC40D);
    ClearObjectInY(y);
    jsr_pop();
    jsr_push(0xC412);
    r.placed = StartObjectAtPlace(0xAE, slot, y);
    jsr_pop();
    y++;
    jsr_push(0xC416);
    ClearObjectInY(y);
    jsr_pop();
    jsr_push(0xC41B);
    r.placed = StartObjectAtPlace(0xAE, slot, y);
    jsr_pop();
    WR8(OBJ->f3040[y], 1);
    y++;
    jsr_push(0xC424);
    ClearObjectInY(y);
    jsr_pop();
    jsr_push(0xC429);
    r.placed = StartObjectAtPlace(0x9C, slot, y);
    jsr_pop();
    ClearObject(slot);
    r.y = y;
    r.arm = ST25_BURST_STARTED;
    return r;
}

/* What LoadRecordOrBurstWhenFlagged leaves, the sound call's included, from the arm it ended on. */
static SoundEnd l8_burst_exit_shim(L8BurstArm a)
{
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e = ended_load(a.state);

    if (a.arm == ST25_BURST_LOADED) {
        r.end = then(e, l8_record_ended(a.record));
        return r;
    }
    e = then(e, ended_load((uint8_t)(a.flags & 0x80)));
    if (a.arm == ST25_BURST_NOT_FLAGGED) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0x16;
    r.before = then(e, ended_load(0x16));
    r.call = a.call;
    r.end = ended_with_y(ended_load(a.placed), a.y);
    return r;
}

void abi_LoadRecordOrBurstWhenFlagged(void)
{
    abi_sound_end(l8_burst_exit_shim(LoadRecordOrBurstWhenFlagged(cpu.x, cpu.y)));
}

/*
 * @routine 0F6C:0D5 SpawnFlashingPieceWhenRising
 * @run 70:B8D5
 * WHAT IT DOES: A rising object (Y speed high bit clear) shows flash flags $10; every fourth frame it starts an object of type $9B in a free slot of $20 to $3F, with Y speed $0800 (moved once) and an X speed from the table at $B90D by its own OBJ_START_ARG (moved once) and flash flags $10.
 * HOW: A test, TST of the wave counter, FindFreeSlotFrom20, StartObjectAtPlace, ApplyVelocityY and X with X and Y swapped.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_flash_piece_exit_shim).
 */
L8FlashPieceArm SpawnFlashingPieceWhenRising(uint8_t slot)
{
    uint8_t kind, value;
    L8FlashPieceArm r = { 0 };

    r.speed = RD8(OBJ->vy_hi[slot]);
    if ((r.speed & 0x80) != 0) {
        r.arm = ST25_PIECE_FALLING;
        return r;
    }
    WR8(OBJ->sprite_attr[slot], 0x10);
    r.wave = RD8(ZP->wave_counter);
    if ((r.wave & 0x03) != 0) {
        r.arm = ST25_PIECE_OFF_FRAME;
        return r;
    }
    jsr_push(0xB8E7);
    r.found.slot = FindFreeSlotFrom20(&r.found.last, &r.found.found);
    jsr_pop();
    if (!r.found.found) {
        r.arm = ST25_PIECE_NONE_FREE;
        return r;
    }
    jsr_push(0xB8EE);
    StartObjectAtPlace(0x9B, slot, r.found.slot);
    jsr_pop();
    WR8(OBJ->vy_hi[r.found.slot], 8);
    jsr_push(0xB8F7);
    r.move1 = ApplyVelocityY(r.found.slot);
    jsr_pop();
    push8(slot);
    kind = RD8(OBJ->f3040[slot]);
    value = rd8((uint16_t)(0xB90D + kind));
    WR8(OBJ->vx_hi[r.found.slot], value);
    jsr_push(0xB905);
    r.move2 = ApplyVelocityX(r.found.slot);
    jsr_pop();
    slot = pull8();
    WR8(OBJ->sprite_attr[r.found.slot], 0x10);
    r.arm = ST25_PIECE_STARTED;
    return r;
}

/* What SpawnFlashingPieceWhenRising leaves in A, Y and the flags, from the arm it ended on. */
Ended l8_flash_piece_exit_shim(L8FlashPieceArm r)
{
    Ended e = ended_load(r.speed);

    if (r.arm == ST25_PIECE_FALLING) {
        return e;
    }
    e = then(e, ended_load(0x10));
    e = then(e, ended_with_overflow(ended_nz((uint8_t)((r.wave & 0x80) | ((r.wave & 0x03) ? 1 : 0))), (r.wave & 0x40) != 0));
    if (r.arm == ST25_PIECE_OFF_FRAME) {
        return e;
    }
    e = then(e, l8_slot_ended(r.found));
    if (r.arm == ST25_PIECE_NONE_FREE) {
        return e;
    }
    e = then(then(e, ended_load(8)), ended_of_axis_move(r.move1));
    e = then(e, ended_of_axis_move(r.move2));
    return then(e, ended_with_y(ended_load(0x10), r.found.slot));
}

void abi_SpawnFlashingPieceWhenRising(void)
{
    abi_ended(l8_flash_piece_exit_shim(SpawnFlashingPieceWhenRising(cpu.x)));
}

/*
 * @routine 0F6C:111 CountDownThenClearOnTimer
 * @run 70:B911
 * WHAT IT DOES: Counts OBJ_3510 down; at zero clears the object.
 * HOW: A memory decrement and a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: the decrement's N and Z (l8_clear_on_timer_exit_shim).
 */
uint8_t CountDownThenClearOnTimer(uint8_t slot)
{
    uint8_t timer = DEC8(OBJ->f3510[slot]);

    if (timer == 0) {
        ClearObject(slot);
    }
    return timer;
}

/* What CountDownThenClearOnTimer leaves: one path, the timer after the DEC behind N and Z. */
Ended l8_clear_on_timer_exit_shim(uint8_t timer)
{
    return ended_nz(timer);
}

void abi_CountDownThenClearOnTimer(void)
{
    abi_ended(l8_clear_on_timer_exit_shim(CountDownThenClearOnTimer(cpu.x)));
}

/*
 * @routine 0F6C:3F0 FlashWhenHurtThenSound
 * @run 70:BBF0
 * WHAT IT DOES: Unless bit 7 of OBJ_HIT_STATUS is set, in the states the table at $BBB0 marks it clears its flash flags, sets them $10 when OBJ_INVULN_TIMER is not 0, copies OBJ_HEALTH to slot $10's, and at 5 hits plays sound $D6, doubles OBJ_INVULN_TIMER twice, goes to state $0D and clears OBJ_SUBSTATE.
 * HOW: Tests, a table read by the state, stores, SoundCommand, two ASLs in memory.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_flash_hurt_exit_shim). The arm past the table test ran in no script; the draft stops there.
 */
L8FlashHurtArm FlashWhenHurtThenSound(uint8_t slot, uint8_t y_entry)
{
    L8FlashHurtArm r = { 0 };

    (void)y_entry;
    r.flags = RD8(OBJ->hit_status[slot]);
    if ((r.flags & 0x80) != 0) {
        r.arm = ST25_HURT_FLAGGED;
        return r;
    }
    r.state = RD8(OBJ->state[slot]);
    r.marked = rd8((uint16_t)(0xBBB0 + r.state));
    if (r.marked == 0) {
        r.arm = ST25_HURT_UNMARKED;
        return r;
    }
    WR8(OBJ->sprite_attr[slot], 0);
    r.timer = RD8(OBJ->invuln_timer[slot]);
    if (r.timer != 0) {
        WR8(OBJ->sprite_attr[slot], 0x10);
        r.flashed = true;
    }
    r.value = RD8(OBJ->health[slot]);
    WR8(OBJ->health[0x10], r.value);
    r.hits = RD8(OBJ->hit_taken[slot]);
    if (r.hits < 5) {
        r.arm = ST25_HURT_FEW_HITS;
        return r;
    }
    jsr_push(0xBC1E);
    r.call = SoundCommand(0xD6, slot, r.state);
    jsr_pop();
    ASL8(OBJ->invuln_timer[slot]);
    r.shifted = ASL8(OBJ->invuln_timer[slot]);
    WR8(OBJ->state[slot], 0x0D);
    WR8(OBJ->substate[slot], 0);
    r.arm = ST25_HURT_SOUNDED;
    return r;
}

/* What FlashWhenHurtThenSound leaves, the sound call's included, from the arm it ended on. */
SoundEnd l8_flash_hurt_exit_shim(L8FlashHurtArm a)
{
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e = ended_load((uint8_t)(a.flags & 0x80));

    if (a.arm == ST25_HURT_FLAGGED) {
        r.end = e;
        return r;
    }
    e = then(e, ended_with_y(ended_nz(a.state), a.state));
    e = then(e, ended_load(a.marked));
    if (a.arm == ST25_HURT_UNMARKED) {
        r.end = e;
        return r;
    }
    e = then(e, ended_load(a.timer));
    if (a.flashed) {
        e = then(e, ended_load(0x10));
    }
    e = then(e, ended_load(a.value));
    e = then(e, ended_compare(a.hits, 5));
    if (a.arm == ST25_HURT_FEW_HITS) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0xD6;
    r.before = then(e, ended_load(0xD6));
    r.call = a.call;
    r.end = ended_with_carry(ended_load(0x0D), a.shifted != 0);
    return r;
}

void abi_FlashWhenHurtThenSound(void)
{
    abi_sound_end(l8_flash_hurt_exit_shim(FlashWhenHurtThenSound(cpu.x, cpu.y)));
}

/*
 * @routine 0F6C:42D EnterState0EWhenFlagged
 * @run 70:BC2D
 * WHAT IT DOES: When bit 7 of OBJ_HIT_STATUS is set and the state is not $0E, copies OBJ_HEALTH to slot $10's, clears OBJ_3250, goes to state $0E and clears OBJ_SUBSTATE.
 * HOW: Two tests, stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_enter_0e_exit_shim). The arm past the flag test ran in no script; the draft stops there.
 */
L8Arm EnterState0EWhenFlagged(uint8_t slot)
{
    uint8_t value;
    L8Arm r = { 0 };

    r.first = RD8(OBJ->hit_status[slot]);
    if ((r.first & 0x80) == 0) {
        r.arm = ST25_ENTER_UNFLAGGED;
        return r;
    }
    r.second = RD8(OBJ->state[slot]);
    if (r.second == 0x0E) {
        r.arm = ST25_ENTER_IN_0E;
        return r;
    }
    value = RD8(OBJ->health[slot]);
    WR8(OBJ->health[0x10], value);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->state[slot], 0x0E);
    WR8(OBJ->substate[slot], 0);
    r.arm = ST25_ENTER_ENTERED;
    return r;
}

/* What EnterState0EWhenFlagged leaves in A and the flags, from the arm it ended on (first the flags, second the state). */
Ended l8_enter_0e_exit_shim(L8Arm r)
{
    Ended e = ended_load((uint8_t)(r.first & 0x80));

    if (r.arm == ST25_ENTER_UNFLAGGED) {
        return e;
    }
    e = then(e, ended_compare(r.second, 0x0E));
    if (r.arm == ST25_ENTER_IN_0E) {
        return e;
    }
    return then(e, ended_load(0x0E));
}

void abi_EnterState0EWhenFlagged(void)
{
    abi_ended(l8_enter_0e_exit_shim(EnterState0EWhenFlagged(cpu.x)));
}

/*
 * @routine 0F6C:44D LoadRecordFrame24
 * @run 70:BC4D
 * WHAT IT DOES: Loads the object's record, sets its frame $24 and clears OBJ_3250.
 * HOW: LoadObjectRecord and two stores.
 * WAITS: no.
 * Entry: X object. Exit: A $24, Y 11, C from the record (l8_frame24_exit_shim).
 */
L8Record LoadRecordFrame24(uint8_t slot)
{
    L8Record r;

    jsr_push(0xBC50);
    r = l8_record(slot);
    jsr_pop();
    WR8(OBJ->frame[slot], 0x24);
    WR8(OBJ->hitbox_on[slot], 0);
    return r;
}

/* What LoadRecordFrame24 leaves: one path, the record's and A $24. */
static Ended l8_frame24_exit_shim(L8Record r)
{
    return then(l8_record_ended(r), ended_load(0x24));
}

void abi_LoadRecordFrame24(void)
{
    abi_ended(l8_frame24_exit_shim(LoadRecordFrame24(cpu.x)));
}

/*
 * @routine 0F6C:459 StartTimerWhenSlot10Ready
 * @run 70:BC59
 * WHAT IT DOES: When the state of slot $10 is 5 or more and its OBJ_HEALTH is not $5C, sets OBJ_34B8 $0A and goes to the next state.
 * HOW: Two compares, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_slot10_ready_exit_shim).
 */
L8Arm StartTimerWhenSlot10Ready(uint8_t slot)
{
    L8Arm r = { 0 };

    r.first = RD8(OBJ->state[0x10]);
    if (r.first < 5) {
        r.arm = ST25_READY_LOW_STATE;
        return r;
    }
    r.second = RD8(OBJ->health[0x10]);
    if (r.second == 0x5C) {
        r.arm = ST25_READY_AT_5C;
        return r;
    }
    WR8(OBJ->f34b8[slot], 0x0A);
    r.third = INC8(OBJ->state[slot]);
    r.arm = ST25_READY_STARTED;
    return r;
}

/* What StartTimerWhenSlot10Ready leaves in A and the flags, from the arm it ended on (first slot $10's state, second its OBJ_HEALTH, third the new state). */
static Ended l8_slot10_ready_exit_shim(L8Arm r)
{
    Ended e = ended_compare(r.first, 5);

    if (r.arm == ST25_READY_LOW_STATE) {
        return e;
    }
    e = then(e, ended_compare(r.second, 0x5C));
    if (r.arm == ST25_READY_AT_5C) {
        return e;
    }
    e = then(e, ended_load(0x0A));
    return then(e, ended_nz(r.third));
}

void abi_StartTimerWhenSlot10Ready(void)
{
    abi_ended(l8_slot10_ready_exit_shim(StartTimerWhenSlot10Ready(cpu.x)));
}

/*
 * @routine 0F6C:580 HurtOnceThenAnswerYes
 * @run 70:BD80
 * WHAT IT DOES: Says yes (C 1) when the object has taken exactly 1 hit, the player's Y low byte is not $9A and slot 0's state is below $10: it then clears its hits and OBJ_3250, sets slot 0's OBJ_3510 $40 and OBJ_3568 $10, OBJ_34B8 $0B, restarts its animation (ClearAnimationThenStepListC585), goes to state 8. Otherwise says no (C 0).
 * HOW: Compares, stores, a call.
 * WAITS: no.
 * Entry: X object, Y kept for the call. Exit: C 1 yes, 0 no (l8_hurt_once_exit_shim). The yes arm ran in no script; the draft stops there.
 */
L8HurtOnceArm HurtOnceThenAnswerYes(uint8_t slot, uint8_t y_entry)
{
    L8HurtOnceArm r = { 0 };

    r.hits = RD8(OBJ->hit_taken[slot]);
    if (r.hits != 1) {
        r.arm = ST25_ONCE_NOT_ONE;
        return r;
    }
    r.ground = RD8(OBJ->y_lo[0]);
    if (r.ground == 0x9A) {
        r.arm = ST25_ONCE_AT_9A;
        return r;
    }
    r.state = RD8(OBJ->state[0]);
    if (r.state >= 0x10) {
        r.arm = ST25_ONCE_BUSY;
        return r;
    }
    WR8(OBJ->hit_taken[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->f3510[0], 0x40);
    WR8(OBJ->f3568[0], 0x10);
    WR8(OBJ->f34b8[slot], 0x0B);
    jsr_push(0xBDAD);
    r.step = ClearAnimationThenStepListC585(slot, y_entry);
    jsr_pop();
    WR8(OBJ->state[slot], 8);
    r.yes = true;
    r.arm = ST25_ONCE_YES;
    return r;
}

/* What HurtOnceThenAnswerYes leaves in A, Y and the flags, from the arm it ended on. */
Ended l8_hurt_once_exit_shim(L8HurtOnceArm r)
{
    Ended e = ended_compare(r.hits, 1);

    if (r.arm == ST25_ONCE_NOT_ONE) {
        return ended_with_carry(e, false);
    }
    e = then(e, ended_compare(r.ground, 0x9A));
    if (r.arm == ST25_ONCE_AT_9A) {
        return ended_with_carry(e, false);
    }
    e = then(e, ended_compare(r.state, 0x10));
    if (r.arm == ST25_ONCE_BUSY) {
        return ended_with_carry(e, false);
    }
    e = then(e, l8_number_step_c585_exit_shim(r.step));
    return ended_with_carry(then(e, ended_load(8)), true);
}

void abi_HurtOnceThenAnswerYes(void)
{
    abi_ended(l8_hurt_once_exit_shim(HurtOnceThenAnswerYes(cpu.x, cpu.y)));
}

/*
 * @routine 0F6C:470 StepListThenStartPieceAndWalk
 * @run 70:BC70
 * WHAT IT DOES: Runs StepListByNumberTableC585; when the list ends clears slot $51 and starts an object of type $AD there at the object's place, sets OBJ_3250 2, state 3 and OBJ_34B8 0, restarts the animation, sets the X speed -$00A0 and, facing left, reverses it.
 * HOW: Two calls, ClearObjectInY, StartObjectAtPlace, stores, a tail jump into NegateVelocityX.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_piece_walk_exit_shim). Reversing the speed ran in no script; the draft stops there.
 */
L8PieceWalkArm StepListThenStartPieceAndWalk(uint8_t slot, uint8_t y_entry)
{
    L8PieceWalkArm r = { 0 };

    jsr_push(0xBC73);
    r.step1 = StepListByNumberTableC585(slot, y_entry);
    jsr_pop();
    if (r.step1.step != FRAME_LIST_ENDED) {
        r.arm = ST25_WALK_STEPPED;
        return r;
    }
    jsr_push(0xBC7A);
    ClearObjectInY(0x51);
    jsr_pop();
    jsr_push(0xBC7F);
    r.placed = StartObjectAtPlace(0xAD, slot, 0x51);
    jsr_pop();
    WR8(OBJ->hitbox_on[slot], 2);
    WR8(OBJ->state[slot], 3);
    WR8(OBJ->f34b8[slot], 0);
    jsr_push(0xBC91);
    r.step2 = ClearAnimationThenStepListC585(slot, 0x51);
    jsr_pop();
    WR8(OBJ->vx_lo[slot], 0x60);
    WR8(OBJ->vx_hi[slot], 0xFF);
    r.facing = RD8(OBJ->facing_left[slot]);
    if (r.facing != 0) {
        r.negated = NegateVelocityX(slot);
        r.arm = ST25_WALK_REVERSED;
        return r;
    }
    r.arm = ST25_WALK_SET;
    return r;
}

/* What StepListThenStartPieceAndWalk leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_piece_walk_exit_shim(L8PieceWalkArm r)
{
    Ended e = l8_number_step_c585_exit_shim(r.step1);

    if (r.arm == ST25_WALK_STEPPED) {
        return e;
    }
    e = then(then(e, ended_with_y(ended_nz(0x51), 0x51)), ended_load(r.placed));
    e = then(e, ended_load(0));
    e = then(e, l8_number_step_c585_exit_shim(r.step2));
    e = then(e, ended_load(r.facing));
    if (r.arm == ST25_WALK_REVERSED) {
        e = then(e, ended_of_sum(r.negated));
    }
    return e;
}

void abi_StepListThenStartPieceAndWalk(void)
{
    abi_ended(l8_piece_walk_exit_shim(StepListThenStartPieceAndWalk(cpu.x, cpu.y)));
}

/*
 * @routine 0F6C:6F7 StartLeapWhenHurtAnswerYes
 * @run 70:BEF7
 * WHAT IT DOES: Says no (C 0) when OBJ_35C0 is 0 or the object has taken fewer than 5 hits. Otherwise clears slot $11, starts an object of type $B0 there at its place, gives it the Y speed -$0800 (moved once), clears its hits and OBJ_3250, sets OBJ_34B8 $0E, restarts the animation, sets the speed X $0140 and Y -$0380 (reversing X when facing left), goes to state 9 and says yes (C 1).
 * HOW: Tests, ClearObjectInY, StartObjectAtPlace, ApplyVelocityY, ClearAnimationThenStepListC585, NegateVelocityX.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: C 1 yes, 0 no (l8_leap_yes_exit_shim). The yes arm ran in no script; the draft stops there.
 */
L8LeapYesArm StartLeapWhenHurtAnswerYes(uint8_t slot, uint8_t y_entry)
{
    L8LeapYesArm r = { 0 };

    (void)y_entry;
    r.aim = RD8(OBJ->f35c0[slot]);
    if (r.aim == 0) {
        r.arm = ST25_LEAP_NO_AIM;
        return r;
    }
    r.hits = RD8(OBJ->hit_taken[slot]);
    if (r.hits < 5) {
        r.arm = ST25_LEAP_FEW_HITS;
        return r;
    }
    jsr_push(0xBF08);
    ClearObjectInY(0x11);
    jsr_pop();
    jsr_push(0xBF0D);
    r.placed = StartObjectAtPlace(0xB0, slot, 0x11);
    jsr_pop();
    WR8(OBJ->vy_hi[slot], 0xF8);
    jsr_push(0xBF15);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    WR8(OBJ->hit_taken[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->f34b8[slot], 0x0E);
    jsr_push(0xBF23);
    r.step = ClearAnimationThenStepListC585(slot, 0x11);
    jsr_pop();
    WR8(OBJ->vy_lo[slot], 0x80);
    WR8(OBJ->vy_hi[slot], 0xFC);
    WR8(OBJ->vx_lo[slot], 0x40);
    WR8(OBJ->vx_hi[slot], 0x01);
    r.facing = RD8(OBJ->facing_left[slot]);
    r.arm = ST25_LEAP_SET;
    if (r.facing != 0) {
        jsr_push(0xBF3F);
        r.negated = NegateVelocityX(slot);
        jsr_pop();
        r.arm = ST25_LEAP_REVERSED;
    }
    WR8(OBJ->state[slot], 9);
    r.yes = true;
    return r;
}

/* What StartLeapWhenHurtAnswerYes leaves in A, Y and the flags, from the arm it ended on. */
Ended l8_leap_yes_exit_shim(L8LeapYesArm r)
{
    Ended e = ended_load(r.aim);

    if (r.arm == ST25_LEAP_NO_AIM) {
        return ended_with_carry(e, false);
    }
    e = then(e, ended_compare(r.hits, 5));
    if (r.arm == ST25_LEAP_FEW_HITS) {
        return ended_with_carry(e, false);
    }
    e = then(then(e, ended_with_y(ended_nz(0x11), 0x11)), ended_load(r.placed));
    e = then(e, ended_load(0xF8));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_load(0x0E));
    e = then(e, l8_number_step_c585_exit_shim(r.step));
    e = then(e, ended_load(r.facing));
    if (r.arm == ST25_LEAP_REVERSED) {
        e = then(e, ended_of_sum(r.negated));
    }
    return ended_with_carry(then(e, ended_load(9)), true);
}

void abi_StartLeapWhenHurtAnswerYes(void)
{
    abi_ended(l8_leap_yes_exit_shim(StartLeapWhenHurtAnswerYes(cpu.x, cpu.y)));
}

/*
 * @routine 0EBE:768 StepListByPhaseThenLand
 * @run 71:CF68
 * WHAT IT DOES: Steps the animation list that the table at $CFA7 names for OBJ_3568 and asks StopOnLanding with a probe height of 8; while it falls the Y speed loses $0014 a frame; on landing it clears the animation, sets frame $45 and OBJ_3510 8 and goes to the next OBJ_SUBSTATE.
 * HOW: StepObjectFrames, StopOnLanding, a 16-bit subtract in memory, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y the probe's column unless the step sets it, C passed on (StepObjectFrames sets it: C 1 when the frame changed). Exit: the mover's registers, then the landing's or the subtract's (l8_phase_land_exit_shim).
 */
L8PhaseLandArm StepListByPhaseThenLand(uint8_t slot, uint8_t y_entry, bool carry_entry)
{
    uint8_t phase, lo, hi, value;
    L8PhaseLandArm r = { 0 };

    (void)y_entry;
    (void)carry_entry;
    phase = RD8(OBJ->f3568[slot]);
    r.y = (uint8_t)(phase << 1);
    lo = rd8((uint16_t)(0xCFA7 + r.y));
    WR8(ZP->temp10, lo);
    hi = rd8((uint16_t)(0xCFA8 + r.y));
    WR8(ZP->temp11, hi);
    jsr_push(0xCF7A);
    r.step = StepObjectFrames(slot, &r.y);
    jsr_pop();
    jsr_push(0xCF7F);
    r.mover = StopOnLanding(8, slot, r.y, r.step == FRAME_CHANGED);
    jsr_pop();
    if (!mover_landed(r.mover)) {
        value = RD8(OBJ->vy_lo[slot]);
        r.diff = sub_bytes(value, 0x14, true);
        WR8(OBJ->vy_lo[slot], r.diff.value);
        value = RD8(OBJ->vy_hi[slot]);
        r.diff = sub_bytes(value, 0, r.diff.carry);
        WR8(OBJ->vy_hi[slot], r.diff.value);
        r.landed = false;
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->frame[slot], 0x45);
    WR8(OBJ->f3510[slot], 8);
    r.count = INC8(OBJ->substate[slot]);
    r.landed = true;
    return r;
}

/* What StepListByPhaseThenLand leaves: the landing wait with the mover, then the subtract's or the landing's registers. */
static LandWait l8_phase_land_exit_shim(L8PhaseLandArm r)
{
    LandWait wait = { true, { 0 }, { 0 } };

    wait.mover = r.mover;
    if (!r.landed) {
        wait.end = ended_of_sum(r.diff);
        return wait;
    }
    wait.end = then(ended_load(8), ended_nz(r.count));
    return wait;
}

void abi_StepListByPhaseThenLand(void)
{
    land_wait_exit_shim(l8_phase_land_exit_shim(StepListByPhaseThenLand(cpu.x, cpu.y, (cpu.p & P_C) != 0)));
}
