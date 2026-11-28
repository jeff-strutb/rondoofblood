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
    return CopyTileBlockToVramMoved(block, carry_in, 0);
}

/* CopyTileBlockToVram's body, with its camera module `moved` on from $A000. */
bool CopyTileBlockToVramMoved(uint8_t block, bool carry_in, uint16_t moved)
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
    jsr_push((uint16_t)(0xA02A + moved));
    ClearVdcControlHigh();
    jsr_pop();
    wr8(ZpWordsDone, 0);
    y = 0;
    do {
        jsr_push((uint16_t)(0xA030 + moved));
        SelectVramWriteAddress();
        jsr_pop();
        low_word = RD8(ZP->temp10);
        WR8(VDC->data_lo, low_word);
        low_word = RD8(ZP->temp11);
        WR8(VDC->data_hi, low_word);
        jsr_push((uint16_t)(0xA03D + moved));
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
 * @routine 10C5:390 CopyTileBlockToVram10C5
 * @run 6F:A390
 * WHAT IT DOES: CopyTileBlockToVram's copy in the program at 10C5, its camera module $390 on.
 * HOW: CopyTileBlockToVram's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CopyTileBlockToVram.
 */
bool CopyTileBlockToVram10C5(uint8_t block, bool carry_in)
{
    return CopyTileBlockToVramMoved(block, carry_in, 0x390);
}

void abi_CopyTileBlockToVram10C5(void)
{
    bool overflow = CopyTileBlockToVram10C5(cpu.a, (cpu.p & P_C) != 0);

    cpu.a = BLOCK_WORDS;
    cpu.y = 0x20;
    abi_nz(0);
    abi_flag(P_C, true);
    abi_flag(P_V, overflow);
}

/*
 * @routine 12D5:389 CopyTileBlockToVram12D5
 * @run 6F:A389
 * WHAT IT DOES: CopyTileBlockToVram's copy in the program at 12D5, its camera module $389 on.
 * HOW: CopyTileBlockToVram's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CopyTileBlockToVram.
 */
bool CopyTileBlockToVram12D5(uint8_t block, bool carry_in)
{
    return CopyTileBlockToVramMoved(block, carry_in, 0x389);
}

void abi_CopyTileBlockToVram12D5(void)
{
    bool overflow = CopyTileBlockToVram12D5(cpu.a, (cpu.p & P_C) != 0);

    cpu.a = BLOCK_WORDS;
    cpu.y = 0x20;
    abi_nz(0);
    abi_flag(P_C, true);
    abi_flag(P_V, overflow);
}
