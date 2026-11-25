/* Lot L7: the stage enemies of four stages and their helpers, as typed C. */
#ifndef RONDO_STAGE_ENEMIES_L7_H
#define RONDO_STAGE_ENEMIES_L7_H

#include "machine.h"
#include "enemy_objects.h"
#include "common_objects.h"

/* Whether a block touches the player, and what the routine's last instructions left. */
typedef struct {
    bool contact;
    Ended end;
} Contact;

/* object/enemies_stage06_temp.c */
Ended WrapVerticalDrift(uint8_t slot);
Ended MoveAndWrapDriftA(uint8_t slot);
Ended MoveAndWrapDriftB(uint8_t slot);
Ended RisingDriftBlock(uint8_t slot);
Ended LevelDriftBlock(uint8_t slot);
Ended StartRisingPiece(uint8_t slot);
Ended StartLevelPiece(uint8_t slot);
Ended DropPieceEvery32(uint8_t slot);
Contact ContactWithPlayer(uint8_t slot);
Ended StepPieceOnParent(uint8_t slot);
Ended FollowParentUntilTouched(uint8_t slot);
Ended FallingPieceMove(uint8_t slot);
Ended FallingPieceStep(uint8_t slot);

Ended HitboxByVerticalDirection(uint8_t slot);
Ended LiftObjectsBySlowFraction(uint8_t slot);
Ended ShiftObjectsX(uint8_t slot);
Ended OrbitingObject(uint8_t slot);
Ended AccelerateAndSway(uint8_t slot);
Ended StepThenCount(uint8_t slot);
Ended StepThenStop(uint8_t slot);
Ended FallUntilLevel2(uint8_t slot);
Ended StartPieceChain(uint8_t slot);
Ended StartPieceStreamA(uint8_t slot, uint8_t y);
Ended StartPieceStreamB(uint8_t slot, uint8_t y);
Ended StartPieceStreamC(uint8_t slot);
Ended StepFallingPieceStream(uint8_t slot);
Ended StepPieceEmitter(uint8_t slot);
Ended StartOnFirstState(uint8_t slot);
Ended FaceByVelocityAndCatch(uint8_t slot);
Ended FallingDebris(uint8_t slot);
Ended FlashByHeight(uint8_t slot);
Ended StartWithEvent(uint8_t slot);
Ended RaiseStateOnFlag(uint8_t slot);
void StopVerticalSpeed(uint8_t slot);
Jump JumpByStageStep(void);

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
BreakEnd BreakIntoPieces(uint8_t slot, uint8_t y_entry);
Ended StartScriptsAndLists(uint8_t slot);
MusicEnd StepThenMusicAgain(uint8_t slot, uint8_t y);
SoundEnd BurstAtHeightE0(uint8_t slot, uint8_t y);
ShowEnd ShowByVelocity(uint8_t slot, uint8_t y);
ShowEnd FollowPathAndShow(uint8_t slot, uint8_t y);
ShowEnd SetHitboxThenFollowPath(uint8_t slot, uint8_t y);
ShowEnd FollowPathB(uint8_t slot, uint8_t y);
SoundEnd StartDriftingPiece(uint8_t slot, uint8_t y);
BurstTail DropAndBurst(uint8_t slot, uint8_t y);
Ended StepFallAfterFour(uint8_t slot);
Jump JumpByStateTableC(uint8_t slot);
Jump FaceBreakAndRun(uint8_t slot, Ended *before);
Jump JumpByStateTableB(uint8_t slot);
void RunDriftObject(uint8_t slot);

/* The registers the original holds at this point: what the typed steps so far left, put into the machine before original code runs. */
static inline void sync(Ended *e)
{
    abi_ended(*e);
    *e = ended_kept();
}

/* The object's own code is entered with X its slot, as the dispatch left the other registers. */
static inline uint8_t enter_handler(uint8_t x, Jump jump)
{
    cpu.x = x;
    jump_shim(jump.end, jump.target);
    return cpu.x;
}
Ended HitDamage(uint8_t slot);
SoundEnd ScoreForKind(uint8_t slot, uint8_t y);
Ended BlendStockRows(uint8_t slot, uint8_t y);
SoundEnd GiveExtraLife(uint8_t slot, uint8_t y);
Ended BlendStageRowsOfPlayer(uint8_t slot, uint8_t y);
Ended StartDebrisBurst(uint8_t slot, uint8_t timer, uint8_t y_entry);

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
    for (unsigned i = 0; i < c->count; i++) {
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
ViewTest InWindowRange(uint8_t slot);
Ended FaceByCue(uint8_t slot);
Ended WalkAndTurnAtEdges(uint8_t slot);
Ended WalkSideways(uint8_t slot);
Ended PickNextAction(uint8_t slot);
Ended WalkTowardPlayer(uint8_t slot);

Ended StartCueScript(uint8_t slot, uint8_t y);
Ended PlayCueOnFrame(uint8_t slot);
SoundEnd StepFlyingCue(uint8_t slot, uint8_t y);
Ended StartTwinSpawner(uint8_t slot);

Ended StepThenQuarterWait(uint8_t slot);
Ended StartShotRing(uint8_t slot);
Ended EnterStateWhenSlotClear(uint8_t slot);
SoundEnd BurstOnLanding(uint8_t slot, uint8_t y);
Ended StartSplitterHigh(uint8_t slot, bool carry_in);
Ended StartSplitterLow(uint8_t slot, bool carry_in);
Ended MaybeShootAhead(uint8_t slot);

Ended PickWaterLevel(void);
SoundEnd WaitForPage2(uint8_t slot, uint8_t y);
Ended RiseUntilAboveGround(uint8_t slot);
Ended WaitForSlot10State4(uint8_t slot);
Ended StartNumberedPiece(uint8_t slot);
Ended FollowPlayerHitbox(uint8_t slot);

Ended RiseAndClear(uint8_t slot);
Ended RaiseStateAfterSlot10(uint8_t slot);
Ended StartChainLink(uint8_t slot);
Ended ChainFollowsPrevious(uint8_t slot);
Ended StartKickedPiece(uint8_t slot);
Ended StepThenClear(uint8_t slot);

void ClearAnimation(uint8_t slot);
Ended EnterNextAndReset(uint8_t slot);
Ended FlipSide(uint8_t slot);
Ended StepListByPhase(uint8_t slot);
Ended SetSwaySpeed(uint8_t slot);
Ended BobAndMove(uint8_t slot);

Ended ReactAtWindowEdge(uint8_t slot);
Ended SwayStep(uint8_t slot);
Ended SwayOrAdvance(uint8_t slot);
Ended SwayThenReact(uint8_t slot);
Ended SwayThenFlip(uint8_t slot);
Ended SwayThenRestart(uint8_t slot);
Jump JumpByStateTable(uint8_t slot);

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
    RandomEnd timers;
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
    Ended thrown;
    RandomEnd timers;
    RandomEnd angle;
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
Ended StepThenThrowShot(uint8_t slot);
Ended StepThenThrowSlowShot(uint8_t slot);
Ended StepThenRest(uint8_t slot);
Ended StepThenRestB(uint8_t slot);
Ended StepThenWaitLonger(uint8_t slot);
Ended StartTwoPieces(uint8_t slot);
Ended SinkToLimit(uint8_t slot);
Ended SinkWhenPartnerGone(uint8_t slot);
Ended SetLongWait(uint8_t slot);
Ended DropShotWhenSlotFree(uint8_t slot);
Ended SinkThenRestart(uint8_t slot);
SoundEnd ShootEveryFourth(uint8_t slot, uint8_t y);
ShootTail SinkAndShootThenRestart(uint8_t slot, uint8_t y);
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
Ended FlickerAndStartPieces(uint8_t slot);
Ended StepPartList(uint8_t slot);
void DoNothing(void);
VramWatch WatchVramForMark(uint8_t slot);
Ended StepMoveOrClear(uint8_t slot);
Ended StepCountClear(uint8_t slot);
Ended StartFallingSpark(uint8_t slot);
Ended StartFromTable(uint8_t slot);
Jump SlideTurnByTable(uint8_t slot, Ended *before);
Ended MoveAlongAndSlow(uint8_t slot);
SoundEnd LaunchThenFall(uint8_t slot, uint8_t y_entry);
Ended StartTileScriptByKind(uint8_t slot);
Ended CountThenTileScript(uint8_t slot);
Jump JumpByStateTableD(uint8_t slot);
Jump JumpByPhaseTable(uint8_t slot);
Ended StepWithGuardTable(uint8_t slot);
Ended StepPhaseThenPause(uint8_t slot);
Ended ShootMoveAndPickSpeed(uint8_t slot);
Jump JumpByStateTableE(uint8_t slot);
Jump JumpByStateTableF(uint8_t slot);
Ended HitRowByKindAndHits(uint8_t slot);
Ended TrackPointOrLand(uint8_t slot);
Ended TrackThenAimAtPlayer(uint8_t slot);
Ended OrbitThenLeave(uint8_t slot);
SoundEnd StartShotsUnlessThere(uint8_t slot);
Ended FollowAndFacePlayer(uint8_t slot);
Ended StepOrClearByKind(uint8_t slot);
SoundEnd StartSoundedPiece(uint8_t slot, uint8_t y);
Ended StartThrownPiece(uint8_t slot);
Ended StartScatteringPiece(uint8_t slot);
Ended StepListAndMove(uint8_t slot);
Ended StartSideDrifter(uint8_t slot);
CueEnd CountThenCue(uint8_t slot, uint8_t y);
Ended FallingWithGravity(uint8_t slot);
Ended AimAtPlayerThenArm(uint8_t slot);
ViewTest InsideScreenBox(uint8_t slot);
Ended CountThenStop(uint8_t slot);
Ended PickNextMoveBySide(uint8_t slot);
Ended StepThenSpawnShot(uint8_t slot);
Ended ResetToStateThree(uint8_t slot);
Ended StartThreeShots(uint8_t slot);
Ended StartWhenHurt(uint8_t slot);
Ended EnterStateSevenOnFlag(uint8_t slot);
Ended StartWithGuard(uint8_t slot);
Ended SlideAngleDown(uint8_t slot);
Ended SlideAngleUp(uint8_t slot);


ViewTest TouchesPlayerMark(uint8_t slot);
Ended CountBothPairs(uint8_t slot);
SoundEnd HealAndClear(uint8_t slot, uint8_t y);
Ended StartOnPlayerAnimation(uint8_t slot);
Ended AdvanceByHealth(uint8_t slot);
Ended PickGroundSpeedInWindow(uint8_t slot);
Ended MoveThenPick(uint8_t slot);
Ended SwingAndPick(uint8_t slot);
Ended FallStepAndLeave(uint8_t slot);
Ended StartTracker(uint8_t slot);
Ended StartThenStepList(uint8_t slot);
void FinalSpinAndEnd(uint8_t slot);
Ended StepFallingSpark(uint8_t slot);
Ended StartFallingSpark09(uint8_t slot);
Ended ChainCountdown(uint8_t slot);
Ended ChainSwings(uint8_t slot);
Ended WalkAndPick(uint8_t slot);
SoundEnd StepShootOnCue(uint8_t slot, uint8_t y);

/* A memory shift right in one read and one write: the value written, and the bit shifted out in `carry_var`. */
#define LSR8_VALUE(field, carry_var) \
    RMW8(field, old_s, ((carry_var) = (old_s & 1) != 0, old_s >> 1))

/* What DrawTileScript called with the item (and the group's Y) leaves: A $FF, the compare's N clear and Z set, C set, Y as the last op left it, and V when a line step ran. */
static inline Ended ended_of_tile_run(TileRun run, uint8_t item)
{
    Ended e = ended_with_a(ended_nz(0), 0xFF);

    e = ended_with_carry(ended_with_y(e, tile_run_y_shim(run, (uint8_t)((uint8_t)(item << 1) + 1))), true);
    return run.stepped_line ? ended_with_overflow(e, run.line_overflow) : e;
}

#endif
