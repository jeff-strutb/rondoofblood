/*
 * Maria's objects (her overlay, bank $6E): kind 17, which holds at her place by her frame and, at frame $52, gives
 * slots 6 to 14 kind 23 at its own place once (OBJ_WORK_D its flag); kind 23, nine objects flung from there by slot,
 * held until her frame $52 and then thrown off; and kind 24, four objects flung from her and cleared when their
 * animation ends.
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
    OBJECT_PLAYER = 0, STATE_HIT = 9,
    K17_FRAME_AT = 0x50, K17_HOLD_A = 0x51, K17_SPAWN_AT = 0x52, K17_FRAME = 0x2D, K17_DX = 9, K17_DY = 0xEF,
    K17_HELD_DX = 7, K17_HELD_DY = 0xE9, K17_SPAWN_KIND = 0x17, K17_SLOTS = 9, K17_SPAWN_FRAME = 0x30,
    K23_FIRST = 6, K23_FALL_LO = 6, K23_LAST_INDEX = 8, K23_ATTR = 0x85, K23_HELD_FRAME = 0x52, K23_VX_ENTRY = 0,
    K23_VY_ENTRY = 0x10, K23_THROWN_FALL_LO = 0xF0, K23_THROWN_FALL_HI = 0xFF, EDGE_LEFT = 0x10, EDGE_RIGHT = 0xF0,
    K24_FIRST = 0x0B, K24_DX = 0xFC, K24_DY = 0x17, K24_FALL_LO = 0x22, K24_ANIMATION = 0x14
};

#define MariaKind23SpeedsYLo_TBL MariaKind23SpeedsYLo
#define MariaKind23SpeedsYHi_TBL MariaKind23SpeedsYHi
#define MariaKind23SpeedsXLo_TBL MariaKind23SpeedsXLo
#define MariaKind23SpeedsXHi_TBL MariaKind23SpeedsXHi
#define MariaKind24Speeds_TBL MariaKind24Speeds

#define P_PUT(field, value) do { p_lda(value); WR8(field, cpu.a); } while (0)

/* TXA, SEC, SBC #first: the object's slot less `first` in A. */
static void p_slot_less(uint8_t first)
{
    p_lda(cpu.x);
    abi_flag(P_C, true);
    p_sbc(first);
}

/* zTemp04 and zTemp06 the offsets, MariaObjectAtPlayer: a JSR returning to `ret`. */
static void at_player(uint8_t dx, uint8_t dy, uint16_t ret)
{
    P_PUT(ZP->temp04, dx);
    P_PUT(ZP->temp06, dy);
    pj_at_player(ret);
}

/*
 * @routine 01E4:3DD MariaKind17
 * @run 6E:DBDD
 * WHAT IT DOES: The handler of kind 17. State 0: cleared when she is hit; at her frame $50 its next state, sprite set
 * 1, frame $2D, her flip and facing, her place plus (9, -$11). After: at her frame $50 the same place; at $51 her place
 * plus (7, -$17); at $52 the first time slots 6 to 14 become kind $17 at its place, then frame $30 and that place; at
 * any other frame it is cleared.
 * HOW: Her OBJ_STATE and OBJ_FRAME; MariaObjectNextState, MariaObjectFromPlayer, MariaObjectAtPlayer; OBJ_WORK_D as the
 * spawn's flag; ClearObject by the tail JMP.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last step made.
 */
void MariaKind17(void)
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
        p_cmp(cpu.a, K17_FRAME_AT);
        if (!p_z()) {
            return;
        }
        pj_next_state(0xDBF9);
        P_PUT(OBJ->sprite_set[cpu.x], 1);
        P_PUT(OBJ->frame[cpu.x], K17_FRAME);
    } else {
        p_lda(RD8(OBJ->frame[OBJECT_PLAYER]));
        p_cmp(cpu.a, K17_FRAME_AT);
        if (!p_z()) {
            p_cmp(cpu.a, K17_HOLD_A);
            if (p_z()) {
                at_player(K17_HELD_DX, K17_HELD_DY, 0xDC2F);
                pj_from_player(0);
                return;
            }
            p_cmp(cpu.a, K17_SPAWN_AT);
            if (!p_z()) {
                pj_clear_object(0);
                return;
            }
            p_lda(RD8(OBJ->work_d[cpu.x]));
            if (p_z()) {
                cpu.y = 0;
                do {
                    P_PUT(OBJ->kind[K23_FIRST + cpu.y], K17_SPAWN_KIND);
                    p_lda(RD8(OBJ->y_lo[cpu.x]));
                    WR8(OBJ->y_lo[K23_FIRST + cpu.y], cpu.a);
                    p_lda(RD8(OBJ->x_lo[cpu.x]));
                    WR8(OBJ->x_lo[K23_FIRST + cpu.y], cpu.a);
                    p_ldy((uint8_t)(cpu.y + 1));
                    p_cmp(cpu.y, K17_SLOTS);
                } while (!p_z());
                abi_nz(INC8(OBJ->work_d[cpu.x]));
            }
            P_PUT(OBJ->frame[cpu.x], K17_SPAWN_FRAME);
            at_player(K17_HELD_DX, K17_HELD_DY, 0xDC61);
            pj_from_player(0);
            return;
        }
    }
    pj_from_player(0xDC06);
    at_player(K17_DX, K17_DY, 0xDC11);
}

void abi_MariaKind17(void)
{
    MariaKind17();
}

/*
 * @routine 01E4:464 MariaKind23
 * @run 6E:DC64
 * WHAT IT DOES: The handler of kind 23. State 0: her flip and facing, a fall of $0006 a frame, its speeds by slot
 * (MariaKind23SpeedsY and X), slot 14 with sprite attribute $85, then its hitbox as item 6's pieces have it. State 1:
 * its animation, move and fall; once her frame is past $52, no Y speed, X speed entry 0 and Y entry $10 the way it
 * faces, a fall of $FFF0, and its next state. Later: the same, and cleared once on the next screen between $10 and $F0.
 * HOW: MariaObjectFromPlayer, the tables by slot less 6, the tail JMP to MariaItem6PieceHitbox; MariaObjectAnimationTick,
 * MariaObjectMove, MariaObjectFallCapped; MariaObjectStopY, the velocity tables, MariaObjectFacingVelocity,
 * MariaObjectNextState; ClearObject by the tail JMP.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last step made.
 */
void MariaKind23(void)
{
    uint8_t index;

    p_ldx_object();
    p_lda(RD8(OBJ->state[cpu.x]));
    if (p_z()) {
        pj_from_player(0xDC6E);
        p_slot_less(K23_FIRST);
        p_ldy(cpu.a);
        index = cpu.y;
        P_PUT(OBJ->fall_lo[cpu.x], K23_FALL_LO);
        P_PUT(OBJ->fall_hi[cpu.x], 0);
        P_PUT(OBJ->vy_lo[cpu.x], TBL8(MariaKind23SpeedsYLo, index));
        P_PUT(OBJ->vy_hi[cpu.x], TBL8(MariaKind23SpeedsYHi, index));
        P_PUT(OBJ->vx_lo[cpu.x], TBL8(MariaKind23SpeedsXLo, index));
        P_PUT(OBJ->vx_hi[cpu.x], TBL8(MariaKind23SpeedsXHi, index));
        p_cmp(cpu.y, K23_LAST_INDEX);
        if (p_z()) {
            P_PUT(OBJ->sprite_attr[cpu.x], K23_ATTR);
        }
        MariaItem6PieceHitbox();
        return;
    }
    p_lda((uint8_t)(cpu.a - 1));
    if (p_z()) {
        pj_animation_tick(0xDCA7);
        pj_move(0xDCAA);
        pj_fall_capped(0xDCAD);
        p_lda(RD8(OBJ->frame[OBJECT_PLAYER]));
        p_cmp(cpu.a, K23_HELD_FRAME);
        if (p_z()) {
            return;
        }
        pj_void_x(0xDCB8, MariaObjectStopY);
        p_lda(K23_VX_ENTRY);
        pj_velocity_x(0xDCBD);
        pj_facing_velocity(0xDCC0);
        p_lda(K23_VY_ENTRY);
        pj_velocity_y(0xDCC5);
        P_PUT(OBJ->fall_lo[cpu.x], K23_THROWN_FALL_LO);
        P_PUT(OBJ->fall_hi[cpu.x], K23_THROWN_FALL_HI);
        pj_next_state(0);
        return;
    }
    pj_animation_tick(0xDCD5);
    pj_move(0xDCD8);
    pj_fall_capped(0xDCDB);
    p_lda(RD8(OBJ->x_hi[cpu.x]));
    if (p_z()) {
        return;
    }
    p_lda(RD8(OBJ->x_lo[cpu.x]));
    p_cmp(cpu.a, EDGE_LEFT);
    if (!p_c()) {
        return;
    }
    p_cmp(cpu.a, EDGE_RIGHT);
    if (p_c()) {
        return;
    }
    pj_clear_object(0);
}

void abi_MariaKind23(void)
{
    MariaKind23();
}

/*
 * @routine 01E4:513 MariaKind24
 * @run 6E:DD13
 * WHAT IT DOES: The handler of kind 24. State 0: her flip and facing, her place plus (-4, $17), its speeds by slot
 * (MariaKind24Speeds) turned the way it faces, a fall of $0022 a frame, animation $14, and its animation's timer
 * lengthened by its slot less 11. After: it falls and moves, and is cleared when its animation has stopped.
 * HOW: MariaObjectFromPlayer, MariaObjectAtPlayer, the table by slot less 11 times four, MariaObjectFacingVelocity,
 * MariaObjectStartAnimation; MariaObjectFallCapped, MariaObjectMove, MariaObjectAnimationRunning's Z, ClearObject by the
 * tail JMP.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last step made.
 */
void MariaKind24(void)
{
    uint8_t index;

    p_ldx_object();
    p_lda(RD8(OBJ->state[cpu.x]));
    if (!p_z()) {
        pj_fall_capped(0xDD68);
        pj_move(0xDD6B);
        pj_animation_running(0xDD6E);
        if (!p_z()) {
            return;
        }
        pj_clear_object(0);
        return;
    }
    pj_from_player(0xDD1D);
    at_player(K24_DX, K24_DY, 0xDD28);
    p_slot_less(K24_FIRST);
    p_asl();
    p_asl();
    p_ldy(cpu.a);
    index = cpu.y;
    P_PUT(OBJ->vx_lo[cpu.x], TBL8(MariaKind24Speeds, index));
    P_PUT(OBJ->vx_hi[cpu.x], TBL8(MariaKind24Speeds, index + 1));
    P_PUT(OBJ->vy_lo[cpu.x], TBL8(MariaKind24Speeds, index + 2));
    P_PUT(OBJ->vy_hi[cpu.x], TBL8(MariaKind24Speeds, index + 3));
    pj_facing_velocity(0xDD4A);
    P_PUT(OBJ->fall_lo[cpu.x], K24_FALL_LO);
    P_PUT(OBJ->fall_hi[cpu.x], 0);
    p_lda(K24_ANIMATION);
    pj_start_animation(0xDD59);
    p_slot_less(K24_FIRST);
    abi_flag(P_C, false);
    p_adc(RD8(OBJ->hurt_timer[cpu.x]));
    WR8(OBJ->hurt_timer[cpu.x], cpu.a);
}

void abi_MariaKind24(void)
{
    MariaKind24();
}
