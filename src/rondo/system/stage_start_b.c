/*
 * The stage's start in the overlays of stages $09 to $10 and the secret stages: the same code as StageStart
 * (stage_flow.c) but for the address of the section order table, so each runs StageStartAt with its own.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "stage_map.h"
#include "stage_regs.h"
#include "stage_start_b.h"


/*
 * @routine 05BD:02D StageStart05BD
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Secret stage 02), as StageStart with this overlay's section order table at $9E77.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart05BD(StageRegs r)
{
    return StageStartAt(r, 0x9E77);
}

void abi_StageStart05BD(void)
{
    stage_regs_load_shim(StageStart05BD(regs_entry_shim()));
}

/*
 * @routine 071D:02D StageStart071D
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Secret stage 03), as StageStart with this overlay's section order table at $9F3B.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart071D(StageRegs r)
{
    return StageStartAt(r, 0x9F3B);
}

void abi_StageStart071D(void)
{
    stage_regs_load_shim(StageStart071D(regs_entry_shim()));
}

/*
 * @routine 087D:02D StageStart087D
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Secret stage 04), as StageStart with this overlay's section order table at $9E64.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart087D(StageRegs r)
{
    return StageStartAt(r, 0x9E64);
}

void abi_StageStart087D(void)
{
    stage_regs_load_shim(StageStart087D(regs_entry_shim()));
}

/*
 * @routine 09DD:02D StageStart09DD
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Secret stage 05), as StageStart with this overlay's section order table at $9EAB.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart09DD(StageRegs r)
{
    return StageStartAt(r, 0x9EAB);
}

void abi_StageStart09DD(void)
{
    stage_regs_load_shim(StageStart09DD(regs_entry_shim()));
}

/*
 * @routine 0B3D:02D StageStart0B3D
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Secret stage 06), as StageStart with this overlay's section order table at $9E76.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart0B3D(StageRegs r)
{
    return StageStartAt(r, 0x9E76);
}

void abi_StageStart0B3D(void)
{
    stage_regs_load_shim(StageStart0B3D(regs_entry_shim()));
}

/*
 * @routine 0C9D:02D StageStart0C9D
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Secret stage 07), as StageStart with this overlay's section order table at $9E79.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart0C9D(StageRegs r)
{
    return StageStartAt(r, 0x9E79);
}

void abi_StageStart0C9D(void)
{
    stage_regs_load_shim(StageStart0C9D(regs_entry_shim()));
}

/*
 * @routine 0DFD:02D StageStart0DFD
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Secret stage 08), as StageStart with this overlay's section order table at $9EA7.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart0DFD(StageRegs r)
{
    return StageStartAt(r, 0x9EA7);
}

void abi_StageStart0DFD(void)
{
    stage_regs_load_shim(StageStart0DFD(regs_entry_shim()));
}

/*
 * @routine 0EAD:02D StageStart0EAD
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Stage 09), as StageStart with this overlay's section order table at $9DF6.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart0EAD(StageRegs r)
{
    return StageStartAt(r, 0x9DF6);
}

void abi_StageStart0EAD(void)
{
    stage_regs_load_shim(StageStart0EAD(regs_entry_shim()));
}

/*
 * @routine 0F5D:02D StageStart0F5D
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Secret stage 09), as StageStart with this overlay's section order table at $9F1C.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart0F5D(StageRegs r)
{
    return StageStartAt(r, 0x9F1C);
}

void abi_StageStart0F5D(void)
{
    stage_regs_load_shim(StageStart0F5D(regs_entry_shim()));
}

/*
 * @routine 100D:02D StageStart100D
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Stage 0A), as StageStart with this overlay's section order table at $9C75.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart100D(StageRegs r)
{
    return StageStartAt(r, 0x9C75);
}

void abi_StageStart100D(void)
{
    stage_regs_load_shim(StageStart100D(regs_entry_shim()));
}

/*
 * @routine 10BD:02D StageStart10BD
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Secret stage 0A), as StageStart with this overlay's section order table at $9F06.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart10BD(StageRegs r)
{
    return StageStartAt(r, 0x9F06);
}

void abi_StageStart10BD(void)
{
    stage_regs_load_shim(StageStart10BD(regs_entry_shim()));
}

/*
 * @routine 116D:02D StageStart116D
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Stage 0B), as StageStart with this overlay's section order table at $9C77.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart116D(StageRegs r)
{
    return StageStartAt(r, 0x9C77);
}

void abi_StageStart116D(void)
{
    stage_regs_load_shim(StageStart116D(regs_entry_shim()));
}

/*
 * @routine 12CD:02D StageStart12CD
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Stage 0C), as StageStart with this overlay's section order table at $9F35.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart12CD(StageRegs r)
{
    return StageStartAt(r, 0x9F35);
}

void abi_StageStart12CD(void)
{
    stage_regs_load_shim(StageStart12CD(regs_entry_shim()));
}

/*
 * @routine 160D:02D StageStart160D
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Stage 0E), as StageStart with this overlay's section order table at $9CA5.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart160D(StageRegs r)
{
    return StageStartAt(r, 0x9CA5);
}

void abi_StageStart160D(void)
{
    stage_regs_load_shim(StageStart160D(regs_entry_shim()));
}

/*
 * @routine 16BD:02D StageStart16BD
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Secret stage 0E), as StageStart with this overlay's section order table at $9F42.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart16BD(StageRegs r)
{
    return StageStartAt(r, 0x9F42);
}

void abi_StageStart16BD(void)
{
    stage_regs_load_shim(StageStart16BD(regs_entry_shim()));
}

/*
 * @routine 18CD:02D StageStart18CD
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start (Stage 10), as StageStart with this overlay's section order table at $9EA3.
 * HOW: StageStartAt.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as StageStart.
 */
StageRegs StageStart18CD(StageRegs r)
{
    return StageStartAt(r, 0x9EA3);
}

void abi_StageStart18CD(void)
{
    stage_regs_load_shim(StageStart18CD(regs_entry_shim()));
}
