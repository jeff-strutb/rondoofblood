/* VRAM updates: tile blocks and the tables that select them. */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "vram_update.h"

/* Tile block tables: the pointers a stage's block index selects, two tables of its overlay. */
typedef struct {
    uint16_t first, second;
} BlockTables;

/*
 * The block index's two table pointers: zTemp0C is first + index * 8 and zTemp04 is second + index * 64,
 * each a 16-bit shift left of the index and an add of the stage's table. The index is kept on the
 * stack across the first. The result is the second pointer's high byte, as the last add leaves it.
 */
static Alu select_block_tables(uint8_t index, BlockTables tables)
{
    uint8_t a = index, high, carry_bit;
    Alu sum;

    push8(a);
    WR8(ZP->temp0_d, 0);
    for (int shift = 0; shift < 3; shift++) {
        carry_bit = (uint8_t)(a >> 7);
        a = (uint8_t)(a << 1);
        ROL8(ZP->temp0_d, carry_bit != 0);
    }
    sum = adc8(a, (uint8_t)tables.first, false);
    WR8(ZP->temp0_c, sum.a);
    high = RD8(ZP->temp0_d);
    sum = adc8(high, (uint8_t)(tables.first >> 8), sum.c);
    WR8(ZP->temp0_d, sum.a);
    a = pull8();
    WR8(ZP->temp05, 0);
    for (int shift = 0; shift < 6; shift++) {
        carry_bit = (uint8_t)(a >> 7);
        a = (uint8_t)(a << 1);
        ROL8(ZP->temp05, carry_bit != 0);
    }
    sum = adc8(a, (uint8_t)tables.second, false);
    WR8(ZP->temp04, sum.a);
    high = RD8(ZP->temp05);
    sum = adc8(high, (uint8_t)(tables.second >> 8), sum.c);
    WR8(ZP->temp05, sum.a);
    return sum;
}

static void block_tables_exit_shim(Alu sum)
{
    abi_alu(sum);
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
Alu SelectBlockTablesA(uint8_t index)
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
Alu SelectBlockTablesB(uint8_t index)
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
Alu SelectBlockTablesC(uint8_t index)
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
Alu SelectBlockTablesD(uint8_t index)
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
Alu SelectBlockTablesE(uint8_t index)
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
Alu SelectBlockTablesF(uint8_t index)
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
Alu SelectBlockTablesG(uint8_t index)
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
Alu SelectBlockTablesH(uint8_t index)
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
Alu SelectBlockTablesI(uint8_t index)
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
Alu SelectBlockTablesJ(uint8_t index)
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
Alu SelectBlockTablesK(uint8_t index)
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
Alu SelectBlockTablesL(uint8_t index)
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
Alu SelectBlockTablesM(uint8_t index)
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
Alu SelectBlockTablesN(uint8_t index)
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
Alu SelectBlockTablesO(uint8_t index)
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
Alu SelectBlockTablesP(uint8_t index)
{
    BlockTables tables = { 0xAC6C, 0xA9EC };

    return select_block_tables(index, tables);
}

void abi_SelectBlockTablesP(void)
{
    block_tables_exit_shim(SelectBlockTablesP(cpu.a));
}

