/*
 * Maria's objects (her overlay, bank $6E): items 3, 6 and 8, each an object whose kind is the item's number
 * (MariaThrowItem puts it in slot 5, or 8), and kind 20, the pieces item 6 breaks into.
 *
 * Item 3 flies along an angle (OBJ_FLY_ANGLE) at a speed (OBJ_FLY_SPEED) that grows, turning a little every eighth
 * frame. Item 6 walks the terrain, then breaks into six pieces of kind 20 that fly out. Item 8 is eight objects in
 * slots 5 to 12, each circling out along its own angle while it scales its fall by the cosine.
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
    OBJECT_PLAYER = 0, SLOT_ITEM = 5, BELOW_SCREEN = 1, ABOVE = 0xFF, ROWS = 8, BIT4 = 0x10,
    I3_ACROSS = 0xF0, I3_DOWN = 0xF0, I3_DY = 4, I3_DX = 5, I3_HALF_H = 0x20, I3_HALF_W = 0x10, I3_WORK_N = 0x0C,
    I3_SPEED_HI = 2, I3_ANIMATION = 8, I3_WORK_N_LATE = 0x0D, I3_SPEED_STEP = 0x30, I3_TURN_FRAMES = 0x07,
    I3_EDGE_LEFT = 0x30, I3_EDGE_RIGHT = 0xD0,
    I6_HALF_H = 8, I6_HALF_W = 5, I6_VX = 9, I6_VY = 7, I6_FALL_LO = 0x24, I6_WORK_N = 0x0C, I6_SPRITE_SET = 1,
    I6_FRAME = 0x2D, I6_LAND_ANIMATION = 0x0B, I6_PIECES_END = 0x0B, KIND_I6_PIECE = 0x14, I6_BROKEN_FRAME = 0x30,
    I6_TIMER = 0x20,
    P_FALL_LO = 0xD0, P_FALL_HI = 0xFF, P_HALF_H = 4, P_HALF_W = 6, P_DX = 0xFC, P_WORK_N = 5, P_FIRST_SLOT = 6,
    I8_LAST_KIND_SLOT = 7, KIND_I8 = 8, I8_SOUND = 0x0A, I8_HALF_H = 8, I8_HALF_W = 5, I8_WORK_N = 6, I8_VX = 0x0F,
    I8_SPEED = 0x80, I8_TURN = 4, HITBOX_STRUCK = 2,
    I3_START = 0xCF47, I3_FLY = 0xCF8F, I3_RUSH = 0xCFCA
};

#define MariaItem3States_TBL MariaItem3States
#define MariaKind2WallAhead_TBL MariaKind2WallAhead
#define MariaItem3Angles_TBL MariaItem3Angles
#define MariaItem3Turns_TBL MariaItem3Turns
#define MariaPieceFacings_TBL MariaPieceFacings
#define MariaPieceVyLo_TBL MariaPieceVyLo
#define MariaPieceVyHi_TBL MariaPieceVyHi
#define MariaPieceVx_TBL MariaPieceVx
#define MariaItem8Adds_TBL MariaItem8Adds
#define MariaItem8Timers_TBL MariaItem8Timers
#define MariaItem8Angles_TBL MariaItem8Angles
#define MariaItem8Animations_TBL MariaItem8Animations

/* ------------------------------------------------------------------------------------------------ item 3 */

/*
 * @routine 01E2:73B MariaItem3
 * @run 6E:CF3B
 * WHAT IT DOES: The handler of item 3's object (kind 3): its state's routine from MariaItem3States (start, fly,
 * rush).
 * HOW: MariaStateDoubled, then the JMP (table,X).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
 */
void MariaItem3(void)
{
    uint16_t target;

    pj_state_doubled(0xCF3E);
    target = p_table_word(MariaItem3States);
    switch (target) {
    case I3_START:
        MariaItem3Start();
        break;
    case I3_FLY:
        MariaItem3Fly();
        break;
    case I3_RUSH:
        MariaItem3Rush();
        break;
    default:
        p_jump_elsewhere(target);
        break;
    }
}

void abi_MariaItem3(void)
{
    MariaItem3();
}

/*
 * @routine 01E2:747 MariaItem3Start
 * @run 6E:CF47
 * WHAT IT DOES: State 0 of item 3's object: from her (facing and flip), $10 back and $10 up from her, a hitbox $20
 * by $40 at 5 across and 4 down, live, OBJ_WORK_N $0C; its angle by facing (MariaItem3Angles), speed $0200; then
 * animation 8 and its next state.
 * HOW: MariaObjectFromPlayer, MariaObjectAtPlayer; the tail JMP to MariaObjectStartAnimation.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as MariaObjectStartAnimation.
 */
void MariaItem3Start(void)
{
    p_ldx_object();
    pj_from_player(0xCF4C);
    p_lda(I3_ACROSS);
    WR8(ZP->temp04, I3_ACROSS);
    p_lda(I3_DOWN);
    WR8(ZP->temp06, I3_DOWN);
    pj_at_player(0xCF57);
    p_lda(I3_DY);
    WR8(OBJ->hitbox_dy[cpu.x], I3_DY);
    p_lda(I3_DX);
    WR8(OBJ->hitbox_dx[cpu.x], I3_DX);
    p_lda(I3_HALF_H);
    WR8(OBJ->hitbox_half_h[cpu.x], I3_HALF_H);
    p_lda(I3_HALF_W);
    WR8(OBJ->hitbox_half_w[cpu.x], I3_HALF_W);
    p_lda(1);
    WR8(OBJ->hitbox_on[cpu.x], 1);
    p_lda(I3_WORK_N);
    WR8(OBJ->work_n[cpu.x], I3_WORK_N);
    p_ldy(RD8(OBJ->facing_left[cpu.x]));
    p_lda(TBL8(MariaItem3Angles, cpu.y));
    WR8(OBJ->fly_angle[cpu.x], cpu.a);
    p_lda(0);
    WR8(OBJ->fly_speed_lo[cpu.x], 0);
    p_lda(I3_SPEED_HI);
    WR8(OBJ->fly_speed_hi[cpu.x], I3_SPEED_HI);
    p_lda(I3_ANIMATION);
    pj_start_animation(0);
}

void abi_MariaItem3Start(void)
{
    MariaItem3Start();
}

/*
 * @routine 01E2:78F MariaItem3Fly
 * @run 6E:CF8F
 * WHAT IT DOES: State 1 of item 3's object: it moves; while its animation runs its angle turns by MariaItem3Turns the
 * way it faces and its velocity follows (on into MariaObjectVelocityFromWork); when the animation has stopped,
 * OBJ_WORK_N $0D, the next animation and its next state.
 * HOW: MariaObjectMove, MariaObjectAnimationRunning's Z; MariaObjectSetAnimation, the tail JMP to MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as MariaObjectVelocityFromWork, or MariaObjectNextState.
 */
void MariaItem3Fly(void)
{
    p_ldx_object();
    pj_move(0xCF94);
    pj_animation_running(0xCF97);
    if (!p_z()) {
        p_ldy(RD8(OBJ->facing_left[cpu.x]));
        p_lda(RD8(OBJ->fly_angle[cpu.x]));
        abi_flag(P_C, false);
        p_adc(TBL8(MariaItem3Turns, cpu.y));
        WR8(OBJ->fly_angle[cpu.x], cpu.a);
        pj_velocity_from_work(0);
        return;
    }
    p_lda(I3_WORK_N_LATE);
    WR8(OBJ->work_n[cpu.x], I3_WORK_N_LATE);
    p_lda((uint8_t)(RD8(OBJ->animation[cpu.x]) + 1));
    pj_set_animation(0xCFA5);
    pj_next_state(0);
}

void abi_MariaItem3Fly(void)
{
    MariaItem3Fly();
}

/*
 * @routine 01E2:7CA MariaItem3Rush
 * @run 6E:CFCA
 * WHAT IT DOES: State 2 of item 3's object: it animates, its speed grows by $0030 a frame, every eighth frame its angle
 * turns by the second pair of MariaItem3Turns; its velocity follows and it moves; on a screen past the first with its
 * X from $30 to $CF it is cleared.
 * HOW: MariaObjectAnimationTick, the 16-bit add, zWaveCounter, a BSR to MariaObjectVelocityFromWork, MariaObjectMove;
 * the tail JMP to ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last test, or ClearObject.
 */
void MariaItem3Rush(void)
{
    uint8_t value;

    p_ldx_object();
    pj_animation_tick(0xCFCF);
    p_lda(RD8(OBJ->fly_speed_lo[cpu.x]));
    abi_flag(P_C, false);
    p_adc(I3_SPEED_STEP);
    WR8(OBJ->fly_speed_lo[cpu.x], cpu.a);
    cpu.a = 0;
    p_adc(RD8(OBJ->fly_speed_hi[cpu.x]));
    WR8(OBJ->fly_speed_hi[cpu.x], cpu.a);
    p_lda((uint8_t)(RD8(ZP->wave_counter) & I3_TURN_FRAMES));
    if (cpu.a == 0) {
        p_ldy(RD8(OBJ->facing_left[cpu.x]));
        p_lda(RD8(OBJ->fly_angle[cpu.x]));
        abi_flag(P_C, false);
        p_adc(TBL8(MariaItem3Turns, cpu.y + 2));
        WR8(OBJ->fly_angle[cpu.x], cpu.a);
    }
    pj_velocity_from_work(0xCFF4);
    pj_move(0xCFF7);
    value = RD8(OBJ->x_hi[cpu.x]);
    p_lda(value);
    if (value == 0) {
        return;
    }
    value = RD8(OBJ->x_lo[cpu.x]);
    p_lda(value);
    p_cmp(value, I3_EDGE_LEFT);
    if (value < I3_EDGE_LEFT) {
        return;
    }
    p_cmp(value, I3_EDGE_RIGHT);
    if (value >= I3_EDGE_RIGHT) {
        return;
    }
    ClearObject(cpu.x);
}

void abi_MariaItem3Rush(void)
{
    MariaItem3Rush();
}

/* ------------------------------------------------------------------------------------------------ item 6 */

/*
 * @routine 01E3:00A MariaItem6
 * @run 6E:D00A
 * WHAT IT DOES: The handler of item 6's object (kind 6), by state. 0: from her, 2 across and 2 up, a hitbox $10 by $0A,
 * live; X speed entry 9 the way it faces, Y speed entry 7, a fall of $0024, OBJ_WORK_N $0C, sprite set 1, frame $2D,
 * its next state. 1: it moves and falls; below the screen it is cleared; on the way down 8 rows above solid ground it
 * lands (its Y up by the row, stopped, Y speed entry $0E on bit 4, animation $0B); with something of bit 1 ahead it
 * stops along X. 2: it moves; when its animation has stopped, slots after it up to $0A are emptied and become pieces of
 * kind 20 at slot 5's place and flip, and it shows frame $30 for $20 frames. 3: it moves until that timer runs out,
 * then is cleared.
 * HOW: DEC of the state in A for each; the calls named; the tail JMPs to MariaObjectNextState, MariaKind2Clear,
 * MariaObjectStartAnimation and ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the last step left it.
 */
void MariaItem6(void)
{
    uint8_t value, state, row;

    p_ldx_object();
    state = RD8(OBJ->state[cpu.x]);
    p_lda(state);
    if (state == 0) {
        pj_from_player(0xD014);
        p_lda(2);
        WR8(ZP->temp04, 2);
        p_lda(0xFE);
        WR8(ZP->temp06, 0xFE);
        pj_at_player(0xD01F);
        p_lda(I6_HALF_H);
        WR8(OBJ->hitbox_half_h[cpu.x], I6_HALF_H);
        p_lda(I6_HALF_W);
        WR8(OBJ->hitbox_half_w[cpu.x], I6_HALF_W);
        p_lda(1);
        WR8(OBJ->hitbox_on[cpu.x], 1);
        p_lda(I6_VX);
        pj_velocity_x(0xD033);
        pj_facing_velocity(0xD036);
        p_lda(I6_VY);
        pj_velocity_y(0xD03B);
        p_lda(I6_FALL_LO);
        WR8(OBJ->fall_lo[cpu.x], I6_FALL_LO);
        p_lda(0);
        WR8(OBJ->fall_hi[cpu.x], 0);
        p_lda(I6_WORK_N);
        WR8(OBJ->work_n[cpu.x], I6_WORK_N);
        p_lda(I6_SPRITE_SET);
        WR8(OBJ->sprite_set[cpu.x], I6_SPRITE_SET);
        p_lda(I6_FRAME);
        WR8(OBJ->frame[cpu.x], I6_FRAME);
        pj_next_state(0);
        return;
    }
    p_lda((uint8_t)(cpu.a - 1));
    if (cpu.a == 0) {
        pj_move(0xD05D);
        pj_fall_capped(0xD060);
        value = RD8(OBJ->y_hi[cpu.x]);
        p_lda(value);
        p_cmp(value, BELOW_SCREEN);
        if (value == BELOW_SCREEN) {
            MariaKind2Clear();
            return;
        }
        p_cmp(value, ABOVE);
        if (value != ABOVE) {
            value = RD8(OBJ->vy_hi[cpu.x]);
            p_lda(value);
            if (!(value & 0x80)) {
                p_lda(ROWS);
                pj_proc(0xD078, MariaObjectProbeBelowAny);
                if (!p_z()) {
                    p_lda(0);
                    pj_proc(0xD07F, MariaObjectProbeBelow);
                    if (p_z()) {
                        p_lda(ROWS);
                        pj_proc(0xD096, MariaObjectProbeBelowAny);
                        push8(cpu.a);
                        value = RD8(OBJ->y_lo[cpu.x]);
                        p_lda(value);
                        abi_flag(P_C, true);
                        p_sbc(RD8(ZP->temp02));
                        WR8(OBJ->y_lo[cpu.x], cpu.a);
                        pj_void_x(0xD0A3, MariaObjectStop);
                        WR8(OBJ->y_frac[cpu.x], 0);
                        row = pull8();
                        p_lda((uint8_t)(row & BIT4));
                        if (cpu.a != 0) {
                            p_lda(0x0E);
                            pj_velocity_y(0xD0B0);
                        }
                        p_lda(I6_LAND_ANIMATION);
                        pj_start_animation(0);
                        return;
                    }
                }
            }
        }
        p_ldy(RD8(OBJ->facing_left[cpu.x]));
        p_lda(TBL8(MariaKind2WallAhead, cpu.y));
        cpu.y = 0;
        pj_proc(0xD08B, MariaObjectProbeAtBit1);
        if (!p_z()) {
            pj_void_x(0xD090, MariaObjectStopX);
        }
        return;
    }
    p_lda((uint8_t)(cpu.a - 1));
    if (cpu.a == 0) {
        uint8_t slot;

        pj_move(0xD0BB);
        pj_animation_running(0xD0BE);
        if (!p_z()) {
            return;
        }
        push8(cpu.x);
        do {
            p_ldx((uint8_t)(cpu.x + 1));
            slot = cpu.x;
            pj_clear_object(0xD0C6);
            p_lda(RD8(OBJ->x_lo[SLOT_ITEM]));
            WR8(OBJ->x_lo[slot], cpu.a);
            p_lda(RD8(OBJ->x_hi[SLOT_ITEM]));
            WR8(OBJ->x_hi[slot], cpu.a);
            p_lda(RD8(OBJ->y_lo[SLOT_ITEM]));
            WR8(OBJ->y_lo[slot], cpu.a);
            p_lda(RD8(OBJ->y_hi[SLOT_ITEM]));
            WR8(OBJ->y_hi[slot], cpu.a);
            p_lda(KIND_I6_PIECE);
            WR8(OBJ->kind[slot], KIND_I6_PIECE);
            p_lda(RD8(OBJ->sprite_flip[SLOT_ITEM]));
            WR8(OBJ->sprite_flip[slot], cpu.a);
            p_cmp(slot, I6_PIECES_END);
        } while (slot != I6_PIECES_END);
        p_ldx(pull8());
        p_lda(I6_BROKEN_FRAME);
        WR8(OBJ->frame[cpu.x], I6_BROKEN_FRAME);
        p_lda(I6_TIMER);
        WR8(OBJ->hurt_timer[cpu.x], I6_TIMER);
        pj_next_state(0);
        return;
    }
    pj_move(0xD0FE);
    value = DEC8(OBJ->hurt_timer[cpu.x]);
    abi_nz(value);
    if (value == 0) {
        ClearObject(cpu.x);
    }
}

void abi_MariaItem6(void)
{
    MariaItem6();
}

/*
 * @routine 01E3:107 MariaItem6Piece
 * @run 6E:D107
 * WHAT IT DOES: The handler of kind 20, one of item 6's pieces (slots 6 to 11). State 0: its facing, Y speed and X
 * speed entry by its slot (MariaPieceFacings, MariaPieceVyLo and Hi, MariaPieceVx), a fall of $FFD0 (rising), then
 * its hitbox (MariaItem6PieceHitbox). Later states: MariaItem6PieceFly.
 * HOW: The slot less 6 indexes the tables; MariaSetVelocityXFromTable, MariaObjectFacingVelocity; on into
 * MariaItem6PieceHitbox; the branch to MariaItem6PieceFly.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the routine run into or branched to.
 */
void MariaItem6Piece(void)
{
    uint8_t value;

    p_ldx_object();
    value = RD8(OBJ->state[cpu.x]);
    p_lda(value);
    if (value == 0) {
        p_lda(cpu.x);
        abi_flag(P_C, true);
        p_sbc(P_FIRST_SLOT);
        p_ldy(cpu.a);
        p_lda(TBL8(MariaPieceFacings, cpu.y));
        WR8(OBJ->facing_left[cpu.x], cpu.a);
        p_lda(P_FALL_LO);
        WR8(OBJ->fall_lo[cpu.x], P_FALL_LO);
        p_lda(P_FALL_HI);
        WR8(OBJ->fall_hi[cpu.x], P_FALL_HI);
        p_lda(TBL8(MariaPieceVyLo, cpu.y));
        WR8(OBJ->vy_lo[cpu.x], cpu.a);
        p_lda(TBL8(MariaPieceVyHi, cpu.y));
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        p_lda(TBL8(MariaPieceVx, cpu.y));
        pj_velocity_x(0xD135);
        pj_facing_velocity(0xD138);
        MariaItem6PieceHitbox();
        return;
    }
    MariaItem6PieceFly();
}

void abi_MariaItem6Piece(void)
{
    MariaItem6Piece();
}

/*
 * @routine 01E3:156 MariaItem6PieceFly
 * @run 6E:D156
 * WHAT IT DOES: An item 6 piece after its start: it animates, moves and falls, and is cleared above the screen.
 * HOW: MariaObjectAnimationTick, MariaObjectMove, MariaObjectFallCapped; the tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: as the compare with $FF, or ClearObject.
 */
void MariaItem6PieceFly(void)
{
    uint8_t value;

    pj_animation_tick(0xD159);
    pj_move(0xD15C);
    pj_fall_capped(0xD15F);
    value = RD8(OBJ->y_hi[cpu.x]);
    p_lda(value);
    p_cmp(value, ABOVE);
    if (value == ABOVE) {
        ClearObject(cpu.x);
    }
}

void abi_MariaItem6PieceFly(void)
{
    MariaItem6PieceFly();
}

/*
 * @routine 01E3:138 MariaItem6PieceHitbox
 * @run 6E:D138
 * WHAT IT DOES: An item piece's hitbox: $08 high, $0C wide, 4 to the left, live; OBJ_WORK_N 5; animation 0 and its
 * next state.
 * HOW: The stores, then the tail JMP to MariaObjectStartAnimation.
 * WAITS: no.
 * Entry: X the object. Exit: as MariaObjectStartAnimation.
 */
void MariaItem6PieceHitbox(void)
{
    p_lda(P_HALF_H);
    WR8(OBJ->hitbox_half_h[cpu.x], P_HALF_H);
    p_lda(P_HALF_W);
    WR8(OBJ->hitbox_half_w[cpu.x], P_HALF_W);
    p_lda(P_DX);
    WR8(OBJ->hitbox_dx[cpu.x], P_DX);
    p_lda(1);
    WR8(OBJ->hitbox_on[cpu.x], 1);
    p_lda(P_WORK_N);
    WR8(OBJ->work_n[cpu.x], P_WORK_N);
    p_lda(0);
    pj_start_animation(0);
}

void abi_MariaItem6PieceHitbox(void)
{
    MariaItem6PieceHitbox();
}

/* ------------------------------------------------------------------------------------------------ item 8 */

/*
 * @routine 01E3:182 MariaItem8
 * @run 6E:D182
 * WHAT IT DOES: The handler of item 8's objects (kind 8, slots 5 to 12). State 0: the one in slot 5 makes slots 6 to
 * 12 kind 8 as well and plays sound $0A; each starts from her (2 across, 2 up), a hitbox $10 by $0A, live, OBJ_WORK_N
 * 6, X speed entry $0F, facing right, with its own angle, delay and animation by slot (MariaItem8Angles,
 * MariaItem8Timers, MariaItem8Animations). Later: while its delay runs it shows no frame and moves against the scroll;
 * then it falls along the cosine of its angle (speed $0080), the angle turning by 4 a frame, and moves. Struck by an
 * object whose hitbox is 2 with OBJ_WORK_P 1 or 2, it ends next frame (timer 1, no hitbox); when its animation has
 * stopped, the item's first use is due again and it is cleared.
 * HOW: The slot less 5 indexes the tables; SoundCommand; ScaleByCosine and zProduct1-2; OBJ_HIT_BY_SLOT;
 * MariaObjectAnimationRunning; the tail JMPs to MariaObjectStartAnimation, MariaObjectAgainstScroll and ClearObject.
 * The add of MariaItem8Adds to $80 at $D1F8 is made and its result left unused, as the assembly does.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the last step left it.
 */
void MariaItem8(void)
{
    uint8_t value, struck;

    p_ldx_object();
    value = RD8(OBJ->state[cpu.x]);
    p_lda(value);
    if (value == 0) {
        p_cmp(cpu.x, SLOT_ITEM);
        if (cpu.x == SLOT_ITEM) {
            p_lda(KIND_I8);
            cpu.y = 0;
            do {
                WR8(OBJ->kind[SLOT_ITEM + 1 + cpu.y], KIND_I8);
                p_ldy((uint8_t)(cpu.y + 1));
                p_cmp(cpu.y, I8_LAST_KIND_SLOT);
            } while (cpu.y != I8_LAST_KIND_SLOT);
            p_lda(I8_SOUND);
            pj_sound(0xD19D);
        }
        pj_from_player(0xD1A0);
        p_lda(2);
        WR8(ZP->temp04, 2);
        p_lda(0xFE);
        WR8(ZP->temp06, 0xFE);
        pj_at_player(0xD1AB);
        p_lda(I8_HALF_H);
        WR8(OBJ->hitbox_half_h[cpu.x], I8_HALF_H);
        p_lda(I8_HALF_W);
        WR8(OBJ->hitbox_half_w[cpu.x], I8_HALF_W);
        p_lda(1);
        WR8(OBJ->hitbox_on[cpu.x], 1);
        p_lda(I8_WORK_N);
        WR8(OBJ->work_n[cpu.x], I8_WORK_N);
        p_lda(I8_VX);
        pj_velocity_x(0xD1C4);
        pj_facing_velocity(0xD1C7);
        p_lda(cpu.x);
        abi_flag(P_C, true);
        p_sbc(SLOT_ITEM);
        p_ldy(cpu.a);
        p_lda(TBL8(MariaItem8Angles, cpu.y));
        WR8(OBJ->fly_angle[cpu.x], cpu.a);
        p_lda(TBL8(MariaItem8Timers, cpu.y));
        WR8(OBJ->anim_timer[cpu.x], cpu.a);
        WR8(OBJ->facing_left[cpu.x], 0);
        p_lda(TBL8(MariaItem8Animations, cpu.y));
        pj_start_animation(0);
        return;
    }
    value = RD8(OBJ->anim_timer[cpu.x]);
    p_lda(value);
    if (value != 0) {
        WR8(OBJ->frame[cpu.x], 0);
        abi_nz(DEC8(OBJ->anim_timer[cpu.x]));
        pj_against_scroll(0);
        return;
    }
    p_lda(cpu.x);
    abi_flag(P_C, true);
    p_sbc(SLOT_ITEM);
    p_ldy(cpu.a);
    p_lda(I8_SPEED);
    WR8(ZP->temp00, I8_SPEED);
    p_lda(I8_SPEED);
    abi_flag(P_C, false);
    p_adc(TBL8(MariaItem8Adds, cpu.y));
    WR8(ZP->temp01, 0);
    p_lda(RD8(OBJ->fly_angle[cpu.x]));
    jsr_push(0xD206);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    p_lda(RD8(OBJ->fly_angle[cpu.x]));
    abi_flag(P_C, false);
    p_adc(I8_TURN);
    WR8(OBJ->fly_angle[cpu.x], cpu.a);
    p_lda(RD8(ZP->product.mid));
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    p_lda(RD8(ZP->product.hi));
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    pj_move(0xD21C);
    p_ldy(RD8(OBJ->hit_by_slot[cpu.x]));
    struck = RD8(OBJ->hitbox_on[cpu.y]);
    p_lda(struck);
    p_cmp(struck, HITBOX_STRUCK);
    if (struck == HITBOX_STRUCK) {
        value = RD8(OBJ->work_p[cpu.x]);
        p_lda(value);
        p_cmp(value, 1);
        if (value != 1) {
            p_cmp(value, 2);
        }
        if (value == 1 || value == 2) {
            p_lda(1);
            WR8(OBJ->hurt_timer[cpu.x], 1);
            WR8(OBJ->work_p[cpu.x], 0);
            WR8(OBJ->hitbox_on[cpu.x], 0);
            return;
        }
    }
    pj_animation_running(0xD234);
    if (!p_z()) {
        return;
    }
    p_lda(1);
    WR8(PVARS->item_first_use, 1);
    ClearObject(cpu.x);
}

void abi_MariaItem8(void)
{
    MariaItem8();
}
