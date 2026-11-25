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
