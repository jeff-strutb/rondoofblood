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

/* The callees the column code reaches by JSR, each at a run address another overlay can hold: the routine filed for
 * this overlay's bytes, called when it is the one resident, the original otherwise (as the original does: it jumps to
 * whatever is mapped there). */
enum {
    SCROLL_X_OFFSET_RUN = 0xA75C, BLOCK_COPY_RUN = 0xA000, SCREEN_MAP_RUN = 0xA6E2, BITMAP_BIT_RUN = 0xA878,
    BLOCK_INDEX_RUN = 0xA83D
};

static bool resident_is(uint16_t run, uint32_t disc)
{
    return resident_at(run) == disc;
}

/* The registers at a JSR whose callee has no C resident: what the original holds there, A, X, Y, the value N and Z
 * come from, C and V. */
typedef struct {
    uint8_t a, x, y, nz;
    bool c, v;
} CallRegs;

static void call_regs_shim(CallRegs r)
{
    cpu.a = r.a;
    cpu.x = r.x;
    cpu.y = r.y;
    abi_nz(r.nz);
    abi_flag(P_C, r.c);
    abi_flag(P_V, r.v);
}

/* After original code: the carry and overflow it left, for the instructions after the call. */
static bool cpu_carry_shim(void)
{
    return (cpu.p & P_C) != 0;
}

static bool cpu_overflow_shim(void)
{
    return (cpu.p & P_V) != 0;
}

/* After original code: Y as it left it (the callee's Y, before a PLY or a TAY replaces it), and its A. */
static uint8_t cpu_y_shim(void)
{
    return cpu.y;
}

static uint8_t cpu_a_shim(void)
{
    return cpu.a;
}

/* The JSR $A80F, to the resident overlay's SelectBlockTables: its add's V, or the original's. */
static bool block_tables(uint16_t ret, CallRegs r)
{
    uint32_t disc = resident_at(BLOCK_TABLES_RUN);
    Sum sum;

    for (unsigned i = 0; i < sizeof BLOCK_TABLES / sizeof BLOCK_TABLES[0]; i++) {
        if (BLOCK_TABLES[i].disc == disc) {
            jsr_push(ret);
            sum = BLOCK_TABLES[i].select(r.a);
            jsr_pop();
            return sum.overflow;
        }
    }
    call_regs_shim(r);
    call_routine(ret, BLOCK_TABLES_RUN);
    return cpu_overflow_shim();
}

/* The JSR $A6E2: the screen map's byte; ScreenMapAt leaves C and V clear. */
static uint8_t screen_map(uint16_t ret, CallRegs r, bool *carry, bool *overflow)
{
    uint8_t byte;

    if (resident_is(SCREEN_MAP_RUN, DISC(0x0255, 0x6E2))) {
        jsr_push(ret);
        byte = ScreenMapAt(r.a);
        jsr_pop();
        *carry = false;
        *overflow = false;
        return byte;
    }
    call_regs_shim(r);
    call_routine(ret, SCREEN_MAP_RUN);
    *carry = cpu_carry_shim();
    *overflow = cpu_overflow_shim();
    return cpu_a_shim();
}

/* The JSR $A878 (or the run into it from BlockIndexForPlace, ret 0): the bit, X and Y kept. */
static BitmapBit bitmap_bit(uint16_t ret, CallRegs r)
{
    BitmapBit bit;

    if (resident_is(BITMAP_BIT_RUN, DISC(0x0256, 0x078))) {
        if (ret != 0) {
            jsr_push(ret);
        }
        bit = BitmapBitShifted(r.x, r.y);
        if (ret != 0) {
            jsr_pop();
        }
        return bit;
    }
    call_regs_shim(r);
    if (ret != 0) {
        call_routine(ret, BITMAP_BIT_RUN);
    } else {
        run_routine(BITMAP_BIT_RUN);
    }
    bit.a = cpu_a_shim();
    bit.x = r.x;
    bit.y = cpu_y_shim();
    bit.carry = cpu_carry_shim();
    return bit;
}

/*
 * @routine 0256:03D BlockIndexForPlace
 * @run 6F:A83D
 * WHAT IT DOES: For the place in zTemp02-03 (X: the low byte and the screen) and zTemp06-07 (Y: the low byte and the
 * screen row): the VRAM address of the place's block in zTemp10-11, the screen map's byte for the place with the
 * stage's block tables selected by it (zTemp0C, zTemp04), and the block's index within the screen in Y; then runs
 * into BitmapBitShifted, whose bit is the block's bank.
 * HOW: Shifts through the carry: the screen's bit 0 above the X byte, the row's bit 0 above the Y byte; ScreenMapAt
 * and the resident SelectBlockTables (each the overlay's routine when it is resident, the original otherwise); Y the X
 * byte's top three bits or'd with the Y byte's bits 5 to 3.
 * WAITS: no.
 * Entry: X the scroll entry, Y as the caller holds it. Exit: as BitmapBitShifted, V from SelectBlockTables' add.
 * Clobbers zTemp00, 01, 04, 05, 0C to 0E, 10 and 11.
 */
BlockIndex BlockIndexForPlace(uint8_t entry, uint8_t y, bool overflow)
{
    BlockIndex r = { 0 };
    CallRegs regs;
    uint8_t screen, low, row, value, map;
    bool carry, shifted;

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
    shifted = LSR8(ZP->temp11);
    value = RD8(ZP->temp03);
    WR8(ZP->temp00, value);
    row = RD8(ZP->temp07);
    regs = (CallRegs){ row, entry, y, row, shifted, overflow };
    map = screen_map(0xA863, regs, &carry, &overflow);
    regs = (CallRegs){ map, entry, y, map, carry, overflow };
    r.overflow = block_tables(0xA866, regs);
    low = RD8(ZP->temp02);
    WR8(ZP->temp00, (uint8_t)(low >> 5));
    value = RD8(ZP->temp06);
    carry = (value & 0x02) != 0;                   /* the second LSR's bit out */
    value = (uint8_t)((value >> 2) & 0x38);
    value = (uint8_t)(value | RD8(ZP->temp00));
    regs = (CallRegs){ value, entry, value, value, carry, r.overflow };
    r.bit = bitmap_bit(0, regs);                    /* runs into $A878 */
    return r;
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
    BlockIndex r = BlockIndexForPlace(cpu.x, cpu.y, (cpu.p & P_V) != 0);

    bitmap_bit_exit_shim(r.bit);
    abi_flag(P_V, r.overflow);
}

/* The JSR $A83D: this overlay's BlockIndexForPlace when it is resident, the original otherwise. */
static BlockIndex block_index(uint16_t ret, CallRegs r)
{
    BlockIndex index;

    if (resident_is(BLOCK_INDEX_RUN, DISC(0x0256, 0x03D))) {
        jsr_push(ret);
        index = BlockIndexForPlace(r.x, r.y, r.v);
        jsr_pop();
        return index;
    }
    call_regs_shim(r);
    call_routine(ret, BLOCK_INDEX_RUN);
    index.bit.a = cpu_a_shim();
    index.bit.x = r.x;
    index.bit.y = cpu_y_shim();
    index.bit.carry = cpu_carry_shim();
    index.overflow = cpu_overflow_shim();
    return index;
}

/*
 * @routine 0255:7A5 DrawBlockColumn
 * @run 6F:A7A5
 * WHAT IT DOES: Draws a column of a layer's tile blocks for scroll entry X: as many blocks as $3E66's low seven bits,
 * from the place the entry has reached (its scroll X plus the overlay's offset of word Y, and its scroll Y, plus the
 * offset at $A781 and $A783 when $3E66's bit 7 is set), down the screen and on into the screen below.
 * HOW: BlockIndexForPlace for the first block; then each block number from the second table (zTemp04) by Y goes to
 * CopyTileBlockToVram with the bank bit; Y steps by 8 (a row of blocks) with the bit read again, and past $40 the
 * next screen row's map byte selects the tables again. zTemp08 counts the blocks. Each JSR goes to the overlay's
 * routine when that is the one resident at its address, to the original otherwise, as the original does.
 * RESIDENT: the code's callees at $A000 (CopyTileBlockToVram) and $A878 (BitmapBitShifted) are bytes $0000 and $0878
 * of the bank that holds this routine (6F under MPR5: sectors 0255 to 0258 and each overlay's copy of them), so when
 * these bytes run at $A7A5 those are the bytes at $A000 and $A878. Every scene record that loads one of the 21
 * overlays reads its sector B+$44 for 12 sectors to $8000 under MPR4 7 (the op before reads B+$38 into bank 7's first
 * three sectors, overwritten by it; the ops after go to banks 10 to 12), so 0255 and 0256 land together and whole;
 * no other read the game makes reaches the bank (the boot's, the old-card program's, the five constant cd_reads), and
 * nothing writes its code bytes after. The resident_at fallbacks are what the original does where other code is
 * mapped; liftcmp's curated cases (tools/rondo/curated/layer_columns.txt) keep zRamBaseBank $68, the game's, so
 * CopyTileBlockToVram's MPR6 lands on a mapper the trace has.
 * WAITS: no.
 * Entry: X the entry, Y the offset's index. Exit: A as the last block copy left it ($10), N 0, Z 1 (the count's
 * decrement), C and V from the copy, Y the last block's index, X kept.
 */
ColumnEnd DrawBlockColumn(uint8_t entry, uint8_t index, bool entry_carry, bool entry_overflow)
{
    ColumnEnd r = { 0 };
    CallRegs regs;
    uint8_t value, count, y, block, map, row;
    BlockIndex place;
    BitmapBit bit;
    bool carry, overflow;
    Sum sum;

    value = RD8(SCROLLE->f3e66[entry]);
    count = (uint8_t)(value & 0x7F);
    WR8(ZP->temp08, count);
    if (resident_is(SCROLL_X_OFFSET_RUN, DISC(0x0255, 0x75C))) {
        jsr_push(0xA7AF);
        sum = ScrollXPlusOffset(entry, index);
        jsr_pop();
        carry = sum.carry;
        overflow = sum.overflow;
    } else {
        regs = (CallRegs){ count, entry, index, count, entry_carry, entry_overflow };
        call_regs_shim(regs);
        call_routine(0xA7AF, SCROLL_X_OFFSET_RUN);
        index = cpu_y_shim();
        carry = cpu_carry_shim();
        overflow = cpu_overflow_shim();
    }
    value = RD8(SCROLL->y_lo[entry]);
    WR8(ZP->temp06, value);
    value = RD8(SCROLLE->y_hi_screen[entry]);
    WR8(ZP->temp07, value);
    y = index;
    value = RD8(SCROLLE->f3e66[entry]);
    if ((value & 0x80) != 0) {
        y = 1;
        value = RD8(SCROLL->y_lo[entry]);
        sum = add_bytes(value, rd8((uint16_t)(COLUMN_Y_OFFSETS + y)), false);
        WR8(ZP->temp06, sum.value);
        value = RD8(SCROLLE->y_hi_screen[entry]);
        sum = add_bytes(value, rd8((uint16_t)(COLUMN_Y_OFFSETS + 2 + y)), sum.carry);
        value = (uint8_t)(sum.value & SCREEN_ROWS_MASK);
        WR8(ZP->temp07, value);
        carry = sum.carry;
        overflow = sum.overflow;
    }
    regs = (CallRegs){ value, entry, y, value, carry, overflow };
    place = block_index(0xA7D6, regs);
    bit = place.bit;
    overflow = place.overflow;
    y = bit.y;
    for (;;) {
        block = RD8_PTR_Y(ZP->temp04, y);
        push8(y);
        if (resident_is(BLOCK_COPY_RUN, DISC(0x0255, 0x000))) {
            jsr_push(0xA7DC);
            overflow = CopyTileBlockToVram(block, bit.carry);
            jsr_pop();
            r.carry = true;
        } else {
            regs = (CallRegs){ block, entry, y, block, bit.carry, overflow };
            call_regs_shim(regs);
            call_routine(0xA7DC, BLOCK_COPY_RUN);
            r.from_original = true;
            r.carry = cpu_carry_shim();
            overflow = cpu_overflow_shim();
        }
        y = pull8();
        if (DEC8(ZP->temp08) == 0) {
            r.y = y;
            r.overflow = overflow;
            return r;
        }
        r.from_original = false;
        sum = add_bytes(y, 0x08, false);
        y = sum.value;
        overflow = sum.overflow;
        if (y < COLUMN_ROWS_BYTE) {
            regs = (CallRegs){ y, entry, y, (uint8_t)(y - COLUMN_ROWS_BYTE), false, overflow };
            bit = bitmap_bit(0xA7EE, regs);
            y = bit.y;
            continue;
        }
        value = RD8(ZP->temp03);
        WR8(ZP->temp00, value);
        row = RD8(ZP->temp07);
        row = (uint8_t)((row + 1) & SCREEN_ROWS_MASK);
        WR8(ZP->temp07, row);
        regs = (CallRegs){ row, entry, y, row, true, overflow };
        map = screen_map(0xA7FE, regs, &carry, &overflow);
        regs = (CallRegs){ map, entry, y, map, carry, overflow };
        overflow = block_tables(0xA801, regs);
        value = RD8(ZP->temp02);
        carry = (value & 0x10) != 0;
        y = (uint8_t)(value >> 5);
        regs = (CallRegs){ y, entry, y, y, carry, overflow };
        bit = bitmap_bit(0xA80C, regs);
        y = bit.y;
    }
}

void abi_DrawBlockColumn(void)
{
    ColumnEnd r = DrawBlockColumn(cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0);

    if (!r.from_original) {
        cpu.a = 0x10;
    }
    cpu.y = r.y;
    abi_nz(0);
    abi_flag(P_C, r.carry);
    abi_flag(P_V, r.overflow);
}
