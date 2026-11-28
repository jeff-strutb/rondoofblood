/*
 * Maria's objects (her overlay, bank $6E): kind 11 (the pieces kind 15 rains), kind 18, and kind 19, ten objects from
 * slot 5 to 14 that fly each to its own place (MariaKind19TargetsX and Y), wait, fall, and wait on slot 14 before
 * they end; slot 14's arrival slows time and starts a fade.
 *
 * These routines hold the processor (maria_objects_p.h).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"
#include "maria_objects_p.h"

enum {
    OBJECT_PLAYER = 0, STATE_HIT = 9, SLOT_FIRST = 5, SLOT_LAST = 0x0E,
    K11_HALF = 0x0E, K11_WORK_N = 0x0D, K11_ANIMATION = 0x15, K11_SPRITE_SET = 3,
    K18_FRAME_AT = 0x50, K18_HOLD_A = 0x51, K18_HOLD_B = 0x52, K18_FRAME = 0x2A, K18_DX = 9, K18_DY = 0xF1,
    K18_HELD_DX = 7, K18_HELD_DY = 0xEC,
    K19_KIND = 0x13, K19_SLOTS = 9, K19_HALF_H = 8, K19_HALF_W = 5, K19_WORK_N = 0x0B, K19_DX = 2, K19_DY = 0xFE,
    K19_SOUND = 0x5F, K19_FALL_ENTRY = 0x0F, K19_FALL_LO = 0xC8, K19_FALL_HI = 0xFF, K19_FALL_FRAMES = 0x20,
    K19_AT_STEP = 2, K19_LAST_STATE = 4, K19_FADE = 2, K19_TURN = 8, K19_ANGLE_MASK = 0xF8, K19_SPEED_HI = 1,
    K19_START = 0xDA8F, K19_AIM = 0xDAD0, K19_WAIT = 0xDB0C, K19_FALL = 0xDB2D, K19_WAIT_LAST = 0xDB51,
    K19_END = 0xDB5D
};

#define MariaKind19TargetsX_TBL MariaKind19TargetsX
#define MariaKind19TargetsY_TBL MariaKind19TargetsY
#define MariaKind19Animations_TBL MariaKind19Animations
#define MariaKind19Waits_TBL MariaKind19Waits

#define P_PUT(field, value) do { p_lda(value); WR8(field, cpu.a); } while (0)

/* A kernel call through its vector, a JSR returning to `ret` or the tail JMP (ret 0), its exit given to cpu. */
static void pj_angle_from_delta(uint16_t ret)
{
    pj_in(ret);
    angle_exit_shim(AngleFromDelta());
    pj_out(ret);
}

static void pj_turn_angle(uint16_t ret)
{
    pj_in(ret);
    turn_angle_exit_shim(TurnAngleToward());
    pj_out(ret);
}

static void pj_velocity_angle(uint16_t ret)
{
    pj_in(ret);
    regs_exit(regs_of_scale(SetVelocityFromAngle(cpu.a, cpu.x)));
    pj_out(ret);
}

/* TXA, SEC, SBC #5, TAY: the object's slot less 5 in Y. */
static void p_slot_index(void)
{
    p_lda(cpu.x);
    abi_flag(P_C, true);
    p_sbc(SLOT_FIRST);
    p_ldy(cpu.a);
}

/* ------------------------------------------------------------------------------------------------ kind 11 */

/*
 * @routine 01E4:1B3 MariaKind11
 * @run 6E:D9B3
 * WHAT IT DOES: The handler of kind 11, a falling piece: in state 0 a hitbox $1C square, live, OBJ_WORK_N $0D,
 * animation $15, sprite set 3, and its next state; after, it moves with its animation and is cleared when the
 * animation has stopped.
 * HOW: OBJ_STATE; MariaObjectSetAnimation, MariaObjectNextState; MariaObjectMove, MariaObjectAnimationRunning's Z,
 * ClearObject by the tail JMP.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last step made.
 */
void MariaKind11(void)
{
    p_ldx_object();
    p_lda(RD8(OBJ->state[cpu.x]));
    if (p_z()) {
        p_lda(K11_HALF);
        WR8(OBJ->hitbox_half_h[cpu.x], cpu.a);
        WR8(OBJ->hitbox_half_w[cpu.x], cpu.a);
        P_PUT(OBJ->hitbox_on[cpu.x], 1);
        P_PUT(OBJ->work_n[cpu.x], K11_WORK_N);
        p_lda(K11_ANIMATION);
        pj_set_animation(0xD9D1);
        P_PUT(OBJ->sprite_set[cpu.x], K11_SPRITE_SET);
        pj_next_state(0);
        return;
    }
    pj_move(0xD9DC);
    pj_animation_running(0xD9DF);
    if (!p_z()) {
        return;
    }
    pj_clear_object(0);
}

void abi_MariaKind11(void)
{
    MariaKind11();
}

/* ------------------------------------------------------------------------------------------------ kind 18 */

/*
 * @routine 01E4:225 MariaKind18
 * @run 6E:DA25
 * WHAT IT DOES: The handler of kind 18. State 0: cleared when she is hit; at her frame $50 its next state, sprite set
 * 1, frame $2A, and her place plus (9, -$0F) with her flip and facing. After: at her frame $50 the same place; at $51
 * and $52 her place plus (7, -$14); at any other frame a kind 3 slot is claimed and it is cleared.
 * HOW: Her OBJ_STATE and OBJ_FRAME; MariaObjectNextState, MariaObjectFromPlayer, MariaObjectAtPlayer with zTemp04 and
 * zTemp06; MariaClaimKind3Slot, ClearObject by the tail JMP.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last step made.
 */
void MariaKind18(void)
{
    p_ldx_object();
    p_lda(RD8(OBJ->state[cpu.x]));
    if (p_z()) {
        p_lda(RD8(OBJ->state[OBJECT_PLAYER]));
        p_cmp(cpu.a, STATE_HIT);
        if (p_z()) {
            pj_clear_object(0);
            return;
        }
        p_lda(RD8(OBJ->frame[OBJECT_PLAYER]));
        p_cmp(cpu.a, K18_FRAME_AT);
        if (!p_z()) {
            return;
        }
        pj_next_state(0xDA41);
        P_PUT(OBJ->sprite_set[cpu.x], 1);
        P_PUT(OBJ->frame[cpu.x], K18_FRAME);
    } else {
        p_lda(RD8(OBJ->frame[OBJECT_PLAYER]));
        p_cmp(cpu.a, K18_FRAME_AT);
        if (!p_z()) {
            p_cmp(cpu.a, K18_HOLD_A);
            if (!p_z()) {
                p_cmp(cpu.a, K18_HOLD_B);
            }
            if (!p_z()) {
                pj_claim_kind3(0xDA6C);
                pj_clear_object(0);
                return;
            }
            P_PUT(ZP->temp04, K18_HELD_DX);
            P_PUT(ZP->temp06, K18_HELD_DY);
            pj_at_player(0xDA7A);
            pj_from_player(0);
            return;
        }
    }
    pj_from_player(0xDA4E);
    P_PUT(ZP->temp04, K18_DX);
    P_PUT(ZP->temp06, K18_DY);
    pj_at_player(0xDA59);
}

void abi_MariaKind18(void)
{
    MariaKind18();
}

/* ------------------------------------------------------------------------------------------------ kind 19 */

/*
 * @routine 01E4:36F MariaObjectAimAtPlace
 * @run 6E:DB6F
 * WHAT IT DOES: Turns a kind 19 object toward its place (MariaKind19TargetsX and Y by slot less 5) by up to 8 a frame
 * (its angle in OBJ_FLY_ANGLE, kept a multiple of 8), and gives it speed $0100 along it.
 * HOW: The place less its own X and Y into zTemp00-03; AngleFromDelta, TurnAngleToward with step 8, SetVelocityFromAngle
 * by the tail JMP (the kernel's vectors at $404E, $406C and $4048).
 * WAITS: no.
 * Entry: X the object. Exit: as SetVelocityFromAngle.
 */
void MariaObjectAimAtPlace(void)
{
    uint8_t index, angle;

    p_slot_index();
    index = cpu.y;
    p_lda(TBL8(MariaKind19TargetsX, index));
    abi_flag(P_C, true);
    p_sbc(RD8(OBJ->x_lo[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    cpu.a = 0;
    p_sbc(RD8(OBJ->x_hi[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    p_lda(TBL8(MariaKind19TargetsY, index));
    abi_flag(P_C, true);
    p_sbc(RD8(OBJ->y_lo[cpu.x]));
    WR8(ZP->temp02, cpu.a);
    cpu.a = 0;
    p_sbc(RD8(OBJ->y_hi[cpu.x]));
    WR8(ZP->temp03, cpu.a);
    pj_angle_from_delta(0xDB95);
    WR8(ZP->temp02, cpu.a);
    P_PUT(ZP->temp00, K19_TURN);
    p_lda(RD8(OBJ->fly_angle[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    pj_turn_angle(0xDBA3);
    p_lda((uint8_t)(cpu.a & K19_ANGLE_MASK));
    WR8(OBJ->fly_angle[cpu.x], cpu.a);
    push8(cpu.a);
    P_PUT(ZP->temp00, 0);
    P_PUT(ZP->temp01, K19_SPEED_HI);
    angle = pull8();
    p_lda(angle);
    pj_velocity_angle(0);
}

void abi_MariaObjectAimAtPlace(void)
{
    MariaObjectAimAtPlace();
}

/*
 * @routine 01E4:28F MariaKind19Start
 * @run 6E:DA8F
 * WHAT IT DOES: Kind 19's state 0: from slot 5, slots 6 to 14 become kind $13 too; each takes a hitbox $10 high and
 * $0A wide, live, OBJ_WORK_N $0B, her flip and facing, her place plus (2, -2), faces right, and starts its animation
 * by slot (MariaKind19Animations).
 * HOW: The stores; MariaObjectFromPlayer, MariaObjectAtPlayer; the tail JMP to MariaObjectStartAnimation.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as MariaObjectStartAnimation.
 */
void MariaKind19Start(void)
{
    p_ldx_object();
    p_cmp(cpu.x, SLOT_FIRST);
    if (p_z()) {
        p_lda(K19_KIND);
        cpu.y = 0;
        do {
            WR8(OBJ->kind[6 + cpu.y], cpu.a);
            p_ldy((uint8_t)(cpu.y + 1));
            p_cmp(cpu.y, K19_SLOTS);
        } while (!p_z());
    }
    P_PUT(OBJ->hitbox_half_h[cpu.x], K19_HALF_H);
    P_PUT(OBJ->hitbox_half_w[cpu.x], K19_HALF_W);
    P_PUT(OBJ->hitbox_on[cpu.x], 1);
    P_PUT(OBJ->work_n[cpu.x], K19_WORK_N);
    pj_from_player(0xDAB7);
    P_PUT(ZP->temp04, K19_DX);
    P_PUT(ZP->temp06, K19_DY);
    pj_at_player(0xDAC2);
    p_slot_index();
    WR8(OBJ->facing_left[cpu.x], 0);
    p_lda(TBL8(MariaKind19Animations, cpu.y));
    pj_start_animation(0);
}

void abi_MariaKind19Start(void)
{
    MariaKind19Start();
}

/*
 * @routine 01E4:2D0 MariaKind19Aim
 * @run 6E:DAD0
 * WHAT IT DOES: Kind 19's state 1: its animation, its aim, its move; once above its place's Y, slot 14 sounds $5F,
 * slows time and starts a fade; each takes its wait (MariaKind19Waits), stops, and its next state.
 * HOW: MariaObjectAnimationTick, MariaObjectAimAtPlace, MariaObjectApplyVelocity; its Y against MariaKind19TargetsY;
 * SoundCommand, TimeSlowMode, zFadeCount, zFadeActive; MariaObjectStop, the tail JMP to MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: short of its place, A its Y, N, Z and C from the compare; else as
 * MariaObjectNextState.
 */
void MariaKind19Aim(void)
{
    p_ldx_object();
    pj_animation_tick(0xDAD5);
    pj_proc(0xDAD8, MariaObjectAimAtPlace);
    pj_apply_velocity(0xDADB);
    p_slot_index();
    p_lda(RD8(OBJ->y_lo[cpu.x]));
    p_cmp(cpu.a, TBL8(MariaKind19TargetsY, cpu.y));
    if (p_c()) {
        return;
    }
    p_cmp(cpu.x, SLOT_LAST);
    if (p_z()) {
        p_lda(K19_SOUND);
        pj_sound(0xDAF2);
        p_lda(1);
        WR8(PVARS->time_slow_mode, cpu.a);
        WR8(ZP->fade_count, cpu.a);
        WR8(ZP->fade_active, cpu.a);
    }
    p_slot_index();
    P_PUT(OBJ->anim_timer[cpu.x], TBL8(MariaKind19Waits, cpu.y));
    pj_void_x(0xDB09, MariaObjectStop);
    pj_next_state(0);
}

void abi_MariaKind19Aim(void)
{
    MariaKind19Aim();
}

/*
 * @routine 01E4:30C MariaKind19Wait
 * @run 6E:DB0C
 * WHAT IT DOES: Kind 19's state 2: its wait counts down every other frame with its animation; when it runs out, the
 * Y speed of entry $0F, a fall of $FFC8 a frame (OBJ_FALL_LO and _HI), and its next state.
 * HOW: TST of zWaveCounter, DEC of OBJ_ANIM_TIMER; MariaObjectAnimationTick by the tail JMP; MariaSetVelocityYFromTable,
 * MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as MariaObjectAnimationTick or MariaObjectNextState.
 */
void MariaKind19Wait(void)
{
    p_ldx_object();
    p_tst(0x01, RD8(ZP->wave_counter));
    if (p_z()) {
        abi_nz(DEC8(OBJ->anim_timer[cpu.x]));
        if (p_z()) {
            p_lda(K19_FALL_ENTRY);
            pj_velocity_y(0xDB20);
            P_PUT(OBJ->fall_lo[cpu.x], K19_FALL_LO);
            P_PUT(OBJ->fall_hi[cpu.x], K19_FALL_HI);
            pj_next_state(0);
            return;
        }
    }
    pj_animation_tick(0);
}

void abi_MariaKind19Wait(void)
{
    MariaKind19Wait();
}

/*
 * @routine 01E4:32D MariaKind19Fall
 * @run 6E:DB2D
 * WHAT IT DOES: Kind 19's state 3: moves and falls; after $20 frames its animation goes back three and is at step 2,
 * and its next state.
 * HOW: MariaObjectApplyVelocity, MariaObjectFallCapped; INC of OBJ_ANIM_TIMER; DECs of OBJ_ANIMATION, the tail JMP to
 * MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: A the count, N, Z and C from comparing it with $20; or as MariaObjectNextState.
 */
void MariaKind19Fall(void)
{
    p_ldx_object();
    pj_apply_velocity(0xDB32);
    pj_fall_capped(0xDB35);
    abi_nz(INC8(OBJ->anim_timer[cpu.x]));
    p_lda(RD8(OBJ->anim_timer[cpu.x]));
    p_cmp(cpu.a, K19_FALL_FRAMES);
    if (!p_z()) {
        return;
    }
    abi_nz(DEC8(OBJ->animation[cpu.x]));
    abi_nz(DEC8(OBJ->animation[cpu.x]));
    abi_nz(DEC8(OBJ->animation[cpu.x]));
    P_PUT(OBJ->animation_at[cpu.x], K19_AT_STEP);
    pj_next_state(0);
}

void abi_MariaKind19Fall(void)
{
    MariaKind19Fall();
}

/*
 * @routine 01E4:351 MariaKind19WaitLast
 * @run 6E:DB51
 * WHAT IT DOES: Kind 19's state 4: waits until slot 14 is in state 4 too, then its next state.
 * HOW: OBJ_STATE of slot 14; the tail JMP to MariaObjectNextState, or MariaKind19Fall's RTS.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: waiting, A slot 14's state, N, Z and C from comparing it with 4; else as
 * MariaObjectNextState.
 */
void MariaKind19WaitLast(void)
{
    p_ldx_object();
    p_lda(RD8(OBJ->state[SLOT_LAST]));
    p_cmp(cpu.a, K19_LAST_STATE);
    if (!p_z()) {
        return;
    }
    pj_next_state(0);
}

void abi_MariaKind19WaitLast(void)
{
    MariaKind19WaitLast();
}

/*
 * @routine 01E4:35D MariaKind19End
 * @run 6E:DB5D
 * WHAT IT DOES: Kind 19's state 5: when its animation has stopped, time back to normal, the fade's count 2, and it is
 * cleared.
 * HOW: MariaObjectAnimationRunning's Z; the stores; ClearObject by the tail JMP.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as MariaObjectAnimationRunning, or ClearObject.
 */
void MariaKind19End(void)
{
    p_ldx_object();
    pj_animation_running(0xDB62);
    if (!p_z()) {
        return;
    }
    WR8(PVARS->time_slow_mode, 0);
    P_PUT(ZP->fade_count, K19_FADE);
    pj_clear_object(0);
}

void abi_MariaKind19End(void)
{
    MariaKind19End();
}

/*
 * @routine 01E4:27D MariaKind19
 * @run 6E:DA7D
 * WHAT IT DOES: The handler of kind 19: its state's routine from MariaKind19States (start, aim, wait, fall, wait for
 * slot 14, end).
 * HOW: MariaStateDoubled, then the JMP (table,X).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
 */
void MariaKind19(void)
{
    uint16_t target;

    pj_state_doubled(0xDA80);
    target = p_table_word(MariaKind19States);
    switch (target) {
    case K19_START:
        MariaKind19Start();
        break;
    case K19_AIM:
        MariaKind19Aim();
        break;
    case K19_WAIT:
        MariaKind19Wait();
        break;
    case K19_FALL:
        MariaKind19Fall();
        break;
    case K19_WAIT_LAST:
        MariaKind19WaitLast();
        break;
    case K19_END:
        MariaKind19End();
        break;
    default:
        p_jump_elsewhere(target);
        break;
    }
}

void abi_MariaKind19(void)
{
    MariaKind19();
}
