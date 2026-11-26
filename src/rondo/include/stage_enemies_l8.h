/* Lot L8: the stage object routines of the overlays at sectors 0259 to 18DD, as typed C. */
#ifndef RONDO_STAGE_ENEMIES_L8_H
#define RONDO_STAGE_ENEMIES_L8_H

#include "machine.h"
#include "enemy_objects.h"
#include "common_objects.h"
#include "stage_enemies_l7.h"

/* The exit of the angle scales (scale_exit_shim): A, N and Z, C, V when an add ran, and Y 0. */
static inline Ended l8_of_scale(ScaleResult r)
{
    Ended e = ended_with_y(ended_with_carry(ended_with_a(ended_nz(r.nz), r.a), r.carry), 0);

    return r.v_set ? ended_with_overflow(e, r.overflow) : e;
}

/* AngleToPlayer, AngleToPoint: A, N, Z, C, V and Y. */
static inline Ended l8_of_angle(AngleExit r)
{
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_load(r.a), r.c), r.v), r.y);
}

/* ApplyGravity and ApplyGravityUnlimited as gravity_exit_shim leaves the registers. */
static inline Ended l8_of_gravity(GravityResult g)
{
    if (g.gained) {
        return ended_with_overflow(ended_with_carry(ended_with_a(ended_nz(g.carried ? g.high : g.low), g.low), g.carried), g.overflow);
    }
    return ended_with_carry(ended_with_a(ended_nz((uint8_t)(g.high - 7)), g.high), true);
}

/* A script slot start: A and Y the script, N and Z from it. */
static inline Ended l8_of_script(uint8_t script)
{
    return ended_with_y(ended_load(script), script);
}

/* CPY: N, Z and C from the compare; A kept. */
static inline Ended l8_compare_y(uint8_t y, uint8_t operand)
{
    return ended_with_carry(ended_nz((uint8_t)(y - operand)), y >= operand);
}

/* A sound routine's exit with no sound: only what its own instructions left. */
static inline SoundEnd l8_sound_none(Ended e)
{
    SoundEnd r = { ended_kept(), e, false, 0, { 0 } };

    return r;
}

/* The usual first three instructions of an object routine: LDA state, BNE, JSR LoadObjectRecord (ret the address after the JSR). */
static inline Ended l8_load_if_new(uint8_t slot, uint16_t ret, uint8_t *state_out)
{
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t last, nz;
    bool carry;
    Ended e = ended_load(state);

    *state_out = state;
    if (state == 0) {
        jsr_push(ret);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        e = ended_then(e, ended_of_record(last, nz, carry));
    }
    return e;
}

/* The usual list step: the list address stored in zTemp10-11 (the high byte first when high_first), StepObjectFrames, and what it leaves (y kept unless the step set it). */
static inline Ended l8_step_list(uint8_t slot, uint16_t list, uint16_t ret, bool high_first, uint8_t *y, FrameStep *step)
{
    uint8_t lo = (uint8_t)list, hi = (uint8_t)(list >> 8);
    Ended e;

    if (high_first) {
        WR8(ZP->temp11, hi);
        WR8(ZP->temp10, lo);
        e = ended_load(lo);
    } else {
        WR8(ZP->temp10, lo);
        WR8(ZP->temp11, hi);
        e = ended_load(hi);
    }
    jsr_push(ret);
    *step = StepObjectFrames(slot, y);
    jsr_pop();
    return ended_then(e, ended_of_frame_step(*step, *step != FRAME_HELD, *y));
}

/* PHX, LDA #list, LDX #channel, JSR StartListChannelKind2, PLX: A 2, N and Z from X. */
static inline Ended l8_list_channel2(uint8_t slot, uint8_t list, uint8_t channel, uint16_t ret)
{
    push8(slot);
    jsr_push(ret);
    StartListChannelKind2(list, channel);
    jsr_pop();
    slot = pull8();
    return ended_with_a(ended_nz(slot), 2);
}

/* PHX, LDA #script, LDX #slot, JSR StartScriptSlotKind1, PLX: A and Y the script, N and Z from X. */
static inline Ended l8_script_kind1(uint8_t slot, uint8_t script, uint8_t target, uint16_t ret)
{
    push8(slot);
    jsr_push(ret);
    StartScriptSlotKind1(script, target);
    jsr_pop();
    slot = pull8();
    return ended_with_y(ended_with_a(ended_nz(slot), script), script);
}

/* RecordObjectEvent's exit: unmarked, A 0 and Z; else A the byte stored, Y its index, C whether the event is a multiple of 8. */
static inline Ended l8_of_event(bool marked, uint8_t stored, uint8_t event, uint8_t byte)
{
    if (!marked) {
        return ended_load(0);
    }
    return ended_with_carry(ended_with_y(ended_load(stored), byte), (event & 7) == 0);
}

/*
 * The converted form (the pattern of ending.c, 441de03): a routine's body returns the arm it ended on and the values behind it,
 * read or made on that path, and what its typed callees returned. Its exit shim alone builds A, Y and the flags from them; the
 * abi_ wrapper sets the registers from the shim. No body builds an exit.
 */

/* LDA state, BNE, JSR LoadObjectRecord: the state read, and the record's result when state 0 loaded it. */
typedef struct {
    uint8_t state;
    bool loaded;
    uint8_t last, nz;
    bool carry;
} L8Load;

/* The list address stored in zTemp10-11 and StepObjectFrames: the byte stored last, the step, and the Y it left. */
typedef struct {
    uint8_t stored;
    FrameStep step;
    uint8_t y;
} L8Step;

/* LoadObjectRecord's result. */
typedef struct {
    uint8_t last, nz;
    bool carry;
} L8Record;

/* FindFreeSlotFrom20, 30 and 50: the slot, the last type read and whether one was free. */
typedef struct {
    uint8_t slot, last;
    bool found;
} L8Slot;

/* What a converted body ended on: the arm and the values behind it (each routine says which it fills). */
typedef struct {
    int arm;
    uint8_t first, second, third, y;
    bool flag;
    L8Load load;
    L8Record record;
    L8Step step;
    L8Slot found;
    AxisMove move;
    RandomResult random;
    ScaleResult scale;
    Sum sum;
} L8Arm;

static inline L8Load l8_load_new(uint8_t slot, uint16_t ret)
{
    L8Load r = { 0 };

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        jsr_push(ret);
        r.last = LoadObjectRecord(slot, &r.nz, &r.carry);
        jsr_pop();
        r.loaded = true;
    }
    return r;
}

static inline L8Record l8_record(uint8_t slot)
{
    L8Record r;

    r.last = LoadObjectRecord(slot, &r.nz, &r.carry);
    return r;
}

static inline L8Step l8_list_step(uint8_t slot, uint16_t list, uint16_t ret, bool high_first, uint8_t y)
{
    uint8_t lo = (uint8_t)list, hi = (uint8_t)(list >> 8);
    L8Step r;

    if (high_first) {
        WR8(ZP->temp11, hi);
        WR8(ZP->temp10, lo);
        r.stored = lo;
    } else {
        WR8(ZP->temp10, lo);
        WR8(ZP->temp11, hi);
        r.stored = hi;
    }
    r.y = y;
    jsr_push(ret);
    r.step = StepObjectFrames(slot, &r.y);
    jsr_pop();
    return r;
}

static inline L8Slot l8_free_slot30(void)
{
    L8Slot r;

    r.slot = FindFreeSlotFrom30(&r.last, &r.found);
    return r;
}

/* PHX, LDA #list, LDX #channel, JSR StartListChannelKind2, PLX: the X pulled. */
static inline uint8_t l8_channel2(uint8_t slot, uint8_t list, uint8_t channel, uint16_t ret)
{
    push8(slot);
    jsr_push(ret);
    StartListChannelKind2(list, channel);
    jsr_pop();
    return pull8();
}

/* PHX, LDA #script, LDX #slot, JSR StartScriptSlotKind1, PLX: the X pulled. */
static inline uint8_t l8_script1(uint8_t slot, uint8_t script, uint8_t target, uint16_t ret)
{
    push8(slot);
    jsr_push(ret);
    StartScriptSlotKind1(script, target);
    jsr_pop();
    return pull8();
}

/* For the exit shims: what each of the above leaves. */
static inline Ended l8_load_ended(L8Load r)
{
    Ended e = ended_load(r.state);

    return r.loaded ? ended_then(e, ended_of_record(r.last, r.nz, r.carry)) : e;
}

static inline Ended l8_record_ended(L8Record r)
{
    return ended_of_record(r.last, r.nz, r.carry);
}

static inline Ended l8_step_ended(L8Step r)
{
    return ended_then(ended_load(r.stored), ended_of_frame_step(r.step, r.step != FRAME_HELD, r.y));
}

static inline Ended l8_slot_ended(L8Slot r)
{
    return ended_of_find_slot(r.slot, r.last, r.found);
}

static inline Ended l8_channel2_ended(uint8_t x)
{
    return ended_with_a(ended_nz(x), 2);
}

static inline Ended l8_script1_ended(uint8_t x, uint8_t script)
{
    return ended_with_y(ended_with_a(ended_nz(x), script), script);
}

/* object/enemies_l8_st11.c */
Ended LoadRecordOrCapPlayerX(uint8_t slot);
Ended StartAtX120OrApplyVelocity(uint8_t slot);
Ended SetBossFixedFields(void);
Ended CountdownThenNextState(uint8_t slot);
Ended SetTimerThenNextState(uint8_t slot, uint8_t timer);
Ended SpawnPiece21Mask1F(uint8_t slot);
Ended SpawnPiece21UnlessMasked(uint8_t slot, uint8_t mask);
Ended CountdownThenSpawnPieces(uint8_t slot);
Ended HoldScrollStepThenNextState(uint8_t slot);

/* object/enemies_l8_st12.c */
/* The structs the converted routines of st12 return where L8Arm does not fit: the arm and the values behind it. */
typedef struct {
    int arm;
    L8Load load;
    uint8_t moved, kind, timer, state, counted, lo;
    Sum sum;
    L8Step step;
} L8St12KindListArm;

typedef struct {
    int arm;
    L8Load load;
    uint8_t moved, other, timer, scroll, counted, flags, x_lo, x_hi, x;
    bool sounded;
    CommandCall call;
    L8Step step;
} L8St12PhaseArm;

typedef struct {
    int arm;
    L8Load load;
    uint8_t state, kind, timer, counted, x_lo, x_hi, x_step, y_lo, y_hi, y_step;
    AxisMove move;
    RandomResult x_random, y_random;
    Sum x_sum, y_sum;
} L8St12DriftArm;

typedef struct {
    int arm;
    uint8_t state, angle, high, mid;
    ScaleResult scale;
    AxisMove move;
    Sum sum;
} L8St12SwingArm;

typedef struct {
    int arm;
    uint8_t flag, timer, script, x;
    RandomResult random, random2;
} L8St12ScriptArm;

typedef struct {
    int arm;
    uint8_t hits, timer, counted, wave, placed;
    bool sounded;
    CommandCall call;
    L8Slot found;
} L8St12HitArm;

typedef struct {
    int arm;
    uint8_t timer, counted, now;
    CommandCall call;
} L8St12Sound77Arm;

typedef struct {
    int arm;
    uint8_t first, timer, mark, at, next, script, x, counted;
} L8St12MarksArm;

typedef struct {
    int arm;
    L8Step step;
    uint8_t counted, frame, placed;
    CommandCall call;
} L8St12BurstArm;

typedef struct {
    int arm;
    L8Step step;
    AngleExit toward;
    uint8_t angle, x_hi, y_hi;
    ScaleResult scale;
    AxisMove move;
    Sum x_near, y_near;
    CommandCall call;
} L8St12HomeArm;

L8Arm StepVerticalScrollTable(uint8_t slot);
Ended l8_vertical_scroll_table_exit_shim(L8Arm r);
L8Arm CountdownThenStartScript80(uint8_t slot);
L8Arm LoadRecordPlaceByKind(uint8_t slot);
L8Arm LoadRecordAtPlayerPlace(uint8_t slot);
L8Arm LoadRecordThenSinkByF3520(uint8_t slot);
L8St12KindListArm LoadRecordThenStepListByKind(uint8_t slot, uint8_t y_entry);
L8Arm LoadRecordSinkThenStepListByKind(uint8_t slot, uint8_t y_entry);
L8St12PhaseArm StateMachineWithSoundAndList(uint8_t slot, uint8_t y_entry);
L8St12DriftArm DriftRecordThenMoveWithJitter(uint8_t slot);
L8St12SwingArm SwingByCosineFromState0B(uint8_t slot);
Ended l8_swing_by_cosine_exit_shim(L8St12SwingArm r);
L8St12ScriptArm StartRandomScriptWhenFlagged(uint8_t slot);
Ended l8_random_script_exit_shim(L8St12ScriptArm r);
L8Arm PushPlayerWhenFlagged(uint8_t slot);
Ended l8_push_player_exit_shim(L8Arm r);
L8St12HitArm HitSoundThenSpawnPieces(uint8_t slot, uint8_t y_entry);
SoundEnd l8_hit_sound_spawn_exit_shim(L8St12HitArm r);
L8Arm EnterState0BWhenFlagged(uint8_t slot);
Ended l8_enter_state0b_exit_shim(L8Arm r);
L8Arm StartPieceAtTopLeft(uint8_t slot);
L8Arm CountDownThenStartScripts(uint8_t slot);
L8St12Sound77Arm CountDownWithSound77(uint8_t slot, uint8_t y_entry);
L8St12MarksArm StartScriptAtTimerMarks(uint8_t slot);
Ended l8_timer_marks_exit_shim(L8St12MarksArm r);
L8Arm SpinOutThenLeave(uint8_t slot, uint8_t y_entry);
L8Arm SpinOnCircle(uint8_t slot);
Ended l8_spin_on_circle_exit_shim(L8Arm r);
L8St12BurstArm StepListThenBurstAtFrame18(uint8_t slot, uint8_t y_entry);
L8St12HomeArm HomeOnPointThenRingAndSound(uint8_t slot, uint8_t y_entry);

/* object/enemies_l8_st13.c */
Ended StepListThenStartChannel85(uint8_t slot, uint8_t y_entry);
Ended StepListThenStopAnimation(uint8_t slot, uint8_t y_entry);
Ended StepListB995ThenWait(uint8_t slot, uint8_t y_entry);
Ended StepListThenStartType2D(uint8_t slot, uint8_t y_entry);
Ended StepListThenClearAnimation(uint8_t slot, uint8_t y_entry);
Ended StepListThenStartType2E(uint8_t slot, uint8_t y_entry);
SoundEnd StepListThenSoundAndStartChannel86(uint8_t slot, uint8_t y_entry);
Ended SpiralOutWhileStepping(uint8_t slot, uint8_t y_entry);
Ended MoveXUntilPastLeftEdge(uint8_t slot);
Ended CountDownThenClearAndAdvanceSlot10(uint8_t slot);
SoundEnd SoundWhenHurtInStatesOneToThree(uint8_t slot, uint8_t y_entry);
Ended LoadRecordAimAtPlayer(uint8_t slot);
Ended NearObject19(uint8_t slot);
Ended StartRiseAndSpawnRing(uint8_t slot);
Ended SpawnRingOfSeven(uint8_t slot);
Ended FallAlongListUntilGround(uint8_t slot, uint8_t y_entry);
Ended StepListCCThenMoveOrClear(uint8_t slot, uint8_t y_entry);
Ended LoadRecordThrowByNumber(uint8_t slot);
Ended FallThenTurnAtTimerEnd(uint8_t slot);
Ended StepListCBThenMoveOrClear(uint8_t slot, uint8_t y_entry);
Ended FollowObject19ThenStepListByMode(uint8_t slot, uint8_t y_entry);
Ended StartScriptByWaveBit(uint8_t slot);
SoundEnd LoadRecordWithSound1C(uint8_t slot);
Ended FollowObject19ThenLaunch(uint8_t slot);
Ended SpawnType30EveryFourthFrame(uint8_t slot);
Ended StepListThenSpawnUntilHurt(uint8_t slot, uint8_t y_entry);
SoundEnd SpawnType2FAtWaveMarks(uint8_t slot, uint8_t y_entry);
SoundEnd StepListSlowSideways(uint8_t slot, uint8_t y_entry);
Ended StepListThenCountdownNextState(uint8_t slot, uint8_t y_entry);
SoundEnd StepListThenSound43AtZero(uint8_t slot, uint8_t y_entry);
Ended SeekObject19ThenHitAndClear(uint8_t slot, uint8_t y_entry);

/* object/enemies_l8_hit_damage.c */
/* A HitDamage variant's values behind its exit: the row times sixteen, the add of the hit, the damage read. */
typedef struct {
    uint8_t row;
    Sum index;
    uint8_t damage;
} L8HitDamage;

L8HitDamage HitDamage0519(uint8_t slot);
L8HitDamage HitDamage05C9(uint8_t slot);
L8HitDamage HitDamage0729(uint8_t slot);
L8HitDamage HitDamage07D9(uint8_t slot);
L8HitDamage HitDamage0889(uint8_t slot);
L8HitDamage HitDamage0939(uint8_t slot);
L8HitDamage HitDamage0A99(uint8_t slot);
L8HitDamage HitDamage0B49(uint8_t slot);
L8HitDamage HitDamage0BF9(uint8_t slot);
L8HitDamage HitDamage0CA9(uint8_t slot);
L8HitDamage HitDamage0D59(uint8_t slot);
L8HitDamage HitDamage0E09(uint8_t slot);
L8HitDamage HitDamage0EB9(uint8_t slot);
L8HitDamage HitDamage1019(uint8_t slot);
L8HitDamage HitDamage10C9(uint8_t slot);
L8HitDamage HitDamage1179(uint8_t slot);
L8HitDamage HitDamage16C9(uint8_t slot);

/* object/enemies_l8_st14.c */
L8Arm LoadRecordFlashByNumberThenMoveOrClear(uint8_t slot, uint8_t y_entry);
L8Arm LoadRecordRandomLiftThenMoveOrClear(uint8_t slot, uint8_t y_entry);

/* object/enemies_l8_st15.c */
Ended LoadRecordThenRecordEventAtFifth(uint8_t slot, uint8_t y_entry);
Ended FlagThenHitsCompare(uint8_t slot);
Ended EnterState5WhenFlagged(uint8_t slot);
Ended LoadRecordStartTwoPieces(uint8_t slot);
Ended SetUpEntryFromLeft(uint8_t slot);
Ended StepListMoveXThenTurnUp(uint8_t slot, uint8_t y_entry);
Ended WaitForScreenByte2(uint8_t slot);
SoundEnd CountDownThenSound84AndAppear(uint8_t slot, uint8_t y_entry);
Ended StepListFallAndSwing(uint8_t slot, uint8_t y_entry);
Ended CountDownThenLeaveRight(uint8_t slot);
Ended InsideCenterBand(uint8_t slot);
Ended FacingByX(uint8_t slot);
Ended StepListThenTurnAround(uint8_t slot, uint8_t y_entry);
Ended StepListThenCountDownToState4(uint8_t slot, uint8_t y_entry);
Ended PickNextPhaseFromTable(uint8_t slot);
SoundEnd StepListThenLeapWithSound(uint8_t slot, uint8_t y_entry);
Ended FlyAndSagWithTwoLists(uint8_t slot);
SoundEnd SoundEvery32Frames(uint8_t slot, uint8_t y_entry);
Ended PickRandomThenState3(uint8_t slot);
Ended StepListSlowDownThenPickRandom(uint8_t slot, uint8_t y_entry);
Ended EdgeCodeByDirection(uint8_t slot);
Ended TurnTowardCenterOrWait(uint8_t slot);
Ended StepSideToSideWithCenterCheck(uint8_t slot, uint8_t y_entry);
SoundEnd StepListThenSoundAndSpawn(uint8_t slot, uint8_t y_entry);
SoundEnd StepListThenBurstAtFrame14(uint8_t slot, uint8_t y_entry);

/* object/enemies_l8_st16.c */
Ended LoadRecordLaunchByFacing(uint8_t slot);
Ended StepListBounceOffFloorAtRandom(uint8_t slot, uint8_t y_entry);
Ended StepListFallThenLandOrClear(uint8_t slot, uint8_t y_entry);
Ended StepListThenClearWhenEnded(uint8_t slot, uint8_t y_entry);
Jump CountFramesThenRunByState(uint8_t slot);
Ended LoadRecordLaunchSideways(uint8_t slot);
Ended StepListFallThenTurnToPlayer(uint8_t slot, uint8_t y_entry);
SoundEnd SoundAndSpawnEveryFourthFrame(uint8_t slot, uint8_t y_entry);
SoundEnd StepListSpawnAndMoveXOrClear(uint8_t slot, uint8_t y_entry);
Ended ClearIfAboveOrFall(uint8_t slot);
Ended LoadRecordRandomFlingOrFall(uint8_t slot);
SoundEnd LoadRecordThenClearEnemiesAtScreen2(uint8_t slot, uint8_t y_entry);
Ended LoadRecordThenFollowOrHomeIn(uint8_t slot);

/* object/enemies_l8_st17.c */
Ended StartRingOfPieces(uint8_t slot);
Ended LoadRecordFlingThenFall(uint8_t slot);
Ended LoadRecordFacingLeftRising(uint8_t slot);
SoundEnd SoundBAAtScreen2(uint8_t slot, uint8_t y_entry);
Ended LoadRecordThenDrawTileScript2(uint8_t slot);
Ended ScatterPiecesDownward(uint8_t slot);
SoundEnd ClearWhenHurtThenWait(uint8_t slot, uint8_t y_entry);
SoundEnd DrawTileScriptStepsThenScatter(uint8_t slot, uint8_t y_entry);

/* object/enemies_l8_st18.c */
Ended CountDownOrPickNextTimer(uint8_t slot);
Ended StepListThenPickStateByLuck(uint8_t slot, uint8_t y_entry);
SoundEnd StepListThenLeapByDistance(uint8_t slot, uint8_t y_entry);
Ended CountDownThenState5(uint8_t slot);
SoundEnd SoundAndTileWriteAtFrame24(uint8_t slot, uint8_t y_entry);
SoundEnd SoundAndSpawnFiveAtFrame27(uint8_t slot, uint8_t y_entry);
Ended StepListThenState5(uint8_t slot, uint8_t y_entry);
SoundEnd SoundThenLeapByFacing(uint8_t slot, uint8_t y_entry);
SoundEnd MaybeSound19AndSpawnPiece(uint8_t slot, uint8_t number, uint8_t y_entry);

/* object/enemies_l8_st19.c */
Ended LoadRecordInStateZero(uint8_t slot);
Ended FaceThePlayerByStateTable(uint8_t slot);
Ended StateTableHurtFlag(uint8_t slot);
SoundEnd EnterState0FWhenFlagged(uint8_t slot, uint8_t y_entry);
Ended HitBoxByFrameTable(uint8_t slot);
Ended LoadRecordStartPieceAtPlace(uint8_t slot);
Ended StartScriptsFromTable(uint8_t slot);
ExitChain FallToGroundAt98(uint8_t slot, uint8_t y_entry, bool carry_in);
ExitChain FallThenSoundAtGround(uint8_t slot, uint8_t y_entry, bool carry_in);
ExitChain MoveXAndProbeWallBounce(uint8_t slot, uint8_t y_entry);
ExitChain StepListThenWallBounce(uint8_t slot, uint8_t y_entry);
ExitChain CountDownThenHopTowardPlayer(uint8_t slot, uint8_t y_entry);
ExitChain CountDownThenHopAwayFromPlayer(uint8_t slot, uint8_t y_entry);
ExitChain StepListFallThenSoundAtGround(uint8_t slot, uint8_t y_entry);
ExitChain StepListThenTileWriteAndSpawn(uint8_t slot, uint8_t y_entry);
ExitChain WaitThenStepListWithSoundAndBounce(uint8_t slot, uint8_t y_entry);
ExitChain StepListFallThenCountLandings(uint8_t slot, uint8_t y_entry);
Ended LoadRecordLeapByNumberThenFall(uint8_t slot);
Ended FollowObject10HitBoxByFrame(uint8_t slot);

/* object/enemies_l8_st20.c */
/* RecordObjectEvent's result. */
typedef struct {
    bool marked;
    uint8_t stored, event, byte;
} L8Event;

typedef struct {
    int arm;
    uint8_t state, kind, value, counted;
    L8Event event;
} EventArm;

typedef struct {
    int arm;
    uint8_t bits, state, marked, facing;
    SideResult side;
} FaceArm;

typedef struct {
    int arm;
    uint8_t hits, bits, level, row, threshold, value, item, item2, placed;
    bool bit1;
    RandomResult random;
    Sum sum, joined;
    L8Slot found;
} DropArm;

typedef struct {
    int arm;
    uint8_t facing, counted;
    L8Step step;
    SideResult side;
    PlayerDistance distance;
} NearArm;

typedef struct {
    uint8_t facing, index, value, counted;
    SideResult side;
} PhaseArm;

typedef struct {
    int arm;
    uint8_t facing, counted;
    bool negated_used;
    Sum negated;
    PlayerDistance distance;
    CommandCall call;
} LeapArm;

/* The launch HopBetweenSidesWithBox's states 0 and 2 share: the facing read and, facing left, NegateVelocityX's sum. */
typedef struct {
    uint8_t facing;
    Sum negated;
} HopLaunch;

typedef struct {
    int arm;
    L8Arm box;
    uint8_t state, y, facing, sign, counted, timer;
    L8Record record;
    RandomResult random, random2;
    Sum sum;
    HopLaunch launch;
    L8Step step;
    AxisMove move;
} HopArm;

/* A scan of the slot kinds from a first slot: the slot it stopped at, the last kind (or joined kinds) read, and whether it stepped. */
typedef struct {
    uint8_t y, kind;
    bool stepped;
} L8Scan;

/*
 * For the exit shims: what a scan of the slot kinds left. Each slot read is a load with Y the slot, each step a CPY of the next slot against the
 * limit; only the last load and the last compare stand. full: it ran past the limit (y is the slot past it, kind the last read).
 */
static inline Ended l8_scan_ended(Ended e, L8Scan s, uint8_t step, uint8_t limit, bool full)
{
    if (full) {
        e = ended_then(e, ended_with_y(ended_load(s.kind), (uint8_t)(s.y - step)));
        return ended_then(e, ended_with_y(l8_compare_y(s.y, limit), s.y));
    }
    if (s.stepped) {
        e = ended_then(e, ended_with_y(l8_compare_y(s.y, limit), s.y));
    }
    return ended_then(e, ended_with_y(ended_load(s.kind), s.y));
}

typedef struct {
    int arm;
    uint8_t state, frame, timer, counted, kind, screen, aim, placed, y_end, left;
    L8Record record;
    L8Scan scan;
    L8Slot found;
    RandomResult random;
    Sum sum;
} SpawnerArm;

EventArm RecordEventWhenSlot10InState0F(uint8_t slot);
FaceArm FaceThePlayerByStateTableB(uint8_t slot);
DropArm MaybeDropItemByPlayerDistance(uint8_t slot);
Ended l8_drop_item_exit_shim(DropArm r);  /* also the exit of 1019:2C5 */
L8Arm HitBoxByFrameTableCE8F(uint8_t slot);
NearArm StepListFacePlayerUntilNear(uint8_t slot, uint8_t y_entry);
PhaseArm FacePlayerThenPickPhase(uint8_t slot);
LeapArm LeapByPlayerDistance(uint8_t slot, uint8_t y_entry);
uint8_t CountDownThenClearPhase(uint8_t slot);

/* object/enemies_l8_st21.c */
typedef struct {
    L8Record record;
    L8Event event;
    uint8_t value, y;
} SplitArm;

typedef struct {
    int arm;
    uint8_t state, value, sides, kind;
    L8Record record;
    Sum diff;
    AxisMove move;
} FollowArm;

typedef struct {
    int arm;
    uint8_t first, second, value;
    CommandCall call;
} HurtSoundArm;

typedef struct {
    int arm;
    uint8_t state, hits, placed, counted[3];
    L8Record record;
    L8Event event;
    CommandCall call;
    Sum sum;
    L8Slot found;
    L8Arm piece[4];
} BreakArm;

SplitArm LoadRecordSplitIntoFourAndClear(uint8_t slot);
FollowArm LoadRecordThenFollowScroll(uint8_t slot);
L8Arm StartPieceWithSpeedFromTable(uint8_t slot);
FollowArm LoadRecordFlipByPlayerThenFall(uint8_t slot);
L8Arm LoadRecordThenClearAtSubStep3(uint8_t slot);
HurtSoundArm SoundWhenHurtThenFlag(uint8_t slot, uint8_t y_entry);
SoundEnd l8_hurt_then_flag_exit_shim(HurtSoundArm r);
HurtSoundArm SoundAndState6WhenFlagged(uint8_t slot, uint8_t y_entry);
SoundEnd l8_state6_when_flagged_exit_shim(HurtSoundArm r);
L8Record LoadRecordFrame0D(uint8_t slot);
L8Arm ClearFourAtScreen2(uint8_t slot);
L8Arm MoveUpUntilY40(uint8_t slot);
L8Arm CountDownThenCopyPhase(uint8_t slot);
uint8_t ResetPieceAndAdvance(uint8_t slot);
BreakArm BreakIntoPiecesAtFifthHit(uint8_t slot, uint8_t y_entry);

/* object/enemies_l8_st22.c */
Ended OrbitCosineSineThenSettle(uint8_t slot);
Ended ResetOrbitAndAdvance(uint8_t slot);
Ended OrbitWithRisingSineThenSettle(uint8_t slot);
Ended StepListByNumberTable(uint8_t slot, uint8_t y_entry);
Ended ClearAnimationThenStepListByNumber(uint8_t slot, uint8_t y_entry);
Ended StartPieceABInFreeSlot(uint8_t slot);
SoundEnd LoadRecordFrameByKindThenFall(uint8_t slot, uint8_t y_entry);
Ended StepListThenStartFivePieces(uint8_t slot, uint8_t y_entry);
Jump StepListThenRunPhaseTable(uint8_t slot, uint8_t y_entry);

/* object/enemies_l8_st23.c */
SoundEnd ThrownPieceThenScatterAndLeave(uint8_t slot, uint8_t y_entry);

/* object/enemies_l8_st24.c */
Ended StartChannel81OnceThenStepScrollTable(uint8_t slot);
Ended NextStateThenStepScrollTable(uint8_t slot);
SoundEnd StepListMarksThenSound74AtC8(uint8_t slot, uint8_t y_entry);
Ended SpinThenCountDownToState(uint8_t slot, uint8_t y_entry);
Ended SpinThenCountDownToPhase80(uint8_t slot, uint8_t y_entry);
Ended SpinThenCountPhaseDown(uint8_t slot, uint8_t y_entry);
Ended SpinThenCountWordDown(uint8_t slot, uint8_t y_entry);
ExitChain CountDownThenDrawAndSpawnRing(uint8_t slot, uint8_t y_entry);

/* object/enemies_l8_st25.c */
Ended StepListByNumberTableC585(uint8_t slot, uint8_t y_entry);
Ended ClearAnimationThenStepListC585(uint8_t slot, uint8_t y_entry);
Ended ToggleFacing(uint8_t slot);
SoundEnd LoadRecordOrBurstWhenFlagged(uint8_t slot, uint8_t y_entry);
Ended SpawnFlashingPieceWhenRising(uint8_t slot);
Ended CountDownThenClearOnTimer(uint8_t slot);
SoundEnd FlashWhenHurtThenSound(uint8_t slot, uint8_t y_entry);
Ended EnterState0EWhenFlagged(uint8_t slot);
Ended LoadRecordFrame24(uint8_t slot);
Ended StartTimerWhenSlot10Ready(uint8_t slot);
Ended HurtOnceThenAnswerYes(uint8_t slot, uint8_t y_entry);
Ended StepListThenStartPieceAndWalk(uint8_t slot, uint8_t y_entry);
Ended StartLeapWhenHurtAnswerYes(uint8_t slot, uint8_t y_entry);

/* object/enemies_l8_st26.c */
typedef struct {
    int arm;
    uint8_t state, timer, again, wait, counted;
    L8Record record;
    AxisMove move;
} DriftArm;

typedef struct {
    int arm;
    uint8_t state, page, joined;
    L8Record record;
    Sum sum, sum_page, diff;
    L8Scan scan;
} PieceSlotsArm;

typedef struct {
    int arm;
    uint8_t state, flash, facing, wait, pick, counted, angle, index, angle2, value, high, low;
    bool picked, index_carry;
    L8Record record;
    L8Step step;
    L8Arm settle;
    SideResult side;
    RandomResult random, random2;
    ScaleResult scale;
    AxisMove move;
    AngleExit toward;
    TurnAngleResult turn;
    Sum wait_sum, target, diff, sum, speed;
} ChaseArm;

typedef struct {
    int arm;
    uint8_t state, kind, x_pulled, counted, timer, kind2, screen, low, placed, number;
    L8Record record;
    L8Arm cap;
    L8Scan scan, scan2;
    RandomResult random;
    Sum sum;
} GatesArm;

DropArm MaybeDropItemByPlayerDistance1019(uint8_t slot);
CommandCall PlaySound58A(uint8_t slot, uint8_t y_entry);
CommandCall PlaySound58B(uint8_t slot, uint8_t y_entry);
L8Arm SettleWhenLowOnFirstRow(uint8_t slot);
L8Arm LoadRecordFrameFromKind(uint8_t slot);
L8Arm CapPlayerXOnScreen1(uint8_t slot);
DriftArm LoadRecordThenDriftAndCount(uint8_t slot);
L8Load LoadRecordAtX80(uint8_t slot);

/* object/enemies_l8_st27.c */
Ended BossFrameRoutines(uint8_t slot, uint8_t y_entry);
Ended HurtSoundsThenRunTable(uint8_t slot, uint8_t y_entry);
Ended BossFrameRoutinesB(uint8_t slot, uint8_t y_entry);
Ended SpawnRowsWhenScrollAtZero(uint8_t slot);
PieceSlotsArm CountDownThenStartPieceInSlots18To1B(uint8_t slot);
LandWait StepListByPhaseThenLand(uint8_t slot, uint8_t y_entry, bool carry_entry);
SoundEnd LoadRecordThenWatchThenDropTwice(uint8_t slot, uint8_t y_entry);
Ended BossPhasesHurtFallAndTileWrites(uint8_t slot, uint8_t y_entry);
HopArm HopBetweenSidesWithBox(uint8_t slot, uint8_t y_entry);
ChaseArm ChaseWanderingPointWithSpeedRamp(uint8_t slot, uint8_t y_entry);
GatesArm SpawnerWithScrollGates(uint8_t slot);
SpawnerArm SpawnerByKindAndScreen(uint8_t slot);
Ended StartBossWhenStageC0OrLoad(uint8_t slot, uint8_t y_entry);
Ended StepScrollThenMoveOrFadeOut(uint8_t slot);
Ended BossFrameRoutinesC(uint8_t slot);
Ended BossFrameRoutinesD(uint8_t slot, uint8_t y_entry);

#endif
