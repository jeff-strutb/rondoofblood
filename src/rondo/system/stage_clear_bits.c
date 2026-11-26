/* The stage's cleared marks: four rows of VRAM words masked by the stage's bit, four times over. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum { ROWS = 4, ROW_STEP = 0x10, NEXT_ROW_BASE = 0x23, SET_RETURN = 0x9300, RUN_RETURN_1 = 0x92DB, RUN_RETURN_2 = 0x92DD,
       RUN_RETURN_3 = 0x92DF, FIRST_ENTRY = 0x92DF };

typedef void (*SetAddress)(uint8_t lo, uint8_t hi);

/*
 * Four rows: each points the VDC at a VRAM word, masks it by the stage's bit (the low and high
 * tables are read by the stage index in the low nibble of zTemp0F), and steps the address on a
 * row; zTemp0F then moves on by $23.
 */
static StageExit clear_bit_rows(uint8_t entry_x, SetAddress set)
{
    StageExit end = { 0, 0, 0, false, false, 0, EXIT_X | EXIT_Y | EXIT_C | EXIT_V };
    uint8_t a;
    uint8_t y;
    uint8_t x_lo;
    uint8_t y_hi;
    uint8_t byte;
    uint8_t count;
    Sum sum;

    a = RD8(ZP->temp0f);
    y = (uint8_t)(a & 0x0F);
    a = RD8(ZP->temp0f);
    a = (uint8_t)(a >> 4);
    sum = add_bytes(a, RD8(ZP->temp08), false);
    WR8(ZP->temp0a, sum.value);
    a = RD8(ZP->temp09);
    WR8(ZP->temp0b, a);
    WR8(ZP->temp0e, ROWS);
    do {
        push8(entry_x);
        push8(y);
        x_lo = RD8(ZP->temp0a);
        y_hi = RD8(ZP->temp0b);
        jsr_push(SET_RETURN);
        set(x_lo, y_hi);
        jsr_pop();
        y = pull8();
        byte = RD8(VDC->data_lo);
        byte = (uint8_t)(byte & rd8((uint16_t)(StageClearBitLo + y)));
        WR8(VDC->data_lo, byte);
        byte = RD8(VDC->data_hi);
        byte = (uint8_t)(byte & rd8((uint16_t)(StageClearBitHi + y)));
        WR8(VDC->data_hi, byte);
        entry_x = pull8();
        a = RD8(ZP->temp0a);
        sum = add_bytes(a, ROW_STEP, false);
        WR8(ZP->temp0a, sum.value);
        count = DEC8(ZP->temp0e);
    } while (count != 0);
    a = RD8(ZP->temp0f);
    sum = add_bytes(a, NEXT_ROW_BASE, false);
    WR8(ZP->temp0f, sum.value);
    end.a = sum.value;
    end.nz = sum.value;
    end.x = entry_x;
    end.y = y;
    end.c = sum.carry;
    end.v = sum.overflow;
    return end;
}

/*
 * @routine 024F:2DF ClearStageBitRows
 * @run 6B:92DF
 * WHAT IT DOES: Masks four rows of VRAM words by the stage's cleared bit and moves zTemp0F on to the
 * next stage's entry.
 * HOW: SetVramAddresses points the VDC, the word's two bytes are read through the data ports, ANDed
 * with StageClearBitLo and StageClearBitHi by the stage index and written back.
 * Nine stage overlays hold this code, the ones whose VRAM routine is at $937D.
 * WAITS: no.
 * Entry: X kept. Exit: A zTemp0F's new value, Y the stage index, C and V from that add.
 */
StageExit ClearStageBitRows(uint8_t x)
{
    return clear_bit_rows(x, SetVramAddresses);
}

void abi_ClearStageBitRows(void)
{
    stage_exit_shim(ClearStageBitRows(cpu.x));
}

/*
 * @routine 05BF:2DF ClearStageBitRowsLong
 * @run 6B:92DF
 * WHAT IT DOES: ClearStageBitRows for the other stage group, whose VRAM routine is at $968F.
 * HOW: The same code calling SetVramAddressesLong.
 * WAITS: no.
 * Entry, exit: as ClearStageBitRows.
 */
StageExit ClearStageBitRowsLong(uint8_t x)
{
    return clear_bit_rows(x, SetVramAddressesLong);
}

void abi_ClearStageBitRowsLong(void)
{
    stage_exit_shim(ClearStageBitRowsLong(cpu.x));
}

/*
 * @routine 024F:2D9 ClearStageBitRowsFour
 * @run 6B:92D9
 * WHAT IT DOES: Runs ClearStageBitRows four times, for four stages' entries.
 * HOW: Three calls and then the run the last call returns into.
 * Every stage overlay holds this code.
 * WAITS: no.
 * Entry, exit: as ClearStageBitRows.
 */
StageExit ClearStageBitRowsFour(uint8_t x)
{
    StageExit end;
    bool long_group = resident_at(FIRST_ENTRY) == DISC(0x05BF, 0x2DF);
    StageExit (*run)(uint8_t x) = long_group ? ClearStageBitRowsLong : ClearStageBitRows;

    jsr_push(RUN_RETURN_1);
    (void)run(x);
    jsr_pop();
    jsr_push(RUN_RETURN_2);
    (void)run(x);
    jsr_pop();
    jsr_push(RUN_RETURN_3);
    (void)run(x);
    jsr_pop();
    end = run(x);
    return end;
}

void abi_ClearStageBitRowsFour(void)
{
    stage_exit_shim(ClearStageBitRowsFour(cpu.x));
}
