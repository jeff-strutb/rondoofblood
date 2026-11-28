/*
 * Stage 00's program: the script slots' frame.
 */
#include "stage_obj_ops.h"
#include "stage_script_slots.h"
#include "calls_b.h"

/*
 * @routine 0251:29F RunScriptSlotsByPause
 * @run 6C:829F
 * WHAT IT DOES: Runs the script slots: every slot from $0B down, or with zPaused's bit 0 set only slots 2 and $0B.
 * HOW: BBR0 of zPaused to the full walk; else BSR for slot 2 and into the same for slot $0B.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void RunScriptSlotsByPause(void)
{
    if (!(RD8(ZP->paused) & 0x01)) {
        run_routine(0x82B3);
        return;
    }
    ldx(0x02);
    call_routine(0x82A6, 0x82A8);
    ldx(0x0B);
    run_routine(0x82A8);
}

void abi_RunScriptSlotsByPause(void)
{
    RunScriptSlotsByPause();
}

/*
 * @routine 0251:2A8 RunScriptSlotIfUsed
 * @run 6C:82A8
 * WHAT IT DOES: Runs script slot X's step when the slot is in use.
 * HOW: A test of ScriptSlotKind.
 * WAITS: no.
 * Entry: X the script slot. Exit: as the step, or A zero.
 */
void RunScriptSlotIfUsed(void)
{
    lda(RD8(SLOTS->kind[cpu.x]));
    if (NE) {
        run_routine(0x82CB);
    }
}

void abi_RunScriptSlotIfUsed(void)
{
    RunScriptSlotIfUsed();
}

/*
 * @routine 0251:2AE RunScriptSlot3Flash
 * @run 6C:82AE
 * WHAT IT DOES: Runs slot 3's flash step.
 * HOW: LDX #3 and a jump to the flash.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void RunScriptSlot3Flash(void)
{
    ldx(0x03);
    run_routine(0x8560);
}

void abi_RunScriptSlot3Flash(void)
{
    RunScriptSlot3Flash();
}

/*
 * @routine 0251:2B3 RunAllScriptSlots
 * @run 6C:82B3
 * WHAT IT DOES: Runs every used script slot from $0B down; with kind 5 in slot 3 only that slot's flash.
 * HOW: The slot kept in zClimbStepBefore across each step.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void RunAllScriptSlots(void)
{
    lda(RD8(SLOTS->kind[3]));
    cmp(cpu.a, 0x05);
    if (EQ) {
        run_routine(0x82AE);
        return;
    }
    ldx(0x0B);
    do {
        lda(RD8(SLOTS->kind[cpu.x]));
        if (NE) {
            WR8(ZP->climb_step_before, cpu.x);
            call_routine(0x82C5, 0x82CB);
            ldx(RD8(ZP->climb_step_before));
        }
        dex();
    } while (PL);
}

void abi_RunAllScriptSlots(void)
{
    RunAllScriptSlots();
}

/*
 * @routine 0251:2CB RunScriptSlotByKind
 * @run 6C:82CB
 * WHAT IT DOES: Goes to the step of script kind A.
 * HOW: A jump through the table at $82DB by kind less one.
 * WAITS: no.
 * Entry: A the slot's kind, X the slot. Exit: as the step.
 */
void RunScriptSlotByKind(void)
{
    uint8_t lo, hi;

    dec_a();
    asl_a();
    tay();
    lo = rd8((uint16_t)(0x82DB + cpu.y));
    lda(lo);
    WR8(ZP->temp00, cpu.a);
    hi = rd8((uint16_t)(0x82DC + cpu.y));
    lda(hi);
    WR8(ZP->temp01, cpu.a);
    run_routine((uint16_t)(lo | (hi << 8)));
}

void abi_RunScriptSlotByKind(void)
{
    RunScriptSlotByKind();
}
