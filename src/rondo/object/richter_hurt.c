/*
 * Richter's reaction to a hit, from his overlay at $A000 (sector $1B9): state 9, its steps through
 * the table at $B22D, and the pieces of it the fall and the landing share. RichterHitReact keeps the
 * state and the step he was hit in, in OBJ_STATE_BEFORE_HIT and OBJ_STEP_BEFORE_HIT; the reaction ends
 * by giving them back.
 *
 * A body returns plain values; its abi_ wrapper gives the registers and flags the assembly leaves,
 * replaying the exits of the callees whose registers and flags survive to the end.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"
#include "access.h"
#include "enemy_objects.h"
#include "player_objects.h"
#include "richter.h"

enum {
    SLOT_FIRST_EXTRA = 1, SLOT_LAST_EXTRA = 2, KIND_KEPT_A = 1, KIND_KEPT_B = 2, KIND_REPLACED = 4,
    SOUND_HURT_LAND = 0x4A, SOUND_HIT = 0x62, SOUND_HIT_EFFECT_20 = 0x12, SOUND_HIT_EFFECT_40 = 0x24,
    HIT_SILENT = 0x60, HIT_EFFECT_10 = 0x10, HIT_EFFECT_20 = 0x20, HIT_EFFECT_40 = 0x40,
    EFFECT_10_KIND = 1, EFFECT_10_TIMER = 0x0A, EFFECT_10_SCRIPT = 0x0A, EFFECT_20_SCRIPT = 0x01,
    EFFECT_40_SCRIPT = 0x17, EFFECT_40_BYTE_E8 = 0xFF, EFFECT_40_BYTE_E9 = 0x01,
    FRAME_CROUCHED = 0x0E, ANIMATION_STOPPED = 0xFF, STATE_CROUCH = 2, FIRST_STAIR_STATE = 5,
    STEP_RESUMED_ITEM_END = 1
};

/*
 * @routine 01BA:70E RichterClearSlotKindFour
 * @run 6D:AF0E
 * WHAT IT DOES: Empties an object slot and gives it kind 4.
 * HOW: ClearObject (through its kernel vector), then 4 into OBJ_KIND.
 * WAITS: no.
 * Entry: X object. Exit: A 4, N and Z from it. X, Y, C and V kept.
 */
void RichterClearSlotKindFour(uint8_t slot)
{
    jsr_push(0xAF11);
    ClearObject(slot);
    jsr_pop();
    WR8(OBJ->kind[slot], KIND_REPLACED);
}

void abi_RichterClearSlotKindFour(void)
{
    RichterClearSlotKindFour(cpu.x);
    abi_ended(ended_load(KIND_REPLACED));
}

/*
 * @routine 01BA:6F3 RichterReplaceSlotsOneTwo
 * @run 6D:AEF3
 * WHAT IT DOES: Gives object slots 1 and 2 kind 4, each emptied first, unless it holds kind 1 or 2.
 * HOW: OBJ_KIND compared with 1 and 2; RichterClearSlotKindFour for slot 1, then a run on into it for
 * slot 2. A kind 1 or 2 in slot 2 ends at RichterClearSlotKindFour's RTS.
 * WAITS: no.
 * Entry: nothing. Exit: X 2. Slot 2 kept: A its kind, N, Z and C from comparing it with 1 or 2; else
 * A 4, N and Z from it, C from comparing the kind with 2. Y and V kept.
 */
Ended RichterReplaceSlotsOneTwo(void)
{
    Ended e = ended_kept();
    uint8_t slot, kind;

    for (slot = SLOT_FIRST_EXTRA; slot <= SLOT_LAST_EXTRA; slot++) {
        kind = RD8(OBJ->kind[slot]);
        e = ended_then(e, ended_compare(kind, KIND_KEPT_A));
        if (kind == KIND_KEPT_A) {
            continue;
        }
        e = ended_then(e, ended_compare(kind, KIND_KEPT_B));
        if (kind == KIND_KEPT_B) {
            continue;
        }
        if (slot == SLOT_FIRST_EXTRA) {
            jsr_push(0xAF02);
            RichterClearSlotKindFour(slot);
            jsr_pop();
        } else {
            RichterClearSlotKindFour(slot);
        }
        e = ended_then(e, ended_load(KIND_REPLACED));
    }
    return e;
}

void abi_RichterReplaceSlotsOneTwo(void)
{
    abi_ended(RichterReplaceSlotsOneTwo());
    cpu.x = SLOT_LAST_EXTRA;
}

/*
 * @routine 01BB:52C RichterHurtLandSound
 * @run 6D:B52C
 * WHAT IT DOES: Slots 1 and 2 to kind 4, then sound $4A.
 * HOW: RichterReplaceSlotsOneTwo, then a tail jump to SoundCommand (through its kernel vector).
 * WAITS: no.
 * Entry: X and Y for the sound driver as SoundCommand takes them (X is 2 by then). Exit: as SoundCommand
 * with $4A, from what RichterReplaceSlotsOneTwo left.
 */
HurtLandSound RichterHurtLandSound(uint8_t y)
{
    HurtLandSound r;

    jsr_push(0xB52F);
    r.slots = RichterReplaceSlotsOneTwo();
    jsr_pop();
    r.sound = SoundCommand(SOUND_HURT_LAND, SLOT_LAST_EXTRA, y);
    return r;
}

void hurt_land_sound_exit_shim(HurtLandSound r)
{
    abi_ended(r.slots);
    cpu.x = SLOT_LAST_EXTRA;
    abi_ended(ended_load(SOUND_HURT_LAND));
    sound_command_exit_shim(SOUND_HURT_LAND, r.sound);
}

void abi_RichterHurtLandSound(void)
{
    hurt_land_sound_exit_shim(RichterHurtLandSound(cpu.y));
}

/*
 * @routine 01BB:565 RichterKnockSpeedX
 * @run 6D:B565
 * WHAT IT DOES: Gives him one pixel a frame of X speed: left for 0, right for 1.
 * HOW: Two words at $B5AA, $FF00 and $0100, by twice A.
 * WAITS: no.
 * Entry: A 0 or 1. Exit: A the speed's high byte, N and Z from it; Y twice the entry; C bit 7 of
 * the entry. X and V kept.
 */
uint8_t RichterKnockSpeedX(uint8_t side)
{
    uint8_t at = (uint8_t)(side << 1);
    uint8_t value;

    value = rd8((uint16_t)(RichterKnockSpeedsX + at));
    WR8(OBJ->vx_lo[0], value);
    value = rd8((uint16_t)(RichterKnockSpeedsX + at + 1));
    WR8(OBJ->vx_hi[0], value);
    return value;
}

void knock_speed_x_exit_shim(uint8_t side, uint8_t high)
{
    cpu.a = high;
    cpu.y = (uint8_t)(side << 1);
    abi_nz(high);
    abi_flag(P_C, (side & 0x80) != 0);
}

void abi_RichterKnockSpeedX(void)
{
    uint8_t side = cpu.a;

    knock_speed_x_exit_shim(side, RichterKnockSpeedX(side));
}

/*
 * @routine 01BB:562 RichterKnockSpeedXFromHit
 * @run 6D:B562
 * WHAT IT DOES: RichterKnockSpeedX for the side the hit throws him to, OBJ_KNOCK_RIGHT.
 * HOW: Runs on into RichterKnockSpeedX.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterKnockSpeedX.
 */
KnockSpeed RichterKnockSpeedXFromHit(void)
{
    KnockSpeed r;

    r.side = RD8(OBJ->knock_right[0]);
    r.high = RichterKnockSpeedX(r.side);
    return r;
}

void abi_RichterKnockSpeedXFromHit(void)
{
    KnockSpeed r = RichterKnockSpeedXFromHit();

    knock_speed_x_exit_shim(r.side, r.high);
}

/*
 * @routine 01BB:592 RichterHurtToCrouch
 * @run 6D:B592
 * WHAT IT DOES: Ends a reaction in a crouch: the crouched frame, the animation stopped, no speed,
 * state 2 at step 0.
 * HOW: OBJ_FRAME $0E, OBJ_HURT_TIMER $FF, RichterStopMoving, OBJ_STATE and OBJ_SUBSTATE.
 * WAITS: no.
 * Entry: nothing. Exit: A 0, N clear, Z set. X, Y, C and V kept.
 */
void RichterHurtToCrouch(void)
{
    WR8(OBJ->frame[0], FRAME_CROUCHED);
    WR8(OBJ->hurt_timer[0], ANIMATION_STOPPED);
    jsr_push(0xB59F);
    RichterStopMoving();
    jsr_pop();
    WR8(OBJ->state[0], STATE_CROUCH);
    WR8(OBJ->substate[0], 0);
}

void abi_RichterHurtToCrouch(void)
{
    RichterHurtToCrouch();
    abi_ended(ended_load(0));
}

/*
 * @routine 01BB:38E RichterHitEffects
 * @run 6D:B38E
 * WHAT IT DOES: What a hit does besides hurting, by bits 4 to 6 of OBJ_HIT_TAKEN, the first set of
 * them in that order; then OBJ_HIT_TAKEN is cleared. Bit 4: slots 1 and 2 emptied as kind 1, slot
 * 1's OBJ_ANIM_TIMER $0A, script $0A. Bit 5: sound $12, PlayerByte3BE2 counted up, script 1. Bit 6:
 * sound $24, PlayerByte3BE8 $FF and PlayerByte3BE9 1, script $17.
 * HOW: Three TSTs of OBJ_HIT_TAKEN; ClearObject and SoundCommand through their kernel vectors; a tail
 * jump to RichterStartHitScript.
 * WAITS: no.
 * Entry: X and Y for the sound driver as SoundCommand takes them. Exit: no bit: OBJ_HIT_TAKEN 0, Z
 * set, N bit 7 of OBJ_HIT_TAKEN as the last TST read it, V clear; A, X, Y and C kept. Otherwise as
 * RichterStartHitScript with the script: C kept; V bit 6 of OBJ_HIT_TAKEN for bit 4, as SoundCommand
 * left it for bits 5 and 6.
 */
HitEffects RichterHitEffects(uint8_t x, uint8_t y)
{
    HitEffects r = { HIT_EFFECT_NONE, 0, 0, 0, { 0 } };
    uint8_t value;

    r.taken = RD8(OBJ->hit_taken[0]);
    if (r.taken & HIT_EFFECT_10) {
        r.how = HIT_EFFECT_SLOTS;
        jsr_push(0xB3A9);
        ClearObject(SLOT_FIRST_EXTRA);
        jsr_pop();
        jsr_push(0xB3AD);
        ClearObject(SLOT_LAST_EXTRA);
        jsr_pop();
        WR8(OBJ->kind[SLOT_FIRST_EXTRA], EFFECT_10_KIND);
        WR8(OBJ->kind[SLOT_LAST_EXTRA], EFFECT_10_KIND);
        WR8(OBJ->anim_timer[SLOT_FIRST_EXTRA], EFFECT_10_TIMER);
        r.script = EFFECT_10_SCRIPT;
    } else {
        r.taken = RD8(OBJ->hit_taken[0]);
        if (r.taken & HIT_EFFECT_20) {
            r.how = HIT_EFFECT_COUNTED;
            jsr_push(0xB3C7);
            r.sound = SoundCommand(SOUND_HIT_EFFECT_20, x, y);
            jsr_pop();
            r.command = SOUND_HIT_EFFECT_20;
            value = rd8(PlayerByte3BE2);
            wr8(PlayerByte3BE2, (uint8_t)(value + 1));
            r.script = EFFECT_20_SCRIPT;
        } else {
            r.taken = RD8(OBJ->hit_taken[0]);
            if (!(r.taken & HIT_EFFECT_40)) {
                WR8(OBJ->hit_taken[0], 0);
                return r;
            }
            r.how = HIT_EFFECT_MARKED;
            jsr_push(0xB3D3);
            r.sound = SoundCommand(SOUND_HIT_EFFECT_40, x, y);
            jsr_pop();
            r.command = SOUND_HIT_EFFECT_40;
            wr8(PlayerByte3BE8, EFFECT_40_BYTE_E8);
            wr8(PlayerByte3BE9, EFFECT_40_BYTE_E9);
            r.script = EFFECT_40_SCRIPT;
        }
    }
    WR8(OBJ->hit_taken[0], 0);
    RichterStartHitScript(r.script);
    return r;
}

void hit_effects_exit_shim(HitEffects r)
{
    abi_flag(P_V, (r.taken & 0x40) != 0);
    switch (r.how) {
    case HIT_EFFECT_NONE:
        abi_nz(0);
        abi_flag(P_N, (r.taken & 0x80) != 0);
        return;
    case HIT_EFFECT_SLOTS:
        break;
    case HIT_EFFECT_COUNTED:
    case HIT_EFFECT_MARKED:
        cpu.a = r.command;
        sound_command_exit_shim(r.command, r.sound);
        break;
    }
    hit_script_exit_shim(r.script);
}

void abi_RichterHitEffects(void)
{
    hit_effects_exit_shim(RichterHitEffects(cpu.x, cpu.y));
}

/*
 * @routine 01BB:383 RichterHitSoundAndEffects
 * @run 6D:B383
 * WHAT IT DOES: Sound $62 for a hit with neither bit 5 nor bit 6 of OBJ_HIT_TAKEN, then
 * RichterHitEffects.
 * HOW: A TST of OBJ_HIT_TAKEN with $60; SoundCommand through its kernel vector; runs on into
 * RichterHitEffects.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as RichterHitEffects, from what SoundCommand left when
 * it ran.
 */
HitSound RichterHitSoundAndEffects(uint8_t x, uint8_t y)
{
    HitSound r = { false, { 0 }, { 0 } };

    if (!(RD8(OBJ->hit_taken[0]) & HIT_SILENT)) {
        r.sounded = true;
        jsr_push(0xB38E);
        r.sound = SoundCommand(SOUND_HIT, x, y);
        jsr_pop();
    }
    r.effects = RichterHitEffects(x, y);
    return r;
}

void abi_RichterHitSoundAndEffects(void)
{
    HitSound r = RichterHitSoundAndEffects(cpu.x, cpu.y);

    if (r.sounded) {
        cpu.a = SOUND_HIT;
        sound_command_exit_shim(SOUND_HIT, r.sound);
    }
    hit_effects_exit_shim(r.effects);
}

/*
 * @routine 01BB:5AE RichterHurtResume
 * @run 6D:B5AE
 * WHAT IT DOES: The reaction's step 3: shows the hurt frame for the state he was hit in, and when
 * OBJ_HIT_FRAMES runs out gives back that state and its step; with step 1 given back it ends as
 * RichterEndItemCrash.
 * HOW: The frame from the two bytes at $B5D5 by (the state less 5) / 2; a tail jump to
 * RichterEndItemCrash.
 * WAITS: no.
 * Entry: nothing. Exit: X the frame's index; V from the subtract. Still counting: A the frame, N and Z
 * from the count, C bit 0 of the state less 5. Given back: A the step, N, Z and C from comparing it
 * with 1; step 1, then as RichterEndItemCrash. Y kept but there.
 */
HurtResume RichterHurtResume(void)
{
    HurtResume r = { 0 };
    uint8_t value;

    r.state = RD8(OBJ->state_before_hit[0]);
    r.index = (uint8_t)((uint8_t)(r.state - FIRST_STAIR_STATE) >> 1);
    r.frame = rd8((uint16_t)(RichterHurtResumeFrames + r.index));
    WR8(OBJ->frame[0], r.frame);
    r.count = DEC8(OBJ->hit_frames[0]);
    if (r.count != 0) {
        return r;
    }
    r.given_back = true;
    value = RD8(OBJ->state_before_hit[0]);
    r.resumed_state = value;
    WR8(OBJ->state[0], value);
    r.step = RD8(OBJ->step_before_hit[0]);
    WR8(OBJ->substate[0], r.step);
    if (r.step != STEP_RESUMED_ITEM_END) {
        return r;
    }
    r.ended = true;
    r.end = RichterEndItemCrash();
    return r;
}

void abi_RichterHurtResume(void)
{
    HurtResume r = RichterHurtResume();
    uint8_t less = (uint8_t)(r.state - FIRST_STAIR_STATE);

    abi_flag(P_V, sub_overflow(r.state, FIRST_STAIR_STATE, less));
    cpu.x = r.index;
    if (!r.given_back) {
        cpu.a = r.frame;
        abi_nz(r.count);
        abi_flag(P_C, (less & 1) != 0);
        return;
    }
    abi_ended(ended_compare(r.step, STEP_RESUMED_ITEM_END));
    if (r.ended) {
        crash_end_exit_shim(r.end);
    }
}
