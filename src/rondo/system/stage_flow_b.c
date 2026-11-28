/*
 * The stage flow's copies in the overlays of stages $09 to $10 and the secret stages that differ from stage_flow.c's
 * only in a table's address.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "stage_map.h"
#include "stage_regs.h"
#include "stage_flow_b.h"

/*
 * @routine 05BD:66A StageSectionNext05BD
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Secret stage 02): zField82 counted on and zField83
 * set from this overlay's section order ($9E77) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext05BD(StageRegs r)
{
    return StageSectionNextAt(r, 0x9E77);
}

void abi_StageSectionNext05BD(void)
{
    stage_regs_load_shim(StageSectionNext05BD(regs_entry_shim()));
}

/*
 * @routine 071D:66A StageSectionNext071D
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Secret stage 03): zField82 counted on and zField83
 * set from this overlay's section order ($9F3B) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext071D(StageRegs r)
{
    return StageSectionNextAt(r, 0x9F3B);
}

void abi_StageSectionNext071D(void)
{
    stage_regs_load_shim(StageSectionNext071D(regs_entry_shim()));
}

/*
 * @routine 087D:66A StageSectionNext087D
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Secret stage 04): zField82 counted on and zField83
 * set from this overlay's section order ($9E64) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext087D(StageRegs r)
{
    return StageSectionNextAt(r, 0x9E64);
}

void abi_StageSectionNext087D(void)
{
    stage_regs_load_shim(StageSectionNext087D(regs_entry_shim()));
}

/*
 * @routine 09DD:66A StageSectionNext09DD
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Secret stage 05): zField82 counted on and zField83
 * set from this overlay's section order ($9EAB) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext09DD(StageRegs r)
{
    return StageSectionNextAt(r, 0x9EAB);
}

void abi_StageSectionNext09DD(void)
{
    stage_regs_load_shim(StageSectionNext09DD(regs_entry_shim()));
}

/*
 * @routine 0B3D:66A StageSectionNext0B3D
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Secret stage 06): zField82 counted on and zField83
 * set from this overlay's section order ($9E76) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext0B3D(StageRegs r)
{
    return StageSectionNextAt(r, 0x9E76);
}

void abi_StageSectionNext0B3D(void)
{
    stage_regs_load_shim(StageSectionNext0B3D(regs_entry_shim()));
}

/*
 * @routine 0C9D:66A StageSectionNext0C9D
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Secret stage 07): zField82 counted on and zField83
 * set from this overlay's section order ($9E79) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext0C9D(StageRegs r)
{
    return StageSectionNextAt(r, 0x9E79);
}

void abi_StageSectionNext0C9D(void)
{
    stage_regs_load_shim(StageSectionNext0C9D(regs_entry_shim()));
}

/*
 * @routine 0DFD:66A StageSectionNext0DFD
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Secret stage 08): zField82 counted on and zField83
 * set from this overlay's section order ($9EA7) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext0DFD(StageRegs r)
{
    return StageSectionNextAt(r, 0x9EA7);
}

void abi_StageSectionNext0DFD(void)
{
    stage_regs_load_shim(StageSectionNext0DFD(regs_entry_shim()));
}

/*
 * @routine 0EAD:66A StageSectionNext0EAD
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Stage 09): zField82 counted on and zField83
 * set from this overlay's section order ($9DF6) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext0EAD(StageRegs r)
{
    return StageSectionNextAt(r, 0x9DF6);
}

void abi_StageSectionNext0EAD(void)
{
    stage_regs_load_shim(StageSectionNext0EAD(regs_entry_shim()));
}

/*
 * @routine 0F5D:66A StageSectionNext0F5D
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Secret stage 09): zField82 counted on and zField83
 * set from this overlay's section order ($9F1C) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext0F5D(StageRegs r)
{
    return StageSectionNextAt(r, 0x9F1C);
}

void abi_StageSectionNext0F5D(void)
{
    stage_regs_load_shim(StageSectionNext0F5D(regs_entry_shim()));
}

/*
 * @routine 100D:66A StageSectionNext100D
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Stage 0A): zField82 counted on and zField83
 * set from this overlay's section order ($9C75) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext100D(StageRegs r)
{
    return StageSectionNextAt(r, 0x9C75);
}

void abi_StageSectionNext100D(void)
{
    stage_regs_load_shim(StageSectionNext100D(regs_entry_shim()));
}

/*
 * @routine 10BD:66A StageSectionNext10BD
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Secret stage 0A): zField82 counted on and zField83
 * set from this overlay's section order ($9F06) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext10BD(StageRegs r)
{
    return StageSectionNextAt(r, 0x9F06);
}

void abi_StageSectionNext10BD(void)
{
    stage_regs_load_shim(StageSectionNext10BD(regs_entry_shim()));
}

/*
 * @routine 116D:66A StageSectionNext116D
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Stage 0B): zField82 counted on and zField83
 * set from this overlay's section order ($9C77) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext116D(StageRegs r)
{
    return StageSectionNextAt(r, 0x9C77);
}

void abi_StageSectionNext116D(void)
{
    stage_regs_load_shim(StageSectionNext116D(regs_entry_shim()));
}

/*
 * @routine 12CD:66A StageSectionNext12CD
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Stage 0C): zField82 counted on and zField83
 * set from this overlay's section order ($9F35) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext12CD(StageRegs r)
{
    return StageSectionNextAt(r, 0x9F35);
}

void abi_StageSectionNext12CD(void)
{
    stage_regs_load_shim(StageSectionNext12CD(regs_entry_shim()));
}

/*
 * @routine 160D:66A StageSectionNext160D
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Stage 0E): zField82 counted on and zField83
 * set from this overlay's section order ($9CA5) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext160D(StageRegs r)
{
    return StageSectionNextAt(r, 0x9CA5);
}

void abi_StageSectionNext160D(void)
{
    stage_regs_load_shim(StageSectionNext160D(regs_entry_shim()));
}

/*
 * @routine 16BD:66A StageSectionNext16BD
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Secret stage 0E): zField82 counted on and zField83
 * set from this overlay's section order ($9F42) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext16BD(StageRegs r)
{
    return StageSectionNextAt(r, 0x9F42);
}

void abi_StageSectionNext16BD(void)
{
    stage_regs_load_shim(StageSectionNext16BD(regs_entry_shim()));
}

/*
 * @routine 18CD:66A StageSectionNext18CD
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section (Stage 10): zField82 counted on and zField83
 * set from this overlay's section order ($9EA3) by it.
 * HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNext18CD(StageRegs r)
{
    return StageSectionNextAt(r, 0x9EA3);
}

void abi_StageSectionNext18CD(void)
{
    stage_regs_load_shim(StageSectionNext18CD(regs_entry_shim()));
}
