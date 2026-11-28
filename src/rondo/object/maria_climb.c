/*
 * Maria on a ledge (states 5 to 8, 01DE-01E0 in bank $6D): her hold on the ledge ahead (5, 6) or behind her (7, 8),
 * the steps by OBJ_SUBSTATE (MariaClimbLowSteps, MariaClimbHighSteps): holding, waiting on the animation, going across
 * to the edge (PlayerClimbReach), turning, going up; and the checks a hold makes: the jump (or the ladder below), the
 * item crash, the attack, the pad that climbs or changes her hold.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"

enum {
    OBJECT_PLAYER = 0,
    STATE_CLIMB_FIRST = 5, STATE_CLIMB_BEHIND = 7,
    STEP_UP = 4, FRAME_HOLD = 5, FRAME_ACROSS = 6, ACROSS_TIMER = 6, HOLD_STOPPED = 0xFF,
    ANIMATION_HOLD_UP = 0x15, ANIMATION_ATTACK = 0x20, ANIMATION_GOING_UP = 0x13, ANIMATION_TURNED = 0x12,
    CLIMB_PAD_DIRECTIONS = 0xF0, CLIMB_PAD_BUTTONS = 0x03, CLIMB_TURN_FRAMES = 7, LEDGE_BIT = 0x04,
    BEHIND_LOOKS = 4, PAD_DOWN = 0x40, PAD_II = 0x02, PAD_I = 0x01, PAD_UP = 0x10,
    CLIMB_STEP_HOLD = 0xAE08, CLIMB_STEP_WAIT = 0xAE71, CLIMB_STEP_ACROSS = 0xAE77, CLIMB_STEP_TURN = 0xAEE2,
    CLIMB_STEP_UP = 0xAF31, CLIMB_STEP_CRASH = 0xA4E7, CLIMB_STEP_HOLD_BEHIND = 0xAF4C
};

#define MariaProbeOffsets_TBL MariaProbeOffsets
#define MariaClimbMoveSpeeds_TBL MariaClimbMoveSpeeds
#define MariaClimbPadsA_TBL MariaClimbPadsA
#define MariaClimbPadsB_TBL MariaClimbPadsB
#define MariaClimbPadsC_TBL MariaClimbPadsC
#define MariaClimbTurnPadsA_TBL MariaClimbTurnPadsA
#define MariaClimbTurnPadsB_TBL MariaClimbTurnPadsB
#define MariaClimbTurnPadsC_TBL MariaClimbTurnPadsC
#define MariaClimbTurnAnimations_TBL MariaClimbTurnAnimations
#define MariaClimbTurnFrames_TBL MariaClimbTurnFrames
#define MariaClimbTurnFacings_TBL MariaClimbTurnFacings
#define MariaClimbTurnStates_TBL MariaClimbTurnStates
#define MariaClimbLowLooks_TBL MariaClimbLowLooks
#define MariaClimbFacings_TBL MariaClimbFacings
#define MariaClimbTurnPads_TBL MariaClimbTurnPads
#define MariaClimbLowSteps_TBL MariaClimbLowSteps
#define MariaClimbHighSteps_TBL MariaClimbHighSteps

/* ------------------------------------------------------------------------------------------------ the checks */

/*
 * @routine 01E0:2C7 MariaReadAtOffset
 * @run 6D:BAC7
 * WHAT IT DOES: RichterReadAtOffset in Maria's overlay: reads the terrain at a point offset from her own X and Y low
 * bytes by a pair from MariaProbeOffsets.
 * HOW: X low plus the table's byte at Y into zTemp00; Y low plus the next becomes the row, its carry the Y high's; the
 * tail JMP to MariaReadOnScreen.
 * WAITS: no.
 * Entry: Y the table's offset. Exit: as MariaReadOnScreen.
 */
OffsetRead MariaReadAtOffset(uint8_t offset)
{
    OffsetRead r;
    uint8_t x_lo = RD8(OBJ->x_lo[OBJECT_PLAYER]);
    Sum column = add_bytes(x_lo, TBL8(MariaProbeOffsets, offset), false);
    uint8_t y_lo;

    WR8(ZP->temp00, column.value);
    y_lo = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    r.row = add_bytes(y_lo, TBL8(MariaProbeOffsets, offset + 1), false);
    /* The read may run the stage's own code, entered with this add's A and flags. */
    abi_ended(ended_of_sum(r.row));
    r.read = MariaReadOnScreen(r.row.value, r.row.carry);
    return r;
}

void abi_MariaReadAtOffset(void)
{
    offset_read_exit_shim(MariaReadAtOffset(cpu.y));
}

/*
 * @routine 01E0:37A MariaJumpOrClimb
 * @run 6D:BB7A
 * WHAT IT DOES: RichterJumpOrClimb in Maria's overlay: with the jump pressed, the pad's second forced press is
 * cleared and, with down held, the answer is whether there is a ladder below her (MariaLadderCheck); else the jump.
 * HOW: A BSR to MariaJumpPressed; its CLC and SEC tails; the tail JMP to MariaLadderCheck.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterJumpOrClimb (C the answer).
 */
JumpOrClimb MariaJumpOrClimb(void)
{
    JumpOrClimb result = { 0 };

    jsr_push(0xBB7C);
    result.pressed = MariaJumpPressed();
    jsr_pop();
    if (result.pressed) {
        WR8(PVARS->forced_press_b, 0);
        result.held = RD8(ZP->pad_held);
        result.up_held = (result.held & PAD_DOWN) != 0;
        if (result.up_held) {
            result.ladder = MariaLadderCheck();
        }
    }
    return result;
}

void abi_MariaJumpOrClimb(void)
{
    jump_or_climb_exit_shim(MariaJumpOrClimb());
}

/*
 * @routine 01E0:424 MariaOrForcedPressB
 * @run 6D:BC24
 * WHAT IT DOES: ORs A into the pad's second forced press.
 * HOW: ORA, STA.
 * WAITS: no.
 * Entry: A the bits. Exit: A the new forced press, N and Z from it. X, Y, C and V kept.
 */
Regs MariaOrForcedPressB(uint8_t bits)
{
    uint8_t value = (uint8_t)(bits | RD8(PVARS->forced_press_b));

    WR8(PVARS->forced_press_b, value);
    return r_lda(regs_kept(), value);
}

void abi_MariaOrForcedPressB(void)
{
    regs_exit(MariaOrForcedPressB(cpu.a));
}

/*
 * @routine 01E0:414 MariaNotePressesB
 * @run 6D:BC14
 * WHAT IT DOES: Keeps this frame's presses of buttons II and I in the pad's second forced press.
 * HOW: Bit 1 of zPadPressed by a BSR to MariaOrForcedPressB, ORed in once more, then bit 0, on into it.
 * WAITS: no.
 * Entry: nothing. Exit: A the forced press, N and Z from it. X, Y, C and V kept.
 */
Regs MariaNotePressesB(void)
{
    Regs r;
    uint8_t value = (uint8_t)(RD8(ZP->pad_pressed) & PAD_II);

    jsr_push(0xBC1A);
    r = MariaOrForcedPressB(value);
    jsr_pop();
    value = (uint8_t)(r.a | RD8(PVARS->forced_press_b));
    WR8(PVARS->forced_press_b, value);
    value = (uint8_t)(RD8(ZP->pad_pressed) & PAD_I);
    return MariaOrForcedPressB(value);
}

void abi_MariaNotePressesB(void)
{
    regs_exit(MariaNotePressesB());
}

/*
 * @routine 01E0:42B MariaItemOrAttack
 * @run 6D:BC2B
 * WHAT IT DOES: On a ledge: the attack button (MariaUseItem); if it did nothing, with button II held and up pressed,
 * her item thrown as MariaUseItem throws it with down held.
 * HOW: A BSR to MariaUseItem; BBR1 of zPadHeld, TST of zPadPressed; branches into MariaUseItem's CLC and its throw.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaUseItem; neither, C clear.
 */
MExit MariaItemOrAttack(void)
{
    MExit e = mx_none();
    TurnWindow use;
    uint8_t value;

    jsr_push(0xBC2D);
    use = MariaUseItem();
    jsr_pop();
    mx_turn(&e, use);
    if (use.end.c_set && use.end.c) {
        return e;
    }
    if (RD8(ZP->pad_held) & PAD_II) {
        value = RD8(ZP->pad_pressed);
        mx_regs(&e, r_tst(regs_kept(), PAD_UP, value));
        if (value & PAD_UP) {
            mx_turn(&e, MariaUseItemFromThrow());
            return e;
        }
    }
    mx_regs(&e, r_c(regs_kept(), false));
    return e;
}

void abi_MariaItemOrAttack(void)
{
    mx_exit(MariaItemOrAttack());
}

/* ------------------------------------------------------------------------------------------------ the hold */

/*
 * @routine 01DE:52C MariaClimbMove
 * @run 6D:AD2C
 * WHAT IT DOES: On a ledge: stopped, unless her animation has just gone to a new step, when she moves by her state's
 * speeds ($0200 or $FE00 across and down).
 * HOW: MariaStopMoving; OBJ_ANIMATION_AT kept in zClimbStepBefore around MariaAnimationRunning; MariaClimbMoveSpeeds by
 * state less 5 doubled.
 * WAITS: no.
 * Entry: nothing. Exit: stopped, as MariaAnimationRunning; the same step, A it, Z and C set; moved, A the Y speed's
 * high byte, N and Z from it, Y the index, C bit 7 of state less 5, V from that subtract.
 */
MExit MariaClimbMove(void)
{
    MExit e = mx_none();
    AnimationStep running;
    uint8_t value, before, index;
    Sum less;

    jsr_push(0xAD2F);
    MariaStopMoving();
    jsr_pop();
    value = RD8(OBJ->animation_at[OBJECT_PLAYER]);
    WR8(ZP->climb_step_before, value);
    jsr_push(0xAD37);
    running = MariaAnimationRunning();
    jsr_pop();
    mx_animation(&e, running);
    if (mx_zero(mx_one_animation(running))) {
        return e;
    }
    value = RD8(OBJ->animation_at[OBJECT_PLAYER]);
    before = RD8(ZP->climb_step_before);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, before));
    if (value == before) {
        return e;
    }
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    less = sub_bytes(value, STATE_CLIMB_FIRST, true);
    index = (uint8_t)(less.value << 1);
    value = TBL8(MariaClimbMoveSpeeds, index);
    WR8(OBJ->vx_hi[OBJECT_PLAYER], value);
    value = TBL8(MariaClimbMoveSpeeds, index + 1);
    WR8(OBJ->vy_hi[OBJECT_PLAYER], value);
    WR8(OBJ->vx_lo[OBJECT_PLAYER], 0);
    WR8(OBJ->vy_lo[OBJECT_PLAYER], 0);
    mx_regs(&e, r_v(r_c(r_lda(r_y_only(regs_kept(), index), value), (less.value & 0x80) != 0), less.overflow));
    return e;
}

void abi_MariaClimbMove(void)
{
    mx_exit(MariaClimbMove());
}

/*
 * @routine 01DE:563 MariaClimbPad
 * @run 6D:AD63
 * WHAT IT DOES: The pad on a ledge's hold: a climbing direction (MariaClimbPadsA, B, and C when the pad's second
 * forced press is clear) with neither button held starts her animation again; a turning direction
 * (MariaClimbTurnPadsA, B, and C when the forced press is clear) held for eight frames (zFieldC4) changes her hold:
 * animation, frame, facing and state from the tables, her animation stopped, step 0, no X or Y fraction.
 * HOW: zPadHeld's directions compared with the tables by state less 5; INC of zFieldC4; MariaSetAnimation with
 * OBJ_ANIMATION by the tail JMP.
 * WAITS: no.
 * Entry: nothing. Exit: as the last compare or test, the INC, the stores (A the state), or MariaSetAnimation; Y the
 * index, V from its subtract or the forced press's TST.
 */
MExit MariaClimbPad(void)
{
    MExit e = mx_none();
    uint8_t value, index, pad, forced, count;
    Sum less;
    bool climb = false, turn = false;

    value = RD8(OBJ->state[OBJECT_PLAYER]);
    less = sub_bytes(value, STATE_CLIMB_FIRST, true);
    index = less.value;
    mx_regs(&e, r_v(r_c(r_ldy(regs_kept(), index), less.carry), less.overflow));
    pad = (uint8_t)(RD8(ZP->pad_held) & CLIMB_PAD_DIRECTIONS);
    value = TBL8(MariaClimbPadsA, index);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), pad), pad, value));
    climb = pad == value;
    if (!climb) {
        value = TBL8(MariaClimbPadsB, index);
        mx_regs(&e, r_cmp(regs_kept(), pad, value));
        climb = pad == value;
    }
    if (!climb) {
        forced = RD8(PVARS->forced_press_b);
        mx_regs(&e, r_tst(regs_kept(), 0xFF, forced));
        if (forced == 0) {
            value = TBL8(MariaClimbPadsC, index);
            mx_regs(&e, r_cmp(regs_kept(), pad, value));
            climb = pad == value;
        }
    }
    if (climb) {
        value = RD8(ZP->pad_held);
        mx_regs(&e, r_tst(regs_kept(), CLIMB_PAD_BUTTONS, value));
        if (value & CLIMB_PAD_BUTTONS) {
            return e;
        }
        value = RD8(OBJ->animation[OBJECT_PLAYER]);
        mx_regs(&e, r_lda(regs_kept(), value));
        mx_animation(&e, MariaSetAnimation(value));
        return e;
    }
    pad = (uint8_t)(RD8(ZP->pad_held) & CLIMB_PAD_DIRECTIONS);
    value = TBL8(MariaClimbTurnPadsA, index);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), pad), pad, value));
    turn = pad == value;
    if (!turn) {
        value = TBL8(MariaClimbTurnPadsB, index);
        mx_regs(&e, r_cmp(regs_kept(), pad, value));
        turn = pad == value;
    }
    if (!turn) {
        forced = RD8(PVARS->forced_press_b);
        mx_regs(&e, r_tst(regs_kept(), 0xFF, forced));
        if (forced != 0) {
            return e;
        }
        value = TBL8(MariaClimbTurnPadsC, index);
        mx_regs(&e, r_cmp(regs_kept(), pad, value));
        if (pad != value) {
            return e;
        }
    }
    count = RD8(ZP->fieldc4);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), count), count, CLIMB_TURN_FRAMES));
    if (count < CLIMB_TURN_FRAMES) {
        mx_regs(&e, r_nz(regs_kept(), INC8(ZP->fieldc4)));
        return e;
    }
    WR8(ZP->fieldc4, 0);
    value = TBL8(MariaClimbTurnAnimations, index);
    WR8(OBJ->animation[OBJECT_PLAYER], value);
    WR8(OBJ->hurt_timer[OBJECT_PLAYER], HOLD_STOPPED);
    value = TBL8(MariaClimbTurnFrames, index);
    WR8(OBJ->frame[OBJECT_PLAYER], value);
    value = TBL8(MariaClimbTurnFacings, index);
    WR8(OBJ->facing_left[OBJECT_PLAYER], value);
    value = TBL8(MariaClimbTurnStates, index);
    WR8(OBJ->state[OBJECT_PLAYER], value);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    WR8(OBJ->x_frac[OBJECT_PLAYER], 0);
    WR8(OBJ->y_frac[OBJECT_PLAYER], 0);
    mx_regs(&e, r_lda(regs_kept(), value));
    return e;
}

void abi_MariaClimbPad(void)
{
    mx_exit(MariaClimbPad());
}

/*
 * @routine 01DE:79B MariaClimbLetGo
 * @run 6D:AF9B
 * WHAT IT DOES: Lets go of the ledge on the jump: the pad's overrides and second forced press cleared, the turn count
 * cleared; with down held, she drops if there is a ladder below her (and stays with none); else she jumps.
 * HOW: MariaClearPadOverrides; TST of zPadHeld; MariaLadderCheck's carry; the tail JMPs to MariaToFall and MariaJump.
 * WAITS: no.
 * Entry: nothing. Exit: no ladder, as MariaLadderCheck; else as MariaToFall or MariaJump.
 */
MExit MariaClimbLetGo(void)
{
    MExit e = mx_none();
    uint8_t held;
    LadderCheck ladder;
    AnimThen fall;

    jsr_push(0xAF9E);
    MariaClearPadOverrides();
    jsr_pop();
    WR8(PVARS->forced_press_b, 0);
    WR8(ZP->fieldc6, 0);
    held = RD8(ZP->pad_held);
    mx_regs(&e, r_tst(regs_kept(), PAD_DOWN, held));
    if (held & PAD_DOWN) {
        jsr_push(0xAFAB);
        ladder = MariaLadderCheck();
        jsr_pop();
        mx_ladder(&e, ladder);
        if (!mx_carry(e)) {
            return e;
        }
        WR8(PVARS->may_leave_top, 0);
        fall = MariaToFall();
        mx_animation(&e, fall.animation);
        mx_regs(&e, fall.regs);
        return e;
    }
    WR8(PVARS->may_leave_top, 0);
    mx_then(&e, MariaJump());
    return e;
}

void abi_MariaClimbLetGo(void)
{
    mx_exit(MariaClimbLetGo());
}

/*
 * @routine 01DE:666 MariaClimbAttack
 * @run 6D:AE66
 * WHAT IT DOES: Her attack from a ledge: what the attack button said (MariaAttack); if her attack's object started,
 * animation $20 and step 1 in the state she is in.
 * HOW: MariaAttack's Z; the JMP into MariaStandAttack past its state's store.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaAttack, or MariaStandAttack.
 */
MExit MariaClimbAttack(uint8_t x, uint8_t y)
{
    MExit e;

    jsr_push(0xAE69);
    e = MariaAttack(x, y);
    jsr_pop();
    if (!mx_zero(e)) {
        return e;
    }
    mx_regs(&e, r_lda(regs_kept(), ANIMATION_ATTACK));
    mx_then(&e, stand_attack_animation(ANIMATION_ATTACK));
    return e;
}

void abi_MariaClimbAttack(void)
{
    mx_exit(MariaClimbAttack(cpu.x, cpu.y));
}

/* The hold's checks after its move and presses, once her animation has stopped ($AE16 on, $AF5A on). */
static bool hold_stopped(MExit *e)
{
    uint8_t timer = RD8(OBJ->hurt_timer[OBJECT_PLAYER]);

    mx_regs(e, r_cmp(r_lda(regs_kept(), timer), timer, HOLD_STOPPED));
    return timer == HOLD_STOPPED;
}

/* The hold's first checks ($AE08 to $AE13, $AF4C to $AF57): the move, the presses, the jump or the ladder. */
static bool hold_checks(MExit *e, bool behind)
{
    JumpOrClimb jump;

    jsr_push(behind ? 0xAF4F : 0xAE0B);
    mx_then(e, MariaClimbMove());
    jsr_pop();
    jsr_push(behind ? 0xAF52 : 0xAE0E);
    mx_regs(e, MariaNotePressesB());
    jsr_pop();
    jsr_push(behind ? 0xAF55 : 0xAE11);
    jump = MariaJumpOrClimb();
    jsr_pop();
    mx_jump_or_climb(e, jump);
    if (mx_carry(*e)) {
        mx_then(e, MariaClimbLetGo());
        return true;
    }
    return false;
}

/*
 * @routine 01DE:608 MariaClimbLowFrame
 * @run 6D:AE08
 * WHAT IT DOES: Step 0 of the hold on the ledge ahead (states 5 and 6): the move, the presses, letting go on the jump;
 * once her animation has stopped: the pad override of the hold set, frame 5, and a look ahead (MariaClimbLowLooks):
 * with nothing there, she goes up (step 4, animation 0, the walking speed, OBJ_ANIMATION $15); else the item crash,
 * the attack, or the pad.
 * HOW: MariaClimbMove, MariaNotePressesB, MariaJumpOrClimb and MariaClimbLetGo; MariaReadAtOffset; MariaTryItemCrash,
 * MariaItemOrAttack, MariaClimbPad by their tail JMPs; MariaSetAnimationZero, MariaSpeedWalk.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the routine it ended in; going up, A $15, N and Z from it, with Y and C as
 * MariaSpeedWalk left them.
 */
MExit MariaClimbLowFrame(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value, index, look;
    ItemCrash crash;
    MExit use;
    OffsetRead read;

    if (hold_checks(&e, false)) {
        return e;
    }
    if (!hold_stopped(&e)) {
        return e;
    }
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    index = (uint8_t)(value - STATE_CLIMB_FIRST);
    WR8(PVARS->pad_overrides[index], 1);
    WR8(OBJ->frame[OBJECT_PLAYER], FRAME_HOLD);
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    {
        Sum less = sub_bytes(value, STATE_CLIMB_FIRST, true);

        index = less.value;
        look = TBL8(MariaClimbLowLooks, index);
        mx_regs(&e, r_v(r_c(r_ldy(r_ldx(r_a_only(regs_kept(), index), index), look), less.carry),
                        less.overflow));
    }
    /* The read may hand the tile to the stage's code, entered with X and Y as they stand. */
    cpu.x = index;
    cpu.y = look;
    jsr_push(0xAE3C);
    read = MariaReadAtOffset(look);
    jsr_pop();
    mx_offset_read(&e, read);
    value = (uint8_t)(mx_a_from(index, look, e) & LEDGE_BIT);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value == 0) {
        WR8(OBJ->substate[OBJECT_PLAYER], STEP_UP);
        jsr_push(0xAE5B);
        mx_then(&e, MariaSetAnimationZero());
        jsr_pop();
        jsr_push(0xAE5E);
        mx_regs(&e, MariaSpeedWalk());
        jsr_pop();
        WR8(OBJ->animation[OBJECT_PLAYER], ANIMATION_HOLD_UP);
        mx_regs(&e, r_lda(regs_kept(), ANIMATION_HOLD_UP));
        return e;
    }
    jsr_push(0xAE43);
    crash = MariaTryItemCrash();
    jsr_pop();
    mx_crash(&e, crash);
    if (crash.how == CRASH_STARTED) {
        mx_then(&e, MariaCrashInAir(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    jsr_push(0xAE4B);
    use = MariaItemOrAttack();
    jsr_pop();
    mx_then(&e, use);
    if (mx_carry(e)) {
        mx_then(&e, MariaClimbAttack(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    mx_then(&e, MariaClimbPad());
    return e;
}

void abi_MariaClimbLowFrame(void)
{
    mx_exit(MariaClimbLowFrame(cpu.x, cpu.y));
}

/*
 * @routine 01DE:74C MariaClimbHighFrame
 * @run 6D:AF4C
 * WHAT IT DOES: Step 0 of the hold on the ledge behind her (states 7 and 8): the move, the presses, letting go on the
 * jump; once her animation has stopped: frame 5, the hold's pad override set; the item crash, the attack; then a look
 * (by state less 7, doubled, plus 4): with bit 2 there, the pad; else she stands.
 * HOW: as MariaClimbLowFrame, with PadOverrides from its third byte and the tail JMPs to MariaClimbPad and MariaStand.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the routine it ended in.
 */
MExit MariaClimbHighFrame(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value, index, look;
    ItemCrash crash;
    MExit use;
    OffsetRead read;
    Sum less;
    AnimThen stand;

    if (hold_checks(&e, true)) {
        return e;
    }
    if (!hold_stopped(&e)) {
        return e;
    }
    WR8(OBJ->frame[OBJECT_PLAYER], FRAME_HOLD);
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    less = sub_bytes(value, STATE_CLIMB_BEHIND, true);
    index = less.value;
    WR8(PVARS->pad_overrides[2 + index], 1);
    mx_regs(&e, r_v(r_c(r_lda(r_ldy(regs_kept(), index), 1), less.carry), less.overflow));
    jsr_push(0xAF76);
    crash = MariaTryItemCrash();
    jsr_pop();
    mx_crash(&e, crash);
    if (crash.how == CRASH_STARTED) {
        mx_then(&e, MariaCrashInAir(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    jsr_push(0xAF7E);
    use = MariaItemOrAttack();
    jsr_pop();
    mx_then(&e, use);
    if (mx_carry(e)) {
        mx_then(&e, MariaClimbAttack(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    less = sub_bytes(value, STATE_CLIMB_BEHIND, true);
    {
        Sum looked = add_bytes((uint8_t)(less.value << 1), BEHIND_LOOKS, false);

        look = looked.value;
        mx_regs(&e, r_v(r_c(r_ldy(r_a_only(regs_kept(), look), look), looked.carry), looked.overflow));
    }
    cpu.x = mx_x_from(x, y, e);
    cpu.y = look;
    jsr_push(0xAF91);
    read = MariaReadAtOffset(look);
    jsr_pop();
    mx_offset_read(&e, read);
    value = (uint8_t)(mx_a_from(x, look, e) & LEDGE_BIT);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value != 0) {
        mx_then(&e, MariaClimbPad());
        return e;
    }
    stand = MariaStand();
    mx_animation(&e, stand.animation);
    mx_regs(&e, stand.regs);
    return e;
}

void abi_MariaClimbHighFrame(void)
{
    mx_exit(MariaClimbHighFrame(cpu.x, cpu.y));
}

/*
 * @routine 01DE:671 MariaClimbWait
 * @run 6D:AE71
 * WHAT IT DOES: A step of the hold that waits on her animation; when it has stopped, she is back in her ledge's hold.
 * HOW: MariaAnimationWait, which ends the step while the animation runs; the tail JMP to MariaClimbHold.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaAnimationWait, or MariaClimbHold.
 */
MExit MariaClimbWait(void)
{
    MExit e = mx_none();
    AnimationWaitEnd wait;

    jsr_push(0xAE74);
    wait = MariaAnimationWait();
    mx_animation(&e, wait.step);
    if (wait.dropped) {
        return e;
    }
    jsr_pop();
    mx_regs(&e, MariaClimbHold());
    return e;
}

void abi_MariaClimbWait(void)
{
    mx_exit(MariaClimbWait());
}

/* The facing for going up from the ledge, no fractions, and the tail JMP to MariaStopX ($AEC2). */
static void across_face(MExit *e, uint8_t index)
{
    uint8_t facing = TBL8(MariaClimbFacings, index);

    WR8(OBJ->facing_left[OBJECT_PLAYER], facing);
    WR8(OBJ->x_frac[OBJECT_PLAYER], 0);
    WR8(OBJ->y_frac[OBJECT_PLAYER], 0);
    mx_regs(e, r_lda(r_y_only(regs_kept(), index), facing));
    MariaStopX();
}

/*
 * @routine 01DE:677 MariaClimbAcross
 * @run 6D:AE77
 * WHAT IT DOES: Step 2 of a hold, going across to the ledge's edge: the jump lets go into a jump; while PlayerClimbReach
 * is not zero she moves a pixel a frame toward the edge ($0100 or $FF00) with her animation; at the edge, on the
 * ledge ahead frame 6 for 6 frames and the next step, on the ledge behind animation $13 and step 0; she faces up the
 * ledge and stops.
 * HOW: MariaJumpOrClimb and the JMP to MariaJump; DEC or INC of the reach; MariaAnimationTick; MariaClimbFacings by
 * state less 5 or 7; the tail JMP to MariaStopX.
 * WAITS: no.
 * Entry: nothing. Exit: as the routine it ended in.
 */
MExit MariaClimbAcross(void)
{
    MExit e = mx_none();
    JumpOrClimb jump;
    uint8_t reach, state;
    Sum less;

    jsr_push(0xAE7A);
    jump = MariaJumpOrClimb();
    jsr_pop();
    mx_jump_or_climb(&e, jump);
    if (mx_carry(e)) {
        mx_then(&e, MariaJump());
        return e;
    }
    reach = RD8(PVARS->climb_reach);
    mx_regs(&e, r_lda(regs_kept(), reach));
    if (reach != 0) {
        bool back = (reach & 0x80) != 0;

        jsr_push(back ? 0xAE9A : 0xAE89);
        mx_animation(&e, MariaAnimationTick());
        jsr_pop();
        WR8(OBJ->vx_lo[OBJECT_PLAYER], 0);
        WR8(OBJ->vx_hi[OBJECT_PLAYER], back ? 0xFF : 0x01);
        mx_regs(&e, r_lda(regs_kept(), back ? 0xFF : 0x01));
        if (back) {
            mx_regs(&e, r_nz(regs_kept(), INC8(PVARS->climb_reach)));
        } else {
            mx_regs(&e, r_nz(regs_kept(), DEC8(PVARS->climb_reach)));
        }
        return e;
    }
    state = RD8(OBJ->state[OBJECT_PLAYER]);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), state), state, STATE_CLIMB_BEHIND));
    if (state < STATE_CLIMB_BEHIND) {
        WR8(OBJ->frame[OBJECT_PLAYER], FRAME_ACROSS);
        WR8(OBJ->hurt_timer[OBJECT_PLAYER], ACROSS_TIMER);
        INC8(OBJ->substate[OBJECT_PLAYER]);
        state = RD8(OBJ->state[OBJECT_PLAYER]);
        less = sub_bytes(state, STATE_CLIMB_FIRST, true);
    } else {
        WR8(OBJ->substate[OBJECT_PLAYER], 0);
        push8(state);
        mx_regs(&e, r_lda(regs_kept(), ANIMATION_GOING_UP));
        jsr_push(0xAEDB);
        mx_animation(&e, MariaSetAnimation(ANIMATION_GOING_UP));
        jsr_pop();
        state = pull8();
        less = sub_bytes(state, STATE_CLIMB_BEHIND, true);
    }
    mx_regs(&e, r_v(r_c(regs_kept(), less.carry), less.overflow));
    across_face(&e, less.value);
    return e;
}

void abi_MariaClimbAcross(void)
{
    mx_exit(MariaClimbAcross());
}

/*
 * @routine 01DE:6E2 MariaClimbTurn
 * @run 6D:AEE2
 * WHAT IT DOES: Step 3 of the hold on the ledge ahead: the jump lets go; when the timer runs out, with the hold's pad
 * (MariaClimbTurnPads) held she turns (animation $12, step 0), else she is back in the hold.
 * HOW: MariaJumpOrClimb and the JMP to MariaJump; DEC of OBJ_HURT_TIMER; the AND with zPadHeld; MariaSetAnimation or a
 * branch to MariaClimbHold.
 * WAITS: no.
 * Entry: nothing. Exit: as the routine it ended in; turned, as MariaSetAnimation.
 */
MExit MariaClimbTurn(void)
{
    MExit e = mx_none();
    JumpOrClimb jump;
    uint8_t timer, state, pad;
    Sum less;

    jsr_push(0xAEE5);
    jump = MariaJumpOrClimb();
    jsr_pop();
    mx_jump_or_climb(&e, jump);
    if (mx_carry(e)) {
        mx_then(&e, MariaJump());
        return e;
    }
    timer = DEC8(OBJ->hurt_timer[OBJECT_PLAYER]);
    mx_regs(&e, r_nz(regs_kept(), timer));
    if (timer != 0) {
        return e;
    }
    state = RD8(OBJ->state[OBJECT_PLAYER]);
    less = sub_bytes(state, STATE_CLIMB_FIRST, true);
    pad = RD8(ZP->pad_held);
    pad &= TBL8(MariaClimbTurnPads, less.value);
    mx_regs(&e, r_v(r_c(r_lda(r_y_only(regs_kept(), less.value), pad), less.carry), less.overflow));
    if (pad == 0) {
        mx_regs(&e, MariaClimbHold());
        return e;
    }
    mx_regs(&e, r_lda(regs_kept(), ANIMATION_TURNED));
    jsr_push(0xAF03);
    mx_animation(&e, MariaSetAnimation(ANIMATION_TURNED));
    jsr_pop();
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    return e;
}

void abi_MariaClimbTurn(void)
{
    mx_exit(MariaClimbTurn());
}

/*
 * @routine 01DE:731 MariaClimbUp
 * @run 6D:AF31
 * WHAT IT DOES: Step 4, going up onto the ledge: when her animation has stopped, she stands.
 * HOW: MariaAnimationRunning's Z; the tail JMP to MariaStand.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaAnimationRunning, or MariaStand.
 */
MExit MariaClimbUp(void)
{
    MExit e = mx_none();
    AnimationStep running;
    AnimThen stand;

    jsr_push(0xAF34);
    running = MariaAnimationRunning();
    jsr_pop();
    mx_animation(&e, running);
    if (!mx_zero(mx_one_animation(running))) {
        return e;
    }
    stand = MariaStand();
    mx_animation(&e, stand.animation);
    mx_regs(&e, stand.regs);
    return e;
}

void abi_MariaClimbUp(void)
{
    mx_exit(MariaClimbUp());
}

/* A hold's step by the word its table gave, as the JMP (table,X) reaches it. */
static MExit climb_step(uint16_t target, uint8_t x, uint8_t y)
{
    MExit none = mx_none();

    switch (target) {
    case CLIMB_STEP_HOLD:
        return MariaClimbLowFrame(x, y);
    case CLIMB_STEP_HOLD_BEHIND:
        return MariaClimbHighFrame(x, y);
    case CLIMB_STEP_WAIT:
        return MariaClimbWait();
    case CLIMB_STEP_ACROSS:
        return MariaClimbAcross();
    case CLIMB_STEP_TURN:
        return MariaClimbTurn();
    case CLIMB_STEP_UP:
        return MariaClimbUp();
    case CLIMB_STEP_CRASH:
        return MariaCrashFrame();
    default:
        none.elsewhere = true;
        return none;
    }
}

/* The state's dispatch: MariaStepIndex, then the JMP (table,X) through the steps' table at `table`. */
static MExit climb_state(bool behind, uint16_t ret, uint8_t y)
{
    MExit e = mx_none();
    uint8_t step, doubled, lo, hi;
    uint16_t target;
    MExit handler;

    jsr_push(ret);
    step = MariaStepIndex();
    jsr_pop();
    doubled = (uint8_t)(step << 1);
    mx_regs(&e, r_c(r_ldx(r_lda(regs_kept(), doubled), doubled), (step & 0x80) != 0));
    lo = behind ? TBL8(MariaClimbHighSteps, doubled) : TBL8(MariaClimbLowSteps, doubled);
    hi = behind ? TBL8(MariaClimbHighSteps, doubled + 1) : TBL8(MariaClimbLowSteps, doubled + 1);
    target = (uint16_t)(lo | (hi << 8));
    handler = climb_step(target, doubled, y);
    if (handler.elsewhere) {
        mx_exit(e);
        run_routine(target);
        e.elsewhere = true;
        return e;
    }
    mx_then(&e, handler);
    return e;
}

/*
 * @routine 01DE:51A MariaStateClimbLow
 * @run 6D:AD1A
 * WHAT IT DOES: States 5 and 6, the hold on the ledge ahead: the step's handler from MariaClimbLowSteps (hold, wait,
 * across, turn, up, the item crash).
 * HOW: MariaStepIndex, then the JMP (table,X).
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the step's handler, after A and X the step doubled and C its bit 7.
 */
MExit MariaStateClimbLow(uint8_t x, uint8_t y)
{
    (void)x;
    return climb_state(false, 0xAD1D, y);
}

void abi_MariaStateClimbLow(void)
{
    mx_exit(MariaStateClimbLow(cpu.x, cpu.y));
}

/*
 * @routine 01DE:73A MariaStateClimbHigh
 * @run 6D:AF3A
 * WHAT IT DOES: States 7 and 8, the hold on the ledge behind her: the step's handler from MariaClimbHighSteps (hold,
 * wait, across, wait, wait, the item crash).
 * HOW: MariaStepIndex, then the JMP (table,X).
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the step's handler, after A and X the step doubled and C its bit 7.
 */
MExit MariaStateClimbHigh(uint8_t x, uint8_t y)
{
    (void)x;
    return climb_state(true, 0xAF3D, y);
}

void abi_MariaStateClimbHigh(void)
{
    mx_exit(MariaStateClimbHigh(cpu.x, cpu.y));
}
