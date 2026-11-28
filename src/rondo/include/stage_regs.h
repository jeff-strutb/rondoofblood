/* The registers and flags a stage routine is entered with and leaves (StageRegs), for the stage writer's files. */
#ifndef RONDO_STAGE_REGS_H
#define RONDO_STAGE_REGS_H

#include "machine.h"
#include "stage_map.h"

/* The registers and flags the routine is entered with, N and Z as the flags hold them. */
static inline StageRegs regs_entry_shim(void)
{
    StageRegs r = { cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0, 0 };

    r.nz = (cpu.p & P_Z) != 0 ? 0 : ((cpu.p & P_N) != 0 ? 0x80 : 0x01);
    return r;
}

/* A load into A: N and Z from the value. */
static inline StageRegs regs_loaded(StageRegs r, uint8_t value)
{
    r.a = value;
    r.nz = value;
    return r;
}

/* A compare with A, X or Y: C from no borrow, N and Z from the difference. */
static inline StageRegs regs_compared(StageRegs r, uint8_t reg, uint8_t value)
{
    r.c = reg >= value;
    r.nz = (uint8_t)(reg - value);
    return r;
}

/* system/stage_flow.c: StageStart (024D:02D) with the section order table at order_table, for the copies whose
 * only difference is that operand. */
StageRegs StageStartAt(StageRegs r, uint16_t order_table);

/* system/stage_flow.c: StagePauseToggle (024D:1A2) with ret the address after its JSR to StagePauseSet. */
StageRegs StagePauseToggleAt(StageRegs r, uint16_t ret);

#endif
