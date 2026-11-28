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
 * 1's OBJ_ANIM_TIMER $0A, script $0A. Bit 5: sound $12, PlayerHeld counted up, script 1. Bit 6:
 * sound $24, PlayerSlowTimerLo $FF and PlayerSlowTimerHi 1, script $17.
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
            value = rd8(PlayerHeld);
            wr8(PlayerHeld, (uint8_t)(value + 1));
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
            wr8(PlayerSlowTimerLo, EFFECT_40_BYTE_E8);
            wr8(PlayerSlowTimerHi, EFFECT_40_BYTE_E9);
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

void hit_sound_exit_shim(HitSound r)
{
    if (r.sounded) {
        cpu.a = SOUND_HIT;
        sound_command_exit_shim(SOUND_HIT, r.sound);
    }
    hit_effects_exit_shim(r.effects);
}

void abi_RichterHitSoundAndEffects(void)
{
    hit_sound_exit_shim(RichterHitSoundAndEffects(cpu.x, cpu.y));
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
    hurt_resume_exit_shim(RichterHurtResume());
}

void hurt_resume_exit_shim(HurtResume r)
{
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

enum {
    HIT_KINDS = 0x07, HIT_KIND_SPIKE = 6, HIT_KIND_THREE = 3, HIT_KIND_FOUR = 4, HIT_KIND_FIVE = 5,
    HIT_SPIKE_BECOMES = 2, HIT_LOW_BITS = 0x03, HIT_LOW_ONE = 1, STEP_ATTACKING = 1,
    STEP_KNOCKED_UP = 2, STEP_DOWN = 6, STEP_SLIDE = 1, STEP_SLIDE_LOW = 0x0B, STEP_RESUME = 3,
    STEP_THROWN_THREE = 7, STEP_THROWN_FOUR = 9, STEP_THROWN_LAST = 0x0A,
    KNOCK_UP_HI = 0xFC, IN_AIR_UP_HI = 0xFD, KIND_FIVE_UP_HI = 0xFB, FRAME_KNOCKED_UP = 0x46,
    ANIM_DOWN = 0x1E, FRAME_SLIDE = 0x72, SLIDE_FRAMES = 8, RESUME_FRAMES = 0x20,
    THROWN_AIR_LO = 0x50, THROWN_AIR_HI = 0xFE, THROWN_GROUND_LO = 0x50, THROWN_GROUND_HI = 0xFC,
    THROWN_FOUR_LO = 0x80, THROWN_FOUR_HI = 0xF9, THROWN_FOUR_VX_LO = 0x80, THROWN_FOUR_VX_HI = 0x03,
    ANIM_THROWN = 0x1F, SOUND_THROWN = 0x63, SOUND_THROWN_LAND = 0x02, ANIM_THROWN_LAND_STOPPED = 0x20,
    ANIM_THROWN_LAND = 0x21, WAVE_EVERY_EIGHTH = 0x07
};

/* Reads the columns and checks for a floor, and once more a pixel lower when there is none: the
 * standing checks of the hurt steps. Gives whether a floor was found. */
static bool hurt_floor_twice(RegLog *log, bool read_first, uint16_t read_ret, uint16_t check_ret,
                             uint16_t read2_ret, uint16_t check2_ret)
{
    FloorResult floor;

    if (read_first) {
        jsr_push(read_ret);
        rl_columns(log, RichterReadColumns());
        jsr_pop();
    }
    jsr_push(check_ret);
    floor = RichterFloorCheck();
    jsr_pop();
    rl_floor(log, floor);
    if (floor.how != FLOOR_NONE) {
        return true;
    }
    rl_ended(log, ended_nz(INC8(OBJ->y_lo[0])));
    jsr_push(read2_ret);
    rl_columns(log, RichterReadColumns());
    jsr_pop();
    jsr_push(check2_ret);
    floor = RichterFloorCheck();
    jsr_pop();
    rl_floor(log, floor);
    return floor.how != FLOOR_NONE;
}

/* The X and Y a log has left, from those the routine was entered with. */
static uint8_t log_x(const RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t ox, oy;

    rl_xy(log, x, y, &ox, &oy);
    return ox;
}

static uint8_t log_y(const RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t ox, oy;

    rl_xy(log, x, y, &ox, &oy);
    return oy;
}

/*
 * @routine 01BB:2EB RichterHurtKnockUp
 * @run 6D:B2EB
 * WHAT IT DOES: Knocks him up: Y speed $FC00, step 2, frame $46, then the hit's sound and effects.
 * HOW: A tail jump to RichterHitSoundAndEffects.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as RichterHitSoundAndEffects, A $46 where it keeps A.
 */
void RichterHurtKnockUp(RegLog *log, uint8_t x, uint8_t y)
{
    WR8(OBJ->vy_lo[0], 0);
    WR8(OBJ->vy_hi[0], KNOCK_UP_HI);
    WR8(OBJ->substate[0], STEP_KNOCKED_UP);
    WR8(OBJ->frame[0], FRAME_KNOCKED_UP);
    rl_ended(log, ended_load(FRAME_KNOCKED_UP));
    rl_hit_sound(log, RichterHitSoundAndEffects(x, y));
}

/*
 * @routine 01BB:302 RichterHurtDown
 * @run 6D:B302
 * WHAT IT DOES: Puts him down: step 6 with animation $1E, then the hit's sound and effects.
 * HOW: RichterSetAnimation; a tail jump to RichterHitSoundAndEffects.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as RichterHitSoundAndEffects, from what
 * RichterSetAnimation left.
 */
void RichterHurtDown(RegLog *log, uint8_t x, uint8_t y)
{
    WR8(OBJ->substate[0], STEP_DOWN);
    rl_ended(log, ended_load(ANIM_DOWN));
    jsr_push(0xB30C);
    rl_animation(log, RichterSetAnimation(ANIM_DOWN));
    jsr_pop();
    rl_hit_sound(log, RichterHitSoundAndEffects(log_x(log, x, y), log_y(log, x, y)));
}

/*
 * @routine 01BB:2DB RichterHurtFromStand
 * @run 6D:B2DB
 * WHAT IT DOES: A hit while standing or walking: knocked up when he was at step 1 or the hit's low
 * bits are not 1; put down otherwise.
 * HOW: OBJ_STEP_BEFORE_HIT against 1, bits 0 and 1 of OBJ_HIT_TAKEN against 1; branches to
 * RichterHurtKnockUp and RichterHurtDown, and runs on into RichterHurtKnockUp.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as the routine it goes to, from the last compare.
 */
void RichterHurtFromStand(RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t value;

    value = RD8(OBJ->step_before_hit[0]);
    rl_ended(log, ended_compare(value, STEP_ATTACKING));
    if (value == STEP_ATTACKING) {
        RichterHurtKnockUp(log, x, y);
        return;
    }
    value = (uint8_t)(RD8(OBJ->hit_taken[0]) & HIT_LOW_BITS);
    rl_ended(log, ended_compare(value, HIT_LOW_ONE));
    if (value == HIT_LOW_ONE) {
        RichterHurtDown(log, x, y);
        return;
    }
    RichterHurtKnockUp(log, x, y);
}

/*
 * @routine 01BB:30F RichterHurtFromCrouch
 * @run 6D:B30F
 * WHAT IT DOES: A hit while crouched: knocked up unless at step 0; otherwise a slide, step $0B when
 * the hit's low bits are 1, else step 1, with frame $72 for 8 frames; then the hit's sound and
 * effects.
 * HOW: A branch to RichterHurtKnockUp; OBJ_HIT_FRAMES; a tail jump to RichterHitSoundAndEffects.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as RichterHurtKnockUp, or as RichterHitSoundAndEffects
 * with A 8 where it keeps A.
 */
void RichterHurtFromCrouch(RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t value, step;

    value = RD8(OBJ->step_before_hit[0]);
    rl_ended(log, ended_load(value));
    if (value != 0) {
        RichterHurtKnockUp(log, x, y);
        return;
    }
    value = (uint8_t)(RD8(OBJ->hit_taken[0]) & HIT_LOW_BITS);
    step = value == HIT_LOW_ONE ? STEP_SLIDE_LOW : STEP_SLIDE;
    WR8(OBJ->substate[0], step);
    WR8(OBJ->frame[0], FRAME_SLIDE);
    WR8(OBJ->hit_frames[0], SLIDE_FRAMES);
    rl_ended(log, ended_with_carry(ended_load(SLIDE_FRAMES), value >= HIT_LOW_ONE));
    rl_hit_sound(log, RichterHitSoundAndEffects(x, y));
}

/*
 * @routine 01BB:333 RichterHurtInAir
 * @run 6D:B333
 * WHAT IT DOES: A hit in the air or in the hit's own state: knocked up, at Y speed $FD00.
 * HOW: RichterHurtKnockUp, then the speed's high byte rewritten.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: A $FD, N set, Z clear; X, Y, C and V as
 * RichterHurtKnockUp left them.
 */
void RichterHurtInAir(RegLog *log, uint8_t x, uint8_t y)
{
    jsr_push(0xB336);
    RichterHurtKnockUp(log, x, y);
    jsr_pop();
    WR8(OBJ->vy_lo[0], 0);
    WR8(OBJ->vy_hi[0], IN_AIR_UP_HI);
    rl_ended(log, ended_load(IN_AIR_UP_HI));
}

/*
 * @routine 01BB:341 RichterHurtClimbKindFive
 * @run 6D:B341
 * WHAT IT DOES: A hit of kind 5 while climbing: nudged off by the second table for states 5 and 7,
 * the first for 6 and 8; knocked up at Y speed $FB00.
 * HOW: TST of bit 0 of OBJ_STATE_BEFORE_HIT; RichterNudgeB or RichterNudgeA; RichterHurtKnockUp.
 * WAITS: no.
 * Entry: Y for the sound driver. Exit: A $FB, N set, Z clear; X, Y, C and V as RichterHurtKnockUp
 * left them.
 */
void RichterHurtClimbKindFive(RegLog *log, uint8_t y)
{
    uint8_t state = RD8(OBJ->state_before_hit[0]);
    Nudge nudge;

    rl_tst(log, 0x01, state);
    if (!(state & 0x01)) {
        jsr_push(0xB34A);
        nudge = RichterNudgeB();
        jsr_pop();
    } else {
        jsr_push(0xB34F);
        nudge = RichterNudgeA();
        jsr_pop();
    }
    rl_nudge(log, nudge);
    jsr_push(0xB352);
    RichterHurtKnockUp(log, nudge.facing, y);
    jsr_pop();
    WR8(OBJ->vy_lo[0], 0);
    WR8(OBJ->vy_hi[0], KIND_FIVE_UP_HI);
    rl_ended(log, ended_load(KIND_FIVE_UP_HI));
}

/*
 * @routine 01BB:35D RichterHurtWhileClimbing
 * @run 6D:B35D
 * @stop B37D
 * WHAT IT DOES: A hit in states 5 to 8: kind 5 as RichterHurtClimbKindFive; at steps 2 to 4 of the
 * state as a hit while standing; otherwise held for $20 frames at step 3, then the hit's sound and
 * effects.
 * HOW: The six bytes at $B37D by OBJ_STEP_BEFORE_HIT; branches to RichterHurtClimbKindFive and
 * RichterHitSoundAndEffects, a jump to RichterHurtFromStand.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as the routine it goes to; held, A 3 where
 * RichterHitSoundAndEffects keeps A, Y the step.
 */
void RichterHurtWhileClimbing(RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t value, step;

    value = (uint8_t)(RD8(OBJ->hit_taken[0]) & HIT_KINDS);
    rl_ended(log, ended_compare(value, HIT_KIND_FIVE));
    if (value == HIT_KIND_FIVE) {
        RichterHurtClimbKindFive(log, y);
        return;
    }
    step = RD8(OBJ->step_before_hit[0]);
    value = rd8((uint16_t)(RichterClimbStepsHurtAsStanding + step));
    rl_ended(log, ended_with_y(ended_load(value), step));
    if (value != 0) {
        RichterHurtFromStand(log, x, step);
        return;
    }
    WR8(OBJ->hit_frames[0], RESUME_FRAMES);
    WR8(OBJ->substate[0], STEP_RESUME);
    rl_ended(log, ended_load(STEP_RESUME));
    rl_hit_sound(log, RichterHitSoundAndEffects(x, step));
}

/* The end both throws share ($B413): the step, animation $1F, PlayerMayLeaveTop cleared, sound $63,
 * then the hit's effects. X is the state he was hit in. */
static void thrown_end(RegLog *log, uint8_t step, uint8_t x, uint8_t y)
{
    WR8(OBJ->substate[0], step);
    rl_ended(log, ended_load(ANIM_THROWN));
    jsr_push(0xB41D);
    rl_animation(log, RichterSetAnimation(ANIM_THROWN));
    jsr_pop();
    WR8(PVARS->may_leave_top, 0);
    rl_ended(log, ended_load(SOUND_THROWN));
    jsr_push(0xB425);
    rl_sound(log, SOUND_THROWN, SoundCommand(SOUND_THROWN, log_x(log, x, y), log_y(log, x, y)));
    jsr_pop();
    rl_hit_effects(log, RichterHitEffects(log_x(log, x, y), log_y(log, x, y)));
}

/*
 * @routine 01BB:3E1 RichterHurtKindThree
 * @run 6D:B3E1
 * WHAT IT DOES: A hit of kind 3 throws him: up at $FE50 if he was in the air, else lifted three
 * pixels and up at $FC50; across at $FD80 or $0280 by OBJ_KNOCK_RIGHT; step 7.
 * HOW: The thirteen bytes at $B428 by OBJ_STATE_BEFORE_HIT (1 for states 3, 4, 11 and 12); RichterLiftThree;
 * the two words at $B435; RichterSetAnimation $1F, SoundCommand $63, a tail jump to RichterHitEffects.
 * WAITS: no.
 * Entry: Y for the sound driver. Exit: as RichterHitEffects.
 */
void RichterHurtKindThree(RegLog *log, uint8_t y)
{
    uint8_t state, value, at, side;

    (void)y;

    state = RD8(OBJ->state_before_hit[0]);
    rl_ended_x(log, ended_nz(state), state);
    value = rd8((uint16_t)(RichterHurtFromAirStates + state));
    rl_ended(log, ended_load(value));
    if (value != 0) {
        WR8(OBJ->vy_lo[0], THROWN_AIR_LO);
        WR8(OBJ->vy_hi[0], THROWN_AIR_HI);
    } else {
        jsr_push(0xB3F8);
        rl_lift(log, RichterLiftThree());
        jsr_pop();
        WR8(OBJ->vy_lo[0], THROWN_GROUND_LO);
        WR8(OBJ->vy_hi[0], THROWN_GROUND_HI);
    }
    side = RD8(OBJ->knock_right[0]);
    at = (uint8_t)(side << 1);
    value = rd8((uint16_t)(RichterThrowSpeedsX + at));
    WR8(OBJ->vx_lo[0], value);
    value = rd8((uint16_t)(RichterThrowSpeedsX + at + 1));
    WR8(OBJ->vx_hi[0], value);
    rl_ended(log, ended_with_carry(ended_with_y(ended_load(STEP_THROWN_THREE), at), (side & 0x80) != 0));
    thrown_end(log, STEP_THROWN_THREE, state, at);
}

/*
 * @routine 01BB:439 RichterHurtKindFour
 * @run 6D:B439
 * WHAT IT DOES: A hit of kind 4 throws him: up at $F980, lifted three pixels, right at $0380; step 9.
 * HOW: RichterLiftThree, then RichterHurtKindThree's end from $B413.
 * WAITS: no.
 * Entry: Y for the sound driver. Exit: as RichterHitEffects.
 */
void RichterHurtKindFour(RegLog *log, uint8_t y)
{
    uint8_t state;

    state = RD8(OBJ->state_before_hit[0]);
    rl_ended_x(log, ended_nz(state), state);
    WR8(OBJ->vy_lo[0], THROWN_FOUR_LO);
    WR8(OBJ->vy_hi[0], THROWN_FOUR_HI);
    jsr_push(0xB449);
    rl_lift(log, RichterLiftThree());
    jsr_pop();
    WR8(OBJ->vx_lo[0], THROWN_FOUR_VX_LO);
    WR8(OBJ->vx_hi[0], THROWN_FOUR_VX_HI);
    rl_ended(log, ended_load(STEP_THROWN_FOUR));
    thrown_end(log, STEP_THROWN_FOUR, state, y);
}

/*
 * @routine 01BB:245 RichterHurtStart
 * @run 6D:B245
 * WHAT IT DOES: The hit's step 0: stands him on a floor a pixel below if there is one, clears the
 * player bytes the hit ends, works out which way the hit throws him, and goes on by the hit's kind
 * (3 and 4 throw him) or else by the state he was hit in. A spike (kind 6) throws him back from his
 * facing and is taken as kind 2.
 * HOW: The floor checks twice; RichterLandOnFloor; OBJ_KNOCK_RIGHT 1 when the object that hit him
 * (OBJ_HIT_BY_SLOT) is left of him; RichterStopMoving; the thirteen words at $B2C1 by
 * OBJ_STATE_BEFORE_HIT.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as the routine it goes to.
 */
void RichterHurtStart(RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t value, kind, hitter, state;
    uint16_t entry;

    if (hurt_floor_twice(log, true, 0xB248, 0xB24B, 0xB253, 0xB256)) {
        jsr_push(0xB25B);
        rl_landing(log, RichterLandOnFloor());
        jsr_pop();
    }
    jsr_push(0xB25E);
    RichterPadOverridesOff();
    jsr_pop();
    WR8(PVARS->attack_kind, 0);
    WR8(PVARS->held, 0);
    WR8(PVARS->held_count, 0);
    WR8(PVARS->slow_timer_lo, 0);
    WR8(PVARS->slow_timer_hi, 0);
    WR8(PVARS2->jump_kind, 0);
    value = RD8(OBJ->facing_left[0]);
    WR8(OBJ->knock_right[0], value);
    kind = (uint8_t)(RD8(OBJ->hit_taken[0]) & HIT_KINDS);
    rl_ended(log, ended_compare(kind, HIT_KIND_SPIKE));
    if (kind == HIT_KIND_SPIKE) {
        WR8(OBJ->hit_taken[0], HIT_SPIKE_BECOMES);
        rl_ended(log, ended_load(HIT_SPIKE_BECOMES));
    } else {
        WR8(OBJ->knock_right[0], 0);
        hitter = RD8(OBJ->hit_by_slot[0]);
        value = RD8(OBJ->x_hi[hitter]);
        rl_ended_x(log, ended_load(value), hitter);
        if (value != 0) {
            if (value & 0x80) {
                rl_ended(log, ended_nz(INC8(OBJ->knock_right[0])));
            }
        } else {
            uint8_t mine;

            value = RD8(OBJ->x_lo[hitter]);
            mine = RD8(OBJ->x_lo[0]);
            rl_ended(log, ended_compare(value, mine));
            if (value < mine) {
                rl_ended(log, ended_nz(INC8(OBJ->knock_right[0])));
            }
        }
    }
    kind = (uint8_t)(RD8(OBJ->hit_taken[0]) & HIT_KINDS);
    rl_ended(log, ended_compare(kind, HIT_KIND_THREE));
    if (kind == HIT_KIND_THREE) {
        RichterHurtKindThree(log, log_y(log, x, y));
        return;
    }
    rl_ended(log, ended_compare(kind, HIT_KIND_FOUR));
    if (kind == HIT_KIND_FOUR) {
        RichterHurtKindFour(log, log_y(log, x, y));
        return;
    }
    jsr_push(0xB2B9);
    RichterStopMoving();
    jsr_pop();
    state = RD8(OBJ->state_before_hit[0]);
    value = (uint8_t)(state << 1);
    rl_ended_x(log, ended_with_carry(ended_load(value), (state & 0x80) != 0), value);
    entry = (uint16_t)(RichterHurtByState + value);
    entry = (uint16_t)(rd8(entry) | (rd8((uint16_t)(entry + 1)) << 8));
    y = log_y(log, x, y);
    x = value;
    switch (entry) {
    case 0xB2DB:
        RichterHurtFromStand(log, x, y);
        break;
    case 0xB30F:
        RichterHurtFromCrouch(log, x, y);
        break;
    case 0xB333:
        RichterHurtInAir(log, x, y);
        break;
    default:
        RichterHurtWhileClimbing(log, x, y);
        break;
    }
}

/*
 * @routine 01BB:457 RichterHurtThrown
 * @run 6D:B457
 * WHAT IT DOES: The throw's steps 7 and 9: falls with gravity and the animation until he lands on a
 * floor on the first screen row while falling, its column answering 0; then he stands on it, slots 1 and 2 go to kind
 * 4, and the next step starts with animation $20 (and sound 2) if the animation had stopped, else
 * $21; its X speed by OBJ_KNOCK_RIGHT, or right from step $0A.
 * HOW: RichterReadColumns, RichterFloorCheck, RichterSolidUnderColumn, RichterColumnRow;
 * RichterDebrisOnMarkedColumns while in the air; RichterFallGravity, a tail jump to
 * RichterAnimationTick; the two words at $B4C1.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: in the air, as RichterAnimationTick. Landed, A the
 * speed's high byte, N and Z from it, Y twice the index, C bit 7 of the index; X 2; V as
 * RichterSetAnimation left it.
 */
void RichterHurtThrown(RegLog *log, uint8_t x, uint8_t y)
{
    FloorResult floor = { FLOOR_NONE, 0, 0, 0, 0 };
    ColumnRead under;
    uint8_t value, column, step, at, side;
    bool landed = false;

    jsr_push(0xB45A);
    rl_columns(log, RichterReadColumns());
    jsr_pop();
    value = RD8(OBJ->y_hi[0]);
    rl_ended(log, ended_load(value));
    if (value == 0) {
        value = RD8(OBJ->vy_hi[0]);
        rl_ended(log, ended_load(value));
        if (!(value & 0x80)) {
            jsr_push(0xB467);
            floor = RichterFloorCheck();
            jsr_pop();
            rl_floor(log, floor);
            if (floor.how != FLOOR_NONE) {
                column = floor.column;
                jsr_push(0xB46C);
                under = RichterSolidUnderColumn(column);
                jsr_pop();
                rl_solid(log, under);
                landed = solid_answer(under) == 0;
            }
            if (!landed) {
                jsr_push(0xB494);
                rl_debris(log, RichterDebrisOnMarkedColumns(log_x(log, x, y), log_y(log, x, y)));
                jsr_pop();
            }
        }
    }
    if (!landed) {
        jsr_push(0xB497);
        rl_gravity(log, RichterFallGravity());
        jsr_pop();
        rl_animation(log, RichterAnimationTick());
        return;
    }
    {
        uint8_t row;
        Sum diff;

        value = RD8(OBJ->y_lo[0]);
        row = RD8(PVARS->column_row[floor.column]);
        diff = add_bytes(value, (uint8_t)~row, true);
        WR8(OBJ->y_lo[0], diff.value);
    }
    WR8(OBJ->vy_lo[0], 0);
    WR8(OBJ->vy_hi[0], 0);
    jsr_push(0xB481);
    rl_slots(log, RichterReplaceSlotsOneTwo());
    jsr_pop();
    value = RD8(OBJ->hurt_timer[0]);
    rl_ended(log, ended_compare(value, ANIMATION_STOPPED));
    if (value == ANIMATION_STOPPED) {
        rl_ended(log, ended_load(SOUND_THROWN_LAND));
        jsr_push(0xB48D);
        rl_sound(log, SOUND_THROWN_LAND, SoundCommand(SOUND_THROWN_LAND, 2, log_y(log, x, y)));
        jsr_pop();
        value = ANIM_THROWN_LAND_STOPPED;
    } else {
        value = ANIM_THROWN_LAND;
    }
    rl_ended(log, ended_load(value));
    jsr_push(0xB49F);
    rl_animation(log, RichterSetAnimation(value));
    jsr_pop();
    INC8(OBJ->substate[0]);
    step = RD8(OBJ->substate[0]);
    side = step == STEP_THROWN_LAST ? 1 : RD8(OBJ->knock_right[0]);
    at = (uint8_t)(side << 1);
    value = rd8((uint16_t)(RichterThrownLandSpeedsX + at));
    WR8(OBJ->vx_lo[0], value);
    value = rd8((uint16_t)(RichterThrownLandSpeedsX + at + 1));
    WR8(OBJ->vx_hi[0], value);
    rl_ended(log, ended_with_carry(ended_with_y(ended_load(value), at), (side & 0x80) != 0));
}

/*
 * @routine 01BB:4C5 RichterHurtThrownLanded
 * @run 6D:B4C5
 * WHAT IT DOES: The throw's steps 8 and $0A: slots 1 and 2 to kind 4 every eighth frame; while a
 * floor is under him or a pixel below, stands on it and, once the animation ends, crouches; with no
 * floor goes back a step.
 * HOW: TST of zWaveCounter's low three bits; RichterReplaceSlotsOneTwo; the floor checks twice;
 * RichterLandOnFloor, RichterAnimationWait, a tail jump to RichterHurtToCrouch.
 * WAITS: no.
 * Entry: nothing. Exit: back a step, N and Z from the step. Landed, as RichterAnimationWait, or as
 * RichterHurtToCrouch.
 */
void RichterHurtThrownLanded(RegLog *log)
{
    AnimationWaitEnd wait;
    uint8_t wave = RD8(ZP->wave_counter);

    rl_tst(log, WAVE_EVERY_EIGHTH, wave);
    if (!(wave & WAVE_EVERY_EIGHTH)) {
        jsr_push(0xB4CD);
        rl_slots(log, RichterReplaceSlotsOneTwo());
        jsr_pop();
    }
    if (!hurt_floor_twice(log, true, 0xB4D0, 0xB4D3, 0xB4DB, 0xB4DE)) {
        rl_ended(log, ended_nz(DEC8(OBJ->substate[0])));
        return;
    }
    jsr_push(0xB4E3);
    rl_landing(log, RichterLandOnFloor());
    jsr_pop();
    jsr_push(0xB4E6);
    wait = RichterAnimationWait();
    rl_animation(log, wait.step);
    if (wait.dropped) {
        return;
    }
    jsr_pop();
    RichterHurtToCrouch();
    rl_ended(log, ended_load(0));
}

/*
 * @routine 01BB:576 RichterHurtSlide
 * @run 6D:B576
 * WHAT IT DOES: The slide's step: falls if no floor is under him or a pixel below; else stands on it
 * and, when OBJ_HIT_FRAMES runs out, crouches.
 * HOW: RichterFloorCheck, again after RichterReadColumns a pixel lower; a tail jump to RichterToFall;
 * RichterLandOnFloor; a branch to RichterHurtToCrouch.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterToFall; counting, N and Z from the count; as RichterHurtToCrouch.
 */
void RichterHurtSlide(RegLog *log)
{
    if (!hurt_floor_twice(log, false, 0, 0xB579, 0xB581, 0xB584)) {
        rl_fall(log, RichterToFall());
        return;
    }
    jsr_push(0xB58C);
    rl_landing(log, RichterLandOnFloor());
    jsr_pop();
    {
        uint8_t left = DEC8(OBJ->hit_frames[0]);

        if (left != 0) {
            rl_ended(log, ended_nz(left));
            return;
        }
    }
    RichterHurtToCrouch();
    rl_ended(log, ended_load(0));
}

/*
 * @routine 01BB:574 RichterHurtSlideStart
 * @run 6D:B574
 * WHAT IT DOES: The slide's first step: the knock speed, then as RichterHurtSlide.
 * HOW: RichterKnockSpeedXFromHit, then runs on into RichterHurtSlide.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterHurtSlide.
 */
void RichterHurtSlideStart(RegLog *log)
{
    KnockSpeed speed;

    jsr_push(0xB576);
    speed = RichterKnockSpeedXFromHit();
    jsr_pop();
    rl_ended(log, ended_with_carry(ended_with_y(ended_load(speed.high), (uint8_t)(speed.side << 1)),
                                   (speed.side & 0x80) != 0));
    RichterHurtSlide(log);
}

/*
 * @routine 01BB:5D7 RichterHurtHoldFrame
 * @run 6D:B5D7
 * WHAT IT DOES: Runs the animation's time but keeps the frame he shows; stands him when it ends.
 * HOW: OBJ_FRAME pushed around RichterAnimationTick; OBJ_HURT_TIMER against $FF; a tail jump to
 * RichterStand.
 * WAITS: no.
 * Entry: nothing. Exit: running, A the timer, N, Z and C from comparing it with $FF; ended, as
 * RichterStand.
 */
void RichterHurtHoldFrame(RegLog *log)
{
    uint8_t value;

    value = RD8(OBJ->frame[0]);
    push8(value);
    jsr_push(0xB5DE);
    rl_animation(log, RichterAnimationTick());
    jsr_pop();
    value = pull8();
    WR8(OBJ->frame[0], value);
    value = RD8(OBJ->hurt_timer[0]);
    rl_ended(log, ended_compare(value, ANIMATION_STOPPED));
    if (value == ANIMATION_STOPPED) {
        rl_animation(log, RichterStand());
    }
}

/*
 * @routine 01BB:5ED RichterHurtDownStep
 * @run 6D:B5ED
 * WHAT IT DOES: The step he is down in (6): falls if no floor is under him or a pixel below; else
 * stands on it; with PlayerHeld set holds the frame as RichterHurtHoldFrame, else runs the
 * animation and stands him when it ends.
 * HOW: RichterFloorCheck twice, RichterToFall, RichterLandOnFloor; RichterAnimationRunning and a tail
 * jump to RichterStand.
 * WAITS: no.
 * Entry: nothing. Exit: as the routine it goes to; running, as RichterAnimationRunning.
 */
void RichterHurtDownStep(RegLog *log)
{
    AnimationStep running;
    uint8_t value;

    if (!hurt_floor_twice(log, false, 0, 0xB5F0, 0xB5F8, 0xB5FB)) {
        rl_fall(log, RichterToFall());
        return;
    }
    jsr_push(0xB603);
    rl_landing(log, RichterLandOnFloor());
    jsr_pop();
    value = RD8(PVARS->held);
    rl_ended(log, ended_load(value));
    if (value != 0) {
        RichterHurtHoldFrame(log);
        return;
    }
    jsr_push(0xB60B);
    running = RichterAnimationRunning();
    jsr_pop();
    rl_animation(log, running);
    if (running.nz == 0) {
        rl_animation(log, RichterStand());
    }
}

/* The wrappers of the routines that keep an exit log. */
#define LOG_ABI_XY(name) void abi_##name(void) { RegLog log = { 0 }; name(&log, cpu.x, cpu.y); reg_log_replay(&log); }
#define LOG_ABI_Y(name) void abi_##name(void) { RegLog log = { 0 }; name(&log, cpu.y); reg_log_replay(&log); }
#define LOG_ABI(name) void abi_##name(void) { RegLog log = { 0 }; name(&log); reg_log_replay(&log); }
LOG_ABI_XY(RichterHurtKnockUp)
LOG_ABI_XY(RichterHurtDown)
LOG_ABI_XY(RichterHurtFromStand)
LOG_ABI_XY(RichterHurtFromCrouch)
LOG_ABI_XY(RichterHurtInAir)
LOG_ABI_Y(RichterHurtClimbKindFive)
LOG_ABI_XY(RichterHurtWhileClimbing)
LOG_ABI_Y(RichterHurtKindThree)
LOG_ABI_Y(RichterHurtKindFour)
LOG_ABI_XY(RichterHurtStart)
LOG_ABI_XY(RichterHurtThrown)
LOG_ABI(RichterHurtThrownLanded)
LOG_ABI(RichterHurtSlide)
LOG_ABI(RichterHurtSlideStart)
LOG_ABI(RichterHurtHoldFrame)
LOG_ABI(RichterHurtDownStep)

enum { HELD_FALL_HI = 0x02, HURT_FALL_HI = 0x07 };

/* Held ($B54E, $B558): Y speed $0200, then RichterHurtLandSound twice, the second as a tail. */
static void held_land_sound(RegLog *log, uint8_t y, uint16_t ret)
{
    HurtLandSound r;

    WR8(OBJ->vy_hi[0], HELD_FALL_HI);
    rl_ended(log, ended_load(HELD_FALL_HI));
    jsr_push(ret);
    r = RichterHurtLandSound(log_y(log, 0, y));
    jsr_pop();
    rl_hurt_land(log, r);
    rl_hurt_land(log, RichterHurtLandSound(log_y(log, 0, y)));
}

/*
 * @routine 01BB:534 RichterHurtLandOnMarked
 * @run 6D:B534
 * WHAT IT DOES: The knock lands on a marked floor: held, Y speed $0200 and the landing's sound twice;
 * else full fall speed and as RichterLandOnMarked.
 * HOW: PlayerHeld; a jump to RichterLandOnMarked; the held end is RichterHurtLandOnMarkedFeet's ($B54E).
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as RichterLandOnMarked, or as RichterHurtLandSound.
 */
void RichterHurtLandOnMarked(RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t value = RD8(PVARS->held);

    rl_ended(log, ended_load(value));
    if (value != 0) {
        held_land_sound(log, y, 0xB556);
        return;
    }
    WR8(OBJ->vy_hi[0], HURT_FALL_HI);
    rl_ended(log, ended_load(HURT_FALL_HI));
    RichterLandOnMarked(log, x, y);
}

/*
 * @routine 01BB:541 RichterHurtLandOnMarkedFeet
 * @run 6D:B541
 * WHAT IT DOES: As RichterHurtLandOnMarked for marked ground under his feet: held, Y speed $0200 and the
 * landing's sound twice; else full fall speed and as RichterLandOnMarkedFeet.
 * HOW: PlayerHeld; a jump to RichterLandOnMarkedFeet.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as RichterLandOnMarkedFeet, or as RichterHurtLandSound.
 */
void RichterHurtLandOnMarkedFeet(RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t value = RD8(PVARS->held);

    rl_ended(log, ended_load(value));
    if (value != 0) {
        held_land_sound(log, y, 0xB560);
        return;
    }
    WR8(OBJ->vy_hi[0], HURT_FALL_HI);
    rl_ended(log, ended_load(HURT_FALL_HI));
    RichterLandOnMarkedFeet(log, x, y);
}

/*
 * @routine 01BB:4ED RichterHurtKnocked
 * @run 6D:B4ED
 * WHAT IT DOES: The knock's steps 2, 4 and 5: the knock speed; rising, gravity; falling, lands on a floor
 * whose column answers 0 (held: at Y speed $0200, then the landing's sound; else at full fall speed), or
 * on a marked floor or marked ground, or debris on marked columns and gravity.
 * HOW: RichterKnockSpeedXFromHit; the landing check; RichterLandOnColumn; RichterOnMarkedFloor,
 * RichterOnMarkedUnderFeet; RichterDebrisOnMarkedColumns; a tail jump to RichterFallGravity. Held, it
 * runs on into RichterHurtLandSound after the landing.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as the path's last routine.
 */
void RichterHurtKnocked(RegLog *log, uint8_t x, uint8_t y)
{
    KnockSpeed speed;
    FloorResult floor;
    ColumnRead under;
    MarkedFloor marked;
    uint8_t value;

    jsr_push(0xB4EF);
    speed = RichterKnockSpeedXFromHit();
    jsr_pop();
    rl_ended(log, ended_with_carry(ended_with_y(ended_load(speed.high), (uint8_t)(speed.side << 1)),
                                   (speed.side & 0x80) != 0));
    value = RD8(OBJ->vy_hi[0]);
    rl_ended(log, ended_load(value));
    if (!(value & 0x80)) {
        bool land = false;

        jsr_push(0xB4F7);
        rl_columns(log, RichterReadColumns());
        jsr_pop();
        jsr_push(0xB4FA);
        floor = RichterFloorCheck();
        jsr_pop();
        rl_floor(log, floor);
        if (floor.how != FLOOR_NONE) {
            jsr_push(0xB4FF);
            under = RichterSolidUnderColumn(floor.column);
            jsr_pop();
            rl_solid(log, under);
            land = solid_answer(under) == 0;
        }
        if (land) {
            uint8_t lx, ly;

            value = RD8(PVARS->held);
            rl_ended(log, ended_load(value));
            if (value == 0) {
                WR8(OBJ->vy_hi[0], HURT_FALL_HI);
                rl_ended(log, ended_load(HURT_FALL_HI));
                rl_xy(log, x, y, &lx, &ly);
                RichterLandOnColumn(log, floor.column, floor.column, ly);
                return;
            }
            WR8(OBJ->vy_hi[0], HELD_FALL_HI);
            rl_ended(log, ended_load(HELD_FALL_HI));
            rl_xy(log, x, y, &lx, &ly);
            jsr_push(0xB52C);
            RichterLandOnColumn(log, floor.column, floor.column, ly);
            jsr_pop();
            rl_hurt_land(log, RichterHurtLandSound(log_y(log, x, y)));
            return;
        }
        jsr_push(0xB511);
        marked = RichterOnMarkedFloor();
        jsr_pop();
        rl_marked(log, marked);
        if (marked.how == FLOOR_MARKED || marked.how == FLOOR_MARKED_DOWN) {
            RichterHurtLandOnMarked(log, log_x(log, x, y), log_y(log, x, y));
            return;
        }
        jsr_push(0xB519);
        marked = RichterOnMarkedUnderFeet(log);
        jsr_pop();
        if (marked.how == FLOOR_MARKED || marked.how == FLOOR_MARKED_DOWN) {
            RichterHurtLandOnMarkedFeet(log, log_x(log, x, y), log_y(log, x, y));
            return;
        }
        jsr_push(0xB521);
        rl_debris(log, RichterDebrisOnMarkedColumns(log_x(log, x, y), log_y(log, x, y)));
        jsr_pop();
    }
    rl_gravity(log, RichterFallGravity());
}

/*
 * @routine 01BB:227 RichterStateHurt
 * @run 6D:B227
 * WHAT IT DOES: State 9, the hit's reaction: runs the step's handler.
 * HOW: The twelve words at $B22D by RichterStepIndex.
 * WAITS: no.
 * Entry: Y for the sound driver. Exit: as the handler leaves them.
 */
void RichterStateHurt(RegLog *log, uint8_t y)
{
    uint8_t step, at;
    uint16_t entry, handler;

    jsr_push(0xB22A);
    step = RichterStepIndex();
    jsr_pop();
    at = (uint8_t)(step << 1);
    rl_ended_x(log, ended_with_carry(ended_load(at), (step & 0x80) != 0), at);
    entry = (uint16_t)(RichterHurtSteps + at);
    handler = (uint16_t)(rd8(entry) | (rd8((uint16_t)(entry + 1)) << 8));
    switch (handler) {
    case 0xB245:
        RichterHurtStart(log, at, y);
        break;
    case 0xB574:
        RichterHurtSlideStart(log);
        break;
    case 0xB4ED:
        RichterHurtKnocked(log, at, y);
        break;
    case 0xB5AE:
        rl_hurt_resume(log, RichterHurtResume());
        break;
    case 0xB5ED:
        RichterHurtDownStep(log);
        break;
    case 0xB457:
        RichterHurtThrown(log, at, y);
        break;
    case 0xB4C5:
        RichterHurtThrownLanded(log);
        break;
    default:
        RichterHurtSlide(log);
        break;
    }
}

LOG_ABI_XY(RichterHurtLandOnMarked)
LOG_ABI_XY(RichterHurtLandOnMarkedFeet)
LOG_ABI_XY(RichterHurtKnocked)
LOG_ABI_Y(RichterStateHurt)
