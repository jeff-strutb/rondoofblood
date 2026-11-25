/* The stage's VRAM region masked with random bits: copied into the mask buffer, masked, and written back. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum { MASK_WORDS = 0x80, MASK_BYTES = 0x80, ROW_LIMIT = 0x10, ROW_STEP = 2 };

/* What the routine leaves at its end: A and the carry and overflow if a masking set them. */
typedef struct {
    uint8_t a;
    bool flags_from_mask;
    Sum mask;
    uint8_t y;
} VramMaskExit;

static VramMaskExit mask_vram_rows(bool long_variant)
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

    jsr_push(0x90B8);
    if (long_variant) {
        SetVramAddressesLong(x_lo, y_hi);
    } else {
        SetVramAddresses(x_lo, y_hi);
    }
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
                jsr_push(0x90EA);
                step = MaskStageRowCopy(x, y);
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
            jsr_push(0x90FB);
            step = MaskStageRow(x, y);
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
uint8_t MaskVramRowsRandom(void)
{
    return mask_vram_rows(false).a;
}

void abi_MaskVramRowsRandom(void)
{
    vram_mask_exit_shim(mask_vram_rows(false));
}

/*
 * @routine 12CF:0B1 MaskVramRowsRandomLong
 * @run 6B:90B1
 * WHAT IT DOES: MaskVramRowsRandom for the stages whose overlay keeps SetVramAddresses at $968F.
 * HOW: The same code, calling SetVramAddressesLong.
 * WAITS: no.
 * Entry and exit: as MaskVramRowsRandom.
 */
uint8_t MaskVramRowsRandomLong(void)
{
    return mask_vram_rows(true).a;
}

void abi_MaskVramRowsRandomLong(void)
{
    vram_mask_exit_shim(mask_vram_rows(true));
}
