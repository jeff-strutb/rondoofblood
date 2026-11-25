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
Ended FallingDebris(uint8_t slot);
Ended FlashByHeight(uint8_t slot);
Ended StartWithEvent(uint8_t slot);
Ended RaiseStateOnFlag(uint8_t slot);
void StopVerticalSpeed(uint8_t slot);
Jump JumpByStageStep(void);

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


/* A memory shift right in one read and one write: the value written, and the bit shifted out in `carry_var`. */
#define LSR8_VALUE(field, carry_var) \
    RMW8(field, old_s, ((carry_var) = (old_s & 1) != 0, old_s >> 1))

#endif
