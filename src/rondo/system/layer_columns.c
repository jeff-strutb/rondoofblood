/*
 * A column of a layer's tile blocks drawn to the VRAM: the place a scroll entry has reached, its blocks read from
 * the level's screen maps through the stage's block tables, each block's sixteen words copied by
 * CopyTileBlockToVram. The code is the same in the overlays of the stages (sector 0255's and its copies); each
 * overlay has its own block tables, so the call at $A80F goes to whichever SelectBlockTables is resident.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "layers.h"
#include "vram_update.h"
#include "layer_columns.h"

enum {
    BLOCK_TABLES_RUN = 0xA80F,
    COLUMN_Y_OFFSETS = 0xA780,         /* the overlay's offsets for an entry with bit 7 of $3E66 set: low bytes at
                                        * $A780, high at $A782, a word each, the second taken */
    COLUMN_ROWS_BYTE = 0x40,           /* past this, the column goes on in the screen below */
    SCREEN_ROWS_MASK = 0x0F
};

typedef Sum (*BlockTablesFn)(uint8_t index);

static const struct {
    uint32_t disc;
    BlockTablesFn select;
} BLOCK_TABLES[] = {
    { DISC(0x0256, 0x00F), SelectBlockTablesA }, { DISC(0x0516, 0x00F), SelectBlockTablesB },
    { DISC(0x05C6, 0x00F), SelectBlockTablesC }, { DISC(0x0726, 0x00F), SelectBlockTablesD },
    { DISC(0x07D6, 0x00F), SelectBlockTablesE }, { DISC(0x0936, 0x00F), SelectBlockTablesF },
    { DISC(0x0A96, 0x00F), SelectBlockTablesG }, { DISC(0x0BF6, 0x00F), SelectBlockTablesH },
    { DISC(0x0D56, 0x00F), SelectBlockTablesI }, { DISC(0x0EB6, 0x00F), SelectBlockTablesJ },
    { DISC(0x1016, 0x00F), SelectBlockTablesK }, { DISC(0x1176, 0x00F), SelectBlockTablesM },
    { DISC(0x1616, 0x00F), SelectBlockTablesO }, { DISC(0x18D6, 0x00F), SelectBlockTablesP },
};

/* The registers at the JSR $A80F when the resident tables have no C: A the screen map's byte (N and Z from it, C and
 * V clear after ScreenMapAt), X the entry, Y as the caller holds it. */
static void block_tables_regs_shim(uint8_t index, uint8_t entry, uint8_t y)
{
    cpu.a = index;
    cpu.x = entry;
    cpu.y = y;
    abi_nz(index);
    abi_flag(P_C, false);
    abi_flag(P_V, false);
}

/* The JSR $A80F, to the resident overlay's SelectBlockTables. */
static void block_tables(uint8_t index, uint16_t ret, uint8_t entry, uint8_t y)
{
    uint32_t disc = resident_at(BLOCK_TABLES_RUN);

    for (unsigned i = 0; i < sizeof BLOCK_TABLES / sizeof BLOCK_TABLES[0]; i++) {
        if (BLOCK_TABLES[i].disc == disc) {
            jsr_push(ret);
            (void)BLOCK_TABLES[i].select(index);
            jsr_pop();
            return;
        }
    }
    block_tables_regs_shim(index, entry, y);
    call_orig(ret, BLOCK_TABLES_RUN);
}

/*
 * @routine 0256:03D BlockIndexForPlace
 * @run 6F:A83D
 * WHAT IT DOES: For the place in zTemp02-03 (X: the low byte and the screen) and zTemp06-07 (Y: the low byte and the
 * screen row): the VRAM address of the place's block in zTemp10-11, the screen map's byte for the place with the
 * stage's block tables selected by it (zTemp0C, zTemp04), and the block's index within the screen in Y; then runs
 * into BitmapBitShifted, whose bit is the block's bank.
 * HOW: Shifts through the carry: the screen's bit 0 above the X byte, the row's bit 0 above the Y byte; ScreenMapAt
 * and the resident SelectBlockTables; Y the X byte's top three bits or'd with the Y byte's bits 5 to 3.
 * WAITS: no.
 * Entry: X the scroll entry. Exit: as BitmapBitShifted. Clobbers zTemp00, 01, 04, 05, 0C to 0E, 10 and 11.
 */
BitmapBit BlockIndexForPlace(uint8_t entry, uint8_t y)
{
    uint8_t screen, low, row, value, map;
    bool carry;

    screen = RD8(ZP->temp03);
    carry = (screen & 0x01) != 0;
    low = RD8(ZP->temp02);
    value = (uint8_t)((low >> 1) | (carry ? 0x80 : 0x00));
    value = (uint8_t)((value >> 2) & 0x3C);
    WR8(ZP->temp10, value);
    row = RD8(ZP->temp07);
    carry = (row & 0x01) != 0;
    value = RD8(ZP->temp06);
    WR8(ZP->temp11, value);
    (void)ROR8(ZP->temp11, carry);
    (void)LSR8(ZP->temp11);
    (void)LSR8(ZP->temp11);
    (void)LSR8(ZP->temp11);
    (void)LSR8(ZP->temp11);
    value = RD8(ZP->temp03);
    WR8(ZP->temp00, value);
    row = RD8(ZP->temp07);
    jsr_push(0xA863);
    map = ScreenMapAt(row);
    jsr_pop();
    block_tables(map, 0xA866, entry, y);
    low = RD8(ZP->temp02);
    WR8(ZP->temp00, (uint8_t)(low >> 5));
    value = RD8(ZP->temp06);
    value = (uint8_t)((value >> 2) & 0x38);
    value = (uint8_t)(value | RD8(ZP->temp00));
    return BitmapBitShifted(entry, value);         /* runs into it at $A878 */
}

static void bitmap_bit_exit_shim(BitmapBit bit)
{
    cpu.a = bit.a;
    cpu.x = bit.x;
    cpu.y = bit.y;
    abi_nz(bit.x);
    abi_flag(P_C, bit.carry);
}

void abi_BlockIndexForPlace(void)
{
    bitmap_bit_exit_shim(BlockIndexForPlace(cpu.x, cpu.y));
}

/*
 * @routine 0255:7A5 DrawBlockColumn
 * @run 6F:A7A5
 * WHAT IT DOES: Draws a column of a layer's tile blocks for scroll entry X: as many blocks as $3E66's low seven bits,
 * from the place the entry has reached (its scroll X plus the overlay's offset of word Y, and its scroll Y, plus the
 * offset at $A781 and $A783 when $3E66's bit 7 is set), down the screen and on into the screen below.
 * HOW: BlockIndexForPlace for the first block; then each block number from the second table (zTemp04) by Y goes to
 * CopyTileBlockToVram with the bank bit; Y steps by 8 (a row of blocks) with the bit read again, and past $40 the
 * next screen row's map byte selects the tables again. zTemp08 counts the blocks.
 * WAITS: no.
 * Entry: X the entry, Y the offset's index. Exit: A $10, N 0, Z 1 (the count's decrement), C 1, V from the last block
 * copy, Y the last block's index, X kept.
 */
ColumnEnd DrawBlockColumn(uint8_t entry, uint8_t index)
{
    ColumnEnd r = { 0 };
    uint8_t value, y, block, map;
    BitmapBit bit;
    Sum sum;

    value = RD8(SCROLLE->f3e66[entry]);
    WR8(ZP->temp08, (uint8_t)(value & 0x7F));
    jsr_push(0xA7AF);
    (void)ScrollXPlusOffset(entry, index);
    jsr_pop();
    value = RD8(SCROLL->y_lo[entry]);
    WR8(ZP->temp06, value);
    value = RD8(SCROLLE->y_hi_screen[entry]);
    WR8(ZP->temp07, value);
    y = index;
    if ((RD8(SCROLLE->f3e66[entry]) & 0x80) != 0) {
        y = 1;
        value = RD8(SCROLL->y_lo[entry]);
        sum = add_bytes(value, rd8((uint16_t)(COLUMN_Y_OFFSETS + y)), false);
        WR8(ZP->temp06, sum.value);
        value = RD8(SCROLLE->y_hi_screen[entry]);
        sum = add_bytes(value, rd8((uint16_t)(COLUMN_Y_OFFSETS + 2 + y)), sum.carry);
        WR8(ZP->temp07, (uint8_t)(sum.value & SCREEN_ROWS_MASK));
    }
    jsr_push(0xA7D6);
    bit = BlockIndexForPlace(entry, y);
    jsr_pop();
    y = bit.y;
    for (;;) {
        block = RD8_PTR_Y(ZP->temp04, y);
        push8(y);
        jsr_push(0xA7DC);
        r.overflow = CopyTileBlockToVram(block, bit.carry);
        jsr_pop();
        y = pull8();
        if (DEC8(ZP->temp08) == 0) {
            r.y = y;
            return r;
        }
        y = (uint8_t)(y + 8);
        if (y < COLUMN_ROWS_BYTE) {
            jsr_push(0xA7EE);
            bit = BitmapBitShifted(entry, y);
            jsr_pop();
            continue;
        }
        value = RD8(ZP->temp03);
        WR8(ZP->temp00, value);
        value = RD8(ZP->temp07);
        value = (uint8_t)((value + 1) & SCREEN_ROWS_MASK);
        WR8(ZP->temp07, value);
        jsr_push(0xA7FE);
        map = ScreenMapAt(value);
        jsr_pop();
        block_tables(map, 0xA801, entry, y);
        value = RD8(ZP->temp02);
        y = (uint8_t)(value >> 5);
        jsr_push(0xA80C);
        bit = BitmapBitShifted(entry, y);
        jsr_pop();
    }
}

void abi_DrawBlockColumn(void)
{
    ColumnEnd r = DrawBlockColumn(cpu.x, cpu.y);

    cpu.a = 0x10;
    cpu.y = r.y;
    abi_nz(0);
    abi_flag(P_C, true);
    abi_flag(P_V, r.overflow);
}
