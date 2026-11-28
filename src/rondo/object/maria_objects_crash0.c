/*
 * Maria's objects (her overlay, bank $6E): kind 13, crash kind 0's object ($0D, the first of MariaCrashKinds' kinds),
 * and kind 21, the pieces it drops. It rises from her at her crash's frame $52 with raster stage 2 (the stage it
 * found kept in PlayerKeptRasterStage), meets her (zFieldD2), hovers, rains pieces into slots 6 to 14 for a while,
 * and leaves upward, putting the raster stage back.
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
    OBJECT_PLAYER = 0, STATE_HIT = 9, CRASH_FRAME_HELD = 0x52, MET = 1, RELEASED = 2, BELOW_SCREEN = 1, ABOVE = 0xFF,
    K13_SCRIPT = 3, K13_RASTER = 2, K13_HALF = 0x20, K13_DY = 0x0C, K13_WORK_N = 0x0D, K13_SPRITE_SET = 1,
    K13_FRAME = 0x23, K13_VY = 8, K13_NEAR = 8, K13_NEAR_SPAN = 0x10, K13_LOW_Y = 0xC0, K13_HOVER_FRAME = 0x22,
    K13_HOVER_VY = 9, K13_HOVER_TIME = 0x20, K13_RAIN_TIME = 0x80, K13_RAIN_SOUND = 0x34, K13_SCRIPT_EVERY = 0x1F,
    K13_LEAVE_VY = 0x0A,
    PIECE_FIRST_SLOT = 6, KIND_PIECE = 0x15, PIECE_SPRITE_SET = 1, PIECE_FRAME = 0x31, PIECE_VX = 4, PIECE_VY = 0x0B,
    PIECE_WORK_N = 0x0B, PIECE_PLACES = 0x0F, PIECE_SLOTS = 9,
    P21_HALF_H = 0x60, P21_HALF_W = 0x70, P21_DY = 0x0C,
    K13_START = 0xD3E6, K13_RISE = 0xD442, K13_HOVER = 0xD498, K13_RAIN = 0xD4C0, K13_LEAVE = 0xD4EC
};

#define MariaCrash0States_TBL MariaCrash0States
#define MariaCrash0PieceFrames_TBL MariaCrash0PieceFrames
#define MariaCrash0PiecePlaces_TBL MariaCrash0PiecePlaces

/*
 * @routine 01E3:3D3 MariaCrash0Object
 * @run 6E:D3D3
 * WHAT IT DOES: The handler of kind 13, crash kind 0's object: its state's routine from MariaCrash0States (start,
 * rise, hover, rain, leave).
 * HOW: MariaStateDoubled, then the JMP (table,X).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
 */
void MariaCrash0Object(void)
{
    uint16_t target;

    pj_state_doubled(0xD3D6);
    target = p_table_word(MariaCrash0States);
    switch (target) {
    case K13_START:
        MariaCrash0Start();
        break;
    case K13_RISE:
        MariaCrash0Rise();
        break;
    case K13_HOVER:
        MariaCrash0Hover();
        break;
    case K13_RAIN:
        MariaCrash0Rain();
        break;
    case K13_LEAVE:
        MariaCrash0Leave();
        break;
    default:
        p_jump_elsewhere(target);
        break;
    }
}

void abi_MariaCrash0Object(void)
{
    MariaCrash0Object();
}

/*
 * @routine 01E3:3E3 MariaCrash0Clear
 * @run 6E:D3E3
 * WHAT IT DOES: Clears crash kind 0's object when she is hit or below the screen before it starts.
 * HOW: The tail JMP to ClearObject through the kernel's vector.
 * WAITS: no.
 * Entry: X the object. Exit: all registers and flags kept.
 */
void MariaCrash0Clear(void)
{
    ClearObject(cpu.x);
}

void abi_MariaCrash0Clear(void)
{
    MariaCrash0Clear();
}

/*
 * @routine 01E3:3E6 MariaCrash0Start
 * @run 6E:D3E6
 * WHAT IT DOES: State 0 of crash kind 0's object: cleared when she is hit or below the screen; at her crash's frame $52
 * it starts script 3 in slot 3, keeps the raster stage (PlayerKeptRasterStage) and sets stage 2, takes a hitbox $40
 * square at $20 across and $0C down, live, OBJ_WORK_N $0D, her facing and X, sprite set 1, frame $23, Y speed entry 8,
 * and its next state.
 * HOW: Her OBJ_STATE, OBJ_Y_HI and OBJ_FRAME; MariaStartScriptSlot3, MariaObjectFromPlayer, MariaSetVelocityYFromTable;
 * the tail JMP to MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: waiting, A her frame, N, Z and C from comparing it with $52; else as the
 * routine jumped to.
 */
void MariaCrash0Start(void)
{
    uint8_t value;

    p_ldx_object();
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, STATE_HIT);
    if (value == STATE_HIT) {
        MariaCrash0Clear();
        return;
    }
    value = RD8(OBJ->y_hi[OBJECT_PLAYER]);
    p_lda((uint8_t)(value - 1));
    if (cpu.a == 0) {
        MariaCrash0Clear();
        return;
    }
    value = RD8(OBJ->frame[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, CRASH_FRAME_HELD);
    if (value != CRASH_FRAME_HELD) {
        return;
    }
    p_lda(K13_SCRIPT);
    pj_script_slot3(0xD401);
    p_lda(RD8(ZP->raster_stage));
    WR8(PVARS->kept_raster_stage, cpu.a);
    p_lda(K13_RASTER);
    WR8(ZP->raster_stage, K13_RASTER);
    p_lda(K13_HALF);
    WR8(OBJ->hitbox_half_h[cpu.x], K13_HALF);
    p_lda(K13_HALF);
    WR8(OBJ->hitbox_half_w[cpu.x], K13_HALF);
    WR8(OBJ->hitbox_dx[cpu.x], K13_HALF);
    p_lda(K13_DY);
    WR8(OBJ->hitbox_dy[cpu.x], K13_DY);
    p_lda(1);
    WR8(OBJ->hitbox_on[cpu.x], 1);
    p_lda(K13_WORK_N);
    WR8(OBJ->work_n[cpu.x], K13_WORK_N);
    pj_from_player(0xD429);
    p_lda(RD8(OBJ->x_lo[OBJECT_PLAYER]));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    p_lda(K13_SPRITE_SET);
    WR8(OBJ->sprite_set[cpu.x], K13_SPRITE_SET);
    p_lda(K13_FRAME);
    WR8(OBJ->frame[cpu.x], K13_FRAME);
    p_lda(K13_VY);
    pj_velocity_y(0xD43E);
    pj_next_state(0);
}

void abi_MariaCrash0Start(void)
{
    MariaCrash0Start();
}

/*
 * @routine 01E3:442 MariaCrash0Rise
 * @run 6E:D442
 * WHAT IT DOES: State 1 of crash kind 0's object: it moves; released at the meeting (zFieldD2 2), it hovers: frame $22,
 * Y speed entry 9, $20 frames, its next state. Within 8 rows of her on the first screen it marks the meeting; when she
 * is hit or below the screen, or it is not near her, it is cleared once it is high on the screen below (Y under $C0 on
 * screen 1).
 * HOW: MariaObjectApplyVelocity; her OBJ_STATE and OBJ_Y_HI; zFieldD2; the Y difference plus 8 against $10; the tail
 * JMPs to MariaObjectNextState and ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last test, or the routine jumped to.
 */
void MariaCrash0Rise(void)
{
    uint8_t value;

    p_ldx_object();
    pj_apply_velocity(0xD447);
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, STATE_HIT);
    if (value != STATE_HIT) {
        value = RD8(OBJ->y_hi[OBJECT_PLAYER]);
        p_lda((uint8_t)(value - 1));
        if (cpu.a != 0) {
            value = RD8(ZP->fieldd2);
            p_lda(value);
            p_cmp(value, RELEASED);
            if (value == RELEASED) {
                p_lda(K13_HOVER_FRAME);
                WR8(OBJ->frame[cpu.x], K13_HOVER_FRAME);
                WR8(ZP->fieldd2, 0);
                p_lda(K13_HOVER_VY);
                pj_velocity_y(0xD490);
                p_lda(K13_HOVER_TIME);
                WR8(OBJ->anim_timer[cpu.x], K13_HOVER_TIME);
                pj_next_state(0);
                return;
            }
            value = RD8(OBJ->y_hi[cpu.x]);
            p_lda(value);
            if (value == 0) {
                p_lda(RD8(OBJ->y_lo[cpu.x]));
                abi_flag(P_C, true);
                p_sbc(RD8(OBJ->y_lo[OBJECT_PLAYER]));
                abi_flag(P_C, false);
                p_adc(K13_NEAR);
                p_cmp(cpu.a, K13_NEAR_SPAN);
                if (cpu.a < K13_NEAR_SPAN) {
                    p_lda(MET);
                    WR8(ZP->fieldd2, MET);
                    return;
                }
            }
        }
    }
    value = RD8(OBJ->y_hi[cpu.x]);
    p_lda(value);
    p_cmp(value, BELOW_SCREEN);
    if (value != BELOW_SCREEN) {
        return;
    }
    value = RD8(OBJ->y_lo[cpu.x]);
    p_lda(value);
    p_cmp(value, K13_LOW_Y);
    if (value >= K13_LOW_Y) {
        return;
    }
    ClearObject(cpu.x);
}

void abi_MariaCrash0Rise(void)
{
    MariaCrash0Rise();
}

/*
 * @routine 01E3:498 MariaCrash0Hover
 * @run 6E:D498
 * WHAT IT DOES: State 2 of crash kind 0's object: it moves, no higher than the top of the map; when its timer runs
 * out it stops, sets $80 frames of rain with sound $34, and goes to its next state.
 * HOW: MariaObjectApplyVelocity; DEC of OBJ_ANIM_TIMER; MariaObjectStop, SoundCommand, the tail JMP to
 * MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the timer's DEC, or MariaObjectNextState.
 */
void MariaCrash0Hover(void)
{
    uint8_t value;

    p_ldx_object();
    pj_apply_velocity(0xD49D);
    value = RD8(OBJ->y_hi[cpu.x]);
    p_lda(value);
    p_cmp(value, ABOVE);
    if (value == ABOVE) {
        WR8(OBJ->y_lo[cpu.x], 0);
        WR8(OBJ->y_hi[cpu.x], 0);
    }
    value = DEC8(OBJ->anim_timer[cpu.x]);
    abi_nz(value);
    if (value != 0) {
        return;
    }
    pj_void_x(0xD4B3, MariaObjectStop);
    p_lda(K13_RAIN_TIME);
    WR8(OBJ->anim_timer[cpu.x], K13_RAIN_TIME);
    p_lda(K13_RAIN_SOUND);
    pj_sound(0xD4BD);
    pj_next_state(0);
}

void abi_MariaCrash0Hover(void)
{
    MariaCrash0Hover();
}

/*
 * @routine 01E3:4C0 MariaCrash0Rain
 * @run 6E:D4C0
 * WHAT IT DOES: State 3 of crash kind 0's object: it moves; every $20 frames script 3 starts again; each frame a piece
 * may drop (MariaCrash0Drop). When the rain's timer runs out it rises (Y speed entry $0A), marks the meeting again
 * (zFieldD2 1), shows frame $23 and goes to its next state.
 * HOW: MariaObjectApplyVelocity; DEC of OBJ_ANIM_TIMER and its low five bits; MariaStartScriptSlot3; the JMP to
 * MariaCrash0Drop; the tail JMP to MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as MariaCrash0Drop, or MariaObjectNextState.
 */
void MariaCrash0Rain(void)
{
    uint8_t value;

    p_ldx_object();
    pj_apply_velocity(0xD4C5);
    value = DEC8(OBJ->anim_timer[cpu.x]);
    abi_nz(value);
    if (value == 0) {
        p_lda(K13_LEAVE_VY);
        pj_velocity_y(0xD4E0);
        p_lda(MET);
        WR8(ZP->fieldd2, MET);
        p_lda(K13_FRAME);
        WR8(OBJ->frame[cpu.x], K13_FRAME);
        pj_next_state(0);
        return;
    }
    p_lda((uint8_t)(RD8(OBJ->anim_timer[cpu.x]) & K13_SCRIPT_EVERY));
    p_cmp(cpu.a, K13_SCRIPT_EVERY);
    if (cpu.a == K13_SCRIPT_EVERY) {
        p_lda(K13_SCRIPT);
        pj_script_slot3(0xD4D8);
    }
    MariaCrash0Drop();
}

void abi_MariaCrash0Rain(void)
{
    MariaCrash0Rain();
}

/*
 * @routine 01E3:4EC MariaCrash0Leave
 * @run 6E:D4EC
 * WHAT IT DOES: State 4 of crash kind 0's object: it moves; above the top of the map it puts the raster stage back
 * (PlayerKeptRasterStage) and is cleared.
 * HOW: MariaObjectMove; the tail JMP to ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the compare, or ClearObject.
 */
void MariaCrash0Leave(void)
{
    uint8_t value;

    p_ldx_object();
    pj_move(0xD4F1);
    value = RD8(OBJ->y_hi[cpu.x]);
    p_lda(value);
    p_cmp(value, ABOVE);
    if (value != ABOVE) {
        return;
    }
    p_lda(RD8(PVARS->kept_raster_stage));
    WR8(ZP->raster_stage, cpu.a);
    ClearObject(cpu.x);
}

void abi_MariaCrash0Leave(void)
{
    MariaCrash0Leave();
}

/*
 * @routine 01E3:501 MariaCrash0Drop
 * @run 6E:D501
 * WHAT IT DOES: On even frames, drops a piece of kind 21 into the next of slots 6 to 14 (OBJ_PIECE_NEXT) if it is
 * empty: at the place OBJ_PIECE_PLACE picks in MariaCrash0PiecePlaces (Y, and X plus the frame count's low three
 * bits), sprite set 1, frame $31, X speed entry 4 the way it faces, Y speed entry $0B, OBJ_WORK_N $0B. The place
 * steps on through sixteen, the slot through nine.
 * HOW: zWaveCounter; the piece's slot in X by SXY while the object's is in Y and on the stack; the velocity tables and
 * MariaObjectFacingVelocity for the piece.
 * WAITS: no.
 * Entry: X the object. Exit: odd frame, A 1, Z clear; slot taken, A its kind, Z clear; else A the next slot, N, Z
 * and C from comparing it with 9 (cleared at 9). Y as the last step left it.
 */
void MariaCrash0Drop(void)
{
    uint8_t slot, place;

    p_lda((uint8_t)(RD8(ZP->wave_counter) & 0x01));
    if (cpu.a != 0) {
        return;
    }
    p_lda(RD8(OBJ->piece_next[cpu.x]));
    abi_flag(P_C, false);
    p_adc(PIECE_FIRST_SLOT);
    p_ldy(cpu.a);
    slot = cpu.y;
    p_lda(RD8(OBJ->kind[slot]));
    if (cpu.a != 0) {
        return;
    }
    p_lda(KIND_PIECE);
    WR8(OBJ->kind[slot], KIND_PIECE);
    p_lda(RD8(OBJ->piece_place[cpu.x]));
    push8(cpu.x);
    {
        uint8_t t = cpu.x;

        cpu.x = cpu.y;
        cpu.y = t;
    }
    p_asl();
    p_asl();
    p_ldy(cpu.a);
    place = cpu.y;
    p_lda(TBL8(MariaCrash0PiecePlaces, place));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    p_lda(TBL8(MariaCrash0PiecePlaces, place + 1));
    WR8(OBJ->y_hi[cpu.x], cpu.a);
    p_lda(TBL8(MariaCrash0PiecePlaces, place + 2));
    WR8(ZP->temp02, cpu.a);
    p_lda(TBL8(MariaCrash0PiecePlaces, place + 3));
    WR8(ZP->temp03, cpu.a);
    p_lda((uint8_t)(RD8(ZP->wave_counter) & 0x07));
    abi_flag(P_C, false);
    p_adc(RD8(ZP->temp02));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    p_lda(RD8(ZP->temp03));
    WR8(OBJ->x_hi[cpu.x], cpu.a);
    p_lda(PIECE_SPRITE_SET);
    WR8(OBJ->sprite_set[cpu.x], PIECE_SPRITE_SET);
    p_lda(PIECE_FRAME);
    WR8(OBJ->frame[cpu.x], PIECE_FRAME);
    p_lda(PIECE_VX);
    pj_velocity_x(0xD556);
    pj_facing_velocity(0xD559);
    p_lda(PIECE_VY);
    pj_velocity_y(0xD55E);
    p_lda(PIECE_WORK_N);
    WR8(OBJ->work_n[cpu.x], PIECE_WORK_N);
    p_ldx(pull8());
    p_lda((uint8_t)((RD8(OBJ->piece_place[cpu.x]) + 1) & PIECE_PLACES));
    WR8(OBJ->piece_place[cpu.x], cpu.a);
    p_lda((uint8_t)(RD8(OBJ->piece_next[cpu.x]) + 1));
    p_cmp(cpu.a, PIECE_SLOTS);
    WR8(OBJ->piece_next[cpu.x], cpu.a);
    if (p_c()) {
        WR8(OBJ->piece_next[cpu.x], 0);
    }
}

void abi_MariaCrash0Drop(void)
{
    MariaCrash0Drop();
}

/*
 * @routine 01E3:5C0 MariaCrash0Piece
 * @run 6E:D5C0
 * WHAT IT DOES: The handler of kind 21, a piece crash kind 0's object drops: a hitbox $C0 by $E0 at $70 across and $0C
 * down, its frame by the frame count and its slot's low bit (MariaCrash0PieceFrames); it moves; below the screen it is
 * cleared; its hitbox is live one frame in eight.
 * HOW: The stores, MariaObjectMove, zWaveCounter; INC of OBJ_HITBOX_ON; the tail JMP to ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: A the frame count's low three bits, N and Z from them; at the eighth, as the
 * INC; or as ClearObject.
 */
void MariaCrash0Piece(void)
{
    uint8_t value;

    p_ldx_object();
    p_lda(P21_HALF_H);
    WR8(OBJ->hitbox_half_h[cpu.x], P21_HALF_H);
    p_lda(P21_HALF_W);
    WR8(OBJ->hitbox_half_w[cpu.x], P21_HALF_W);
    WR8(OBJ->hitbox_dx[cpu.x], P21_HALF_W);
    p_lda(P21_DY);
    WR8(OBJ->hitbox_dy[cpu.x], P21_DY);
    p_lda(cpu.x);
    p_lda((uint8_t)(cpu.a & 0x01));
    p_lda((uint8_t)(cpu.a ^ RD8(ZP->wave_counter)));
    p_lda((uint8_t)(cpu.a & 0x03));
    p_ldy(cpu.a);
    p_lda(TBL8(MariaCrash0PieceFrames, cpu.y));
    WR8(OBJ->frame[cpu.x], cpu.a);
    pj_move(0xD5E5);
    value = RD8(OBJ->y_hi[cpu.x]);
    p_lda(value);
    p_cmp(value, BELOW_SCREEN);
    if (value == BELOW_SCREEN) {
        ClearObject(cpu.x);
        return;
    }
    WR8(OBJ->hitbox_on[cpu.x], 0);
    p_lda((uint8_t)(RD8(ZP->wave_counter) & 0x07));
    if (cpu.a != 0) {
        return;
    }
    abi_nz(INC8(OBJ->hitbox_on[cpu.x]));
}

void abi_MariaCrash0Piece(void)
{
    MariaCrash0Piece();
}
