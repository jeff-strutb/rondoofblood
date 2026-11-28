/* The stage start of stages 02 and 04 to 08: the same code as StageStart (024D:02D) but for the section order table. */
#include "machine.h"
#include "stage_map.h"
#include "stage_regs.h"

/*
 * @routine 050D:02D StageStart050D
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Stage 02), as StageStart with this overlay's section order table at $9C20.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart050D(StageRegs r)
{
    return StageStartAt(r, 0x9C20);
}

void abi_StageStart050D(void)
{
    stage_regs_load_shim(StageStart050D(regs_entry_shim()));
}

/*
 * @routine 07CD:02D StageStart07CD
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Stage 04), as StageStart with this overlay's section order table at $9CD6.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart07CD(StageRegs r)
{
    return StageStartAt(r, 0x9CD6);
}

void abi_StageStart07CD(void)
{
    stage_regs_load_shim(StageStart07CD(regs_entry_shim()));
}

/*
 * @routine 092D:02D StageStart092D
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Stage 05), as StageStart with this overlay's section order table at $9CD1.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart092D(StageRegs r)
{
    return StageStartAt(r, 0x9CD1);
}

void abi_StageStart092D(void)
{
    stage_regs_load_shim(StageStart092D(regs_entry_shim()));
}

/*
 * @routine 0A8D:02D StageStart0A8D
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Stage 06), as StageStart with this overlay's section order table at $9CAC.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart0A8D(StageRegs r)
{
    return StageStartAt(r, 0x9CAC);
}

void abi_StageStart0A8D(void)
{
    stage_regs_load_shim(StageStart0A8D(regs_entry_shim()));
}

/*
 * @routine 0BED:02D StageStart0BED
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Stage 07), as StageStart with this overlay's section order table at $9CC4.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart0BED(StageRegs r)
{
    return StageStartAt(r, 0x9CC4);
}

void abi_StageStart0BED(void)
{
    stage_regs_load_shim(StageStart0BED(regs_entry_shim()));
}

/*
 * @routine 0D4D:02D StageStart0D4D
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Stage 08), as StageStart with this overlay's section order table at $9CE7.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart0D4D(StageRegs r)
{
    return StageStartAt(r, 0x9CE7);
}

void abi_StageStart0D4D(void)
{
    stage_regs_load_shim(StageStart0D4D(regs_entry_shim()));
}
