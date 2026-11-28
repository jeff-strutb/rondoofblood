/*
 * Richter in the air, from his overlay at $A000 (sector $1B9): the jump and the fall (states 3, 4, 11
 * and 12), and the landing that ends them.
 *
 * A body returns plain values; its abi_ wrapper gives the registers and flags the assembly leaves,
 * replaying the exits of the callees whose registers and flags survive to the end.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"
#include "access.h"
#include "enemy_objects.h"
#include "player_objects.h"
#include "richter.h"

enum {
    COLUMN_DEBRIS_BIT = 0x08, SOUND_DEBRIS = 0x23, SLOT_LAST_EXTRA = 2, KIND_CLEARED_BY_DEBRIS = 1,
    SCRIPT_DEBRIS = 0x08, DEBRIS_BELOW = 0x10, DEBRIS_TIMER = 0, SCRIPT_SLOT = 7
};

/*
 * @routine 01BA:68B RichterDebrisOnMarkedColumns
 * @run 6D:AE8B
 * WHAT IT DOES: When a column read under him has bit 3: sound $23; each of slots 2 down to 1 holding
 * kind 1 emptied, with script 8; a burst of debris 16 pixels below him.
 * HOW: RichterColumnHit's three bytes ORed; SoundCommand and ClearObject through their kernel
 * vectors, RichterStartHitScript. The script leaves X 7, so the count goes on down from 6 after it.
 * His Y is pushed, moved down for RichterStartDebrisBurst with timer 0, and pulled back.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: no bit 3: A 0, N clear, Z set; X, Y, C and V kept.
 * Otherwise A his Y low byte, N and Z from it; X 0; Y 8 if a slot was emptied, else kept; C as
 * RichterStartDebrisBurst; V from adding the carry to his Y high byte.
 */
DebrisColumns RichterDebrisOnMarkedColumns(uint8_t x, uint8_t y)
{
    DebrisColumns r = { 0 };
    Sum low, high;
    uint8_t value, slot;

    value = RD8(PVARS->column_hit[0]);
    value |= RD8(PVARS->column_hit[1]);
    value |= RD8(PVARS->column_hit[2]);
    if (!(value & COLUMN_DEBRIS_BIT)) {
        return r;
    }
    r.burst_started = true;
    jsr_push(0xAE9E);
    SoundCommand(SOUND_DEBRIS, x, y);
    jsr_pop();
    for (slot = SLOT_LAST_EXTRA; slot != 0; slot--) {
        if (RD8(OBJ->kind[slot]) != KIND_CLEARED_BY_DEBRIS) {
            continue;
        }
        jsr_push(0xAEA9);
        ClearObject(slot);
        jsr_pop();
        jsr_push(0xAEAE);
        RichterStartHitScript(SCRIPT_DEBRIS);
        jsr_pop();
        slot = SCRIPT_SLOT;
        y = SCRIPT_DEBRIS;
    }
    value = RD8(OBJ->y_lo[0]);
    push8(value);
    low = add_bytes(value, DEBRIS_BELOW, false);
    WR8(OBJ->y_lo[0], low.value);
    value = RD8(OBJ->y_hi[0]);
    push8(value);
    high = add_bytes(value, 0, low.carry);
    WR8(OBJ->y_hi[0], high.value);
    r.overflow = high.overflow;
    jsr_push(0xAEC7);
    r.burst = RichterStartDebrisBurst(0, DEBRIS_TIMER, y);
    jsr_pop();
    value = pull8();
    WR8(OBJ->y_hi[0], value);
    value = pull8();
    WR8(OBJ->y_lo[0], value);
    r.y_low = value;
    return r;
}

void debris_columns_exit_shim(DebrisColumns r)
{
    if (!r.burst_started) {
        abi_ended(ended_load(0));
        return;
    }
    cpu.x = 0;
    abi_flag(P_V, r.overflow);
    abi_ended(start_debris_burst_exit_shim(r.burst));
    abi_ended(ended_load(r.y_low));
}

void abi_RichterDebrisOnMarkedColumns(void)
{
    debris_columns_exit_shim(RichterDebrisOnMarkedColumns(cpu.x, cpu.y));
}

enum {
    PAD_I = 0x01, PAD_DOWN_HELD = 0x10, PAD_RIGHT = 0x20, PAD_LEFT = 0x80, MARK_FROM_ROW = 0x10,
    MARKS = 0x24, MARK_ALWAYS = 0x20, STEP_ATTACK = 1, FALL_STATE = 4, FALL_STEP_ATTACK = 1,
    FALL_ATTACK_VY_HI = 0x03, FALL_ATTACK_DRIFT = 4, FALL_PULL_LO = 0x48, FALL_TOP_SPEED = 7,
    FRAME_FALL_END = 0x12, ANIMATION_STOPPED = 0xFF, SCRIPT_AIR_STEP2 = 0x0C, AIR_STEP2_TIMER = 0x40,
    SCRIPT_AIR_STEP3_END = 0x09, KIND_SLOT_FIVE_SHORT = 0x11, KIND_SLOT_FIVE_NEXT = 0x0D,
    AIR_STEP3_LONG_TIMER = 0x28, SLOT_FIVE = 5, BAND_AIR_CONTROL = 3, BAND_KNOCK_TEST = 2,
    KNOCK_TIMER_LIMIT = 8, CROUCH_LAND_STATE = 2, LAND_CROUCH_STATE = 0x0A, ANIM_LAND = 0x07,
    SOUND_LAND = 0x02, LAND_SLIDE_PUSH = 0x01, LAND_CROUCH_ANIMATION = 0x06, LAND_FORCE_FRAMES = 0x10,
    LAND_FORCE_PAD = 0x40, MARKED_LIFT = 0x08, MARK_FORCE_FRAMES = 0x06, CLIMB_PROBE_RIGHT = 0x08,
    CLIMB_PROBE_LEFT = 0xF8, CLIMB_PROBE_LOW = 0x0E, CLIMB_PROBE_DEEP = 0x1E, CLIMB_MARK = 0x04,
    FIRST_CLIMB_STATE = 5, FALL_NO_DRIFT = 0xFF, JUMP_KNOCKED = 0x01
};

/* A JSR to a routine that may drop its caller's return address (RichterAnimationWait): true when it did,
 * and the caller then ends at once. */
static bool wait_dropped(RegLog *log, uint16_t ret)
{
    AnimationWaitEnd wait;

    jsr_push(ret);
    wait = RichterAnimationWait();
    rl_animation(log, wait.step);
    if (wait.dropped) {
        return true;
    }
    jsr_pop();
    return false;
}

/* The landing check the states in the air make: the columns, then a floor whose column answers 0.
 * True to land, at that column. */
static bool air_landing(RegLog *log, uint8_t *column, uint16_t read_ret, uint16_t check_ret, uint16_t solid_ret)
{
    FloorResult floor;
    ColumnRead under;

    jsr_push(read_ret);
    rl_columns(log, RichterReadColumns());
    jsr_pop();
    jsr_push(check_ret);
    floor = RichterFloorCheck();
    jsr_pop();
    rl_floor(log, floor);
    if (floor.how == FLOOR_NONE) {
        return false;
    }
    *column = floor.column;
    jsr_push(solid_ret);
    under = RichterSolidUnderColumn(floor.column);
    jsr_pop();
    rl_solid(log, under);
    return solid_answer(under) == 0;
}

/* Then a marked floor, and marked ground under his feet. */
typedef enum { AIR_NONE, AIR_LAND, AIR_MARKED, AIR_MARKED_FEET } AirFloor;

static AirFloor air_marked(RegLog *log, uint16_t marked_ret, uint16_t feet_ret)
{
    MarkedFloor marked;

    jsr_push(marked_ret);
    marked = RichterOnMarkedFloor();
    jsr_pop();
    rl_marked(log, marked);
    if (marked.how == FLOOR_MARKED || marked.how == FLOOR_MARKED_DOWN) {
        return AIR_MARKED;
    }
    jsr_push(feet_ret);
    marked = RichterOnMarkedUnderFeet(log);
    jsr_pop();
    if (marked.how == FLOOR_MARKED || marked.how == FLOOR_MARKED_DOWN) {
        return AIR_MARKED_FEET;
    }
    return AIR_NONE;
}

/* The X and Y a log has left, from those the routine was entered with. */
static void log_regs(const RegLog *log, uint8_t *x, uint8_t *y)
{
    rl_xy(log, *x, *y, x, y);
}

/*
 * @routine 01BA:3A4 RichterOnMarkedUnderFeet
 * @run 6D:ABA4
 * WHAT IT DOES: RichterOnMarkedFloor for the ground under his feet: once he is $10 down the screen, a
 * read under his soles with bit 5, or with bit 2 while down is held.
 * HOW: OBJ_Y_LO; RichterReadUnderFeet's answer; bit 4 of zPadHeld; its CLC and SEC are
 * RichterOnMarkedFloor's own ($ABA0, $ABA2).
 * WAITS: no.
 * Entry: nothing. Exit: C set if marked; A, N, Z and V as RichterOnMarkedFloor leaves them, from the
 * read's answer; X and Y as RichterReadUnderFeet left them.
 */
MarkedFloor RichterOnMarkedUnderFeet(RegLog *log)
{
    MarkedFloor floor = { FLOOR_TOO_HIGH, 0, 0 };
    ColumnRead under;

    floor.value = RD8(OBJ->y_lo[0]);
    rl_ended(log, ended_compare(floor.value, MARK_FROM_ROW));
    if (floor.value < MARK_FROM_ROW) {
        return floor;
    }
    jsr_push(0xABAE);
    under = RichterReadUnderFeet();
    jsr_pop();
    rl_under_feet(log, under);
    floor.value = (uint8_t)(column_answer(under) & MARKS);
    floor.how = FLOOR_UNMARKED;
    rl_ended(log, ended_load(floor.value));
    if (floor.value == 0) {
        rl_ended(log, ended_with_carry(ended_kept(), false));
        return floor;
    }
    floor.value &= MARK_ALWAYS;
    rl_ended(log, ended_load(floor.value));
    if (floor.value != 0) {
        floor.how = FLOOR_MARKED;
        rl_ended(log, ended_with_carry(ended_kept(), true));
        return floor;
    }
    floor.held = RD8(ZP->pad_held);
    rl_tst(log, PAD_DOWN_HELD, floor.held);
    floor.how = (floor.held & PAD_DOWN_HELD) ? FLOOR_MARKED_DOWN : FLOOR_NOT_DOWN;
    rl_ended(log, ended_with_carry(ended_kept(), floor.how == FLOOR_MARKED_DOWN));
    return floor;
}

void abi_RichterOnMarkedUnderFeet(void)
{
    RegLog log = { 0 };

    RichterOnMarkedUnderFeet(&log);
    reg_log_replay(&log);
}

/*
 * @routine 01BA:31F RichterAirTurnSpeed
 * @run 6D:AB1F
 * WHAT IT DOES: After a turn in the air, unless attacking (step 1): the turn's X speed by facing.
 * HOW: The two words at $AB85.
 * WAITS: no.
 * Entry: nothing. Exit: attacking, A 1, N, Z and C from comparing the step with 1; else A the speed's
 * high byte, N and Z from it, Y twice the facing, C bit 7 of the facing. X and V kept.
 */
void RichterAirTurnSpeed(RegLog *log)
{
    uint8_t value, facing, at;

    value = RD8(OBJ->substate[0]);
    rl_ended(log, ended_compare(value, STEP_ATTACK));
    if (value == STEP_ATTACK) {
        return;
    }
    facing = RD8(OBJ->facing_left[0]);
    at = (uint8_t)(facing << 1);
    value = rd8((uint16_t)(RichterAirTurnSpeeds + at));
    WR8(OBJ->vx_lo[0], value);
    value = rd8((uint16_t)(RichterAirTurnSpeeds + at + 1));
    WR8(OBJ->vx_hi[0], value);
    rl_ended(log, ended_with_carry(ended_with_y(ended_load(value), at), (facing & 0x80) != 0));
}

/*
 * @routine 01BA:338 RichterAirTurnFacingLeft
 * @run 6D:AB38
 * WHAT IT DOES: Still in the air facing left: left held gives the turn's speed; right held turns him
 * right, counts the turn (zFieldC6) and gives it.
 * HOW: TSTs of zPadHeld; RichterAirTurnSpeed.
 * WAITS: no.
 * Entry: nothing. Exit: neither held, Z set, N and V bits 7 and 6 of zPadHeld; else as
 * RichterAirTurnSpeed.
 */
void RichterAirTurnFacingLeft(RegLog *log)
{
    uint8_t held = RD8(ZP->pad_held);

    rl_tst(log, PAD_LEFT, held);
    if (held & PAD_LEFT) {
        RichterAirTurnSpeed(log);
        return;
    }
    held = RD8(ZP->pad_held);
    rl_tst(log, PAD_RIGHT, held);
    if (!(held & PAD_RIGHT)) {
        return;
    }
    rl_ended(log, ended_nz(INC8(ZP->field_c6)));
    WR8(OBJ->facing_left[0], 0);
    RichterAirTurnSpeed(log);
}

/*
 * @routine 01BA:308 RichterAirTurnStill
 * @run 6D:AB08
 * WHAT IT DOES: In the air with no X speed: as RichterAirTurnFacingLeft when he faces left; facing
 * right, right held gives the turn's speed, left held turns him left, counts it and gives it.
 * HOW: TSTs of zPadHeld; runs on into RichterAirTurnSpeed after the turn.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterAirTurnFacingLeft.
 */
void RichterAirTurnStill(RegLog *log)
{
    uint8_t facing, held;

    facing = RD8(OBJ->facing_left[0]);
    rl_ended(log, ended_load(facing));
    if (facing != 0) {
        RichterAirTurnFacingLeft(log);
        return;
    }
    held = RD8(ZP->pad_held);
    rl_tst(log, PAD_RIGHT, held);
    if (held & PAD_RIGHT) {
        RichterAirTurnSpeed(log);
        return;
    }
    held = RD8(ZP->pad_held);
    rl_tst(log, PAD_LEFT, held);
    if (!(held & PAD_LEFT)) {
        return;
    }
    rl_ended(log, ended_nz(INC8(ZP->field_c6)));
    WR8(OBJ->facing_left[0], 1);
    rl_ended(log, ended_load(1));
    RichterAirTurnSpeed(log);
}

/*
 * @routine 01BA:34B RichterAirControl
 * @run 6D:AB4B
 * WHAT IT DOES: The pad's turn in the air: with no X speed, as RichterAirTurnStill; moving one way
 * while facing the other, the pad held the way he moves turns him to face it, counts the turn
 * (zFieldC6), and with I held gives the turn's speed.
 * HOW: OBJ_VX_HI and LO; TSTs of zPadHeld; a jump to RichterAirTurnSpeed.
 * WAITS: no.
 * Entry: nothing. Exit: as the path it took; N, Z and V from the last TST where it ends on one.
 */
void RichterAirControl(RegLog *log)
{
    uint8_t value, held;

    value = RD8(OBJ->vx_hi[0]);
    value |= RD8(OBJ->vx_lo[0]);
    rl_ended(log, ended_load(value));
    if (value == 0) {
        RichterAirTurnStill(log);
        return;
    }
    value = RD8(OBJ->vx_hi[0]);
    rl_ended(log, ended_load(value));
    if (!(value & 0x80)) {
        value = RD8(OBJ->facing_left[0]);
        rl_ended(log, ended_load(value));
        if (value != 0) {
            return;
        }
        held = RD8(ZP->pad_held);
        rl_tst(log, PAD_LEFT, held);
        if (!(held & PAD_LEFT)) {
            return;
        }
        WR8(OBJ->facing_left[0], 1);
        rl_ended(log, ended_load(1));
    } else {
        value = RD8(OBJ->facing_left[0]);
        rl_ended(log, ended_load(value));
        if (value == 0) {
            return;
        }
        held = RD8(ZP->pad_held);
        rl_tst(log, PAD_RIGHT, held);
        if (!(held & PAD_RIGHT)) {
            return;
        }
        WR8(OBJ->facing_left[0], 0);
    }
    rl_ended(log, ended_nz(INC8(ZP->field_c6)));
    held = RD8(ZP->pad_held);
    rl_tst(log, PAD_I, held);
    if (!(held & PAD_I)) {
        return;
    }
    RichterAirTurnSpeed(log);
}

/*
 * @routine 01BA:1DD RichterFallAttacking
 * @run 6D:A9DD
 * WHAT IT DOES: The floor goes from under an attack: the attack's animation in the air, then, while
 * that runs, a fall at step 1 with Y speed $0300 and the drift after 4 frames; once it has ended, an
 * ordinary fall.
 * HOW: RichterPadOverridesOff; RichterAirAttackAnimations by PlayerAttackKind into OBJ_ANIMATION;
 * RichterAnimationRunning; tail jumps to RichterToFall and RichterStopX.
 * WAITS: no.
 * Entry: nothing. Exit: falling attacking, A 4, N and Z from it, X the attack kind; Y and C as
 * RichterAnimationRunning. Ended, as RichterToFall.
 */
void RichterFallAttacking(RegLog *log)
{
    AnimationStep running;
    uint8_t kind, value;

    jsr_push(0xA9E0);
    RichterPadOverridesOff();
    jsr_pop();
    kind = RD8(PVARS->attack_kind);
    value = rd8((uint16_t)(RichterAirAttackAnimations + kind));
    WR8(OBJ->animation[0], value);
    rl_ended_x(log, ended_load(value), kind);
    jsr_push(0xA9EC);
    running = RichterAnimationRunning();
    jsr_pop();
    rl_animation(log, running);
    if (running.nz == 0) {
        rl_fall(log, RichterToFall());
        return;
    }
    WR8(OBJ->state[0], FALL_STATE);
    WR8(OBJ->substate[0], FALL_STEP_ATTACK);
    WR8(OBJ->vy_lo[0], 0);
    WR8(OBJ->vy_hi[0], FALL_ATTACK_VY_HI);
    WR8(OBJ->fall_drift_delay[0], FALL_ATTACK_DRIFT);
    rl_ended(log, ended_load(FALL_ATTACK_DRIFT));
    RichterStopX();
}

/*
 * @routine 01BA:211 RichterDownOnFirstColumnFloor
 * @run 6D:AA11
 * WHAT IT DOES: Says whether, with down held and not rising, the columns read have a floor at the
 * first column that answers 0. Nothing is known to reach it.
 * HOW: TST of zPadHeld; RichterReadColumns, RichterFloorCheck, RichterSolidUnderColumn; the column in X.
 * WAITS: no.
 * Entry: nothing. Exit: C the answer; A, X, Y, N, Z and V as the last test left them.
 */
bool RichterDownOnFirstColumnFloor(RegLog *log)
{
    FloorResult floor;
    ColumnRead under;
    uint8_t value = RD8(ZP->pad_held);

    rl_tst(log, PAD_DOWN_HELD, value);
    if (!(value & PAD_DOWN_HELD)) {
        rl_ended(log, ended_with_carry(ended_kept(), false));
        return false;
    }
    value = RD8(OBJ->vy_hi[0]);
    rl_ended(log, ended_load(value));
    if (value & 0x80) {
        rl_ended(log, ended_with_carry(ended_kept(), false));
        return false;
    }
    jsr_push(0xAA1E);
    rl_columns(log, RichterReadColumns());
    jsr_pop();
    jsr_push(0xAA21);
    floor = RichterFloorCheck();
    jsr_pop();
    rl_floor(log, floor);
    if (floor.how == FLOOR_NONE) {
        rl_ended(log, ended_with_carry(ended_kept(), false));
        return false;
    }
    jsr_push(0xAA26);
    under = RichterSolidUnderColumn(floor.column);
    jsr_pop();
    rl_solid(log, under);
    if (solid_answer(under) != 0) {
        rl_ended(log, ended_with_carry(ended_kept(), false));
        return false;
    }
    rl_ended(log, ended_with_carry(ended_load(floor.column), floor.column == 0));
    return floor.column == 0;
}

/*
 * @routine 01BA:29E RichterAirStep2
 * @run 6D:AA9E
 * WHAT IT DOES: The air's step 2: falls with gravity until the animation ends; then script $0C, the
 * animation timer $40, no speed, and step 3.
 * HOW: RichterFallGravity, RichterAnimationWait, RichterStartHitScript, RichterStopMoving.
 * WAITS: no.
 * Entry: nothing. Exit: waiting, as RichterAnimationWait; else N and Z from the step, A and Y $0C,
 * X 7, C and V as the script left them.
 */
void RichterAirStep2(RegLog *log)
{
    jsr_push(0xAAA1);
    rl_gravity(log, RichterFallGravity());
    jsr_pop();
    if (wait_dropped(log, 0xAAA4)) {
        return;
    }
    jsr_push(0xAAA9);
    RichterStartHitScript(SCRIPT_AIR_STEP2);
    jsr_pop();
    rl_script(log, SCRIPT_AIR_STEP2);
    WR8(OBJ->hurt_timer[0], AIR_STEP2_TIMER);
    rl_ended(log, ended_load(AIR_STEP2_TIMER));
    jsr_push(0xAAB1);
    RichterStopMoving();
    jsr_pop();
    rl_ended(log, ended_nz(INC8(OBJ->substate[0])));
}

/*
 * @routine 01BA:2B5 RichterAirStep3
 * @run 6D:AAB5
 * WHAT IT DOES: The air's step 3: shows one of two frames by bit 1 of the animation timer and counts
 * it down; at its end script 9, OBJ_ANIM_TIMER 1 ($28 unless slot 5 holds kind $11), the item's
 * first use set, the item's animation, step 0 (1 when slot 5 holds kind $0D), and the hitbox on.
 * HOW: RichterAirStep3Frames; RichterStartHitScript, RichterSetAnimation by RichterAirStep3Animations.
 * WAITS: no.
 * Entry: nothing. Exit: counting, A the frame, N and Z from the count, Y the frame's index, C bit 0 of
 * the timer. Ended, A slot 5's kind, N, Z and C from comparing it with $0D, or N and Z from the step;
 * X and Y as RichterSetAnimation left them.
 */
void RichterAirStep3(RegLog *log)
{
    uint8_t value, at, item;

    value = RD8(OBJ->hurt_timer[0]);
    at = (uint8_t)((value >> 1) & 0x01);
    value = (uint8_t)rd8((uint16_t)(RichterAirStep3Frames + at));
    WR8(OBJ->frame[0], value);
    {
        uint8_t timer = DEC8(OBJ->hurt_timer[0]);

        rl_ended(log, ended_with_carry(ended_with_y(ended_with_a(ended_nz(timer), value), at),
                                       ((uint8_t)(timer + 1) & 0x01) != 0));
        if (timer != 0) {
            return;
        }
    }
    jsr_push(0xAACC);
    RichterStartHitScript(SCRIPT_AIR_STEP3_END);
    jsr_pop();
    rl_script(log, SCRIPT_AIR_STEP3_END);
    WR8(OBJ->anim_timer[0], 1);
    value = RD8(OBJ->kind[SLOT_FIVE]);
    rl_ended(log, ended_compare(value, KIND_SLOT_FIVE_SHORT));
    if (value != KIND_SLOT_FIVE_SHORT) {
        WR8(OBJ->anim_timer[0], AIR_STEP3_LONG_TIMER);
    }
    WR8(PVARS->item_first_use, 1);
    item = RD8(ZP->field93);
    value = rd8((uint16_t)(RichterAirStep3Animations + item));
    rl_ended(log, ended_with_y(ended_load(value), item));
    jsr_push(0xAAEA);
    rl_animation(log, RichterSetAnimation(value));
    jsr_pop();
    WR8(OBJ->substate[0], 0);
    WR8(OBJ->hitbox_on[0], 1);
    value = RD8(OBJ->kind[SLOT_FIVE]);
    rl_ended(log, ended_compare(value, KIND_SLOT_FIVE_NEXT));
    if (value != KIND_SLOT_FIVE_NEXT) {
        return;
    }
    rl_ended(log, ended_nz(INC8(OBJ->substate[0])));
}

/*
 * @routine 01BA:65E RichterStateLandCrouch
 * @run 6D:AE5E
 * WHAT IT DOES: State 10, the crouch a hard landing ends in: falls if the footing goes; else stands on
 * the floor (stopped at a ledge) and, when the animation ends, crouches (state 2, step 0).
 * HOW: RichterReadColumns, RichterHasNoFooting, RichterToFall (no drift); RichterLandOnFloor,
 * RichterLedgeCheck, RichterStopX, RichterAnimationWait.
 * WAITS: no.
 * Entry: nothing. Exit: fell, A $FF, N set, Z clear, the rest as RichterToFall. Waiting, as
 * RichterAnimationWait. Crouched, A 0, Z set; the rest as RichterAnimationWait.
 */
void RichterStateLandCrouch(RegLog *log)
{
    FootingCheck footing;
    LedgeCheck ledge;

    jsr_push(0xAE61);
    rl_columns(log, RichterReadColumns());
    jsr_pop();
    jsr_push(0xAE64);
    footing = RichterHasNoFooting();
    jsr_pop();
    rl_footing(log, footing);
    if (footing.no_floor) {
        jsr_push(0xAE85);
        rl_fall(log, RichterToFall());
        jsr_pop();
        WR8(OBJ->fall_drift_delay[0], FALL_NO_DRIFT);
        rl_ended(log, ended_load(FALL_NO_DRIFT));
        return;
    }
    jsr_push(0xAE69);
    rl_landing(log, RichterLandOnFloor());
    jsr_pop();
    jsr_push(0xAE6C);
    ledge = RichterLedgeCheck();
    jsr_pop();
    rl_ledge(log, ledge);
    if (ledge.c) {
        jsr_push(0xAE71);
        RichterStopX();
        jsr_pop();
    }
    if (wait_dropped(log, 0xAE74)) {
        return;
    }
    jsr_push(0xAE77);
    RichterStopX();
    jsr_pop();
    WR8(OBJ->state[0], CROUCH_LAND_STATE);
    WR8(OBJ->substate[0], 0);
    rl_ended(log, ended_load(0));
}

/* The climb a landing on a marked floor goes into: state, frame and animation by the index (facing, or
 * facing plus 2 for the other side), after the move to the tile's edge ($AD8E). */
static void climb_land_set(RegLog *log, uint8_t at)
{
    uint8_t value;

    value = rd8((uint16_t)(RichterClimbLandFrames + at));
    WR8(OBJ->frame[0], value);
    value = rd8((uint16_t)(RichterClimbLandStates + at));
    WR8(OBJ->state[0], value);
    value = rd8((uint16_t)(RichterClimbLandAnimations + at));
    WR8(OBJ->animation[0], value);
    rl_ended_x(log, ended_load(value), at);
}

/* The same landing while attacking ($ADFE): the state, and the attack's animation for that climb. */
static void climb_land_attacking(RegLog *log, uint8_t at)
{
    uint8_t state, half, kind, index, value;
    Sum less, sum;

    state = rd8((uint16_t)(RichterClimbLandStates + at));
    WR8(OBJ->state[0], state);
    less = add_bytes(state, (uint8_t)~FIRST_CLIMB_STATE, true);
    half = (uint8_t)(less.value >> 1);
    WR8(ZP->temp00, half);
    kind = RD8(PVARS->attack_kind);
    sum = add_bytes((uint8_t)(kind << 1), RD8(ZP->temp00), false);
    index = sum.value;
    value = rd8((uint16_t)(RichterClimbLandAttackAnimations + index));
    WR8(OBJ->animation[0], value);
    rl_ended_x(log, ended_with_overflow(ended_with_carry(ended_load(value), sum.carry), sum.overflow), index);
    rl_animation(log, RichterAnimationTick());
}

/* A probe below him for the climb's mark (bit 2): X at his X plus dx, the row his Y plus dy. */
static bool climb_mark_below(RegLog *log, uint8_t dx, uint8_t dy, uint16_t ret)
{
    Sum at, row;
    TileRead read;
    uint8_t answer;

    at = add_bytes(RD8(OBJ->x_lo[0]), dx, false);
    WR8(ZP->temp00, at.value);
    row = add_bytes(RD8(OBJ->y_lo[0]), dy, false);
    rl_ended(log, ended_with_overflow(ended_with_carry(ended_load(row.value), row.carry), row.overflow));
    jsr_push(ret);
    read = RichterReadBelow(row.value, row.carry);
    jsr_pop();
    rl_read_below(log, read, !row.carry);
    answer = row.carry ? 0 : tile_answer(read);
    rl_ended(log, ended_load((uint8_t)(answer & CLIMB_MARK)));
    return (answer & CLIMB_MARK) != 0;
}

/* $AD76 on: an ordinary landing unless PlayerClimbLandFacing is 0; then into the climb on the right side,
 * at step 1 attacking. */
static void climb_land_right(RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t value, column;
    MovedStopped moved;

    value = RD8(PVARS->climb_land_facing);
    rl_ended(log, ended_load(value));
    if (value != 0) {
        log_regs(log, &x, &y);
        RichterLand(log, x, y);
        return;
    }
    value = RD8(OBJ->substate[0]);
    rl_ended(log, ended_compare(value, STEP_ATTACK));
    column = RD8(PVARS->column_x[0]);
    if (value == STEP_ATTACK) {
        Sum sum;

        value = rd8((uint16_t)(RichterClimbLandStepsRight + column));
        sum = add_bytes(value, RD8(OBJ->x_lo[0]), false);
        WR8(OBJ->x_lo[0], sum.value);
        climb_land_attacking(log, RD8(OBJ->facing_left[0]));
        return;
    }
    value = rd8((uint16_t)(RichterClimbLandStepsRight + column));
    jsr_push(0xAD8E);
    moved = RichterMoveStopped(value);
    jsr_pop();
    rl_move_stopped(log, moved.moved, moved.facing);
    climb_land_set(log, moved.facing);
}

/* $ADDB on: into the climb on the left side. */
static void climb_land_left(RegLog *log)
{
    uint8_t value, column;
    MovedStopped moved;

    value = RD8(OBJ->substate[0]);
    rl_ended(log, ended_compare(value, STEP_ATTACK));
    column = RD8(PVARS->column_x[0]);
    if (value == STEP_ATTACK) {
        Sum sum;

        value = rd8((uint16_t)(RichterClimbLandStepsLeft + column));
        sum = add_bytes(value, RD8(OBJ->x_lo[0]), false);
        WR8(OBJ->x_lo[0], sum.value);
        climb_land_attacking(log, (uint8_t)(RD8(OBJ->facing_left[0]) + 2));
        return;
    }
    value = rd8((uint16_t)(RichterClimbLandStepsLeft + column));
    jsr_push(0xADEA);
    moved = RichterMoveStopped(value);
    jsr_pop();
    rl_move_stopped(log, moved.moved, moved.facing);
    climb_land_set(log, (uint8_t)(moved.facing + 2));
}

/* What RichterKnockSpeedX leaves: A the speed's high byte, Y twice the side, C bit 7 of the side. */
static void knock_speed_log(RegLog *log, uint8_t side, uint8_t high)
{
    rl_ended(log, ended_with_carry(ended_with_y(ended_load(high), (uint8_t)(side << 1)), (side & 0x80) != 0));
}

/* $AD2C on, shared by both landings on a mark: the landing's sound when knocked back or falling at full
 * speed, the forced pad, PlayerMayLeaveTop, the stop; then the probes for the climb's side. */
static void land_on_mark(RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t value;

    bool sounded;

    value = (uint8_t)(RD8(PVARS2->jump_kind) & JUMP_KNOCKED);
    rl_ended(log, ended_load(value));
    sounded = value != 0;
    if (!sounded) {
        value = RD8(OBJ->vy_hi[0]);
        rl_ended(log, ended_compare(value, FALL_TOP_SPEED));
        sounded = value == FALL_TOP_SPEED;
    }
    if (sounded) {
        jsr_push(0xAD3D);
        rl_slots(log, RichterReplaceSlotsOneTwo());
        jsr_pop();
        rl_ended(log, ended_load(SOUND_LAND));
        log_regs(log, &x, &y);
        jsr_push(0xAD42);
        rl_sound(log, SOUND_LAND, SoundCommand(SOUND_LAND, x, y));
        jsr_pop();
        WR8(OBJ->pad_force_frames[0], MARK_FORCE_FRAMES);
        WR8(OBJ->pad_forced[0], 0);
        rl_ended(log, ended_load(MARK_FORCE_FRAMES));
    }
    WR8(PVARS2->jump_kind, 0);
    WR8(PVARS->may_leave_top, 1);
    WR8(PVARS2->jump_kind, 0);
    WR8(PVARS->climb_land_facing, 0);
    rl_ended(log, ended_load(1));
    jsr_push(0xAD5B);
    RichterStopMoving();
    jsr_pop();
    WR8(OBJ->x_frac[0], 0);
    WR8(OBJ->y_frac[0], 0);
    if (climb_mark_below(log, CLIMB_PROBE_RIGHT, CLIMB_PROBE_LOW, 0xAD72)) {
        climb_land_right(log, x, y);
        return;
    }
    if (climb_mark_below(log, CLIMB_PROBE_LEFT, CLIMB_PROBE_LOW, 0xADB2)) {
        climb_land_left(log);
        return;
    }
    value = RD8(OBJ->facing_left[0]);
    WR8(PVARS->climb_land_facing, value);
    if (climb_mark_below(log, CLIMB_PROBE_LEFT, CLIMB_PROBE_DEEP, 0xADCD)) {
        climb_land_right(log, x, y);
        return;
    }
    value = (uint8_t)(RD8(OBJ->facing_left[0]) ^ 0x01);
    rl_ended(log, ended_load(value));
    if (value == 0) {
        climb_land_left(log);
        return;
    }
    log_regs(log, &x, &y);
    RichterLand(log, x, y);
}

/*
 * @routine 01BA:514 RichterLandOnMarkedFeet
 * @run 6D:AD14
 * WHAT IT DOES: Lands on the mark under his feet: his Y set 8 below where the read met it, then as
 * RichterLandOnMarked from its sound on.
 * HOW: zTemp02, the read's row; a branch into RichterLandOnMarked at $AD2C.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as RichterLandOnMarked.
 */
void RichterLandOnMarkedFeet(RegLog *log, uint8_t x, uint8_t y)
{
    Sum sum, diff;

    sum = add_bytes(RD8(OBJ->y_lo[0]), MARKED_LIFT, false);
    diff = add_bytes(sum.value, (uint8_t)~RD8(ZP->temp02), true);
    WR8(OBJ->y_lo[0], diff.value);
    rl_ended(log, ended_with_overflow(ended_with_carry(ended_load(diff.value), diff.carry), diff.overflow));
    land_on_mark(log, x, y);
}

/*
 * @routine 01BA:522 RichterLandOnMarked
 * @run 6D:AD22
 * WHAT IT DOES: Lands on a marked floor: stands on it (the first column's row), sounds the landing when
 * knocked back or at full fall speed (and forces the pad for 6 frames), lets him go above the map, stops
 * him; then a mark below him a step to either side puts him on the climb that way, at its tile's edge,
 * going on with the attack if he was attacking; otherwise an ordinary landing.
 * HOW: RichterColumnRow; RichterReplaceSlotsOneTwo and SoundCommand 2; RichterReadBelow at his X plus
 * and less 8, and $0E and $1E below; RichterMoveStopped by RichterClimbLandStepsRight or Left; the
 * RichterClimbLand tables; jumps to RichterLand.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as RichterLand, or the climb's last store, or as
 * RichterAnimationTick when attacking.
 */
void RichterLandOnMarked(RegLog *log, uint8_t x, uint8_t y)
{
    Sum diff;

    diff = add_bytes(RD8(OBJ->y_lo[0]), (uint8_t)~RD8(PVARS->column_row[0]), true);
    WR8(OBJ->y_lo[0], diff.value);
    rl_ended(log, ended_with_overflow(ended_with_carry(ended_load(diff.value), diff.carry), diff.overflow));
    land_on_mark(log, x, y);
}

/*
 * @routine 01BA:4E9 RichterLandAttacking
 * @run 6D:ACE9
 * WHAT IT DOES: Lands while attacking: state 0, the attack going on in the standing attack's
 * animation for its kind; stands when it has ended, else stops.
 * HOW: RichterPadOverridesOff, RichterLandAttackAnimations into OBJ_ANIMATION, RichterAnimationRunning;
 * tail jumps to RichterStopMoving and RichterStand.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterAnimationRunning, X the attack kind; ended, as RichterStand.
 */
void RichterLandAttacking(RegLog *log)
{
    AnimationStep running;
    uint8_t kind, value;

    jsr_push(0xACEC);
    RichterPadOverridesOff();
    jsr_pop();
    WR8(PVARS2->jump_kind, 0);
    WR8(OBJ->state[0], 0);
    kind = RD8(PVARS->attack_kind);
    value = rd8((uint16_t)(RichterLandAttackAnimations + kind));
    WR8(OBJ->animation[0], value);
    rl_ended_x(log, ended_load(value), kind);
    jsr_push(0xAD00);
    running = RichterAnimationRunning();
    jsr_pop();
    rl_animation(log, running);
    if (running.nz != 0) {
        RichterStopMoving();
        return;
    }
    rl_animation(log, RichterStand());
}

/*
 * @routine 01BA:485 RichterLandHard
 * @run 6D:AC85
 * WHAT IT DOES: A hard landing, at full fall speed or out of a knock-back: slots 1 and 2 to kind 4; an
 * attack lands as RichterLandAttacking; else sound 2 and the landing crouch (state 10, animation 7), a
 * knock-back sliding on backward unless pushed; at step 1 (an attack's step), the crouch's animation
 * while it runs, then a plain crouch (state 2) with down forced for 16 frames.
 * HOW: RichterPadOverridesOff, RichterReplaceSlotsOneTwo, SoundCommand, RichterSetAnimation; bit 0 of
 * PlayerJumpKind; TST of zPlayerPush; RichterKnockSpeedX by facing; RichterAnimationRunning.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as the last routine it calls; the crouch, A $40, N and Z
 * from it.
 */
void RichterLandHard(RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t value;
    AnimationStep running;

    jsr_push(0xAC88);
    RichterPadOverridesOff();
    jsr_pop();
    jsr_push(0xAC8B);
    rl_slots(log, RichterReplaceSlotsOneTwo());
    jsr_pop();
    value = RD8(PVARS->attack_kind);
    rl_ended(log, ended_load(value));
    if (value != 0) {
        RichterLandAttacking(log);
        return;
    }
    rl_ended(log, ended_load(SOUND_LAND));
    log_regs(log, &x, &y);
    jsr_push(0xAC95);
    rl_sound(log, SOUND_LAND, SoundCommand(SOUND_LAND, x, y));
    jsr_pop();
    value = RD8(OBJ->substate[0]);
    rl_ended(log, ended_compare(value, STEP_ATTACK));
    if (value == STEP_ATTACK) {
        WR8(OBJ->animation[0], LAND_CROUCH_ANIMATION);
        rl_ended(log, ended_load(LAND_CROUCH_ANIMATION));
        jsr_push(0xACD4);
        running = RichterAnimationRunning();
        jsr_pop();
        rl_animation(log, running);
        if (running.nz != 0) {
            WR8(OBJ->state[0], CROUCH_LAND_STATE);
            jsr_push(0xACDE);
            RichterStopMoving();
            jsr_pop();
            WR8(OBJ->pad_force_frames[0], LAND_FORCE_FRAMES);
            WR8(OBJ->pad_forced[0], LAND_FORCE_PAD);
            rl_ended(log, ended_load(LAND_FORCE_PAD));
            return;
        }
    }
    WR8(OBJ->state[0], LAND_CROUCH_STATE);
    WR8(OBJ->substate[0], 0);
    rl_ended(log, ended_load(ANIM_LAND));
    jsr_push(0xACAB);
    rl_animation(log, RichterSetAnimation(ANIM_LAND));
    jsr_pop();
    value = RD8(PVARS2->jump_kind);
    WR8(PVARS2->jump_kind, 0);
    value &= JUMP_KNOCKED;
    rl_ended(log, ended_load(value));
    if (value == 0) {
        RichterStopMoving();
        return;
    }
    value = RD8(ZP->player_push);
    rl_tst(log, LAND_SLIDE_PUSH, value);
    if (value & LAND_SLIDE_PUSH) {
        RichterStopMoving();
        return;
    }
    WR8(OBJ->vy_lo[0], 0);
    WR8(OBJ->vy_hi[0], 0);
    value = RD8(OBJ->facing_left[0]);
    knock_speed_log(log, value, RichterKnockSpeedX(value));
}

/*
 * @routine 01BA:457 RichterLand
 * @run 6D:AC57
 * WHAT IT DOES: Lands: hard at full fall speed; attacking, as RichterLandAttacking; out of a jump the way
 * he faced, or any plain jump, he stands; out of a knock-back, hard.
 * HOW: OBJ_VY_HI against 7; OBJ_SUBSTATE against 1; PlayerJumpKind's bit 7, then not zero; jumps to
 * RichterLandHard, RichterLandAttacking, RichterStand.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as the routine it goes to.
 */
void RichterLand(RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t value;

    value = RD8(OBJ->vy_hi[0]);
    rl_ended(log, ended_compare(value, FALL_TOP_SPEED));
    if (value == FALL_TOP_SPEED) {
        RichterLandHard(log, x, y);
        return;
    }
    value = RD8(OBJ->substate[0]);
    rl_ended(log, ended_compare(value, STEP_ATTACK));
    if (value == STEP_ATTACK) {
        RichterLandAttacking(log);
        return;
    }
    value = RD8(PVARS2->jump_kind);
    rl_ended(log, ended_load(value));
    if (!(value & 0x80) && value != 0) {
        RichterLandHard(log, x, y);
        return;
    }
    rl_animation(log, RichterStand());
}

/*
 * @routine 01BA:451 RichterLandOnColumn
 * @run 6D:AC51
 * WHAT IT DOES: Lands on the floor at a column: lifted out of it, PlayerWhipStruck cleared, then as
 * RichterLand.
 * HOW: RichterLiftByColumn; runs on into RichterLand.
 * WAITS: no.
 * Entry: X the column; Y for the sound driver. Exit: as RichterLand.
 */
void RichterLandOnColumn(RegLog *log, uint8_t column, uint8_t x, uint8_t y)
{
    (void)x;
    jsr_push(0xAC54);
    rl_column_lift(log, RichterLiftByColumn(column));
    jsr_pop();
    WR8(PVARS->whip_struck, 0);
    x = column;
    log_regs(log, &x, &y);
    RichterLand(log, x, y);
}

/*
 * @routine 01BA:475 RichterAirAttack
 * @run 6D:AC75
 * WHAT IT DOES: Starts an attack in the air: its animation by the attack's kind, then as
 * RichterAttackWithAnimation.
 * HOW: RichterPadOverridesOff; RichterAirAttackAnimations; a jump to RichterAttackWithAnimation.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterAttackWithAnimation.
 */
void RichterAirAttack(RegLog *log)
{
    uint8_t kind, value;

    jsr_push(0xAC78);
    RichterPadOverridesOff();
    jsr_pop();
    kind = RD8(PVARS->attack_kind);
    value = rd8((uint16_t)(RichterAirAttackAnimations + kind));
    rl_ended_x(log, ended_load(value), kind);
    rl_attack(log, RichterAttackWithAnimation(value));
}

/*
 * @routine 01BA:163 RichterStateFall
 * @run 6D:A963
 * WHAT IT DOES: State 4, falling: lands on a floor whose column answers 0; once OBJ_FALL_DRIFT_DELAY runs
 * out drifts by facing; debris on marked columns; lands on a marked floor or marked ground; gravity at
 * $48 a frame to $0700; at step 0 the item may be used (an attack in the air), at step 1 (attacking)
 * the attack runs to its end, frame $12, step 0.
 * HOW: The landing check; RichterFallDriftSpeeds; RichterDebrisOnMarkedColumns; RichterOnMarkedFloor and
 * RichterOnMarkedUnderFeet; RichterNoteItemPress, RichterUseItem, RichterAirAttack; RichterAnimationWait.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as the path's last routine; attack ended, A 0, Z set.
 */
void RichterStateFall(RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t value, column = 0, facing, at;
    AirFloor marked;
    ItemUse use;
    Sum low, high;

    if (air_landing(log, &column, 0xA966, 0xA969, 0xA96E)) {
        log_regs(log, &x, &y);
        RichterLandOnColumn(log, column, x, y);
        return;
    }
    value = RD8(OBJ->fall_drift_delay[0]);
    rl_ended(log, ended_load(value));
    if (value != 0) {
        value = DEC8(OBJ->fall_drift_delay[0]);
        rl_ended(log, ended_nz(value));
        if (value == 0) {
            facing = RD8(OBJ->facing_left[0]);
            at = (uint8_t)(facing << 1);
            value = rd8((uint16_t)(RichterFallDriftSpeeds + at));
            WR8(OBJ->vx_lo[0], value);
            value = rd8((uint16_t)(RichterFallDriftSpeeds + at + 1));
            WR8(OBJ->vx_hi[0], value);
            rl_ended(log, ended_with_carry(ended_with_y(ended_load(value), at), (facing & 0x80) != 0));
        }
    }
    log_regs(log, &x, &y);
    jsr_push(0xA991);
    rl_debris(log, RichterDebrisOnMarkedColumns(x, y));
    jsr_pop();
    marked = air_marked(log, 0xA994, 0xA99C);
    if (marked != AIR_NONE) {
        log_regs(log, &x, &y);
        if (marked == AIR_MARKED) {
            RichterLandOnMarked(log, x, y);
        } else {
            RichterLandOnMarkedFeet(log, x, y);
        }
        return;
    }
    low = add_bytes(RD8(OBJ->vy_lo[0]), FALL_PULL_LO, false);
    WR8(OBJ->vy_lo[0], low.value);
    high = add_bytes(RD8(OBJ->vy_hi[0]), 0, low.carry);
    WR8(OBJ->vy_hi[0], high.value);
    rl_gravity(log, high);
    if (!(high.value & 0x80) && high.value >= FALL_TOP_SPEED) {
        WR8(OBJ->vy_lo[0], 0);
    }
    value = RD8(OBJ->substate[0]);
    rl_ended(log, ended_compare(value, 0));
    if (value == 0) {
        jsr_push(0xA9C5);
        rl_note_press(log, RichterNoteItemPress());
        jsr_pop();
        jsr_push(0xA9C8);
        use = RichterUseItem();
        jsr_pop();
        rl_item_use(log, use);
        if (use.how != USE_NO_PRESS) {
            RichterAirAttack(log);
        }
        return;
    }
    if (wait_dropped(log, 0xA9D1)) {
        return;
    }
    WR8(OBJ->frame[0], FRAME_FALL_END);
    WR8(OBJ->substate[0], 0);
    WR8(PVARS->attack_kind, 0);
    rl_ended(log, ended_load(FRAME_FALL_END));
}

/*
 * @routine 01BA:256 RichterAirStep1
 * @run 6D:AA56
 * WHAT IT DOES: The air's step 1, an attack in the air: the pad's turn if he has not turned and his
 * scroll band is below 3; while falling, the landings as RichterStateFall makes them and the debris;
 * gravity; when the attack's animation ends, frame $12, step 0, no attack.
 * HOW: RichterAirControl; the landing check, RichterOnMarkedFloor, RichterOnMarkedUnderFeet,
 * RichterDebrisOnMarkedColumns; RichterFallGravity; RichterAnimationWait.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: as the path's last routine; ended, A $12, N and Z from it.
 */
void RichterAirStep1(RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t value, column = 0;
    AirFloor marked;

    value = RD8(ZP->field_c6);
    rl_ended(log, ended_load(value));
    if (value == 0) {
        value = RD8(OBJ->scroll_band[0]);
        rl_ended(log, ended_compare(value, BAND_AIR_CONTROL));
        if (value < BAND_AIR_CONTROL) {
            jsr_push(0xAA64);
            RichterAirControl(log);
            jsr_pop();
        }
    }
    value = RD8(OBJ->vy_hi[0]);
    rl_ended(log, ended_load(value));
    if (!(value & 0x80)) {
        if (air_landing(log, &column, 0xAA6C, 0xAA6F, 0xAA74)) {
            log_regs(log, &x, &y);
            RichterLandOnColumn(log, column, x, y);
            return;
        }
        marked = air_marked(log, 0xAA7C, 0xAA84);
        if (marked != AIR_NONE) {
            log_regs(log, &x, &y);
            if (marked == AIR_MARKED) {
                RichterLandOnMarked(log, x, y);
            } else {
                RichterLandOnMarkedFeet(log, x, y);
            }
            return;
        }
        log_regs(log, &x, &y);
        jsr_push(0xAA8C);
        rl_debris(log, RichterDebrisOnMarkedColumns(x, y));
        jsr_pop();
    }
    jsr_push(0xAA8F);
    rl_gravity(log, RichterFallGravity());
    jsr_pop();
    if (wait_dropped(log, 0xAA92)) {
        return;
    }
    WR8(PVARS->attack_kind, 0);
    WR8(OBJ->frame[0], FRAME_FALL_END);
    WR8(OBJ->substate[0], 0);
    rl_ended(log, ended_load(FRAME_FALL_END));
}

/*
 * @routine 01BA:3E6 RichterAirStep0
 * @run 6D:ABE6
 * WHAT IT DOES: The air's step 0: in a plain jump not carrying him the way he faces, with his scroll band
 * below 2 (band 1, or the animation timer below 8), a new press of I is a knock-back (RichterKnockBack);
 * otherwise the pad's turn if he has not turned, the landings and the debris while falling as
 * RichterAirStep1, gravity, the animation, and the item's use (an attack in the air).
 * HOW: PlayerJumpKind; bit 0 of OBJ_VX_HI's shifts against the facing; OBJ_SCROLL_BAND; OBJ_HURT_TIMER at X;
 * zPadPressed; then as RichterAirStep1's checks, RichterAnimationTick, RichterNoteItemPress, RichterUseItem.
 * WAITS: no.
 * Entry: X the step's offset (0); X and Y for the sound driver. Exit: as the path's last routine.
 */
void RichterAirStep0(RegLog *log, uint8_t x, uint8_t y)
{
    uint8_t value, column = 0;
    AirFloor marked;
    ItemUse use;
    bool knock = false;

    value = RD8(PVARS2->jump_kind);
    rl_ended(log, ended_load(value));
    if (value == 0) {
        bool backward = true;

        value = RD8(OBJ->vx_hi[0]);
        rl_ended(log, ended_load(value));
        if (value != 0) {
            uint8_t shifted = (uint8_t)((value >> 1 >> 1) | ((value & 0x01) << 7));

            shifted ^= RD8(OBJ->facing_left[0]);
            shifted &= 0x01;
            rl_ended(log, ended_with_carry(ended_load(shifted), (value & 0x02) != 0));
            backward = shifted != 0;
        }
        if (backward) {
            value = RD8(OBJ->scroll_band[0]);
            rl_ended(log, ended_compare(value, BAND_KNOCK_TEST));
            if (value < BAND_KNOCK_TEST) {
                bool timer_ok = true;

                value = (uint8_t)(value - 1);
                rl_ended(log, ended_nz(value));
                if (value != 0) {
                    value = RD8(OBJ->hurt_timer[x]);
                    rl_ended(log, ended_compare(value, KNOCK_TIMER_LIMIT));
                    timer_ok = value < KNOCK_TIMER_LIMIT;
                }
                if (timer_ok) {
                    value = (uint8_t)(rd8(zPadPressed) & PAD_I);
                    rl_ended(log, ended_load(value));
                    knock = value != 0;
                }
            }
        }
    }
    if (knock) {
        rl_knock(log, RichterKnockBack());
        return;
    }
    value = RD8(ZP->field_c6);
    rl_ended(log, ended_load(value));
    if (value == 0) {
        jsr_push(0xAC1A);
        RichterAirControl(log);
        jsr_pop();
    }
    value = RD8(OBJ->vy_hi[0]);
    rl_ended(log, ended_load(value));
    if (!(value & 0x80)) {
        if (air_landing(log, &column, 0xAC22, 0xAC25, 0xAC2A)) {
            log_regs(log, &x, &y);
            RichterLandOnColumn(log, column, x, y);
            return;
        }
        marked = air_marked(log, 0xAC32, 0xAC3A);
        if (marked != AIR_NONE) {
            log_regs(log, &x, &y);
            if (marked == AIR_MARKED) {
                RichterLandOnMarked(log, x, y);
            } else {
                RichterLandOnMarkedFeet(log, x, y);
            }
            return;
        }
        log_regs(log, &x, &y);
        jsr_push(0xAC42);
        rl_debris(log, RichterDebrisOnMarkedColumns(x, y));
        jsr_pop();
    }
    jsr_push(0xAC45);
    rl_gravity(log, RichterFallGravity());
    jsr_pop();
    jsr_push(0xAC48);
    rl_animation(log, RichterAnimationTick());
    jsr_pop();
    jsr_push(0xAC4B);
    rl_note_press(log, RichterNoteItemPress());
    jsr_pop();
    jsr_push(0xAC4E);
    use = RichterUseItem();
    jsr_pop();
    rl_item_use(log, use);
    if (use.how != USE_NO_PRESS) {
        RichterAirAttack(log);
    }
}

/*
 * @routine 01BA:3BD RichterStateAir
 * @run 6D:ABBD
 * WHAT IT DOES: States 3, 11 and 12, in the air: runs the step's handler.
 * HOW: The four words at $ABC3 by RichterStepIndex.
 * WAITS: no.
 * Entry: Y for the sound driver. Exit: as the handler leaves them.
 */
void RichterStateAir(RegLog *log, uint8_t y)
{
    uint8_t step, at;
    uint16_t entry, handler;

    jsr_push(0xABC0);
    step = RichterStepIndex();
    jsr_pop();
    at = (uint8_t)(step << 1);
    rl_ended_x(log, ended_with_carry(ended_load(at), (step & 0x80) != 0), at);
    entry = (uint16_t)(RichterAirSteps + at);
    handler = (uint16_t)(rd8(entry) | (rd8((uint16_t)(entry + 1)) << 8));
    switch (handler) {
    case 0xABE6:
        RichterAirStep0(log, at, y);
        break;
    case 0xAA56:
        RichterAirStep1(log, at, y);
        break;
    case 0xAA9E:
        RichterAirStep2(log);
        break;
    default:
        RichterAirStep3(log);
        break;
    }
}

/* The wrappers of the routines that keep an exit log. */
#define LOG_ABI_XY(name) void abi_##name(void) { RegLog log = { 0 }; name(&log, cpu.x, cpu.y); reg_log_replay(&log); }
#define LOG_ABI(name) void abi_##name(void) { RegLog log = { 0 }; name(&log); reg_log_replay(&log); }
LOG_ABI(RichterAirTurnSpeed)
LOG_ABI(RichterAirTurnFacingLeft)
LOG_ABI(RichterAirTurnStill)
LOG_ABI(RichterAirControl)
LOG_ABI(RichterFallAttacking)
LOG_ABI(RichterAirStep2)
LOG_ABI(RichterAirStep3)
LOG_ABI(RichterStateLandCrouch)
LOG_ABI_XY(RichterLandOnMarkedFeet)
LOG_ABI_XY(RichterLandOnMarked)
LOG_ABI(RichterLandAttacking)
LOG_ABI_XY(RichterLandHard)
LOG_ABI_XY(RichterLand)
LOG_ABI(RichterAirAttack)
LOG_ABI_XY(RichterStateFall)
LOG_ABI_XY(RichterAirStep1)
LOG_ABI_XY(RichterAirStep0)

void abi_RichterDownOnFirstColumnFloor(void)
{
    RegLog log = { 0 };

    RichterDownOnFirstColumnFloor(&log);
    reg_log_replay(&log);
}

void abi_RichterLandOnColumn(void)
{
    RegLog log = { 0 };

    RichterLandOnColumn(&log, cpu.x, cpu.x, cpu.y);
    reg_log_replay(&log);
}

void abi_RichterStateAir(void)
{
    RegLog log = { 0 };

    RichterStateAir(&log, cpu.y);
    reg_log_replay(&log);
}
