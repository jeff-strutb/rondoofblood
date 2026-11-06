/*
 * What the map holds under a point, from the stage overlay: the tile the background has
 * there, read from VRAM, and through a page of attributes what that tile is. Attributes of
 * $80-$9B are slopes and steps, each with a handler that moves the point's row within the
 * tile. The two lookups read a probe's columns around an object into TileHit.
 *
 * This is the copy in the overlay at sector $50D. 11 of the 40 overlays hold the same code at
 * the same addresses, differing only in the probe table's address, and the scripts run two
 * of them, $24D and $50D. The other overlays' shapes have no C yet.
 *
 * zTemp00-09 and zTemp0A-0D are RAM these routines leave behind, so they are written here.
 * Only the wrappers and exit shims know registers and flags.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

void SelectVramReadAddress(void);

enum {
    TILE_FIRST_HANDLED = 0x80, TILE_END_HANDLED = 0x9C, TILE_HANDLED_RESULT = 0x81,
    TILE_PAGE_BASE = 0x7D, PROBE_COLUMNS = 4, PROBE_COLUMN_BYTES = 4, PROBE_NO_COLUMN = 0x80,
    HIT_NEEDS_SECOND_LOOK = 0xA1, HIT_SECOND_LOOK_SKIP = 0x80, SECOND_LOOK_SOLID = 0x01,
    HIT_CLEARED_BY_SECOND_LOOK = 0x5E, SECOND_LOOK_RAISE = 0xF8
};

static uint16_t read_word(uint16_t lo, uint16_t hi)
{
    return (uint16_t)(rd8(lo) | (rd8(hi) << 8));
}

static void add_to_row(uint8_t amount)
{
    wr8(zTemp02, (uint8_t)(rd8(zTemp02) + amount));
}

/* Byte $column of a table of eight, added to the row. */
static void step_by_column(uint16_t table)
{
    add_to_row(rd8((uint16_t)(table + rd8(zTemp00))));
}

/* A table of eight rows of eight, by row and column, gives the new row. */
static void row_from_grid(uint16_t table)
{
    wr8(zTemp02, rd8((uint16_t)(table + (uint8_t)((rd8(zTemp02) << 3) + rd8(zTemp00)))));
}

static uint8_t row_offset_for_column(void)
{
    return rd8((uint16_t)(TileRowOffsets + rd8(zTemp00)));
}

/*
 * @routine 050F:752 TileStepC
 * @run 6B:9752
 * WHAT IT DOES: A slope tile: moves the point's row by a table on its column.
 * HOW: TileStepsC by column, added to the row.
 * Entry: zTemp00 column, zTemp02 row. Exit: as tile_handler_exit_shim for the tile in A.
 */
void TileStepC(void)
{
    step_by_column(TileStepsC);
}

/*
 * @routine 050F:75F TileRowA
 * @run 6B:975F
 * WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
 * HOW: TileRowsA, eight rows of eight.
 * Entry and exit: as TileStepC.
 */
void TileRowA(void)
{
    row_from_grid(TileRowsA);
}

/*
 * @routine 050F:770 TileRowB
 * @run 6B:9770
 * WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
 * HOW: TileRowsB, eight rows of eight.
 * Entry and exit: as TileStepC.
 */
void TileRowB(void)
{
    row_from_grid(TileRowsB);
}

/*
 * @routine 050F:781 TileHalfStepA
 * @run 6B:9781
 * WHAT IT DOES: The tiles $87-$8A: a step that differs between the tile's left and right
 * halves.
 * HOW: A pair of bytes in TileHalfStepsA by tile, the second for the right half.
 * Entry: A tile, zTemp00 column, zTemp02 row. Exit: as TileStepC.
 */
void TileHalfStepA(uint8_t tile)
{
    uint8_t pair = (uint8_t)((tile - 0x83) << 1);

    add_to_row(rd8((uint16_t)(TileHalfStepsA + pair + ((rd8(zTemp00) & 4) ? 1 : 0))));
}

/*
 * @routine 050F:795 TileHalfStepB
 * @run 6B:9795
 * WHAT IT DOES: The tiles $8C-$8F: as TileHalfStepA, the other way round.
 * HOW: A pair of bytes in TileHalfStepsB by tile, the second for the left half.
 * Entry and exit: as TileHalfStepA.
 */
void TileHalfStepB(uint8_t tile)
{
    uint8_t pair = (uint8_t)((tile - 0x8C) << 1);

    add_to_row(rd8((uint16_t)(TileHalfStepsB + pair + ((rd8(zTemp00) & 4) ? 0 : 1))));
}

/*
 * @routine 050F:7A9 TileNudgeA
 * @run 6B:97A9
 * WHAT IT DOES: The tiles $86 and $8B: the point's row one lower.
 * HOW: An increment.
 * Entry: zTemp02 row. Exit: as TileStepC, Y, C and V kept.
 */
void TileNudgeA(void)
{
    add_to_row(1);
}

/*
 * @routine 050F:7AE TileStepD
 * @run 6B:97AE
 * WHAT IT DOES: A slope tile: moves the point's row by a table on its column.
 * HOW: TileStepsD by column, added to the row.
 * Entry and exit: as TileStepC.
 */
void TileStepD(void)
{
    step_by_column(TileStepsD);
}

/*
 * @routine 050F:7BB TileRowC
 * @run 6B:97BB
 * WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
 * HOW: TileRowsC, eight rows of eight.
 * Entry and exit: as TileStepC.
 */
void TileRowC(void)
{
    row_from_grid(TileRowsC);
}

/*
 * @routine 050F:7CC TileRowD
 * @run 6B:97CC
 * WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
 * HOW: TileRowsD, eight rows of eight.
 * Entry and exit: as TileStepC.
 */
void TileRowD(void)
{
    row_from_grid(TileRowsD);
}

/*
 * @routine 0510:0FD TileStepTwice
 * @run 6B:98FD
 * WHAT IT DOES: A slope tile: moves the row by the offset found through the column's offset.
 * HOW: One past the column's byte of TileRowOffsets indexes that table again.
 * Entry and exit: as TileStepC.
 */
void TileStepTwice(void)
{
    uint8_t index = (uint8_t)(row_offset_for_column() + 1);

    add_to_row((uint8_t)(rd8((uint16_t)(TileRowOffsets + index)) + 1));
}

/*
 * @routine 0510:106 TileRowOffset
 * @run 6B:9906
 * WHAT IT DOES: A slope tile: a new row from TileRowsE, with the column's offset for its column.
 * HOW: Row times eight, plus the offset and one, indexes the table.
 * Entry and exit: as TileStepC.
 */
void TileRowOffset(void)
{
    uint8_t index = (uint8_t)((rd8(zTemp02) << 3) + row_offset_for_column() + 1);

    wr8(zTemp02, rd8((uint16_t)(TileRowsE + index)));
}

/*
 * @routine 0510:114 TileStepOffset
 * @run 6B:9914
 * WHAT IT DOES: A slope tile: moves the row by one more than the column's offset.
 * HOW: TileRowOffsets by column, added to the row with one.
 * Entry and exit: as TileStepC.
 */
void TileStepOffset(void)
{
    add_to_row((uint8_t)(row_offset_for_column() + 1));
}

/*
 * @routine 0510:11F TileRowE
 * @run 6B:991F
 * WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
 * HOW: TileRowsE, eight rows of eight.
 * Entry and exit: as TileStepC.
 */
void TileRowE(void)
{
    row_from_grid(TileRowsE);
}

/*
 * @routine 0510:170 TileNudgeB
 * @run 6B:9970
 * WHAT IT DOES: The tile $94: the point's row one lower.
 * HOW: An increment.
 * Entry and exit: as TileNudgeA.
 */
void TileNudgeB(void)
{
    add_to_row(1);
}

/*
 * @routine 0510:175 TileLift7
 * @run 6B:9975
 * WHAT IT DOES: The tile $95: the point's row seven higher.
 * HOW: A subtract.
 * Entry: zTemp02 row. Exit: as TileStepC, Y kept.
 */
void TileLift7(void)
{
    add_to_row((uint8_t)(0u - 7u));
}

/*
 * @routine 0510:17F TileLift6
 * @run 6B:997F
 * WHAT IT DOES: The tile $96: the point's row six higher.
 * HOW: A subtract.
 * Entry and exit: as TileLift7.
 */
void TileLift6(void)
{
    add_to_row((uint8_t)(0u - 6u));
}

/*
 * @routine 0510:189 TileLift5
 * @run 6B:9989
 * WHAT IT DOES: The tile $97: the point's row five higher.
 * HOW: A subtract.
 * Entry and exit: as TileLift7.
 */
void TileLift5(void)
{
    add_to_row((uint8_t)(0u - 5u));
}

/*
 * @routine 0510:193 TileLift4
 * @run 6B:9993
 * WHAT IT DOES: The tile $98: the point's row four higher.
 * HOW: A subtract.
 * Entry and exit: as TileLift7.
 */
void TileLift4(void)
{
    add_to_row((uint8_t)(0u - 4u));
}

/*
 * @routine 0510:19D TileLift3
 * @run 6B:999D
 * WHAT IT DOES: The tile $99: the point's row three higher.
 * HOW: A subtract.
 * Entry and exit: as TileLift7.
 */
void TileLift3(void)
{
    add_to_row((uint8_t)(0u - 3u));
}

/*
 * @routine 0510:1A7 TileLift2
 * @run 6B:99A7
 * WHAT IT DOES: The tile $9A: the point's row two higher.
 * HOW: A subtract.
 * Entry and exit: as TileLift7.
 */
void TileLift2(void)
{
    add_to_row((uint8_t)(0u - 2u));
}

/*
 * @routine 0510:1B1 TileLift1
 * @run 6B:99B1
 * WHAT IT DOES: The tile $9B: the point's row one higher.
 * HOW: A subtract.
 * Entry and exit: as TileLift7.
 */
void TileLift1(void)
{
    add_to_row((uint8_t)(0u - 1u));
}

/* A, Y, C and V a handler leaves. The row and column are those it started with. */
static void tile_add_shim(uint8_t before, uint8_t amount)
{
    abi_flag(P_C, before + amount > 0xFF);
    abi_flag(P_V, (~(before ^ amount) & (before ^ (uint8_t)(before + amount)) & 0x80) != 0);
}

static void tile_handler_exit_shim(uint8_t tile, uint8_t row, uint8_t column)
{
    uint8_t offset = rd8((uint16_t)(TileRowOffsets + column));
    uint8_t scaled = (uint8_t)(row << 3);
    uint8_t half = (column & 4) ? 1 : 0;

    cpu.a = TILE_HANDLED_RESULT;
    abi_nz(TILE_HANDLED_RESULT);
    switch (tile) {
    case 0x80:
    case 0x83:
        cpu.y = column;
        tile_add_shim(row, rd8((uint16_t)((tile == 0x80 ? TileStepsC : TileStepsD) + column)));
        break;
    case 0x81: case 0x82: case 0x84: case 0x85: case 0x90:
        cpu.y = (uint8_t)(scaled + column);
        tile_add_shim(scaled, column);
        break;
    case 0x87: case 0x88: case 0x89: case 0x8A:
        cpu.y = (uint8_t)(((tile - 0x83) << 1) + half);
        tile_add_shim(row, rd8((uint16_t)(TileHalfStepsA + cpu.y)));
        break;
    case 0x8C: case 0x8D: case 0x8E: case 0x8F:
        cpu.y = (uint8_t)(((tile - 0x8C) << 1) + (half ^ 1));
        tile_add_shim(row, rd8((uint16_t)(TileHalfStepsB + cpu.y)));
        break;
    case 0x91:
        cpu.y = column;
        tile_add_shim(row, offset);
        break;
    case 0x92:
        cpu.y = (uint8_t)(scaled + offset + 1);
        tile_add_shim(scaled, offset);
        break;
    case 0x93:
        cpu.y = (uint8_t)(offset + 1);
        tile_add_shim(row, rd8((uint16_t)(TileRowOffsets + cpu.y)));
        break;
    case 0x95: case 0x96: case 0x97: case 0x98: case 0x99: case 0x9A: case 0x9B: {
        uint8_t lift = (uint8_t)(TILE_END_HANDLED - tile);
        uint8_t after = (uint8_t)(row - lift);

        abi_flag(P_C, row >= lift);
        abi_flag(P_V, ((row ^ lift) & (row ^ after) & 0x80) != 0);
        break;
    }
    default:
        break;
    }
}

/* Each wrapper: the tile is in A, and the handler leaves the exit above. */
static void handler_exit_shim(uint8_t row, uint8_t column)
{
    tile_handler_exit_shim(cpu.a, row, column);
}

void abi_TileStepC(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileStepC();
    handler_exit_shim(row, column);
}

void abi_TileRowA(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileRowA();
    handler_exit_shim(row, column);
}

void abi_TileRowB(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileRowB();
    handler_exit_shim(row, column);
}

void abi_TileHalfStepA(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileHalfStepA(cpu.a);
    handler_exit_shim(row, column);
}

void abi_TileHalfStepB(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileHalfStepB(cpu.a);
    handler_exit_shim(row, column);
}

void abi_TileNudgeA(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileNudgeA();
    handler_exit_shim(row, column);
}

void abi_TileStepD(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileStepD();
    handler_exit_shim(row, column);
}

void abi_TileRowC(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileRowC();
    handler_exit_shim(row, column);
}

void abi_TileRowD(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileRowD();
    handler_exit_shim(row, column);
}

void abi_TileStepTwice(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileStepTwice();
    handler_exit_shim(row, column);
}

void abi_TileRowOffset(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileRowOffset();
    handler_exit_shim(row, column);
}

void abi_TileStepOffset(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileStepOffset();
    handler_exit_shim(row, column);
}

void abi_TileRowE(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileRowE();
    handler_exit_shim(row, column);
}

void abi_TileNudgeB(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileNudgeB();
    handler_exit_shim(row, column);
}

void abi_TileLift7(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileLift7();
    handler_exit_shim(row, column);
}

void abi_TileLift6(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileLift6();
    handler_exit_shim(row, column);
}

void abi_TileLift5(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileLift5();
    handler_exit_shim(row, column);
}

void abi_TileLift4(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileLift4();
    handler_exit_shim(row, column);
}

void abi_TileLift3(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileLift3();
    handler_exit_shim(row, column);
}

void abi_TileLift2(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileLift2();
    handler_exit_shim(row, column);
}

void abi_TileLift1(void)
{
    uint8_t row = rd8(zTemp02), column = rd8(zTemp00);

    TileLift1();
    handler_exit_shim(row, column);
}

static void run_tile_handler(uint8_t tile)
{
    switch (tile) {
    case 0x80: TileStepC(); break;
    case 0x81: TileRowA(); break;
    case 0x82: TileRowB(); break;
    case 0x83: TileStepD(); break;
    case 0x84: TileRowC(); break;
    case 0x85: TileRowD(); break;
    case 0x86: case 0x8B: TileNudgeA(); break;
    case 0x87: case 0x88: case 0x89: case 0x8A: TileHalfStepA(tile); break;
    case 0x8C: case 0x8D: case 0x8E: case 0x8F: TileHalfStepB(tile); break;
    case 0x90: TileRowE(); break;
    case 0x91: TileStepOffset(); break;
    case 0x92: TileRowOffset(); break;
    case 0x93: TileStepTwice(); break;
    case 0x94: TileNudgeB(); break;
    case 0x95: TileLift7(); break;
    case 0x96: TileLift6(); break;
    case 0x97: TileLift5(); break;
    case 0x98: TileLift4(); break;
    case 0x99: TileLift3(); break;
    case 0x9A: TileLift2(); break;
    default: TileLift1(); break;
    }
}

/*
 * @routine 050F:67F TerrainTileRead
 * @run 6B:967F
 * WHAT IT DOES: Says what the map holds at a point: the attribute of the background tile
 * there, 0 if its page has none, $81 for a slope or step, which moves the row in the tile.
 * HOW: The point plus the scroll gives the tile's place in the 64 by 128 background; its
 * entry is read from VRAM. The entry's low byte and a page by its high bits give the
 * attribute. Attributes $80-$9B go through a table of handlers.
 * Entry: zTemp00-01 X, zTemp02-03 Y. Exit: A the answer, zTemp00 the column, zTemp02 the row.
 * X kept. Clobbers zTemp04-09.
 */
TileRead TerrainTileRead(void)
{
    uint16_t x = (uint16_t)(read_word(zTemp00, zTemp01) + read_word(ScrollXLo, ScrollXHi));
    uint16_t y = (uint16_t)(read_word(zTemp02, zTemp03) + read_word(ScrollYLo, ScrollYHi));
    uint8_t across = (uint8_t)((x & 0x1FF) >> 3);
    uint8_t low_row = (uint8_t)y;
    uint8_t address_lo = (uint8_t)(((low_row << 3) & 0xC0) + (across & 0x3F));
    uint8_t address_hi = (uint8_t)((((y >> 8) & 1) << 3) | (low_row >> 5));
    TileRead read = { TILE_PLAIN, 0, 0, 0, (uint8_t)(x & 7), (uint8_t)(low_row & 7) };
    uint8_t entry_lo;

    wr8(zTemp04, (uint8_t)x);
    wr8(zTemp00, read.column);
    wr8(zTemp02, read.row);
    wr8(zTemp05, address_lo);
    wr8(zTemp06, address_lo);
    wr8(zTemp07, address_hi);
    SelectVramReadAddress();
    wr8(VDC_DATA_LO, address_lo);
    wr8(VDC_DATA_HI, address_hi);
    SelectVramData();
    entry_lo = rd8(VDC_DATA_LO);
    read.high = rd8(VDC_DATA_HI) & 7;
    wr8(zTemp08, entry_lo);
    wr8(zTemp09, read.high);
    read.page = rd8((uint16_t)(SaveBuffer + read.high));
    if (read.page & 0x80) {
        read.kind = TILE_EMPTY_PAGE;
        return read;
    }
    wr8(zTemp09, (uint8_t)(read.page + TILE_PAGE_BASE));
    read.attribute = rd8((uint16_t)(entry_lo | ((read.page + TILE_PAGE_BASE) << 8)));
    if (read.attribute < TILE_FIRST_HANDLED)
        return read;
    if (read.attribute >= TILE_END_HANDLED) {
        read.kind = TILE_PAST_HANDLED;
        return read;
    }
    wr8(zTemp04, rd8((uint16_t)(TileHandlers + ((read.attribute - TILE_FIRST_HANDLED) << 1))));
    wr8(zTemp05, rd8((uint16_t)(TileHandlers + 1 + ((read.attribute - TILE_FIRST_HANDLED) << 1))));
    run_tile_handler(read.attribute);
    read.kind = TILE_HANDLED;
    return read;
}

/* A, Y, C and V as a read leaves them. The page's add sets V, and an empty page's is clear. */
void terrain_read_exit_shim(TileRead read)
{
    bool page_overflow = read.page >= 3;

    if (read.kind == TILE_BY_ORIGINAL)
        return;
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
        tile_handler_exit_shim(read.attribute, read.row, read.column);
        break;
    }
}

void abi_TerrainTileRead(void)
{
    terrain_read_exit_shim(TerrainTileRead());
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

/*
 * Is an overlay of this family in RAM? The 11 that hold this code at these addresses share these
 * jumps at $8006-$8011, and no other overlay does.
 */
static bool terrain_family_resident(void)
{
    static const uint8_t jumps[] = {
        0x4C, 0x7F, 0x96, 0x4C, 0xD9, 0x92, 0x4C, 0xAB, 0x95, 0x4C, 0x0B, 0x96
    };

    for (unsigned i = 0; i < sizeof jumps; i++)
        if (rd8((uint16_t)(FrameOverlay8006 + i)) != jumps[i])
            return false;
    return true;
}

/* One call to another overlay's copy of the read, which has no C yet. */
static TileRead original_tile_read_shim(void)
{
    TileRead read = { TILE_BY_ORIGINAL, 0, 0, 0, 0, 0 };

    call_original(FrameOverlay8006);
    read.attribute = cpu.a;
    return read;
}

/* The read at $8006 of whichever stage overlay is in RAM. */
TileRead StageTileRead(void)
{
    return terrain_family_resident() ? TerrainTileRead() : original_tile_read_shim();
}

/* Each overlay of the family has its own probe table; its object records' address names it. */
static uint16_t family_probe_table(void)
{
    static const struct { uint16_t records, probes; } family[] = {
        { 0xAC96, 0xC11F }, /* sector $24D */
        { 0xACCB, 0xCBE0 }, /* sector $50D */
        { 0xD0D5, 0xD9DE }, /* sector $7CD */
        { 0xAC8F, 0xB547 }, /* sector $92D */
        { 0xACE4, 0xBD92 }, /* sector $A8D */
        { 0xACA8, 0xD658 }, /* sector $BED */
        { 0xAD92, 0xD771 }, /* sector $D4D */
        { 0xAD40, 0xB23F }, /* sector $EAD */
        { 0xAD1C, 0xD2CF }, /* sector $100D */
        { 0xAD37, 0xD9A2 }, /* sector $116D */
        { 0xAD32, 0xD6C8 }  /* sector $160D */
    };
    uint16_t records = read_word(OverlayObjectRecords, OverlayObjectRecords + 1);

    for (unsigned i = 0; i < sizeof family / sizeof family[0]; i++)
        if (family[i].records == records)
            return family[i].probes;
    return TerrainProbes;
}

static uint16_t probe_columns(uint8_t probe)
{
    uint16_t entry = (uint16_t)(family_probe_table() + (uint8_t)(probe << 1));
    uint16_t columns = (uint16_t)(rd8(entry) | (rd8((uint16_t)(entry + 1)) << 8));

    wr8(zTemp0C, (uint8_t)columns);
    wr8(zTemp0D, (uint8_t)(columns >> 8));
    wr8(zTemp0A, 0);
    return columns;
}

/*
 * @routine 050F:5AB TerrainLookup
 * @run 6B:95AB
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
 * attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
 * HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
 * reads there. Answers go to TileHit, TileDepthX and TileDepthY.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
 * read or add. Clobbers zTemp00-0D.
 */
LookupEnd TerrainLookup(uint8_t probe, uint8_t object)
{
    uint16_t columns = probe_columns(probe);
    LookupEnd end = { false, false, 0, 0, false, { TILE_PLAIN, 0, 0, 0, 0, 0 }, 0 };

    for (uint8_t column = 0; column < PROBE_COLUMNS; column++) {
        uint16_t at = (uint16_t)(columns + column * PROBE_COLUMN_BYTES);
        uint8_t offset_low = rd8(at);
        uint8_t offset_high = rd8((uint16_t)(at + 1));
        uint8_t attribute = 0;

        end.offset_low = offset_low;
        end.object_low = rd8((uint16_t)(OBJ_X_LO + object));
        end.last_skipped = offset_high == PROBE_NO_COLUMN;
        if (end.last_skipped) {
            wr8(zTemp00, 0);
            wr8(zTemp02, 0);
        } else {
            uint16_t x = (uint16_t)(end.object_low | (rd8((uint16_t)(OBJ_X_HI + object)) << 8));
            uint16_t y = (uint16_t)(rd8((uint16_t)(OBJ_Y_LO + object)) |
                                    (rd8((uint16_t)(OBJ_Y_HI + object)) << 8));

            x = (uint16_t)(x + (offset_low | (offset_high << 8)));
            y = (uint16_t)(y + (rd8((uint16_t)(at + 2)) | (rd8((uint16_t)(at + 3)) << 8)));
            wr8(zTemp00, (uint8_t)x);
            wr8(zTemp01, (uint8_t)(x >> 8));
            wr8(zTemp02, (uint8_t)y);
            wr8(zTemp03, (uint8_t)(y >> 8));
            end.last = TerrainTileRead();
            end.read_any = true;
            attribute = end.last.kind == TILE_EMPTY_PAGE ? 0 :
                        end.last.kind == TILE_HANDLED ? TILE_HANDLED_RESULT : end.last.attribute;
        }
        wr8((uint16_t)(TileHit + column), attribute);
        wr8((uint16_t)(TileDepthX + column), rd8(zTemp00));
        wr8((uint16_t)(TileDepthY + column), rd8(zTemp02));
        wr8(zTemp0A, (uint8_t)(column + 1));
    }
    return end;
}

/* Y 16 and A 4 with C set and Z set from the compare that ended the loop; V by the last column. */
void lookup_exit_shim(LookupEnd end)
{
    if (end.by_original)
        return;
    if (end.last_skipped) {
        uint8_t sum = (uint8_t)(end.offset_low + end.object_low);

        abi_flag(P_V, (~(end.offset_low ^ end.object_low) & (end.offset_low ^ sum) & 0x80) != 0);
    } else if (end.read_any) {
        terrain_read_exit_shim(end.last);
    }
    cpu.a = PROBE_COLUMNS;
    cpu.y = PROBE_COLUMNS * PROBE_COLUMN_BYTES;
    abi_nz(0);
    abi_flag(P_C, true);
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
 * Entry and exit: as TerrainLookup.
 */
LookupEnd TerrainLookupAlt(uint8_t probe, uint8_t object)
{
    LookupEnd end = TerrainLookup(probe, object);
    uint16_t columns = probe_columns(probe);

    for (uint8_t column = 0; column < PROBE_COLUMNS; column++) {
        uint16_t at = (uint16_t)(columns + column * PROBE_COLUMN_BYTES);
        uint16_t hit_field = (uint16_t)(TileHit + column);
        uint8_t hit = rd8(hit_field) & HIT_NEEDS_SECOND_LOOK;

        wr8(zTemp0A, column);
        if (hit != 0 && !(hit & HIT_SECOND_LOOK_SKIP)) {
            uint16_t x = (uint16_t)(rd8((uint16_t)(OBJ_X_LO + object)) |
                                    (rd8((uint16_t)(OBJ_X_HI + object)) << 8));
            uint16_t y = (uint16_t)(rd8((uint16_t)(OBJ_Y_LO + object)) |
                                    (rd8((uint16_t)(OBJ_Y_HI + object)) << 8));
            uint8_t raised;
            TileRead read;

            x = (uint16_t)(x + (rd8(at) | (rd8((uint16_t)(at + 1)) << 8)));
            y = (uint16_t)(y + (rd8((uint16_t)(at + 2)) | (rd8((uint16_t)(at + 3)) << 8)));
            raised = (uint8_t)((uint8_t)y + SECOND_LOOK_RAISE);
            raised = (uint8_t)(raised + 0xFF + ((uint8_t)y + SECOND_LOOK_RAISE > 0xFF ? 1 : 0));
            wr8(zTemp00, (uint8_t)x);
            wr8(zTemp01, (uint8_t)(x >> 8));
            wr8(zTemp02, raised);
            wr8(zTemp03, (uint8_t)(y >> 8));
            read = TerrainTileRead();
            end.last = read;
            end.read_any = true;
            end.last_skipped = false;
            if ((read.kind == TILE_HANDLED ? TILE_HANDLED_RESULT :
                 read.kind == TILE_EMPTY_PAGE ? 0 : read.attribute) & SECOND_LOOK_SOLID)
                wr8(hit_field, rd8(hit_field) & HIT_CLEARED_BY_SECOND_LOOK);
        }
    }
    wr8(zTemp0A, PROBE_COLUMNS);
    return end;
}

void abi_TerrainLookupAlt(void)
{
    lookup_exit_shim(TerrainLookupAlt(cpu.a, cpu.x));
}

/* One call to another overlay's copy of a lookup, which has no C yet. */
static LookupEnd original_lookup_shim(uint16_t vector, uint8_t probe, uint8_t object, uint8_t column)
{
    LookupEnd end = { true };

    cpu.a = probe;
    abi_nz(probe);
    cpu.x = object;
    cpu.y = column;
    call_original(vector);
    end.column_after = cpu.y;
    return end;
}

/*
 * The lookups at $800C and $800F of whichever stage overlay is in RAM. Y on entry is a column
 * only to another overlay's code, and all of this copy's leave it past their last column.
 */
LookupEnd StageLookup(uint8_t probe, uint8_t object, uint8_t column)
{
    LookupEnd end;

    if (!terrain_family_resident())
        return original_lookup_shim(OverlayTerrainLookup, probe, object, column);
    end = TerrainLookup(probe, object);
    end.column_after = PROBE_COLUMNS * PROBE_COLUMN_BYTES;
    return end;
}

LookupEnd StageLookupAlt(uint8_t probe, uint8_t object, uint8_t column)
{
    LookupEnd end;

    if (!terrain_family_resident())
        return original_lookup_shim(OverlayTerrainLookupAlt, probe, object, column);
    end = TerrainLookupAlt(probe, object);
    end.column_after = PROBE_COLUMNS * PROBE_COLUMN_BYTES;
    return end;
}
