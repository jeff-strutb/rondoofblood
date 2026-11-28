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
#include "access.h"
#include "enemy_objects.h"
#include "player_objects.h"

enum {
    WALL_ANSWER = 0x02, WALL_COLUMNS = 6, WALL_FIRST_COLUMN_STANDING = 0,
    WALL_FIRST_COLUMN_CROUCHING = 3, WALL_FIRST_COLUMN_IN_AIR = 5,
    STATE_CROUCHING = 2, STATE_IN_AIR = 3, STATE_FALLING = 4,
    RICHTER_COLUMNS = 5, COLUMNS_BELOW_WAIST = 0x0B, COLUMNS_BELOW_FEET = 0x16,
    BLOCKED_COLUMNS = 2, MARIA_BLOCKED_COLUMNS = 3, MARIA_THIRD_COLUMN = 2, TILE_BLOCKS = 0x24, FLOOR_COLUMNS = 3, TILE_FLOOR = 0x81,
    PUSH_ON_FLOOR = 0x01, PUSH_LIFTING = 0x20, CHARACTER_RICHTER = 0, COLUMNS_BELOW_LOW = 0x12, COLUMNS_BELOW_CHECK = 0x0E,
    FLOOR_TOP_OF_MAP = 0x02, ROW_ABOVE_THE_MAP = 0xFF, COLUMNS_BELOW_SOLES = 0x1E, PAD_UP = 0x40, PAD_DOWN = 0x10,
    PAD_JUMP_PRESS = 0x01, CAMERA_NO_LADDER = 0x01, CAMERA_LADDER = 0x18, LADDER_LOWEST_Y = 0xB0,
    LADDER_FIRST_REACH = 0x18, LADDER_STEP = 8, TILE_SOLID_BIT = 0x01, TILE_LADDER_BITS = 0x05,
    TILE_BLOCK_ANY = 0x20
};

FloorResult RichterFloorOddScreen(void);
void RichterWallPushed(void);
static void floor_clears_push(FloorResult *result);
static void read_entry_kept_shim(Sum row);

/* The wall probe holds the side in Y throughout; a read leaves Y as it leaves it. */
static void wall_side_shim(uint8_t side)
{
    cpu.y = side;
}

static void wall_entry_shim(Sum high, uint8_t column)
{
    cpu.x = column;
    read_entry_kept_shim(high);
}
static void read_entry_shim(Sum row, uint8_t x, uint8_t y);

/* The point's X and Y high byte are set, or zero: a read of the point or no answer. */
static TileRead read_point_or_nothing(bool off_the_map)
{
    TileRead nothing = { TILE_EMPTY_PAGE, 0, 0, 0, 0, 0 };

    return off_the_map ? nothing : StageTileRead();
}

/* The overlay's read may run its own code: it is entered with the add's A and flags. */
static void read_above_entry_shim(bool on_the_map)
{
    cpu.a = 0;
    abi_nz(0);
    abi_flag(P_C, on_the_map);
    abi_flag(P_V, false);
}

/*
 * @routine 01BB:6D2 RichterReadAbove
 * @run 6D:B6D2
 * WHAT IT DOES: Reads the terrain at a row that may lie above the map: a row off the top
 * reads as nothing.
 * HOW: The row's add, with its carry, decides the point's Y high byte; only zero is read.
 * WAITS: no.
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
    read_above_entry_shim(on_the_map);
    return read_point_or_nothing(false);
}

/*
 * When nothing was read, A is 0 by CLA, which leaves N and Z as they stood on the HuC6280: A5
 * shows the original's P 01 from an entry of 01 where setting Z gave 03. Otherwise as the read.
 */
void read_or_nothing_exit_shim(TileRead read, bool read_made)
{
    if (read_made)
        terrain_read_exit_shim(read);
    else
        cpu.a = 0;
}

void read_above_exit_shim(TileRead read, bool on_the_map)
{
    if (on_the_map && read.kind == TILE_BY_ORIGINAL) {
        return;
    }
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
 * @routine 01E0:1F9 MariaReadAbove
 * @run 6D:B9F9
 * WHAT IT DOES: RichterReadAbove in Maria's player overlay (01DD).
 * HOW: The same instructions; her overlay keeps its reads in another order, so its branches go into MariaReadBelow.
 * Untraced: the row above the map ($BA00 taken).
 * WAITS: no.
 * Entry, exit: as RichterReadAbove.
 */
TileRead MariaReadAbove(uint8_t row, bool on_the_map)
{
    return RichterReadAbove(row, on_the_map);
}

void abi_MariaReadAbove(void)
{
    bool on_the_map = (cpu.p & P_C) != 0;

    read_above_exit_shim(MariaReadAbove(cpu.a, on_the_map), on_the_map);
}

/*
 * @routine 01BB:6DD RichterReadBelow
 * @run 6D:B6DD
 * WHAT IT DOES: Reads the terrain at a row on the first screen down: a row past it reads as
 * nothing.
 * HOW: The carry from the row's add says it passed; otherwise X and Y high bytes are zero.
 * WAITS: no.
 * Entry: A row, C its add's carry, zTemp00 X. Exit: as the read, or A 0 with N, Z, C and V kept.
 */
static TileRead read_below(uint8_t row, bool carry)
{
    wr8(zTemp02, row);
    if (carry)
        return read_point_or_nothing(true);
    wr8(zTemp03, 0);
    wr8(zTemp01, 0);
    return read_point_or_nothing(false);
}

TileRead RichterReadBelow(uint8_t row, bool carry)
{
    return read_below(row, carry);
}

void abi_RichterReadBelow(void)
{
    bool carry = (cpu.p & P_C) != 0;

    read_or_nothing_exit_shim(RichterReadBelow(cpu.a, carry), !carry);
}

/*
 * @routine 01E0:20E MariaReadBelow
 * @run 6D:BA0E
 * WHAT IT DOES: RichterReadBelow in Maria's overlay (01DD): reads the terrain at a row on the first screen down; a row
 * past it reads as nothing.
 * HOW: The same instructions as RichterReadBelow.
 * WAITS: no.
 * Entry, exit: as RichterReadBelow.
 */
TileRead MariaReadBelow(uint8_t row, bool carry)
{
    return read_below(row, carry);
}

void abi_MariaReadBelow(void)
{
    bool carry = (cpu.p & P_C) != 0;

    read_or_nothing_exit_shim(MariaReadBelow(cpu.a, carry), !carry);
}

/*
 * @routine 01BB:6EA RichterReadOnScreen
 * @run 6D:B6EA
 * WHAT IT DOES: Reads the terrain at a row, the Y high byte taken from Richter's own.
 * HOW: His Y high byte plus the carry from the row's add.
 * WAITS: no.
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
 * @routine 01E0:204 MariaReadOnScreen
 * @run 6D:BA04
 * WHAT IT DOES: RichterReadOnScreen in Maria's player overlay (01DD).
 * HOW: The same instructions; its branch goes into MariaReadBelow.
 * Untraced: all of it; nothing in her overlay that a script ran calls it.
 * WAITS: no.
 * Entry, exit: as RichterReadOnScreen.
 */
TileRead MariaReadOnScreen(uint8_t row, bool carry)
{
    return RichterReadOnScreen(row, carry);
}

void abi_MariaReadOnScreen(void)
{
    terrain_read_exit_shim(MariaReadOnScreen(cpu.a, (cpu.p & P_C) != 0));
}

/*
 * @routine 01BB:6F4 RichterReadIfOnPage
 * @run 6D:B6F4
 * WHAT IT DOES: Reads the terrain at the point in zTemp00-03, unless the Y high byte just
 * added is not zero.
 * HOW: Tests what the caller's add left.
 * WAITS: no.
 * Entry: Z from that byte, zTemp00-03 the point. Exit: as the read, or A 0 with N and Z kept.
 */
static TileRead read_if_on_page(bool on_page)
{
    return read_point_or_nothing(!on_page);
}

TileRead RichterReadIfOnPage(bool on_page)
{
    return read_if_on_page(on_page);
}

void abi_RichterReadIfOnPage(void)
{
    bool on_page = (cpu.p & P_Z) != 0;

    read_or_nothing_exit_shim(RichterReadIfOnPage(on_page), on_page);
}

/*
 * @routine 01E0:21B MariaReadIfOnPage
 * @run 6D:BA1B
 * WHAT IT DOES: RichterReadIfOnPage in Maria's overlay: reads the terrain at the point in zTemp00-03, unless the Y high byte just
 * added is not zero.
 * HOW: The same instructions as RichterReadIfOnPage; its branch goes to the end of MariaReadBelow.
 * WAITS: no.
 * Entry, exit: as RichterReadIfOnPage.
 */
TileRead MariaReadIfOnPage(bool on_page)
{
    return read_if_on_page(on_page);
}

void abi_MariaReadIfOnPage(void)
{
    bool on_page = (cpu.p & P_Z) != 0;

    read_or_nothing_exit_shim(MariaReadIfOnPage(on_page), on_page);
}

/*
 * @routine 01BB:6F9 RichterColumnOffset
 * @run 6D:B6F9
 * WHAT IT DOES: Gives how far across a probe's column lies, by column and facing.
 * HOW: A table of pairs, one for each way he faces.
 * WAITS: no.
 * Entry: X column. Exit: A the offset, N and Z from it, Y its index, C and V from the index's
 * add. X kept.
 */
static ColumnOffset column_offset(uint8_t column, uint16_t table)
{
    ColumnOffset r;
    uint8_t facing = RD8(OBJ->facing_left[0]);
    Sum index = add_bytes((uint8_t)(column << 1), facing, false);

    r.value = rd8((uint16_t)(table + index.value));
    r.index = index.value;
    r.carry = index.carry;
    r.overflow = index.overflow;
    return r;
}

static void column_offset_exit_shim(ColumnOffset r)
{
    cpu.a = r.value;
    abi_nz(r.value);
    cpu.y = r.index;
    abi_flag(P_C, r.carry);
    abi_flag(P_V, r.overflow);
}

ColumnOffset RichterColumnOffset(uint8_t column)
{
    return column_offset(column, RichterColumnOffsets);
}

void abi_RichterColumnOffset(void)
{
    column_offset_exit_shim(RichterColumnOffset(cpu.x));
}

/* Maria's overlay's copy of the table of column offsets, $31C above Richter's. */
enum { MARIA_COLUMN_OFFSETS = RichterColumnOffsets + 0x31C };

/*
 * @routine 01E0:220 MariaColumnOffset
 * @run 6D:BA20
 * WHAT IT DOES: RichterColumnOffset in Maria's overlay: how far across a probe's column lies, by column and facing.
 * HOW: The same instructions, with her overlay's table at $BE69.
 * WAITS: no.
 * Entry, exit: as RichterColumnOffset.
 */
ColumnOffset MariaColumnOffset(uint8_t column)
{
    return column_offset(column, MARIA_COLUMN_OFFSETS);
}

void abi_MariaColumnOffset(void)
{
    column_offset_exit_shim(MariaColumnOffset(cpu.x));
}

/*
 * @routine 01BC:367 RichterWallPushed
 * @run 6D:BB67
 * WHAT IT DOES: The wall probe's answer when a push holds him against that side: a wall as
 * deep as the push says.
 * HOW: zPlayerPushDepth to zTemp00. Sits before RichterWall, which branches back to it.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, N and Z clear. X, Y, C and V kept.
 */
static void wall_pushed(void)
{
    wr8(zTemp00, rd8(zPlayerPushDepth));
}

void RichterWallPushed(void)
{
    wall_pushed();
}

void abi_RichterWallPushed(void)
{
    RichterWallPushed();
    cpu.a = WALL_ANSWER;
    abi_nz(WALL_ANSWER);
}

/*
 * @routine 01E0:683 MariaWallPushed
 * @run 6D:BE83
 * WHAT IT DOES: RichterWallPushed in Maria's overlay: the wall probe's answer when a push holds her against that side.
 * HOW: The same instructions as RichterWallPushed. Sits before MariaWall, which branches back to it.
 * WAITS: no.
 * Entry, exit: as RichterWallPushed.
 */
void MariaWallPushed(void)
{
    wall_pushed();
}

void abi_MariaWallPushed(void)
{
    MariaWallPushed();
    cpu.a = WALL_ANSWER;
    abi_nz(WALL_ANSWER);
}

/* Maria's overlay keeps the wall probe's four tables $31C above Richter's. */
enum { MARIA_TABLES = 0x31C };

/* The column the wall probe starts at by player state: Richter's, in the air (3, 4) 5, crouching (2) 3. */
static uint8_t richter_wall_first_column(uint8_t state)
{
    return state == STATE_IN_AIR || state == STATE_FALLING ? WALL_FIRST_COLUMN_IN_AIR :
           state == STATE_CROUCHING ? WALL_FIRST_COLUMN_CROUCHING : WALL_FIRST_COLUMN_STANDING;
}

/* Maria's: states 3, 4, $0F and $0E 5, state $0D 2. */
static uint8_t maria_wall_first_column(uint8_t state)
{
    return state == STATE_IN_AIR || state == STATE_FALLING || state == 0x0F || state == 0x0E ? WALL_FIRST_COLUMN_IN_AIR :
           state == 0x0D ? 2 : WALL_FIRST_COLUMN_STANDING;
}

/* A character's overlay's wall probe: its tables, the return of its read and the copies it reaches. */
typedef struct {
    uint16_t push_bits, reach, lift, lift_high, read_return;
    void (*pushed)(void);
    TileRead (*read_if_on_page)(bool on_page);
    uint8_t (*first_column)(uint8_t state);
} WallLayout;

static const WallLayout richter_wall = {
    RichterWallPushBits, RichterWallReach, RichterWallLift, RichterWallLiftHigh, 0xBBAF,
    RichterWallPushed, RichterReadIfOnPage, richter_wall_first_column
};
static const WallLayout maria_wall = {
    RichterWallPushBits + MARIA_TABLES, RichterWallReach + MARIA_TABLES, RichterWallLift + MARIA_TABLES,
    RichterWallLiftHigh + MARIA_TABLES, 0xBED3, MariaWallPushed, MariaReadIfOnPage, maria_wall_first_column
};

/*
 * @routine 01BC:36E RichterWall
 * @run 6D:BB6E
 * WHAT IT DOES: Asks whether Richter runs into a wall on one side, and how deep: what pushes
 * him that way counts as a wall, and otherwise up to six points down his side are read,
 * fewer when he crouches or is in the air.
 * HOW: The reach by side, then each column's height added to his position.
 * WAITS: no.
 * Entry: Y side. Exit: A 2 with N 0, Z 0 at a wall, zTemp00 how deep; else Z set. Other
 * registers and flags as the last step left them.
 */
static WallProbe wall_probe(uint8_t side, const WallLayout *layout)
{
    WallProbe probe = { WALL_BARRED };
    uint8_t state, reach, value, x_lo, y_lo, y_hi, column;
    Sum across, low, high;
    TileRead read;

    probe.mode = rd8(PlayerMayLeaveTop);
    if (probe.mode == 1) {
        return probe;
    }
    value = RD8(ZP->player_push);
    if (value & rd8((uint16_t)(layout->push_bits + side))) {
        layout->pushed();
        probe.how = WALL_PUSHED;
        return probe;
    }
    reach = rd8((uint16_t)(layout->reach + side));
    wall_side_shim(side);
    WR8(ZP->temp10, reach);
    state = RD8(OBJ->state[0]);
    column = layout->first_column(state);
    probe.column = column;
    for (; probe.column < WALL_COLUMNS; probe.column++) {
        x_lo = RD8(OBJ->x_lo[0]);
        value = RD8(ZP->temp10);
        across = add_bytes(x_lo, value, false);
        WR8(ZP->temp00, across.value);
        WR8(ZP->temp01, 0);
        y_lo = RD8(OBJ->y_lo[0]);
        low = add_bytes(y_lo, rd8((uint16_t)(layout->lift + probe.column)), false);
        WR8(ZP->temp02, low.value);
        y_hi = RD8(OBJ->y_hi[0]);
        high = add_bytes(y_hi, rd8((uint16_t)(layout->lift_high + probe.column)), low.carry);
        WR8(ZP->temp03, high.value);
        probe.v = high.overflow;
        probe.read_made = high.value == 0;
        wall_entry_shim(high, probe.column);
        jsr_push(layout->read_return);
        read = layout->read_if_on_page(probe.read_made);
        jsr_pop();
        if (probe.read_made) {
            probe.last_read = read;
            probe.any_read = true;
            if (tile_answer(read) & WALL_ANSWER) {
                probe.how = WALL_FOUND;
                return probe;
            }
        }
    }
    probe.how = WALL_NONE;
    return probe;
}

WallProbe RichterWall(uint8_t side)
{
    return wall_probe(side, &richter_wall);
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
        abi_flag(P_C, probe.mode >= 1);
        break;
    case WALL_FOUND:
        terrain_read_exit_shim(probe.last_read);
        cpu.a = WALL_ANSWER;
        abi_nz(WALL_ANSWER);
        cpu.x = probe.column;
        break;
    case WALL_NONE:
        if (probe.any_read) {
            terrain_read_exit_shim(probe.last_read);
        }
        if (!probe.read_made) {
            abi_flag(P_V, probe.v);
        }
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

/*
 * @routine 01E0:68A MariaWall
 * @run 6D:BE8A
 * WHAT IT DOES: RichterWall in Maria's overlay: whether she runs into a wall on one side, and how deep. Her states
 * choose the first column otherwise: 3, 4, $0E and $0F start at 5, $0D at 2, the rest at 0.
 * HOW: RichterWall's instructions with her overlay's tables, MariaWallPushed and MariaReadIfOnPage, and more
 * compares of the state.
 * WAITS: no.
 * Entry, exit: as RichterWall.
 */
WallProbe MariaWall(uint8_t side)
{
    return wall_probe(side, &maria_wall);
}

void abi_MariaWall(void)
{
    wall_exit_shim(MariaWall(cpu.y));
}

/* The overlay's read may run its own code: it is entered with the row add's A and flags. */
static void read_entry_kept_shim(Sum row);
static void read_entry_shim(Sum row, uint8_t x, uint8_t y)
{
    cpu.x = x;
    cpu.y = y;
    read_entry_kept_shim(row);
}

/* The same with X and Y as they stand. */
static void read_entry_kept_shim(Sum row)
{
    cpu.a = row.value;
    abi_nz(row.value);
    abi_flag(P_C, row.carry);
    abi_flag(P_V, row.overflow);
}

/* A character's overlay's five-column read: the returns of its two calls and the copies it calls. */
typedef struct {
    uint16_t offset_return, read_return;
    ColumnOffset (*offset)(uint8_t column);
    TileRead (*read_below)(uint8_t row, bool carry);
} ColumnsLayout;

static const ColumnsLayout richter_columns = { 0xBB2B, 0xBB3A, RichterColumnOffset, RichterReadBelow };
static const ColumnsLayout maria_columns = { 0xBE47, 0xBE56, MariaColumnOffset, MariaReadBelow };

static ColumnsEnd read_five_columns(uint8_t below, const ColumnsLayout *layout)
{
    ColumnsEnd end = { false, false, { TILE_EMPTY_PAGE, 0, 0, 0, 0, 0 }, 0, below, 0, 0, false };
    ColumnOffset offset;
    Sum across, row;
    uint8_t value, base;

    WR8(ZP->temp10, below);
    for (uint8_t column = 0; column < RICHTER_COLUMNS; column++) {
        jsr_push(layout->offset_return);
        offset = layout->offset(column);
        jsr_pop();
        base = RD8(OBJ->x_lo[0]);
        across = add_bytes(offset.value, base, false);
        WR8(ZP->temp00, across.value);
        base = RD8(OBJ->y_lo[0]);
        value = RD8(ZP->temp10);
        row = add_bytes(base, value, false);
        read_entry_shim(row, column, offset.index);
        jsr_push(layout->read_return);
        end.last = layout->read_below(row.value, row.carry);
        jsr_pop();
        end.read_made = !row.carry;
        end.row = base;
        end.below = value;
        end.v = row.overflow;
        end.index = offset.index;
        value = end.read_made ? tile_answer(end.last) : 0;
        wr8((uint16_t)(RichterColumnHit + column), value);
        value = RD8(ZP->temp02);
        wr8((uint16_t)(RichterColumnRow + column), value);
        value = RD8(ZP->temp00);
        wr8((uint16_t)(RichterColumnX + column), value);
        end.a = value;
    }
    return end;
}

/* X 5, Z set and C set from the loop's compare; A the last column's X. */
void columns_exit_shim(ColumnsEnd end)
{
    if (end.by_original) {
        return;
    }
    if (end.read_made) {
        terrain_read_exit_shim(end.last);
    } else {
        cpu.y = end.index;
        abi_flag(P_V, end.v);
    }
    cpu.a = end.a;
    cpu.x = RICHTER_COLUMNS;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 01BC:31D RichterReadColumnsHigh
 * @run 6D:BB1D
 * WHAT IT DOES: Reads the five columns of terrain at his waist.
 * HOW: As RichterReadColumns, $0B below his position.
 * WAITS: no.
 * Entry and exit: as RichterReadColumns.
 */
ColumnsEnd RichterReadColumnsHigh(void)
{
    return read_five_columns(COLUMNS_BELOW_WAIST, &richter_columns);
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
 * WAITS: no.
 * Entry: nothing. Exit: A the last column's X, X 5, N 0, Z 1, C 1, Y and V from the last read,
 * or its row's add if that passed the screen. Clobbers zTemp00-03, zTemp10.
 */
ColumnsEnd RichterReadColumns(void)
{
    return read_five_columns(COLUMNS_BELOW_FEET, &richter_columns);
}

void abi_RichterReadColumns(void)
{
    columns_exit_shim(RichterReadColumns());
}

/*
 * @routine 01E0:639 MariaReadColumnsHigh
 * @run 6D:BE39
 * WHAT IT DOES: RichterReadColumnsHigh in Maria's overlay: the five columns of terrain at her waist.
 * HOW: As MariaReadColumns, $0B below her position.
 * WAITS: no.
 * Entry and exit: as RichterReadColumns.
 */
ColumnsEnd MariaReadColumnsHigh(void)
{
    return read_five_columns(COLUMNS_BELOW_WAIST, &maria_columns);
}

void abi_MariaReadColumnsHigh(void)
{
    columns_exit_shim(MariaReadColumnsHigh());
}

/*
 * @routine 01E0:63F MariaReadColumns
 * @run 6D:BE3F
 * WHAT IT DOES: RichterReadColumns in Maria's overlay: the five columns of terrain under her, into RichterColumnHit,
 * RichterColumnRow and RichterColumnX; the step her overlay makes after she has moved.
 * HOW: The same instructions as RichterReadColumns, calling MariaColumnOffset and MariaReadBelow.
 * WAITS: no.
 * Entry, exit: as RichterReadColumns.
 */
ColumnsEnd MariaReadColumns(void)
{
    return read_five_columns(COLUMNS_BELOW_FEET, &maria_columns);
}

void abi_MariaReadColumns(void)
{
    columns_exit_shim(MariaReadColumns());
}

/*
 * @routine 01BC:2AD RichterColumnsBlocked
 * @run 6D:BAAD
 * WHAT IT DOES: Says whether either of the first two columns read holds a blocking tile.
 * HOW: Bits $24 of RichterColumnHit.
 * WAITS: no.
 * Entry: nothing. Exit: at one, A those bits, X the column; else A 0, X 2, C 1, Z 1. V and,
 * at one, C kept. Y kept.
 */
static BlockedColumns columns_blocked(int columns)
{
    BlockedColumns r = { -1, 0 };
    uint8_t hit;

    for (int column = 0; column < columns; column++) {
        hit = rd8((uint16_t)(RichterColumnHit + column));
        if (hit & TILE_BLOCKS) {
            r.column = column;
            r.bits = (uint8_t)(hit & TILE_BLOCKS);
            return r;
        }
    }
    return r;
}

BlockedColumns RichterColumnsBlocked(void)
{
    return columns_blocked(BLOCKED_COLUMNS);
}

/* At a blocked column: A its bits, X the column, C clear past the first; else A 0, X the count, C and Z set. */
static void columns_blocked_exit_shim(BlockedColumns r, int columns)
{
    if (r.column >= 0) {
        cpu.x = (uint8_t)r.column;
        cpu.a = r.bits;
        abi_nz(cpu.a);
        abi_flag(P_C, r.column > 0 ? false : (cpu.p & P_C) != 0);
    } else {
        cpu.x = (uint8_t)columns;
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, true);
    }
}

void abi_RichterColumnsBlocked(void)
{
    columns_blocked_exit_shim(RichterColumnsBlocked(), BLOCKED_COLUMNS);
}

/*
 * @routine 01E0:5C9 MariaColumnsBlocked
 * @run 6D:BDC9
 * WHAT IT DOES: RichterColumnsBlocked in Maria's player overlay (01DD), over three columns, not two.
 * HOW: RichterColumnsBlocked's body, with the count her CPX compares with.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterColumnsBlocked, X 3 when no column blocks.
 */
BlockedColumns MariaColumnsBlocked(void)
{
    return columns_blocked(MARIA_BLOCKED_COLUMNS);
}

void abi_MariaColumnsBlocked(void)
{
    columns_blocked_exit_shim(MariaColumnsBlocked(), MARIA_BLOCKED_COLUMNS);
}

/* A floor was found: RMB 0 of the push, then RMB 5 unless the push is carrying him up; the A the check ends with. */
static void floor_clears_push(FloorResult *result)
{
    uint8_t push, up_x, up_y;

    push = RD8(ZP->player_push);
    WR8(ZP->player_push, (uint8_t)(push & (uint8_t)~PUSH_ON_FLOOR));
    up_x = RD8(ZP->fieldca);
    result->field_ca = up_x;
    result->a = up_x;
    if (!(up_x & 0x80)) {
        up_y = rd8(zPushYHi);
        result->a = up_y;
        if (up_y & 0x80) {
            return;
        }
    }
    push = RD8(ZP->player_push);
    WR8(ZP->player_push, (uint8_t)(push & (uint8_t)~PUSH_LIFTING));
}

/*
 * @routine 01BC:2BC RichterFloorOddScreen
 * @run 6D:BABC
 * WHAT IT DOES: The floor check's answer on an odd screen across: a floor at the first column,
 * its depth cleared, as the floor check finds one.
 * HOW: Sits before RichterFloorCheck, which branches back to it, and goes on into its found
 * floor's end.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterFloorCheck finding a floor at column 0.
 */
static FloorResult floor_odd_screen(void)
{
    FloorResult result = { FLOOR_HIGH_ON_ODD_SCREEN, 0, 0, 0, 0 };

    wr8(RichterColumnRow, 0);
    result.push_before = 0;
    floor_clears_push(&result);
    return result;
}

FloorResult RichterFloorOddScreen(void)
{
    return floor_odd_screen();
}

void abi_RichterFloorOddScreen(void)
{
    floor_check_exit_shim(RichterFloorOddScreen());
}

/*
 * @routine 01E0:5D8 MariaFloorOddScreen
 * @run 6D:BDD8
 * WHAT IT DOES: RichterFloorOddScreen in Maria's overlay: a floor at the first column on an odd screen across.
 * HOW: The same instructions. Sits before MariaFloorCheck, which branches back to it.
 * WAITS: no.
 * Entry, exit: as RichterFloorOddScreen.
 */
FloorResult MariaFloorOddScreen(void)
{
    return floor_odd_screen();
}

void abi_MariaFloorOddScreen(void)
{
    floor_check_exit_shim(MariaFloorOddScreen());
}

/*
 * @routine 01BC:2C2 RichterFloorCheck
 * @run 6D:BAC2
 * WHAT IT DOES: Says whether the columns read have a floor under Richter, and at which: on
 * odd screens across, always at the first, with its depth cleared. A push that was already
 * carrying him counts as floor too.
 * HOW: Bits $81 of the first three columns, but bit 7 alone counts only for the first.
 * WAITS: no.
 * Entry: nothing. Exit: C the answer, X the column. A, N, Z and V as the path left them.
 */
static FloorResult floor_check(FloorResult (*odd_screen)(void))
{
    FloorResult result = { FLOOR_NONE, 0, 0, 0, 0 };
    uint8_t hit, push;

    hit = RD8(OBJ->x_hi[0]);
    if (hit & 1) {
        return odd_screen();
    }
    for (uint8_t column = 0; column < FLOOR_COLUMNS; column++) {
        hit = (uint8_t)(rd8((uint16_t)(RichterColumnHit + column)) & TILE_FLOOR);
        result.a = hit;
        if (hit != 0 && (!(hit & 0x80) || column == 0)) {
            result.how = FLOOR_FOUND;
            result.column = column;
            floor_clears_push(&result);
            return result;
        }
    }
    push = RD8(ZP->player_push);
    result.push_before = push;
    if (push & PUSH_ON_FLOOR) {
        result.field_ca = RD8(ZP->fieldca);
        wr8(RichterColumnRow, result.field_ca);
        result.how = FLOOR_FROM_PUSH;
        return result;
    }
    push = RD8(ZP->player_push);
    WR8(ZP->player_push, (uint8_t)(push & (uint8_t)~PUSH_ON_FLOOR));
    push = RD8(ZP->player_push);
    WR8(ZP->player_push, (uint8_t)(push & (uint8_t)~PUSH_LIFTING));
    return result;
}

void floor_check_exit_shim(FloorResult result)
{
    uint8_t before = result.push_before;

    if (result.how == FLOOR_NONE) {
        cpu.a = result.a;
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
        cpu.a = result.field_ca;
    } else {
        cpu.a = result.a;
    }
    abi_nz(cpu.a);
    abi_flag(P_C, true);
}

FloorResult RichterFloorCheck(void)
{
    return floor_check(RichterFloorOddScreen);
}

void abi_RichterFloorCheck(void)
{
    floor_check_exit_shim(RichterFloorCheck());
}

/*
 * @routine 01E0:5DE MariaFloorCheck
 * @run 6D:BDDE
 * WHAT IT DOES: RichterFloorCheck in Maria's overlay: whether the columns read have a floor under her, and at which.
 * HOW: The same instructions, branching back to MariaFloorOddScreen on an odd screen.
 * WAITS: no.
 * Entry, exit: as RichterFloorCheck.
 */
FloorResult MariaFloorCheck(void)
{
    return floor_check(MariaFloorOddScreen);
}

void abi_MariaFloorCheck(void)
{
    floor_check_exit_shim(MariaFloorCheck());
}

/*
 * One column's point read a fixed number of rows below him, as the low column check, the floor and the solid check make it:
 * the offset (a call), its add to his X, his Y plus the rows, and the read (a call) entered with that add's A and flags.
 */
static ColumnRead column_probe(uint8_t column, uint8_t below, uint16_t ret_offset, uint16_t ret_read,
                               const ColumnsLayout *layout)
{
    ColumnRead one = { column, 0, below, false, { TILE_EMPTY_PAGE, 0, 0, 0, 0, 0 }, 0, false };
    ColumnOffset offset;
    Sum across, row;
    uint8_t base;

    jsr_push(ret_offset);
    offset = layout->offset(column);
    jsr_pop();
    base = RD8(OBJ->x_lo[0]);
    across = add_bytes(offset.value, base, false);
    WR8(ZP->temp00, across.value);
    base = RD8(OBJ->y_lo[0]);
    row = add_bytes(base, below, false);
    read_entry_shim(row, column, offset.index);
    jsr_push(ret_read);
    one.read = layout->read_below(row.value, row.carry);
    jsr_pop();
    one.row = base;
    one.read_made = !row.carry;
    one.index = offset.index;
    one.v = row.overflow;
    return one;
}

/*
 * @routine 01BC:2FF RichterLowColumnsBlocked
 * @run 6D:BAFF
 * WHAT IT DOES: Reads the two columns lowest under Richter and says whether either blocks.
 * HOW: Each is read $12 below him, and bits $24 of the answer tell.
 * WAITS: no.
 * Entry: nothing. Exit: at one, A those bits, X the column; else A 0, X 2, C 1, Z 1. Y and V
 * from the last read.
 */
static LowColumns low_columns_blocked(int columns, uint16_t offset_ret, uint16_t read_ret, const ColumnsLayout *layout)
{
    LowColumns low = { -1, false, { TILE_EMPTY_PAGE, 0, 0, 0, 0, 0 }, 0, 0, 0, false };
    ColumnRead one;

    for (int column = 0; column < columns; column++) {
        one = column_probe((uint8_t)column, COLUMNS_BELOW_LOW, offset_ret, read_ret, layout);
        low.row = one.row;
        low.read_made = one.read_made;
        low.last = one.read;
        low.index = one.index;
        low.v = one.v;
        low.answer = one.read_made ? tile_answer(one.read) : 0;
        if (low.answer & TILE_BLOCKS) {
            low.blocked_column = column;
            return low;
        }
    }
    return low;
}

LowColumns RichterLowColumnsBlocked(void)
{
    return low_columns_blocked(BLOCKED_COLUMNS, 0xBB03, 0xBB12, &richter_columns);
}

static void low_columns_exit_columns(LowColumns low, int columns)
{
    if (low.read_made) {
        terrain_read_exit_shim(low.last);
    } else {
        cpu.y = low.index;
        abi_flag(P_V, low.v);
    }
    if (low.blocked_column >= 0) {
        cpu.x = (uint8_t)low.blocked_column;
        cpu.a = low.answer & TILE_BLOCKS;
        abi_nz(cpu.a);
    } else {
        cpu.x = (uint8_t)columns;
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, true);
    }
}

void low_columns_exit_shim(LowColumns low)
{
    low_columns_exit_columns(low, BLOCKED_COLUMNS);
}

void abi_RichterLowColumnsBlocked(void)
{
    low_columns_exit_shim(RichterLowColumnsBlocked());
}

/*
 * @routine 01E0:61B MariaLowColumnsBlocked
 * @run 6D:BE1B
 * WHAT IT DOES: RichterLowColumnsBlocked in Maria's player overlay (01DD), over three columns, not two.
 * HOW: RichterLowColumnsBlocked's body with her column offset and read (MariaColumnOffset, MariaReadBelow) and the
 * count her CPX compares with.
 * Untraced: the return at a blocked column ($BE32); no script had a low column block under her.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterLowColumnsBlocked, X 3 when no column blocks.
 */
LowColumns MariaLowColumnsBlocked(void)
{
    return low_columns_blocked(MARIA_BLOCKED_COLUMNS, 0xBE1F, 0xBE2E, &maria_columns);
}

void abi_MariaLowColumnsBlocked(void)
{
    low_columns_exit_columns(MariaLowColumnsBlocked(), MARIA_BLOCKED_COLUMNS);
}

/*
 * @routine 01BC:3C1 RichterFloor
 * @run 6D:BBC1
 * WHAT IT DOES: Asks whether Richter stands on a floor, and lifts him out of it: by how deep
 * the column read, but not above the top of the map. A push from below does not count.
 * HOW: RichterReadColumns and RichterFloorCheck, then the column once more $0E below him.
 * WAITS: no.
 * Entry: X kept unless a column was found. Exit: A 1 and N 0, Z 0 on a floor, else A 0, Z set.
 * C from the lift, V as the last step left it. Clobbers zTemp00-03, zTemp10.
 */
/* A character's overlay's floor probe: the returns of its four calls, the copies it calls, and the Y it sets past
 * the top of the map. */
typedef struct {
    uint16_t columns_return, check_return, offset_return, read_return;
    ColumnsEnd (*columns)(void);
    FloorResult (*check)(void);
    const ColumnsLayout *probe;
    uint8_t top_y;
} FloorLayout;

static const FloorLayout richter_floor = {
    0xBBC9, 0xBBCC, 0xBBD1, 0xBBE0, RichterReadColumns, RichterFloorCheck, &richter_columns, FLOOR_TOP_OF_MAP
};
static const FloorLayout maria_floor = {
    0xBEED, 0xBEF0, 0xBEF5, 0xBF04, MariaReadColumns, MariaFloorCheck, &maria_columns, 0
};

static FloorProbe floor_probe(const FloorLayout *layout)
{
    FloorProbe probe = { 0 };
    ColumnRead one;
    uint8_t value, answer, y_lo, y_hi, push;
    Sum low, high;

    value = rd8(PlayerMayLeaveTop);
    if (value != 0) {
        probe.how = FLOOR_BARRED;
        return probe;
    }
    jsr_push(layout->columns_return);
    probe.columns = layout->columns();
    jsr_pop();
    jsr_push(layout->check_return);
    probe.check = layout->check();
    jsr_pop();
    if (probe.check.how == FLOOR_NONE) {
        probe.how = FLOOR_NOTHING_UNDER;
        return probe;
    }
    one = column_probe(probe.check.column, COLUMNS_BELOW_CHECK, layout->offset_return, layout->read_return, layout->probe);
    probe.row = one.row;
    probe.read_made = one.read_made;
    probe.read = one.read;
    probe.index = one.index;
    probe.probe_v = one.v;
    if (one.read_made) {
        answer = (uint8_t)(tile_answer(one.read) & TILE_FLOOR);
        if (answer != 0 && !(answer & 0x80)) {
            probe.how = FLOOR_COLUMN_SOLID;
            return probe;
        }
    }
    WR8(ZP->temp10, 0);
    value = rd8((uint16_t)(RichterColumnRow + probe.check.column));
    if (value & 0x80) {
        DEC8(ZP->temp10);
    }
    y_lo = RD8(OBJ->y_lo[0]);
    value = rd8((uint16_t)(RichterColumnRow + probe.check.column));
    low = sub_bytes(y_lo, value, true);
    WR8(OBJ->y_lo[0], low.value);
    y_hi = RD8(OBJ->y_hi[0]);
    value = RD8(ZP->temp10);
    high = sub_bytes(y_hi, value, low.carry);
    WR8(OBJ->y_hi[0], high.value);
    probe.lift_c = high.carry;
    probe.lift_v = high.overflow;
    if (high.value & 0x80) {
        WR8(OBJ->y_lo[0], layout->top_y);
        WR8(OBJ->y_hi[0], 0);
        probe.how = FLOOR_OFF_THE_TOP;
        return probe;
    }
    push = RD8(ZP->player_push);
    probe.push = push;
    probe.how = (push & PUSH_LIFTING) ? FLOOR_LANDED_PUSHED : FLOOR_LANDED;
    return probe;
}

FloorProbe RichterFloor(void)
{
    return floor_probe(&richter_floor);
}

void floor_exit_shim(FloorProbe probe)
{
    if (probe.how == FLOOR_BY_ORIGINAL) {
        return;
    }
    if (probe.how == FLOOR_BARRED) {
        cpu.a = 0;
        abi_nz(0);
        return;
    }
    if (!(probe.read_made && probe.read.kind == TILE_BY_ORIGINAL)) {
        columns_exit_shim(probe.columns);
        floor_check_exit_shim(probe.check);
    }
    if (probe.how == FLOOR_NOTHING_UNDER) {
        cpu.a = 0;
        abi_nz(0);
        return;
    }
    if (probe.read_made) {
        terrain_read_exit_shim(probe.read);
    } else {
        cpu.y = probe.index;
        abi_flag(P_V, probe.probe_v);
    }
    if (probe.how == FLOOR_COLUMN_SOLID) {
        cpu.a = 0;
        abi_nz(0);
        return;
    }
    abi_flag(P_C, probe.lift_c);
    abi_flag(P_V, probe.lift_v);
    if (probe.how != FLOOR_OFF_THE_TOP) {
        abi_flag(P_V, (probe.push & 0x40) != 0);
    }
    cpu.a = probe.how == FLOOR_LANDED_PUSHED ? 0 : 1;
    abi_nz(cpu.a);
}

void abi_RichterFloor(void)
{
    floor_exit_shim(RichterFloor());
}

/*
 * @routine 01E0:6E5 MariaFloor
 * @run 6D:BEE5
 * WHAT IT DOES: RichterFloor in Maria's overlay: whether she stands on a floor, and lifts her out of it; a lift past
 * the top of the map leaves her Y at 0, not 2.
 * HOW: RichterFloor's instructions calling MariaReadColumns, MariaFloorCheck, MariaColumnOffset and MariaReadBelow.
 * WAITS: no.
 * Entry, exit: as RichterFloor.
 */
FloorProbe MariaFloor(void)
{
    return floor_probe(&maria_floor);
}

void abi_MariaFloor(void)
{
    floor_exit_shim(MariaFloor());
}

uint8_t column_answer(ColumnRead one)
{
    return one.read_made ? tile_answer(one.read) : 0;
}

void column_read_exit_shim(ColumnRead one)
{
    if (one.read_made) {
        terrain_read_exit_shim(one.read);
    } else {
        cpu.y = one.index;
        abi_flag(P_V, one.v);
    }
}

/*
 * @routine 01BA:22F RichterSolidUnderColumn
 * @run 6D:AA2F
 * WHAT IT DOES: Says whether one column of terrain, $0E below Richter, is solid to stand on:
 * a tile with bit 7 set does not count.
 * HOW: Bit 0 of the answer, unless bit 7 is set.
 * WAITS: no.
 * Entry: X column. Exit: A the answer, N and Z from it. X kept; Y, C and V from the read.
 */
ColumnRead RichterSolidUnderColumn(uint8_t column)
{
    return column_probe(column, COLUMNS_BELOW_CHECK, 0xAA32, 0xAA41, &richter_columns);
}

uint8_t solid_answer(ColumnRead one)
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
 * @routine 01DE:1C9 MariaSolidUnderColumn
 * @run 6D:A9C9
 * WHAT IT DOES: RichterSolidUnderColumn in Maria's player overlay (01DD).
 * HOW: The same instructions, calling MariaColumnOffset and MariaReadBelow.
 * WAITS: no.
 * Entry, exit: as RichterSolidUnderColumn.
 */
ColumnRead MariaSolidUnderColumn(uint8_t column)
{
    return column_probe(column, COLUMNS_BELOW_CHECK, 0xA9CC, 0xA9DB, &maria_columns);
}

void abi_MariaSolidUnderColumn(void)
{
    ColumnRead one = MariaSolidUnderColumn(cpu.x);

    column_read_exit_shim(one);
    cpu.a = solid_answer(one);
    abi_nz(cpu.a);
}

/*
 * @routine 01BA:248 RichterReadUnderFeet
 * @run 6D:AA48
 * WHAT IT DOES: Reads the terrain straight under his feet, $1E below his position.
 * HOW: His own X with no column's offset.
 * WAITS: no.
 * Entry: nothing. Exit: as the read.
 */
static ColumnRead read_under_feet(TileRead (*read_below)(uint8_t row, bool carry))
{
    ColumnRead one = { 0, 0, COLUMNS_BELOW_SOLES, false, { TILE_EMPTY_PAGE, 0, 0, 0, 0, 0 }, 0, false };
    uint8_t value, base;
    Sum row;

    value = RD8(OBJ->x_lo[0]);
    WR8(ZP->temp00, value);
    base = RD8(OBJ->y_lo[0]);
    row = add_bytes(base, COLUMNS_BELOW_SOLES, false);
    read_entry_kept_shim(row);
    one.read = read_below(row.value, row.carry);
    one.row = base;
    one.read_made = !row.carry;
    return one;
}

ColumnRead RichterReadUnderFeet(void)
{
    return read_under_feet(RichterReadBelow);
}

/* Past the screen the row's add leaves all four flags, and the CLA that follows changes none. */
void under_feet_exit_shim(ColumnRead one)
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
 * @routine 01DE:1E2 MariaReadUnderFeet
 * @run 6D:A9E2
 * WHAT IT DOES: RichterReadUnderFeet in Maria's player overlay (01DD).
 * HOW: The same instructions, its tail jump to MariaReadBelow.
 * WAITS: no.
 * Entry, exit: as RichterReadUnderFeet.
 */
ColumnRead MariaReadUnderFeet(void)
{
    return read_under_feet(MariaReadBelow);
}

void abi_MariaReadUnderFeet(void)
{
    under_feet_exit_shim(MariaReadUnderFeet());
}

/*
 * @routine 01BC:068 RichterUpHeld
 * @run 6D:B868
 * WHAT IT DOES: Says whether up is held.
 * HOW: Bit 6 of zPadHeld.
 * WAITS: no.
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
 * @routine 01E0:373 MariaUpHeld
 * @run 6D:BB73
 * WHAT IT DOES: RichterUpHeld in Maria's player overlay (01DD), the same bytes.
 * HOW: RichterUpHeld's body.
 * WAITS: no.
 * Entry, exit: as RichterUpHeld.
 */
bool MariaUpHeld(void)
{
    return RichterUpHeld();
}

void abi_MariaUpHeld(void)
{
    abi_flag(P_C, MariaUpHeld());
}

/*
 * @routine 01BC:07E RichterJumpPressed
 * @run 6D:B87E
 * WHAT IT DOES: Says whether the jump button was newly pressed.
 * HOW: Bit 0 of zPadPressed.
 * WAITS: no.
 * Entry: nothing. Exit: C the answer. All else kept.
 */
static bool jump_pressed_body(void)
{
    return (rd8(zPadPressed) & PAD_JUMP_PRESS) != 0;
}

bool RichterJumpPressed(void)
{
    return jump_pressed_body();
}

void abi_RichterJumpPressed(void)
{
    abi_flag(P_C, RichterJumpPressed());
}

/*
 * @routine 01E0:389 MariaJumpPressed
 * @run 6D:BB89
 * WHAT IT DOES: RichterJumpPressed in Maria's player overlay (01DD), the same instructions at her address.
 * HOW: RichterJumpPressed's body.
 * WAITS: no.
 * Entry, exit: as RichterJumpPressed.
 */
bool MariaJumpPressed(void)
{
    return jump_pressed_body();
}

void abi_MariaJumpPressed(void)
{
    abi_flag(P_C, MariaJumpPressed());
}

/*
 * @routine 01BC:085 RichterHasNoFooting
 * @run 6D:B885
 * WHAT IT DOES: Says whether Richter has nothing to stand on: no floor found, or the column
 * found is not solid under him.
 * HOW: RichterFloorCheck, then RichterSolidUnderColumn.
 * WAITS: no.
 * Entry: nothing. Exit: C the answer; A, X, Y and V as the last step left them.
 */
/* One character's copy of the footing check: its two calls' return addresses and its floor and column checks. */
typedef struct {
    uint16_t floor_ret, solid_ret;
    FloorResult (*floor)(void);
    ColumnRead (*solid)(uint8_t column);
} FootingLayout;

static FootingCheck has_no_footing(const FootingLayout *layout)
{
    FootingCheck footing = { { FLOOR_NONE } };

    jsr_push(layout->floor_ret);
    footing.check = layout->floor();
    jsr_pop();
    if (footing.check.how == FLOOR_NONE) {
        footing.no_floor = true;
        return footing;
    }
    footing.read_made = true;
    jsr_push(layout->solid_ret);
    footing.under = layout->solid(footing.check.column);
    jsr_pop();
    footing.no_floor = solid_answer(footing.under) != 0;
    return footing;
}

FootingCheck RichterHasNoFooting(void)
{
    static const FootingLayout richter = { 0xB888, 0xB88D, RichterFloorCheck, RichterSolidUnderColumn };

    return has_no_footing(&richter);
}

void footing_exit_shim(FootingCheck footing)
{
    if (!(footing.read_made && footing.under.read_made && footing.under.read.kind == TILE_BY_ORIGINAL)) {
        floor_check_exit_shim(footing.check);
    }
    if (footing.read_made) {
        column_read_exit_shim(footing.under);
        cpu.a = solid_answer(footing.under);
        abi_nz(cpu.a);
    }
    abi_flag(P_C, footing.no_floor);
}

void abi_RichterHasNoFooting(void)
{
    footing_exit_shim(RichterHasNoFooting());
}

/*
 * @routine 01E0:390 MariaHasNoFooting
 * @run 6D:BB90
 * WHAT IT DOES: RichterHasNoFooting in Maria's player overlay (01DD).
 * HOW: The same instructions with her checks, MariaFloorCheck and MariaSolidUnderColumn.
 * WAITS: no.
 * Entry, exit: as RichterHasNoFooting.
 */
FootingCheck MariaHasNoFooting(void)
{
    static const FootingLayout maria = { 0xBB93, 0xBB98, MariaFloorCheck, MariaSolidUnderColumn };

    return has_no_footing(&maria);
}

void abi_MariaHasNoFooting(void)
{
    footing_exit_shim(MariaHasNoFooting());
}

/*
 * @routine 01BC:0F2 RichterLadderPoint
 * @run 6D:B8F2
 * WHAT IT DOES: Sets the point the ladder check reads: his X, and his Y plus the offset in zTemp10-11, into zTemp00-03.
 * HOW: Two adds with the carry between them.
 * WAITS: no.
 * Entry: zTemp10-11 the offset. Exit: A the Y high byte of the point, N, Z, C and V from its add. X and Y kept.
 */
Sum RichterLadderPoint(void)
{
    uint8_t value, base;
    Sum low, high;

    value = RD8(OBJ->x_lo[0]);
    WR8(ZP->temp00, value);
    WR8(ZP->temp01, 0);
    base = RD8(OBJ->y_lo[0]);
    value = RD8(ZP->temp10);
    low = add_bytes(base, value, false);
    WR8(ZP->temp02, low.value);
    base = RD8(OBJ->y_hi[0]);
    value = RD8(ZP->temp11);
    high = add_bytes(base, value, low.carry);
    WR8(ZP->temp03, high.value);
    return high;
}

void abi_RichterLadderPoint(void)
{
    Sum high = RichterLadderPoint();

    cpu.a = high.value;
    abi_nz(high.value);
    abi_flag(P_C, high.carry);
    abi_flag(P_V, high.overflow);
}

/* The overlay's read may run its own code: it is entered with that add's A and flags (X and Y as they stand). */
static void ladder_entry_shim(Sum high)
{
    read_entry_kept_shim(high);
}

static LadderCheck ladder_end(LadderCheck check, LadderEnd how, uint8_t a, uint8_t nz, bool carry)
{
    check.how = how;
    check.a = a;
    check.nz = nz;
    check.c = carry;
    return check;
}

/*
 * @routine 01BC:093 RichterLadderCheck
 * @run 6D:B893
 * WHAT IT DOES: Says whether there is something to climb down from here: looks downward from
 * $18 below him, a tile at a time, for the first solid tile with one of bits 0 or 2 above it.
 * HOW: The section's camera mode settles it in two cases; low on the screen it cannot be.
 * WAITS: no.
 * Entry: nothing. Exit: C the answer. A, Y and V as the last step left them. X kept. Clobbers
 * zTemp00-03, zTemp10-11.
 */
LadderCheck RichterLadderCheck(void)
{
    LadderCheck check = { LADDER_CAMERA_NO };
    uint8_t value, answer;
    Sum high, step;

    value = rd8(CameraMode);
    if (value == CAMERA_NO_LADDER) {
        return ladder_end(check, LADDER_CAMERA_NO, value, 0, false);
    }
    value = rd8(CameraMode);
    if (value == CAMERA_LADDER) {
        return ladder_end(check, LADDER_CAMERA_YES, value, 0, true);
    }
    value = RD8(OBJ->y_hi[0]);
    if (value != 0) {
        return ladder_end(check, LADDER_OFF_SCREEN, value, value, false);
    }
    value = RD8(OBJ->y_lo[0]);
    if (value >= LADDER_LOWEST_Y) {
        return ladder_end(check, LADDER_TOO_LOW, value, (uint8_t)(value - LADDER_LOWEST_Y), false);
    }
    WR8(ZP->temp10, LADDER_FIRST_REACH);
    WR8(ZP->temp11, 0);
    jsr_push(0xB8B5);
    high = RichterLadderPoint();
    jsr_pop();
    for (bool first = true;; first = false) {
        if (high.value != 0) {
            check.v_set = true;
            check.v = high.overflow;
            return ladder_end(check, LADDER_OFF_PAGE, high.value, high.value, false);
        }
        ladder_entry_shim(high);
        jsr_push(first ? 0xB8BA : 0xB8C7);
        check.last = StageTileRead();
        jsr_pop();
        check.any_read = true;
        answer = (uint8_t)(tile_answer(check.last) & TILE_SOLID_BIT);
        if (answer != 0) {
            if (first) {
                return ladder_end(check, LADDER_BLOCKED, answer, answer, false);
            }
            break;
        }
        value = RD8(ZP->temp10);
        step = add_bytes(LADDER_STEP, value, false);
        WR8(ZP->temp10, step.value);
        if (step.carry) {
            check.v_set = true;
            check.v = step.overflow;
            return ladder_end(check, LADDER_REACH_OUT, step.value, step.value, false);
        }
        jsr_push(0xB8C2);
        high = RichterLadderPoint();
        jsr_pop();
    }
    value = RD8(ZP->temp10);
    step = add_bytes(0xF8, value, false);
    WR8(ZP->temp10, step.value);
    value = RD8(ZP->temp11);
    high = add_bytes(0xFF, value, step.carry);
    WR8(ZP->temp11, high.value);
    jsr_push(0xB8E5);
    high = RichterLadderPoint();
    jsr_pop();
    ladder_entry_shim(high);
    jsr_push(0xB8E8);
    check.last = StageTileRead();
    jsr_pop();
    answer = (uint8_t)(tile_answer(check.last) & TILE_LADDER_BITS);
    check.answer = answer;
    return ladder_end(check, LADDER_ANSWERED, answer, answer, answer == 0);
}

bool ladder_found(LadderCheck check)
{
    return check.how == LADDER_CAMERA_YES || (check.how == LADDER_ANSWERED && check.answer == 0);
}

void ladder_exit_shim(LadderCheck check)
{
    if (check.any_read) {
        terrain_read_exit_shim(check.last);
    }
    if (check.v_set) {
        abi_flag(P_V, check.v);
    }
    cpu.a = check.a;
    abi_nz(check.nz);
    abi_flag(P_C, check.c);
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
 * WAITS: no.
 * Entry: nothing. Exit: C the answer, with what the last step left.
 */
JumpOrClimb RichterJumpOrClimb(void)
{
    JumpOrClimb result = { false };

    jsr_push(0xB871);
    result.pressed = RichterJumpPressed();
    jsr_pop();
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
/*
 * @routine 01BC:241 RichterLedgeCheck
 * @run 6D:BA41
 * WHAT IT DOES: Says whether a low or a high column in front of him gives him a ledge to act
 * on, unless a push holds him. zTemp01 says which: 0 low, 1 high.
 * HOW: RichterLowColumnsBlocked with down held, RichterColumnsBlocked with up held; bit $20 of
 * the tile counts whatever the pad.
 * WAITS: no.
 * Entry: nothing. Exit: C the answer, with what the last step left.
 */
/*
 * @routine 01BC:250 RichterLedgeLow
 * @run 6D:BA50
 * WHAT IT DOES: The ledge check's low half: the two low columns read; a column that blocks counts when its bit $20 is set or
 * down is held, and then zTemp01 is cleared.
 * HOW: RichterLowColumnsBlocked, the answer's bit $20, bit 4 of zPadHeld.
 * WAITS: no.
 * Entry: nothing. Exit: C set at a ledge, A the bit looked at, X the column; else C clear (A 0, X 2 when no column blocks).
 */
LedgeLowPart RichterLedgeLow(void)
{
    LedgeLowPart r = { false, { -1 }, 0, BLOCKED_COLUMNS, false };
    uint8_t bits, held;

    jsr_push(0xBA53);
    r.low = RichterLowColumnsBlocked();
    jsr_pop();
    if (r.low.blocked_column < 0) {
        return r;
    }
    bits = (uint8_t)(r.low.answer & TILE_BLOCK_ANY);
    r.a = bits;
    r.x = (uint8_t)r.low.blocked_column;
    r.found = bits != 0;
    if (!r.found) {
        held = RD8(ZP->pad_held);
        r.found = (held & PAD_DOWN) != 0;
    }
    if (r.found) {
        WR8(ZP->temp01, 0);
        r.c = true;
    }
    return r;
}

void abi_RichterLedgeLow(void)
{
    LedgeLowPart r = RichterLedgeLow();

    low_columns_exit_shim(r.low);
    cpu.a = r.a;
    cpu.x = r.x;
    abi_nz(r.a);
    abi_flag(P_C, r.c);
}

/*
 * @routine 01BC:264 RichterLedgeHigh
 * @run 6D:BA64
 * WHAT IT DOES: The ledge check's high half: the first two columns read; a column that blocks counts when its bit $20 is set or
 * up is held, and then zTemp01 is set to 1.
 * HOW: RichterColumnsBlocked, the answer's bit $20, bit 6 of zPadHeld.
 * WAITS: no.
 * Entry: nothing. Exit: C set at a ledge with A 1, X the column; else C clear (A the bit looked at, X 2 when no column blocks).
 */
LedgeHighPart RichterLedgeHigh(void)
{
    LedgeHighPart r = { false, { -1, 0 }, 0, BLOCKED_COLUMNS, false };
    uint8_t bits, held;

    jsr_push(0xBA67);
    r.high = RichterColumnsBlocked();
    jsr_pop();
    if (r.high.column < 0) {
        return r;
    }
    bits = (uint8_t)(r.high.bits & TILE_BLOCK_ANY);
    r.a = bits;
    r.x = (uint8_t)r.high.column;
    r.found = bits != 0;
    if (!r.found) {
        held = RD8(ZP->pad_held);
        r.found = (held & PAD_UP) != 0;
    }
    if (r.found) {
        WR8(ZP->temp01, 1);
        r.a = 1;
        r.c = true;
    }
    return r;
}

void abi_RichterLedgeHigh(void)
{
    LedgeHighPart r = RichterLedgeHigh();

    cpu.a = r.a;
    cpu.x = r.x;
    abi_nz(r.a);
    abi_flag(P_C, r.c);
}

LedgeCheck RichterLedgeCheck(void)
{
    LedgeCheck check = { LEDGE_PUSHED };
    LedgeLowPart low;
    LedgeHighPart high;
    uint8_t push;

    push = RD8(ZP->player_push);
    check.push = push;
    check.high_column = -1;
    if (push & PUSH_ON_FLOOR) {
        return check;
    }
    jsr_push(0xBA48);
    low = RichterLedgeLow();
    jsr_pop();
    check.low = low.low;
    if (low.found) {
        check.how = LEDGE_LOW;
        check.a = low.a;
        check.x = low.x;
        check.c = true;
        return check;
    }
    jsr_push(0xBA4C);
    high = RichterLedgeHigh();
    jsr_pop();
    check.high_column = high.high.column;
    check.how = high.found ? LEDGE_HIGH : LEDGE_NONE;
    check.a = high.a;
    check.x = high.x;
    check.c = high.c;
    return check;
}

void ledge_check_exit_shim(LedgeCheck check)
{
    if (check.how == LEDGE_PUSHED) {
        abi_flag(P_V, (check.push & 0x40) != 0);
        abi_flag(P_N, (check.push & 0x80) != 0);
        abi_flag(P_Z, false);
        abi_flag(P_C, false);
        return;
    }
    low_columns_exit_shim(check.low);
    cpu.a = check.a;
    cpu.x = check.x;
    abi_nz(check.a);
    abi_flag(P_C, check.c);
}

void abi_RichterLedgeCheck(void)
{
    ledge_check_exit_shim(RichterLedgeCheck());
}

/*
 * One half of Maria's ledge check: what its column check found, and the A, X, N and Z and C it leaves. With three
 * columns her CPX #2 can be equal, so a column of bit $20 at the third column counts only with the pad, and the
 * compare's N and Z stand.
 */
typedef struct { bool blocked; bool found; LowColumns low; BlockedColumns high; uint8_t a, nz, x; bool c; } MariaLedgePart;

/* The bit $20 test shared by both halves: a column of bit $20 counts unless it is the third (CPX #2). */
static void maria_ledge_bits(MariaLedgePart *r, uint8_t bits, uint8_t column, uint8_t pad_bit)
{
    uint8_t held;

    r->blocked = true;
    r->x = column;
    r->a = (uint8_t)(bits & TILE_BLOCK_ANY);
    r->nz = r->a;
    if (r->a != 0) {
        r->nz = (uint8_t)(column - MARIA_THIRD_COLUMN);
        r->found = column != MARIA_THIRD_COLUMN;
    }
    if (!r->found) {
        held = RD8(ZP->pad_held);
        r->found = (held & pad_bit) != 0;
    }
}

/*
 * @routine 01E0:574 MariaLedgeLow
 * @run 6D:BD74
 * WHAT IT DOES: RichterLedgeLow in Maria's player overlay (01DD): her three low columns, the third counting only with
 * down held.
 * HOW: MariaLowColumnsBlocked, the answer's bit $20 and the column against 2, bit 4 of zPadHeld; zTemp01 cleared at a
 * ledge.
 * Untraced: everything after a column blocks ($BD79 to $BD87); no script had a low column block under her.
 * WAITS: no.
 * Entry: nothing. Exit: C set at a ledge; A the bit $20 looked at, X the column, N and Z from the bit or from the
 * column less 2. No column: as MariaLowColumnsBlocked with C clear.
 */
MariaLedgePart MariaLedgeLow(void)
{
    MariaLedgePart r = { false, false, { -1 }, { -1, 0 }, 0, 0, MARIA_BLOCKED_COLUMNS, false };

    jsr_push(0xBD77);
    r.low = MariaLowColumnsBlocked();
    jsr_pop();
    if (r.low.blocked_column < 0) {
        return r;
    }
    maria_ledge_bits(&r, r.low.answer, (uint8_t)r.low.blocked_column, PAD_DOWN);
    if (r.found) {
        WR8(ZP->temp01, 0);
        r.c = true;
    }
    return r;
}

static void maria_ledge_low_exit_shim(MariaLedgePart r)
{
    low_columns_exit_columns(r.low, MARIA_BLOCKED_COLUMNS);
    if (r.blocked) {
        cpu.a = r.a;
        cpu.x = r.x;
        abi_nz(r.nz);
    }
    abi_flag(P_C, r.c);
}

void abi_MariaLedgeLow(void)
{
    maria_ledge_low_exit_shim(MariaLedgeLow());
}

/*
 * @routine 01E0:588 MariaLedgeHigh
 * @run 6D:BD88
 * WHAT IT DOES: RichterLedgeHigh in Maria's player overlay (01DD): her first three columns, the third counting only
 * with up held.
 * HOW: MariaColumnsBlocked, the answer's bit $20 and the column against 2, bit 6 of zPadHeld; zTemp01 set to 1 at a
 * ledge.
 * Untraced: everything after a column blocks ($BD8D to $BD9D).
 * WAITS: no.
 * Entry: nothing. Exit: C set at a ledge with A 1, N and Z from it, X the column; else C clear, A the bit $20
 * looked at, N and Z from it or from the column less 2. No column: A 0, X 3, Z set, C clear.
 */
MariaLedgePart MariaLedgeHigh(void)
{
    MariaLedgePart r = { false, false, { -1 }, { -1, 0 }, 0, 0, MARIA_BLOCKED_COLUMNS, false };

    jsr_push(0xBD8B);
    r.high = MariaColumnsBlocked();
    jsr_pop();
    if (r.high.column < 0) {
        return r;
    }
    maria_ledge_bits(&r, r.high.bits, (uint8_t)r.high.column, PAD_UP);
    if (r.found) {
        r.a = 1;
        r.nz = 1;
        WR8(ZP->temp01, r.a);
        r.c = true;
    }
    return r;
}

static void maria_ledge_high_exit_shim(MariaLedgePart r)
{
    cpu.a = r.a;
    cpu.x = r.x;
    abi_nz(r.nz);
    abi_flag(P_C, r.c);
}

void abi_MariaLedgeHigh(void)
{
    maria_ledge_high_exit_shim(MariaLedgeHigh());
}

/* Maria's ledge check: the push, then her low half and her high half. */
typedef struct { LedgeEnd how; uint8_t push; MariaLedgePart low, high; } MariaLedge;

/*
 * @routine 01E0:565 MariaLedgeCheck
 * @run 6D:BD65
 * WHAT IT DOES: RichterLedgeCheck in Maria's player overlay (01DD), on her halves, MariaLedgeLow and MariaLedgeHigh.
 * HOW: The same instructions at her addresses.
 * Untraced: the return while a push holds her ($BD72 from the TST) and the two returns at a ledge ($BD9C).
 * WAITS: no.
 * Entry: nothing. Exit: C the answer, with what the last half left; held by a push, N and V the push's bits 7 and 6,
 * Z and C clear.
 */
MariaLedge MariaLedgeCheck(void)
{
    MariaLedge check = { LEDGE_PUSHED };

    check.push = RD8(ZP->player_push);
    if (check.push & PUSH_ON_FLOOR) {
        return check;
    }
    jsr_push(0xBD6C);
    check.low = MariaLedgeLow();
    jsr_pop();
    if (check.low.c) {
        check.how = LEDGE_LOW;
        return check;
    }
    jsr_push(0xBD70);
    check.high = MariaLedgeHigh();
    jsr_pop();
    check.how = check.high.c ? LEDGE_HIGH : LEDGE_NONE;
    return check;
}

static void maria_ledge_check_exit_shim(MariaLedge check)
{
    if (check.how == LEDGE_PUSHED) {
        abi_flag(P_V, (check.push & 0x40) != 0);
        abi_flag(P_N, (check.push & 0x80) != 0);
        abi_flag(P_Z, false);
        abi_flag(P_C, false);
        return;
    }
    maria_ledge_low_exit_shim(check.low);
    if (check.how != LEDGE_LOW) {
        maria_ledge_high_exit_shim(check.high);
    }
}

void abi_MariaLedgeCheck(void)
{
    maria_ledge_check_exit_shim(MariaLedgeCheck());
}

/*
 * @routine 01BC:280 RichterLiftByColumn
 * @run 6D:BA80
 * WHAT IT DOES: Lifts him out of the floor by how deep this column read, and not above the top
 * of the map.
 * HOW: The depth in RichterColumnRow, signed, which is cleared.
 * WAITS: no.
 * Entry: X column. Exit: A his Y high byte, or 2 at the top; N and Z from it; C and V from the
 * subtract. X and Y kept. Clobbers zTemp10.
 */
ColumnLift RichterLiftByColumn(uint8_t column)
{
    uint16_t depth_field = (uint16_t)(RichterColumnRow + column);
    ColumnLift lift = { false, 0, 0, 0, false, false };
    uint8_t value, y_lo, y_hi;
    Sum low, high;

    WR8(ZP->temp10, 0);
    value = rd8(depth_field);
    if (value & 0x80) {
        DEC8(ZP->temp10);
    }
    y_lo = RD8(OBJ->y_lo[0]);
    value = rd8(depth_field);
    low = sub_bytes(y_lo, value, true);
    WR8(OBJ->y_lo[0], low.value);
    y_hi = RD8(OBJ->y_hi[0]);
    value = RD8(ZP->temp10);
    high = sub_bytes(y_hi, value, low.carry);
    WR8(OBJ->y_hi[0], high.value);
    lift.c = high.carry;
    lift.v = high.overflow;
    if (high.value & 0x80) {
        WR8(OBJ->y_lo[0], FLOOR_TOP_OF_MAP);
        WR8(OBJ->y_hi[0], 0);
        lift.off_the_top = true;
        lift.a = FLOOR_TOP_OF_MAP;
    } else {
        lift.a = high.value;
    }
    wr8(depth_field, 0);
    return lift;
}

void column_lift_exit_shim(ColumnLift lift)
{
    cpu.a = lift.a;
    abi_nz(cpu.a);
    abi_flag(P_C, lift.c);
    abi_flag(P_V, lift.v);
}

void abi_RichterLiftByColumn(void)
{
    column_lift_exit_shim(RichterLiftByColumn(cpu.x));
}

/*
 * @routine 01E0:5A4 MariaLiftByColumn
 * @run 6D:BDA4
 * WHAT IT DOES: Lifts Maria out of the floor by how deep this column read; above the top of the map she is put at 0.
 * HOW: Her Y less the column's depth in RichterColumnRow, signed. Unlike RichterLiftByColumn the depth is not
 * cleared, and a lift past the top leaves her Y at 0, not 2.
 * WAITS: no.
 * Entry: X column. Exit: A her Y high byte, N and Z from it; C and V from the subtract. X and Y kept. Clobbers zTemp10.
 */
ColumnLift MariaLiftByColumn(uint8_t column)
{
    uint16_t depth_field = (uint16_t)(RichterColumnRow + column);
    ColumnLift lift = { false, 0, 0, 0, false, false };
    uint8_t value, y_lo, y_hi;
    Sum low, high;

    WR8(ZP->temp10, 0);
    value = rd8(depth_field);
    if (value & 0x80) {
        DEC8(ZP->temp10);
    }
    y_lo = RD8(OBJ->y_lo[0]);
    value = rd8(depth_field);
    low = sub_bytes(y_lo, value, true);
    WR8(OBJ->y_lo[0], low.value);
    y_hi = RD8(OBJ->y_hi[0]);
    value = RD8(ZP->temp10);
    high = sub_bytes(y_hi, value, low.carry);
    WR8(OBJ->y_hi[0], high.value);
    lift.c = high.carry;
    lift.v = high.overflow;
    lift.a = high.value;
    if (high.value & 0x80) {
        WR8(OBJ->y_lo[0], 0);
        WR8(OBJ->y_hi[0], 0);
        lift.off_the_top = true;
    }
    return lift;
}

void abi_MariaLiftByColumn(void)
{
    column_lift_exit_shim(MariaLiftByColumn(cpu.x));
}

/*
 * @routine 01BC:27A RichterLandOnFloor
 * @run 6D:BA7A
 * WHAT IT DOES: If there is a floor under him, lifts him out of it.
 * HOW: RichterFloorCheck, then runs on into RichterLiftByColumn.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterLiftByColumn, or as RichterFloorCheck with no floor.
 */
static LandingLift land_on_floor(uint16_t floor_ret, FloorResult (*floor)(void), ColumnLift (*lift)(uint8_t column))
{
    LandingLift landing = { { FLOOR_NONE } };

    jsr_push(floor_ret);
    landing.check = floor();
    jsr_pop();
    if (landing.check.how == FLOOR_NONE)
        return landing;
    landing.lifted = true;
    landing.lift = lift(landing.check.column);
    return landing;
}

LandingLift RichterLandOnFloor(void)
{
    return land_on_floor(0xBA7D, RichterFloorCheck, RichterLiftByColumn);
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

/*
 * @routine 01E0:59E MariaLandOnFloor
 * @run 6D:BD9E
 * WHAT IT DOES: RichterLandOnFloor in Maria's player overlay (01DD).
 * HOW: The same instructions with her checks: MariaFloorCheck, then a branch into MariaLiftByColumn.
 * WAITS: no.
 * Entry, exit: as RichterLandOnFloor.
 */
LandingLift MariaLandOnFloor(void)
{
    return land_on_floor(0xBDA1, MariaFloorCheck, MariaLiftByColumn);
}

void abi_MariaLandOnFloor(void)
{
    landing_exit_shim(MariaLandOnFloor());
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

    if (resident_at(PlayerOvlWall) == DISC(0x01B9, 0x021)) {
        return RichterWall(side);
    }
    if (resident_at(PlayerOvlWall) == DISC(0x01DD, 0x021)) {
        return MariaWall(side);
    }
    other.answer = player_overlay_shim(PlayerOvlWall, true, side);
    return other;
}

FloorProbe PlayerFloor(void)
{
    FloorProbe other = { FLOOR_BY_ORIGINAL };

    if (resident_at(PlayerOvlFloor) == DISC(0x01B9, 0x024)) {
        return RichterFloor();
    }
    if (resident_at(PlayerOvlFloor) == DISC(0x01DD, 0x024)) {
        return MariaFloor();
    }
    other.answer = player_overlay_shim(PlayerOvlFloor, false, 0);
    return other;
}

ColumnsEnd PlayerMoved(void)
{
    ColumnsEnd other = { true };

    if (resident_at(PlayerOvlMoved) == DISC(0x01B9, 0x006)) {
        return RichterReadColumns();
    }
    if (resident_at(PlayerOvlMoved) == DISC(0x01DD, 0x006)) {
        return MariaReadColumns();
    }
    player_overlay_shim(PlayerOvlMoved, false, 0);
    return other;
}

/*
 * Maria's own probes (her overlay 01DD): three columns read for the tile's bit $02, above her head and below her
 * feet. PlayerByte3BE4 has no symbol yet: it is named PlayerByte3BE3 plus one until the row lands.
 */
enum { TILE_BIT_02 = 0x02, MARIA_ROWS_ABOVE = 0xFC, MARIA_ROWS_BELOW = 0x04, MARIA_FEET_MARKED = PlayerByte3BE3 + 1 };

/* One of her three columns read for bit $02: what the read left, and the bit. */
typedef struct {
    int column;           /* the column of bit $02, or -1 */
    uint8_t index;        /* MariaColumnOffset's Y */
    bool above;           /* read by MariaReadAbove, not MariaReadBelow */
    bool on_map;          /* the row's add kept it on the map */
    TileRead last;
    uint8_t answer;       /* A after the read */
} MarkedColumns;

/* The three columns: the offset (nudged a pixel inward for the read above), her X, her Y plus `rows`, the read. */
static MarkedColumns marked_columns(bool above, uint8_t rows, uint16_t offset_ret, uint16_t read_ret)
{
    MarkedColumns r = { -1, 0, above, false, { TILE_EMPTY_PAGE, 0, 0, 0, 0, 0 }, 0 };
    ColumnOffset offset;
    Sum across, row;
    uint8_t value, base;

    for (int column = 0; column < MARIA_BLOCKED_COLUMNS; column++) {
        jsr_push(offset_ret);
        offset = MariaColumnOffset((uint8_t)column);
        jsr_pop();
        value = offset.value;
        if (above && value != 0) {
            value = (value & 0x80) ? (uint8_t)(value + 1) : (uint8_t)(value - 1);
        }
        base = RD8(OBJ->x_lo[0]);
        across = add_bytes(value, base, false);
        WR8(ZP->temp00, across.value);
        base = RD8(OBJ->y_lo[0]);
        row = add_bytes(base, rows, false);
        read_entry_shim(row, (uint8_t)column, offset.index);
        r.index = offset.index;
        jsr_push(read_ret);
        if (above) {
            r.on_map = row.carry;
            r.last = MariaReadAbove(row.value, row.carry);
        } else {
            r.on_map = !row.carry;
            r.last = MariaReadBelow(row.value, row.carry);
        }
        jsr_pop();
        r.answer = r.on_map ? tile_answer(r.last) : 0;
        if (r.answer & TILE_BIT_02) {
            r.column = column;
            return r;
        }
    }
    return r;
}

/* The registers the last read left, then A the bit: at the column of the bit, X the column; else X 3, Z and C set. */
static void marked_columns_exit_shim(MarkedColumns r)
{
    cpu.y = r.index;
    if (r.above) {
        read_above_exit_shim(r.last, r.on_map);
    } else {
        read_or_nothing_exit_shim(r.last, r.on_map);
    }
    cpu.a = (uint8_t)(r.answer & TILE_BIT_02);
    abi_nz(cpu.a);
    if (r.column >= 0) {
        cpu.x = (uint8_t)r.column;
        return;
    }
    cpu.x = MARIA_BLOCKED_COLUMNS;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 01E0:325 MariaMarkedAbove
 * @run 6D:BB25
 * WHAT IT DOES: Reads her three columns four rows above her, each a pixel nearer her middle, and says whether one
 * has the tile's bit $02.
 * HOW: MariaColumnOffset with the offset nudged toward zero, MariaReadAbove.
 * Untraced: the return at a column of bit $02 ($BB44).
 * WAITS: no.
 * Entry: nothing. Exit: at that column, A 2, N and Z from it, X the column; else A 0, X 3, Z and C set. Y, C and V
 * as the last read left them. Clobbers zTemp00-03.
 */
MarkedColumns MariaMarkedAbove(void)
{
    return marked_columns(true, MARIA_ROWS_ABOVE, 0xBB29, 0xBB40);
}

void abi_MariaMarkedAbove(void)
{
    marked_columns_exit_shim(MariaMarkedAbove());
}

/*
 * @routine 01E0:34B MariaFeetMarked
 * @run 6D:BB4B
 * WHAT IT DOES: Reads her three columns four rows below her and notes in PlayerByte3BE4 whether one has the tile's
 * bit $02.
 * HOW: MariaColumnOffset, MariaReadBelow; the byte cleared first and incremented at such a column.
 * Untraced: the count at a column of bit $02 ($BB6B).
 * WAITS: no.
 * Entry: nothing. Exit: at that column, A 2, X the column, N and Z from the increment; else A 0, X 3, Z and C set.
 * Y, C and V as the last read left them. Clobbers zTemp00-03.
 */
MarkedColumns MariaFeetMarked(void)
{
    MarkedColumns r;

    wr8(MARIA_FEET_MARKED, 0);
    r = marked_columns(false, MARIA_ROWS_BELOW, 0xBB52, 0xBB61);
    if (r.column >= 0) {
        wr8(MARIA_FEET_MARKED, (uint8_t)(rd8(MARIA_FEET_MARKED) + 1));
    }
    return r;
}

void abi_MariaFeetMarked(void)
{
    MarkedColumns r = MariaFeetMarked();

    marked_columns_exit_shim(r);
    if (r.column >= 0) {
        abi_nz(1);
    }
}

/* Up held, or a column of bit $02 above her: the probe and the pad. */
typedef struct { MarkedColumns above; bool c; } UpOrMarked;

/*
 * @routine 01E0:36F MariaUpOrMarkedAbove
 * @run 6D:BB6F
 * WHAT IT DOES: Says whether a column above her has the tile's bit $02, or else whether up is held.
 * HOW: MariaMarkedAbove, then on into MariaUpHeld.
 * Untraced: the answer at a column of bit $02 ($BB71 taken).
 * WAITS: no.
 * Entry: nothing. Exit: C the answer; A, X, Y, N, Z and V as MariaMarkedAbove left them.
 */
UpOrMarked MariaUpOrMarkedAbove(void)
{
    UpOrMarked r;

    jsr_push(0xBB71);
    r.above = MariaMarkedAbove();
    jsr_pop();
    r.c = r.above.column >= 0 || MariaUpHeld();
    return r;
}

void abi_MariaUpOrMarkedAbove(void)
{
    UpOrMarked r = MariaUpOrMarkedAbove();

    marked_columns_exit_shim(r.above);
    abi_flag(P_C, r.c);
}
