/*
 * TEMPORARY: the stage 0841 object routines of lot L6 (enemy kinds not yet identified), by routine. Bodies return how the routine's last
 * instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
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

/*
 * @routine 088B:6C5 ClearFrame
 * @run 70:B6C5
 * WHAT IT DOES: Sets the object's frame to 0.
 * HOW: A store.
 * WAITS: no.
 * Entry: X object. Exit: all kept.
 */
Ended ClearFrame(uint8_t slot)
{
    WR8(OBJ->frame[slot], 0);
    return ended_kept();
}

void abi_ClearFrame(void)
{
    abi_ended(ClearFrame(cpu.x));
}

/*
 * @routine 088B:21D TurnAroundInState9
 * @run 70:B21D
 * WHAT IT DOES: Sets the object's state to 9 and reverses its X speed (a tail jump into NegateVelocityX).
 * HOW: A store, NegateVelocityX.
 * WAITS: no.
 * Entry: X object. Exit: as NegateVelocityX.
 */
Ended TurnAroundInState9(uint8_t slot)
{
    WR8(OBJ->state[slot], 9);
    return then(ended_load(9), ended_of_sum(NegateVelocityX(slot)));
}

void abi_TurnAroundInState9(void)
{
    abi_ended(TurnAroundInState9(cpu.x));
}

/*
 * @routine 088B:225 FallBackInState9
 * @run 70:B225
 * WHAT IT DOES: Sets the object's state to 9 and reverses its Y speed (a tail jump into NegateVelocityY).
 * HOW: A store, NegateVelocityY.
 * WAITS: no.
 * Entry: X object. Exit: as NegateVelocityY.
 */
Ended FallBackInState9(uint8_t slot)
{
    WR8(OBJ->state[slot], 9);
    return then(ended_load(9), ended_of_sum(NegateVelocityY(slot)));
}

void abi_FallBackInState9(void)
{
    abi_ended(FallBackInState9(cpu.x));
}

/*
 * @routine 088A:71E PlaySoundTwo
 * @run 70:AF1E
 * WHAT IT DOES: Plays sound 2.
 * HOW: SoundCommand.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: see abi_sound_end.
 */
SoundEnd PlaySoundTwo(uint8_t slot, uint8_t y_entry)
{
    SoundEnd r = { { 0 }, { 0 }, true, 2, { 0 } };

    r.before = ended_load(2);
    jsr_push(0xAF23);
    r.call = SoundCommand(2, slot, y_entry);
    jsr_pop();
    r.end = ended_kept();
    return r;
}

void abi_PlaySoundTwo(void)
{
    abi_sound_end(PlaySoundTwo(cpu.x, cpu.y));
}

/*
 * @routine 088A:31C LoadThenSpeedF0
 * @run 70:AB1C
 * WHAT IT DOES: Loads the object's record and sets its X speed high to $F0.
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: A $F0, N set, Z clear; C and Y from the record.
 */
Ended LoadThenSpeedF0(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;
    Ended e;

    jsr_push(0xAB1F);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->vx_hi[slot], 0xF0);
    return then(e, ended_load(0xF0));
}

void abi_LoadThenSpeedF0(void)
{
    abi_ended(LoadThenSpeedF0(cpu.x));
}

/*
 * @routine 088A:325 NextStateOnScrollY
 * @run 70:AB25
 * WHAT IT DOES: Goes on to the next state when the scroll's Y high byte is not zero.
 * HOW: A read and an INC.
 * WAITS: no.
 * Entry: X object. Exit: the load, or the new state's N and Z.
 */
Ended NextStateOnScrollY(uint8_t slot)
{
    uint8_t high, state;

    high = RD8(SCROLL->y_hi[0]);
    if (high == 0) {
        return ended_load(high);
    }
    state = INC8(OBJ->state[slot]);
    return then(ended_load(high), ended_nz(state));
}

void abi_NextStateOnScrollY(void)
{
    abi_ended(NextStateOnScrollY(cpu.x));
}

/*
 * @routine 088A:765 StepListByIndex088A
 * @run 70:AF65
 * WHAT IT DOES: Steps the animation list whose address the table at $AF72 gives for the index in Y.
 * HOW: Two table reads (bank 70), a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y the table index. Exit: as StepObjectFrames.
 */
Ended StepListByIndex088A(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    uint8_t y = y_entry;

    WR8(ZP->temp10, rd8((uint16_t)(0xAF72 + y)));
    WR8(ZP->temp11, rd8((uint16_t)(0xAF73 + y)));
    step = StepObjectFrames(slot, &y);
    return ended_of_frame_step(step, step != FRAME_HELD, y);
}

void abi_StepListByIndex088A(void)
{
    abi_ended(StepListByIndex088A(cpu.x, cpu.y));
}

/*
 * @routine 088B:777 SetTimersOf12To15
 * @run 70:B777
 * WHAT IT DOES: Sets OBJ_34B8 of the objects 18 to 21 to 1.
 * HOW: A loop of Y from 18.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, Y 22, the compare of Y with 22.
 */
Ended SetTimersOf12To15(void)
{
    uint8_t y;

    for (y = 0x12; y < 0x16; y++) {
        WR8(OBJ->f34b8[y], 1);
    }
    return then(ended_load(1), ended_with_y(ended_with_carry(ended_nz(0), true), 0x16));
}

void abi_SetTimersOf12To15(void)
{
    abi_ended(SetTimersOf12To15());
}

/*
 * @routine 088A:52D CountThenNextStateF3510
 * @run 70:AD2D
 * WHAT IT DOES: Counts OBJ_3510 down; at zero it sets it to $30 and goes on to the next state.
 * HOW: DEC, a store, INC.
 * WAITS: no.
 * Entry: X object. Exit: the count, or the new state's N and Z (A $30).
 */
Ended CountThenNextStateF3510(uint8_t slot)
{
    uint8_t left, state;
    Ended e;

    left = DEC8(OBJ->f3510[slot]);
    e = ended_nz(left);
    if (left != 0) {
        return e;
    }
    WR8(OBJ->f3510[slot], 0x30);
    state = INC8(OBJ->state[slot]);
    return then(then(e, ended_load(0x30)), ended_nz(state));
}

void abi_CountThenNextStateF3510(void)
{
    abi_ended(CountThenNextStateF3510(cpu.x));
}

/*
 * @routine 088A:724 SetFrameThenCountToNext
 * @run 70:AF24
 * WHAT IT DOES: Sets the object's frame (OBJ_2A68) to $0C and counts OBJ_3568 down; at zero it sets it to $10 and goes on to the next state.
 * HOW: A store, DEC, a store, INC.
 * WAITS: no.
 * Entry: X object. Exit: the count, or the new state's N and Z (A $10).
 */
Ended SetFrameThenCountToNext(uint8_t slot)
{
    uint8_t left, state;
    Ended e;

    WR8(OBJ->frame[slot], 0x0C);
    left = DEC8(OBJ->f3568[slot]);
    e = then(ended_load(0x0C), ended_nz(left));
    if (left != 0) {
        return e;
    }
    WR8(OBJ->f3568[slot], 0x10);
    state = INC8(OBJ->state[slot]);
    return then(then(e, ended_load(0x10)), ended_nz(state));
}

void abi_SetFrameThenCountToNext(void)
{
    abi_ended(SetFrameThenCountToNext(cpu.x));
}

/*
 * @routine 088A:41E StartListSoundThenSpawn20
 * @run 70:AC1E
 * WHAT IT DOES: Starts the script list $83 on channel 4, starts a kind $20 object in slot $10 at this object's place and clears this object.
 * HOW: StartListChannelKind2, StartObjectAtPlace, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: as ClearObject.
 */
Ended StartListSoundThenSpawn20(uint8_t slot)
{
    uint8_t placed;
    Ended e;

    push8(slot);
    jsr_push(0xAC26);
    StartListChannelKind2(0x83, 4);
    jsr_pop();
    slot = pull8();
    jsr_push(0xAC2E);
    placed = StartObjectAtPlace(0x20, slot, 0x10);
    jsr_pop();
    e = ended_with_y(ended_load(placed), 0x10);
    ClearObject(slot);
    return e;
}

void abi_StartListSoundThenSpawn20(void)
{
    abi_ended(StartListSoundThenSpawn20(cpu.x));
}

/*
 * @routine 088B:084 StartFallWithFrame21
 * @run 70:B084
 * WHAT IT DOES: Zeroes the object's X speed, sets its Y speed high to $FD and its frame to $21 and goes on to the next OBJ_2800.
 * HOW: Stores and an INC.
 * WAITS: no.
 * Entry: X object. Exit: A $21, N and Z from the new OBJ_2800.
 */
Ended StartFallWithFrame21(uint8_t slot)
{
    uint8_t phase;

    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0xFD);
    WR8(OBJ->frame[slot], 0x21);
    phase = INC8(OBJ->f2800[slot]);
    return then(ended_load(0x21), ended_nz(phase));
}

void abi_StartFallWithFrame21(void)
{
    abi_ended(StartFallWithFrame21(cpu.x));
}

/*
 * @routine 088B:240 SpeedBySideOfScreen
 * @run 70:B240
 * WHAT IT DOES: Sets OBJ_3510 to 2, takes the X speed high from the two-byte table at $B259 by bit 7 of the object's X low and clears the X speed low, then moves the object.
 * HOW: Two ROLs, a table read (bank 70), a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object, C passed into the first ROL. Exit: as ApplyVelocity, Y the table index.
 */
Ended SpeedBySideOfScreen(uint8_t slot, bool carry_in)
{
    AxisMove move;
    uint8_t low, first, second, index, speed;
    bool carry_one, carry_two;

    WR8(OBJ->f3510[slot], 2);
    low = RD8(OBJ->x_lo[slot]);
    first = (uint8_t)((low << 1) | (carry_in ? 1 : 0));
    carry_one = (low & 0x80) != 0;
    second = (uint8_t)((first << 1) | (carry_one ? 1 : 0));
    carry_two = (first & 0x80) != 0;
    (void)carry_two;
    index = (uint8_t)(second & 0x01);
    speed = rd8((uint16_t)(0xB259 + index));
    WR8(OBJ->vx_hi[slot], speed);
    WR8(OBJ->vx_lo[slot], 0);
    move = ApplyVelocity(slot);
    return ended_with_y(ended_of_axis_move(move), index);
}

void abi_SpeedBySideOfScreen(void)
{
    abi_ended(SpeedBySideOfScreen(cpu.x, (cpu.p & P_C) != 0));
}

/*
 * @routine 088A:359 StartThenFallAlongList
 * @run 70:AB59
 * WHAT IT DOES: In state 0 loads the object's record. Later it steps the animation list at $AB72, applies gravity without a limit and moves the object.
 * HOW: LoadObjectRecord, StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: state 0: as LoadObjectRecord; else as ApplyVelocity.
 */
Ended StartThenFallAlongList(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    GravityResult gravity;
    AxisMove move;
    uint8_t state, nz, last, y = y_entry;
    bool carry;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        return then(ended_load(state), ended_of_record(last, nz, carry));
    }
    WR8(ZP->temp11, 0xAB);
    WR8(ZP->temp10, 0x72);
    jsr_push(0xAB6C);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    jsr_push(0xAB6F);
    gravity = ApplyGravityUnlimited(slot);
    jsr_pop();
    move = ApplyVelocity(slot);
    (void)gravity;
    return then(then(ended_load(state), ended_of_frame_step(step, step != FRAME_HELD, y)), ended_with_y(ended_of_axis_move(move), y));
}

void abi_StartThenFallAlongList(void)
{
    abi_ended(StartThenFallAlongList(cpu.x, cpu.y));
}
