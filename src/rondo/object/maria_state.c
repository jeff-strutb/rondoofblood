/*
 * Maria's frame (01DD, bank $6D): the pad, the spike check while pushed, the state frame, the turn window, the item's
 * hearts, the animation's sound on a change and her hitbox; and the state frame: the death, falling off the bottom of
 * the map, the invincibility, the hit timer, a hit's damage or its cancel, the hold, and the state's handler
 * (MariaStateHandlers, eighteen states).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"

enum {
    OBJECT_PLAYER = 0,
    STATE_HIT = 9, STATE_ITEM4 = 0x10,
    HIT_KIND_MASK = 0x07, HIT_KIND_FIVE = 5, HIT_KIND_THREE = 3, HIT_HELD_BIT = 0x20,
    ANIMATION_CRASH = 0x2A, ANIMATION_CRASH_AIR = 0x2B,
    HIT_TIMER = 0x30, SCRIPT_HIT_OVER = 9, SCRIPT_HELD_HIT = 1, SCRIPT_HOLD_START = 0, HOLD_PRESSES = 8,
    BELOW_THE_MAP = 0xFF, PAD_DIRECTIONS = 0xF0, SUB_STEP_LIFE_LOST = 4, SOUND_LIFE_LOST = 0x68, KEEP_LAST_POINT = 1,
    STATE_STAND_AT = 0xA5E2, STATE_WALK_AT = 0xA7EB, STATE_CROUCH_AT = 0xA89C, STATE_JUMP_AT = 0xAA6A,
    STATE_FALL_AT = 0xACA0, STATE_CLIMB_LOW_AT = 0xAD1A, STATE_CLIMB_HIGH_AT = 0xAF3A, STATE_HURT_AT = 0xB288,
    STATE_LANDED_AT = 0xAC1F, STATE_SLIDE_HELD_AT = 0xB103, STATE_SLIDE_AIR_AT = 0xB222, STATE_SLIDE_AT = 0xB1B5,
    STATE_ITEM4_AT = 0xB6AE, STATE_SEQUENCE_AT = 0xB8EA
};

#define MariaStateHandlers_TBL MariaStateHandlers

/* ------------------------------------------------------------------------------------------------ the state frame */

/*
 * @routine 01DD:2C8 MariaStateDispatch
 * @run 6D:A2C8
 * WHAT IT DOES: Runs her state's handler (MariaStateHandlers); below the map (Y high $FF) the pad's buttons are kept
 * and its directions dropped first.
 * HOW: The pad bytes ANDed with $F0; OBJ_STATE doubled, the JMP (table,X).
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the state's handler, after A and X the state doubled and C its bit 7.
 */
MExit MariaStateDispatch(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value, state, doubled, lo, hi;
    uint16_t target;
    MExit handler;

    value = RD8(OBJ->y_hi[OBJECT_PLAYER]);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, BELOW_THE_MAP));
    if (value == BELOW_THE_MAP) {
        value = (uint8_t)(RD8(ZP->pad_held) & PAD_DIRECTIONS);
        WR8(ZP->pad_held, value);
        value = (uint8_t)(RD8(ZP->pad_pressed) & PAD_DIRECTIONS);
        WR8(ZP->pad_pressed, value);
    }
    state = RD8(OBJ->state[OBJECT_PLAYER]);
    doubled = (uint8_t)(state << 1);
    mx_regs(&e, r_c(r_ldx(r_lda(regs_kept(), doubled), doubled), (state & 0x80) != 0));
    lo = TBL8(MariaStateHandlers, doubled);
    hi = TBL8(MariaStateHandlers, doubled + 1);
    target = (uint16_t)(lo | (hi << 8));
    y = mx_y_from(x, y, e);
    switch (target) {
    case STATE_STAND_AT:
        handler = MariaStateStand(doubled, y);
        break;
    case STATE_WALK_AT:
        handler = MariaStateWalk(doubled, y);
        break;
    case STATE_CROUCH_AT:
        handler = MariaStateCrouch(doubled, y);
        break;
    case STATE_JUMP_AT:
        handler = MariaStateJump(doubled, y);
        break;
    case STATE_FALL_AT:
        handler = MariaStateFall(doubled, y);
        break;
    case STATE_CLIMB_LOW_AT:
        handler = MariaStateClimbLow(doubled, y);
        break;
    case STATE_CLIMB_HIGH_AT:
        handler = MariaStateClimbHigh(doubled, y);
        break;
    case STATE_HURT_AT:
        handler = MariaStateHurt(doubled, y);
        break;
    case STATE_LANDED_AT:
        handler = MariaStateLanded();
        break;
    case STATE_SLIDE_HELD_AT:
        handler = MariaStateSlideHeld(doubled, y);
        break;
    case STATE_SLIDE_AIR_AT:
        handler = MariaStateSlideAir(doubled, y);
        break;
    case STATE_SLIDE_AT:
        handler = MariaStateSlide(doubled, y);
        break;
    case STATE_ITEM4_AT:
        handler = MariaStateItem4(y);
        break;
    case STATE_SEQUENCE_AT:
        handler = MariaStateSequence();
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

void abi_MariaStateDispatch(void)
{
    mx_exit(MariaStateDispatch(cpu.x, cpu.y));
}

/*
 * @routine 01DD:299 MariaHeldFrame
 * @run 6D:A299
 * WHAT IT DOES: The hold on her (PlayerHeld), outside the hit's state: its first frame starts script 0 and counts
 * PlayerHeldCount to 1; each frame a button is newly pressed counts it on, and at 8 the hold ends (script 9) and the
 * state's handler runs; else the frame ends here. Not held, the state's handler.
 * HOW: PlayerHeld, OBJ_STATE, PlayerHeldCount, zPadPressed; MariaStartHitScript; on into MariaStateDispatch.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaStateDispatch; held, A the count or the press, N and Z from it.
 */
MExit MariaHeldFrame(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value;

    value = RD8(PVARS->held);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value != 0) {
        value = RD8(OBJ->state[OBJECT_PLAYER]);
        mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, STATE_HIT));
        if (value != STATE_HIT) {
            value = RD8(PVARS->held_count);
            mx_regs(&e, r_lda(regs_kept(), value));
            if (value == 0) {
                mx_regs(&e, r_a_only(regs_kept(), SCRIPT_HOLD_START));
                jsr_push(0xA2AE);
                MariaStartHitScript(SCRIPT_HOLD_START);
                jsr_pop();
                mx_script(&e, SCRIPT_HOLD_START);
                mx_regs(&e, r_nz(regs_kept(), INC8(PVARS->held_count)));
            }
            value = RD8(ZP->pad_pressed);
            mx_regs(&e, r_lda(regs_kept(), value));
            if (value == 0) {
                return e;
            }
            INC8(PVARS->held_count);
            value = RD8(PVARS->held_count);
            mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, HOLD_PRESSES));
            if (value != HOLD_PRESSES) {
                return e;
            }
            WR8(PVARS->held, 0);
            mx_regs(&e, r_lda(regs_kept(), SCRIPT_HIT_OVER));
            jsr_push(0xA2C8);
            MariaStartHitScript(SCRIPT_HIT_OVER);
            jsr_pop();
            mx_script(&e, SCRIPT_HIT_OVER);
        }
    }
    mx_then(&e, MariaStateDispatch(mx_x_from(x, y, e), mx_y_from(x, y, e)));
    return e;
}

void abi_MariaHeldFrame(void)
{
    mx_exit(MariaHeldFrame(cpu.x, cpu.y));
}

/*
 * @routine 01DD:282 MariaHitTaken
 * @run 6D:A282
 * WHAT IT DOES: A hit paid for in health: the hit timer $30; held, the held hit's script 1 and the hit cleared; else
 * her hit reaction. Then as MariaHeldFrame.
 * HOW: PlayerHeld; MariaStartHitScript or MariaHitReact; a branch, or a run on, into MariaHeldFrame.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaHeldFrame.
 */
MExit MariaHitTaken(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t held;

    WR8(OBJ->anim_timer[OBJECT_PLAYER], HIT_TIMER);
    held = RD8(PVARS->held);
    mx_regs(&e, r_lda(regs_kept(), held));
    if (held != 0) {
        mx_regs(&e, r_lda(regs_kept(), SCRIPT_HELD_HIT));
        jsr_push(0xA291);
        MariaStartHitScript(SCRIPT_HELD_HIT);
        jsr_pop();
        mx_script(&e, SCRIPT_HELD_HIT);
        WR8(OBJ->hit_taken[OBJECT_PLAYER], 0);
    } else {
        jsr_push(0xA299);
        MariaHitReact();
        jsr_pop();
        mx_script(&e, 8);
    }
    mx_then(&e, MariaHeldFrame(mx_x_from(x, y, e), mx_y_from(x, y, e)));
    return e;
}

void abi_MariaHitTaken(void)
{
    mx_exit(MariaHitTaken(cpu.x, cpu.y));
}

/*
 * @routine 01DD:307 MariaOutOfHealth
 * @run 6D:A307
 * WHAT IT DOES: No health left: with PlayerKeepsLastPoint 1 she keeps one point and the hit is paid as
 * MariaHitTaken; else, held by a hit of bit 5 PlayerHeld is set, and the life is lost: zSubStep 4, the door's gate and
 * the top cleared, her hitbox off, sound $68.
 * HOW: The stores; the JMP to MariaHitTaken; the tail JMP to SoundCommand.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaHitTaken, or SoundCommand with $68.
 */
MExit MariaOutOfHealth(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t keep, hit;

    WR8(ZP->player_health, 0);
    keep = RD8(PVARS2->keeps_last_point);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), keep), keep, KEEP_LAST_POINT));
    if (keep == KEEP_LAST_POINT) {
        INC8(ZP->player_health);
        mx_then(&e, MariaHitTaken(x, y));
        return e;
    }
    hit = RD8(OBJ->hit_taken[OBJECT_PLAYER]);
    mx_regs(&e, r_tst(regs_kept(), HIT_HELD_BIT, hit));
    if (hit & HIT_HELD_BIT) {
        WR8(PVARS->held, 1);
        mx_regs(&e, r_lda(regs_kept(), 1));
    }
    WR8(ZP->player_health, 0);
    WR8(ZP->sub_step, SUB_STEP_LIFE_LOST);
    WR8(PVARS->door_ready, 0);
    WR8(PVARS->may_leave_top, 0);
    WR8(OBJ->hitbox_on[OBJECT_PLAYER], 0);
    mx_regs(&e, r_lda(regs_kept(), SOUND_LIFE_LOST));
    mx_sound(&e, SOUND_LIFE_LOST, SoundCommand(SOUND_LIFE_LOST, mx_x_from(x, y, e), mx_y_from(x, y, e)));
    return e;
}

void abi_MariaOutOfHealth(void)
{
    mx_exit(MariaOutOfHealth(cpu.x, cpu.y));
}

/*
 * @routine 01DD:1E9 MariaStateFrame
 * @run 6D:A1E9
 * WHAT IT DOES: A frame of her state. Dying, the death's wait. Off the bottom of the map (Y high 1, or Y low $FF on
 * the screen above), she dies. Then the invincibility; while the hit timer runs no hit is taken, and when it ends with
 * no hold or slowing, script 9; a hit: of kind 5 with her not above the map, or of kind 3 or more in the item crash
 * or item 4's state, it is dropped; else its damage is half again more, paid from her health: none left, out of
 * health; else MariaHitTaken. Then the hold and the state's handler.
 * HOW: PlayerDying, her Y; MariaInvincibleTick; OBJ_ANIM_TIMER, PlayerHeld, PlayerSlowTimer; OBJ_HIT_TAKEN's kind,
 * OBJ_ANIMATION and OBJ_STATE; OBJ_HEALTH (the hit's damage) with LSR and ADC; zPlayerHealth's SBC; branches and JMPs to
 * MariaDyingStep, MariaDie, MariaOutOfHealth, MariaHitTaken, MariaHeldFrame and MariaStateDispatch.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the routine it ended in.
 */
MExit MariaStateFrame(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value, kind, damage;
    Sum more, left;

    value = RD8(PVARS2->dying);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value != 0) {
        mx_dying(&e, MariaDyingStep());
        return e;
    }
    value = RD8(OBJ->y_hi[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (!(value & 0x80)) {
        value = (uint8_t)(value - 1);
        mx_regs(&e, r_lda(regs_kept(), value));
        if (value == 0) {
            mx_sound(&e, 0x60, MariaDie(mx_x_from(x, y, e), mx_y_from(x, y, e)));
            return e;
        }
        value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
        mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, BELOW_THE_MAP));
        if (value >= BELOW_THE_MAP) {
            mx_sound(&e, 0x60, MariaDie(mx_x_from(x, y, e), mx_y_from(x, y, e)));
            return e;
        }
    }
    jsr_push(0xA200);
    mx_invincible(&e, MariaInvincibleTick(mx_x_from(x, y, e), mx_y_from(x, y, e)));
    jsr_pop();
    value = RD8(OBJ->anim_timer[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value != 0) {
        WR8(OBJ->hit_taken[OBJECT_PLAYER], 0);
        value = DEC8(OBJ->anim_timer[OBJECT_PLAYER]);
        mx_regs(&e, r_nz(regs_kept(), value));
        if (value != 0) {
            mx_then(&e, MariaHeldFrame(mx_x_from(x, y, e), mx_y_from(x, y, e)));
            return e;
        }
        value = RD8(PVARS->held);
        value |= RD8(PVARS->slow_timer_lo);
        value |= RD8(PVARS->slow_timer_hi);
        mx_regs(&e, r_lda(regs_kept(), value));
        if (value == 0) {
            mx_regs(&e, r_lda(regs_kept(), SCRIPT_HIT_OVER));
            jsr_push(0xA220);
            MariaStartHitScript(SCRIPT_HIT_OVER);
            jsr_pop();
            mx_script(&e, SCRIPT_HIT_OVER);
        }
    }
    value = RD8(OBJ->hit_taken[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value == 0) {
        mx_then(&e, MariaHeldFrame(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    kind = (uint8_t)(RD8(OBJ->hit_taken[OBJECT_PLAYER]) & HIT_KIND_MASK);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), kind), kind, HIT_KIND_FIVE));
    if (kind == HIT_KIND_FIVE) {
        value = RD8(PVARS->may_leave_top);
        mx_regs(&e, r_lda(regs_kept(), value));
        if (value == 0) {
            WR8(OBJ->hit_taken[OBJECT_PLAYER], 0);
            mx_then(&e, MariaStateDispatch(mx_x_from(x, y, e), mx_y_from(x, y, e)));
            return e;
        }
    }
    value = RD8(OBJ->animation[OBJECT_PLAYER]);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, ANIMATION_CRASH));
    bool dropped = false, reacts_only = false;
    if (value != ANIMATION_CRASH) {
        mx_regs(&e, r_cmp(regs_kept(), value, ANIMATION_CRASH_AIR));
    }
    if (value == ANIMATION_CRASH || value == ANIMATION_CRASH_AIR) {
        kind = (uint8_t)(RD8(OBJ->hit_taken[OBJECT_PLAYER]) & HIT_KIND_MASK);
        mx_regs(&e, r_cmp(r_lda(regs_kept(), kind), kind, HIT_KIND_THREE));
        dropped = kind < HIT_KIND_THREE;
    } else {
        value = RD8(OBJ->state[OBJECT_PLAYER]);
        mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, STATE_ITEM4));
        if (value == STATE_ITEM4) {
            kind = (uint8_t)(RD8(OBJ->hit_taken[OBJECT_PLAYER]) & HIT_KIND_MASK);
            mx_regs(&e, r_cmp(r_lda(regs_kept(), kind), kind, HIT_KIND_THREE));
            if (kind >= HIT_KIND_THREE) {
                reacts_only = true;
            } else {
                dropped = true;
            }
        }
    }
    if (dropped) {
        WR8(OBJ->hit_taken[OBJECT_PLAYER], 0);
        mx_then(&e, MariaStateDispatch(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    if (reacts_only) {
        jsr_push(0xA299);
        MariaHitReact();
        jsr_pop();
        mx_script(&e, 8);
        mx_then(&e, MariaHeldFrame(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    damage = RD8(OBJ->health[OBJECT_PLAYER]);
    more = add_bytes((uint8_t)(damage >> 1), RD8(OBJ->health[OBJECT_PLAYER]), false);
    WR8(OBJ->health[OBJECT_PLAYER], more.value);
    value = RD8(ZP->player_health);
    left = sub_bytes(value, RD8(OBJ->health[OBJECT_PLAYER]), true);
    WR8(ZP->player_health, left.value);
    mx_regs(&e, r_v(r_c(r_lda(regs_kept(), left.value), left.carry), left.overflow));
    if (!left.carry || left.value == 0) {
        mx_then(&e, MariaOutOfHealth(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    mx_then(&e, MariaHitTaken(mx_x_from(x, y, e), mx_y_from(x, y, e)));
    return e;
}

void abi_MariaStateFrame(void)
{
    mx_exit(MariaStateFrame(cpu.x, cpu.y));
}

/* ------------------------------------------------------------------------------------------------ her frame */

/*
 * @routine 01DD:050 MariaFrame
 * @run 6D:A050
 * WHAT IT DOES: One frame of Maria: the pad, forced from OBJ_WORK_F while OBJ_WORK_E counts down, else read (with
 * PlayerStartPose cleared); the forced presses ORed in; the pad's overrides cleared; the attack delay counted down; her
 * pad sequence; the spike check while pushed; the state frame; the turn window; the item's hearts; the animation's
 * sound when her animation changed; her hitbox from her tables by animation.
 * HOW: OBJ_FRAME kept on the stack across the frame's calls; MariaAnimationHitboxIndex by animation, four bytes of
 * MariaAnimationHitbox.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: A the hitbox's last byte, N and Z from it, X the hitbox's offset, C bit 6 of the
 * index; Y and V as the last call left them.
 */
MExit MariaFrame(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value, before, index, offset;
    TurnWindow window;

    value = RD8(OBJ->work_e[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value != 0) {
        DEC8(OBJ->work_e[OBJECT_PLAYER]);
        value = RD8(ZP->pad_held);
        WR8(ZP->pad_previous, value);
        value = RD8(OBJ->work_f[OBJECT_PLAYER]);
        WR8(ZP->pad_held, value);
        WR8(ZP->pad_pressed, value);
        mx_regs(&e, r_lda(regs_kept(), value));
    } else {
        WR8(PVARS2->start_pose, 0);
        jsr_push(0xA06B);
        value = MariaReadPad();
        jsr_pop();
        mx_regs(&e, r_lda(regs_kept(), value));
    }
    value = RD8(PVARS->forced_press);
    value |= RD8(ZP->pad_pressed);
    value |= RD8(PVARS->forced_press_b);
    WR8(ZP->pad_pressed, value);
    before = RD8(OBJ->frame[OBJECT_PLAYER]);
    push8(before);
    mx_regs(&e, r_lda(regs_kept(), before));
    jsr_push(0xA07C);
    MariaClearPadOverrides();
    jsr_pop();
    value = RD8(OBJ->event_id[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value != 0) {
        mx_regs(&e, r_nz(regs_kept(), DEC8(OBJ->event_id[OBJECT_PLAYER])));
    }
    jsr_push(0xA087);
    window = MariaPadSequence();
    jsr_pop();
    mx_turn(&e, window);
    jsr_push(0xA08A);
    mx_push_hit(&e, MariaHitWhilePushed());
    jsr_pop();
    jsr_push(0xA08D);
    mx_then(&e, MariaStateFrame(mx_x_from(x, y, e), mx_y_from(x, y, e)));
    jsr_pop();
    jsr_push(0xA090);
    window = MariaCheckTurnWindow();
    jsr_pop();
    mx_turn(&e, window);
    jsr_push(0xA093);
    mx_hearts(&e, MariaCheckItemHearts());
    jsr_pop();
    before = pull8();
    value = (uint8_t)(before ^ RD8(OBJ->frame[OBJECT_PLAYER]));
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value != 0) {
        jsr_push(0xA09C);
        mx_regs(&e, MariaAnimationSound());
        jsr_pop();
    }
    value = RD8(OBJ->frame[OBJECT_PLAYER]);
    index = TBL8(MariaAnimationHitboxIndex, value);
    offset = (uint8_t)(index << 2);
    value = TBL8(MariaAnimationHitbox, offset);
    WR8(OBJ->hitbox_dx[OBJECT_PLAYER], value);
    value = TBL8(MariaAnimationHitbox, offset + 1);
    WR8(OBJ->hitbox_dy[OBJECT_PLAYER], value);
    value = TBL8(MariaAnimationHitbox, offset + 2);
    WR8(OBJ->hitbox_half_w[OBJECT_PLAYER], value);
    value = TBL8(MariaAnimationHitbox, offset + 3);
    WR8(OBJ->hitbox_half_h[OBJECT_PLAYER], value);
    mx_regs(&e, r_c(r_lda(r_x_only(regs_kept(), offset), value), (index & 0x40) != 0));
    return e;
}

void abi_MariaFrame(void)
{
    mx_exit(MariaFrame(cpu.x, cpu.y));
}

/*
 * @routine 01DD:000 MariaVecFrame
 * @run 6D:A000
 * WHAT IT DOES: Her overlay's first vector: goes to MariaFrame.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as MariaFrame.
 */
MExit MariaVecFrame(uint8_t x, uint8_t y)
{
    return MariaFrame(x, y);
}

void abi_MariaVecFrame(void)
{
    mx_exit(MariaVecFrame(cpu.x, cpu.y));
}
