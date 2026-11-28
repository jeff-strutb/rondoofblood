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

/* The arms the routines end on (L8Arm). */
enum { FLASH_START, FLASH_LIST_ENDED, FLASH_MOVED };
enum { LIFT_START, LIFT_LIST_ENDED, LIFT_MOVED };

/*
 * @routine 025D:040 LoadRecordFlashByNumberThenMoveOrClear
 * @run 71:C040
 * WHAT IT DOES: In state 0 loads the record, gives it the speed X -$0400 and Y -$0180 (OBJ_VY_LO $80) and the flash flags from the table at $C061 by OBJ_34B8. Later it steps the animation list at $C076 and moves, or is cleared when the list ends.
 * HOW: LoadObjectRecord, stores, StepObjectFrames, tail jumps into ClearObject or ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (flash_exit_shim).
 */
L8Arm LoadRecordFlashByNumberThenMoveOrClear(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };
    uint8_t state, y;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(0xC048);
        r.record = l8_record(slot);
        jsr_pop();
        WR8(OBJ->vy_hi[slot], 0xFE);
        WR8(OBJ->vy_lo[slot], 0x80);
        WR8(OBJ->vx_hi[slot], 0xFC);
        y = RD8(OBJ->f34b8[slot]);
        r.first = rd8((uint16_t)(0xC061 + y));
        WR8(OBJ->sprite_attr[slot], r.first);
        r.y = y;
        r.arm = FLASH_START;
        return r;
    }
    r.step = l8_list_step(slot, 0xC076, 0xC06E, false, y_entry);
    if (r.step.step == FRAME_LIST_ENDED) {
        ClearObject(slot);
        r.arm = FLASH_LIST_ENDED;
        return r;
    }
    r.move = ApplyVelocity(slot);
    r.arm = FLASH_MOVED;
    return r;
}

/* What LoadRecordFlashByNumberThenMoveOrClear leaves in A, Y and the flags, from the arm it ended on. */
static Ended flash_exit_shim(L8Arm r)
{
    switch (r.arm) {
    case FLASH_START:
        return then(then(l8_record_ended(r.record), ended_load(0xFC)), ended_with_y(ended_load(r.first), r.y));
    case FLASH_LIST_ENDED:
        return l8_step_ended(r.step);
    default:
        return then(l8_step_ended(r.step), ended_of_axis_move(r.move));
    }
}

void abi_LoadRecordFlashByNumberThenMoveOrClear(void)
{
    abi_ended(flash_exit_shim(LoadRecordFlashByNumberThenMoveOrClear(cpu.x, cpu.y)));
}

/*
 * @routine 025D:0C0 LoadRecordRandomLiftThenMoveOrClear
 * @run 71:C0C0
 * WHAT IT DOES: In state 0 loads the record, gives it a random Y speed from -$10 to $0F and the X speed $18, moves it once and sets the speed from the angle $68 with the radius $0100. Later it steps the animation list at $C0FC and moves, or is cleared when the list ends.
 * HOW: LoadObjectRecord, NextRandom, ApplyVelocity, SetVelocityFromAngle, StepObjectFrames, tail jumps into ClearObject or ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (lift_exit_shim).
 */
L8Arm LoadRecordRandomLiftThenMoveOrClear(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };
    uint8_t state;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(0xC0C8);
        r.record = l8_record(slot);
        jsr_pop();
        jsr_push(0xC0CB);
        r.random = NextRandom();
        jsr_pop();
        r.first = r.random.value & 0x1F;
        r.sum = sub_bytes(r.first, 0x10, true);
        WR8(OBJ->vy_hi[slot], r.sum.value);
        WR8(OBJ->vx_hi[slot], 0x18);
        jsr_push(0xC0DB);
        r.move = ApplyVelocity(slot);
        jsr_pop();
        WR8(ZP->temp00, 0);
        WR8(ZP->temp01, 1);
        jsr_push(0xC0E8);
        r.scale = SetVelocityFromAngle(0x68, slot);
        jsr_pop();
        r.arm = LIFT_START;
        return r;
    }
    r.step = l8_list_step(slot, 0xC0FC, 0xC0F4, false, y_entry);
    if (r.step.step == FRAME_LIST_ENDED) {
        ClearObject(slot);
        r.arm = LIFT_LIST_ENDED;
        return r;
    }
    r.move = ApplyVelocity(slot);
    r.arm = LIFT_MOVED;
    return r;
}

/* What LoadRecordRandomLiftThenMoveOrClear leaves in A, Y and the flags, from the arm it ended on. */
static Ended lift_exit_shim(L8Arm r)
{
    Ended e;

    switch (r.arm) {
    case LIFT_START:
        e = then(l8_record_ended(r.record), ended_of_random(r.random));
        e = then(e, ended_load(r.first));
        e = then(e, ended_of_sum(r.sum));
        e = then(e, ended_load(0x18));
        e = then(e, ended_of_axis_move(r.move));
        return then(then(e, ended_load(0x68)), l8_of_scale(r.scale));
    case LIFT_LIST_ENDED:
        return l8_step_ended(r.step);
    default:
        return then(l8_step_ended(r.step), ended_of_axis_move(r.move));
    }
}

void abi_LoadRecordRandomLiftThenMoveOrClear(void)
{
    abi_ended(lift_exit_shim(LoadRecordRandomLiftThenMoveOrClear(cpu.x, cpu.y)));
}
