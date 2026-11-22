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
enum { ScrollYWantedLo = 0x3EB8, ScrollYWantedHi = 0x3EB9, ListWordLo = 0x3CA0, ListWordHi = 0x3CA8 };

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
Alu AddToListWord(uint8_t amount, uint8_t entry)
{
    uint8_t low = rd8((uint16_t)(ListWordLo + entry));
    Alu sum = adc8(amount, low, false);

    wr8((uint16_t)(ListWordLo + entry), sum.a);
    uint8_t high = rd8((uint16_t)(ListWordHi + entry));
    sum = adc8(0, high, sum.c);
    wr8((uint16_t)(ListWordHi + entry), sum.a);
    return sum;
}

void abi_AddToListWord(void)
{
    abi_alu(AddToListWord(cpu.a, cpu.x));
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
 * @routine 12D6:263 ScreenMapOfEntryC
 * @run 6F:A6DA
 * WHAT IT DOES: ScreenMapOfEntry, as the stage at 1291 has its own copy of it.
 * HOW: The same body, shared through screen_map_of_entry.
 * WAITS: no.
 * Entry, exit: as ScreenMapOfEntry.
 */
uint8_t ScreenMapOfEntryC(uint8_t entry)
{
    return screen_map_of_entry(entry);
}

void abi_ScreenMapOfEntryC(void)
{
    screen_map_exit_shim(ScreenMapOfEntryC(cpu.x));
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
Alu ScrollXAddWrap(uint8_t amount, uint8_t entry)
{
    uint8_t low = RD8(SCROLL->x_lo[entry]);
    Alu sum = adc8(amount, low, false);
    uint8_t screen;
    Alu moved;

    WR8(SCROLL->x_lo[entry], sum.a);
    screen = RD8(SCROLLE->x_hi_screen[entry]);
    moved = adc8(screen, 0xFE, sum.c);
    moved.a = (uint8_t)(moved.a & 0x0F);
    moved.nz = moved.a;
    WR8(SCROLLE->x_hi_screen[entry], moved.a);
    WR8(SCROLL->x_hi[entry], moved.a);
    return moved;
}

void abi_ScrollXAddWrap(void)
{
    abi_alu(ScrollXAddWrap(cpu.a, cpu.x));
}
