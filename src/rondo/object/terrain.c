/*
 * What the map holds under a point, from the stage overlay: the tile the background has
 * there, read from VRAM, and through a page of attributes what that tile is. Attributes of
 * $80-$9B are slopes and steps, each with a handler that moves the point's row within the
 * tile. The two lookups read a probe's columns around an object into TileHit.
 *
 * Eleven stage overlays hold this code at these addresses. The tile read and the handlers
 * are the same bytes in all eleven; each overlay's two lookups differ only in the address of
 * its probe table, so each of the eleven has its own routine, sharing one body. Which one is
 * loaded is asked of the machine layer (resident_at); no other overlay holds this code, and
 * for those the lookups stay the original's.
 *
 * zTemp00-09 and zTemp0A-0D are RAM these routines leave behind, so they are written here.
 * Only the wrappers and exit shims know registers and flags.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"

enum {
    TILE_FIRST_HANDLED = 0x80, TILE_END_HANDLED = 0x9C, TILE_HANDLED_RESULT = 0x81,
    TILE_PAGE_BASE = 0x7D, PROBE_COLUMNS = 4, PROBE_COLUMN_BYTES = 4, PROBE_NO_COLUMN = 0x80,
    HIT_NEEDS_SECOND_LOOK = 0xA1, HIT_SECOND_LOOK_SKIP = 0x80, SECOND_LOOK_SOLID = 0x01,
    HIT_CLEARED_BY_SECOND_LOOK = 0x5E, SECOND_LOOK_RAISE = 0xF8,
    TILE_READ_RUN = 0x967F, LOOKUP_RUN = 0x95AB, LOOKUP_ALT_RUN = 0x960B
};

static HandlerExit exit_with(uint8_t y, Sum sum)
{
    HandlerExit out = { true, y, true, sum.carry, sum.overflow };

    return out;
}

/* A step by a table on the column: the table's byte for the column is added to the row. */
static HandlerExit step_by_column(uint16_t table)
{
    uint8_t column = RD8(ZP->temp00);
    uint8_t step = rd8((uint16_t)(table + column));
    uint8_t row = RD8(ZP->temp02);
    Sum sum = add_bytes(step, row, false);

    WR8(ZP->temp02, sum.value);
    return exit_with(column, sum);
}

/* A table of eight rows of eight, by row and column, gives the new row. */
static HandlerExit row_from_grid(uint16_t table)
{
    uint8_t row = RD8(ZP->temp02);
    uint8_t column = RD8(ZP->temp00);
    Sum index = add_bytes((uint8_t)(row << 3), column, false);
    uint8_t value = rd8((uint16_t)(table + index.value));

    WR8(ZP->temp02, value);
    return exit_with(index.value, index);
}

/* A row one lower or higher by a small amount: a subtract from the row. */
static HandlerExit lift_row(uint8_t amount)
{
    uint8_t row = RD8(ZP->temp02);
    Sum sum = sub_bytes(row, amount, true);
    HandlerExit out = { false, 0, true, sum.carry, sum.overflow };

    WR8(ZP->temp02, sum.value);
    return out;
}

static HandlerExit nudge_row(void)
{
    HandlerExit none = { false, 0, false, false, false };

    INC8(ZP->temp02);
    return none;
}

/*
 * @routine 050F:752 TileStepC
 * @run 6B:9752
 * WHAT IT DOES: A slope tile: moves the point's row by a table on its column.
 * HOW: TileStepsC by column, added to the row.
 * WAITS: no.
 * Entry: zTemp00 column, zTemp02 row. Exit: A $81; Y, C and V as the handler's last step
 * leaves them (handler_exit_shim).
 */
HandlerExit TileStepC(void)
{
    return step_by_column(TileStepsC);
}

/*
 * @routine 050F:75F TileRowA
 * @run 6B:975F
 * WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
 * HOW: TileRowsA, eight rows of eight.
 * WAITS: no.
 * Entry and exit: as TileStepC.
 */
HandlerExit TileRowA(void)
{
    return row_from_grid(TileRowsA);
}

/*
 * @routine 050F:770 TileRowB
 * @run 6B:9770
 * WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
 * HOW: TileRowsB, eight rows of eight.
 * WAITS: no.
 * Entry and exit: as TileStepC.
 */
HandlerExit TileRowB(void)
{
    return row_from_grid(TileRowsB);
}

/* The tiles $87-$8A and $8C-$8F: a step that differs between the tile's halves. */
static HandlerExit half_step(uint16_t table, uint8_t first_tile, uint8_t tile, bool right_for_bit)
{
    uint8_t pair = (uint8_t)((uint8_t)(tile - first_tile) << 1);
    uint8_t column = RD8(ZP->temp00);
    uint8_t index = (uint8_t)(pair + (((column & 4) != 0) == right_for_bit ? 1 : 0));
    uint8_t step = rd8((uint16_t)(table + index));
    uint8_t row = RD8(ZP->temp02);
    Sum sum = add_bytes(step, row, false);

    WR8(ZP->temp02, sum.value);
    return exit_with(index, sum);
}

/*
 * @routine 050F:781 TileHalfStepA
 * @run 6B:9781
 * WHAT IT DOES: The tiles $87-$8A: a step that differs between the tile's left and right
 * halves.
 * HOW: A pair of bytes in TileHalfStepsA by tile, the second for the right half.
 * WAITS: no.
 * Entry: A tile, zTemp00 column, zTemp02 row. Exit: as TileStepC.
 */
HandlerExit TileHalfStepA(uint8_t tile)
{
    return half_step(TileHalfStepsA, 0x83, tile, true);
}

/*
 * @routine 050F:795 TileHalfStepB
 * @run 6B:9795
 * WHAT IT DOES: The tiles $8C-$8F: as TileHalfStepA, the other way round.
 * HOW: A pair of bytes in TileHalfStepsB by tile, the second for the left half.
 * WAITS: no.
 * Entry and exit: as TileHalfStepA.
 */
HandlerExit TileHalfStepB(uint8_t tile)
{
    return half_step(TileHalfStepsB, 0x8C, tile, false);
}

/*
 * @routine 050F:7A9 TileNudgeA
 * @run 6B:97A9
 * WHAT IT DOES: The tiles $86 and $8B: the point's row one lower.
 * HOW: An increment.
 * WAITS: no.
 * Entry: zTemp02 row. Exit: A $81; Y, C and V kept.
 */
HandlerExit TileNudgeA(void)
{
    return nudge_row();
}

/*
 * @routine 050F:7AE TileStepD
 * @run 6B:97AE
 * WHAT IT DOES: A slope tile: moves the point's row by a table on its column.
 * HOW: TileStepsD by column, added to the row.
 * WAITS: no.
 * Entry and exit: as TileStepC.
 */
HandlerExit TileStepD(void)
{
    return step_by_column(TileStepsD);
}

/*
 * @routine 050F:7BB TileRowC
 * @run 6B:97BB
 * WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
 * HOW: TileRowsC, eight rows of eight.
 * WAITS: no.
 * Entry and exit: as TileStepC.
 */
HandlerExit TileRowC(void)
{
    return row_from_grid(TileRowsC);
}

/*
 * @routine 050F:7CC TileRowD
 * @run 6B:97CC
 * WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
 * HOW: TileRowsD, eight rows of eight.
 * WAITS: no.
 * Entry and exit: as TileStepC.
 */
HandlerExit TileRowD(void)
{
    return row_from_grid(TileRowsD);
}

/* The row's step by the offset found for the column, one more. */
static HandlerExit step_offset(uint8_t column, uint8_t y)
{
    uint8_t row = RD8(ZP->temp02);
    uint8_t offset = rd8((uint16_t)(TileRowOffsets + y));
    Sum sum = add_bytes(row, offset, false);

    (void)column;
    WR8(ZP->temp02, (uint8_t)(sum.value + 1));
    return exit_with(y, sum);
}

/*
 * @routine 0510:0FD TileStepTwice
 * @run 6B:98FD
 * WHAT IT DOES: A slope tile: moves the row by the offset found through the column's offset.
 * HOW: One past the column's byte of TileRowOffsets indexes that table again.
 * WAITS: no.
 * Entry and exit: as TileStepC.
 */
HandlerExit TileStepTwice(void)
{
    uint8_t column = RD8(ZP->temp00);
    uint8_t first = rd8((uint16_t)(TileRowOffsets + column));
    uint8_t y = (uint8_t)(first + 1);

    return step_offset(column, y);
}

/*
 * @routine 0510:106 TileRowOffset
 * @run 6B:9906
 * WHAT IT DOES: A slope tile: a new row from TileRowsE, with the column's offset for its column.
 * HOW: Row times eight, plus the offset and one, indexes the table.
 * WAITS: no.
 * Entry and exit: as TileStepC.
 */
HandlerExit TileRowOffset(void)
{
    uint8_t row = RD8(ZP->temp02);
    uint8_t column = RD8(ZP->temp00);
    uint8_t offset = rd8((uint16_t)(TileRowOffsets + column));
    Sum sum = add_bytes((uint8_t)(row << 3), offset, false);
    uint8_t index = (uint8_t)(sum.value + 1);
    uint8_t value = rd8((uint16_t)(TileRowsE + index));

    WR8(ZP->temp02, value);
    return exit_with(index, sum);
}

/*
 * @routine 0510:114 TileStepOffset
 * @run 6B:9914
 * WHAT IT DOES: A slope tile: moves the row by one more than the column's offset.
 * HOW: TileRowOffsets by column, added to the row with one.
 * WAITS: no.
 * Entry and exit: as TileStepC.
 */
HandlerExit TileStepOffset(void)
{
    uint8_t column = RD8(ZP->temp00);

    return step_offset(column, column);
}

/*
 * @routine 0510:11F TileRowE
 * @run 6B:991F
 * WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
 * HOW: TileRowsE, eight rows of eight.
 * WAITS: no.
 * Entry and exit: as TileStepC.
 */
HandlerExit TileRowE(void)
{
    return row_from_grid(TileRowsE);
}

/*
 * @routine 0510:170 TileNudgeB
 * @run 6B:9970
 * WHAT IT DOES: The tile $94: the point's row one lower.
 * HOW: An increment.
 * WAITS: no.
 * Entry and exit: as TileNudgeA.
 */
HandlerExit TileNudgeB(void)
{
    return nudge_row();
}

/*
 * @routine 0510:175 TileLift7
 * @run 6B:9975
 * WHAT IT DOES: The tile $95: the point's row seven higher.
 * HOW: A subtract.
 * WAITS: no.
 * Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
 */
HandlerExit TileLift7(void)
{
    return lift_row(7);
}

/*
 * @routine 0510:17F TileLift6
 * @run 6B:997F
 * WHAT IT DOES: The tile $96: the point's row six higher.
 * HOW: A subtract.
 * WAITS: no.
 * Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
 */
HandlerExit TileLift6(void)
{
    return lift_row(6);
}

/*
 * @routine 0510:189 TileLift5
 * @run 6B:9989
 * WHAT IT DOES: The tile $97: the point's row five higher.
 * HOW: A subtract.
 * WAITS: no.
 * Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
 */
HandlerExit TileLift5(void)
{
    return lift_row(5);
}

/*
 * @routine 0510:193 TileLift4
 * @run 6B:9993
 * WHAT IT DOES: The tile $98: the point's row four higher.
 * HOW: A subtract.
 * WAITS: no.
 * Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
 */
HandlerExit TileLift4(void)
{
    return lift_row(4);
}

/*
 * @routine 0510:19D TileLift3
 * @run 6B:999D
 * WHAT IT DOES: The tile $99: the point's row three higher.
 * HOW: A subtract.
 * WAITS: no.
 * Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
 */
HandlerExit TileLift3(void)
{
    return lift_row(3);
}

/*
 * @routine 0510:1A7 TileLift2
 * @run 6B:99A7
 * WHAT IT DOES: The tile $9A: the point's row two higher.
 * HOW: A subtract.
 * WAITS: no.
 * Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
 */
HandlerExit TileLift2(void)
{
    return lift_row(2);
}

/*
 * @routine 0510:1B1 TileLift1
 * @run 6B:99B1
 * WHAT IT DOES: The tile $9B: the point's row one higher.
 * HOW: A subtract.
 * WAITS: no.
 * Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
 */
HandlerExit TileLift1(void)
{
    return lift_row(1);
}

/* A handler's exit: A $81, Y and C and V where it set them. */
static void handler_exit_shim(HandlerExit e)
{
    cpu.a = TILE_HANDLED_RESULT;
    abi_nz(TILE_HANDLED_RESULT);
    if (e.y_set) {
        cpu.y = e.y;
    }
    if (e.flags_set) {
        abi_flag(P_C, e.carry);
        abi_flag(P_V, e.overflow);
    }
}

void abi_TileStepC(void)
{
    handler_exit_shim(TileStepC());
}

void abi_TileRowA(void)
{
    handler_exit_shim(TileRowA());
}

void abi_TileRowB(void)
{
    handler_exit_shim(TileRowB());
}

void abi_TileHalfStepA(void)
{
    handler_exit_shim(TileHalfStepA(cpu.a));
}

void abi_TileHalfStepB(void)
{
    handler_exit_shim(TileHalfStepB(cpu.a));
}

void abi_TileNudgeA(void)
{
    handler_exit_shim(TileNudgeA());
}

void abi_TileStepD(void)
{
    handler_exit_shim(TileStepD());
}

void abi_TileRowC(void)
{
    handler_exit_shim(TileRowC());
}

void abi_TileRowD(void)
{
    handler_exit_shim(TileRowD());
}

void abi_TileStepTwice(void)
{
    handler_exit_shim(TileStepTwice());
}

void abi_TileRowOffset(void)
{
    handler_exit_shim(TileRowOffset());
}

void abi_TileStepOffset(void)
{
    handler_exit_shim(TileStepOffset());
}

void abi_TileRowE(void)
{
    handler_exit_shim(TileRowE());
}

void abi_TileNudgeB(void)
{
    handler_exit_shim(TileNudgeB());
}

void abi_TileLift7(void)
{
    handler_exit_shim(TileLift7());
}

void abi_TileLift6(void)
{
    handler_exit_shim(TileLift6());
}

void abi_TileLift5(void)
{
    handler_exit_shim(TileLift5());
}

void abi_TileLift4(void)
{
    handler_exit_shim(TileLift4());
}

void abi_TileLift3(void)
{
    handler_exit_shim(TileLift3());
}

void abi_TileLift2(void)
{
    handler_exit_shim(TileLift2());
}

void abi_TileLift1(void)
{
    handler_exit_shim(TileLift1());
}

/* Runs the handler at an address the table names, if it is one of this code's. */
static bool run_tile_handler(uint16_t code, uint8_t tile, HandlerExit *out)
{
    switch (code) {
    case 0x9752:
        *out = TileStepC();
        return true;
    case 0x975F:
        *out = TileRowA();
        return true;
    case 0x9770:
        *out = TileRowB();
        return true;
    case 0x9781:
        *out = TileHalfStepA(tile);
        return true;
    case 0x9795:
        *out = TileHalfStepB(tile);
        return true;
    case 0x97A9:
        *out = TileNudgeA();
        return true;
    case 0x97AE:
        *out = TileStepD();
        return true;
    case 0x97BB:
        *out = TileRowC();
        return true;
    case 0x97CC:
        *out = TileRowD();
        return true;
    case 0x98FD:
        *out = TileStepTwice();
        return true;
    case 0x9906:
        *out = TileRowOffset();
        return true;
    case 0x9914:
        *out = TileStepOffset();
        return true;
    case 0x991F:
        *out = TileRowE();
        return true;
    case 0x9970:
        *out = TileNudgeB();
        return true;
    case 0x9975:
        *out = TileLift7();
        return true;
    case 0x997F:
        *out = TileLift6();
        return true;
    case 0x9989:
        *out = TileLift5();
        return true;
    case 0x9993:
        *out = TileLift4();
        return true;
    case 0x999D:
        *out = TileLift3();
        return true;
    case 0x99A7:
        *out = TileLift2();
        return true;
    case 0x99B1:
        *out = TileLift1();
        return true;
    default:
        return false;
    }
}

static void tile_to_original_shim(uint8_t tile, uint8_t y, uint16_t code);

/*
 * @routine 050F:67F TerrainTileRead
 * @run 6B:967F
 * WHAT IT DOES: Says what the map holds at a point: the attribute of the background tile
 * there, 0 if its page has none, $81 for a slope or step, which moves the row in the tile.
 * HOW: The point plus the scroll gives the tile's place in the 64 by 128 background; its
 * entry is read from VRAM. The entry's low byte and a page by its high bits give the
 * attribute. Attributes $80-$9B go through a table of handlers, entered by a jump.
 * WAITS: no.
 * Entry: zTemp00-01 X, zTemp02-03 Y. Exit: A the answer, zTemp00 the column, zTemp02 the row.
 * X kept. Clobbers zTemp04-09.
 */
TileRead TerrainTileRead(void)
{
    TileRead read = { TILE_PLAIN, 0, 0, 0, 0, 0, { false, 0, false, false, false } };
    uint8_t byte;
    uint8_t other;
    uint8_t quarter;
    uint8_t entry_lo;
    uint8_t entry_hi;
    uint8_t high;
    uint8_t attribute;
    Sum x_low;
    Sum x_high;
    Sum y_low;
    Sum y_high;
    Sum row_sum;
    bool carry;
    uint8_t value;

    byte = RD8(ZP->temp00);
    other = RD8(SCROLL->x_lo[0]);
    x_low = add_bytes(byte, other, false);
    WR8(ZP->temp04, x_low.value);
    byte = RD8(ZP->temp01);
    other = RD8(SCROLL->x_hi[0]);
    x_high = add_bytes(byte, other, x_low.carry);
    carry = (x_high.value & 1) != 0;
    value = RD8(ZP->temp04);
    value = (uint8_t)((carry ? 0x80 : 0) | (value >> 1));
    value = (uint8_t)((value >> 2) & 0x3F);
    WR8(ZP->temp05, value);
    value = RD8(ZP->temp04);
    read.column = (uint8_t)(value & 7);
    WR8(ZP->temp00, read.column);
    byte = RD8(ZP->temp02);
    other = RD8(SCROLL->y_lo[0]);
    y_low = add_bytes(byte, other, false);
    WR8(ZP->temp06, y_low.value);
    byte = RD8(ZP->temp03);
    other = RD8(SCROLL->y_hi[0]);
    y_high = add_bytes(byte, other, y_low.carry);
    WR8(ZP->temp07, (uint8_t)(y_high.value & 1));
    value = RD8(ZP->temp06);
    carry = (value & 0x80) != 0;
    ROL8(ZP->temp07, carry);
    value = (uint8_t)(value << 1);
    carry = (value & 0x80) != 0;
    ROL8(ZP->temp07, carry);
    value = (uint8_t)(value << 1);
    carry = (value & 0x80) != 0;
    ROL8(ZP->temp07, carry);
    value = (uint8_t)(value << 1);
    quarter = RD8(ZP->temp05);
    row_sum = add_bytes((uint8_t)(value & 0xC0), quarter, false);
    WR8(ZP->temp05, row_sum.value);
    value = RD8(ZP->temp06);
    read.row = (uint8_t)(value & 7);
    WR8(ZP->temp02, read.row);
    value = RD8(ZP->temp05);
    WR8(ZP->temp06, value);
    jsr_push(0x96CE);
    SelectVramReadAddress();
    jsr_pop();
    value = RD8(ZP->temp05);
    WR8(VDC->data_lo, value);
    value = RD8(ZP->temp07);
    WR8(VDC->data_hi, value);
    jsr_push(0x96DB);
    SelectVramData();
    jsr_pop();
    entry_lo = RD8(VDC->data_lo);
    WR8(ZP->temp08, entry_lo);
    entry_hi = RD8(VDC->data_hi);
    high = (uint8_t)(entry_hi & 7);
    WR8(ZP->temp09, high);
    read.high = high;
    read.page = rd8((uint16_t)(SaveBuffer + high));
    if (read.page & 0x80) {
        read.kind = TILE_EMPTY_PAGE;
        return read;
    }
    byte = RD8(ZP->temp08);
    WR8(ZP->temp08, byte);
    read.page = rd8((uint16_t)(SaveBuffer + high));
    WR8(ZP->temp09, (uint8_t)(read.page + TILE_PAGE_BASE));
    attribute = RD8_PTR(ZP->temp08);
    read.attribute = attribute;
    if ((attribute & 0x80) == 0) {
        return read;
    }
    if (attribute >= TILE_END_HANDLED) {
        read.kind = TILE_PAST_HANDLED;
        return read;
    }
    push8(attribute);
    value = (uint8_t)((uint8_t)(attribute - TILE_FIRST_HANDLED) << 1);
    byte = rd8((uint16_t)(TileHandlers + value));
    WR8(ZP->temp04, byte);
    byte = rd8((uint16_t)(TileHandlers + 1 + value));
    WR8(ZP->temp05, byte);
    attribute = pull8();
    {
        uint16_t code = RD16_LH2(ZP->temp04, ZP->temp05);

        read.kind = TILE_HANDLED;
        if (!run_tile_handler(code, attribute, &read.handler)) {
            read.kind = TILE_BY_ORIGINAL;
            tile_to_original_shim(attribute, value, code);
            read.attribute = cpu.a;
        }
    }
    return read;
}

/* The pushed status byte at the lookups' PHP: the carry and overflow of the add, B set, the rest clear. */
static uint8_t pushed_status(Sum sum)
{
    return (uint8_t)(P_B | (sum.carry ? P_C : 0) | (sum.overflow ? P_V : 0));
}

/* What a read answers in A. */
uint8_t tile_answer(TileRead read)
{
    switch (read.kind) {
    case TILE_EMPTY_PAGE:
        return 0;
    case TILE_HANDLED:
        return TILE_HANDLED_RESULT;
    default:
        return read.attribute;
    }
}

/* A handler's table entry that is not one of this code's: the original's code runs, entered as the jump leaves the registers. */
static void tile_to_original_shim(uint8_t tile, uint8_t y, uint16_t code)
{
    cpu.a = tile;
    cpu.y = y;
    abi_nz(tile);
    abi_flag(P_C, false);
    abi_flag(P_V, true);
    call_original(code);
}

/* A, Y, C and V as a read leaves them. The page's add sets V, and an empty page's is clear. */
void terrain_read_exit_shim(TileRead read)
{
    bool page_overflow = read.page >= 3;

    if (read.kind == TILE_BY_ORIGINAL) {
        return;
    }
    cpu.y = read.high;
    switch (read.kind) {
    case TILE_BY_ORIGINAL:
        break;
    case TILE_EMPTY_PAGE:
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, false);
        abi_flag(P_V, false);
        break;
    case TILE_PLAIN:
        cpu.a = read.attribute;
        abi_nz(cpu.a);
        abi_flag(P_C, false);
        abi_flag(P_V, page_overflow);
        break;
    case TILE_PAST_HANDLED:
        cpu.a = read.attribute;
        abi_nz((uint8_t)(read.attribute - TILE_END_HANDLED));
        abi_flag(P_C, true);
        abi_flag(P_V, page_overflow);
        break;
    case TILE_HANDLED:
        /* The jump to the handler leaves C clear and V set, which the nudges keep. */
        abi_flag(P_C, false);
        abi_flag(P_V, true);
        cpu.y = (uint8_t)((read.attribute - TILE_FIRST_HANDLED) << 1);
        handler_exit_shim(read.handler);
        break;
    }
}

void abi_TerrainTileRead(void)
{
    terrain_read_exit_shim(TerrainTileRead());
}

/*
 * The read at $8006 of whichever stage overlay is in RAM: this code, when the overlay holds it, else
 * the overlay's own, which has no C.
 */
TileRead StageTileRead(void)
{
    TileRead read = { TILE_BY_ORIGINAL, 0, 0, 0, 0, 0, { false, 0, false, false, false } };

    if (resident_at(TILE_READ_RUN) == DISC(0x050F, 0x67F)) {
        return TerrainTileRead();
    }
    call_original(FrameOverlay8006);
    read.attribute = cpu.a;
    return read;
}

/* One column's offsets from the object, and what its lookup found. */
static void store_column(uint8_t column, uint8_t answer)
{
    uint8_t value;

    WR8(LEVEL->tile_hit[column], answer);
    value = RD8(ZP->temp00);
    WR8(LEVEL->tile_depth_x[column], value);
    value = RD8(ZP->temp02);
    WR8(LEVEL->tile_depth_y[column], value);
}

/*
 * Reads the terrain at the four columns of a probe around an object: the attribute, and how far
 * into the tile, for each. A column marked as absent reads as nothing. The probe table is the
 * overlay's.
 */
static LookupEnd lookup_columns(uint8_t probe, uint8_t object, uint16_t table)
{
    LookupEnd end = { false, false, 0, 0, false, { TILE_PLAIN, 0, 0, 0, 0, 0, { false, 0, false, false, false } }, 0 };
    uint8_t y = (uint8_t)(probe << 1);
    uint8_t byte;
    uint8_t column;

    byte = rd8((uint16_t)(table + y));
    WR8(ZP->temp0c, byte);
    byte = rd8((uint16_t)(table + 1 + y));
    WR8(ZP->temp0d, byte);
    y = 0;
    WR8(ZP->temp0a, 0);
    for (;;) {
        uint8_t offset_low = RD8_PTR_Y(ZP->temp0c, y);
        uint8_t object_low = RD8(OBJ->x_lo[object]);
        Sum low = add_bytes(offset_low, object_low, false);
        uint8_t offset_high;
        uint8_t answer;

        WR8(ZP->temp00, low.value);
        y++;
        push8(pushed_status(low));
        offset_high = RD8_PTR_Y(ZP->temp0c, y);
        end.offset_low = offset_low;
        end.object_low = object_low;
        if (offset_high == PROBE_NO_COLUMN) {
            end.last_skipped = true;
            pull8();
            y = (uint8_t)(y + 3);
            push8(y);
            WR8(ZP->temp00, 0);
            WR8(ZP->temp02, 0);
            answer = 0;
        } else {
            uint8_t carry_byte = pull8();
            uint8_t object_high = RD8(OBJ->x_hi[object]);
            Sum high = add_bytes(offset_high, object_high, (carry_byte & P_C) != 0);
            uint8_t y_offset;
            Sum ylow;
            Sum yhigh;
            TileRead read;

            end.last_skipped = false;
            WR8(ZP->temp01, high.value);
            y++;
            y_offset = RD8_PTR_Y(ZP->temp0c, y);
            byte = RD8(OBJ->y_lo[object]);
            ylow = add_bytes(y_offset, byte, false);
            WR8(ZP->temp02, ylow.value);
            y++;
            y_offset = RD8_PTR_Y(ZP->temp0c, y);
            byte = RD8(OBJ->y_hi[object]);
            yhigh = add_bytes(y_offset, byte, ylow.carry);
            WR8(ZP->temp03, yhigh.value);
            y++;
            push8(y);
            jsr_push(0x95E6);
            read = TerrainTileRead();
            jsr_pop();
            end.last = read;
            end.read_any = true;
            answer = tile_answer(read);
        }
        column = RD8(ZP->temp0a);
        store_column(column, answer);
        INC8(ZP->temp0a);
        y = pull8();
        byte = RD8(ZP->temp0a);
        if (byte == PROBE_COLUMNS) {
            break;
        }
    }
    return end;
}

/* Y 16 and A 4 with C set and Z set from the compare that ended the loop; V by the last column. */
void lookup_exit_shim(LookupEnd end)
{
    if (end.by_original) {
        return;
    }
    if (end.last_skipped) {
        Sum sum = add_bytes(end.offset_low, end.object_low, false);

        abi_flag(P_V, sum.overflow);
    } else if (end.read_any) {
        terrain_read_exit_shim(end.last);
    }
    cpu.a = PROBE_COLUMNS;
    cpu.y = PROBE_COLUMNS * PROBE_COLUMN_BYTES;
    abi_nz(0);
    abi_flag(P_C, true);
    abi_flag(P_B, true);    /* the last PLP pulled the B the PHP pushed */
}

/*
 * The second look: TerrainLookup, then each column that found floor looked at again eight
 * pixels above its point; if that too is solid the floor bits come off the column's answer.
 */
static LookupEnd lookup_alt_columns(uint8_t probe, uint8_t object, uint16_t table, LookupEnd (*lookup)(uint8_t, uint8_t))
{
    LookupEnd end;
    uint8_t y;
    uint8_t byte;
    uint8_t column;

    push8(probe);
    jsr_push(0x960F);
    end = lookup(probe, object);
    jsr_pop();
    probe = pull8();
    y = (uint8_t)(probe << 1);
    byte = rd8((uint16_t)(table + y));
    WR8(ZP->temp0c, byte);
    byte = rd8((uint16_t)(table + 1 + y));
    WR8(ZP->temp0d, byte);
    y = 0;
    WR8(ZP->temp0a, 0);
    for (;;) {
        uint8_t hit;
        uint8_t wanted;

        push8(y);
        column = RD8(ZP->temp0a);
        hit = RD8(LEVEL->tile_hit[column]);
        y = pull8();
        wanted = (uint8_t)(hit & HIT_NEEDS_SECOND_LOOK);
        if ((wanted & HIT_SECOND_LOOK_SKIP) != 0 || wanted == 0) {
            y = (uint8_t)(y + 4);
        } else {
            uint8_t offset;
            uint8_t other;
            Sum low;
            Sum high;
            Sum first;
            Sum second;
            TileRead read;

            offset = RD8_PTR_Y(ZP->temp0c, y);
            other = RD8(OBJ->x_lo[object]);
            low = add_bytes(offset, other, false);
            WR8(ZP->temp00, low.value);
            y++;
            offset = RD8_PTR_Y(ZP->temp0c, y);
            other = RD8(OBJ->x_hi[object]);
            high = add_bytes(offset, other, low.carry);
            WR8(ZP->temp01, high.value);
            y++;
            offset = RD8_PTR_Y(ZP->temp0c, y);
            other = RD8(OBJ->y_lo[object]);
            low = add_bytes(offset, other, false);
            WR8(ZP->temp02, low.value);
            y++;
            offset = RD8_PTR_Y(ZP->temp0c, y);
            other = RD8(OBJ->y_hi[object]);
            high = add_bytes(offset, other, low.carry);
            WR8(ZP->temp03, high.value);
            y++;
            other = RD8(ZP->temp02);
            first = add_bytes(SECOND_LOOK_RAISE, other, false);
            WR8(ZP->temp02, first.value);
            other = RD8(ZP->temp02);
            second = add_bytes(0xFF, other, first.carry);
            WR8(ZP->temp02, second.value);
            push8(y);
            jsr_push(0x965F);
            read = TerrainTileRead();
            jsr_pop();
            end.last = read;
            end.read_any = true;
            end.last_skipped = false;
            column = RD8(ZP->temp0a);
            if ((tile_answer(read) & SECOND_LOOK_SOLID) != 0) {
                column = RD8(ZP->temp0a);
                hit = RD8(LEVEL->tile_hit[column]);
                WR8(LEVEL->tile_hit[column], (uint8_t)(hit & HIT_CLEARED_BY_SECOND_LOOK));
            }
            y = pull8();
        }
        INC8(ZP->temp0a);
        byte = RD8(ZP->temp0a);
        if (byte == PROBE_COLUMNS) {
            break;
        }
    }
    return end;
}

static LookupEnd TerrainLookup_body(uint8_t probe, uint8_t object)
{
    return lookup_columns(probe, object, 0xCBE0);
}

/*
 * @routine 050F:5AB TerrainLookup
 * @run 6B:95AB
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
 * attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
 * HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
 * reads there. Answers go to TileHit, TileDepthX and TileDepthY.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
 * read or add. Clobbers zTemp00-0D.
 */
LookupEnd TerrainLookup(uint8_t probe, uint8_t object)
{
    return TerrainLookup_body(probe, object);
}

void abi_TerrainLookup(void)
{
    lookup_exit_shim(TerrainLookup(cpu.a, cpu.x));
}

/*
 * @routine 050F:60B TerrainLookupAlt
 * @run 6B:960B
 * WHAT IT DOES: TerrainLookup, then a second look at each column that found floor: eight
 * pixels above it, and if that too is solid the floor bits come off the column's answer.
 * HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
 * as the original has it, not the high.
 * WAITS: no.
 * Entry and exit: as TerrainLookup.
 */
LookupEnd TerrainLookupAlt(uint8_t probe, uint8_t object)
{
    return lookup_alt_columns(probe, object, 0xCBE0, TerrainLookup_body);
}

void abi_TerrainLookupAlt(void)
{
    lookup_exit_shim(TerrainLookupAlt(cpu.a, cpu.x));
}

static LookupEnd TerrainLookupB_body(uint8_t probe, uint8_t object)
{
    return lookup_columns(probe, object, 0xC11F);
}

/*
 * @routine 024F:5AB TerrainLookupB
 * @run 6B:95AB
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
 * attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
 * HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
 * reads there. Answers go to TileHit, TileDepthX and TileDepthY.
 * The same code as TerrainLookup; this overlay's probe table is at $C11F.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
 * read or add. Clobbers zTemp00-0D.
 */
LookupEnd TerrainLookupB(uint8_t probe, uint8_t object)
{
    return TerrainLookupB_body(probe, object);
}

void abi_TerrainLookupB(void)
{
    lookup_exit_shim(TerrainLookupB(cpu.a, cpu.x));
}

/*
 * @routine 024F:60B TerrainLookupAltB
 * @run 6B:960B
 * WHAT IT DOES: TerrainLookupB, then a second look at each column that found floor: eight
 * pixels above it, and if that too is solid the floor bits come off the column's answer.
 * HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
 * as the original has it, not the high.
 * The same code as TerrainLookupAlt; this overlay's probe table is at $C11F.
 * WAITS: no.
 * Entry and exit: as TerrainLookupB.
 */
LookupEnd TerrainLookupAltB(uint8_t probe, uint8_t object)
{
    return lookup_alt_columns(probe, object, 0xC11F, TerrainLookupB_body);
}

void abi_TerrainLookupAltB(void)
{
    lookup_exit_shim(TerrainLookupAltB(cpu.a, cpu.x));
}

static LookupEnd TerrainLookupC_body(uint8_t probe, uint8_t object)
{
    return lookup_columns(probe, object, 0xD9DE);
}

/*
 * @routine 07CF:5AB TerrainLookupC
 * @run 6B:95AB
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
 * attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
 * HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
 * reads there. Answers go to TileHit, TileDepthX and TileDepthY.
 * The same code as TerrainLookup; this overlay's probe table is at $D9DE.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
 * read or add. Clobbers zTemp00-0D.
 */
LookupEnd TerrainLookupC(uint8_t probe, uint8_t object)
{
    return TerrainLookupC_body(probe, object);
}

void abi_TerrainLookupC(void)
{
    lookup_exit_shim(TerrainLookupC(cpu.a, cpu.x));
}

/*
 * @routine 07CF:60B TerrainLookupAltC
 * @run 6B:960B
 * WHAT IT DOES: TerrainLookupC, then a second look at each column that found floor: eight
 * pixels above it, and if that too is solid the floor bits come off the column's answer.
 * HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
 * as the original has it, not the high.
 * The same code as TerrainLookupAlt; this overlay's probe table is at $D9DE.
 * WAITS: no.
 * Entry and exit: as TerrainLookupC.
 */
LookupEnd TerrainLookupAltC(uint8_t probe, uint8_t object)
{
    return lookup_alt_columns(probe, object, 0xD9DE, TerrainLookupC_body);
}

void abi_TerrainLookupAltC(void)
{
    lookup_exit_shim(TerrainLookupAltC(cpu.a, cpu.x));
}

static LookupEnd TerrainLookupD_body(uint8_t probe, uint8_t object)
{
    return lookup_columns(probe, object, 0xB547);
}

/*
 * @routine 092F:5AB TerrainLookupD
 * @run 6B:95AB
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
 * attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
 * HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
 * reads there. Answers go to TileHit, TileDepthX and TileDepthY.
 * The same code as TerrainLookup; this overlay's probe table is at $B547.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
 * read or add. Clobbers zTemp00-0D.
 */
LookupEnd TerrainLookupD(uint8_t probe, uint8_t object)
{
    return TerrainLookupD_body(probe, object);
}

void abi_TerrainLookupD(void)
{
    lookup_exit_shim(TerrainLookupD(cpu.a, cpu.x));
}

/*
 * @routine 092F:60B TerrainLookupAltD
 * @run 6B:960B
 * WHAT IT DOES: TerrainLookupD, then a second look at each column that found floor: eight
 * pixels above it, and if that too is solid the floor bits come off the column's answer.
 * HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
 * as the original has it, not the high.
 * The same code as TerrainLookupAlt; this overlay's probe table is at $B547.
 * WAITS: no.
 * Entry and exit: as TerrainLookupD.
 */
LookupEnd TerrainLookupAltD(uint8_t probe, uint8_t object)
{
    return lookup_alt_columns(probe, object, 0xB547, TerrainLookupD_body);
}

void abi_TerrainLookupAltD(void)
{
    lookup_exit_shim(TerrainLookupAltD(cpu.a, cpu.x));
}

static LookupEnd TerrainLookupE_body(uint8_t probe, uint8_t object)
{
    return lookup_columns(probe, object, 0xBD92);
}

/*
 * @routine 0A8F:5AB TerrainLookupE
 * @run 6B:95AB
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
 * attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
 * HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
 * reads there. Answers go to TileHit, TileDepthX and TileDepthY.
 * The same code as TerrainLookup; this overlay's probe table is at $BD92.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
 * read or add. Clobbers zTemp00-0D.
 */
LookupEnd TerrainLookupE(uint8_t probe, uint8_t object)
{
    return TerrainLookupE_body(probe, object);
}

void abi_TerrainLookupE(void)
{
    lookup_exit_shim(TerrainLookupE(cpu.a, cpu.x));
}

/*
 * @routine 0A8F:60B TerrainLookupAltE
 * @run 6B:960B
 * WHAT IT DOES: TerrainLookupE, then a second look at each column that found floor: eight
 * pixels above it, and if that too is solid the floor bits come off the column's answer.
 * HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
 * as the original has it, not the high.
 * The same code as TerrainLookupAlt; this overlay's probe table is at $BD92.
 * WAITS: no.
 * Entry and exit: as TerrainLookupE.
 */
LookupEnd TerrainLookupAltE(uint8_t probe, uint8_t object)
{
    return lookup_alt_columns(probe, object, 0xBD92, TerrainLookupE_body);
}

void abi_TerrainLookupAltE(void)
{
    lookup_exit_shim(TerrainLookupAltE(cpu.a, cpu.x));
}

static LookupEnd TerrainLookupF_body(uint8_t probe, uint8_t object)
{
    return lookup_columns(probe, object, 0xD658);
}

/*
 * @routine 0BEF:5AB TerrainLookupF
 * @run 6B:95AB
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
 * attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
 * HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
 * reads there. Answers go to TileHit, TileDepthX and TileDepthY.
 * The same code as TerrainLookup; this overlay's probe table is at $D658.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
 * read or add. Clobbers zTemp00-0D.
 */
LookupEnd TerrainLookupF(uint8_t probe, uint8_t object)
{
    return TerrainLookupF_body(probe, object);
}

void abi_TerrainLookupF(void)
{
    lookup_exit_shim(TerrainLookupF(cpu.a, cpu.x));
}

/*
 * @routine 0BEF:60B TerrainLookupAltF
 * @run 6B:960B
 * WHAT IT DOES: TerrainLookupF, then a second look at each column that found floor: eight
 * pixels above it, and if that too is solid the floor bits come off the column's answer.
 * HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
 * as the original has it, not the high.
 * The same code as TerrainLookupAlt; this overlay's probe table is at $D658.
 * WAITS: no.
 * Entry and exit: as TerrainLookupF.
 */
LookupEnd TerrainLookupAltF(uint8_t probe, uint8_t object)
{
    return lookup_alt_columns(probe, object, 0xD658, TerrainLookupF_body);
}

void abi_TerrainLookupAltF(void)
{
    lookup_exit_shim(TerrainLookupAltF(cpu.a, cpu.x));
}

static LookupEnd TerrainLookupG_body(uint8_t probe, uint8_t object)
{
    return lookup_columns(probe, object, 0xD771);
}

/*
 * @routine 0D4F:5AB TerrainLookupG
 * @run 6B:95AB
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
 * attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
 * HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
 * reads there. Answers go to TileHit, TileDepthX and TileDepthY.
 * The same code as TerrainLookup; this overlay's probe table is at $D771.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
 * read or add. Clobbers zTemp00-0D.
 */
LookupEnd TerrainLookupG(uint8_t probe, uint8_t object)
{
    return TerrainLookupG_body(probe, object);
}

void abi_TerrainLookupG(void)
{
    lookup_exit_shim(TerrainLookupG(cpu.a, cpu.x));
}

/*
 * @routine 0D4F:60B TerrainLookupAltG
 * @run 6B:960B
 * WHAT IT DOES: TerrainLookupG, then a second look at each column that found floor: eight
 * pixels above it, and if that too is solid the floor bits come off the column's answer.
 * HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
 * as the original has it, not the high.
 * The same code as TerrainLookupAlt; this overlay's probe table is at $D771.
 * WAITS: no.
 * Entry and exit: as TerrainLookupG.
 */
LookupEnd TerrainLookupAltG(uint8_t probe, uint8_t object)
{
    return lookup_alt_columns(probe, object, 0xD771, TerrainLookupG_body);
}

void abi_TerrainLookupAltG(void)
{
    lookup_exit_shim(TerrainLookupAltG(cpu.a, cpu.x));
}

static LookupEnd TerrainLookupH_body(uint8_t probe, uint8_t object)
{
    return lookup_columns(probe, object, 0xB23F);
}

/*
 * @routine 0EAF:5AB TerrainLookupH
 * @run 6B:95AB
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
 * attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
 * HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
 * reads there. Answers go to TileHit, TileDepthX and TileDepthY.
 * The same code as TerrainLookup; this overlay's probe table is at $B23F.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
 * read or add. Clobbers zTemp00-0D.
 */
LookupEnd TerrainLookupH(uint8_t probe, uint8_t object)
{
    return TerrainLookupH_body(probe, object);
}

void abi_TerrainLookupH(void)
{
    lookup_exit_shim(TerrainLookupH(cpu.a, cpu.x));
}

/*
 * @routine 0EAF:60B TerrainLookupAltH
 * @run 6B:960B
 * WHAT IT DOES: TerrainLookupH, then a second look at each column that found floor: eight
 * pixels above it, and if that too is solid the floor bits come off the column's answer.
 * HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
 * as the original has it, not the high.
 * The same code as TerrainLookupAlt; this overlay's probe table is at $B23F.
 * WAITS: no.
 * Entry and exit: as TerrainLookupH.
 */
LookupEnd TerrainLookupAltH(uint8_t probe, uint8_t object)
{
    return lookup_alt_columns(probe, object, 0xB23F, TerrainLookupH_body);
}

void abi_TerrainLookupAltH(void)
{
    lookup_exit_shim(TerrainLookupAltH(cpu.a, cpu.x));
}

static LookupEnd TerrainLookupI_body(uint8_t probe, uint8_t object)
{
    return lookup_columns(probe, object, 0xD2CF);
}

/*
 * @routine 100F:5AB TerrainLookupI
 * @run 6B:95AB
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
 * attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
 * HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
 * reads there. Answers go to TileHit, TileDepthX and TileDepthY.
 * The same code as TerrainLookup; this overlay's probe table is at $D2CF.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
 * read or add. Clobbers zTemp00-0D.
 */
LookupEnd TerrainLookupI(uint8_t probe, uint8_t object)
{
    return TerrainLookupI_body(probe, object);
}

void abi_TerrainLookupI(void)
{
    lookup_exit_shim(TerrainLookupI(cpu.a, cpu.x));
}

/*
 * @routine 100F:60B TerrainLookupAltI
 * @run 6B:960B
 * WHAT IT DOES: TerrainLookupI, then a second look at each column that found floor: eight
 * pixels above it, and if that too is solid the floor bits come off the column's answer.
 * HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
 * as the original has it, not the high.
 * The same code as TerrainLookupAlt; this overlay's probe table is at $D2CF.
 * WAITS: no.
 * Entry and exit: as TerrainLookupI.
 */
LookupEnd TerrainLookupAltI(uint8_t probe, uint8_t object)
{
    return lookup_alt_columns(probe, object, 0xD2CF, TerrainLookupI_body);
}

void abi_TerrainLookupAltI(void)
{
    lookup_exit_shim(TerrainLookupAltI(cpu.a, cpu.x));
}

static LookupEnd TerrainLookupJ_body(uint8_t probe, uint8_t object)
{
    return lookup_columns(probe, object, 0xD9A2);
}

/*
 * @routine 116F:5AB TerrainLookupJ
 * @run 6B:95AB
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
 * attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
 * HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
 * reads there. Answers go to TileHit, TileDepthX and TileDepthY.
 * The same code as TerrainLookup; this overlay's probe table is at $D9A2.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
 * read or add. Clobbers zTemp00-0D.
 */
LookupEnd TerrainLookupJ(uint8_t probe, uint8_t object)
{
    return TerrainLookupJ_body(probe, object);
}

void abi_TerrainLookupJ(void)
{
    lookup_exit_shim(TerrainLookupJ(cpu.a, cpu.x));
}

/*
 * @routine 116F:60B TerrainLookupAltJ
 * @run 6B:960B
 * WHAT IT DOES: TerrainLookupJ, then a second look at each column that found floor: eight
 * pixels above it, and if that too is solid the floor bits come off the column's answer.
 * HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
 * as the original has it, not the high.
 * The same code as TerrainLookupAlt; this overlay's probe table is at $D9A2.
 * WAITS: no.
 * Entry and exit: as TerrainLookupJ.
 */
LookupEnd TerrainLookupAltJ(uint8_t probe, uint8_t object)
{
    return lookup_alt_columns(probe, object, 0xD9A2, TerrainLookupJ_body);
}

void abi_TerrainLookupAltJ(void)
{
    lookup_exit_shim(TerrainLookupAltJ(cpu.a, cpu.x));
}

static LookupEnd TerrainLookupK_body(uint8_t probe, uint8_t object)
{
    return lookup_columns(probe, object, 0xD6C8);
}

/*
 * @routine 160F:5AB TerrainLookupK
 * @run 6B:95AB
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
 * attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
 * HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
 * reads there. Answers go to TileHit, TileDepthX and TileDepthY.
 * The same code as TerrainLookup; this overlay's probe table is at $D6C8.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
 * read or add. Clobbers zTemp00-0D.
 */
LookupEnd TerrainLookupK(uint8_t probe, uint8_t object)
{
    return TerrainLookupK_body(probe, object);
}

void abi_TerrainLookupK(void)
{
    lookup_exit_shim(TerrainLookupK(cpu.a, cpu.x));
}

/*
 * @routine 160F:60B TerrainLookupAltK
 * @run 6B:960B
 * WHAT IT DOES: TerrainLookupK, then a second look at each column that found floor: eight
 * pixels above it, and if that too is solid the floor bits come off the column's answer.
 * HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
 * as the original has it, not the high.
 * The same code as TerrainLookupAlt; this overlay's probe table is at $D6C8.
 * WAITS: no.
 * Entry and exit: as TerrainLookupK.
 */
LookupEnd TerrainLookupAltK(uint8_t probe, uint8_t object)
{
    return lookup_alt_columns(probe, object, 0xD6C8, TerrainLookupK_body);
}

void abi_TerrainLookupAltK(void)
{
    lookup_exit_shim(TerrainLookupAltK(cpu.a, cpu.x));
}

/* One call to another overlay's copy of a lookup, which has no C yet. */
static LookupEnd original_lookup_shim(uint16_t vector, uint8_t probe, uint8_t object, uint8_t column, bool carry_in)
{
    LookupEnd end = { true };

    cpu.a = probe;
    abi_nz(probe);
    cpu.x = object;
    cpu.y = column;
    abi_flag(P_C, carry_in);
    call_original(vector);
    end.column_after = cpu.y;
    return end;
}

typedef struct {
    uint32_t disc;
    LookupEnd (*lookup)(uint8_t, uint8_t);
    LookupEnd (*alt)(uint8_t, uint8_t);
} LookupVariant;

static const LookupVariant lookup_variants[] = {
    { DISC(0x050F, 0x5AB), TerrainLookup, TerrainLookupAlt },
    { DISC(0x024F, 0x5AB), TerrainLookupB, TerrainLookupAltB },
    { DISC(0x07CF, 0x5AB), TerrainLookupC, TerrainLookupAltC },
    { DISC(0x092F, 0x5AB), TerrainLookupD, TerrainLookupAltD },
    { DISC(0x0A8F, 0x5AB), TerrainLookupE, TerrainLookupAltE },
    { DISC(0x0BEF, 0x5AB), TerrainLookupF, TerrainLookupAltF },
    { DISC(0x0D4F, 0x5AB), TerrainLookupG, TerrainLookupAltG },
    { DISC(0x0EAF, 0x5AB), TerrainLookupH, TerrainLookupAltH },
    { DISC(0x100F, 0x5AB), TerrainLookupI, TerrainLookupAltI },
    { DISC(0x116F, 0x5AB), TerrainLookupJ, TerrainLookupAltJ },
    { DISC(0x160F, 0x5AB), TerrainLookupK, TerrainLookupAltK },
};

static const LookupVariant *resident_lookup(uint32_t disc)
{
    unsigned i;

    for (i = 0; i < sizeof lookup_variants / sizeof lookup_variants[0]; i++) {
        if (lookup_variants[i].disc == disc) {
            return &lookup_variants[i];
        }
    }
    return 0;
}

/*
 * The lookups at $800C and $800F of whichever stage overlay is in RAM. Y on entry is a column
 * only to another overlay's code, and all of this code's leave it past their last column.
 */
LookupEnd StageLookup(uint8_t probe, uint8_t object, uint8_t column, bool carry_in)
{
    const LookupVariant *variant = resident_lookup(resident_at(LOOKUP_RUN));
    LookupEnd end;

    if (variant == 0) {
        return original_lookup_shim(OverlayTerrainLookup, probe, object, column, carry_in);
    }
    end = variant->lookup(probe, object);
    end.column_after = PROBE_COLUMNS * PROBE_COLUMN_BYTES;
    return end;
}

LookupEnd StageLookupAlt(uint8_t probe, uint8_t object, uint8_t column, bool carry_in)
{
    const LookupVariant *variant = resident_lookup(resident_at(LOOKUP_RUN));
    LookupEnd end;

    if (variant == 0) {
        return original_lookup_shim(OverlayTerrainLookupAlt, probe, object, column, carry_in);
    }
    end = variant->alt(probe, object);
    end.column_after = PROBE_COLUMNS * PROBE_COLUMN_BYTES;
    return end;
}
