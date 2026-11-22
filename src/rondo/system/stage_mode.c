/* The stage's mode handlers: small routines the game mode table runs a step at a time. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "stage_map.h"

/*
 * @routine 024E:023 StageWaitTimerStep
 * @run 6B:8823
 * WHAT IT DOES: Counts a wait down and, when it reaches zero, moves the stage on one step.
 * HOW: zWaitTimer3 down by one; when that gives zero, zField80 up by one.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the byte last changed. A, X, Y, C and V kept.
 */
uint8_t StageWaitTimerStep(void)
{
    uint8_t timer = DEC8(ZP->wait_timer3);

    if (timer != 0) {
        return timer;
    }
    return INC8(ZP->field80);
}

void abi_StageWaitTimerStep(void)
{
    abi_nz(StageWaitTimerStep());
}

/*
 * @routine 050E:019 StageWaitTimerToStep2
 * @run 6B:8819
 * WHAT IT DOES: Counts a wait down and, when it reaches zero, sets the stage's step to 2.
 * HOW: zWaitTimer3 down by one; when that gives zero, zField80 is set to 2.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the byte last handled (the count, or the 2 stored). A is 2
 * when the count reached zero, else kept. X, Y, C and V kept.
 */
static uint8_t wait_to_step2(bool *reached)
{
    uint8_t timer = DEC8(ZP->wait_timer3);

    *reached = timer == 0;
    if (timer != 0) {
        return timer;
    }
    WR8(ZP->field80, 2);
    return 2;
}

void StageWaitTimerToStep2(void)
{
    bool reached;

    wait_to_step2(&reached);
}

void abi_StageWaitTimerToStep2(void)
{
    bool reached;
    uint8_t value = wait_to_step2(&reached);

    if (reached) {
        cpu.a = 2;
    }
    abi_nz(value);
}
