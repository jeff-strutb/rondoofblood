/*
 * Richter's questions about the ground, from his overlay at $A000: reads of the terrain at
 * points around him, and the wall probe the camera asks before it moves him. The points are
 * offsets from his position, by column and by which way he faces.
 *
 * zTemp00-03 hold the point, and zTemp10 the reach; these are RAM the routines leave behind.
 * Only the wrappers and exit shims know registers and flags.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

enum {
    WALL_ANSWER = 0x02, WALL_COLUMNS = 6, WALL_FIRST_COLUMN_STANDING = 0,
    WALL_FIRST_COLUMN_CROUCHING = 3, WALL_FIRST_COLUMN_IN_AIR = 5,
    STATE_CROUCHING = 2, STATE_IN_AIR = 3, STATE_FALLING = 4,
    RICHTER_COLUMNS = 5, COLUMNS_BELOW_WAIST = 0x0B, COLUMNS_BELOW_FEET = 0x16,
    BLOCKED_COLUMNS = 2, TILE_BLOCKS = 0x24, FLOOR_COLUMNS = 3, TILE_FLOOR = 0x81,
    PUSH_ON_FLOOR = 0x01, PUSH_LIFTING = 0x20, CHARACTER_RICHTER = 0, COLUMNS_BELOW_LOW = 0x12, COLUMNS_BELOW_CHECK = 0x0E,
    FLOOR_TOP_OF_MAP = 0x02, ROW_ABOVE_THE_MAP = 0xFF, COLUMNS_BELOW_SOLES = 0x1E, PAD_UP = 0x40, PAD_DOWN = 0x10,
    PAD_JUMP_PRESS = 0x01, CAMERA_NO_LADDER = 0x01, CAMERA_LADDER = 0x18, LADDER_LOWEST_Y = 0xB0,
    LADDER_FIRST_REACH = 0x18, LADDER_STEP = 8, TILE_SOLID_BIT = 0x01, TILE_LADDER_BITS = 0x05,
    TILE_BLOCK_ANY = 0x20
};

/* The point's X and Y high byte are set, or zero: a read of the point or no answer. */
static TileRead read_point_or_nothing(bool off_the_map)
{
    TileRead nothing = { TILE_EMPTY_PAGE, 0, 0, 0, 0, 0 };

    return off_the_map ? nothing : StageTileRead();
}

/*
 * @routine 01BB:6D2 RichterReadAbove
 * @run 6D:B6D2
 * WHAT IT DOES: Reads the terrain at a row that may lie above the map: a row off the top
 * reads as nothing.
 * HOW: The row's add, with its carry, decides the point's Y high byte; only zero is read.
 * Entry: A row, C its add's carry: set while the row stays on the map. zTemp00 X. Exit: as the
 * read, or A 0 with N and Z from the add.
 */
TileRead RichterReadAbove(uint8_t row, bool on_the_map)
{
    uint8_t high = (uint8_t)(0xFF + (on_the_map ? 1 : 0));

    wr8(zTemp02, row);
    wr8(zTemp03, high);
    if (high != 0)
        return read_point_or_nothing(true);
    wr8(zTemp01, 0);
    return read_point_or_nothing(false);
}

/*
 * When nothing was read, A is 0 by CLA, which leaves N and Z as they stood on the HuC6280: A5
 * shows the original's P 01 from an entry of 01 where setting Z gave 03. Otherwise as the read.
 */
static void read_or_nothing_exit_shim(TileRead read, bool read_made)
{
    if (read_made)
        terrain_read_exit_shim(read);
    else
        cpu.a = 0;
}

void read_above_exit_shim(TileRead read, bool on_the_map)
{
    abi_flag(P_V, false);
    abi_flag(P_C, on_the_map);
    if (!on_the_map)
        abi_nz(ROW_ABOVE_THE_MAP);
    read_or_nothing_exit_shim(read, on_the_map);
}

void abi_RichterReadAbove(void)
{
    bool on_the_map = (cpu.p & P_C) != 0;

    read_above_exit_shim(RichterReadAbove(cpu.a, on_the_map), on_the_map);
}

/*
 * @routine 01BB:6DD RichterReadBelow
 * @run 6D:B6DD
 * WHAT IT DOES: Reads the terrain at a row on the first screen down: a row past it reads as
 * nothing.
 * HOW: The carry from the row's add says it passed; otherwise X and Y high bytes are zero.
 * Entry: A row, C its add's carry, zTemp00 X. Exit: as the read, or A 0 with N, Z, C and V kept.
 */
TileRead RichterReadBelow(uint8_t row, bool carry)
{
    wr8(zTemp02, row);
    if (carry)
        return read_point_or_nothing(true);
    wr8(zTemp03, 0);
    wr8(zTemp01, 0);
    return read_point_or_nothing(false);
}

void abi_RichterReadBelow(void)
{
    bool carry = (cpu.p & P_C) != 0;

    read_or_nothing_exit_shim(RichterReadBelow(cpu.a, carry), !carry);
}

/*
 * @routine 01BB:6EA RichterReadOnScreen
 * @run 6D:B6EA
 * WHAT IT DOES: Reads the terrain at a row, the Y high byte taken from Richter's own.
 * HOW: His Y high byte plus the carry from the row's add.
 * Entry: A row, C its add's carry, zTemp00 X. Exit: as the read.
 */
TileRead RichterReadOnScreen(uint8_t row, bool carry)
{
    wr8(zTemp02, row);
    wr8(zTemp03, (uint8_t)(rd8(OBJ_Y_HI) + (carry ? 1 : 0)));
    wr8(zTemp01, 0);
    return read_point_or_nothing(false);
}

void abi_RichterReadOnScreen(void)
{
    terrain_read_exit_shim(RichterReadOnScreen(cpu.a, (cpu.p & P_C) != 0));
}

/*
 * @routine 01BB:6F4 RichterReadIfOnPage
 * @run 6D:B6F4
 * WHAT IT DOES: Reads the terrain at the point in zTemp00-03, unless the Y high byte just
 * added is not zero.
 * HOW: Tests what the caller's add left.
 * Entry: Z from that byte, zTemp00-03 the point. Exit: as the read, or A 0 with N and Z kept.
 */
TileRead RichterReadIfOnPage(bool on_page)
{
    return read_point_or_nothing(!on_page);
}

void abi_RichterReadIfOnPage(void)
{
    bool on_page = (cpu.p & P_Z) != 0;

    read_or_nothing_exit_shim(RichterReadIfOnPage(on_page), on_page);
}

/*
 * @routine 01BB:6F9 RichterColumnOffset
 * @run 6D:B6F9
 * WHAT IT DOES: Gives how far across a probe's column lies, by column and facing.
 * HOW: A table of pairs, one for each way he faces.
 * Entry: X column. Exit: A the offset, N and Z from it, Y its index, C and V from the index's
 * add. X kept.
 */
uint8_t RichterColumnOffset(uint8_t column)
{
    return rd8((uint16_t)(RichterColumnOffsets + (uint8_t)((column << 1) + rd8(OBJ_2AC0))));
}

void abi_RichterColumnOffset(void)
{
    uint8_t doubled = (uint8_t)(cpu.x << 1);
    uint8_t facing = rd8(OBJ_2AC0);
    uint8_t index = (uint8_t)(doubled + facing);

    cpu.a = RichterColumnOffset(cpu.x);
    abi_nz(cpu.a);
    cpu.y = index;
    abi_flag(P_C, doubled + facing > 0xFF);
    abi_flag(P_V, (~(doubled ^ facing) & (doubled ^ index) & 0x80) != 0);
}

/*
 * @routine 01BC:36E RichterWall
 * @run 6D:BB6E
 * WHAT IT DOES: Asks whether Richter runs into a wall on one side, and how deep: what pushes
 * him that way counts as a wall, and otherwise up to six points down his side are read,
 * fewer when he crouches or is in the air.
 * HOW: The reach by side, then each column's height added to his position.
 * Entry: Y side. Exit: A 2 with N 0, Z 0 at a wall, zTemp00 how deep; else Z set. Other
 * registers and flags as the last step left them.
 */
WallProbe RichterWall(uint8_t side)
{
    WallProbe probe = { WALL_BARRED };
    uint8_t state;
    uint8_t reach;

    if (rd8(PlayerMayLeaveTop) == 1)
        return probe;
    if (rd8(zPlayerPush) & rd8((uint16_t)(RichterWallPushBits + side))) {
        wr8(zTemp00, rd8(zPlayerPushDepth));
        probe.how = WALL_PUSHED;
        return probe;
    }
    reach = rd8((uint16_t)(RichterWallReach + side));
    wr8(zTemp10, reach);
    state = rd8(OBJ_27A8);
    probe.column = state == STATE_IN_AIR || state == STATE_FALLING ? WALL_FIRST_COLUMN_IN_AIR :
                   state == STATE_CROUCHING ? WALL_FIRST_COLUMN_CROUCHING : WALL_FIRST_COLUMN_STANDING;
    for (; probe.column < WALL_COLUMNS; probe.column++) {
        uint16_t y = (uint16_t)(rd8(OBJ_Y_LO) | (rd8(OBJ_Y_HI) << 8));

        probe.last_row = y;
        probe.last_lift = (uint16_t)(rd8((uint16_t)(RichterWallLift + probe.column)) |
                                     (rd8((uint16_t)(RichterWallLiftHigh + probe.column)) << 8));
        y = (uint16_t)(y + probe.last_lift);
        wr8(zTemp00, (uint8_t)(rd8(OBJ_X_LO) + reach));
        wr8(zTemp01, 0);
        wr8(zTemp02, (uint8_t)y);
        wr8(zTemp03, (uint8_t)(y >> 8));
        probe.read_made = (y >> 8) == 0;
        if (probe.read_made) {
            probe.last_read = RichterReadIfOnPage(true);
            probe.any_read = true;
            if (tile_answer(probe.last_read) & WALL_ANSWER) {
                probe.how = WALL_FOUND;
                return probe;
            }
        }
    }
    probe.how = WALL_NONE;
    return probe;
}

/* C and V from a column's height add, for a column off the page. */
static void wall_lift_flags_shim(WallProbe probe)
{
    uint16_t sum = (uint16_t)(probe.last_row + probe.last_lift);

    abi_flag(P_C, sum < probe.last_row);
    abi_flag(P_V, (~(probe.last_row ^ probe.last_lift) & (probe.last_row ^ sum) & 0x8000) != 0);
}

void wall_exit_shim(WallProbe probe)
{
    switch (probe.how) {
    case WALL_BY_ORIGINAL:
        break;
    case WALL_BARRED:
        cpu.a = 1;
        abi_nz(0);
        abi_flag(P_C, true);
        break;
    case WALL_PUSHED:
        cpu.a = WALL_ANSWER;
        abi_nz(WALL_ANSWER);
        abi_flag(P_C, rd8(PlayerMayLeaveTop) >= 1);
        break;
    case WALL_FOUND:
        terrain_read_exit_shim(probe.last_read);
        cpu.a = WALL_ANSWER;
        abi_nz(WALL_ANSWER);
        cpu.x = probe.column;
        break;
    case WALL_NONE:
        if (probe.any_read)
            terrain_read_exit_shim(probe.last_read);
        if (!probe.read_made)
            wall_lift_flags_shim(probe);
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, true);
        cpu.x = WALL_COLUMNS;
        break;
    }
}

void abi_RichterWall(void)
{
    wall_exit_shim(RichterWall(cpu.y));
}

/* A column's point: its offset across from Richter, and a row below him. Reads it. */
static TileRead read_column(uint8_t column, uint8_t below)
{
    uint8_t row = rd8(OBJ_Y_LO);

    wr8(zTemp00, (uint8_t)(RichterColumnOffset(column) + rd8(OBJ_X_LO)));
    return RichterReadBelow((uint8_t)(row + below), row + below > 0xFF);
}

static ColumnsEnd read_five_columns(uint8_t below)
{
    ColumnsEnd end = { false, false, { TILE_EMPTY_PAGE, 0, 0, 0, 0, 0 }, 0, below };

    wr8(zTemp10, below);
    for (uint8_t column = 0; column < RICHTER_COLUMNS; column++) {
        end.row = rd8(OBJ_Y_LO);
        end.read_made = end.row + below <= 0xFF;
        end.last = read_column(column, below);
        wr8((uint16_t)(RichterColumnHit + column), end.read_made ? tile_answer(end.last) : 0);
        wr8((uint16_t)(RichterColumnRow + column), rd8(zTemp02));
        wr8((uint16_t)(RichterColumnX + column), rd8(zTemp00));
    }
    return end;
}

/* X 5, Z set and C set from the loop's compare; A the last column's X. */
void columns_exit_shim(ColumnsEnd end)
{
    if (end.by_original)
        return;
    if (end.read_made) {
        terrain_read_exit_shim(end.last);
    } else {
        uint8_t sum = (uint8_t)(end.row + end.below);

        cpu.y = (uint8_t)(((RICHTER_COLUMNS - 1) << 1) + rd8(OBJ_2AC0));
        abi_flag(P_V, (~(end.row ^ end.below) & (end.row ^ sum) & 0x80) != 0);
    }
    cpu.a = rd8(zTemp00);
    cpu.x = RICHTER_COLUMNS;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 01BC:31D RichterReadColumnsHigh
 * @run 6D:BB1D
 * WHAT IT DOES: Reads the five columns of terrain at his waist.
 * HOW: As RichterReadColumns, $0B below his position.
 * Entry and exit: as RichterReadColumns.
 */
ColumnsEnd RichterReadColumnsHigh(void)
{
    return read_five_columns(COLUMNS_BELOW_WAIST);
}

void abi_RichterReadColumnsHigh(void)
{
    columns_exit_shim(RichterReadColumnsHigh());
}

/*
 * @routine 01BC:323 RichterReadColumns
 * @run 6D:BB23
 * WHAT IT DOES: Reads the five columns of terrain under Richter, for what the floor checks
 * look at: the step the overlay makes after he has moved.
 * HOW: Each column's answer, row and X go to RichterColumnHit, RichterColumnRow and
 * RichterColumnX.
 * Entry: nothing. Exit: A the last column's X, X 5, N 0, Z 1, C 1, Y and V from the last read,
 * or its row's add if that passed the screen. Clobbers zTemp00-03, zTemp10.
 */
ColumnsEnd RichterReadColumns(void)
{
    return read_five_columns(COLUMNS_BELOW_FEET);
}

void abi_RichterReadColumns(void)
{
    columns_exit_shim(RichterReadColumns());
}

/*
 * @routine 01BC:2AD RichterColumnsBlocked
 * @run 6D:BAAD
 * WHAT IT DOES: Says whether either of the first two columns read holds a blocking tile.
 * HOW: Bits $24 of RichterColumnHit.
 * Entry: nothing. Exit: at one, A those bits, X the column; else A 0, X 2, C 1, Z 1. V and,
 * at one, C kept. Y kept.
 */
int RichterColumnsBlocked(void)
{
    for (int column = 0; column < BLOCKED_COLUMNS; column++)
        if (rd8((uint16_t)(RichterColumnHit + column)) & TILE_BLOCKS)
            return column;
    return -1;
}

void abi_RichterColumnsBlocked(void)
{
    int column = RichterColumnsBlocked();

    if (column >= 0) {
        cpu.x = (uint8_t)column;
        cpu.a = rd8((uint16_t)(RichterColumnHit + column)) & TILE_BLOCKS;
        abi_nz(cpu.a);
        abi_flag(P_C, column > 0 ? false : (cpu.p & P_C) != 0);
    } else {
        cpu.x = BLOCKED_COLUMNS;
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, true);
    }
}

/* A floor was found: the push's flags come off, unless it is carrying him up. */
static void floor_clears_push(void)
{
    wr8(zPlayerPush, rd8(zPlayerPush) & (uint8_t)~PUSH_ON_FLOOR);
    if ((rd8(zFieldCA) & 0x80) || !(rd8(zPushYHi) & 0x80))
        wr8(zPlayerPush, rd8(zPlayerPush) & (uint8_t)~PUSH_LIFTING);
}

/*
 * @routine 01BC:2C2 RichterFloorCheck
 * @run 6D:BAC2
 * WHAT IT DOES: Says whether the columns read have a floor under Richter, and at which: on
 * odd screens across, always at the first, with its depth cleared. A push that was already
 * carrying him counts as floor too.
 * HOW: Bits $81 of the first three columns, but bit 7 alone counts only for the first.
 * Entry: nothing. Exit: C the answer, X the column. A, N, Z and V as the path left them.
 */
FloorResult RichterFloorCheck(void)
{
    FloorResult result = { FLOOR_NONE, 0, rd8(zPlayerPush) };

    if (rd8(OBJ_X_HI) & 1) {
        wr8(RichterColumnRow, 0);
        result.how = FLOOR_HIGH_ON_ODD_SCREEN;
        floor_clears_push();
        return result;
    }
    for (uint8_t column = 0; column < FLOOR_COLUMNS; column++) {
        uint8_t hit = rd8((uint16_t)(RichterColumnHit + column)) & TILE_FLOOR;

        if (hit != 0 && (!(hit & 0x80) || column == 0)) {
            result.how = FLOOR_FOUND;
            result.column = column;
            floor_clears_push();
            return result;
        }
    }
    if (result.push_before & PUSH_ON_FLOOR) {
        wr8(RichterColumnRow, rd8(zFieldCA));
        result.how = FLOOR_FROM_PUSH;
        return result;
    }
    wr8(zPlayerPush, result.push_before & (uint8_t)~(PUSH_ON_FLOOR | PUSH_LIFTING));
    return result;
}

static void floor_check_exit_shim(FloorResult result)
{
    uint8_t before = result.push_before;

    if (result.how == FLOOR_NONE) {
        cpu.a = 0;
        cpu.x = FLOOR_COLUMNS;
        abi_nz(0);
        abi_flag(P_N, (before & 0x80) != 0);
        abi_flag(P_V, (before & 0x40) != 0);
        abi_flag(P_C, false);
        return;
    }
    cpu.x = result.column;
    if (result.how == FLOOR_FROM_PUSH) {
        abi_flag(P_V, (before & 0x40) != 0);
        cpu.a = rd8(zFieldCA);
    } else {
        cpu.a = (rd8(zFieldCA) & 0x80) ? rd8(zFieldCA) : rd8(zPushYHi);
    }
    abi_nz(cpu.a);
    abi_flag(P_C, true);
}

void abi_RichterFloorCheck(void)
{
    floor_check_exit_shim(RichterFloorCheck());
}

/* How a read of the two low columns ended. */
typedef struct {
    int blocked_column;
    bool read_made;
    TileRead last;
    uint8_t answer;
    uint8_t row;
} LowColumns;

/*
 * @routine 01BC:2FF RichterLowColumnsBlocked
 * @run 6D:BAFF
 * WHAT IT DOES: Reads the two columns lowest under Richter and says whether either blocks.
 * HOW: Each is read $12 below him, and bits $24 of the answer tell.
 * Entry: nothing. Exit: at one, A those bits, X the column; else A 0, X 2, C 1, Z 1. Y and V
 * from the last read.
 */
LowColumns RichterLowColumnsBlocked(void)
{
    LowColumns low = { -1, false, { TILE_EMPTY_PAGE, 0, 0, 0, 0, 0 }, 0, 0 };

    for (int column = 0; column < BLOCKED_COLUMNS; column++) {
        low.row = rd8(OBJ_Y_LO);
        low.read_made = low.row + COLUMNS_BELOW_LOW <= 0xFF;
        low.last = read_column((uint8_t)column, COLUMNS_BELOW_LOW);
        low.answer = low.read_made ? tile_answer(low.last) : 0;
        if (low.answer & TILE_BLOCKS) {
            low.blocked_column = column;
            return low;
        }
    }
    return low;
}

static void low_columns_exit_shim(LowColumns low)
{
    if (low.read_made) {
        terrain_read_exit_shim(low.last);
    } else {
        uint8_t sum = (uint8_t)(low.row + COLUMNS_BELOW_LOW);

        cpu.y = (uint8_t)(((BLOCKED_COLUMNS - 1) << 1) + rd8(OBJ_2AC0));
        abi_flag(P_V, (~(low.row ^ COLUMNS_BELOW_LOW) & (low.row ^ sum) & 0x80) != 0);
    }
    if (low.blocked_column >= 0) {
        cpu.x = (uint8_t)low.blocked_column;
        cpu.a = low.answer & TILE_BLOCKS;
        abi_nz(cpu.a);
    } else {
        cpu.x = BLOCKED_COLUMNS;
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, true);
    }
}

void abi_RichterLowColumnsBlocked(void)
{
    low_columns_exit_shim(RichterLowColumnsBlocked());
}

/*
 * @routine 01BC:3C1 RichterFloor
 * @run 6D:BBC1
 * WHAT IT DOES: Asks whether Richter stands on a floor, and lifts him out of it: by how deep
 * the column read, but not above the top of the map. A push from below does not count.
 * HOW: RichterReadColumns and RichterFloorCheck, then the column once more $0E below him.
 * Entry: X kept unless a column was found. Exit: A 1 and N 0, Z 0 on a floor, else A 0, Z set.
 * C from the lift, V as the last step left it. Clobbers zTemp00-03, zTemp10.
 */
FloorProbe RichterFloor(void)
{
    FloorProbe probe = { 0 };

    if (rd8(PlayerMayLeaveTop) != 0) {
        probe.how = FLOOR_BARRED;
        return probe;
    }
    probe.columns = RichterReadColumns();
    probe.check = RichterFloorCheck();
    if (probe.check.how == FLOOR_NONE) {
        probe.how = FLOOR_NOTHING_UNDER;
        return probe;
    }
    probe.row = rd8(OBJ_Y_LO);
    probe.read_made = probe.row + COLUMNS_BELOW_CHECK <= 0xFF;
    probe.read = read_column(probe.check.column, COLUMNS_BELOW_CHECK);
    if (probe.read_made) {
        uint8_t answer = tile_answer(probe.read) & TILE_FLOOR;

        if (answer != 0 && !(answer & 0x80)) {
            probe.how = FLOOR_COLUMN_SOLID;
            return probe;
        }
    }
    probe.depth = (uint16_t)(int16_t)(int8_t)rd8((uint16_t)(RichterColumnRow + probe.check.column));
    wr8(zTemp10, (uint8_t)(probe.depth >> 8));
    probe.y = (uint16_t)(rd8(OBJ_Y_LO) | (rd8(OBJ_Y_HI) << 8));
    if ((uint16_t)(probe.y - probe.depth) & 0x8000) {
        wr8(OBJ_Y_LO, FLOOR_TOP_OF_MAP);
        wr8(OBJ_Y_HI, 0);
        probe.how = FLOOR_OFF_THE_TOP;
        return probe;
    }
    wr8(OBJ_Y_LO, (uint8_t)(probe.y - probe.depth));
    wr8(OBJ_Y_HI, (uint8_t)((uint16_t)(probe.y - probe.depth) >> 8));
    probe.how = (rd8(zPlayerPush) & PUSH_LIFTING) ? FLOOR_LANDED_PUSHED : FLOOR_LANDED;
    return probe;
}

void floor_exit_shim(FloorProbe probe)
{
    uint16_t lifted = (uint16_t)(probe.y - probe.depth);

    if (probe.how == FLOOR_BY_ORIGINAL)
        return;
    if (probe.how == FLOOR_BARRED) {
        cpu.a = 0;
        abi_nz(0);
        return;
    }
    columns_exit_shim(probe.columns);
    floor_check_exit_shim(probe.check);
    if (probe.how == FLOOR_NOTHING_UNDER) {
        cpu.a = 0;
        abi_nz(0);
        return;
    }
    if (probe.read_made) {
        terrain_read_exit_shim(probe.read);
    } else {
        uint8_t sum = (uint8_t)(probe.row + COLUMNS_BELOW_CHECK);

        cpu.y = (uint8_t)((probe.check.column << 1) + rd8(OBJ_2AC0));
        abi_flag(P_V, (~(probe.row ^ COLUMNS_BELOW_CHECK) & (probe.row ^ sum) & 0x80) != 0);
    }
    if (probe.how == FLOOR_COLUMN_SOLID) {
        cpu.a = 0;
        abi_nz(0);
        return;
    }
    abi_flag(P_C, probe.y >= probe.depth);
    abi_flag(P_V, ((probe.y ^ probe.depth) & (probe.y ^ lifted) & 0x8000) != 0);
    if (probe.how != FLOOR_OFF_THE_TOP)
        abi_flag(P_V, (rd8(zPlayerPush) & 0x40) != 0);
    cpu.a = probe.how == FLOOR_LANDED_PUSHED ? 0 : 1;
    abi_nz(cpu.a);
}

void abi_RichterFloor(void)
{
    floor_exit_shim(RichterFloor());
}

/* How a read of one column ended, for what the wrapper leaves. */
typedef struct {
    uint8_t column;
    uint8_t row;
    uint8_t below;
    bool read_made;
    TileRead read;
} ColumnRead;

static ColumnRead read_one_column(uint8_t column, uint8_t below)
{
    ColumnRead one = { column, rd8(OBJ_Y_LO), below };

    one.read_made = one.row + below <= 0xFF;
    one.read = read_column(column, below);
    return one;
}

static uint8_t column_answer(ColumnRead one)
{
    return one.read_made ? tile_answer(one.read) : 0;
}

static void column_read_exit_shim(ColumnRead one)
{
    if (one.read_made) {
        terrain_read_exit_shim(one.read);
    } else {
        uint8_t sum = (uint8_t)(one.row + one.below);

        cpu.y = (uint8_t)((one.column << 1) + rd8(OBJ_2AC0));
        abi_flag(P_V, (~(one.row ^ one.below) & (one.row ^ sum) & 0x80) != 0);
    }
}

/*
 * @routine 01BA:22F RichterSolidUnderColumn
 * @run 6D:AA2F
 * WHAT IT DOES: Says whether one column of terrain, $0E below Richter, is solid to stand on:
 * a tile with bit 7 set does not count.
 * HOW: Bit 0 of the answer, unless bit 7 is set.
 * Entry: X column. Exit: A the answer, N and Z from it. X kept; Y, C and V from the read.
 */
ColumnRead RichterSolidUnderColumn(uint8_t column)
{
    return read_one_column(column, COLUMNS_BELOW_CHECK);
}

static uint8_t solid_answer(ColumnRead one)
{
    uint8_t answer = column_answer(one) & TILE_FLOOR;

    return (answer & 0x80) ? 0 : answer;
}

void abi_RichterSolidUnderColumn(void)
{
    ColumnRead one = RichterSolidUnderColumn(cpu.x);

    column_read_exit_shim(one);
    cpu.a = solid_answer(one);
    abi_nz(cpu.a);
}

/*
 * @routine 01BA:248 RichterReadUnderFeet
 * @run 6D:AA48
 * WHAT IT DOES: Reads the terrain straight under his feet, $1E below his position.
 * HOW: His own X with no column's offset.
 * Entry: nothing. Exit: as the read.
 */
ColumnRead RichterReadUnderFeet(void)
{
    ColumnRead one = { 0, rd8(OBJ_Y_LO), COLUMNS_BELOW_SOLES };

    wr8(zTemp00, rd8(OBJ_X_LO));
    one.read_made = one.row + COLUMNS_BELOW_SOLES <= 0xFF;
    one.read = RichterReadBelow((uint8_t)(one.row + COLUMNS_BELOW_SOLES), !one.read_made);
    return one;
}

/* Past the screen the row's add leaves all four flags, and the CLA that follows changes none. */
static void under_feet_exit_shim(ColumnRead one)
{
    if (one.read_made) {
        terrain_read_exit_shim(one.read);
    } else {
        uint8_t sum = (uint8_t)(one.row + one.below);

        abi_flag(P_C, true);
        abi_flag(P_V, (~(one.row ^ one.below) & (one.row ^ sum) & 0x80) != 0);
        abi_nz(sum);
        cpu.a = 0;
    }
}

void abi_RichterReadUnderFeet(void)
{
    under_feet_exit_shim(RichterReadUnderFeet());
}

/*
 * @routine 01BC:068 RichterUpHeld
 * @run 6D:B868
 * WHAT IT DOES: Says whether up is held.
 * HOW: Bit 6 of zPadHeld.
 * Entry: nothing. Exit: C the answer. All else kept.
 */
bool RichterUpHeld(void)
{
    return (rd8(zPadHeld) & PAD_UP) != 0;
}

void abi_RichterUpHeld(void)
{
    abi_flag(P_C, RichterUpHeld());
}

/*
 * @routine 01BC:07E RichterJumpPressed
 * @run 6D:B87E
 * WHAT IT DOES: Says whether the jump button was newly pressed.
 * HOW: Bit 0 of zPadPressed.
 * Entry: nothing. Exit: C the answer. All else kept.
 */
bool RichterJumpPressed(void)
{
    return (rd8(zPadPressed) & PAD_JUMP_PRESS) != 0;
}

void abi_RichterJumpPressed(void)
{
    abi_flag(P_C, RichterJumpPressed());
}

/* How a fall-through check ended. */
typedef struct {
    FloorResult check;
    bool read_made;
    ColumnRead under;
    bool no_floor;
} FootingCheck;

/*
 * @routine 01BC:085 RichterHasNoFooting
 * @run 6D:B885
 * WHAT IT DOES: Says whether Richter has nothing to stand on: no floor found, or the column
 * found is not solid under him.
 * HOW: RichterFloorCheck, then RichterSolidUnderColumn.
 * Entry: nothing. Exit: C the answer; A, X, Y and V as the last step left them.
 */
FootingCheck RichterHasNoFooting(void)
{
    FootingCheck footing = { RichterFloorCheck() };

    if (footing.check.how == FLOOR_NONE) {
        footing.no_floor = true;
        return footing;
    }
    footing.read_made = true;
    footing.under = RichterSolidUnderColumn(footing.check.column);
    footing.no_floor = solid_answer(footing.under) != 0;
    return footing;
}

void abi_RichterHasNoFooting(void)
{
    FootingCheck footing = RichterHasNoFooting();

    floor_check_exit_shim(footing.check);
    if (footing.read_made) {
        column_read_exit_shim(footing.under);
        cpu.a = solid_answer(footing.under);
        abi_nz(cpu.a);
    }
    abi_flag(P_C, footing.no_floor);
}

/* The point this far under him at his own X. Returns whether it is on the first page down. */
static bool set_ladder_point(uint16_t reach)
{
    uint16_t y = (uint16_t)((rd8(OBJ_Y_LO) | (rd8(OBJ_Y_HI) << 8)) + reach);

    wr8(zTemp00, rd8(OBJ_X_LO));
    wr8(zTemp01, 0);
    wr8(zTemp02, (uint8_t)y);
    wr8(zTemp03, (uint8_t)(y >> 8));
    return (y >> 8) == 0;
}

/*
 * @routine 01BC:093 RichterLadderCheck
 * @run 6D:B893
 * WHAT IT DOES: Says whether there is something to climb down from here: looks downward from
 * $18 below him, a tile at a time, for the first solid tile with one of bits 0 or 2 above it.
 * HOW: The section's camera mode settles it in two cases; low on the screen it cannot be.
 * Entry: nothing. Exit: C the answer. A, Y and V as the last step left them. X kept. Clobbers
 * zTemp00-03, zTemp10-11.
 */
LadderCheck RichterLadderCheck(void)
{
    LadderCheck check = { LADDER_CAMERA_NO };
    uint8_t mode = rd8(CameraMode);

    if (mode == CAMERA_NO_LADDER)
        return check;
    if (mode == CAMERA_LADDER) {
        check.how = LADDER_CAMERA_YES;
        return check;
    }
    if (rd8(OBJ_Y_HI) != 0) {
        check.how = LADDER_OFF_SCREEN;
        return check;
    }
    if (rd8(OBJ_Y_LO) >= LADDER_LOWEST_Y) {
        check.how = LADDER_TOO_LOW;
        return check;
    }
    for (check.reach = LADDER_FIRST_REACH;; check.reach = (uint16_t)(check.reach + LADDER_STEP)) {
        wr8(zTemp10, (uint8_t)check.reach);
        wr8(zTemp11, 0);
        if (check.reach > 0xFF) {
            check.how = LADDER_REACH_OUT;
            return check;
        }
        if (!set_ladder_point(check.reach)) {
            check.how = LADDER_OFF_PAGE;
            return check;
        }
        check.last = StageTileRead();
        check.any_read = true;
        if (tile_answer(check.last) & TILE_SOLID_BIT)
            break;
    }
    if (check.reach == LADDER_FIRST_REACH) {
        check.how = LADDER_BLOCKED;
        return check;
    }
    check.reach = (uint16_t)(check.reach - LADDER_STEP);
    wr8(zTemp10, (uint8_t)check.reach);
    wr8(zTemp11, (uint8_t)(check.reach >> 8));
    set_ladder_point(check.reach);
    check.last = StageTileRead();
    check.answer = tile_answer(check.last) & TILE_LADDER_BITS;
    check.how = LADDER_ANSWERED;
    return check;
}

bool ladder_found(LadderCheck check)
{
    return check.how == LADDER_CAMERA_YES || (check.how == LADDER_ANSWERED && check.answer == 0);
}

void ladder_exit_shim(LadderCheck check)
{
    uint8_t y_low = rd8(OBJ_Y_LO);

    if (check.any_read)
        terrain_read_exit_shim(check.last);
    switch (check.how) {
    case LADDER_CAMERA_NO:
        cpu.a = CAMERA_NO_LADDER;
        abi_nz(0);
        abi_flag(P_C, false);
        break;
    case LADDER_CAMERA_YES:
        cpu.a = CAMERA_LADDER;
        abi_nz(0);
        abi_flag(P_C, true);
        break;
    case LADDER_OFF_SCREEN:
        cpu.a = rd8(OBJ_Y_HI);
        abi_nz(cpu.a);
        abi_flag(P_C, false);
        break;
    case LADDER_TOO_LOW:
        cpu.a = y_low;
        abi_nz((uint8_t)(y_low - LADDER_LOWEST_Y));
        abi_flag(P_C, false);
        break;
    case LADDER_OFF_PAGE:
        cpu.a = rd8(zTemp03);
        abi_nz(cpu.a);
        abi_flag(P_C, false);
        abi_flag(P_V, cpu.a == 0x80);
        break;
    case LADDER_BLOCKED:
        cpu.a = TILE_SOLID_BIT;
        abi_nz(cpu.a);
        abi_flag(P_C, false);
        break;
    case LADDER_REACH_OUT:
        cpu.a = (uint8_t)check.reach;
        abi_nz(cpu.a);
        abi_flag(P_C, false);
        abi_flag(P_V, false);
        break;
    case LADDER_ANSWERED:
        cpu.a = check.answer;
        abi_nz(cpu.a);
        abi_flag(P_C, check.answer == 0);
        break;
    }
}

void abi_RichterLadderCheck(void)
{
    ladder_exit_shim(RichterLadderCheck());
}

/* How the jump-or-climb check ended. */
typedef struct {
    bool pressed;
    bool up_held;
    uint8_t held;
    LadderCheck ladder;
} JumpOrClimb;

/*
 * @routine 01BC:06F RichterJumpOrClimb
 * @run 6D:B86F
 * WHAT IT DOES: Says whether jump was pressed and he may act on it: with up held, only if there
 * is a ladder to take.
 * HOW: RichterJumpPressed, then RichterLadderCheck when up is held; the latched buttons clear.
 * Entry: nothing. Exit: C the answer, with what the last step left.
 */
JumpOrClimb RichterJumpOrClimb(void)
{
    JumpOrClimb result = { RichterJumpPressed() };

    if (result.pressed) {
        wr8(PadForcedPressB, 0);
        result.held = rd8(zPadHeld);
        result.up_held = (result.held & PAD_UP) != 0;
        if (result.up_held)
            result.ladder = RichterLadderCheck();
    }
    return result;
}

void abi_RichterJumpOrClimb(void)
{
    JumpOrClimb result = RichterJumpOrClimb();

    if (!result.pressed) {
        abi_flag(P_C, false);
        return;
    }
    abi_nz(0);
    abi_flag(P_N, (result.held & 0x80) != 0);
    abi_flag(P_V, (result.held & 0x40) != 0);
    if (result.up_held)
        ladder_exit_shim(result.ladder);
    else
        abi_flag(P_C, true);
}

/* A blocking tile counts by its bit $20, or else only if the pad presses that way. */
static bool column_counts(uint8_t answer, uint8_t pad_bit)
{
    return (answer & TILE_BLOCK_ANY) != 0 || (rd8(zPadHeld) & pad_bit) != 0;
}

/* How the ledge check ended. */
typedef enum { LEDGE_PUSHED, LEDGE_LOW, LEDGE_HIGH, LEDGE_NONE } LedgeEnd;

typedef struct {
    LedgeEnd how;
    uint8_t push;
    LowColumns low;
    int high_column;
} LedgeCheck;

/*
 * @routine 01BC:241 RichterLedgeCheck
 * @run 6D:BA41
 * WHAT IT DOES: Says whether a low or a high column in front of him gives him a ledge to act
 * on, unless a push holds him. zTemp01 says which: 0 low, 1 high.
 * HOW: RichterLowColumnsBlocked with down held, RichterColumnsBlocked with up held; bit $20 of
 * the tile counts whatever the pad.
 * Entry: nothing. Exit: C the answer, with what the last step left.
 */
LedgeCheck RichterLedgeCheck(void)
{
    LedgeCheck check = { LEDGE_PUSHED };

    check.push = rd8(zPlayerPush);
    check.high_column = -1;
    if (check.push & PUSH_ON_FLOOR)
        return check;
    check.low = RichterLowColumnsBlocked();
    if (check.low.blocked_column >= 0 && column_counts(check.low.answer, PAD_DOWN)) {
        wr8(zTemp01, 0);
        check.how = LEDGE_LOW;
        return check;
    }
    check.high_column = RichterColumnsBlocked();
    if (check.high_column >= 0 &&
        column_counts(rd8((uint16_t)(RichterColumnHit + check.high_column)), PAD_UP)) {
        wr8(zTemp01, 1);
        check.how = LEDGE_HIGH;
        return check;
    }
    check.how = LEDGE_NONE;
    return check;
}

void abi_RichterLedgeCheck(void)
{
    LedgeCheck check = RichterLedgeCheck();
    uint8_t low_bit = check.low.answer & TILE_BLOCK_ANY;

    abi_flag(P_V, (check.push & 0x40) != 0);
    if (check.how == LEDGE_PUSHED) {
        abi_nz(check.push & PUSH_ON_FLOOR);
        abi_flag(P_N, (check.push & 0x80) != 0);
        abi_flag(P_C, false);
        return;
    }
    low_columns_exit_shim(check.low);
    if (check.low.blocked_column >= 0) {
        cpu.a = low_bit;
        abi_nz(low_bit);
        if (low_bit != 0)
            abi_flag(P_N, true);
    }
    if (check.how == LEDGE_LOW) {
        abi_flag(P_C, true);
        return;
    }
    if (check.high_column >= 0) {
        uint8_t high_bit = rd8((uint16_t)(RichterColumnHit + check.high_column)) & TILE_BLOCK_ANY;

        cpu.x = (uint8_t)check.high_column;
        cpu.a = high_bit;
        abi_nz(high_bit);
        if (high_bit != 0)
            abi_flag(P_N, true);
    } else {
        cpu.x = BLOCKED_COLUMNS;
        cpu.a = 0;
        abi_nz(0);
    }
    if (check.how == LEDGE_HIGH) {
        cpu.a = 1;
        abi_nz(1);
        abi_flag(P_C, true);
    } else {
        abi_flag(P_C, false);
    }
}

/*
 * @routine 01BC:280 RichterLiftByColumn
 * @run 6D:BA80
 * WHAT IT DOES: Lifts him out of the floor by how deep this column read, and not above the top
 * of the map.
 * HOW: The depth in RichterColumnRow, signed, which is cleared.
 * Entry: X column. Exit: A his Y high byte, or 2 at the top; N and Z from it; C and V from the
 * subtract. X and Y kept. Clobbers zTemp10.
 */
ColumnLift RichterLiftByColumn(uint8_t column)
{
    uint16_t depth_field = (uint16_t)(RichterColumnRow + column);
    ColumnLift lift;
    uint16_t lifted;

    lift.depth = (uint16_t)(int16_t)(int8_t)rd8(depth_field);
    lift.y = (uint16_t)(rd8(OBJ_Y_LO) | (rd8(OBJ_Y_HI) << 8));
    lifted = (uint16_t)(lift.y - lift.depth);
    wr8(zTemp10, (uint8_t)(lift.depth >> 8));
    lift.off_the_top = (lifted & 0x8000) != 0;
    wr8(OBJ_Y_LO, lift.off_the_top ? FLOOR_TOP_OF_MAP : (uint8_t)lifted);
    wr8(OBJ_Y_HI, lift.off_the_top ? 0 : (uint8_t)(lifted >> 8));
    wr8(depth_field, 0);
    return lift;
}

void column_lift_exit_shim(ColumnLift lift)
{
    uint16_t lifted = (uint16_t)(lift.y - lift.depth);

    cpu.a = lift.off_the_top ? FLOOR_TOP_OF_MAP : (uint8_t)(lifted >> 8);
    abi_nz(cpu.a);
    abi_flag(P_C, lift.y >= lift.depth);
    abi_flag(P_V, ((lift.y ^ lift.depth) & (lift.y ^ lifted) & 0x8000) != 0);
}

void abi_RichterLiftByColumn(void)
{
    column_lift_exit_shim(RichterLiftByColumn(cpu.x));
}

/* How a landing lift ended. */
typedef struct {
    FloorResult check;
    bool lifted;
    ColumnLift lift;
} LandingLift;

/*
 * @routine 01BC:27A RichterLandOnFloor
 * @run 6D:BA7A
 * WHAT IT DOES: If there is a floor under him, lifts him out of it.
 * HOW: RichterFloorCheck, then runs on into RichterLiftByColumn.
 * Entry: nothing. Exit: as RichterLiftByColumn, or as RichterFloorCheck with no floor.
 */
LandingLift RichterLandOnFloor(void)
{
    LandingLift landing = { RichterFloorCheck() };

    if (landing.check.how == FLOOR_NONE)
        return landing;
    landing.lifted = true;
    landing.lift = RichterLiftByColumn(landing.check.column);
    return landing;
}

void landing_exit_shim(LandingLift landing)
{
    floor_check_exit_shim(landing.check);
    if (landing.lifted)
        column_lift_exit_shim(landing.lift);
}

void abi_RichterLandOnFloor(void)
{
    landing_exit_shim(RichterLandOnFloor());
}

/* The answers a caller branches on. */
bool wall_found(WallProbe probe)
{
    return probe.how == WALL_BY_ORIGINAL ? probe.answer != 0 :
           probe.how == WALL_PUSHED || probe.how == WALL_FOUND;
}

bool floor_found(FloorProbe probe)
{
    return probe.how == FLOOR_BY_ORIGINAL ? probe.answer != 0 :
           probe.how == FLOOR_LANDED || probe.how == FLOOR_OFF_THE_TOP;
}

/* What each leaves in A, for a caller that tests A rather than the flags it left. */
uint8_t wall_answer(WallProbe probe)
{
    switch (probe.how) {
    case WALL_BARRED:
        return 1;
    case WALL_PUSHED:
    case WALL_FOUND:
        return WALL_ANSWER;
    case WALL_BY_ORIGINAL:
        return probe.answer;
    default:
        return 0;
    }
}

uint8_t floor_answer(FloorProbe probe)
{
    if (probe.how == FLOOR_BY_ORIGINAL)
        return probe.answer;
    return probe.how == FLOOR_LANDED || probe.how == FLOOR_OFF_THE_TOP ? 1 : 0;
}

/* One call to the other character's overlay, which has no C yet. */
static uint8_t player_overlay_shim(uint16_t entry, bool set_side, uint8_t side)
{
    if (set_side)
        cpu.y = side;
    call_original(entry);
    return cpu.a;
}

/* The player overlay's entries, for whichever character's overlay is in RAM. */
WallProbe PlayerWall(uint8_t side)
{
    WallProbe other = { WALL_BY_ORIGINAL };

    if (rd8(zCharacterLoaded) == CHARACTER_RICHTER)
        return RichterWall(side);
    other.answer = player_overlay_shim(PlayerOvlWall, true, side);
    return other;
}

FloorProbe PlayerFloor(void)
{
    FloorProbe other = { FLOOR_BY_ORIGINAL };

    if (rd8(zCharacterLoaded) == CHARACTER_RICHTER)
        return RichterFloor();
    other.answer = player_overlay_shim(PlayerOvlFloor, false, 0);
    return other;
}

ColumnsEnd PlayerMoved(void)
{
    ColumnsEnd other = { true };

    if (rd8(zCharacterLoaded) == CHARACTER_RICHTER)
        return RichterReadColumns();
    player_overlay_shim(PlayerOvlMoved, false, 0);
    return other;
}
