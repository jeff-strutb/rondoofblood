/* The stage's mask buffer: 128 bytes the stage's map drawing masks a row at a time. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum { MASK_STEPS = 8 };

/*
 * Eight cells of the buffer, at the table's offsets from a row base, each ANDed with the
 * mask in zTemp00: from the buffer at `from` cells further on (a copy) or from itself.
 * The caller's X and Y go on the stack and come back.
 */
static MaskExit mask_cells(uint8_t x, uint8_t y, uint16_t from)
{
    MaskExit out = { 0, { 0, false, false } };
    int step;

    WR8(ZP->temp08, x);
    push8(x);
    push8(y);
    for (step = MASK_STEPS - 1; step >= 0; step--) {
        uint8_t offset;
        uint8_t base;
        uint8_t cell;
        uint8_t mask;

        offset = TBL8(StageMaskOffsets, step);
        base = RD8(ZP->temp08);
        out.last = add_bytes(offset, base, false);
        cell = RD8(STAGEWORK->mask_buffer[(uint16_t)(out.last.value + from)]);
        mask = RD8(ZP->temp00);
        out.a = (uint8_t)(cell & mask);
        WR8(STAGEWORK->mask_buffer[out.last.value], out.a);
    }
    pull8();
    pull8();
    return out;
}

static void show_mask_exit(MaskExit out, uint8_t x)
{
    cpu.a = out.a;
    abi_nz(x);
    abi_flag(P_C, out.last.carry);
    abi_flag(P_V, out.last.overflow);
}

/*
 * @routine 024F:113 MaskStageRowCopy
 * @run 6B:9113
 * WHAT IT DOES: Masks one row of the stage's mask buffer from the cells two on: each of the
 * row's eight cells takes the cell two bytes after it, ANDed with the mask in zTemp00.
 * HOW: The row's base offset is added to each of the eight offsets of StageMaskOffsets, from
 * the last to the first. X and Y go on the stack and come back.
 * WAITS: no.
 * Entry: X the row's base offset, zTemp00 the mask, Y kept for the caller. Exit: A the last
 * cell written, N and Z from X, C and V from the last offset add. X and Y kept.
 */
MaskExit MaskStageRowCopy(uint8_t x, uint8_t y)
{
    return mask_cells(x, y, 2);
}

void abi_MaskStageRowCopy(void)
{
    uint8_t x = cpu.x;

    show_mask_exit(mask_cells(x, cpu.y, 2), x);
}

/*
 * @routine 024F:12E MaskStageRow
 * @run 6B:912E
 * WHAT IT DOES: Masks one row of the stage's mask buffer in place: each of the row's eight
 * cells is ANDed with the mask in zTemp00.
 * HOW: As MaskStageRowCopy, from the cell itself.
 * WAITS: no.
 * Entry: X the row's base offset, zTemp00 the mask, Y kept for the caller. Exit: as
 * MaskStageRowCopy.
 */
MaskExit MaskStageRow(uint8_t x, uint8_t y)
{
    return mask_cells(x, y, 0);
}

void abi_MaskStageRow(void)
{
    uint8_t x = cpu.x;

    show_mask_exit(mask_cells(x, cpu.y, 0), x);
}
