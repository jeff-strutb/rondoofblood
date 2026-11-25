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
Ended StepVerticalScrollTable(uint8_t slot);
Ended CountdownThenStartScript80(uint8_t slot);
Ended LoadRecordPlaceByKind(uint8_t slot);
Ended LoadRecordAtPlayerPlace(uint8_t slot);
Ended LoadRecordThenSinkByF3520(uint8_t slot);
Ended LoadRecordThenStepListByKind(uint8_t slot, uint8_t y_entry);
Ended LoadRecordSinkThenStepListByKind(uint8_t slot, uint8_t y_entry);
SoundEnd StateMachineWithSoundAndList(uint8_t slot, uint8_t y_entry);
Ended DriftRecordThenMoveWithJitter(uint8_t slot);
Ended SwingByCosineFromState0B(uint8_t slot);
Ended StartRandomScriptWhenFlagged(uint8_t slot);
Ended PushPlayerWhenFlagged(uint8_t slot);
SoundEnd HitSoundThenSpawnPieces(uint8_t slot, uint8_t y_entry);
Ended EnterState0BWhenFlagged(uint8_t slot);
Ended StartPieceAtTopLeft(uint8_t slot);
Ended CountDownThenStartScripts(uint8_t slot);
SoundEnd CountDownWithSound77(uint8_t slot, uint8_t y_entry);
Ended StartScriptAtTimerMarks(uint8_t slot);
Ended SpinOutThenLeave(uint8_t slot, uint8_t y_entry);
Ended SpinOnCircle(uint8_t slot);
SoundEnd StepListThenBurstAtFrame18(uint8_t slot, uint8_t y_entry);
SoundEnd HomeOnPointThenRingAndSound(uint8_t slot, uint8_t y_entry);

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
Ended HitDamage0519(uint8_t slot);
Ended HitDamage05C9(uint8_t slot);
Ended HitDamage0729(uint8_t slot);
Ended HitDamage07D9(uint8_t slot);
Ended HitDamage0889(uint8_t slot);
Ended HitDamage0939(uint8_t slot);
Ended HitDamage0A99(uint8_t slot);
Ended HitDamage0B49(uint8_t slot);
Ended HitDamage0BF9(uint8_t slot);
Ended HitDamage0CA9(uint8_t slot);
Ended HitDamage0D59(uint8_t slot);
Ended HitDamage0E09(uint8_t slot);
Ended HitDamage0EB9(uint8_t slot);
Ended HitDamage1019(uint8_t slot);
Ended HitDamage10C9(uint8_t slot);
Ended HitDamage1179(uint8_t slot);
Ended HitDamage16C9(uint8_t slot);

/* object/enemies_l8_st14.c */
Ended LoadRecordFlashByNumberThenMoveOrClear(uint8_t slot, uint8_t y_entry);
Ended LoadRecordRandomLiftThenMoveOrClear(uint8_t slot, uint8_t y_entry);

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
Ended RecordEventWhenSlot10InState0F(uint8_t slot);
Ended FaceThePlayerByStateTableB(uint8_t slot);
Ended MaybeDropItemByPlayerDistance(uint8_t slot);
Ended HitBoxByFrameTableCE8F(uint8_t slot);
Ended StepListFacePlayerUntilNear(uint8_t slot, uint8_t y_entry);
Ended FacePlayerThenPickPhase(uint8_t slot);
SoundEnd LeapByPlayerDistance(uint8_t slot, uint8_t y_entry);
Ended CountDownThenClearPhase(uint8_t slot);

/* object/enemies_l8_st21.c */
Ended LoadRecordSplitIntoFourAndClear(uint8_t slot);
Ended LoadRecordThenFollowScroll(uint8_t slot);
Ended StartPieceWithSpeedFromTable(uint8_t slot);
Ended LoadRecordFlipByPlayerThenFall(uint8_t slot);
Ended LoadRecordThenClearAtSubStep3(uint8_t slot);
SoundEnd SoundWhenHurtThenFlag(uint8_t slot, uint8_t y_entry);
SoundEnd SoundAndState6WhenFlagged(uint8_t slot, uint8_t y_entry);
Ended LoadRecordFrame0D(uint8_t slot);
Ended ClearFourAtScreen2(uint8_t slot);
Ended MoveUpUntilY40(uint8_t slot);
Ended CountDownThenCopyPhase(uint8_t slot);
Ended ResetPieceAndAdvance(uint8_t slot);
SoundEnd BreakIntoPiecesAtFifthHit(uint8_t slot, uint8_t y_entry);

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
Ended MaybeDropItemByPlayerDistance1019(uint8_t slot);
SoundEnd PlaySound58A(uint8_t slot, uint8_t y_entry);
SoundEnd PlaySound58B(uint8_t slot, uint8_t y_entry);
Ended SettleWhenLowOnFirstRow(uint8_t slot);
Ended LoadRecordFrameFromKind(uint8_t slot);
Ended CapPlayerXOnScreen1(uint8_t slot);
Ended LoadRecordThenDriftAndCount(uint8_t slot);
Ended LoadRecordAtX80(uint8_t slot);

/* object/enemies_l8_st27.c */
Ended BossFrameRoutines(uint8_t slot, uint8_t y_entry);
Ended HurtSoundsThenRunTable(uint8_t slot, uint8_t y_entry);
Ended BossFrameRoutinesB(uint8_t slot, uint8_t y_entry);
Ended SpawnRowsWhenScrollAtZero(uint8_t slot);

#endif
