/*
 * The card's kanji font: Shift-JIS codes made into glyph numbers, and glyphs copied out of the
 * font ROM, 16 by 16 dots or 12 by 12.
 */
#include "card.h"
#include "syscard.h"

enum {
    LEAD_BASE = 0x81, LEAD_LIMIT = 0x6F, LEAD_GAP_START = 0x1F, LEAD_GAP_END = 0x3F,
    LEAD_GAP_SIZE = 0x40, JIS_FIRST = 0x21,
    TRAIL_BASE = 0x40, TRAIL_LIMIT = 0xBD, TRAIL_HOLE = 0x3F, TRAIL_SECOND_ROW = 0x5F,
    TRAIL_FIRST_STEP = 0x20, TRAIL_SECOND_STEP = 0x3E,
    ROW_ALPHANUMERIC = 0x23, ROW_GREEK = 0x26, ROW_CYRILLIC = 0x27,
    ROW_FIRST_UNUSED = 0x28, ROW_KANJI = 0x30, ROW_LIMIT = 0x50, ROW_UNUSED_COUNT = 8,
    GLYPH_BYTES_16 = 0x20, GLYPH_ROWS_12 = 0x12, GLYPH_CLEAR_12 = 8,
    WINDOW_LOW = 0x60, WINDOW_HIGH = 0xA0, WINDOW_OFFSET_BITS = 0x1F,
    TWELVE_DOT = 0x01, MPR3 = 0x08, MPR4 = 0x10, MPR5 = 0x20, MPR6_BIT = 0x40
};

static bool add_overflows(uint8_t a, uint8_t b, bool carry)
{
    int r = (int8_t)a + (int8_t)b + (carry ? 1 : 0);

    return r < -128 || r > 127;
}

static bool sub_overflows(uint8_t a, uint8_t b, bool borrow)
{
    int r = (int8_t)a - (int8_t)b - (borrow ? 1 : 0);

    return r < -128 || r > 127;
}

/* The column's adjustment for the rows that are not laid out as the kanji are. */
static uint8_t adjust_column(uint8_t row, uint8_t column)
{
    if (row == ROW_ALPHANUMERIC) {
        if (column >= 0x61)
            column = (uint8_t)(column - 0x06);
        if (column >= 0x41)
            column = (uint8_t)(column - 0x07);
        return (uint8_t)(column - 0x0F);
    }
    if (row == ROW_GREEK)
        return column >= 0x41 ? (uint8_t)(column - 0x08) : column;
    return column >= 0x51 ? (uint8_t)(column - 0x0F) : column;
}

/*
 * @card 00:F1E3 sjis_to_font_index
 * WHAT IT DOES: Makes the Shift-JIS code in _AH, _AL into the number of its glyph in the font,
 * or refuses a code the font does not hold.
 * HOW: Shift-JIS to JIS: the first byte gives two rows, the second the column and which of the
 * two. The alphanumeric, Greek and Cyrillic rows close up their gaps; rows $28-$2F are empty and
 * those after them move down eight. The number is the column plus the row's first glyph from
 * FontRowStarts.
 * Entry: _AH, _AL. Exit, found: _AX the number, A its high byte, N and Z from it, C clear, V from
 * adding the carry, Y the row's offset in the table. Refused: C set; A, N, Z and V from where it
 * stopped. X kept, and Y if refused.
 */
FontIndex sjis_to_font_index(void)
{
    FontIndex index = { FONT_FOUND, 0, 0, 0, 0, false, 0, 0 };
    uint8_t pair, step, column, row_for_table, low;

    index.lead = rd8(CardAh);
    pair = (uint8_t)(index.lead - LEAD_BASE);
    if (pair >= LEAD_LIMIT) {
        index.failure = FONT_LEAD_HIGH;
        return index;
    }
    if (pair >= LEAD_GAP_START) {
        if (pair < LEAD_GAP_END) {
            index.failure = FONT_LEAD_GAP;
            return index;
        }
        pair = (uint8_t)(pair - LEAD_GAP_SIZE);
    }
    index.row = (uint8_t)((pair << 1) + JIS_FIRST + (pair >> 7));
    wr8(CardAh, index.row);
    index.trail = rd8(CardAl);
    step = (uint8_t)(index.trail - TRAIL_BASE);
    if (step >= TRAIL_LIMIT) {
        index.failure = FONT_TRAIL_HIGH;
        return index;
    }
    if (step == TRAIL_HOLE) {
        index.failure = FONT_TRAIL_GAP;
        return index;
    }
    index.column = step < TRAIL_HOLE ? (uint8_t)(step + 1) : step;
    index.second_half = index.column >= TRAIL_SECOND_ROW;
    if (index.second_half) {
        index.row++;
        wr8(CardAh, index.row);
        column = (uint8_t)(index.column - TRAIL_SECOND_STEP);
    } else {
        column = (uint8_t)(index.column + TRAIL_FIRST_STEP);
    }
    wr8(CardAl, column);
    row_for_table = index.row;
    if (index.row == ROW_ALPHANUMERIC || index.row == ROW_GREEK || index.row == ROW_CYRILLIC) {
        column = adjust_column(index.row, column);
        wr8(CardAl, column);
    } else if (index.row >= ROW_FIRST_UNUSED) {
        if (index.row < ROW_KANJI) {
            index.failure = FONT_ROW_UNUSED;
            return index;
        }
        if (index.row >= ROW_LIMIT) {
            index.failure = FONT_ROW_HIGH;
            return index;
        }
        row_for_table = (uint8_t)(index.row - ROW_UNUSED_COUNT);
    }
    index.table_index = (uint8_t)((uint8_t)(row_for_table - JIS_FIRST) << 1);
    low = rd8((uint16_t)(FontRowStarts + index.table_index));
    index.carry_in = (uint8_t)(column - JIS_FIRST) + low > 0xFF;
    wr8(CardAl, (uint8_t)(column - JIS_FIRST + low));
    wr8(CardAh, (uint8_t)(rd8((uint16_t)(FontRowStarts + index.table_index + 1)) + index.carry_in));
    return index;
}

/* C, V, A, N and Z as sjis_to_font_index leaves them; X and Y left to the caller. */
static void abi_font_index_shim(FontIndex index)
{
    uint8_t pair = (uint8_t)(index.lead - LEAD_BASE);
    uint8_t step = (uint8_t)(index.trail - TRAIL_BASE);

    abi_flag(P_C, index.failure != FONT_FOUND);
    switch (index.failure) {
    case FONT_FOUND:
        cpu.a = rd8(CardAh);
        abi_nz(cpu.a);
        abi_flag(P_V, add_overflows(rd8((uint16_t)(FontRowStarts + index.table_index + 1)), 0,
                                    index.carry_in != 0));
        return;
    case FONT_LEAD_HIGH:
    case FONT_LEAD_GAP:
        cpu.a = pair;
        abi_nz((uint8_t)(pair - (index.failure == FONT_LEAD_HIGH ? LEAD_LIMIT : LEAD_GAP_END)));
        abi_flag(P_V, sub_overflows(index.lead, LEAD_BASE, false));
        return;
    case FONT_TRAIL_HIGH:
    case FONT_TRAIL_GAP:
        cpu.a = step;
        abi_nz((uint8_t)(step - (index.failure == FONT_TRAIL_HIGH ? TRAIL_LIMIT : TRAIL_HOLE)));
        abi_flag(P_V, sub_overflows(index.trail, TRAIL_BASE, false));
        return;
    case FONT_ROW_UNUSED:
    case FONT_ROW_HIGH:
        cpu.a = index.row;
        abi_nz((uint8_t)(index.row - (index.failure == FONT_ROW_UNUSED ? ROW_KANJI : ROW_LIMIT)));
        abi_flag(P_V, index.second_half ? sub_overflows(index.column, TRAIL_SECOND_STEP, false)
                                        : add_overflows(index.column, TRAIL_FIRST_STEP, false));
        return;
    }
}

void abi_sjis_to_font_index(void)
{
    FontIndex index = sjis_to_font_index();

    abi_font_index_shim(index);
    if (index.failure == FONT_FOUND)
        cpu.y = index.table_index;
}

/*
 * @card 00:F300 put_font_byte
 * WHAT IT DOES: Writes a byte of a glyph where _BX points, and moves _BX on.
 * HOW: A store and a 16-bit increment.
 * Entry: A, _BX. Exit: N and Z from _BL after the increment, or from _BH if _BL went to zero. A,
 * X, Y, C, V kept.
 */
void put_font_byte(uint8_t byte)
{
    uint16_t at = (uint16_t)(rd8(CardBl) | (rd8(CardBh) << 8));

    wr8(at, byte);
    at++;
    wr8(CardBl, (uint8_t)at);
    wr8(CardBh, (uint8_t)(at >> 8));
}

void abi_put_font_byte(void)
{
    put_font_byte(cpu.a);
    abi_nz(rd8(CardBl) != 0 ? rd8(CardBl) : rd8(CardBh));
}

/*
 * @card 00:F2CC expand_12dot_glyph
 * WHAT IT DOES: Writes a 12-dot glyph as sixteen rows of two bytes where _BX points.
 * HOW: The font packs two rows of twelve dots in three bytes; each row is written as twelve
 * dots and four clear ones. Twelve rows, then eight bytes of zero for the last four.
 * Entry: CardPointerLo the glyph, _BX. Exit: A 0, X 0, Y $FF, N 1, Z 0, C 1. V kept. _BX moved on
 * by 24.
 */
void expand_12dot_glyph(void)
{
    uint16_t glyph = (uint16_t)(rd8(CardPointerLo) | (rd8(CardPointerHi) << 8));
    uint16_t end;

    for (unsigned i = 0; i < GLYPH_ROWS_12; i += 3) {
        uint8_t first = rd8((uint16_t)(glyph + i));
        uint8_t middle = rd8((uint16_t)(glyph + i + 1));
        uint8_t last = rd8((uint16_t)(glyph + i + 2));

        put_font_byte(first);
        put_font_byte(middle & 0xF0);
        wr8(CardFontShift, (uint8_t)(last << 4));
        put_font_byte((uint8_t)((middle << 4) | (last >> 4)));
        put_font_byte((uint8_t)(rd8(CardFontShift) & 0xF0));
    }
    end = (uint16_t)(rd8(CardBl) | (rd8(CardBh) << 8));
    for (int i = GLYPH_CLEAR_12 - 1; i >= 0; i--)
        wr8((uint16_t)(end + i), 0);
}

void abi_expand_12dot_glyph(void)
{
    expand_12dot_glyph();
    cpu.a = 0;
    cpu.x = 0;
    cpu.y = 0xFF;
    abi_nz(0xFF);
    abi_flag(P_C, true);
}

/* Maps the font's bank and the one after it at the window, keeping the two MPRs it replaces. */
static uint8_t map_font_window_shim(uint8_t window, uint8_t bank)
{
    uint8_t first = window == WINDOW_HIGH ? MPR5 : MPR3;
    uint16_t kept = (uint16_t)(CardFontSavedMprs + (window == WINDOW_HIGH ? 2 : 0));

    wr8(kept, tma(first));
    wr8((uint16_t)(kept + 1), tma((uint8_t)(first << 1)));
    tam(first, bank);
    tam((uint8_t)(first << 1), (uint8_t)(bank + 1));
    return first;
}

/* Puts the two MPRs back, and gives the second. */
static uint8_t unmap_font_window_shim(uint8_t window)
{
    uint8_t first = window == WINDOW_HIGH ? MPR5 : MPR3;
    uint16_t kept = (uint16_t)(CardFontSavedMprs + (window == WINDOW_HIGH ? 2 : 0));

    tam(first, rd8(kept));
    tam((uint8_t)(first << 1), rd8((uint16_t)(kept + 1)));
    return rd8((uint16_t)(kept + 1));
}

/*
 * @card 00:F124 ex_getfnt
 * WHAT IT DOES: Copies the glyph of the Shift-JIS code in _AX to _BX: 16 by 16 dots, or 12 by 12
 * if bit 0 of _DH is set, as 32 bytes either way.
 * HOW: sjis_to_font_index; the glyph's offset in the font is its number times 32, or 18 for 12
 * dots, in 24 bits. Its bank, counted from bank 8 for 16 dots or 24 for 12, goes to a window
 * that misses the caller's buffer, $6000 if _BX is at or above $A000 and $A000 if not; then the
 * glyph is copied or expanded, and the MPRs put back.
 * Entry: _AX, _BX, _DH. Exit, copied: A 0, X 0, Y $20 (or $FF for 12 dots), N and Z from the
 * second MPR put back, C set, V clear. Refused: A 1, N 0, Z 0, C set, V as sjis_to_font_index,
 * X and Y kept. _CL holds the window, CardPointerLo the glyph's address there.
 */
FontGet ex_getfnt(void)
{
    FontGet get = { { FONT_FOUND, 0, 0, 0, 0, false, 0, 0 }, false, 0 };
    uint8_t window, size;
    uint32_t offset;
    uint16_t number, glyph, buffer;
    uint8_t bank_high, bank;

    get.index = sjis_to_font_index();
    if (get.index.failure != FONT_FOUND)
        return get;
    window = (int8_t)(rd8(CardBh) - WINDOW_HIGH) >= 0 ? WINDOW_LOW : WINDOW_HIGH;
    wr8(CardCl, window);
    number = (uint16_t)(rd8(CardAl) | (rd8(CardAh) << 8));
    size = rd8(CardDh);
    get.twelve_dot = (size & TWELVE_DOT) != 0;
    offset = get.twelve_dot ? (uint32_t)number * 18u : (uint32_t)number * 32u;
    bank_high = (uint8_t)((offset >> 16) + (uint8_t)(size << 1) + 1 + (size >> 7));
    bank = (uint8_t)((((uint16_t)bank_high << 8) | ((offset >> 8) & 0xFF)) >> 5);
    glyph = (uint16_t)((((((offset >> 8) & WINDOW_OFFSET_BITS) + window) & 0xFF) << 8) | (offset & 0xFF));
    wr8(CardPointerLo, (uint8_t)glyph);
    wr8(CardPointerHi, (uint8_t)(glyph >> 8));
    wr8(CardCopyLo, bank);
    map_font_window_shim(window, bank);
    if (get.twelve_dot) {
        expand_12dot_glyph();
    } else {
        buffer = (uint16_t)(rd8(CardBl) | (rd8(CardBh) << 8));
        for (unsigned i = 0; i < GLYPH_BYTES_16; i++)
            wr8((uint16_t)(buffer + i), rd8((uint16_t)(glyph + i)));
    }
    get.restored_high = unmap_font_window_shim(window);
    return get;
}

void abi_ex_getfnt(void)
{
    ex_getfnt_exit_shim(ex_getfnt());
}

/* ex_getfnt's exit, for its wrapper and for a caller's that ends in it. */
void ex_getfnt_exit_shim(FontGet get)
{
    if (get.index.failure != FONT_FOUND) {
        abi_font_index_shim(get.index);
        cpu.a = 1;
        abi_nz(1);
        return;
    }
    cpu.a = 0;
    cpu.x = 0;
    cpu.y = get.twelve_dot ? 0xFF : GLYPH_BYTES_16;
    abi_nz(get.restored_high);
    abi_flag(P_C, true);
    abi_flag(P_V, false);
}
