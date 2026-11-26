/* The stage's opening sequence: a counter stepped each frame that starts scripts and lists and the score blend. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum {
    SEQUENCE_LAST = 0x1A, SEQUENCE_ENDING = 0x11, PAUSED_BIT1 = 0x02, PAUSED_BIT0 = 0x01,
    FIRST_SLOT = 2, SECOND_SLOT = 0x0B, LIST_CHANNEL = 2, LIST_BASE = 0x64, FRAME_SLOT = 0x57,
    FIRST_SCRIPTS = 0x8F64, SECOND_SCRIPTS = 0x8F71, FRAMES = 0x8F7E, LISTS = 0x8F8B,
    FIRST_RETURN = 0x8F1A, SECOND_RETURN = 0x8F24, CLEAR_RETURN = 0x8F58, STOCK_RETURN = 0x8F61,
    ROW_ENTRIES = 5
};

/* The registers and flags a stage routine's exit sets; what its set mask does not name stays as it was. */
void stage_exit_shim(StageExit end)
{
    if ((end.set & EXIT_CPU) != 0) {
        return;
    }
    cpu.a = end.a;
    abi_nz(end.nz);
    if ((end.set & EXIT_X) != 0) {
        cpu.x = end.x;
    }
    if ((end.set & EXIT_Y) != 0) {
        cpu.y = end.y;
    }
    if ((end.set & EXIT_C) != 0) {
        abi_flag(P_C, end.c);
    }
    if ((end.set & EXIT_V) != 0) {
        abi_flag(P_V, end.v);
    }
}

/* The registers and flags original code or a wrapper left in cpu, as an exit that sets them all. */
StageExit stage_exit_from_cpu_shim(void)
{
    StageExit end = { cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0, 0, EXIT_X | EXIT_Y | EXIT_C | EXIT_V };

    end.nz = (cpu.p & P_Z) != 0 ? 0 : ((cpu.p & P_N) != 0 ? 0x80 : 0x01);
    return end;
}

/* What a stage routine's code holds in the registers while it calls pieces that take them from the machine. */
StageRegs stage_regs_after(StageExit end, StageRegs before)
{
    StageRegs now = before;

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

void stage_regs_load_shim(StageRegs r)
{
    cpu.a = r.a;
    cpu.x = r.x;
    cpu.y = r.y;
    abi_nz(r.nz);
    abi_flag(P_C, r.c);
    abi_flag(P_V, r.v);
}

StageRegs stage_regs_entry_shim(void)
{
    StageRegs r = { cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0, cpu.a };

    return r;
}

StageRegs stage_regs_from_cpu_shim(StageRegs before)
{
    return stage_regs_after(stage_exit_from_cpu_shim(), before);
}

static StageExit exit_of_step(BlendStepEnd step)
{
    StageExit end = { step.a, 0, ROW_ENTRIES, step.c, step.v, step.a, EXIT_C | EXIT_V };

    if (step.y_set) {
        end.set |= EXIT_Y;
    }
    return end;
}

/*
 * The counter reaching its last step: back to zero, the blend flag toggled, and the paused bit 0
 * cleared when the flag is now set.
 */
static StageExit sequence_wraps(void)
{
    StageExit end = { 0, 0, 0, true, false, 0, EXIT_C };
    uint8_t flag;

    RMW8(ZP->paused, paused, paused & ~PAUSED_BIT1);
    WR8(STAGEVARS->state_350e[1], 0);
    flag = RD8(STAGEVARS->state_35be[1]);
    flag = (uint8_t)(flag ^ 0x01);
    WR8(STAGEVARS->state_35be[1], flag);
    if (flag != 0) {
        RMW8(ZP->paused, paused, paused & ~PAUSED_BIT0);
    }
    end.a = flag;
    end.nz = flag;
    return end;
}

/* An odd step: at the ending step with the flag clear, starts the blends over; otherwise nothing. */
static StageExit sequence_odd_step(uint8_t step, uint8_t x, uint8_t y)
{
    StageExit end = { step, 0, 0, step >= SEQUENCE_ENDING, false, (uint8_t)(step - SEQUENCE_ENDING), EXIT_C };
    uint8_t flag;
    BlendStepEnd stock;

    if (step != SEQUENCE_ENDING) {
        return end;
    }
    flag = RD8(STAGEVARS->state_35be[1]);
    if (flag == 0) {
        WR8(STAGEVARS->blend_on[1], 1);
        jsr_push(CLEAR_RETURN);
        ClearStageStateBytes();
        jsr_pop();
        return exit_of_step(BlendStageScoreRows(x));
    }
    WR8(STAGEVARS->blend_on[1], 0);
    jsr_push(STOCK_RETURN);
    stock = BlendStageStockRows(x, y);
    jsr_pop();
    return exit_of_step(BlendStageScoreAddRows(x, stock.y_set ? ROW_ENTRIES : y));
}

/* An even step: starts the step's two scripts, sets the sprite frame, and starts the step's list if it has one. */
static StageExit sequence_even_step(void)
{
    StageExit end = { 0, 0, 0, false, false, 0, EXIT_X | EXIT_Y | EXIT_C };
    uint8_t index;
    uint8_t script;
    uint8_t frame;
    uint8_t list;
    Sum sum;

    LSR8(ZP->temp00);
    index = RD8(ZP->temp00);
    script = rd8((uint16_t)(FIRST_SCRIPTS + index));
    jsr_push(FIRST_RETURN);
    StartScriptSlotKind4(script, FIRST_SLOT);
    jsr_pop();
    index = RD8(ZP->temp00);
    script = rd8((uint16_t)(SECOND_SCRIPTS + index));
    jsr_push(SECOND_RETURN);
    end.y = StartScriptSlotKind4(script, SECOND_SLOT);
    jsr_pop();
    index = RD8(ZP->temp00);
    frame = rd8((uint16_t)(FRAMES + index));
    WR8(OBJ->frame[FRAME_SLOT], frame);
    index = RD8(ZP->temp00);
    list = rd8((uint16_t)(LISTS + index));
    end.x = index;
    end.a = list;
    end.nz = list;
    if (list == 0) {
        return end;
    }
    end.x = LIST_CHANNEL;
    if ((list & 0x80) != 0) {
        sum = add_bytes(LIST_BASE, RD8(STAGEVARS->state_35be[1]), false);
        end.a = StartListChannelKind2(sum.value, LIST_CHANNEL);
        end.nz = end.a;
        end.c = sum.carry;
        end.v = sum.overflow;
        end.set |= EXIT_V;
        return end;
    }
    end.a = StartListChannelKind2(list, LIST_CHANNEL);
    end.nz = end.a;
    return end;
}

/*
 * @routine 024E:6ED StepStageSequence
 * @run 6B:8EED
 * WHAT IT DOES: Steps the stage's opening sequence one count: at the last count ($1A) it
 * starts again with the blend flag toggled; on an even count it starts the count's two script
 * slots, sets a sprite frame and starts the count's list; at count $11 it starts the score
 * blends.
 * HOW: The count is StageVar350F (the second slot of the blend column). The tables of
 * scripts, frames and lists are at $8F64, $8F71, $8F7E and $8F8B, a byte each, by half the count.
 * An odd count other than $11 does nothing.
 * Every stage overlay holds the same code here.
 * WAITS: no.
 * Entry: X and Y as the blends take them. Exit: as the last step left it.
 */
StageExit StepStageSequence(uint8_t x, uint8_t y)
{
    uint8_t count = RD8(STAGEVARS->state_350e[1]);

    WR8(ZP->temp00, count);
    if (count == SEQUENCE_LAST) {
        return sequence_wraps();
    }
    INC8(STAGEVARS->state_350e[1]);
    if ((RD8(ZP->temp00) & 0x01) != 0) {
        return sequence_odd_step(RD8(ZP->temp00), x, y);
    }
    return sequence_even_step();
}

void abi_StepStageSequence(void)
{
    stage_exit_shim(StepStageSequence(cpu.x, cpu.y));
}
