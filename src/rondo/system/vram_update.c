/* VRAM updates: tile blocks and the tables that select them. */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "vram_update.h"
#include "layers.h"

/* The block graphics sit at $C000, 32 bytes a block; MPR6 holds their bank. */
enum { BLOCK_BANK_OFFSET = 0x0B, BLOCK_SOURCE_PAGE = 0xC0, BLOCK_WORDS = 16, ROW_WORDS = 4, VRAM_ROW_STEP = 0x40,
       MPR6 = 0x40, ZpBlockSourceHigh = 0x2013, ZpWordsDone = 0x2017 };

/*
 * @routine 0255:000 CopyTileBlockToVram
 * @run 6F:A000
 * WHAT IT DOES: Copies one 4 by 4 tile block's sixteen words to the VRAM, four to a row, from the
 * block graphics, starting at the VRAM address in zTemp10 and going down a row (64 words) after each four.
 * HOW: Maps the block bank into MPR6, makes the source pointer from the block number (32 bytes a
 * block from $C000), and for each row selects the VRAM write address, writes the address, selects
 * the data port and writes four words.
 * WAITS: no.
 * Entry: A the block number, carry the add into the bank (ADC #0 first), zTemp10 the VRAM address.
 * Exit: A $10, Z set, C set, N clear, Y $20, V from the last high byte add; X kept.
 * Names: zp $13, $17 (the pointer's high byte and the count) are by address until the records name them.
 */
bool CopyTileBlockToVram(uint8_t block, bool carry_in)
{
    uint8_t a = block, base, high, low_word, y, count;
    Sum sum;
    bool carry_bit;

    push8(a);
    sum = add_bytes(BLOCK_BANK_OFFSET, 0, carry_in);
    base = RD8(ZP->ram_base_bank);
    sum = add_bytes(sum.value, base, false);
    tam(MPR6, sum.value);
    a = pull8();
    wr8(ZpBlockSourceHigh, 0);
    for (int shift = 0; shift < 5; shift++) {
        carry_bit = (a & 0x80) != 0;
        a = (uint8_t)(a << 1);
        rol_mem(ZpBlockSourceHigh, carry_bit);
    }
    sum = add_bytes(a, 0, false);
    WR8(ZP->temp12, sum.value);
    high = rd8(ZpBlockSourceHigh);
    sum = add_bytes(high, BLOCK_SOURCE_PAGE, sum.carry);
    wr8(ZpBlockSourceHigh, sum.value);
    jsr_push(0xA02A);
    ClearVdcControlHigh();
    jsr_pop();
    wr8(ZpWordsDone, 0);
    y = 0;
    do {
        jsr_push(0xA030);
        SelectVramWriteAddress();
        jsr_pop();
        low_word = RD8(ZP->temp10);
        WR8(VDC->data_lo, low_word);
        low_word = RD8(ZP->temp11);
        WR8(VDC->data_hi, low_word);
        jsr_push(0xA03D);
        SelectVramData();
        jsr_pop();
        do {
            low_word = RD8_PTR_Y(ZP->temp12, y);
            WR8(VDC->data_lo, low_word);
            y++;
            low_word = RD8_PTR_Y(ZP->temp12, y);
            WR8(VDC->data_hi, low_word);
            y++;
            count = rd8(ZpWordsDone);
            wr8(ZpWordsDone, (uint8_t)(count + 1));
            count = rd8(ZpWordsDone);
        } while ((count & (ROW_WORDS - 1)) != 0);
        low_word = RD8(ZP->temp10);
        sum = add_bytes(VRAM_ROW_STEP, low_word, false);
        WR8(ZP->temp10, sum.value);
        high = RD8(ZP->temp11);
        sum = add_bytes(0, high, sum.carry);
        WR8(ZP->temp11, (uint8_t)(sum.value & 0x0F));
        count = rd8(ZpWordsDone);
    } while (count != BLOCK_WORDS);
    return sum.overflow;
}

void abi_CopyTileBlockToVram(void)
{
    bool overflow = CopyTileBlockToVram(cpu.a, (cpu.p & P_C) != 0);

    cpu.a = BLOCK_WORDS;
    cpu.y = 0x20;
    abi_nz(0);
    abi_flag(P_C, true);
    abi_flag(P_V, overflow);
}

/* Tile block tables: the pointers a stage's block index selects, two tables of its overlay. */
typedef struct {
    uint16_t first, second;
} BlockTables;

/*
 * The block index's two table pointers: zTemp0C is first + index * 8 and zTemp04 is second + index * 64,
 * each a 16-bit shift left of the index and an add of the stage's table. The index is kept on the
 * stack across the first. The result is the second pointer's high byte, as the last add leaves it.
 */
static Sum select_block_tables(uint8_t index, BlockTables tables)
{
    uint8_t a = index, high, carry_bit;
    Sum sum;

    push8(a);
    WR8(ZP->temp0_d, 0);
    for (int shift = 0; shift < 3; shift++) {
        carry_bit = (uint8_t)(a >> 7);
        a = (uint8_t)(a << 1);
        ROL8(ZP->temp0_d, carry_bit != 0);
    }
    sum = add_bytes(a, (uint8_t)tables.first, false);
    WR8(ZP->temp0_c, sum.value);
    high = RD8(ZP->temp0_d);
    sum = add_bytes(high, (uint8_t)(tables.first >> 8), sum.carry);
    WR8(ZP->temp0_d, sum.value);
    a = pull8();
    WR8(ZP->temp05, 0);
    for (int shift = 0; shift < 6; shift++) {
        carry_bit = (uint8_t)(a >> 7);
        a = (uint8_t)(a << 1);
        ROL8(ZP->temp05, carry_bit != 0);
    }
    sum = add_bytes(a, (uint8_t)tables.second, false);
    WR8(ZP->temp04, sum.value);
    high = RD8(ZP->temp05);
    sum = add_bytes(high, (uint8_t)(tables.second >> 8), sum.carry);
    WR8(ZP->temp05, sum.value);
    return sum;
}

static void block_tables_exit_shim(Sum sum)
{
    cpu.a = sum.value;
    abi_nz(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    cpu.y = 0;
}
/*
 * @routine 0256:00F SelectBlockTablesA
 * @run 6F:A80F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $AAAC and $A9EC.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesA(uint8_t index)
{
    BlockTables tables = { 0xAAAC, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesA(void)
{
    block_tables_exit_shim(SelectBlockTablesA(cpu.a));
}

/*
 * @routine 0516:00F SelectBlockTablesB
 * @run 6F:A80F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $AFEC and $A9EC.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesB(uint8_t index)
{
    BlockTables tables = { 0xAFEC, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesB(void)
{
    block_tables_exit_shim(SelectBlockTablesB(cpu.a));
}

/*
 * @routine 05C6:00F SelectBlockTablesC
 * @run 6F:A80F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $AAEC and $A9EC.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesC(uint8_t index)
{
    BlockTables tables = { 0xAAEC, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesC(void)
{
    block_tables_exit_shim(SelectBlockTablesC(cpu.a));
}

/*
 * @routine 0726:00F SelectBlockTablesD
 * @run 6F:A80F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $AB2C and $A9EC.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesD(uint8_t index)
{
    BlockTables tables = { 0xAB2C, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesD(void)
{
    block_tables_exit_shim(SelectBlockTablesD(cpu.a));
}

/*
 * @routine 07D6:00F SelectBlockTablesE
 * @run 6F:A80F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $B3AC and $A9EC.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesE(uint8_t index)
{
    BlockTables tables = { 0xB3AC, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesE(void)
{
    block_tables_exit_shim(SelectBlockTablesE(cpu.a));
}

/*
 * @routine 0936:00F SelectBlockTablesF
 * @run 6F:A80F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $B4AC and $A9EC.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesF(uint8_t index)
{
    BlockTables tables = { 0xB4AC, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesF(void)
{
    block_tables_exit_shim(SelectBlockTablesF(cpu.a));
}

/*
 * @routine 0A96:00F SelectBlockTablesG
 * @run 6F:A80F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $B5AC and $A9EC.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesG(uint8_t index)
{
    BlockTables tables = { 0xB5AC, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesG(void)
{
    block_tables_exit_shim(SelectBlockTablesG(cpu.a));
}

/*
 * @routine 0BF6:00F SelectBlockTablesH
 * @run 6F:A80F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $B02C and $A9EC.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesH(uint8_t index)
{
    BlockTables tables = { 0xB02C, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesH(void)
{
    block_tables_exit_shim(SelectBlockTablesH(cpu.a));
}

/*
 * @routine 0D56:00F SelectBlockTablesI
 * @run 6F:A80F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $B6AC and $A9EC.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesI(uint8_t index)
{
    BlockTables tables = { 0xB6AC, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesI(void)
{
    block_tables_exit_shim(SelectBlockTablesI(cpu.a));
}

/*
 * @routine 0EB6:00F SelectBlockTablesJ
 * @run 6F:A80F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $B4EC and $A9EC.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesJ(uint8_t index)
{
    BlockTables tables = { 0xB4EC, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesJ(void)
{
    block_tables_exit_shim(SelectBlockTablesJ(cpu.a));
}

/*
 * @routine 1016:00F SelectBlockTablesK
 * @run 6F:A80F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $B2EC and $A9EC.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesK(uint8_t index)
{
    BlockTables tables = { 0xB2EC, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesK(void)
{
    block_tables_exit_shim(SelectBlockTablesK(cpu.a));
}

/*
 * @routine 10C6:39F SelectBlockTablesL
 * @run 6F:AB9F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $B03C and $AD7C.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesL(uint8_t index)
{
    BlockTables tables = { 0xB03C, 0xAD7C };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesL(void)
{
    block_tables_exit_shim(SelectBlockTablesL(cpu.a));
}

/*
 * @routine 1176:00F SelectBlockTablesM
 * @run 6F:A80F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $B5EC and $A9EC.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesM(uint8_t index)
{
    BlockTables tables = { 0xB5EC, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesM(void)
{
    block_tables_exit_shim(SelectBlockTablesM(cpu.a));
}

/*
 * @routine 12D6:398 SelectBlockTablesN
 * @run 6F:AB98
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $AEF5 and $AD75.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesN(uint8_t index)
{
    BlockTables tables = { 0xAEF5, 0xAD75 };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesN(void)
{
    block_tables_exit_shim(SelectBlockTablesN(cpu.a));
}

/*
 * @routine 1616:00F SelectBlockTablesO
 * @run 6F:A80F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $B1EC and $A9EC.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesO(uint8_t index)
{
    BlockTables tables = { 0xB1EC, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesO(void)
{
    block_tables_exit_shim(SelectBlockTablesO(cpu.a));
}

/*
 * @routine 18D6:00F SelectBlockTablesP
 * @run 6F:A80F
 * WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
 * table's entry, zTemp04 the second's, for the stage whose overlay this is.
 * HOW: select_block_tables, with this stage's tables at $AC6C and $A9EC.
 * WAITS: no.
 * Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
 * Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
 */
Sum SelectBlockTablesP(uint8_t index)
{
    BlockTables tables = { 0xAC6C, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesP(void)
{
    block_tables_exit_shim(SelectBlockTablesP(cpu.a));
}


/* The VRAM lists of the 00C5 overlay: a list of entries (a tile bank, a VRAM address, a source), each either a block of
 * words copied to the VDC or a stream of commands that fill, repeat and move the write address. One loop in two
 * levels (vram_list for the entries, vram_commands for a stream); each piece the original enters from outside is
 * an entry of it. Fields with no name yet (rows requested): zero page $13 to $17, $F8 and $F9, $9802 (the overlay's
 * table of lists), $8360 (the table the untraced index path reads). */
enum {
    ZpStreamHigh = 0x2013, ZpListLow = 0x2014, ZpListHigh = 0x2015, ZpAddressLow = 0x2016, ZpAddressHigh = 0x2017,
    ZpTablePointer = 0x20F8, ListTableWord = 0x9802, MPR5 = 0x20, STREAM_END = 0xFF, STREAM_ADDRESS_ADD = 0xFE,
    STREAM_ADDRESS = 0xFD, LIST_END = 0xFF, LIST_BANK_FIRST = 0x28, LIST_BANK_LAST = 0x30, LIST_BANK_SHIFT = 0x18,
    LIST_BANK_REPLACEMENT = 0x31, STREAM_FORM_WORDS = 0x80
};

typedef enum {
    VRAM_COMMAND_NEXT,          /* 00C5:201 */
    VRAM_COMMAND_FETCH,         /* 00C5:1CD */
    VRAM_COMMAND_REPEAT,        /* 00C5:20E */
    VRAM_COMMAND_PAIRS,         /* 00C5:22B, inside 20E */
    VRAM_COMMAND_ADDRESS_ADD,   /* 00C5:247, inside 20E */
    VRAM_COMMAND_ADDRESS        /* 00C5:266, inside 20E */
} VramCommand;

/* A word write to the VDC's data port: low half, then high half. */
static void put_word(uint8_t low, uint8_t high)
{
    WR8(VDC->data_lo, low);
    WR8(VDC->data_hi, high);
}

/*
 * One stream of commands, from the command at `first` (the others are what the stream's commands jump to), with
 * Y the stream's index and A, as the entry at 20E and 22B has it, the command byte. A stream ends at $FF with
 * A $FF, C and Z set; the registers and V are the last step's.
 */
static VramExit vram_commands(VramCommand first, uint8_t a, uint8_t y, bool carry, bool overflow)
{
    VramCommand state = first;
    VramExit out = { 0 };
    Sum sum;
    uint8_t value, count, other;

    for (;;) {
        switch (state) {
        case VRAM_COMMAND_NEXT:
            y++;
            value = RD8(ZP->temp12);
            sum = add_bytes(y, value, false);
            WR8(ZP->temp12, sum.value);
            a = sum.value;
            carry = sum.carry;
            overflow = sum.overflow;
            if (sum.carry) {
                count = rd8(ZpStreamHigh);
                wr8(ZpStreamHigh, (uint8_t)(count + 1));
            }
            state = VRAM_COMMAND_FETCH;
            break;
        case VRAM_COMMAND_FETCH:
            y = 0;
            a = RD8_PTR_Y(ZP->temp12, y);
            if (a == 0) {
                state = VRAM_COMMAND_PAIRS;
            } else if (!(a & 0x80)) {
                state = VRAM_COMMAND_REPEAT;
            } else if (a == STREAM_END) {
                out.a = a;
                out.y = y;
                out.carry = true;
                out.overflow = overflow;
                return out;
            } else if (a == STREAM_ADDRESS_ADD) {
                state = VRAM_COMMAND_ADDRESS_ADD;
            } else if (a == STREAM_ADDRESS) {
                state = VRAM_COMMAND_ADDRESS;
            } else {
                a = (uint8_t)(a & 0x7F);
                WR8(ZP->temp08, a);
                do {
                    y++;
                    value = RD8_PTR_Y(ZP->temp12, y);
                    WR8(VDC->data_lo, value);
                    y++;
                    value = RD8_PTR_Y(ZP->temp12, y);
                    WR8(VDC->data_hi, value);
                    count = DEC8(ZP->temp08);
                } while (count != 0);
                state = VRAM_COMMAND_NEXT;
            }
            break;
        case VRAM_COMMAND_REPEAT:
            y++;
            WR8(ZP->temp08, a);
            value = RD8_PTR_Y(ZP->temp12, y);
            WR8(ZP->temp09, value);
            y++;
            value = RD8_PTR_Y(ZP->temp12, y);
            WR8(ZP->temp0_a, value);
            do {
                value = RD8(ZP->temp09);
                WR8(VDC->data_lo, value);
                other = RD8(ZP->temp0_a);
                WR8(VDC->data_hi, other);
                count = DEC8(ZP->temp08);
            } while (count != 0);
            state = VRAM_COMMAND_NEXT;
            break;
        case VRAM_COMMAND_PAIRS:
            y++;
            value = RD8_PTR_Y(ZP->temp12, y);
            WR8(ZP->temp08, value);
            y++;
            value = RD8_PTR_Y(ZP->temp12, y);
            WR8(ZP->temp09, value);
            do {
                y++;
                value = RD8_PTR_Y(ZP->temp12, y);
                WR8(VDC->data_lo, value);
                other = RD8(ZP->temp09);
                WR8(VDC->data_hi, other);
                count = DEC8(ZP->temp08);
            } while (count != 0);
            state = VRAM_COMMAND_NEXT;
            break;
        case VRAM_COMMAND_ADDRESS_ADD:
            y++;
            jsr_push(0x824B);
            SelectVramWriteAddress();
            jsr_pop();
            value = RD8_PTR_Y(ZP->temp12, y);
            other = rd8(ZpAddressLow);
            sum = add_bytes(value, other, false);
            WR8(VDC->data_lo, sum.value);
            y++;
            value = RD8_PTR_Y(ZP->temp12, y);
            other = rd8(ZpAddressHigh);
            sum = add_bytes(value, other, sum.carry);
            carry = sum.carry;
            overflow = sum.overflow;
            value = sum.value;
            other = RD8(ZP->temp11);
            if (!(other & 1)) {
                value = (uint8_t)(value & 0x0F);
            }
            WR8(VDC->data_hi, value);
            jsr_push(0x8263);
            SelectVramData();
            jsr_pop();
            state = VRAM_COMMAND_NEXT;
            break;
        case VRAM_COMMAND_ADDRESS:
            y++;
            jsr_push(0x826A);
            SelectVramWriteAddress();
            jsr_pop();
            value = RD8_PTR_Y(ZP->temp12, y);
            wr8(ZpAddressLow, value);
            WR8(VDC->data_lo, value);
            y++;
            value = RD8_PTR_Y(ZP->temp12, y);
            wr8(ZpAddressHigh, value);
            WR8(VDC->data_hi, value);
            jsr_push(0x827C);
            SelectVramData();
            jsr_pop();
            state = VRAM_COMMAND_NEXT;
            break;
        }
    }
}

/*
 * The list loop: entries of a list read through the pointer in zp $14, each naming a tile bank (mapped
 * from the base bank), a VRAM address, a source and a form, until the list's $FF.
 */
static VramExit vram_list(uint8_t a, uint8_t x, uint8_t y, bool carry, bool overflow)
{
    VramExit out = { 0 };
    VramExit stream;
    Sum sum;
    uint8_t value, bank, flags, saved_y;

    for (;;) {
        a = rd8_zp_y(ZpListLow, y);
        if (a == LIST_END) {
            out.a = a;
            out.x = x;
            out.y = y;
            out.carry = true;
            out.overflow = overflow;
            return out;
        }
        carry = true;
        if (a == 0) {
            value = RD8(ZP->idle_flag);
            if (value & 1) {
                a = LIST_BANK_REPLACEMENT;
            }
        }
        carry = a >= LIST_BANK_FIRST;
        if (a >= LIST_BANK_FIRST) {
            carry = a >= LIST_BANK_LAST;
            if (a < LIST_BANK_LAST) {
                value = RD8(ZP->idle_flag);
                if (!(value & 1)) {
                    sum = sub_bytes(a, LIST_BANK_SHIFT, true);
                    a = sum.value;
                    carry = sum.carry;
                    overflow = sum.overflow;
                }
            }
        }
        value = RD8(ZP->ram_base_bank);
        sum = add_bytes(a, value, false);
        carry = sum.carry;
        overflow = sum.overflow;
        bank = sum.value;
        tam(MPR5, bank);
        tam(MPR6, (uint8_t)(bank + 1));
        a = (uint8_t)(bank + 1);
        y++;
        flags = rd8_zp_y(ZpListLow, y);
        WR8(ZP->temp11, flags);
        y++;
        jsr_push(0x8198);
        ClearVdcControlHigh();
        jsr_pop();
        jsr_push(0x819B);
        SelectVramWriteAddress();
        jsr_pop();
        value = rd8_zp_y(ZpListLow, y);
        y++;
        wr8(ZpAddressLow, value);
        WR8(VDC->data_lo, value);
        value = rd8_zp_y(ZpListLow, y);
        y++;
        wr8(ZpAddressHigh, value);
        WR8(VDC->data_hi, value);
        value = rd8_zp_y(ZpListLow, y);
        WR8(ZP->temp12, value);
        y++;
        value = rd8_zp_y(ZpListLow, y);
        wr8(ZpStreamHigh, value);
        y++;
        jsr_push(0x81B8);
        SelectVramData();
        jsr_pop();
        flags = RD8(ZP->temp11);
        a = flags;
        if (flags & STREAM_FORM_WORDS) {
            a = rd8_zp_y(ZpListLow, y);
            y++;
            x = a;
            push8(y);
            jsr_push(0x81CA);
            sum = CopyWordsToVdc(x);
            jsr_pop();
            x = 0;
            a = sum.value;
            carry = sum.carry;
            overflow = sum.overflow;
            y = pull8();
        } else {
            push8(y);
            jsr_push(0x81BF);
            stream = vram_commands(VRAM_COMMAND_FETCH, a, y, carry, overflow);
            jsr_pop();
            a = stream.a;
            carry = stream.carry;
            overflow = stream.overflow;
            saved_y = pull8();
            y = saved_y;
        }
    }
}

static void vram_exit_shim(VramExit out)
{
    if (out.left) {
        return;
    }
    cpu.a = out.a;
    cpu.x = out.x;
    cpu.y = out.y;
    abi_nz(0);                      /* every exit is the CMP #$FF that matched */
    abi_flag(P_C, out.carry);
    abi_flag(P_V, out.overflow);
}

/* Live registers and flags of an entry, as its wrapper meets them. */
static bool entry_carry_shim(void)
{
    return (cpu.p & P_C) != 0;
}

static bool entry_overflow_shim(void)
{
    return (cpu.p & P_V) != 0;
}

/* The 15 bytes before VramListFromIndex in every overlay: its second table's pointer for a list number of $80 or more. */
static VramExit vram_list_from_table(uint16_t table, uint8_t a, uint8_t x, bool overflow)
{
    uint8_t y = (uint8_t)(a << 1);
    bool carry = (a & 0x80) != 0;
    uint8_t value = rd8((uint16_t)(table + y));

    wr8(ZpListLow, value);
    value = rd8((uint16_t)(table + 1 + y));
    wr8(ZpListHigh, value);
    return vram_list(value, x, 0, carry, overflow);
}

/*
 * @routine 00C5:142 VramListFromTableA
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $8360,
 * gives its pointer.
 * HOW: A doubled indexes the table at $8360, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableA(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x8360, a, x, overflow);
}

void abi_VramListFromTableA(void)
{
    vram_exit_shim(VramListFromTableA(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 0111:142 VramListFromTableB
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $82AE,
 * gives its pointer.
 * HOW: A doubled indexes the table at $82AE, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableB(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x82AE, a, x, overflow);
}

void abi_VramListFromTableB(void)
{
    vram_exit_shim(VramListFromTableB(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 0251:142 VramListFromTableC
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $901F,
 * gives its pointer.
 * HOW: A doubled indexes the table at $901F, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableC(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x901F, a, x, overflow);
}

void abi_VramListFromTableC(void)
{
    vram_exit_shim(VramListFromTableC(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 0511:142 VramListFromTableD
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $9349,
 * gives its pointer.
 * HOW: A doubled indexes the table at $9349, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableD(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x9349, a, x, overflow);
}

void abi_VramListFromTableD(void)
{
    vram_exit_shim(VramListFromTableD(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 05C1:142 VramListFromTableE
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $8E4B,
 * gives its pointer.
 * HOW: A doubled indexes the table at $8E4B, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableE(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x8E4B, a, x, overflow);
}

void abi_VramListFromTableE(void)
{
    vram_exit_shim(VramListFromTableE(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 0721:142 VramListFromTableF
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $8E95,
 * gives its pointer.
 * HOW: A doubled indexes the table at $8E95, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableF(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x8E95, a, x, overflow);
}

void abi_VramListFromTableF(void)
{
    vram_exit_shim(VramListFromTableF(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 07D1:142 VramListFromTableG
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $9236,
 * gives its pointer.
 * HOW: A doubled indexes the table at $9236, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableG(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x9236, a, x, overflow);
}

void abi_VramListFromTableG(void)
{
    vram_exit_shim(VramListFromTableG(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 0881:142 VramListFromTableH
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $901E,
 * gives its pointer.
 * HOW: A doubled indexes the table at $901E, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableH(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x901E, a, x, overflow);
}

void abi_VramListFromTableH(void)
{
    vram_exit_shim(VramListFromTableH(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 0931:142 VramListFromTableI
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $8E90,
 * gives its pointer.
 * HOW: A doubled indexes the table at $8E90, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableI(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x8E90, a, x, overflow);
}

void abi_VramListFromTableI(void)
{
    vram_exit_shim(VramListFromTableI(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 09E1:142 VramListFromTableJ
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $9020,
 * gives its pointer.
 * HOW: A doubled indexes the table at $9020, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableJ(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x9020, a, x, overflow);
}

void abi_VramListFromTableJ(void)
{
    vram_exit_shim(VramListFromTableJ(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 0A91:142 VramListFromTableK
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $8E5D,
 * gives its pointer.
 * HOW: A doubled indexes the table at $8E5D, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableK(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x8E5D, a, x, overflow);
}

void abi_VramListFromTableK(void)
{
    vram_exit_shim(VramListFromTableK(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 0B41:142 VramListFromTableL
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $8DE0,
 * gives its pointer.
 * HOW: A doubled indexes the table at $8DE0, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableL(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x8DE0, a, x, overflow);
}

void abi_VramListFromTableL(void)
{
    vram_exit_shim(VramListFromTableL(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 0BF1:142 VramListFromTableM
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $8CA7,
 * gives its pointer.
 * HOW: A doubled indexes the table at $8CA7, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableM(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x8CA7, a, x, overflow);
}

void abi_VramListFromTableM(void)
{
    vram_exit_shim(VramListFromTableM(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 0D51:142 VramListFromTableN
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $8D9C,
 * gives its pointer.
 * HOW: A doubled indexes the table at $8D9C, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableN(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x8D9C, a, x, overflow);
}

void abi_VramListFromTableN(void)
{
    vram_exit_shim(VramListFromTableN(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 0E01:142 VramListFromTableO
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $8D36,
 * gives its pointer.
 * HOW: A doubled indexes the table at $8D36, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableO(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x8D36, a, x, overflow);
}

void abi_VramListFromTableO(void)
{
    vram_exit_shim(VramListFromTableO(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 0EB1:142 VramListFromTableP
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $8FBB,
 * gives its pointer.
 * HOW: A doubled indexes the table at $8FBB, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableP(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x8FBB, a, x, overflow);
}

void abi_VramListFromTableP(void)
{
    vram_exit_shim(VramListFromTableP(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 0F61:142 VramListFromTableQ
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $8D2E,
 * gives its pointer.
 * HOW: A doubled indexes the table at $8D2E, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableQ(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x8D2E, a, x, overflow);
}

void abi_VramListFromTableQ(void)
{
    vram_exit_shim(VramListFromTableQ(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 1011:142 VramListFromTableR
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $8F58,
 * gives its pointer.
 * HOW: A doubled indexes the table at $8F58, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableR(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x8F58, a, x, overflow);
}

void abi_VramListFromTableR(void)
{
    vram_exit_shim(VramListFromTableR(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 10C1:142 VramListFromTableS
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $8E56,
 * gives its pointer.
 * HOW: A doubled indexes the table at $8E56, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableS(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x8E56, a, x, overflow);
}

void abi_VramListFromTableS(void)
{
    vram_exit_shim(VramListFromTableS(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 1171:142 VramListFromTableT
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $9089,
 * gives its pointer.
 * HOW: A doubled indexes the table at $9089, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableT(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x9089, a, x, overflow);
}

void abi_VramListFromTableT(void)
{
    vram_exit_shim(VramListFromTableT(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 1611:142 VramListFromTableU
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $917E,
 * gives its pointer.
 * HOW: A doubled indexes the table at $917E, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableU(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x917E, a, x, overflow);
}

void abi_VramListFromTableU(void)
{
    vram_exit_shim(VramListFromTableU(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 16C1:142 VramListFromTableV
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $8FEB,
 * gives its pointer.
 * HOW: A doubled indexes the table at $8FEB, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableV(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x8FEB, a, x, overflow);
}

void abi_VramListFromTableV(void)
{
    vram_exit_shim(VramListFromTableV(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * @routine 18D1:142 VramListFromTableW
 * @run 6C:8142
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table of lists, at $92F6,
 * gives its pointer.
 * HOW: A doubled indexes the table at $92F6, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * Every overlay has its own copy of these 15 bytes with its own table, so each is a routine of its own; VramListFromIndex branches here.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 */
VramExit VramListFromTableW(uint8_t a, uint8_t x, bool overflow)
{
    return vram_list_from_table(0x92F6, a, x, overflow);
}

void abi_VramListFromTableW(void)
{
    vram_exit_shim(VramListFromTableW(cpu.a, cpu.x, entry_overflow_shim()));
}

/*
 * The branch the routine takes for a list number of $80 or more goes to $8142, which is not in this routine and
 * is another table in each overlay (the operand differs from one to the next): it runs as the original, with the
 * registers the branch left, and joins VramListRun at $8169 itself.
 */
static VramExit branch_to_8142_shim(uint8_t a, uint8_t x, bool carry, bool overflow)
{
    VramExit out = { 0 };

    cpu.a = a;
    cpu.x = x;
    abi_flag(P_C, carry);
    abi_flag(P_V, overflow);
    call_original(0x8142);
    out.left = true;
    return out;
}

/*
 * The variant of the 15 bytes at $8142 that this overlay has, by what the machine says is resident there: its C, as a
 * typed call. One this tree has no routine for (an overlay no script has run) runs as the original.
 */
static VramExit table_variant_shim(uint8_t a, uint8_t x, bool carry, bool overflow)
{
    static const struct { uint32_t disc; VramExit (*run)(uint8_t a, uint8_t x, bool overflow); } variants[] = {
        { DISC(0x00C5, 0x142), VramListFromTableA },
        { DISC(0x0111, 0x142), VramListFromTableB },
        { DISC(0x0251, 0x142), VramListFromTableC },
        { DISC(0x0511, 0x142), VramListFromTableD },
        { DISC(0x05C1, 0x142), VramListFromTableE },
        { DISC(0x0721, 0x142), VramListFromTableF },
        { DISC(0x07D1, 0x142), VramListFromTableG },
        { DISC(0x0881, 0x142), VramListFromTableH },
        { DISC(0x0931, 0x142), VramListFromTableI },
        { DISC(0x09E1, 0x142), VramListFromTableJ },
        { DISC(0x0A91, 0x142), VramListFromTableK },
        { DISC(0x0B41, 0x142), VramListFromTableL },
        { DISC(0x0BF1, 0x142), VramListFromTableM },
        { DISC(0x0D51, 0x142), VramListFromTableN },
        { DISC(0x0E01, 0x142), VramListFromTableO },
        { DISC(0x0EB1, 0x142), VramListFromTableP },
        { DISC(0x0F61, 0x142), VramListFromTableQ },
        { DISC(0x1011, 0x142), VramListFromTableR },
        { DISC(0x10C1, 0x142), VramListFromTableS },
        { DISC(0x1171, 0x142), VramListFromTableT },
        { DISC(0x1611, 0x142), VramListFromTableU },
        { DISC(0x16C1, 0x142), VramListFromTableV },
        { DISC(0x18D1, 0x142), VramListFromTableW }
    };
    uint32_t resident = resident_at(0x8142);

    for (unsigned i = 0; i < sizeof variants / sizeof variants[0]; i++) {
        if (variants[i].disc == resident) {
            return variants[i].run(a, x, overflow);
        }
    }
    return branch_to_8142_shim(a, x, carry, overflow);
}

/*
 * @routine 00C5:151 VramListFromIndex
 * @run 6C:8151
 * WHAT IT DOES: Runs the VRAM list whose number is in A: the overlay's table of lists gives its pointer.
 * HOW: An entry into the list loop (vram_list): A doubled indexes the table at $9802, the pointer goes
 * to zp $14, Y 0, and the loop runs on at VramListRun. A number of $80 or more branches to $8142, which
 * reads the overlay's own second table the same way (the original's code, run as it is).
 * WAITS: no.
 * Entry: A the list number, N the branch's (a caller loads A first). Exit: as VramListRun.
 * Not in this routine: the bytes of $8142 to $8150, run as the original.
 */
VramExit VramListFromIndex(uint8_t a, uint8_t x, bool negative, bool carry, bool overflow)
{
    uint8_t y, value;

    if (negative) {
        return table_variant_shim(a, x, carry, overflow);
    }
    y = (uint8_t)(a << 1);
    carry = (a & 0x80) != 0;
    value = rd8(ListTableWord);
    wr8(ZpTablePointer, value);
    value = rd8(ListTableWord + 1);
    wr8(ZpTablePointer + 1, value);
    value = rd8_zp_y(ZpTablePointer, y);
    wr8(ZpListLow, value);
    y++;
    value = rd8_zp_y(ZpTablePointer, y);
    wr8(ZpListHigh, value);
    return vram_list(value, x, 0, carry, overflow);
}

void abi_VramListFromIndex(void)
{
    vram_exit_shim(VramListFromIndex(cpu.a, cpu.x, (cpu.p & P_N) != 0, entry_carry_shim(), entry_overflow_shim()));
}

/*
 * @routine 00C5:169 VramListRun
 * @run 6C:8169
 * WHAT IT DOES: Runs the VRAM list at zp $14, Y its place: each entry maps its tile bank, sets the VRAM
 * write address, and copies words or runs a stream of commands, until the list's $FF.
 * HOW: The list loop, vram_list. The bank byte $28 to $2F is lowered by $18 unless the idle flag's bit 0
 * is set, and a zero one becomes $31 when it is (untraced).
 * WAITS: no.
 * Entry: zp $14 the list, Y the place in it. Exit: A $FF, C and Z set, N clear, X and Y and V as the last
 * entry leaves them.
 * Untraced: the bank $31 replacement.
 */
VramExit VramListRun(uint8_t x, uint8_t y, bool carry, bool overflow)
{
    return vram_list(0, x, y, carry, overflow);
}

void abi_VramListRun(void)
{
    vram_exit_shim(VramListRun(cpu.x, cpu.y, entry_carry_shim(), entry_overflow_shim()));
}

/*
 * @routine 00C5:1CD VramCommandsFetch
 * @run 6C:81CD
 * WHAT IT DOES: Runs one stream of VRAM commands from the pointer in zp $12: counted words, a word repeated, pairs
 * of a byte and a fixed byte, a new write address (absolute, or added to the last), until $FF.
 * HOW: An entry into the command loop (vram_commands), at its first fetch. The commands that run on from the
 * overlay's other entries (VramCommandsRepeat and the entries it jumps to) are cases of the same loop.
 * WAITS: no.
 * Entry: zp $12 the stream. Exit: A $FF, C and Z set, N clear, Y the index of the $FF, V from the last add.
 * Untraced: the command $FD (set the write address).
 */
VramExit VramCommandsFetch(bool carry, bool overflow)
{
    return vram_commands(VRAM_COMMAND_FETCH, 0, 0, carry, overflow);
}

static VramExit commands_exit_shim(VramExit out)
{
    out.x = cpu.x;
    return out;
}

void abi_VramCommandsFetch(void)
{
    vram_exit_shim(commands_exit_shim(VramCommandsFetch(entry_carry_shim(), entry_overflow_shim())));
}

/*
 * @routine 00C5:201 VramCommandsNext
 * @run 6C:8201
 * WHAT IT DOES: Goes on to the next command of the stream: the pointer moves past this one's bytes.
 * HOW: An entry into the command loop (vram_commands), at its advance.
 * WAITS: no.
 * Entry: Y the index of the command's last byte, zp $12 the stream. Exit: as VramCommandsFetch.
 */
VramExit VramCommandsNext(uint8_t y, bool carry, bool overflow)
{
    return vram_commands(VRAM_COMMAND_NEXT, 0, y, carry, overflow);
}

void abi_VramCommandsNext(void)
{
    vram_exit_shim(commands_exit_shim(VramCommandsNext(cpu.y, entry_carry_shim(), entry_overflow_shim())));
}

/*
 * @routine 00C5:20E VramCommandsRepeat
 * @run 6C:820E
 * WHAT IT DOES: The commands from $820E on: a counted word written again and again, and the stream's other
 * forms.
 * HOW: An entry into the command loop (vram_commands), at the repeat command with A its count; the pairs
 * command at $822B, the address-add at $8247 and the address set at $8266 are entries of their own.
 * WAITS: no.
 * Entry: A the count, Y the index of the command, zp $12 the stream. Exit: as VramCommandsFetch.
 */
VramExit VramCommandsRepeat(uint8_t a, uint8_t y, bool carry, bool overflow)
{
    return vram_commands(VRAM_COMMAND_REPEAT, a, y, carry, overflow);
}

void abi_VramCommandsRepeat(void)
{
    vram_exit_shim(commands_exit_shim(VramCommandsRepeat(cpu.a, cpu.y, entry_carry_shim(), entry_overflow_shim())));
}

/*
 * @routine 00C5:22B VramCommandsPairs
 * @run 6C:822B
 * WHAT IT DOES: The stream's pairs command: a count, a fixed high byte, then that many low bytes, each written to the
 * VDC's data port with the fixed byte as its high half.
 * HOW: An entry into the command loop (vram_commands), at the pairs command; VramCommandsFetch jumps here for a command byte of 0.
 * WAITS: no.
 * Entry: Y the index of the command byte, zp $12 the stream. Exit: as VramCommandsFetch.
 */
VramExit VramCommandsPairs(uint8_t y, bool carry, bool overflow)
{
    return vram_commands(VRAM_COMMAND_PAIRS, 0, y, carry, overflow);
}

void abi_VramCommandsPairs(void)
{
    vram_exit_shim(commands_exit_shim(VramCommandsPairs(cpu.y, entry_carry_shim(), entry_overflow_shim())));
}

/*
 * @routine 00C5:247 VramCommandsAddressAdd
 * @run 6C:8247
 * WHAT IT DOES: The stream's address-add command ($FE): the VDC's write address is set to the last address set plus the
 * two bytes that follow, its high byte masked to four bits unless bit 0 of zp $11 is set.
 * HOW: An entry into the command loop (vram_commands), at the address add; VramCommandsFetch jumps here for $FE.
 * WAITS: no.
 * Entry: Y the index of the command byte, zp $12 the stream. Exit: as VramCommandsFetch.
 */
VramExit VramCommandsAddressAdd(uint8_t y, bool carry, bool overflow)
{
    return vram_commands(VRAM_COMMAND_ADDRESS_ADD, 0, y, carry, overflow);
}

void abi_VramCommandsAddressAdd(void)
{
    vram_exit_shim(commands_exit_shim(VramCommandsAddressAdd(cpu.y, entry_carry_shim(), entry_overflow_shim())));
}

/*
 * @routine 00C5:266 VramCommandsAddress
 * @run 6C:8266
 * WHAT IT DOES: The stream's address command ($FD): the VDC's write address and the kept copy of it are set to the two bytes
 * that follow.
 * HOW: An entry into the command loop (vram_commands), at the address set; VramCommandsFetch jumps here for $FD.
 * WAITS: no.
 * Entry: Y the index of the command byte, zp $12 the stream. Exit: as VramCommandsFetch.
 * Untraced: the whole routine.
 */
VramExit VramCommandsAddress(uint8_t y, bool carry, bool overflow)
{
    return vram_commands(VRAM_COMMAND_ADDRESS, 0, y, carry, overflow);
}

void abi_VramCommandsAddress(void)
{
    vram_exit_shim(commands_exit_shim(VramCommandsAddress(cpu.y, entry_carry_shim(), entry_overflow_shim())));
}

/*
 * A palette in blocks: the list at zp $00 and Y gives the VCE's colour address, then a pointer (zp $02) to the blocks:
 * a count, then for each block a byte of ninth bits and eight colours' low bytes, nine bytes a block. Each colour goes
 * to the VCE's data port as a word, the low byte and a high byte of one bit taken from the ninth bits, top first.
 */
static PaletteLoad load_packed_palette(uint8_t y)
{
    PaletteLoad out;
    Sum sum = { 0 };
    uint8_t value, left;

    value = RD8_PTR_Y(ZP->temp00, y);
    WR8(VCE->addr_lo, value);
    y++;
    value = RD8_PTR_Y(ZP->temp00, y);
    WR8(VCE->addr_hi, value);
    y++;
    value = RD8_PTR_Y(ZP->temp00, y);
    WR8(ZP->temp02, value);
    y++;
    value = RD8_PTR_Y(ZP->temp00, y);
    WR8(ZP->temp03, value);
    value = RD8_PTR(ZP->temp02);
    WR8(ZP->temp05, value);
    for (;;) {
        y = 1;
        value = RD8_PTR_Y(ZP->temp02, y);
        WR8(ZP->temp04, value);
        do {
            bool bit;

            y++;
            value = RD8_PTR_Y(ZP->temp02, y);
            WR8(VCE->data_lo, value);
            bit = ASL8(ZP->temp04);
            WR8(VCE->data_hi, bit ? 1 : 0);
        } while (y < 9);
        value = RD8(ZP->temp02);
        sum = add_bytes(value, 9, false);
        WR8(ZP->temp02, sum.value);
        if (sum.carry) {
            INC8(ZP->temp03);
        }
        left = DEC8(ZP->temp05);
        if (left & 0x80) {
            break;
        }
    }
    out.sum = sum;
    out.left = left;
    return out;
}

static void palette_load_exit_shim(PaletteLoad load)
{
    cpu.a = load.sum.value;
    cpu.y = 9;
    abi_nz(load.left);
    abi_flag(P_C, load.sum.carry);
    abi_flag(P_V, load.sum.overflow);
}

/*
 * @routine 0251:4BF LoadPackedPalette
 * @run 6C:84BF
 * WHAT IT DOES: Loads a packed palette into the VCE from a list: the colour address, a pointer to the blocks, then the
 * colours, eight a block with their ninth bits in the block's first byte.
 * HOW: load_packed_palette.
 * WAITS: no.
 * Entry: zp $00 the list, Y its place in it. Exit: A the last block pointer's low byte, Y 9, N and Z from the block
 * count's last value, C and V from the pointer's add. X kept.
 */
PaletteLoad LoadPackedPalette(uint8_t y)
{
    return load_packed_palette(y);
}

void abi_LoadPackedPalette(void)
{
    palette_load_exit_shim(LoadPackedPalette(cpu.y));
}

/*
 * @routine 00C5:580 LoadPackedPaletteB
 * @run 6C:8580
 * WHAT IT DOES: LoadPackedPalette, as the 00C5 overlay has it at $8580.
 * HOW: The same bytes at another run address; load_packed_palette.
 * WAITS: no.
 * Entry, exit: as LoadPackedPalette.
 */
PaletteLoad LoadPackedPaletteB(uint8_t y)
{
    return load_packed_palette(y);
}

void abi_LoadPackedPaletteB(void)
{
    palette_load_exit_shim(LoadPackedPaletteB(cpu.y));
}

/*
 * @routine 0111:4CE LoadPackedPaletteC
 * @run 6C:84CE
 * WHAT IT DOES: LoadPackedPalette, as the 0111 overlay has it at $84CE.
 * HOW: The same bytes at another run address; load_packed_palette.
 * WAITS: no.
 * Entry, exit: as LoadPackedPalette.
 */
PaletteLoad LoadPackedPaletteC(uint8_t y)
{
    return load_packed_palette(y);
}

void abi_LoadPackedPaletteC(void)
{
    palette_load_exit_shim(LoadPackedPaletteC(cpu.y));
}
