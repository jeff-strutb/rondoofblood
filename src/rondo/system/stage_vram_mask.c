/* The stage's VRAM region masked with random bits: copied into the mask buffer, masked, and written back. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"
#include "stage_regs.h"

enum { MASK_WORDS = 0x80, MASK_BYTES = 0x80, ROW_LIMIT = 0x10, ROW_STEP = 2 };


/* The body, at the address it runs at (its JSRs return to at + 7, + $39 and + $4A), with the C of its VRAM address
 * setter and of the two row masks it calls. */
static VramMaskExit mask_vram_rows(uint16_t at, void (*point)(uint8_t lo, uint8_t hi),
                                   MaskExit (*row_copy)(uint8_t x, uint8_t y), MaskExit (*row)(uint8_t x, uint8_t y))
{
    VramMaskExit out = { 0, false, { 0, false, false }, 0 };
    uint8_t x;
    uint8_t y;
    uint8_t byte;
    uint8_t random;
    uint8_t other;
    MaskExit step;
    uint8_t x_lo = RD8(ZP->temp10);
    uint8_t y_hi = RD8(ZP->temp11);

    jsr_push((uint16_t)(at + 7));
    point(x_lo, y_hi);
    jsr_pop();
    for (x = MASK_WORDS; x != 0; x = (uint8_t)(x - ROW_STEP)) {
        byte = RD8(VDC->data_lo);
        WR8(STAGEWORK->mask_buffer[(uint8_t)(x - 1)], byte);
        byte = RD8(VDC->data_hi);
        WR8(STAGEWORK->mask_buffer[(uint8_t)(x - 2)], byte);
    }
    y = RD8(ZP->sprites_left);
    out.y = y;
    if (y >= ROW_LIMIT) {
        int cell;

        for (cell = MASK_BYTES - 1; cell >= 0; cell--) {
            WR8(STAGEWORK->mask_buffer[cell], 0);
        }
    } else {
        y--;
        if (y != 0) {
            x = 0;
            do {
                random = RD8(ZP->random2);
                byte = (uint8_t)((random >> 1) | 0x80);
                other = RD8(ZP->random2);
                WR8(ZP->temp00, (uint8_t)(byte | other));
                jsr_push((uint16_t)(at + 0x39));
                step = row_copy(x, y);
                jsr_pop();
                out.mask = step.last;
                out.flags_from_mask = true;
                x = (uint8_t)(x + ROW_STEP);
                y--;
            } while (y != 0);
        }
        y = RD8(ZP->sprites_left);
        x = 0;
        do {
            random = RD8(ZP->random2);
            other = RD8(ZP->random3);
            WR8(ZP->temp00, (uint8_t)(random | other));
            jsr_push((uint16_t)(at + 0x4A));
            step = row(x, y);
            jsr_pop();
            out.mask = step.last;
            out.flags_from_mask = true;
            x = (uint8_t)(x + ROW_STEP);
            y--;
        } while (y != 0);
        out.y = 0;
    }
    for (x = MASK_WORDS; x != 0; x = (uint8_t)(x - ROW_STEP)) {
        byte = RD8(STAGEWORK->mask_buffer[(uint8_t)(x - 1)]);
        WR8(VDC->data_lo, byte);
        out.a = RD8(STAGEWORK->mask_buffer[(uint8_t)(x - 2)]);
        WR8(VDC->data_hi, out.a);
    }
    return out;
}

static void vram_mask_exit_shim(VramMaskExit out)
{
    cpu.a = out.a;
    cpu.x = 0;
    cpu.y = out.y;
    abi_nz(0);
    if (out.flags_from_mask) {
        abi_flag(P_C, out.mask.carry);
        abi_flag(P_V, out.mask.overflow);
    } else {
        abi_flag(P_C, true);
    }
}

/*
 * @routine 024F:0B1 MaskVramRowsRandom
 * @run 6B:90B1
 * WHAT IT DOES: Fades part of a VRAM region out in a random pattern: reads $80 words from the
 * VRAM address in zTemp10-11 into the mask buffer, masks rows of the buffer with the random
 * bytes, as many rows as zMultiplierHi says (all of it cleared when that is $10 or more), and
 * writes the words back.
 * HOW: SetVramAddresses points the VDC at the region for reading and writing. Rows below the
 * last are masked from the cells two on (MaskStageRowCopy) with a mask made of zRandom2 and its
 * high bit; the last pass masks in place (MaskStageRow) with zRandom2 ORed with zRandom3.
 * WAITS: no.
 * Entry: zTemp10-11 the VRAM address, zMultiplierHi the row count. Exit: A the last byte
 * written, X 0, N 0, Z 1, C and V from the last masking (C 1 when the buffer was cleared), Y 0
 * or the row count when cleared.
 */
VramMaskExit MaskVramRowsRandom(void)
{
    return mask_vram_rows(0x90B1, SetVramAddresses, MaskStageRowCopy, MaskStageRow);
}

void abi_MaskVramRowsRandom(void)
{
    vram_mask_exit_shim(mask_vram_rows(0x90B1, SetVramAddresses, MaskStageRowCopy, MaskStageRow));
}

/*
 * @routine 12CF:0B1 MaskVramRowsRandomLong
 * @run 6B:90B1
 * WHAT IT DOES: MaskVramRowsRandom for the stages whose overlay keeps SetVramAddresses at $968F.
 * HOW: The same code, calling SetVramAddressesLong.
 * WAITS: no.
 * Entry and exit: as MaskVramRowsRandom.
 */
VramMaskExit MaskVramRowsRandomLong(void)
{
    return mask_vram_rows(0x90B1, SetVramAddressesLong, MaskStageRowCopy, MaskStageRow);
}

void abi_MaskVramRowsRandomLong(void)
{
    vram_mask_exit_shim(mask_vram_rows(0x90B1, SetVramAddressesLong, MaskStageRowCopy, MaskStageRow));
}

/* MaskVramRowsRandom's body for the builds of the overlay that hold it elsewhere (stage_programs_census.c), at the
 * address their code runs at, with their VRAM address setter and row masks; and what it leaves, set on the registers. */
VramMaskExit MaskVramRowsRandomAt(uint16_t at, void (*point)(uint8_t lo, uint8_t hi),
                                  MaskExit (*row_copy)(uint8_t x, uint8_t y), MaskExit (*row)(uint8_t x, uint8_t y))
{
    return mask_vram_rows(at, point, row_copy, row);
}

void mask_vram_rows_exit_shim(VramMaskExit out)
{
    vram_mask_exit_shim(out);
}
