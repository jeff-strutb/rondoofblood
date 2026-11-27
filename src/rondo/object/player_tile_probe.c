/*
 * The tile read of the stage overlays whose code sits at $9991 (the same logic as TerrainTileRead at $967F, whose overlays
 * hold their own copy of it, with the handler table at $9A2C): what the map holds at a point, and which of the handlers
 * the table names are this code's. The table's other entries ($9A64 and its neighbours, none run by any script) go to the
 * original's code.
 *
 * Every overlay that holds it holds the same bytes and the same table, so one routine serves them all.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "player_objects.h"

enum {
    PROBE_FIRST_HANDLED = 0x80, PROBE_END_HANDLED = 0x9C, PROBE_HANDLED_RESULT = 0x81, PROBE_PAGE_BASE = 0x7D,
    PROBE_HANDLERS = 0x9A2C
};

/* A handler's exit as the probe's TileRead carries it. */
static HandlerExit handler_of_ended(Ended e)
{
    HandlerExit out;

    out.y_set = e.y_set;
    out.y = e.y;
    out.flags_set = e.c_set || e.v_set;
    out.carry = e.c;
    out.overflow = e.v;
    return out;
}

/* Runs the handler at an address the table names, if it is one of this code's. */
static bool run_probe_handler(uint16_t code, uint8_t tile, HandlerExit *out)
{
    switch (code) {
    case 0x9A93:
        *out = handler_of_ended(tile_half_step_exit_shim(TileHalfStepAAt9A93(tile)));
        return true;
    case 0x9AA7:
        *out = handler_of_ended(tile_half_step_exit_shim(TileHalfStepBAt9AA7(tile)));
        return true;
    case 0x9C82:
        *out = handler_of_ended(tile_nudge_exit_shim(TileNudgeAt9C82()));
        return true;
    case 0x9C87:
        *out = handler_of_ended(tile_row_up_exit_shim(TileLift7At9C87()));
        return true;
    case 0x9C91:
        *out = handler_of_ended(tile_row_up_exit_shim(TileLift6At9C91()));
        return true;
    case 0x9C9B:
        *out = handler_of_ended(tile_row_up_exit_shim(TileLift5At9C9B()));
        return true;
    case 0x9CA5:
        *out = handler_of_ended(tile_row_up_exit_shim(TileLift4At9CA5()));
        return true;
    case 0x9CAF:
        *out = handler_of_ended(tile_row_up_exit_shim(TileLift3At9CAF()));
        return true;
    case 0x9CB9:
        *out = handler_of_ended(tile_row_up_exit_shim(TileLift2At9CB9()));
        return true;
    case 0x9CC3:
        *out = handler_of_ended(tile_row_up_exit_shim(TileLift1At9CC3()));
        return true;
    default:
        return false;
    }
}

/* A handler's table entry that is not one of this code's: the original's code runs, entered as the jump leaves the registers. */
static uint8_t tile_to_original_shim(uint8_t tile, uint8_t y, uint16_t code)
{
    cpu.a = tile;
    cpu.y = y;
    abi_nz(tile);
    abi_flag(P_C, false);
    abi_flag(P_V, true);
    run_routine(code);
    return cpu.a;
}

/*
 * @routine 05C0:191 ProbeTileRead9991
 * @run 6B:9991
 * WHAT IT DOES: Says what the map holds at a point: the attribute of the background tile there, 0 if its page has none,
 * $81 for a slope or step, which moves the row in the tile.
 * HOW: The point plus the scroll gives the tile's place in the 64 by 128 background; its entry is read from VRAM. The
 * entry's low byte and a page by its high bits give the attribute. Attributes $80-$9B go through the table at $9A2C.
 * WAITS: no.
 * Entry: zTemp00-01 X, zTemp02-03 Y. Exit: A the answer, zTemp00 the column, zTemp02 the row. X kept. Clobbers zTemp04-09.
 */
TileRead ProbeTileRead9991(void)
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
    jsr_push(0x99E0);
    SelectVramReadAddress();
    jsr_pop();
    value = RD8(ZP->temp05);
    WR8(VDC->data_lo, value);
    value = RD8(ZP->temp07);
    WR8(VDC->data_hi, value);
    jsr_push(0x99ED);
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
    WR8(ZP->temp09, (uint8_t)(read.page + PROBE_PAGE_BASE));
    attribute = RD8_PTR(ZP->temp08);
    read.attribute = attribute;
    if ((attribute & 0x80) == 0) {
        return read;
    }
    if (attribute >= PROBE_END_HANDLED) {
        read.kind = TILE_PAST_HANDLED;
        return read;
    }
    push8(attribute);
    value = (uint8_t)((uint8_t)(attribute - PROBE_FIRST_HANDLED) << 1);
    byte = rd8((uint16_t)(PROBE_HANDLERS + value));
    WR8(ZP->temp04, byte);
    byte = rd8((uint16_t)(PROBE_HANDLERS + 1 + value));
    WR8(ZP->temp05, byte);
    attribute = pull8();
    {
        uint16_t code = RD16_LH2(ZP->temp04, ZP->temp05);

        read.kind = TILE_HANDLED;
        if (!run_probe_handler(code, attribute, &read.handler)) {
            read.kind = TILE_BY_ORIGINAL;
            read.attribute = tile_to_original_shim(attribute, value, code);
        }
    }
    return read;
}

void abi_ProbeTileRead9991(void)
{
    terrain_read_exit_shim(ProbeTileRead9991());
}
