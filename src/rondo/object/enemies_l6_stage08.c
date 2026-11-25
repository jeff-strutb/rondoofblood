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

/* The exit of the angle scales (scale_exit_shim): A, N and Z, C, V when an add ran, and Y 0. */
static Ended ended_of_scale(ScaleResult r)
{
    Ended e = ended_with_y(ended_with_carry(ended_with_a(ended_nz(r.nz), r.a), r.carry), 0);

    return r.v_set ? ended_with_overflow(e, r.overflow) : e;
}

/* AngleToPlayer's exit: A, N, Z, C, V and Y. */
static Ended ended_of_angle(AngleExit r)
{
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_load(r.a), r.c), r.v), r.y);
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

/*
 * @routine 088A:39B FrameBySideThenTimerOnScroll
 * @run 70:AB9B
 * WHAT IT DOES: Sets the object's frame to $25 when its X low is under $C0, else 0; once the scroll's Y high byte is not zero it sets OBJ_34B8 to $C0 and goes on to the next state.
 * HOW: Stores, compares, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the scroll's load, or the new state's N and Z (A $C0).
 */
Ended FrameBySideThenTimerOnScroll(uint8_t slot)
{
    uint8_t low, high, state;
    Ended e;

    WR8(OBJ->frame[slot], 0);
    low = RD8(OBJ->x_lo[slot]);
    e = ended_compare(low, 0xC0);
    if (low < 0xC0) {
        WR8(OBJ->frame[slot], 0x25);
        e = then(e, ended_load(0x25));
    }
    high = RD8(SCROLL->y_hi[0]);
    e = then(e, ended_load(high));
    if (high == 0) {
        return e;
    }
    WR8(OBJ->f34b8[slot], 0xC0);
    state = INC8(OBJ->state[slot]);
    return then(then(e, ended_load(0xC0)), ended_nz(state));
}

void abi_FrameBySideThenTimerOnScroll(void)
{
    abi_ended(FrameBySideThenTimerOnScroll(cpu.x));
}

/*
 * @routine 088A:400 StepListACB2ThenNextOnScreen
 * @run 70:AC00
 * WHAT IT DOES: Steps the animation list at $ACB2, moves the object along Y and goes on to the next state once its Y position is on the first screen (high byte not zero and low byte under $C0).
 * HOW: StepObjectFrames, ApplyVelocityY, compares, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the last compare or load, or the new state's N and Z.
 */
Ended StepListACB2ThenNextOnScreen(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    AxisMove move;
    uint8_t y = y_entry, high, low, state;
    Ended e;

    WR8(ZP->temp10, 0xB2);
    WR8(ZP->temp11, 0xAC);
    jsr_push(0xAC0B);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    jsr_push(0xAC0E);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    high = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_load(high));
    if (high == 0) {
        return e;
    }
    low = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(low, 0xC0));
    if (low >= 0xC0) {
        return e;
    }
    state = INC8(OBJ->state[slot]);
    return then(e, ended_nz(state));
}

void abi_StepListACB2ThenNextOnScreen(void)
{
    abi_ended(StepListACB2ThenNextOnScreen(cpu.x, cpu.y));
}

/*
 * @routine 088A:53B FallAndBlinkThenClear
 * @run 70:AD3B
 * WHAT IT DOES: Adds $0008 to the object's Y speed and moves the object. Its frame is 0, or on every other frame of the wave counter OBJ_34B8 plus $47. Counts OBJ_3510 down and clears the object at zero.
 * HOW: A 16-bit add, ApplyVelocity, a test of zTemp wave bit 0, an add, DEC, ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: the count, or as ClearObject.
 */
Ended FallAndBlinkThenClear(uint8_t slot)
{
    AxisMove move;
    Sum low, high, frame;
    uint8_t wave, left;
    Ended e;

    low = add_bytes(RD8(OBJ->vy_lo[slot]), 0x08, false);
    WR8(OBJ->vy_lo[slot], low.value);
    high = add_bytes(RD8(OBJ->vy_hi[slot]), 0x00, low.carry);
    WR8(OBJ->vy_hi[slot], high.value);
    e = ended_of_sum(high);
    jsr_push(0xAD4F);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    WR8(OBJ->frame[slot], 0);
    wave = (uint8_t)(RD8(ZP->wave_counter) & 0x01);
    e = then(e, ended_load(wave));
    if (wave != 0) {
        frame = add_bytes(RD8(OBJ->f34b8[slot]), 0x47, false);
        WR8(OBJ->frame[slot], frame.value);
        e = then(e, ended_of_sum(frame));
    }
    left = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(left));
    if (left == 0) {
        jsr_push(0xAD69);
        ClearObject(slot);
        jsr_pop();
    }
    return e;
}

void abi_FallAndBlinkThenClear(void)
{
    abi_ended(FallAndBlinkThenClear(cpu.x));
}

/*
 * @routine 088A:4FC LoadThenSpeedFromTable
 * @run 70:ACFC
 * WHAT IT DOES: Loads the object's record, takes its X speed from the two-byte entries of the table at $AD21 by OBJ_34B8, its timer from the table at $AD29 and its frame as OBJ_34B8 plus $47.
 * HOW: LoadObjectRecord, table reads (bank 70), an add.
 * WAITS: no.
 * Entry: X object. Exit: A the frame, N, Z, C and V from the add; Y OBJ_34B8.
 */
Ended LoadThenSpeedFromTable(uint8_t slot)
{
    uint8_t nz, last, index, y;
    bool carry;
    Sum sum;
    Ended e;

    jsr_push(0xACFF);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    index = RD8(OBJ->f34b8[slot]);
    y = (uint8_t)(index << 1);
    e = then(e, ended_with_carry(ended_with_y(ended_load(y), y), (index & 0x80) != 0));
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xAD22 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xAD21 + y)));
    y = RD8(OBJ->f34b8[slot]);
    e = then(e, ended_with_y(ended_nz(y), y));
    WR8(OBJ->f3510[slot], rd8((uint16_t)(0xAD29 + y)));
    sum = add_bytes(y, 0x47, false);
    WR8(OBJ->frame[slot], sum.value);
    return then(e, ended_of_sum(sum));
}

void abi_LoadThenSpeedFromTable(void)
{
    abi_ended(LoadThenSpeedFromTable(cpu.x));
}

/*
 * @routine 088B:6C9 PlaceFromTableCopySlot10
 * @run 70:B6C9
 * WHAT IT DOES: Puts the object at the X and Y low bytes the tables at $BB66 and $BBA6 give for the row the table at $B736 names by OBJ_3510, copies the sprite set, frame and facing of the object in slot $10 and sets its flash flags to $10.
 * HOW: Table reads (bank 70), copies.
 * WAITS: no.
 * Entry: X object. Exit: A $10, N and Z from it; Y the row.
 */
Ended PlaceFromTableCopySlot10(uint8_t slot)
{
    uint8_t index, row;
    Ended e;

    index = RD8(OBJ->f3510[slot]);
    row = rd8((uint16_t)(0xB736 + index));
    e = then(ended_with_y(ended_nz(index), index), ended_with_y(ended_load(row), row));
    WR8(OBJ->x_lo[slot], rd8((uint16_t)(0xBB66 + row)));
    WR8(OBJ->y_lo[slot], rd8((uint16_t)(0xBBA6 + row)));
    WR8(OBJ->sprite_set[slot], RD8(OBJ->sprite_set[0x10]));
    WR8(OBJ->frame[slot], RD8(OBJ->frame[0x10]));
    WR8(OBJ->facing_left[slot], RD8(OBJ->facing_left[0x10]));
    WR8(OBJ->flash_flags[slot], 0x10);
    return then(e, ended_load(0x10));
}

void abi_PlaceFromTableCopySlot10(void)
{
    abi_ended(PlaceFromTableCopySlot10(cpu.x));
}

/*
 * @routine 088A:4B9 StartThreeNumberedObjects
 * @run 70:ACB9
 * WHAT IT DOES: Clears the objects in slots $16 to $18, starts a kind $28 object in each at the place of the object in slot $3E, gives each sprite set 7, its number (0 to 2) as OBJ_34B8 and the frame number plus $47, and then clears the object in slot $3E.
 * HOW: A loop of ClearObjectInY and StartObjectAtPlace, ClearObject.
 * WAITS: no.
 * Entry: X object (kept). Exit: the last compare of Y with $19; A the last frame.
 */
Ended StartThreeNumberedObjects(uint8_t slot)
{
    Sum frame;
    uint8_t y, number;
    Ended e = ended_kept();

    push8(slot);
    WR8(ZP->temp00, 0);
    for (y = 0x16; y < 0x19; y++) {
        jsr_push(0xACC3);
        ClearObjectInY(y);
        jsr_pop();
        jsr_push(0xACC8);
        StartObjectAtPlace(0x28, 0x3E, y);
        jsr_pop();
        WR8(OBJ->sprite_set[y], 7);
        number = RD8(ZP->temp00);
        WR8(OBJ->f34b8[y], number);
        frame = add_bytes(number, 0x47, false);
        WR8(OBJ->frame[y], frame.value);
        INC8(ZP->temp00);
        e = then(ended_of_sum(frame), ended_with_y(ended_with_carry(ended_nz((uint8_t)(y + 1 - 0x19)), (uint8_t)(y + 1) >= 0x19), (uint8_t)(y + 1)));
    }
    jsr_push(0xACE2);
    ClearObject(0x3E);
    jsr_pop();
    slot = pull8();
    return then(e, ended_nz(slot));
}

void abi_StartThreeNumberedObjects(void)
{
    uint8_t slot = cpu.x;

    abi_ended(StartThreeNumberedObjects(slot));
}

/*
 * @routine 088B:705 PlaceFromTableThenStepList
 * @run 70:B705
 * WHAT IT DOES: Puts the object at the X and Y low bytes the tables at $BB66 and $BBA6 give for the row the table at $B73A names by OBJ_3510, sets sprite set 7, copies the facing of the object in slot $10 and steps the animation list the table at $B73E gives for OBJ_3510.
 * HOW: Table reads (bank 70), StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y overwritten. Exit: as StepObjectFrames.
 */
Ended PlaceFromTableThenStepList(uint8_t slot)
{
    FrameStep step;
    uint8_t index, row, y, timer;
    Ended e;

    index = RD8(OBJ->f3510[slot]);
    row = rd8((uint16_t)(0xB73A + index));
    e = then(ended_with_y(ended_nz(index), index), ended_with_y(ended_load(row), row));
    WR8(OBJ->x_lo[slot], rd8((uint16_t)(0xBB66 + row)));
    WR8(OBJ->y_lo[slot], rd8((uint16_t)(0xBBA6 + row)));
    WR8(OBJ->sprite_set[slot], 7);
    WR8(OBJ->facing_left[slot], RD8(OBJ->facing_left[0x10]));
    timer = RD8(OBJ->f3510[slot]);
    y = (uint8_t)(timer << 1);
    e = then(e, ended_with_carry(ended_with_y(ended_load(y), y), (timer & 0x80) != 0));
    WR8(ZP->temp10, rd8((uint16_t)(0xB73E + y)));
    WR8(ZP->temp11, rd8((uint16_t)(0xB73F + y)));
    jsr_push(0xB735);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    return then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
}

void abi_PlaceFromTableThenStepList(void)
{
    abi_ended(PlaceFromTableThenStepList(cpu.x));
}

/*
 * @routine 088C:43C StartThenWaitForField83
 * @run 70:BC3C
 * WHAT IT DOES: In state 0 loads the object's record and sets frame 2 and sprite set 6. In state 1, once zField83 is not zero, it sets the scroll Y high to 1, frame $46, sprite set 7, X low $80, Y low $58 and goes on to the next state.
 * HOW: LoadObjectRecord, a DEC, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the DEC, the load of zField83, or the new state's N and Z (A $58).
 */
Ended StartThenWaitForField83(uint8_t slot)
{
    uint8_t state, nz, last, count, field, next;
    bool carry;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    count = state;
    if (state == 0) {
        jsr_push(0xBC44);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        e = then(e, ended_of_record(last, nz, carry));
        WR8(OBJ->frame[slot], 2);
        WR8(OBJ->sprite_set[slot], 6);
        e = then(then(e, ended_load(2)), ended_load(6));
        count = 6;
    }
    count--;
    e = then(e, ended_load(count));
    if (count != 0) {
        return e;
    }
    field = RD8(ZP->field83);
    e = then(e, ended_load(field));
    if (field == 0) {
        return e;
    }
    WR8(SCROLL->y_hi[0], 1);
    WR8(OBJ->frame[slot], 0x46);
    WR8(OBJ->sprite_set[slot], 7);
    WR8(OBJ->x_lo[slot], 0x80);
    WR8(OBJ->y_lo[slot], 0x58);
    next = INC8(OBJ->state[slot]);
    return then(then(e, ended_load(0x58)), ended_nz(next));
}

void abi_StartThenWaitForField83(void)
{
    abi_ended(StartThenWaitForField83(cpu.x));
}

/*
 * @routine 088A:699 LoadThenStartFiveFromTable
 * @run 70:AE99
 * WHAT IT DOES: Loads the object's record, clears OBJ_3778 and its Y speed, sets OBJ_34B8 to $30 and bit 2 of zField8E, copies its X and Y low bytes into all 64 entries of the two tables at $BB66 and $BBA6, then clears the objects in slots $11 to $15 and starts in each an object whose kind the table at $AEDA gives, at this object's place.
 * HOW: LoadObjectRecord, two loops, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: the last compare of Y with $16.
 */
Ended LoadThenStartFiveFromTable(uint8_t slot)
{
    uint8_t nz, last, flags, y, kind, placed = 0;
    bool carry;
    Ended e;

    jsr_push(0xAE9C);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->f3778[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->f34b8[slot], 0x30);
    flags = (uint8_t)(RD8(ZP->field8e) | 0x04);
    WR8(ZP->field8e, flags);
    for (y = 0; y < 0x40; y++) {
        wr8((uint16_t)(0xBB66 + y), RD8(OBJ->x_lo[slot]));
        wr8((uint16_t)(0xBBA6 + y), RD8(OBJ->y_lo[slot]));
    }
    for (y = 0x11; y < 0x16; y++) {
        jsr_push(0xAEC7);
        ClearObjectInY(y);
        jsr_pop();
        push8(y);
        kind = rd8((uint16_t)(0xAEDA + (y - 0x11)));
        y = pull8();
        jsr_push(0xAED4);
        placed = StartObjectAtPlace(kind, slot, y);
        jsr_pop();
    }
    e = then(e, ended_load(flags));
    return then(e, ended_with_overflow(ended_with_y(ended_with_a(ended_with_carry(ended_nz(0), true), placed), 0x16), false));
}

void abi_LoadThenStartFiveFromTable(void)
{
    abi_ended(LoadThenStartFiveFromTable(cpu.x));
}

/*
 * @routine 088B:098 FallThenSoundAtPeak
 * @run 70:B098
 * WHAT IT DOES: Applies gravity and moves the object; once its Y speed high byte is not negative it sends sound $9A and goes on to the next OBJ_2800.
 * HOW: ApplyGravity, ApplyVelocity, SoundCommand, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound's push. Exit: see the wrapper.
 */
SoundEnd FallThenSoundAtPeak(uint8_t slot, uint8_t y_entry)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    GravityResult gravity;
    AxisMove move;
    uint8_t speed, phase;
    Ended e;

    jsr_push(0xB09B);
    gravity = ApplyGravity(slot);
    jsr_pop();
    jsr_push(0xB09E);
    move = ApplyVelocity(slot);
    jsr_pop();
    (void)gravity;
    speed = RD8(OBJ->vy_hi[slot]);
    e = then(then(ended_of_axis_move(move), ended_load(speed)), ended_kept());
    if ((speed & 0x80) != 0) {
        r.end = e;
        return r;
    }
    r.before = then(e, ended_load(0x9A));
    r.sounded = true;
    r.command = 0x9A;
    jsr_push(0xB0A8);
    r.call = SoundCommand(0x9A, slot, y_entry);
    jsr_pop();
    phase = INC8(OBJ->f2800[slot]);
    r.end = ended_nz(phase);
    return r;
}

void abi_FallThenSoundAtPeak(void)
{
    abi_sound_end(FallThenSoundAtPeak(cpu.x, cpu.y));
}

/*
 * @routine 088B:0E8 SpeedFromRandomRow
 * @run 70:B0E8
 * WHAT IT DOES: Takes the object's two speeds (four bytes: X low and high, Y low and high) from the row of the table at $B109 that a random number from 0 to 7 picks.
 * HOW: NextRandom, table reads (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: A the last byte read, Y the row offset, C clear.
 */
Ended SpeedFromRandomRow(uint8_t slot)
{
    RandomResult random;
    uint8_t y, value;

    jsr_push(0xB0EB);
    random = NextRandom();
    jsr_pop();
    y = (uint8_t)((random.value & 0x07) << 2);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB10A + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB109 + y)));
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(0xB10C + y)));
    value = rd8((uint16_t)(0xB10B + y));
    WR8(OBJ->vy_lo[slot], value);
    return then(then(ended_of_random(random), ended_with_carry(ended_with_y(ended_load((uint8_t)(random.value & 0x07)), y), false)), ended_load(value));
}

void abi_SpeedFromRandomRow(void)
{
    abi_ended(SpeedFromRandomRow(cpu.x));
}

/*
 * @routine 088A:63F SetHitBoxFromFrameTable
 * @run 70:AE3F
 * WHAT IT DOES: Takes the object's hit box (OBJ_3930, OBJ_39E0, OBJ_3988, OBJ_3A38) from the four-byte row of the table at $B8F7 that its frame less one names.
 * HOW: Table reads (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: A the last byte read, Y the row offset.
 */
Ended SetHitBoxFromFrameTable(uint8_t slot)
{
    Sum row;
    uint8_t y, value;
    Ended e;

    row = sub_bytes(RD8(OBJ->frame[slot]), 1, true);
    y = (uint8_t)(row.value << 2);
    e = then(ended_of_sum(row), ended_with_carry(ended_with_y(ended_load(y), y), (row.value & 0x40) != 0));
    WR8(OBJ->hitbox_dx[slot], rd8((uint16_t)(0xB8F7 + y)));
    WR8(OBJ->hitbox_dy[slot], rd8((uint16_t)(0xB8F8 + y)));
    WR8(OBJ->hitbox_half_w[slot], rd8((uint16_t)(0xB8F9 + y)));
    value = rd8((uint16_t)(0xB8FA + y));
    WR8(OBJ->hitbox_half_h[slot], value);
    return then(e, ended_load(value));
}

void abi_SetHitBoxFromFrameTable(void)
{
    abi_ended(SetHitBoxFromFrameTable(cpu.x));
}

/*
 * @routine 088B:7AF FollowSlot10WithHitBox
 * @run 70:B7AF
 * WHAT IT DOES: In state 0 loads the object's record. Later it takes the place, facing and OBJ_3250 of the object in slot $10 and a hit box from the four-byte row of the table at $B9DF that slot's frame less one names; with a zero half width and half height it clears OBJ_3250. When the object in slot $10 is no longer of kind $20 this object is cleared.
 * HOW: LoadObjectRecord, CopyObjectPlaceBack, table reads (bank 70), ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: the compare with $20, or as ClearObject.
 */
Ended FollowSlot10WithHitBox(uint8_t slot)
{
    Sum row;
    uint8_t state, nz, last, y, value, half_w, kind;
    bool carry;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        return then(ended_load(state), ended_of_record(last, nz, carry));
    }
    jsr_push(0xB7BC);
    last = CopyObjectPlaceBack(0x10, slot);
    jsr_pop();
    e = then(then(ended_load(state), ended_with_y(ended_load(last), 0x10)), ended_nz(last));
    row = sub_bytes(RD8(OBJ->frame[0x10]), 1, true);
    y = (uint8_t)(row.value << 2);
    e = then(then(e, ended_of_sum(row)), ended_with_carry(ended_with_y(ended_load(y), y), (row.value & 0x40) != 0));
    WR8(OBJ->hitbox_dx[slot], rd8((uint16_t)(0xB9DF + y)));
    WR8(OBJ->hitbox_dy[slot], rd8((uint16_t)(0xB9E0 + y)));
    WR8(OBJ->hitbox_half_w[slot], rd8((uint16_t)(0xB9E1 + y)));
    value = rd8((uint16_t)(0xB9E2 + y));
    WR8(OBJ->hitbox_half_h[slot], value);
    e = then(e, ended_load(value));
    value = RD8(OBJ->facing_left[0x10]);
    WR8(OBJ->facing_left[slot], value);
    e = then(e, ended_load(value));
    value = RD8(OBJ->hitbox_on[0x10]);
    WR8(OBJ->hitbox_on[slot], value);
    e = then(e, ended_load(value));
    half_w = RD8(OBJ->hitbox_half_w[slot]);
    value = (uint8_t)(half_w | RD8(OBJ->hitbox_half_h[slot]));
    e = then(e, ended_load(value));
    if (value == 0) {
        WR8(OBJ->hitbox_on[slot], 0);
    }
    kind = RD8(OBJ->kind[0x10]);
    e = then(e, ended_compare(kind, 0x20));
    if (kind != 0x20) {
        ClearObject(slot);
    }
    return e;
}

void abi_FollowSlot10WithHitBox(void)
{
    abi_ended(FollowSlot10WithHitBox(cpu.x));
}

/*
 * @routine 088B:285 SoundThenAimRandomLength
 * @run 70:B285
 * WHAT IT DOES: Sends sound $2B, then sets the object's speed along the angle to the player with a length from a table at $B2A7 by a random 0 to 3, and goes on to the next OBJ_2800.
 * HOW: SoundCommand, AngleToPlayer, NextRandom, SetVelocityFromAngle, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound's push. Exit: see the wrapper.
 */
SoundEnd SoundThenAimRandomLength(uint8_t slot, uint8_t y_entry)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    AngleExit angle;
    RandomResult random;
    ScaleResult scale;
    uint8_t index, phase;
    Ended e;

    r.before = ended_load(0x2B);
    r.sounded = true;
    r.command = 0x2B;
    jsr_push(0xB28A);
    r.call = SoundCommand(0x2B, slot, y_entry);
    jsr_pop();
    jsr_push(0xB28D);
    angle = AngleToPlayer(slot);
    jsr_pop();
    e = ended_of_angle(angle);
    push8(angle.a);
    jsr_push(0xB291);
    random = NextRandom();
    jsr_pop();
    index = (uint8_t)((random.value & 0x03) << 1);
    e = then(then(e, ended_of_random(random)), ended_with_carry(ended_with_y(ended_load((uint8_t)(random.value & 0x03)), index), false));
    WR8(ZP->temp00, rd8((uint16_t)(0xB2A7 + index)));
    WR8(ZP->temp01, rd8((uint16_t)(0xB2A8 + index)));
    angle.a = pull8();
    e = then(e, ended_load(angle.a));
    jsr_push(0xB2A3);
    scale = SetVelocityFromAngle(angle.a, slot);
    jsr_pop();
    e = then(e, ended_of_scale(scale));
    phase = INC8(OBJ->f2800[slot]);
    r.end = then(e, ended_nz(phase));
    return r;
}

void abi_SoundThenAimRandomLength(void)
{
    abi_sound_end(SoundThenAimRandomLength(cpu.x, cpu.y));
}
