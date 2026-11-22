/* The stage's tile colour rows: five bytes a row, blended between two rows and written into VRAM words. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum { ROW_ENTRIES = 5, BLEND_NONE = 5 };

/* zTemp00-01 or zTemp06-07 := the row's address: StageTileRows plus five times the row, carries as the adds leave them. */
#define ROW_POINTER(row, lo_field, hi_field, v_out)                       \
    do {                                                                  \
        uint8_t shifted_ = (uint8_t)((row) << 2);                         \
        bool carry_ = ((row) & 0x40) != 0;                                \
        uint8_t byte_;                                                    \
        Sum s1_, s2_, s3_;                                                \
        WR8(lo_field, (row));                                             \
        byte_ = RD8(lo_field);                                            \
        s1_ = add_bytes(shifted_, byte_, carry_);                         \
        s2_ = add_bytes(s1_.value, (uint8_t)StageTileRows, s1_.carry);    \
        WR8(lo_field, s2_.value);                                         \
        s3_ = add_bytes(0, (uint8_t)(StageTileRows >> 8), s2_.carry);     \
        WR8(hi_field, s3_.value);                                         \
        (v_out) = s3_.overflow;                                           \
    } while (0)

/* The value of entry y: from the first row, or while a blend runs from the second row's entries on. */
static uint8_t row_value(uint8_t y, uint8_t x, bool *v)
{
    if (x != BLEND_NONE) {
        uint8_t on = RD8(STAGEVARS->blend_on[0]);

        if (on != 0) {
            uint8_t five;
            uint8_t value;
            Sum sum;

            WR8(ZP->temp05, y);
            five = RD8(ZP->temp05);
            sum = add_bytes(x, five, false);
            *v = sum.overflow;
            push8(y);
            if (sum.value < ROW_ENTRIES) {
                value = RD8_PTR_Y(ZP->temp06, sum.value);
            } else {
                Sum rest = sub_bytes(sum.value, ROW_ENTRIES, true);

                *v = rest.overflow;
                value = RD8_PTR_Y(ZP->temp00, rest.value);
            }
            pull8();
            return value;
        }
    }
    return RD8_PTR_Y(ZP->temp00, y);
}

/* One pass over the five entries, written into the VDC data word the way kind 1, 2 or 3 packs them. */
static uint8_t write_entries(int kind, uint8_t *x_out, bool *v)
{
    uint8_t x = RD8(STAGEVARS->blend_step[0]);
    uint8_t a = 0;
    uint8_t y;

    for (y = 0; y < ROW_ENTRIES; y++) {
        uint8_t value = row_value(y, x, v);
        uint8_t low;
        uint8_t high;
        uint8_t part;
        bool carry;

        if (kind == 1) {
            WR8(ZP->temp03, 0);
            carry = (value & 0x80) != 0;
            value = (uint8_t)(value << 1);
            ROL8(ZP->temp03, carry);
            carry = (value & 0x80) != 0;
            value = (uint8_t)(value << 1);
            ROL8(ZP->temp03, carry);
            WR8(ZP->temp02, value);
            low = RD8(VDC->data_lo);
            part = RD8(ZP->temp02);
            WR8(VDC->data_lo, (uint8_t)((low & 0x0F) | part));
            high = RD8(VDC->data_hi);
            part = RD8(ZP->temp03);
            a = (uint8_t)((high & 0xFC) | part);
            WR8(VDC->data_hi, a);
        } else if (kind == 2) {
            WR8(ZP->temp02, (uint8_t)(value << 2));
            low = RD8(VDC->data_lo);
            WR8(VDC->data_lo, low);
            high = RD8(VDC->data_hi);
            part = RD8(ZP->temp02);
            a = (uint8_t)((high & 0x0F) | part);
            WR8(VDC->data_hi, a);
        } else {
            WR8(ZP->temp02, 0);
            carry = (value & 0x80) != 0;
            value = (uint8_t)(value << 1);
            ROL8(ZP->temp02, carry);
            carry = (value & 0x80) != 0;
            value = (uint8_t)(value << 1);
            ROL8(ZP->temp02, carry);
            low = RD8(VDC->data_lo);
            part = RD8(ZP->temp02);
            WR8(VDC->data_lo, (uint8_t)((low & 0xFC) | part));
            a = RD8(VDC->data_hi);
            WR8(VDC->data_hi, a);
        }
    }
    *x_out = x;
    return a;
}

/* Calls whichever SetStageTileVramAddress the stage's overlay holds at $8E87. */
static void set_tile_address(uint16_t ret)
{
    jsr_push(ret);
    if (resident_at(0x8E87) == DISC(0x024E, 0x687)) {
        SetStageTileVramAddress();
    } else {
        SetStageTileVramAddressLong();
    }
    jsr_pop();
}

static TileRowsExit blend_rows(uint8_t phase, uint8_t row_a, uint8_t row_b)
{
    TileRowsExit out = { 0, 0, false };
    uint8_t step;
    bool v = false;

    WR8(ZP->temp04, phase);
    ROW_POINTER(row_a, ZP->temp00, ZP->temp01, v);
    ROW_POINTER(row_b, ZP->temp06, ZP->temp07, v);
    set_tile_address(0x8DA8);
    step = RD8(ZP->temp04);
    if ((step & 1) == 0) {
        out.a = write_entries(1, &out.x, &v);
    } else {
        write_entries(2, &out.x, &v);
        INC8(ZP->temp04);
        set_tile_address(0x8E40);
        out.a = write_entries(3, &out.x, &v);
    }
    out.v = v;
    return out;
}

/*
 * @routine 024E:584 BlendStageTileRows
 * @run 6B:8D84
 * WHAT IT DOES: Writes five tile colour values into the VRAM words of one tile row step of the
 * stage, taken from one row of StageTileRows, or, while a blend runs (StageBlendOn), from
 * that row and a second one, StageBlendStep entries along.
 * HOW: Each row is five bytes at StageTileRows plus five times its number, its address kept in
 * zTemp00-01 and zTemp06-07. SetStageTileVramAddress points the VDC at the step's word.
 * An even step packs each value into the word's top bits; an odd one writes the low word
 * then, one step on, the next word.
 * WAITS: no.
 * Entry: X the step (zTemp04), A the first row, Y the second row. Exit: A the last byte
 * written, X the blend step, Y 5, N 0, Z 1, C 1, V from the last add.
 */
TileRowsExit BlendStageTileRows(uint8_t phase, uint8_t row_a, uint8_t row_b)
{
    return blend_rows(phase, row_a, row_b);
}

/* What the blend leaves in A, X, Y and the flags. */
void blend_exit_shim(TileRowsExit out)
{
    cpu.a = out.a;
    cpu.x = out.x;
    cpu.y = ROW_ENTRIES;
    abi_nz(0);
    abi_flag(P_C, true);
    abi_flag(P_V, out.v);
}

void abi_BlendStageTileRows(void)
{
    blend_exit_shim(blend_rows(cpu.x, cpu.a, cpu.y));
}

/*
 * @routine 024F:266 BlendStageRowPair
 * @run 6B:9266
 * WHAT IT DOES: Blends the two rows a byte names: its low nibble's row at the step X, its high
 * nibble's row at the step after.
 * HOW: BlendStageTileRows for the low nibble, called, then the same for the high nibble with
 * the step one on, which this routine jumps to with no return of its own. The second row of
 * both blends is the one in Y at entry for the first and 5 for the second (BlendStageTileRows
 * leaves Y 5).
 * WAITS: no.
 * Entry: A the byte, X the step, Y the second row of the first blend. Exit: as BlendStageTileRows.
 */
TileRowsExit BlendStageRowPair(uint8_t x, uint8_t a, uint8_t y)
{
    push8(x);
    push8(a);
    jsr_push(0x926D);
    BlendStageTileRows(x, (uint8_t)(a & 0x0F), y);
    jsr_pop();
    a = pull8();
    x = pull8();
    return BlendStageTileRows((uint8_t)(x + 1), (uint8_t)(a >> 4), ROW_ENTRIES);
}

void abi_BlendStageRowPair(void)
{
    blend_exit_shim(BlendStageRowPair(cpu.x, cpu.a, cpu.y));
}

/*
 * @routine 024F:236 BlendStageRowChanges
 * @run 6B:9236
 * WHAT IT DOES: Blends the rows a byte changed: of the two bytes (the old in zBiosAl, the new in
 * A), for each nibble that differs, the old nibble's row is blended in at the step X (low
 * nibble) or the step after (high nibble).
 * HOW: The low nibbles are compared, and BlendStageTileRows called with the old low nibble as
 * the second row when they differ; then the same for the high nibbles one step on.
 * WAITS: no.
 * Entry: A the new byte, zBiosAl the old, X the step. Exit: A the high nibble when nothing
 * was blended last, else as BlendStageTileRows; X the step plus one or BlendStageTileRows's; N 0, Z 1,
 * C 1; Y 5 once a blend ran, else kept.
 */
typedef struct {
    uint8_t a;
    uint8_t x;
    bool y_set;
    bool v_set;
    bool v;
} ChangesExit;

static ChangesExit blend_changes(uint8_t x, uint8_t new_byte)
{
    ChangesExit out = { 0, 0, false, false, false };
    uint8_t old_low;
    uint8_t low;
    uint8_t high;
    uint8_t old_high;
    TileRowsExit blend;

    push8(x);
    WR8(ZP->bios_b.lo, new_byte);
    old_low = RD8(ZP->bios_a.lo);
    old_low = (uint8_t)(old_low & 0x0F);
    WR8(ZP->bios_a.hi, old_low);
    low = RD8(ZP->bios_b.lo);
    low = (uint8_t)(low & 0x0F);
    old_low = RD8(ZP->bios_a.hi);
    if (low != old_low) {
        old_low = RD8(ZP->bios_a.hi);
        jsr_push(0x924C);
        blend = BlendStageTileRows(x, low, old_low);
        jsr_pop();
        out.y_set = true;
        out.v_set = true;
        out.v = blend.v;
    }
    high = RD8(ZP->bios_a.lo);
    high = (uint8_t)(high >> 4);
    WR8(ZP->bios_a.hi, high);
    old_high = RD8(ZP->bios_b.lo);
    old_high = (uint8_t)(old_high >> 4);
    x = pull8();
    x = (uint8_t)(x + 1);
    high = RD8(ZP->bios_a.hi);
    out.a = old_high;
    out.x = x;
    if (old_high != high) {
        high = RD8(ZP->bios_a.hi);
        jsr_push(0x9265);
        blend = BlendStageTileRows(x, old_high, high);
        jsr_pop();
        out.a = blend.a;
        out.x = blend.x;
        out.y_set = true;
        out.v_set = true;
        out.v = blend.v;
    }
    return out;
}

TileRowsExit BlendStageRowChanges(uint8_t x, uint8_t new_byte)
{
    ChangesExit out = blend_changes(x, new_byte);
    TileRowsExit result = { out.a, out.x, out.v };

    return result;
}

void abi_BlendStageRowChanges(void)
{
    ChangesExit out = blend_changes(cpu.x, cpu.a);

    cpu.a = out.a;
    cpu.x = out.x;
    if (out.y_set) {
        cpu.y = ROW_ENTRIES;
    }
    abi_nz(0);
    abi_flag(P_C, true);
    if (out.v_set) {
        abi_flag(P_V, out.v);
    }
}
