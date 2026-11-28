/*
 * Maria's objects (her overlay, bank $6E): kind 16, kind 22 and kind 15. Kinds 15 and 16 fly along an angle
 * (OBJ_FLY_ANGLE) at a speed (OBJ_SWOOP_SPEED_LO and _HI), which MariaObjectFlyAlong turns into their velocity each
 * frame; both start at her frame $51 and are cleared when she is hit. Kind 15 starts in slot 5 with two more of its
 * kind in slots 6 to 8, rides, can be steered up and down by the pad, and rains pieces of kind 11.
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
    OBJECT_PLAYER = 0, STATE_HIT = 9, START_FRAME = 0x51, SLOT_FIRST = 5,
    K16_HALF = 0x40, K16_WORK_N = 0x0E, K16_Y_LO = 0xA3, K16_X_LO = 0xC4, K16_X_HI = 0xFF, K16_FRAME = 0x28,
    K16_SPEED_HI = 3, K16_ANGLE = 0x24, K16_TIMER = 0x44, K16_FLY_TIMER = 0x10, K16_VY_ENTRY = 0x0D,
    K16_VX_ENTRY = 0x0E, K16_WAIT_TIMER = 8, K16_NEXT_KIND = 0x16, K16_DRIFT_TIMER = 0xD0, RESET_SPEED_LO = 0x40,
    K16_CIRCLE_AT = 0x30, K16_CIRCLE_ANGLE = 0x38, K16_SCRIPT = 2, SCRIPT_EVERY = 0x1F,
    K22_HALF = 0x60, K22_WORK_N = 0x0D, K22_FRAME = 0x29, K22_SPEED_ADD = 0x18, K22_SOUND = 0x37,
    K15_KIND = 0x0F, K15_HALF = 0x20, K15_WORK_N = 0x0E, K15_FRAME = 0x26, K15_TIMER = 0x7F, K15_SCRIPT = 1,
    K15_SOUND = 0x46, K15_ANIM_TIMER = 0x40, K15_ANIMATION = 0x12, K15_HALF_H = 0x10, K15_HALF_W = 0x18,
    K15_SWOOP_VX_HI = 8, K15_SWOOP_FRAME = 0x27, K15_SWOOP_SOUND = 0x1E, K15_RIDE_WAIT = 4, K15_LIVE_HIT = 2,
    K15_RIDE_ADD = 0x80, K15_EDGE_RIGHT = 0x40, K15_EDGE_LEFT = 0xC0, K15_RAIN_TIMER = 0x80, K15_RAIN_EVERY = 7,
    K15_RAIN_FIRST = 6, K15_RAIN_END = 0x10, K15_PIECE_KIND = 0x0B, K15_RAIN_SOUND = 0x15,
    PAD_UP_DOWN = 0x50, PAD_UP = 0x10,
    K16_START = 0xD610, K16_FLY = 0xD666, K16_WAIT = 0xD682, K16_DRIFT = 0xD69E, K16_CIRCLE = 0xD6D9,
    K15_START = 0xD792, K15_FLY = 0xD817, K15_SWOOP = 0xD859, K15_RIDE = 0xD89B, K15_SPAWN = 0xD913, K15_RAIN = 0xD938
};

#define MariaKind15StartPlaces_TBL MariaKind15StartPlaces
#define MariaKind15StartPlacesLeft_TBL MariaKind15StartPlacesLeft
#define MariaKind15StartSpeedsX_TBL MariaKind15StartSpeedsX
#define MariaKind15StartSpeedsY_TBL MariaKind15StartSpeedsY
#define MariaKind15RideSpeedsHi_TBL MariaKind15RideSpeedsHi
#define MariaKind15RainPlaces_TBL MariaKind15RainPlaces

/* LDA #value, STA field, as the handlers set up an object. */
#define P_PUT(field, value) do { p_lda(value); WR8(field, cpu.a); } while (0)

/* ClearObject through the kernel's vector: a JSR returning to `ret`, or the tail JMP (ret 0). */
static void pj_clear_in_y(uint16_t ret)
{
    pj_in(ret);
    ClearObjectInY(cpu.y);
    pj_out(ret);
}

/* SetVelocityFromAngle through the kernel's vector at $4048: A the angle, X the object, zTemp00-01 the speed. */
static void pj_velocity_angle(uint16_t ret)
{
    pj_in(ret);
    regs_exit(regs_of_scale(SetVelocityFromAngle(cpu.a, cpu.x)));
    pj_out(ret);
}

/* The last two instructions of MariaObjectFromPlayer ($C390): her facing to the object's; a JSR there. */
static void pj_facing_from_player(uint16_t ret)
{
    pj_in(ret);
    p_lda(RD8(OBJ->facing_left[OBJECT_PLAYER]));
    WR8(OBJ->facing_left[cpu.x], cpu.a);
    pj_out(ret);
}

/* ------------------------------------------------------------------------------------------------ the flying steps */

/*
 * @routine 01E3:6C6 MariaObjectFlyAlong
 * @run 6E:D6C6
 * WHAT IT DOES: Moves an object along its angle at its speed: OBJ_SWOOP_SPEED_LO and _HI to zTemp00-01, OBJ_FLY_ANGLE
 * to SetVelocityFromAngle, then its velocity applied.
 * HOW: The loads and stores; SetVelocityFromAngle through the kernel's vector at $4048; the tail JMP to
 * MariaObjectApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: as MariaObjectApplyVelocity.
 */
void MariaObjectFlyAlong(void)
{
    p_lda(RD8(OBJ->swoop_speed_lo[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    p_lda(RD8(OBJ->swoop_speed_hi[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    p_lda(RD8(OBJ->fly_angle[cpu.x]));
    pj_velocity_angle(0xD6D6);
    pj_apply_velocity(0);
}

void abi_MariaObjectFlyAlong(void)
{
    MariaObjectFlyAlong();
}

/*
 * @routine 01E3:6B3 MariaObjectFlyReset
 * @run 6E:D6B3
 * WHAT IT DOES: Stops an object and sets it flying slowly at angle 0 (speed $0040), then its next state.
 * HOW: MariaObjectStop, the stores, the tail JMP to MariaObjectNextState.
 * WAITS: no.
 * Entry: X the object. Exit: as MariaObjectNextState.
 */
void MariaObjectFlyReset(void)
{
    pj_void_x(0xD6B6, MariaObjectStop);
    P_PUT(OBJ->swoop_speed_lo[cpu.x], RESET_SPEED_LO);
    P_PUT(OBJ->swoop_speed_hi[cpu.x], 0);
    WR8(OBJ->fly_angle[cpu.x], 0);
    pj_next_state(0);
}

void abi_MariaObjectFlyReset(void)
{
    MariaObjectFlyReset();
}

/* ------------------------------------------------------------------------------------------------ kind 16 */

/*
 * @routine 01E3:60D MariaKind16Clear
 * @run 6E:D60D
 * WHAT IT DOES: Clears kind 16's object when she is hit before it starts.
 * HOW: The tail JMP to ClearObject through the kernel's vector.
 * WAITS: no.
 * Entry: X the object. Exit: all registers and flags kept.
 */
void MariaKind16Clear(void)
{
    ClearObject(cpu.x);
}

void abi_MariaKind16Clear(void)
{
    MariaKind16Clear();
}

/*
 * @routine 01E3:610 MariaKind16Start
 * @run 6E:D610
 * WHAT IT DOES: Kind 16's state 0: cleared when she is hit; at her frame $51 it takes a hitbox $80 square, live,
 * OBJ_WORK_N $0E, the place $FFC4, $A3 (left of the screen), sprite set 1, frame $28, its mark, speed $0300 at angle
 * $24, timer $44, and its next state.
 * HOW: Her OBJ_STATE and OBJ_FRAME; the stores, MariaObjectMarkOn; the tail JMP to MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: waiting, A her frame, N, Z and C from comparing it with $51; else as the
 * routine jumped to.
 */
void MariaKind16Start(void)
{
    p_ldx_object();
    p_lda(RD8(OBJ->state[OBJECT_PLAYER]));
    p_cmp(cpu.a, STATE_HIT);
    if (p_z()) {
        MariaKind16Clear();
        return;
    }
    p_lda(RD8(OBJ->frame[OBJECT_PLAYER]));
    p_cmp(cpu.a, START_FRAME);
    if (!p_z()) {
        return;
    }
    p_lda(K16_HALF);
    WR8(OBJ->hitbox_half_h[cpu.x], cpu.a);
    WR8(OBJ->hitbox_half_w[cpu.x], cpu.a);
    P_PUT(OBJ->hitbox_on[cpu.x], 1);
    P_PUT(OBJ->work_n[cpu.x], K16_WORK_N);
    P_PUT(OBJ->y_lo[cpu.x], K16_Y_LO);
    P_PUT(OBJ->x_lo[cpu.x], K16_X_LO);
    P_PUT(OBJ->x_hi[cpu.x], K16_X_HI);
    P_PUT(OBJ->sprite_set[cpu.x], 1);
    P_PUT(OBJ->frame[cpu.x], K16_FRAME);
    pj_mark_on(0xD64E);
    P_PUT(OBJ->swoop_speed_lo[cpu.x], 0);
    P_PUT(OBJ->swoop_speed_hi[cpu.x], K16_SPEED_HI);
    P_PUT(OBJ->fly_angle[cpu.x], K16_ANGLE);
    P_PUT(OBJ->hurt_timer[cpu.x], K16_TIMER);
    pj_next_state(0);
}

void abi_MariaKind16Start(void)
{
    MariaKind16Start();
}

/*
 * @routine 01E3:666 MariaKind16Fly
 * @run 6E:D666
 * WHAT IT DOES: Kind 16's state 1: flies along its angle, turning by two a frame; when its timer runs out it stops,
 * the timer $10, and its next state.
 * HOW: MariaObjectFlyAlong, two DECs of OBJ_FLY_ANGLE, DEC of the timer; MariaObjectStop, MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: N and Z from the timer, or as MariaObjectNextState.
 */
void MariaKind16Fly(void)
{
    p_ldx_object();
    pj_proc(0xD66B, MariaObjectFlyAlong);
    abi_nz(DEC8(OBJ->fly_angle[cpu.x]));
    abi_nz(DEC8(OBJ->fly_angle[cpu.x]));
    abi_nz(DEC8(OBJ->hurt_timer[cpu.x]));
    if (!p_z()) {
        return;
    }
    P_PUT(OBJ->hurt_timer[cpu.x], K16_FLY_TIMER);
    pj_void_x(0xD67E, MariaObjectStop);
    pj_next_state(0xD681);
}

void abi_MariaKind16Fly(void)
{
    MariaKind16Fly();
}

/*
 * @routine 01E3:682 MariaKind16Wait
 * @run 6E:D682
 * WHAT IT DOES: Kind 16's state 2: waits out its timer; then the Y and X speeds of entries $0D and $0E, turned the
 * way it faces, the timer 8, and its next state.
 * HOW: DEC of the timer; MariaSetVelocityYFromTable, MariaSetVelocityXFromTable, MariaObjectFacingVelocity; the tail
 * JMP to MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: N and Z from the timer, or as MariaObjectNextState.
 */
void MariaKind16Wait(void)
{
    p_ldx_object();
    abi_nz(DEC8(OBJ->hurt_timer[cpu.x]));
    if (!p_z()) {
        return;
    }
    p_lda(K16_VY_ENTRY);
    pj_velocity_y(0xD68E);
    p_lda(K16_VX_ENTRY);
    pj_velocity_x(0xD693);
    pj_facing_velocity(0xD696);
    P_PUT(OBJ->hurt_timer[cpu.x], K16_WAIT_TIMER);
    pj_next_state(0);
}

void abi_MariaKind16Wait(void)
{
    MariaKind16Wait();
}

/*
 * @routine 01E3:69E MariaKind16Drift
 * @run 6E:D69E
 * WHAT IT DOES: Kind 16's state 3: moves by its velocity; when its timer runs out, slot 6's kind becomes $16, its
 * timer $D0, and it flies again slowly (MariaObjectFlyReset).
 * HOW: MariaObjectApplyVelocity, DEC of the timer, the stores; on into MariaObjectFlyReset.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: N and Z from the timer, or as MariaObjectFlyReset.
 */
void MariaKind16Drift(void)
{
    p_ldx_object();
    pj_apply_velocity(0xD6A3);
    abi_nz(DEC8(OBJ->hurt_timer[cpu.x]));
    if (!p_z()) {
        return;
    }
    P_PUT(OBJ->kind[6], K16_NEXT_KIND);
    P_PUT(OBJ->hurt_timer[cpu.x], K16_DRIFT_TIMER);
    MariaObjectFlyReset();
}

void abi_MariaKind16Drift(void)
{
    MariaKind16Drift();
}

/*
 * @routine 01E3:6D9 MariaKind16Circle
 * @run 6E:D6D9
 * WHAT IT DOES: Kind 16's state 4: flies along its angle, turning the other way by two a frame; at timer $30 its speed
 * becomes $0340 at angle $38; every $20 frames script 2; when the timer runs out the object and the next slot are
 * cleared.
 * HOW: MariaObjectFlyAlong, INCs of OBJ_FLY_ANGLE, DEC of the timer, MariaStartScriptSlot3; ClearObject (the second by
 * the tail JMP) through the kernel's vector.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last step made.
 */
void MariaKind16Circle(void)
{
    p_ldx_object();
    pj_proc(0xD6DE, MariaObjectFlyAlong);
    p_lda(RD8(OBJ->hurt_timer[cpu.x]));
    p_cmp(cpu.a, K16_CIRCLE_AT);
    if (p_z()) {
        P_PUT(OBJ->swoop_speed_lo[cpu.x], RESET_SPEED_LO);
        P_PUT(OBJ->swoop_speed_hi[cpu.x], K16_SPEED_HI);
        P_PUT(OBJ->fly_angle[cpu.x], K16_CIRCLE_ANGLE);
    }
    abi_nz(INC8(OBJ->fly_angle[cpu.x]));
    abi_nz(INC8(OBJ->fly_angle[cpu.x]));
    abi_nz(DEC8(OBJ->hurt_timer[cpu.x]));
    p_lda((uint8_t)(RD8(OBJ->hurt_timer[cpu.x]) & SCRIPT_EVERY));
    p_cmp(cpu.a, 1);
    if (p_z()) {
        p_lda(K16_SCRIPT);
        pj_script_slot3(0xD70B);
    }
    p_lda(RD8(OBJ->hurt_timer[cpu.x]));
    if (!p_z()) {
        return;
    }
    p_ldx((uint8_t)(cpu.x + 1));
    pj_clear_object(0xD717);
    p_ldx((uint8_t)(cpu.x - 1));
    pj_clear_object(0);
}

void abi_MariaKind16Circle(void)
{
    MariaKind16Circle();
}

/*
 * @routine 01E3:5FD MariaKind16
 * @run 6E:D5FD
 * WHAT IT DOES: The handler of kind 16: its state's routine from MariaKind16States (start, fly, wait, drift, circle).
 * HOW: MariaStateDoubled, then the JMP (table,X).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
 */
void MariaKind16(void)
{
    uint16_t target;

    pj_state_doubled(0xD600);
    target = p_table_word(MariaKind16States);
    switch (target) {
    case K16_START:
        MariaKind16Start();
        break;
    case K16_FLY:
        MariaKind16Fly();
        break;
    case K16_WAIT:
        MariaKind16Wait();
        break;
    case K16_DRIFT:
        MariaKind16Drift();
        break;
    case K16_CIRCLE:
        MariaKind16Circle();
        break;
    default:
        p_jump_elsewhere(target);
        break;
    }
}

void abi_MariaKind16(void)
{
    MariaKind16();
}

/* ------------------------------------------------------------------------------------------------ kind 22 */

/*
 * @routine 01E3:71B MariaKind22
 * @run 6E:D71B
 * WHAT IT DOES: The handler of kind 22. State 0: a hitbox $C0 square, live, OBJ_WORK_N $0D, sprite set 1, frame $29,
 * its mark, the Y and X speeds of entries $0D and $0E turned the way it faces, and its next state. After: it moves,
 * and its Y speed grows by $0018 a frame and is its X speed too; once off the screen sideways (X high not zero) sound
 * $37 and its X and Y high bytes cleared.
 * HOW: OBJ_STATE; the stores, MariaObjectMarkOn, the velocity tables, MariaObjectFacingVelocity, MariaObjectNextState;
 * MariaObjectApplyVelocity, the 16-bit add (CLA and ADC for the high byte), SoundCommand.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last step made.
 */
void MariaKind22(void)
{
    Sum s;

    p_ldx_object();
    p_lda(RD8(OBJ->state[cpu.x]));
    if (p_z()) {
        p_ldx_object();
        p_lda(K22_HALF);
        WR8(OBJ->hitbox_half_h[cpu.x], cpu.a);
        WR8(OBJ->hitbox_half_w[cpu.x], cpu.a);
        P_PUT(OBJ->hitbox_on[cpu.x], 1);
        P_PUT(OBJ->work_n[cpu.x], K22_WORK_N);
        P_PUT(OBJ->sprite_set[cpu.x], 1);
        P_PUT(OBJ->frame[cpu.x], K22_FRAME);
        pj_mark_on(0xD743);
        p_lda(K16_VY_ENTRY);
        pj_velocity_y(0xD748);
        p_lda(K16_VX_ENTRY);
        pj_velocity_x(0xD74D);
        pj_facing_velocity(0xD750);
        pj_next_state(0);
        return;
    }
    pj_apply_velocity(0xD756);
    s = add_bytes(K22_SPEED_ADD, RD8(OBJ->vy_lo[cpu.x]), false);
    p_sum(s);
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    cpu.a = 0;
    p_adc(RD8(OBJ->vy_hi[cpu.x]));
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    p_lda(RD8(OBJ->x_hi[cpu.x]));
    if (p_z()) {
        return;
    }
    p_lda(K22_SOUND);
    pj_sound(0xD776);
    WR8(OBJ->x_hi[cpu.x], 0);
    WR8(OBJ->y_hi[cpu.x], 0);
}

void abi_MariaKind22(void)
{
    MariaKind22();
}

/* ------------------------------------------------------------------------------------------------ kind 15 */

/*
 * @routine 01E3:78F MariaKind15Clear
 * @run 6E:D78F
 * WHAT IT DOES: Clears kind 15's object when she is hit before it starts.
 * HOW: The tail JMP to ClearObject through the kernel's vector.
 * WAITS: no.
 * Entry: X the object. Exit: all registers and flags kept.
 */
void MariaKind15Clear(void)
{
    ClearObject(cpu.x);
}

void abi_MariaKind15Clear(void)
{
    MariaKind15Clear();
}

/*
 * @routine 01E3:792 MariaKind15Start
 * @run 6E:D792
 * WHAT IT DOES: Kind 15's state 0: cleared when she is hit; at her frame $51, from slot 5 slots 6 to 8 become kind 15
 * too; each takes her facing, a hitbox $40 square, live, OBJ_WORK_N $0E, its place by slot (MariaKind15StartPlaces,
 * or ...Left when it faces left) and speeds (MariaKind15StartSpeedsX and Y) turned the way it faces, sprite set 1,
 * frame $26, timer $7F, and its next state.
 * HOW: Her OBJ_STATE and OBJ_FRAME; the stores; the tables by slot less 5 doubled; MariaObjectFacingVelocity; the
 * tail JMP to MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: waiting, A her frame, N, Z and C from comparing it with $51; else as the
 * routine jumped to.
 */
void MariaKind15Start(void)
{
    uint8_t index;

    p_ldx_object();
    p_lda(RD8(OBJ->state[OBJECT_PLAYER]));
    p_cmp(cpu.a, STATE_HIT);
    if (p_z()) {
        MariaKind15Clear();
        return;
    }
    p_lda(RD8(OBJ->frame[OBJECT_PLAYER]));
    p_cmp(cpu.a, START_FRAME);
    if (!p_z()) {
        return;
    }
    p_cmp(cpu.x, SLOT_FIRST);
    if (p_z()) {
        p_lda(K15_KIND);
        WR8(OBJ->kind[6], cpu.a);
        WR8(OBJ->kind[7], cpu.a);
        WR8(OBJ->kind[8], cpu.a);
    }
    pj_facing_from_player(0xD7B5);
    p_lda(K15_HALF);
    WR8(OBJ->hitbox_half_h[cpu.x], cpu.a);
    WR8(OBJ->hitbox_half_w[cpu.x], cpu.a);
    P_PUT(OBJ->hitbox_on[cpu.x], 1);
    P_PUT(OBJ->work_n[cpu.x], K15_WORK_N);
    p_lda(cpu.x);
    abi_flag(P_C, true);
    p_sbc(SLOT_FIRST);
    p_asl();
    p_ldy(cpu.a);
    index = cpu.y;
    P_PUT(OBJ->x_lo[cpu.x], TBL8(MariaKind15StartPlaces, index));
    P_PUT(OBJ->y_lo[cpu.x], TBL8(MariaKind15StartPlaces, index + 1));
    p_lda(RD8(OBJ->facing_left[cpu.x]));
    if (!p_z()) {
        P_PUT(OBJ->x_lo[cpu.x], TBL8(MariaKind15StartPlacesLeft, index));
        P_PUT(OBJ->y_lo[cpu.x], TBL8(MariaKind15StartPlacesLeft, index + 1));
    }
    P_PUT(OBJ->vx_lo[cpu.x], TBL8(MariaKind15StartSpeedsX, index));
    P_PUT(OBJ->vx_hi[cpu.x], TBL8(MariaKind15StartSpeedsX, index + 1));
    P_PUT(OBJ->vy_lo[cpu.x], TBL8(MariaKind15StartSpeedsY, index));
    P_PUT(OBJ->vy_hi[cpu.x], TBL8(MariaKind15StartSpeedsY, index + 1));
    pj_facing_velocity(0xD805);
    P_PUT(OBJ->sprite_set[cpu.x], 1);
    P_PUT(OBJ->frame[cpu.x], K15_FRAME);
    P_PUT(OBJ->hurt_timer[cpu.x], K15_TIMER);
    pj_next_state(0);
}

void abi_MariaKind15Start(void)
{
    MariaKind15Start();
}

/*
 * @routine 01E4:017 MariaKind15Fly
 * @run 6E:D817
 * WHAT IT DOES: Kind 15's state 1: moves and keeps its mark; every $20 frames script 1 and sound $46; when its timer
 * runs out, its animation timer $40, animation $12, and it flies again slowly; outside slot 5 it then shows nothing and
 * its hitbox is off. Its hitbox is $20 high, $30 wide.
 * HOW: MariaObjectApplyVelocity, MariaObjectMarkOn, MariaStartScriptSlot3, SoundCommand; DEC of the timer;
 * MariaObjectSetAnimation, MariaObjectFlyReset.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: A $18, N and Z from it; the rest as the last call left it.
 */
void MariaKind15Fly(void)
{
    p_ldx_object();
    pj_apply_velocity(0xD81C);
    pj_mark_on(0xD81F);
    p_lda((uint8_t)(RD8(OBJ->hurt_timer[cpu.x]) & SCRIPT_EVERY));
    p_cmp(cpu.a, 1);
    if (p_z()) {
        p_lda(K15_SCRIPT);
        pj_script_slot3(0xD82D);
        p_lda(K15_SOUND);
        pj_sound(0xD832);
    }
    abi_nz(DEC8(OBJ->hurt_timer[cpu.x]));
    if (p_z()) {
        P_PUT(OBJ->anim_timer[cpu.x], K15_ANIM_TIMER);
        p_lda(K15_ANIMATION);
        pj_set_animation(0xD841);
        pj_proc(0xD844, MariaObjectFlyReset);
        p_cmp(cpu.x, SLOT_FIRST);
        if (!p_z()) {
            WR8(OBJ->frame[cpu.x], 0);
            WR8(OBJ->hitbox_on[cpu.x], 0);
        }
    }
    P_PUT(OBJ->hitbox_half_h[cpu.x], K15_HALF_H);
    P_PUT(OBJ->hitbox_half_w[cpu.x], K15_HALF_W);
}

void abi_MariaKind15Fly(void)
{
    MariaKind15Fly();
}

/*
 * @routine 01E4:059 MariaKind15Swoop
 * @run 6E:D859
 * WHAT IT DOES: Kind 15's state 2: slot 5 drops its mark and runs its animation; it flies along its angle, turning by
 * two a frame; when its animation timer runs out, that timer is set by slot (slot less 5), it stops, X speed $08xx the
 * way it faces, script 1, frame $27, no hitter, sound $1E, and its next state.
 * HOW: MariaObjectMarkOff, MariaObjectAnimationTick, MariaObjectFlyAlong, DECs; MariaObjectStop,
 * MariaObjectFacingVelocity, MariaStartScriptSlot3, SoundCommand, the tail JMP to MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: N and Z from the animation timer, or as MariaObjectNextState.
 */
void MariaKind15Swoop(void)
{
    p_ldx_object();
    p_cmp(cpu.x, SLOT_FIRST);
    if (p_z()) {
        pj_mark_off(0xD862);
        pj_animation_tick(0xD865);
    }
    pj_proc(0xD868, MariaObjectFlyAlong);
    abi_nz(DEC8(OBJ->fly_angle[cpu.x]));
    abi_nz(DEC8(OBJ->fly_angle[cpu.x]));
    abi_nz(DEC8(OBJ->anim_timer[cpu.x]));
    if (!p_z()) {
        return;
    }
    p_lda(cpu.x);
    abi_flag(P_C, true);
    p_sbc(SLOT_FIRST);
    WR8(OBJ->anim_timer[cpu.x], cpu.a);
    pj_void_x(0xD87E, MariaObjectStop);
    P_PUT(OBJ->vx_hi[cpu.x], K15_SWOOP_VX_HI);
    pj_facing_velocity(0xD886);
    p_lda(K15_SCRIPT);
    pj_script_slot3(0xD88B);
    P_PUT(OBJ->frame[cpu.x], K15_SWOOP_FRAME);
    WR8(OBJ->hit_by_slot[cpu.x], 0);
    p_lda(K15_SWOOP_SOUND);
    pj_sound(0xD898);
    pj_next_state(0);
}

void abi_MariaKind15Swoop(void)
{
    MariaKind15Swoop();
}

/*
 * @routine 01E4:0FB MariaObjectSteerByPad
 * @run 6E:D8FB
 * WHAT IT DOES: With up or down held, the object's Y speed's high byte goes one less (up) or one more (down).
 * HOW: zPadHeld's bits 4 and 6; the ADC.
 * WAITS: no.
 * Entry: X the object. Exit: A the AND or the new high byte, N and Z from it; C and V from the add. X and Y kept.
 */
void MariaObjectSteerByPad(void)
{
    p_lda((uint8_t)(RD8(ZP->pad_held) & PAD_UP_DOWN));
    if (p_z()) {
        return;
    }
    p_lda((uint8_t)(cpu.a & PAD_UP));
    p_lda(p_z() ? 0x01 : 0xFF);
    abi_flag(P_C, false);
    p_adc(RD8(OBJ->vy_hi[cpu.x]));
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
}

void abi_MariaObjectSteerByPad(void)
{
    MariaObjectSteerByPad();
}

/* Kind 15's ride ($D8C4 on): it moves, speeds up the way it faces, is steered, and leaves by the far side. */
static void kind15_ride_on(void)
{
    Sum s;

    pj_move(0xD8C7);
    p_ldy(RD8(OBJ->facing_left[cpu.x]));
    s = add_bytes(K15_RIDE_ADD, RD8(OBJ->vx_lo[cpu.x]), false);
    p_sum(s);
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    p_lda(TBL8(MariaKind15RideSpeedsHi, cpu.y));
    p_adc(RD8(OBJ->vx_hi[cpu.x]));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    pj_proc(0xD8DF, MariaObjectSteerByPad);
    p_lda(RD8(OBJ->x_hi[cpu.x]));
    if (p_z()) {
        return;
    }
    p_lda(RD8(OBJ->vx_hi[cpu.x]));
    if (!p_n()) {
        p_lda(RD8(OBJ->x_lo[cpu.x]));
        p_cmp(cpu.a, K15_EDGE_RIGHT);
        if (!p_c()) {
            return;
        }
        pj_clear_object(0);
        return;
    }
    p_lda(RD8(OBJ->x_lo[cpu.x]));
    p_cmp(cpu.a, K15_EDGE_LEFT);
    if (!p_c()) {
        pj_clear_object(0);
    }
}

/*
 * @routine 01E4:09B MariaKind15Ride
 * @run 6E:D89B
 * WHAT IT DOES: Kind 15's state 3: slot 5, once the object its hit found has a live hitbox (2 or more) and health not
 * negative, waits 4 frames, stops, and goes to its next state. Else while its animation timer runs nothing; then it
 * rides: moves, X speed up by $80 and the way it faces (MariaKind15RideSpeedsHi), steered by the pad, and cleared once
 * past $40 on the right or under $C0 on the left of the next screen.
 * HOW: OBJ_HIT_BY_SLOT, that object's OBJ_HITBOX_ON and OBJ_HEALTH; MariaObjectStop, MariaObjectNextState; DEC of the
 * timer; MariaObjectMove, the 16-bit add, MariaObjectSteerByPad, ClearObject by the tail JMP.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last step made.
 */
void MariaKind15Ride(void)
{
    p_ldx_object();
    p_cmp(cpu.x, SLOT_FIRST);
    if (p_z()) {
        p_ldy(RD8(OBJ->hit_by_slot[cpu.x]));
        p_lda(RD8(OBJ->hitbox_on[cpu.y]));
        p_cmp(cpu.a, K15_LIVE_HIT);
        if (p_c()) {
            p_lda(RD8(OBJ->health[cpu.y]));
            if (!p_n()) {
                P_PUT(OBJ->anim_timer[cpu.x], K15_RIDE_WAIT);
                pj_void_x(0xD8B8, MariaObjectStop);
                pj_next_state(0);
                return;
            }
        }
    }
    p_lda(RD8(OBJ->anim_timer[cpu.x]));
    if (!p_z()) {
        abi_nz(DEC8(OBJ->anim_timer[cpu.x]));
        return;
    }
    kind15_ride_on();
}

void abi_MariaKind15Ride(void)
{
    MariaKind15Ride();
}

/*
 * @routine 01E4:113 MariaKind15Spawn
 * @run 6E:D913
 * WHAT IT DOES: Kind 15's state 4: rides until its animation timer runs out; then slots 6 to 8 are emptied, the timer
 * $80, its mark, it stops, and its next state.
 * HOW: DEC of the timer, the JMP into MariaKind15Ride's ride; ClearObjectInY three times through the kernel's vector;
 * MariaObjectMarkOn, MariaObjectStop, the tail JMP to MariaObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the ride, or MariaObjectNextState.
 */
void MariaKind15Spawn(void)
{
    p_ldx_object();
    abi_nz(DEC8(OBJ->anim_timer[cpu.x]));
    if (!p_z()) {
        kind15_ride_on();
        return;
    }
    p_ldy(K15_RAIN_FIRST);
    pj_clear_in_y(0xD922);
    p_ldy((uint8_t)(cpu.y + 1));
    pj_clear_in_y(0xD926);
    p_ldy((uint8_t)(cpu.y + 1));
    pj_clear_in_y(0xD92A);
    P_PUT(OBJ->anim_timer[cpu.x], K15_RAIN_TIMER);
    pj_mark_on(0xD932);
    pj_void_x(0xD935, MariaObjectStop);
    pj_next_state(0);
}

void abi_MariaKind15Spawn(void)
{
    MariaKind15Spawn();
}

/*
 * @routine 01E4:138 MariaKind15Rain
 * @run 6E:D938
 * WHAT IT DOES: Kind 15's state 5: moves; every eighth frame, in the first empty slot of 6 to 15, a piece of kind 11 at
 * its place plus MariaKind15RainPlaces by OBJ_RAIN_COUNT, with sound $15, and the count on; when its timer runs out it
 * is cleared.
 * HOW: MariaObjectMove; OBJ_KIND by Y; ClearObjectInY through the kernel's vector; the 16-bit adds of the place with X
 * on the stack; SoundCommand; DEC of the timer, ClearObject by the tail JMP.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: N and Z from the timer, or as ClearObject.
 */
void MariaKind15Rain(void)
{
    uint8_t place;

    p_ldx_object();
    pj_move(0xD93D);
    p_lda((uint8_t)(RD8(OBJ->anim_timer[cpu.x]) & K15_RAIN_EVERY));
    if (p_z()) {
        p_ldy(K15_RAIN_FIRST);
        for (;;) {
            p_lda(RD8(OBJ->kind[cpu.y]));
            if (p_z()) {
                pj_clear_in_y(0xD95C);
                P_PUT(OBJ->kind[cpu.y], K15_PIECE_KIND);
                push8(cpu.x);
                p_lda((uint8_t)(RD8(OBJ->rain_count[cpu.x]) & 0x07));
                p_asl();
                p_asl();
                p_ldx(cpu.a);
                place = cpu.x;
                P_PUT(OBJ->x_lo[cpu.y], TBL8(MariaKind15RainPlaces, place));
                P_PUT(OBJ->x_hi[cpu.y], TBL8(MariaKind15RainPlaces, place + 1));
                P_PUT(OBJ->y_lo[cpu.y], TBL8(MariaKind15RainPlaces, place + 2));
                P_PUT(OBJ->y_hi[cpu.y], TBL8(MariaKind15RainPlaces, place + 3));
                p_ldx(pull8());
                p_lda(RD8(OBJ->x_lo[cpu.x]));
                abi_flag(P_C, false);
                p_adc(RD8(OBJ->x_lo[cpu.y]));
                WR8(OBJ->x_lo[cpu.y], cpu.a);
                p_lda(RD8(OBJ->x_hi[cpu.x]));
                p_adc(RD8(OBJ->x_hi[cpu.y]));
                WR8(OBJ->x_hi[cpu.y], cpu.a);
                p_lda(RD8(OBJ->y_lo[cpu.x]));
                abi_flag(P_C, false);
                p_adc(RD8(OBJ->y_lo[cpu.y]));
                WR8(OBJ->y_lo[cpu.y], cpu.a);
                p_lda(RD8(OBJ->y_hi[cpu.x]));
                p_adc(RD8(OBJ->y_hi[cpu.y]));
                WR8(OBJ->y_hi[cpu.y], cpu.a);
                p_lda(K15_RAIN_SOUND);
                pj_sound(0xD9AE);
                abi_nz(INC8(OBJ->rain_count[cpu.x]));
                break;
            }
            p_ldy((uint8_t)(cpu.y + 1));
            p_cmp(cpu.y, K15_RAIN_END);
            if (p_z()) {
                break;
            }
        }
    }
    abi_nz(DEC8(OBJ->anim_timer[cpu.x]));
    if (!p_z()) {
        return;
    }
    pj_clear_object(0);
}

void abi_MariaKind15Rain(void)
{
    MariaKind15Rain();
}

/*
 * @routine 01E3:77D MariaKind15
 * @run 6E:D77D
 * WHAT IT DOES: The handler of kind 15: its state's routine from MariaKind15States (start, fly, swoop, ride, spawn,
 * rain).
 * HOW: MariaStateDoubled, then the JMP (table,X).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
 */
void MariaKind15(void)
{
    uint16_t target;

    pj_state_doubled(0xD780);
    target = p_table_word(MariaKind15States);
    switch (target) {
    case K15_START:
        MariaKind15Start();
        break;
    case K15_FLY:
        MariaKind15Fly();
        break;
    case K15_SWOOP:
        MariaKind15Swoop();
        break;
    case K15_RIDE:
        MariaKind15Ride();
        break;
    case K15_SPAWN:
        MariaKind15Spawn();
        break;
    case K15_RAIN:
        MariaKind15Rain();
        break;
    default:
        p_jump_elsewhere(target);
        break;
    }
}

void abi_MariaKind15(void)
{
    MariaKind15();
}
