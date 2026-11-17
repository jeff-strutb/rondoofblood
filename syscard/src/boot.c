/* The card's boot, from bank 1 at $C000: the display and the start screen. */
#include "card.h"
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
    wr8(CardAl, (uint8_t)list);
    wr8(CardAh, (uint8_t)(list >> 8));
}

/* Selects a VDC register, keeping a copy of the choice. */
static void select_vdc_register(uint8_t reg)
{
    wr8(CardVdcReg, reg);
    wr8(VdcAddress, reg);
}

/*
 * @card 01:CBC3 boot_display
 * WHAT IT DOES: Sets the display up at boot: VRAM cleared, the start screen's palettes asked for,
 * the font and the title picture loaded, and the background shown.
 * HOW: clear_vram; the background and sprite colours both from $C800 in bank 1, four 16-colour
 * sets each, and CardColorCommand 2 to have the next frame write them; ex_vsync waits that
 * frame; load_font; ex_bgon.
 * Entry: bank 1 at $C000. Exit: as load_font: A what MPR4 held, N and Z from it, X 0, Y 8,
 * C 0, V 0.
 */
uint8_t boot_display(void)
{
    uint8_t mpr4;

    clear_vram();
    wr8(CardColorBackground, (uint8_t)BOOT_PALETTE);
    wr8((uint16_t)(CardColorBackground + COLOR_ADDRESS_HIGH), (uint8_t)(BOOT_PALETTE >> 8));
    wr8(CardColorSprites, (uint8_t)BOOT_PALETTE);
    wr8((uint16_t)(CardColorSprites + COLOR_ADDRESS_HIGH), (uint8_t)(BOOT_PALETTE >> 8));
    wr8((uint16_t)(CardColorBackground + COLOR_SETS), BOOT_PALETTE_SETS);
    wr8((uint16_t)(CardColorSprites + COLOR_SETS), BOOT_PALETTE_SETS);
    wr8(CardColorCommand, COLOR_WRITE);
    ex_vsync();
    mpr4 = load_font();
    ex_bgon();
    return mpr4;
}

void abi_boot_display(void)
{
    cpu.a = boot_display();
    cpu.x = 0;
    cpu.y = LOAD_FONT_EXIT_Y;
    abi_nz(cpu.a);
    abi_flag(P_C, false);
    abi_flag(P_V, false);
}

/*
 * @card 01:C922 text_cursor
 * WHAT IT DOES: Points VRAM writes at a place on the start screen, by column and row.
 * HOW: The row times 32, with the column's low six bits set into it, in CardPointerLo-Hi, to
 * the write address register; then the data register selected.
 * Entry: X column, Y row. Exit: A 2, N 0, Z 0, C 0, V bit 1 of the row. X, Y kept.
 * CardPointerLo-Hi the address.
 */
uint16_t text_cursor(uint8_t column, uint8_t row)
{
    uint16_t at = (uint16_t)((row << TEXT_ROW_SHIFT) | (column & TEXT_COLUMN_BITS));

    wr8(CardPointerLo, (uint8_t)at);
    wr8(CardPointerHi, (uint8_t)(at >> 8));
    select_vdc_register(VDC_REG_MAWR);
    wr8(VdcDataLo, (uint8_t)at);
    wr8(VdcDataHi, (uint8_t)(at >> 8));
    select_vdc_register(VDC_REG_VWR);
    return at;
}

/* V is bit 6 of the address's low byte before the column went in: bit 1 of the row. */
static void text_cursor_exit_shim(uint8_t row)
{
    cpu.a = TEXT_CURSOR_EXIT_A;
    abi_nz(cpu.a);
    abi_flag(P_C, false);
    abi_flag(P_V, ((row << TEXT_ROW_SHIFT) & P_V) != 0);
}

void abi_text_cursor(void)
{
    text_cursor(cpu.x, cpu.y);
    text_cursor_exit_shim(cpu.y);
}

static uint8_t text_list_byte(uint8_t index)
{
    uint16_t list = (uint16_t)(rd8(CardAl) | (rd8(CardAh) << 8));

    return rd8((uint16_t)(list + index));
}

/* The list from index, in a text's characters or at a header. */
static TextRun write_texts(uint8_t index, bool in_characters)
{
    TextRun run = { 0, false, 0, 0 };

    for (;;) {
        if (!in_characters) {
            uint8_t mark = text_list_byte(index);

            if (mark == TEXT_END) {
                run.end = index;
                return run;
            }
            wr8(CardDh, (uint8_t)(mark + TEXT_ATTRIBUTE_BIAS));
            run.column = text_list_byte(++index);
            run.row = text_list_byte(++index);
            text_cursor(run.column, run.row);
            run.placed = true;
            index++;
        }
        in_characters = false;
        for (;;) {
            uint8_t character = text_list_byte(index++);

            if (character == TEXT_END)
                break;
            wr8(VdcDataLo, character);
            wr8(VdcDataHi, rd8(CardDh));
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
 * Entry: _AX the list. Exit: A $FF, Y the end mark's index, N 0, Z 1, C 1; X the last text's
 * column and V bit 1 of its row, or both kept if the list is empty. CardDh, CardPointerLo-Hi.
 */
TextRun print_text(void)
{
    return write_texts(0, false);
}

static void print_text_exit_shim(TextRun run)
{
    if (run.placed) {
        cpu.x = run.column;
        text_cursor_exit_shim(run.row);
    }
    cpu.a = TEXT_END;
    cpu.y = run.end;
    abi_nz(0);
    abi_flag(P_C, true);
}

void abi_print_text(void)
{
    print_text_exit_shim(print_text());
}

/*
 * @card 01:C96A print_text_run
 * WHAT IT DOES: Writes characters of a text from print_text's list, then the texts after it.
 * HOW: Each byte to the VDC's data port, low byte, and CardDh to its high byte, until an $FF;
 * then on at print_text_record with the next header.
 * Entry: _AX the list, Y the index of the next character, CardDh, the VDC set to write. Exit: as
 * print_text, X and V kept if no header follows.
 */
TextRun print_text_run(uint8_t index)
{
    return write_texts(index, true);
}

void abi_print_text_run(void)
{
    print_text_exit_shim(print_text_run(cpu.y));
}

static void show_text(uint16_t list)
{
    set_text_list(list);
    print_text();
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
 * Entry: bank 1 at $C000. Exit, select: A 1, X 8, Y $14, N 0, Z 0, C 1, V 1. Run: A 0, and the
 * rest as cd_test_unit leaves them over X 8, Y $14, V 1 (X 0 if it waited); I clear if it ever
 * waited. _AX, CardDh, CardPointerLo-Hi.
 */
StartScreen start_screen(void)
{
    StartScreen screen = { .choice = START_RUN, .drive = { .tested = true }, .waited = false };

    for (;;) {
        MemOpen open;
        uint8_t pressed;

        show_text(TEXT_TITLE);
        open = ex_memopen();
        if (open.present) {
            if (open.bank & SUPER_RAM_FLAG) {
                wr8(SuperRamKey, SUPER_RAM_KEY_A);
                wr8(SuperRamKey, SUPER_RAM_KEY_B);
            }
            show_text(TEXT_SUPER_RAM);
        } else {
            show_text(TEXT_NO_SUPER_RAM);
        }
        for (;;) {
            if ((rd8(CardFrameTick) & BLINK_PERIOD) < BLINK_SHOWN) {
                show_text(TEXT_PUSH_RUN);
            } else {
                text_cursor(PROMPT_COLUMN, PROMPT_ROW);
                wr8(CardDh, PROMPT_ATTRIBUTE);
                set_text_list(TEXT_BLANK_LINE_CHARACTERS);
                print_text_run(0);
            }
            pressed = rd8(CardPadPressed);
            if (pressed == PAD_RUN)
                break;
            if (pressed == PAD_SELECT) {
                screen.choice = START_SELECT;
                return screen;
            }
        }
        show_text(TEXT_BLANK_LINE);
        show_text(TEXT_JUST_A_MOMENT);
        for (;;) {
            uint8_t error;

            screen.drive.test = cd_test_unit();
            if (!screen.drive.test.failed)
                return screen;
            error = cd_stat_result(screen.drive);
            if (error == CD_NO_DISC) {
                show_text(TEXT_SET_DISC);
                reset_cd_unit();
                break;
            }
            if (error == CD_COVER_OPEN) {
                show_text(TEXT_CLOSE_COVER);
                break;
            }
            for (unsigned frame = 0; frame < DRIVE_WAIT_FRAMES; frame++)
                ex_vsync();
            screen.waited = true;
        }
    }
}

/* Both ways out follow a prompt line: the last text at column 8, row $12, its end at $14. */
static void start_screen_exit_shim(StartScreen screen)
{
    cpu.x = PROMPT_COLUMN;
    cpu.y = PROMPT_END;
    abi_flag(P_V, true);
    if (screen.waited) {
        cpu.x = 0;
        abi_flag(P_I, false);
    }
    if (screen.choice == START_SELECT) {
        cpu.a = START_SELECT;
        abi_nz(cpu.a);
        abi_flag(P_C, true);
        return;
    }
    cd_stat_exit_shim(screen.drive);
    cpu.a = START_RUN;
}

void abi_start_screen(void)
{
    start_screen_exit_shim(start_screen());
}
