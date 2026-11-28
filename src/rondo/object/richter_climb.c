/*
 * Richter climbing, from his overlay at $A000 (sector $1B9): states 5 to 8, each a way along the
 * climb (5 up and right, 6 up and left, 7 down and right, 8 down and left, by the speeds at $AF8F),
 * their steps through the tables at $AF5A (states 5 and 6) and $B1BC (7 and 8), the pad's turn back
 * along the climb, and the attack on it.
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
    FIRST_CLIMB_STATE = 5, FIRST_HIGH_CLIMB_STATE = 7, PAD_ACROSS = 0xA0, PAD_DIRECTIONS = 0xF0,
    PAD_BUTTONS = 0x03, CLIMB_BACK_WAIT = 7, ANIMATION_STOPPED = 0xFF, FRAME_CLIMB_LOW = 0x32,
    FRAME_CLIMB_HIGH = 0x33, CLIMB_MARK = 0x04, STEP_LOW_STEPPING = 4, FRAME_LOW_STEPPING = 7,
    TIMER_LOW_STEPPING = 4, FRAME_HIGH_STEPPING = 5, TIMER_HIGH_STEPPING = 2, HIGH_OFFSET_BASE = 4,
    TURN_UNTIL = 3, REACH_FRAME = 6, REACH_TIMER = 6, ANIM_HIGH_REACH = 0x08, ANIM_PULL_UP = 0x04
};

/* What a terrain read's answer is, as the registers it leaves give it to an AND. */
static uint8_t read_answer(OffsetRead r)
{
    return tile_answer(r.read);
}

/*
 * @routine 01BA:7D5 RichterClimbBack
 * @run 6D:AFD5
 * WHAT IT DOES: Turns him back along the climb once zFieldC4 has counted 7 frames (counting it up
 * till then): the way back's animation, frame, facing and state, the animation stopped, step 0, the
 * fractions cleared.
 * HOW: The four-byte tables at $B02F, $B033, $B037 and $B03B by Y; below 7 the INC and RTS of
 * RichterClimbPad ($AFD2).
 * WAITS: no.
 * Entry: Y the state less 5. Exit: counting, N and Z from the count, C clear, A its old value. Turned,
 * A the new state, N and Z from it, C set. X, Y and V kept.
 */
void RichterClimbBack(RegLog *log, uint8_t way)
{
    uint8_t value = RD8(ZP->field_c4);

    rl_ended(log, ended_compare(value, CLIMB_BACK_WAIT));
    if (value < CLIMB_BACK_WAIT) {
        rl_ended(log, ended_nz(INC8(ZP->field_c4)));
        return;
    }
    WR8(ZP->field_c4, 0);
    value = rd8((uint16_t)(RichterClimbBackAnimations + way));
    WR8(OBJ->animation[0], value);
    WR8(OBJ->hurt_timer[0], ANIMATION_STOPPED);
    value = rd8((uint16_t)(RichterClimbBackFrames + way));
    WR8(OBJ->frame[0], value);
    value = rd8((uint16_t)(RichterClimbBackFacings + way));
    WR8(OBJ->facing_left[0], value);
    value = rd8((uint16_t)(RichterClimbBackStates + way));
    WR8(OBJ->state[0], value);
    WR8(OBJ->substate[0], 0);
    WR8(OBJ->x_frac[0], 0);
    WR8(OBJ->y_frac[0], 0);
    rl_ended(log, ended_load(value));
}

/*
 * @routine 01BB:004 RichterClimbOn
 * @run 6D:B004
 * WHAT IT DOES: Goes on along the climb, unless a button is held: the climb's animation for the state,
 * by the other of the two (OBJ_ANIMATION plus 1, its bit 0).
 * HOW: TST of zPadHeld; the four bytes at $B03F by Y; a tail jump to RichterSetAnimation.
 * WAITS: no.
 * Entry: Y the state less 5. Exit: a button held, Z clear, N and V bits 7 and 6 of zPadHeld; else as
 * RichterSetAnimation.
 */
void RichterClimbOn(RegLog *log, uint8_t way)
{
    uint8_t held = RD8(ZP->pad_held);
    Sum sum;
    uint8_t value;

    rl_tst(log, PAD_BUTTONS, held);
    if (held & PAD_BUTTONS) {
        return;
    }
    value = (uint8_t)((uint8_t)(RD8(OBJ->animation[0]) + 1) & 0x01);
    sum = add_bytes(value, rd8((uint16_t)(RichterClimbAnimations + way)), false);
    rl_ended(log, ended_with_overflow(ended_with_carry(ended_load(sum.value), sum.carry), sum.overflow));
    rl_animation(log, RichterSetAnimation(sum.value));
}

/*
 * @routine 01BA:797 RichterClimbPad
 * @run 6D:AF97
 * WHAT IT DOES: The pad on the climb: held the way it goes (both directions, the one across, or with
 * the forced press clear the one up or down) goes on; held the way back likewise turns him back;
 * else nothing.
 * HOW: zPadHeld's directions against the tables at $B017, $B01B, $B01F (on) and $B023, $B027, $B02B
 * (back) by the state less 5, PadForcedPressB tested between; RichterClimbOn, RichterClimbBack.
 * WAITS: no.
 * Entry: nothing. Exit: as the routine it goes to; else A the directions held, N, Z and C from the
 * last compare, or the TST's Z, N and V; Y the state less 5.
 */
/* A CMP of the directions held with a climb table's byte: logged, and whether they are equal. */
static bool held_is(RegLog *log, uint8_t held, uint16_t table, uint8_t way)
{
    uint8_t value = rd8((uint16_t)(table + way));

    rl_ended(log, ended_compare(held, value));
    return held == value;
}

void RichterClimbPad(RegLog *log)
{
    uint8_t state, way, held, forced;
    Sum less;

    state = RD8(OBJ->state[0]);
    less = add_bytes(state, (uint8_t)~FIRST_CLIMB_STATE, true);
    way = less.value;
    held = (uint8_t)(RD8(ZP->pad_held) & PAD_DIRECTIONS);
    rl_ended(log, ended_with_y(ended_with_overflow(ended_load(held), less.overflow), way));
    if (held_is(log, held, RichterClimbPadOn, way) || held_is(log, held, RichterClimbPadOnAcross, way)) {
        RichterClimbOn(log, way);
        return;
    }
    forced = RD8(PVARS->forced_press_b);
    rl_tst(log, 0xFF, forced);
    if (forced == 0 && held_is(log, held, RichterClimbPadOnUpDown, way)) {
        RichterClimbOn(log, way);
        return;
    }
    held = (uint8_t)(RD8(ZP->pad_held) & PAD_DIRECTIONS);
    rl_ended(log, ended_load(held));
    if (held_is(log, held, RichterClimbPadBack, way) || held_is(log, held, RichterClimbPadBackAcross, way)) {
        RichterClimbBack(log, way);
        return;
    }
    forced = RD8(PVARS->forced_press_b);
    rl_tst(log, 0xFF, forced);
    if (forced != 0) {
        return;
    }
    if (held_is(log, held, RichterClimbPadBackUpDown, way)) {
        RichterClimbBack(log, way);
    }
}

/*
 * @routine 01BA:766 RichterClimbStepSpeed
 * @run 6D:AF66
 * WHAT IT DOES: Stops him, then, each time the animation moves on a step while it runs, gives the
 * climb's step: two pixels across and two up or down by the state.
 * HOW: RichterStopMoving; OBJ_ANIMATION_AT kept in zp $D9 around RichterAnimationRunning; the eight
 * bytes at $AF8F by twice the state less 5, X high and Y high speeds.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterAnimationRunning, or A the Y speed, N and Z from it, Y the offset,
 * C bit 7 of the state less 5, V from the subtract. X kept.
 */
void RichterClimbStepSpeed(RegLog *log)
{
    AnimationStep running;
    uint8_t at, value;

    jsr_push(0xAF69);
    RichterStopMoving();
    jsr_pop();
    value = RD8(OBJ->animation_at[0]);
    WR8(ZP->richter_object, value);
    rl_ended(log, ended_load(value));
    jsr_push(0xAF71);
    running = RichterAnimationRunning();
    jsr_pop();
    rl_animation(log, running);
    if (running.nz == 0) {
        return;
    }
    value = RD8(OBJ->animation_at[0]);
    {
        uint8_t before = RD8(ZP->richter_object);

        rl_ended(log, ended_compare(value, before));
        if (value == before) {
            return;
        }
    }
    {
        uint8_t state = RD8(OBJ->state[0]);
        Sum less = add_bytes(state, (uint8_t)~FIRST_CLIMB_STATE, true);

        at = (uint8_t)(less.value << 1);
        value = rd8((uint16_t)(RichterClimbStepSpeeds + at));
        WR8(OBJ->vx_hi[0], value);
        value = rd8((uint16_t)(RichterClimbStepSpeeds + at + 1));
        WR8(OBJ->vy_hi[0], value);
        rl_ended(log, ended_with_overflow(ended_with_carry(ended_with_y(ended_load(value), at),
                                                           (less.value & 0x80) != 0), less.overflow));
    }
}

/*
 * @routine 01BA:717 RichterClimbAttack
 * @run 6D:AF17
 * WHAT IT DOES: Starts an attack on the climb: the pad held across, other than the way it goes, turns
 * him back first; then no whip strike yet, the forced press cleared, stopped, and the attack with the
 * crash frame's animation.
 * HOW: zPadHeld against $B01B by the state less 5; RichterClimbBack; RichterStopMoving;
 * RichterCrashFrame; a jump to RichterAttackWithAnimation.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterAttackWithAnimation.
 */
void RichterClimbAttack(RegLog *log)
{
    uint8_t state, way, held;
    CrashFrame frame;
    Sum less;

    state = RD8(OBJ->state[0]);
    less = add_bytes(state, (uint8_t)~FIRST_CLIMB_STATE, true);
    way = less.value;
    held = (uint8_t)(RD8(ZP->pad_held) & PAD_ACROSS);
    rl_ended(log, ended_with_y(ended_with_overflow(ended_with_carry(ended_load(held), less.carry),
                                                   less.overflow), way));
    if (held != 0) {
        uint8_t on = rd8((uint16_t)(RichterClimbPadOnAcross + way));

        rl_ended(log, ended_compare(held, on));
        if (held != on) {
            jsr_push(0xAF2C);
            RichterClimbBack(log, way);
            jsr_pop();
        }
    }
    WR8(PVARS->whip_struck, 0);
    WR8(PVARS->forced_press_b, 0);
    jsr_push(0xAF35);
    RichterStopMoving();
    jsr_pop();
    jsr_push(0xAF37);
    frame = RichterCrashFrame();
    jsr_pop();
    rl_crash_frame(log, frame);
    rl_attack(log, RichterAttackWithAnimation(frame.frame));
}

/* The way along a climb, its state less the given first state, and the subtract that found it. */
static uint8_t climb_way(RegLog *log, uint8_t first, bool to_x)
{
    uint8_t state = RD8(OBJ->state[0]);
    Sum less = add_bytes(state, (uint8_t)~first, true);

    if (to_x) {
        rl_ended_x(log, ended_with_overflow(ended_with_carry(ended_load(less.value), less.carry), less.overflow),
                   less.value);
    } else {
        rl_ended(log, ended_with_y(ended_with_overflow(ended_with_carry(ended_load(less.value), less.carry),
                                                       less.overflow), less.value));
    }
    return less.value;
}

/* The choice the climb's step 0 ends in: an item crash, an item's use, or the pad. */
static void climb_after_crash(RegLog *log, bool high)
{
    ItemCrash crash;
    UpPressUse use;

    jsr_push(high ? 0xB1F2 : 0xB07E);
    crash = RichterTryItemCrash();
    jsr_pop();
    rl_item_crash(log, crash);
    if (crash.how == CRASH_STARTED) {
        rl_ledge_hurt(log, high ? RichterLedgeHurtB() : RichterLedgeHurtA());
        return;
    }
    jsr_push(high ? 0xB1FA : 0xB086);
    use = RichterUseItemOrUpPress();
    jsr_pop();
    rl_up_press(log, use);
    if (use.use.how != USE_NO_PRESS) {
        RichterClimbAttack(log);
        return;
    }
    if (high) {
        uint8_t way = climb_way(log, FIRST_HIGH_CLIMB_STATE, false);
        Sum at = add_bytes((uint8_t)(way << 1), HIGH_OFFSET_BASE, false);
        OffsetRead read;

        rl_ended(log, ended_with_y(ended_with_carry(ended_load(at.value), at.carry), at.value));
        jsr_push(0xB20D);
        read = RichterReadAtOffset(at.value);
        jsr_pop();
        rl_offset_read(log, read);
        rl_ended(log, ended_load((uint8_t)(read_answer(read) & CLIMB_MARK)));
        if (read_answer(read) & CLIMB_MARK) {
            RichterClimbPad(log);
            return;
        }
        jsr_push(0xB217);
        rl_speed(log, RichterSpeedWalk());
        jsr_pop();
        WR8(OBJ->substate[0], STEP_LOW_STEPPING);
        WR8(OBJ->frame[0], FRAME_HIGH_STEPPING);
        WR8(OBJ->hurt_timer[0], TIMER_HIGH_STEPPING);
        rl_ended(log, ended_load(TIMER_HIGH_STEPPING));
        return;
    }
    RichterClimbPad(log);
}

/* The start both step 0s share: the step's speed, the latch, the jump or the climb up off it. */
static bool climb_step0_start(RegLog *log, bool high)
{
    JumpOrClimb jump;

    jsr_push(high ? 0xB1CB : 0xB046);
    RichterClimbStepSpeed(log);
    jsr_pop();
    jsr_push(high ? 0xB1CE : 0xB049);
    rl_ended(log, ended_load(RichterLatchButtons()));
    jsr_pop();
    jsr_push(high ? 0xB1D1 : 0xB04C);
    jump = RichterJumpOrClimb();
    jsr_pop();
    rl_jump_climb(log, jump);
    if (jump.pressed && !jump.up_held) {
        rl_up_whip(log, high ? RichterUpWhipRight() : RichterUpWhipLeft());
        return true;
    }
    if (jump.pressed && jump.up_held && jump.ladder.c) {
        rl_up_whip(log, high ? RichterUpWhipRight() : RichterUpWhipLeft());
        return true;
    }
    {
        uint8_t timer = RD8(OBJ->hurt_timer[0]);

        rl_ended(log, ended_compare(timer, ANIMATION_STOPPED));
        return timer != ANIMATION_STOPPED;
    }
}

/*
 * @routine 01BB:043 RichterClimbLowStep0
 * @run 6D:B043
 * WHAT IT DOES: States 5 and 6, step 0: the step's speed, the latch; a jump or a climb off it as
 * RichterUpWhipLeft; when the animation ends, the climb's pad override for the way, frame $32, and a
 * look below by the way: on the mark, an item crash, the item's use (an attack), or the pad; off it,
 * step 4 with frame 7 for 4 frames.
 * HOW: RichterClimbStepSpeed, RichterLatchButtons, RichterJumpOrClimb; PadOverrides by the state less
 * 5; RichterReadAtOffset by the two bytes at $B09E; RichterTryItemCrash and RichterLedgeHurtA,
 * RichterUseItemOrUpPress and RichterClimbAttack, RichterClimbPad.
 * WAITS: no.
 * Entry: nothing. Exit: as the routine it goes to; still animating, A the timer, N, Z and C from
 * comparing it with $FF; stepping, A 4, N and Z from it.
 */
void RichterClimbLowStep0(RegLog *log)
{
    uint8_t way, offset;
    OffsetRead read;

    if (climb_step0_start(log, false)) {
        return;
    }
    way = climb_way(log, FIRST_CLIMB_STATE, true);
    WR8(PVARS->pad_overrides[way], 1);
    WR8(OBJ->frame[0], FRAME_CLIMB_LOW);
    way = climb_way(log, FIRST_CLIMB_STATE, true);
    offset = rd8((uint16_t)(RichterClimbLowOffsets + way));
    rl_ended(log, ended_with_y(ended_nz(offset), offset));
    jsr_push(0xB077);
    read = RichterReadAtOffset(offset);
    jsr_pop();
    rl_offset_read(log, read);
    rl_ended(log, ended_load((uint8_t)(read_answer(read) & CLIMB_MARK)));
    if (read_answer(read) & CLIMB_MARK) {
        climb_after_crash(log, false);
        return;
    }
    WR8(OBJ->substate[0], STEP_LOW_STEPPING);
    WR8(OBJ->frame[0], FRAME_LOW_STEPPING);
    WR8(OBJ->hurt_timer[0], TIMER_LOW_STEPPING);
    rl_ended(log, ended_load(TIMER_LOW_STEPPING));
}

/*
 * @routine 01BB:1C8 RichterClimbHighStep0
 * @run 6D:B1C8
 * WHAT IT DOES: States 7 and 8, step 0: as RichterClimbLowStep0, with RichterUpWhipRight, frame $33,
 * the overrides from the third; an item crash as RichterLedgeHurtB; then a look below by the way: on
 * the mark the pad, off it the walk's speed and step 4 with frame 5 for 2 frames.
 * HOW: As RichterClimbLowStep0; RichterReadAtOffset at twice the state less 7, plus 4; RichterSpeedWalk.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterClimbLowStep0; stepping, A 2, N and Z from it.
 */
void RichterClimbHighStep0(RegLog *log)
{
    uint8_t way;

    if (climb_step0_start(log, true)) {
        return;
    }
    WR8(OBJ->frame[0], FRAME_CLIMB_HIGH);
    way = climb_way(log, FIRST_HIGH_CLIMB_STATE, false);
    WR8(PVARS->pad_overrides[2 + way], 1);
    rl_ended(log, ended_load(1));
    climb_after_crash(log, true);
}

/*
 * @routine 01BB:0C6 RichterClimbStep1
 * @run 6D:B0C6
 * WHAT IT DOES: The climb's step 1, an item crash on it: the pad's turn for the animation's first three
 * steps; the crash frame's animation; at its end, as RichterEndItemCrash.
 * HOW: OBJ_ANIMATION_AT against 3; RichterTurnOnPad; RichterCrashFrame into OBJ_ANIMATION;
 * RichterAnimationWait; a jump to RichterEndItemCrash.
 * WAITS: no.
 * Entry: nothing. Exit: waiting, as RichterAnimationWait; else as RichterEndItemCrash.
 */
void RichterClimbStep1(RegLog *log)
{
    AnimationWaitEnd wait;
    CrashFrame frame;
    uint8_t value;

    value = RD8(OBJ->animation_at[0]);
    rl_ended(log, ended_compare(value, TURN_UNTIL));
    if (value < TURN_UNTIL) {
        jsr_push(0xB0D0);
        rl_turn_pad(log, RichterTurnOnPad());
        jsr_pop();
    }
    value = RD8(OBJ->animation_at[0]);
    rl_ended(log, ended_compare(value, 6));
    jsr_push(0xB0DC);
    frame = RichterCrashFrame();
    jsr_pop();
    rl_crash_frame(log, frame);
    WR8(OBJ->animation[0], frame.frame);
    jsr_push(0xB0E2);
    wait = RichterAnimationWait();
    rl_animation(log, wait.step);
    if (wait.dropped) {
        return;
    }
    jsr_pop();
    rl_crash_end(log, RichterEndItemCrash());
}

/* The facing a climb's reach ends in, by Y; then no fraction and no X speed ($B130). */
static void reach_end(RegLog *log, uint8_t way)
{
    uint8_t value = rd8((uint16_t)(RichterClimbReachFacings + way));

    WR8(OBJ->facing_left[0], value);
    WR8(OBJ->x_frac[0], 0);
    WR8(OBJ->y_frac[0], 0);
    rl_ended(log, ended_with_y(ended_load(value), way));
    jsr_push(0xB140);
    RichterStopX();
    jsr_pop();
}

/*
 * @routine 01BB:0E5 RichterClimbReach
 * @run 6D:B0E5
 * WHAT IT DOES: The climb's step 2: a jump takes him into the air; while PlayerClimbReach is not zero,
 * moves him a pixel a frame toward it, counting it to zero; then, in states 5 and 6, frame 6 for 6
 * frames, step 3, the facing by the way; in 7 and 8, step 0 with animation 8 and the facing.
 * HOW: RichterJumpPressed, a jump to RichterToAir; RichterAnimationTick; the facings at $B1A0;
 * RichterStopX; A pushed around RichterSetAnimation.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterToAir; reaching, N and Z from the count, A the speed's high byte;
 * else A the facing, N and Z from it, Y the way, the rest as RichterStopX keeps them.
 */
void RichterClimbReach(RegLog *log)
{
    uint8_t value, state;
    bool jump;

    jsr_push(0xB0E8);
    jump = RichterJumpPressed();
    jsr_pop();
    rl_jump_pressed(log, jump);
    if (jump) {
        rl_air(log, RichterToAir());
        return;
    }
    value = RD8(PVARS->climb_reach);
    rl_ended(log, ended_load(value));
    if (value != 0) {
        jsr_push(value & 0x80 ? 0xB108 : 0xB0F7);
        rl_animation(log, RichterAnimationTick());
        jsr_pop();
        WR8(OBJ->vx_lo[0], 0);
        if (!(value & 0x80)) {
            WR8(OBJ->vx_hi[0], 1);
            rl_ended(log, ended_with_a(ended_nz(DEC8(PVARS->climb_reach)), 1));
        } else {
            WR8(OBJ->vx_hi[0], 0xFF);
            rl_ended(log, ended_with_a(ended_nz(INC8(PVARS->climb_reach)), 0xFF));
        }
        return;
    }
    state = RD8(OBJ->state[0]);
    rl_ended(log, ended_compare(state, FIRST_HIGH_CLIMB_STATE));
    if (state < FIRST_HIGH_CLIMB_STATE) {
        Sum less;

        WR8(OBJ->frame[0], REACH_FRAME);
        WR8(OBJ->hurt_timer[0], REACH_TIMER);
        INC8(OBJ->substate[0]);
        state = RD8(OBJ->state[0]);
        less = add_bytes(state, (uint8_t)~FIRST_CLIMB_STATE, true);
        rl_ended(log, ended_with_overflow(ended_with_carry(ended_load(less.value), less.carry), less.overflow));
        reach_end(log, less.value);
        return;
    }
    {
        Sum less;

        WR8(OBJ->substate[0], 0);
        push8(state);
        rl_ended(log, ended_load(ANIM_HIGH_REACH));
        jsr_push(0xB14A);
        rl_animation(log, RichterSetAnimation(ANIM_HIGH_REACH));
        jsr_pop();
        state = pull8();
        less = add_bytes(state, (uint8_t)~FIRST_HIGH_CLIMB_STATE, true);
        rl_ended(log, ended_with_overflow(ended_with_carry(ended_load(less.value), less.carry), less.overflow));
        reach_end(log, less.value);
    }
}

/*
 * @routine 01BB:151 RichterClimbPullUp
 * @run 6D:B151
 * WHAT IT DOES: The climb's step 3: a jump takes him into the air; when the timer runs out, a step
 * across by the way and, with the pad held up that way, the pull's animation (4) at step 0; else as
 * RichterEndItemCrash.
 * HOW: RichterJumpPressed, RichterToAir; OBJ_HURT_TIMER; the steps at $B1A2 and pads at $B1A4 by the
 * state less 5; RichterSetAnimation; a branch to RichterEndItemCrash.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterToAir; counting, N and Z from the count; pulled up, as
 * RichterSetAnimation; else as RichterEndItemCrash.
 */
void RichterClimbPullUp(RegLog *log)
{
    uint8_t state, way, value;
    bool jump;
    Sum less, moved;

    jsr_push(0xB154);
    jump = RichterJumpPressed();
    jsr_pop();
    rl_jump_pressed(log, jump);
    if (jump) {
        rl_air(log, RichterToAir());
        return;
    }
    value = DEC8(OBJ->hurt_timer[0]);
    rl_ended(log, ended_nz(value));
    if (value != 0) {
        return;
    }
    state = RD8(OBJ->state[0]);
    less = add_bytes(state, (uint8_t)~FIRST_CLIMB_STATE, true);
    way = less.value;
    moved = add_bytes(RD8(OBJ->x_lo[0]), rd8((uint16_t)(RichterClimbPullSteps + way)), false);
    WR8(OBJ->x_lo[0], moved.value);
    value = (uint8_t)(RD8(ZP->pad_held) & rd8((uint16_t)(RichterClimbPullPads + way)));
    rl_ended(log, ended_with_y(ended_with_overflow(ended_with_carry(ended_load(value), moved.carry),
                                                   moved.overflow), way));
    if (value == 0) {
        rl_crash_end(log, RichterEndItemCrash());
        return;
    }
    rl_ended(log, ended_load(ANIM_PULL_UP));
    jsr_push(0xB17C);
    rl_animation(log, RichterSetAnimation(ANIM_PULL_UP));
    jsr_pop();
    WR8(OBJ->substate[0], 0);
}

/*
 * @routine 01BB:1B3 RichterClimbStandTimer
 * @run 6D:B1B3
 * WHAT IT DOES: The climb's steps 5 (and 3 of states 7 and 8): as RichterStandStep2Timer.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as RichterStandStep2Timer.
 */
void RichterClimbStandTimer(RegLog *log)
{
    rl_crash_stand(log, RichterStandStep2Timer());
}

/* A climb state's step by its table of six words. */
static void climb_step(RegLog *log, uint16_t steps, uint16_t index_ret, bool high)
{
    uint8_t step, at;
    uint16_t entry, handler;

    jsr_push(index_ret);
    step = RichterStepIndex();
    jsr_pop();
    at = (uint8_t)(step << 1);
    rl_ended_x(log, ended_with_carry(ended_load(at), (step & 0x80) != 0), at);
    entry = (uint16_t)(steps + at);
    handler = (uint16_t)(rd8(entry) | (rd8((uint16_t)(entry + 1)) << 8));
    switch (handler) {
    case 0xB043:
        RichterClimbLowStep0(log);
        break;
    case 0xB1C8:
        RichterClimbHighStep0(log);
        break;
    case 0xB0C6:
        RichterClimbStep1(log);
        break;
    case 0xB0E5:
        RichterClimbReach(log);
        break;
    case 0xB151:
        RichterClimbPullUp(log);
        break;
    case 0xB1AA:
        rl_countdown(log, CountDownThenStand());
        break;
    default:
        RichterClimbStandTimer(log);
        break;
    }
    (void)high;
}

/*
 * @routine 01BA:754 RichterStateClimbLow
 * @run 6D:AF54
 * WHAT IT DOES: States 5 and 6, climbing up: runs the step's handler.
 * HOW: The six words at $AF5A by RichterStepIndex.
 * WAITS: no.
 * Entry: nothing. Exit: as the handler leaves them.
 */
void RichterStateClimbLow(RegLog *log)
{
    climb_step(log, RichterClimbLowSteps, 0xAF57, false);
}

/*
 * @routine 01BB:1B6 RichterStateClimbHigh
 * @run 6D:B1B6
 * WHAT IT DOES: States 7 and 8, climbing down: runs the step's handler.
 * HOW: The six words at $B1BC by RichterStepIndex.
 * WAITS: no.
 * Entry: nothing. Exit: as the handler leaves them.
 */
void RichterStateClimbHigh(RegLog *log)
{
    climb_step(log, RichterClimbHighSteps, 0xB1B9, true);
}

void abi_RichterClimbBack(void)
{
    RegLog log = { 0 };

    RichterClimbBack(&log, cpu.y);
    reg_log_replay(&log);
}

void abi_RichterClimbOn(void)
{
    RegLog log = { 0 };

    RichterClimbOn(&log, cpu.y);
    reg_log_replay(&log);
}

#define LOG_ABI(name) void abi_##name(void) { RegLog log = { 0 }; name(&log); reg_log_replay(&log); }
LOG_ABI(RichterClimbPad)
LOG_ABI(RichterClimbStepSpeed)
LOG_ABI(RichterClimbAttack)
LOG_ABI(RichterClimbLowStep0)
LOG_ABI(RichterClimbHighStep0)
LOG_ABI(RichterClimbStep1)
LOG_ABI(RichterClimbReach)
LOG_ABI(RichterClimbPullUp)
LOG_ABI(RichterClimbStandTimer)
LOG_ABI(RichterStateClimbLow)
LOG_ABI(RichterStateClimbHigh)
