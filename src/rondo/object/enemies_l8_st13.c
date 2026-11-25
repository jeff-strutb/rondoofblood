/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 025C (enemy kinds not yet identified), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l8.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/*
 * @routine 025C:131 StepListThenStartChannel85
 * @run 70:B931
 * WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero starts list $85 on channel 5, sets OBJ_3670 5, OBJ_3510 $7B, the byte before the frame $2F and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, a memory decrement, StartListChannelKind2 between a PHX and a PLX, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListThenStartChannel85(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, timer;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xB6DA, 0xB93C, false, &y, &step);

    timer = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(timer));
    if (timer != 0) {
        return e;
    }
    e = then(e, l8_list_channel2(slot, 0x85, 5, 0xB949));
    WR8(OBJ->f3670[slot], 5);
    WR8(OBJ->f3510[slot], 0x7B);
    WR8(OBJ->frame[slot - 1], 0x2F);
    e = then(e, ended_load(0x2F));
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_StepListThenStartChannel85(void)
{
    abi_ended(StepListThenStartChannel85(cpu.x, cpu.y));
}

/*
 * @routine 025C:15D StepListThenStopAnimation
 * @run 70:B95D
 * WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero clears OBJ_3670 and the animation step and timer and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, a memory decrement, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListThenStopAnimation(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, timer;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xB6DA, 0xB968, false, &y, &step);

    timer = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(timer));
    if (timer != 0) {
        return e;
    }
    WR8(OBJ->f3670[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_StepListThenStopAnimation(void)
{
    abi_ended(StepListThenStopAnimation(cpu.x, cpu.y));
}

/*
 * @routine 025C:17A StepListB995ThenWait
 * @run 70:B97A
 * WHAT IT DOES: Steps the animation list at $B995; when it ends sets OBJ_3670 6 and OBJ_3510 $3A and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListB995ThenWait(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xB995, 0xB985, false, &y, &step);

    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->f3670[slot], 6);
    WR8(OBJ->f3510[slot], 0x3A);
    e = then(e, ended_load(0x3A));
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_StepListB995ThenWait(void)
{
    abi_ended(StepListB995ThenWait(cpu.x, cpu.y));
}

/*
 * @routine 025C:19C StepListThenStartType2D
 * @run 70:B99C
 * WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero clears slot $50, starts an object of type $2D there at the object's place, sets OBJ_3670 7 and OBJ_3510 $83 and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, a memory decrement, ClearObjectInY, StartObjectAtPlace, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListThenStartType2D(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, timer, placed;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xB6DA, 0xB9A7, false, &y, &step);

    timer = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(timer));
    if (timer != 0) {
        return e;
    }
    jsr_push(0xB9B1);
    ClearObjectInY(0x50);
    jsr_pop();
    jsr_push(0xB9B6);
    placed = StartObjectAtPlace(0x2D, slot, 0x50);
    jsr_pop();
    e = then(e, ended_with_y(ended_nz(0x50), 0x50));
    e = then(e, ended_load(placed));
    WR8(OBJ->f3670[slot], 7);
    WR8(OBJ->f3510[slot], 0x83);
    e = then(e, ended_load(0x83));
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_StepListThenStartType2D(void)
{
    abi_ended(StepListThenStartType2D(cpu.x, cpu.y));
}

/*
 * @routine 025C:1C4 StepListThenClearAnimation
 * @run 70:B9C4
 * WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero clears the animation step and timer and OBJ_3670 and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, a memory decrement, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListThenClearAnimation(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, timer;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xB6DA, 0xB9CF, false, &y, &step);

    timer = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(timer));
    if (timer != 0) {
        return e;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f3670[slot], 0);
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_StepListThenClearAnimation(void)
{
    abi_ended(StepListThenClearAnimation(cpu.x, cpu.y));
}

/*
 * @routine 025C:1E1 StepListThenStartType2E
 * @run 70:B9E1
 * WHAT IT DOES: Steps the animation list at $BA0E; when it ends clears OBJ_3720, sets OBJ_3670 8, OBJ_34B8 1 and OBJ_3510 $80, goes to the next OBJ_2800, clears slot $30 and starts an object of type $2E there at the object's place.
 * HOW: StepObjectFrames, stores, an INC, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListThenStartType2E(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, placed;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xBA0E, 0xB9EC, false, &y, &step);

    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->f3720[slot], 0);
    WR8(OBJ->f3670[slot], 8);
    WR8(OBJ->f34b8[slot], 1);
    WR8(OBJ->f3510[slot], 0x80);
    e = then(e, ended_load(0x80));
    e = then(e, ended_nz(INC8(OBJ->f2800[slot])));
    jsr_push(0xBA08);
    ClearObjectInY(0x30);
    jsr_pop();
    jsr_push(0xBA0D);
    placed = StartObjectAtPlace(0x2E, slot, 0x30);
    jsr_pop();
    e = then(e, ended_with_y(ended_nz(0x30), 0x30));
    return then(e, ended_load(placed));
}

void abi_StepListThenStartType2E(void)
{
    abi_ended(StepListThenStartType2E(cpu.x, cpu.y));
}

/*
 * @routine 025C:215 StepListThenSoundAndStartChannel86
 * @run 70:BA15
 * WHAT IT DOES: Steps the animation list at $B6DA; when the 16-bit timer OBJ_34B8:OBJ_3510 counts down to zero starts list $86 on channel 5, plays sound $76, clears the animation, OBJ_35C0, OBJ_38D8 and OBJ_3670, sets OBJ_3880 2 and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, a 16-bit subtract in memory, StartListChannelKind2 between a PHX and a PLX, SoundCommand, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
SoundEnd StepListThenSoundAndStartChannel86(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, low, high, again, joined;
    FrameStep step;
    Sum diff;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e = l8_step_list(slot, 0xB6DA, 0xBA20, false, &y, &step);

    low = RD8(OBJ->f3510[slot]);
    diff = sub_bytes(low, 1, true);
    WR8(OBJ->f3510[slot], diff.value);
    high = RD8(OBJ->f34b8[slot]);
    diff = sub_bytes(high, 0, diff.carry);
    WR8(OBJ->f34b8[slot], diff.value);
    again = RD8(OBJ->f3510[slot]);
    joined = (uint8_t)(diff.value | again);
    e = then(then(e, ended_of_sum(diff)), ended_load(joined));
    if (joined != 0) {
        r.end = e;
        return r;
    }
    e = then(e, l8_list_channel2(slot, 0x86, 5, 0xBA3E));
    r.sounded = true;
    r.command = 0x76;
    r.before = then(e, ended_load(0x76));
    jsr_push(0xBA44);
    r.call = SoundCommand(0x76, slot, y);
    jsr_pop();
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f35c0[slot], 0);
    WR8(OBJ->f3880[slot], 2);
    WR8(OBJ->f38d8[slot], 0);
    WR8(OBJ->f3670[slot], 0);
    r.end = then(ended_load(2), ended_nz(INC8(OBJ->f2800[slot])));
    return r;
}

void abi_StepListThenSoundAndStartChannel86(void)
{
    abi_sound_end(StepListThenSoundAndStartChannel86(cpu.x, cpu.y));
}

/*
 * @routine 025C:25C SpiralOutWhileStepping
 * @run 70:BA5C
 * WHAT IT DOES: Steps the animation list at $BA9A while moving the object by the speed made from OBJ_35C0 and the radius OBJ_3880:OBJ_38D8; the radius shrinks by one and OBJ_35C0 turns on by 4; when OBJ_35C0 wraps it goes to the next OBJ_2800.
 * HOW: StepObjectFrames, SetVelocityFromAngle, ApplyVelocity, a 16-bit subtract in memory, an add.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended SpiralOutWhileStepping(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, value, angle;
    FrameStep step;
    ScaleResult scale;
    AxisMove move;
    Sum sum;
    Ended e = l8_step_list(slot, 0xBA9A, 0xBA67, false, &y, &step);

    value = RD8(OBJ->f38d8[slot]);
    WR8(ZP->temp00, value);
    e = then(e, ended_load(value));
    value = RD8(OBJ->f3880[slot]);
    WR8(ZP->temp01, value);
    e = then(e, ended_load(value));
    angle = RD8(OBJ->f35c0[slot]);
    e = then(e, ended_load(angle));
    jsr_push(0xBA77);
    scale = SetVelocityFromAngle(angle, slot);
    jsr_pop();
    e = then(e, l8_of_scale(scale));
    jsr_push(0xBA7A);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    value = RD8(OBJ->f38d8[slot]);
    sum = sub_bytes(value, 1, true);
    WR8(OBJ->f38d8[slot], sum.value);
    value = RD8(OBJ->f3880[slot]);
    sum = sub_bytes(value, 0, sum.carry);
    WR8(OBJ->f3880[slot], sum.value);
    e = then(e, ended_of_sum(sum));
    angle = RD8(OBJ->f35c0[slot]);
    sum = add_bytes(angle, 4, false);
    WR8(OBJ->f35c0[slot], sum.value);
    e = then(e, ended_of_sum(sum));
    if (!sum.carry) {
        return e;
    }
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_SpiralOutWhileStepping(void)
{
    abi_ended(SpiralOutWhileStepping(cpu.x, cpu.y));
}

/*
 * @routine 025C:2A7 MoveXUntilPastLeftEdge
 * @run 70:BAA7
 * WHAT IT DOES: Moves the object along X by its speed; once its X high byte is not 0 and its low byte is below $80 sets OBJ_3510 $20 and goes to the next OBJ_2800.
 * HOW: ApplyVelocityX, two tests, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended MoveXUntilPastLeftEdge(uint8_t slot)
{
    uint8_t high, low;
    AxisMove move;
    Ended e;

    jsr_push(0xBAAA);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = ended_of_axis_move(move);
    high = RD8(OBJ->x_hi[slot]);
    e = then(e, ended_load(high));
    if (high == 0) {
        return e;
    }
    low = RD8(OBJ->x_lo[slot]);
    e = then(e, ended_load(low));
    if ((low & 0x80) == 0) {
        return e;
    }
    WR8(OBJ->f3510[slot], 0x20);
    e = then(e, ended_load(0x20));
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_MoveXUntilPastLeftEdge(void)
{
    abi_ended(MoveXUntilPastLeftEdge(cpu.x));
}

/*
 * @routine 025C:2BD CountDownThenClearAndAdvanceSlot10
 * @run 70:BABD
 * WHAT IT DOES: Counts OBJ_3510 down; at zero goes to the next state of slot $10 and clears the object.
 * HOW: A memory decrement, an INC, ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended CountDownThenClearAndAdvanceSlot10(uint8_t slot)
{
    uint8_t timer;
    Ended e;

    timer = DEC8(OBJ->f3510[slot]);
    e = ended_nz(timer);
    if (timer != 0) {
        return e;
    }
    e = then(e, ended_nz(INC8(OBJ->state[0x10])));
    jsr_push(0xBAC8);
    ClearObject(slot);
    jsr_pop();
    return e;
}

void abi_CountDownThenClearAndAdvanceSlot10(void)
{
    abi_ended(CountDownThenClearAndAdvanceSlot10(cpu.x));
}

/*
 * @routine 025C:2EA SoundWhenHurtInStatesOneToThree
 * @run 70:BAEA
 * WHAT IT DOES: In states 1 to 3, when OBJ_3568 is 1 or 2 or bit 7 of OBJ_31A0 is set, plays sound $43 and goes to state 4 (state 5 when OBJ_3568 is 2).
 * HOW: Compares, SoundCommand, a store.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The OBJ_3568 = 2 arm ran in no script; the draft stops there.
 */
SoundEnd SoundWhenHurtInStatesOneToThree(uint8_t slot, uint8_t y_entry)
{
    uint8_t state, mark, flags, next;
    uint16_t ret;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        r.end = e;
        return r;
    }
    e = then(e, ended_compare(state, 4));
    if (state >= 4) {
        r.end = e;
        return r;
    }
    mark = RD8(OBJ->f3568[slot]);
    e = then(e, ended_compare(mark, 1));
    next = 4;
    ret = 0xBB0A;
    if (mark != 1) {
        e = then(e, ended_compare(mark, 2));
        if (mark == 2) {
            next = 5;
            ret = 0xBB15;
        } else {
            flags = RD8(OBJ->status_flags[slot]);
            e = then(e, ended_load((uint8_t)(flags & 0x80)));
            if ((flags & 0x80) == 0) {
                r.end = e;
                return r;
            }
        }
    }
    r.sounded = true;
    r.command = 0x43;
    r.before = then(e, ended_load(0x43));
    jsr_push(ret);
    r.call = SoundCommand(0x43, slot, y_entry);
    jsr_pop();
    WR8(OBJ->state[slot], next);
    r.end = ended_load(next);
    return r;
}

void abi_SoundWhenHurtInStatesOneToThree(void)
{
    abi_sound_end(SoundWhenHurtInStatesOneToThree(cpu.x, cpu.y));
}

/*
 * @routine 025C:31B LoadRecordAimAtPlayer
 * @run 70:BB1B
 * WHAT IT DOES: Loads the object's record, sets X speed -$20 and moves it, finds the angle to the point $FF00:$00F0 from the player's side and sets the speed from it with a radius of $0280; OBJ_3510 $40.
 * HOW: LoadObjectRecord, ApplyVelocityX, AngleToPlayerOffset, SetVelocityFromAngle.
 * WAITS: no.
 * Entry: X object. Exit: A $40.
 */
Ended LoadRecordAimAtPlayer(uint8_t slot)
{
    uint8_t last, nz, angle;
    bool carry;
    AngleExit toward;
    ScaleResult scale;
    AxisMove move;
    Ended e;

    jsr_push(0xBB1E);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->vx_hi[slot], 0xE0);
    e = then(e, ended_load(0xE0));
    jsr_push(0xBB26);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    WR8(ZP->temp08, 0);
    WR8(ZP->temp09, 0);
    WR8(ZP->temp0a, 0xF0);
    WR8(ZP->temp0b, 0xFF);
    jsr_push(0xBB35);
    toward = AngleToPlayerOffset(slot);
    jsr_pop();
    e = then(e, l8_of_angle(toward));
    push8(toward.a);
    WR8(ZP->temp00, 0x80);
    WR8(ZP->temp01, 2);
    angle = pull8();
    e = then(e, ended_load(angle));
    jsr_push(0xBB42);
    scale = SetVelocityFromAngle(angle, slot);
    jsr_pop();
    e = then(e, l8_of_scale(scale));
    WR8(OBJ->f3510[slot], 0x40);
    return then(e, ended_load(0x40));
}

void abi_LoadRecordAimAtPlayer(void)
{
    abi_ended(LoadRecordAimAtPlayer(cpu.x));
}

/* The absolute difference of two bytes as the assembly makes it: SEC, SBC, then EOR #$FF and INC when it borrowed. */
static uint8_t abs_difference(uint8_t a, uint8_t b, bool *overflow)
{
    Sum diff = sub_bytes(a, b, true);
    uint8_t value = diff.value;

    *overflow = diff.overflow;
    if (!diff.carry) {
        value = (uint8_t)(~value);
        value = (uint8_t)(value + 1);
    }
    return value;
}

/*
 * @routine 025C:3E9 NearObject19
 * @run 70:BBE9
 * WHAT IT DOES: Says whether the object is within 7 of the object in slot $19 on both axes (low bytes).
 * HOW: Two absolute differences and compares; C 1 when near.
 * WAITS: no.
 * Entry: X object. Exit: C 1 near, 0 not; A the last difference, N and Z from the compare, V from the last subtract.
 */
Ended NearObject19(uint8_t slot)
{
    uint8_t mine, other, diff;
    bool overflow;
    Ended e;

    mine = RD8(OBJ->x_lo[slot]);
    other = RD8(OBJ->x_lo[0x19]);
    diff = abs_difference(mine, other, &overflow);
    e = ended_with_overflow(ended_compare(diff, 8), overflow);
    if (diff >= 8) {
        return ended_with_carry(e, false);
    }
    mine = RD8(OBJ->y_lo[slot]);
    other = RD8(OBJ->y_lo[0x19]);
    diff = abs_difference(mine, other, &overflow);
    e = ended_with_overflow(ended_compare(diff, 8), overflow);
    return ended_with_carry(e, diff < 8);
}

void abi_NearObject19(void)
{
    abi_ended(NearObject19(cpu.x));
}

/*
 * @routine 025C:425 StartRiseAndSpawnRing
 * @run 70:BC25
 * WHAT IT DOES: Starts the ring of seven objects (SpawnRingOfSeven), sets OBJ_37D0 of slot $19 to 1, clears the object's OBJ_3250, animation and X speed, gives it the Y speed -$0400 and goes to the next OBJ_2800.
 * HOW: A call, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: A $FC, N and Z from the new OBJ_2800.
 */
Ended StartRiseAndSpawnRing(uint8_t slot)
{
    Ended e;

    jsr_push(0xBC28);
    e = SpawnRingOfSeven(slot);
    jsr_pop();
    WR8(OBJ->f37d0[0x19], 1);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0xFC);
    WR8(OBJ->vy_lo[slot], 0);
    e = then(e, ended_load(0xFC));
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_StartRiseAndSpawnRing(void)
{
    abi_ended(StartRiseAndSpawnRing(cpu.x));
}

/*
 * @routine 025C:448 SpawnRingOfSeven
 * @run 70:BC48
 * WHAT IT DOES: Clears the slots $20 to $26 and starts an object of type $2A in each at the object's place, giving each its number 0 to 6 in OBJ_34B8.
 * HOW: A loop of ClearObjectInY, StartObjectAtPlace and a counter in zTemp00.
 * WAITS: no.
 * Entry: X object. Exit: Y $27, C 1, N and Z from the compare, A the last number.
 */
Ended SpawnRingOfSeven(uint8_t slot)
{
    uint8_t y = 0x20, count;

    WR8(ZP->temp00, 0);
    do {
        jsr_push(0xBC4F);
        ClearObjectInY(y);
        jsr_pop();
        jsr_push(0xBC54);
        StartObjectAtPlace(0x2A, slot, y);
        jsr_pop();
        count = RD8(ZP->temp00);
        WR8(OBJ->f34b8[y], count);
        INC8(ZP->temp00);
        y++;
    } while (y < 0x27);
    return ended_with_y(ended_with_a(l8_compare_y(y, 0x27), count), y);
}

void abi_SpawnRingOfSeven(void)
{
    abi_ended(SpawnRingOfSeven(cpu.x));
}

/*
 * @routine 025C:461 FallAlongListUntilGround
 * @run 70:BC61
 * WHAT IT DOES: Steps the animation list at $BC98 while the object falls (ApplyGravityUnlimited, ApplyVelocityY); once its Y low byte reaches $B8 it takes sprite set 3, frame 0, the speed X -$0400, Y -$0200, clears the animation and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocityY, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended FallAlongListUntilGround(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, ground;
    FrameStep step;
    GravityResult gravity;
    AxisMove move;
    Ended e = l8_step_list(slot, 0xBC98, 0xBC6C, false, &y, &step);

    jsr_push(0xBC6F);
    gravity = ApplyGravityUnlimited(slot);
    jsr_pop();
    (void)gravity;
    jsr_push(0xBC72);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    ground = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(ground, 0xB8));
    if (ground < 0xB8) {
        return e;
    }
    WR8(OBJ->sprite_set[slot], 3);
    WR8(OBJ->frame[slot], 0);
    WR8(OBJ->vx_hi[slot], 0xFC);
    WR8(OBJ->vy_hi[slot], 0xFE);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    e = then(e, ended_load(0xFE));
    return then(e, ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_FallAlongListUntilGround(void)
{
    abi_ended(FallAlongListUntilGround(cpu.x, cpu.y));
}

/*
 * @routine 025C:4B9 StepListCCThenMoveOrClear
 * @run 70:BCB9
 * WHAT IT DOES: Steps the animation list at $BCCC; while it runs the object moves by its speed, when it ends the object is cleared.
 * HOW: StepObjectFrames, then a tail jump into ClearObject or ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step and then ClearObject (all kept), or as ApplyVelocity.
 */
Ended StepListCCThenMoveOrClear(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry;
    FrameStep step;
    AxisMove move;
    Ended e = l8_step_list(slot, 0xBCCC, 0xBCC4, false, &y, &step);

    if (step == FRAME_LIST_ENDED) {
        ClearObject(slot);
        return e;
    }
    move = ApplyVelocity(slot);
    return then(e, ended_of_axis_move(move));
}

void abi_StepListCCThenMoveOrClear(void)
{
    abi_ended(StepListCCThenMoveOrClear(cpu.x, cpu.y));
}

/*
 * @routine 025C:534 LoadRecordThrowByNumber
 * @run 70:BD34
 * WHAT IT DOES: Loads the object's record, then gives it the speed (both axes) from a four byte entry of the table at $BD66 picked by its number OBJ_34B8, the timer OBJ_3510 from a table at $BD86 and the frame $28 plus the number.
 * HOW: LoadObjectRecord, table reads by the number times 4, an add.
 * WAITS: no.
 * Entry: X object. Exit: A the frame, C and V from the add, Y the number.
 */
Ended LoadRecordThrowByNumber(uint8_t slot)
{
    uint8_t last, nz, number, row, value, y;
    bool carry;
    Sum sum;
    Ended e;

    jsr_push(0xBD37);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    number = RD8(OBJ->f34b8[slot]);
    row = (uint8_t)(number << 2);
    value = rd8((uint16_t)(0xBD67 + row));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xBD66 + row));
    WR8(OBJ->vx_lo[slot], value);
    value = rd8((uint16_t)(0xBD69 + row));
    WR8(OBJ->vy_hi[slot], value);
    value = rd8((uint16_t)(0xBD68 + row));
    WR8(OBJ->vy_lo[slot], value);
    y = RD8(OBJ->f34b8[slot]);
    value = rd8((uint16_t)(0xBD86 + y));
    WR8(OBJ->f3510[slot], value);
    sum = add_bytes(y, 0x28, false);
    WR8(OBJ->frame[slot], sum.value);
    return then(then(e, ended_with_y(ended_load(value), y)), ended_of_sum(sum));
}

void abi_LoadRecordThrowByNumber(void)
{
    abi_ended(LoadRecordThrowByNumber(cpu.x));
}

/*
 * @routine 025C:58E FallThenTurnAtTimerEnd
 * @run 70:BD8E
 * WHAT IT DOES: The object falls and moves; when OBJ_3510 counts down to zero it takes sprite set 3, frame 0, the speed X -$0400, Y -$0100, clears the animation and goes to the next state.
 * HOW: ApplyGravityUnlimited, ApplyVelocity, a memory decrement, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended FallThenTurnAtTimerEnd(uint8_t slot)
{
    uint8_t timer;
    GravityResult gravity;
    AxisMove move;
    Ended e;

    jsr_push(0xBD91);
    gravity = ApplyGravityUnlimited(slot);
    jsr_pop();
    (void)gravity;
    jsr_push(0xBD94);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = ended_of_axis_move(move);
    timer = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(timer));
    if (timer != 0) {
        return e;
    }
    WR8(OBJ->sprite_set[slot], 3);
    WR8(OBJ->frame[slot], 0);
    WR8(OBJ->vx_hi[slot], 0xFC);
    WR8(OBJ->vy_hi[slot], 0xFF);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    e = then(e, ended_load(0xFF));
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_FallThenTurnAtTimerEnd(void)
{
    abi_ended(FallThenTurnAtTimerEnd(cpu.x));
}

/*
 * @routine 025C:5B8 StepListCBThenMoveOrClear
 * @run 70:BDB8
 * WHAT IT DOES: Steps the animation list at $BDCB; while it runs the object moves by its speed, when it ends the object is cleared.
 * HOW: StepObjectFrames, then a tail jump into ClearObject or ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step and then ClearObject (all kept), or as ApplyVelocity.
 */
Ended StepListCBThenMoveOrClear(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry;
    FrameStep step;
    AxisMove move;
    Ended e = l8_step_list(slot, 0xBDCB, 0xBDC3, false, &y, &step);

    if (step == FRAME_LIST_ENDED) {
        ClearObject(slot);
        return e;
    }
    move = ApplyVelocity(slot);
    return then(e, ended_of_axis_move(move));
}

void abi_StepListCBThenMoveOrClear(void)
{
    abi_ended(StepListCBThenMoveOrClear(cpu.x, cpu.y));
}

/*
 * @routine 025C:5F8 FollowObject19ThenStepListByMode
 * @run 70:BDF8
 * WHAT IT DOES: In state 0 loads the record. Later it takes the place of slot $19, restarts its animation when OBJ_3670 of slot $19 changed, and shows frame by it: from a table at $BE33 when the entry is below $80, otherwise by stepping the list the entry names (low nibble times two, at $BE3C).
 * HOW: LoadObjectRecord, CopyObjectPlaceBack, table reads, a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended FollowObject19ThenStepListByMode(uint8_t slot, uint8_t y_entry)
{
    uint8_t state, last, nz, mode, kept, entry, lo, hi, y;
    bool carry;
    FrameStep step;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        return ended_of_record(last, nz, carry);
    }
    jsr_push(0xBE05);
    last = CopyObjectPlaceBack(0x19, slot);
    jsr_pop();
    e = then(ended_with_y(ended_nz(0x19), 0x19), ended_load(last));
    mode = RD8(OBJ->f3670[0x19]);
    kept = RD8(OBJ->f34b8[slot]);
    e = then(e, ended_compare(mode, kept));
    if (mode != kept) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
    }
    WR8(OBJ->f34b8[slot], mode);
    y = RD8(OBJ->f3670[0x19]);
    e = then(e, ended_with_y(ended_nz(y), y));
    entry = rd8((uint16_t)(0xBE33 + y));
    e = then(e, ended_load(entry));
    if ((entry & 0x80) == 0) {
        WR8(OBJ->frame[slot], entry);
        return e;
    }
    entry = (uint8_t)((entry & 0x0F) << 1);
    e = then(e, ended_load(entry));
    y = entry;
    e = ended_with_y(e, y);
    lo = rd8((uint16_t)(0xBE3C + y));
    WR8(ZP->temp10, lo);
    e = then(e, ended_load(lo));
    hi = rd8((uint16_t)(0xBE3D + y));
    WR8(ZP->temp11, hi);
    e = then(e, ended_load(hi));
    step = StepObjectFrames(slot, &y);
    return then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
}

void abi_FollowObject19ThenStepListByMode(void)
{
    abi_ended(FollowObject19ThenStepListByMode(cpu.x, cpu.y));
}

/*
 * @routine 025C:6D6 StartScriptByWaveBit
 * @run 70:BED6
 * WHAT IT DOES: Starts script $8F or $90 (by bit 0 of the wave counter) as kind 1 in slot 4.
 * HOW: StartScriptSlotKind1 between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object. Exit: A and Y the script, N and Z from X, C and V from the add.
 */
Ended StartScriptByWaveBit(uint8_t slot)
{
    uint8_t wave;
    Sum sum;

    push8(slot);
    wave = RD8(ZP->wave_counter) & 0x01;
    sum = add_bytes(wave, 0x8F, false);
    jsr_push(0xBEE3);
    StartScriptSlotKind1(sum.value, 4);
    jsr_pop();
    slot = pull8();
    return then(ended_of_sum(sum), ended_with_y(ended_with_a(ended_nz(slot), sum.value), sum.value));
}

void abi_StartScriptByWaveBit(void)
{
    abi_ended(StartScriptByWaveBit(cpu.x));
}

/*
 * @routine 025C:6E5 LoadRecordWithSound1C
 * @run 70:BEE5
 * WHAT IT DOES: Loads the object's record and plays sound $1C.
 * HOW: LoadObjectRecord, SoundCommand.
 * WAITS: no.
 * Entry: X object. Exit: as the sound call.
 */
SoundEnd LoadRecordWithSound1C(uint8_t slot)
{
    uint8_t last, nz;
    bool carry;
    SoundEnd r = l8_sound_none(ended_kept());

    jsr_push(0xBEE8);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    r.sounded = true;
    r.command = 0x1C;
    r.before = then(ended_of_record(last, nz, carry), ended_load(0x1C));
    jsr_push(0xBEED);
    r.call = SoundCommand(0x1C, slot, 11);
    jsr_pop();
    return r;
}

void abi_LoadRecordWithSound1C(void)
{
    abi_sound_end(LoadRecordWithSound1C(cpu.x));
}

/*
 * @routine 025C:6EE FollowObject19ThenLaunch
 * @run 70:BEEE
 * WHAT IT DOES: Takes the place of slot $19 and steps the animation list at $BF16; when it ends sets the speed from the angle $68 with the radius $0400, OBJ_3250 2 and goes to the next state.
 * HOW: CopyObjectPlaceBack, StepObjectFrames, SetVelocityFromAngle, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended FollowObject19ThenLaunch(uint8_t slot)
{
    uint8_t last, y = 0x19;
    FrameStep step;
    ScaleResult scale;
    Ended e;

    jsr_push(0xBEF3);
    last = CopyObjectPlaceBack(0x19, slot);
    jsr_pop();
    e = then(ended_with_y(ended_nz(0x19), 0x19), ended_load(last));
    e = then(e, l8_step_list(slot, 0xBF16, 0xBEFE, false, &y, &step));
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 4);
    jsr_push(0xBF0D);
    scale = SetVelocityFromAngle(0x68, slot);
    jsr_pop();
    e = then(then(e, ended_load(0x68)), l8_of_scale(scale));
    WR8(OBJ->hitbox_on[slot], 2);
    e = then(e, ended_load(2));
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_FollowObject19ThenLaunch(void)
{
    abi_ended(FollowObject19ThenLaunch(cpu.x));
}

/*
 * @routine 025C:797 SpawnType30EveryFourthFrame
 * @run 70:BF97
 * WHAT IT DOES: When the wave counter's low two bits are 0, starts an object of type $30 in a free slot of $20 to $3F at the object's place.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended SpawnType30EveryFourthFrame(uint8_t slot)
{
    uint8_t wave, found_slot, last, placed;
    bool found;
    Ended e;

    wave = RD8(ZP->wave_counter) & 0x03;
    e = ended_load(wave);
    if (wave != 0) {
        return e;
    }
    jsr_push(0xBFA0);
    found_slot = FindFreeSlotFrom20(&last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, last, found));
    if (!found) {
        return e;
    }
    jsr_push(0xBFA7);
    placed = StartObjectAtPlace(0x30, slot, found_slot);
    jsr_pop();
    return then(e, ended_load(placed));
}

void abi_SpawnType30EveryFourthFrame(void)
{
    abi_ended(SpawnType30EveryFourthFrame(cpu.x));
}

/*
 * @routine 025C:757 StepListThenSpawnUntilHurt
 * @run 70:BF57
 * WHAT IT DOES: Steps the animation list at $BF8E, moves the object and may spawn a piece (SpawnType30EveryFourthFrame); once bit 7 of OBJ_31A0 is set it takes the speed X $00C0, Y -$0300, clears OBJ_3250 and the animation and goes to the next state.
 * HOW: StepObjectFrames, ApplyVelocity, a call, a test, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListThenSpawnUntilHurt(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, flags;
    FrameStep step;
    AxisMove move;
    Ended e = l8_step_list(slot, 0xBF8E, 0xBF62, false, &y, &step);

    jsr_push(0xBF65);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    jsr_push(0xBF68);
    e = then(e, SpawnType30EveryFourthFrame(slot));
    jsr_pop();
    flags = RD8(OBJ->status_flags[slot]);
    e = then(e, ended_load((uint8_t)(flags & 0x80)));
    if ((flags & 0x80) == 0) {
        return e;
    }
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0xC0);
    WR8(OBJ->vy_hi[slot], 0xFD);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    e = then(e, ended_load(0xFD));
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_StepListThenSpawnUntilHurt(void)
{
    abi_ended(StepListThenSpawnUntilHurt(cpu.x, cpu.y));
}

/*
 * @routine 025C:7ED SpawnType2FAtWaveMarks
 * @run 70:BFED
 * WHAT IT DOES: When the wave counter (low six bits) equals one of the eight marks at $C028, starts an object of type $2F in a free slot of $20 to $3F, plays sound $18 and gives the new object the speed from the table at $C030 (two bytes a mark), then moves it once.
 * HOW: A loop over the marks, FindFreeSlotFrom20, StartObjectAtPlace, SoundCommand, ApplyVelocity with the slots swapped.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. zD9 and zDA keep their address form until they have names.
 */
SoundEnd SpawnType2FAtWaveMarks(uint8_t slot, uint8_t y_entry)
{
    uint8_t wave, mark, y = 0, found_slot, last, placed, index;
    bool found;
    AxisMove move;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    wave = RD8(ZP->wave_counter) & 0x3F;
    e = ended_load(wave);
    for (;;) {
        mark = rd8((uint16_t)(0xC028 + y));
        e = ended_with_y(ended_compare(wave, mark), y);
        if (wave == mark) {
            break;
        }
        y++;
        e = then(e, ended_with_y(ended_nz(y), y));
        e = then(e, l8_compare_y(y, 8));
        if (y >= 8) {
            r.end = e;
            return r;
        }
    }
    wr8(0x20D9, slot);
    wr8(0x20DA, y);
    jsr_push(0xC004);
    found_slot = FindFreeSlotFrom20(&last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, last, found));
    if (!found) {
        r.end = e;
        return r;
    }
    jsr_push(0xC00B);
    placed = StartObjectAtPlace(0x2F, slot, found_slot);
    jsr_pop();
    r.sounded = true;
    r.command = 0x18;
    r.before = then(then(e, ended_load(placed)), ended_load(0x18));
    jsr_push(0xC010);
    r.call = SoundCommand(0x18, slot, found_slot);
    jsr_pop();
    index = (uint8_t)(rd8(0x20DA) << 1);
    WR8(OBJ->vx_hi[found_slot], rd8((uint16_t)(0xC030 + index)));
    WR8(OBJ->vy_hi[found_slot], rd8((uint16_t)(0xC031 + index)));
    jsr_push(0xC024);
    move = ApplyVelocity(found_slot);
    jsr_pop();
    slot = rd8(0x20D9);
    r.end = then(ended_of_axis_move(move), ended_with_y(ended_nz(slot), found_slot));
    return r;
}

void abi_SpawnType2FAtWaveMarks(void)
{
    abi_sound_end(SpawnType2FAtWaveMarks(cpu.x, cpu.y));
}

/*
 * @routine 025C:7A8 StepListSlowSideways
 * @run 70:BFA8
 * WHAT IT DOES: Steps the animation list at $BFDC; the X speed loses $0008 and the Y speed gains $0020 a frame; the object moves and may spawn a piece (SpawnType2FAtWaveMarks).
 * HOW: StepObjectFrames, two 16-bit adds in memory, ApplyVelocity, a call.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the call.
 */
SoundEnd StepListSlowSideways(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, value;
    FrameStep step;
    Sum sum;
    AxisMove move;
    SoundEnd r;
    Ended e = l8_step_list(slot, 0xBFDC, 0xBFB3, false, &y, &step);

    value = RD8(OBJ->vx_lo[slot]);
    sum = sub_bytes(value, 8, true);
    WR8(OBJ->vx_lo[slot], sum.value);
    value = RD8(OBJ->vx_hi[slot]);
    sum = sub_bytes(value, 0, sum.carry);
    WR8(OBJ->vx_hi[slot], sum.value);
    value = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(value, 0x20, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    value = RD8(OBJ->vy_hi[slot]);
    sum = add_bytes(value, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    e = then(e, ended_of_sum(sum));
    jsr_push(0xBFD8);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    jsr_push(0xBFDB);
    r = SpawnType2FAtWaveMarks(slot, y);
    jsr_pop();
    return sound_end_after(e, r);
}

void abi_StepListSlowSideways(void)
{
    abi_sound_end(StepListSlowSideways(cpu.x, cpu.y));
}

/*
 * @routine 025C:348 StepListThenCountdownNextState
 * @run 70:BB48
 * WHAT IT DOES: Steps the animation list at $BB69, sets OBJ_3250 2, moves the object and counts OBJ_3510 down; at zero sets it $40 and goes to the next state.
 * HOW: StepObjectFrames, ApplyVelocity, a memory decrement, a store, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListThenCountdownNextState(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, timer;
    FrameStep step;
    AxisMove move;
    Ended e = l8_step_list(slot, 0xBB69, 0xBB53, false, &y, &step);

    WR8(OBJ->hitbox_on[slot], 2);
    e = then(e, ended_load(2));
    jsr_push(0xBB5B);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    timer = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(timer));
    if (timer != 0) {
        return e;
    }
    WR8(OBJ->f3510[slot], 0x40);
    e = then(e, ended_load(0x40));
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_StepListThenCountdownNextState(void)
{
    abi_ended(StepListThenCountdownNextState(cpu.x, cpu.y));
}

/*
 * @routine 025C:376 StepListThenSound43AtZero
 * @run 70:BB76
 * WHAT IT DOES: Steps the animation list at $BB69; when OBJ_3510 counts down to zero plays sound $43 and goes to the next state.
 * HOW: StepObjectFrames, a memory decrement, SoundCommand, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
SoundEnd StepListThenSound43AtZero(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, timer;
    FrameStep step;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e = l8_step_list(slot, 0xBB69, 0xBB81, false, &y, &step);

    timer = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(timer));
    if (timer != 0) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0x43;
    r.before = then(e, ended_load(0x43));
    jsr_push(0xBB8B);
    r.call = SoundCommand(0x43, slot, y);
    jsr_pop();
    r.end = ended_nz(INC8(OBJ->state[slot]));
    return r;
}

void abi_StepListThenSound43AtZero(void)
{
    abi_sound_end(StepListThenSound43AtZero(cpu.x, cpu.y));
}

/*
 * @routine 025C:38F SeekObject19ThenHitAndClear
 * @run 70:BB8F
 * WHAT IT DOES: Steps the animation list at $BB69 while flying toward slot $19 with the radius $0280; when near it (NearObject19) sets the state of slot $19 to 7, its OBJ_3670 2, the frame of slot $18 1 and its animation clear, picks OBJ_36C8 of slot $19 from the table at $BBE5 by a random number and clears the object.
 * HOW: StepObjectFrames, AngleToPoint, SetVelocityFromAngle, ApplyVelocity, NearObject19, NextRandom, ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended SeekObject19ThenHitAndClear(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, value, pick;
    FrameStep step;
    AngleExit toward;
    ScaleResult scale;
    AxisMove move;
    RandomResult random;
    Ended e = l8_step_list(slot, 0xBB69, 0xBB9A, false, &y, &step), near;

    WR8(ZP->temp09, 0);
    value = RD8(OBJ->x_lo[0x19]);
    WR8(ZP->temp08, value);
    e = then(e, ended_load(value));
    WR8(ZP->temp0b, 0);
    value = RD8(OBJ->y_lo[0x19]);
    WR8(ZP->temp0a, value);
    e = then(e, ended_load(value));
    jsr_push(0xBBAB);
    toward = AngleToPoint(slot);
    jsr_pop();
    e = then(e, l8_of_angle(toward));
    push8(toward.a);
    WR8(ZP->temp00, 0x80);
    WR8(ZP->temp01, 2);
    value = pull8();
    e = then(e, ended_load(value));
    jsr_push(0xBBB8);
    scale = SetVelocityFromAngle(value, slot);
    jsr_pop();
    e = then(e, l8_of_scale(scale));
    jsr_push(0xBBBB);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    jsr_push(0xBBBE);
    near = NearObject19(slot);
    jsr_pop();
    e = then(e, near);
    if (!near.c) {
        return e;
    }
    WR8(OBJ->state[0x19], 7);
    WR8(OBJ->f3670[0x19], 2);
    WR8(OBJ->frame[0x18], 1);
    WR8(OBJ->anim_step[0x18], 0);
    WR8(OBJ->anim_timer[0x18], 0);
    e = then(e, ended_load(0));
    jsr_push(0xBBD8);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = random.value & 0x03;
    e = then(e, ended_with_y(ended_load(pick), pick));
    value = rd8((uint16_t)(0xBBE5 + pick));
    WR8(OBJ->f36c8[0x19], value);
    e = then(e, ended_load(value));
    jsr_push(0xBBE4);
    ClearObject(slot);
    jsr_pop();
    return e;
}

void abi_SeekObject19ThenHitAndClear(void)
{
    abi_ended(SeekObject19ThenHitAndClear(cpu.x, cpu.y));
}
