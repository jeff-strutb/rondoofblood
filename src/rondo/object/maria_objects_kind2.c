/*
 * Maria's objects (her overlay, bank $6E): kind 2, an object that walks the terrain. It starts where she is
 * (MariaKind2Start), walks the way it faces with the terrain read under and ahead of it (states 1 and 6), turns or
 * leaps at a wall (states 3 and 4 in the air), lands, climbs a step (state 5), and is cleared when it leaves the
 * screen's middle or falls past the bottom.
 *
 * These routines hold the processor (maria_objects_p.h): `cpu` is the register file.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"
#include "maria_objects_p.h"

enum {
    K2_HALF_H = 5, K2_HALF_W = 0x0C, K2_WORK_H = 0x24, K2_WORK_N = 0x0C, K2_TURNS = 3, K2_START_FRAME = 0x0B,
    K2_TURN_FRAME = 0x0C, K2_STATE_LEAP = 3, K2_STATE_TURNED = 4, K2_STATE_WALK = 1, K2_STATE_LANDED = 2,
    K2_STATE_CLIMB = 5, K2_STATE_FALL_WALK = 6, K2_BELOW_SCREEN = 1, K2_ABOVE = 0xFF, K2_BIT5 = 0x20, K2_BIT3 = 0x08,
    K2_BIT4 = 0x10, K2_ROWS = 8, K2_EDGE_RIGHT = 0xF0, K2_EDGE_LEFT = 0x10, K2_CAMERA_SPECIAL = 0x17,
    K2_SCREEN_LAST = 0x0E, K2_SPECIAL_X = 0x58,
    K2_START = 0xC95F, K2_WALK = 0xC9A4, K2_STATE2 = 0xCA6F, K2_AIR = 0xCB07, K2_STATE5 = 0xCB89
};

#define MariaKind2States_TBL MariaKind2States
#define MariaKind2WallAhead_TBL MariaKind2WallAhead
#define MariaKind2DropAhead_TBL MariaKind2DropAhead
#define MariaKind2FloorAhead_TBL MariaKind2FloorAhead
#define MariaKind2StepAhead_TBL MariaKind2StepAhead


/* LDA #n, JSR to a probe that takes A rows (and Y) and is processor-holding. */
static void probe(uint16_t ret, void (*fn)(void))
{
    pj_proc(ret, fn);
}

/* $CA17 on: the state in A, Y speed entry 3, X speed entry 2 the way it faces, frame $0C. */
static void turn_tail(void)
{
    WR8(OBJ->state[cpu.x], cpu.a);
    p_lda(3);
    pj_velocity_y(0xCA1F);
    p_lda(2);
    pj_velocity_x(0xCA24);
    pj_facing_velocity(0xCA27);
    p_lda(K2_TURN_FRAME);
    WR8(OBJ->frame[cpu.x], K2_TURN_FRAME);
}

/*
 * @routine 01E2:14B MariaKind2
 * @run 6E:C94B
 * WHAT IT DOES: The handler of kind 2: its state's routine from MariaKind2States (start, walk, the edge of the
 * screen, in the air twice, the step, walk again).
 * HOW: MariaStateDoubled, then the JMP (table,X).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
 */
void MariaKind2(void)
{
    uint16_t target;

    pj_state_doubled(0xC94E);
    target = p_table_word(MariaKind2States);
    switch (target) {
    case K2_START:
        MariaKind2Start();
        break;
    case K2_WALK:
        MariaKind2Walk();
        break;
    case K2_STATE2:
        MariaKind2State2();
        break;
    case K2_AIR:
        MariaKind2Air();
        break;
    case K2_STATE5:
        MariaKind2State5();
        break;
    default:
        p_jump_elsewhere(target);
        break;
    }
}

void abi_MariaKind2(void)
{
    MariaKind2();
}

/*
 * @routine 01E2:15F MariaKind2Start
 * @run 6E:C95F
 * WHAT IT DOES: State 0 of kind 2: from her (facing and flip), 2 across and 2 up from her, a hitbox 12 wide and 5
 * high, live; X speed entry 1 the way it faces; a fall of $0024 a frame (OBJ_FALL_LO and HI), OBJ_WORK_N $0C, three
 * OBJ_WALL_TURNS; animation 0 and its next state; frame $0B.
 * HOW: MariaObjectFromPlayer, MariaObjectAtPlayer, MariaSetVelocityXFromTable, MariaObjectFacingVelocity,
 * MariaObjectStartAnimation.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: A $0B, N and Z from it; the rest as MariaObjectStartAnimation left it.
 */
void MariaKind2Start(void)
{
    p_ldx_object();
    pj_from_player(0xC964);
    p_lda(2);
    WR8(ZP->temp04, 2);
    p_lda(0xFE);
    WR8(ZP->temp06, 0xFE);
    pj_at_player(0xC96F);
    p_lda(K2_HALF_H);
    WR8(OBJ->hitbox_half_h[cpu.x], K2_HALF_H);
    p_lda(K2_HALF_W);
    WR8(OBJ->hitbox_half_w[cpu.x], K2_HALF_W);
    p_lda(1);
    WR8(OBJ->hitbox_on[cpu.x], 1);
    p_lda(1);
    pj_velocity_x(0xC983);
    pj_facing_velocity(0xC986);
    p_lda(K2_WORK_H);
    WR8(OBJ->fall_lo[cpu.x], K2_WORK_H);
    p_lda(0);
    WR8(OBJ->fall_hi[cpu.x], 0);
    p_lda(K2_WORK_N);
    WR8(OBJ->work_n[cpu.x], K2_WORK_N);
    p_lda(K2_TURNS);
    WR8(OBJ->wall_turns[cpu.x], K2_TURNS);
    cpu.a = 0;
    pj_start_animation(0xC99E);
    p_lda(K2_START_FRAME);
    WR8(OBJ->frame[cpu.x], K2_START_FRAME);
}

void abi_MariaKind2Start(void)
{
    MariaKind2Start();
}

/*
 * @routine 01E2:1A4 MariaKind2Walk
 * @run 6E:C9A4
 * WHAT IT DOES: States 1 and 6 of kind 2, walking: hit, it turns (MariaKind2Turn); it moves and falls, animates while
 * it has an animation; below the screen it is cleared. On the way down, a mark of bit 5 under it leaps it
 * (MariaKind2Leap) and solid ground lands it (MariaKind2Land); then a bit-5 mark at its feet leaps it, bit 3 a step
 * ahead climbs it (MariaKind2Step); with nothing of bit 1 ahead it goes to the edge check (MariaKind2Edge); else a
 * wall: its turns counted down, a turn (state 3) while some are left, else state 4.
 * HOW: MariaObjectHitByChecked's carry; MariaObjectMove, MariaObjectFallCapped, MariaObjectAnimationTick; the probes
 * below and at offsets (MariaObjectProbeBelowAny, MariaObjectProbeBelow, MariaObjectProbeAt, MariaObjectProbeAtBit1).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the last step left it.
 */
void MariaKind2Walk(void)
{
    uint8_t value;

    p_ldx_object();
    pj_hit_by_checked(0xC9A9);
    if (p_c()) {
        MariaKind2Turn();
        return;
    }
    pj_move(0xC9AE);
    pj_fall_capped(0xC9B1);
    value = RD8(OBJ->animation[cpu.x]);
    p_lda(value);
    if (value != 0) {
        pj_animation_tick(0xC9B9);
    }
    value = RD8(OBJ->y_hi[cpu.x]);
    p_lda(value);
    p_cmp(value, K2_BELOW_SCREEN);
    if (value == K2_BELOW_SCREEN) {
        MariaKind2Clear();
        return;
    }
    p_cmp(value, K2_ABOVE);
    if (value != K2_ABOVE) {
        value = RD8(OBJ->vy_hi[cpu.x]);
        p_lda(value);
        if (!(value & 0x80)) {
            p_lda(K2_ROWS);
            probe(0xC9D1, MariaObjectProbeBelowAny);
            if (!p_z()) {
                p_lda((uint8_t)(cpu.a & K2_BIT5));
                if (cpu.a != 0) {
                    MariaKind2Leap();
                    return;
                }
                p_lda(0);
                probe(0xC9DF, MariaObjectProbeBelow);
                if (p_z()) {
                    MariaKind2Land();
                    return;
                }
            }
        }
    }
    p_lda(0);
    probe(0xC9E6, MariaObjectProbeBelowAny);
    p_lda((uint8_t)(cpu.a & K2_BIT5));
    if (cpu.a != 0) {
        MariaKind2Leap();
        return;
    }
    p_lda(0);
    p_ldy(K2_ROWS);
    probe(0xC9F4, MariaObjectProbeAt);
    p_lda((uint8_t)(cpu.a & K2_BIT3));
    if (cpu.a != 0) {
        MariaKind2Step();
        return;
    }
    p_ldy(RD8(OBJ->facing_left[cpu.x]));
    p_lda(TBL8(MariaKind2WallAhead, cpu.y));
    cpu.y = 0;
    probe(0xCA05, MariaObjectProbeAtBit1);
    if (p_z()) {
        MariaKind2Edge();
        return;
    }
    value = DEC8(OBJ->wall_turns[cpu.x]);
    abi_nz(value);
    if (value != 0) {
        MariaKind2Turn();
        return;
    }
    p_lda(K2_STATE_TURNED);
    turn_tail();
}

void abi_MariaKind2Walk(void)
{
    MariaKind2Walk();
}

/*
 * @routine 01E2:210 MariaKind2Turn
 * @run 6E:CA10
 * WHAT IT DOES: Turns kind 2 from a wall or a hit: state 3 while it has turns left in OBJ_WALL_TURNS, else state 4; Y
 * speed entry 3, X speed entry 2 the way it faces, frame $0C.
 * HOW: The velocity tables, MariaObjectFacingVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A $0C, N and Z from it; the rest as MariaObjectFacingVelocity left it.
 */
void MariaKind2Turn(void)
{
    uint8_t value = RD8(OBJ->wall_turns[cpu.x]);

    p_lda(value);
    p_lda(value == 0 ? K2_STATE_TURNED : K2_STATE_LEAP);
    turn_tail();
}

void abi_MariaKind2Turn(void)
{
    MariaKind2Turn();
}

/*
 * @routine 01E2:22D MariaKind2Edge
 * @run 6E:CA2D
 * WHAT IT DOES: Kind 2 with nothing ahead of it: once its Y speed's high byte is $FF (rising), a mark of bit 5 under
 * it, 8 rows below or at its feet, leaps it.
 * HOW: MariaObjectProbeBelowAny twice; the JMP to MariaKind2Leap.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or MariaKind2Leap.
 */
void MariaKind2Edge(void)
{
    uint8_t value = RD8(OBJ->vy_hi[cpu.x]);

    p_lda(value);
    p_cmp(value, K2_ABOVE);
    if (value != K2_ABOVE) {
        return;
    }
    p_lda(K2_ROWS);
    probe(0xCA3A, MariaObjectProbeBelowAny);
    p_lda((uint8_t)(cpu.a & K2_BIT5));
    if (cpu.a != 0) {
        MariaKind2Leap();
        return;
    }
    p_lda(0);
    probe(0xCA46, MariaObjectProbeBelowAny);
    p_lda((uint8_t)(cpu.a & K2_BIT5));
    if (cpu.a != 0) {
        MariaKind2Leap();
    }
}

void abi_MariaKind2Edge(void)
{
    MariaKind2Edge();
}

/*
 * @routine 01E2:24E MariaKind2Land
 * @run 6E:CA4E
 * WHAT IT DOES: Lands kind 2 on the ground 8 rows below it: its Y up by the row the probe left in zTemp02, no Y speed
 * or fraction, animation 4, state 2.
 * HOW: MariaObjectProbeBelowAny, the subtract, MariaObjectStopY, MariaObjectStartAnimation.
 * WAITS: no.
 * Entry: X the object. Exit: A 2, N and Z from it; the rest as MariaObjectStartAnimation left it.
 */
void MariaKind2Land(void)
{
    uint8_t value;

    p_lda(K2_ROWS);
    probe(0xCA53, MariaObjectProbeBelowAny);
    value = RD8(OBJ->y_lo[cpu.x]);
    p_lda(value);
    abi_flag(P_C, true);
    p_sbc(RD8(ZP->temp02));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    pj_void_x(0xCA5F, MariaObjectStopY);
    WR8(OBJ->y_frac[cpu.x], 0);
    p_lda(4);
    pj_start_animation(0xCA67);
    p_lda(K2_STATE_LANDED);
    WR8(OBJ->state[cpu.x], K2_STATE_LANDED);
}

void abi_MariaKind2Land(void)
{
    MariaKind2Land();
}

/*
 * @routine 01E2:26F MariaKind2State2
 * @run 6E:CA6F
 * WHAT IT DOES: State 2 of kind 2: on a screen past the first (OBJ_X_HI not zero) with its X from $10 to $EF there,
 * it is cleared; else it walks on (MariaKind2Walk2).
 * HOW: OBJ_X_HI and OBJ_X_LO; on into MariaKind2Clear, or a branch to MariaKind2Walk2.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as MariaKind2Clear or MariaKind2Walk2.
 */
void MariaKind2State2(void)
{
    uint8_t value;

    p_ldx_object();
    value = RD8(OBJ->x_hi[cpu.x]);
    p_lda(value);
    if (value != 0) {
        value = RD8(OBJ->x_lo[cpu.x]);
        p_lda(value);
        p_cmp(value, K2_EDGE_RIGHT);
        if (value < K2_EDGE_RIGHT) {
            p_cmp(value, K2_EDGE_LEFT);
            if (value >= K2_EDGE_LEFT) {
                MariaKind2Clear();
                return;
            }
        }
    }
    MariaKind2Walk2();
}

void abi_MariaKind2State2(void)
{
    MariaKind2State2();
}

/*
 * @routine 01E2:281 MariaKind2Clear
 * @run 6E:CA81
 * WHAT IT DOES: Clears kind 2's object.
 * HOW: The tail JMP to ClearObject through the kernel's vector.
 * WAITS: no.
 * Entry: X the object. Exit: all registers and flags kept.
 */
void MariaKind2Clear(void)
{
    ClearObject(cpu.x);
}

void abi_MariaKind2Clear(void)
{
    MariaKind2Clear();
}

/*
 * @routine 01E2:284 MariaKind2Walk2
 * @run 6E:CA84
 * @stop CAD4
 * WHAT IT DOES: Kind 2 walking in state 2: hit, it turns; it animates and moves; 8 rows below it, bit 7 of the answer
 * lifts it by the row in zTemp02. Otherwise with nothing solid ahead and below, or something of bit 1 ahead, it drops
 * (MariaKind2Drop); under bit 4 straight below, Y speed entry $0E.
 * HOW: MariaObjectHitByChecked, MariaObjectAnimationTick, MariaObjectMove; MariaObjectProbe, MariaObjectProbeAtSolid,
 * MariaObjectProbeAtBit1; MariaKind2FloorAhead and MariaKind2DropAhead by the facing.
 * WAITS: no.
 * Entry: X the object. Exit: the processor as the last step left it.
 */
void MariaKind2Walk2(void)
{
    uint8_t value;

    pj_hit_by_checked(0xCA87);
    if (p_c()) {
        MariaKind2Turn();
        return;
    }
    pj_animation_tick(0xCA8C);
    pj_move(0xCA8F);
    p_lda(K2_ROWS);
    WR8(ZP->temp06, K2_ROWS);
    WR8(ZP->temp07, 0);
    WR8(ZP->temp04, 0);
    WR8(ZP->temp05, 0);
    probe(0xCA9C, MariaObjectProbe);
    p_lda((uint8_t)(cpu.a & 0x80));
    if (cpu.a != 0) {
        value = RD8(OBJ->y_lo[cpu.x]);
        p_lda(value);
        abi_flag(P_C, true);
        p_sbc(RD8(ZP->temp02));
        WR8(OBJ->y_lo[cpu.x], cpu.a);
        return;
    }
    p_ldy(RD8(OBJ->facing_left[cpu.x]));
    p_lda(TBL8(MariaKind2FloorAhead, cpu.y));
    p_ldy(K2_ROWS);
    probe(0xCAAB, MariaObjectProbeAtSolid);
    if (p_z()) {
        MariaKind2Drop();
        return;
    }
    p_ldy(RD8(OBJ->facing_left[cpu.x]));
    p_lda(TBL8(MariaKind2DropAhead, cpu.y));
    p_ldy(2);
    probe(0xCAB8, MariaObjectProbeAtBit1);
    if (!p_z()) {
        MariaKind2Drop();
        return;
    }
    p_ldy(K2_ROWS);
    cpu.a = 0;
    probe(0xCAC0, MariaObjectProbeAtSolid);
    p_lda((uint8_t)(cpu.a & K2_BIT4));
    if (cpu.a == 0) {
        return;
    }
    p_lda(0x0E);
    pj_velocity_y(0xCAC9);
}

void abi_MariaKind2Walk2(void)
{
    MariaKind2Walk2();
}

/*
 * @routine 01E2:2D8 MariaKind2Leap
 * @run 6E:CAD8
 * WHAT IT DOES: Kind 2 leaps: Y speed entry 3, X speed entry $0A the way it faces; animation 4 unless it is in state 6
 * already; state 6.
 * HOW: The velocity tables, MariaObjectFacingVelocity, MariaObjectStartAnimation.
 * WAITS: no.
 * Entry: X the object. Exit: A 6, N and Z from it; the rest as the last call left it.
 */
void MariaKind2Leap(void)
{
    uint8_t state;

    p_lda(3);
    pj_velocity_y(0xCADD);
    p_lda(0x0A);
    pj_velocity_x(0xCAE2);
    pj_facing_velocity(0xCAE5);
    state = RD8(OBJ->state[cpu.x]);
    p_lda(state);
    p_cmp(state, K2_STATE_FALL_WALK);
    if (state != K2_STATE_FALL_WALK) {
        p_lda(4);
        pj_start_animation(0xCAF1);
    }
    p_lda(K2_STATE_FALL_WALK);
    WR8(OBJ->state[cpu.x], K2_STATE_FALL_WALK);
}

void abi_MariaKind2Leap(void)
{
    MariaKind2Leap();
}

/*
 * @routine 01E2:2F7 MariaKind2Drop
 * @run 6E:CAF7
 * WHAT IT DOES: Kind 2 drops: Y speed entry 3, animation 5, state 1.
 * HOW: MariaSetVelocityYFromTable, MariaObjectStartAnimation.
 * WAITS: no.
 * Entry: X the object. Exit: A 1, N and Z from it; the rest as MariaObjectStartAnimation left it.
 */
void MariaKind2Drop(void)
{
    p_lda(3);
    pj_velocity_y(0xCAFC);
    p_lda(5);
    pj_start_animation(0xCB01);
    p_lda(K2_STATE_WALK);
    WR8(OBJ->state[cpu.x], K2_STATE_WALK);
}

void abi_MariaKind2Drop(void)
{
    MariaKind2Drop();
}

/*
 * @routine 01E2:307 MariaKind2Air
 * @run 6E:CB07
 * WHAT IT DOES: States 3 and 4 of kind 2, in the air: it moves and falls; below the screen it is cleared. In state 3
 * on the way down, a bit-5 mark under it leaps it and solid ground lands it (then walks, state 2, live); with nothing
 * below, bit 3 at its feet climbs it; otherwise the edge check.
 * HOW: MariaObjectMove, MariaObjectFallCapped; MariaObjectProbeBelowAny, MariaObjectProbeBelow, MariaObjectProbeAt;
 * MariaKind2Land, the velocity table, MariaObjectFacingVelocity.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the last step left it.
 */
void MariaKind2Air(void)
{
    uint8_t value;

    p_ldx_object();
    pj_move(0xCB0C);
    pj_fall_capped(0xCB0F);
    value = RD8(OBJ->y_hi[cpu.x]);
    p_lda(value);
    p_cmp(value, K2_BELOW_SCREEN);
    if (value == K2_BELOW_SCREEN) {
        MariaKind2Clear();
        return;
    }
    p_cmp(value, K2_ABOVE);
    if (value != K2_ABOVE) {
        value = RD8(OBJ->state[cpu.x]);
        p_lda(value);
        p_cmp(value, K2_STATE_TURNED);
        if (value != K2_STATE_TURNED) {
            value = RD8(OBJ->vy_hi[cpu.x]);
            p_lda(value);
            if (!(value & 0x80)) {
                p_lda(K2_ROWS);
                probe(0xCB2E, MariaObjectProbeBelowAny);
                if (p_z()) {
                    p_lda(0);
                    p_ldy(K2_ROWS);
                    probe(0xCB5E, MariaObjectProbeAt);
                    p_lda((uint8_t)(cpu.a & K2_BIT3));
                    if (cpu.a != 0) {
                        MariaKind2Step();
                        return;
                    }
                } else {
                    p_lda((uint8_t)(cpu.a & K2_BIT5));
                    if (cpu.a != 0) {
                        MariaKind2Leap();
                        return;
                    }
                    p_lda(0);
                    probe(0xCB3C, MariaObjectProbeBelow);
                    if (p_z()) {
                        pj_proc(0xCB44, MariaKind2Land);
                        p_lda(1);
                        pj_velocity_x(0xCB49);
                        pj_facing_velocity(0xCB4C);
                        p_lda(K2_STATE_LANDED);
                        WR8(OBJ->state[cpu.x], K2_STATE_LANDED);
                        p_lda(1);
                        WR8(OBJ->hitbox_on[cpu.x], 1);
                        return;
                    }
                }
            }
        }
    }
    MariaKind2Edge();
}

void abi_MariaKind2Air(void)
{
    MariaKind2Air();
}

/*
 * @routine 01E2:362 MariaKind2Step
 * @run 6E:CB62
 * WHAT IT DOES: Kind 2 climbs a step: its Y up by the row in zTemp02 and down 6, no Y speed, X speed entry 6 the way
 * it faces, state 5, animation $0A.
 * HOW: The subtract and add, MariaObjectStopY, the velocity table, MariaObjectFacingVelocity, the tail JMP to
 * MariaObjectSetAnimation.
 * WAITS: no.
 * Entry: X the object. Exit: as MariaObjectSetAnimation.
 */
void MariaKind2Step(void)
{
    uint8_t value = RD8(OBJ->y_lo[cpu.x]);

    p_lda(value);
    abi_flag(P_C, true);
    p_sbc(RD8(ZP->temp02));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    p_lda(6);
    abi_flag(P_C, false);
    p_adc(RD8(OBJ->y_lo[cpu.x]));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    pj_void_x(0xCB77, MariaObjectStopY);
    p_lda(6);
    pj_velocity_x(0xCB7C);
    pj_facing_velocity(0xCB7F);
    p_lda(K2_STATE_CLIMB);
    WR8(OBJ->state[cpu.x], K2_STATE_CLIMB);
    p_lda(0x0A);
    pj_set_animation(0);
}

void abi_MariaKind2Step(void)
{
    MariaKind2Step();
}

/*
 * @routine 01E2:389 MariaKind2State5
 * @run 6E:CB89
 * WHAT IT DOES: State 5 of kind 2, on the step: on a screen past the first with its X from $10 to $EF it is cleared; hit, it turns; it
 * animates; in camera mode $17 its speed follows the scroll (MariaKind2Special); it moves; with something of bit 1
 * ahead it goes up 6 rows and drops on with X speed entry 7 and Y speed entry 0.
 * HOW: OBJ_X_HI and OBJ_X_LO, the tail JMP to ClearObject; MariaObjectHitByChecked, MariaObjectAnimationTick,
 * MariaObjectMove, MariaObjectProbeAtBit1 with MariaKind2StepAhead by the facing; MariaKind2Drop and the velocity tables.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the last step left it.
 */
void MariaKind2State5(void)
{
    uint8_t value;

    p_ldx_object();
    value = RD8(OBJ->x_hi[cpu.x]);
    p_lda(value);
    if (value != 0) {
        value = RD8(OBJ->x_lo[cpu.x]);
        p_lda(value);
        p_cmp(value, K2_EDGE_RIGHT);
        if (value < K2_EDGE_RIGHT) {
            p_cmp(value, K2_EDGE_LEFT);
            if (value >= K2_EDGE_LEFT) {
                ClearObject(cpu.x);
                return;
            }
        }
    }
    pj_hit_by_checked(0xCBA1);
    if (p_c()) {
        MariaKind2Turn();
        return;
    }
    pj_animation_tick(0xCBA9);
    value = RD8(SCROLLLIM->camera_mode);
    p_lda(value);
    p_cmp(value, K2_CAMERA_SPECIAL);
    if (value == K2_CAMERA_SPECIAL) {
        pj_proc(0xCBB3, MariaKind2Special);
    }
    pj_move(0xCBB6);
    p_ldy(RD8(OBJ->facing_left[cpu.x]));
    p_lda(TBL8(MariaKind2StepAhead, cpu.y));
    p_ldy(2);
    probe(0xCBC1, MariaObjectProbeAtBit1);
    if (p_z()) {
        return;
    }
    p_lda(0xFA);
    abi_flag(P_C, false);
    p_adc(RD8(OBJ->y_lo[cpu.x]));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    pj_proc(0xCBCF, MariaKind2Drop);
    p_lda(7);
    pj_velocity_x(0xCBD4);
    pj_facing_velocity(0xCBD7);
    p_lda(0);
    pj_velocity_y(0xCBDC);
}

void abi_MariaKind2State5(void)
{
    MariaKind2State5();
}

/*
 * @routine 01E2:3DF MariaKind2Special
 * @run 6E:CBDF
 * WHAT IT DOES: Kind 2 in camera mode $17: off screen 0 its Y speed is $0080 and it moves against the scroll
 * (MariaObjectAgainstScroll); on the last screen ($0E) from X $58 on, Y speed $0480 and nothing more.
 * HOW: ScrollXHiScreen (read twice); the stores; MariaObjectAgainstScroll.
 * WAITS: no.
 * Entry: X the object. Exit: screen 0, A 0, Z set; else as MariaObjectAgainstScroll, or A 4 with N and Z from it.
 */
void MariaKind2Special(void)
{
    uint8_t value = RD8(SCROLLE->x_hi_screen[0]);

    p_lda(value);
    if (value == 0) {
        return;
    }
    value = RD8(SCROLLE->x_hi_screen[0]);
    p_lda(value);
    p_cmp(value, K2_SCREEN_LAST);
    if (value == K2_SCREEN_LAST) {
        value = RD8(OBJ->x_lo[cpu.x]);
        p_lda(value);
        p_cmp(value, K2_SPECIAL_X);
        if (value >= K2_SPECIAL_X) {
            p_lda(0x80);
            WR8(OBJ->vy_lo[cpu.x], 0x80);
            p_lda(4);
            WR8(OBJ->vy_hi[cpu.x], 4);
            return;
        }
    }
    p_lda(0x80);
    WR8(OBJ->vy_lo[cpu.x], 0x80);
    p_lda(0);
    WR8(OBJ->vy_hi[cpu.x], 0);
    pj_against_scroll(0xCBF8);
}

void abi_MariaKind2Special(void)
{
    MariaKind2Special();
}
