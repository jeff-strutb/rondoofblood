/*
 * Maria standing (state 0, 01DD in bank $6D): the state's steps by OBJ_SUBSTATE (MariaStandSteps: still, attacking,
 * the item crash), the ground checks every grounded step makes first, and the ledge she takes hold of.
 *
 * The ground checks end the step themselves when one fires: they pull the step's return address off the stack
 * (PLA, PLA) and jump to the fall, the jump, the item crash or the attack, whose RTS then returns to the step's
 * caller. MariaGroundChecks says so (dropped), and the step ends with nothing more.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"

enum {
    OBJECT_PLAYER = 0,
    STATE_STAND = 0, STATE_WALK = 1, STEP_ATTACK = 1, STEP_CROUCH_LOOK = 9, LOOK_TIMER = 0x20,
    ANIMATION_STAND_LOOK = 7, ANIMATION_WALK = 0, ATTACK_ANIMATION = 0x20, STAND_FRAME_START = 1,
    PAD_I = 0x01, PAD_UP = 0x10, PAD_RIGHT = 0x20, PAD_LEFT = 0x80, COLUMN_SOLID = 0x01,
    STAND_STILL_STEP = 0xA61C, STAND_ATTACK_STEP = 0xA4C4, STAND_CRASH_STEP = 0xA4E7,
    CLIMB_STEP = 2, CLIMB_PAST_EDGE = 8, CLIMB_LOW_BELOW = 0x0A, CLIMB_HIGH_BELOW = 0x1E, CLIMB_EDGE_BIT = 0x04,
    CLIMB_LOW_NEAR = 7, CLIMB_HIGH_EDGE_ADD = 7,
    STATE_CLIMB_LOW_EDGE = 5, STATE_CLIMB_LOW_OPEN = 6, STATE_CLIMB_HIGH_EDGE = 7, STATE_CLIMB_HIGH_OPEN = 8
};

#define MariaStandSteps_TBL MariaStandSteps

/* ------------------------------------------------------------------------------------------------ the ground checks */

/* PLA, PLA: the caller's return address taken off the stack; A the second byte pulled, N and Z from it. */
static Regs drop_caller(void)
{
    uint8_t value;

    pull8();
    value = pull8();
    return r_lda(regs_kept(), value);
}

/* The X and Y a chain leaves from known registers at its start, for a call made after it that is given them. */
static uint8_t x_after(uint8_t x, uint8_t y, MExit e)
{
    MExit t = mx_none();

    mx_regs(&t, r_y_only(r_x_only(regs_kept(), x), y));
    mx_then(&t, e);
    return mx_x(t);
}

static uint8_t y_after(uint8_t x, uint8_t y, MExit e)
{
    MExit t = mx_none();

    mx_regs(&t, r_y_only(r_x_only(regs_kept(), x), y));
    mx_then(&t, e);
    return mx_y(t);
}

/*
 * @routine 01DD:5ED MariaGroundChecks
 * @run 6D:A5ED
 * WHAT IT DOES: The checks a step on the ground makes first: the columns under her; no footing, she falls; the jump
 * pressed, she jumps; a landing; then the item crash and the attack button. When one of them fires, the step ends
 * there: its return address is pulled and the fall, the jump, the item crash or the attack runs in its place.
 * HOW: MariaReadColumns, MariaHasNoFooting, MariaJumpPressed, MariaLandOnFloor, MariaTryItemCrash and MariaUseItem,
 * each carry set firing; PLA, PLA, then the JMP to MariaToFall, MariaJump, MariaCrashOnGround or MariaAttackFromUse.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: nothing fired, as MariaUseItem with C clear. Fired, the caller's return address
 * gone and S two higher; as the routine jumped to, after A the return address's high byte.
 */
MariaDrop MariaGroundChecks(uint8_t x, uint8_t y)
{
    MariaDrop r = { mx_none(), false };
    FootingCheck footing;
    ItemCrash crash;
    TurnWindow use;
    bool jump;
    AnimThen fall;

    jsr_push(0xA5F0);
    mx_columns(&r.e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xA5F3);
    footing = MariaHasNoFooting();
    jsr_pop();
    mx_footing(&r.e, footing);
    if (footing.no_floor) {
        r.dropped = true;
        mx_regs(&r.e, drop_caller());
        fall = MariaToFall();
        mx_animation(&r.e, fall.animation);
        mx_regs(&r.e, fall.regs);
        return r;
    }
    jsr_push(0xA5F8);
    jump = MariaJumpPressed();
    jsr_pop();
    mx_regs(&r.e, r_c(regs_kept(), jump));
    if (jump) {
        r.dropped = true;
        mx_regs(&r.e, drop_caller());
        mx_then(&r.e, MariaJump());
        return r;
    }
    jsr_push(0xA5FD);
    mx_landing(&r.e, MariaLandOnFloor());
    jsr_pop();
    jsr_push(0xA600);
    crash = MariaTryItemCrash();
    jsr_pop();
    mx_crash(&r.e, crash);
    if (crash.how == CRASH_STARTED) {
        r.dropped = true;
        mx_regs(&r.e, drop_caller());
        mx_then(&r.e, MariaCrashOnGround(x_after(x, y, r.e), y_after(x, y, r.e)));
        return r;
    }
    jsr_push(0xA605);
    use = MariaUseItem();
    jsr_pop();
    mx_turn(&r.e, use);
    if (use.end.c_set && use.end.c) {
        r.dropped = true;
        mx_regs(&r.e, drop_caller());
        mx_then(&r.e, MariaAttackFromUse(x_after(x, y, r.e), y_after(x, y, r.e)));
        return r;
    }
    return r;
}

void abi_MariaGroundChecks(void)
{
    mx_exit(MariaGroundChecks(cpu.x, cpu.y).e);
}

/* ------------------------------------------------------------------------------------------------ the ledge */

/*
 * @routine 01DD:746 MariaClimbStart
 * @run 6D:A746
 * WHAT IT DOES: Takes hold of the ledge MariaLedgeCheck found, to climb it: the low ledge (zTemp01 0) or the high one,
 * each in one of two holds by whether the point past the ledge's edge reads bit 2: states 5 and 6 low, 7 and 8 high.
 * PlayerClimbReach is how far she has to go across to the edge; on the high ledge she faces the way the reach points.
 * She stops, and the state's step is 2.
 * HOW: The column's offset (MariaColumnOffset) and 8 more past it, read $0A below her for the low ledge, $1E for the
 * high (MariaReadBelow); the reach from the offset less zTemp00 for the low, less the column's RichterColumnX for the
 * high. Animation 0 unless she was walking. The pad's forced presses go off first.
 * WAITS: no.
 * Entry: X the column, zTemp00 the low ledge's X, zTemp01 low or high. Exit: A 2, N and Z clear, X the state; Y the
 * column's offset index (low) or the reach (high), or as MariaSetAnimation; C set unless she was standing, or as
 * MariaSetAnimation; V from the reach's last add or subtract.
 */
MExit MariaClimbStart(uint8_t column)
{
    MExit e = mx_none();
    ColumnOffset offset;
    TileRead read;
    uint8_t value, base, state, was, reach_y;
    bool past;
    Sum across, reach, row;
    AnimationStep animation;

    jsr_push(0xA749);
    MariaPadForcesOff();
    jsr_pop();
    WR8(ZP->fieldc4, 0);
    WR8(PVARS->may_leave_top, 1);
    if (RD8(ZP->temp01) == 0) {
        value = RD8(ZP->temp00);
        push8(value);
        jsr_push(0xA75A);
        offset = MariaColumnOffset(column);
        jsr_pop();
        base = RD8(OBJ->x_lo[OBJECT_PLAYER]);
        across = add_bytes(add_bytes(offset.value, base, false).value, CLIMB_PAST_EDGE, false);
        WR8(ZP->temp00, across.value);
        base = RD8(OBJ->y_lo[OBJECT_PLAYER]);
        row = add_bytes(base, CLIMB_LOW_BELOW, false);
        /* The read may hand the tile to the stage's code, entered with X the column and Y the offset's index. */
        cpu.x = column;
        cpu.y = offset.index;
        jsr_push(0xA76C);
        read = MariaReadBelow(row.value, row.carry);
        jsr_pop();
        past = (tile_answer(read) & CLIMB_EDGE_BIT) != 0;
        value = pull8();
        WR8(ZP->temp00, value);
        jsr_push(past ? 0xA776 : 0xA79E);
        offset = MariaColumnOffset(column);
        jsr_pop();
        base = RD8(ZP->temp00);
        across = sub_bytes(offset.value, base, true);
        if (past) {
            reach = sub_bytes(across.value, CLIMB_PAST_EDGE, true);
            state = STATE_CLIMB_LOW_EDGE;
        } else {
            reach = add_bytes(add_bytes(across.value, CLIMB_LOW_NEAR, false).value, CLIMB_PAST_EDGE, false);
            state = STATE_CLIMB_LOW_OPEN;
        }
        reach_y = offset.index;
    } else {
        jsr_push(0xA7AE);
        offset = MariaColumnOffset(column);
        jsr_pop();
        base = RD8(OBJ->x_lo[OBJECT_PLAYER]);
        across = add_bytes(add_bytes(offset.value, base, false).value, CLIMB_PAST_EDGE, false);
        WR8(ZP->temp00, across.value);
        base = RD8(OBJ->y_lo[OBJECT_PLAYER]);
        row = add_bytes(base, CLIMB_HIGH_BELOW, false);
        cpu.x = column;
        cpu.y = offset.index;
        jsr_push(0xA7C0);
        read = MariaReadBelow(row.value, row.carry);
        jsr_pop();
        past = (tile_answer(read) & CLIMB_EDGE_BIT) != 0;
        jsr_push(past ? 0xA7C7 : 0xA7E3);
        offset = MariaColumnOffset(column);
        jsr_pop();
        base = RD8(PVARS->column_x[column]);
        across = sub_bytes(offset.value, base, true);
        if (past) {
            reach = add_bytes(across.value, CLIMB_HIGH_EDGE_ADD, false);
            state = STATE_CLIMB_HIGH_EDGE;
        } else {
            reach = across;
            state = STATE_CLIMB_HIGH_OPEN;
        }
        reach_y = reach.value;
        WR8(OBJ->facing_left[OBJECT_PLAYER], 0);
        if (reach.value & 0x80) {
            INC8(OBJ->facing_left[OBJECT_PLAYER]);
        }
    }
    WR8(PVARS->climb_reach, reach.value);
    was = RD8(OBJ->state[OBJECT_PLAYER]);
    mx_regs(&e, r_v(r_cmp(r_y_only(regs_kept(), reach_y), was, STATE_WALK), reach.overflow));
    if (was != STATE_WALK) {
        mx_regs(&e, r_lda(regs_kept(), ANIMATION_WALK));
        jsr_push(0xA78D);
        animation = MariaSetAnimation(ANIMATION_WALK);
        jsr_pop();
        mx_animation(&e, animation);
    }
    WR8(OBJ->substate[OBJECT_PLAYER], CLIMB_STEP);
    WR8(OBJ->state[OBJECT_PLAYER], state);
    mx_regs(&e, r_x_only(r_lda(regs_kept(), CLIMB_STEP), state));
    MariaStopMoving();
    return e;
}

void abi_MariaClimbStart(void)
{
    mx_exit(MariaClimbStart(cpu.x));
}

/* ------------------------------------------------------------------------------------------------ standing still */

/*
 * @routine 01DD:61C MariaStandFrame
 * @run 6D:A61C
 * WHAT IT DOES: Step 0 of standing (and of the standing attack, which calls it): the ground checks; a ledge, she takes
 * hold of it; MariaUpHeld, she crouches. In step 0: up pressed, she looks up (animation 7, its step 9, timer $20, one
 * tick); outside state 0, while PlayerStartPose is set, frame 1; else when her animation has stopped, with no push on
 * the floor and all three columns under her solid, the next idle animation (OBJ_WORK_A plus one), otherwise animation
 * 7. Then, right or left held, she faces that way and, unless button I is held, walks (state 1, animation 0).
 * HOW: MariaGroundChecks, MariaLedgeCheck and MariaClimbStart, MariaUpHeld and MariaCrouch; TST of zPadPressed and
 * zPadHeld; MariaSetAnimation, MariaAnimationTick, MariaAnimationRunning; the pad's forced presses off before facing.
 * The walk runs on into MariaSetAnimationZero.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the check or move that ended it; standing on with no direction, Y 1, Z set, N
 * clear, V bit 6 of zPadHeld; facing with button I held, Y the facing, Z clear, V bit 6 of zPadHeld.
 */
MExit MariaStandFrame(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    MariaDrop checks;
    MariaLedge ledge;
    bool up;
    uint8_t value, held, facing;
    AnimationStep animation;
    AnimThen crouch;

    jsr_push(0xA61F);
    checks = MariaGroundChecks(x, y);
    if (checks.dropped) {
        return checks.e;
    }
    jsr_pop();
    mx_then(&e, checks.e);
    jsr_push(0xA622);
    ledge = MariaLedgeCheck();
    jsr_pop();
    mx_ledge(&e, ledge);
    if (mx_carry(e)) {
        mx_then(&e, MariaClimbStart(x_after(x, y, e)));
        return e;
    }
    jsr_push(0xA62A);
    up = MariaUpHeld();
    jsr_pop();
    mx_regs(&e, r_c(regs_kept(), up));
    if (up) {
        crouch = MariaCrouch();
        mx_animation(&e, crouch.animation);
        mx_regs(&e, crouch.regs);
        return e;
    }
    value = RD8(OBJ->substate[OBJECT_PLAYER]);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, STEP_ATTACK));
    if (value != STEP_ATTACK) {
        value = RD8(ZP->pad_pressed);
        mx_regs(&e, r_tst(regs_kept(), PAD_UP, value));
        if (value & PAD_UP) {
            WR8(OBJ->work_a[OBJECT_PLAYER], ANIMATION_STAND_LOOK);
            mx_regs(&e, r_lda(regs_kept(), ANIMATION_STAND_LOOK));
            jsr_push(0xA643);
            mx_animation(&e, MariaSetAnimation(ANIMATION_STAND_LOOK));
            jsr_pop();
            WR8(OBJ->scroll_band[OBJECT_PLAYER], STEP_CROUCH_LOOK);
            WR8(OBJ->hurt_timer[OBJECT_PLAYER], LOOK_TIMER);
            mx_regs(&e, r_lda(regs_kept(), LOOK_TIMER));
            jsr_push(0xA650);
            mx_animation(&e, MariaAnimationTick());
            jsr_pop();
        } else {
            value = RD8(OBJ->state[OBJECT_PLAYER]);
            mx_regs(&e, r_lda(regs_kept(), value));
            if (value != STATE_STAND) {
                value = RD8(PVARS2->f3bf0);
                mx_regs(&e, r_lda(regs_kept(), value));
            }
            if (value != 0) {
                WR8(OBJ->frame[OBJECT_PLAYER], STAND_FRAME_START);
                mx_regs(&e, r_lda(regs_kept(), STAND_FRAME_START));
            } else {
                jsr_push(0xA666);
                animation = MariaAnimationRunning();
                jsr_pop();
                mx_animation(&e, animation);
                if (mx_zero(mx_one_animation(animation))) {
                    bool idle = false;

                    value = RD8(ZP->player_push);
                    value &= 0x01;
                    mx_regs(&e, r_lda(regs_kept(), value));
                    if (value == 0) {
                        value = RD8(PVARS->column_hit[0]);
                        value &= RD8(PVARS->column_hit[1]);
                        value &= RD8(PVARS->column_hit[2]);
                        value &= COLUMN_SOLID;
                        mx_regs(&e, r_lda(regs_kept(), value));
                        idle = value != 0;
                    }
                    if (idle) {
                        value = INC8(OBJ->work_a[OBJECT_PLAYER]);
                        value = RD8(OBJ->work_a[OBJECT_PLAYER]);
                        mx_regs(&e, r_lda(regs_kept(), value));
                        jsr_push(0xA684);
                        mx_animation(&e, MariaSetAnimation(value));
                        jsr_pop();
                    } else {
                        mx_regs(&e, r_lda(regs_kept(), ANIMATION_STAND_LOOK));
                        jsr_push(0xA696);
                        mx_animation(&e, MariaSetAnimation(ANIMATION_STAND_LOOK));
                        jsr_pop();
                    }
                }
            }
        }
    }
    facing = 0;
    mx_regs(&e, r_y_only(regs_kept(), facing));
    held = RD8(ZP->pad_held);
    mx_regs(&e, r_tst(regs_kept(), PAD_RIGHT, held));
    if (!(held & PAD_RIGHT)) {
        facing = 1;
        mx_regs(&e, r_ldy(regs_kept(), facing));
        held = RD8(ZP->pad_held);
        mx_regs(&e, r_tst(regs_kept(), PAD_LEFT, held));
        if (!(held & PAD_LEFT)) {
            return e;
        }
    }
    jsr_push(0xA69B);
    MariaPadForcesOff();
    jsr_pop();
    WR8(OBJ->facing_left[OBJECT_PLAYER], facing);
    held = RD8(ZP->pad_held);
    mx_regs(&e, r_tst(regs_kept(), PAD_I, held));
    if (held & PAD_I) {
        return e;
    }
    WR8(OBJ->state[OBJECT_PLAYER], STATE_WALK);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    mx_then(&e, MariaSetAnimationZero());
    return e;
}

void abi_MariaStandFrame(void)
{
    mx_exit(MariaStandFrame(cpu.x, cpu.y));
}

/*
 * @routine 01DD:4C4 MariaStandAttackFrame
 * @run 6D:A4C4
 * WHAT IT DOES: Step 1 of standing, the attack: while her animation runs, as standing still (MariaStandFrame), and
 * with OBJ_WORK_A $20 (the attack's animation) still set that is all; when it has stopped, or OBJ_WORK_A has changed,
 * the columns under her, a landing with footing, and back to standing in state 0.
 * HOW: MariaAnimationRunning's Z; MariaStandFrame; OBJ_WORK_A compared with $20; MariaReadColumns, MariaHasNoFooting,
 * MariaLandOnFloor; the tail JMP to MariaStand.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: held, A $20, Z and C set; not in state 0, A the state, N and Z from it; else as
 * MariaStand.
 */
MExit MariaStandAttackFrame(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    AnimationStep running;
    FootingCheck footing;
    uint8_t value;
    AnimThen stand;

    jsr_push(0xA4C7);
    running = MariaAnimationRunning();
    jsr_pop();
    mx_animation(&e, running);
    if (!mx_zero(mx_one_animation(running))) {
        jsr_push(0xA4CC);
        mx_then(&e, MariaStandFrame(x_after(x, y, e), y_after(x, y, e)));
        jsr_pop();
        value = RD8(OBJ->work_a[OBJECT_PLAYER]);
        mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, ATTACK_ANIMATION));
        if (value == ATTACK_ANIMATION) {
            return e;
        }
    }
    jsr_push(0xA4D7);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xA4DA);
    footing = MariaHasNoFooting();
    jsr_pop();
    mx_footing(&e, footing);
    if (!footing.no_floor) {
        jsr_push(0xA4DF);
        mx_landing(&e, MariaLandOnFloor());
        jsr_pop();
    }
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value != STATE_STAND) {
        return e;
    }
    stand = MariaStand();
    mx_animation(&e, stand.animation);
    mx_regs(&e, stand.regs);
    return e;
}

void abi_MariaStandAttackFrame(void)
{
    mx_exit(MariaStandAttackFrame(cpu.x, cpu.y));
}

/*
 * @routine 01DD:5E2 MariaStateStand
 * @run 6D:A5E2
 * WHAT IT DOES: State 0: the step's handler, from MariaStandSteps (still, the attack, the item crash).
 * HOW: A BSR to MariaStepIndex, then the JMP (table,X).
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the step's handler, after A and X the step doubled and C its bit 7.
 */
MExit MariaStateStand(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t step, doubled, lo, hi;
    uint16_t target;
    Regs at_jump;

    jsr_push(0xA5E4);
    step = MariaStepIndex();
    jsr_pop();
    doubled = (uint8_t)(step << 1);
    at_jump = r_c(r_ldx(r_lda(regs_kept(), doubled), doubled), (step & 0x80) != 0);
    mx_regs(&e, at_jump);
    lo = TBL8(MariaStandSteps, doubled);
    hi = TBL8(MariaStandSteps, doubled + 1);
    target = (uint16_t)(lo | (hi << 8));
    switch (target) {
    case STAND_STILL_STEP:
        mx_then(&e, MariaStandFrame(doubled, y));
        break;
    case STAND_ATTACK_STEP:
        mx_then(&e, MariaStandAttackFrame(doubled, y));
        break;
    case STAND_CRASH_STEP:
        mx_then(&e, MariaCrashFrame());
        break;
    default:
        mx_exit(e);
        run_routine(target);
        e.elsewhere = true;
        break;
    }
    (void)x;
    return e;
}

void abi_MariaStateStand(void)
{
    mx_exit(MariaStateStand(cpu.x, cpu.y));
}
