/* Lot L8: the stage object routines of the overlays at sectors 0259 to 18DD, as typed C. */
#ifndef RONDO_STAGE_ENEMIES_L8_H
#define RONDO_STAGE_ENEMIES_L8_H

#include "machine.h"
#include "enemy_objects.h"
#include "common_objects.h"

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

#endif
