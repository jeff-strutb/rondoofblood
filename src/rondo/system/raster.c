/*
 * Raster effects: the screen split into horizontal bands, each with its own
 * scroll. At vblank the first band's compare line goes in; each raster interrupt
 * then runs the frame's effect, one step per band: it writes that band's scroll,
 * and sets the line of the next, or runs out of bands. Band 8 of the scroll tables
 * is the working band most effects build their next scroll in.
 *
 * Every handler ends on the frame interrupt's exit (its tag says @ends 4287), which pulls
 * every register and returns from the interrupt. Each routine returns the registers and flags
 * it holds at that jump, from the assembly path by path; its abi_ wrapper puts them in the
 * registers.
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

/*
 * What a handler holds when it jumps to the frame interrupt's exit: the registers and the flags. Registers
 * are dead there (the exit pulls them), but the jump is judged on them, so each path says what it holds.
 * Held is what a path keeps of its entry: X, Y, and the carry and overflow.
 */
typedef struct {
    uint8_t x, y;
    bool c, v;
} Held;

typedef struct {
    uint8_t a, x, y, nz;
    bool c, v;
} Exit;

static Held held_in_shim(void)
{
    Held h = { cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0 };

    return h;
}

static uint8_t nz_of(uint8_t value)
{
    return (uint8_t)((value & P_N) | (value == 0 ? P_Z : 0));
}

static Exit exit_of(uint8_t a, uint8_t x, uint8_t y, bool c, bool v)
{
    Exit e = { a, x, y, nz_of(a), c, v };

    return e;
}

/* Interrupts on, and the path's registers as the exit leaves them: for a path to return. */
static Exit finish(Exit e)
{
    cli();
    return e;
}

/* The wrapper's: what the path holds goes into the registers the original has at the jump to the exit. */
static void exit_regs_shim(Exit e)
{
    cpu.a = e.a;
    cpu.x = e.x;
    cpu.y = e.y;
    cpu.p = (uint8_t)((cpu.p & ~(P_N | P_Z)) | e.nz);
    abi_flag(P_C, e.c);
    abi_flag(P_V, e.v);
}

/* JSR WriteBandScroll, JSR SelectVdcRegister and JSR SelectVdcControl, each framed. */
static uint8_t write_band_at(uint16_t ret, uint8_t band)
{
    uint8_t a;

    jsr_push(ret);
    a = WriteBandScroll(band);
    jsr_pop();
    return a;
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
static uint8_t background_off_at(uint16_t ret)
{
    uint8_t control;

    select_control_at(ret);
    control = (uint8_t)(RD8(ZP->vdc_cr.lo) & CR_BG_OFF_MASK);
    WR8(VDC->data_lo, control);
    return control;
}

static uint8_t background_on_at(uint16_t ret)
{
    uint8_t control;

    select_control_at(ret);
    control = (uint8_t)(RD8(ZP->vdc_cr.lo) | CR_BG_ON);
    WR8(VDC->data_lo, control);
    return control;
}

/* The working band takes another band's X scroll, low then high. */
static void work_x_from(uint8_t band)
{
    uint8_t byte = RD8(BANDS->bgx_lo[band]);

    WR8(BANDS->bgx_lo[WORK_BAND], byte);
    byte = RD8(BANDS->bgx_hi[band]);
    WR8(BANDS->bgx_hi[WORK_BAND], byte);
}

/* INC RasterStep: its N and Z are the new step's, which the paths that end on it hold. */
static uint8_t next_step(void)
{
    return INC8(RASTERST->step);
}

/*
 * The working band's Y is a band's with a byte added to the low: the low is read, the byte read,
 * the sum written, then the high read with the carry added and written. The byte is the original's
 * table by step, or a constant.
 */
static Sum work_y_from_plus(uint8_t band, uint8_t add)
{
    Sum low = add_bytes(RD8(BANDS->bgy_lo[band]), add, false);
    Sum high;

    WR8(BANDS->bgy_lo[WORK_BAND], low.value);
    high = add_bytes(0, RD8(BANDS->bgy_hi[band]), low.carry);
    WR8(BANDS->bgy_hi[WORK_BAND], high.value);
    return high;
}

/* The working band's Y low then high, as a stored constant. */
static void work_y_set(uint8_t low, uint8_t high)
{
    WR8(BANDS->bgy_lo[WORK_BAND], low);
    WR8(BANDS->bgy_hi[WORK_BAND], high);
}

/* The compare line as the original's tail writes it: A plus $40 low, the carry into the high. */
static Sum line_word_of(uint8_t line)
{
    Sum low = add_bytes(0x40, line, false);
    Sum high;

    WR8(VDC->data_lo, low.value);
    high = add_bytes(0, 0, low.carry);
    WR8(VDC->data_hi, high.value);
    return high;
}

/* The shared tail of the single-byte lines: A plus $40 low and a zero high. */
static Sum line_byte_tail(uint8_t line)
{
    Sum low = add_bytes(line, RCR_BASE, false);

    WR8(VDC->data_lo, low.value);
    st2(0);
    return low;
}

/* The working band's scroll is written, the step counted, and the compare register selected. */
static void write_work_then_select(uint16_t write_ret, uint16_t select_ret)
{
    (void)write_band_at(write_ret, WORK_BAND);
    next_step();
    select_at(select_ret, VDC_REG_RCR);
}

/* Each table's read: by the disc table the traces settled, or by its named address where none did. The index may reach past a byte: a word's high byte. */
typedef uint8_t (*TableRead)(uint16_t index);

static uint8_t read_Effect6Bands(uint16_t index)
{
    return TBL8(Effect6Bands, index);
}

static uint8_t read_Effect6YAdds(uint16_t index)
{
    return TBL8(Effect6YAdds, index);
}

static uint8_t read_Effect6Lines(uint16_t index)
{
    return TBL8(Effect6Lines, index);
}

static uint8_t read_Effect8YAdds(uint16_t index)
{
    return TBL8(Effect8YAdds, index);
}

static uint8_t read_Effect8Lines(uint16_t index)
{
    return TBL8(Effect8Lines, index);
}

static uint8_t read_Effect12Bands(uint16_t index)
{
    return TBL8(Effect12Bands, index);
}

static uint8_t read_Effect12Lines(uint16_t index)
{
    return TBL8(Effect12Lines, index);
}

static uint8_t read_Effect13Bands(uint16_t index)
{
    return TBL8(Effect13Bands, index);
}

static uint8_t read_Effect13YAdds(uint16_t index)
{
    return TBL8(Effect13YAdds, index);
}

static uint8_t read_Effect13Lines(uint16_t index)
{
    return TBL8(Effect13Lines, index);
}

static uint8_t read_Effect16Bands(uint16_t index)
{
    return TBL8(Effect16Bands, index);
}

static uint8_t read_Effect16YAdds(uint16_t index)
{
    return TBL8(Effect16YAdds, index);
}

static uint8_t read_Effect16Lines(uint16_t index)
{
    return TBL8(Effect16Lines, index);
}

static uint8_t read_Effect17Bands(uint16_t index)
{
    return TBL8(Effect17Bands, index);
}

static uint8_t read_Effect17Lines(uint16_t index)
{
    return TBL8(Effect17Lines, index);
}

static uint8_t read_DeepWave(uint16_t index)
{
    return TBL8(DeepWave, index);
}

static uint8_t read_Effect7YAdds(uint16_t index)
{
    return rd8((uint16_t)(Effect7YAdds + index));
}

static uint8_t read_Effect8Bands(uint16_t index)
{
    return rd8((uint16_t)(Effect8Bands + index));
}

static uint8_t read_Effect12YAdds(uint16_t index)
{
    return rd8((uint16_t)(Effect12YAdds + index));
}

static uint8_t read_Effect17YAdds(uint16_t index)
{
    return rd8((uint16_t)(Effect17YAdds + index));
}

static uint8_t read_Effect30Bands(uint16_t index)
{
    return rd8((uint16_t)(Effect30Bands + index));
}

static uint8_t read_Effect30YAdds(uint16_t index)
{
    return rd8((uint16_t)(Effect30YAdds + index));
}

static uint8_t read_Effect30Lines(uint16_t index)
{
    return rd8((uint16_t)(Effect30Lines + index));
}

static uint8_t read_WaterWave(uint16_t index)
{
    return rd8((uint16_t)(WaterWave + index));
}

/*
 * The middle steps of the many-band effects: the working band takes a band chosen by a table by
 * step, its Y moved on by a second table, and the next line comes from a third. The reads are in
 * the original's order: the band, the byte added to Y's low between its read and its write, the
 * line. They leave A the line's high byte (the carry into it), X 8, Y the step, and nothing carried.
 */
static Exit band_table_step(uint8_t step, TableRead bands, TableRead y_adds, TableRead lines,
                            uint16_t write_ret, uint16_t select_ret)
{
    uint8_t band = bands(step);
    uint8_t low_byte;
    uint8_t add;
    uint8_t line;
    Sum low;
    Sum high;

    work_x_from(band);
    low_byte = RD8(BANDS->bgy_lo[band]);
    add = y_adds(step);
    low = add_bytes(low_byte, add, false);
    WR8(BANDS->bgy_lo[WORK_BAND], low.value);
    high = add_bytes(0, RD8(BANDS->bgy_hi[band]), low.carry);
    WR8(BANDS->bgy_hi[WORK_BAND], high.value);
    write_work_then_select(write_ret, select_ret);
    line = lines(step);
    high = line_word_of(line);
    return finish(exit_of(high.value, WORK_BAND, step, high.carry, high.overflow));
}

/*
 * The same with the second table in words, for Y moves past one byte. The doubled step is an
 * eight-bit Y, so the second and third tables are read by the step's low seven bits; Y after the
 * call is that index again.
 */
static Exit band_table_step_word(uint8_t step, TableRead bands, TableRead y_adds, TableRead lines,
                                 uint16_t write_ret, uint16_t select_ret)
{
    uint8_t band = bands(step);
    uint8_t index = (uint8_t)(step << 1);
    uint8_t low_byte;
    uint8_t high_byte;
    uint8_t add;
    uint8_t line;
    Sum low;
    Sum high;

    work_x_from(band);
    low_byte = RD8(BANDS->bgy_lo[band]);
    add = y_adds(index);
    low = add_bytes(low_byte, add, false);
    WR8(BANDS->bgy_lo[WORK_BAND], low.value);
    high_byte = RD8(BANDS->bgy_hi[band]);
    add = y_adds((uint16_t)(index + 1));
    high = add_bytes(high_byte, add, low.carry);
    WR8(BANDS->bgy_hi[WORK_BAND], high.value);
    write_work_then_select(write_ret, select_ret);
    line = lines((uint8_t)(index >> 1));
    high = line_word_of(line);
    return finish(exit_of(high.value, WORK_BAND, (uint8_t)(index >> 1), high.carry, high.overflow));
}

/* The working band's X is band 1's, moved by a word of a wave table. */
static void work_x_band1_plus_wave(TableRead wave, uint8_t index)
{
    uint8_t low_byte = wave(index);
    Sum low = add_bytes(low_byte, RD8(BANDS->bgx_lo[1]), false);
    Sum high;
    uint8_t high_byte;

    WR8(BANDS->bgx_lo[WORK_BAND], low.value);
    high_byte = wave((uint16_t)(index + 1));
    high = add_bytes(high_byte, RD8(BANDS->bgx_hi[1]), low.carry);
    WR8(BANDS->bgx_hi[WORK_BAND], high.value);
}

/* A table by a count of the frame's, over eight, plus the step: where a wave starts. */
static uint8_t wave_index(uint8_t counter, uint8_t shift)
{
    uint8_t byte = (uint8_t)(counter >> shift);
    Sum sum = add_bytes(byte, RD8(RASTERST->step), false);

    return (uint8_t)((sum.value & 7) << 1);
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
Exit RasterEndBand1(Held h)
{
    uint8_t a = write_band_at(0x452E, 1);

    return finish(exit_of(a, 1, h.y, h.c, h.v));
}

void abi_RasterEndBand1(void)
{
    exit_regs_shim(RasterEndBand1(held_in_shim()));
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
Exit RasterEndWorkBand(Held h)
{
    uint8_t a = write_band_at(0x452E, WORK_BAND);

    return finish(exit_of(a, WORK_BAND, h.y, h.c, h.v));
}

void abi_RasterEndWorkBand(void)
{
    exit_regs_shim(RasterEndWorkBand(held_in_shim()));
}

/* The body RasterSetNextLine and RasterBandThenLine share: the step counted, the compare line from the stack. */
static Exit line_from_stack(uint8_t x, Held h)
{
    uint8_t line;

    next_step();
    select_at(0x50FC, VDC_REG_RCR);
    line = pull8();
    WR8(VDC->data_lo, line);
    st2(0);
    return finish(exit_of(line, x, h.y, h.c, h.v));
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
Exit RasterSetNextLine(uint8_t line, Held h)
{
    push8(line);
    return line_from_stack(h.x, h);
}

void abi_RasterSetNextLine(void)
{
    exit_regs_shim(RasterSetNextLine(cpu.a, held_in_shim()));
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
Exit RasterBandThenLine(uint8_t band, uint8_t line, Held h)
{
    push8(line);
    (void)write_band_at(0x50F4, band);
    return line_from_stack(band, h);
}

void abi_RasterBandThenLine(void)
{
    exit_regs_shim(RasterBandThenLine(cpu.x, cpu.a, held_in_shim()));
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
Exit RasterEffect2(Held h)
{
    uint8_t byte;

    if (RD8(RASTERST->step) == 0) {
        return RasterBandThenLine(1, 0xBF, h);
    }
    work_x_from(0);
    byte = RD8(BANDS->bgy_hi[0]);
    WR8(BANDS->bgy_hi[WORK_BAND], byte);
    WR8(BANDS->bgy_lo[WORK_BAND], 0x7F);
    return RasterEndWorkBand(h);
}

void abi_RasterEffect2(void)
{
    exit_regs_shim(RasterEffect2(held_in_shim()));
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
Exit RasterEffect3(Held h)
{
    uint8_t step = RD8(RASTERST->step);
    uint8_t byte;
    Sum line;

    if (step == 0) {
        byte = (uint8_t)(RD8(BANDS->bgy_lo[3]) >> 1);
        WR8(BANDS->bgy_lo[WORK_BAND], byte);
        line = sub_bytes(0xAF, RD8(BANDS->bgy_lo[WORK_BAND]), true);
        return RasterBandThenLine(1, line.value, (Held){ h.x, h.y, line.carry, line.overflow });
    }
    if (step == 1) {
        work_x_from(2);
        (void)work_y_from_plus(2, 0x6F);
        line = sub_bytes(0xFF, RD8(BANDS->bgy_lo[3]), true);
        return RasterBandThenLine(WORK_BAND, line.value, (Held){ h.x, h.y, line.carry, line.overflow });
    }
    work_x_from(0);
    WR8(BANDS->bgy_lo[WORK_BAND], 0xBF);
    return RasterEndWorkBand(h);
}

void abi_RasterEffect3(void)
{
    exit_regs_shim(RasterEffect3(held_in_shim()));
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
Exit RasterEffect4(Held h)
{
    uint8_t byte;

    work_x_from(0);
    byte = RD8(BANDS->bgy_lo[1]);
    WR8(BANDS->bgy_lo[WORK_BAND], byte);
    byte = RD8(BANDS->bgy_hi[1]);
    WR8(BANDS->bgy_hi[WORK_BAND], byte);
    return RasterEndWorkBand(h);
}

void abi_RasterEffect4(void)
{
    exit_regs_shim(RasterEffect4(held_in_shim()));
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
Exit RasterEffect5(Held h)
{
    uint8_t step = RD8(RASTERST->step);
    Sum high;

    if (step == 0) {
        return RasterBandThenLine(1, 0x5F, h);
    }
    if (step == 1) {
        work_x_from(2);
        high = work_y_from_plus(2, 0x1F);
        return RasterBandThenLine(WORK_BAND, 0x7F, (Held){ h.x, h.y, high.carry, high.overflow });
    }
    if (step == 2) {
        work_x_from(1);
        high = work_y_from_plus(1, 0x3F);
        return RasterBandThenLine(WORK_BAND, 0xFF, (Held){ h.x, h.y, high.carry, high.overflow });
    }
    work_x_from(0);
    high = work_y_from_plus(0, 0xBF);
    return RasterEndWorkBand((Held){ h.x, h.y, high.carry, high.overflow });
}

void abi_RasterEffect5(void)
{
    exit_regs_shim(RasterEffect5(held_in_shim()));
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
Exit RasterEffect6(Held h)
{
    uint8_t step = RD8(RASTERST->step);

    if (step == 0) {
        return RasterBandThenLine(4, 0x67, h);
    }
    if (step == 6) {
        work_x_from(1);
        WR8(BANDS->bgy_lo[WORK_BAND], 0xDB);
        return RasterEndWorkBand((Held){ h.x, h.y, true, h.v });
    }
    return band_table_step(step, read_Effect6Bands, read_Effect6YAdds, read_Effect6Lines, 0x468D, 0x4695);
}

void abi_RasterEffect6(void)
{
    exit_regs_shim(RasterEffect6(held_in_shim()));
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
Exit RasterEffect7(Held h)
{
    uint8_t step = RD8(RASTERST->step);

    if (step == 0) {
        return RasterBandThenLine(4, 0x67, h);
    }
    if (step == 6) {
        work_x_from(1);
        work_y_set(0x1B, 0x01);
        return RasterEndWorkBand((Held){ h.x, h.y, true, h.v });
    }
    return band_table_step_word(step, read_Effect6Bands, read_Effect7YAdds, read_Effect6Lines, 0x4709, 0x4714);
}

void abi_RasterEffect7(void)
{
    exit_regs_shim(RasterEffect7(held_in_shim()));
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
Exit RasterEffect8Step4(Held h)
{
    (void)background_off_at(0x4733);
    return RasterSetNextLine(0xE0, h);
}

void abi_RasterEffect8Step4(void)
{
    exit_regs_shim(RasterEffect8Step4(held_in_shim()));
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
Exit RasterEffect8Step0(Held h)
{
    INC8(BANDS->bgy_hi[5]);
    (void)write_band_at(0x4747, 5);
    DEC8(BANDS->bgy_hi[5]);
    return RasterSetNextLine(0x5F, (Held){ 5, h.y, h.c, h.v });
}

void abi_RasterEffect8Step0(void)
{
    exit_regs_shim(RasterEffect8Step0(held_in_shim()));
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
Exit RasterEffect8Step3(Held h)
{
    work_x_from(0);
    work_y_set(0x6F, 0);
    return RasterBandThenLine(WORK_BAND, 0xDF, h);
}

void abi_RasterEffect8Step3(void)
{
    exit_regs_shim(RasterEffect8Step3(held_in_shim()));
}

/* Colours $101-$10F back from where the step below kept them. Leaves A the last byte read, X 0. */
static Exit restore_saved_colours(Held h)
{
    uint8_t byte = 0;

    select_sprite_palette();
    for (uint8_t colour = PALETTE_SAVED_COLOURS; colour != 0; colour--) {
        byte = RD8(SAVCOL->lo[colour]);
        WR8(VCE->data_lo, byte);
        byte = RD8(SAVCOL->hi[colour]);
        WR8(VCE->data_hi, byte);
    }
    {
        Exit e = exit_of(byte, 0, h.y, h.c, h.v);

        e.nz = P_Z; /* the last DEX, X now 0, set the flags after the last load */
        return finish(e);
    }
}

/*
 * Colours $101-$10F kept in RAM and replaced by one flat colour, sprites to flicker
 * dark, for the rest of the frame's bands; then a compare at line $121 to put them
 * back. The background's enable follows the frame's count, every other frame. Leaves A 6,
 * X the frame count's low bit.
 */
static Exit flatten_colours(Held h)
{
    uint8_t which;
    uint8_t control;

    select_sprite_palette();
    for (uint8_t colour = PALETTE_SAVED_COLOURS; colour != 0; colour--) {
        uint8_t byte = RD8(VCE->data_lo);

        WR8(SAVCOL->lo[colour], byte);
        byte = RD8(VCE->data_hi);
        WR8(SAVCOL->hi[colour], byte);
    }
    select_sprite_palette();
    for (uint8_t colour = PALETTE_SAVED_COLOURS; colour != 0; colour--) {
        WR8(VCE->data_lo, COLOUR_FILL);
        WR8(VCE->data_hi, 0);
    }
    select_control_at(0x4817);
    which = (uint8_t)(RD8(ZP->frame_count) & 1);
    control = RD8(ZP->vdc_cr.lo);
    control = (uint8_t)(control & TBL8(FlickerMasks, which));
    WR8(VDC->data_lo, control);
    next_step();
    select_at(0x482C, VDC_REG_RCR);
    st1(0x21);
    st2(0x01);
    return finish(exit_of(VDC_REG_RCR, which, h.y, h.c, h.v));
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
Exit RasterEffect8(Held h)
{
    uint8_t step = RD8(RASTERST->step);
    Held carried = { h.x, h.y, true, h.v };

    if (step == 0) {
        return RasterEffect8Step0(h);
    }
    if (step < 3) {
        return band_table_step_word(step, read_Effect8Bands, read_Effect8YAdds, read_Effect8Lines, 0x47C5, 0x47D0);
    }
    if (step == 3) {
        return RasterEffect8Step3(carried);
    }
    if (step == 4) {
        return RasterEffect8Step4(carried);
    }
    if (step == 5) {
        return flatten_colours(carried);
    }
    return restore_saved_colours(carried);
}

void abi_RasterEffect8(void)
{
    exit_regs_shim(RasterEffect8(held_in_shim()));
}

/* Effects 9 and 10: two bands of a three-step split, the last at a fixed Y. */
static Exit three_step_split(Held h, uint8_t line0, uint8_t y1, uint8_t line1, uint8_t y2)
{
    uint8_t step = RD8(RASTERST->step);

    if (step == 0) {
        return RasterBandThenLine(0, line0, h);
    }
    if (step == 1) {
        work_x_from(1);
        work_y_set(y1, 0x01);
        return RasterBandThenLine(WORK_BAND, line1, h);
    }
    work_x_from(0);
    work_y_set(y2, 0);
    return RasterEndWorkBand(h);
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
Exit RasterEffect9(Held h)
{
    return three_step_split(h, 0x7F, 0x3F, 0xBF, 0x7F);
}

void abi_RasterEffect9(void)
{
    exit_regs_shim(RasterEffect9(held_in_shim()));
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
Exit RasterEffect10(Held h)
{
    return three_step_split(h, 0x9F, 0x5F, 0xDF, 0x9F);
}

void abi_RasterEffect10(void)
{
    exit_regs_shim(RasterEffect10(held_in_shim()));
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
Exit RasterEffect11(Held h)
{
    uint8_t control = background_off_at(0x48C4);

    return finish(exit_of(control, h.x, h.y, h.c, h.v));
}

void abi_RasterEffect11(void)
{
    exit_regs_shim(RasterEffect11(held_in_shim()));
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
Exit RasterEffect12(Held h)
{
    uint8_t step = RD8(RASTERST->step);

    if (step == 0) {
        return RasterBandThenLine(2, 0x7F, h);
    }
    if (step == 4) {
        work_x_from(5);
        WR8(BANDS->bgy_lo[WORK_BAND], 0xB8);
        return RasterEndWorkBand((Held){ h.x, h.y, true, h.v });
    }
    return band_table_step(step, read_Effect12Bands, read_Effect12YAdds, read_Effect12Lines, 0x4918, 0x4920);
}

void abi_RasterEffect12(void)
{
    exit_regs_shim(RasterEffect12(held_in_shim()));
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
Exit RasterEffect13(Held h)
{
    uint8_t step = RD8(RASTERST->step);

    if (step == 0) {
        INC8(BANDS->bgy_hi[4]);
        (void)write_band_at(0x4972, 4);
        DEC8(BANDS->bgy_hi[4]);
        return RasterSetNextLine(0x5F, (Held){ 4, h.y, h.c, h.v });
    }
    if (step == 5) {
        uint8_t a;

        work_x_from(0);
        WR8(BANDS->bgy_lo[WORK_BAND], 0xCF);
        (void)write_band_at(0x4958, WORK_BAND);
        a = RD8(SCROLL->x_lo[7]);
        if (a != 0) {
            a = background_off_at(0x4960);
        }
        return finish(exit_of(a, WORK_BAND, h.y, true, h.v));
    }
    return band_table_step_word(step, read_Effect13Bands, read_Effect13YAdds, read_Effect13Lines, 0x49A4, 0x49AF);
}

void abi_RasterEffect13(void)
{
    exit_regs_shim(RasterEffect13(held_in_shim()));
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
Exit RasterEffect14(Held h)
{
    uint8_t step = RD8(RASTERST->step);
    uint8_t byte;

    if (step == 0) {
        return RasterBandThenLine(4, 0x6F, h);
    }
    if (step == 2) {
        return flatten_colours((Held){ h.x, h.y, true, h.v });
    }
    if (step == 3) {
        return restore_saved_colours((Held){ h.x, h.y, true, h.v });
    }
    work_x_from(0);
    byte = RD8(BANDS->bgy_hi[0]);
    WR8(BANDS->bgy_hi[WORK_BAND], byte);
    WR8(BANDS->bgy_lo[WORK_BAND], 0x2F);
    write_work_then_select(0x49FA, 0x4A02);
    st1(0x11);
    st2(0x01);
    return finish(exit_of(VDC_REG_RCR, WORK_BAND, h.y, step >= 3, h.v));
}

void abi_RasterEffect14(void)
{
    exit_regs_shim(RasterEffect14(held_in_shim()));
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
Exit RasterEffect15(Held h)
{
    uint8_t step = RD8(RASTERST->step);
    Sum line;

    if (step == 0) {
        line = sub_bytes(0xCF, RD8(SCROLLLIM->raster_water_line), true);
        return RasterBandThenLine(2, line.value, (Held){ h.x, h.y, line.carry, line.overflow });
    }
    if (step >= 0x15) {
        work_x_from(0);
        WR8(BANDS->bgy_hi[WORK_BAND], 0x01);
        WR8(BANDS->bgy_lo[WORK_BAND], 0xB7);
        return RasterEndWorkBand((Held){ h.x, h.y, true, h.v });
    }
    {
        /* The step here is 1 to $14: two lines a step down from band 1, a byte that wraps. */
        uint8_t doubled = (uint8_t)((uint8_t)(RD8(RASTERST->step) - 1) << 1);
        Sum first = add_bytes(doubled, RD8(BANDS->bgy_lo[1]), false);
        Sum low = add_bytes(first.value, 0x8F, false);
        Sum high;
        Sum sum;
        uint8_t wave;
        uint8_t byte;

        WR8(BANDS->bgy_lo[WORK_BAND], low.value);
        high = add_bytes(0, RD8(BANDS->bgy_hi[1]), low.carry);
        WR8(BANDS->bgy_hi[WORK_BAND], high.value);
        wave = wave_index(RD8(ZP->frame_tick), 3);
        work_x_band1_plus_wave(read_WaterWave, wave);
        (void)write_band_at(0x4A7D, WORK_BAND);
        select_at(0x4A82, VDC_REG_RCR);
        byte = (uint8_t)(RD8(RASTERST->step) << 1);
        WR8(BANDS->bgy_lo[WORK_BAND], byte);
        line = sub_bytes(0xCF, RD8(SCROLLLIM->raster_water_line), true);
        sum = add_bytes(line.value, RD8(BANDS->bgy_lo[WORK_BAND]), false);
        WR8(VDC->data_lo, sum.value);
        high = add_bytes(0, 0, sum.carry);
        WR8(VDC->data_hi, high.value);
        {
            Exit e = exit_of(high.value, WORK_BAND, wave, false, sum.overflow);

            e.nz = nz_of(next_step());
            return finish(e);
        }
    }
}

void abi_RasterEffect15(void)
{
    exit_regs_shim(RasterEffect15(held_in_shim()));
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
Exit RasterEffect16(Held h)
{
    uint8_t step = RD8(RASTERST->step);

    if (step == 0) {
        return RasterBandThenLine(5, 0x57, h);
    }
    if (step == 5) {
        work_x_from(1);
        WR8(BANDS->bgy_lo[WORK_BAND], 0xBF);
        return RasterEndWorkBand((Held){ h.x, h.y, true, h.v });
    }
    return band_table_step(step, read_Effect16Bands, read_Effect16YAdds, read_Effect16Lines, 0x4AFB, 0x4B03);
}

void abi_RasterEffect16(void)
{
    exit_regs_shim(RasterEffect16(held_in_shim()));
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
Exit RasterEffect17(Held h)
{
    uint8_t step = RD8(RASTERST->step);

    if (step == 0) {
        return RasterBandThenLine(4, 0x67, h);
    }
    if (step == 5) {
        work_x_from(2);
        WR8(BANDS->bgy_lo[WORK_BAND], 0xD7);
        return RasterEndWorkBand((Held){ h.x, h.y, true, h.v });
    }
    return band_table_step(step, read_Effect17Bands, read_Effect17YAdds, read_Effect17Lines, 0x4B6B, 0x4B73);
}

void abi_RasterEffect17(void)
{
    exit_regs_shim(RasterEffect17(held_in_shim()));
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
Exit RasterEffect18(Held h)
{
    uint8_t step = RD8(RASTERST->step);

    if (step == 0) {
        Sum line;

        effect18_work_band();
        write_work_then_select(0x4BC9, 0x4BD1);
        line = line_byte_tail(RD8(BANDS->bgy_lo[2]));
        return finish(exit_of(line.value, WORK_BAND, h.y, line.carry, line.overflow));
    }
    if (step == 1) {
        uint8_t byte = RD8(BANDS->bgx_lo[0]);
        Sum low;
        Sum high;
        Sum line;

        WR8(BANDS->bgx_lo[1], byte);
        byte = RD8(BANDS->bgx_hi[0]);
        WR8(BANDS->bgx_hi[1], byte);
        byte = RD8(BANDS->bgy_lo[0]);
        low = add_bytes(byte, RD8(BANDS->bgy_lo[2]), false);
        WR8(BANDS->bgy_lo[1], low.value);
        high = add_bytes(0, 0, low.carry);
        WR8(BANDS->bgy_hi[1], high.value);
        (void)write_band_at(0x4BF8, 1);
        next_step();
        select_at(0x4C00, VDC_REG_RCR);
        line = sub_bytes(0xE0, RD8(BANDS->bgy_lo[2]), true);
        high = line_word_of(line.value);
        return finish(exit_of(high.value, 1, h.y, high.carry, high.overflow));
    }
    effect18_work_band();
    return RasterEndWorkBand(h);
}

void abi_RasterEffect18(void)
{
    exit_regs_shim(RasterEffect18(held_in_shim()));
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
Exit RasterEffect19(Held h)
{
    uint8_t step = RD8(RASTERST->step);
    Sum line;

    if (step == 0) {
        (void)write_band_at(0x4C38, 0);
        (void)background_off_at(0x4C3B);
        next_step();
        select_at(0x4C4A, VDC_REG_RCR);
        line = line_byte_tail(RD8(BANDS->bgy_lo[5]));
        return finish(exit_of(line.value, 0, h.y, line.carry, line.overflow));
    }
    if (step == 1) {
        (void)write_band_at(0x4C55, 1);
        (void)background_on_at(0x4C58);
        next_step();
        select_at(0x4C67, VDC_REG_RCR);
        line = line_word_of(RD8(BANDS->bgy_lo[6]));
        return finish(exit_of(line.value, 1, h.y, line.carry, line.overflow));
    }
    {
        uint8_t a;

        if (RD8(BANDS->bgy_hi[6]) == 0) {
            (void)background_off_at(0x4C25);
        }
        a = write_band_at(0x4C31, 2);
        return finish(exit_of(a, 2, h.y, h.c, h.v));
    }
}

void abi_RasterEffect19(void)
{
    exit_regs_shim(RasterEffect19(held_in_shim()));
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
Exit RasterEffect20(Held h)
{
    uint8_t step = RD8(RASTERST->step);
    uint8_t line;
    Sum sum;

    if (step == 0) {
        (void)write_band_at(0x4C9E, 3);
        next_step();
        select_at(0x4CA6, VDC_REG_RCR);
        line = RD8(BANDS->bgy_lo[5]);
        if (line >= 0x60) {
            line = 0x5F;
        } else {
            next_step();
        }
        sum = line_byte_tail(line);
        return finish(exit_of(sum.value, 3, h.y, sum.carry, sum.overflow));
    }
    if (step == 1) {
        (void)write_band_at(0x4CBB, 0);
        line = RD8(BANDS->bgy_lo[5]);
        if (line >= 0x77) {
            WR8(RASTERST->step, 4);
            select_at(0x4CDA, VDC_REG_RCR);
            sum = line_byte_tail(0xA7);
        } else {
            next_step();
            select_at(0x4CCA, VDC_REG_RCR);
            sum = line_byte_tail(RD8(BANDS->bgy_lo[5]));
        }
        return finish(exit_of(sum.value, 0, h.y, sum.carry, sum.overflow));
    }
    if (step == 2) {
        (void)write_band_at(0x4D07, 1);
        (void)background_off_at(0x4D0A);
        next_step();
        select_at(0x4D19, VDC_REG_RCR);
        line = RD8(BANDS->bgy_lo[6]);
        if (line == 0xBF) {
            Exit equal = exit_of(0xBF, 1, h.y, true, h.v);

            equal.nz = P_Z;
            return finish(equal);
        }
        if (line >= 0xA7) {
            next_step();
        }
        sum = line_byte_tail(line);
        return finish(exit_of(sum.value, 1, h.y, sum.carry, sum.overflow));
    }
    if (step == 3) {
        (void)write_band_at(0x4CE4, 6);
        (void)background_on_at(0x4CE7);
        next_step();
        select_at(0x4CF6, VDC_REG_RCR);
        line = RD8(BANDS->bgy_lo[6]);
        sum = line_byte_tail(line >= 0xA8 ? line : 0xA7);
        return finish(exit_of(sum.value, 6, h.y, sum.carry, sum.overflow));
    }
    {
        uint8_t a;

        (void)background_on_at(0x4C8A);
        a = write_band_at(0x4C96, 4);
        return finish(exit_of(a, 4, h.y, h.c, h.v));
    }
}

void abi_RasterEffect20(void)
{
    exit_regs_shim(RasterEffect20(held_in_shim()));
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
Exit RasterEffect21(Held h)
{
    uint8_t step = RD8(RASTERST->step);

    if (step == 0) {
        return RasterBandThenLine(2, 0x7F, h);
    }
    if (step == 1) {
        return RasterBandThenLine(1, 0x87, h);
    }
    if (step == 2) {
        return RasterBandThenLine(3, 0xFF, h);
    }
    if (step == 3) {
        work_x_from(0);
        work_y_set(0xBF, 0);
        write_work_then_select(0x4D95, 0x4D9D);
        st1(0x07);
        st2(0x01);
        return finish(exit_of(VDC_REG_RCR, WORK_BAND, h.y, h.c, h.v));
    }
    {
        uint8_t stage = RD8(ZP->raster_stage);

        if (stage == 3) {
            (void)background_off_at(0x4D49);
        }
        work_x_from(0);
        work_y_set(0xC7, 0);
        return RasterEndWorkBand((Held){ h.x, h.y, stage >= 3, h.v });
    }
}

void abi_RasterEffect21(void)
{
    exit_regs_shim(RasterEffect21(held_in_shim()));
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
Exit RasterEffect22(Held h)
{
    if (RD8(RASTERST->step) == 0) {
        work_y_set(0x10, 0x01);
        work_x_from(2);
        return RasterBandThenLine(WORK_BAND, 0x67, h);
    }
    {
        Sum low;

        work_x_from(0);
        low = add_bytes(0x27, RD8(BANDS->bgy_lo[0]), false);
        WR8(BANDS->bgy_lo[WORK_BAND], low.value);
        WR8(BANDS->bgy_hi[WORK_BAND], 0);
        return RasterEndWorkBand((Held){ h.x, h.y, low.carry, low.overflow });
    }
}

void abi_RasterEffect22(void)
{
    exit_regs_shim(RasterEffect22(held_in_shim()));
}

/* Effects 23 and 29: flatten the colours and restore them, after a first line. */
static Exit flatten_after_line(Held h, uint8_t first_line, uint16_t select_ret)
{
    uint8_t step = RD8(RASTERST->step);

    if (step == 0) {
        next_step();
        select_at(select_ret, VDC_REG_RCR);
        st1(first_line);
        st2(0x01);
        return finish(exit_of(VDC_REG_RCR, h.x, h.y, h.c, h.v));
    }
    if (step == 1) {
        return flatten_colours(h);
    }
    return restore_saved_colours(h);
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
Exit RasterEffect23(Held h)
{
    return flatten_after_line(h, 0x01, 0x4DF7);
}

void abi_RasterEffect23(void)
{
    exit_regs_shim(RasterEffect23(held_in_shim()));
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
Exit RasterEffect25(Held h)
{
    if (RD8(RASTERST->step) == 0) {
        return flatten_colours(h);
    }
    return restore_saved_colours(h);
}

void abi_RasterEffect25(void)
{
    exit_regs_shim(RasterEffect25(held_in_shim()));
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
Exit RasterEffect29(Held h)
{
    return flatten_after_line(h, 0x18, 0x4E1F);
}

void abi_RasterEffect29(void)
{
    exit_regs_shim(RasterEffect29(held_in_shim()));
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
Exit RasterEffect24(Held h)
{
    uint8_t step = RD8(RASTERST->step);

    if (step == 0) {
        WR8(BANDS->bgx_lo[2], 0xFF);
        WR8(BANDS->bgx_hi[2], 0);
        return RasterBandThenLine(0, 0xFF, h);
    }
    if (step >= 0x11) {
        return RasterEndBand1((Held){ h.x, h.y, true, h.v });
    }
    {
        /* The step here is 1 to $10, so doubling it one less carries nothing. */
        uint8_t doubled = (uint8_t)((uint8_t)(RD8(RASTERST->step) - 1) << 1);
        uint8_t wave;
        uint8_t beat;
        uint8_t counter;
        Sum low;
        Sum high;

        WR8(BANDS->bgy_lo[WORK_BAND], add_bytes(doubled, RD8(BANDS->bgy_lo[1]), false).value);
        WR8(BANDS->bgy_hi[WORK_BAND], 0);
        counter = RD8(ZP->wave_counter);
        wave = wave_index(counter, 3);
        work_x_band1_plus_wave(read_DeepWave, wave);
        (void)write_band_at(0x4E65, WORK_BAND);
        select_at(0x4E6A, VDC_REG_RCR);
        counter = RD8(ZP->wave_counter);
        beat = (uint8_t)((uint8_t)(add_bytes((uint8_t)(counter >> 3), RD8(RASTERST->step), false).value >> 3) & 1);
        low = add_bytes(add_bytes(beat, 2, false).value, RD8(BANDS->bgx_lo[2]), false);
        WR8(BANDS->bgx_lo[2], low.value);
        WR8(VDC->data_lo, low.value);
        high = add_bytes(0, RD8(BANDS->bgx_hi[2]), low.carry);
        WR8(BANDS->bgx_hi[2], high.value);
        WR8(VDC->data_hi, high.value);
        {
            Exit e = exit_of(high.value, WORK_BAND, wave, high.carry, high.overflow);

            e.nz = nz_of(next_step());
            return finish(e);
        }
    }
}

void abi_RasterEffect24(void)
{
    exit_regs_shim(RasterEffect24(held_in_shim()));
}

/*
 * Effects 26 and 27: a table of bands in RAM, each marked: bit 7 clear stands still, set
 * follows the deep wave. Each step is a line or eight apart.
 */
static Exit marked_band_step(Held h, uint8_t limit, uint8_t shift, uint16_t write_ret, uint16_t select_ret)
{
    uint8_t step = RD8(RASTERST->step);
    uint8_t mark;
    uint8_t byte;
    Sum low;
    Sum high;

    if (step == 0) {
        return RasterBandThenLine(0, 0x41, h);
    }
    if (step >= limit) {
        return RasterEndBand1((Held){ h.x, h.y, true, h.v });
    }
    step = RD8(RASTERST->step);
    byte = (uint8_t)(((uint8_t)(step - 1) << shift) + 1);
    WR8(BANDS->bgy_lo[WORK_BAND], byte);
    mark = RD8(RMARKS->mark[step]);
    if (!(mark & 0x80)) {
        WR8(BANDS->bgx_lo[WORK_BAND], 0);
        WR8(BANDS->bgy_hi[WORK_BAND], 0);
        WR8(BANDS->bgx_hi[WORK_BAND], 0x01);
    } else {
        uint8_t index;
        uint8_t counter;

        WR8(BANDS->bgy_hi[WORK_BAND], 0x01);
        counter = RD8(ZP->wave_counter);
        index = (uint8_t)((add_bytes((uint8_t)(counter >> 2), RD8(RASTERST->step), false).value & 7) << 1);
        byte = TBL8(DeepWave, index);
        WR8(BANDS->bgx_lo[WORK_BAND], byte);
        byte = TBL8(DeepWave, index + 1);
        WR8(BANDS->bgx_hi[WORK_BAND], byte);
    }
    mark = (uint8_t)(RD8(RMARKS->mark[step]) & 0x7F);
    WR8(BANDS->bgx_lo[WORK_BAND], add_bytes(mark, RD8(BANDS->bgx_lo[WORK_BAND]), false).value);
    (void)write_band_at(write_ret, WORK_BAND);
    select_at(select_ret, VDC_REG_RCR);
    low = add_bytes((uint8_t)(RD8(RASTERST->step) << shift), 0x41, false);
    WR8(VDC->data_lo, low.value);
    high = add_bytes(0, 0, low.carry);
    WR8(VDC->data_hi, high.value);
    {
        Exit e = exit_of(high.value, WORK_BAND, step, false, low.overflow);

        e.nz = nz_of(next_step());
        return finish(e);
    }
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
Exit RasterEffect26(Held h)
{
    return marked_band_step(h, 0x71, 1, 0x4F06, 0x4F0B);
}

void abi_RasterEffect26(void)
{
    exit_regs_shim(RasterEffect26(held_in_shim()));
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
Exit RasterEffect27(Held h)
{
    return marked_band_step(h, 0x1D, 3, 0x4F7E, 0x4F83);
}

void abi_RasterEffect27(void)
{
    exit_regs_shim(RasterEffect27(held_in_shim()));
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
Exit RasterEffect28(Held h)
{
    uint8_t step = RD8(RASTERST->step);
    uint8_t byte;
    Sum low;
    Sum high;

    if (step == 0) {
        return RasterBandThenLine(3, 0xA7, h);
    }
    if (step >= 0x0C) {
        work_x_from(0);
        work_y_set(0x7F, 0);
        return RasterEndWorkBand((Held){ h.x, h.y, true, h.v });
    }
    byte = add_bytes((uint8_t)(RD8(RASTERST->step) << 1), 0x67, false).value;
    WR8(BANDS->bgy_lo[WORK_BAND], byte);
    if (RD8(RASTERST->step) == 1) {
        DEC8(BANDS->bgy_lo[WORK_BAND]);
        DEC8(BANDS->bgy_lo[WORK_BAND]);
    }
    WR8(BANDS->bgy_hi[WORK_BAND], 0);
    byte = RD8(BANDS->bgx_lo[3]);
    WR8(BANDS->bgx_lo[WORK_BAND], byte);
    byte = RD8(BANDS->bgx_hi[3]);
    WR8(BANDS->bgx_hi[WORK_BAND], byte);
    byte = (uint8_t)(RD8(ZP->frame_tick) >> 3);
    byte = (uint8_t)(add_bytes(byte, RD8(RASTERST->step), false).value >> 1);
    if ((byte & 0x0F) == 0) {
        uint8_t random = (uint8_t)(RD8(ZP->random3) & 1);

        low = add_bytes(add_bytes(random, 1, false).value, RD8(BANDS->bgx_lo[WORK_BAND]), false);
        WR8(BANDS->bgx_lo[WORK_BAND], low.value);
        high = add_bytes(0, RD8(BANDS->bgx_hi[WORK_BAND]), low.carry);
        WR8(BANDS->bgx_hi[WORK_BAND], high.value);
    }
    (void)write_band_at(0x5013, WORK_BAND);
    select_at(0x5018, VDC_REG_RCR);
    low = add_bytes((uint8_t)(RD8(RASTERST->step) << 1), 0xA9, false);
    WR8(VDC->data_lo, low.value);
    high = add_bytes(0, 0, low.carry);
    WR8(VDC->data_hi, high.value);
    {
        Exit e = exit_of(high.value, WORK_BAND, h.y, false, low.overflow);

        e.nz = nz_of(next_step());
        return finish(e);
    }
}

void abi_RasterEffect28(void)
{
    exit_regs_shim(RasterEffect28(held_in_shim()));
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
Exit RasterEffect30(Held h)
{
    uint8_t step = RD8(RASTERST->step);

    if (step == 0) {
        return RasterBandThenLine(1, 0x8F, h);
    }
    if (step == 4) {
        work_x_from(0);
        work_y_set(0xAF, 0);
        return RasterEndWorkBand((Held){ h.x, h.y, true, h.v });
    }
    return band_table_step(step, read_Effect30Bands, read_Effect30YAdds, read_Effect30Lines, 0x507A, 0x5082);
}

void abi_RasterEffect30(void)
{
    exit_regs_shim(RasterEffect30(held_in_shim()));
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
Exit RasterEffect31(Held h)
{
    uint8_t step = RD8(RASTERST->step);

    if (step == 0) {
        (void)background_off_at(0x50BE);
        next_step();
        select_at(0x50CD, VDC_REG_RCR);
        WR8(VDC->data_lo, 0x57);
        st2(0);
        return finish(exit_of(0x57, h.x, h.y, h.c, h.v));
    }
    if (step == 2) {
        work_x_from(0);
        work_y_set(0xA7, 0);
        return RasterEndWorkBand((Held){ h.x, h.y, true, h.v });
    }
    (void)background_on_at(0x50DA);
    WR8(BANDS->bgy_lo[1], 0x17);
    return RasterBandThenLine(1, 0xE7, (Held){ h.x, h.y, step >= 2, h.v });
}

void abi_RasterEffect31(void)
{
    exit_regs_shim(RasterEffect31(held_in_shim()));
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
/* The handler is entered with A and X the effect doubled and the carry the effect's top bit, N and Z from A. */
static void dispatch_registers_shim(uint8_t effect, uint8_t index)
{
    cpu.a = index;
    cpu.x = index;
    cpu.p = (uint8_t)((cpu.p & ~(P_N | P_Z)) | nz_of(index));
    abi_flag(P_C, (effect & 0x80) != 0);
}

void RasterInterrupt(void)
{
    static void (*const effects[])(void) = {
        abi_RasterDone, abi_RasterEndBand1, abi_RasterEffect2, abi_RasterEffect3,
        abi_RasterEffect4, abi_RasterEffect5, abi_RasterEffect6, abi_RasterEffect7,
        abi_RasterEffect8, abi_RasterEffect9, abi_RasterEffect10, abi_RasterEffect11,
        abi_RasterEffect12, abi_RasterEffect13, abi_RasterEffect14, abi_RasterEffect15,
        abi_RasterEffect16, abi_RasterEffect17, abi_RasterEffect18, abi_RasterEffect19,
        abi_RasterEffect20, abi_RasterEffect21, abi_RasterEffect22, abi_RasterEffect23,
        abi_RasterEffect24, abi_RasterEffect25, abi_RasterEffect26, abi_RasterEffect27,
        abi_RasterEffect28, abi_RasterEffect29, abi_RasterEffect30, abi_RasterEffect31
    };
    uint8_t effect = RD8(RASTERST->effect_now);
    uint8_t index = (uint8_t)(effect << 1);

    (void)TBL8(RasterEffects, index);
    (void)TBL8(RasterEffects, index + 1);
    dispatch_registers_shim(effect, index);
    effects[effect]();
}

void abi_RasterInterrupt(void)
{
    RasterInterrupt();
}

/* The band tables' columns copied at the frame's start, from the entry's row down: each row a byte of the game's scroll to the frame's. */
#define BAND_COPY_ROWS(X) \
    X(SCROLL->x_lo[7], BANDS->bgx_lo[7]) \
    X(SCROLL->x_lo[6], BANDS->bgx_lo[6]) \
    X(SCROLL->x_hi[6], BANDS->bgx_hi[6]) \
    X(SCROLL->y_lo[6], BANDS->bgy_lo[6]) \
    X(SCROLL->y_hi[6], BANDS->bgy_hi[6]) \
    X(SCROLL->x_lo[5], BANDS->bgx_lo[5]) \
    X(SCROLL->x_hi[5], BANDS->bgx_hi[5]) \
    X(SCROLL->y_lo[5], BANDS->bgy_lo[5]) \
    X(SCROLL->y_hi[5], BANDS->bgy_hi[5]) \
    X(SCROLL->x_lo[4], BANDS->bgx_lo[4]) \
    X(SCROLL->x_hi[4], BANDS->bgx_hi[4]) \
    X(SCROLL->y_lo[4], BANDS->bgy_lo[4]) \
    X(SCROLL->y_hi[4], BANDS->bgy_hi[4]) \
    X(SCROLL->x_lo[3], BANDS->bgx_lo[3]) \
    X(SCROLL->x_hi[3], BANDS->bgx_hi[3]) \
    X(SCROLL->y_lo[3], BANDS->bgy_lo[3]) \
    X(SCROLL->y_hi[3], BANDS->bgy_hi[3]) \
    X(LEVELW->y.lo, SCROLLLIM->raster_water_line) \
    X(SCROLL->x_lo[2], BANDS->bgx_lo[2]) \
    X(SCROLL->x_hi[2], BANDS->bgx_hi[2]) \
    X(SCROLL->y_lo[2], BANDS->bgy_lo[2]) \
    X(SCROLL->y_hi[2], BANDS->bgy_hi[2]) \
    X(SCROLL->x_lo[1], BANDS->bgx_lo[1]) \
    X(SCROLL->x_hi[1], BANDS->bgx_hi[1]) \
    X(SCROLL->y_lo[1], BANDS->bgy_lo[1]) \
    X(SCROLL->y_hi[1], BANDS->bgy_hi[1])
enum {
#define COUNT_ROW(from, to) +1
    BAND_COPIES = 0 BAND_COPY_ROWS(COUNT_ROW)
#undef COUNT_ROW
};

/* Returns whether anything was copied, and the last byte if so. */
static bool copy_band_columns(unsigned first, uint8_t *last)
{
    unsigned row = 0;

#define COPY_ROW(from, to) \
    if (row++ >= first) { \
        uint8_t byte = RD8(from); \
        WR8(to, byte); \
        *last = byte; \
    }
    BAND_COPY_ROWS(COPY_ROW);
#undef COPY_ROW
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
    uint8_t effect = RD8(RASTERST->effect_now);
    uint8_t index = (uint8_t)(effect << 1);

    (void)TBL8(RasterStartEntries, index);
    (void)TBL8(RasterStartEntries, index + 1);
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
