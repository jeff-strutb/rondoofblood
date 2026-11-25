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

#endif
