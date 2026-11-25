/*
 * TEMPORARY: the stage object routines of lot L6 for the overlays at sectors 10C9 to 10CF (enemy kinds not yet identified), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

/*
 * @routine 10CD:181 StepListC192ThenClear
 * @run 71:C181
 * WHAT IT DOES: Steps the animation list at $C192; when it ends the object is cleared.
 * HOW: StepObjectFrames, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step.
 */
Ended StepListC192ThenClear(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    uint8_t y = y_entry;
    Ended e;

    WR8(ZP->temp10, 0x92);
    WR8(ZP->temp11, 0xC1);
    jsr_push(0xC18C);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step == FRAME_LIST_ENDED) {
        ClearObject(slot);
    }
    return e;
}

void abi_StepListC192ThenClear(void)
{
    abi_ended(StepListC192ThenClear(cpu.x, cpu.y));
}
