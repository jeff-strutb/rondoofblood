/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 025D (enemy kinds not yet identified), by routine. Bodies return how the
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
 * @routine 025D:040 LoadRecordFlashByNumberThenMoveOrClear
 * @run 71:C040
 * WHAT IT DOES: In state 0 loads the record, gives it the speed X -$0400 and Y -$0180 (OBJ_VY_LO $80) and the flash flags from the table at $C061 by OBJ_34B8. Later it steps the animation list at $C076 and moves, or is cleared when the list ends.
 * HOW: LoadObjectRecord, stores, StepObjectFrames, tail jumps into ClearObject or ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended LoadRecordFlashByNumberThenMoveOrClear(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, state, last, nz, value;
    bool carry;
    FrameStep step;
    AxisMove move;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(0xC048);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        e = ended_of_record(last, nz, carry);
        WR8(OBJ->vy_hi[slot], 0xFE);
        WR8(OBJ->vy_lo[slot], 0x80);
        WR8(OBJ->vx_hi[slot], 0xFC);
        e = then(e, ended_load(0xFC));
        y = RD8(OBJ->f34b8[slot]);
        value = rd8((uint16_t)(0xC061 + y));
        WR8(OBJ->flash_flags[slot], value);
        return then(e, ended_with_y(ended_load(value), y));
    }
    e = l8_step_list(slot, 0xC076, 0xC06E, false, &y, &step);
    if (step == FRAME_LIST_ENDED) {
        ClearObject(slot);
        return e;
    }
    move = ApplyVelocity(slot);
    return then(e, ended_of_axis_move(move));
}

void abi_LoadRecordFlashByNumberThenMoveOrClear(void)
{
    abi_ended(LoadRecordFlashByNumberThenMoveOrClear(cpu.x, cpu.y));
}

/*
 * @routine 025D:0C0 LoadRecordRandomLiftThenMoveOrClear
 * @run 71:C0C0
 * WHAT IT DOES: In state 0 loads the record, gives it a random Y speed from -$10 to $0F and the X speed $18, moves it once and sets the speed from the angle $68 with the radius $0100. Later it steps the animation list at $C0FC and moves, or is cleared when the list ends.
 * HOW: LoadObjectRecord, NextRandom, ApplyVelocity, SetVelocityFromAngle, StepObjectFrames, tail jumps into ClearObject or ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended LoadRecordRandomLiftThenMoveOrClear(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, state, last, nz, masked;
    bool carry;
    FrameStep step;
    RandomResult random;
    AxisMove move;
    ScaleResult scale;
    Sum diff;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(0xC0C8);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        e = ended_of_record(last, nz, carry);
        jsr_push(0xC0CB);
        random = NextRandom();
        jsr_pop();
        e = then(e, ended_of_random(random));
        masked = random.value & 0x1F;
        e = then(e, ended_load(masked));
        diff = sub_bytes(masked, 0x10, true);
        WR8(OBJ->vy_hi[slot], diff.value);
        e = then(e, ended_of_sum(diff));
        WR8(OBJ->vx_hi[slot], 0x18);
        e = then(e, ended_load(0x18));
        jsr_push(0xC0DB);
        move = ApplyVelocity(slot);
        jsr_pop();
        e = then(e, ended_of_axis_move(move));
        WR8(ZP->temp00, 0);
        WR8(ZP->temp01, 1);
        jsr_push(0xC0E8);
        scale = SetVelocityFromAngle(0x68, slot);
        jsr_pop();
        return then(then(e, ended_load(0x68)), l8_of_scale(scale));
    }
    e = l8_step_list(slot, 0xC0FC, 0xC0F4, false, &y, &step);
    if (step == FRAME_LIST_ENDED) {
        ClearObject(slot);
        return e;
    }
    move = ApplyVelocity(slot);
    return then(e, ended_of_axis_move(move));
}

void abi_LoadRecordRandomLiftThenMoveOrClear(void)
{
    abi_ended(LoadRecordRandomLiftThenMoveOrClear(cpu.x, cpu.y));
}
