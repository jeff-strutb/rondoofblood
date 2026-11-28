/*
 * Stage 04's objects, run C00E.
 */
#include "stage_obj_ops.h"
#include "stage04_objects_i.h"
#include "calls_b.h"

/*
 * @routine 07DD:00E FallStep07DE
 * @run 71:C00E
 * WHAT IT DOES: Falls without the speed limit and moves.
 * HOW: ApplyGravityUnlimited and a tail jump to ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FallStep07DE(uint8_t slot)
{
    (void)slot;
    JSR(0xC011, k_gravity_unlimited());
    k_move();
}

void abi_FallStep07DE(void)
{
    FallStep07DE(cpu.x);
}
