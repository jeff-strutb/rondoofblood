/*
 * Raster effects: the screen split into horizontal bands, each with its own
 * scroll. At vblank the first band's compare line goes in; each raster interrupt
 * then runs the frame's effect, one step per band: it writes that band's scroll,
 * and sets the line of the next, or runs out of bands. Band 8 of the scroll tables
 * is the working band most effects build their next scroll in.
 *
 * Every handler ends on the frame interrupt's exit, which pulls every register
 * and returns from the interrupt: registers and flags are dead there, only memory
 * and hardware are visible, so no handler has a wrapper's exit state.
 *
 * Each call into WriteBandScroll and the VDC selects is framed as the original's
 * JSR is: the return address goes on the stack for the callee's own pushes to land
 * where the original's do. The address is each call's own, so every site names it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"

enum {
    RCR_BASE = 0x40, VDC_REG_RCR = 0x06, WORK_BAND = 8, CR_BG_OFF_MASK = 0xBF, CR_BG_ON = 0x40,
    PALETTE_SAVED_COLOURS = 0x0F, COLOUR_FILL = 0x07, COLOUR_SPRITE_BANK = 0x01
};

/* JSR WriteBandScroll, JSR SelectVdcRegister and JSR SelectVdcControl, each framed. */
static void write_band_at(uint16_t ret, uint8_t band)
{
    jsr_push(ret);
    WriteBandScroll(band);
    jsr_pop();
}

static void select_at(uint16_t ret, uint8_t reg)
{
    jsr_push(ret);
    SelectVdcRegister(reg);
    jsr_pop();
}

static void select_control_at(uint16_t ret)
{
    jsr_push(ret);
    SelectVdcControl();
    jsr_pop();
}

/* The background's enable in the VDC's control, off or on. */
static void background_off_at(uint16_t ret)
{
    uint8_t control;

    select_control_at(ret);
    control = (uint8_t)(RD8(ZP->vdc_cr.lo) & CR_BG_OFF_MASK);
    WR8(VDC->data_lo, control);
}

static void background_on_at(uint16_t ret)
{
    uint8_t control;

    select_control_at(ret);
    control = (uint8_t)(RD8(ZP->vdc_cr.lo) | CR_BG_ON);
    WR8(VDC->data_lo, control);
}

/* The working band takes another band's X scroll, low then high. */
static void work_x_from(uint8_t band)
{
    uint8_t byte = RD8(BANDS->bgx_lo[band]);

    WR8(BANDS->bgx_lo[WORK_BAND], byte);
    byte = RD8(BANDS->bgx_hi[band]);
    WR8(BANDS->bgx_hi[WORK_BAND], byte);
}

static void next_step(void)
{
    uint8_t step = (uint8_t)(rd8(RasterStep) + 1);

    wr8(RasterStep, step);
}

/*
 * The working band's Y is a band's with a byte added to the low: the low is read, the byte read,
 * the sum written, then the high read with the carry added and written. The byte is the original's
 * table by step, or a constant.
 */
static void work_y_from_plus(uint8_t band, uint8_t add)
{
    Sum low = add_bytes(RD8(BANDS->bgy_lo[band]), add, false);
    Sum high;

    WR8(BANDS->bgy_lo[WORK_BAND], low.value);
    high = add_bytes(0, RD8(BANDS->bgy_hi[band]), low.carry);
    WR8(BANDS->bgy_hi[WORK_BAND], high.value);
}

/* The same with the byte read from a table between the low's read and its write. */
static void work_y_from_plus_table(uint8_t band, uint16_t table, uint8_t index)
{
    uint8_t low_byte = RD8(BANDS->bgy_lo[band]);
    Sum low = add_bytes(low_byte, rd8((uint16_t)(table + index)), false);
    Sum high;

    WR8(BANDS->bgy_lo[WORK_BAND], low.value);
    high = add_bytes(0, RD8(BANDS->bgy_hi[band]), low.carry);
    WR8(BANDS->bgy_hi[WORK_BAND], high.value);
}

/* The same with a word of the table: its low byte on the low, its high on the high. */
static void work_y_from_plus_word(uint8_t band, uint16_t table, uint8_t index)
{
    uint8_t low_byte = RD8(BANDS->bgy_lo[band]);
    Sum low = add_bytes(low_byte, rd8((uint16_t)(table + index)), false);
    Sum high;
    uint8_t high_byte;

    WR8(BANDS->bgy_lo[WORK_BAND], low.value);
    high_byte = RD8(BANDS->bgy_hi[band]);
    high = add_bytes(high_byte, rd8((uint16_t)(table + 1 + index)), low.carry);
    WR8(BANDS->bgy_hi[WORK_BAND], high.value);
}

/* The working band's Y low then high, as a stored constant. */
static void work_y_set(uint8_t low, uint8_t high)
{
    WR8(BANDS->bgy_lo[WORK_BAND], low);
    WR8(BANDS->bgy_hi[WORK_BAND], high);
}

/* The compare line as the original's tail writes it: A plus $40 low, the carry into the high. */
static void line_word_of(uint8_t line)
{
    Sum low = add_bytes(0x40, line, false);
    Sum high;

    WR8(VDC->data_lo, low.value);
    high = add_bytes(0, 0, low.carry);
    WR8(VDC->data_hi, high.value);
}

/* The shared tail of the single-byte lines: A plus $40 low, a zero high, and the interrupt on. */
static void line_byte_tail(uint8_t line)
{
    Sum low = add_bytes(line, RCR_BASE, false);

    WR8(VDC->data_lo, low.value);
    st2(0);
    cli();
}

/* The working band's scroll is written, the step counted, and the compare register selected. */
static void write_work_then_select(uint16_t write_ret, uint16_t select_ret)
{
    write_band_at(write_ret, WORK_BAND);
    next_step();
    select_at(select_ret, VDC_REG_RCR);
}

/*
 * The middle steps of the many-band effects: the working band takes a band chosen by a table by
 * step, its Y moved on by a second table, and the next line comes from a third.
 */
static void band_table_step(uint8_t step, uint16_t bands, uint16_t y_adds, uint16_t lines,
                            uint16_t write_ret, uint16_t select_ret)
{
    uint8_t band = rd8((uint16_t)(bands + step));
    uint8_t line;

    work_x_from(band);
    work_y_from_plus_table(band, y_adds, step);
    write_work_then_select(write_ret, select_ret);
    line = rd8((uint16_t)(lines + step));
    line_word_of(line);
    cli();
}

/* The same with the second table in words, for Y moves past one byte. */
static void band_table_step_word(uint8_t step, uint16_t bands, uint16_t y_adds, uint16_t lines,
                                 uint16_t write_ret, uint16_t select_ret)
{
    uint8_t band = rd8((uint16_t)(bands + step));
    uint8_t index = (uint8_t)(step << 1);
    uint8_t line;

    work_x_from(band);
    work_y_from_plus_word(band, y_adds, index);
    write_work_then_select(write_ret, select_ret);
    line = rd8((uint16_t)(lines + step));
    line_word_of(line);
    cli();
}

/* A table by a count of the frame's, over eight, plus the step: where a wave starts. */
static uint8_t wave_index(uint8_t counter, uint8_t shift)
{
    uint8_t byte = (uint8_t)(counter >> shift);
    Sum sum = add_bytes(byte, rd8(RasterStep), false);

    return (uint8_t)((sum.value & 7) << 1);
}

/* The working band's X is band 1's, moved by a word of a wave table. */
static void work_x_band1_plus_wave(uint16_t wave, uint8_t index)
{
    uint8_t low_byte = rd8((uint16_t)(wave + index));
    Sum low = add_bytes(low_byte, RD8(BANDS->bgx_lo[1]), false);
    Sum high;
    uint8_t high_byte;

    WR8(BANDS->bgx_lo[WORK_BAND], low.value);
    high_byte = rd8((uint16_t)(wave + 1 + index));
    high = add_bytes(high_byte, RD8(BANDS->bgx_hi[1]), low.carry);
    WR8(BANDS->bgx_hi[WORK_BAND], high.value);
}

/* Colours $101-$10F: the VCE's address pointed at the sprite palette's second bank. */
static void select_sprite_palette(void)
{
    WR8(VCE->addr_lo, COLOUR_SPRITE_BANK);
    WR8(VCE->addr_hi, COLOUR_SPRITE_BANK);
}

/*
 * @routine 0043:52E RasterDone
 * @run 69:452E
 * @ends 4287
 * WHAT IT DOES: Ends a raster interrupt, with interrupts back on for the next.
 * HOW: CLI, then the frame interrupt's exit.
 * WAITS: no.
 * Entry: from a raster effect. Exit: to that exit.
 */
void abi_RasterDone(void)
{
    cli();
}

/*
 * @routine 0043:529 RasterEndBand1
 * @run 69:4529
 * @ends 4287
 * WHAT IT DOES: A raster effect's last band: band 1's scroll, and no more lines.
 * HOW: WriteBandScroll, then RasterDone. Effect 1 is this alone.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEndBand1(void)
{
    write_band_at(0x452E, 1);
    cli();
}

void abi_RasterEndBand1(void)
{
    RasterEndBand1();
}

/*
 * @routine 0043:532 RasterEndWorkBand
 * @run 69:4532
 * @ends 4287
 * WHAT IT DOES: A raster effect's last band: the working band's scroll, and no more
 * lines.
 * HOW: WriteBandScroll, then RasterDone.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEndWorkBand(void)
{
    write_band_at(0x452E, WORK_BAND);
    cli();
}

void abi_RasterEndWorkBand(void)
{
    RasterEndWorkBand();
}

/* The body RasterSetNextLine and RasterBandThenLine share: the step counted, the compare line from the stack. */
static void line_from_stack(void)
{
    uint8_t line;

    next_step();
    select_at(0x50FC, VDC_REG_RCR);
    line = pull8();
    WR8(VDC->data_lo, line);
    st2(0);
    cli();
}

/*
 * @routine 0045:0ED RasterSetNextLine
 * @run 69:50ED
 * @ends 4287
 * WHAT IT DOES: Moves a raster effect on: the next band's compare line goes in.
 * HOW: Counts the step, selects the compare register, writes the line as the VDC
 * counts it.
 * WAITS: no.
 * Entry: A line. Exit: to the frame interrupt's exit.
 */
void RasterSetNextLine(uint8_t line)
{
    push8(line);
    line_from_stack();
}

void abi_RasterSetNextLine(void)
{
    RasterSetNextLine(cpu.a);
}

/*
 * @routine 0045:0F0 RasterBandThenLine
 * @run 69:50F0
 * @ends 4287
 * WHAT IT DOES: The usual raster step: writes a band's scroll, then the next band's
 * compare line.
 * HOW: WriteBandScroll, then runs into RasterSetNextLine's body.
 * WAITS: no.
 * Entry: X band, A line. Exit: to the frame interrupt's exit.
 */
void RasterBandThenLine(uint8_t band, uint8_t line)
{
    push8(line);
    write_band_at(0x50F4, band);
    line_from_stack();
}

void abi_RasterBandThenLine(void)
{
    RasterBandThenLine(cpu.x, cpu.a);
}

/*
 * @routine 0043:536 RasterEffect2
 * @run 69:4536
 * @ends 4287
 * WHAT IT DOES: Raster effect 2: two bands, split at line $BF.
 * HOW: Band 1, then band 0's X and Y high with Y low $7F.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect2(void)
{
    uint8_t byte;

    if (rd8(RasterStep) == 0) {
        RasterBandThenLine(1, 0xBF);
        return;
    }
    work_x_from(0);
    byte = RD8(BANDS->bgy_hi[0]);
    WR8(BANDS->bgy_hi[WORK_BAND], byte);
    WR8(BANDS->bgy_lo[WORK_BAND], 0x7F);
    RasterEndWorkBand();
}

void abi_RasterEffect2(void)
{
    RasterEffect2();
}

/*
 * @routine 0043:55C RasterEffect3
 * @run 69:455C
 * @ends 4287
 * WHAT IT DOES: Raster effect 3: three bands, the first split moving with half of band
 * 3's Y scroll.
 * HOW: The split's line comes from that scroll; the second band takes band 2 with its
 * Y offset; the last, band 0.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect3(void)
{
    uint8_t step = rd8(RasterStep);
    uint8_t byte;

    if (step == 0) {
        byte = (uint8_t)(RD8(BANDS->bgy_lo[3]) >> 1);
        WR8(BANDS->bgy_lo[WORK_BAND], byte);
        byte = sub_bytes(0xAF, RD8(BANDS->bgy_lo[WORK_BAND]), true).value;
        RasterBandThenLine(1, byte);
    } else if (step == 1) {
        work_x_from(2);
        work_y_from_plus(2, 0x6F);
        byte = sub_bytes(0xFF, RD8(BANDS->bgy_lo[3]), true).value;
        RasterBandThenLine(WORK_BAND, byte);
    } else {
        work_x_from(0);
        WR8(BANDS->bgy_lo[WORK_BAND], 0xBF);
        RasterEndWorkBand();
    }
}

void abi_RasterEffect3(void)
{
    RasterEffect3();
}

/*
 * @routine 0043:5B1 RasterEffect4
 * @run 69:45B1
 * @ends 4287
 * WHAT IT DOES: Raster effect 4: band 0's X with band 1's Y, in one step.
 * HOW: Copies them to the working band.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect4(void)
{
    uint8_t byte;

    work_x_from(0);
    byte = RD8(BANDS->bgy_lo[1]);
    WR8(BANDS->bgy_lo[WORK_BAND], byte);
    byte = RD8(BANDS->bgy_hi[1]);
    WR8(BANDS->bgy_hi[WORK_BAND], byte);
    RasterEndWorkBand();
}

void abi_RasterEffect4(void)
{
    RasterEffect4();
}

/*
 * @routine 0043:5CC RasterEffect5
 * @run 69:45CC
 * @ends 4287
 * WHAT IT DOES: Raster effect 5: four bands, split at lines $5F, $7F and $FF.
 * HOW: Bands 1, 2 and 1 with their Y offsets, then band 0.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect5(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(1, 0x5F);
    } else if (step == 1) {
        work_x_from(2);
        work_y_from_plus(2, 0x1F);
        RasterBandThenLine(WORK_BAND, 0x7F);
    } else if (step == 2) {
        work_x_from(1);
        work_y_from_plus(1, 0x3F);
        RasterBandThenLine(WORK_BAND, 0xFF);
    } else {
        work_x_from(0);
        work_y_from_plus(0, 0xBF);
        RasterEndWorkBand();
    }
}

void abi_RasterEffect5(void)
{
    RasterEffect5();
}

/*
 * @routine 0043:643 RasterEffect6
 * @run 69:4643
 * @ends 4287
 * WHAT IT DOES: Raster effect 6: seven bands from tables.
 * HOW: Band 4, five steps from the tables, then band 1 at Y $DB.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect6(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(4, 0x67);
    } else if (step == 6) {
        work_x_from(1);
        WR8(BANDS->bgy_lo[WORK_BAND], 0xDB);
        RasterEndWorkBand();
    } else {
        band_table_step(step, Effect6Bands, Effect6YAdds, Effect6Lines, 0x468D, 0x4695);
    }
}

void abi_RasterEffect6(void)
{
    RasterEffect6();
}

/*
 * @routine 0043:6B6 RasterEffect7
 * @run 69:46B6
 * @ends 4287
 * WHAT IT DOES: Raster effect 7: seven bands from tables, with word Y offsets.
 * HOW: Band 4, five steps from the tables, then band 1 at Y $11B.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect7(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(4, 0x67);
    } else if (step == 6) {
        work_x_from(1);
        work_y_set(0x1B, 0x01);
        RasterEndWorkBand();
    } else {
        band_table_step_word(step, Effect6Bands, Effect7YAdds, Effect6Lines, 0x4709, 0x4714);
    }
}

void abi_RasterEffect7(void)
{
    RasterEffect7();
}

/*
 * @routine 0043:730 RasterEffect8Step4
 * @run 69:4730
 * @ends 4287
 * WHAT IT DOES: Effect 8's fifth step: turns the background off below the band.
 * HOW: Clears the background bit in the VDC's control, and sets line $E0.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect8Step4(void)
{
    background_off_at(0x4733);
    RasterSetNextLine(0xE0);
}

void abi_RasterEffect8Step4(void)
{
    RasterEffect8Step4();
}

/*
 * @routine 0043:73F RasterEffect8Step0
 * @run 69:473F
 * @ends 4287
 * WHAT IT DOES: Effect 8's first step: band 5 scrolled one line further, for the one
 * band only.
 * HOW: Raises its Y high byte around WriteBandScroll, and sets line $5F.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect8Step0(void)
{
    INC8(BANDS->bgy_hi[5]);
    write_band_at(0x4747, 5);
    DEC8(BANDS->bgy_hi[5]);
    RasterSetNextLine(0x5F);
}

void abi_RasterEffect8Step0(void)
{
    RasterEffect8Step0();
}

/*
 * @routine 0043:74F RasterEffect8Step3
 * @run 69:474F
 * @ends 4287
 * WHAT IT DOES: Effect 8's fourth step: band 0's X at Y $6F, and line $DF.
 * HOW: Builds the working band and writes it.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect8Step3(void)
{
    work_x_from(0);
    work_y_set(0x6F, 0);
    RasterBandThenLine(WORK_BAND, 0xDF);
}

void abi_RasterEffect8Step3(void)
{
    RasterEffect8Step3();
}

/* Colours $101-$10F back from where the step below kept them. */
static void restore_saved_colours(void)
{
    select_sprite_palette();
    for (uint8_t colour = PALETTE_SAVED_COLOURS; colour != 0; colour--) {
        uint8_t byte = rd8((uint16_t)(SavedColoursLo + colour));

        WR8(VCE->data_lo, byte);
        byte = rd8((uint16_t)(SavedColoursHi + colour));
        WR8(VCE->data_hi, byte);
    }
    cli();
}

/*
 * Colours $101-$10F kept in RAM and replaced by one flat colour, sprites to flicker
 * dark, for the rest of the frame's bands; then a compare at line $121 to put them
 * back. The background's enable follows the frame's count, every other frame.
 */
static void flatten_colours(void)
{
    uint8_t which;
    uint8_t control;

    select_sprite_palette();
    for (uint8_t colour = PALETTE_SAVED_COLOURS; colour != 0; colour--) {
        uint8_t byte = RD8(VCE->data_lo);

        wr8((uint16_t)(SavedColoursLo + colour), byte);
        byte = RD8(VCE->data_hi);
        wr8((uint16_t)(SavedColoursHi + colour), byte);
    }
    select_sprite_palette();
    for (uint8_t colour = PALETTE_SAVED_COLOURS; colour != 0; colour--) {
        WR8(VCE->data_lo, COLOUR_FILL);
        WR8(VCE->data_hi, 0);
    }
    select_control_at(0x4817);
    which = (uint8_t)(RD8(ZP->frame_count) & 1);
    control = RD8(ZP->vdc_cr.lo);
    control = (uint8_t)(control & rd8((uint16_t)(FlickerMasks + which)));
    WR8(VDC->data_lo, control);
    next_step();
    select_at(0x482C, VDC_REG_RCR);
    st1(0x21);
    st2(0x01);
    cli();
}

/*
 * @routine 0043:76A RasterEffect8
 * @run 69:476A
 * @ends 4287
 * WHAT IT DOES: Raster effect 8: the most elaborate of the splits. Two bands from
 * tables, a third, the background off, then sprites' colours flattened below a line
 * and restored at the screen's end.
 * HOW: A step per band, as the steps above.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect8(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0)
        RasterEffect8Step0();
    else if (step < 3)
        band_table_step_word(step, Effect8Bands, Effect8YAdds, Effect8Lines, 0x47C5, 0x47D0);
    else if (step == 3)
        RasterEffect8Step3();
    else if (step == 4)
        RasterEffect8Step4();
    else if (step == 5)
        flatten_colours();
    else
        restore_saved_colours();
}

void abi_RasterEffect8(void)
{
    RasterEffect8();
}

/* Effects 9 and 10: two bands of a three-step split, the last at a fixed Y. */
static void three_step_split(uint8_t line0, uint8_t y1, uint8_t line1, uint8_t y2)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(0, line0);
    } else if (step == 1) {
        work_x_from(1);
        work_y_set(y1, 0x01);
        RasterBandThenLine(WORK_BAND, line1);
    } else {
        work_x_from(0);
        work_y_set(y2, 0);
        RasterEndWorkBand();
    }
}

/*
 * @routine 0044:03D RasterEffect9
 * @run 69:483D
 * @ends 4287
 * WHAT IT DOES: Raster effect 9: three bands, split at lines $7F and $BF.
 * HOW: Band 0, band 1 at Y $13F, band 0 at Y $7F.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect9(void)
{
    three_step_split(0x7F, 0x3F, 0xBF, 0x7F);
}

void abi_RasterEffect9(void)
{
    RasterEffect9();
}

/*
 * @routine 0044:07F RasterEffect10
 * @run 69:487F
 * @ends 4287
 * WHAT IT DOES: Raster effect 10: three bands, split at lines $9F and $DF.
 * HOW: Band 0, band 1 at Y $15F, band 0 at Y $9F.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect10(void)
{
    three_step_split(0x9F, 0x5F, 0xDF, 0x9F);
}

void abi_RasterEffect10(void)
{
    RasterEffect10();
}

/*
 * @routine 0044:0C1 RasterEffect11
 * @run 69:48C1
 * @ends 4287
 * WHAT IT DOES: Raster effect 11: the background off below the line.
 * HOW: Clears its bit in the VDC's control.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect11(void)
{
    background_off_at(0x48C4);
    cli();
}

void abi_RasterEffect11(void)
{
    RasterEffect11();
}

/*
 * @routine 0044:0CE RasterEffect12
 * @run 69:48CE
 * @ends 4287
 * WHAT IT DOES: Raster effect 12: five bands from tables.
 * HOW: Band 2, three steps from the tables, then band 5 at Y $B8.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect12(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(2, 0x7F);
    } else if (step == 4) {
        work_x_from(5);
        WR8(BANDS->bgy_lo[WORK_BAND], 0xB8);
        RasterEndWorkBand();
    } else {
        band_table_step(step, Effect12Bands, Effect12YAdds, Effect12Lines, 0x4918, 0x4920);
    }
}

void abi_RasterEffect12(void)
{
    RasterEffect12();
}

/*
 * @routine 0044:139 RasterEffect13
 * @run 69:4939
 * @ends 4287
 * WHAT IT DOES: Raster effect 13: six bands from tables, the first scrolled one line
 * further for the one band, the last with the background off at the game's say.
 * HOW: Raises band 4's Y high around its write, then the tables, then band 0.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect13(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        INC8(BANDS->bgy_hi[4]);
        write_band_at(0x4972, 4);
        DEC8(BANDS->bgy_hi[4]);
        RasterSetNextLine(0x5F);
    } else if (step == 5) {
        work_x_from(0);
        WR8(BANDS->bgy_lo[WORK_BAND], 0xCF);
        write_band_at(0x4958, WORK_BAND);
        if (RD8(SCROLL->x_lo[7]) != 0)
            background_off_at(0x4960);
        cli();
    } else {
        band_table_step_word(step, Effect13Bands, Effect13YAdds, Effect13Lines, 0x49A4, 0x49AF);
    }
}

void abi_RasterEffect13(void)
{
    RasterEffect13();
}

/*
 * @routine 0044:1D1 RasterEffect14
 * @run 69:49D1
 * @ends 4287
 * WHAT IT DOES: Raster effect 14: bands with the sprite colours flattened and restored
 * over the last two.
 * HOW: Band 4, band 0, then the colours' two steps.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect14(void)
{
    uint8_t step = rd8(RasterStep);
    uint8_t byte;

    if (step == 0) {
        RasterBandThenLine(4, 0x6F);
    } else if (step == 2) {
        flatten_colours();
    } else if (step == 3) {
        restore_saved_colours();
    } else {
        work_x_from(0);
        byte = RD8(BANDS->bgy_hi[0]);
        WR8(BANDS->bgy_hi[WORK_BAND], byte);
        WR8(BANDS->bgy_lo[WORK_BAND], 0x2F);
        write_work_then_select(0x49FA, 0x4A02);
        st1(0x11);
        st2(0x01);
        cli();
    }
}

void abi_RasterEffect14(void)
{
    RasterEffect14();
}

/*
 * @routine 0044:216 RasterEffect15
 * @run 69:4A16
 * @ends 4287
 * WHAT IT DOES: Raster effect 15: a water wave: the band each line pair shifted
 * sideways by a table that moves with the frame's count, for 20 steps.
 * HOW: Band 2, then each step's X from band 1 plus the wave, Y moving on two lines
 * with the step; then band 0.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect15(void)
{
    uint8_t step = rd8(RasterStep);
    uint8_t byte;

    if (step == 0) {
        byte = sub_bytes(0xCF, rd8(RasterWaterLine), true).value;
        RasterBandThenLine(2, byte);
        return;
    }
    if (step >= 0x15) {
        work_x_from(0);
        WR8(BANDS->bgy_hi[WORK_BAND], 0x01);
        WR8(BANDS->bgy_lo[WORK_BAND], 0xB7);
        RasterEndWorkBand();
        return;
    }
    {
        /* The step here is 1 to $14: two lines a step down from band 1, a byte that wraps. */
        uint8_t doubled = (uint8_t)((uint8_t)(rd8(RasterStep) - 1) << 1);
        Sum first = add_bytes(doubled, RD8(BANDS->bgy_lo[1]), false);
        Sum low = add_bytes(first.value, 0x8F, false);
        Sum high;
        uint8_t wave;

        WR8(BANDS->bgy_lo[WORK_BAND], low.value);
        high = add_bytes(0, RD8(BANDS->bgy_hi[1]), low.carry);
        WR8(BANDS->bgy_hi[WORK_BAND], high.value);
        wave = wave_index(RD8(ZP->frame_tick), 3);
        work_x_band1_plus_wave(WaterWave, wave);
        write_band_at(0x4A7D, WORK_BAND);
        select_at(0x4A82, VDC_REG_RCR);
        byte = (uint8_t)(rd8(RasterStep) << 1);
        WR8(BANDS->bgy_lo[WORK_BAND], byte);
        {
            Sum line = sub_bytes(0xCF, rd8(RasterWaterLine), true);
            Sum sum = add_bytes(line.value, RD8(BANDS->bgy_lo[WORK_BAND]), false);
            Sum top;

            WR8(VDC->data_lo, sum.value);
            top = add_bytes(0, 0, sum.carry);
            WR8(VDC->data_hi, top.value);
        }
        next_step();
        cli();
    }
}

void abi_RasterEffect15(void)
{
    RasterEffect15();
}

/*
 * @routine 0044:2B1 RasterEffect16
 * @run 69:4AB1
 * @ends 4287
 * WHAT IT DOES: Raster effect 16: six bands from tables.
 * HOW: Band 5, four steps from the tables, then band 1 at Y $BF.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect16(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(5, 0x57);
    } else if (step == 5) {
        work_x_from(1);
        WR8(BANDS->bgy_lo[WORK_BAND], 0xBF);
        RasterEndWorkBand();
    } else {
        band_table_step(step, Effect16Bands, Effect16YAdds, Effect16Lines, 0x4AFB, 0x4B03);
    }
}

void abi_RasterEffect16(void)
{
    RasterEffect16();
}

/*
 * @routine 0044:321 RasterEffect17
 * @run 69:4B21
 * @ends 4287
 * WHAT IT DOES: Raster effect 17: six bands from tables.
 * HOW: Band 4, four steps from the tables, then band 2 at Y $D7.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect17(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(4, 0x67);
    } else if (step == 5) {
        work_x_from(2);
        WR8(BANDS->bgy_lo[WORK_BAND], 0xD7);
        RasterEndWorkBand();
    } else {
        band_table_step(step, Effect17Bands, Effect17YAdds, Effect17Lines, 0x4B6B, 0x4B73);
    }
}

void abi_RasterEffect17(void)
{
    RasterEffect17();
}

/* Band 0's X low with X high 1, at Y $168: the fixed band of effect 18. */
static void effect18_work_band(void)
{
    uint8_t byte = RD8(BANDS->bgx_lo[0]);

    WR8(BANDS->bgx_lo[WORK_BAND], byte);
    WR8(BANDS->bgx_hi[WORK_BAND], 0x01);
    WR8(BANDS->bgy_lo[WORK_BAND], 0x68);
    WR8(BANDS->bgy_hi[WORK_BAND], 0x01);
}

/*
 * @routine 0044:38F RasterEffect18
 * @run 69:4B8F
 * @ends 4287
 * WHAT IT DOES: Raster effect 18: a band whose height the game sets, then the rest of
 * the screen following it.
 * HOW: The fixed band to the line in band 3's Y, then band 1 built from band 0 moved
 * down by that height, then the fixed band again.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect18(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        effect18_work_band();
        write_work_then_select(0x4BC9, 0x4BD1);
        line_byte_tail(RD8(BANDS->bgy_lo[2]));
    } else if (step == 1) {
        uint8_t byte = RD8(BANDS->bgx_lo[0]);
        Sum low;
        Sum high;

        WR8(BANDS->bgx_lo[1], byte);
        byte = RD8(BANDS->bgx_hi[0]);
        WR8(BANDS->bgx_hi[1], byte);
        byte = RD8(BANDS->bgy_lo[0]);
        low = add_bytes(byte, RD8(BANDS->bgy_lo[2]), false);
        WR8(BANDS->bgy_lo[1], low.value);
        high = add_bytes(0, 0, low.carry);
        WR8(BANDS->bgy_hi[1], high.value);
        write_band_at(0x4BF8, 1);
        next_step();
        select_at(0x4C00, VDC_REG_RCR);
        {
            Sum line = sub_bytes(0xE0, RD8(BANDS->bgy_lo[2]), true);

            line_word_of(line.value);
        }
        cli();
    } else {
        effect18_work_band();
        RasterEndWorkBand();
    }
}

void abi_RasterEffect18(void)
{
    RasterEffect18();
}

/*
 * @routine 0044:415 RasterEffect19
 * @run 69:4C15
 * @ends 4287
 * WHAT IT DOES: Raster effect 19: a stretch of screen with the background off, its
 * edges set by two bytes the game moves.
 * HOW: Band 0 background off to the first; band 1 background on to the second; band 2,
 * background off at the game's say.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect19(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        write_band_at(0x4C38, 0);
        background_off_at(0x4C3B);
        next_step();
        select_at(0x4C4A, VDC_REG_RCR);
        line_byte_tail(RD8(BANDS->bgy_lo[5]));
    } else if (step == 1) {
        write_band_at(0x4C55, 1);
        background_on_at(0x4C58);
        next_step();
        select_at(0x4C67, VDC_REG_RCR);
        line_word_of(RD8(BANDS->bgy_lo[6]));
        cli();
    } else {
        if (RD8(BANDS->bgy_hi[6]) == 0)
            background_off_at(0x4C25);
        write_band_at(0x4C31, 2);
        cli();
    }
}

void abi_RasterEffect19(void)
{
    RasterEffect19();
}

/*
 * @routine 0044:479 RasterEffect20
 * @run 69:4C79
 * @ends 4287
 * WHAT IT DOES: Raster effect 20: as effect 19 with the edges allowed to meet: steps
 * skip when a moving edge passes a fixed line at $60, $77, $A7 or $BF.
 * HOW: Band 3, band 0 and band 6 up to those lines, band 1 background off, band 4
 * background on.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect20(void)
{
    uint8_t step = rd8(RasterStep);
    uint8_t line;

    if (step == 0) {
        write_band_at(0x4C9E, 3);
        next_step();
        select_at(0x4CA6, VDC_REG_RCR);
        line = RD8(BANDS->bgy_lo[5]);
        if (line >= 0x60)
            line = 0x5F;
        else
            next_step();
        line_byte_tail(line);
    } else if (step == 1) {
        write_band_at(0x4CBB, 0);
        line = RD8(BANDS->bgy_lo[5]);
        if (line >= 0x77) {
            wr8(RasterStep, 4);
            select_at(0x4CDA, VDC_REG_RCR);
            line_byte_tail(0xA7);
        } else {
            next_step();
            select_at(0x4CCA, VDC_REG_RCR);
            line_byte_tail(RD8(BANDS->bgy_lo[5]));
        }
    } else if (step == 2) {
        write_band_at(0x4D07, 1);
        background_off_at(0x4D0A);
        next_step();
        select_at(0x4D19, VDC_REG_RCR);
        line = RD8(BANDS->bgy_lo[6]);
        if (line == 0xBF) {
            cli();
        } else {
            if (line >= 0xA7)
                next_step();
            line_byte_tail(line);
        }
    } else if (step == 3) {
        write_band_at(0x4CE4, 6);
        background_on_at(0x4CE7);
        next_step();
        select_at(0x4CF6, VDC_REG_RCR);
        line = RD8(BANDS->bgy_lo[6]);
        line_byte_tail(line >= 0xA8 ? line : 0xA7);
    } else {
        background_on_at(0x4C8A);
        write_band_at(0x4C96, 4);
        cli();
    }
}

void abi_RasterEffect20(void)
{
    RasterEffect20();
}

/*
 * @routine 0044:532 RasterEffect21
 * @run 69:4D32
 * @ends 4287
 * WHAT IT DOES: Raster effect 21: five bands split at $7F, $87 and $FF, then a late
 * compare, the background off in the last at a stage value.
 * HOW: Bands 2, 1, 3, then band 0 twice.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect21(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(2, 0x7F);
    } else if (step == 1) {
        RasterBandThenLine(1, 0x87);
    } else if (step == 2) {
        RasterBandThenLine(3, 0xFF);
    } else if (step == 3) {
        work_x_from(0);
        work_y_set(0xBF, 0);
        write_work_then_select(0x4D95, 0x4D9D);
        st1(0x07);
        st2(0x01);
        cli();
    } else {
        if (RD8(ZP->raster_stage) == 3)
            background_off_at(0x4D49);
        work_x_from(0);
        work_y_set(0xC7, 0);
        RasterEndWorkBand();
    }
}

void abi_RasterEffect21(void)
{
    RasterEffect21();
}

/*
 * @routine 0044:5A4 RasterEffect22
 * @run 69:4DA4
 * @ends 4287
 * WHAT IT DOES: Raster effect 22: two bands: band 2 at Y $110 to line $67, band 0 after.
 * HOW: Builds the working band each step.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect22(void)
{
    if (rd8(RasterStep) == 0) {
        work_y_set(0x10, 0x01);
        work_x_from(2);
        RasterBandThenLine(WORK_BAND, 0x67);
    } else {
        work_x_from(0);
        WR8(BANDS->bgy_lo[WORK_BAND], add_bytes(0x27, RD8(BANDS->bgy_lo[0]), false).value);
        WR8(BANDS->bgy_hi[WORK_BAND], 0);
        RasterEndWorkBand();
    }
}

void abi_RasterEffect22(void)
{
    RasterEffect22();
}

/* Effects 23 and 29: flatten the colours and restore them, after a first line. */
static void flatten_after_line(uint8_t first_line, uint16_t select_ret)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        next_step();
        select_at(select_ret, VDC_REG_RCR);
        st1(first_line);
        st2(0x01);
        cli();
    } else if (step == 1) {
        flatten_colours();
    } else {
        restore_saved_colours();
    }
}

/*
 * @routine 0044:5E1 RasterEffect23
 * @run 69:4DE1
 * @ends 4287
 * WHAT IT DOES: Raster effect 23: the sprite colours flattened from line $141's compare
 * and restored after.
 * HOW: A first compare, then the colours' two steps.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect23(void)
{
    flatten_after_line(0x01, 0x4DF7);
}

void abi_RasterEffect23(void)
{
    RasterEffect23();
}

/*
 * @routine 0044:5FE RasterEffect25
 * @run 69:4DFE
 * @ends 4287
 * WHAT IT DOES: Raster effect 25: the sprite colours flattened at once and restored
 * after.
 * HOW: The colours' two steps.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect25(void)
{
    if (rd8(RasterStep) == 0)
        flatten_colours();
    else
        restore_saved_colours();
}

void abi_RasterEffect25(void)
{
    RasterEffect25();
}

/*
 * @routine 0044:609 RasterEffect29
 * @run 69:4E09
 * @ends 4287
 * WHAT IT DOES: Raster effect 29: the sprite colours flattened from a later compare than
 * effect 23's, and restored after.
 * HOW: A first compare, then the colours' two steps.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect29(void)
{
    flatten_after_line(0x18, 0x4E1F);
}

void abi_RasterEffect29(void)
{
    RasterEffect29();
}

/*
 * @routine 0044:626 RasterEffect24
 * @run 69:4E26
 * @ends 4287
 * WHAT IT DOES: Raster effect 24: water under a line with a wave that moves on its own
 * counter, for 16 steps; each step the background moves further down, two or three
 * lines by a slower wave.
 * HOW: Band 0, then each step's X from band 1 plus the wave. The next compare line is
 * kept in band 2's X bytes, itself moved on each step.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect24(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        WR8(BANDS->bgx_lo[2], 0xFF);
        WR8(BANDS->bgx_hi[2], 0);
        RasterBandThenLine(0, 0xFF);
        return;
    }
    if (step >= 0x11) {
        RasterEndBand1();
        return;
    }
    {
        /* The step here is 1 to $10, so doubling it one less carries nothing. */
        uint8_t doubled = (uint8_t)((uint8_t)(rd8(RasterStep) - 1) << 1);
        uint8_t wave;
        uint8_t beat;
        uint8_t counter;
        Sum low;
        Sum high;

        WR8(BANDS->bgy_lo[WORK_BAND], add_bytes(doubled, RD8(BANDS->bgy_lo[1]), false).value);
        WR8(BANDS->bgy_hi[WORK_BAND], 0);
        counter = RD8(ZP->wave_counter);
        wave = wave_index(counter, 3);
        work_x_band1_plus_wave(DeepWave, wave);
        write_band_at(0x4E65, WORK_BAND);
        select_at(0x4E6A, VDC_REG_RCR);
        counter = RD8(ZP->wave_counter);
        beat = (uint8_t)((uint8_t)(add_bytes((uint8_t)(counter >> 3), rd8(RasterStep), false).value >> 3) & 1);
        low = add_bytes(add_bytes(beat, 2, false).value, RD8(BANDS->bgx_lo[2]), false);
        WR8(BANDS->bgx_lo[2], low.value);
        WR8(VDC->data_lo, low.value);
        high = add_bytes(0, RD8(BANDS->bgx_hi[2]), low.carry);
        WR8(BANDS->bgx_hi[2], high.value);
        WR8(VDC->data_hi, high.value);
        next_step();
        cli();
    }
}

void abi_RasterEffect24(void)
{
    RasterEffect24();
}

/*
 * Effects 26 and 27: a table of bands in RAM, each marked: bit 7 clear stands still, set
 * follows the deep wave. Each step is a line or eight apart.
 */
static void marked_band_step(uint8_t limit, uint8_t shift, uint16_t write_ret, uint16_t select_ret)
{
    uint8_t step = rd8(RasterStep);
    uint8_t mark;
    uint8_t byte;

    if (step == 0) {
        RasterBandThenLine(0, 0x41);
        return;
    }
    if (step >= limit) {
        RasterEndBand1();
        return;
    }
    step = rd8(RasterStep);
    byte = (uint8_t)(((uint8_t)(step - 1) << shift) + 1);
    WR8(BANDS->bgy_lo[WORK_BAND], byte);
    mark = rd8((uint16_t)(RasterMarks + step));
    if (!(mark & 0x80)) {
        WR8(BANDS->bgx_lo[WORK_BAND], 0);
        WR8(BANDS->bgy_hi[WORK_BAND], 0);
        WR8(BANDS->bgx_hi[WORK_BAND], 0x01);
    } else {
        uint8_t index;
        uint8_t counter;

        WR8(BANDS->bgy_hi[WORK_BAND], 0x01);
        counter = RD8(ZP->wave_counter);
        index = (uint8_t)((add_bytes((uint8_t)(counter >> 2), rd8(RasterStep), false).value & 7) << 1);
        byte = rd8((uint16_t)(DeepWave + index));
        WR8(BANDS->bgx_lo[WORK_BAND], byte);
        byte = rd8((uint16_t)(DeepWave + 1 + index));
        WR8(BANDS->bgx_hi[WORK_BAND], byte);
    }
    mark = (uint8_t)(rd8((uint16_t)(RasterMarks + step)) & 0x7F);
    WR8(BANDS->bgx_lo[WORK_BAND], add_bytes(mark, RD8(BANDS->bgx_lo[WORK_BAND]), false).value);
    write_band_at(write_ret, WORK_BAND);
    select_at(select_ret, VDC_REG_RCR);
    {
        Sum low = add_bytes((uint8_t)(rd8(RasterStep) << shift), 0x41, false);
        Sum high;

        WR8(VDC->data_lo, low.value);
        high = add_bytes(0, 0, low.carry);
        WR8(VDC->data_hi, high.value);
    }
    next_step();
    cli();
}

/*
 * @routine 0044:6B0 RasterEffect26
 * @run 69:4EB0
 * @ends 4287
 * WHAT IT DOES: Raster effect 26: a marked band every two lines, for $70 steps.
 * HOW: See marked_band_step.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect26(void)
{
    marked_band_step(0x71, 1, 0x4F06, 0x4F0B);
}

void abi_RasterEffect26(void)
{
    RasterEffect26();
}

/*
 * @routine 0044:726 RasterEffect27
 * @run 69:4F26
 * @ends 4287
 * WHAT IT DOES: Raster effect 27: a marked band every eight lines, for $1C steps.
 * HOW: See marked_band_step.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect27(void)
{
    marked_band_step(0x1D, 3, 0x4F7E, 0x4F83);
}

void abi_RasterEffect27(void)
{
    RasterEffect27();
}

/*
 * @routine 0044:7A0 RasterEffect28
 * @run 69:4FA0
 * @ends 4287
 * WHAT IT DOES: Raster effect 28: a band every two lines from line $A7, each shaking a
 * pixel or two sideways, now and then, at random.
 * HOW: Band 3, then steps with band 3's X; when the frame's count and step land on
 * zero, the top of the random byte is added. Then band 0.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect28(void)
{
    uint8_t step = rd8(RasterStep);
    uint8_t byte;

    if (step == 0) {
        RasterBandThenLine(3, 0xA7);
        return;
    }
    if (step >= 0x0C) {
        work_x_from(0);
        work_y_set(0x7F, 0);
        RasterEndWorkBand();
        return;
    }
    byte = add_bytes((uint8_t)(rd8(RasterStep) << 1), 0x67, false).value;
    WR8(BANDS->bgy_lo[WORK_BAND], byte);
    if (rd8(RasterStep) == 1) {
        DEC8(BANDS->bgy_lo[WORK_BAND]);
        DEC8(BANDS->bgy_lo[WORK_BAND]);
    }
    WR8(BANDS->bgy_hi[WORK_BAND], 0);
    byte = RD8(BANDS->bgx_lo[3]);
    WR8(BANDS->bgx_lo[WORK_BAND], byte);
    byte = RD8(BANDS->bgx_hi[3]);
    WR8(BANDS->bgx_hi[WORK_BAND], byte);
    byte = (uint8_t)(RD8(ZP->frame_tick) >> 3);
    byte = (uint8_t)(add_bytes(byte, rd8(RasterStep), false).value >> 1);
    if ((byte & 0x0F) == 0) {
        uint8_t random = (uint8_t)(RD8(ZP->random3) & 1);
        Sum low = add_bytes(add_bytes(random, 1, false).value, RD8(BANDS->bgx_lo[WORK_BAND]), false);
        Sum high;

        WR8(BANDS->bgx_lo[WORK_BAND], low.value);
        high = add_bytes(0, RD8(BANDS->bgx_hi[WORK_BAND]), low.carry);
        WR8(BANDS->bgx_hi[WORK_BAND], high.value);
    }
    write_band_at(0x5013, WORK_BAND);
    select_at(0x5018, VDC_REG_RCR);
    {
        Sum low = add_bytes((uint8_t)(rd8(RasterStep) << 1), 0xA9, false);
        Sum high;

        WR8(VDC->data_lo, low.value);
        high = add_bytes(0, 0, low.carry);
        WR8(VDC->data_hi, high.value);
    }
    next_step();
    cli();
}

void abi_RasterEffect28(void)
{
    RasterEffect28();
}

/*
 * @routine 0045:02D RasterEffect30
 * @run 69:502D
 * @ends 4287
 * WHAT IT DOES: Raster effect 30: five bands from tables.
 * HOW: Band 1, three steps from the tables, then band 0 at Y $AF.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect30(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(1, 0x8F);
    } else if (step == 4) {
        work_x_from(0);
        work_y_set(0xAF, 0);
        RasterEndWorkBand();
    } else {
        band_table_step(step, Effect30Bands, Effect30YAdds, Effect30Lines, 0x507A, 0x5082);
    }
}

void abi_RasterEffect30(void)
{
    RasterEffect30();
}

/*
 * @routine 0045:09B RasterEffect31
 * @run 69:509B
 * @ends 4287
 * WHAT IT DOES: Raster effect 31: the background off from one line and back on at
 * another, with band 1's Y set for the second.
 * HOW: Background off and line $57; background on and line $E7; band 0.
 * WAITS: no.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect31(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        background_off_at(0x50BE);
        next_step();
        select_at(0x50CD, VDC_REG_RCR);
        WR8(VDC->data_lo, 0x57);
        st2(0);
        cli();
    } else if (step == 2) {
        work_x_from(0);
        work_y_set(0xA7, 0);
        RasterEndWorkBand();
    } else {
        background_on_at(0x50DA);
        WR8(BANDS->bgy_lo[1], 0x17);
        RasterBandThenLine(1, 0xE7);
    }
}

void abi_RasterEffect31(void)
{
    RasterEffect31();
}

/*
 * @routine 0043:4E1 RasterInterrupt
 * @run 69:44E1
 * @ends 4287
 * WHAT IT DOES: A raster interrupt: runs this frame's effect.
 * HOW: Through the table by effect number. Effect 0 only ends the interrupt.
 * WAITS: no.
 * Entry: from FrameInterrupt, the effect below 32, which is all the table holds. Exit:
 * to its exit.
 */
void RasterInterrupt(void)
{
    static void (*const effects[])(void) = {
        abi_RasterDone, RasterEndBand1, abi_RasterEffect2, abi_RasterEffect3,
        abi_RasterEffect4, abi_RasterEffect5, abi_RasterEffect6, abi_RasterEffect7,
        abi_RasterEffect8, abi_RasterEffect9, abi_RasterEffect10, abi_RasterEffect11,
        abi_RasterEffect12, abi_RasterEffect13, abi_RasterEffect14, abi_RasterEffect15,
        abi_RasterEffect16, abi_RasterEffect17, abi_RasterEffect18, abi_RasterEffect19,
        abi_RasterEffect20, abi_RasterEffect21, abi_RasterEffect22, abi_RasterEffect23,
        abi_RasterEffect24, abi_RasterEffect25, abi_RasterEffect26, abi_RasterEffect27,
        abi_RasterEffect28, abi_RasterEffect29, abi_RasterEffect30, abi_RasterEffect31
    };
    uint8_t effect = rd8(RasterEffectNow);
    uint8_t index = (uint8_t)(effect << 1);

    (void)rd8((uint16_t)(RasterEffects + index));
    (void)rd8((uint16_t)(RasterEffects + 1 + index));
    effects[effect]();
}

void abi_RasterInterrupt(void)
{
    RasterInterrupt();
}

/* The band tables' columns copied at the frame's start, from the entry's row down. */
static const struct { uint16_t from, to; } band_copies[] = {
    { ScrollXLo + 7, ScrollBgxLo + 7 }, { ScrollXLo + 6, ScrollBgxLo + 6 },
    { ScrollXHi + 6, ScrollBgxHi + 6 }, { ScrollYLo + 6, ScrollBgyLo + 6 },
    { ScrollYHi + 6, ScrollBgyHi + 6 }, { ScrollXLo + 5, ScrollBgxLo + 5 },
    { ScrollXHi + 5, ScrollBgxHi + 5 }, { ScrollYLo + 5, ScrollBgyLo + 5 },
    { ScrollYHi + 5, ScrollBgyHi + 5 }, { ScrollXLo + 4, ScrollBgxLo + 4 },
    { ScrollXHi + 4, ScrollBgxHi + 4 }, { ScrollYLo + 4, ScrollBgyLo + 4 },
    { ScrollYHi + 4, ScrollBgyHi + 4 }, { ScrollXLo + 3, ScrollBgxLo + 3 },
    { ScrollXHi + 3, ScrollBgxHi + 3 }, { ScrollYLo + 3, ScrollBgyLo + 3 },
    { ScrollYHi + 3, ScrollBgyHi + 3 }, { RasterWaterWanted, RasterWaterLine },
    { ScrollXLo + 2, ScrollBgxLo + 2 }, { ScrollXHi + 2, ScrollBgxHi + 2 },
    { ScrollYLo + 2, ScrollBgyLo + 2 }, { ScrollYHi + 2, ScrollBgyHi + 2 },
    { ScrollXLo + 1, ScrollBgxLo + 1 }, { ScrollXHi + 1, ScrollBgxHi + 1 },
    { ScrollYLo + 1, ScrollBgyLo + 1 }, { ScrollYHi + 1, ScrollBgyHi + 1 }
};
enum { BAND_COPIES = sizeof band_copies / sizeof band_copies[0] };

/* Returns whether anything was copied, and the last byte if so. */
static bool copy_band_columns(unsigned first, uint8_t *last)
{
    for (unsigned i = first; i < BAND_COPIES; i++) {
        uint8_t byte = rd8(band_copies[i].from);

        wr8(band_copies[i].to, byte);
        *last = byte;
    }
    return first < BAND_COPIES;
}

static void abi_copy_from(unsigned first)
{
    uint8_t last;

    if (copy_band_columns(first, &last)) {
        cpu.a = last;
        abi_nz(last);
    }
}

/*
 * @routine 0043:444 CopyBands7Down
 * @run 69:4444
 * WHAT IT DOES: Copies the game's scroll to the frame's bands, for an effect using
 * bands up to 7.
 * HOW: Byte by byte, down to band 1; the later entries start lower.
 * WAITS: no.
 * Entry: nothing. Exit: A the last byte, N and Z from it. X and Y kept.
 */
void abi_CopyBands7Down(void)
{
    abi_copy_from(0);
}

/*
 * @routine 0043:44A CopyBands6Down
 * @run 69:444A
 * WHAT IT DOES: Copies the game's scroll to the frame's bands, up to band 6.
 * HOW: As CopyBands7Down.
 * WAITS: no.
 * Entry and exit: as CopyBands7Down.
 */
void abi_CopyBands6Down(void)
{
    abi_copy_from(1);
}

/*
 * @routine 0043:47A CopyBands4Down
 * @run 69:447A
 * WHAT IT DOES: Copies the game's scroll to the frame's bands, up to band 4.
 * HOW: As CopyBands7Down.
 * WAITS: no.
 * Entry and exit: as CopyBands7Down.
 */
void abi_CopyBands4Down(void)
{
    abi_copy_from(9);
}

/*
 * @routine 0043:492 CopyBands3Down
 * @run 69:4492
 * WHAT IT DOES: Copies the game's scroll to the frame's bands, up to band 3.
 * HOW: As CopyBands7Down.
 * WAITS: no.
 * Entry and exit: as CopyBands7Down.
 */
void abi_CopyBands3Down(void)
{
    abi_copy_from(13);
}

/*
 * @routine 0043:49E CopyBandsYFrom3
 * @run 69:449E
 * WHAT IT DOES: Copies the game's scroll to the frame's bands: band 3's Y only, then
 * from band 2 down.
 * HOW: As CopyBands7Down.
 * WAITS: no.
 * Entry and exit: as CopyBands7Down.
 */
void abi_CopyBandsYFrom3(void)
{
    abi_copy_from(15);
}

/*
 * @routine 0043:4AA CopyBandsWaterDown
 * @run 69:44AA
 * WHAT IT DOES: Copies the game's scroll to the frame's bands from band 2 down, with the
 * water line first.
 * HOW: As CopyBands7Down.
 * WAITS: no.
 * Entry and exit: as CopyBands7Down.
 */
void abi_CopyBandsWaterDown(void)
{
    abi_copy_from(17);
}

/*
 * @routine 0043:4C8 CopyBand1
 * @run 69:44C8
 * WHAT IT DOES: Copies the game's scroll to the frame's band 1 alone.
 * HOW: As CopyBands7Down.
 * WAITS: no.
 * Entry and exit: as CopyBands7Down.
 */
void abi_CopyBand1(void)
{
    abi_copy_from(22);
}

/*
 * @routine 0043:4E0 CopyNoBands
 * @run 69:44E0
 * WHAT IT DOES: The effects with no band of their own copy nothing.
 * HOW: Returns.
 * WAITS: no.
 * Entry: nothing. Exit: everything kept.
 */
void abi_CopyNoBands(void)
{
}

/*
 * @routine 0043:3FC RasterStart
 * @run 69:43FC
 * WHAT IT DOES: At the frame's start, copies the bands this frame's effect uses.
 * HOW: By effect number through a table of the copy's entries.
 * WAITS: no.
 * Entry: nothing. Exit: as the copy; A the effect doubled and N, Z and C from that
 * if the effect copies nothing. X the effect doubled. Y kept.
 */
bool RasterStart(uint8_t *effect_out, uint8_t *last)
{
    /* Row of band_copies each effect starts from, BAND_COPIES for none. */
    static const uint8_t first_row[] = {
        BAND_COPIES, 22, 22, 15, 22, 13, 1, 1, 1, 22, 22, BAND_COPIES, 1, 0, 9, 17, 1, 1, 17, 1, 1, 9, 17, BAND_COPIES, 17, BAND_COPIES, BAND_COPIES, BAND_COPIES, 13, BAND_COPIES, 9, 17
    };
    uint8_t effect = rd8(RasterEffectNow);
    uint8_t index = (uint8_t)(effect << 1);

    (void)rd8((uint16_t)(RasterStartEntries + index));
    (void)rd8((uint16_t)(RasterStartEntries + 1 + index));
    *effect_out = effect;
    return copy_band_columns(first_row[effect], last);
}

void abi_RasterStart(void)
{
    uint8_t effect;
    uint8_t last;
    uint8_t index;

    bool copied = RasterStart(&effect, &last);

    index = (uint8_t)(effect << 1);
    cpu.x = index;
    abi_flag(P_C, (effect & 0x80) != 0);
    cpu.a = copied ? last : index;
    abi_nz(cpu.a);
}
