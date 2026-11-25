/*
 * TEMPORARY: the stage object routines of lot L8 for the overlays at sectors 0F6C and 0F6D (enemy kinds not yet identified), by routine. Bodies return how the
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
 * @routine 0F6D:365 StepListByNumberTableC585
 * @run 71:C365
 * WHAT IT DOES: Steps the animation list that the table at $C585 names for OBJ_34B8.
 * HOW: A table read of two bytes into zTemp10-11, a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step.
 */
Ended StepListByNumberTableC585(uint8_t slot, uint8_t y_entry)
{
    uint8_t number, y, lo, hi;
    FrameStep step;
    Ended e;

    (void)y_entry;
    number = RD8(OBJ->f34b8[slot]);
    y = (uint8_t)(number << 1);
    e = ended_with_y(ended_with_carry(ended_load(y), number >= 0x80), y);
    lo = rd8((uint16_t)(0xC585 + y));
    WR8(ZP->temp10, lo);
    e = then(e, ended_load(lo));
    hi = rd8((uint16_t)(0xC586 + y));
    WR8(ZP->temp11, hi);
    e = then(e, ended_load(hi));
    step = StepObjectFrames(slot, &y);
    return then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
}

void abi_StepListByNumberTableC585(void)
{
    abi_ended(StepListByNumberTableC585(cpu.x, cpu.y));
}

/*
 * @routine 0F6D:35F ClearAnimationThenStepListC585
 * @run 71:C35F
 * WHAT IT DOES: Clears the animation step and timer and runs StepListByNumberTableC585.
 * HOW: Two stores, running into the next routine.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as StepListByNumberTableC585.
 */
Ended ClearAnimationThenStepListC585(uint8_t slot, uint8_t y_entry)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    return StepListByNumberTableC585(slot, y_entry);
}

void abi_ClearAnimationThenStepListC585(void)
{
    abi_ended(ClearAnimationThenStepListC585(cpu.x, cpu.y));
}

/*
 * @routine 0F6D:377 ToggleFacing
 * @run 71:C377
 * WHAT IT DOES: Turns the object round.
 * HOW: An EOR of bit 0 and a store.
 * WAITS: no.
 * Entry: X object. Exit: A the new facing, N and Z from it.
 */
Ended ToggleFacing(uint8_t slot)
{
    uint8_t facing, value;

    facing = RD8(OBJ->facing_left[slot]);
    value = (uint8_t)(facing ^ 0x01);
    value = (uint8_t)(value & 0x01);
    WR8(OBJ->facing_left[slot], value);
    return ended_load(value);
}

void abi_ToggleFacing(void)
{
    abi_ended(ToggleFacing(cpu.x));
}

/*
 * @routine 0F6D:3F3 LoadRecordOrBurstWhenFlagged
 * @run 71:C3F3
 * WHAT IT DOES: In state 0 loads the record. Later, when bit 7 of OBJ_31A0 is set, plays sound $16, clears slots $52 to $54 and starts objects of type $AE in $52 and $53 (the second with OBJ_3040 1) and of type $9C in $54 at its place, then clears itself.
 * HOW: LoadObjectRecord, SoundCommand, ClearObjectInY and StartObjectAtPlace three times, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The burst ran in no script; the draft stops there.
 */
SoundEnd LoadRecordOrBurstWhenFlagged(uint8_t slot, uint8_t y_entry)
{
    uint8_t state, last, nz, flags, y, placed;
    bool carry;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        jsr_push(0xC3FB);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        r.end = then(e, ended_of_record(last, nz, carry));
        return r;
    }
    flags = RD8(OBJ->status_flags[slot]);
    e = then(e, ended_load((uint8_t)(flags & 0x80)));
    if ((flags & 0x80) == 0) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0x16;
    r.before = then(e, ended_load(0x16));
    jsr_push(0xC408);
    r.call = SoundCommand(0x16, slot, y_entry);
    jsr_pop();
    y = 0x52;
    jsr_push(0xC40D);
    ClearObjectInY(y);
    jsr_pop();
    jsr_push(0xC412);
    placed = StartObjectAtPlace(0xAE, slot, y);
    jsr_pop();
    y++;
    jsr_push(0xC416);
    ClearObjectInY(y);
    jsr_pop();
    jsr_push(0xC41B);
    placed = StartObjectAtPlace(0xAE, slot, y);
    jsr_pop();
    WR8(OBJ->f3040[y], 1);
    y++;
    jsr_push(0xC424);
    ClearObjectInY(y);
    jsr_pop();
    jsr_push(0xC429);
    placed = StartObjectAtPlace(0x9C, slot, y);
    jsr_pop();
    ClearObject(slot);
    r.end = ended_with_y(ended_load(placed), y);
    return r;
}

void abi_LoadRecordOrBurstWhenFlagged(void)
{
    abi_sound_end(LoadRecordOrBurstWhenFlagged(cpu.x, cpu.y));
}

/*
 * @routine 0F6C:0D5 SpawnFlashingPieceWhenRising
 * @run 70:B8D5
 * WHAT IT DOES: A rising object (Y speed high bit clear) shows flash flags $10; every fourth frame it starts an object of type $9B in a free slot of $20 to $3F, with Y speed $0800 (moved once) and an X speed from the table at $B90D by its own OBJ_3040 (moved once) and flash flags $10.
 * HOW: A test, TST of the wave counter, FindFreeSlotFrom20, StartObjectAtPlace, ApplyVelocityY and X with X and Y swapped.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended SpawnFlashingPieceWhenRising(uint8_t slot)
{
    uint8_t speed, wave, found_slot, last, placed, kind, value;
    bool found;
    AxisMove move;
    Ended e;

    speed = RD8(OBJ->vy_hi[slot]);
    e = ended_load(speed);
    if ((speed & 0x80) != 0) {
        return e;
    }
    WR8(OBJ->flash_flags[slot], 0x10);
    e = then(e, ended_load(0x10));
    wave = RD8(ZP->wave_counter);
    e = then(e, ended_with_overflow(ended_nz((uint8_t)((wave & 0x80) | ((wave & 0x03) ? 1 : 0))), (wave & 0x40) != 0));
    if ((wave & 0x03) != 0) {
        return e;
    }
    jsr_push(0xB8E7);
    found_slot = FindFreeSlotFrom20(&last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, last, found));
    if (!found) {
        return e;
    }
    jsr_push(0xB8EE);
    placed = StartObjectAtPlace(0x9B, slot, found_slot);
    jsr_pop();
    WR8(OBJ->vy_hi[found_slot], 8);
    jsr_push(0xB8F7);
    move = ApplyVelocityY(found_slot);
    jsr_pop();
    e = then(then(e, ended_load(8)), ended_of_axis_move(move));
    push8(slot);
    kind = RD8(OBJ->f3040[slot]);
    value = rd8((uint16_t)(0xB90D + kind));
    WR8(OBJ->vx_hi[found_slot], value);
    jsr_push(0xB905);
    move = ApplyVelocityX(found_slot);
    jsr_pop();
    slot = pull8();
    e = then(e, ended_of_axis_move(move));
    WR8(OBJ->flash_flags[found_slot], 0x10);
    (void)placed;
    return then(e, ended_with_y(ended_load(0x10), found_slot));
}

void abi_SpawnFlashingPieceWhenRising(void)
{
    abi_ended(SpawnFlashingPieceWhenRising(cpu.x));
}

/*
 * @routine 0F6C:111 CountDownThenClearOnTimer
 * @run 70:B911
 * WHAT IT DOES: Counts OBJ_3510 down; at zero clears the object.
 * HOW: A memory decrement and a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: the decrement's N and Z.
 */
Ended CountDownThenClearOnTimer(uint8_t slot)
{
    uint8_t timer = DEC8(OBJ->f3510[slot]);

    if (timer == 0) {
        ClearObject(slot);
    }
    return ended_nz(timer);
}

void abi_CountDownThenClearOnTimer(void)
{
    abi_ended(CountDownThenClearOnTimer(cpu.x));
}

/*
 * @routine 0F6C:3F0 FlashWhenHurtThenSound
 * @run 70:BBF0
 * WHAT IT DOES: Unless bit 7 of OBJ_31A0 is set, in the states the table at $BBB0 marks it clears its flash flags, sets them $10 when OBJ_3A90 is not 0, copies OBJ_32A8 to slot $10's, and at 5 hits plays sound $D6, doubles OBJ_3A90 twice, goes to state $0D and clears OBJ_2800.
 * HOW: Tests, a table read by the state, stores, SoundCommand, two ASLs in memory.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The arm past the table test ran in no script; the draft stops there.
 */
SoundEnd FlashWhenHurtThenSound(uint8_t slot, uint8_t y_entry)
{
    uint8_t flags, y, marked, timer, value, hits, shifted;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    flags = RD8(OBJ->status_flags[slot]);
    e = ended_load((uint8_t)(flags & 0x80));
    if ((flags & 0x80) != 0) {
        r.end = e;
        return r;
    }
    y = RD8(OBJ->state[slot]);
    e = then(e, ended_with_y(ended_nz(y), y));
    marked = rd8((uint16_t)(0xBBB0 + y));
    e = then(e, ended_load(marked));
    if (marked == 0) {
        r.end = e;
        return r;
    }
    WR8(OBJ->flash_flags[slot], 0);
    timer = RD8(OBJ->timer[slot]);
    e = then(e, ended_load(timer));
    if (timer != 0) {
        WR8(OBJ->flash_flags[slot], 0x10);
        e = then(e, ended_load(0x10));
    }
    value = RD8(OBJ->f32a8[slot]);
    WR8(OBJ->f32a8[0x10], value);
    e = then(e, ended_load(value));
    hits = RD8(OBJ->hit_taken[slot]);
    e = then(e, ended_compare(hits, 5));
    if (hits < 5) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0xD6;
    r.before = then(e, ended_load(0xD6));
    jsr_push(0xBC1E);
    r.call = SoundCommand(0xD6, slot, y);
    jsr_pop();
    ASL8(OBJ->timer[slot]);
    shifted = ASL8(OBJ->timer[slot]);
    WR8(OBJ->state[slot], 0x0D);
    WR8(OBJ->f2800[slot], 0);
    r.end = ended_with_carry(ended_load(0x0D), shifted != 0);
    return r;
}

void abi_FlashWhenHurtThenSound(void)
{
    abi_sound_end(FlashWhenHurtThenSound(cpu.x, cpu.y));
}

/*
 * @routine 0F6C:42D EnterState0EWhenFlagged
 * @run 70:BC2D
 * WHAT IT DOES: When bit 7 of OBJ_31A0 is set and the state is not $0E, copies OBJ_32A8 to slot $10's, clears OBJ_3250, goes to state $0E and clears OBJ_2800.
 * HOW: Two tests, stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. The arm past the flag test ran in no script; the draft stops there.
 */
Ended EnterState0EWhenFlagged(uint8_t slot)
{
    uint8_t flags, state, value;
    Ended e;

    flags = RD8(OBJ->status_flags[slot]);
    e = ended_load((uint8_t)(flags & 0x80));
    if ((flags & 0x80) == 0) {
        return e;
    }
    state = RD8(OBJ->state[slot]);
    e = then(e, ended_compare(state, 0x0E));
    if (state == 0x0E) {
        return e;
    }
    value = RD8(OBJ->f32a8[slot]);
    WR8(OBJ->f32a8[0x10], value);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->state[slot], 0x0E);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_load(0x0E));
}

void abi_EnterState0EWhenFlagged(void)
{
    abi_ended(EnterState0EWhenFlagged(cpu.x));
}

/*
 * @routine 0F6C:44D LoadRecordFrame24
 * @run 70:BC4D
 * WHAT IT DOES: Loads the object's record, sets its frame $24 and clears OBJ_3250.
 * HOW: LoadObjectRecord and two stores.
 * WAITS: no.
 * Entry: X object. Exit: A $24, Y 11, C from the record.
 */
Ended LoadRecordFrame24(uint8_t slot)
{
    uint8_t last, nz;
    bool carry;
    Ended e;

    jsr_push(0xBC50);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->frame[slot], 0x24);
    WR8(OBJ->hitbox_on[slot], 0);
    return then(e, ended_load(0x24));
}

void abi_LoadRecordFrame24(void)
{
    abi_ended(LoadRecordFrame24(cpu.x));
}

/*
 * @routine 0F6C:459 StartTimerWhenSlot10Ready
 * @run 70:BC59
 * WHAT IT DOES: When the state of slot $10 is 5 or more and its OBJ_32A8 is not $5C, sets OBJ_34B8 $0A and goes to the next state.
 * HOW: Two compares, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended StartTimerWhenSlot10Ready(uint8_t slot)
{
    uint8_t state, value;
    Ended e;

    state = RD8(OBJ->state[0x10]);
    e = ended_compare(state, 5);
    if (state < 5) {
        return e;
    }
    value = RD8(OBJ->f32a8[0x10]);
    e = then(e, ended_compare(value, 0x5C));
    if (value == 0x5C) {
        return e;
    }
    WR8(OBJ->f34b8[slot], 0x0A);
    e = then(e, ended_load(0x0A));
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_StartTimerWhenSlot10Ready(void)
{
    abi_ended(StartTimerWhenSlot10Ready(cpu.x));
}

/*
 * @routine 0F6C:580 HurtOnceThenAnswerYes
 * @run 70:BD80
 * WHAT IT DOES: Says yes (C 1) when the object has taken exactly 1 hit, the player's Y low byte is not $9A and slot 0's state is below $10: it then clears its hits and OBJ_3250, sets slot 0's OBJ_3510 $40 and OBJ_3568 $10, OBJ_34B8 $0B, restarts its animation (ClearAnimationThenStepListC585), goes to state 8. Otherwise says no (C 0).
 * HOW: Compares, stores, a call.
 * WAITS: no.
 * Entry: X object, Y kept for the call. Exit: C 1 yes, 0 no. The yes arm ran in no script; the draft stops there.
 */
Ended HurtOnceThenAnswerYes(uint8_t slot, uint8_t y_entry)
{
    uint8_t hits, ground, state;
    Ended e;

    hits = RD8(OBJ->hit_taken[slot]);
    e = ended_compare(hits, 1);
    if (hits != 1) {
        return ended_with_carry(e, false);
    }
    ground = RD8(OBJ->y_lo[0]);
    e = then(e, ended_compare(ground, 0x9A));
    if (ground == 0x9A) {
        return ended_with_carry(e, false);
    }
    state = RD8(OBJ->state[0]);
    e = then(e, ended_compare(state, 0x10));
    if (state >= 0x10) {
        return ended_with_carry(e, false);
    }
    WR8(OBJ->hit_taken[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->f3510[0], 0x40);
    WR8(OBJ->f3568[0], 0x10);
    WR8(OBJ->f34b8[slot], 0x0B);
    jsr_push(0xBDAD);
    e = then(e, ClearAnimationThenStepListC585(slot, y_entry));
    jsr_pop();
    WR8(OBJ->state[slot], 8);
    return ended_with_carry(then(e, ended_load(8)), true);
}

void abi_HurtOnceThenAnswerYes(void)
{
    abi_ended(HurtOnceThenAnswerYes(cpu.x, cpu.y));
}

/*
 * @routine 0F6C:470 StepListThenStartPieceAndWalk
 * @run 70:BC70
 * WHAT IT DOES: Runs StepListByNumberTableC585; when the list ends clears slot $51 and starts an object of type $AD there at the object's place, sets OBJ_3250 2, state 3 and OBJ_34B8 0, restarts the animation, sets the X speed -$00A0 and, facing left, reverses it.
 * HOW: Two calls, ClearObjectInY, StartObjectAtPlace, stores, a tail jump into NegateVelocityX.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran. Reversing the speed ran in no script; the draft stops there.
 */
Ended StepListThenStartPieceAndWalk(uint8_t slot, uint8_t y_entry)
{
    uint8_t placed, facing;
    Sum negated;
    Ended e;

    jsr_push(0xBC73);
    e = StepListByNumberTableC585(slot, y_entry);
    jsr_pop();
    if (e.a != 0) {
        return e;
    }
    jsr_push(0xBC7A);
    ClearObjectInY(0x51);
    jsr_pop();
    jsr_push(0xBC7F);
    placed = StartObjectAtPlace(0xAD, slot, 0x51);
    jsr_pop();
    e = then(then(e, ended_with_y(ended_nz(0x51), 0x51)), ended_load(placed));
    WR8(OBJ->hitbox_on[slot], 2);
    WR8(OBJ->state[slot], 3);
    WR8(OBJ->f34b8[slot], 0);
    e = then(e, ended_load(0));
    jsr_push(0xBC91);
    e = then(e, ClearAnimationThenStepListC585(slot, 0x51));
    jsr_pop();
    WR8(OBJ->vx_lo[slot], 0x60);
    WR8(OBJ->vx_hi[slot], 0xFF);
    facing = RD8(OBJ->facing_left[slot]);
    e = then(e, ended_load(facing));
    if (facing != 0) {
        negated = NegateVelocityX(slot);
        e = then(e, ended_of_sum(negated));
    }
    return e;
}

void abi_StepListThenStartPieceAndWalk(void)
{
    abi_ended(StepListThenStartPieceAndWalk(cpu.x, cpu.y));
}

/*
 * @routine 0F6C:6F7 StartLeapWhenHurtAnswerYes
 * @run 70:BEF7
 * WHAT IT DOES: Says no (C 0) when OBJ_35C0 is 0 or the object has taken fewer than 5 hits. Otherwise clears slot $11, starts an object of type $B0 there at its place, gives it the Y speed -$0800 (moved once), clears its hits and OBJ_3250, sets OBJ_34B8 $0E, restarts the animation, sets the speed X $0140 and Y -$0380 (reversing X when facing left), goes to state 9 and says yes (C 1).
 * HOW: Tests, ClearObjectInY, StartObjectAtPlace, ApplyVelocityY, ClearAnimationThenStepListC585, NegateVelocityX.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: C 1 yes, 0 no. The yes arm ran in no script; the draft stops there.
 */
Ended StartLeapWhenHurtAnswerYes(uint8_t slot, uint8_t y_entry)
{
    uint8_t aim, hits, placed, facing;
    AxisMove move;
    Sum negated;
    Ended e;

    (void)y_entry;
    aim = RD8(OBJ->f35c0[slot]);
    e = ended_load(aim);
    if (aim == 0) {
        return ended_with_carry(e, false);
    }
    hits = RD8(OBJ->hit_taken[slot]);
    e = then(e, ended_compare(hits, 5));
    if (hits < 5) {
        return ended_with_carry(e, false);
    }
    jsr_push(0xBF08);
    ClearObjectInY(0x11);
    jsr_pop();
    jsr_push(0xBF0D);
    placed = StartObjectAtPlace(0xB0, slot, 0x11);
    jsr_pop();
    e = then(then(e, ended_with_y(ended_nz(0x11), 0x11)), ended_load(placed));
    WR8(OBJ->vy_hi[slot], 0xF8);
    e = then(e, ended_load(0xF8));
    jsr_push(0xBF15);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    WR8(OBJ->hit_taken[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->f34b8[slot], 0x0E);
    e = then(e, ended_load(0x0E));
    jsr_push(0xBF23);
    e = then(e, ClearAnimationThenStepListC585(slot, 0x11));
    jsr_pop();
    WR8(OBJ->vy_lo[slot], 0x80);
    WR8(OBJ->vy_hi[slot], 0xFC);
    WR8(OBJ->vx_lo[slot], 0x40);
    WR8(OBJ->vx_hi[slot], 0x01);
    facing = RD8(OBJ->facing_left[slot]);
    e = then(e, ended_load(facing));
    if (facing != 0) {
        jsr_push(0xBF3F);
        negated = NegateVelocityX(slot);
        jsr_pop();
        e = then(e, ended_of_sum(negated));
    }
    WR8(OBJ->state[slot], 9);
    return ended_with_carry(then(e, ended_load(9)), true);
}

void abi_StartLeapWhenHurtAnswerYes(void)
{
    abi_ended(StartLeapWhenHurtAnswerYes(cpu.x, cpu.y));
}

/*
 * @routine 0EBE:768 StepListByPhaseThenLand
 * @run 71:CF68
 * WHAT IT DOES: Steps the animation list that the table at $CFA7 names for OBJ_3568 and asks StopOnLanding with a probe height of 8; while it falls the Y speed loses $0014 a frame; on landing it clears the animation, sets frame $45 and OBJ_3510 8 and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, StopOnLanding, a 16-bit subtract in memory, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y the probe's column unless the step sets it, C passed on. Exit: the mover's registers, then the landing's or the subtract's.
 */
LandWait StepListByPhaseThenLand(uint8_t slot, uint8_t y_entry, bool carry_entry)
{
    LandWait wait = { true, { 0 }, { 0 } };
    uint8_t phase, y, lo, hi, value;
    FrameStep step;
    Sum diff;
    Ended e;

    (void)y_entry;
    phase = RD8(OBJ->f3568[slot]);
    y = (uint8_t)(phase << 1);
    lo = rd8((uint16_t)(0xCFA7 + y));
    WR8(ZP->temp10, lo);
    hi = rd8((uint16_t)(0xCFA8 + y));
    WR8(ZP->temp11, hi);
    jsr_push(0xCF7A);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    jsr_push(0xCF7F);
    wait.mover = StopOnLanding(8, slot, y, ended_carry_or(e, carry_entry));
    jsr_pop();
    if (!mover_landed(wait.mover)) {
        value = RD8(OBJ->vy_lo[slot]);
        diff = sub_bytes(value, 0x14, true);
        WR8(OBJ->vy_lo[slot], diff.value);
        value = RD8(OBJ->vy_hi[slot]);
        diff = sub_bytes(value, 0, diff.carry);
        WR8(OBJ->vy_hi[slot], diff.value);
        wait.end = ended_of_sum(diff);
        return wait;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->frame[slot], 0x45);
    WR8(OBJ->f3510[slot], 8);
    wait.end = then(ended_load(8), ended_nz(INC8(OBJ->f2800[slot])));
    return wait;
}

void abi_StepListByPhaseThenLand(void)
{
    land_wait_exit_shim(StepListByPhaseThenLand(cpu.x, cpu.y, (cpu.p & P_C) != 0));
}
