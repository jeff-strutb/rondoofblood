/* The stage's frame handler: the tick, the player and camera, the object frame and the ending effect. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum {
    TICK_RETURN = 0x8856, COMMON_A_RETURN = 0x8859, PLAYER_RETURN = 0x885C, COMMON_B_RETURN = 0x885F,
    OBJECTS_RETURN = 0x8862, ENDING_RETURN = 0x8865, COMMON_A_RUN = 0x80AB, COMMON_B_RUN = 0x80BF, TICK_RUN = 0x8B6D,
    DONE_BIT = 0x01, MODE_AFTER = 1, STAGE_ZERO_TIMER = 0x10, STAGE_NONZERO_TIMER = 1
};

void abi_PlayerAndCameraFrame(void);

typedef struct {
    uint8_t a;
    uint8_t x;
    uint8_t y;
    bool c;
    bool v;
    uint8_t nz;
} Regs;

static Regs regs_of(StageExit end, Regs before)
{
    Regs now = before;

    if ((end.set & EXIT_CPU) != 0) {
        end = stage_exit_from_cpu_shim();
    }
    now.a = end.a;
    now.nz = end.nz;
    if ((end.set & EXIT_X) != 0) {
        now.x = end.x;
    }
    if ((end.set & EXIT_Y) != 0) {
        now.y = end.y;
    }
    if ((end.set & EXIT_C) != 0) {
        now.c = end.c;
    }
    if ((end.set & EXIT_V) != 0) {
        now.v = end.v;
    }
    return now;
}

/* The registers and flags as the stage code holds them, put in the machine for a piece that takes them from there. */
static void load_regs_shim(Regs r)
{
    cpu.a = r.a;
    cpu.x = r.x;
    cpu.y = r.y;
    abi_nz(r.nz);
    abi_flag(P_C, r.c);
    abi_flag(P_V, r.v);
}

/* Original code at a call, run with the registers as they stand. */
static Regs original_call_shim(Regs r, uint16_t ret, uint16_t run)
{
    load_regs_shim(r);
    call_orig(ret, run);
    return regs_of(stage_exit_from_cpu_shim(), r);
}

static Regs player_and_camera_shim(Regs r)
{
    load_regs_shim(r);
    abi_PlayerAndCameraFrame();
    return regs_of(stage_exit_from_cpu_shim(), r);
}

static StageExit exit_of(Regs r)
{
    StageExit end = { r.a, r.x, r.y, r.c, r.v, r.nz, EXIT_X | EXIT_Y | EXIT_C | EXIT_V };

    return end;
}

/* One frame of the stage: the tick, the common code, the player and camera, the objects, the ending. */
static StageExit run_stage_frame(Regs r)
{
    StageExit end = { 0, 0, 0, false, false, 0, 0 };
    uint8_t a;

    r.nz = INC8(ZP->wave_counter);
    jsr_push(TICK_RETURN);
    if (resident_at(TICK_RUN) == DISC(0x05BE, 0x36D)) {
        r = regs_of(StepStageFrameLong(r.x, r.y, r.v), r);
    } else {
        r = regs_of(StepStageFrame(r.x, r.y, r.v), r);
    }
    jsr_pop();
    r = original_call_shim(r, COMMON_A_RETURN, COMMON_A_RUN);
    jsr_push(PLAYER_RETURN);
    r = player_and_camera_shim(r);
    jsr_pop();
    r = original_call_shim(r, COMMON_B_RETURN, COMMON_B_RUN);
    jsr_push(OBJECTS_RETURN);
    StepScrollAndRunObjects(r.x);
    jsr_pop();
    r = regs_of(stage_exit_from_cpu_shim(), r);
    jsr_push(ENDING_RETURN);
    r = regs_of(StepStageEnding(r.x, r.y), r);
    jsr_pop();
    if ((RD8(ZP->field8e) & DONE_BIT) == 0) {
        return exit_of(r);
    }
    wr8(PlayerByte3BD8, 0);
    WR8(ZP->game_sub_mode, MODE_AFTER);
    a = RD8(ZP->stage);
    end.set = EXIT_X | EXIT_Y | EXIT_C | EXIT_V;
    end.x = r.x;
    end.y = r.y;
    end.c = r.c;
    end.v = r.v;
    if (a == 0) {
        WR8(OBJ->f3510[0], STAGE_ZERO_TIMER);
        end.a = STAGE_ZERO_TIMER;
        end.nz = STAGE_ZERO_TIMER;
        return end;
    }
    WR8(OBJ->f3510[0], STAGE_NONZERO_TIMER);
    wr8(PlayerByte3BF0, 0);
    end.a = STAGE_NONZERO_TIMER;
    end.nz = STAGE_NONZERO_TIMER;
    return end;
}

/*
 * @routine 024E:051 RunStageFrame
 * @run 6B:8851
 * WHAT IT DOES: One frame of a stage: counts the wave, ticks the stage (StepStageFrame), runs the
 * player and the camera and the objects, and steps the ending effect; when the ending is done it
 * puts the game in mode 1 and sets the object timer by the stage.
 * HOW: Calls in turn StepStageFrame, the overlay's two common pieces at $80AB and $80BF (original
 * code), PlayerAndCameraFrame, StepScrollAndRunObjects and StepStageEnding.
 * Every stage overlay holds the same code here.
 * WAITS: it can wait where a handler's code does.
 * Entry: X and Y as the tick takes them; A, C and V as the caller left them, which the pieces after
 * a tick that ends early are called with. Exit: as the last piece left it.
 */
StageExit RunStageFrame(uint8_t a, uint8_t x, uint8_t y, bool c, bool v)
{
    Regs r = { a, x, y, c, v, 0 };

    return run_stage_frame(r);
}

void abi_RunStageFrame(void)
{
    stage_exit_shim(RunStageFrame(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}
