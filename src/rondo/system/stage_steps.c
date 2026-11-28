/*
 * The stage overlay's per-frame steps through a death or a walk-out (sector 024D-024E, bank $6B at $8000): the player
 * walked to a mark, the CD paused, the stock taken, the waits between. Each takes the registers it is entered with and
 * gives back those it leaves.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "syscard.h"
#include "stage_map.h"
#include "stage_regs.h"

typedef StageRegs Regs;

/* A JSR to the routine loaded at an overlay or player address: the registers as they stand, and what it leaves. */
static Regs call_to(Regs r, uint16_t ret, uint16_t run)
{
    stage_regs_load_shim(r);
    call_routine(ret, run);
    return stage_regs_from_cpu_shim(r);
}

/*
 * @routine 024E:012 StageStockWait
 * @run 6B:8812
 * WHAT IT DOES: Starts the wait after a death: $60 frames in zWaitTimer3, the next step.
 * HOW: A store and an INC of zField80.
 * WAITS: no.
 * Entry: nothing. Exit: A $60, N and Z from zField80's INC. X, Y, C, V kept.
 */
StageRegs StageStockWait(StageRegs r)
{
    r = regs_loaded(r, 0x60);
    WR8(ZP->wait_timer3, 0x60);
    r.nz = INC8(ZP->field80);
    return r;
}

void abi_StageStockWait(void)
{
    stage_regs_load_shim(StageStockWait(regs_entry_shim()));
}

/*
 * @routine 024E:00B StageStockTake
 * @run 6B:880B
 * @stop 8812
 * WHAT IT DOES: Takes one from the stock, in decimal, then StageStockWait.
 * HOW: SED, SEC, SBC #1, CLD; running into StageStockWait.
 * WAITS: no.
 * Entry: A the stock. Exit: as StageStockWait, C 1 when the stock was not zero.
 */
StageRegs StageStockTake(StageRegs r)
{
    uint8_t low = (uint8_t)((r.a & 0x0F) - 1);
    uint8_t high = (uint8_t)((r.a >> 4) - ((low >> 4) & 1));
    uint8_t result = (uint8_t)((high << 4) | (low & 0x0F));

    if (low & 0x10) {
        result = (uint8_t)(result - 0x06);
    }
    if (high & 0x10) {
        result = (uint8_t)(result - 0x60);
    }
    r.c = ((high >> 4) & 1) == 0;
    r = regs_loaded(r, result);
    WR8(ZP->stock, result);
    return StageStockWait(r);
}

void abi_StageStockTake(void)
{
    stage_regs_load_shim(StageStockTake(regs_entry_shim()));
}

/*
 * @routine 024E:002 StageStockCheck
 * @run 6B:8802
 * WHAT IT DOES: With no stock left goes straight to the wait; else step 2.
 * HOW: LDA zStock, BEQ to StageStockWait.
 * WAITS: no.
 * Entry: nothing. Exit: stock 0: as StageStockWait; else A 2, N 0, Z 0.
 */
StageRegs StageStockCheck(StageRegs r)
{
    r = regs_loaded(r, RD8(ZP->stock));
    if (r.a == 0) {
        return StageStockWait(r);
    }
    r = regs_loaded(r, 0x02);
    WR8(ZP->field80, 0x02);
    return r;
}

void abi_StageStockCheck(void)
{
    stage_regs_load_shim(StageStockCheck(regs_entry_shim()));
}

/*
 * @routine 024E:019 StageWaitThenStepTwo
 * @run 6B:8819
 * WHAT IT DOES: Counts zWaitTimer3 down; when it runs out, step 2.
 * HOW: DEC, BEQ.
 * WAITS: no.
 * Entry: nothing. Exit: counting: N and Z from the DEC; else A 2, N 0, Z 0. X, Y, C, V kept.
 */
StageRegs StageWaitThenStepTwo(StageRegs r)
{
    r.nz = DEC8(ZP->wait_timer3);
    if (r.nz != 0) {
        return r;
    }
    r = regs_loaded(r, 0x02);
    WR8(ZP->field80, 0x02);
    return r;
}

void abi_StageWaitThenStepTwo(void)
{
    stage_regs_load_shim(StageWaitThenStepTwo(regs_entry_shim()));
}

/*
 * @routine 024D:7F3 StagePauseCdThenStock
 * @run 6B:87F3
 * WHAT IT DOES: Pauses the CD, retrying while the card reports an error, then takes a life (StageStockTake), or with
 * none left sets zModeDone $80.
 * HOW: JSR cd_pause until A is zero; LDA zStock, BNE to StageStockTake.
 * WAITS: yes, for the card's cd_pause; the stack there is the caller's and the call's two bytes.
 * Entry: X as the card takes it. Exit: with stock as StageStockTake; else A $80, N 1, Z 0.
 */
StageRegs StagePauseCdThenStock(StageRegs r)
{
    do {
        CdPause paused;

        stage_regs_load_shim(r);
        jsr_push(0x87F6);
        paused = vec_cd_pause(r.x);
        jsr_pop();
        cd_pause_exit_shim(paused);
        r = stage_regs_from_cpu_shim(r);
        r.x = r.a;
        r.nz = r.a;
    } while (r.x != 0);
    r = regs_loaded(r, RD8(ZP->stock));
    if (r.a != 0) {
        return StageStockTake(r);
    }
    r = regs_loaded(r, 0x80);
    WR8(ZP->mode_done, 0x80);
    return r;
}

void abi_StagePauseCdThenStock(void)
{
    stage_regs_load_shim(StagePauseCdThenStock(regs_entry_shim()));
}

/*
 * @routine 024D:48D StageWalkOutScroll
 * @run 6B:848D
 * WHAT IT DOES: A frame of walking the player out of a cleared room: the scroll stepped by the route's direction, the
 * player moved with it, the camera and objects run; at the route's stop the scroll halts and the next step comes.
 * HOW: (zField8C - 1) AND 1 picks the route; WalkScrollStepByRoute into entry 0's X step, WalkPlayerStepByRoute added
 * to the player's X; StageCameraStep and StepScrollAndRunObjects; the X compared with WalkStopXByRoute.
 * WAITS: as the calls.
 * Entry: nothing. Exit: walking: A the player's X, N, Z and C from the compare, Y the route; stopped: N and Z from
 * zField85's INC.
 */
StageRegs StageWalkOutScroll(StageRegs r)
{
    return StageWalkOutScrollAt(r, 0x848D, WalkScrollStepByRoute, WalkPlayerStepByRoute, WalkStopXByRoute, 0x80BF,
                                0x8A4F, 0x80C5);
}

/* StageWalkOutScroll's body, at the address it runs at (its JSRs return to at + $1F, + $22 and + $34), with its three
 * tables by route and where its calls to StageCameraStep, StepScrollAndRunObjects and StageScrollSpeedsOff go: the
 * builds of the overlay that hold these instructions at another address (stage_programs_census.c) pass their own. */
StageRegs StageWalkOutScrollAt(StageRegs r, uint16_t at, uint16_t scroll_steps, uint16_t player_steps, uint16_t stop_x,
                                 uint16_t camera, uint16_t step, uint16_t scroll_off)
{
    uint8_t route;
    Sum sum;

    route = (uint8_t)((uint8_t)(RD8(ZP->field8c) - 1) & 0x01);
    r.y = route;
    r = regs_loaded(r, rd8((uint16_t)(scroll_steps + route)));
    WR8(SCROLLE->step_x_hi[0], r.a);
    WR8(SCROLLE->step_x_lo[0], 0);
    WR8(SCROLL->x_frac[0], 0);
    sum = add_bytes(RD8(OBJ->x_lo[0]), rd8((uint16_t)(player_steps + route)), false);
    r = regs_loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(OBJ->x_lo[0], sum.value);
    r = call_to(r, (uint16_t)(at + 0x1F), camera);
    r = call_to(r, (uint16_t)(at + 0x22), step);
    route = (uint8_t)((uint8_t)(RD8(ZP->field8c) - 1) & 0x01);
    r.y = route;
    r = regs_loaded(r, RD8(OBJ->x_lo[0]));
    r = regs_compared(r, r.a, rd8((uint16_t)(stop_x + route)));
    if (r.nz != 0 || !r.c) {
        return r;
    }
    r = call_to(r, (uint16_t)(at + 0x34), scroll_off);
    WR8(SCROLLE->step_x_hi[0], 0);
    r.nz = INC8(ZP->field85);
    return r;
}

void abi_StageWalkOutScroll(void)
{
    stage_regs_load_shim(StageWalkOutScroll(regs_entry_shim()));
}

/*
 * @routine 024D:50A StageShowAndStep
 * @run 6B:850A
 * WHAT IT DOES: The player placed anew with his start frame, the next step.
 * HOW: The player's $A003, StageShowStartFrame, INC zField85.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from zField85's INC; the rest as StageShowStartFrame left them.
 */
StageRegs StageShowAndStep(StageRegs r)
{
    r = call_to(r, 0x850D, 0xA003);
    r = call_to(r, 0x8510, 0x8448);
    r.nz = INC8(ZP->field85);
    return r;
}

void abi_StageShowAndStep(void)
{
    stage_regs_load_shim(StageShowAndStep(regs_entry_shim()));
}

/*
 * @routine 024D:68C StageStopAndStep
 * @run 6B:868C
 * WHAT IT DOES: The scroll stopped, a frame of the objects, the next step.
 * HOW: The $80C5 and $8A4F calls, INC zField85.
 * WAITS: as the calls.
 * Entry: nothing. Exit: N and Z from zField85's INC; the rest as the objects left them.
 */
StageRegs StageStopAndStep(StageRegs r)
{
    r = call_to(r, 0x868F, 0x80C5);
    r = call_to(r, 0x8692, 0x8A4F);
    r.nz = INC8(ZP->field85);
    return r;
}

void abi_StageStopAndStep(void)
{
    stage_regs_load_shim(StageStopAndStep(regs_entry_shim()));
}

/*
 * @routine 024D:720 StageLeaveByRoute
 * @run 6B:8720
 * WHAT IT DOES: Keeps the route in zField8C, ends the mode, holds the display two frames and hides the player.
 * HOW: Stores and an INC.
 * WAITS: no.
 * Entry: A the route. Exit: A 2, N 0, Z 0. X, Y, C, V kept.
 */
StageRegs StageLeaveByRoute(StageRegs r)
{
    WR8(ZP->field8c, r.a);
    INC8(ZP->mode_done);
    r = regs_loaded(r, 0x02);
    WR8(ZP->display_delay, 0x02);
    WR8(OBJ->frame[0], 0);
    return r;
}

void abi_StageLeaveByRoute(void)
{
    stage_regs_load_shim(StageLeaveByRoute(regs_entry_shim()));
}
