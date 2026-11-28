/*
 * Maria's objects (her overlay, bank $6E): kind 12, item 4's second object (MariaThrowItem puts it in slot 9 when she
 * has $50 hearts or more), and kind 10, the object of her pad sequence's move (MariaSequenceMoveStart puts it in slot
 * 15). Each lives while she is in the state that made it: $10 for item 4, $11 for the sequence's move.
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
    OBJECT_PLAYER = 0, STATE_ITEM4 = 0x10, STATE_SEQUENCE = 0x11, BELOW_SCREEN = 1, ITEM4_STEP_THROWN = 1,
    PLAYER_FRAME_HELD = 0x42, SEQ_WORK_N = 0x0D, SEQ_HALF_H = 6, SEQ_HALF_W = 0x0F, SEQ_DY = 3, SEQ_DX = 0x0F,
    SEQ_START_FRAME = 0x11, SEQ_STOP_FRAME = 0x47, SEQ_FLY_FRAME = 0x46, SEQ_TIMER = 0x20, SEQ_FALL_LO = 0x24,
    SEQ_LAST_FRAME = 0x12,
    K12_START = 0xCDBC, K12_FOLLOW = 0xCDDF, K12_HELD = 0xCE41,
    K10_START = 0xCE7F, K10_RISE = 0xCEB7, K10_TURN = 0xCEEA, K10_FALL = 0xCF22
};

void MariaObjectFallStart7(void);                                    /* object/maria_objects_items.c */

/* ------------------------------------------------------------------------------------------------ kind 12 */

/*
 * @routine 01E2:5B0 MariaItem4Extra
 * @run 6E:CDB0
 * WHAT IT DOES: The handler of kind 12, item 4's second object: its state's routine from MariaItem4ExtraStates.
 * HOW: MariaStateDoubled, then the JMP (table,X).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
 */
void MariaItem4Extra(void)
{
    uint16_t target;

    pj_state_doubled(0xCDB3);
    target = p_table_word(MariaItem4ExtraStates);
    switch (target) {
    case K12_START:
        MariaItem4ExtraStart();
        break;
    case K12_FOLLOW:
        MariaItem4ExtraFollow();
        break;
    case K12_HELD:
        MariaItem4ExtraHeld();
        break;
    default:
        p_jump_elsewhere(target);
        break;
    }
}

void abi_MariaItem4Extra(void)
{
    MariaItem4Extra();
}

/*
 * @routine 01E2:5BC MariaItem4ExtraStart
 * @run 6E:CDBC
 * WHAT IT DOES: State 0 of kind 12: while she is in item 4's state on the screen, it waits for her step 1, then takes
 * her X and starts falling ($0038 a frame) with animation 7. Out of that state, or with her below the screen, it is
 * cleared.
 * HOW: Her OBJ_STATE, OBJ_Y_HI and OBJ_SUBSTATE; the JMP to MariaObjectFallStart7, or to ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: waiting, A her step, N, Z and C from comparing it with 1; else as the routine
 * jumped to.
 */
void MariaItem4ExtraStart(void)
{
    uint8_t value;

    p_ldx_object();
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, STATE_ITEM4);
    if (value == STATE_ITEM4) {
        value = RD8(OBJ->y_hi[OBJECT_PLAYER]);
        p_lda((uint8_t)(value - 1));
        if (cpu.a != 0) {
            value = RD8(OBJ->substate[OBJECT_PLAYER]);
            p_lda(value);
            p_cmp(value, ITEM4_STEP_THROWN);
            if (value != ITEM4_STEP_THROWN) {
                return;
            }
            p_lda(RD8(OBJ->x_lo[OBJECT_PLAYER]));
            WR8(OBJ->x_lo[cpu.x], cpu.a);
            MariaObjectFallStart7();
            return;
        }
    }
    ClearObject(cpu.x);
}

void abi_MariaItem4ExtraStart(void)
{
    MariaItem4ExtraStart();
}

/*
 * @routine 01E2:5DF MariaItem4ExtraFollow
 * @run 6E:CDDF
 * WHAT IT DOES: State 1 of kind 12: from her (facing and flip), animates, moves, takes her X and falls; below the
 * screen it is cleared. While she is in item 4's state and it is near her, it goes above her, keeps her X less its own
 * in OBJ_HELD_X_OFFSET, and goes to its next state.
 * HOW: MariaObjectFromPlayer, MariaObjectAnimationTick, MariaObjectMove, MariaObjectFallCapped, MariaObjectNearPlayer's
 * carry, a BSR to MariaObjectAbovePlayer, the tail JMP to MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last step or the routine jumped to.
 */
void MariaItem4ExtraFollow(void)
{
    uint8_t value;

    p_ldx_object();
    pj_from_player(0xCDE4);
    pj_animation_tick(0xCDE7);
    pj_move(0xCDEA);
    p_lda(RD8(OBJ->x_lo[OBJECT_PLAYER]));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    pj_fall_capped(0xCDF3);
    value = RD8(OBJ->y_hi[cpu.x]);
    p_lda(value);
    p_cmp(value, BELOW_SCREEN);
    if (value == BELOW_SCREEN) {
        ClearObject(cpu.x);
        return;
    }
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, STATE_ITEM4);
    if (value != STATE_ITEM4) {
        return;
    }
    pj_near_player(0xCE04);
    if (!p_c()) {
        return;
    }
    pj_above_player(0xCE08);
    p_lda(RD8(OBJ->x_lo[OBJECT_PLAYER]));
    abi_flag(P_C, true);
    p_sbc(RD8(OBJ->x_lo[cpu.x]));
    WR8(OBJ->held_x_offset[cpu.x], cpu.a);
    pj_next_state(0);
}

void abi_MariaItem4ExtraFollow(void)
{
    MariaItem4ExtraFollow();
}

/*
 * @routine 01E2:641 MariaItem4ExtraHeld
 * @run 6E:CE41
 * WHAT IT DOES: State 2 of kind 12, held above her: from her; it animates unless her frame is $42; it stays above
 * her at her X less OBJ_HELD_X_OFFSET; when she leaves item 4's state, or is below the screen, it is cleared.
 * HOW: MariaObjectFromPlayer, MariaObjectAnimationTick, MariaObjectAbovePlayer; the tail JMPs to ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: held, A her Y's high byte less one, N and Z from it; else as ClearObject.
 */
void MariaItem4ExtraHeld(void)
{
    uint8_t value;

    p_ldx_object();
    pj_from_player(0xCE46);
    value = RD8(OBJ->frame[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, PLAYER_FRAME_HELD);
    if (value != PLAYER_FRAME_HELD) {
        pj_animation_tick(0xCE50);
    }
    pj_above_player(0xCE53);
    p_lda(RD8(OBJ->x_lo[OBJECT_PLAYER]));
    abi_flag(P_C, true);
    p_sbc(RD8(OBJ->held_x_offset[cpu.x]));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, STATE_ITEM4);
    if (value != STATE_ITEM4) {
        ClearObject(cpu.x);
        return;
    }
    value = RD8(OBJ->y_hi[OBJECT_PLAYER]);
    p_lda((uint8_t)(value - 1));
    if (cpu.a == 0) {
        ClearObject(cpu.x);
    }
}

void abi_MariaItem4ExtraHeld(void)
{
    MariaItem4ExtraHeld();
}

/* ------------------------------------------------------------------------------------------------ kind 10 */

/*
 * @routine 01E2:671 MariaSequenceObject
 * @run 6E:CE71
 * WHAT IT DOES: The handler of kind 10, the object of her pad sequence's move: its state's routine from
 * MariaSequenceObjectStates (start, rise, turn, fall).
 * HOW: MariaStateDoubled, then the JMP (table,X).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
 */
void MariaSequenceObject(void)
{
    uint16_t target;

    pj_state_doubled(0xCE74);
    target = p_table_word(MariaSequenceObjectStates);
    switch (target) {
    case K10_START:
        MariaSequenceObjectStart();
        break;
    case K10_RISE:
        MariaSequenceObjectRise();
        break;
    case K10_TURN:
        MariaSequenceObjectTurn();
        break;
    case K10_FALL:
        MariaSequenceObjectFall();
        break;
    default:
        p_jump_elsewhere(target);
        break;
    }
}

void abi_MariaSequenceObject(void)
{
    MariaSequenceObject();
}

/*
 * @routine 01E2:67F MariaSequenceObjectStart
 * @run 6E:CE7F
 * WHAT IT DOES: State 0 of kind 10: from her (facing and flip) at her place, X speed entry 3 the way it faces,
 * OBJ_WORK_N $0D, a hitbox $1E by $0C at 15 across and 3 down, live; animation 0 and its next state; frame $11.
 * HOW: MariaObjectFromPlayer, MariaObjectAtPlayerPlace, MariaSetVelocityXFromTable, MariaObjectFacingVelocity,
 * MariaObjectStartAnimation.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: A $11, N and Z from it; the rest as MariaObjectStartAnimation left it.
 */
void MariaSequenceObjectStart(void)
{
    p_ldx_object();
    pj_from_player(0xCE84);
    pj_proc(0xCE87, MariaObjectAtPlayerPlace);
    p_lda(3);
    pj_velocity_x(0xCE8C);
    pj_facing_velocity(0xCE8F);
    p_lda(SEQ_WORK_N);
    WR8(OBJ->work_n[cpu.x], SEQ_WORK_N);
    p_lda(SEQ_HALF_H);
    WR8(OBJ->hitbox_half_h[cpu.x], SEQ_HALF_H);
    p_lda(SEQ_HALF_W);
    WR8(OBJ->hitbox_half_w[cpu.x], SEQ_HALF_W);
    p_lda(SEQ_DY);
    WR8(OBJ->hitbox_dy[cpu.x], SEQ_DY);
    p_lda(SEQ_DX);
    WR8(OBJ->hitbox_dx[cpu.x], SEQ_DX);
    p_lda(1);
    WR8(OBJ->hitbox_on[cpu.x], 1);
    cpu.a = 0;
    pj_start_animation(0xCEB1);
    p_lda(SEQ_START_FRAME);
    WR8(OBJ->frame[cpu.x], SEQ_START_FRAME);
}

void abi_MariaSequenceObjectStart(void)
{
    MariaSequenceObjectStart();
}

/*
 * @routine 01E2:6B7 MariaSequenceObjectRise
 * @run 6E:CEB7
 * WHAT IT DOES: State 1 of kind 10: while she is in the sequence's state it moves; at her frame $47 it stops along X
 * and goes to its next state; either way its frame follows hers (MariaObjectFrameFromPlayer). Out of that state it is
 * cleared.
 * HOW: Her OBJ_STATE and OBJ_FRAME; MariaObjectMove, MariaObjectStopX, MariaObjectNextState, on into
 * MariaObjectFrameFromPlayer; the branch to MariaSequenceObjectClear.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as MariaObjectFrameFromPlayer, or ClearObject.
 */
void MariaSequenceObjectRise(void)
{
    uint8_t value;

    p_ldx_object();
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, STATE_SEQUENCE);
    if (value != STATE_SEQUENCE) {
        MariaSequenceObjectClear();
        return;
    }
    pj_move(0xCEC3);
    value = RD8(OBJ->frame[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, SEQ_STOP_FRAME);
    if (value == SEQ_STOP_FRAME) {
        pj_void_x(0xCECD, MariaObjectStopX);
        pj_next_state(0xCED0);
    }
    pj_frame_from_player(0);
}

void abi_MariaSequenceObjectRise(void)
{
    MariaSequenceObjectRise();
}

/*
 * @routine 01E2:6E7 MariaSequenceObjectClear
 * @run 6E:CEE7
 * WHAT IT DOES: Clears kind 10's object when she has left the sequence's state.
 * HOW: The tail JMP to ClearObject through the kernel's vector.
 * WAITS: no.
 * Entry: X the object. Exit: all registers and flags kept.
 */
void MariaSequenceObjectClear(void)
{
    ClearObject(cpu.x);
}

void abi_MariaSequenceObjectClear(void)
{
    MariaSequenceObjectClear();
}

/*
 * @routine 01E2:6EA MariaSequenceObjectTurn
 * @run 6E:CEEA
 * WHAT IT DOES: State 2 of kind 10: while she is in the sequence's state it moves with its frame following hers; at
 * her frame $46 it sets off: timer $20, X speed entry 5 the way it faces, Y speed entry 3, a fall of $0024 a frame, no
 * hitbox, and its next state. Out of that state it is cleared.
 * HOW: MariaObjectMove, a BSR to MariaObjectFrameFromPlayer; the velocity tables, MariaObjectFacingVelocity; the tail
 * JMP to MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: waiting, A her frame, N, Z and C from comparing it with $46; else as the routine
 * jumped to.
 */
void MariaSequenceObjectTurn(void)
{
    uint8_t value;

    p_ldx_object();
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, STATE_SEQUENCE);
    if (value != STATE_SEQUENCE) {
        MariaSequenceObjectClear();
        return;
    }
    pj_move(0xCEF6);
    pj_frame_from_player(0xCEF8);
    value = RD8(OBJ->frame[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, SEQ_FLY_FRAME);
    if (value != SEQ_FLY_FRAME) {
        return;
    }
    p_lda(SEQ_TIMER);
    WR8(OBJ->hurt_timer[cpu.x], SEQ_TIMER);
    p_lda(5);
    pj_velocity_x(0xCF0A);
    pj_facing_velocity(0xCF0D);
    p_lda(3);
    pj_velocity_y(0xCF12);
    p_lda(SEQ_FALL_LO);
    WR8(OBJ->fall_lo[cpu.x], SEQ_FALL_LO);
    p_lda(0);
    WR8(OBJ->fall_hi[cpu.x], 0);
    WR8(OBJ->hitbox_on[cpu.x], 0);
    pj_next_state(0);
}

void abi_MariaSequenceObjectTurn(void)
{
    MariaSequenceObjectTurn();
}

/*
 * @routine 01E2:722 MariaSequenceObjectFall
 * @run 6E:CF22
 * WHAT IT DOES: State 3 of kind 10: until its timer runs out it moves and falls, shows frame $12 and keeps its mark
 * on; then it is cleared.
 * HOW: DEC of OBJ_HURT_TIMER; MariaObjectMove, MariaObjectFallCapped, MariaObjectMarkOn; the tail JMP to ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as MariaObjectMarkOn, or ClearObject.
 */
void MariaSequenceObjectFall(void)
{
    uint8_t value;

    p_ldx_object();
    value = DEC8(OBJ->hurt_timer[cpu.x]);
    abi_nz(value);
    if (value == 0) {
        ClearObject(cpu.x);
        return;
    }
    pj_move(0xCF2C);
    pj_fall_capped(0xCF2F);
    p_lda(SEQ_LAST_FRAME);
    WR8(OBJ->frame[cpu.x], SEQ_LAST_FRAME);
    pj_mark_on(0xCF37);
}

void abi_MariaSequenceObjectFall(void)
{
    MariaSequenceObjectFall();
}
