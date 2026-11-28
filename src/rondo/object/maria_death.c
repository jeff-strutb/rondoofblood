/*
 * Maria's death (bank $6E, run from her overlay's vectors at $A00C and $A00F): the leap the death starts with, and
 * the frame of her death: knocked up and falling to a floor, the fall's end, then the stage's point steps
 * (MariaStagePointFrame, maria_objects_probe.c) until zField89 says it is over.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"
#include "maria_objects.h"

enum {
    OBJECT_PLAYER = 0,
    STATE_DEATH_LEAP = 3, LEAP_TIMER = 6, LEAP_VY_LO = 0xC0, LEAP_VY_HI = 0xFC, ANIMATION_DEATH_LEAP = 0x1E,
    ANIMATION_DEATH_DOWN = 0x25, SCRIPT_DEATH_LEAP = 0x0B, SCRIPT_HELD = 1, SCRIPT_DEATH_FALL = 8,
    SCRIPT_DEATH_DOWN = 2, DEATH_GRAVITY = 0x20, FALL_SPEED_TOP = 7, DEATH_OFF_MAP = 1, TILE_BIT3 = 0x08,
    DISSOLVE_CLOCK = 0x40
};

void MariaStagePointFrame(void);                                    /* object/maria_objects_probe.c */

/*
 * @routine 01E1:484 MariaDeathLeap
 * @run 6E:C484
 * WHAT IT DOES: The leap her death starts with: the side she is knocked to, lifted three rows, Y speed $FCC0,
 * animation $1E (plus OBJ_KNOCK_FORWARD), the death's sound, state 3 step 0, timer 6, script $0B, and held, script 1.
 * HOW: MariaHurtSide, MariaLiftThree, MariaSetAnimation, MariaDeathSound, MariaStartHitScript; the tail JMP to it when
 * held.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaStartHitScript with $0B, then TST of PlayerHeld (Z set, N and V its
 * bits 7 and 6), or as MariaStartHitScript with 1.
 */
MExit MariaDeathLeap(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    Sum animation;
    uint8_t held;

    jsr_push(0xC487);
    mx_regs(&e, MariaHurtSide());
    jsr_pop();
    jsr_push(0xC48A);
    mx_regs(&e, MariaLiftThree());
    jsr_pop();
    WR8(OBJ->vy_lo[OBJECT_PLAYER], LEAP_VY_LO);
    WR8(OBJ->vy_hi[OBJECT_PLAYER], LEAP_VY_HI);
    animation = add_bytes(ANIMATION_DEATH_LEAP, RD8(OBJ->knock_forward[OBJECT_PLAYER]), false);
    mx_regs(&e, r_v(r_c(r_lda(regs_kept(), animation.value), animation.carry), animation.overflow));
    jsr_push(0xC49D);
    mx_animation(&e, MariaSetAnimation(animation.value));
    jsr_pop();
    jsr_push(0xC4A0);
    mx_sound(&e, 0x60, MariaDeathSound(mx_x_from(x, y, e), mx_y_from(x, y, e)));
    jsr_pop();
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    WR8(OBJ->state[OBJECT_PLAYER], STATE_DEATH_LEAP);
    WR8(OBJ->anim_timer[OBJECT_PLAYER], LEAP_TIMER);
    jsr_push(0xC4B2);
    MariaStartHitScript(SCRIPT_DEATH_LEAP);
    jsr_pop();
    mx_script(&e, SCRIPT_DEATH_LEAP);
    held = RD8(PVARS->held);
    mx_regs(&e, r_tst(regs_kept(), 0xFF, held));
    if (held != 0) {
        MariaStartHitScript(SCRIPT_HELD);
        mx_script(&e, SCRIPT_HELD);
    }
    return e;
}

void abi_MariaDeathLeap(void)
{
    mx_exit(MariaDeathLeap(cpu.x, cpu.y));
}

/*
 * @routine 01E1:3A9 MariaDeathFrame
 * @run 6E:C3A9
 * WHAT IT DOES: A frame of her death. Step 0: the leap's timer, and at its end script 8 unless held; her animation and
 * the knock's speed; falling, a floor her column stands on lifts her onto it, stops her, animation $25, step 1, and
 * unless held script 2; else a bit-3 tile under column 0 starts the burst; gravity $0020 held at $07xx; off the bottom
 * of the map (Y high 1), it is over (zField89 1). Step 1: the columns, the floor, a landing, and when her animation
 * has stopped, the dissolve set up (OBJ_DISSOLVE_PHASE_K 0, OBJ_DISSOLVE_CLOCK_J $40) and step 2. Later steps,
 * MariaStagePointFrame.
 * HOW: OBJ_SUBSTATE; DEC of OBJ_ANIM_TIMER, TST of PlayerHeld, MariaStartHitScript; MariaAnimationTick, MariaKnockSpeedX;
 * MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn, MariaLiftByColumn, MariaStopMoving, MariaSetAnimation;
 * MariaBit3Burst; the 16-bit add; MariaLandOnFloor, MariaAnimationRunning; the JMP to MariaStagePointFrame and a branch
 * into its end.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the routine it ended in, or the last test.
 */
MExit MariaDeathFrame(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    FloorResult floor;
    AnimationStep running;
    uint8_t step, value, column, held;
    Sum lo, hi;

    step = RD8(OBJ->substate[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), step));
    if (step != 0) {
        step = (uint8_t)(step - 1);
        mx_regs(&e, r_lda(regs_kept(), step));
        if (step != 0) {
            mx_exit(e);
            MariaStagePointFrame();
            e.elsewhere = true;
            return e;
        }
        jsr_push(0xC42B);
        mx_columns(&e, MariaReadColumns());
        jsr_pop();
        jsr_push(0xC42E);
        mx_floor(&e, MariaFloorCheck());
        jsr_pop();
        jsr_push(0xC431);
        mx_landing(&e, MariaLandOnFloor());
        jsr_pop();
        jsr_push(0xC434);
        running = MariaAnimationRunning();
        jsr_pop();
        mx_animation(&e, running);
        if (!mx_zero(mx_one_animation(running))) {
            return e;
        }
        WR8(OBJ->dissolve_phase_k[OBJECT_PLAYER], 0);
        WR8(OBJ->dissolve_clock_j[OBJECT_PLAYER], DISSOLVE_CLOCK);
        mx_regs(&e, r_lda(regs_kept(), DISSOLVE_CLOCK));
        mx_regs(&e, r_nz(regs_kept(), INC8(OBJ->substate[OBJECT_PLAYER])));
        return e;
    }
    value = RD8(OBJ->anim_timer[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value != 0) {
        value = DEC8(OBJ->anim_timer[OBJECT_PLAYER]);
        mx_regs(&e, r_nz(regs_kept(), value));
        if (value == 0) {
            held = RD8(PVARS->held);
            mx_regs(&e, r_tst(regs_kept(), 0xFF, held));
            if (held == 0) {
                mx_regs(&e, r_lda(regs_kept(), SCRIPT_DEATH_FALL));
                jsr_push(0xC3C9);
                MariaStartHitScript(SCRIPT_DEATH_FALL);
                jsr_pop();
                mx_script(&e, SCRIPT_DEATH_FALL);
            }
        }
    }
    jsr_push(0xC3CC);
    mx_animation(&e, MariaAnimationTick());
    jsr_pop();
    jsr_push(0xC3CF);
    mx_regs(&e, MariaKnockSpeedX());
    jsr_pop();
    value = RD8(OBJ->vy_hi[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (!(value & 0x80)) {
        bool landed = false;

        jsr_push(0xC3D7);
        mx_columns(&e, MariaReadColumns());
        jsr_pop();
        jsr_push(0xC3DA);
        floor = MariaFloorCheck();
        jsr_pop();
        mx_floor(&e, floor);
        if (floor.how != FLOOR_NONE) {
            column = mx_x_from(x, y, e);
            jsr_push(0xC3DF);
            mx_solid(&e, MariaSolidUnderColumn(column));
            jsr_pop();
            landed = mx_zero(e);
        }
        if (landed) {
            column = mx_x_from(x, y, e);
            jsr_push(0xC3E4);
            mx_lift(&e, MariaLiftByColumn(column));
            jsr_pop();
            jsr_push(0xC3E7);
            MariaStopMoving();
            jsr_pop();
            mx_regs(&e, r_lda(regs_kept(), ANIMATION_DEATH_DOWN));
            jsr_push(0xC3EC);
            mx_animation(&e, MariaSetAnimation(ANIMATION_DEATH_DOWN));
            jsr_pop();
            mx_regs(&e, r_nz(regs_kept(), INC8(OBJ->substate[OBJECT_PLAYER])));
            held = RD8(PVARS->held);
            mx_regs(&e, r_tst(regs_kept(), 0xFF, held));
            if (held != 0) {
                return e;
            }
            mx_regs(&e, r_lda(regs_kept(), SCRIPT_DEATH_DOWN));
            MariaStartHitScript(SCRIPT_DEATH_DOWN);
            mx_script(&e, SCRIPT_DEATH_DOWN);
            return e;
        }
        value = RD8(PVARS->column_hit[0]);
        mx_regs(&e, r_tst(regs_kept(), TILE_BIT3, value));
        if (value & TILE_BIT3) {
            jsr_push(0xC404);
            mx_then(&e, MariaBit3Burst(mx_x_from(x, y, e), mx_y_from(x, y, e)));
            jsr_pop();
        }
    }
    value = RD8(OBJ->vy_lo[OBJECT_PLAYER]);
    lo = add_bytes(value, DEATH_GRAVITY, false);
    WR8(OBJ->vy_lo[OBJECT_PLAYER], lo.value);
    value = RD8(OBJ->vy_hi[OBJECT_PLAYER]);
    hi = add_bytes(value, 0, lo.carry);
    WR8(OBJ->vy_hi[OBJECT_PLAYER], hi.value);
    mx_regs(&e, r_v(r_c(r_lda(regs_kept(), hi.value), hi.carry), hi.overflow));
    if (!(hi.value & 0x80)) {
        mx_regs(&e, r_cmp(regs_kept(), hi.value, FALL_SPEED_TOP));
        if (hi.value >= FALL_SPEED_TOP) {
            WR8(OBJ->vy_lo[OBJECT_PLAYER], 0);
        }
    }
    value = RD8(OBJ->y_hi[OBJECT_PLAYER]);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, DEATH_OFF_MAP));
    if (value != DEATH_OFF_MAP) {
        return e;
    }
    WR8(ZP->field89, 1);
    mx_regs(&e, r_lda(regs_kept(), 1));
    return e;
}

void abi_MariaDeathFrame(void)
{
    mx_exit(MariaDeathFrame(cpu.x, cpu.y));
}

/*
 * @routine 01DD:00C MariaVecDeathLeap
 * @run 6D:A00C
 * WHAT IT DOES: Her overlay's vector to MariaDeathLeap.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as MariaDeathLeap.
 */
MExit MariaVecDeathLeap(uint8_t x, uint8_t y)
{
    return MariaDeathLeap(x, y);
}

void abi_MariaVecDeathLeap(void)
{
    mx_exit(MariaVecDeathLeap(cpu.x, cpu.y));
}

/*
 * @routine 01DD:015 MariaVecClearObjectsBut
 * @run 6D:A015
 * WHAT IT DOES: Her overlay's vector to MariaClearObjectsBut.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as MariaClearObjectsBut.
 */
Regs MariaVecClearObjectsBut(void)
{
    return MariaClearObjectsBut();
}

void abi_MariaVecClearObjectsBut(void)
{
    regs_exit(MariaVecClearObjectsBut());
}
