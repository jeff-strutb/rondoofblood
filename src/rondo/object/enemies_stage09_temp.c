/*
 * TEMPORARY: the stage 09A1 enemies and their helpers (disc sectors 093C and 09E9 to 09ED), by routine until
 * each enemy kind is identified. Bodies return what they ended on (ObjArm, enemy_objects.h); each routine's
 * exit shim builds A, Y and the flags from it, as in 441de03, and its abi_ wrapper applies them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

static Ended of_sum(Sum sum)
{
    return ended_of_sum(sum);
}

/* The steps of an animation list at `list`, with the list's address in zTemp10-11; the registers it left in `e`. */
static FrameStep step_list(uint8_t slot, uint16_t list, uint16_t ret, uint8_t *y, bool high_first)
{
    FrameStep step;

    if (high_first) {
        WR8(ZP->temp11, (uint8_t)(list >> 8));
        WR8(ZP->temp10, (uint8_t)list);
    } else {
        WR8(ZP->temp10, (uint8_t)list);
        WR8(ZP->temp11, (uint8_t)(list >> 8));
    }
    jsr_push(ret);
    step = StepObjectFrames(slot, y);
    jsr_pop();
    return step;
}


/* CPY and CPX: N, Z and C from the register less the operand; A kept. */
static Ended cpy(uint8_t reg, uint8_t operand)
{
    Ended e = ended_nz((uint8_t)(reg - operand));

    return ended_with_carry(e, reg >= operand);
}

/* ApplyGravity and ApplyGravityUnlimited's exit: A, N, Z, C and V as gravity_exit_shim leaves them. */
static Ended ended_of_gravity(GravityResult g)
{
    Ended e;

    if (g.gained) {
        e = ended_with_a(ended_nz(g.carried ? g.high : g.low), g.low);
        return ended_with_overflow(ended_with_carry(e, g.carried), g.overflow);
    }
    e = ended_with_a(ended_nz((uint8_t)(g.high - 7)), g.high);
    return ended_with_carry(e, true);
}

/* StartSoleScriptSlot's exit. */
static Ended ended_of_sole(SoleSlot r)
{
    uint8_t a = r.started ? r.script : r.busy_kind;

    return ended_with_carry(ended_with_y(ended_load(a), r.script), r.started);
}

/* The arms the routines below end on. */
enum { FC_SCROLLING, FC_UNMARKED, FC_FACED };
enum { WT_STRAIGHT, WT_TURNED };
enum { WS_PLAIN, WS_NEGATED };
enum { PN_WAITING, PN_PICKED };
enum { WP_FAR, WP_NEAR };
enum { PC_NO_FRAME, PC_NO_CUE, PC_CUE };
enum { SF_ENDED, SF_HELD, SF_NO_CUE, SF_CUE };
enum { QW_STEPPING, QW_TIMER_OK, QW_RAISED };
enum { ES_WAITING, ES_ENTERED };
enum { BL_ABOVE, BL_NOT_YET, BL_BURST };
enum { IW_HIGH, IW_OUT, IW_ONE };
enum { SS_STRAIGHT, SS_NEGATED };
enum { MS_NO_COUNT, MS_NO_PICK, MS_NO_SLOT, MS_SHOT };
enum { W2_WAITING, W2_LATER, W2_SOUNDED };
enum { RU_HIGH, RU_LOW, RU_ENTERED };
enum { W4_EARLY, W4_ENTERED };
enum { FH_LOAD, FH_FOLLOW };
enum { RC_HIGH, RC_LOW, RC_CLEARED };
enum { RS_LEAD, RS_STATE, RS_RAISED };
enum { CL_OTHER, CL_LAST };
enum { CF_FIRST, CF_NEXT };
enum { RE_OFF_EDGE, RE_WRONG_STATE, RE_ENTERED };
enum { SA_ADVANCED, SA_SWAYED };
enum { SY_SWAYED, SY_FLIPPED };
enum { CC_COUNTING, CC_ENTERED };
enum { CS_STEADY, CS_NEXT };
enum { WA_PICK_ONLY, WA_STATE1 };
enum { SC_ENDED, SC_NO_FRAME, SC_BITS, SC_NO_SLOT, SC_SHOT };
enum { AH_NO_STATE, AH_BLOCKED, AH_ADVANCED, AH_FLAGGED };
enum { PG_OUTSIDE, PG_PICKED };
enum { FL_FALLING, FL_LEFT };
enum { TM_NO_STATE, TM_STATE4, TM_NO_MODE, TM_TABLE };
enum { HC_FULL, HC_HEALED, HC_CAPPED };
enum { OP_RECORD, OP_STEP };
enum { ST_PLAIN, ST_STARTED };
enum { FS_SPINNING, FS_OTHER_SLOT, FS_ENDED };
enum { SP_ENDED, SP_MOVING };
enum { SK_STEP, SK_START };
enum { PW_NOT_TENTH, PW_LOW_STATE, PW_PICKED };
enum { SW_NO_ROW, SW_NO_HIT, SW_SWITCHED };

/*
 * @routine 09EA:55A FaceByCue
 * @run 70:AD5A
 * WHAT IT DOES: While the screen is not scrolling along X, and in a state the table at $AD70 marks, the object
 * faces the way the player is (bit 0 of SideOfPlayer into its facing).
 * HOW: Reads ScrollSpeedX and the table by state, then SideOfPlayer.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the last load or AND; Y the state when the table was read.
 */
ObjArm FaceByCue(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = rd8(ScrollSpeedX);
    r.arm = FC_SCROLLING;
    if (r.first != 0) {
        return r;
    }
    r.second = RD8(OBJ->state[slot]);
    r.third = rd8((uint16_t)(0xAD70 + r.second));
    r.arm = FC_UNMARKED;
    if (r.third == 0) {
        return r;
    }
    jsr_push(0xAD6A);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.fourth = (uint8_t)(r.side.sides & 0x01);
    WR8(OBJ->facing_left[slot], r.fourth);
    r.arm = FC_FACED;
    return r;
}

static Ended face_by_cue_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == FC_SCROLLING) {
        return e;
    }
    e = then(e, ended_with_y(ended_load(r.third), r.second));
    if (r.arm == FC_UNMARKED) {
        return e;
    }
    e = then(e, ended_of_side(r.side));
    return then(e, ended_load(r.fourth));
}

void abi_FaceByCue(void)
{
    abi_ended(face_by_cue_exit_shim(FaceByCue(cpu.x)));
}

/*
 * @routine 09EA:6CC WalkAndTurnAtEdges
 * @run 70:AECC
 * WHAT IT DOES: Moves the object along X; when its X plus the scroll reaches an edge (page 3 from $30, or any other
 * page below $48) it turns round: the X speed is reversed and the move made again.
 * HOW: ApplyVelocityX, the add of ScrollXLo and ScrollXHiScreen, then the compare; the turn is NegateVelocityX and a
 * tail jump into ApplyVelocityX.
 * Untraced: both arms that turn, and the page 3 branch (AEE9 and AEEF).
 * WAITS: no.
 * Entry: X object. Exit: no turn: A the low byte, N, Z, C from its compare; turn: as ApplyVelocityX.
 */
ObjArm WalkAndTurnAtEdges(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t low, high, scroll;
    bool turn;

    jsr_push(0xAECF);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    low = RD8(OBJ->x_lo[slot]);
    scroll = rd8(ScrollXLo);
    r.sum = add_bytes(low, scroll, false);
    WR8(ZP->temp00, r.sum.value);
    high = RD8(OBJ->x_hi[slot]);
    scroll = rd8(ScrollXHiScreen);
    r.sum2 = add_bytes(high, scroll, r.sum.carry);
    r.first = RD8(ZP->temp00);
    if (r.sum2.value == 3) {
        turn = r.first >= 0x30;
    } else {
        turn = r.first < 0x48;
    }
    r.arm = WT_STRAIGHT;
    if (!turn) {
        return r;
    }
    jsr_push(0xAEF2);
    NegateVelocityX(slot);
    jsr_pop();
    r.move2 = ApplyVelocityX(slot);
    r.arm = WT_TURNED;
    return r;
}

static Ended walk_and_turn_at_edges_exit_shim(ObjArm r)
{
    Ended e = ended_of_axis_move(r.move);

    e = then(e, of_sum(r.sum));
    e = then(e, of_sum(r.sum2));
    e = then(e, ended_compare(r.sum2.value, 3));
    if (r.sum2.value == 3) {
        e = then(e, ended_compare(r.first, 0x30));
    } else {
        e = then(e, ended_compare(r.first, 0x48));
    }
    if (r.arm == WT_STRAIGHT) {
        return e;
    }
    return ended_of_axis_move(r.move2);
}

void abi_WalkAndTurnAtEdges(void)
{
    abi_ended(walk_and_turn_at_edges_exit_shim(WalkAndTurnAtEdges(cpu.x)));
}

/*
 * @routine 09EA:6A2 WalkSideways
 * @run 70:AEA2
 * WHAT IT DOES: Steps the walk animation (list $AEC3), sets the X speed to $FFC0 (left), reverses it when the object's
 * direction byte OBJ_WORK_F is set, and moves with WalkAndTurnAtEdges.
 * HOW: StepObjectFrames, NegateVelocityX, WalkAndTurnAtEdges.
 * WAITS: no.
 * Entry: X object. Exit: as WalkAndTurnAtEdges.
 */
ObjArm WalkSideways(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    r.step = step_list(slot, 0xAEC3, 0xAEAD, &y, false);
    r.y = y;
    WR8(OBJ->vx_hi[slot], 0xFF);
    WR8(OBJ->vx_lo[slot], 0xC0);
    r.first = RD8(OBJ->angle_hi[slot]);
    r.arm = WS_PLAIN;
    if (r.first != 0) {
        jsr_push(0xAEBF);
        r.sum = NegateVelocityX(slot);
        jsr_pop();
        r.arm = WS_NEGATED;
    }
    jsr_push(0xAEC2);
    r.sub = obj_sub(WalkAndTurnAtEdges(slot));
    jsr_pop();
    return r;
}

static Ended walk_sideways_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    e = then(e, ended_load(r.first));
    if (r.arm == WS_NEGATED) {
        e = then(e, of_sum(r.sum));
    }
    return then(e, walk_and_turn_at_edges_exit_shim(obj_arm(r.sub)));
}

void abi_WalkSideways(void)
{
    abi_ended(walk_sideways_exit_shim(WalkSideways(cpu.x)));
}

/*
 * @routine 09EA:6F5 PickNextAction
 * @run 70:AEF5
 * WHAT IT DOES: While its wait OBJ_WORK_E runs, counts it down. At zero it picks a random state (from the pair of bytes
 * at $AF28), plays the sound the pair's second byte names, resets its animation and picks a new wait from the table
 * at $AF24 by a random 0 to 3.
 * HOW: NextRandom twice, SoundCommand.
 * WAITS: no.
 * Entry: X object, Y kept unless the picks set it. Exit: waiting: A the old wait, N and Z from the new; else A the
 * new wait, Y its index, C and V from the second random.
 */
ObjArm PickNextAction(uint8_t slot)
{
    ObjArm r = { 0 };
    RandomResult random;
    uint8_t picked, index, byte;

    r.first = RD8(OBJ->work_e[slot]);
    r.arm = PN_WAITING;
    if (r.first != 0) {
        r.second = DEC8(OBJ->work_e[slot]);
        return r;
    }
    jsr_push(0xAF01);
    random = NextRandom();
    jsr_pop();
    picked = (uint8_t)(random.value & 0x01);
    index = (uint8_t)(picked << 1);
    byte = rd8((uint16_t)(0xAF28 + index));
    WR8(OBJ->state[slot], byte);
    byte = rd8((uint16_t)(0xAF29 + index));
    jsr_push(0xAF11);
    SoundCommand(byte, slot, index);
    jsr_pop();
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    jsr_push(0xAF1A);
    r.random = NextRandom();
    jsr_pop();
    r.y = (uint8_t)(r.random.value & 0x03);
    r.third = rd8((uint16_t)(0xAF24 + r.y));
    WR8(OBJ->work_e[slot], r.third);
    r.arm = PN_PICKED;
    return r;
}

static Ended pick_next_action_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == PN_WAITING) {
        return then(e, ended_nz(r.second));
    }
    return then(ended_of_random(r.random), ended_with_y(ended_load(r.third), r.y));
}

void abi_PickNextAction(void)
{
    abi_ended(pick_next_action_exit_shim(PickNextAction(cpu.x)));
}

/*
 * @routine 09EA:689 WalkTowardPlayer
 * @run 70:AE89
 * WHAT IT DOES: PickNextAction and WalkSideways, then the facing byte becomes the walk direction and, if the player is
 * within $40 along X, the object enters state 2.
 * HOW: DistanceXToPlayer.
 * Untraced: the state 2 arm (AE9C).
 * WAITS: no.
 * Entry: X object. Exit: the last compare, A the distance.
 */
ObjArm WalkTowardPlayer(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xAE8C);
    r.sub = obj_sub(PickNextAction(slot));
    jsr_pop();
    jsr_push(0xAE8F);
    r.sub2 = obj_sub(WalkSideways(slot));
    jsr_pop();
    r.first = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->angle_hi[slot], r.first);
    jsr_push(0xAE98);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.arm = WP_FAR;
    if (r.distance.distance >= 0x40) {
        return r;
    }
    WR8(OBJ->state[slot], 2);
    r.arm = WP_NEAR;
    return r;
}

static Ended walk_toward_player_exit_shim(ObjArm r)
{
    Ended e = pick_next_action_exit_shim(obj_arm(r.sub));

    e = then(e, walk_sideways_exit_shim(obj_arm(r.sub2)));
    e = then(e, ended_load(r.first));
    e = then(e, ended_of_distance(r.distance));
    e = then(e, ended_compare(r.distance.distance, 0x40));
    if (r.arm == WP_FAR) {
        return e;
    }
    return then(e, ended_load(2));
}

void abi_WalkTowardPlayer(void)
{
    abi_ended(walk_toward_player_exit_shim(WalkTowardPlayer(cpu.x)));
}

/*
 * @routine 09EA:7D0 StartCueScript
 * @run 70:AFD0
 * WHAT IT DOES: Starts the script slot 5 with the script $80 plus Y (the last part of PlayCueOnFrame).
 * HOW: X is pushed and pulled around StartScriptSlotKind4.
 * WAITS: no.
 * Entry: X object, Y the cue's index. Exit: A and Y the script, N and Z from X, C and V from the add of $80.
 */
ObjArm StartCueScript(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };

    push8(slot);
    r.sum = add_bytes(y, 0x80, false);
    jsr_push(0xAFDA);
    r.first = StartScriptSlotKind4(r.sum.value, 5);
    jsr_pop();
    r.second = pull8();
    return r;
}

static Ended start_cue_script_exit_shim(ObjArm r)
{
    Ended e = of_sum(r.sum);

    e = then(e, ended_with_y(ended_load(r.first), r.first));
    return then(e, ended_nz(r.second));
}

void abi_StartCueScript(void)
{
    abi_ended(start_cue_script_exit_shim(StartCueScript(cpu.x, cpu.y)));
}

/*
 * @routine 09EA:792 PlayCueOnFrame
 * @run 70:AF92
 * @stop AFB9
 * WHAT IT DOES: An object showing frame 8 or $0A starts a script when its animation timer equals one of the cue
 * counts in a table at $AFB9 (frame 8: the first eight; frame $0A: the next fourteen); the script is $80 plus the
 * cue's index.
 * HOW: A scan of the table with the timer, ending in StartCueScript.
 * Untraced: the table (AFB9 to AFCF) is data the traced run only read.
 * WAITS: no.
 * Entry: X object. Exit: no cue: A and flags of the last compare; cue: as StartCueScript.
 */
ObjArm PlayCueOnFrame(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y, limit, timer, cue, n = 0;

    r.first = RD8(OBJ->frame[slot]);
    if (r.first == 8) {
        y = 0;
        limit = 8;
        WR8(ZP->temp00, limit);
    } else {
        r.arm = PC_NO_FRAME;
        if (r.first != 0x0A) {
            return r;
        }
        y = 8;
        limit = 0x16;
        WR8(ZP->temp00, limit);
    }
    do {
        timer = RD8(OBJ->anim_timer[slot]);
        cue = rd8((uint16_t)(0xAFB9 + y));
        r.bytes[2 * n] = timer;
        r.bytes[2 * n + 1] = cue;
        n++;
        r.fourth = n;
        if (timer == cue) {
            r.arm = PC_CUE;
            r.sub = obj_sub(StartCueScript(slot, y));
            return r;
        }
        y++;
        limit = RD8(ZP->temp00);
        r.third = limit;
    } while (y < limit);
    r.arm = PC_NO_CUE;
    return r;
}

/* The scan of the table: each pass's compare, then INY and CPY; the cue's pass ends in StartCueScript. */
static Ended play_cue_on_frame_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 8);
    uint8_t y, i;

    if (r.first == 8) {
        y = 0;
        e = then(e, ended_with_y(ended_load(8), y));
    } else {
        e = then(e, ended_compare(r.first, 0x0A));
        if (r.arm == PC_NO_FRAME) {
            return e;
        }
        y = 8;
        e = then(e, ended_with_y(ended_load(0x16), y));
    }
    for (i = 0; i < r.fourth; i++) {
        e = then(e, ended_compare(r.bytes[2 * i], r.bytes[2 * i + 1]));
        if (r.arm == PC_CUE && i + 1 == r.fourth) {
            return then(e, start_cue_script_exit_shim(obj_arm(r.sub)));
        }
        y++;
        e = then(e, ended_with_y(ended_nz(y), y));
        e = then(e, cpy(y, r.third));
    }
    return e;
}

void abi_PlayCueOnFrame(void)
{
    abi_ended(play_cue_on_frame_exit_shim(PlayCueOnFrame(cpu.x)));
}

/*
 * @routine 09EA:7DC StepFlyingCue
 * @run 70:AFDC
 * WHAT IT DOES: Steps the animation list at $AFFE; when it ends the object enters state 2; on a new frame, at animation
 * step $1A, sound $1A plays.
 * HOW: StepObjectFrames, SoundCommand.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
 */
ObjArm StepFlyingCue(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };

    r.step = step_list(slot, 0xAFFE, 0xAFE7, &y, true);
    r.y = y;
    if (r.step == FRAME_LIST_ENDED) {
        WR8(OBJ->state[slot], 2);
        r.arm = SF_ENDED;
        return r;
    }
    r.arm = SF_HELD;
    if (r.step != FRAME_CHANGED) {
        return r;
    }
    r.first = RD8(OBJ->anim_step[slot]);
    r.arm = SF_NO_CUE;
    if (r.first != 0x1A) {
        return r;
    }
    r.arm = SF_CUE;
    r.sounded = true;
    r.command = 0x1A;
    jsr_push(0xAFF7);
    r.call = SoundCommand(0x1A, slot, y);
    jsr_pop();
    return r;
}

static SoundEnd step_flying_cue_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = frame_step_exit_shim(r);

    s.end = e;
    if (r.arm == SF_ENDED) {
        s.end = then(e, ended_load(2));
        return s;
    }
    if (r.arm == SF_HELD) {
        return s;
    }
    s.end = then(e, ended_compare(r.first, 0x1A));
    if (r.arm == SF_NO_CUE) {
        return s;
    }
    s.before = then(s.end, ended_load(0x1A));
    s.end = ended_kept();
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    return s;
}

void abi_StepFlyingCue(void)
{
    abi_sound_end(step_flying_cue_exit_shim(StepFlyingCue(cpu.x, cpu.y)));
}

/*
 * @routine 09EA:664 StartTwinSpawner
 * @run 70:AE64
 * WHAT IT DOES: Loads the object's record, clears OBJ_WORK_H, sets its wait to $60 and sets bit 2 of zField8E; then
 * clears objects $11 and $12 and starts a kind $22 and a kind $27 at this object's place in them.
 * HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace; the last is a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte copied, N and Z from it, Y $12, C from the record.
 */
ObjArm StartTwinSpawner(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t nz, last, flags;
    bool carry;

    jsr_push(0xAE67);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    r.flag = carry;
    WR8(OBJ->work_h[slot], 0);
    WR8(OBJ->work_e[slot], 0x60);
    flags = RD8(ZP->field8_e);
    flags = (uint8_t)(flags | 0x04);
    WR8(ZP->field8_e, flags);
    jsr_push(0xAE7A);
    ClearObjectInY(0x11);
    jsr_pop();
    jsr_push(0xAE7F);
    StartObjectAtPlace(0x22, slot, 0x11);
    jsr_pop();
    jsr_push(0xAE84);
    ClearObjectInY(0x12);
    jsr_pop();
    r.first = StartObjectAtPlace(0x27, slot, 0x12);
    return r;
}

static Ended start_twin_spawner_exit_shim(ObjArm r)
{
    return ended_with_carry(ended_with_y(ended_load(r.first), 0x12), r.flag);
}

void abi_StartTwinSpawner(void)
{
    abi_ended(start_twin_spawner_exit_shim(StartTwinSpawner(cpu.x)));
}

/*
 * @routine 09EB:03B StepThenQuarterWait
 * @run 70:B03B
 * WHAT IT DOES: Steps the animation list at $B060; when it ends the object's wait OBJ_WORK_E is quartered, it enters state 2
 * and its timer is raised to at least 3.
 * HOW: StepObjectFrames, two LSRs of memory, a compare.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: the last compare or load.
 */
ObjArm StepThenQuarterWait(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    r.step = step_list(slot, 0xB060, 0xB046, &y, true);
    r.y = y;
    r.arm = QW_STEPPING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    r.first = LSR8_VALUE(OBJ->work_e[slot], r.flag);
    r.second = LSR8_VALUE(OBJ->work_e[slot], r.other);
    WR8(OBJ->state[slot], 2);
    r.third = RD8(OBJ->invuln_timer[slot]);
    r.arm = QW_TIMER_OK;
    if (r.third >= 3) {
        return r;
    }
    WR8(OBJ->invuln_timer[slot], 3);
    r.arm = QW_RAISED;
    return r;
}

static Ended step_then_quarter_wait_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == QW_STEPPING) {
        return e;
    }
    e = then(e, ended_with_carry(ended_nz(r.first), r.flag));
    e = then(e, ended_with_carry(ended_nz(r.second), r.other));
    e = then(e, ended_load(2));
    e = then(e, ended_compare(r.third, 3));
    if (r.arm == QW_TIMER_OK) {
        return e;
    }
    return then(e, ended_load(3));
}

void abi_StepThenQuarterWait(void)
{
    abi_ended(step_then_quarter_wait_exit_shim(StepThenQuarterWait(cpu.x)));
}

/*
 * @routine 09EB:067 StartShotRing
 * @run 70:B067
 * WHAT IT DOES: Clears objects $11 and $12, starts ten objects of kind $28 at this object's place in slots $18 to $27, each
 * with this object's facing; then resets the animation, puts the object off the left of the screen (X from $C0 less the scroll,
 * Y $FFC0), stops it and enters its next state.
 * HOW: ClearObjectInY and StartObjectAtPlace in a loop.
 * WAITS: no.
 * Entry: X object. Exit: A $C0, Y $28, N and Z from the new state, C and V from the subtract of the scroll.
 */
ObjArm StartShotRing(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t to, facing, scroll;

    jsr_push(0xB06C);
    ClearObjectInY(0x11);
    jsr_pop();
    jsr_push(0xB070);
    ClearObjectInY(0x12);
    jsr_pop();
    for (to = 0x18; to < 0x28; to++) {
        jsr_push(0xB075);
        ClearObjectInY(to);
        jsr_pop();
        jsr_push(0xB07A);
        StartObjectAtPlace(0x28, slot, to);
        jsr_pop();
        facing = RD8(OBJ->facing_left[slot]);
        WR8(OBJ->facing_left[to], facing);
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->x_hi[slot], 0);
    scroll = rd8(ScrollXLo);
    r.sum = sub_bytes(0xC0, scroll, true);
    WR8(OBJ->x_lo[slot], r.sum.value);
    WR8(OBJ->y_hi[slot], 0xFF);
    WR8(OBJ->y_lo[slot], 0xC0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->frame[slot], 0);
    r.first = INC8(OBJ->state[slot]);
    return r;
}

static Ended start_shot_ring_exit_shim(ObjArm r)
{
    return then(of_sum(r.sum), ended_with_y(ended_with_a(ended_nz(r.first), 0xC0), 0x28));
}

void abi_StartShotRing(void)
{
    abi_ended(start_shot_ring_exit_shim(StartShotRing(cpu.x)));
}

/*
 * @routine 09EB:0AE EnterStateWhenSlotClear
 * @run 70:B0AE
 * WHAT IT DOES: The object enters its next state once object $26's kind is 0 (the one it waits for is gone).
 * HOW: A read of that kind and an INC.
 * WAITS: no.
 * Entry: X object. Exit: A the kind read, N and Z from it or from the new state.
 */
ObjArm EnterStateWhenSlotClear(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->kind[0x26]);
    r.arm = ES_WAITING;
    if (r.first != 0) {
        return r;
    }
    r.second = INC8(OBJ->state[slot]);
    r.arm = ES_ENTERED;
    return r;
}

static Ended enter_state_when_slot_clear_exit_shim(ObjArm r)
{
    if (r.arm == ES_WAITING) {
        return ended_load(r.first);
    }
    return then(ended_load(r.first), ended_nz(r.second));
}

void abi_EnterStateWhenSlotClear(void)
{
    abi_ended(enter_state_when_slot_clear_exit_shim(EnterStateWhenSlotClear(cpu.x)));
}

/*
 * @routine 09EB:0B7 BurstOnLanding
 * @run 70:B0B7
 * WHAT IT DOES: A thing that falls (list $B101, unlimited gravity) and bursts when it reaches Y $0040 on the first page: sound
 * $A1, script slot 3 started, kind $29 objects started in slots $12 to $19, a sideways speed of $0140 and the next state.
 * HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocityY, SoundCommand, StartSoleScriptSlot, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
 */
ObjArm BurstOnLanding(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t to;

    r.step = step_list(slot, 0xB101, 0xB0C2, &y, true);
    r.y = y;
    jsr_push(0xB0C5);
    r.gravity = ApplyGravityUnlimited(slot);
    jsr_pop();
    jsr_push(0xB0C8);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.first = RD8(OBJ->y_hi[slot]);
    r.arm = BL_ABOVE;
    if (r.first != 0) {
        return r;
    }
    r.second = RD8(OBJ->y_lo[slot]);
    r.arm = BL_NOT_YET;
    if (r.second < 0x40) {
        return r;
    }
    r.arm = BL_BURST;
    r.sounded = true;
    r.command = 0xA1;
    jsr_push(0xB0D9);
    r.call = SoundCommand(0xA1, slot, y);
    jsr_pop();
    push8(slot);
    jsr_push(0xB0E0);
    r.sole = StartSoleScriptSlot(0, 3);
    jsr_pop();
    slot = pull8();
    r.third = slot;
    for (to = 0x12; to < 0x1A; to++) {
        jsr_push(0xB0E8);
        r.bytes[to - 0x12] = StartObjectAtPlace(0x29, slot, to);
        jsr_pop();
    }
    WR8(OBJ->vx_hi[slot], 1);
    WR8(OBJ->vx_lo[slot], 0x40);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.fourth = INC8(OBJ->state[slot]);
    return r;
}

/* The sound call's leak sits before the later instructions: the wrapper shows it first, then the rest. */
static SoundEnd burst_on_landing_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = frame_step_exit_shim(r);
    uint8_t to;

    e = then(e, ended_of_gravity(r.gravity));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_load(r.first));
    s.end = e;
    if (r.arm == BL_ABOVE) {
        return s;
    }
    e = then(e, ended_compare(r.second, 0x40));
    s.end = e;
    if (r.arm == BL_NOT_YET) {
        return s;
    }
    s.before = then(e, ended_load(0xA1));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    e = then(ended_load(0), ended_of_sole(r.sole));
    e = then(e, ended_nz(r.third));
    for (to = 0x12; to < 0x1A; to++) {
        e = then(e, ended_with_y(ended_load(r.bytes[to - 0x12]), to));
        e = then(e, ended_with_y(cpy((uint8_t)(to + 1), 0x1A), (uint8_t)(to + 1)));
    }
    s.end = then(e, ended_with_a(ended_nz(r.fourth), 0x40));
    return s;
}

void abi_BurstOnLanding(void)
{
    abi_sound_end(burst_on_landing_exit_shim(BurstOnLanding(cpu.x, cpu.y)));
}

/*
 * @routine 09EB:16F InWindowRange
 * @run 70:B16F
 * WHAT IT DOES: Says whether the object is inside the page range the stage keeps its things alive in: the object's X plus the scroll,
 * high byte 3 or more with the low byte $C0 or more, or high byte 1 with the low byte under $C0.
 * HOW: The add of ScrollXLo and ScrollXHiScreen and two compares.
 * WAITS: no.
 * Entry: X object. Exit: C set when inside; A the last load, N and Z from the last compare, V from the add.
 */
ObjArm InWindowRange(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t low, high, scroll;

    low = RD8(OBJ->x_lo[slot]);
    scroll = rd8(ScrollXLo);
    r.sum = add_bytes(low, scroll, false);
    WR8(ZP->temp00, r.sum.value);
    high = RD8(OBJ->x_hi[slot]);
    scroll = rd8(ScrollXHiScreen);
    r.sum2 = add_bytes(high, scroll, r.sum.carry);
    if (r.sum2.value >= 3) {
        r.first = RD8(ZP->temp00);
        r.flag = r.first >= 0xC0;
        r.arm = IW_HIGH;
        return r;
    }
    if (r.sum2.value != 1) {
        r.flag = false;
        r.arm = IW_OUT;
        return r;
    }
    r.first = RD8(ZP->temp00);
    r.flag = r.first < 0xC0;
    r.arm = IW_ONE;
    return r;
}

/* Whether the object is in view, and what the routine left. */
static ViewTest in_window_range_exit_shim(ObjArm r)
{
    ViewTest v = { r.flag, { 0 } };
    Ended e = of_sum(r.sum);

    e = then(e, of_sum(r.sum2));
    e = then(e, ended_compare(r.sum2.value, 3));
    if (r.arm == IW_HIGH) {
        e = then(e, ended_compare(r.first, 0xC0));
        v.end = then(e, ended_with_carry(ended_kept(), r.flag));
        return v;
    }
    e = then(e, ended_compare(r.sum2.value, 1));
    if (r.arm == IW_OUT) {
        v.end = then(e, ended_with_carry(ended_kept(), false));
        return v;
    }
    e = then(e, ended_compare(r.first, 0xC0));
    v.end = then(e, ended_with_carry(ended_kept(), r.flag));
    return v;
}

void abi_InWindowRange(void)
{
    abi_ended(in_window_range_exit_shim(InWindowRange(cpu.x)).end);
}

/* The start two splitters share: speeds from a random pick, Y low byte given, then the side. */
static ObjArm splitter_start(uint8_t slot, uint8_t carry_in, uint8_t y_low, uint16_t ret_sum, uint16_t ret_side, uint16_t ret_negate)
{
    ObjArm r = { 0 };
    uint8_t picked, byte;

    jsr_push(ret_sum);
    r.sum = PlayerDistanceSum(slot, carry_in != 0);
    jsr_pop();
    picked = (uint8_t)(r.sum.value & 0x03);
    r.first = (uint8_t)(picked << 1);
    byte = rd8((uint16_t)(0xB1C0 + r.first));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xB1BF + r.first));
    WR8(OBJ->vx_lo[slot], byte);
    WR8(OBJ->y_lo[slot], y_low);
    r.second = y_low;
    WR8(OBJ->work_i[slot], 0);
    r.third = INC8(OBJ->substate[slot]);
    jsr_push(ret_side);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.fourth = (uint8_t)(r.side.sides & 0x01);
    r.arm = SS_STRAIGHT;
    if (r.fourth == 0) {
        return r;
    }
    jsr_push(ret_negate);
    r.sum2 = NegateVelocityX(slot);
    jsr_pop();
    r.arm = SS_NEGATED;
    return r;
}

static Ended splitter_start_exit_shim(ObjArm r)
{
    Ended e = of_sum(r.sum);

    e = then(e, ended_with_y(ended_with_carry(ended_load(r.first), false), r.first));
    e = then(e, ended_with_a(ended_nz(r.third), r.second));
    e = then(e, ended_of_side(r.side));
    e = then(e, ended_load(r.fourth));
    if (r.arm == SS_STRAIGHT) {
        return e;
    }
    return then(e, of_sum(r.sum2));
}

/*
 * @routine 09EB:196 StartSplitterHigh
 * @run 70:B196
 * WHAT IT DOES: Starts a falling splitter at Y low $48: its X speed is picked from a table at $B1BF by two bits of the
 * player's distance sum, OBJ_WORK_I is cleared, OBJ_SUBSTATE moves on, and when the object is on the player's left its X speed is
 * reversed.
 * HOW: PlayerDistanceSum, SideOfPlayer, NegateVelocityX.
 * Untraced: the reversal (B1BB).
 * WAITS: no.
 * Entry: X object, C passed to PlayerDistanceSum. Exit: the last load, AND or subtract.
 */
ObjArm StartSplitterHigh(uint8_t slot, bool carry_in)
{
    return splitter_start(slot, carry_in, 0x48, 0xB199, 0xB1B7, 0xB1BE);
}

void abi_StartSplitterHigh(void)
{
    abi_ended(splitter_start_exit_shim(StartSplitterHigh(cpu.x, (cpu.p & P_C) != 0)));
}

/*
 * @routine 09EB:1E8 StartSplitterLow
 * @run 70:B1E8
 * WHAT IT DOES: StartSplitterHigh with the Y low byte $58.
 * HOW: As StartSplitterHigh.
 * Untraced: the reversal (B20D).
 * WAITS: no.
 * Entry and exit: as StartSplitterHigh.
 */
ObjArm StartSplitterLow(uint8_t slot, bool carry_in)
{
    return splitter_start(slot, carry_in, 0x58, 0xB1EB, 0xB209, 0xB210);
}

void abi_StartSplitterLow(void)
{
    abi_ended(splitter_start_exit_shim(StartSplitterLow(cpu.x, (cpu.p & P_C) != 0)));
}

/*
 * @routine 09EB:21A MaybeShootAhead
 * @run 70:B21A
 * WHAT IT DOES: Every 32nd count of the object's OBJ_WORK_G, with a chance of three in four, starts a kind $2A object at its place in
 * the first free slot from $30 and gives it the object's facing.
 * HOW: NextRandom, FindFreeSlotFrom30, StartObjectAtPlace.
 * Untraced: one byte.
 * WAITS: no.
 * Entry: X object. Exit: the last load, AND or INC.
 */
ObjArm MaybeShootAhead(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t found_slot;
    bool found;

    r.first = RD8(OBJ->angle_lo[slot]);
    r.first = (uint8_t)(r.first & 0x1F);
    r.arm = MS_NO_COUNT;
    if (r.first != 0) {
        return r;
    }
    jsr_push(0xB224);
    r.random = NextRandom();
    jsr_pop();
    r.second = (uint8_t)(r.random.value & 0x03);
    r.arm = MS_NO_PICK;
    if (r.second == 0) {
        return r;
    }
    jsr_push(0xB22B);
    found_slot = FindFreeSlotFrom30(&r.third, &found);
    jsr_pop();
    r.fourth = found_slot;
    r.flag = found;
    r.arm = MS_NO_SLOT;
    if (!found) {
        return r;
    }
    jsr_push(0xB232);
    r.fifth = StartObjectAtPlace(0x2A, slot, found_slot);
    jsr_pop();
    r.sixth = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[found_slot], r.sixth);
    r.arm = MS_SHOT;
    return r;
}

static Ended maybe_shoot_ahead_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == MS_NO_COUNT) {
        return e;
    }
    e = then(e, ended_of_random(r.random));
    e = then(e, ended_load(r.second));
    if (r.arm == MS_NO_PICK) {
        return e;
    }
    e = then(e, ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), r.third), r.fourth), r.flag));
    if (r.arm == MS_NO_SLOT) {
        return e;
    }
    e = then(e, ended_load(r.fifth));
    return then(e, ended_load(r.sixth));
}

void abi_MaybeShootAhead(void)
{
    abi_ended(maybe_shoot_ahead_exit_shim(MaybeShootAhead(cpu.x)));
}

/*
 * @routine 09EB:51E PickWaterLevel
 * @run 70:B51E
 * WHAT IT DOES: Sets the water the stage wants (RasterWaterWanted and LevelYHi) to a two-byte pair from a table at $B531 picked by
 * two bits of the wave counter.
 * HOW: Reads the wave counter and the table (bank 70).
 * WAITS: no.
 * Entry: nothing. Exit: A the second byte, Y the index, N and Z from A, C 0.
 */
ObjArm PickWaterLevel(void)
{
    ObjArm r = { 0 };
    uint8_t wave, byte;

    wave = RD8(ZP->wave_counter);
    wave = (uint8_t)(wave & 0x03);
    r.y = (uint8_t)(wave << 1);
    byte = rd8((uint16_t)(0xB531 + r.y));
    wr8(RasterWaterWanted, byte);
    r.first = rd8((uint16_t)(0xB532 + r.y));
    wr8(LevelYHi, r.first);
    return r;
}

static Ended pick_water_level_exit_shim(ObjArm r)
{
    return ended_with_carry(ended_with_y(ended_load(r.first), r.y), false);
}

void abi_PickWaterLevel(void)
{
    abi_ended(pick_water_level_exit_shim(PickWaterLevel()));
}

/*
 * @routine 09EB:55F WaitForPage2
 * @run 70:B55F
 * WHAT IT DOES: When the level position's high byte is 2, the object's timer is set to $40 and it enters its next state; the
 * object in slot $10 also plays sound $A2.
 * HOW: A compare of ScrollXHiScreen, an INC, SoundCommand.
 * WAITS: no.
 * Entry: X object, Y pushed by the sound driver. Exit: see the wrapper.
 */
ObjArm WaitForPage2(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };

    r.first = rd8(ScrollXHiScreen);
    r.third = slot;
    r.arm = W2_WAITING;
    if (r.first != 2) {
        return r;
    }
    WR8(OBJ->life_timer[slot], 0x40);
    r.second = INC8(OBJ->state[slot]);
    r.arm = W2_LATER;
    if (slot != 0x10) {
        return r;
    }
    r.arm = W2_SOUNDED;
    r.sounded = true;
    r.command = 0xA2;
    jsr_push(0xB577);
    r.call = SoundCommand(0xA2, slot, y);
    jsr_pop();
    return r;
}

static SoundEnd wait_for_page2_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = ended_compare(r.first, 2);

    s.end = e;
    if (r.arm == W2_WAITING) {
        return s;
    }
    e = then(e, ended_load(0x40));
    e = then(e, ended_nz(r.second));
    e = then(e, cpy(r.third, 0x10));
    s.end = e;
    if (r.arm == W2_LATER) {
        return s;
    }
    s.before = then(e, ended_load(0xA2));
    s.end = ended_kept();
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    return s;
}

void abi_WaitForPage2(void)
{
    abi_sound_end(wait_for_page2_exit_shim(WaitForPage2(cpu.x, cpu.y)));
}

/*
 * @routine 09EB:5BB RiseUntilAboveGround
 * @run 70:B5BB
 * WHAT IT DOES: Moves the object, takes $0040 off its Y speed (it climbs), and when its Y position has passed from negative
 * to a low byte under $80 it enters its next state.
 * HOW: ApplyVelocity, a 16-bit subtract.
 * WAITS: no.
 * Entry: X object. Exit: the last load or INC.
 */
ObjArm RiseUntilAboveGround(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t low, high;

    jsr_push(0xB5BE);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    low = RD8(OBJ->vy_lo[slot]);
    r.sum = sub_bytes(low, 0x40, true);
    WR8(OBJ->vy_lo[slot], r.sum.value);
    high = RD8(OBJ->vy_hi[slot]);
    r.sum2 = sub_bytes(high, 0, r.sum.carry);
    WR8(OBJ->vy_hi[slot], r.sum2.value);
    r.first = RD8(OBJ->y_hi[slot]);
    r.arm = RU_HIGH;
    if ((r.first & 0x80) == 0) {
        return r;
    }
    r.second = RD8(OBJ->y_lo[slot]);
    r.arm = RU_LOW;
    if ((r.second & 0x80) != 0) {
        return r;
    }
    r.third = INC8(OBJ->state[slot]);
    r.arm = RU_ENTERED;
    return r;
}

static Ended rise_until_above_ground_exit_shim(ObjArm r)
{
    Ended e = ended_of_axis_move(r.move);

    e = then(e, of_sum(r.sum));
    e = then(e, of_sum(r.sum2));
    e = then(e, ended_load(r.first));
    if (r.arm == RU_HIGH) {
        return e;
    }
    e = then(e, ended_load(r.second));
    if (r.arm == RU_LOW) {
        return e;
    }
    return then(e, ended_nz(r.third));
}

void abi_RiseUntilAboveGround(void)
{
    abi_ended(rise_until_above_ground_exit_shim(RiseUntilAboveGround(cpu.x)));
}

/*
 * @routine 09EB:5DD WaitForSlot10State4
 * @run 70:B5DD
 * WHAT IT DOES: Once the object in slot $10 has reached state 4, this object's timer becomes $40 and it enters its next state.
 * HOW: A compare of that state.
 * WAITS: no.
 * Entry: X object. Exit: the last compare, load or INC.
 */
ObjArm WaitForSlot10State4(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->state[0x10]);
    r.arm = W4_EARLY;
    if (r.first < 4) {
        return r;
    }
    WR8(OBJ->life_timer[slot], 0x40);
    r.second = INC8(OBJ->state[slot]);
    r.arm = W4_ENTERED;
    return r;
}

static Ended wait_for_slot10_state4_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 4);

    if (r.arm == W4_EARLY) {
        return e;
    }
    e = then(e, ended_load(0x40));
    return then(e, ended_nz(r.second));
}

void abi_WaitForSlot10State4(void)
{
    abi_ended(wait_for_slot10_state4_exit_shim(WaitForSlot10State4(cpu.x)));
}

/*
 * @routine 09EB:7EB StartNumberedPiece
 * @run 70:B7EB
 * WHAT IT DOES: Loads the object's record; its timer comes from a table at $B801 by the slot less $18; its frame is that number plus
 * $12.
 * HOW: LoadObjectRecord and an add.
 * WAITS: no.
 * Entry: X object. Exit: A the frame, N, Z, C and V from the add, Y the number.
 */
ObjArm StartNumberedPiece(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t nz, byte;
    bool carry;

    jsr_push(0xB7EE);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    r.sum = sub_bytes(slot, 0x18, true);
    byte = rd8((uint16_t)(0xB801 + r.sum.value));
    WR8(OBJ->life_timer[slot], byte);
    r.sum2 = add_bytes(r.sum.value, 0x12, false);
    WR8(OBJ->frame[slot], r.sum2.value);
    return r;
}

static Ended start_numbered_piece_exit_shim(ObjArm r)
{
    return ended_with_y(of_sum(r.sum2), r.sum.value);
}

void abi_StartNumberedPiece(void)
{
    abi_ended(start_numbered_piece_exit_shim(StartNumberedPiece(cpu.x)));
}

/*
 * @routine 09EB:7A4 FollowPlayerHitbox
 * @run 70:B7A4
 * WHAT IT DOES: In state 0 loads the object's record; later it takes the player's place and sets its hitbox from a four-byte row of the
 * table at $BC8B picked by the player's frame less 5.
 * HOW: LoadObjectRecord (a tail jump), CopyObjectPlaceBack from slot $10, the table (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: state 0: as LoadObjectRecord; else A the last table byte, Y the row, N and Z from A, C and V from the subtract.
 */
ObjArm FollowPlayerHitbox(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state, frame, byte;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        r.arm = FH_LOAD;
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        return r;
    }
    r.arm = FH_FOLLOW;
    jsr_push(0xB7B1);
    r.third = CopyObjectPlaceBack(0x10, slot);
    jsr_pop();
    frame = RD8(OBJ->frame[0x10]);
    r.sum = sub_bytes(frame, 5, true);
    r.fourth = (uint8_t)(r.sum.value << 2);
    byte = rd8((uint16_t)(0xBC8B + r.fourth));
    WR8(OBJ->hitbox_dx[slot], byte);
    byte = rd8((uint16_t)(0xBC8C + r.fourth));
    WR8(OBJ->hitbox_dy[slot], byte);
    byte = rd8((uint16_t)(0xBC8D + r.fourth));
    WR8(OBJ->hitbox_half_w[slot], byte);
    r.fifth = rd8((uint16_t)(0xBC8E + r.fourth));
    WR8(OBJ->hitbox_half_h[slot], r.fifth);
    return r;
}

static Ended follow_player_hitbox_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == FH_LOAD) {
        return record_exit_shim(r);
    }
    e = ended_with_y(ended_load(r.third), 0x10);
    e = then(e, ended_with_y(ended_with_carry(of_sum(r.sum), (r.sum.value & 0x40) != 0), r.fourth));
    return then(e, ended_load(r.fifth));
}

void abi_FollowPlayerHitbox(void)
{
    abi_ended(follow_player_hitbox_exit_shim(FollowPlayerHitbox(cpu.x)));
}

/*
 * @routine 09EC:01A RiseAndClear
 * @run 70:B81A
 * WHAT IT DOES: Moves the object, takes $0040 off its Y speed (it climbs) and clears it once its Y position has passed from negative
 * to a low byte under $80.
 * HOW: ApplyVelocity, a 16-bit subtract, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: the last load or subtract.
 */
ObjArm RiseAndClear(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t low, high;

    jsr_push(0xB81D);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    low = RD8(OBJ->vy_lo[slot]);
    r.sum = sub_bytes(low, 0x40, true);
    WR8(OBJ->vy_lo[slot], r.sum.value);
    high = RD8(OBJ->vy_hi[slot]);
    r.sum2 = sub_bytes(high, 0, r.sum.carry);
    WR8(OBJ->vy_hi[slot], r.sum2.value);
    r.first = RD8(OBJ->y_hi[slot]);
    r.arm = RC_HIGH;
    if ((r.first & 0x80) == 0) {
        return r;
    }
    r.second = RD8(OBJ->y_lo[slot]);
    r.arm = RC_LOW;
    if ((r.second & 0x80) != 0) {
        return r;
    }
    ClearObject(slot);
    r.arm = RC_CLEARED;
    return r;
}

static Ended rise_and_clear_exit_shim(ObjArm r)
{
    Ended e = ended_of_axis_move(r.move);

    e = then(e, of_sum(r.sum));
    e = then(e, of_sum(r.sum2));
    e = then(e, ended_load(r.first));
    if (r.arm == RC_HIGH) {
        return e;
    }
    return then(e, ended_load(r.second));
}

void abi_RiseAndClear(void)
{
    abi_ended(rise_and_clear_exit_shim(RiseAndClear(cpu.x)));
}

/*
 * @routine 09EC:059 RaiseStateAfterSlot10
 * @run 70:B859
 * WHAT IT DOES: Once the object in slot $10 is in state $0A or later, an object below state 3 enters state 3.
 * HOW: Two compares.
 * Untraced: the whole arm after the first compare (B864 to B86C).
 * WAITS: no.
 * Entry: X object. Exit: the last compare or load.
 */
ObjArm RaiseStateAfterSlot10(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->state[0x10]);
    r.arm = RS_LEAD;
    if (r.first < 0x0A) {
        return r;
    }
    r.second = RD8(OBJ->state[slot]);
    r.arm = RS_STATE;
    if (r.second >= 3) {
        return r;
    }
    WR8(OBJ->state[slot], 3);
    r.arm = RS_RAISED;
    return r;
}

static Ended raise_state_after_slot10_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 0x0A);

    if (r.arm == RS_LEAD) {
        return e;
    }
    e = then(e, ended_compare(r.second, 3));
    if (r.arm == RS_STATE) {
        return e;
    }
    return then(e, ended_load(3));
}

void abi_RaiseStateAfterSlot10(void)
{
    abi_ended(raise_state_after_slot10_exit_shim(RaiseStateAfterSlot10(cpu.x)));
}

/*
 * @routine 09EC:06D StartChainLink
 * @run 70:B86D
 * WHAT IT DOES: Loads the object's record, sets OBJ_WORK_E to $41 and its timer from a table at $B888 by the slot less $12; the object in slot
 * $19 also moves to its next frame.
 * HOW: LoadObjectRecord and an INC.
 * WAITS: no.
 * Entry: X object. Exit: slot $19: N and Z of the new frame; else the compare of X with $19.
 */
ObjArm StartChainLink(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t nz;
    bool carry;

    jsr_push(0xB870);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    WR8(OBJ->work_e[slot], 0x41);
    r.sum = sub_bytes(slot, 0x12, true);
    r.first = rd8((uint16_t)(0xB888 + r.sum.value));
    WR8(OBJ->life_timer[slot], r.first);
    r.third = slot;
    r.arm = CL_OTHER;
    if (slot != 0x19) {
        return r;
    }
    r.second = INC8(OBJ->frame[slot]);
    r.arm = CL_LAST;
    return r;
}

static Ended start_chain_link_exit_shim(ObjArm r)
{
    Ended e = ended_with_y(of_sum(r.sum), r.sum.value);

    e = then(e, ended_load(r.first));
    e = then(e, cpy(r.third, 0x19));
    if (r.arm == CL_OTHER) {
        return e;
    }
    return then(e, ended_nz(r.second));
}

void abi_StartChainLink(void)
{
    abi_ended(start_chain_link_exit_shim(StartChainLink(cpu.x)));
}

/*
 * @routine 09EC:0F4 ChainFollowsPrevious
 * @run 70:B8F4
 * WHAT IT DOES: A link of a chain: object $12 takes the place of slot $10 and moves sideways by a table byte ($B923) picked by that
 * slot's facing, with Y speed 8; any other link takes the place of the link before it and takes its speed from its angle OBJ_WORK_E with a
 * length of $0A00; both then move.
 * HOW: CopyObjectPlaceBack, SetVelocityFromAngle, and a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocity, Y the table index, or 0 as the angle scale leaves it.
 */
ObjArm ChainFollowsPrevious(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t byte, angle, before;

    if (slot == 0x12) {
        r.arm = CF_FIRST;
        jsr_push(0xB8FD);
        CopyObjectPlaceBack(0x10, slot);
        jsr_pop();
        r.first = RD8(OBJ->facing_left[0x10]);
        byte = rd8((uint16_t)(0xB923 + r.first));
        WR8(OBJ->vx_hi[slot], byte);
        WR8(OBJ->vy_hi[slot], 8);
        r.move = ApplyVelocity(slot);
        return r;
    }
    r.arm = CF_NEXT;
    before = (uint8_t)(slot - 1);
    jsr_push(0xB914);
    CopyObjectPlaceBack(before, slot);
    jsr_pop();
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 0x0A);
    angle = RD8(OBJ->work_e[slot]);
    jsr_push(0xB920);
    SetVelocityFromAngle(angle, slot);
    jsr_pop();
    r.move = ApplyVelocity(slot);
    return r;
}

static Ended chain_follows_previous_exit_shim(ObjArm r)
{
    return then(ended_with_y(ended_kept(), r.arm == CF_FIRST ? r.first : 0), ended_of_axis_move(r.move));
}

void abi_ChainFollowsPrevious(void)
{
    abi_ended(chain_follows_previous_exit_shim(ChainFollowsPrevious(cpu.x)));
}

/*
 * @routine 09EC:1A5 StartKickedPiece
 * @run 70:B9A5
 * WHAT IT DOES: Loads the object's record, gives it an X speed from a table at $B9BD by its facing and a Y speed of $10, moves it once and
 * stops its Y speed.
 * HOW: LoadObjectRecord, ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocity with Y the facing.
 */
ObjArm StartKickedPiece(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t nz, byte;
    bool carry;

    jsr_push(0xB9A8);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    r.first = RD8(OBJ->facing_left[slot]);
    byte = rd8((uint16_t)(0xB9BD + r.first));
    WR8(OBJ->vx_hi[slot], byte);
    WR8(OBJ->vy_hi[slot], 0x10);
    jsr_push(0xB9B9);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    WR8(OBJ->vy_hi[slot], 0);
    return r;
}

static Ended start_kicked_piece_exit_shim(ObjArm r)
{
    return then(ended_with_y(ended_kept(), r.first), ended_of_axis_move(r.move));
}

void abi_StartKickedPiece(void)
{
    abi_ended(start_kicked_piece_exit_shim(StartKickedPiece(cpu.x)));
}

/*
 * @routine 09EC:203 StepThenClear
 * @run 70:BA03
 * WHAT IT DOES: Steps the animation list at $BA14; when it ends the object is cleared.
 * HOW: StepObjectFrames and a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step.
 */
ObjArm StepThenClear(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    r.step = step_list(slot, 0xBA14, 0xBA0E, &y, true);
    r.y = y;
    if (r.step == FRAME_LIST_ENDED) {
        ClearObject(slot);
    }
    return r;
}

void abi_StepThenClear(void)
{
    abi_ended(frame_step_exit_shim(StepThenClear(cpu.x)));
}

/*
 * @routine 09ED:032 ClearAnimation
 * @run 71:C032
 * WHAT IT DOES: Restarts the object's animation: step and timer 0.
 * HOW: Two stores.
 * WAITS: no.
 * Entry: X object. Exit: every register and flag kept.
 */
void ClearAnimation(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
}

void abi_ClearAnimation(void)
{
    ClearAnimation(cpu.x);
}

/*
 * @routine 09ED:06E EnterNextAndReset
 * @run 71:C06E
 * WHAT IT DOES: Moves the object on one state and its OBJ_WORK_E on one, restarts its animation and gives it a countdown of $20 in
 * its angle byte.
 * HOW: Two INCs, ClearAnimation, a store.
 * WAITS: no.
 * Entry: X object. Exit: A $20, N and Z from it.
 */
ObjArm EnterNextAndReset(uint8_t slot)
{
    ObjArm r = { 0 };

    INC8(OBJ->state[slot]);
    INC8(OBJ->work_e[slot]);
    jsr_push(0xC077);
    ClearAnimation(slot);
    jsr_pop();
    WR8(OBJ->angle_hi[slot], 0x20);
    return r;
}

static Ended enter_next_and_reset_exit_shim(ObjArm r)
{
    (void)r;
    return ended_load(0x20);
}

void abi_EnterNextAndReset(void)
{
    abi_ended(enter_next_and_reset_exit_shim(EnterNextAndReset(cpu.x)));
}

/*
 * @routine 09ED:07D FlipSide
 * @run 71:C07D
 * WHAT IT DOES: Flips bit 0 of the object's life timer (the side it works on).
 * HOW: An EOR.
 * WAITS: no.
 * Entry: X object. Exit: A the new value, N and Z from it.
 */
ObjArm FlipSide(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t side;

    side = RD8(OBJ->life_timer[slot]);
    r.first = (uint8_t)(side ^ 0x01);
    WR8(OBJ->life_timer[slot], r.first);
    return r;
}

static Ended flip_side_exit_shim(ObjArm r)
{
    return ended_load(r.first);
}

void abi_FlipSide(void)
{
    abi_ended(flip_side_exit_shim(FlipSide(cpu.x)));
}

/*
 * @routine 09ED:039 StepListByPhase
 * @run 71:C039
 * WHAT IT DOES: The object faces as its life timer says and steps the animation list that a table at $C103 names for its OBJ_WORK_E.
 * HOW: Reads the list's address into zTemp10-11; a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, Y kept unless the step sets it (the index otherwise).
 */
ObjArm StepListByPhase(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t facing, phase, index, y;

    facing = RD8(OBJ->life_timer[slot]);
    WR8(OBJ->facing_left[slot], facing);
    phase = RD8(OBJ->work_e[slot]);
    index = (uint8_t)(phase << 1);
    y = index;
    WR8(ZP->temp10, rd8((uint16_t)(0xC103 + index)));
    WR8(ZP->temp11, rd8((uint16_t)(0xC104 + index)));
    r.step = StepObjectFrames(slot, &y);
    r.y = y;
    return r;
}

/* The step's exit with Y always taken as set (the list index is Y's entry here). */
static Ended step_list_by_phase_exit_shim(ObjArm r)
{
    return ended_of_frame_step(r.step, true, r.y);
}

void abi_StepListByPhase(void)
{
    abi_ended(step_list_by_phase_exit_shim(StepListByPhase(cpu.x)));
}

/*
 * @routine 09ED:0C3 SetSwaySpeed
 * @run 71:C0C3
 * WHAT IT DOES: Sets the object's X speed from a word of a table at $C0DE, picked by OBJ_WORK_E times 4 plus life timer times 2.
 * HOW: Two shifts, an add with carry, two table reads (bank 71).
 * WAITS: no.
 * Entry: X object. Exit: A the high byte, Y the index, N and Z from A, C and V from the add.
 */
ObjArm SetSwaySpeed(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t phase, side, byte, mix;
    bool carry;

    phase = RD8(OBJ->work_e[slot]);
    mix = (uint8_t)(phase << 2);
    WR8(ZP->temp10, mix);
    side = RD8(OBJ->life_timer[slot]);
    carry = (side & 0x80) != 0;
    mix = RD8(ZP->temp10);
    r.sum = add_bytes((uint8_t)(side << 1), mix, carry);
    byte = rd8((uint16_t)(0xC0DE + r.sum.value));
    WR8(OBJ->vx_lo[slot], byte);
    r.first = rd8((uint16_t)(0xC0DF + r.sum.value));
    WR8(OBJ->vx_hi[slot], r.first);
    return r;
}

static Ended set_sway_speed_exit_shim(ObjArm r)
{
    return ended_with_y(then(of_sum(r.sum), ended_load(r.first)), r.sum.value);
}

void abi_SetSwaySpeed(void)
{
    abi_ended(set_sway_speed_exit_shim(SetSwaySpeed(cpu.x)));
}

/*
 * @routine 09ED:086 BobAndMove
 * @run 71:C086
 * WHAT IT DOES: Makes the object bob: OBJ_WORK_H counts up and its sine (scaled by 3FF) added to OBJ_WORK_G is its Y low byte; between
 * Y $B0 and the right place on screen (X from $80 to $9F once the scroll is added) a random wobble of -1 to 2 is added; then it moves.
 * HOW: ScaleBySine, NextRandom, a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocity, Y 0 as the sine's exit leaves it.
 */
ObjArm BobAndMove(uint8_t slot)
{
    ObjArm r = { 0 };
    RandomResult random;
    uint8_t phase, base, x_low, wobble;
    Sum sum;

    WR8(ZP->temp00, 0xFF);
    WR8(ZP->temp01, 3);
    INC8(OBJ->work_h[slot]);
    phase = RD8(OBJ->work_h[slot]);
    jsr_push(0xC097);
    ScaleBySine(phase);
    jsr_pop();
    base = RD8(ZP->product.hi);
    sum = add_bytes(base, RD8(OBJ->angle_lo[slot]), false);
    WR8(OBJ->y_lo[slot], sum.value);
    if (sum.value < 0xB0) {
        x_low = RD8(OBJ->x_lo[slot]);
        sum = add_bytes(x_low, rd8(ScrollXLo), false);
        if (sum.value >= 0x80 && sum.value < 0xA0) {
            jsr_push(0xC0B6);
            random = NextRandom();
            jsr_pop();
            wobble = (uint8_t)((random.value & 0x03) - 1);
            base = RD8(OBJ->y_lo[slot]);
            sum = add_bytes(base, wobble, false);
            WR8(OBJ->y_lo[slot], sum.value);
        }
    }
    r.move = ApplyVelocity(slot);
    return r;
}

static Ended bob_and_move_exit_shim(ObjArm r)
{
    return then(ended_with_y(ended_kept(), 0), ended_of_axis_move(r.move));
}

void abi_BobAndMove(void)
{
    abi_ended(bob_and_move_exit_shim(BobAndMove(cpu.x)));
}

/*
 * @routine 09ED:051 ReactAtWindowEdge
 * @run 71:C051
 * WHAT IT DOES: When the object's X plus the scroll is at the edge its side (life timer 0 or 1) names ($E0 or more for 0, under $20 for 1) and it
 * is in state 1, it goes on as EnterNextAndReset.
 * HOW: ScrollXLo and two compares; the last step runs on into the next routine's bytes.
 * Untraced: one byte (the fall-through branch).
 * WAITS: no.
 * Entry: X object. Exit: the last compare or the load; Y the side.
 */
ObjArm ReactAtWindowEdge(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t x_low;

    x_low = RD8(OBJ->x_lo[slot]);
    r.sum = add_bytes(x_low, rd8(ScrollXLo), false);
    r.first = RD8(OBJ->life_timer[slot]);
    r.arm = RE_OFF_EDGE;
    if (r.first == 0) {
        if (r.sum.value < 0xE0) {
            return r;
        }
    } else {
        if (r.sum.value >= 0x20) {
            return r;
        }
    }
    r.second = RD8(OBJ->state[slot]);
    r.arm = RE_WRONG_STATE;
    if (r.second != 1) {
        return r;
    }
    r.sub = obj_sub(EnterNextAndReset(slot));
    r.arm = RE_ENTERED;
    return r;
}

static Ended react_at_window_edge_exit_shim(ObjArm r)
{
    Ended e = of_sum(r.sum);

    e = then(e, ended_with_y(ended_nz(r.first), r.first));
    if (r.first == 0) {
        e = then(e, ended_compare(r.sum.value, 0xE0));
    } else {
        e = then(e, ended_compare(r.sum.value, 0x20));
    }
    if (r.arm == RE_OFF_EDGE) {
        return e;
    }
    e = then(e, ended_compare(r.second, 1));
    if (r.arm == RE_WRONG_STATE) {
        return e;
    }
    return then(e, enter_next_and_reset_exit_shim(obj_arm(r.sub)));
}

void abi_ReactAtWindowEdge(void)
{
    abi_ended(react_at_window_edge_exit_shim(ReactAtWindowEdge(cpu.x)));
}

/* The three steps of a swaying object, which two routines branch into the middle of: the list, the speed, the bob. */
ObjArm SwayStep(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xC006);
    r.sub = obj_sub(StepListByPhase(slot));
    jsr_pop();
    jsr_push(0xC009);
    r.sub2 = obj_sub(SetSwaySpeed(slot));
    jsr_pop();
    r.sub3 = obj_sub(BobAndMove(slot));
    return r;
}

static Ended sway_step_exit_shim(ObjArm r)
{
    Ended e = step_list_by_phase_exit_shim(obj_arm(r.sub));

    e = then(e, set_sway_speed_exit_shim(obj_arm(r.sub2)));
    return then(e, bob_and_move_exit_shim(obj_arm(r.sub3)));
}

/*
 * @routine 09EC:7FE SwayOrAdvance
 * @run 70:BFFE
 * WHAT IT DOES: Counts the object's angle byte down; at zero it goes on as EnterNextAndReset, otherwise it sways (SwayStep).
 * HOW: DEC, then the tail calls.
 * WAITS: no.
 * Entry: X object. Exit: as EnterNextAndReset or BobAndMove.
 */
ObjArm SwayOrAdvance(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = DEC8(OBJ->angle_hi[slot]);
    if (r.first == 0) {
        r.arm = SA_ADVANCED;
        r.sub = obj_sub(EnterNextAndReset(slot));
        return r;
    }
    r.arm = SA_SWAYED;
    r.sub = obj_sub(SwayStep(slot));
    return r;
}

static Ended sway_or_advance_exit_shim(ObjArm r)
{
    if (r.arm == SA_ADVANCED) {
        return enter_next_and_reset_exit_shim(obj_arm(r.sub));
    }
    return then(ended_nz(r.first), sway_step_exit_shim(obj_arm(r.sub)));
}

void abi_SwayOrAdvance(void)
{
    abi_ended(sway_or_advance_exit_shim(SwayOrAdvance(cpu.x)));
}

/*
 * @routine 09EC:7F2 SwayThenReact
 * @run 70:BFF2
 * WHAT IT DOES: Steps the animation by phase, sets the sway speed, bobs and moves, then reacts at the window edge.
 * HOW: StepListByPhase, SetSwaySpeed, BobAndMove, a tail jump into ReactAtWindowEdge.
 * WAITS: no.
 * Entry: X object. Exit: as ReactAtWindowEdge.
 */
ObjArm SwayThenReact(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xBFF5);
    r.sub = obj_sub(StepListByPhase(slot));
    jsr_pop();
    jsr_push(0xBFF8);
    r.sub2 = obj_sub(SetSwaySpeed(slot));
    jsr_pop();
    jsr_push(0xBFFB);
    r.sub3 = obj_sub(BobAndMove(slot));
    jsr_pop();
    r.sub4 = obj_sub(ReactAtWindowEdge(slot));
    return r;
}

static Ended sway_then_react_exit_shim(ObjArm r)
{
    Ended e = step_list_by_phase_exit_shim(obj_arm(r.sub));

    e = then(e, set_sway_speed_exit_shim(obj_arm(r.sub2)));
    e = then(e, bob_and_move_exit_shim(obj_arm(r.sub3)));
    return then(e, react_at_window_edge_exit_shim(obj_arm(r.sub4)));
}

void abi_SwayThenReact(void)
{
    abi_ended(sway_then_react_exit_shim(SwayThenReact(cpu.x)));
}

/*
 * @routine 09ED:00C SwayThenFlip
 * @run 71:C00C
 * WHAT IT DOES: Counts the angle byte down; at zero it flips the side, goes on as EnterNextAndReset and sets OBJ_WORK_E to 1; otherwise it sways.
 * HOW: DEC, FlipSide, EnterNextAndReset, or SwayStep.
 * WAITS: no.
 * Entry: X object. Exit: at zero A 1 from the load; else as BobAndMove.
 */
ObjArm SwayThenFlip(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = DEC8(OBJ->angle_hi[slot]);
    if (r.first != 0) {
        r.arm = SY_SWAYED;
        r.sub = obj_sub(SwayStep(slot));
        return r;
    }
    r.arm = SY_FLIPPED;
    jsr_push(0xC016);
    r.sub = obj_sub(FlipSide(slot));
    jsr_pop();
    jsr_push(0xC019);
    r.sub2 = obj_sub(EnterNextAndReset(slot));
    jsr_pop();
    WR8(OBJ->work_e[slot], 1);
    return r;
}

static Ended sway_then_flip_exit_shim(ObjArm r)
{
    Ended e = ended_nz(r.first);

    if (r.arm == SY_SWAYED) {
        return then(e, sway_step_exit_shim(obj_arm(r.sub)));
    }
    e = then(e, flip_side_exit_shim(obj_arm(r.sub)));
    e = then(e, enter_next_and_reset_exit_shim(obj_arm(r.sub2)));
    return then(e, ended_load(1));
}

void abi_SwayThenFlip(void)
{
    abi_ended(sway_then_flip_exit_shim(SwayThenFlip(cpu.x)));
}

/*
 * @routine 09ED:01F SwayThenRestart
 * @run 71:C01F
 * WHAT IT DOES: Counts the angle byte down; at zero it goes on as EnterNextAndReset, puts the object in state 1 and OBJ_WORK_E at 0; otherwise it sways.
 * HOW: DEC, EnterNextAndReset, or SwayStep.
 * WAITS: no.
 * Entry: X object. Exit: at zero A 1 from the load; else as BobAndMove.
 */
ObjArm SwayThenRestart(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = DEC8(OBJ->angle_hi[slot]);
    if (r.first != 0) {
        r.arm = SY_SWAYED;
        r.sub = obj_sub(SwayStep(slot));
        return r;
    }
    r.arm = SY_FLIPPED;
    jsr_push(0xC029);
    r.sub = obj_sub(EnterNextAndReset(slot));
    jsr_pop();
    WR8(OBJ->state[slot], 1);
    WR8(OBJ->work_e[slot], 0);
    return r;
}

static Ended sway_then_restart_exit_shim(ObjArm r)
{
    Ended e = ended_nz(r.first);

    if (r.arm == SY_SWAYED) {
        return then(e, sway_step_exit_shim(obj_arm(r.sub)));
    }
    e = then(e, enter_next_and_reset_exit_shim(obj_arm(r.sub)));
    return then(e, ended_load(1));
}

void abi_SwayThenRestart(void)
{
    abi_ended(sway_then_restart_exit_shim(SwayThenRestart(cpu.x)));
}

/*
 * @routine 09EC:7BC JumpByStateTable
 * @run 70:BFBC
 * WHAT IT DOES: Goes to the code a table at $BFCE names for the object's state.
 * HOW: The table's two bytes go to zTemp10-11 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
ObjArm JumpByStateTable(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t index, low;

    r.third = RD8(OBJ->state[slot]);
    index = (uint8_t)(r.third << 1);
    r.second = index;
    low = rd8((uint16_t)(0xBFCE + index));
    WR8(ZP->temp10, low);
    r.first = rd8((uint16_t)(0xBFCF + index));
    WR8(ZP->temp11, r.first);
    r.target = RD16_LH2(ZP->temp10, ZP->temp11);
    return r;
}

static Jump jump_by_state_table_exit_shim(ObjArm r)
{
    Jump jump = { { 0 }, r.target, true };

    jump.end = ended_with_carry(ended_with_y(ended_load(r.first), r.second), (r.third & 0x80) != 0);
    return jump;
}

void abi_JumpByStateTable(void)
{
    Jump jump = jump_by_state_table_exit_shim(JumpByStateTable(cpu.x));

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 09EC:090 ChainCountdown
 * @run 70:B890
 * WHAT IT DOES: A link of the chain follows the one before it (ChainFollowsPrevious); its timer counts down and at zero it enters its next state.
 * HOW: ChainFollowsPrevious, DEC, INC.
 * WAITS: no.
 * Entry: X object. Exit: as ChainFollowsPrevious, then N and Z of the count or of the new state.
 */
ObjArm ChainCountdown(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB893);
    r.sub = obj_sub(ChainFollowsPrevious(slot));
    jsr_pop();
    r.first = DEC8(OBJ->life_timer[slot]);
    r.arm = CC_COUNTING;
    if (r.first != 0) {
        return r;
    }
    r.second = INC8(OBJ->state[slot]);
    r.arm = CC_ENTERED;
    return r;
}

static Ended chain_countdown_exit_shim(ObjArm r)
{
    Ended e = then(chain_follows_previous_exit_shim(obj_arm(r.sub)), ended_nz(r.first));

    if (r.arm == CC_COUNTING) {
        return e;
    }
    return then(e, ended_nz(r.second));
}

void abi_ChainCountdown(void)
{
    abi_ended(chain_countdown_exit_shim(ChainCountdown(cpu.x)));
}

/*
 * @routine 09EC:09C ChainSwings
 * @run 70:B89C
 * WHAT IT DOES: A link follows the one before it, takes slot $10's facing, and swings: the cosine of its angle OBJ_WORK_G (multiplier $0200) is added to OBJ_WORK_F and OBJ_WORK_E as
 * a 16-bit pair, and a 16-bit step from a table at $B8E4 (by OBJ_WORK_I and 7) is added to OBJ_WORK_H and OBJ_WORK_G; a carry out of that goes up one on OBJ_WORK_I.
 * HOW: ChainFollowsPrevious, ScaleByCosine, table reads (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: the last add or INC.
 */
ObjArm ChainSwings(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t facing, angle, low, high, phase, step;
    Sum sum;

    jsr_push(0xB89F);
    ChainFollowsPrevious(slot);
    jsr_pop();
    facing = RD8(OBJ->facing_left[0x10]);
    WR8(OBJ->facing_left[slot], facing);
    WR8(ZP->temp01, 2);
    WR8(ZP->temp00, 0);
    angle = RD8(OBJ->angle_lo[slot]);
    jsr_push(0xB8B3);
    ScaleByCosine(angle);
    jsr_pop();
    low = RD8(OBJ->angle_hi[slot]);
    sum = add_bytes(low, RD8(ZP->product.mid), false);
    WR8(OBJ->angle_hi[slot], sum.value);
    high = RD8(OBJ->work_e[slot]);
    r.sum = add_bytes(high, RD8(ZP->product.hi), sum.carry);
    WR8(OBJ->work_e[slot], r.sum.value);
    phase = RD8(OBJ->work_i[slot]);
    phase = (uint8_t)(phase & 0x07);
    r.first = (uint8_t)(phase << 1);
    step = RD8(OBJ->work_h[slot]);
    r.sum2 = add_bytes(step, rd8((uint16_t)(0xB8E4 + r.first)), false);
    WR8(OBJ->work_h[slot], r.sum2.value);
    low = RD8(OBJ->angle_lo[slot]);
    r.sum3 = add_bytes(low, rd8((uint16_t)(0xB8E5 + r.first)), r.sum2.carry);
    WR8(OBJ->angle_lo[slot], r.sum3.value);
    r.arm = CS_STEADY;
    if (!r.sum3.carry) {
        return r;
    }
    r.second = INC8(OBJ->work_i[slot]);
    r.arm = CS_NEXT;
    return r;
}

/* The first add's chain is dead (the second add's flags replace it); the exit is the rest. */
static Ended chain_swings_exit_shim(ObjArm r)
{
    Ended e = of_sum(r.sum);

    e = then(e, ended_with_y(ended_with_carry(ended_load(r.first), false), r.first));
    e = then(e, of_sum(r.sum2));
    e = then(e, of_sum(r.sum3));
    if (r.arm == CS_STEADY) {
        return e;
    }
    return then(e, ended_nz(r.second));
}

void abi_ChainSwings(void)
{
    abi_ended(chain_swings_exit_shim(ChainSwings(cpu.x)));
}

/*
 * @routine 09EA:72C WalkAndPick
 * @run 70:AF2C
 * WHAT IT DOES: WalkSideways; the walk direction OBJ_WORK_F becomes the facing turned round; when the player is $80 or more away along X the object enters state 1; then
 * PickNextAction (a tail jump).
 * HOW: WalkSideways, DistanceXToPlayer, PickNextAction.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: as PickNextAction.
 */
ObjArm WalkAndPick(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t facing;

    jsr_push(0xAF2F);
    r.sub = obj_sub(WalkSideways(slot));
    jsr_pop();
    facing = RD8(OBJ->facing_left[slot]);
    r.first = (uint8_t)(facing ^ 0x01);
    WR8(OBJ->angle_hi[slot], r.first);
    jsr_push(0xAF3A);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.arm = WA_PICK_ONLY;
    if (r.distance.distance >= 0x80) {
        WR8(OBJ->state[slot], 1);
        r.arm = WA_STATE1;
    }
    r.sub2 = obj_sub(PickNextAction(slot));
    return r;
}

static Ended walk_and_pick_exit_shim(ObjArm r)
{
    Ended e = walk_sideways_exit_shim(obj_arm(r.sub));

    e = then(e, ended_load(r.first));
    e = then(e, ended_of_distance(r.distance));
    e = then(e, ended_compare(r.distance.distance, 0x80));
    if (r.arm == WA_STATE1) {
        e = then(e, ended_load(1));
    }
    return then(e, pick_next_action_exit_shim(obj_arm(r.sub2)));
}

void abi_WalkAndPick(void)
{
    abi_ended(walk_and_pick_exit_shim(WalkAndPick(cpu.x)));
}

/*
 * @routine 09EA:746 StepShootOnCue
 * @run 70:AF46
 * WHAT IT DOES: Steps the animation list at $AF89; when it ends the object enters state 2; on a new frame with animation step 3 it plays sound $26; then PlayCueOnFrame runs;
 * on frame $0A with OBJ_WORK_G's low three bits clear it starts a kind $24 object at its place in the first free slot from $30, facing as it does.
 * HOW: StepObjectFrames, SoundCommand, PlayCueOnFrame, FindFreeSlotFrom30, StartObjectAtPlace.
 * Untraced: one byte.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
 */
ObjArm StepShootOnCue(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t found_slot;
    bool found;

    r.step = step_list(slot, 0xAF89, 0xAF51, &y, true);
    r.y = y;
    if (r.step == FRAME_LIST_ENDED) {
        WR8(OBJ->state[slot], 2);
        r.arm = SC_ENDED;
        return r;
    }
    if (r.step == FRAME_CHANGED) {
        r.first = RD8(OBJ->anim_step[slot]);
        if (r.first == 3) {
            r.sounded = true;
            r.command = 0x26;
            jsr_push(0xAF61);
            r.call = SoundCommand(0x26, slot, y);
            jsr_pop();
        }
    }
    jsr_push(0xAF64);
    r.sub = obj_sub(PlayCueOnFrame(slot));
    jsr_pop();
    r.second = RD8(OBJ->frame[slot]);
    r.arm = SC_NO_FRAME;
    if (r.second != 0x0A) {
        return r;
    }
    r.third = RD8(OBJ->angle_lo[slot]);
    r.third = (uint8_t)(r.third & 0x07);
    r.arm = SC_BITS;
    if (r.third != 0) {
        return r;
    }
    jsr_push(0xAF75);
    found_slot = FindFreeSlotFrom30(&r.fourth, &found);
    jsr_pop();
    r.fifth = found_slot;
    r.flag = found;
    r.arm = SC_NO_SLOT;
    if (!found) {
        return r;
    }
    jsr_push(0xAF7C);
    r.sixth = StartObjectAtPlace(0x24, slot, found_slot);
    jsr_pop();
    r.seventh = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[found_slot], r.seventh);
    r.arm = SC_SHOT;
    return r;
}

static SoundEnd step_shoot_on_cue_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = frame_step_exit_shim(r);

    s.end = e;
    if (r.arm == SC_ENDED) {
        s.end = then(e, ended_load(2));
        return s;
    }
    if (r.step == FRAME_CHANGED) {
        e = then(e, ended_compare(r.first, 3));
        if (r.first == 3) {
            s.before = then(e, ended_load(0x26));
            s.sounded = true;
            s.command = r.command;
            s.call = r.call;
            e = ended_kept();
        }
    }
    e = then(e, play_cue_on_frame_exit_shim(obj_arm(r.sub)));
    e = then(e, ended_compare(r.second, 0x0A));
    s.end = e;
    if (r.arm == SC_NO_FRAME) {
        return s;
    }
    e = then(e, ended_load(r.third));
    s.end = e;
    if (r.arm == SC_BITS) {
        return s;
    }
    e = then(e, ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), r.fourth), r.fifth), r.flag));
    s.end = e;
    if (r.arm == SC_NO_SLOT) {
        return s;
    }
    e = then(e, ended_load(r.sixth));
    s.end = then(e, ended_load(r.seventh));
    return s;
}

void abi_StepShootOnCue(void)
{
    abi_sound_end(step_shoot_on_cue_exit_shim(StepShootOnCue(cpu.x, cpu.y)));
}

/*
 * @routine 09EA:5CB AdvanceByHealth
 * @run 70:ADCB
 * WHAT IT DOES: An object with a state moves to its next phase (OBJ_WORK_H) when its health OBJ_HEALTH falls below the limit a table at $AE09 gives for the phase (phase 2 moves on
 * when bit 7 of the health is set): its state becomes the byte a table at $AE0B gives, the phase goes up, the animation and OBJ_SUBSTATE are reset, and from phase 2 its OBJ_PACE loses
 * bits 3 to 6.
 * HOW: Table reads (bank 70).
 * Untraced: the phase 2 test (ADD7) and the OBJ_PACE mask (AE00).
 * WAITS: no.
 * Entry: X object. Exit: the last compare, load or AND.
 */
ObjArm AdvanceByHealth(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t health;

    r.first = RD8(OBJ->state[slot]);
    r.arm = AH_NO_STATE;
    if (r.first == 0) {
        return r;
    }
    r.second = RD8(OBJ->work_h[slot]);
    r.arm = AH_BLOCKED;
    if (r.second == 2) {
        health = RD8(OBJ->health[slot]);
        r.third = (uint8_t)(health & 0x80);
        if (r.third == 0) {
            return r;
        }
    } else {
        r.third = RD8(OBJ->health[slot]);
        r.fourth = rd8((uint16_t)(0xAE09 + r.second));
        if (r.third >= r.fourth) {
            return r;
        }
    }
    r.fifth = rd8((uint16_t)(0xAE0B + r.second));
    WR8(OBJ->state[slot], r.fifth);
    INC8(OBJ->work_h[slot]);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->substate[slot], 0);
    r.sixth = RD8(OBJ->work_h[slot]);
    r.arm = AH_ADVANCED;
    if (r.sixth < 2) {
        return r;
    }
    r.seventh = RD8(OBJ->pace[slot]);
    r.seventh = (uint8_t)(r.seventh & 0x87);
    WR8(OBJ->pace[slot], r.seventh);
    r.arm = AH_FLAGGED;
    return r;
}

static Ended advance_by_health_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == AH_NO_STATE) {
        return e;
    }
    e = then(e, ended_with_y(ended_nz(r.second), r.second));
    e = then(e, cpy(r.second, 2));
    if (r.second == 2) {
        e = then(e, ended_load(r.third));
    } else {
        e = then(e, ended_compare(r.third, r.fourth));
    }
    if (r.arm == AH_BLOCKED) {
        return e;
    }
    e = then(e, ended_load(r.fifth));
    e = then(e, ended_compare(r.sixth, 2));
    if (r.arm == AH_ADVANCED) {
        return e;
    }
    return then(e, ended_load(r.seventh));
}

void abi_AdvanceByHealth(void)
{
    abi_ended(advance_by_health_exit_shim(AdvanceByHealth(cpu.x)));
}

/*
 * @routine 09EB:14D PickGroundSpeedInWindow
 * @run 70:B14D
 * WHAT IT DOES: When the object is inside the window (InWindowRange), OBJ_SUBSTATE takes a byte of a sixteen-byte table at $B15F picked at random.
 * HOW: InWindowRange, NextRandom, a table read (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: outside: the window test's flags; inside: A the table byte, Y the pick, C and V from the random.
 */
ObjArm PickGroundSpeedInWindow(uint8_t slot)
{
    ObjArm r = { 0 };
    ObjArm view;

    jsr_push(0xB150);
    view = InWindowRange(slot);
    jsr_pop();
    r.sub = obj_sub(view);
    r.arm = PG_OUTSIDE;
    if (!view.flag) {
        return r;
    }
    jsr_push(0xB155);
    r.random = NextRandom();
    jsr_pop();
    r.first = (uint8_t)(r.random.value & 0x0F);
    r.second = rd8((uint16_t)(0xB15F + r.first));
    WR8(OBJ->substate[slot], r.second);
    r.arm = PG_PICKED;
    return r;
}

static Ended pick_ground_speed_in_window_exit_shim(ObjArm r)
{
    Ended e = in_window_range_exit_shim(obj_arm(r.sub)).end;

    if (r.arm == PG_OUTSIDE) {
        return e;
    }
    e = then(e, ended_of_random(r.random));
    e = then(e, ended_with_y(ended_load(r.first), r.first));
    return then(e, ended_load(r.second));
}

void abi_PickGroundSpeedInWindow(void)
{
    abi_ended(pick_ground_speed_in_window_exit_shim(PickGroundSpeedInWindow(cpu.x)));
}

/*
 * @routine 09EB:14A MoveThenPick
 * @run 70:B14A
 * WHAT IT DOES: Moves the object along X, then PickGroundSpeedInWindow.
 * HOW: ApplyVelocityX, then the next routine's bytes.
 * WAITS: no.
 * Entry: X object. Exit: as PickGroundSpeedInWindow.
 */
ObjArm MoveThenPick(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB14D);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    r.sub = obj_sub(PickGroundSpeedInWindow(slot));
    return r;
}

static Ended move_then_pick_exit_shim(ObjArm r)
{
    return then(ended_of_axis_move(r.move), pick_ground_speed_in_window_exit_shim(obj_arm(r.sub)));
}

void abi_MoveThenPick(void)
{
    abi_ended(move_then_pick_exit_shim(MoveThenPick(cpu.x)));
}

/*
 * @routine 09EB:1C7 SwingAndPick
 * @run 70:B1C7
 * WHAT IT DOES: Sets the object's Y speed to the cosine of its OBJ_WORK_I count (multiplier $0150), moves it, counts OBJ_WORK_I up and goes on as PickGroundSpeedInWindow.
 * HOW: ScaleByCosine, ApplyVelocity, a tail jump into PickGroundSpeedInWindow.
 * WAITS: no.
 * Entry: X object. Exit: as PickGroundSpeedInWindow.
 */
ObjArm SwingAndPick(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t angle, high, low;

    WR8(ZP->temp00, 0x50);
    WR8(ZP->temp01, 1);
    angle = RD8(OBJ->work_i[slot]);
    jsr_push(0xB1D5);
    ScaleByCosine(angle);
    jsr_pop();
    high = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], high);
    low = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], low);
    jsr_push(0xB1E2);
    ApplyVelocity(slot);
    jsr_pop();
    INC8(OBJ->work_i[slot]);
    r.sub = obj_sub(PickGroundSpeedInWindow(slot));
    return r;
}

static Ended swing_and_pick_exit_shim(ObjArm r)
{
    return then(ended_with_y(ended_kept(), 0), pick_ground_speed_in_window_exit_shim(obj_arm(r.sub)));
}

void abi_SwingAndPick(void)
{
    abi_ended(swing_and_pick_exit_shim(SwingAndPick(cpu.x)));
}

/*
 * @routine 09EC:1BF FallStepAndLeave
 * @run 70:B9BF
 * WHAT IT DOES: Steps the animation list at $B9F2, adds $0010 to the Y speed and moves down; once Y low is $C0 or more the hitbox goes off, the animation resets and the
 * object enters its next state.
 * HOW: StepObjectFrames, ApplyVelocityY.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the last compare or INC.
 */
ObjArm FallStepAndLeave(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0, low, high;

    r.step = step_list(slot, 0xB9F2, 0xB9CA, &y, true);
    r.y = y;
    low = RD8(OBJ->vy_lo[slot]);
    r.sum = add_bytes(low, 0x10, false);
    WR8(OBJ->vy_lo[slot], r.sum.value);
    high = RD8(OBJ->vy_hi[slot]);
    r.sum2 = add_bytes(high, 0, r.sum.carry);
    WR8(OBJ->vy_hi[slot], r.sum2.value);
    jsr_push(0xB9DE);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.first = RD8(OBJ->y_lo[slot]);
    r.arm = FL_FALLING;
    if (r.first < 0xC0) {
        return r;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.second = INC8(OBJ->state[slot]);
    r.arm = FL_LEFT;
    return r;
}

static Ended fall_step_and_leave_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    e = then(e, of_sum(r.sum));
    e = then(e, of_sum(r.sum2));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_compare(r.first, 0xC0));
    if (r.arm == FL_FALLING) {
        return e;
    }
    return then(e, ended_nz(r.second));
}

void abi_FallStepAndLeave(void)
{
    abi_ended(fall_step_and_leave_exit_shim(FallStepAndLeave(cpu.x)));
}

/*
 * @routine 093C:082 TouchesPlayerMark
 * @run 70:B882
 * WHAT IT DOES: Says, with C, whether a mark applies to the object: false (C clear) when the object has no state or is in state 4 or the player byte TimeSlowMode is 0;
 * otherwise a table byte decides: with the player byte 2 the table at $A166, else the table at $A146, indexed by the wave counter's low three bits plus $18; a nonzero
 * byte (zero for the first table) says yes (C set).
 * HOW: Compares and a table read (bank 70).
 * Untraced: the player byte 2 arm (B88E to B89A).
 * WAITS: no.
 * Entry: X object. Exit: C the answer; A and N, Z as the last load or compare left them.
 */
ObjArm TouchesPlayerMark(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t wave;

    r.first = RD8(OBJ->state[slot]);
    r.arm = TM_NO_STATE;
    if (r.first == 0) {
        return r;
    }
    r.arm = TM_STATE4;
    if (r.first == 4) {
        return r;
    }
    r.second = rd8(TimeSlowMode);
    r.arm = TM_NO_MODE;
    if (r.second == 0) {
        return r;
    }
    wave = RD8(ZP->wave_counter);
    r.third = (uint8_t)(wave & 0x07);
    r.sum = add_bytes(r.third, 0x18, false);
    r.fourth = r.sum.value;
    if (r.second == 2) {
        r.fifth = rd8((uint16_t)(0xA166 + r.fourth));
        r.flag = r.fifth != 0;
    } else {
        r.fifth = rd8((uint16_t)(0xA146 + r.fourth));
        r.flag = r.fifth != 0;
    }
    r.arm = TM_TABLE;
    return r;
}

/* Whether the mark applies (C), and what the routine left. */
static ViewTest touches_player_mark_exit_shim(ObjArm r)
{
    ViewTest v = { r.flag, { 0 } };
    Ended e = ended_load(r.first);

    if (r.arm == TM_NO_STATE) {
        v.end = then(e, ended_with_carry(ended_kept(), false));
        return v;
    }
    e = then(e, ended_compare(r.first, 4));
    if (r.arm == TM_STATE4) {
        v.end = then(e, ended_with_carry(ended_kept(), false));
        return v;
    }
    e = then(e, ended_load(r.second));
    if (r.arm == TM_NO_MODE) {
        v.end = then(e, ended_with_carry(ended_kept(), false));
        return v;
    }
    e = then(e, ended_compare(r.second, 2));
    e = then(e, ended_load(r.third));
    e = then(e, of_sum(r.sum));
    e = then(e, ended_with_y(ended_kept(), r.fourth));
    e = then(e, ended_load(r.fifth));
    v.end = then(e, ended_with_carry(ended_kept(), r.flag));
    return v;
}

void abi_TouchesPlayerMark(void)
{
    abi_ended(touches_player_mark_exit_shim(TouchesPlayerMark(cpu.x)).end);
}

/*
 * @routine 093C:1DD CountBothPairs
 * @run 70:B9DD
 * WHAT IT DOES: Runs CountTimerPairDownOrRepick and CountAnglePairDownOrRepick.
 * HOW: Two calls.
 * WAITS: no.
 * Entry: X object. Exit: as the second, over the first's.
 */
ObjArm CountBothPairs(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB9E0);
    r.sub = obj_sub(CountTimerPairDownOrRepick(slot));
    jsr_pop();
    jsr_push(0xB9E3);
    r.sub2 = obj_sub(CountAnglePairDownOrRepick(slot));
    jsr_pop();
    return r;
}

static Ended count_both_pairs_exit_shim(ObjArm r)
{
    Ended e = count_timer_pair_down_or_repick_exit_shim(obj_arm(r.sub));

    return then(e, count_angle_pair_down_or_repick_exit_shim(obj_arm(r.sub2)));
}

void abi_CountBothPairs(void)
{
    abi_ended(count_both_pairs_exit_shim(CountBothPairs(cpu.x)));
}

/*
 * @routine 09E9:6B0 HealAndClear
 * @run 70:A6B0
 * WHAT IT DOES: An item that heals: the player's health goes up $28 (to at most $5C) and the item is cleared; when the health is already $5C the object instead plays sound $0B
 * and becomes a kind $9F in state 0 with a timer of 3.
 * HOW: A compare, an add, a tail jump into ClearObject.
 * Untraced: the full-health arm (A6B6) and the cap (A6D4).
 * WAITS: no.
 * Entry: X object. Exit: the last compare, add or load.
 */
ObjArm HealAndClear(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t health;

    r.first = RD8(ZP->player_health);
    if (r.first == 0x5C) {
        r.arm = HC_FULL;
        r.sounded = true;
        r.command = 0x0B;
        jsr_push(0xA6BB);
        r.call = SoundCommand(0x0B, slot, y);
        jsr_pop();
        WR8(OBJ->kind[slot], 0x9F);
        WR8(OBJ->life_timer[slot], 3);
        WR8(OBJ->state[slot], 0);
        return r;
    }
    health = RD8(ZP->player_health);
    r.sum = add_bytes(health, 0x28, false);
    WR8(ZP->player_health, r.sum.value);
    r.arm = HC_HEALED;
    if (r.sum.value >= 0x5C) {
        WR8(ZP->player_health, 0x5C);
        r.arm = HC_CAPPED;
    }
    ClearObject(slot);
    return r;
}

static SoundEnd heal_and_clear_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = ended_compare(r.first, 0x5C);

    if (r.arm == HC_FULL) {
        s.before = then(e, ended_load(0x0B));
        s.sounded = true;
        s.command = r.command;
        s.call = r.call;
        s.end = ended_load(0);
        return s;
    }
    e = then(e, of_sum(r.sum));
    e = then(e, ended_compare(r.sum.value, 0x5C));
    if (r.arm == HC_CAPPED) {
        e = then(e, ended_load(0x5C));
    }
    s.end = e;
    return s;
}

void abi_HealAndClear(void)
{
    abi_sound_end(heal_and_clear_exit_shim(HealAndClear(cpu.x, cpu.y)));
}

/*
 * @routine 09EB:6CD StartOnPlayerAnimation
 * @run 70:B6CD
 * WHAT IT DOES: In state 0 loads the object's record and sets its hitbox offsets to $15 and $14; later it takes the place and facing of slot $10 and steps the animation list that
 * a table at $B707 names for slot $10's state (the list index from the table at $B6FF).
 * HOW: LoadObjectRecord, CopyObjectPlaceBack, table reads (bank 70), a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: state 0: A $14 from the store; else as StepObjectFrames.
 */
ObjArm StartOnPlayerAnimation(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state, y;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        r.arm = OP_RECORD;
        jsr_push(0xB6D5);
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        jsr_pop();
        WR8(OBJ->hitbox_dx[slot], 0x15);
        WR8(OBJ->hitbox_dy[slot], 0x14);
        return r;
    }
    r.arm = OP_STEP;
    jsr_push(0xB6E5);
    r.third = CopyObjectPlaceBack(0x10, slot);
    jsr_pop();
    r.fourth = RD8(OBJ->facing_left[0x10]);
    WR8(OBJ->facing_left[slot], r.fourth);
    r.fifth = RD8(OBJ->state[0x10]);
    r.sixth = rd8((uint16_t)(0xB6FF + r.fifth));
    y = r.sixth;
    WR8(ZP->temp10, rd8((uint16_t)(0xB707 + r.sixth)));
    WR8(ZP->temp11, rd8((uint16_t)(0xB708 + r.sixth)));
    r.step = StepObjectFrames(slot, &y);
    r.y = y;
    return r;
}

static Ended start_on_player_animation_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == OP_RECORD) {
        return then(record_exit_shim(r), ended_load(0x14));
    }
    e = ended_with_y(ended_load(r.third), 0x10);
    e = then(e, ended_load(r.fourth));
    e = then(e, ended_with_y(ended_load(r.sixth), r.fifth));
    e = then(e, ended_with_y(ended_kept(), r.sixth));
    return then(e, ended_of_frame_step(r.step, true, r.y));
}

void abi_StartOnPlayerAnimation(void)
{
    abi_ended(start_on_player_animation_exit_shim(StartOnPlayerAnimation(cpu.x)));
}

/*
 * @routine 09EC:7D8 StartTracker
 * @run 70:BFD8
 * WHAT IT DOES: Loads the object's record, sets its sprite set to 6 and its frame to 2, and notes its Y low byte in OBJ_WORK_G and its next kind in OBJ_WORK_H.
 * HOW: LoadObjectRecord and four stores.
 * WAITS: no.
 * Entry: X object. Exit: A the next kind, N and Z from it, Y 11, C from the record.
 */
ObjArm StartTracker(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t low;

    jsr_push(0xBFDB);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    WR8(OBJ->sprite_set[slot], 6);
    WR8(OBJ->frame[slot], 2);
    low = RD8(OBJ->y_lo[slot]);
    WR8(OBJ->angle_lo[slot], low);
    r.third = RD8(OBJ->start_arg[slot]);
    WR8(OBJ->work_h[slot], r.third);
    return r;
}

static Ended start_tracker_exit_shim(ObjArm r)
{
    return ended_then(record_exit_shim(r), ended_load(r.third));
}

void abi_StartTracker(void)
{
    abi_ended(start_tracker_exit_shim(StartTracker(cpu.x)));
}

/*
 * @routine 09ED:0EA StartThenStepList
 * @run 71:C0EA
 * WHAT IT DOES: In state 0 loads the object's record and sets its sprite set to 6; every frame it steps the animation list at $C112.
 * HOW: LoadObjectRecord, StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: as the step.
 */
ObjArm StartThenStepList(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state, y = 0;

    r.arm = ST_PLAIN;
    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(0xC0F2);
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        jsr_pop();
        WR8(OBJ->sprite_set[slot], 6);
        r.arm = ST_STARTED;
    }
    r.step = step_list(slot, 0xC112, 0xC102, &y, true);
    r.y = y;
    return r;
}

static Ended start_then_step_list_exit_shim(ObjArm r)
{
    Ended e = ended_kept();

    if (r.arm == ST_STARTED) {
        e = ended_then(record_exit_shim(r), ended_load(6));
    }
    return ended_then(e, frame_step_exit_shim(r));
}

void abi_StartThenStepList(void)
{
    abi_ended(start_then_step_list_exit_shim(StartThenStepList(cpu.x)));
}

/* What the move, the angle add and the length subtract left, and for a slot that ran out its compare with $10 (every arm applies it where the original is at that point). */
static Ended final_spin_and_end_chain(ObjArm r)
{
    Ended e = then(ended_with_y(ended_kept(), 0), ended_of_axis_move(r.move));

    e = then(e, of_sum(r.sum));
    e = then(e, of_sum(r.sum2));
    if (r.arm == FS_SPINNING) {
        return e;
    }
    return then(e, cpy(r.third, 0x10));
}

/* The registers the music's wait is entered with. */
static void final_spin_and_end_before_wait_shim(ObjArm r)
{
    abi_ended(final_spin_and_end_chain(r));
}

/* The registers StartSoleScriptSlot left, and then those the pull of the slot left. */
static void final_spin_and_end_sole_shim(SoleSlot sole)
{
    abi_ended(ended_of_sole(sole));
}

static void final_spin_and_end_slot_shim(uint8_t slot)
{
    abi_ended(ended_nz(slot));
}

/* The exit: the registers of the arm the routine ended on. */
static void final_spin_and_end_exit_shim(ObjArm r)
{
    if (r.arm != FS_ENDED) {
        abi_ended(final_spin_and_end_chain(r));
        return;
    }
    abi_ended(ended_with_carry(ended_with_a(ended_with_y(ended_nz(0), 0x20), 1), true));
}

/*
 * @routine 09EB:654 FinalSpinAndEnd
 * @run 70:B654
 * WHAT IT DOES: The last object of the stage's final scene: it is held at X $00B0, Y $0098, its speed follows its angle (OBJ_WORK_F, with the length OBJ_WORK_E) and it moves; the angle turns on by
 * OBJ_WORK_H and the length shrinks by OBJ_WORK_G; when the length runs out, if this is slot $10, the music plays again, script slot 3 starts, sound $A1 plays, the object becomes kind $21 in state 0,
 * the water line and the scroll limit are set ($0000 and $027E) and slots $11 to $1F are cleared.
 * HOW: SetVelocityFromAngle, ApplyVelocity, 8-bit subtract, CdPlayAgain, StartSoleScriptSlot, SoundCommand, ClearObjectInY.
 * WAITS: yes, on CdPlayAgain's drive (the stack there is the caller's return, this call's two bytes and X pushed (2) and zeros).
 * Entry: X object, Y the sound driver's. Exit: see the wrapper.
 */
ObjArm FinalSpinAndEnd(uint8_t slot)
{
    ObjArm r = { 0 };
    TrackPlay track;
    CommandCall call;
    uint8_t angle, length, add, sub, to;
    SoleSlot sole;

    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->x_lo[slot], 0xB0);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0x98);
    WR8(ZP->temp00, 0);
    length = RD8(OBJ->work_e[slot]);
    WR8(ZP->temp01, length);
    angle = RD8(OBJ->angle_hi[slot]);
    jsr_push(0xB671);
    SetVelocityFromAngle(angle, slot);
    jsr_pop();
    jsr_push(0xB674);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    angle = RD8(OBJ->angle_hi[slot]);
    add = RD8(OBJ->work_h[slot]);
    r.sum = add_bytes(angle, add, false);
    WR8(OBJ->angle_hi[slot], r.sum.value);
    length = RD8(OBJ->work_e[slot]);
    sub = RD8(OBJ->angle_lo[slot]);
    r.sum2 = sub_bytes(length, sub, true);
    WR8(OBJ->work_e[slot], r.sum2.value);
    r.arm = FS_SPINNING;
    if (r.sum2.carry) {
        return r;
    }
    WR8(OBJ->work_e[slot], 0);
    r.third = slot;
    r.arm = FS_OTHER_SLOT;
    if (slot != 0x10) {
        return r;
    }
    r.arm = FS_ENDED;
    final_spin_and_end_before_wait_shim(r);
    push8(slot);
    RMW8(ZP->field8_e, old, old & 0xF7);
    RMW8(ZP->field8_e, old, old & 0xEF);
    jsr_push(0xB699);
    track = CdPlayAgain(slot, 0);
    jsr_pop();
    play_track_exit_shim(track);
    jsr_push(0xB69F);
    sole = StartSoleScriptSlot(0, 3);
    jsr_pop();
    final_spin_and_end_sole_shim(sole);
    slot = pull8();
    final_spin_and_end_slot_shim(slot);
    jsr_push(0xB6A5);
    call = SoundCommand(0xA1, slot, 0);
    jsr_pop();
    sound_command_exit_shim(0xA1, call);
    WR8(OBJ->kind[slot], 0x21);
    WR8(OBJ->state[slot], 0);
    wr8(LevelYHi, 0);
    wr8(RasterWaterWanted, 0);
    wr8(ScrollXMaxHi, 2);
    wr8(ScrollXMaxLo, 0x7E);
    WR8(OBJ->frame[slot], 1);
    for (to = 0x11; to < 0x20; to++) {
        jsr_push(0xB6C7);
        ClearObjectInY(to);
        jsr_pop();
    }
    return r;
}

void abi_FinalSpinAndEnd(void)
{
    final_spin_and_end_exit_shim(FinalSpinAndEnd(cpu.x));
}

/*
 * @routine 09EB:752 StepFallingSpark
 * @run 70:B752
 * WHAT IT DOES: Steps the animation list at $B781; when it ends the object is cleared; otherwise it moves, takes $0018 off its Y speed and, from frame $10, turns its hitbox off (the part of the
 * routine at $B725 that comes after its table).
 * HOW: StepObjectFrames, ApplyVelocity, a 16-bit subtract, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: ended: as the step; else the last compare or subtract.
 */
ObjArm StepFallingSpark(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0, low, high;

    r.step = step_list(slot, 0xB781, 0xB75D, &y, true);
    r.y = y;
    r.arm = SP_ENDED;
    if (r.step == FRAME_LIST_ENDED) {
        ClearObject(slot);
        return r;
    }
    jsr_push(0xB765);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    low = RD8(OBJ->vy_lo[slot]);
    r.sum = sub_bytes(low, 0x18, true);
    WR8(OBJ->vy_lo[slot], r.sum.value);
    high = RD8(OBJ->vy_hi[slot]);
    r.sum2 = sub_bytes(high, 0, r.sum.carry);
    WR8(OBJ->vy_hi[slot], r.sum2.value);
    r.first = RD8(OBJ->frame[slot]);
    if (r.first >= 0x10) {
        WR8(OBJ->hitbox_on[slot], 0);
    }
    r.arm = SP_MOVING;
    return r;
}

static Ended step_falling_spark_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == SP_ENDED) {
        return e;
    }
    e = ended_then(e, ended_of_axis_move(r.move));
    e = ended_then(e, ended_of_sum(r.sum));
    e = ended_then(e, ended_of_sum(r.sum2));
    return ended_then(e, ended_compare(r.first, 0x10));
}

void abi_StepFallingSpark(void)
{
    abi_ended(step_falling_spark_exit_shim(StepFallingSpark(cpu.x)));
}

/*
 * @routine 09EB:725 StartFallingSpark09
 * @run 70:B725
 * @stop B74E
 * WHAT IT DOES: In state 0 loads the object's record, sets its hitbox offsets to $E0 and $F8, takes its X speed from a table at $B74E by its facing and a Y speed of 3; in any other state it
 * goes on as StepFallingSpark (the code after the table).
 * HOW: LoadObjectRecord and stores, table reads (bank 70).
 * Untraced: the table (B74E to B751).
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: state 0: A 3, Y the facing doubled; else as StepFallingSpark.
 */
ObjArm StartFallingSpark09(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state, byte;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        r.arm = SK_STEP;
        r.sub = obj_sub(StepFallingSpark(slot));
        return r;
    }
    r.arm = SK_START;
    jsr_push(0xB72D);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    WR8(OBJ->hitbox_dx[slot], 0xE0);
    WR8(OBJ->hitbox_dy[slot], 0xF8);
    r.third = RD8(OBJ->facing_left[slot]);
    r.fourth = (uint8_t)(r.third << 1);
    byte = rd8((uint16_t)(0xB74F + r.fourth));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xB74E + r.fourth));
    WR8(OBJ->vx_lo[slot], byte);
    WR8(OBJ->vy_hi[slot], 3);
    return r;
}

static Ended start_falling_spark09_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == SK_STEP) {
        return step_falling_spark_exit_shim(obj_arm(r.sub));
    }
    e = ended_then(record_exit_shim(r), ended_with_y(ended_with_carry(ended_load(r.fourth), (r.third & 0x80) != 0), r.fourth));
    return ended_then(e, ended_load(3));
}

void abi_StartFallingSpark09(void)
{
    abi_ended(start_falling_spark09_exit_shim(StartFallingSpark09(cpu.x)));
}

/*
 * @routine 09EB:211 ShootMoveAndPickSpeed
 * @run 70:B211
 * WHAT IT DOES: MaybeShootAhead, a move in X (ApplyVelocityX), and a tail jump into PickGroundSpeedInWindow.
 * HOW: Two calls and a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as PickGroundSpeedInWindow.
 */
ObjArm ShootMoveAndPickSpeed(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB214);
    r.sub = obj_sub(MaybeShootAhead(slot));
    jsr_pop();
    jsr_push(0xB217);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    r.sub2 = obj_sub(PickGroundSpeedInWindow(slot));
    return r;
}

static Ended shoot_move_and_pick_speed_exit_shim(ObjArm r)
{
    Ended e = maybe_shoot_ahead_exit_shim(obj_arm(r.sub));

    e = then(e, ended_of_axis_move(r.move));
    return then(e, pick_ground_speed_in_window_exit_shim(obj_arm(r.sub2)));
}

void abi_ShootMoveAndPickSpeed(void)
{
    abi_ended(shoot_move_and_pick_speed_exit_shim(ShootMoveAndPickSpeed(cpu.x)));
}

/*
 * @routine 09EB:539 JumpByStateTableE
 * @run 70:B539
 * WHAT IT DOES: Goes to the code a table at $B54B names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
ObjArm JumpByStateTableE(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t index, low;

    r.third = RD8(OBJ->state[slot]);
    index = (uint8_t)(r.third << 1);
    r.second = index;
    low = rd8((uint16_t)(0xB54B + index));
    WR8(ZP->temp00, low);
    r.first = rd8((uint16_t)(0xB54C + index));
    WR8(ZP->temp01, r.first);
    r.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return r;
}

static Jump jump_by_state_table_e_exit_shim(ObjArm r)
{
    return jump_by_state_table_exit_shim(r);
}

void abi_JumpByStateTableE(void)
{
    Jump jump = jump_by_state_table_e_exit_shim(JumpByStateTableE(cpu.x));

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 09EA:630 JumpByStateTableF
 * @run 70:AE30
 * WHAT IT DOES: Goes to the code a table at $AE42 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
ObjArm JumpByStateTableF(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t index, low;

    r.third = RD8(OBJ->state[slot]);
    index = (uint8_t)(r.third << 1);
    r.second = index;
    low = rd8((uint16_t)(0xAE42 + index));
    WR8(ZP->temp00, low);
    r.first = rd8((uint16_t)(0xAE43 + index));
    WR8(ZP->temp01, r.first);
    r.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return r;
}

static Jump jump_by_state_table_f_exit_shim(ObjArm r)
{
    return jump_by_state_table_exit_shim(r);
}

void abi_JumpByStateTableF(void)
{
    Jump jump = jump_by_state_table_f_exit_shim(JumpByStateTableF(cpu.x));

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 09E9:28D HitRowByKindAndHits
 * @run 70:A28D
 * WHAT IT DOES: Puts the byte of a table at $AB9F picked by 16 times OBJ_2EE0 plus the hits taken (OBJ_HIT_TAKEN) into zTemp00.
 * HOW: Four shifts, an add, a table read (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: A the table byte, Y its index, C and V from the add.
 */
ObjArm HitRowByKindAndHits(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t group;

    group = RD8(OBJ->work_a[slot]);
    r.first = (uint8_t)(group << 4);
    r.flag = (group & 0x10) != 0;
    WR8(ZP->temp00, r.first);
    r.second = RD8(OBJ->hit_taken[slot]);
    r.sum = add_bytes(r.second, RD8(ZP->temp00), false);
    r.third = rd8((uint16_t)(0xAB9F + r.sum.value));
    WR8(ZP->temp00, r.third);
    return r;
}

static Ended hit_row_by_kind_and_hits_exit_shim(ObjArm r)
{
    Ended e = ended_with_carry(ended_nz(r.first), r.flag);

    e = then(e, ended_load(r.second));
    e = then(e, of_sum(r.sum));
    return then(e, ended_with_y(ended_load(r.third), r.sum.value));
}

void abi_HitRowByKindAndHits(void)
{
    abi_ended(hit_row_by_kind_and_hits_exit_shim(HitRowByKindAndHits(cpu.x)));
}

/*
 * @routine 09EB:108 StepFacingAndJump
 * @run 70:B108
 * WHAT IT DOES: Steps the animation list at $B13B, sets the facing flag OBJ_FACING_LEFT from the sign of the X speed's high byte (1 when it is not negative) and goes on to the code a table at $B131 names for OBJ_SUBSTATE.
 * HOW: StepObjectFrames, two rotates through the carry the step leaves, a table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE.
 */
ObjArm StepFacingAndJump(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y, low;

    WR8(ZP->temp11, 0xB1);
    WR8(ZP->temp10, 0x3B);
    jsr_push(0xB113);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.first = RD8(OBJ->vx_hi[slot]);
    r.second = (uint8_t)(((r.first >> 7) ^ 1) & 1);
    WR8(OBJ->facing_left[slot], r.second);
    r.third = RD8(OBJ->substate[slot]);
    r.fourth = (uint8_t)(r.third << 1);
    low = rd8((uint16_t)(0xB131 + r.fourth));
    WR8(ZP->temp00, low);
    r.fifth = rd8((uint16_t)(0xB132 + r.fourth));
    WR8(ZP->temp01, r.fifth);
    r.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return r;
}

/* What the step, the two rotates and the index left before the jump. */
static Ended step_facing_and_jump_before_shim(ObjArm r)
{
    Ended e = ended_of_frame_step(r.step, true, r.y);

    e = then(e, ended_with_carry(ended_nz((uint8_t)(r.second ^ 1)), (r.first & 0x40) != 0));
    e = then(e, ended_nz(r.second));
    return then(e, ended_with_carry(ended_with_y(ended_load(r.fourth), r.fourth), (r.third & 0x80) != 0));
}

static Jump step_facing_and_jump_exit_shim(ObjArm r)
{
    Jump jump = { { 0 }, r.target, true };

    jump.end = ended_load(r.fifth);
    return jump;
}

void abi_StepFacingAndJump(void)
{
    ObjArm r = StepFacingAndJump(cpu.x);
    Jump jump = step_facing_and_jump_exit_shim(r);

    abi_ended(step_facing_and_jump_before_shim(r));
    jump_shim(jump.end, jump.target);
}

/*
 * @routine 09EB:4FF PickWaterOnTenth
 * @run 70:B4FF
 * WHAT IT DOES: Runs the code the state table names (JumpByStateTableE); then, for object $10 in a state of 2 or more, picks the water level (PickWaterLevel), sets PlayerStartPose to 2 and gives
 * object 0 the angle word $0003 (OBJ_WORK_E 3, OBJ_WORK_F 0).
 * HOW: JumpByStateTableE, PickWaterLevel.
 * WAITS: no.
 * Entry: X object. Exit: the compare of X with $10, of the state with 2, or the last store's load.
 */
ObjArm PickWaterOnTenth(uint8_t slot)
{
    ObjArm r = { 0 };
    ObjArm table;

    jsr_push(0xB502);
    table = JumpByStateTableE(slot);
    slot = enter_state_handler(slot, jump_by_state_table_e_exit_shim(table));
    jsr_pop();
    r.first = slot;
    r.arm = PW_NOT_TENTH;
    if (slot != 0x10) {
        return r;
    }
    r.second = RD8(OBJ->state[slot]);
    r.arm = PW_LOW_STATE;
    if (r.second < 2) {
        return r;
    }
    jsr_push(0xB510);
    r.sub = obj_sub(PickWaterLevel());
    jsr_pop();
    wr8(PlayerStartPose, 2);
    WR8(OBJ->work_e[0], 3);
    WR8(OBJ->angle_hi[0], 0);
    r.arm = PW_PICKED;
    return r;
}

static Ended pick_water_on_tenth_exit_shim(ObjArm r)
{
    Ended e = cpy(r.first, 0x10);

    if (r.arm == PW_NOT_TENTH) {
        return e;
    }
    e = then(e, ended_compare(r.second, 2));
    if (r.arm == PW_LOW_STATE) {
        return e;
    }
    e = then(e, pick_water_level_exit_shim(obj_arm(r.sub)));
    return then(e, ended_load(3));
}

void abi_PickWaterOnTenth(void)
{
    abi_ended(pick_water_on_tenth_exit_shim(PickWaterOnTenth(cpu.x)));
}

/*
 * @routine 09EA:588 SwitchStateOnHitRow
 * @run 70:AD88
 * WHAT IT DOES: For a state that a table at $ADB0 gives a row number, looks up the hit row (HitRowByKindAndHits); when that is not 0 the object takes the state a table at $ADC8 gives for the row,
 * plays sound $9F and clears OBJ_SUBSTATE, OBJ_ANIM_STEP and OBJ_ANIM_TIMER.
 * HOW: HitRowByKindAndHits, SoundCommand, table reads (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: see abi_sound_end.
 */
ObjArm SwitchStateOnHitRow(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->state[slot]);
    r.second = rd8((uint16_t)(0xADB0 + r.first));
    r.arm = SW_NO_ROW;
    if (r.second == 0) {
        return r;
    }
    WR8(ZP->temp01, r.second);
    jsr_push(0xAD95);
    r.sub = obj_sub(HitRowByKindAndHits(slot));
    jsr_pop();
    r.third = RD8(ZP->temp00);
    r.arm = SW_NO_HIT;
    if (r.third == 0) {
        return r;
    }
    r.fourth = RD8(ZP->temp01);
    r.fifth = rd8((uint16_t)(0xADC8 + r.fourth));
    WR8(OBJ->state[slot], r.fifth);
    r.arm = SW_SWITCHED;
    r.sounded = true;
    r.command = 0x9F;
    jsr_push(0xADA6);
    r.call = SoundCommand(0x9F, slot, r.fourth);
    jsr_pop();
    WR8(OBJ->substate[slot], 0);
    WR8(OBJ->f3408[slot], 0);
    WR8(OBJ->f3460[slot], 0);
    return r;
}

static SoundEnd switch_state_on_hit_row_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = ended_with_y(ended_nz(r.first), r.first);

    e = then(e, ended_load(r.second));
    s.end = e;
    if (r.arm == SW_NO_ROW) {
        return s;
    }
    e = then(e, hit_row_by_kind_and_hits_exit_shim(obj_arm(r.sub)));
    e = then(e, ended_load(r.third));
    s.end = e;
    if (r.arm == SW_NO_HIT) {
        return s;
    }
    e = then(e, ended_with_y(ended_nz(r.fourth), r.fourth));
    e = then(e, ended_load(r.fifth));
    e = then(e, ended_load(0x9F));
    s.before = e;
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.end = ended_kept();
    return s;
}

void abi_SwitchStateOnHitRow(void)
{
    abi_sound_end(switch_state_on_hit_row_exit_shim(SwitchStateOnHitRow(cpu.x)));
}

/* What StepWithHitTable's instructions have left once the jump's flags are in: both ways round the sound. */
static Ended step_with_hit_table_chain(ObjArm r)
{
    Ended e = then(ended_nz(r.first), face_by_cue_exit_shim(obj_arm(r.sub)));
    SoundEnd hit = switch_state_on_hit_row_exit_shim(obj_arm(r.sub2));

    if (hit.sounded) {
        e = hit.end;
    } else {
        e = then(e, hit.end);
    }
    e = then(e, advance_by_health_exit_shim(obj_arm(r.sub3)));
    e = then(e, ended_with_overflow(ended_with_carry(ended_with_y(ended_load(r.fifth), r.sixth), r.flag), r.other));
    return then(e, jump_by_state_table_f_exit_shim(obj_arm(r.sub4)).end);
}

/* The sound's call is shown with the registers the flags before it left: A, N, Z and C first, then the call's own. */
static void step_with_hit_table_sound_shim(ObjArm r)
{
    Ended e = then(ended_nz(r.first), face_by_cue_exit_shim(obj_arm(r.sub)));
    SoundEnd hit = switch_state_on_hit_row_exit_shim(obj_arm(r.sub2));

    abi_ended(then(e, hit.before));
    sound_command_exit_shim(hit.command, hit.call);
}

static void step_with_hit_table_enter_shim(ObjArm r)
{
    abi_ended(step_with_hit_table_chain(r));
}

/*
 * @routine 09EA:547 StepWithHitTable
 * @run 70:AD47
 * WHAT IT DOES: One frame: OBJ_WORK_G goes up, then FaceByCue, SwitchStateOnHitRow, AdvanceByHealth, SetHitboxFromFrameFive, and the code the state table at $AE42 names (JumpByStateTableF).
 * HOW: INC and five calls.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to.
 */
ObjArm StepWithHitTable(uint8_t slot)
{
    ObjArm r = { 0 };
    HitboxRow row;

    r.first = INC8(OBJ->angle_lo[slot]);
    jsr_push(0xAD4D);
    r.sub = obj_sub(FaceByCue(slot));
    jsr_pop();
    jsr_push(0xAD50);
    r.sub2 = obj_sub(SwitchStateOnHitRow(slot));
    jsr_pop();
    if (r.sub2.sounded) {
        step_with_hit_table_sound_shim(r);
    }
    jsr_push(0xAD53);
    r.sub3 = obj_sub(AdvanceByHealth(slot));
    jsr_pop();
    jsr_push(0xAD56);
    row = SetHitboxFromFrameFive(slot);
    jsr_pop();
    r.fifth = row.last;
    r.sixth = row.offset;
    r.flag = row.carry;
    r.other = row.overflow;
    jsr_push(0xAD59);
    r.sub4 = obj_sub(JumpByStateTableF(slot));
    step_with_hit_table_enter_shim(r);
    slot = enter_state_handler(slot, jump_by_state_table_f_exit_shim(obj_arm(r.sub4)));
    jsr_pop();
    return r;
}

/* The flags were handed to the handler; what the typed code leaves is the machine as it came back. */
static SoundEnd step_with_hit_table_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };

    (void)r;
    s.end = ended_kept();
    return s;
}

void abi_StepWithHitTable(void)
{
    abi_sound_end(step_with_hit_table_exit_shim(StepWithHitTable(cpu.x)));
}
