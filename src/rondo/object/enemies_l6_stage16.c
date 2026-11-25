/*
 * TEMPORARY: the stage object routines of lot L6 for the overlays at sectors 16C9 to 16CF (enemy kinds not yet identified), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
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
 * @routine 16CB:3EE StepListB404ThenTimer
 * @run 70:B3EE
 * WHAT IT DOES: Steps the animation list at $B404; when it ends, OBJ_34B8 becomes $48 and the object goes to the next OBJ_2800.
 * HOW: StepObjectFrames, a store, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new OBJ_2800's N and Z (A $48) when the list ended.
 */
Ended StepListB404ThenTimer(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    uint8_t y = y_entry, phase;
    Ended e;

    WR8(ZP->temp10, 0x04);
    WR8(ZP->temp11, 0xB4);
    jsr_push(0xB3F9);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->f34b8[slot], 0x48);
    phase = INC8(OBJ->f2800[slot]);
    return then(then(e, ended_load(0x48)), ended_nz(phase));
}

void abi_StepListB404ThenTimer(void)
{
    abi_ended(StepListB404ThenTimer(cpu.x, cpu.y));
}

/*
 * @routine 16CB:20E LoadThenStartRise
 * @run 70:B20E
 * WHAT IT DOES: Loads the object's record, sets its flip byte (OBJ_2FE8) to $80, its Y speed to $FE80 and OBJ_3720 to $E0.
 * HOW: LoadObjectRecord, stores.
 * WAITS: no.
 * Entry: X object. Exit: A $E0, N set, Z clear; C and Y from the record.
 */
Ended LoadThenStartRise(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;
    Ended e;

    jsr_push(0xB211);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->sprite_flip[slot], 0x80);
    WR8(OBJ->vy_hi[slot], 0xFE);
    WR8(OBJ->vy_lo[slot], 0x80);
    WR8(OBJ->f3720[slot], 0xE0);
    return then(e, ended_load(0xE0));
}

void abi_LoadThenStartRise(void)
{
    abi_ended(LoadThenStartRise(cpu.x));
}
