/* Lot L7: the stage enemies of four stages and their helpers, as typed C. */
#ifndef RONDO_STAGE_ENEMIES_L7_H
#define RONDO_STAGE_ENEMIES_L7_H

#include "machine.h"
#include "enemy_objects.h"
#include "common_objects.h"

/* The values a contact test read and what it ended on: the player is touched (the player stands on the object) or not. */
typedef struct {
    bool contact;
    int arm;
    SideResult side;
    uint8_t v[48];
} ContactArm;

/* A piece that asks the contact test, and what it ended on. */
typedef struct {
    ObjArm arm;
    ContactArm contact;
} ContactStep;

/* A follow routine: the move from the path tables, then ShowByVelocity's result. */
typedef struct {
    ObjArm head;
    ObjArm show;
} PathShow;

typedef struct {
    ObjArm first;
    PathShow path;
} HitboxPathShow;

/* A tile piece object: its own arm and, when it asks, TilePieceInRange072A's. */
typedef struct {
    ObjArm arm;
    ObjArm range;
} PieceObjArm;

Ended contact_exit_shim(ContactArm a);
ContactArm ContactWithPlayerBox14By20(uint8_t slot);
Ended start_debris_burst_exit_shim(ObjArm r);

/* object/enemies_stage06_temp.c */
ObjArm WrapVerticalDrift(uint8_t slot);
ObjArm MoveAndWrapDriftA(uint8_t slot);
ObjArm MoveAndWrapDriftB(uint8_t slot);
ObjArm RisingDriftBlock(uint8_t slot);
ObjArm LevelDriftBlock(uint8_t slot);
ObjArm StartRisingPiece(uint8_t slot);
ObjArm StartLevelPiece(uint8_t slot);
ObjArm DropPieceEvery32(uint8_t slot);
ContactArm ContactWithPlayer(uint8_t slot);
ContactStep StepPieceOnParent(uint8_t slot);
ContactStep FollowParentUntilTouched(uint8_t slot);
ContactStep FallingPieceMove(uint8_t slot);
ContactStep FallingPieceStep(uint8_t slot);

ObjArm HitboxByVerticalDirection(uint8_t slot);
ObjArm LiftObjectsBySlowFraction(uint8_t slot);
ObjArm ShiftObjectsX(uint8_t slot);
ObjArm OrbitingObject(uint8_t slot);
ObjArm AccelerateAndSway(uint8_t slot);
ObjArm StepThenCount(uint8_t slot);
ObjArm StepThenStop(uint8_t slot);
ObjArm FallUntilLevel2(uint8_t slot);
ObjArm StartPieceChain(uint8_t slot);
ObjArm StartPieceStreamA(uint8_t slot, uint8_t y);
ObjArm StartPieceStreamB(uint8_t slot, uint8_t y);
ObjArm StartPieceStreamC(uint8_t slot);
ObjArm StepFallingPieceStream(uint8_t slot);
ObjArm StepPieceEmitter(uint8_t slot);
ObjArm StartOnFirstState(uint8_t slot);
ObjArm FaceByVelocityAndCatch(uint8_t slot);
ObjArm FallingDebris(uint8_t slot);
ObjArm FlashByHeight(uint8_t slot);
ObjArm StartWithEvent(uint8_t slot);
ObjArm RaiseStateOnFlag(uint8_t slot);
void StopVerticalSpeed(uint8_t slot);
ObjArm JumpByStageStep(void);

typedef struct {
    Ended first;
    Ended head;
    SoundEnd burst;
    Ended tail;
} ShowEnd;
typedef struct {
    SoundEnd pre;
    Ended tail;
} BurstTail;
typedef struct {
    SoundEnd sound;
    Ended burst;
    bool burst_run;
    Ended unused;
    uint8_t unused2;
    Ended tail;
} BreakEnd;
typedef struct {
    SoundEnd sound;
    bool played;
    Ended pre;
    TrackPlay track;
    Ended post;
} MusicEnd;
ObjArm BreakIntoPieces(uint8_t slot, uint8_t y_entry);
ObjArm StartScriptsAndLists(uint8_t slot);
ObjArm StepThenMusicAgain(uint8_t slot, uint8_t y);
ObjArm BurstAtHeightE0(uint8_t slot, uint8_t y);
ObjArm ShowByVelocity(uint8_t slot, uint8_t y);
PathShow FollowPathAndShow(uint8_t slot, uint8_t y);
HitboxPathShow SetHitboxThenFollowPath(uint8_t slot, uint8_t y);
PathShow FollowPathB(uint8_t slot, uint8_t y);
ObjArm StartDriftingPiece(uint8_t slot, uint8_t y);
ObjArm DropAndBurst(uint8_t slot, uint8_t y);
ObjArm StepFallAfterFour(uint8_t slot);
Jump JumpByStateTableC(uint8_t slot);
Jump FaceBreakAndRun(uint8_t slot, Ended *before);
ObjArm JumpByStateTableB(uint8_t slot);
ObjArm RunDriftObject(uint8_t slot);

/* The registers the original holds at this point: what the typed steps so far left, put into the machine before original code runs. */
static inline void sync(Ended *e)
{
    abi_ended(*e);
    *e = ended_kept();
}

/* The object's own code is entered with X its slot, as the dispatch left the other registers. A dispatch with nothing to jump to (the object's kind was cleared meanwhile, the original's `beq` to its `rts`) leaves only its registers. */
static inline uint8_t enter_handler(uint8_t x, Jump jump)
{
    cpu.x = x;
    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
    return cpu.x;
}
/* A converted routine that asks a mover (a landing wait, LandWait, or a move wait, MoveWait, common_objects.h): whether
 * the mover was asked, how it went, and what the routine ended on. */
typedef struct {
    bool asked;
    Mover mover;
    ObjArm arm;
} MoverArm;

/* A converted routine that asks the hitbox table and the player-state check (FrameLookup, FiveResult, common_objects.h): their results and what the routine ended on. */
typedef struct {
    ObjArm arm;
    FrameLookup lookup;
    FiveResult five;
} GuardArm;

Ended HitDamage(uint8_t slot);
SoundEnd ScoreForKind(uint8_t slot, uint8_t y);
ObjArm BlendStockRows(uint8_t slot, uint8_t y);
Ended blend_stock_rows_exit_shim(ObjArm r);
ObjArm GiveExtraLife(uint8_t slot, uint8_t y);
SoundEnd give_extra_life_exit_shim(ObjArm r);
ObjArm BlendStageRowsOfPlayer(uint8_t slot, uint8_t y);
Ended blend_stage_rows_of_player_exit_shim(ObjArm r);
ObjArm StartDebrisBurst(uint8_t slot, uint8_t timer, uint8_t y_entry);

/* An exit built from several callees' exits in order: each step is a plain exit, a sound call's, or a first-screen sound's. */
typedef enum { STEP_ENDED, STEP_SOUND_END, STEP_SCREEN_END, STEP_PROBE } StepKind;
typedef struct {
    StepKind kind;
    Ended e;
    SoundEnd s;
    ScreenSoundEnd c;
    Probe p;
} ExitStep;
typedef struct {
    ExitStep step[12];
    unsigned count;
} ExitChain;
static inline void chain_ended(ExitChain *c, Ended e)
{
    ExitStep st = { STEP_ENDED, e, { { 0 }, { 0 }, false, 0, { 0 } }, { { 0 }, { 0 }, false, { 0 } }, { 0 } };

    c->step[c->count++] = st;
}
static inline void chain_sound(ExitChain *c, SoundEnd s)
{
    ExitStep st = { STEP_SOUND_END, { 0 }, s, { { 0 }, { 0 }, false, { 0 } }, { 0 } };

    c->step[c->count++] = st;
}
static inline void chain_screen(ExitChain *c, ScreenSoundEnd s)
{
    ExitStep st = { STEP_SCREEN_END, { 0 }, { { 0 }, { 0 }, false, 0, { 0 } }, s, { 0 } };

    c->step[c->count++] = st;
}
static inline void chain_probe(ExitChain *c, Probe p)
{
    ExitStep st = { STEP_PROBE, { 0 }, { { 0 }, { 0 }, false, 0, { 0 } }, { { 0 }, { 0 }, false, { 0 } }, p };

    c->step[c->count++] = st;
}
static inline void abi_chain(const ExitChain *c)
{
    unsigned first = 0;

    /* A probe that ran original code left that code's registers and flags: the steps before it were never in the machine. */
    for (unsigned i = 0; i < c->count; i++) {
        if (c->step[i].kind == STEP_PROBE && probe_ran_original(c->step[i].p)) {
            first = i;
        }
    }
    for (unsigned i = first; i < c->count; i++) {
        if (c->step[i].kind == STEP_ENDED) {
            abi_ended(c->step[i].e);
        } else if (c->step[i].kind == STEP_SOUND_END) {
            abi_sound_end(c->step[i].s);
        } else if (c->step[i].kind == STEP_PROBE) {
            probe_exit_shim(c->step[i].p);
        } else {
            abi_screen_sound_end(c->step[i].c);
        }
    }
}
ExitChain ObjectHit(uint8_t slot, uint8_t y);
ExitChain BounceSparkOffWalls(uint8_t slot, bool carry_in);

/* Whether an object is inside a window, and what the routine left. */
typedef struct {
    bool in_view;
    Ended end;
} ViewTest;

/* object/enemies_stage09_temp.c */
ObjArm InWindowRange(uint8_t slot);
ObjArm FaceByCue(uint8_t slot);
ObjArm WalkAndTurnAtEdges(uint8_t slot);
ObjArm WalkSideways(uint8_t slot);
ObjArm PickNextAction(uint8_t slot);
ObjArm WalkTowardPlayer(uint8_t slot);

ObjArm StartCueScript(uint8_t slot, uint8_t y);
ObjArm PlayCueOnFrame(uint8_t slot);
ObjArm StepFlyingCue(uint8_t slot, uint8_t y);
ObjArm StartTwinSpawner(uint8_t slot);

ObjArm StepThenQuarterWait(uint8_t slot);
ObjArm StartShotRing(uint8_t slot);
ObjArm EnterStateWhenSlotClear(uint8_t slot);
ObjArm BurstOnLanding(uint8_t slot, uint8_t y);
ObjArm StartSplitterHigh(uint8_t slot, bool carry_in);
ObjArm StartSplitterLow(uint8_t slot, bool carry_in);
ObjArm MaybeShootAhead(uint8_t slot);

ObjArm PickWaterLevel(void);
ObjArm WaitForPage2(uint8_t slot, uint8_t y);
ObjArm RiseUntilAboveGround(uint8_t slot);
ObjArm WaitForSlot10State4(uint8_t slot);
ObjArm StartNumberedPiece(uint8_t slot);
ObjArm FollowPlayerHitbox(uint8_t slot);

ObjArm RiseAndClear(uint8_t slot);
ObjArm RaiseStateAfterSlot10(uint8_t slot);
ObjArm StartChainLink(uint8_t slot);
ObjArm ChainFollowsPrevious(uint8_t slot);
ObjArm StartKickedPiece(uint8_t slot);
ObjArm StepThenClear(uint8_t slot);

void ClearAnimation(uint8_t slot);
ObjArm EnterNextAndReset(uint8_t slot);
ObjArm FlipSide(uint8_t slot);
ObjArm StepListByPhase(uint8_t slot);
ObjArm SetSwaySpeed(uint8_t slot);
ObjArm BobAndMove(uint8_t slot);

ObjArm ReactAtWindowEdge(uint8_t slot);
ObjArm SwayStep(uint8_t slot);
ObjArm SwayOrAdvance(uint8_t slot);
ObjArm SwayThenReact(uint8_t slot);
ObjArm SwayThenFlip(uint8_t slot);
ObjArm SwayThenRestart(uint8_t slot);
ObjArm JumpByStateTable(uint8_t slot);

/* object/enemies_stage0a_temp.c */
Ended BreakWhenHit(uint8_t slot);
Ended StartWithPartner(uint8_t slot);
LandWait StopWhenLanded(uint8_t slot, uint8_t column, bool carry_in);
Ended SlowToPlayer(uint8_t slot);
Ended ChooseAttackByDistance(uint8_t slot);
Ended TurnWhenWaitEnds(uint8_t slot);
Ended StepWithArmor(uint8_t slot, uint8_t y);
Ended ChooseNextMove(uint8_t slot, bool carry_in);
ScreenSoundEnd StepAndShout(uint8_t slot);

/* A shooting routine's exit: what came before, the sound call, what followed it, and the rest. */
typedef struct {
    Ended before;
    Ended end;
    bool called;
    SoundOnScreen sound;
    Ended mid;
    AxisMove move;
} ShotEnd;
ShotEnd ShootArrow(uint8_t slot, uint8_t cue);
ShotEnd FlashAndCount(uint8_t slot, uint8_t y);


/* Exits of the hopping and waiting routines of stage 0A51. */
typedef struct {
    Ended before;
    bool probed;
    Probe probe;
    Ended after;
} HopEnd;
typedef struct {
    Ended before;
    bool picked;
    ObjArm timers;   /* what PickTimersAtRandomB or PickAngleAtRandomB ended on */
    Ended after;
} WaitEnd;
typedef struct {
    WaitEnd first;
    WaitEnd second;
} WaitPair;
HopEnd HopAndSettle(uint8_t slot, uint8_t column, bool carry_in);
Ended SetHitboxByFrame(uint8_t slot);
WaitEnd CountDownThenAdvance(uint8_t slot);
WaitEnd CountDownAngleThenAdvance(uint8_t slot);
WaitPair CountBothWaits(uint8_t slot);
void wait_exit_shim(WaitEnd r);

/* A routine that picks a throw, then timers, then an angle: the exits of the three in order. */
typedef struct {
    Ended before;
    ObjArm thrown;   /* what ThrowByNextKindB, PickTimersAtRandomB and PickAngleAtRandomB ended on */
    ObjArm timers;
    ObjArm angle;
} ThrowChain;
ThrowChain StartKnifeThrower(uint8_t slot);

/* BounceAtWalls: two probes and what each step left. */
typedef struct {
    Ended e0;
    Probe probe1;
    bool second;
    Ended e1;
    Probe probe2;
    bool turned;
    Ended e2;
    Ended e3;
} TurnEnd;
TurnEnd BounceAtWalls(uint8_t slot, uint8_t column);

typedef struct {
    Ended pre;
    bool walked;
    TurnEnd turn;
} WalkEnd;
typedef struct {
    WalkEnd walk;
    Ended rest;
} AttackWalk;
typedef struct {
    Ended head;
    WalkEnd walk;
    Ended mid;
    Ended tail;
} ChaseEnd;
TurnEnd BounceWithHeight4(uint8_t slot, uint8_t column);
WalkEnd WalkByPhase(uint8_t slot, uint8_t phase, uint8_t column);
AttackWalk WalkAndAttackA(uint8_t slot, uint8_t column);
AttackWalk WalkAndAttackB(uint8_t slot, uint8_t column);
ChaseEnd ChasePlayerA(uint8_t slot, uint8_t column, bool carry_in);
ChaseEnd ChasePlayerB(uint8_t slot, uint8_t column, bool carry_in);
void bounce_exit_shim(TurnEnd r);
void walk_phase_exit_shim(WalkEnd r);

typedef struct {
    Ended pre;
    bool bounced;
    TurnEnd turn;
} StepBounce;
typedef struct {
    Ended before;
    Ended end;
    bool called;
    SoundOnScreen sound;
    Ended unused;
} FollowEnd;
StepBounce StepAndLunge(uint8_t slot, uint8_t y);
StepBounce StepAndPounce(uint8_t slot, uint8_t y);
AttackWalk WalkThenTurn(uint8_t slot, uint8_t column);
FollowEnd FollowPartner(uint8_t slot, uint8_t y);

/* object/enemies_stage0c_temp.c */
typedef struct {
    Ended pre;
    bool played;
    TrackPlay track;
    bool sounded;
    uint8_t command;
    CommandCall call;
    Ended end;
} CueEnd;
typedef struct {
    SoundEnd shoot;
    Ended tail;
} ShootTail;
ObjArm StepThenThrowShot(uint8_t slot);
ObjArm StepThenThrowSlowShot(uint8_t slot);
ObjArm StepThenRest(uint8_t slot);
ObjArm StepThenRestB(uint8_t slot);
ObjArm StepThenWaitLonger(uint8_t slot);
ObjArm StartTwoPieces(uint8_t slot);
ObjArm SinkToLimit(uint8_t slot);
ObjArm SinkWhenPartnerGone(uint8_t slot);
ObjArm SetLongWait(uint8_t slot);
ObjArm DropShotWhenSlotFree(uint8_t slot);
ObjArm SinkThenRestart(uint8_t slot);
ObjArm ShootEveryFourth(uint8_t slot, uint8_t y);
ObjArm SinkAndShootThenRestart(uint8_t slot, uint8_t y);
typedef struct {
    Ended start;
    bool started;
    uint8_t low;
    uint8_t high;
    Ended read;
    bool cleared;
    uint8_t unused1;
    uint8_t unused2;
    Ended event;
} VramWatch;
ObjArm FlickerAndStartPieces(uint8_t slot);
ObjArm StepPartList(uint8_t slot);
void DoNothing(void);
ObjArm WatchVramForMark(uint8_t slot);
ObjArm StepMoveOrClear(uint8_t slot);
ObjArm StepCountClear(uint8_t slot);
ObjArm StartFallingSpark(uint8_t slot);
ObjArm StartFromTable(uint8_t slot);
ObjArm SlideTurnByTable(uint8_t slot);
ObjArm MoveAlongAndSlow(uint8_t slot);
ObjArm LaunchThenFall(uint8_t slot, uint8_t y_entry);
ObjArm StartTileScriptByKind(uint8_t slot);
ObjArm CountThenTileScript(uint8_t slot);
ObjArm JumpByStateTableD(uint8_t slot);
ObjArm JumpByPhaseTable(uint8_t slot);
GuardArm StepWithGuardTable(uint8_t slot);
ObjArm StepPhaseThenPause(uint8_t slot);
ObjArm ShootMoveAndPickSpeed(uint8_t slot);
ObjArm JumpByStateTableE(uint8_t slot);
ObjArm JumpByStateTableF(uint8_t slot);
ObjArm HitRowByKindAndHits(uint8_t slot);
ObjArm StepFacingAndJump(uint8_t slot);
ObjArm PickWaterOnTenth(uint8_t slot);
ObjArm SwitchStateOnHitRow(uint8_t slot);
ObjArm StepWithHitTable(uint8_t slot);
ObjArm TrackPointOrLand(uint8_t slot);
ObjArm TrackThenAimAtPlayer(uint8_t slot);
ObjArm OrbitThenLeave(uint8_t slot);
ObjArm StartShotsUnlessThere(uint8_t slot);
ObjArm FollowAndFacePlayer(uint8_t slot);
ObjArm StepOrClearByKind(uint8_t slot);
ObjArm StartSoundedPiece(uint8_t slot, uint8_t y);
ObjArm StartThrownPiece(uint8_t slot);
ObjArm StartScatteringPiece(uint8_t slot);
ObjArm StepListAndMove(uint8_t slot);
ObjArm StartSideDrifter(uint8_t slot);
ObjArm CountThenCue(uint8_t slot, uint8_t y);
ObjArm FallingWithGravity(uint8_t slot);
ObjArm AimAtPlayerThenArm(uint8_t slot);
ObjArm InsideScreenBox(uint8_t slot);
ObjArm CountThenStop(uint8_t slot);
ObjArm PickNextMoveBySide(uint8_t slot);
ObjArm StepThenSpawnShot(uint8_t slot);
ObjArm ResetToStateThree(uint8_t slot);
ObjArm StartThreeShots(uint8_t slot);
ObjArm StartWhenHurt(uint8_t slot);
ObjArm EnterStateSevenOnFlag(uint8_t slot);
ObjArm StartWithGuard(uint8_t slot);
ObjArm SlideAngleDown(uint8_t slot);
ObjArm SlideAngleUp(uint8_t slot);


ObjArm TouchesPlayerMark(uint8_t slot);
ObjArm CountBothPairs(uint8_t slot);
ObjArm HealAndClear(uint8_t slot, uint8_t y);
ObjArm StartOnPlayerAnimation(uint8_t slot);
ObjArm AdvanceByHealth(uint8_t slot);
ObjArm PickGroundSpeedInWindow(uint8_t slot);
ObjArm MoveThenPick(uint8_t slot);
ObjArm SwingAndPick(uint8_t slot);
ObjArm FallStepAndLeave(uint8_t slot);
ObjArm StartTracker(uint8_t slot);
ObjArm StartThenStepList(uint8_t slot);
ObjArm FinalSpinAndEnd(uint8_t slot);
ObjArm StepFallingSpark(uint8_t slot);
ObjArm StartFallingSpark09(uint8_t slot);
ObjArm ChainCountdown(uint8_t slot);
ObjArm ChainSwings(uint8_t slot);
ObjArm WalkAndPick(uint8_t slot);
ObjArm StepShootOnCue(uint8_t slot, uint8_t y);

/* A memory shift right in one read and one write: the value written, and the bit shifted out in `carry_var`. */
#define LSR8_VALUE(field, carry_var) \
    RMW8(field, old_s, ((carry_var) = (old_s & 1) != 0, old_s >> 1))

/* The call the loop makes to its overlay's dispatch (JSR $A313): the wrapper of the overlay's own DispatchByKind, or the original code at $A313 when it has none; gives X as the object's code left it. */
static inline uint8_t call_kind_dispatch(uint8_t x, void (*abi_dispatch)(void))
{
    cpu.x = x;
    if (abi_dispatch != 0) {
        abi_dispatch();
    } else {
        call_original(0xA313);
    }
    return cpu.x;
}
Ended RunObjects(uint8_t entry_x);
Ended RunObjects07D9(uint8_t entry_x);
Ended RunObjects0889(uint8_t entry_x);
Ended RunObjects0A99(uint8_t entry_x);
Ended RunObjects0B49(uint8_t entry_x);
Ended RunObjects0CA9(uint8_t entry_x);
Ended RunObjects0D59(uint8_t entry_x);
Ended RunObjects0E09(uint8_t entry_x);
Ended RunObjects0EB9(uint8_t entry_x);
Ended RunObjects0F69(uint8_t entry_x);
Ended RunObjects1019(uint8_t entry_x);
Ended RunObjects10C9(uint8_t entry_x);
Ended RunObjects1179(uint8_t entry_x);
Ended RunObjects12D9(uint8_t entry_x);
Ended RunObjects13F9(uint8_t entry_x);
Ended RunObjects1619(uint8_t entry_x);
Ended RunObjects16C9(uint8_t entry_x);
Ended RunObjects18D9(uint8_t entry_x);
uint8_t BitMaskFromIndex072A(uint8_t count);
PieceObjArm TilePieceObject072A(uint8_t slot);
ObjArm ShatterOnHits(uint8_t slot);
ObjArm TilePieceInRange072A(uint8_t slot);
ObjArm DrawTilePiece072A(uint8_t slot);

/* object/enemies_l6_*.c: lot L6 */
ObjArm LoadRecordA04(uint8_t slot);
ObjArm LoadRecordB04(uint8_t slot);
ObjArm LoadRecordA06(uint8_t slot);
ObjArm ClearFrame(uint8_t slot);
ObjArm TurnAroundInState9(uint8_t slot);
ObjArm FallBackInState9(uint8_t slot);
ObjArm PlaySoundTwo(uint8_t slot, uint8_t y_entry);
ObjArm ClearHitsTaken(uint8_t slot);
ObjArm StepListB819(uint8_t slot, uint8_t y_entry);
ObjArm RandomTimer10(uint8_t slot);
ObjArm CopyScrollToTimer(uint8_t slot);
ObjArm LoadThenSpeedF0(uint8_t slot);
ObjArm NextStateOnScrollY(uint8_t slot);
ObjArm StepListByIndex088A(uint8_t slot, uint8_t y_entry);
ObjArm SetTimersOf12To15(void);
ObjArm RandomTimerC0(uint8_t slot);
ObjArm NextPhaseUnlessKind25(uint8_t slot);
ObjArm SetHitBox0C18(uint8_t slot);
ObjArm StartPieceAtNextSlot(uint8_t slot);
ObjArm DrawTileScriptByHighBits(uint8_t slot);
ObjArm CountThenNextStateF3510(uint8_t slot);
ObjArm SetFrameThenCountToNext(uint8_t slot);
ObjArm StartListSoundThenSpawn20(uint8_t slot);
ObjArm StartFallWithFrame21(uint8_t slot);
ObjArm HalveDirectionWhenHurt(uint8_t slot);
ObjArm LoadThenSpawn24(uint8_t slot);
ObjArm ClearSlot4EUnlessCamera18(uint8_t slot);
ObjArm StepListB5FEToState4(uint8_t slot, uint8_t y_entry);
ObjArm StepListB37FThenNext(uint8_t slot, uint8_t y_entry);
ObjArm StepListB697ThenStopY(uint8_t slot, uint8_t y_entry);
ObjArm StateSixOnStatusBit7(uint8_t slot);
ObjArm StepListByTimerWithGravity(uint8_t slot);
ObjArm StepListB404ThenTimer(uint8_t slot, uint8_t y_entry);
ObjArm LoadThenStartRise(uint8_t slot);
ObjArm LoadThenRiseFast(uint8_t slot);
ObjArm SpeedBySideOfScreen(uint8_t slot, bool carry_in);
ObjArm StartThenFallAlongList(uint8_t slot, uint8_t y_entry);
ObjArm ClearSlotsWhenScrollZero(uint8_t slot);
ObjArm FrameBySideThenTimerOnScroll(uint8_t slot);
ObjArm StepListACB2ThenNextOnScreen(uint8_t slot, uint8_t y_entry);
ObjArm StepListB37FUnlessFar(uint8_t slot, uint8_t y_entry);
ObjArm StartFourPiecesWithCount(uint8_t slot);
ObjArm CountDownThenState6(uint8_t slot);
ObjArm StepListB36AThenCount(uint8_t slot, uint8_t y_entry);
ObjArm StartKindFromTableAtSlot16(uint8_t slot);
ObjArm FallAndBlinkThenClear(uint8_t slot);
ObjArm StepListB66BToState2(uint8_t slot, uint8_t y_entry);
ObjArm StartPhaseWhenScrolled(uint8_t slot);
ObjArm LoadThenSpeedFromTable(uint8_t slot);
ObjArm StepListB25CThenStartList(uint8_t slot, uint8_t y_entry);
ObjArm StartFirstObjectWhenCamera2(uint8_t slot);
ObjArm StepListB728ThenRiseSlowly(uint8_t slot, uint8_t y_entry);
ObjArm CountThenJumpRight(uint8_t slot);
ObjArm StepListBC5CThenAimAtPlayer(uint8_t slot, uint8_t y_entry);
ObjArm LoadOrFallAlongListByTimer(uint8_t slot);
MoverArm StepListB3B8ThenLand(uint8_t slot, uint8_t y_entry, bool carry_entry);
MoverArm LandThenFaceAndRun(uint8_t slot, uint8_t column, bool carry_in);
ObjArm StepListB2A7ThenPlace(uint8_t slot, uint8_t y_entry);
ObjArm LoadThenRandomPlace(uint8_t slot);
ObjArm PlaceFromTableCopySlot10(uint8_t slot);
ObjArm StartThreeNumberedObjects(uint8_t slot);
ObjArm PlaceFromTableThenStepList(uint8_t slot);
ObjArm LoadThenSinkAlongList(uint8_t slot, uint8_t y_entry);
ObjArm LoadThenStartFiveFromTable(uint8_t slot);
ObjArm SetHitBoxFromTable(uint8_t slot);
ObjArm CopyPlaceAndHitBoxOfSlot10(uint8_t slot);
ObjArm RiseThenPlaceAndDrop(uint8_t slot);
ObjArm WalkWhileNearPlayer(uint8_t slot, uint8_t y_entry);
ObjArm LoadThenFallAndFlash(uint8_t slot);
ObjArm LoadThenSpawnWhileScrolling(uint8_t slot);
ObjArm SpawnPointFromTables(uint8_t slot);
ObjArm JumpAwayThenLand(uint8_t slot);
ObjArm StateSevenOnStatusBit7(uint8_t slot);
ObjArm SpawnOnTimerInTable(uint8_t slot);
/* What a routine that sends a sound and then plays the music again leaves, in the order of its calls. */
typedef struct {
    Ended pre;
    bool sounded;
    uint8_t command;
    CommandCall call;
    bool played;
    TrackPlay track;
    Ended end;
} SoundPlayEnd;
ObjArm RiseThenCueMusic(uint8_t slot, uint8_t y_entry);
ObjArm LoadThenStepListBreakAtFive(uint8_t slot, uint8_t y_entry);
ObjArm FollowBlockWhileState5(uint8_t slot);
ObjArm LoadThenBreakIntoThrownPiece(uint8_t slot, uint8_t y_entry);
ObjArm StepListIndex0AThenSpawn(uint8_t slot);
ObjArm LoadThenSwingAndDrop(uint8_t slot);
/* A routine that may ask MoveAndLand: what it left before, the mover, what its instructions left after. */
typedef struct {
    Ended pre;
    bool asked;
    Mover mover;
    Ended end;
} MoveWait;
MoverArm WalkTurnOrJumpOnLanding(uint8_t slot, uint8_t y_entry);
ObjArm NearPlayerThenPlaceOrState11(uint8_t slot);
MoverArm LandThenNextState(uint8_t slot, uint8_t column, bool carry_in);
ObjArm SetRasterSpawnThenStepList(uint8_t slot, uint8_t y_entry);
ObjArm CountScriptsThenStepList(uint8_t slot, uint8_t y_entry);
ObjArm CheckPlayerThenStepListIndex08(uint8_t slot);
ObjArm DrawStepCountThenClear(uint8_t slot);
ObjArm StateFiveOnStatusBit7(uint8_t slot);
ObjArm CountThenSoleScriptAndSound(uint8_t slot);
ObjArm StartScriptWhenCountInTable(uint8_t slot);
ObjArm LoadThenStartTwoScripts(uint8_t slot);
ObjArm SpawnInFreeSlotEvery30(uint8_t slot);
ObjArm SoundThenAimRandomLength(uint8_t slot, uint8_t y_entry);
ObjArm CountThenSpawnInState2(uint8_t slot);
ObjArm WaitForPressThenAnimate(uint8_t slot, uint8_t y_entry);
ObjArm LoadThenScatterAlongList(uint8_t slot, uint8_t y_entry);
ObjArm TurnTowardPlayerAndMove(uint8_t slot, uint8_t y_entry);
ObjArm FollowSlot10WithHitBox(uint8_t slot);
ObjArm SpeedFromRandomRow(uint8_t slot);
ObjArm SetHitBoxFromFrameTable(uint8_t slot);
ObjArm CountThenRiseWithFlash(uint8_t slot);
ObjArm LoadThenPlaceAtLeft(uint8_t slot);
ObjArm StepListB53CToState5(uint8_t slot, uint8_t y_entry);
ObjArm ResetAnimThenNextPhase(uint8_t slot);
ObjArm SetFirstObjectTimersThenCount(uint8_t slot);
ObjArm FallThenSoundAtPeak(uint8_t slot, uint8_t y_entry);
ObjArm StepListB1BAThenSound(uint8_t slot, uint8_t y_entry);
ObjArm RiseThenPlaceWithSound(uint8_t slot, uint8_t y_entry);
ObjArm StepListC170ThenMoveAlongAngle(uint8_t slot, uint8_t y_entry);
ObjArm StartThenWaitForField83(uint8_t slot);
ObjArm TurnTowardPlayerOrStepList(uint8_t slot, uint8_t y_entry);
ObjArm StepListC103ThenPickAim(uint8_t slot, uint8_t y_entry);
ObjArm LoadThenStateByKindByte(uint8_t slot);
ObjArm StepListB9B7ThenAimAtPlayer(uint8_t slot, uint8_t y_entry);
ObjArm StartJumpUpLeft(uint8_t slot);
ObjArm FallThenStopForRaster(uint8_t slot);
Jump JumpByPhaseTable072C004(uint8_t slot);
Jump JumpByStateTable072C763(uint8_t slot);
Jump JumpByStateTable093C3FF(uint8_t slot);
Jump JumpByStateTable093C4E5(uint8_t slot);
Jump JumpByStateTable09EB7D3(uint8_t slot);
Jump JumpByStateTable09EC18D(uint8_t slot);
Jump JumpByStateTable0BFC12B(uint8_t slot);
Jump JumpByStateTable0CAB12F(uint8_t slot);
Jump JumpByPhaseTable0CAB4D7(uint8_t slot);
Jump JumpByStateTable0CAC47A(uint8_t slot);
Jump JumpByStateTable0CAC59D(uint8_t slot);
Jump JumpByStateTable0CAC6C3(uint8_t slot);
Jump JumpByStateTable051A190(uint8_t slot);
Jump JumpByStateTable072B1B1(uint8_t slot);
Jump JumpByStateTable072B24C(uint8_t slot);
Jump JumpByStateTable0F6C3C0(uint8_t slot);
Jump JumpByStateTable05CB1CF(uint8_t slot);
Jump JumpByStateTable025B545(uint8_t slot);
Jump JumpByStateTable0B4B64C(uint8_t slot);
Jump JumpByStateTable0F6B3EC(uint8_t slot);
Jump JumpByPhaseTable088B06C(uint8_t slot);
Jump JumpByPhaseTable025C068(uint8_t slot);
Jump JumpByPhaseTable025C40D(uint8_t slot);
Jump JumpByStateTable025C51C(uint8_t slot);
Jump JumpByStateTable051B225(uint8_t slot);
Jump JumpByPhaseTable05CB212(uint8_t slot);
Jump JumpByPhaseTable05CB4B8(uint8_t slot);
Jump JumpByStateTable05CC0E9(uint8_t slot);
Jump JumpByStateTable088A304(uint8_t slot);
Jump JumpByStateTable0B4A4DC(uint8_t slot);
Jump JumpByStateTable0B4A573(uint8_t slot);
Jump JumpByPhaseTable0B4C0D7(uint8_t slot);
Jump JumpByPhaseTable0B4C42F(uint8_t slot);
Jump JumpByStateTable0B4D08F(uint8_t slot);
Jump JumpByPhaseTable051B350(uint8_t slot);
Jump JumpByPhaseTable051B5B5(uint8_t slot);
Jump JumpByStateTable088A4E4(uint8_t slot);
Jump JumpByPhaseTable0E0B16C(uint8_t slot);
ObjArm StepListC192ThenClear(uint8_t slot, uint8_t y_entry);

/* What DrawTileScript called with the item (and the group's Y) leaves: A $FF, the compare's N clear and Z set, C set, Y as the last op left it, and V when a line step ran. */
static inline Ended ended_of_tile_run(TileRun run, uint8_t item)
{
    Ended e = ended_with_a(ended_nz(0), 0xFF);

    e = ended_with_carry(ended_with_y(e, tile_run_y_shim(run, (uint8_t)((uint8_t)(item << 1) + 1))), true);
    return run.stepped_line ? ended_with_overflow(e, run.line_overflow) : e;
}

#endif
