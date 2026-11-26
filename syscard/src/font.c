/*
 * The card's kanji font: Shift-JIS codes made into glyph numbers, and glyphs copied out of the
 * font ROM, 16 by 16 dots or 12 by 12.
 *
 * FontRowStarts, the table of each JIS row's first glyph, is read by its address and an offset: it has no record yet
 * (row request to the typed access session).
 */
#include "card.h"
#include "card_access.h"
#include "syscard.h"

enum {
    LEAD_BASE = 0x81, LEAD_LIMIT = 0x6F, LEAD_GAP_START = 0x1F, LEAD_GAP_END = 0x3F,
    LEAD_GAP_SIZE = 0x40, JIS_FIRST = 0x21,
    TRAIL_BASE = 0x40, TRAIL_LIMIT = 0xBD, TRAIL_HOLE = 0x3F, TRAIL_SECOND_ROW = 0x5F,
    TRAIL_FIRST_STEP = 0x20, TRAIL_SECOND_STEP = 0x3E,
    ROW_ALPHANUMERIC = 0x23, ROW_GREEK = 0x26, ROW_CYRILLIC = 0x27,
    ROW_FIRST_UNUSED = 0x28, ROW_KANJI = 0x30, ROW_LIMIT = 0x50, ROW_UNUSED_COUNT = 8,
    ALPHA_FIRST_CUT = 0x61, ALPHA_FIRST_STEP = 0x06, ALPHA_SECOND_CUT = 0x41, ALPHA_SECOND_STEP = 0x07,
    ALPHA_STEP = 0x0F, GREEK_CUT = 0x41, GREEK_STEP = 0x08, CYRILLIC_CUT = 0x51, CYRILLIC_STEP = 0x0F,
    GLYPH_BYTES_16 = 0x20, GLYPH_ROW_BYTES_12 = 0x12, GLYPH_ROW_STEP_12 = 3, GLYPH_CLEAR_12 = 8,
    WINDOW_LOW = 0x60, WINDOW_HIGH = 0xA0, WINDOW_OFFSET_BITS = 0x1F, TWELVE_DOT = 0x01,
    MPR3 = 0x08, MPR4 = 0x10, MPR5 = 0x20, MPR6_BIT = 0x40,
    FONT_ADD_ROUNDS_12 = 8, FONT_SHIFT_ROUNDS_16 = 5, BANK_SHIFT_ROUNDS = 5, NIBBLE_SHIFT_ROUNDS = 4,
    SAVED_MPR3 = 3, SAVED_MPR5 = 5
};

/* C set, and A, N, Z and V as the instruction that sent it here left them; X and Y kept. */
static FontIndex font_refusal(uint8_t a, uint8_t compared, bool overflow)
{
    FontIndex index = { false, { 0 } };

    index.exit.a = a;
    index.exit.nz = compared;
    index.exit.c = true;
    index.exit.v = overflow;
    index.exit.set_a = index.exit.set_nz = index.exit.set_c = index.exit.set_v = true;
    return index;
}

/*
 * @card 00:F1E3 sjis_to_font_index
 * WHAT IT DOES: Makes the Shift-JIS code in _AH, _AL into the number of its glyph in the font,
 * or refuses a code the font does not hold.
 * HOW: Shift-JIS to JIS: the first byte gives two rows, the second the column and which of the
 * two. The alphanumeric, Greek and Cyrillic rows close up their gaps; rows $28-$2F are empty and
 * those after them move down eight. The number is the column plus the row's first glyph from
 * FontRowStarts.
 * WAITS: no.
 * Entry: _AH, _AL. Exit, found: _AX the number, A its high byte, N and Z from it, C clear, V from
 * adding the carry, Y the row's offset in the table. Refused: C set; A, N, Z and V from where it
 * stopped. X kept, and Y if refused.
 */
FontIndex sjis_to_font_index(void)
{
    FontIndex index = { true, { 0 } };
    Sum lead = sub8(RD8(CZP->a.hi), LEAD_BASE, true);
    Sum trail;
    Sum column;
    Sum row_sum;
    Sum low;
    Sum high;
    uint8_t a = lead.value;
    uint8_t row;
    uint8_t value;
    uint8_t y;

    if (a >= LEAD_LIMIT)
        return font_refusal(a, (uint8_t)(a - LEAD_LIMIT), lead.overflow);
    if (a >= LEAD_GAP_START) {
        if (a < LEAD_GAP_END)
            return font_refusal(a, (uint8_t)(a - LEAD_GAP_END), lead.overflow);
        a = (uint8_t)(a - LEAD_GAP_SIZE);
    }
    row = (uint8_t)((a << 1) + JIS_FIRST);
    WR8(CZP->a.hi, row);
    trail = sub8(RD8(CZP->a.lo), TRAIL_BASE, true);
    a = trail.value;
    if (a >= TRAIL_LIMIT)
        return font_refusal(a, (uint8_t)(a - TRAIL_LIMIT), trail.overflow);
    if (a == TRAIL_HOLE)
        return font_refusal(a, 0, trail.overflow);
    if (a < TRAIL_HOLE)
        a++;
    if (a >= TRAIL_SECOND_ROW) {
        row = INC8(CZP->a.hi);
        column = sub8(a, TRAIL_SECOND_STEP, true);
    } else {
        column = add8(a, TRAIL_FIRST_STEP, false);
    }
    a = column.value;
    WR8(CZP->a.lo, a);
    a = RD8(CZP->a.hi);
    if (a == ROW_ALPHANUMERIC) {
        a = RD8(CZP->a.lo);
        if (a >= ALPHA_FIRST_CUT)
            a = (uint8_t)(a - ALPHA_FIRST_STEP);
        if (a >= ALPHA_SECOND_CUT)
            a = (uint8_t)(a - ALPHA_SECOND_STEP);
        a = (uint8_t)(a - ALPHA_STEP);
        WR8(CZP->a.lo, a);
        a = RD8(CZP->a.hi);
    } else if (a == ROW_GREEK) {
        a = RD8(CZP->a.lo);
        if (a >= GREEK_CUT)
            a = (uint8_t)(a - GREEK_STEP);
        WR8(CZP->a.lo, a);
        a = RD8(CZP->a.hi);
    } else if (a == ROW_CYRILLIC) {
        a = RD8(CZP->a.lo);
        if (a >= CYRILLIC_CUT)
            a = (uint8_t)(a - CYRILLIC_STEP);
        WR8(CZP->a.lo, a);
        a = RD8(CZP->a.hi);
    } else if (a >= ROW_FIRST_UNUSED) {
        if (a < ROW_KANJI)
            return font_refusal(a, (uint8_t)(a - ROW_KANJI), column.overflow);
        if (a >= ROW_LIMIT)
            return font_refusal(a, (uint8_t)(a - ROW_LIMIT), column.overflow);
        a = (uint8_t)(a - ROW_UNUSED_COUNT);
    }
    y = (uint8_t)((uint8_t)(a - JIS_FIRST) << 1);
    value = RD8(CZP->a.lo);
    row_sum = sub8(value, JIS_FIRST, true);
    low = add8(row_sum.value, rd8((uint16_t)(FontRowStarts + y)), false);
    WR8(CZP->a.lo, low.value);
    high = add8(rd8((uint16_t)(FontRowStarts + 1 + y)), 0, low.carry);
    WR8(CZP->a.hi, high.value);
    (void)row;
    index.exit.a = index.exit.nz = high.value;
    index.exit.y = y;
    index.exit.c = false;
    index.exit.v = high.overflow;
    index.exit.set_a = index.exit.set_nz = index.exit.set_y = index.exit.set_c = index.exit.set_v = true;
    return index;
}

void abi_sjis_to_font_index(void)
{
    card_exit_apply(sjis_to_font_index().exit);
}

/*
 * @card 00:F300 put_font_byte
 * WHAT IT DOES: Writes a byte of a glyph where _BX points, and moves _BX on.
 * HOW: A store through _BX and a 16-bit increment.
 * WAITS: no.
 * Entry: A, _BX. Exit: N and Z from _BL after the increment, or from _BH if _BL went to zero. A,
 * X, Y, C, V kept.
 */
CardExit put_font_byte(uint8_t byte)
{
    CardExit e = { 0 };
    uint8_t low;

    WR8_PTR(CZP->b.lo, byte);
    low = INC8(CZP->b.lo);
    e.nz = low != 0 ? low : INC8(CZP->b.hi);
    e.set_nz = true;
    return e;
}

void abi_put_font_byte(void)
{
    card_exit_apply(put_font_byte(cpu.a));
}

/*
 * @card 00:F2CC expand_12dot_glyph
 * WHAT IT DOES: Writes a 12-dot glyph as sixteen rows of two bytes where _BX points.
 * HOW: The font packs two rows of twelve dots in three bytes; each row is written as twelve
 * dots and four clear ones. Twelve rows, then eight bytes of zero for the last four.
 * WAITS: no.
 * Entry: CardPointerLo the glyph, _BX. Exit: A 0, X 0, Y $FF, N 1, Z 0, C 1. V kept. _BX moved on
 * by 24.
 */
CardExit expand_12dot_glyph(void)
{
    CardExit e = { 0 };
    uint8_t y = 0;

    do {
        uint8_t first = RD8_PTR_Y(CZP->pointer.lo, y++);
        uint8_t middle;
        uint8_t last;
        uint8_t a;

        jsr_push(0xF2D2);
        (void)put_font_byte(first);
        jsr_pop();
        middle = RD8_PTR_Y(CZP->pointer.lo, y++);
        push8(middle);
        jsr_push(0xF2DA);
        (void)put_font_byte((uint8_t)(middle & 0xF0));
        jsr_pop();
        last = RD8_PTR_Y(CZP->pointer.lo, y++);
        WR8(CRAM->font_shift, last);
        a = pull8();
        for (unsigned round = 0; round < NIBBLE_SHIFT_ROUNDS; round++) {
            uint8_t shift = RD8(CRAM->font_shift);

            WR8(CRAM->font_shift, (uint8_t)(shift << 1));
            a = (uint8_t)((a << 1) | (shift >> 7));
        }
        jsr_push(0xF2EC);
        (void)put_font_byte(a);
        jsr_pop();
        a = (uint8_t)(RD8(CRAM->font_shift) & 0xF0);
        jsr_push(0xF2F3);
        (void)put_font_byte(a);
        jsr_pop();
    } while (y < GLYPH_ROW_BYTES_12);
    for (int i = GLYPH_CLEAR_12 - 1; i >= 0; i--)
        WR8_PTR_Y(CZP->b.lo, (uint8_t)i, 0);
    e.a = 0;
    e.x = 0;
    e.y = 0xFF;
    e.nz = 0xFF;
    e.c = true;
    e.set_a = e.set_x = e.set_y = e.set_nz = e.set_c = true;
    return e;
}

void abi_expand_12dot_glyph(void)
{
    card_exit_apply(expand_12dot_glyph());
}

/* ASL pointer, ROL pointer+1, ROL copy: the 24-bit value doubled, each byte read and written. */
static void double_glyph_offset(void)
{
    uint8_t value = RD8(CZP->pointer.lo);
    uint8_t carry = (uint8_t)(value >> 7);

    WR8(CZP->pointer.lo, (uint8_t)(value << 1));
    value = RD8(CZP->pointer.hi);
    WR8(CZP->pointer.hi, (uint8_t)((value << 1) | carry));
    carry = (uint8_t)(value >> 7);
    value = RD8(CZP->copy.lo);
    WR8(CZP->copy.lo, (uint8_t)((value << 1) | carry));
}

/*
 * @card 00:F124 ex_getfnt
 * WHAT IT DOES: Copies the glyph of the Shift-JIS code in _AX to _BX: 16 by 16 dots, or 12 by 12
 * if bit 0 of _DH is set, as 32 bytes either way.
 * HOW: sjis_to_font_index; the glyph's offset in the font is its number times 32, or 18 for 12
 * dots, in 24 bits. Its bank, counted from bank 8 for 16 dots or 24 for 12, goes to a window
 * that misses the caller's buffer, $6000 if _BX is at or above $A000 and $A000 if not; then the
 * glyph is copied or expanded, and the MPRs put back.
 * WAITS: no.
 * Entry: _AX, _BX, _DH. Exit, copied: A 0, X 0, Y $20 (or $FF for 12 dots), N and Z from the
 * second MPR put back, C set, V clear. Refused: A 1, N 0, Z 0, C set, V as sjis_to_font_index,
 * X and Y kept. _CL holds the window, CardPointerLo the glyph's address there.
 */
CardExit ex_getfnt(void)
{
    CardExit e = { 0 };
    FontIndex index;
    Sum adjust;
    Sum sum;
    uint8_t a;
    uint8_t carry;
    uint8_t size;
    bool twelve_dot;
    bool high_window;
    unsigned round;

    jsr_push(0xF127);
    index = sjis_to_font_index();
    jsr_pop();
    if (!index.found) {
        e = index.exit;
        e.a = 1;
        e.nz = 1;
        e.set_a = e.set_nz = true;
        return e;
    }
    WR8(CZP->c.lo, WINDOW_HIGH);
    adjust = sub8(RD8(CZP->b.hi), WINDOW_HIGH, true);
    high_window = (adjust.value & 0x80) != 0;
    if (high_window) {
        WR8(CRAM->saved_mprs[SAVED_MPR5], tma(MPR5));
        WR8(CRAM->saved_mprs[SAVED_MPR5 + 1], tma(MPR6_BIT));
    } else {
        WR8(CZP->c.lo, WINDOW_LOW);
        WR8(CRAM->saved_mprs[SAVED_MPR3], tma(MPR3));
        WR8(CRAM->saved_mprs[SAVED_MPR3 + 1], tma(MPR4));
    }
    a = RD8(CZP->a.lo);
    WR8(CZP->pointer.lo, a);
    a = RD8(CZP->a.hi);
    WR8(CZP->pointer.hi, a);
    WR8(CZP->copy.lo, 0);
    size = RD8(CZP->d.hi);
    twelve_dot = (size & TWELVE_DOT) != 0;
    if (twelve_dot) {
        for (round = 0; round < FONT_ADD_ROUNDS_12; round++) {
            uint8_t kept = RD8(CZP->pointer.lo);
            Sum low_sum = add8(kept, RD8(CZP->a.lo), false);
            Sum high_sum;
            Sum top_sum;

            WR8(CZP->pointer.lo, low_sum.value);
            kept = RD8(CZP->pointer.hi);
            high_sum = add8(kept, RD8(CZP->a.hi), low_sum.carry);
            WR8(CZP->pointer.hi, high_sum.value);
            top_sum = add8(0, RD8(CZP->copy.lo), high_sum.carry);
            WR8(CZP->copy.lo, top_sum.value);
        }
        double_glyph_offset();
    } else {
        for (round = 0; round < FONT_SHIFT_ROUNDS_16; round++)
            double_glyph_offset();
    }
    size = RD8(CZP->d.hi);
    carry = (uint8_t)(size >> 7);
    sum = add8((uint8_t)(((uint8_t)(size << 1)) + 1), RD8(CZP->copy.lo), carry != 0);
    WR8(CZP->copy.lo, sum.value);
    a = RD8(CZP->pointer.hi);
    for (round = 0; round < BANK_SHIFT_ROUNDS; round++) {
        uint8_t shifted = RD8(CZP->copy.lo);

        WR8(CZP->copy.lo, (uint8_t)(shifted >> 1));
        a = (uint8_t)((a >> 1) | (uint8_t)(shifted << 7));
    }
    WR8(CZP->copy.lo, a);
    a = (uint8_t)(RD8(CZP->pointer.hi) & WINDOW_OFFSET_BITS);
    sum = add8(a, RD8(CZP->c.lo), false);
    WR8(CZP->pointer.hi, sum.value);
    a = RD8(CZP->copy.lo);
    if ((RD8(CZP->c.lo) & 0x80) != 0) {
        tam(MPR5, a);
        tam(MPR6_BIT, (uint8_t)(a + 1));
    } else {
        tam(MPR3, a);
        tam(MPR4, (uint8_t)(a + 1));
    }
    if (RD8(CZP->d.hi) & TWELVE_DOT) {
        jsr_push(0xF1C8);
        (void)expand_12dot_glyph();
        jsr_pop();
    } else {
        for (uint8_t y = 0; y != GLYPH_BYTES_16; y++) {
            uint8_t byte = RD8_PTR_Y(CZP->pointer.lo, y);

            WR8_PTR_Y(CZP->b.lo, y, byte);
        }
    }
    if ((RD8(CZP->c.lo) & 0x80) != 0) {
        tam(MPR5, RD8(CRAM->saved_mprs[SAVED_MPR5]));
        a = RD8(CRAM->saved_mprs[SAVED_MPR5 + 1]);
        tam(MPR6_BIT, a);
    } else {
        tam(MPR3, RD8(CRAM->saved_mprs[SAVED_MPR3]));
        a = RD8(CRAM->saved_mprs[SAVED_MPR3 + 1]);
        tam(MPR4, a);
    }
    e.a = 0;
    e.x = 0;
    e.y = twelve_dot ? 0xFF : GLYPH_BYTES_16;
    e.nz = a;
    e.c = true;
    e.v = false;
    e.set_a = e.set_x = e.set_y = e.set_nz = e.set_c = e.set_v = true;
    return e;
}

void abi_ex_getfnt(void)
{
    card_exit_apply(ex_getfnt());
}
