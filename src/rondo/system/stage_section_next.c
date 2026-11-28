/*
 * Stages 04 and 05's copies of StageSectionNext: only their section orders differ.
 */
#include "machine.h"
#include "ram.h"
#include "stage_map.h"
#include "stage_regs.h"
#include "stage_flow_b.h"
#include "stage_section_next.h"

/*
 * @routine 07CD:66A StageSectionNext07CD
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (stage 04): zField82 counted on and zField83
 * set from this overlay's section order ($9CD6) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext07CD(StageRegs r)
{
    return StageSectionNextAt(r, 0x9CD6);
}

void abi_StageSectionNext07CD(void)
{
    stage_regs_load_shim(StageSectionNext07CD(regs_entry_shim()));
}

/*
 * @routine 092D:66A StageSectionNext092D
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (stage 05): zField82 counted on and zField83
 * set from this overlay's section order ($9CD1) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext092D(StageRegs r)
{
    return StageSectionNextAt(r, 0x9CD1);
}

void abi_StageSectionNext092D(void)
{
    stage_regs_load_shim(StageSectionNext092D(regs_entry_shim()));
}
