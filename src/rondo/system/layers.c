/* Background layers: the scroll step each layer moves by in a frame. */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "layers.h"

/* The exit of a routine that ends in a ROR of the low byte: A, N and Z from it, C the bit shifted out. */
static void halved_exit_shim(ScrollStepHalved halved)
{
    cpu.a = halved.low;
    abi_nz(halved.low);
    abi_flag(P_C, halved.carry);
}

/* The exit of a routine whose last act was an add or subtract: A, N and Z from its value, C and V from it. */
static void sum_exit_shim(Sum sum)
{
    cpu.a = sum.value;
    abi_nz(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

/*
 * @routine 0515:132 HalveScrollStep
 * @run 6F:A132
 * WHAT IT DOES: Sets one entry of this frame's X scroll step to half another's, so a layer
 * behind the first moves at half its speed.
 * HOW: A signed 16-bit shift right: the high byte keeps its sign (the first read of it
 * only gives the sign, the second is shifted), and the bit it loses goes into the low byte's
 * top. The high byte is written before the low byte is read.
 * WAITS: no.
 * Entry: X the entry to halve, Y the entry that gets the result. Exit: A the low byte written, N
 * and Z from it, C the low bit shifted out. X and Y kept.
 */
ScrollStepHalved HalveScrollStep(uint8_t from, uint8_t to)
{
    ScrollStepHalved halved;
    uint8_t sign_byte, high, low;
    uint8_t sign;

    sign_byte = RD8(SCROLLE->step_x_hi[from]);
    sign = sign_byte >> 7;
    high = RD8(SCROLLE->step_x_hi[from]);
    WR8(SCROLLE->step_x_hi[to], (uint8_t)((high >> 1) | (sign << 7)));
    low = RD8(SCROLLE->step_x_lo[from]);
    halved.low = (uint8_t)((low >> 1) | ((high & 1) << 7));
    WR8(SCROLLE->step_x_lo[to], halved.low);
    halved.carry = (low & 1) != 0;
    return halved;
}

void abi_HalveScrollStep(void)
{
    halved_exit_shim(HalveScrollStep(cpu.x, cpu.y));
}

/*
 * @routine 07D5:131 HalveScrollStepFromFirst
 * @run 6F:A131
 * WHAT IT DOES: Sets entry Y of this frame's X scroll step to half of entry 0's.
 * HOW: Entry 0 into HalveScrollStep. The one byte is a CLX that runs on into it.
 * WAITS: no.
 * Entry: Y the entry that gets the result. Exit: as HalveScrollStep, X 0.
 */
ScrollStepHalved HalveScrollStepFromFirst(uint8_t to)
{
    return HalveScrollStep(0, to);
}

void abi_HalveScrollStepFromFirst(void)
{
    cpu.x = 0;
    halved_exit_shim(HalveScrollStepFromFirst(cpu.y));
}

/*
 * @routine 0515:12F HalveScrollStepFirstToSecond
 * @run 6F:A12F
 * WHAT IT DOES: Sets entry 1 of this frame's X scroll step to half of entry 0's.
 * HOW: Entry 0 to entry 1 through HalveScrollStep. The three bytes load Y and clear X, then
 * run on into it.
 * WAITS: no.
 * Entry: nothing. Exit: as HalveScrollStep, X 0 and Y 1.
 */
ScrollStepHalved HalveScrollStepFirstToSecond(void)
{
    return HalveScrollStep(0, 1);
}

void abi_HalveScrollStepFirstToSecond(void)
{
    cpu.y = 1;
    cpu.x = 0;
    halved_exit_shim(HalveScrollStepFirstToSecond());
}

/* Level screen map: 16 screens across, one byte each, at $3D00. */
enum { SCREEN_MAP_PAGE = 0x3D, SCREEN_MAP_ROW_SHIFT = 4 };
/* Fields with no name yet (rows requested from the owner of the records): the scroll's second word
 * and the list's 16-bit counters. */
enum { ScrollYWantedLo = 0x3EB8, ScrollYWantedHi = 0x3EB9 };

static void clear_scroll_speeds(void)
{
    for (int entry = 7; entry >= 0; entry--) {
        WR8(SCROLLE->speed_x[entry], 0);
        WR8(SCROLLE->speed_y[entry], 0);
    }
}

static void clear_scroll_speeds_exit_shim(void)
{
    cpu.x = 0xFF;
    abi_nz(0xFF);
}

/*
 * @routine 0255:067 ClearScrollSpeeds
 * @run 6F:A067
 * WHAT IT DOES: Stops every layer's scroll: the eight X speeds and the eight Y speeds go to zero.
 * HOW: A loop from entry 7 down to 0, the X column then the Y column each time.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N set, Z clear. A and C kept.
 */
void ClearScrollSpeeds(void)
{
    clear_scroll_speeds();
}

void abi_ClearScrollSpeeds(void)
{
    ClearScrollSpeeds();
    clear_scroll_speeds_exit_shim();
}

/*
 * @routine 10C5:3F7 ClearScrollSpeedsB
 * @run 6F:A3F7
 * WHAT IT DOES: ClearScrollSpeeds, as the stage at 10C1 has its own copy of it.
 * HOW: The same body, shared through clear_scroll_speeds.
 * WAITS: no.
 * Entry, exit: as ClearScrollSpeeds.
 */
void ClearScrollSpeedsB(void)
{
    clear_scroll_speeds();
}

void abi_ClearScrollSpeedsB(void)
{
    ClearScrollSpeedsB();
    clear_scroll_speeds_exit_shim();
}

/*
 * @routine 12D5:3F0 ClearScrollSpeedsC
 * @run 6F:A3F0
 * WHAT IT DOES: ClearScrollSpeeds, as the stage at 1291 has its own copy of it.
 * HOW: The same body, shared through clear_scroll_speeds.
 * WAITS: no.
 * Entry, exit: as ClearScrollSpeeds.
 */
void ClearScrollSpeedsC(void)
{
    clear_scroll_speeds();
}

void abi_ClearScrollSpeedsC(void)
{
    ClearScrollSpeedsC();
    clear_scroll_speeds_exit_shim();
}

/*
 * @routine 0255:122 ScrollYFromLevelWord
 * @run 6F:A122
 * WHAT IT DOES: Takes the level's own scroll Y word as the scroll Y the game wants.
 * HOW: Two bytes copied, low then high, each read then written.
 * WAITS: no.
 * Entry: nothing. Exit: A the high byte, N and Z from it. X, Y, C and V kept.
 * Names: the source word has no name past its first byte (RasterWaterWanted holds it in the
 * symbols); an enum of this file stands for both bytes until the owner adds them.
 */
uint8_t ScrollYFromLevelWord(void)
{
    uint8_t low = rd8(ScrollYWantedLo);
    WR8(SCROLL->y_lo[0], low);
    uint8_t high = rd8(ScrollYWantedHi);
    WR8(SCROLL->y_hi[0], high);
    return high;
}

void abi_ScrollYFromLevelWord(void)
{
    cpu.a = ScrollYFromLevelWord();
    abi_nz(cpu.a);
}

/*
 * @routine 00C5:133 AddToListWord
 * @run 6C:8133
 * WHAT IT DOES: Adds a byte to entry X of a list's 16-bit counter, carrying into the high byte.
 * HOW: The low byte is read and written, then the high byte gets the carry.
 * WAITS: no.
 * Entry: A the amount, X the entry. Exit: A the high byte, N Z C V from its add. X, Y kept.
 * Names: the two columns ($3CA0, $3CA8) have no name yet, so the enum of this file gives them.
 */
Sum AddToListWord(uint8_t amount, uint8_t entry)
{
    uint8_t low = rd8((uint16_t)(ListWordLo + entry));
    Sum sum = add_bytes(amount, low, false);

    wr8((uint16_t)(ListWordLo + entry), sum.value);
    uint8_t high = rd8((uint16_t)(ListWordHi + entry));
    sum = add_bytes(0, high, sum.carry);
    wr8((uint16_t)(ListWordHi + entry), sum.value);
    return sum;
}

void abi_AddToListWord(void)
{
    sum_exit_shim(AddToListWord(cpu.a, cpu.x));
}

/*
 * The map byte for a screen row, the A of ScreenMapAt: the row shifted up a nibble, the column
 * (kept by the caller in zTemp00) or'd in, and the byte at that place in the $3D00 page read
 * through a zero page pointer.
 */
static uint8_t screen_map_at(uint8_t row)
{
    uint8_t place = (uint8_t)(row << SCREEN_MAP_ROW_SHIFT);
    uint8_t column = RD8(ZP->temp00);

    place = (uint8_t)(place | column);
    WR8(ZP->temp00, place);
    WR8(ZP->temp01, SCREEN_MAP_PAGE);
    return RD8_PTR(ZP->temp00);
}

static uint8_t screen_map_of_entry(uint8_t entry)
{
    uint8_t column = RD8(SCROLLE->x_hi_screen[entry]);
    uint8_t row;

    WR8(ZP->temp00, column);
    row = RD8(SCROLLE->y_hi_screen[entry]);
    return screen_map_at(row);
}

/* The exit of ScreenMapAt: A and N, Z from the byte; the ADC of 0 before it left C and V clear. */
static void screen_map_exit_shim(uint8_t byte)
{
    cpu.a = byte;
    abi_nz(byte);
    abi_flag(P_C, false);
    abi_flag(P_V, false);
}

/*
 * @routine 0255:6E2 ScreenMapAt
 * @run 6F:A6E2
 * WHAT IT DOES: Reads the level map's byte for the screen at row A, column zTemp00.
 * HOW: A shifted up four bits and or'd with zTemp00 is the place; it goes back to zTemp00 with
 * $3D in zTemp01 as a pointer, and the byte is read through it.
 * WAITS: no.
 * Entry: A the row, zTemp00 the column. Exit: A the byte, N and Z from it, C and V clear.
 * X and Y kept. Clobbers zTemp00 and zTemp01.
 */
uint8_t ScreenMapAt(uint8_t row)
{
    return screen_map_at(row);
}

void abi_ScreenMapAt(void)
{
    screen_map_exit_shim(ScreenMapAt(cpu.a));
}

/*
 * @routine 10C6:272 ScreenMapAtB
 * @run 6F:AA72
 * WHAT IT DOES: ScreenMapAt, as the stage at 1081 has it, run at $AA72.
 * HOW: The same bytes as ScreenMapAt at another run address; its C is screen_map_at.
 * WAITS: no.
 * Entry, exit: as ScreenMapAt.
 */
uint8_t ScreenMapAtB(uint8_t row)
{
    return screen_map_at(row);
}

void abi_ScreenMapAtB(void)
{
    screen_map_exit_shim(ScreenMapAtB(cpu.a));
}

/*
 * @routine 0515:6DA ScreenMapOfEntry
 * @run 6F:A6DA
 * WHAT IT DOES: Reads the level map's byte for the screen entry X of the scroll stands on.
 * HOW: Its column goes to zTemp00, its row is loaded, and the eight bytes run on into ScreenMapAt.
 * WAITS: no.
 * Entry: X the entry. Exit: as ScreenMapAt.
 */
uint8_t ScreenMapOfEntry(uint8_t entry)
{
    return screen_map_of_entry(entry);
}

void abi_ScreenMapOfEntry(void)
{
    screen_map_exit_shim(ScreenMapOfEntry(cpu.x));
}

/*
 * @routine 10C6:26A ScreenMapOfEntryB
 * @run 6F:AA6A
 * WHAT IT DOES: ScreenMapOfEntry, as the stage at 1081 has its own copy of it.
 * HOW: The same body, shared through screen_map_of_entry.
 * WAITS: no.
 * Entry, exit: as ScreenMapOfEntry.
 */
uint8_t ScreenMapOfEntryB(uint8_t entry)
{
    return screen_map_of_entry(entry);
}

void abi_ScreenMapOfEntryB(void)
{
    screen_map_exit_shim(ScreenMapOfEntryB(cpu.x));
}

/*
 * @routine 0256:180 ScrollXAddWrap
 * @run 6F:A980
 * WHAT IT DOES: Adds A to entry X of the scroll X the game wants, and moves its screen back by
 * two plus the carry, wrapping within sixteen screens.
 * HOW: The low byte is read and written; the level position's screen byte gets $FE and the carry,
 * is masked to four bits, and goes to both ScrollXHiScreen and ScrollXHi.
 * WAITS: no.
 * Entry: A the amount, X the entry. Exit: A the screen written, N and Z from it, C and V from the
 * add of $FE. X, Y kept.
 */
Sum ScrollXAddWrap(uint8_t amount, uint8_t entry)
{
    uint8_t low = RD8(SCROLL->x_lo[entry]);
    Sum sum = add_bytes(amount, low, false);
    uint8_t screen;
    Sum moved;

    WR8(SCROLL->x_lo[entry], sum.value);
    screen = RD8(SCROLLE->x_hi_screen[entry]);
    moved = add_bytes(screen, 0xFE, sum.carry);
    moved.value = (uint8_t)(moved.value & 0x0F);
    WR8(SCROLLE->x_hi_screen[entry], moved.value);
    WR8(SCROLL->x_hi[entry], moved.value);
    return moved;
}

void abi_ScrollXAddWrap(void)
{
    sum_exit_shim(ScrollXAddWrap(cpu.a, cpu.x));
}

/* Fields with no name yet (rows requested): the scroll's offset switch, and the list pointer's high byte. */
enum { ScrollOffsetSwitch = 0x3EBA, ZpWordsHigh = 0x2013 };

/*
 * @routine 0255:073 ScrollSpeedsFromBands
 * @run 6F:A073
 * WHAT IT DOES: Sets each layer's scroll speed this frame to how far the game's wanted scroll is
 * from the band's: X and Y, for all eight entries.
 * HOW: From entry 7 down to 0, the speed along X is ScrollXLo less ScrollBgxLo, then the speed
 * along Y is ScrollYLo less ScrollBgyLo, each a subtract of the low bytes with carry set.
 * WAITS: no.
 * Entry: nothing. Exit: A the last Y speed, X $FF, N set, Z clear, C and V from its subtract. Y kept.
 */
Sum ScrollSpeedsFromBands(void)
{
    Sum difference = { 0 };

    for (int entry = 7; entry >= 0; entry--) {
        uint8_t position = RD8(SCROLL->x_lo[entry]);
        uint8_t band = RD8(BANDS->bgx_lo[entry]);

        difference = sub_bytes(position, band, true);
        WR8(SCROLLE->speed_x[entry], difference.value);
        position = RD8(SCROLL->y_lo[entry]);
        band = RD8(BANDS->bgy_lo[entry]);
        difference = sub_bytes(position, band, true);
        WR8(SCROLLE->speed_y[entry], difference.value);
    }
    return difference;
}

void abi_ScrollSpeedsFromBands(void)
{
    Sum last = ScrollSpeedsFromBands();

    sum_exit_shim(last);
    cpu.x = 0xFF;
}

/*
 * @routine 0256:078 BitmapBitShifted
 * @run 6F:A878
 * WHAT IT DOES: Looks up bit (Y and 7) of the bitmap's byte Y/8 and leaves it in the carry. The
 * bitmap is read through the pointer in zTemp0C.
 * HOW: The byte goes to zTemp0E and is shifted right (Y and 7) plus one times; the last bit out is
 * the carry. X and Y are saved on the stack and put back.
 * WAITS: no.
 * Entry: Y the bit number, zTemp0C the bitmap. Exit: A Y and 7; C the bit; N and Z from X; X and Y
 * kept. Clobbers zTemp0E.
 */
BitmapBit BitmapBitShifted(uint8_t x, uint8_t y)
{
    BitmapBit bit;
    uint8_t byte, steps;

    push8(y);
    byte = RD8_PTR_Y(ZP->temp0_c, (uint8_t)((y >> 3) & 0x07));
    WR8(ZP->temp0_e, byte);
    y = pull8();
    bit.a = (uint8_t)(y & 0x07);
    push8(x);
    steps = (uint8_t)(bit.a + 1);
    do {
        bit.carry = LSR8(ZP->temp0_e);
        steps--;
    } while (steps != 0);
    bit.x = pull8();
    bit.y = y;
    return bit;
}

void abi_BitmapBitShifted(void)
{
    BitmapBit bit = BitmapBitShifted(cpu.x, cpu.y);

    cpu.a = bit.a;
    cpu.x = bit.x;
    cpu.y = bit.y;
    abi_nz(bit.x);
    abi_flag(P_C, bit.carry);
}

/*
 * @routine 00C5:27F CopyWordsToVdc
 * @run 6C:827F
 * WHAT IT DOES: Writes X blocks of sixteen words from the pointer in zTemp12 to the VDC's data
 * port, moving the pointer on a block at a time.
 * HOW: Each word is its low byte to the data port's low half, its high byte to the high half,
 * read through the pointer by Y. The pointer then goes up by $20 (the carry into its high byte).
 * WAITS: no.
 * Entry: X the number of blocks (0 is 256), zTemp12 the source, the VDC set to write. Exit: A the
 * pointer's high byte, X 0, Y $20, Z set, C and V from that last add.
 * Names: the pointer's high byte ($2013) is by address until the records name it.
 */
Sum CopyWordsToVdc(uint8_t blocks)
{
    Sum sum = { 0 };
    uint8_t y, byte, pointer;

    do {
        y = 0;
        do {
            byte = RD8_PTR_Y(ZP->temp12, y);
            WR8(VDC->data_lo, byte);
            y++;
            byte = RD8_PTR_Y(ZP->temp12, y);
            WR8(VDC->data_hi, byte);
            y++;
        } while (y != 0x20);
        pointer = RD8(ZP->temp12);
        sum = add_bytes(0x20, pointer, false);
        WR8(ZP->temp12, sum.value);
        pointer = rd8(ZpWordsHigh);
        sum = add_bytes(0, pointer, sum.carry);
        wr8(ZpWordsHigh, sum.value);
        blocks--;
    } while (blocks != 0);
    return sum;
}

void abi_CopyWordsToVdc(void)
{
    Sum last = CopyWordsToVdc(cpu.x);

    sum_exit_shim(last);
    cpu.x = 0;
    cpu.y = 0x20;
}

/* A scroll position and the offset the game adds to it, as the pair of bytes the camera's callers use. */
typedef struct {
    uint16_t low_column, screen_column, table, destination;
} ScrollOffsetShape;

static Sum scroll_plus_offset(uint8_t entry, uint8_t index, ScrollOffsetShape shape)
{
    uint8_t position, offset, screen;
    Sum sum;

    position = rd8(ScrollOffsetSwitch);
    if (position != 0) {
        position = RD8_IDX(shape.low_column, entry);
        offset = rd8((uint16_t)(shape.table - 2 + index));
    } else {
        position = RD8_IDX(shape.low_column, entry);
        offset = rd8((uint16_t)(shape.table + index));
    }
    sum = add_bytes(position, offset, false);
    WR8_IDX(shape.destination, 0, sum.value);
    screen = RD8_IDX(shape.screen_column, entry);
    offset = rd8((uint16_t)(shape.table + 2 + index));
    sum = add_bytes(screen, offset, sum.carry);
    sum.value = (uint8_t)(sum.value & 0x0F);
    WR8_IDX(shape.destination, 1, sum.value);
    return sum;
}

static ScrollOffsetShape x_offset_shape(uint16_t table)
{
    ScrollOffsetShape shape = { COL(SCROLL->x_lo), COL(SCROLLE->x_hi_screen), table, ADDR_OF(ZP->temp02) };

    return shape;
}

static ScrollOffsetShape y_offset_shape(uint16_t table)
{
    ScrollOffsetShape shape = { COL(SCROLL->y_lo), COL(SCROLLE->y_hi_screen), table, ADDR_OF(ZP->temp06) };

    return shape;
}

/*
 * @routine 0255:75C ScrollXPlusOffset
 * @run 6F:A75C
 * WHAT IT DOES: The scroll X of entry X plus an offset from a table of the overlay (word Y), as a
 * point on the level: the low byte to zTemp02, the screen (kept to 4 bits) to zTemp03.
 * HOW: The offset's low byte is added to ScrollXLo, and its high byte with the carry to
 * ScrollXHiScreen. When the switch at $3EBA is set the low byte comes from the word before.
 * WAITS: no.
 * Entry: X the entry, Y the table index (a word). Exit: A the screen, N and Z from it, C and V from
 * its add. X and Y kept. The switch's arm is untraced (00C5 bytes $5..$D of this routine).
 * Names: the switch is by address until the records name it.
 */
Sum ScrollXPlusOffset(uint8_t entry, uint8_t index)
{
    return scroll_plus_offset(entry, index, x_offset_shape(0xA780));
}

void abi_ScrollXPlusOffset(void)
{
    sum_exit_shim(ScrollXPlusOffset(cpu.x, cpu.y));
}

/*
 * @routine 10C6:2EC ScrollXPlusOffsetB
 * @run 6F:AAEC
 * WHAT IT DOES: ScrollXPlusOffset with the offsets of the stage at 1081.
 * HOW: The body is scroll_plus_offset's; the table is at $AB10.
 * WAITS: no.
 * Entry, exit: as ScrollXPlusOffset.
 */
Sum ScrollXPlusOffsetB(uint8_t entry, uint8_t index)
{
    return scroll_plus_offset(entry, index, x_offset_shape(0xAB10));
}

void abi_ScrollXPlusOffsetB(void)
{
    sum_exit_shim(ScrollXPlusOffsetB(cpu.x, cpu.y));
}

/*
 * @routine 12D6:2E5 ScrollXPlusOffsetC
 * @run 6F:AAE5
 * WHAT IT DOES: ScrollXPlusOffset with the offsets of the stage at 1291.
 * HOW: The body is scroll_plus_offset's; the table is at $AB09.
 * WAITS: no.
 * Entry, exit: as ScrollXPlusOffset.
 */
Sum ScrollXPlusOffsetC(uint8_t entry, uint8_t index)
{
    return scroll_plus_offset(entry, index, x_offset_shape(0xAB09));
}

void abi_ScrollXPlusOffsetC(void)
{
    sum_exit_shim(ScrollXPlusOffsetC(cpu.x, cpu.y));
}

/*
 * @routine 0726:092 ScrollYPlusOffset
 * @run 6F:A892
 * WHAT IT DOES: ScrollXPlusOffset along Y: ScrollYLo and ScrollYHiScreen plus the table's word,
 * the result in zTemp06 and zTemp07.
 * HOW: The body is scroll_plus_offset's; the table is at $A780.
 * WAITS: no.
 * Entry, exit: as ScrollXPlusOffset.
 */
Sum ScrollYPlusOffset(uint8_t entry, uint8_t index)
{
    return scroll_plus_offset(entry, index, y_offset_shape(0xA780));
}

void abi_ScrollYPlusOffset(void)
{
    sum_exit_shim(ScrollYPlusOffset(cpu.x, cpu.y));
}

/*
 * @routine 10C6:422 ScrollYPlusOffsetB
 * @run 6F:AC22
 * WHAT IT DOES: ScrollYPlusOffset with the offsets of the stage at 1081.
 * HOW: The body is scroll_plus_offset's; the table is at $AB10.
 * WAITS: no.
 * Entry, exit: as ScrollXPlusOffset.
 */
Sum ScrollYPlusOffsetB(uint8_t entry, uint8_t index)
{
    return scroll_plus_offset(entry, index, y_offset_shape(0xAB10));
}

void abi_ScrollYPlusOffsetB(void)
{
    sum_exit_shim(ScrollYPlusOffsetB(cpu.x, cpu.y));
}

/* One axis of the scroll and its step, as columns of entries. */
typedef struct {
    uint16_t step_low, step_high, fraction, low, high, screen, table;
} AdvanceShape;

static AdvanceShape x_advance_shape(uint16_t table)
{
    AdvanceShape shape = { COL(SCROLLE->step_x_lo), COL(SCROLLE->step_x_hi), COL(SCROLL->x_frac),
                           COL(SCROLL->x_lo), COL(SCROLL->x_hi), COL(SCROLLE->x_hi_screen), table };

    return shape;
}

static AdvanceShape y_advance_shape(uint16_t table)
{
    AdvanceShape shape = { COL(SCROLLE->step_y_lo), COL(SCROLLE->step_y_hi), COL(SCROLL->y_frac),
                           COL(SCROLL->y_lo), COL(SCROLL->y_hi), COL(SCROLLE->y_hi_screen), table };

    return shape;
}

/*
 * The scroll of entry `entry` moved on by its step: the fraction and low bytes take the step's
 * two bytes, and the carry and the step's sign (the table gives $00 or $FF by sign) go into the
 * high byte and the level position's screen, kept to four bits. The sign comes from the step's
 * high byte, read twice, as the assembly does.
 */
static AdvanceResult advance_scroll(uint8_t entry, AdvanceShape shape)
{
    AdvanceResult result;
    uint8_t high, low, step, sign, spread;
    Sum sum;

    high = RD8_IDX(shape.step_high, entry);
    sign = (uint8_t)(high >> 7);
    step = RD8_IDX(shape.step_low, entry);
    low = RD8_IDX(shape.fraction, entry);
    sum = add_bytes(step, low, false);
    WR8_IDX(shape.fraction, entry, sum.value);
    high = RD8_IDX(shape.step_high, entry);
    low = RD8_IDX(shape.low, entry);
    sum = add_bytes(high, low, sum.carry);
    WR8_IDX(shape.low, entry, sum.value);
    spread = rd8((uint16_t)(shape.table + sign));
    sum = add_bytes(0, spread, sum.carry);
    spread = sum.value;
    push8(spread);
    high = RD8_IDX(shape.high, entry);
    sum = add_bytes(spread, high, false);
    WR8_IDX(shape.high, entry, sum.value);
    spread = pull8();
    low = RD8_IDX(shape.screen, entry);
    sum = add_bytes(spread, low, false);
    sum.value = (uint8_t)(sum.value & 0x0F);
    WR8_IDX(shape.screen, entry, sum.value);
    result.sum = sum;
    result.sign = sign;
    return result;
}

/*
 * @routine 0255:6F4 AdvanceScrollX
 * @run 6F:A6F4
 * WHAT IT DOES: Moves entry X's scroll along X by its step.
 * HOW: See advance_scroll. The table at $A727 gives the sign extension.
 * WAITS: no.
 * Entry: X the entry. Exit: A the screen, N and Z from it, C and V from its add. X kept; Y the step's sign
 * (AdvanceResult).
 */
AdvanceResult AdvanceScrollX(uint8_t entry)
{
    return advance_scroll(entry, x_advance_shape(0xA727));
}

/*
 * @routine 10C6:284 AdvanceScrollXB
 * @run 6F:AA84
 * WHAT IT DOES: AdvanceScrollX with the sign table of the stage at 1081 ($AAB7).
 * HOW: See advance_scroll.
 * WAITS: no.
 * Entry, exit: as AdvanceScrollX.
 */
AdvanceResult AdvanceScrollXB(uint8_t entry)
{
    return advance_scroll(entry, x_advance_shape(0xAAB7));
}

/*
 * @routine 12D6:27D AdvanceScrollXC
 * @run 6F:AA7D
 * WHAT IT DOES: AdvanceScrollX with the sign table of the stage at 1291 ($AAB0).
 * HOW: See advance_scroll.
 * WAITS: no.
 * Entry, exit: as AdvanceScrollX.
 */
AdvanceResult AdvanceScrollXC(uint8_t entry)
{
    return advance_scroll(entry, x_advance_shape(0xAAB0));
}

/*
 * @routine 0725:729 AdvanceScrollY
 * @run 6F:A729
 * WHAT IT DOES: Moves entry X's scroll along Y by its step.
 * HOW: See advance_scroll. The table at $A727 gives the sign extension.
 * WAITS: no.
 * Entry, exit: as AdvanceScrollX.
 */
AdvanceResult AdvanceScrollY(uint8_t entry)
{
    return advance_scroll(entry, y_advance_shape(0xA727));
}

/*
 * @routine 10C6:2B9 AdvanceScrollYB
 * @run 6F:AAB9
 * WHAT IT DOES: AdvanceScrollY with the sign table of the stage at 1081 ($AAB7).
 * HOW: See advance_scroll.
 * WAITS: no.
 * Entry, exit: as AdvanceScrollX.
 */
AdvanceResult AdvanceScrollYB(uint8_t entry)
{
    return advance_scroll(entry, y_advance_shape(0xAAB7));
}


static void advance_exit_shim(AdvanceResult result)
{
    cpu.y = result.sign;
    sum_exit_shim(result.sum);
}

void abi_AdvanceScrollX(void)
{
    advance_exit_shim(AdvanceScrollX(cpu.x));
}
void abi_AdvanceScrollXB(void)
{
    advance_exit_shim(AdvanceScrollXB(cpu.x));
}
void abi_AdvanceScrollXC(void)
{
    advance_exit_shim(AdvanceScrollXC(cpu.x));
}
void abi_AdvanceScrollY(void)
{
    advance_exit_shim(AdvanceScrollY(cpu.x));
}
void abi_AdvanceScrollYB(void)
{
    advance_exit_shim(AdvanceScrollYB(cpu.x));
}
