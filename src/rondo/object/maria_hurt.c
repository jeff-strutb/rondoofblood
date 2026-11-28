/*
 * Maria's hit (state 9, 01DF in bank $6D): its start by the state she was hit in (MariaHurtByState), the hit's
 * effects by bits 4 to 6 of OBJ_HIT_TAKEN, the throws of kinds 3 and 4, and the steps by OBJ_SUBSTATE
 * (MariaHurtSteps): start, the slide, knocked up, resume, down, thrown, thrown and landed. Richter's are in
 * richter_hurt.c; hers have her own numbers and, from the hold (PlayerHeld), a landing on a marked floor that never
 * returns (MariaHurtHeldLanding).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"

enum {
    OBJECT_PLAYER = 0, SLOT_SEQUENCE = 0x0F,
    HIT_KIND_MASK = 0x07, HIT_LOW_MASK = 0x03, HIT_KIND_THREE = 3, HIT_KIND_FOUR = 4, HIT_KIND_FIVE = 5,
    HIT_KIND_SPIKE = 6, HIT_KIND_SPIKE_AS = 2,
    HIT_HOLD = 0x10, HIT_HELD = 0x20, HIT_SLOW = 0x40, HIT_SOUNDLESS = 0x60,
    STATE_CROUCH = 2,
    STEP_KNOCKED = 2, STEP_RESUME = 3, STEP_DOWN = 6, STEP_SLIDE = 1, STEP_SLIDE_LOW = 0x0B, STEP_THROWN = 7,
    STEP_THROWN_FAR = 9, STEP_HOLD_STEP = 1, STEP_HOLD_STEP_B = 5,
    ANIMATION_KNOCKED = 0x22, ANIMATION_DOWN = 0x1A, ANIMATION_THROWN = 0x1B, ANIMATION_THROWN_LAND = 0x1D,
    ANIMATION_THROWN_HELD = 0x24, FRAME_SLIDE = 0x16, FRAME_CLIMB_HIT = 0x2C, FRAME_CROUCHED = 0x1B,
    SLIDE_FRAMES = 8, CLIMB_HIT_FRAMES = 0x20, HOLD_STOPPED = 0xFF,
    KNOCK_VY_HI = 0xFC, AIR_VY_HI = 0xFE, CLIMB_FIVE_VY_HI = 0xFB, THROW_VY_LO = 0x50, THROW_GROUND_VY_HI = 0xFC,
    THROW_AIR_VY_HI = 0xFE, THROW_FOUR_VY_HI = 0xF9, THROW_FOUR_VX_LO = 0x80, THROW_FOUR_VX_HI = 0x04,
    LAND_VY_HI = 7, HELD_VY_HI = 2,
    SOUND_HIT = 0x66, SOUND_HOLD = 0x12, SOUND_SLOW = 0x24, SOUND_THROW = 0x67, SOUND_LANDED = 2, SOUND_LAND_HURT = 0x4A,
    SCRIPT_HOLD = 0x0A, SCRIPT_HELD = 1, SCRIPT_SLOW = 0x1C, HOLD_KIND = 1, HOLD_TIMER = 0x0A,
    SLOW_TIMER_LO = 0xFF, SLOW_TIMER_HI = 1, THROWN_LAND_EVERY = 0x07,
    HURT_FROM_STAND = 0xB36C, HURT_FROM_CROUCH = 0xB3A4, HURT_IN_AIR = 0xB3C8, HURT_WHILE_CLIMBING = 0xB3E4,
    HURT_KNOCK_UP = 0xB37C, HURT_FROM_ITEM4 = 0xB360, HURT_FROM_SEQUENCE = 0xB365,
    STEP_START_AT = 0xB2F3, STEP_SLIDE_START_AT = 0xB61D, STEP_KNOCKED_AT = 0xB593, STEP_RESUME_AT = 0xB657,
    STEP_DOWN_AT = 0xB68A, STEP_THROWN_AT = 0xB50D, STEP_LANDED_AT = 0xB56E, STEP_SLIDE_AT = 0xB61F
};

#define MariaHurtByState_TBL MariaHurtByState
#define MariaHurtThrowKinds_TBL MariaHurtThrowKinds
#define MariaThrowSpeedsX_TBL MariaThrowSpeedsX
#define MariaThrownLandSpeedsX_TBL MariaThrownLandSpeedsX
#define MariaKnockSpeeds_TBL MariaKnockSpeeds
#define MariaHurtSteps_TBL MariaHurtSteps

/* A SoundCommand at a JSR returning to `ret`, given the X and Y the chain leaves. */
static void sound_at(MExit *e, uint8_t command, uint16_t ret, uint8_t x, uint8_t y)
{
    mx_regs(e, r_lda(regs_kept(), command));
    jsr_push(ret);
    mx_sound(e, command, SoundCommand(command, mx_x_from(x, y, *e), mx_y_from(x, y, *e)));
    jsr_pop();
}

/* ------------------------------------------------------------------------------------------------ the hit's start */

/*
 * @routine 01DF:2A6 MariaHurtSide
 * @run 6D:B2A6
 * WHAT IT DOES: Says which way the hit knocks her (OBJ_KNOCK_RIGHT): a spike (kind 6, made kind 2) the way she faces;
 * kind 4 to the right; else away from the object that hit her (OBJ_HIT_BY_SLOT) by its X; and OBJ_KNOCK_FORWARD 1 when
 * that is the way she faces.
 * HOW: The kind in bits 0 to 2 of OBJ_HIT_TAKEN; the hitter's X high byte's sign, or its X low against hers.
 * WAITS: no.
 * Entry: nothing. Exit: A OBJ_KNOCK_FORWARD, N and Z from it; X the hitter's slot when it was read; C from the last
 * compare made. Y and V kept.
 */
Regs MariaHurtSide(void)
{
    Regs r = regs_kept();
    uint8_t value, kind, slot;

    value = RD8(OBJ->facing_left[OBJECT_PLAYER]);
    WR8(OBJ->knock_right[OBJECT_PLAYER], value);
    kind = (uint8_t)(RD8(OBJ->hit_taken[OBJECT_PLAYER]) & HIT_KIND_MASK);
    r = r_cmp(r, kind, HIT_KIND_SPIKE);
    if (kind == HIT_KIND_SPIKE) {
        WR8(OBJ->hit_taken[OBJECT_PLAYER], HIT_KIND_SPIKE_AS);
    } else {
        WR8(OBJ->knock_right[OBJECT_PLAYER], 0);
        kind = (uint8_t)(RD8(OBJ->hit_taken[OBJECT_PLAYER]) & HIT_KIND_MASK);
        r = r_cmp(r, kind, HIT_KIND_FOUR);
        if (kind == HIT_KIND_FOUR) {
            INC8(OBJ->knock_right[OBJECT_PLAYER]);
        } else {
            slot = RD8(OBJ->hit_by_slot[OBJECT_PLAYER]);
            r = r_x_only(r, slot);
            value = RD8(OBJ->x_hi[slot]);
            if (value != 0) {
                if (value & 0x80) {
                    INC8(OBJ->knock_right[OBJECT_PLAYER]);
                }
            } else {
                value = RD8(OBJ->x_lo[slot]);
                uint8_t mine = RD8(OBJ->x_lo[OBJECT_PLAYER]);

                r = r_cmp(r, value, mine);
                if (value < mine) {
                    INC8(OBJ->knock_right[OBJECT_PLAYER]);
                }
            }
        }
    }
    value = RD8(OBJ->knock_right[OBJECT_PLAYER]);
    value ^= RD8(OBJ->facing_left[OBJECT_PLAYER]);
    value &= 0x01;
    WR8(OBJ->knock_forward[OBJECT_PLAYER], value);
    return r_lda(r, value);
}

void abi_MariaHurtSide(void)
{
    regs_exit(MariaHurtSide());
}

/*
 * @routine 01DF:409 MariaHitEffects
 * @run 6D:B409
 * WHAT IT DOES: What a hit does besides hurting, by bits 4 to 6 of OBJ_HIT_TAKEN, the first set of them: bit 4 empties
 * slots 1 and 2 and makes them kind 1 (slot 1's timer $0A), script $0A; bit 5 the hold (sound $12, PlayerHeld counted
 * up), script 1; bit 6 the slowing (sound $24, PlayerSlowTimer $01FF), script $1C. The hit is cleared.
 * HOW: TST of OBJ_HIT_TAKEN; ClearObject, SoundCommand; the tail JMP to MariaStartHitScript.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: none of them, A kept, Z set from the last TST, N and V its bits 7 and 6; else as
 * MariaStartHitScript.
 */
MExit MariaHitEffects(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t hit, script;

    hit = RD8(OBJ->hit_taken[OBJECT_PLAYER]);
    mx_regs(&e, r_tst(regs_kept(), HIT_HOLD, hit));
    if (hit & HIT_HOLD) {
        jsr_push(0xB424);
        ClearObject(1);
        jsr_pop();
        jsr_push(0xB428);
        ClearObject(2);
        jsr_pop();
        WR8(OBJ->kind[1], HOLD_KIND);
        WR8(OBJ->kind[2], HOLD_KIND);
        WR8(OBJ->anim_timer[1], HOLD_TIMER);
        mx_regs(&e, r_ldx(regs_kept(), 2));
        script = SCRIPT_HOLD;
    } else {
        hit = RD8(OBJ->hit_taken[OBJECT_PLAYER]);
        mx_regs(&e, r_tst(regs_kept(), HIT_HELD, hit));
        if (hit & HIT_HELD) {
            sound_at(&e, SOUND_HOLD, 0xB442, x, y);
            INC8(PVARS->held);
            script = SCRIPT_HELD;
        } else {
            hit = RD8(OBJ->hit_taken[OBJECT_PLAYER]);
            mx_regs(&e, r_tst(regs_kept(), HIT_SLOW, hit));
            if (!(hit & HIT_SLOW)) {
                WR8(OBJ->hit_taken[OBJECT_PLAYER], 0);
                return e;
            }
            sound_at(&e, SOUND_SLOW, 0xB44E, x, y);
            WR8(PVARS->slow_timer_lo, SLOW_TIMER_LO);
            WR8(PVARS->slow_timer_hi, SLOW_TIMER_HI);
            script = SCRIPT_SLOW;
        }
    }
    WR8(OBJ->hit_taken[OBJECT_PLAYER], 0);
    MariaStartHitScript(script);
    mx_script(&e, script);
    return e;
}

void abi_MariaHitEffects(void)
{
    mx_exit(MariaHitEffects(cpu.x, cpu.y));
}

/*
 * @routine 01DF:3FE MariaHitSoundAndEffects
 * @run 6D:B3FE
 * WHAT IT DOES: Sound $66 for a hit with neither bit 5 nor bit 6 of OBJ_HIT_TAKEN, then the hit's effects.
 * HOW: TST, SoundCommand, and on into MariaHitEffects.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaHitEffects.
 */
MExit MariaHitSoundAndEffects(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t hit = RD8(OBJ->hit_taken[OBJECT_PLAYER]);

    mx_regs(&e, r_tst(regs_kept(), HIT_SOUNDLESS, hit));
    if (!(hit & HIT_SOUNDLESS)) {
        sound_at(&e, SOUND_HIT, 0xB409, x, y);
    }
    mx_then(&e, MariaHitEffects(mx_x_from(x, y, e), mx_y_from(x, y, e)));
    return e;
}

void abi_MariaHitSoundAndEffects(void)
{
    mx_exit(MariaHitSoundAndEffects(cpu.x, cpu.y));
}

/*
 * @routine 01DF:37C MariaHurtKnockUp
 * @run 6D:B37C
 * WHAT IT DOES: Knocks her up: step 2, animation $22 (plus OBJ_KNOCK_FORWARD), Y speed $FC00, then the hit's sound and
 * effects.
 * HOW: MariaSetAnimation, the stores, the JMP to MariaHitSoundAndEffects.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaHitSoundAndEffects.
 */
MExit MariaHurtKnockUp(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    Sum animation;

    WR8(OBJ->substate[OBJECT_PLAYER], STEP_KNOCKED);
    animation = add_bytes(ANIMATION_KNOCKED, RD8(OBJ->knock_forward[OBJECT_PLAYER]), false);
    mx_regs(&e, r_v(r_c(r_lda(regs_kept(), animation.value), animation.carry), animation.overflow));
    jsr_push(0xB38A);
    mx_animation(&e, MariaSetAnimation(animation.value));
    jsr_pop();
    WR8(OBJ->vy_lo[OBJECT_PLAYER], 0);
    WR8(OBJ->vy_hi[OBJECT_PLAYER], KNOCK_VY_HI);
    mx_regs(&e, r_lda(regs_kept(), KNOCK_VY_HI));
    mx_then(&e, MariaHitSoundAndEffects(mx_x_from(x, y, e), mx_y_from(x, y, e)));
    return e;
}

void abi_MariaHurtKnockUp(void)
{
    mx_exit(MariaHurtKnockUp(cpu.x, cpu.y));
}

/*
 * @routine 01DF:397 MariaHurtDown
 * @run 6D:B397
 * WHAT IT DOES: Puts her down: step 6 with animation $1A, then the hit's sound and effects.
 * HOW: MariaSetAnimation, the JMP to MariaHitSoundAndEffects.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaHitSoundAndEffects.
 */
MExit MariaHurtDown(uint8_t x, uint8_t y)
{
    MExit e = mx_none();

    WR8(OBJ->substate[OBJECT_PLAYER], STEP_DOWN);
    mx_regs(&e, r_lda(regs_kept(), ANIMATION_DOWN));
    jsr_push(0xB3A1);
    mx_animation(&e, MariaSetAnimation(ANIMATION_DOWN));
    jsr_pop();
    mx_then(&e, MariaHitSoundAndEffects(mx_x_from(x, y, e), mx_y_from(x, y, e)));
    return e;
}

void abi_MariaHurtDown(void)
{
    mx_exit(MariaHurtDown(cpu.x, cpu.y));
}

/*
 * @routine 01DF:36C MariaHurtFromStand
 * @run 6D:B36C
 * WHAT IT DOES: A hit while standing or walking: knocked up at step 1 or unless the hit's low bits are 1; then down.
 * HOW: OBJ_STEP_BEFORE_HIT and bits 0 and 1 of OBJ_HIT_TAKEN; on into MariaHurtKnockUp or the JMP to MariaHurtDown.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaHurtKnockUp or MariaHurtDown.
 */
MExit MariaHurtFromStand(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value = RD8(OBJ->step_before_hit[OBJECT_PLAYER]);

    mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, 1));
    if (value != 1) {
        value = (uint8_t)(RD8(OBJ->hit_taken[OBJECT_PLAYER]) & HIT_LOW_MASK);
        mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, 1));
        if (value == 1) {
            mx_then(&e, MariaHurtDown(x, y));
            return e;
        }
    }
    mx_then(&e, MariaHurtKnockUp(x, y));
    return e;
}

void abi_MariaHurtFromStand(void)
{
    mx_exit(MariaHurtFromStand(cpu.x, cpu.y));
}

/*
 * @routine 01DF:3A4 MariaHurtFromCrouch
 * @run 6D:B3A4
 * WHAT IT DOES: A hit while crouched: knocked up unless at step 0; otherwise a slide, step $0B when the hit's low bits
 * are 1 else step 1, frame $16 for 8 frames; then the hit's sound and effects.
 * HOW: OBJ_STEP_BEFORE_HIT and bits 0 and 1 of OBJ_HIT_TAKEN; the JMPs to MariaHurtKnockUp and MariaHitSoundAndEffects.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaHurtKnockUp or MariaHitSoundAndEffects.
 */
MExit MariaHurtFromCrouch(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value = RD8(OBJ->step_before_hit[OBJECT_PLAYER]), step;

    mx_regs(&e, r_lda(regs_kept(), value));
    if (value != 0) {
        mx_then(&e, MariaHurtKnockUp(x, y));
        return e;
    }
    value = (uint8_t)(RD8(OBJ->hit_taken[OBJECT_PLAYER]) & HIT_LOW_MASK);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, 1));
    step = value == 1 ? STEP_SLIDE_LOW : STEP_SLIDE;
    WR8(OBJ->substate[OBJECT_PLAYER], step);
    WR8(OBJ->frame[OBJECT_PLAYER], FRAME_SLIDE);
    WR8(OBJ->hit_frames[OBJECT_PLAYER], SLIDE_FRAMES);
    mx_regs(&e, r_lda(regs_kept(), SLIDE_FRAMES));
    mx_then(&e, MariaHitSoundAndEffects(x, y));
    return e;
}

void abi_MariaHurtFromCrouch(void)
{
    mx_exit(MariaHurtFromCrouch(cpu.x, cpu.y));
}

/* Knocked up, then her Y speed's high byte set ($B3C8, $B3D6): the JSR to MariaHurtKnockUp returns to `ret`. */
static MExit knock_then(uint8_t x, uint8_t y, uint16_t ret, uint8_t vy_hi)
{
    MExit e;

    jsr_push(ret);
    e = MariaHurtKnockUp(x, y);
    jsr_pop();
    WR8(OBJ->vy_lo[OBJECT_PLAYER], 0);
    WR8(OBJ->vy_hi[OBJECT_PLAYER], vy_hi);
    mx_regs(&e, r_lda(regs_kept(), vy_hi));
    return e;
}

/*
 * @routine 01DF:3C8 MariaHurtInAir
 * @run 6D:B3C8
 * WHAT IT DOES: A hit in the air, on the ledge ahead or in the hit's own state: knocked up, at Y speed $FE00.
 * HOW: MariaHurtKnockUp, then the speed.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: A $FE, N and Z from it; the rest as MariaHurtKnockUp left it.
 */
MExit MariaHurtInAir(uint8_t x, uint8_t y)
{
    return knock_then(x, y, 0xB3CB, AIR_VY_HI);
}

void abi_MariaHurtInAir(void)
{
    mx_exit(MariaHurtInAir(cpu.x, cpu.y));
}

/*
 * @routine 01DF:3D6 MariaHurtClimbKindFive
 * @run 6D:B3D6
 * WHAT IT DOES: A hit of kind 5 on the ledge behind her: knocked up, at Y speed $FB00.
 * HOW: MariaHurtKnockUp, then the speed.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: A $FB, N and Z from it; the rest as MariaHurtKnockUp left it.
 */
MExit MariaHurtClimbKindFive(uint8_t x, uint8_t y)
{
    return knock_then(x, y, 0xB3D9, CLIMB_FIVE_VY_HI);
}

void abi_MariaHurtClimbKindFive(void)
{
    mx_exit(MariaHurtClimbKindFive(cpu.x, cpu.y));
}

/*
 * @routine 01DF:3E4 MariaHurtWhileClimbing
 * @run 6D:B3E4
 * WHAT IT DOES: A hit on the ledge behind her (states 7 and 8): kind 5 as MariaHurtClimbKindFive; else frame $2C for
 * $20 frames, step 3, then the hit's sound and effects.
 * HOW: The kind; the stores; a branch into MariaHitSoundAndEffects.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaHurtClimbKindFive or MariaHitSoundAndEffects.
 */
MExit MariaHurtWhileClimbing(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t kind = (uint8_t)(RD8(OBJ->hit_taken[OBJECT_PLAYER]) & HIT_KIND_MASK);

    mx_regs(&e, r_cmp(r_lda(regs_kept(), kind), kind, HIT_KIND_FIVE));
    if (kind == HIT_KIND_FIVE) {
        mx_then(&e, MariaHurtClimbKindFive(x, y));
        return e;
    }
    WR8(OBJ->hit_frames[OBJECT_PLAYER], CLIMB_HIT_FRAMES);
    WR8(OBJ->frame[OBJECT_PLAYER], FRAME_CLIMB_HIT);
    WR8(OBJ->substate[OBJECT_PLAYER], STEP_RESUME);
    mx_regs(&e, r_lda(regs_kept(), STEP_RESUME));
    mx_then(&e, MariaHitSoundAndEffects(x, y));
    return e;
}

void abi_MariaHurtWhileClimbing(void)
{
    mx_exit(MariaHurtWhileClimbing(cpu.x, cpu.y));
}

/*
 * @routine 01DF:360 MariaHurtFromItem4
 * @run 6D:B360
 * WHAT IT DOES: A hit in item 4's state ($10): item 4 ended (MariaClearSlot5AsKind9), then knocked up.
 * HOW: MariaClearSlot5AsKind9, then a branch to MariaHurtKnockUp.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaHurtKnockUp.
 */
MExit MariaHurtFromItem4(uint8_t x, uint8_t y)
{
    MExit e = mx_none();

    mx_regs(&e, r_x_only(r_a_only(regs_kept(), 0x19), 5));
    jsr_push(0xB363);
    mx_sound(&e, 0x19, MariaClearSlot5AsKind9(y));
    jsr_pop();
    mx_then(&e, MariaHurtKnockUp(mx_x_from(x, y, e), mx_y_from(x, y, e)));
    return e;
}

void abi_MariaHurtFromItem4(void)
{
    mx_exit(MariaHurtFromItem4(cpu.x, cpu.y));
}

/*
 * @routine 01DF:365 MariaHurtFromSequence
 * @run 6D:B365
 * WHAT IT DOES: A hit in her pad sequence's move (state $11): its object in slot 15 emptied, then knocked up.
 * HOW: ClearObject, then a branch to MariaHurtKnockUp.
 * WAITS: no.
 * Entry: Y the caller's. Exit: as MariaHurtKnockUp.
 */
MExit MariaHurtFromSequence(uint8_t y)
{
    MExit e = mx_none();

    mx_regs(&e, r_ldx(regs_kept(), SLOT_SEQUENCE));
    jsr_push(0xB36A);
    ClearObject(SLOT_SEQUENCE);
    jsr_pop();
    mx_then(&e, MariaHurtKnockUp(SLOT_SEQUENCE, y));
    return e;
}

void abi_MariaHurtFromSequence(void)
{
    mx_exit(MariaHurtFromSequence(cpu.y));
}

/* The throws' end ($B489 on, $B4F4 on): the animation $1B plus OBJ_KNOCK_FORWARD, the step, sound $67, the effects. */
static void throw_end(MExit *e, uint8_t x, uint8_t y, uint8_t step, uint16_t anim_ret, uint16_t sound_ret)
{
    Sum animation = add_bytes(ANIMATION_THROWN, RD8(OBJ->knock_forward[OBJECT_PLAYER]), false);

    mx_regs(e, r_v(r_c(r_lda(regs_kept(), animation.value), animation.carry), animation.overflow));
    jsr_push(anim_ret);
    mx_animation(e, MariaSetAnimation(animation.value));
    jsr_pop();
    WR8(OBJ->substate[OBJECT_PLAYER], step);
    WR8(PVARS->may_leave_top, 0);
    sound_at(e, SOUND_THROW, sound_ret, x, y);
    mx_then(e, MariaHitEffects(mx_x_from(x, y, *e), mx_y_from(x, y, *e)));
}

/*
 * @routine 01DF:45C MariaHurtKindThree
 * @run 6D:B45C
 * WHAT IT DOES: A hit of kind 3 throws her: stopped; in the air up at $FE50, else (item 4's state ended first)
 * lifted three pixels and up at $FC50; across by MariaThrowSpeedsX; animation $1B (plus OBJ_KNOCK_FORWARD), step 7,
 * she may not leave the top; sound $67, the hit's effects.
 * HOW: MariaStopMoving, MariaHurtThrowKinds by the state she was hit in, MariaClearSlot5AsKind9, MariaLiftThree;
 * MariaSetAnimation, SoundCommand, the JMP to MariaHitEffects.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaHitEffects.
 */
MExit MariaHurtKindThree(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t state, kind, side, value;

    jsr_push(0xB45F);
    MariaStopMoving();
    jsr_pop();
    state = RD8(OBJ->state_before_hit[OBJECT_PLAYER]);
    kind = TBL8(MariaHurtThrowKinds, state);
    mx_regs(&e, r_lda(r_ldx(regs_kept(), state), kind));
    if (kind != 0 && kind != 2) {
        mx_regs(&e, r_cmp(regs_kept(), kind, 2));
        WR8(OBJ->vy_lo[OBJECT_PLAYER], THROW_VY_LO);
        WR8(OBJ->vy_hi[OBJECT_PLAYER], THROW_AIR_VY_HI);
    } else {
        if (kind == 2) {
            mx_regs(&e, r_cmp(regs_kept(), kind, 2));
            mx_regs(&e, r_x_only(r_a_only(regs_kept(), 0x19), 5));
            jsr_push(0xB46E);
            mx_sound(&e, 0x19, MariaClearSlot5AsKind9(mx_y_from(x, y, e)));
            jsr_pop();
        }
        jsr_push(0xB47F);
        mx_regs(&e, MariaLiftThree());
        jsr_pop();
        WR8(OBJ->vy_lo[OBJECT_PLAYER], THROW_VY_LO);
        WR8(OBJ->vy_hi[OBJECT_PLAYER], THROW_GROUND_VY_HI);
    }
    side = RD8(OBJ->knock_right[OBJECT_PLAYER]);
    value = TBL8(MariaThrowSpeedsX, (uint8_t)(side << 1));
    WR8(OBJ->vx_lo[OBJECT_PLAYER], value);
    value = TBL8(MariaThrowSpeedsX, (uint8_t)(side << 1) + 1);
    WR8(OBJ->vx_hi[OBJECT_PLAYER], value);
    mx_regs(&e, r_y_only(regs_kept(), (uint8_t)(side << 1)));
    throw_end(&e, x, y, STEP_THROWN, 0xB4A3, 0xB4B0);
    return e;
}

void abi_MariaHurtKindThree(void)
{
    mx_exit(MariaHurtKindThree(cpu.x, cpu.y));
}

/*
 * @routine 01DF:4C8 MariaHurtKindFour
 * @run 6D:B4C8
 * WHAT IT DOES: A hit of kind 4 throws her to the right: stopped (item 4's state ended first), up at $F900, lifted
 * three pixels, right at $0480; animation $1B (plus OBJ_KNOCK_FORWARD), step 9, she may not leave the top; sound $67,
 * the hit's effects.
 * HOW: As MariaHurtKindThree.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaHitEffects.
 */
MExit MariaHurtKindFour(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t state, kind;

    WR8(OBJ->knock_right[OBJECT_PLAYER], 1);
    jsr_push(0xB4D0);
    MariaStopMoving();
    jsr_pop();
    state = RD8(OBJ->state_before_hit[OBJECT_PLAYER]);
    kind = TBL8(MariaHurtThrowKinds, state);
    mx_regs(&e, r_cmp(r_lda(r_ldx(regs_kept(), state), kind), kind, 2));
    if (kind == 2) {
        mx_regs(&e, r_x_only(r_a_only(regs_kept(), 0x19), 5));
        jsr_push(0xB4DD);
        mx_sound(&e, 0x19, MariaClearSlot5AsKind9(mx_y_from(x, y, e)));
        jsr_pop();
    }
    WR8(OBJ->vy_lo[OBJECT_PLAYER], 0);
    WR8(OBJ->vy_hi[OBJECT_PLAYER], THROW_FOUR_VY_HI);
    jsr_push(0xB4EA);
    mx_regs(&e, MariaLiftThree());
    jsr_pop();
    WR8(OBJ->vx_lo[OBJECT_PLAYER], THROW_FOUR_VX_LO);
    WR8(OBJ->vx_hi[OBJECT_PLAYER], THROW_FOUR_VX_HI);
    throw_end(&e, x, y, STEP_THROWN_FAR, 0xB4FD, 0xB50A);
    return e;
}

void abi_MariaHurtKindFour(void)
{
    mx_exit(MariaHurtKindFour(cpu.x, cpu.y));
}

/*
 * @routine 01DF:2F3 MariaHurtStart
 * @run 6D:B2F3
 * WHAT IT DOES: The hit's step 0: stands her on a floor at her Y or a pixel below if there is one; clears the pad's
 * forced presses, the attack's outcome, the hold and the slowing; says which way she is knocked; kinds 3 and 4 throw
 * her; else she stops and the hit's start by the state she was in runs (MariaHurtByState).
 * HOW: MariaReadColumns and MariaFloorCheck (INC of her Y between), MariaLandOnFloor; MariaPadForcesOff; MariaHurtSide;
 * the JMPs to MariaHurtKindThree and MariaHurtKindFour; MariaStopMoving; OBJ_STATE_BEFORE_HIT doubled, the JMP
 * (table,X).
 * WAITS: no.
 * Entry: Y the caller's. Exit: as the handler it ended in.
 */
MExit MariaHurtStart(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    FloorResult floor;
    uint8_t kind, state, doubled, lo, hi;
    uint16_t target;
    MExit handler;

    jsr_push(0xB2F6);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xB2F9);
    floor = MariaFloorCheck();
    jsr_pop();
    mx_floor(&e, floor);
    if (floor.how == FLOOR_NONE) {
        INC8(OBJ->y_lo[OBJECT_PLAYER]);
        jsr_push(0xB301);
        mx_columns(&e, MariaReadColumns());
        jsr_pop();
        jsr_push(0xB304);
        floor = MariaFloorCheck();
        jsr_pop();
        mx_floor(&e, floor);
    }
    if (floor.how != FLOOR_NONE) {
        jsr_push(0xB309);
        mx_landing(&e, MariaLandOnFloor());
        jsr_pop();
    }
    jsr_push(0xB30C);
    MariaPadForcesOff();
    jsr_pop();
    WR8(PVARS->attack_kind, 0);
    WR8(PVARS->held, 0);
    WR8(PVARS->held_count, 0);
    WR8(PVARS->slow_timer_lo, 0);
    WR8(PVARS->slow_timer_hi, 0);
    jsr_push(0xB31E);
    mx_regs(&e, MariaHurtSide());
    jsr_pop();
    kind = (uint8_t)(RD8(OBJ->hit_taken[OBJECT_PLAYER]) & HIT_KIND_MASK);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), kind), kind, HIT_KIND_THREE));
    if (kind == HIT_KIND_THREE) {
        mx_then(&e, MariaHurtKindThree(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    mx_regs(&e, r_cmp(regs_kept(), kind, HIT_KIND_FOUR));
    if (kind == HIT_KIND_FOUR) {
        mx_then(&e, MariaHurtKindFour(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    jsr_push(0xB334);
    MariaStopMoving();
    jsr_pop();
    state = RD8(OBJ->state_before_hit[OBJECT_PLAYER]);
    doubled = (uint8_t)(state << 1);
    mx_regs(&e, r_c(r_ldx(r_lda(regs_kept(), doubled), doubled), (state & 0x80) != 0));
    lo = TBL8(MariaHurtByState, doubled);
    hi = TBL8(MariaHurtByState, doubled + 1);
    target = (uint16_t)(lo | (hi << 8));
    y = mx_y_from(x, y, e);
    switch (target) {
    case HURT_FROM_STAND:
        handler = MariaHurtFromStand(doubled, y);
        break;
    case HURT_FROM_CROUCH:
        handler = MariaHurtFromCrouch(doubled, y);
        break;
    case HURT_IN_AIR:
        handler = MariaHurtInAir(doubled, y);
        break;
    case HURT_WHILE_CLIMBING:
        handler = MariaHurtWhileClimbing(doubled, y);
        break;
    case HURT_KNOCK_UP:
        handler = MariaHurtKnockUp(doubled, y);
        break;
    case HURT_FROM_ITEM4:
        handler = MariaHurtFromItem4(doubled, y);
        break;
    case HURT_FROM_SEQUENCE:
        handler = MariaHurtFromSequence(y);
        break;
    default:
        mx_exit(e);
        run_routine(target);
        e.elsewhere = true;
        return e;
    }
    mx_then(&e, handler);
    return e;
}

void abi_MariaHurtStart(void)
{
    mx_exit(MariaHurtStart(cpu.x, cpu.y));
}

/* ------------------------------------------------------------------------------------------------ the hit's steps */

/*
 * @routine 01DF:60B MariaKnockSpeedX
 * @run 6D:B60B
 * WHAT IT DOES: Gives her the knock's X speed for the side the hit throws her to ($FF00 left, $0100 right).
 * HOW: MariaKnockSpeeds by OBJ_KNOCK_RIGHT doubled.
 * WAITS: no.
 * Entry: nothing. Exit: A the high byte, N and Z from it; Y the side doubled; C bit 7 of the side. X and V kept.
 */
Regs MariaKnockSpeedX(void)
{
    uint8_t side = RD8(OBJ->knock_right[OBJECT_PLAYER]);
    uint8_t index = (uint8_t)(side << 1);
    uint8_t value = TBL8(MariaKnockSpeeds, index);

    WR8(OBJ->vx_lo[OBJECT_PLAYER], value);
    value = TBL8(MariaKnockSpeeds, index + 1);
    WR8(OBJ->vx_hi[OBJECT_PLAYER], value);
    return r_c(r_lda(r_y_only(regs_kept(), index), value), (side & 0x80) != 0);
}

void abi_MariaKnockSpeedX(void)
{
    regs_exit(MariaKnockSpeedX());
}

/* The hit ends in a crouch ($B63B): frame $1B, the animation stopped, no speed, state 2, step 0. */
static void hurt_to_crouch(MExit *e)
{
    WR8(OBJ->frame[OBJECT_PLAYER], FRAME_CROUCHED);
    WR8(OBJ->hurt_timer[OBJECT_PLAYER], HOLD_STOPPED);
    jsr_push(0xB648);
    MariaStopMoving();
    jsr_pop();
    WR8(OBJ->state[OBJECT_PLAYER], STATE_CROUCH);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    mx_regs(e, r_lda(regs_kept(), 0));
}

/*
 * @routine 01DF:61F MariaHurtSlide
 * @run 6D:B61F
 * WHAT IT DOES: The slide's step (1 and $0B): falls if no floor is under her or a pixel below; else stands on it, and
 * when its frames run out the hit ends in a crouch.
 * HOW: MariaFloorCheck (MariaReadColumns before the second), MariaToFall by its JMP, MariaLandOnFloor; DEC of
 * OBJ_HIT_FRAMES.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaToFall; frames left, N and Z from them; ended, A 0, Z set.
 */
MExit MariaHurtSlide(void)
{
    MExit e = mx_none();
    FloorResult floor;
    AnimThen fall;
    uint8_t left;

    jsr_push(0xB622);
    floor = MariaFloorCheck();
    jsr_pop();
    mx_floor(&e, floor);
    if (floor.how == FLOOR_NONE) {
        INC8(OBJ->y_lo[OBJECT_PLAYER]);
        jsr_push(0xB62A);
        mx_columns(&e, MariaReadColumns());
        jsr_pop();
        jsr_push(0xB62D);
        floor = MariaFloorCheck();
        jsr_pop();
        mx_floor(&e, floor);
        if (floor.how == FLOOR_NONE) {
            fall = MariaToFall();
            mx_animation(&e, fall.animation);
            mx_regs(&e, fall.regs);
            return e;
        }
    }
    jsr_push(0xB635);
    mx_landing(&e, MariaLandOnFloor());
    jsr_pop();
    left = DEC8(OBJ->hit_frames[OBJECT_PLAYER]);
    mx_regs(&e, r_nz(regs_kept(), left));
    if (left != 0) {
        return e;
    }
    hurt_to_crouch(&e);
    return e;
}

void abi_MariaHurtSlide(void)
{
    mx_exit(MariaHurtSlide());
}

/*
 * @routine 01DF:61D MariaHurtSlideStart
 * @run 6D:B61D
 * WHAT IT DOES: The slide's first step: the knock speed, then as MariaHurtSlide.
 * HOW: A BSR to MariaKnockSpeedX, on into MariaHurtSlide.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaHurtSlide.
 */
MExit MariaHurtSlideStart(void)
{
    MExit e = mx_none();

    jsr_push(0xB61F);
    mx_regs(&e, MariaKnockSpeedX());
    jsr_pop();
    mx_then(&e, MariaHurtSlide());
    return e;
}

void abi_MariaHurtSlideStart(void)
{
    mx_exit(MariaHurtSlideStart());
}

/*
 * @routine 01DF:657 MariaHurtResume
 * @run 6D:B657
 * WHAT IT DOES: The hit's step 3: when its frames run out she is back in the state and step she was hit in, and at
 * step 1 or 5 back in her ledge's hold.
 * HOW: DEC of OBJ_HIT_FRAMES; OBJ_STATE_BEFORE_HIT and OBJ_STEP_BEFORE_HIT; the JMP to MariaClimbHold.
 * WAITS: no.
 * Entry: nothing. Exit: waiting, N and Z from the frames left; else A the step, N, Z and C from comparing it with 1
 * or 5, or as MariaClimbHold.
 */
MExit MariaHurtResume(void)
{
    MExit e = mx_none();
    uint8_t left = DEC8(OBJ->hit_frames[OBJECT_PLAYER]);
    uint8_t value;

    mx_regs(&e, r_nz(regs_kept(), left));
    if (left != 0) {
        return e;
    }
    value = RD8(OBJ->state_before_hit[OBJECT_PLAYER]);
    WR8(OBJ->state[OBJECT_PLAYER], value);
    value = RD8(OBJ->step_before_hit[OBJECT_PLAYER]);
    WR8(OBJ->substate[OBJECT_PLAYER], value);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, STEP_HOLD_STEP));
    if (value != STEP_HOLD_STEP) {
        mx_regs(&e, r_cmp(regs_kept(), value, STEP_HOLD_STEP_B));
        if (value != STEP_HOLD_STEP_B) {
            return e;
        }
    }
    mx_regs(&e, MariaClimbHold());
    return e;
}

void abi_MariaHurtResume(void)
{
    mx_exit(MariaHurtResume());
}

/*
 * @routine 01DF:674 MariaHurtHoldFrame
 * @run 6D:B674
 * WHAT IT DOES: Runs the animation's time but keeps the frame she shows; stands her when it ends.
 * HOW: Her frame on the stack around MariaAnimationTick; OBJ_HURT_TIMER $FF ends it; the tail JMP to MariaStand.
 * WAITS: no.
 * Entry: nothing. Exit: running, A the timer, N, Z and C from comparing it with $FF; else as MariaStand.
 */
MExit MariaHurtHoldFrame(void)
{
    MExit e = mx_none();
    uint8_t frame = RD8(OBJ->frame[OBJECT_PLAYER]), timer;
    AnimThen stand;

    push8(frame);
    jsr_push(0xB67B);
    mx_animation(&e, MariaAnimationTick());
    jsr_pop();
    frame = pull8();
    WR8(OBJ->frame[OBJECT_PLAYER], frame);
    timer = RD8(OBJ->hurt_timer[OBJECT_PLAYER]);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), timer), timer, HOLD_STOPPED));
    if (timer != HOLD_STOPPED) {
        return e;
    }
    stand = MariaStand();
    mx_animation(&e, stand.animation);
    mx_regs(&e, stand.regs);
    return e;
}

void abi_MariaHurtHoldFrame(void)
{
    mx_exit(MariaHurtHoldFrame());
}

/*
 * @routine 01DF:68A MariaHurtDownStep
 * @run 6D:B68A
 * WHAT IT DOES: The step she is down in (6): falls if no floor is under her or a pixel below; else stands on it; held,
 * as MariaHurtHoldFrame; else when her animation has stopped she stands.
 * HOW: MariaFloorCheck (MariaReadColumns before the second), MariaToFall by its JMP, MariaLandOnFloor, PlayerHeld, a
 * branch to MariaHurtHoldFrame, MariaAnimationRunning, the JMP to MariaStand.
 * WAITS: no.
 * Entry: nothing. Exit: as the routine it ended in.
 */
MExit MariaHurtDownStep(void)
{
    MExit e = mx_none();
    FloorResult floor;
    AnimThen moved;
    AnimationStep running;
    uint8_t held;

    jsr_push(0xB68D);
    floor = MariaFloorCheck();
    jsr_pop();
    mx_floor(&e, floor);
    if (floor.how == FLOOR_NONE) {
        INC8(OBJ->y_lo[OBJECT_PLAYER]);
        jsr_push(0xB695);
        mx_columns(&e, MariaReadColumns());
        jsr_pop();
        jsr_push(0xB698);
        floor = MariaFloorCheck();
        jsr_pop();
        mx_floor(&e, floor);
        if (floor.how == FLOOR_NONE) {
            moved = MariaToFall();
            mx_animation(&e, moved.animation);
            mx_regs(&e, moved.regs);
            return e;
        }
    }
    jsr_push(0xB6A0);
    mx_landing(&e, MariaLandOnFloor());
    jsr_pop();
    held = RD8(PVARS->held);
    mx_regs(&e, r_lda(regs_kept(), held));
    if (held != 0) {
        mx_then(&e, MariaHurtHoldFrame());
        return e;
    }
    jsr_push(0xB6A8);
    running = MariaAnimationRunning();
    jsr_pop();
    mx_animation(&e, running);
    if (!mx_zero(mx_one_animation(running))) {
        return e;
    }
    moved = MariaStand();
    mx_animation(&e, moved.animation);
    mx_regs(&e, moved.regs);
    return e;
}

void abi_MariaHurtDownStep(void)
{
    mx_exit(MariaHurtDownStep());
}

/*
 * @routine 01DF:5F7 MariaHurtHeldLanding
 * @run 6D:B5F7
 * WHAT IT DOES: The held landing on a marked floor. Where Richter's matching routine (in RichterHurtLandOnMarkedFeet)
 * calls his landing's sound, hers calls its own address: her Y speed's high byte 2, then a JSR to itself, without end.
 * HOW: LDA #2, STA OBJ_VY_HI, JSR $B5F7: each round makes the same store and pushes another return address ($B5FF);
 * the stack wraps in its page. The BRA after it is never reached.
 * WAITS: never ends.
 * Entry: anything. Exit: none.
 */
void MariaHurtHeldLanding(void)
{
    for (;;) {
        WR8(OBJ->vy_hi[OBJECT_PLAYER], HELD_VY_HI);
        jsr_push(0xB5FF);
    }
}

void abi_MariaHurtHeldLanding(void)
{
    MariaHurtHeldLanding();
}

/*
 * @routine 01DF:601 MariaHurtHeldLandingFeet
 * @run 6D:B601
 * WHAT IT DOES: The same for a marked floor under her feet: her Y speed's high byte 2, then the JSR to
 * MariaHurtHeldLanding, which never returns.
 * HOW: LDA #2, STA OBJ_VY_HI, JSR $B5F7.
 * WAITS: never ends.
 * Entry: anything. Exit: none.
 */
void MariaHurtHeldLandingFeet(void)
{
    WR8(OBJ->vy_hi[OBJECT_PLAYER], HELD_VY_HI);
    jsr_push(0xB609);
    MariaHurtHeldLanding();
}

void abi_MariaHurtHeldLandingFeet(void)
{
    MariaHurtHeldLandingFeet();
}

/*
 * @routine 01DF:5DD MariaHurtLandOnMarked
 * @run 6D:B5DD
 * WHAT IT DOES: The knock lands on a marked floor: held, MariaHurtHeldLanding; else full fall speed and as
 * MariaLandOnMarked.
 * HOW: PlayerHeld; the JMP to MariaLandOnMarked.
 * WAITS: held, never ends.
 * Entry: X and Y the caller's. Exit: as MariaLandOnMarked.
 */
MExit MariaHurtLandOnMarked(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t held = RD8(PVARS->held);

    mx_regs(&e, r_lda(regs_kept(), held));
    if (held != 0) {
        MariaHurtHeldLanding();
    }
    WR8(OBJ->vy_hi[OBJECT_PLAYER], LAND_VY_HI);
    mx_regs(&e, r_lda(regs_kept(), LAND_VY_HI));
    mx_then(&e, MariaLandOnMarked(x, y));
    return e;
}

void abi_MariaHurtLandOnMarked(void)
{
    mx_exit(MariaHurtLandOnMarked(cpu.x, cpu.y));
}

/*
 * @routine 01DF:5EA MariaHurtLandOnMarkedFeet
 * @run 6D:B5EA
 * WHAT IT DOES: As MariaHurtLandOnMarked for marked ground under her feet: held, MariaHurtHeldLandingFeet; else full
 * fall speed and as MariaLandOnMarkedFeet.
 * HOW: PlayerHeld; the JMP to MariaLandOnMarkedFeet.
 * WAITS: held, never ends.
 * Entry: X and Y the caller's. Exit: as MariaLandOnMarkedFeet.
 */
MExit MariaHurtLandOnMarkedFeet(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t held = RD8(PVARS->held);

    mx_regs(&e, r_lda(regs_kept(), held));
    if (held != 0) {
        MariaHurtHeldLandingFeet();
    }
    WR8(OBJ->vy_hi[OBJECT_PLAYER], LAND_VY_HI);
    mx_regs(&e, r_lda(regs_kept(), LAND_VY_HI));
    mx_then(&e, MariaLandOnMarkedFeet(x, y));
    return e;
}

void abi_MariaHurtLandOnMarkedFeet(void)
{
    mx_exit(MariaHurtLandOnMarkedFeet(cpu.x, cpu.y));
}

/*
 * @routine 01DF:593 MariaHurtKnocked
 * @run 6D:B593
 * WHAT IT DOES: The knock's steps 2, 4 and 5: her animation, the knock speed; rising, light gravity; falling, lands on
 * a floor her column stands on (held: Y speed $0200, the landing, slots 1 and 2 as kind 5 and sound $4A; else full fall
 * speed and MariaJumpLand), or on a marked floor; else the bit-3 burst and light gravity.
 * HOW: MariaAnimationTick, a BSR to MariaKnockSpeedX; MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn,
 * PlayerHeld; MariaJumpLand, MariaReplaceSlotsOneTwoKind5, SoundCommand; MariaOnMarkedFloor, MariaFeetOnMarkedFloor and
 * their hurt landings; MariaCheckBit3Under; the tail JMP to MariaGravityLight.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the routine it ended in.
 */
MExit MariaHurtKnocked(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    FloorResult floor;
    MarkedFloor marked;
    MExit feet;
    uint8_t value, column;

    jsr_push(0xB596);
    mx_animation(&e, MariaAnimationTick());
    jsr_pop();
    jsr_push(0xB598);
    mx_regs(&e, MariaKnockSpeedX());
    jsr_pop();
    value = RD8(OBJ->vy_hi[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value & 0x80) {
        mx_regs(&e, MariaGravityLight());
        return e;
    }
    jsr_push(0xB5A0);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xB5A3);
    floor = MariaFloorCheck();
    jsr_pop();
    mx_floor(&e, floor);
    if (floor.how != FLOOR_NONE) {
        column = mx_x_from(x, y, e);
        jsr_push(0xB5A8);
        mx_solid(&e, MariaSolidUnderColumn(column));
        jsr_pop();
        if (mx_zero(e)) {
            column = mx_x_from(x, y, e);
            value = RD8(PVARS->held);
            mx_regs(&e, r_lda(regs_kept(), value));
            if (value == 0) {
                WR8(OBJ->vy_hi[OBJECT_PLAYER], LAND_VY_HI);
                mx_regs(&e, r_lda(regs_kept(), LAND_VY_HI));
                mx_then(&e, MariaJumpLand(column, mx_y_from(x, y, e)));
                return e;
            }
            WR8(OBJ->vy_hi[OBJECT_PLAYER], HELD_VY_HI);
            mx_regs(&e, r_lda(regs_kept(), HELD_VY_HI));
            jsr_push(0xB5D5);
            mx_then(&e, MariaJumpLand(column, mx_y_from(x, y, e)));
            jsr_pop();
            jsr_push(0xB5D8);
            mx_regs(&e, MariaReplaceSlotsOneTwoKind5());
            jsr_pop();
            mx_regs(&e, r_lda(regs_kept(), SOUND_LAND_HURT));
            mx_sound(&e, SOUND_LAND_HURT,
                     SoundCommand(SOUND_LAND_HURT, mx_x_from(x, y, e), mx_y_from(x, y, e)));
            return e;
        }
    }
    jsr_push(0xB5BA);
    marked = MariaOnMarkedFloor();
    jsr_pop();
    mx_on_marked(&e, marked);
    if (marked.how == FLOOR_MARKED || marked.how == FLOOR_MARKED_DOWN) {
        mx_then(&e, MariaHurtLandOnMarked(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    jsr_push(0xB5C2);
    feet = MariaFeetOnMarkedFloor();
    jsr_pop();
    mx_then(&e, feet);
    if (mx_carry(e)) {
        mx_then(&e, MariaHurtLandOnMarkedFeet(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    jsr_push(0xB5CA);
    mx_then(&e, MariaCheckBit3Under(mx_x_from(x, y, e), mx_y_from(x, y, e)));
    jsr_pop();
    mx_regs(&e, MariaGravityLight());
    return e;
}

void abi_MariaHurtKnocked(void)
{
    mx_exit(MariaHurtKnocked(cpu.x, cpu.y));
}

/*
 * @routine 01DF:50D MariaHurtThrown
 * @run 6D:B50D
 * WHAT IT DOES: The throw's steps 7 and 9: on the map's first screen and falling, lands on a floor her column stands on
 * (her Y up by its row, no Y speed, slots 1 and 2 as kind 5; animation $1D with sound 2 when her animation has
 * stopped, else $24; the next step and the landing's X speed); else the bit-3 burst; light gravity and her animation.
 * HOW: MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn; MariaReplaceSlotsOneTwoKind5, SoundCommand,
 * MariaSetAnimation, MariaThrownLandSpeedsX; MariaCheckBit3Under, MariaGravityLight and the tail JMP to
 * MariaAnimationTick.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: landed, A the speed's high byte, N and Z from it, Y the side doubled; else as
 * MariaAnimationTick.
 */
MExit MariaHurtThrown(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    FloorResult floor;
    uint8_t value, column, timer, animation, side, index;
    Sum row;

    jsr_push(0xB510);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    value = RD8(OBJ->y_hi[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value == 0) {
        value = RD8(OBJ->vy_hi[OBJECT_PLAYER]);
        mx_regs(&e, r_lda(regs_kept(), value));
        if (!(value & 0x80)) {
            jsr_push(0xB51D);
            floor = MariaFloorCheck();
            jsr_pop();
            mx_floor(&e, floor);
            if (floor.how != FLOOR_NONE) {
                column = mx_x_from(x, y, e);
                jsr_push(0xB522);
                mx_solid(&e, MariaSolidUnderColumn(column));
                jsr_pop();
                if (mx_zero(e)) {
                    column = mx_x_from(x, y, e);
                    value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
                    row = sub_bytes(value, RD8(PVARS->column_row[column]), true);
                    WR8(OBJ->y_lo[OBJECT_PLAYER], row.value);
                    WR8(OBJ->vy_lo[OBJECT_PLAYER], 0);
                    WR8(OBJ->vy_hi[OBJECT_PLAYER], 0);
                    mx_regs(&e, r_v(r_c(r_lda(regs_kept(), row.value), row.carry), row.overflow));
                    jsr_push(0xB537);
                    mx_regs(&e, MariaReplaceSlotsOneTwoKind5());
                    jsr_pop();
                    timer = RD8(OBJ->hurt_timer[OBJECT_PLAYER]);
                    mx_regs(&e, r_cmp(r_lda(regs_kept(), timer), timer, HOLD_STOPPED));
                    if (timer == HOLD_STOPPED) {
                        sound_at(&e, SOUND_LANDED, 0xB543, x, y);
                        animation = ANIMATION_THROWN_LAND;
                    } else {
                        animation = ANIMATION_THROWN_HELD;
                    }
                    mx_regs(&e, r_lda(regs_kept(), animation));
                    jsr_push(0xB555);
                    mx_animation(&e, MariaSetAnimation(animation));
                    jsr_pop();
                    mx_regs(&e, r_nz(regs_kept(), INC8(OBJ->substate[OBJECT_PLAYER])));
                    side = RD8(OBJ->knock_right[OBJECT_PLAYER]);
                    index = (uint8_t)(side << 1);
                    value = TBL8(MariaThrownLandSpeedsX, index);
                    WR8(OBJ->vx_lo[OBJECT_PLAYER], value);
                    value = TBL8(MariaThrownLandSpeedsX, index + 1);
                    WR8(OBJ->vx_hi[OBJECT_PLAYER], value);
                    mx_regs(&e, r_c(r_lda(r_y_only(regs_kept(), index), value), (side & 0x80) != 0));
                    return e;
                }
            }
            jsr_push(0xB54A);
            mx_then(&e, MariaCheckBit3Under(mx_x_from(x, y, e), mx_y_from(x, y, e)));
            jsr_pop();
        }
    }
    jsr_push(0xB54D);
    mx_regs(&e, MariaGravityLight());
    jsr_pop();
    mx_animation(&e, MariaAnimationTick());
    return e;
}

void abi_MariaHurtThrown(void)
{
    mx_exit(MariaHurtThrown(cpu.x, cpu.y));
}

/*
 * @routine 01DF:56E MariaHurtThrownLanded
 * @run 6D:B56E
 * WHAT IT DOES: The throw's steps 8 and $0A: slots 1 and 2 as kind 5 every eighth frame; with a floor under her or a
 * pixel below, while her animation runs nothing more, and when it has stopped the hit ends in a crouch; with none, back
 * to the step before.
 * HOW: TST of zWaveCounter, MariaReplaceSlotsOneTwoKind5; MariaReadColumns and MariaFloorCheck (INC of her Y between);
 * MariaAnimationWait, which ends the step while it runs, then the JMP to the crouch's end ($B63B); DEC of the step.
 * WAITS: no.
 * Entry: nothing. Exit: as the routine it ended in; back, N and Z from the step.
 */
MExit MariaHurtThrownLanded(void)
{
    MExit e = mx_none();
    FloorResult floor;
    AnimationWaitEnd wait;
    uint8_t count = RD8(ZP->wave_counter);

    mx_regs(&e, r_tst(regs_kept(), THROWN_LAND_EVERY, count));
    if ((count & THROWN_LAND_EVERY) == 0) {
        jsr_push(0xB576);
        mx_regs(&e, MariaReplaceSlotsOneTwoKind5());
        jsr_pop();
    }
    jsr_push(0xB579);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xB57C);
    floor = MariaFloorCheck();
    jsr_pop();
    mx_floor(&e, floor);
    if (floor.how == FLOOR_NONE) {
        INC8(OBJ->y_lo[OBJECT_PLAYER]);
        jsr_push(0xB584);
        mx_columns(&e, MariaReadColumns());
        jsr_pop();
        jsr_push(0xB587);
        floor = MariaFloorCheck();
        jsr_pop();
        mx_floor(&e, floor);
        if (floor.how == FLOOR_NONE) {
            mx_regs(&e, r_nz(regs_kept(), DEC8(OBJ->substate[OBJECT_PLAYER])));
            return e;
        }
    }
    jsr_push(0xB58C);
    wait = MariaAnimationWait();
    mx_animation(&e, wait.step);
    if (wait.dropped) {
        return e;
    }
    jsr_pop();
    hurt_to_crouch(&e);
    return e;
}

void abi_MariaHurtThrownLanded(void)
{
    mx_exit(MariaHurtThrownLanded());
}

/*
 * @routine 01DF:288 MariaStateHurt
 * @run 6D:B288
 * WHAT IT DOES: State 9, the hit's reaction: the step's handler from MariaHurtSteps.
 * HOW: MariaStepIndex, then the JMP (table,X).
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the step's handler, after A and X the step doubled and C its bit 7.
 */
MExit MariaStateHurt(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t step, doubled, lo, hi;
    uint16_t target;
    MExit handler;

    jsr_push(0xB28B);
    step = MariaStepIndex();
    jsr_pop();
    doubled = (uint8_t)(step << 1);
    mx_regs(&e, r_c(r_ldx(r_lda(regs_kept(), doubled), doubled), (step & 0x80) != 0));
    lo = TBL8(MariaHurtSteps, doubled);
    hi = TBL8(MariaHurtSteps, doubled + 1);
    target = (uint16_t)(lo | (hi << 8));
    (void)x;
    switch (target) {
    case STEP_START_AT:
        handler = MariaHurtStart(doubled, y);
        break;
    case STEP_SLIDE_START_AT:
        handler = MariaHurtSlideStart();
        break;
    case STEP_KNOCKED_AT:
        handler = MariaHurtKnocked(doubled, y);
        break;
    case STEP_RESUME_AT:
        handler = MariaHurtResume();
        break;
    case STEP_DOWN_AT:
        handler = MariaHurtDownStep();
        break;
    case STEP_THROWN_AT:
        handler = MariaHurtThrown(doubled, y);
        break;
    case STEP_LANDED_AT:
        handler = MariaHurtThrownLanded();
        break;
    case STEP_SLIDE_AT:
        handler = MariaHurtSlide();
        break;
    default:
        mx_exit(e);
        run_routine(target);
        e.elsewhere = true;
        return e;
    }
    mx_then(&e, handler);
    return e;
}

void abi_MariaStateHurt(void)
{
    mx_exit(MariaStateHurt(cpu.x, cpu.y));
}
