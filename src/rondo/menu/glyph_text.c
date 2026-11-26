/*
 * The menus' glyph text: a string of two-byte codes (the first byte the code's high byte, a zero
 * ending the string), each pair of glyphs drawn as one 16 by 16 tile pair: the glyph's dots from
 * the card's font (or from a table for a few codes), merged into a buffer at $3B40 and written to
 * VRAM, and the map's tile numbers for them written under it. The string is read through the
 * pointer in zTemp00 and zTemp01; the place of its map at zTemp08 and zTemp09 and of its tiles at
 * zTemp10 and zTemp11 are the first four bytes of it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "syscard.h"

enum {
    GLYPH_NEXT_ROW = 0x3D, GLYPH_ROW_ADVANCE = 3, TILE_STEP = 0x60,
    GLYPH_FIRST = GLYPH_BUFFER, GLYPH_SECOND = 0x3B60, GLYPH_SECOND_LESS = 0x3B5F,
    TABLE_FIRST = 0x8620, TABLE_C9 = 0x8600, TABLE_72 = 0x8640, TABLE_E081 = 0x85E0,
    CODE_9C = 0x9C, CODE_E0 = 0xE0, LOW_66 = 0x66, LOW_C9 = 0xC9, LOW_72 = 0x72, LOW_81 = 0x81,
    GLYPH_BYTES = 0x20, GLYPH_ROWS_AT_END = 0x31, SHIFTS = 4, FONT_LOW_BUFFER = 0x40, FONT_HIGH_BUFFER = 0x3B,
    FONT_SECOND_LOW = 0x60
};

/* The offsets WriteGlyphRowsTwice draws the merged glyph from, in order. */
static const uint8_t ROW_OFFSETS[] = { 0x00, 0x01, 0x21, 0x10, 0x11 };

/*
 * @routine 00C1:660 WriteGlyphMapRows
 * @run 6B:8660
 * WHAT IT DOES: Writes the map entries of a glyph pair: three tile numbers at the place in zTemp08 and
 * zTemp09 (which then moves on by 3), and under it, at that place plus $40, three more. The tile numbers
 * are the tile place in zTemp10 and zTemp11 shifted down four, less 6, 5 and 4 for the first row and
 * less 3, 2 and 1 for the second.
 * HOW: SelectVramWriteAddress and SelectVramData for each row; the tile place shifted down four with ones
 * shifted in at the top, in zBiosAl and zBiosAh, the six words by subtracting from it.
 * WAITS: no.
 * Entry: nothing. Exit: A the last high byte, N and Z from it, C and V from its subtract.
 */
Exit WriteGlyphMapRows(void)
{
    Exit exit = { 0 };
    uint8_t place, high, tile_low, tile_high, carry_in, shifted;
    unsigned total;
    uint8_t k, index_low;
    bool borrow_free;

    jsr_push(0x8663);
    SelectVramWriteAddress();
    jsr_pop();
    place = RD8(ZP->temp08);
    WR8(VDC->data_lo, place);
    total = (unsigned)place + GLYPH_ROW_ADVANCE;
    WR8(ZP->temp08, (uint8_t)total);
    place = RD8(ZP->temp09);
    WR8(VDC->data_hi, place);
    if (total > 0xFF) {
        INC8(ZP->temp09);
    }
    jsr_push(0x8679);
    SelectVramData();
    jsr_pop();
    tile_low = RD8(ZP->temp10);
    WR8(ZP->bios_a.lo, tile_low);
    high = RD8(ZP->temp11);
    for (k = 0; k < SHIFTS; k++) {
        carry_in = (uint8_t)(high & 1);
        high = (uint8_t)((high >> 1) | 0x80);
        shifted = RD8(ZP->bios_a.lo);
        WR8(ZP->bios_a.lo, (uint8_t)((shifted >> 1) | (carry_in << 7)));
    }
    WR8(ZP->bios_a.hi, high);
    for (index_low = 6; index_low >= 4; index_low--) {
        tile_low = RD8(ZP->bios_a.lo);
        WR8(VDC->data_lo, (uint8_t)(tile_low - index_low));
        borrow_free = tile_low >= index_low;
        tile_high = RD8(ZP->bios_a.hi);
        WR8(VDC->data_hi, (uint8_t)(tile_high - (borrow_free ? 0 : 1)));
    }
    jsr_push(0x86C1);
    SelectVramWriteAddress();
    jsr_pop();
    place = RD8(ZP->temp08);
    total = (unsigned)place + GLYPH_NEXT_ROW;
    WR8(VDC->data_lo, (uint8_t)total);
    place = RD8(ZP->temp09);
    WR8(VDC->data_hi, (uint8_t)(place + (total > 0xFF ? 1 : 0)));
    jsr_push(0x86D2);
    SelectVramData();
    jsr_pop();
    for (index_low = 3; index_low >= 1; index_low--) {
        tile_low = RD8(ZP->bios_a.lo);
        WR8(VDC->data_lo, (uint8_t)(tile_low - index_low));
        borrow_free = tile_low >= index_low;
        tile_high = RD8(ZP->bios_a.hi);
        shifted = (uint8_t)(tile_high - (borrow_free ? 0 : 1));
        WR8(VDC->data_hi, shifted);
        exit.a = shifted;
        exit.nz = shifted;
        exit.c = tile_high >= (borrow_free ? 0 : 1);
        exit.v = (tile_high & (tile_high ^ shifted) & 0x80) != 0;
        exit.v_set = true;
    }
    exit.x_kept = true;
    exit.y_kept = true;
    return exit;
}

void abi_WriteGlyphMapRows(void)
{
    exit_shim(WriteGlyphMapRows());
}

/* Copies a glyph's 32 bytes from a table to a buffer, from the last byte down. */
static void copy_glyph(uint16_t table, uint16_t buffer)
{
    int x;

    for (x = GLYPH_BYTES - 1; x >= 0; x--) {
        uint8_t byte = rd8((uint16_t)(table + x));

        wr8((uint16_t)(buffer + x), byte);
    }
}

/* The card's font for the code in zBiosAl and zBiosAh, into the buffer in zBiosBl and zBiosBh. */
static void get_glyph(uint8_t buffer_low, uint8_t buffer_high, uint16_t ret)
{
    WR8(ZP->bios_b.lo, buffer_low);
    WR8(ZP->bios_b.hi, buffer_high);
    WR8(ZP->bios_d.hi, 1);
    jsr_push(ret);
    (void)vec_ex_getfnt();
    jsr_pop();
}

/*
 * @routine 00C1:4D5 DrawGlyphPair
 * @run 6B:84D5
 * WHAT IT DOES: Draws the next two glyphs of the string as one tile pair: each glyph's 16 by 16 dots
 * from the card's font (or from a table at $8620, $8600, $8640 or $85E0 for the codes $9C??, $9CC9, $9C72
 * and $E081; a zero byte in place of the second glyph leaves it blank), the second shifted right four
 * dots and merged over the first in the buffer at $3B40, then written to VRAM at the tile place in zTemp10
 * and zTemp11 (which moves on by $60).
 * HOW: SelectVramWriteAddress and SelectVramData; ex_getfnt for each glyph the tables do not hold; the merge
 * by LSR and ROR of pairs of buffer bytes four times and an OR; WriteGlyphRowsTwice with X $00, $01, $21,
 * $10, $11 and $31.
 * WAITS: no.
 * Entry: the string at zTemp00 and zTemp01. Exit: as WriteGlyphRowsTwice: A the last byte written, X
 * that plus 16, Y 8, Z and C set, N clear. V as the tile place's add left it.
 * not checked: agrees for 40 events, then differs inside the card's sjis_to_font_index.
 */
Exit DrawGlyphPair(void)
{
    Exit exit = { 0 };
    uint8_t place, high_code, low_code, second, x, shifts, carry, byte, merged, v_add_a;
    unsigned total;
    int i;
    GlyphRows rows;
    bool overflow;

    jsr_push(0x84D8);
    SelectVramWriteAddress();
    jsr_pop();
    place = RD8(ZP->temp10);
    WR8(VDC->data_lo, place);
    total = (unsigned)place + TILE_STEP;
    overflow = ((place ^ total) & (TILE_STEP ^ total) & 0x80) != 0;
    WR8(ZP->temp10, (uint8_t)total);
    place = RD8(ZP->temp11);
    WR8(VDC->data_hi, place);
    if (total > 0xFF) {
        INC8(ZP->temp11);
    }
    jsr_push(0x84EE);
    SelectVramData();
    jsr_pop();
    high_code = RD8_PTR(ZP->temp00);
    WR8(ZP->bios_a.hi, high_code);
    jsr_push(0x84F6);
    IncrementTempWord();
    jsr_pop();
    low_code = RD8_PTR(ZP->temp00);
    WR8(ZP->bios_a.lo, low_code);
    jsr_push(0x84FE);
    IncrementTempWord();
    jsr_pop();
    if (high_code == CODE_9C) {
        copy_glyph(TABLE_FIRST, GLYPH_FIRST);
    } else {
        get_glyph(FONT_LOW_BUFFER, FONT_HIGH_BUFFER, 0x8522);
    }
    second = RD8_PTR(ZP->temp00);
    if (second == 0) {
        for (i = GLYPH_BYTES - 1; i >= 0; i--) {
            wr8((uint16_t)(GLYPH_SECOND + i), 0);
        }
    } else {
        WR8(ZP->bios_a.hi, second);
        jsr_push(0x8536);
        IncrementTempWord();
        jsr_pop();
        low_code = RD8_PTR(ZP->temp00);
        WR8(ZP->bios_a.lo, low_code);
        jsr_push(0x853E);
        IncrementTempWord();
        jsr_pop();
        if (second == CODE_9C && low_code == LOW_C9) {
            copy_glyph(TABLE_C9, GLYPH_SECOND);
        } else if (second == CODE_9C && low_code == LOW_72) {
            copy_glyph(TABLE_72, GLYPH_SECOND);
        } else if (second == CODE_E0 && low_code == LOW_81) {
            copy_glyph(TABLE_E081, GLYPH_SECOND);
        } else {
            get_glyph(FONT_SECOND_LOW, FONT_HIGH_BUFFER, 0x8588);
        }
    }
    for (x = GLYPH_BYTES - 1; (int8_t)x >= 0; x = (uint8_t)(x - 2)) {
        for (shifts = 0; shifts < SHIFTS; shifts++) {
            byte = rd8((uint16_t)(GLYPH_SECOND_LESS + x));
            carry = (uint8_t)(byte & 1);
            wr8((uint16_t)(GLYPH_SECOND_LESS + x), (uint8_t)(byte >> 1));
            byte = rd8((uint16_t)(GLYPH_SECOND + x));
            wr8((uint16_t)(GLYPH_SECOND + x), (uint8_t)((byte >> 1) | (carry << 7)));
        }
        merged = rd8((uint16_t)(GLYPH_FIRST + x));
        merged = (uint8_t)(merged | rd8((uint16_t)(GLYPH_SECOND_LESS + x)));
        wr8((uint16_t)(GLYPH_FIRST + x), merged);
    }
    for (i = 0; i < 5; i++) {
        jsr_push((uint16_t)(0x85B4 + 5 * i));
        (void)WriteGlyphRowsTwice(ROW_OFFSETS[i]);
        jsr_pop();
    }
    rows = WriteGlyphRowsTwice(GLYPH_ROWS_AT_END);
    (void)v_add_a;
    exit.a = rows.a;
    exit.x = rows.x;
    exit.y = 8;
    exit.nz = 0;
    exit.c = true;
    exit.v = overflow;
    exit.v_set = true;
    return exit;
}

void abi_DrawGlyphPair(void)
{
    exit_shim(DrawGlyphPair());
}

/*
 * @routine 00C1:4AC DrawGlyphText
 * @run 6B:84AC
 * WHAT IT DOES: Draws a string of glyph pairs: the string's first four bytes are the map place (two
 * bytes, zTemp08 and zTemp09) and the tile place (zTemp10 and zTemp11); then, until a zero byte, each pair
 * of glyphs is drawn (DrawGlyphPair) and its map entries written (WriteGlyphMapRows).
 * HOW: Four bytes read through zTemp00 and zTemp01, IncrementTempWord after each; the loop.
 * WAITS: no.
 * Entry: the string at zTemp00 and zTemp01. Exit: A 0, Z set, N clear; X, Y, C and V as the last pair's
 * map rows left them, or as the pointer's last step left them when there was no pair.
 * not checked: matches only with the callees as filed wrappers on both sides.
 */
Exit DrawGlyphText(void)
{
    Exit exit = { 0 };
    Exit pair;
    uint8_t value;
    bool drawn = false;

    exit.x_kept = exit.y_kept = exit.c_kept = true;
    value = RD8_PTR(ZP->temp00);
    WR8(ZP->temp08, value);
    jsr_push(0x84B3);
    IncrementTempWord();
    jsr_pop();
    value = RD8_PTR(ZP->temp00);
    WR8(ZP->temp09, value);
    jsr_push(0x84BA);
    IncrementTempWord();
    jsr_pop();
    value = RD8_PTR(ZP->temp00);
    WR8(ZP->temp10, value);
    jsr_push(0x84C1);
    IncrementTempWord();
    jsr_pop();
    value = RD8_PTR(ZP->temp00);
    WR8(ZP->temp11, value);
    jsr_push(0x84C8);
    IncrementTempWord();
    jsr_pop();
    for (;;) {
        value = RD8_PTR(ZP->temp00);
        if (value == 0) {
            break;
        }
        jsr_push(0x84CF);
        pair = DrawGlyphPair();
        jsr_pop();
        jsr_push(0x84D2);
        exit = WriteGlyphMapRows();
        jsr_pop();
        exit.x = pair.x;
        exit.y = pair.y;
        exit.x_kept = exit.y_kept = false;
        drawn = true;
    }
    (void)drawn;
    exit.a = 0;
    exit.a_kept = false;
    exit.nz = 0;
    return exit;
}

void abi_DrawGlyphText(void)
{
    exit_shim(DrawGlyphText());
}
