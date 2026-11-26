/* The card's boot, from bank 1 at $C000: the display and the start screen. */
#include "card.h"
#include "card_access.h"
#include "syscard.h"

enum {
    BOOT_PALETTE = 0xC800, BOOT_PALETTE_SETS = 4, COLOR_WRITE = 0x02,
    COLOR_ADDRESS_HIGH = 1, COLOR_SETS = 2, LOAD_FONT_EXIT_Y = 8,
    VDC_REG_MAWR = 0x00, VDC_REG_VWR = 0x02, TEXT_ROW_SHIFT = 5, TEXT_COLUMN_BITS = 0x3F,
    TEXT_END = 0xFF, TEXT_ATTRIBUTE_BIAS = 3, TEXT_CURSOR_EXIT_A = VDC_REG_VWR
};

/* The start screen's texts in bank 1, each a list for print_text. */
enum {
    TEXT_TITLE = 0xC97B, TEXT_SUPER_RAM = 0xCA0C, TEXT_NO_SUPER_RAM = 0xCA56,
    TEXT_PUSH_RUN = 0xCA84, TEXT_BLANK_LINE = 0xCA99, TEXT_BLANK_LINE_CHARACTERS = 0xCA9C,
    TEXT_JUST_A_MOMENT = 0xCAB1, TEXT_SET_DISC = 0xCAC6, TEXT_CLOSE_COVER = 0xCADB
};

enum {
    SUPER_RAM_FLAG = 0x80, SUPER_RAM_KEY_A = 0xAA, SUPER_RAM_KEY_B = 0x55,
    BLINK_PERIOD = 0x3F, BLINK_SHOWN = 0x20, PROMPT_COLUMN = 8, PROMPT_ROW = 0x12,
    PROMPT_ATTRIBUTE = 3, PROMPT_END = 0x14, PAD_SELECT = 0x04, PAD_RUN = 0x08,
    CD_NO_DISC = 0x0B, CD_COVER_OPEN = 0x0D, DRIVE_WAIT_FRAMES = 0x46
};

/* _AX to a text list, as print_text takes it. */
static void set_text_list(uint16_t list)
{
    WR8(CZP->a.lo, (uint8_t)list);
    WR8(CZP->a.hi, (uint8_t)(list >> 8));
}

/* Selects a VDC register, keeping a copy of the choice. */
static void select_vdc_register(uint8_t reg)
{
    WR8(CZP->vdc_reg, reg);
    WR8(CVDC->addr, reg);
}

/*
 * @card 01:CBC3 boot_display
 * WHAT IT DOES: Sets the display up at boot: VRAM cleared, the start screen's palettes asked for,
 * the font and the title picture loaded, and the background shown.
 * HOW: clear_vram; the background and sprite colours both from $C800 in bank 1, four 16-colour
 * sets each, and CardColorCommand 2 to have the next frame write them; ex_vsync waits that
 * frame; load_font; ex_bgon.
 * WAITS: yes, a frame, through ex_vsync; the stack there is the caller's frame and this call's two bytes.
 * Entry: bank 1 at $C000. Exit: as load_font: A what MPR4 held, N and Z from it, X 0, Y 8,
 * C 0, V 0.
 */
CardExit boot_display(void)
{
    CardExit e;

    jsr_push(0xCBC5);
    (void)clear_vram();
    jsr_pop();
    wr8(CardColorBackground, (uint8_t)BOOT_PALETTE);
    wr8((uint16_t)(CardColorBackground + COLOR_ADDRESS_HIGH), (uint8_t)(BOOT_PALETTE >> 8));
    wr8(CardColorSprites, (uint8_t)BOOT_PALETTE);
    wr8((uint16_t)(CardColorSprites + COLOR_ADDRESS_HIGH), (uint8_t)(BOOT_PALETTE >> 8));
    wr8((uint16_t)(CardColorBackground + COLOR_SETS), BOOT_PALETTE_SETS);
    wr8((uint16_t)(CardColorSprites + COLOR_SETS), BOOT_PALETTE_SETS);
    WR8(CRAM->color_command, COLOR_WRITE);
    jsr_push(0xCBE9);
    (void)ex_vsync();
    jsr_pop();
    jsr_push(0xCBEC);
    e = load_font();
    jsr_pop();
    jsr_push(0xCBEF);
    ex_bgon();
    jsr_pop();
    return e;
}

void abi_boot_display(void)
{
    card_exit_apply(boot_display());
}

/*
 * @card 01:C922 text_cursor
 * WHAT IT DOES: Points VRAM writes at a place on the start screen, by column and row.
 * HOW: The row times 32, with the column's low six bits set into it, in CardPointerLo-Hi, to
 * the write address register; then the data register selected.
 * WAITS: no.
 * Entry: X column, Y row. Exit: A 2, N 0, Z 0, C 0, V bit 1 of the row. X, Y kept.
 * CardPointerLo-Hi the address.
 */
CardExit text_cursor(uint8_t column, uint8_t row)
{
    CardExit e = { 0 };
    uint8_t value;
    uint8_t low;
    uint8_t held;

    WR8(CZP->pointer.lo, 0);
    WR8(CZP->pointer.hi, row);
    for (unsigned i = 0; i < 3; i++) {
        bool carry;

        value = RD8(CZP->pointer.hi);
        WR8(CZP->pointer.hi, (uint8_t)(value >> 1));
        carry = (value & 1) != 0;
        low = RD8(CZP->pointer.lo);
        WR8(CZP->pointer.lo, (uint8_t)((low >> 1) | (carry ? 0x80 : 0)));
    }
    held = TSB8(CZP->pointer.lo, (uint8_t)(column & TEXT_COLUMN_BITS));
    select_vdc_register(VDC_REG_MAWR);
    value = RD8(CZP->pointer.lo);
    WR8(CVDC->data_lo, value);
    value = RD8(CZP->pointer.hi);
    WR8(CVDC->data_hi, value);
    select_vdc_register(VDC_REG_VWR);
    e.a = VDC_REG_VWR;
    e.nz = VDC_REG_VWR;
    e.c = false;
    e.v = (held & 0x40) != 0;
    e.set_a = e.set_nz = e.set_c = e.set_v = true;
    return e;
}

void abi_text_cursor(void)
{
    card_exit_apply(text_cursor(cpu.x, cpu.y));
}

/* The list from index, in a text's characters or at a header; what the exit shows is the last header's. */
static CardExit text_loop(uint8_t y, bool in_characters)
{
    CardExit e = { 0 };

    for (;;) {
        if (!in_characters) {
            uint8_t mark = RD8_PTR_Y(CZP->a.lo, y);
            uint8_t column;
            uint8_t row;
            CardExit placed;

            if (mark == TEXT_END) {
                e.a = TEXT_END;
                e.nz = 0;
                e.y = y;
                e.c = true;
                e.set_a = e.set_nz = e.set_y = e.set_c = true;
                return e;
            }
            WR8(CZP->d.hi, (uint8_t)(mark + TEXT_ATTRIBUTE_BIAS));
            y++;
            column = RD8_PTR_Y(CZP->a.lo, y);
            e.x = column;
            e.set_x = true;
            y++;
            row = RD8_PTR_Y(CZP->a.lo, y);
            push8(y);
            jsr_push(0xC968);
            placed = text_cursor(column, row);
            jsr_pop();
            e.v = placed.v;
            e.set_v = true;
            y = pull8();
            y++;
        }
        in_characters = false;
        for (;;) {
            uint8_t character = RD8_PTR_Y(CZP->a.lo, y);
            uint8_t attribute;

            y++;
            if (character == TEXT_END) {
                break;
            }
            WR8(CVDC->data_lo, character);
            attribute = RD8(CZP->d.hi);
            WR8(CVDC->data_hi, attribute);
        }
    }
}

/*
 * @card 01:C950 print_text
 * WHAT IT DOES: Writes a list of texts on the start screen.
 * HOW: Each text is a header, the attribute byte less 3 then the column and the row, and its
 * characters to an $FF; an $FF in place of a header ends the list. Each header's attribute plus
 * 3 goes to CardDh, the high byte written with every character; text_cursor places it. Falls
 * into print_text_run for the characters.
 * WAITS: no.
 * Entry: _AX the list. Exit: A $FF, Y the end mark's index, N 0, Z 1, C 1; X the last text's
 * column and V bit 1 of its row, or both kept if the list is empty. CardDh, CardPointerLo-Hi.
 */
CardExit print_text(void)
{
    return text_loop(0, false);
}

void abi_print_text(void)
{
    card_exit_apply(print_text());
}

/*
 * @card 01:C96A print_text_run
 * WHAT IT DOES: Writes a text's characters from index Y, and goes on with the rest of the list.
 * HOW: Each character to the VDC's data port with CardDh as the high byte, to an $FF; then as print_text.
 * WAITS: no.
 * Entry: _AX the list, Y the index of the characters, CardDh. Exit: as print_text.
 */
CardExit print_text_run(uint8_t y)
{
    return text_loop(y, true);
}

void abi_print_text_run(void)
{
    card_exit_apply(print_text_run(cpu.y));
}

/* A list printed from the start, between its frame. */
static CardExit print_list(uint16_t list, uint16_t ret)
{
    CardExit e;

    set_text_list(list);
    jsr_push(ret);
    e = print_text();
    jsr_pop();
    return e;
}

/*
 * @card 01:C860 start_screen
 * WHAT IT DOES: The card's start screen: shows the title and whether the Super System Card's RAM is
 * there, blinks PUSH RUN BUTTON! until run or select is pressed, and on run waits for a disc.
 * HOW: print_text with the title, then ex_memopen picks one of two RAM messages ($AA, $55 to
 * SuperRamKey first for the Super System Card's own). Each pass of the wait shows the prompt for 32
 * frames of every 64 and blanks it for the rest, until pad 1's CardPadPressed is run alone or select
 * alone. On run: the line cleared, JUST A MOMENT..., then cd_test_unit until it passes. Not ready
 * (4) or an error it does not name waits 70 frames and tries again; no disc ($0B) says so, resets
 * the CD unit and starts over; cover open ($0D) says so and starts over.
 * WAITS: yes, on the pad and the drive, with frames through ex_vsync; the stack there is the caller's
 * frame and this call's two bytes.
 * Entry: bank 1 at $C000. Exit, select: A 1, X 8, Y $14, N 0, Z 0, C 1, V 1. Run: A 0, and the
 * rest as cd_test_unit leaves them over X 8, Y $14, V 1 (X 0 if it waited); I clear if it ever
 * waited. _AX, CardDh, CardPointerLo-Hi.
 */
CardExit start_screen(void)
{
    CardExit r = { 0 };
    CardExit step;
    MemOpen open;
    uint8_t value;
    uint8_t pressed;
    uint8_t x;

    for (;;) {
        r = card_exit_over(r, print_list(TEXT_TITLE, 0xC86B));
        jsr_push(0xC86E);
        open = ex_memopen();
        jsr_pop();
        if (open.present) {
            if (open.bank & SUPER_RAM_FLAG) {
                wr8(SuperRamKey, SUPER_RAM_KEY_A);
                wr8(SuperRamKey, SUPER_RAM_KEY_B);
            }
            r = card_exit_over(r, print_list(TEXT_SUPER_RAM, 0xC88A));
        } else {
            r = card_exit_over(r, print_list(TEXT_NO_SUPER_RAM, 0xC897));
        }
        for (;;) {
            value = RD8(CRAM->frame_tick);
            if ((uint8_t)(value & BLINK_PERIOD) < BLINK_SHOWN) {
                r = card_exit_over(r, print_list(TEXT_PUSH_RUN, 0xC8AB));
            } else {
                jsr_push(0xC8B4);
                step = text_cursor(PROMPT_COLUMN, PROMPT_ROW);
                jsr_pop();
                r = card_exit_over(r, step);
                WR8(CZP->d.hi, PROMPT_ATTRIBUTE);
                set_text_list(TEXT_BLANK_LINE_CHARACTERS);
                jsr_push(0xC8C4);
                step = print_text_run(0);
                jsr_pop();
                r = card_exit_over(r, step);
            }
            pressed = RD8(CRAM->pad_pressed[0]);
            if (pressed == PAD_RUN) {
                break;
            }
            if (pressed == PAD_SELECT) {
                r.a = 1;
                r.nz = 1;
                r.c = true;
                r.set_a = r.set_nz = r.set_c = true;
                return r;
            }
        }
        r = card_exit_over(r, print_list(TEXT_BLANK_LINE, 0xC8DC));
        r = card_exit_over(r, print_list(TEXT_JUST_A_MOMENT, 0xC8E6));
        for (;;) {
            uint8_t error;
            bool again = false;

            jsr_push(0xC8E9);
            step = cd_test_unit(r.x);
            jsr_pop();
            r = card_exit_over(r, step);
            if (!r.c) {
                r.a = 0;
                r.set_a = true;
                return r;
            }
            error = r.a;
            if (error == CD_NO_DISC) {
                r = card_exit_over(r, print_list(TEXT_SET_DISC, 0xC8FF));
                jsr_push(0xC902);
                (void)reset_cd_unit();
                jsr_pop();
                break;
            }
            if (error == CD_COVER_OPEN) {
                r = card_exit_over(r, print_list(TEXT_CLOSE_COVER, 0xC913));
                break;
            }
            x = DRIVE_WAIT_FRAMES;
            do {
                push8(x);
                jsr_push(0xC91C);
                (void)ex_vsync();
                jsr_pop();
                x = pull8();
                x--;
            } while (x != 0);
            r.x = 0;
            r.set_x = true;
            (void)again;
        }
    }
}

void abi_start_screen(void)
{
    card_exit_apply(start_screen());
}
