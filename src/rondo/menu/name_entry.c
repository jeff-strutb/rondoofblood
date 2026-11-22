/*
 * The name menu: the name being typed is eight characters at NAME_CHARS, the number typed so
 * far at NAME_POS; each character is also drawn as a tile in the row from VRAM $00D0, and a
 * blank tile after the last marks where the next goes. A cursor object (object 2 and its pair 3)
 * moves over a grid of characters read from a table. The addresses are placeholders until the
 * names land (front_end.h lists them).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"

/*
 * @routine 00C3:605 ShowNameCursor
 * @run 6B:9605
 * WHAT IT DOES: Draws the blank tile where the next character of the name will go, and keeps a
 * space in the name buffer there.
 * HOW: Sets the VDC's write address to the tile row's place for the typed count, writes the
 * blank tile, and stores $20 in the buffer at the count.
 * WAITS: no.
 * Entry: nothing. Exit: A $20, N and Z clear, C and V from the add of the place, Y the count; X
 * kept.
 */
NameExit ShowNameCursor(void)
{
    NameExit result;
    uint8_t position, place;
    unsigned total;

    result.full = false;
    jsr_push(0x9608);
    SelectVramWriteAddress();
    jsr_pop();
    position = rd8(NAME_POS);
    total = (unsigned)position + 0xD0;
    place = (uint8_t)total;
    WR8(VDC->data_lo, place);
    st2(0x00);
    jsr_push(0x9616);
    SelectVramData();
    jsr_pop();
    st1(0xFF);
    st2(0x10);
    result.y = rd8(NAME_POS);
    wr8((uint16_t)(NAME_CHARS + result.y), 0x20);
    result.carry = total > 0xFF;
    result.overflow = ((position ^ place) & (0xD0 ^ place) & 0x80) != 0;
    result.a = 0x20;
    return result;
}

void abi_ShowNameCursor(void)
{
    NameExit result = ShowNameCursor();

    cpu.a = result.a;
    cpu.y = result.y;
    abi_nz(result.a);
    abi_flag(P_C, result.carry);
    abi_flag(P_V, result.overflow);
}

/*
 * @routine 00C3:623 NameBackspace
 * @run 6B:9623
 * WHAT IT DOES: Takes back the last character of the name: its tile is redrawn as the erased
 * tile, the count goes down by one, and the cursor tile goes where the character was.
 * HOW: Sets the VDC's write address to the last tile's place, writes the erased tile ($113D),
 * subtracts one from the count, stores a space at the new count and goes to ShowNameCursor.
 * WAITS: no.
 * Entry: the count at least 1. Exit: as ShowNameCursor.
 */
NameExit NameBackspace(void)
{
    uint8_t position, count, place;

    jsr_push(0x9626);
    SelectVramWriteAddress();
    jsr_pop();
    position = rd8(NAME_POS);
    place = (uint8_t)(position + 0xD0);
    WR8(VDC->data_lo, place);
    st2(0x00);
    jsr_push(0x9634);
    SelectVramData();
    jsr_pop();
    st1(0x3D);
    st2(0x11);
    position = rd8(NAME_POS);
    count = (uint8_t)(position - 1);
    wr8(NAME_POS, count);
    position = rd8(NAME_POS);
    wr8((uint16_t)(NAME_CHARS + position), 0x20);
    return ShowNameCursor();
}

void abi_NameBackspace(void)
{
    NameExit result = NameBackspace();

    cpu.a = result.a;
    cpu.y = result.y;
    abi_nz(result.a);
    abi_flag(P_C, result.carry);
    abi_flag(P_V, result.overflow);
}

/*
 * @routine 00C3:5D2 NameTypeCharacter
 * @run 6B:95D2
 * WHAT IT DOES: Types the character the cursor is on: it is drawn at the next place, put in the
 * buffer, the count goes up by one, and unless the name is full the cursor tile follows it.
 * HOW: The character is read from the grid table at the row (OBJ->f3618 of object 2) times 8
 * plus the column (OBJ->f34b8 of object 2), with the carry of the row's third shift added in as
 * the original does; written to the VDC as a tile with palette 1 and to the buffer at the count.
 * The count rises; at 8 the routine stops, otherwise it runs on into ShowNameCursor.
 * WAITS: no.
 * Entry: nothing. Exit: name full: A 8, Z and C set, N clear, Y the count before, V from the
 * grid index add; else as ShowNameCursor.
 */
NameExit NameTypeCharacter(void)
{
    NameExit result;
    uint8_t position, row, column, character, count, place, grid, grid_base;
    bool shifted_out, grid_overflow;
    unsigned total;

    result.full = false;

    jsr_push(0x95D5);
    SelectVramWriteAddress();
    jsr_pop();
    position = rd8(NAME_POS);
    place = (uint8_t)(position + 0xD0);
    WR8(VDC->data_lo, place);
    st2(0x00);
    jsr_push(0x95E3);
    SelectVramData();
    jsr_pop();
    row = RD8(OBJ->f3618[2]);
    shifted_out = (row & 0x20) != 0;
    column = RD8(OBJ->f34b8[2]);
    grid_base = (uint8_t)(row << 3);
    total = (unsigned)grid_base + column + (shifted_out ? 1 : 0);
    grid = (uint8_t)total;
    grid_overflow = ((grid_base ^ grid) & (column ^ grid) & 0x80) != 0;
    character = rd8((uint16_t)(NAME_GRID + grid));
    WR8(VDC->data_lo, character);
    result.y = rd8(NAME_POS);
    wr8((uint16_t)(NAME_CHARS + result.y), character);
    st2(0x11);
    position = rd8(NAME_POS);
    wr8(NAME_POS, (uint8_t)(position + 1));
    count = rd8(NAME_POS);
    if (count == NAME_LENGTH) {
        result.full = true;
        result.a = count;
        result.carry = true;
        result.overflow = grid_overflow;
        return result;
    }
    return ShowNameCursor();
}

void abi_NameTypeCharacter(void)
{
    NameExit result = NameTypeCharacter();

    cpu.a = result.a;
    cpu.y = result.y;
    abi_nz(result.full ? 0 : result.a);
    abi_flag(P_C, result.carry);
    abi_flag(P_V, result.overflow);
}

/*
 * @routine 00C3:665 NameCursorTarget
 * @run 6B:9665
 * WHAT IT DOES: Sends the name cursor object (and its pair, the next object) to the place of the
 * grid character it is on: the last row, the "end" key, is wide, from $60 to $A0.
 * HOW: The object's row (OBJ->f3618) picks the Y place from a table, written to both objects;
 * on row 4 the X places are $60 and $A0, otherwise the column (OBJ->f34b8) picks a place from a
 * table for both; both are marked as moving.
 * WAITS: no.
 * Entry: X the object. Exit: A 1, N and Z clear, Y the row or the column; X kept; C as the last
 * compare left it.
 */
NameCursorExit NameCursorTarget(uint8_t object)
{
    NameCursorExit result;
    uint8_t place;

    result.y = RD8(OBJ->f3618[object]);
    place = rd8((uint16_t)(NAME_ROW_PLACES + result.y));
    WR8(OBJ->f35c0[object], place);
    WR8(OBJ->f35c0[object + 1], place);
    result.carry = result.y >= 4;
    if (result.y == 4) {
        WR8(OBJ->f3568[object], 0x60);
        WR8(OBJ->f3568[object + 1], 0xA0);
        result.carry = true;
    } else {
        result.y = RD8(OBJ->f34b8[object]);
        place = rd8((uint16_t)(NAME_COLUMN_PLACES + result.y));
        WR8(OBJ->f3568[object], place);
        WR8(OBJ->f3568[object + 1], place);
    }
    WR8(OBJ->f3510[object], 1);
    WR8(OBJ->f3510[object + 1], 1);
    return result;
}

void abi_NameCursorTarget(void)
{
    NameCursorExit result = NameCursorTarget(cpu.x);

    cpu.a = 1;
    cpu.y = result.y;
    abi_nz(1);
    abi_flag(P_C, result.carry);
}
