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
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

enum {
    RCR_BASE = 0x40, VDC_REG_RCR = 0x06, WORK_BAND = 8, CR_BG_OFF_MASK = 0xBF, CR_BG_ON = 0x40,
    PALETTE_SAVED_COLOURS = 0x0F, COLOUR_FILL = 0x07, COLOUR_SPRITE_BANK = 0x01
};

/* The working band takes another band's X scroll. */
static void work_x_from(uint8_t band)
{
    uint8_t byte = rd8((uint16_t)(ScrollBgxLo + band));

    wr8(ScrollBgxLo + WORK_BAND, byte);
    byte = rd8((uint16_t)(ScrollBgxHi + band));
    wr8(ScrollBgxHi + WORK_BAND, byte);
}

static void work_y_lo(uint8_t value)
{
    wr8(ScrollBgyLo + WORK_BAND, value);
}

static void work_y_hi(uint8_t value)
{
    wr8(ScrollBgyHi + WORK_BAND, value);
}

/* Y low is a field plus a constant, Y high another field plus the carry. */
static void work_y_sum(uint16_t low_field, uint8_t add, uint16_t high_field)
{
    uint8_t byte = rd8(low_field);
    Alu r = adc8(byte, add, false);

    work_y_lo(r.a);
    byte = rd8(high_field);
    r = adc8(0, byte, r.c);
    work_y_hi(r.a);
}

static void next_step(void)
{
    uint8_t step = (uint8_t)(rd8(RasterStep) + 1);

    wr8(RasterStep, step);
}

/* The compare line, as the VDC counts: $40 is the first visible. */
static void write_line_word(uint8_t line)
{
    Alu r = adc8(RCR_BASE, line, false);

    wr8(VDC_DATA_LO, r.a);
    r = adc8(0, 0, r.c);
    wr8(VDC_DATA_HI, r.a);
}

/* A compare line in screen lines, the high byte zero, written past the MPRs. */
static void write_line_byte(uint8_t line)
{
    wr8(VDC_DATA_LO, (uint8_t)(line + RCR_BASE));
    st2(0);
    cli();
}

static void set_background(uint8_t mask, uint8_t set)
{
    uint8_t control;

    SelectVdcControl();
    control = (uint8_t)((rd8(zVdcCrLo) & mask) | set);
    wr8(VDC_DATA_LO, control);
}

static void background_off(void)
{
    set_background(CR_BG_OFF_MASK, 0);
}

static void background_on(void)
{
    set_background(0xFF, CR_BG_ON);
}

/*
 * @routine 0043:52E RasterDone
 * @run 69:452E
 * @ends 4287
 * WHAT IT DOES: Ends a raster interrupt, with interrupts back on for the next.
 * HOW: CLI, then the frame interrupt's exit.
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
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEndBand1(void)
{
    WriteBandScroll(1);
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
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEndWorkBand(void)
{
    WriteBandScroll(WORK_BAND);
    cli();
}

void abi_RasterEndWorkBand(void)
{
    RasterEndWorkBand();
}

/*
 * @routine 0045:0ED RasterSetNextLine
 * @run 69:50ED
 * @ends 4287
 * WHAT IT DOES: Moves a raster effect on: the next band's compare line goes in.
 * HOW: Counts the step, selects the compare register, writes the line as the VDC
 * counts it.
 * Entry: A line. Exit: to the frame interrupt's exit.
 */
void RasterSetNextLine(uint8_t line)
{
    next_step();
    SelectVdcRegister(VDC_REG_RCR);
    wr8(VDC_DATA_LO, line);
    st2(0);
    cli();
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
 * Entry: X band, A line. Exit: to the frame interrupt's exit.
 */
void RasterBandThenLine(uint8_t band, uint8_t line)
{
    WriteBandScroll(band);
    RasterSetNextLine(line);
}

void abi_RasterBandThenLine(void)
{
    RasterBandThenLine(cpu.x, cpu.a);
}

/*
 * The middle steps of the many-band effects: the working band takes a band chosen by
 * a table by step, its Y moved on by a second table, and the next line comes from a
 * third.
 */
static void band_table_step(uint8_t step, uint16_t bands, uint16_t y_adds, uint16_t lines)
{
    uint8_t band = rd8((uint16_t)(bands + step));
    uint8_t byte;
    uint8_t other;
    Alu r;

    work_x_from(band);
    byte = rd8((uint16_t)(ScrollBgyLo + band));
    other = rd8((uint16_t)(y_adds + step));
    r = adc8(byte, other, false);
    work_y_lo(r.a);
    byte = rd8((uint16_t)(ScrollBgyHi + band));
    r = adc8(0, byte, r.c);
    work_y_hi(r.a);
    WriteBandScroll(WORK_BAND);
    next_step();
    SelectVdcRegister(VDC_REG_RCR);
    write_line_word(rd8((uint16_t)(lines + step)));
    cli();
}

/* The same with the second table in words, for Y moves past one byte. */
static void band_table_step_word(uint8_t step, uint16_t bands, uint16_t y_adds, uint16_t lines)
{
    uint8_t band = rd8((uint16_t)(bands + step));
    uint8_t index = (uint8_t)(step << 1);
    uint8_t byte;
    uint8_t other;
    Alu r;

    work_x_from(band);
    byte = rd8((uint16_t)(ScrollBgyLo + band));
    other = rd8((uint16_t)(y_adds + index));
    r = adc8(byte, other, false);
    work_y_lo(r.a);
    byte = rd8((uint16_t)(ScrollBgyHi + band));
    other = rd8((uint16_t)(y_adds + 1 + index));
    r = adc8(byte, other, r.c);
    work_y_hi(r.a);
    WriteBandScroll(WORK_BAND);
    next_step();
    SelectVdcRegister(VDC_REG_RCR);
    write_line_word(rd8((uint16_t)(lines + step)));
    cli();
}

/*
 * @routine 0043:536 RasterEffect2
 * @run 69:4536
 * @ends 4287
 * WHAT IT DOES: Raster effect 2: two bands, split at line $BF.
 * HOW: Band 1, then band 0's X and Y high with Y low $7F.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect2(void)
{
    uint8_t byte;

    if (rd8(RasterStep) == 0) {
        RasterBandThenLine(1, 0xBF);
        return;
    }
    work_x_from(0);
    byte = rd8(ScrollBgyHi);
    work_y_hi(byte);
    work_y_lo(0x7F);
    RasterEndWorkBand();
}

/*
 * @routine 0043:55C RasterEffect3
 * @run 69:455C
 * @ends 4287
 * WHAT IT DOES: Raster effect 3: three bands, the first split moving with half of band
 * 3's Y scroll.
 * HOW: The split's line comes from that scroll; the second band takes band 2 with its
 * Y offset; the last, band 0.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect3(void)
{
    uint8_t step = rd8(RasterStep);
    uint8_t byte;

    if (step == 0) {
        byte = (uint8_t)(rd8(ScrollBgyLo + 3) >> 1);
        work_y_lo(byte);
        byte = rd8(ScrollBgyLo + WORK_BAND);
        RasterBandThenLine(1, (uint8_t)(0xAF - byte));
    } else if (step == 1) {
        work_x_from(2);
        work_y_sum(ScrollBgyLo + 2, 0x6F, ScrollBgyHi + 2);
        byte = rd8(ScrollBgyLo + 3);
        RasterBandThenLine(WORK_BAND, (uint8_t)(0xFF - byte));
    } else {
        work_x_from(0);
        work_y_lo(0xBF);
        RasterEndWorkBand();
    }
}

/*
 * @routine 0043:5B1 RasterEffect4
 * @run 69:45B1
 * @ends 4287
 * WHAT IT DOES: Raster effect 4: band 0's X with band 1's Y, in one step.
 * HOW: Copies them to the working band.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect4(void)
{
    uint8_t byte;

    work_x_from(0);
    byte = rd8(ScrollBgyLo + 1);
    work_y_lo(byte);
    byte = rd8(ScrollBgyHi + 1);
    work_y_hi(byte);
    RasterEndWorkBand();
}

/*
 * @routine 0043:5CC RasterEffect5
 * @run 69:45CC
 * @ends 4287
 * WHAT IT DOES: Raster effect 5: four bands, split at lines $5F, $7F and $FF.
 * HOW: Bands 1, 2 and 1 with their Y offsets, then band 0.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect5(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(1, 0x5F);
    } else if (step == 1) {
        work_x_from(2);
        work_y_sum(ScrollBgyLo + 2, 0x1F, ScrollBgyHi + 2);
        RasterBandThenLine(WORK_BAND, 0x7F);
    } else if (step == 2) {
        work_x_from(1);
        work_y_sum(ScrollBgyLo + 1, 0x3F, ScrollBgyHi + 1);
        RasterBandThenLine(WORK_BAND, 0xFF);
    } else {
        work_x_from(0);
        work_y_sum(ScrollBgyLo, 0xBF, ScrollBgyHi);
        RasterEndWorkBand();
    }
}

/*
 * @routine 0043:643 RasterEffect6
 * @run 69:4643
 * @ends 4287
 * WHAT IT DOES: Raster effect 6: seven bands from tables.
 * HOW: Band 4, five steps from the tables, then band 1 at Y $DB.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect6(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(4, 0x67);
    } else if (step == 6) {
        work_x_from(1);
        work_y_lo(0xDB);
        RasterEndWorkBand();
    } else {
        band_table_step(step, Effect6Bands, Effect6YAdds, Effect6Lines);
    }
}

/*
 * @routine 0043:6B6 RasterEffect7
 * @run 69:46B6
 * @ends 4287
 * WHAT IT DOES: Raster effect 7: seven bands from tables, with word Y offsets.
 * HOW: Band 4, five steps from the tables, then band 1 at Y $11B.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect7(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(4, 0x67);
    } else if (step == 6) {
        work_x_from(1);
        work_y_lo(0x1B);
        work_y_hi(0x01);
        RasterEndWorkBand();
    } else {
        band_table_step_word(step, Effect6Bands, Effect7YAdds, Effect6Lines);
    }
}

/*
 * @routine 0043:730 RasterEffect8Step4
 * @run 69:4730
 * @ends 4287
 * WHAT IT DOES: Effect 8's fifth step: turns the background off below the band.
 * HOW: Clears the background bit in the VDC's control, and sets line $E0.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect8Step4(void)
{
    background_off();
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
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect8Step0(void)
{
    uint8_t byte = (uint8_t)(rd8(ScrollBgyHi + 5) + 1);

    wr8(ScrollBgyHi + 5, byte);
    WriteBandScroll(5);
    byte = (uint8_t)(rd8(ScrollBgyHi + 5) - 1);
    wr8(ScrollBgyHi + 5, byte);
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
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void RasterEffect8Step3(void)
{
    work_x_from(0);
    work_y_lo(0x6F);
    work_y_hi(0);
    RasterBandThenLine(WORK_BAND, 0xDF);
}

void abi_RasterEffect8Step3(void)
{
    RasterEffect8Step3();
}

static void select_sprite_palette(void)
{
    wr8(VCE_ADDR_LO, COLOUR_SPRITE_BANK);
    wr8(VCE_ADDR_HI, COLOUR_SPRITE_BANK);
}

/* Colours $101-$10F back from where the step below kept them. */
static void restore_saved_colours(void)
{
    select_sprite_palette();
    for (uint8_t colour = PALETTE_SAVED_COLOURS; colour != 0; colour--) {
        uint8_t byte = rd8((uint16_t)(SavedColoursLo + colour));

        wr8(VCE_DATA_LO, byte);
        byte = rd8((uint16_t)(SavedColoursHi + colour));
        wr8(VCE_DATA_HI, byte);
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
        uint8_t byte = rd8(VCE_DATA_LO);

        wr8((uint16_t)(SavedColoursLo + colour), byte);
        byte = rd8(VCE_DATA_HI);
        wr8((uint16_t)(SavedColoursHi + colour), byte);
    }
    select_sprite_palette();
    for (uint8_t colour = PALETTE_SAVED_COLOURS; colour != 0; colour--) {
        wr8(VCE_DATA_LO, COLOUR_FILL);
        wr8(VCE_DATA_HI, 0);
    }
    SelectVdcControl();
    which = rd8(zFrameCount) & 1;
    control = rd8(zVdcCrLo);
    control &= rd8((uint16_t)(FlickerMasks + which));
    wr8(VDC_DATA_LO, control);
    next_step();
    SelectVdcRegister(VDC_REG_RCR);
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
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect8(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0)
        RasterEffect8Step0();
    else if (step < 3)
        band_table_step_word(step, Effect8Bands, Effect8YAdds, Effect8Lines);
    else if (step == 3)
        RasterEffect8Step3();
    else if (step == 4)
        RasterEffect8Step4();
    else if (step == 5)
        flatten_colours();
    else
        restore_saved_colours();
}

/* Two bands of a three-step split, the last at a fixed Y. */
static void three_step_split(uint8_t line0, uint8_t y1, uint8_t line1, uint8_t y2)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(0, line0);
    } else if (step == 1) {
        work_x_from(1);
        work_y_lo(y1);
        work_y_hi(0x01);
        RasterBandThenLine(WORK_BAND, line1);
    } else {
        work_x_from(0);
        work_y_lo(y2);
        work_y_hi(0);
        RasterEndWorkBand();
    }
}

/*
 * @routine 0044:03D RasterEffect9
 * @run 69:483D
 * @ends 4287
 * WHAT IT DOES: Raster effect 9: three bands, split at lines $7F and $BF.
 * HOW: Band 0, band 1 at Y $13F, band 0 at Y $7F.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect9(void)
{
    three_step_split(0x7F, 0x3F, 0xBF, 0x7F);
}

/*
 * @routine 0044:07F RasterEffect10
 * @run 69:487F
 * @ends 4287
 * WHAT IT DOES: Raster effect 10: three bands, split at lines $9F and $DF.
 * HOW: Band 0, band 1 at Y $15F, band 0 at Y $9F.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect10(void)
{
    three_step_split(0x9F, 0x5F, 0xDF, 0x9F);
}

/*
 * @routine 0044:0C1 RasterEffect11
 * @run 69:48C1
 * @ends 4287
 * WHAT IT DOES: Raster effect 11: the background off below the line.
 * HOW: Clears its bit in the VDC's control.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect11(void)
{
    background_off();
    cli();
}

/*
 * @routine 0044:0CE RasterEffect12
 * @run 69:48CE
 * @ends 4287
 * WHAT IT DOES: Raster effect 12: five bands from tables.
 * HOW: Band 2, three steps from the tables, then band 5 at Y $B8.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect12(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(2, 0x7F);
    } else if (step == 4) {
        work_x_from(5);
        work_y_lo(0xB8);
        RasterEndWorkBand();
    } else {
        band_table_step(step, Effect12Bands, Effect12YAdds, Effect12Lines);
    }
}

/*
 * @routine 0044:139 RasterEffect13
 * @run 69:4939
 * @ends 4287
 * WHAT IT DOES: Raster effect 13: six bands from tables, the first scrolled one line
 * further for the one band, the last with the background off at the game's say.
 * HOW: Raises band 4's Y high around its write, then the tables, then band 0.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect13(void)
{
    uint8_t step = rd8(RasterStep);
    uint8_t byte;

    if (step == 0) {
        byte = (uint8_t)(rd8(ScrollBgyHi + 4) + 1);
        wr8(ScrollBgyHi + 4, byte);
        WriteBandScroll(4);
        byte = (uint8_t)(rd8(ScrollBgyHi + 4) - 1);
        wr8(ScrollBgyHi + 4, byte);
        RasterSetNextLine(0x5F);
    } else if (step == 5) {
        work_x_from(0);
        work_y_lo(0xCF);
        WriteBandScroll(WORK_BAND);
        if (rd8(ScrollXLo + 7) != 0)
            background_off();
        cli();
    } else {
        band_table_step_word(step, Effect13Bands, Effect13YAdds, Effect13Lines);
    }
}

/*
 * @routine 0044:1D1 RasterEffect14
 * @run 69:49D1
 * @ends 4287
 * WHAT IT DOES: Raster effect 14: bands with the sprite colours flattened and restored
 * over the last two.
 * HOW: Band 4, band 0, then the colours' two steps.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect14(void)
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
        byte = rd8(ScrollBgyHi);
        work_y_hi(byte);
        work_y_lo(0x2F);
        WriteBandScroll(WORK_BAND);
        next_step();
        SelectVdcRegister(VDC_REG_RCR);
        st1(0x11);
        st2(0x01);
        cli();
    }
}

/* A byte of the frame's count over 8, plus the step: where a wave starts. */
static uint8_t wave_index(uint16_t counter_field, uint8_t shift)
{
    uint8_t byte = (uint8_t)(rd8(counter_field) >> shift);
    uint8_t step = rd8(RasterStep);

    return (uint8_t)(((uint8_t)(byte + step) & 7) << 1);
}

/*
 * @routine 0044:216 RasterEffect15
 * @run 69:4A16
 * @ends 4287
 * WHAT IT DOES: Raster effect 15: a water wave: the band each line pair shifted
 * sideways by a table that moves with the frame's count, for 20 steps.
 * HOW: Band 2, then each step's X from band 1 plus the wave, Y moving on two lines
 * with the step; then band 0.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect15(void)
{
    uint8_t step = rd8(RasterStep);
    uint8_t byte;
    uint8_t other;
    uint8_t index;
    Alu r;

    if (step == 0) {
        byte = rd8(RasterWaterLine);
        RasterBandThenLine(2, (uint8_t)(0xCF - byte));
        return;
    }
    if (step >= 0x15) {
        work_x_from(0);
        work_y_hi(0x01);
        work_y_lo(0xB7);
        RasterEndWorkBand();
        return;
    }
    step = (uint8_t)(rd8(RasterStep) - 1);
    other = rd8(ScrollBgyLo + 1);
    r = adc8((uint8_t)(step << 1), other, (step & 0x80) != 0);
    r = adc8(r.a, 0x8F, false);
    work_y_lo(r.a);
    other = rd8(ScrollBgyHi + 1);
    r = adc8(0, other, r.c);
    work_y_hi(r.a);
    index = wave_index(zFrameTick, 3);
    byte = rd8((uint16_t)(WaterWave + index));
    other = rd8(ScrollBgxLo + 1);
    r = adc8(byte, other, false);
    wr8(ScrollBgxLo + WORK_BAND, r.a);
    byte = rd8((uint16_t)(WaterWave + 1 + index));
    other = rd8(ScrollBgxHi + 1);
    r = adc8(byte, other, r.c);
    wr8(ScrollBgxHi + WORK_BAND, r.a);
    WriteBandScroll(WORK_BAND);
    SelectVdcRegister(VDC_REG_RCR);
    step = (uint8_t)(rd8(RasterStep) << 1);
    work_y_lo(step);
    other = rd8(RasterWaterLine);
    byte = (uint8_t)(0xCF - other);
    other = rd8(ScrollBgyLo + WORK_BAND);
    r = adc8(byte, other, false);
    wr8(VDC_DATA_LO, r.a);
    wr8(VDC_DATA_HI, r.c ? 1 : 0);
    next_step();
    cli();
}

/*
 * @routine 0044:2B1 RasterEffect16
 * @run 69:4AB1
 * @ends 4287
 * WHAT IT DOES: Raster effect 16: six bands from tables.
 * HOW: Band 5, four steps from the tables, then band 1 at Y $BF.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect16(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(5, 0x57);
    } else if (step == 5) {
        work_x_from(1);
        work_y_lo(0xBF);
        RasterEndWorkBand();
    } else {
        band_table_step(step, Effect16Bands, Effect16YAdds, Effect16Lines);
    }
}

/*
 * @routine 0044:321 RasterEffect17
 * @run 69:4B21
 * @ends 4287
 * WHAT IT DOES: Raster effect 17: six bands from tables.
 * HOW: Band 4, four steps from the tables, then band 2 at Y $D7.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect17(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(4, 0x67);
    } else if (step == 5) {
        work_x_from(2);
        work_y_lo(0xD7);
        RasterEndWorkBand();
    } else {
        band_table_step(step, Effect17Bands, Effect17YAdds, Effect17Lines);
    }
}

/* Band 0's X low with X high 1, at Y $168: the fixed band of effect 18. */
static void effect18_work_band(void)
{
    uint8_t byte = rd8(ScrollBgxLo);

    wr8(ScrollBgxLo + WORK_BAND, byte);
    wr8(ScrollBgxHi + WORK_BAND, 0x01);
    work_y_lo(0x68);
    work_y_hi(0x01);
}

/*
 * @routine 0044:38F RasterEffect18
 * @run 69:4B8F
 * @ends 4287
 * WHAT IT DOES: Raster effect 18: a band whose height the game sets, then the rest of
 * the screen following it.
 * HOW: The fixed band to the line in band 3's Y, then band 1 built from band 0 moved
 * down by that height, then the fixed band again.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect18(void)
{
    uint8_t step = rd8(RasterStep);
    uint8_t byte;
    uint8_t other;
    Alu r;

    if (step == 0) {
        effect18_work_band();
        WriteBandScroll(WORK_BAND);
        next_step();
        SelectVdcRegister(VDC_REG_RCR);
        write_line_byte(rd8(ScrollBgyLo + 2));
    } else if (step == 1) {
        byte = rd8(ScrollBgxLo);
        wr8(ScrollBgxLo + 1, byte);
        byte = rd8(ScrollBgxHi);
        wr8(ScrollBgxHi + 1, byte);
        byte = rd8(ScrollBgyLo);
        other = rd8(ScrollBgyLo + 2);
        r = adc8(byte, other, false);
        wr8(ScrollBgyLo + 1, r.a);
        r = adc8(0, 0, r.c);
        wr8(ScrollBgyHi + 1, r.a);
        WriteBandScroll(1);
        next_step();
        SelectVdcRegister(VDC_REG_RCR);
        other = rd8(ScrollBgyLo + 2);
        write_line_word((uint8_t)(0xE0 - other));
        cli();
    } else {
        effect18_work_band();
        RasterEndWorkBand();
    }
}

/*
 * @routine 0044:415 RasterEffect19
 * @run 69:4C15
 * @ends 4287
 * WHAT IT DOES: Raster effect 19: a stretch of screen with the background off, its
 * edges set by two bytes the game moves.
 * HOW: Band 0 background off to the first; band 1 background on to the second; band 2,
 * background off at the game's say.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect19(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        WriteBandScroll(0);
        background_off();
        next_step();
        SelectVdcRegister(VDC_REG_RCR);
        write_line_byte(rd8(ScrollBgyLo + 5));
    } else if (step == 1) {
        WriteBandScroll(1);
        background_on();
        next_step();
        SelectVdcRegister(VDC_REG_RCR);
        write_line_word(rd8(ScrollBgyLo + 6));
        cli();
    } else {
        if (rd8(ScrollBgyHi + 6) == 0)
            background_off();
        WriteBandScroll(2);
        cli();
    }
}

/*
 * @routine 0044:479 RasterEffect20
 * @run 69:4C79
 * @ends 4287
 * WHAT IT DOES: Raster effect 20: as effect 19 with the edges allowed to meet: steps
 * skip when a moving edge passes a fixed line at $60, $77, $A7 or $BF.
 * HOW: Band 3, band 0 and band 6 up to those lines, band 1 background off, band 4
 * background on.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect20(void)
{
    uint8_t step = rd8(RasterStep);
    uint8_t line;

    if (step == 0) {
        WriteBandScroll(3);
        next_step();
        SelectVdcRegister(VDC_REG_RCR);
        line = rd8(ScrollBgyLo + 5);
        if (line >= 0x60)
            line = 0x5F;
        else
            next_step();
        write_line_byte(line);
    } else if (step == 1) {
        WriteBandScroll(0);
        line = rd8(ScrollBgyLo + 5);
        if (line >= 0x77) {
            wr8(RasterStep, 4);
            SelectVdcRegister(VDC_REG_RCR);
            write_line_byte(0xA7);
        } else {
            next_step();
            SelectVdcRegister(VDC_REG_RCR);
            write_line_byte(rd8(ScrollBgyLo + 5));
        }
    } else if (step == 2) {
        WriteBandScroll(1);
        background_off();
        next_step();
        SelectVdcRegister(VDC_REG_RCR);
        line = rd8(ScrollBgyLo + 6);
        if (line == 0xBF) {
            cli();
        } else {
            if (line >= 0xA7)
                next_step();
            write_line_byte(line);
        }
    } else if (step == 3) {
        WriteBandScroll(6);
        background_on();
        next_step();
        SelectVdcRegister(VDC_REG_RCR);
        line = rd8(ScrollBgyLo + 6);
        write_line_byte(line >= 0xA8 ? line : 0xA7);
    } else {
        background_on();
        WriteBandScroll(4);
        cli();
    }
}

/*
 * @routine 0044:532 RasterEffect21
 * @run 69:4D32
 * @ends 4287
 * WHAT IT DOES: Raster effect 21: five bands split at $7F, $87 and $FF, then a late
 * compare, the background off in the last at a stage value.
 * HOW: Bands 2, 1, 3, then band 0 twice.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect21(void)
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
        work_y_lo(0xBF);
        work_y_hi(0);
        WriteBandScroll(WORK_BAND);
        next_step();
        SelectVdcRegister(VDC_REG_RCR);
        st1(0x07);
        st2(0x01);
        cli();
    } else {
        if (rd8(zRasterStage) == 3)
            background_off();
        work_x_from(0);
        work_y_lo(0xC7);
        work_y_hi(0);
        RasterEndWorkBand();
    }
}

/*
 * @routine 0044:5A4 RasterEffect22
 * @run 69:4DA4
 * @ends 4287
 * WHAT IT DOES: Raster effect 22: two bands: band 2 at Y $110 to line $67, band 0 after.
 * HOW: Builds the working band each step.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect22(void)
{
    uint8_t byte;
    Alu r;

    if (rd8(RasterStep) == 0) {
        work_y_lo(0x10);
        work_y_hi(0x01);
        work_x_from(2);
        RasterBandThenLine(WORK_BAND, 0x67);
    } else {
        work_x_from(0);
        byte = rd8(ScrollBgyLo);
        r = adc8(0x27, byte, false);
        work_y_lo(r.a);
        work_y_hi(0);
        RasterEndWorkBand();
    }
}

/* Effects 23, 25 and 29: flatten the colours and restore them, after a first line. */
static void flatten_after_line(uint8_t first_line)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        next_step();
        SelectVdcRegister(VDC_REG_RCR);
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
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect23(void)
{
    flatten_after_line(0x01);
}

/*
 * @routine 0044:5FE RasterEffect25
 * @run 69:4DFE
 * @ends 4287
 * WHAT IT DOES: Raster effect 25: the sprite colours flattened at once and restored
 * after.
 * HOW: The colours' two steps.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect25(void)
{
    if (rd8(RasterStep) == 0)
        flatten_colours();
    else
        restore_saved_colours();
}

/*
 * @routine 0044:609 RasterEffect29
 * @run 69:4E09
 * @ends 4287
 * WHAT IT DOES: Raster effect 29: the sprite colours flattened from a later compare than
 * effect 23's, and restored after.
 * HOW: A first compare, then the colours' two steps.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect29(void)
{
    flatten_after_line(0x18);
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
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect24(void)
{
    uint8_t step = rd8(RasterStep);
    uint8_t byte;
    uint8_t other;
    uint8_t index;
    Alu r;

    if (step == 0) {
        wr8(ScrollBgxLo + 2, 0xFF);
        wr8(ScrollBgxHi + 2, 0);
        RasterBandThenLine(0, 0xFF);
        return;
    }
    if (step >= 0x11) {
        RasterEndBand1();
        return;
    }
    step = (uint8_t)(rd8(RasterStep) - 1);
    other = rd8(ScrollBgyLo + 1);
    r = adc8((uint8_t)(step << 1), other, (step & 0x80) != 0);
    work_y_lo(r.a);
    work_y_hi(0);
    index = wave_index(zWaveCounter, 3);
    byte = rd8((uint16_t)(DeepWave + index));
    other = rd8(ScrollBgxLo + 1);
    r = adc8(byte, other, false);
    wr8(ScrollBgxLo + WORK_BAND, r.a);
    byte = rd8((uint16_t)(DeepWave + 1 + index));
    other = rd8(ScrollBgxHi + 1);
    r = adc8(byte, other, r.c);
    wr8(ScrollBgxHi + WORK_BAND, r.a);
    WriteBandScroll(WORK_BAND);
    SelectVdcRegister(VDC_REG_RCR);
    byte = (uint8_t)(rd8(zWaveCounter) >> 3);
    other = rd8(RasterStep);
    byte = (uint8_t)((uint8_t)(byte + other) >> 3) & 1;
    other = rd8(ScrollBgxLo + 2);
    r = adc8((uint8_t)(byte + 2), other, false);
    wr8(ScrollBgxLo + 2, r.a);
    wr8(VDC_DATA_LO, r.a);
    other = rd8(ScrollBgxHi + 2);
    r = adc8(0, other, r.c);
    wr8(ScrollBgxHi + 2, r.a);
    wr8(VDC_DATA_HI, r.a);
    next_step();
    cli();
}

/*
 * Effects 26 and 27: a table of bands in RAM, each marked: bit 7 clear stands still, set
 * follows the deep wave. Each step is a line or eight apart.
 */
static void marked_band_step(uint8_t limit, uint8_t shift)
{
    uint8_t step = rd8(RasterStep);
    uint8_t mark;
    uint8_t byte;
    uint8_t other;
    Alu r;

    if (step == 0) {
        RasterBandThenLine(0, 0x41);
        return;
    }
    if (step >= limit) {
        RasterEndBand1();
        return;
    }
    step = rd8(RasterStep);
    work_y_lo((uint8_t)(((uint8_t)(step - 1) << shift) + 1));
    mark = rd8((uint16_t)(RasterMarks + step));
    if (!(mark & 0x80)) {
        wr8(ScrollBgxLo + WORK_BAND, 0);
        work_y_hi(0);
        wr8(ScrollBgxHi + WORK_BAND, 0x01);
    } else {
        uint8_t index;

        work_y_hi(0x01);
        index = wave_index(zWaveCounter, 2);
        byte = rd8((uint16_t)(DeepWave + index));
        wr8(ScrollBgxLo + WORK_BAND, byte);
        byte = rd8((uint16_t)(DeepWave + 1 + index));
        wr8(ScrollBgxHi + WORK_BAND, byte);
    }
    mark = rd8((uint16_t)(RasterMarks + step)) & 0x7F;
    other = rd8(ScrollBgxLo + WORK_BAND);
    wr8(ScrollBgxLo + WORK_BAND, (uint8_t)(mark + other));
    WriteBandScroll(WORK_BAND);
    SelectVdcRegister(VDC_REG_RCR);
    byte = rd8(RasterStep);
    r = adc8((uint8_t)(byte << shift), 0x41, false);
    wr8(VDC_DATA_LO, r.a);
    wr8(VDC_DATA_HI, r.c ? 1 : 0);
    next_step();
    cli();
}

/*
 * @routine 0044:6B0 RasterEffect26
 * @run 69:4EB0
 * @ends 4287
 * WHAT IT DOES: Raster effect 26: a marked band every two lines, for $70 steps.
 * HOW: See marked_band_step.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect26(void)
{
    marked_band_step(0x71, 1);
}

/*
 * @routine 0044:726 RasterEffect27
 * @run 69:4F26
 * @ends 4287
 * WHAT IT DOES: Raster effect 27: a marked band every eight lines, for $1C steps.
 * HOW: See marked_band_step.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect27(void)
{
    marked_band_step(0x1D, 3);
}

/*
 * @routine 0044:7A0 RasterEffect28
 * @run 69:4FA0
 * @ends 4287
 * WHAT IT DOES: Raster effect 28: a band every two lines from line $A7, each shaking a
 * pixel or two sideways, now and then, at random.
 * HOW: Band 3, then steps with band 3's X; when the frame's count and step land on
 * zero, the top of the random byte is added. Then band 0.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect28(void)
{
    uint8_t step = rd8(RasterStep);
    uint8_t byte;
    uint8_t other;
    Alu r;

    if (step == 0) {
        RasterBandThenLine(3, 0xA7);
        return;
    }
    if (step >= 0x0C) {
        work_x_from(0);
        work_y_lo(0x7F);
        work_y_hi(0);
        RasterEndWorkBand();
        return;
    }
    step = rd8(RasterStep);
    work_y_lo((uint8_t)((step << 1) + 0x67));
    step = rd8(RasterStep);
    if (step == 1) {
        byte = (uint8_t)(rd8(ScrollBgyLo + WORK_BAND) - 1);
        work_y_lo(byte);
        byte = (uint8_t)(rd8(ScrollBgyLo + WORK_BAND) - 1);
        work_y_lo(byte);
    }
    work_y_hi(0);
    work_x_from(3);
    byte = (uint8_t)(rd8(zFrameTick) >> 3);
    other = rd8(RasterStep);
    if (((uint8_t)((uint8_t)(byte + other) >> 1) & 0x0F) == 0) {
        byte = (uint8_t)((rd8(zRandom3) & 1) + 1);
        other = rd8(ScrollBgxLo + WORK_BAND);
        r = adc8(byte, other, false);
        wr8(ScrollBgxLo + WORK_BAND, r.a);
        other = rd8(ScrollBgxHi + WORK_BAND);
        r = adc8(0, other, r.c);
        wr8(ScrollBgxHi + WORK_BAND, r.a);
    }
    WriteBandScroll(WORK_BAND);
    SelectVdcRegister(VDC_REG_RCR);
    byte = rd8(RasterStep);
    r = adc8((uint8_t)(byte << 1), 0xA9, false);
    wr8(VDC_DATA_LO, r.a);
    wr8(VDC_DATA_HI, r.c ? 1 : 0);
    next_step();
    cli();
}

/*
 * @routine 0045:02D RasterEffect30
 * @run 69:502D
 * @ends 4287
 * WHAT IT DOES: Raster effect 30: five bands from tables.
 * HOW: Band 1, three steps from the tables, then band 0 at Y $AF.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect30(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        RasterBandThenLine(1, 0x8F);
    } else if (step == 4) {
        work_x_from(0);
        work_y_lo(0xAF);
        work_y_hi(0);
        RasterEndWorkBand();
    } else {
        band_table_step(step, Effect30Bands, Effect30YAdds, Effect30Lines);
    }
}

/*
 * @routine 0045:09B RasterEffect31
 * @run 69:509B
 * @ends 4287
 * WHAT IT DOES: Raster effect 31: the background off from one line and back on at
 * another, with band 1's Y set for the second.
 * HOW: Background off and line $57; background on and line $E7; band 0.
 * Entry: raster interrupt. Exit: to the frame interrupt's exit.
 */
void abi_RasterEffect31(void)
{
    uint8_t step = rd8(RasterStep);

    if (step == 0) {
        background_off();
        next_step();
        SelectVdcRegister(VDC_REG_RCR);
        wr8(VDC_DATA_LO, 0x57);
        st2(0);
        cli();
    } else if (step == 2) {
        work_x_from(0);
        work_y_lo(0xA7);
        work_y_hi(0);
        RasterEndWorkBand();
    } else {
        background_on();
        wr8(ScrollBgyLo + 1, 0x17);
        RasterBandThenLine(1, 0xE7);
    }
}

/*
 * @routine 0043:4E1 RasterInterrupt
 * @run 69:44E1
 * @ends 4287
 * WHAT IT DOES: A raster interrupt: runs this frame's effect.
 * HOW: Through the table by effect number. Effect 0 only ends the interrupt.
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
