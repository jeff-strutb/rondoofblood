/*
 * Maria's objects (her overlay, bank $6E): the objects of her item crashes. MariaCrashStart puts the crash's object in
 * slot 5 with the kind MariaCrashKinds gives the item ($0D to $13); kind $0E (14) is crash kind 1's.
 *
 * zFieldD2 is the meeting of the crash and its object: the object sets it to 1 when it reaches her (within $10 across);
 * her crash phase (MariaCrashSignal) sets it to 2; at 2 the object goes on (MariaObjectFrame24Next clears it).
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
    OBJECT_PLAYER = 0, STATE_HIT = 9, CRASH_FRAME_HELD = 0x52, MET = 1, RELEASED = 2, NEAR = 0x10, NEAR_SPAN = 0x20,
    K14_HALF_H = 0x20, K14_HALF_W = 0x10, K14_WORK_N = 0x0D, K14_SPRITE_SET = 1, K14_FRAME = 0x25, K14_VX = 8,
    K14_MIRROR = 0x68,
    K14_START = 0xD286, K14_COME = 0xD307, K14_CROSS = 0xD367, K14_GO = 0xD39A
};

#define MariaCrash1Sides_TBL MariaCrash1Sides
#define MariaCrash1Sounds_TBL MariaCrash1Sounds

/* Within $10 across of her on the first screen: zFieldD2 1, the meeting. Returns false when not (the BCS taken). */
static bool met_her(void)
{
    uint8_t value = RD8(OBJ->x_hi[cpu.x]);

    p_lda(value);
    if (value != 0) {
        return false;
    }
    p_lda(RD8(OBJ->x_lo[cpu.x]));
    abi_flag(P_C, true);
    p_sbc(RD8(OBJ->x_lo[OBJECT_PLAYER]));
    abi_flag(P_C, false);
    p_adc(NEAR);
    p_cmp(cpu.a, NEAR_SPAN);
    if (cpu.a >= NEAR_SPAN) {
        return false;
    }
    p_lda(MET);
    WR8(ZP->fieldd2, MET);
    return true;
}

/* BSR or JSR to MariaObjectFarOffScreen; cleared when it is far off the screen (the tail JMP to ClearObject). */
static void clear_if_far(uint16_t ret)
{
    pj_far_off_screen(ret);
    if (!p_z()) {
        ClearObject(cpu.x);
    }
}

/*
 * @routine 01E3:273 MariaCrash1Object
 * @run 6E:D273
 * WHAT IT DOES: The handler of kind 14, crash kind 1's object: its state's routine from MariaCrash1States (start, come
 * to her, cross twice, go).
 * HOW: MariaStateDoubled, then the JMP (table,X).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
 */
void MariaCrash1Object(void)
{
    uint16_t target;

    pj_state_doubled(0xD276);
    target = p_table_word(MariaCrash1States);
    switch (target) {
    case K14_START:
        MariaCrash1Start();
        break;
    case K14_COME:
        MariaCrash1Come();
        break;
    case K14_CROSS:
        MariaCrash1Cross();
        break;
    case K14_GO:
        MariaCrash1Go();
        break;
    default:
        p_jump_elsewhere(target);
        break;
    }
}

void abi_MariaCrash1Object(void)
{
    MariaCrash1Object();
}

/*
 * @routine 01E3:283 MariaCrash1Clear
 * @run 6E:D283
 * WHAT IT DOES: Clears crash kind 1's object when she is hit or below the screen.
 * HOW: The tail JMP to ClearObject through the kernel's vector.
 * WAITS: no.
 * Entry: X the object. Exit: all registers and flags kept.
 */
void MariaCrash1Clear(void)
{
    ClearObject(cpu.x);
}

void abi_MariaCrash1Clear(void)
{
    MariaCrash1Clear();
}

/*
 * @routine 01E3:286 MariaCrash1Start
 * @run 6E:D286
 * WHAT IT DOES: State 0 of crash kind 1's object: when she is hit or below the screen it is cleared; it waits for her
 * crash's frame $52, then starts script 0 in slot 3, a hitbox $40 by $20, live, OBJ_WORK_N $0D, takes her facing and
 * her Y (kept in OBJ_WORK_D) and her X (kept in OBJ_WORK_E), sprite set 1, frame $25, and enters from the side.
 * HOW: Her OBJ_STATE, OBJ_Y_HI and OBJ_FRAME; MariaStartScriptSlot3, MariaObjectFromPlayer; on into MariaCrash1Enter.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: waiting, A her frame, N, Z and C from comparing it with $52; else as
 * MariaCrash1Clear or MariaCrash1Enter.
 */
void MariaCrash1Start(void)
{
    uint8_t value;

    p_ldx_object();
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, STATE_HIT);
    if (value == STATE_HIT) {
        MariaCrash1Clear();
        return;
    }
    value = RD8(OBJ->y_hi[OBJECT_PLAYER]);
    p_lda((uint8_t)(value - 1));
    if (cpu.a == 0) {
        MariaCrash1Clear();
        return;
    }
    value = RD8(OBJ->frame[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, CRASH_FRAME_HELD);
    if (value != CRASH_FRAME_HELD) {
        MariaCrash1Wait();
        return;
    }
    cpu.a = 0;
    pj_script_slot3(0xD2A0);
    p_lda(K14_HALF_H);
    WR8(OBJ->hitbox_half_h[cpu.x], K14_HALF_H);
    p_lda(K14_HALF_W);
    WR8(OBJ->hitbox_half_w[cpu.x], K14_HALF_W);
    p_lda(0);
    WR8(OBJ->hitbox_dx[cpu.x], 0);
    p_lda(0);
    WR8(OBJ->hitbox_dy[cpu.x], 0);
    p_lda(1);
    WR8(OBJ->hitbox_on[cpu.x], 1);
    p_lda(K14_WORK_N);
    WR8(OBJ->work_n[cpu.x], K14_WORK_N);
    pj_from_player(0xD2C1);
    p_lda(RD8(OBJ->y_lo[OBJECT_PLAYER]));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    WR8(OBJ->work_d[cpu.x], cpu.a);
    p_lda(RD8(OBJ->x_lo[OBJECT_PLAYER]));
    WR8(OBJ->work_e[cpu.x], cpu.a);
    p_lda(K14_SPRITE_SET);
    WR8(OBJ->sprite_set[cpu.x], K14_SPRITE_SET);
    p_lda(K14_FRAME);
    WR8(OBJ->frame[cpu.x], K14_FRAME);
    MariaCrash1Enter();
}

void abi_MariaCrash1Start(void)
{
    MariaCrash1Start();
}

/*
 * @routine 01E3:2DA MariaCrash1Enter
 * @run 6E:D2DA
 * WHAT IT DOES: Crash kind 1's object enters from the side it faces away from: its X at that edge (MariaCrash1Sides),
 * its next state, X speed entry 8 the way it faces, and its sound by the way it moves (MariaCrash1Sounds: $39 right,
 * $38 left).
 * HOW: The facing doubled; MariaObjectNextState, the velocity table, MariaObjectFacingVelocity; bit 7 of the X speed
 * rolled into bit 0; the tail JMP to SoundCommand.
 * WAITS: no.
 * Entry: X the object. Exit: as SoundCommand.
 */
void MariaCrash1Enter(void)
{
    uint8_t index;

    p_lda(RD8(OBJ->facing_left[cpu.x]));
    p_asl();
    p_ldy(cpu.a);
    index = cpu.y;
    p_lda(TBL8(MariaCrash1Sides, index));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    p_lda(TBL8(MariaCrash1Sides, index + 1));
    WR8(OBJ->x_hi[cpu.x], cpu.a);
    pj_next_state(0xD2EE);
    p_lda(K14_VX);
    pj_velocity_x(0xD2F3);
    pj_facing_velocity(0xD2F6);
    p_lda(RD8(OBJ->vx_hi[cpu.x]));
    p_asl();
    {
        bool c = p_c();
        bool out = (cpu.a & 0x80) != 0;

        p_lda((uint8_t)((cpu.a << 1) | (c ? 1 : 0)));
        abi_flag(P_C, out);
    }
    p_lda((uint8_t)(cpu.a & 0x01));
    p_ldy(cpu.a);
    p_lda(TBL8(MariaCrash1Sounds, cpu.y));
    pj_sound(0);
}

void abi_MariaCrash1Enter(void)
{
    MariaCrash1Enter();
}

/*
 * @routine 01E3:304 MariaCrash1Wait
 * @run 6E:D304
 * WHAT IT DOES: Crash kind 1's object waiting for her crash's frame: nothing more this frame.
 * HOW: RTS (the branch from MariaCrash1Start).
 * WAITS: no.
 * Entry, exit: all registers and flags kept.
 */
void MariaCrash1Wait(void)
{
}

void abi_MariaCrash1Wait(void)
{
    MariaCrash1Wait();
}

/*
 * @routine 01E3:307 MariaCrash1Come
 * @run 6E:D307
 * WHAT IT DOES: State 1 of crash kind 1's object: it moves toward her; when she is hit or below the screen, or it is on
 * another screen or not near her, it is cleared once far off the screen; at the meeting released (zFieldD2 2) it goes
 * on (MariaObjectFrame24Next); within $10 of her, it marks the meeting (zFieldD2 1).
 * HOW: MariaObjectMove; her OBJ_STATE and OBJ_Y_HI; zFieldD2; the X difference plus $10 against $20; a BSR to
 * MariaObjectFarOffScreen and the tail JMP to ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: met, A 1; released, as MariaObjectFrame24Next; else as
 * MariaObjectFarOffScreen, or ClearObject.
 */
void MariaCrash1Come(void)
{
    uint8_t value;

    p_ldx_object();
    pj_move(0xD30C);
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
                pj_frame24_next(0);
                return;
            }
            if (met_her()) {
                return;
            }
        }
    }
    clear_if_far(0xD339);
}

void abi_MariaCrash1Come(void)
{
    MariaCrash1Come();
}

/*
 * @routine 01E3:367 MariaCrash1Cross
 * @run 6E:D367
 * WHAT IT DOES: States 2 and 3 of crash kind 1's object: its frame follows her frame (MariaObjectFrameByPlayerFrame);
 * it moves; once far off the screen it turns: its Y mirrored about $68, its facing the other way, and it enters again
 * (MariaCrash1Enter).
 * HOW: BSRs to MariaObjectFrameByPlayerFrame and MariaObjectFarOffScreen; MariaObjectMove; the JMP to MariaCrash1Enter.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: on screen, as MariaObjectFarOffScreen; else as MariaCrash1Enter.
 */
void MariaCrash1Cross(void)
{
    uint8_t value;

    p_ldx_object();
    pj_frame_by_player_frame(0xD36B);
    pj_move(0xD36E);
    pj_far_off_screen(0xD370);
    if (p_z()) {
        return;
    }
    p_lda(K14_MIRROR);
    abi_flag(P_C, true);
    p_sbc(RD8(OBJ->y_lo[cpu.x]));
    abi_flag(P_C, false);
    p_adc(K14_MIRROR);
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    value = RD8(OBJ->facing_left[cpu.x]);
    p_lda((uint8_t)(value ^ 0xFF));
    p_lda((uint8_t)(cpu.a & 0x01));
    WR8(OBJ->facing_left[cpu.x], cpu.a);
    MariaCrash1Enter();
}

void abi_MariaCrash1Cross(void)
{
    MariaCrash1Cross();
}

/*
 * @routine 01E3:39A MariaCrash1Go
 * @run 6E:D39A
 * WHAT IT DOES: State 4 of crash kind 1's object: it moves with frame $25; until the meeting it follows her frame and,
 * within $10 of her on the first screen, marks the meeting (zFieldD2 1) and shows frame $25 again; otherwise it is
 * cleared once far off the screen (MariaCrash1Leave).
 * HOW: MariaObjectMove, BSRs to MariaObjectFrame25 and MariaObjectFrameByPlayerFrame; the X difference plus $10 against
 * $20; on into MariaObjectFrame25, or the branch to MariaCrash1Leave.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: met, as MariaObjectFrame25; else as MariaCrash1Leave.
 */
void MariaCrash1Go(void)
{
    uint8_t value;

    p_ldx_object();
    pj_move(0xD39F);
    pj_frame25(0xD3A1);
    value = RD8(ZP->fieldd2);
    p_lda(value);
    if (value == 0) {
        pj_frame_by_player_frame(0xD3A7);
        if (met_her()) {
            pj_frame25(0);
            return;
        }
    }
    MariaCrash1Leave();
}

void abi_MariaCrash1Go(void)
{
    MariaCrash1Go();
}

/*
 * @routine 01E3:3C4 MariaCrash1Leave
 * @run 6E:D3C4
 * WHAT IT DOES: Clears crash kind 1's object once it is far off the screen.
 * HOW: MariaObjectFarOffScreen's Z; the tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: as MariaObjectFarOffScreen, or ClearObject.
 */
void MariaCrash1Leave(void)
{
    clear_if_far(0xD3C7);
}

void abi_MariaCrash1Leave(void)
{
    MariaCrash1Leave();
}
