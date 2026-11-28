/*
 * Maria in the air (01DE, bank $6D): the jump (states 3, 11 and 12), the fall (state 4), the hard landing (state $0A),
 * her second jump in the air, and how she lands: on the floor, on a marked floor (the ledge marks bits 2 and 5 of a
 * column), or onto a ledge's hold from the jump.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"

enum {
    OBJECT_PLAYER = 0,
    STATE_CROUCH = 2, STATE_JUMP = 3, STATE_LANDED = 0x0A, STATE_LANDING_LOW = 0x0D,
    STEP_SECOND_JUMP = 4,
    ANIMATION_LANDED = 0x10, ANIMATION_SECOND_JUMP = 0x0C, ANIMATION_SECOND_JUMP_UP = 0x14, ANIMATION_HOVER = 0x11,
    FRAME_CROUCHED = 0x1B, HOVER_STEPS = 4,
    SOUND_LANDED = 2, HARD_LANDING = 6, LANDING_SPEED_TOP = 7,
    SECOND_JUMP_VY_HI = 0xFD, FALL_GRAVITY = 0x48, LIGHT_GRAVITY = 0x30, FALL_SPEED_TOP = 7,
    MARK_FLOOR = 0x24, MARK_DOWN_ONLY = 0x20, MARK_TOO_HIGH = 0x10, FEET_ROW_LIMIT = 3, FEET_FALL_VY_HI = 3,
    PAD_UP = 0x10, PAD_RIGHT = 0x20, PAD_LEFT = 0x80,
    LEDGE_ACROSS = 8, LEDGE_BACK = 0xF8, LEDGE_LOW_BELOW = 0x0E, LEDGE_FAR_BELOW = 0x1E, LEDGE_FLOOR_BELOW = 0x16,
    LEDGE_BIT = 0x04, HOLD_STOPPED = 0xFF, LEDGE_BEHIND_POSES = 2
};

#define MariaLedgeStates_TBL MariaLedgeStates
#define MariaLedgeFrames_TBL MariaLedgeFrames
#define MariaLedgeAnimations_TBL MariaLedgeAnimations
#define MariaLedgeNudgeX_TBL MariaLedgeNudgeX
#define MariaLedgeNudgeY_TBL MariaLedgeNudgeY
#define MariaLedgeNudgeXBehind_TBL MariaLedgeNudgeXBehind
#define MariaLedgeNudgeYBehind_TBL MariaLedgeNudgeYBehind
#define MariaFallDriftSpeeds_TBL MariaFallDriftSpeeds

/* ------------------------------------------------------------------------------------------------ gravity */

/*
 * @routine 01DE:1F0 MariaGravityLight
 * @run 6D:A9F0
 * WHAT IT DOES: Light gravity: her Y speed $0030 more, held to $07xx on the way down (the low byte cleared there).
 * HOW: A 16-bit add; a high byte of 7 or more, not negative, clears the low.
 * WAITS: no.
 * Entry: nothing. Exit: A the high byte, N and Z from it, or from comparing it with 7; C the add's, or the compare's;
 * V the add's. X and Y kept.
 */
Regs MariaGravityLight(void)
{
    uint8_t value = RD8(OBJ->vy_lo[OBJECT_PLAYER]);
    Sum lo = add_bytes(value, LIGHT_GRAVITY, false);
    Sum hi;
    Regs r;

    WR8(OBJ->vy_lo[OBJECT_PLAYER], lo.value);
    value = RD8(OBJ->vy_hi[OBJECT_PLAYER]);
    hi = add_bytes(value, 0, lo.carry);
    WR8(OBJ->vy_hi[OBJECT_PLAYER], hi.value);
    r = r_v(r_c(r_lda(regs_kept(), hi.value), hi.carry), hi.overflow);
    if (hi.value & 0x80) {
        return r;
    }
    r = r_cmp(r, hi.value, FALL_SPEED_TOP);
    if (hi.value >= FALL_SPEED_TOP) {
        WR8(OBJ->vy_lo[OBJECT_PLAYER], 0);
    }
    return r;
}

void abi_MariaGravityLight(void)
{
    regs_exit(MariaGravityLight());
}

/*
 * @routine 01DE:440 MariaJumpGravity
 * @run 6D:AC40
 * WHAT IT DOES: Her jump's gravity, except in the hover: animation $11 on the way down in its first four steps.
 * HOW: OBJ_ANIMATION, the Y speed's sign and OBJ_SCROLL_BAND (the animation's step); the tail JMP to MariaFallGravity.
 * WAITS: no.
 * Entry: nothing. Exit: hovering, A the step, N, Z and C from comparing it with 4; else as MariaFallGravity.
 */
MExit MariaJumpGravity(void)
{
    MExit e = mx_none();
    uint8_t value = RD8(OBJ->animation[OBJECT_PLAYER]);

    mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, ANIMATION_HOVER));
    if (value == ANIMATION_HOVER) {
        value = RD8(OBJ->vy_hi[OBJECT_PLAYER]);
        mx_regs(&e, r_lda(regs_kept(), value));
        if (!(value & 0x80)) {
            value = RD8(OBJ->scroll_band[OBJECT_PLAYER]);
            mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, HOVER_STEPS));
            if (value < HOVER_STEPS) {
                return e;
            }
        }
    }
    mx_gravity(&e, MariaFallGravity());
    return e;
}

void abi_MariaJumpGravity(void)
{
    mx_exit(MariaJumpGravity());
}

/* ------------------------------------------------------------------------------------------------ the ledge from a jump */

/*
 * @routine 01DE:36B MariaLedgePose
 * @run 6D:AB6B
 * WHAT IT DOES: Gives her the hold on a ledge for a pose: frame, state and OBJ_ANIMATION from her tables.
 * HOW: MariaLedgeFrames, MariaLedgeStates, MariaLedgeAnimations by X.
 * WAITS: no.
 * Entry: X the pose (her facing, plus 2 for the ledge behind her). Exit: A the animation, N and Z from it. X, Y, C and V
 * kept.
 */
Regs MariaLedgePose(uint8_t pose)
{
    uint8_t value = TBL8(MariaLedgeFrames, pose);

    WR8(OBJ->frame[OBJECT_PLAYER], value);
    value = TBL8(MariaLedgeStates, pose);
    WR8(OBJ->state[OBJECT_PLAYER], value);
    value = TBL8(MariaLedgeAnimations, pose);
    WR8(OBJ->animation[OBJECT_PLAYER], value);
    return r_lda(regs_kept(), value);
}

void abi_MariaLedgePose(void)
{
    regs_exit(MariaLedgePose(cpu.x));
}

/*
 * @routine 01DE:37E MariaLedgeHoldAt
 * @run 6D:AB7E
 * WHAT IT DOES: Puts her Y at the ledge's, her animation stopped and step 0, and loads her facing.
 * HOW: The stores, then LDX of OBJ_FACING_LEFT.
 * WAITS: no.
 * Entry: A her new Y's low byte. Exit: A $FF, X the facing, N and Z from it. Y, C and V kept.
 */
Regs MariaLedgeHoldAt(uint8_t y_lo)
{
    WR8(OBJ->y_lo[OBJECT_PLAYER], y_lo);
    WR8(OBJ->hurt_timer[OBJECT_PLAYER], HOLD_STOPPED);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    return r_ldx(r_a_only(regs_kept(), HOLD_STOPPED), RD8(OBJ->facing_left[OBJECT_PLAYER]));
}

void abi_MariaLedgeHoldAt(void)
{
    regs_exit(MariaLedgeHoldAt(cpu.a));
}

/*
 * @routine 01DE:3E2 MariaLedgeFloorBelow
 * @run 6D:ABE2
 * WHAT IT DOES: Reads the terrain at her X, $16 rows below her, for bit 2.
 * HOW: zTemp00 her X; MariaReadBelow with her Y plus $16 and its carry; AND #4.
 * WAITS: no.
 * Entry: X and Y the caller's (the read's handler is entered with them). Exit: A bit 2 of the read, N and Z from it;
 * Y, C and V as MariaReadBelow left them. X kept.
 */
MExit MariaLedgeFloorBelow(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value = RD8(OBJ->x_lo[OBJECT_PLAYER]);
    Sum row;

    WR8(ZP->temp00, value);
    value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    row = add_bytes(value, LEDGE_FLOOR_BELOW, false);
    mx_regs(&e, r_v(r_c(r_lda(regs_kept(), row.value), row.carry), row.overflow));
    cpu.x = x;
    cpu.y = y;
    jsr_push(0xABF2);
    mx_read_below(&e, MariaReadBelow(row.value, row.carry), row.carry);
    jsr_pop();
    mx_regs(&e, r_lda(regs_kept(), (uint8_t)(mx_a_from(x, y, e) & LEDGE_BIT)));
    return e;
}

void abi_MariaLedgeFloorBelow(void)
{
    mx_exit(MariaLedgeFloorBelow(cpu.x, cpu.y));
}

/* A read for bit 2 at her X plus an offset and her Y plus rows, at a JSR returning to `ret`. */
static uint8_t ledge_read(MExit *e, uint8_t x, uint8_t y, uint8_t across, uint8_t rows, uint16_t ret)
{
    uint8_t value = RD8(OBJ->x_lo[OBJECT_PLAYER]);
    Sum point = add_bytes(value, across, false);
    Sum row;
    uint8_t answer;

    WR8(ZP->temp00, point.value);
    value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    row = add_bytes(value, rows, false);
    mx_regs(e, r_v(r_c(r_lda(regs_kept(), row.value), row.carry), row.overflow));
    cpu.x = mx_x_from(x, y, *e);
    cpu.y = mx_y_from(x, y, *e);
    jsr_push(ret);
    mx_read_below(e, MariaReadBelow(row.value, row.carry), row.carry);
    jsr_pop();
    answer = (uint8_t)(mx_a_from(x, y, *e) & LEDGE_BIT);
    mx_regs(e, r_lda(regs_kept(), answer));
    return answer;
}

/*
 * Onto the ledge's hold: her X and Y moved by the tables by column 0's pixel, the pose (her facing, plus 2 for the
 * ledge behind her), and, with no floor of bit 2 below the hold, her Y back up by the table's row.
 */
static void ledge_hold(MExit *e, uint8_t x, uint8_t y, bool behind, uint16_t hold_ret, uint16_t pose_ret,
                       uint16_t floor_ret)
{
    uint8_t pixel = RD8(PVARS->column_x[0]);
    uint8_t value, base, pose;
    Sum moved;
    MExit floor;

    value = behind ? TBL8(MariaLedgeNudgeXBehind, pixel) : TBL8(MariaLedgeNudgeX, pixel);
    base = RD8(OBJ->x_lo[OBJECT_PLAYER]);
    moved = add_bytes(value, base, false);
    WR8(OBJ->x_lo[OBJECT_PLAYER], moved.value);
    value = behind ? TBL8(MariaLedgeNudgeYBehind, pixel) : TBL8(MariaLedgeNudgeY, pixel);
    base = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    moved = add_bytes(value, base, false);
    mx_regs(e, r_v(r_c(r_ldx(r_lda(regs_kept(), moved.value), pixel), moved.carry), moved.overflow));
    push8(pixel);
    jsr_push(hold_ret);
    mx_regs(e, MariaLedgeHoldAt(moved.value));
    jsr_pop();
    pose = mx_x_from(x, y, *e);
    if (behind) {
        pose = (uint8_t)(pose + LEDGE_BEHIND_POSES);
        mx_regs(e, r_ldx(regs_kept(), pose));
    }
    jsr_push(pose_ret);
    mx_regs(e, MariaLedgePose(pose));
    jsr_pop();
    pixel = pull8();
    mx_regs(e, r_ldx(regs_kept(), pixel));
    jsr_push(floor_ret);
    floor = MariaLedgeFloorBelow(pixel, mx_y_from(x, y, *e));
    jsr_pop();
    mx_then(e, floor);
    if (!mx_zero(*e)) {
        return;
    }
    value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    moved = sub_bytes(value, behind ? TBL8(MariaLedgeNudgeYBehind, pixel) : TBL8(MariaLedgeNudgeY, pixel), true);
    WR8(OBJ->y_lo[OBJECT_PLAYER], moved.value);
    mx_regs(e, r_v(r_c(r_lda(regs_kept(), moved.value), moved.carry), moved.overflow));
}

/*
 * From a landing on a marked floor ($AB0F): a hard one ($07xx) replaces slots 1 and 2 as kind 5 with sound 2; she may
 * leave the top, stops, loses her X and Y fractions, and takes the hold of the ledge ahead where the point 8 across
 * and $0E below reads bit 2; of the ledge behind her where 8 back and $0E below does; else the one ahead where 8 back
 * and $1E below does, and otherwise the one behind.
 */
static void land_on_mark(MExit *e, uint8_t x, uint8_t y)
{
    uint8_t value = RD8(OBJ->vy_hi[OBJECT_PLAYER]);

    mx_regs(e, r_cmp(r_lda(regs_kept(), value), value, LANDING_SPEED_TOP));
    if (value == LANDING_SPEED_TOP) {
        jsr_push(0xAB19);
        mx_regs(e, MariaReplaceSlotsOneTwoKind5());
        jsr_pop();
        jsr_push(0xAB1E);
        mx_sound(e, SOUND_LANDED, SoundCommand(SOUND_LANDED, mx_x_from(x, y, *e), mx_y_from(x, y, *e)));
        jsr_pop();
    }
    WR8(PVARS->may_leave_top, 1);
    mx_regs(e, r_lda(regs_kept(), 1));
    jsr_push(0xAB26);
    MariaStopMoving();
    jsr_pop();
    WR8(OBJ->x_frac[OBJECT_PLAYER], 0);
    WR8(OBJ->y_frac[OBJECT_PLAYER], 0);
    if (ledge_read(e, x, y, LEDGE_ACROSS, LEDGE_LOW_BELOW, 0xAB3D) != 0) {
        ledge_hold(e, x, y, false, 0xAB58, 0xAB5A, 0xAB5E);
        return;
    }
    mx_then(e, MariaLandOnMarkedBack(x, y));
}

/*
 * @routine 01DE:38D MariaLandOnMarkedBack
 * @run 6D:AB8D
 * WHAT IT DOES: The landing on a mark when nothing is 8 across from her: the hold of the ledge behind her where the
 * point 8 back and $0E below reads bit 2; else of the ledge ahead where 8 back and $1E below does; else the one
 * behind.
 * HOW: Two reads by MariaReadBelow; the hold behind (poses 2 and 3, MariaLedgeNudgeXBehind and MariaLedgeNudgeYBehind),
 * or a branch into MariaLandOnMarked's hold ahead.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the hold's last read or its Y's subtract.
 */
MExit MariaLandOnMarkedBack(uint8_t x, uint8_t y)
{
    MExit e = mx_none();

    if (ledge_read(&e, x, y, LEDGE_BACK, LEDGE_LOW_BELOW, 0xAB9E) == 0 &&
        ledge_read(&e, x, y, LEDGE_BACK, LEDGE_FAR_BELOW, 0xABB3) != 0) {
        ledge_hold(&e, x, y, false, 0xAB58, 0xAB5A, 0xAB5E);
        return e;
    }
    ledge_hold(&e, x, y, true, 0xABCE, 0xABD2, 0xABD5);
    return e;
}

void abi_MariaLandOnMarkedBack(void)
{
    mx_exit(MariaLandOnMarkedBack(cpu.x, cpu.y));
}

/*
 * @routine 01DE:305 MariaLandOnMarked
 * @run 6D:AB05
 * WHAT IT DOES: Lands her on a marked floor: her Y up by column 0's row in its tile, then onto a ledge's hold.
 * HOW: A subtract of RichterColumnRow; the landing on a mark ($AB0F on).
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the hold's last read or its Y's subtract.
 */
MExit MariaLandOnMarked(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    Sum moved = sub_bytes(value, RD8(PVARS->column_row[0]), true);

    WR8(OBJ->y_lo[OBJECT_PLAYER], moved.value);
    land_on_mark(&e, x, y);
    return e;
}

void abi_MariaLandOnMarked(void)
{
    mx_exit(MariaLandOnMarked(cpu.x, cpu.y));
}

/*
 * @routine 01DE:2F7 MariaLandOnMarkedFeet
 * @run 6D:AAF7
 * WHAT IT DOES: Lands her on the marked floor under her feet: her Y moved by 8 less zTemp02 (the row the feet's read
 * left), then onto a ledge's hold.
 * HOW: The add and subtract, then a branch into MariaLandOnMarked's landing.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaLandOnMarked.
 */
MExit MariaLandOnMarkedFeet(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    Sum moved = add_bytes(value, LEDGE_ACROSS, false);

    moved = sub_bytes(moved.value, RD8(ZP->temp02), true);
    WR8(OBJ->y_lo[OBJECT_PLAYER], moved.value);
    land_on_mark(&e, x, y);
    return e;
}

void abi_MariaLandOnMarkedFeet(void)
{
    mx_exit(MariaLandOnMarkedFeet(cpu.x, cpu.y));
}

/*
 * @routine 01DE:241 MariaFeetOnMarkedFloor
 * @run 6D:AA41
 * WHAT IT DOES: Whether her feet land on a marked floor: below row $10, under her feet a mark of bit 5, or of bit 2
 * with up held, and the feet's row in it (zTemp02) under 3: C set. A row of 3 or more starts her falling at $0300
 * instead, C clear.
 * HOW: MariaReadUnderFeet; the tests; the tails of MariaOnMarkedFloor (SEC or CLC, RTS).
 * WAITS: no.
 * Entry: nothing. Exit: A the last value loaded or tested, N and Z from it or its compare; C the answer; Y and V as
 * MariaReadUnderFeet left them.
 */
MExit MariaFeetOnMarkedFloor(void)
{
    MExit e = mx_none();
    uint8_t value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    uint8_t marks;

    mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, MARK_TOO_HIGH));
    if (value < MARK_TOO_HIGH) {
        mx_regs(&e, r_c(regs_kept(), false));
        return e;
    }
    jsr_push(0xAA4B);
    mx_under_feet(&e, MariaReadUnderFeet());
    jsr_pop();
    marks = (uint8_t)(mx_a_from(0, 0, e) & MARK_FLOOR);
    mx_regs(&e, r_lda(regs_kept(), marks));
    if (marks == 0) {
        mx_regs(&e, r_c(regs_kept(), false));
        return e;
    }
    marks &= MARK_DOWN_ONLY;
    mx_regs(&e, r_lda(regs_kept(), marks));
    if (marks == 0) {
        value = RD8(ZP->pad_held);
        mx_regs(&e, r_tst(regs_kept(), PAD_UP, value));
        if (!(value & PAD_UP)) {
            mx_regs(&e, r_c(regs_kept(), false));
            return e;
        }
    }
    value = RD8(ZP->temp02);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, FEET_ROW_LIMIT));
    if (value >= FEET_ROW_LIMIT) {
        mx_regs(&e, r_c(regs_kept(), true));
        return e;
    }
    WR8(OBJ->vy_hi[OBJECT_PLAYER], FEET_FALL_VY_HI);
    WR8(OBJ->vy_lo[OBJECT_PLAYER], 0);
    mx_regs(&e, r_c(r_lda(regs_kept(), FEET_FALL_VY_HI), false));
    return e;
}

void abi_MariaFeetOnMarkedFloor(void)
{
    mx_exit(MariaFeetOnMarkedFloor());
}

/* ------------------------------------------------------------------------------------------------ landing */

/*
 * @routine 01DE:2B8 MariaJumpLand
 * @run 6D:AAB8
 * WHAT IT DOES: Lands her on the floor her column found: lifted onto it; with a mark above her, the walls tried as in
 * state $0D; then a soft landing stands her, a hard one (Y speed $06xx or more, or the mark still above) is state $0A
 * with animation $10, slots 1 and 2 replaced as kind 5 and sound 2, and no speed.
 * HOW: MariaLiftByColumn, MariaMarkedAbove's Z, PlayerWallLeft and PlayerWallRight; MariaStand by its tail JMP, or
 * MariaPadForcesOff, MariaReplaceSlotsOneTwoKind5, SoundCommand, MariaSetAnimation and the tail JMP to MariaStopMoving.
 * WAITS: no.
 * Entry: X the column, Y the caller's. Exit: as MariaStand, or MariaSetAnimation.
 */
MExit MariaJumpLand(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value;
    bool hard = false;
    AnimThen stand;

    jsr_push(0xAABB);
    mx_lift(&e, MariaLiftByColumn(x));
    jsr_pop();
    jsr_push(0xAABE);
    mx_marked(&e, MariaMarkedAbove());
    jsr_pop();
    if (!mx_zero(e)) {
        WR8(OBJ->state[OBJECT_PLAYER], STATE_LANDING_LOW);
        mx_regs(&e, r_lda(regs_kept(), STATE_LANDING_LOW));
        jsr_push(0xAAC8);
        mx_wall(&e, PlayerWallLeft());
        jsr_pop();
        jsr_push(0xAACB);
        mx_wall(&e, PlayerWallRight());
        jsr_pop();
        jsr_push(0xAACE);
        mx_marked(&e, MariaMarkedAbove());
        jsr_pop();
        hard = !mx_zero(e);
    }
    if (!hard) {
        value = RD8(OBJ->vy_hi[OBJECT_PLAYER]);
        mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, HARD_LANDING));
        if (value < HARD_LANDING) {
            stand = MariaStand();
            mx_animation(&e, stand.animation);
            mx_regs(&e, stand.regs);
            return e;
        }
    }
    jsr_push(0xAADD);
    MariaPadForcesOff();
    jsr_pop();
    jsr_push(0xAAE0);
    mx_regs(&e, MariaReplaceSlotsOneTwoKind5());
    jsr_pop();
    jsr_push(0xAAE5);
    mx_sound(&e, SOUND_LANDED, SoundCommand(SOUND_LANDED, mx_x_from(x, y, e), mx_y_from(x, y, e)));
    jsr_pop();
    WR8(OBJ->state[OBJECT_PLAYER], STATE_LANDED);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    mx_regs(&e, r_lda(regs_kept(), ANIMATION_LANDED));
    jsr_push(0xAAF4);
    mx_animation(&e, MariaSetAnimation(ANIMATION_LANDED));
    jsr_pop();
    MariaStopMoving();
    return e;
}

void abi_MariaJumpLand(void)
{
    mx_exit(MariaJumpLand(cpu.x, cpu.y));
}

/*
 * @routine 01DE:41F MariaStateLanded
 * @run 6D:AC1F
 * WHAT IT DOES: State $0A, after a hard landing: the columns under her; no footing, she falls; a landing; while her
 * animation runs, nothing more; when it has stopped, she crouches (state 2, step 0, frame $1B).
 * HOW: MariaReadColumns, MariaHasNoFooting and the JMP to MariaToFall, MariaLandOnFloor, MariaAnimationWait.
 * WAITS: no.
 * Entry: nothing. Exit: as the routine it ended in; else A 0, Z set.
 */
MExit MariaStateLanded(void)
{
    MExit e = mx_none();
    FootingCheck footing;
    AnimationWaitEnd wait;
    AnimThen fall;

    jsr_push(0xAC22);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xAC25);
    footing = MariaHasNoFooting();
    jsr_pop();
    mx_footing(&e, footing);
    if (footing.no_floor) {
        fall = MariaToFall();
        mx_animation(&e, fall.animation);
        mx_regs(&e, fall.regs);
        return e;
    }
    jsr_push(0xAC2D);
    mx_landing(&e, MariaLandOnFloor());
    jsr_pop();
    jsr_push(0xAC30);
    wait = MariaAnimationWait();
    mx_animation(&e, wait.step);
    if (wait.dropped) {
        return e;
    }
    jsr_pop();
    WR8(OBJ->frame[OBJECT_PLAYER], FRAME_CROUCHED);
    WR8(OBJ->state[OBJECT_PLAYER], STATE_CROUCH);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    mx_regs(&e, r_lda(regs_kept(), 0));
    return e;
}

void abi_MariaStateLanded(void)
{
    mx_exit(MariaStateLanded());
}

/* ------------------------------------------------------------------------------------------------ the jump */

/*
 * @routine 01DE:457 MariaSecondJump
 * @run 6D:AC57
 * WHAT IT DOES: Her second jump in the air: slots 11 to 14 claimed (MariaClaimSlotsB), the turn count cleared, state 3
 * step 4, Y speed $FD00 up, animation $0C; with right or left held she faces that way and takes the walking speed,
 * else animation $14.
 * HOW: MariaClaimSlotsB, MariaStopMoving, MariaSetAnimation; BBS5 and BBS7 of zPadHeld (two reads); the tail JMPs to
 * MariaSetAnimation and MariaSpeedWalk.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaSetAnimation with $14, or MariaSpeedWalk.
 */
MExit MariaSecondJump(void)
{
    MExit e = mx_none();
    uint8_t claimed, facing = 0;

    jsr_push(0xAC5A);
    claimed = MariaClaimSlotsB();
    jsr_pop();
    mx_regs(&e, r_c(r_nz(r_x_only(r_a_only(regs_kept(), claimed), 0x0F), 0), true));
    WR8(ZP->fieldc6, 0);
    WR8(OBJ->state[OBJECT_PLAYER], STATE_JUMP);
    WR8(OBJ->substate[OBJECT_PLAYER], STEP_SECOND_JUMP);
    jsr_push(0xAC69);
    MariaStopMoving();
    jsr_pop();
    WR8(OBJ->vy_lo[OBJECT_PLAYER], 0);
    WR8(OBJ->vy_hi[OBJECT_PLAYER], SECOND_JUMP_VY_HI);
    mx_regs(&e, r_lda(regs_kept(), ANIMATION_SECOND_JUMP));
    jsr_push(0xAC78);
    mx_animation(&e, MariaSetAnimation(ANIMATION_SECOND_JUMP));
    jsr_pop();
    mx_regs(&e, r_y_only(regs_kept(), facing));
    if (!(RD8(ZP->pad_held) & PAD_RIGHT)) {
        facing = 1;
        mx_regs(&e, r_ldy(regs_kept(), facing));
        if (!(RD8(ZP->pad_held) & PAD_LEFT)) {
            mx_regs(&e, r_lda(regs_kept(), ANIMATION_SECOND_JUMP_UP));
            mx_animation(&e, MariaSetAnimation(ANIMATION_SECOND_JUMP_UP));
            return e;
        }
    }
    WR8(OBJ->facing_left[OBJECT_PLAYER], facing);
    mx_regs(&e, MariaSpeedWalk());
    return e;
}

void abi_MariaSecondJump(void)
{
    mx_exit(MariaSecondJump());
}

/*
 * The checks for a landing on the way down, shared by the jump and the fall ($AA76 to $AA96 and $ACA0 to $ACDB): a
 * floor her column stands on lands her; a marked floor, or one under her feet, takes her onto a ledge. Returns true
 * when one of them ended the state's frame (its tail JMP), with the exit in e.
 */
static bool landing_checks(MExit *e, uint8_t x, uint8_t y, bool fall)
{
    FloorResult floor;
    MarkedFloor marked;
    MExit feet;

    jsr_push(fall ? 0xACA3 : 0xAA79);
    mx_columns(e, MariaReadColumns());
    jsr_pop();
    jsr_push(fall ? 0xACA6 : 0xAA7C);
    floor = MariaFloorCheck();
    jsr_pop();
    mx_floor(e, floor);
    if (floor.how != FLOOR_NONE) {
        uint8_t column = mx_x_from(x, y, *e);

        jsr_push(fall ? 0xACAB : 0xAA81);
        mx_solid(e, MariaSolidUnderColumn(column));
        jsr_pop();
        if (mx_zero(*e)) {
            mx_then(e, MariaJumpLand(mx_x_from(x, y, *e), mx_y_from(x, y, *e)));
            return true;
        }
    }
    if (fall) {
        return false;
    }
    jsr_push(0xAA89);
    marked = MariaOnMarkedFloor();
    jsr_pop();
    mx_on_marked(e, marked);
    if (marked.how == FLOOR_MARKED || marked.how == FLOOR_MARKED_DOWN) {
        mx_then(e, MariaLandOnMarked(mx_x_from(x, y, *e), mx_y_from(x, y, *e)));
        return true;
    }
    jsr_push(0xAA91);
    feet = MariaFeetOnMarkedFloor();
    jsr_pop();
    mx_then(e, feet);
    if (mx_carry(*e)) {
        mx_then(e, MariaLandOnMarkedFeet(mx_x_from(x, y, *e), mx_y_from(x, y, *e)));
        return true;
    }
    jsr_push(0xAA99);
    mx_then(e, MariaCheckBit3Under(mx_x_from(x, y, *e), mx_y_from(x, y, *e)));
    jsr_pop();
    return false;
}

/*
 * @routine 01DE:26A MariaStateJump
 * @run 6D:AA6A
 * WHAT IT DOES: States 3, 11 and 12, the jump: a turn in the air until one is counted (zFieldC6); on the way down, the
 * landings (floor, marked floor, marked floor under her feet) and the bit-3 burst; the jump's gravity; her animation;
 * the item press noted; the attack button; in step 0, the jump pressed again is her second jump.
 * HOW: MariaAirTurn; the Y speed's sign; MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn's Z, MariaOnMarkedFloor,
 * MariaFeetOnMarkedFloor, MariaCheckBit3Under; MariaJumpGravity, MariaAnimationTick, MariaNoteItemPress, MariaUseItem,
 * MariaJumpPressed; each ending in its tail JMP.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the routine it ended in; else as the last check made.
 */
MExit MariaStateJump(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value;
    TurnWindow use;
    bool jump;

    value = RD8(ZP->fieldc6);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value == 0) {
        jsr_push(0xAA71);
        mx_tst(&e, MariaAirTurn());
        jsr_pop();
    }
    value = RD8(OBJ->vy_hi[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (!(value & 0x80) && landing_checks(&e, x, y, false)) {
        return e;
    }
    jsr_push(0xAA9C);
    mx_then(&e, MariaJumpGravity());
    jsr_pop();
    jsr_push(0xAA9F);
    mx_animation(&e, MariaAnimationTick());
    jsr_pop();
    jsr_push(0xAAA2);
    mx_tst(&e, richter_note_item_press_exit_shim(MariaNoteItemPress()));
    jsr_pop();
    jsr_push(0xAAA5);
    use = MariaUseItem();
    jsr_pop();
    mx_turn(&e, use);
    if (use.end.c_set && use.end.c) {
        mx_then(&e, MariaAttack(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    value = RD8(OBJ->substate[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value != 0) {
        return e;
    }
    jsr_push(0xAAB2);
    jump = MariaJumpPressed();
    jsr_pop();
    mx_regs(&e, r_c(regs_kept(), jump));
    if (jump) {
        mx_then(&e, MariaSecondJump());
    }
    return e;
}

void abi_MariaStateJump(void)
{
    mx_exit(MariaStateJump(cpu.x, cpu.y));
}

/*
 * @routine 01DE:4A0 MariaStateFall
 * @run 6D:ACA0
 * WHAT IT DOES: State 4, the fall: a floor her column stands on lands her; when the drift delay (OBJ_FALL_DRIFT_DELAY)
 * runs out she takes the drift's speed; the marked-floor landings, the bit-3 burst; gravity $0048, held at $07xx; her
 * animation; in step 0 the item press, the attack button and the second jump.
 * HOW: MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn; DEC of the delay, MariaFallDriftSpeeds; MariaOnMarkedFloor,
 * MariaFeetOnMarkedFloor, MariaCheckBit3Under; the 16-bit add; MariaAnimationTick, MariaNoteItemPress, MariaUseItem,
 * MariaJumpPressed; each ending in its tail JMP.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the routine it ended in; else as the last check made.
 */
MExit MariaStateFall(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value, facing, index;
    MarkedFloor marked;
    MExit feet;
    TurnWindow use;
    Sum lo, hi;
    bool jump;

    if (landing_checks(&e, x, y, true)) {
        return e;
    }
    value = RD8(OBJ->fall_drift_delay[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value != 0) {
        value = DEC8(OBJ->fall_drift_delay[OBJECT_PLAYER]);
        mx_regs(&e, r_nz(regs_kept(), value));
        if (value == 0) {
            facing = RD8(OBJ->facing_left[OBJECT_PLAYER]);
            index = (uint8_t)(facing << 1);
            value = TBL8(MariaFallDriftSpeeds, index);
            WR8(OBJ->vx_lo[OBJECT_PLAYER], value);
            value = TBL8(MariaFallDriftSpeeds, index + 1);
            WR8(OBJ->vx_hi[OBJECT_PLAYER], value);
            mx_regs(&e, r_c(r_lda(r_y_only(regs_kept(), index), value), (facing & 0x80) != 0));
        }
    }
    jsr_push(0xACCE);
    marked = MariaOnMarkedFloor();
    jsr_pop();
    mx_on_marked(&e, marked);
    if (marked.how == FLOOR_MARKED || marked.how == FLOOR_MARKED_DOWN) {
        mx_then(&e, MariaLandOnMarked(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    jsr_push(0xACD6);
    feet = MariaFeetOnMarkedFloor();
    jsr_pop();
    mx_then(&e, feet);
    if (mx_carry(e)) {
        mx_then(&e, MariaLandOnMarkedFeet(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    jsr_push(0xACDE);
    mx_then(&e, MariaCheckBit3Under(mx_x_from(x, y, e), mx_y_from(x, y, e)));
    jsr_pop();
    value = RD8(OBJ->vy_lo[OBJECT_PLAYER]);
    lo = add_bytes(value, FALL_GRAVITY, false);
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
    jsr_push(0xACFB);
    mx_animation(&e, MariaAnimationTick());
    jsr_pop();
    value = RD8(OBJ->substate[OBJECT_PLAYER]);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, 0));
    if (value != 0) {
        return e;
    }
    jsr_push(0xAD05);
    mx_tst(&e, richter_note_item_press_exit_shim(MariaNoteItemPress()));
    jsr_pop();
    jsr_push(0xAD08);
    use = MariaUseItem();
    jsr_pop();
    mx_turn(&e, use);
    if (use.end.c_set && use.end.c) {
        mx_then(&e, MariaAttack(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    jsr_push(0xAD10);
    jump = MariaJumpPressed();
    jsr_pop();
    mx_regs(&e, r_c(regs_kept(), jump));
    if (jump) {
        mx_then(&e, MariaSecondJump());
    }
    return e;
}

void abi_MariaStateFall(void)
{
    mx_exit(MariaStateFall(cpu.x, cpu.y));
}
