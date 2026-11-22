/*
 * The BIOS's video setters: the VDC's registers through its address port, the memory width, the display
 * size, the sprite table, the palettes.
 *
 * A routine's registers and flags at its exit are a CardExit (syscard.h). The VDC's register select is
 * written to the zero page copy and to the port, each time. Bytes named by address, not yet by field: the
 * sprite buffer, the palette sets' counts and the colour read stub in ROM.
 */
#include "card.h"
#include "card_access.h"
#include "syscard.h"

enum {
    VDC_REG_MAWR = 0x00, VDC_REG_VWR = 0x02, VDC_REG_CR = 0x05, VDC_REG_RCR = 0x06, VDC_REG_BXR = 0x07,
    VDC_REG_MWR = 0x09, VDC_REG_HSR = 0x0A, VDC_REG_HDR = 0x0B, VDC_REG_VPR = 0x0C, VDC_REG_VDW = 0x0D,
    VDC_REG_VCR = 0x0E, VDC_REG_DCR = 0x0F, VDC_REG_SOUR = 0x10, VDC_REG_DESR = 0x11, VDC_REG_LENR = 0x12,
    VDC_REG_SATB = 0x13,
    MWR_SIZE_BITS = 0x70, MWR_DOT_BITS = 0x8F, SCREEN_SIZE_MASK = 0x07, SIZE_SHIFT = 4,
    CR_BIT_RC = 2, CR_BIT_VBLANK = 3, CR_BIT_SPRITES = 6, CR_BIT_BACKGROUND = 7, CRHI_BIT_3 = 3, CRHI_BIT_4 = 4,
    IRQ_FLAG_FRAME = 1, SAT_SPRITES = 0x40, SPRITE_BYTES = 8, SAT_ENTRY_SHIFT = 2, DCR_MASK = 0x1F,
    SCRMOD_FALLBACK_COLUMNS = 0x20, SCRMOD_FALLBACK_ROWS = 0x1E, ROWS_MIN = 0x0A, ROWS_LIMIT = 0x1F,
    COLUMNS_LOW_MIN = 0x0A, COLUMNS_LOW_LIMIT = 0x23, COLUMNS_HIGH_MIN = 0x10, COLUMNS_HIGH_LIMIT = 0x2D,
    MODE_KEEP_BITS = 0x05, MODE_HIGH_BIT = 0x01, MODE_VCE_FLIP = 0x04
};

#define STUB(i) ((uint16_t)(CardBlockStub + (i)))

/* ex_scrmod and its siblings select a VDC register by writing it to the zero page copy and to the port. */
static void select_vdc_register(uint8_t reg)
{
    WR8(CZP->vdc_reg, reg);
    WR8(CVDC->addr, reg);
}

/* RMB and SMB on a zero page byte: one read, one write. */
static void clear_zp_bit(uint16_t at, unsigned bit)
{
    uint8_t value = rd8(at);

    wr8(at, (uint8_t)(value & ~(1u << bit)));
}

static void set_zp_bit(uint16_t at, unsigned bit)
{
    uint8_t value = rd8(at);

    wr8(at, (uint8_t)(value | (1u << bit)));
}

static CardExit exit_a(uint8_t a)
{
    CardExit e = { 0 };

    e.a = e.nz = a;
    e.set_a = e.set_nz = true;
    return e;
}

/* The memory width register's bits kept, and its new ones: the register, then its copy at the port. */
static CardExit set_memory_width(uint8_t keep, uint8_t bits)
{
    CardExit e;
    uint8_t value;
    uint8_t width;

    value = RD8(CRAM->vdc_mwr);
    WR8(CRAM->vdc_mwr, (uint8_t)(value & keep));
    value = RD8(CRAM->vdc_mwr);
    WR8(CRAM->vdc_mwr, (uint8_t)(bits | value));
    select_vdc_register(VDC_REG_MWR);
    width = RD8(CRAM->vdc_mwr);
    WR8(CVDC->data_lo, width);
    e = exit_a(width);
    e.y = bits;
    e.set_y = true;
    return e;
}

/*
 * @card 00:E267 ex_scrsiz
 * WHAT IT DOES: Sets the size of the background map.
 * HOW: The size's three bits go to bits 4-6 of the memory width register.
 * WAITS: no.
 * Entry: A size. Exit: A the register, N and Z from it, Y the bits, C clear. X and V kept.
 */
CardExit ex_scrsiz(uint8_t size)
{
    CardExit e = set_memory_width(MWR_DOT_BITS, (uint8_t)((size & SCREEN_SIZE_MASK) << SIZE_SHIFT));

    e.c = false;
    e.set_c = true;
    return e;
}

/* ex_scrsiz's exit, for its wrapper and for a caller's that ends in it. */
void ex_scrsiz_exit_shim(CardExit e)
{
    card_exit_apply(e);
}

void abi_ex_scrsiz(void)
{
    ex_scrsiz_exit_shim(ex_scrsiz(cpu.a));
}

/*
 * @card 00:E272 ex_dotmod
 * WHAT IT DOES: Sets the VRAM dot width and sprite width of the memory width register.
 * HOW: The mode's bits outside the map size go to the register.
 * WAITS: no.
 * Entry: A mode. Exit: A the register, N and Z from it, Y the bits. X, C, V kept.
 */
CardExit ex_dotmod(uint8_t mode)
{
    return set_memory_width(MWR_SIZE_BITS, (uint8_t)(mode & MWR_DOT_BITS));
}

void abi_ex_dotmod(void)
{
    card_exit_apply(ex_dotmod(cpu.a));
}

/*
 * @card 00:E341 set_screen_rows
 * WHAT IT DOES: Sets the vertical display: the rows of tiles in CardScreenRows times eight lines,
 * and the colour clock from CardScreenMode.
 * HOW: The VDC's vertical display start, height and end registers; the end at $7E plus half the lines,
 * the carry of the halving being the one; then the mode with bit 2 flipped goes to the VCE.
 * WAITS: no.
 * Entry: CardScreenRows, CardScreenMode. Exit: A what went to the VCE, N and Z from it, X the
 * lines less one, C and V from adding the end's base. Y kept.
 */
CardExit set_screen_rows(void)
{
    CardExit e = { 0 };
    Sum start;
    Sum end;
    uint8_t rows;
    uint8_t lines;
    uint8_t last;
    uint8_t mode;
    bool shifted;

    select_vdc_register(VDC_REG_VPR);
    st1(2);
    rows = RD8(CRAM->screen_rows);
    lines = (uint8_t)(rows << 3);
    last = (uint8_t)(lines - 1);
    start = add8((uint8_t)(~(lines >> 1)), 0x88, false);
    WR8(CVDC->data_hi, start.value);
    select_vdc_register(VDC_REG_VDW);
    WR8(CVDC->data_lo, last);
    st2(0);
    select_vdc_register(VDC_REG_VCR);
    shifted = (uint8_t)(last + 2) & 1;
    end = add8((uint8_t)((uint8_t)(last + 2) >> 1), 0x7D, shifted);
    WR8(CVDC->data_lo, end.value);
    st2(0);
    mode = RD8(CRAM->screen_mode);
    WR8(CVCE->control, (uint8_t)(mode ^ MODE_VCE_FLIP));
    e.a = e.nz = (uint8_t)(mode ^ MODE_VCE_FLIP);
    e.x = last;
    e.c = end.carry;
    e.v = end.overflow;
    e.set_a = e.set_nz = e.set_x = e.set_c = e.set_v = true;
    return e;
}

void abi_set_screen_rows(void)
{
    card_exit_apply(set_screen_rows());
}

/*
 * @card 00:E29D ex_scrmod
 * WHAT IT DOES: Sets the screen mode: bit 0 of A the wide dot clock, bit 2 the colour clock, X the
 * columns of tiles, Y the rows.
 * HOW: Refuses a size the VDC cannot show (rows under 10 or over 30, columns by the clock) by setting the
 * fallback mode instead; sets the vertical display, then the horizontal sync, start, width and end, and the
 * memory width register's dot width.
 * WAITS: no.
 * Entry: A, X, Y. Exit, set: C clear, A the memory width register, N and Z from it, X the
 * columns less one, Y the dot bits, V from adding the end's base. Refused: as ex_scrmod_fallback.
 */
CardExit ex_scrmod(uint8_t mode, uint8_t columns, uint8_t rows)
{
    CardExit e;
    uint8_t value;
    uint8_t first_columns;
    Sum start;
    Sum width;
    Sum end;
    bool high_clock;

    WR8(CRAM->screen_mode, mode);
    WR8(CRAM->screen_mode, (uint8_t)(mode & MODE_KEEP_BITS));
    WR8(CRAM->screen_columns, columns);
    WR8(CRAM->screen_rows, rows);
    value = RD8(CRAM->screen_rows);
    if (value < ROWS_MIN || value >= ROWS_LIMIT) {
        return ex_scrmod_fallback();
    }
    value = RD8(CRAM->screen_mode);
    high_clock = (value & MODE_HIGH_BIT) != 0;
    value = RD8(CRAM->screen_columns);
    if (value < (high_clock ? COLUMNS_HIGH_MIN : COLUMNS_LOW_MIN) ||
        value >= (high_clock ? COLUMNS_HIGH_LIMIT : COLUMNS_LOW_LIMIT)) {
        return ex_scrmod_fallback();
    }
    jsr_push(high_clock ? 0xE30B : 0xE2C9);
    (void)set_screen_rows();
    jsr_pop();
    select_vdc_register(VDC_REG_HSR);
    st1(high_clock ? 3 : 2);
    first_columns = RD8(CRAM->screen_columns);
    start = add8((uint8_t)~(first_columns >> 1), high_clock ? 0x19 : 0x13, false);
    WR8(CVDC->data_hi, start.value);
    value = RD8(CRAM->screen_columns);
    width = add8(start.value, value, false);
    select_vdc_register(VDC_REG_HDR);
    WR8(CVDC->data_lo, (uint8_t)(first_columns - 1));
    end = add8((uint8_t)~width.value, high_clock ? 0x35 : 0x27, false);
    WR8(CVDC->data_hi, end.value);
    jsr_push(high_clock ? 0xE33F : 0xE2FC);
    e = ex_dotmod(high_clock ? 0x0A : 0);
    jsr_pop();
    e.x = (uint8_t)(first_columns - 1);
    e.c = false;
    e.v = end.overflow;
    e.set_x = e.set_c = e.set_v = true;
    return e;
}

void abi_ex_scrmod(void)
{
    card_exit_apply(ex_scrmod(cpu.a, cpu.x, cpu.y));
}

/*
 * @card 00:E292 ex_scrmod_fallback
 * WHAT IT DOES: The screen mode ex_scrmod sets when it is given one it cannot: 32 columns, 30 rows.
 * HOW: ex_scrmod with A 0, X $20, Y $1E.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, N 0, Z 0, C set, X $1F, Y 0, V as ex_scrmod leaves it.
 */
CardExit ex_scrmod_fallback(void)
{
    CardExit e;

    jsr_push(0xE299);
    e = ex_scrmod(0, SCRMOD_FALLBACK_COLUMNS, SCRMOD_FALLBACK_ROWS);
    jsr_pop();
    e.a = 1;
    e.nz = 1;
    e.c = true;
    e.set_a = e.set_nz = e.set_c = true;
    return e;
}

void abi_ex_scrmod_fallback(void)
{
    card_exit_apply(ex_scrmod_fallback());
}

/*
 * @card 00:E5DA ex_satclr
 * WHAT IT DOES: Clears the sprite attribute table in VRAM: 256 words of zero from CardSatAddress, and
 * CardSatFlag.
 * HOW: The write address to the table's, then 256 words through the data port.
 * WAITS: no.
 * Entry: CardSatAddress. Exit: A 2, X 0, N 0, Z 1. Y, C, V kept.
 */
CardExit ex_satclr(void)
{
    CardExit e = { 0 };
    uint8_t value;

    select_vdc_register(VDC_REG_MAWR);
    value = RD8(CRAM->sat_address);
    WR8(CVDC->data_lo, value);
    value = rd8((uint16_t)(CardSatAddress + 1));
    WR8(CVDC->data_hi, value);
    select_vdc_register(VDC_REG_VWR);
    for (unsigned i = 0; i < 256; i++) {
        st1(0);
        st2(0);
    }
    WR8(CRAM->sat_flag, 0);
    e.a = VDC_REG_VWR;
    e.x = 0;
    e.nz = 0;
    e.set_a = e.set_x = e.set_nz = true;
    return e;
}

void abi_ex_satclr(void)
{
    card_exit_apply(ex_satclr());
}

/*
 * @card 00:E63C ex_sprput
 * WHAT IT DOES: Adds the sprite in CardSprite to the sprite attribute table, if it holds fewer than 64.
 * HOW: Writes its eight bytes at CardSatAddress plus four times the count, and counts it.
 * WAITS: no.
 * Entry: CardSprite, Y (pushed). Exit, full: A the count, N and Z from its compare with 64, C set. Put: A the
 * last byte, N and Z from Y, C set. X, V kept.
 * Not run by any script.
 */
CardExit ex_sprput(uint8_t y)
{
    CardExit e = { 0 };
    uint8_t count = RD8(CRAM->sat_flag);
    uint8_t value;
    Sum low;
    Sum high;
    unsigned i;

    e.a = count;
    e.nz = (uint8_t)(count - SAT_SPRITES);
    e.c = count >= SAT_SPRITES;
    e.set_a = e.set_nz = e.set_c = true;
    if (count >= SAT_SPRITES) {
        return e;
    }
    push8(y);
    select_vdc_register(VDC_REG_MAWR);
    count = RD8(CRAM->sat_flag);
    value = RD8(CRAM->sat_address);
    low = add8((uint8_t)(count << SAT_ENTRY_SHIFT), value, false);
    WR8(CVDC->data_lo, low.value);
    value = rd8((uint16_t)(CardSatAddress + 1));
    high = add8(value, 0, low.carry);
    WR8(CVDC->data_hi, high.value);
    select_vdc_register(VDC_REG_VWR);
    for (i = 0; i < SPRITE_BYTES; i += 2) {
        value = rd8((uint16_t)(CardSprite + i));
        WR8(CVDC->data_lo, value);
        value = rd8((uint16_t)(CardSprite + i + 1));
        WR8(CVDC->data_hi, value);
    }
    (void)INC8(CRAM->sat_flag);
    y = pull8();
    e.a = value;
    e.nz = y;
    e.c = true;
    return e;
}

void abi_ex_sprput(void)
{
    card_exit_apply(ex_sprput(cpu.y));
}

/* The VDC control register's low or high byte put out through the port, after the bits changed. */
static CardExit control_out(uint16_t select_reg, bool high)
{
    uint8_t value;

    select_vdc_register((uint8_t)select_reg);
    if (high) {
        value = RD8(CZP->vdc_cr.hi);
        WR8(CVDC->data_hi, value);
    } else {
        value = RD8(CZP->vdc_cr.lo);
        WR8(CVDC->data_lo, value);
    }
    return exit_a(value);
}

/*
 * @card 00:E382 ex_imode
 * WHAT IT DOES: Sets how far the VRAM address moves after an access: 1, 32, 64 or 128 words.
 * HOW: Bits 3 and 4 of the control register's high byte are cleared and set from A's bits 0-1, then the
 * register is written.
 * WAITS: no.
 * Entry: A mode. Exit: A the high byte, N and Z from it. X, Y, C, V kept.
 * Not run by any script.
 */
CardExit ex_imode(uint8_t mode)
{
    uint8_t value;

    clear_zp_bit(ADDR_OF8(CZP->vdc_cr.hi), CRHI_BIT_3);
    clear_zp_bit(ADDR_OF8(CZP->vdc_cr.hi), CRHI_BIT_4);
    value = RD8(CZP->vdc_cr.hi);
    WR8(CZP->vdc_cr.hi, (uint8_t)(((uint8_t)(mode << 3) & 0x18) | value));
    return control_out(VDC_REG_CR, true);
}

void abi_ex_imode(void)
{
    card_exit_apply(ex_imode(cpu.a));
}

/*
 * @card 00:E391 ex_vmode
 * WHAT IT DOES: Sets the control register's increment to suit a vertical map.
 * HOW: Bit 3 set, then bit 4 and bit 3 by the memory width register.
 * WAITS: no.
 * Entry: nothing. Exit: A the high byte, N and Z from it. X, Y, C, V kept.
 * Not run by any script.
 */
CardExit ex_vmode(void)
{
    uint8_t value;

    set_zp_bit(ADDR_OF8(CZP->vdc_cr.hi), CRHI_BIT_3);
    value = RD8(CRAM->vdc_mwr);
    if ((value & 0x30) == 0) {
        clear_zp_bit(ADDR_OF8(CZP->vdc_cr.hi), CRHI_BIT_4);
    } else {
        set_zp_bit(ADDR_OF8(CZP->vdc_cr.hi), CRHI_BIT_4);
        clear_zp_bit(ADDR_OF8(CZP->vdc_cr.hi), CRHI_BIT_3);
    }
    return control_out(VDC_REG_CR, true);
}

void abi_ex_vmode(void)
{
    card_exit_apply(ex_vmode());
}

/*
 * @card 00:E3A4 ex_hmode
 * WHAT IT DOES: Sets the control register's increment to one word, for a horizontal map.
 * HOW: Bits 3 and 4 of the high byte cleared, then the register written.
 * WAITS: no.
 * Entry: nothing. Exit: A the high byte, N and Z from it. X, Y, C, V kept.
 */
CardExit ex_hmode(void)
{
    clear_zp_bit(ADDR_OF8(CZP->vdc_cr.hi), CRHI_BIT_3);
    clear_zp_bit(ADDR_OF8(CZP->vdc_cr.hi), CRHI_BIT_4);
    return control_out(VDC_REG_CR, true);
}

void abi_ex_hmode(void)
{
    card_exit_apply(ex_hmode());
}

/*
 * @card 00:E3B5 ex_vsync
 * WHAT IT DOES: Waits for the next frame's interrupt, turning the vertical blank interrupt on if it is off.
 * HOW: Does nothing if bit 1 of CardIrqFlags is set. Else interrupts on, ex_irqon if the vertical blank
 * interrupt is off, then waits for CardFrameTick to change.
 * WAITS: yes, for the frame count the interrupt keeps; the stack there is the caller's frame.
 * Entry: nothing. Exit: A the count as first read, N, Z, C from its compare with the next value; X, Y, V kept.
 */
CardExit ex_vsync(void)
{
    CardExit e = { 0 };
    uint8_t flags = RD8(CZP->irq_flags);
    uint8_t value;
    uint8_t first;
    uint8_t now;

    if (flags & (1u << IRQ_FLAG_FRAME)) {
        return e;
    }
    cli();
    value = RD8(CZP->vdc_cr.lo);
    if (!(value & (1u << CR_BIT_VBLANK))) {
        jsr_push(0xE3BE);
        (void)ex_irqon();
        jsr_pop();
    }
    first = RD8(CRAM->frame_tick);
    do {
        now = RD8(CRAM->frame_tick);
    } while (now == first);
    e.a = first;
    e.nz = (uint8_t)(first - now);
    e.c = first >= now;
    e.set_a = e.set_nz = e.set_c = true;
    return e;
}

void abi_ex_vsync(void)
{
    card_exit_apply(ex_vsync());
}

/*
 * @card 00:E3C7 ex_rcron
 * WHAT IT DOES: Turns the raster compare interrupt on.
 * HOW: Bit 2 of the control register's low byte set, then the register written.
 * WAITS: no.
 * Entry: nothing. Exit: A the low byte, N and Z from it. X, Y, C, V kept.
 * Not run by any script.
 */
CardExit ex_rcron(void)
{
    set_zp_bit(ADDR_OF8(CZP->vdc_cr.lo), CR_BIT_RC);
    return control_out(VDC_REG_CR, false);
}

void abi_ex_rcron(void)
{
    card_exit_apply(ex_rcron());
}

/*
 * @card 00:E3CB ex_rcroff
 * WHAT IT DOES: Turns the raster compare interrupt off.
 * HOW: Bit 2 of the control register's low byte cleared, then the register written.
 * WAITS: no.
 * Entry: nothing. Exit: A the low byte, N and Z from it. X, Y, C, V kept.
 * Not run by any script.
 */
CardExit ex_rcroff(void)
{
    clear_zp_bit(ADDR_OF8(CZP->vdc_cr.lo), CR_BIT_RC);
    return control_out(VDC_REG_CR, false);
}

void abi_ex_rcroff(void)
{
    card_exit_apply(ex_rcroff());
}

/*
 * @card 00:E3CF ex_irqon
 * WHAT IT DOES: Turns the vertical blank interrupt on.
 * HOW: Bit 3 of the control register's low byte set, then the register written.
 * WAITS: no.
 * Entry: nothing. Exit: A the low byte, N and Z from it. X, Y, C, V kept.
 */
CardExit ex_irqon(void)
{
    set_zp_bit(ADDR_OF8(CZP->vdc_cr.lo), CR_BIT_VBLANK);
    return control_out(VDC_REG_CR, false);
}

void abi_ex_irqon(void)
{
    card_exit_apply(ex_irqon());
}

/*
 * @card 00:E3D3 ex_irqoff
 * WHAT IT DOES: Turns the vertical blank interrupt off.
 * HOW: Bit 3 of the control register's low byte cleared, then the register written.
 * WAITS: no.
 * Entry: nothing. Exit: A the low byte, N and Z from it. X, Y, C, V kept.
 */
CardExit ex_irqoff(void)
{
    clear_zp_bit(ADDR_OF8(CZP->vdc_cr.lo), CR_BIT_VBLANK);
    return control_out(VDC_REG_CR, false);
}

void abi_ex_irqoff(void)
{
    card_exit_apply(ex_irqoff());
}

/*
 * @card 00:E3E2 ex_bgon
 * WHAT IT DOES: Turns the background on, at the next control register write.
 * HOW: Bit 7 of the control register's low byte set.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 */
void ex_bgon(void)
{
    set_zp_bit(ADDR_OF8(CZP->vdc_cr.lo), CR_BIT_BACKGROUND);
}

void abi_ex_bgon(void)
{
    ex_bgon();
}

/*
 * @card 00:E3E5 ex_bgoff
 * WHAT IT DOES: Turns the background off, at the next control register write.
 * HOW: Bit 7 of the control register's low byte cleared.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 * Not run by any script.
 */
void ex_bgoff(void)
{
    clear_zp_bit(ADDR_OF8(CZP->vdc_cr.lo), CR_BIT_BACKGROUND);
}

void abi_ex_bgoff(void)
{
    ex_bgoff();
}

/*
 * @card 00:E3E8 ex_spron
 * WHAT IT DOES: Turns the sprites on, at the next control register write.
 * HOW: Bit 6 of the control register's low byte set.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 * Not run by any script.
 */
void ex_spron(void)
{
    set_zp_bit(ADDR_OF8(CZP->vdc_cr.lo), CR_BIT_SPRITES);
}

void abi_ex_spron(void)
{
    ex_spron();
}

/*
 * @card 00:E3EB ex_sproff
 * WHAT IT DOES: Turns the sprites off, at the next control register write.
 * HOW: Bit 6 of the control register's low byte cleared.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 * Not run by any script.
 */
void ex_sproff(void)
{
    clear_zp_bit(ADDR_OF8(CZP->vdc_cr.lo), CR_BIT_SPRITES);
}

void abi_ex_sproff(void)
{
    ex_sproff();
}

/*
 * @card 00:E3EE ex_dspon
 * WHAT IT DOES: Turns the sprites and the background on, at the next control register write.
 * HOW: Bits 6 and 7 of the control register's low byte set.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 * Not run by any script.
 */
void ex_dspon(void)
{
    set_zp_bit(ADDR_OF8(CZP->vdc_cr.lo), CR_BIT_SPRITES);
    set_zp_bit(ADDR_OF8(CZP->vdc_cr.lo), CR_BIT_BACKGROUND);
}

void abi_ex_dspon(void)
{
    ex_dspon();
}

/*
 * @card 00:E3F3 ex_dspoff
 * WHAT IT DOES: Turns the sprites and the background off, at the next control register write.
 * HOW: Bits 6 and 7 of the control register's low byte cleared.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 * Not run by any script.
 */
void ex_dspoff(void)
{
    clear_zp_bit(ADDR_OF8(CZP->vdc_cr.lo), CR_BIT_SPRITES);
    clear_zp_bit(ADDR_OF8(CZP->vdc_cr.lo), CR_BIT_BACKGROUND);
}

void abi_ex_dspoff(void)
{
    ex_dspoff();
}

/*
 * @card 00:E3F8 ex_dmamod
 * WHAT IT DOES: Sets the VDC's DMA control register from the low five bits of A, and keeps them.
 * HOW: CardVdcDcr, then the register select and the value to the port.
 * WAITS: no.
 * Entry: A. Exit: A the register, N and Z from it. X, Y, C, V kept.
 */
CardExit ex_dmamod(uint8_t mode)
{
    uint8_t value;

    WR8(CRAM->vdc_dcr, (uint8_t)(mode & DCR_MASK));
    select_vdc_register(VDC_REG_DCR);
    value = RD8(CRAM->vdc_dcr);
    WR8(CVDC->data_lo, value);
    return exit_a(value);
}

void abi_ex_dmamod(void)
{
    card_exit_apply(ex_dmamod(cpu.a));
}

/*
 * @card 00:E40B ex_sprdma
 * WHAT IT DOES: Gives the VDC the sprite attribute table's address for its DMA.
 * HOW: The register select, then CardSatAddress's two bytes to the port.
 * WAITS: no.
 * Entry: CardSatAddress. Exit: A its high byte, N and Z from it. X, Y, C, V kept.
 */
CardExit ex_sprdma(void)
{
    uint8_t value;

    select_vdc_register(VDC_REG_SATB);
    value = RD8(CRAM->sat_address);
    WR8(CVDC->data_lo, value);
    value = rd8((uint16_t)(CardSatAddress + 1));
    WR8(CVDC->data_hi, value);
    return exit_a(value);
}

void abi_ex_sprdma(void)
{
    card_exit_apply(ex_sprdma());
}

/*
 * @card 00:E41F ex_setrcr
 * WHAT IT DOES: Sets the raster compare register to A and X.
 * HOW: A is pushed, the register selected, A pulled and written, then X.
 * WAITS: no.
 * Entry: A low, X high. Exit: A as given, N and Z from it. X, Y, C, V kept.
 * Not run by any script.
 */
CardExit ex_setrcr(uint8_t a, uint8_t x)
{
    push8(a);
    select_vdc_register(VDC_REG_RCR);
    a = pull8();
    WR8(CVDC->data_lo, a);
    WR8(CVDC->data_hi, x);
    return exit_a(a);
}

void abi_ex_setrcr(void)
{
    card_exit_apply(ex_setrcr(cpu.a, cpu.x));
}

/*
 * @card 00:E42F ex_setred
 * WHAT IT DOES: Sets the VRAM read address to A and X, and leaves the VRAM data register selected.
 * HOW: A is pushed, the read register selected, A pulled and written, X, then the data register selected.
 * WAITS: no.
 * Entry: A low, X high. Exit: A 2, N 0, Z 0. X, Y, C, V kept.
 * Not run by any script.
 */
CardExit ex_setred(uint8_t a, uint8_t x)
{
    push8(a);
    select_vdc_register(0x01);
    a = pull8();
    WR8(CVDC->data_lo, a);
    WR8(CVDC->data_hi, x);
    select_vdc_register(VDC_REG_VWR);
    return exit_a(VDC_REG_VWR);
}

void abi_ex_setred(void)
{
    card_exit_apply(ex_setred(cpu.a, cpu.x));
}

/*
 * @card 00:E446 ex_setwrt
 * WHAT IT DOES: Sets the VRAM write address to A and X, and leaves the VRAM data register selected.
 * HOW: A is pushed, the write register selected, A pulled and written, X, then the data register selected.
 * WAITS: no.
 * Entry: A low, X high. Exit: A 2, N 0, Z 0. X, Y, C, V kept.
 * Not run by any script.
 */
CardExit ex_setwrt(uint8_t a, uint8_t x)
{
    push8(a);
    select_vdc_register(VDC_REG_MAWR);
    a = pull8();
    WR8(CVDC->data_lo, a);
    WR8(CVDC->data_hi, x);
    select_vdc_register(VDC_REG_VWR);
    return exit_a(VDC_REG_VWR);
}

void abi_ex_setwrt(void)
{
    card_exit_apply(ex_setwrt(cpu.a, cpu.x));
}

/*
 * @card 00:E45D ex_setdma
 * WHAT IT DOES: Starts a VRAM to VRAM DMA: _BX the source, _DX the destination, _CX the length.
 * HOW: The register select is kept on the stack; the three registers are set; the select is put back.
 * WAITS: no.
 * Entry: _BX, _CX, _DX. Exit: A the register select it kept, N and Z from it. X, Y, C, V kept.
 * Not run by any script.
 */
CardExit ex_setdma(void)
{
    uint8_t value;

    value = RD8(CZP->vdc_reg);
    push8(value);
    select_vdc_register(VDC_REG_SOUR);
    value = RD8(CZP->b.lo);
    WR8(CVDC->data_lo, value);
    value = RD8(CZP->b.hi);
    WR8(CVDC->data_hi, value);
    select_vdc_register(VDC_REG_DESR);
    value = RD8(CZP->d.lo);
    WR8(CVDC->data_lo, value);
    value = RD8(CZP->d.hi);
    WR8(CVDC->data_hi, value);
    select_vdc_register(VDC_REG_LENR);
    value = RD8(CZP->c.lo);
    WR8(CVDC->data_lo, value);
    value = RD8(CZP->c.hi);
    WR8(CVDC->data_hi, value);
    value = pull8();
    select_vdc_register(value);
    return exit_a(value);
}

void abi_ex_setdma(void)
{
    card_exit_apply(ex_setdma());
}

/*
 * @card 01:CBF0 clear_vram
 * WHAT IT DOES: Fills VRAM with the blank tile: $4000 words of $0200.
 * HOW: The write address to 0, the data register selected, then the word $0200 sixteen thousand times.
 * WAITS: no.
 * Entry: bank 1 at $C000. Exit: X 0, Y 0, A 2, N 0, Z 0, C, V kept.
 */
CardExit clear_vram(void)
{
    CardExit e = { 0 };

    select_vdc_register(VDC_REG_MAWR);
    st1(0);
    st2(0);
    select_vdc_register(VDC_REG_VWR);
    for (unsigned i = 0; i < 64u * 64u; i++) {
        WR8(CVDC->data_lo, 0);
        WR8(CVDC->data_hi, 2);
    }
    e.a = 2;
    e.x = 0;
    e.y = 0;
    e.nz = 0;
    e.set_a = e.set_x = e.set_y = e.set_nz = true;
    return e;
}

void abi_clear_vram(void)
{
    card_exit_apply(clear_vram());
}

enum {
    FONT_VRAM_LO = 0x00, FONT_VRAM_HI = 0x20, FONT_ROM_HI = 0xC0, FONT_ROWS = 8, FONT_PASSES = 2,
    PICTURE_BANK = 0x16, PICTURE_BYTES = 0x2000, MPR_BANK4 = 0x10, MPR_BANK5 = 0x20
};

/* The block moves the CPU makes: TIA, TAI and TII push Y, A and X, move the bytes, and pull them back. */
static void block_move_tia(uint16_t src, uint16_t dst, uint16_t len, uint8_t a, uint8_t x, uint8_t y)
{
    push8(y);
    push8(a);
    push8(x);
    for (uint16_t i = 0; i < len; i++) {
        uint8_t byte = rd8((uint16_t)(src + i));

        wr8((uint16_t)(dst + (i & 1)), byte);
    }
    (void)pull8();
    (void)pull8();
    (void)pull8();
}

/*
 * @card 01:CC9E load_font
 * WHAT IT DOES: Loads the card's font and its title picture's tiles into VRAM from $2000.
 * HOW: 256 one-plane glyphs of eight rows from $C000 in bank 1, each row written as both bytes
 * of a word, twice over, for 16 words a tile; then banks $16 and $17 mapped at $8000 for a block
 * move of $2000 bytes to the data port, and the MPRs put back.
 * WAITS: no.
 * Entry: bank 1 at $C000. Exit: A what MPR4 held, N and Z from it, X 0, Y 8, C 0, V 0.
 * CardPointerLo-Hi $C800, CardCopyLo 0.
 */
CardExit load_font(void)
{
    CardExit e = { 0 };
    uint8_t value;
    uint8_t bank4;
    uint8_t bank5;
    Sum sum;
    uint8_t y = 0;

    select_vdc_register(VDC_REG_MAWR);
    st1(FONT_VRAM_LO);
    st2(FONT_VRAM_HI);
    select_vdc_register(VDC_REG_VWR);
    WR8(CZP->pointer.lo, 0);
    WR8(CZP->pointer.hi, FONT_ROM_HI);
    WR8(CZP->copy.lo, 0);
    do {
        for (unsigned pass = 0; pass < FONT_PASSES; pass++) {
            y = 0;
            do {
                value = RD8_PTR_Y(CZP->pointer.lo, y);
                WR8(CVDC->data_lo, value);
                WR8(CVDC->data_hi, value);
                y++;
            } while (y < FONT_ROWS);
        }
        value = RD8(CZP->pointer.lo);
        sum = add8(value, FONT_ROWS, false);
        WR8(CZP->pointer.lo, sum.value);
        value = RD8(CZP->pointer.hi);
        sum = add8(value, 0, sum.carry);
        WR8(CZP->pointer.hi, sum.value);
    } while (DEC8(CZP->copy.lo) != 0);
    bank4 = tma(MPR_BANK4);
    push8(bank4);
    bank5 = tma(MPR_BANK5);
    push8(bank5);
    tam(MPR_BANK4, PICTURE_BANK);
    tam(MPR_BANK5, PICTURE_BANK + 1);
    block_move_tia(Mpr4Window, VdcDataLo, PICTURE_BYTES, PICTURE_BANK + 1, 0, FONT_ROWS);
    value = pull8();
    tam(MPR_BANK5, value);
    value = pull8();
    tam(MPR_BANK4, value);
    e.a = e.nz = value;
    e.x = 0;
    e.y = FONT_ROWS;
    e.c = sum.carry;
    e.v = sum.overflow;
    e.set_a = e.set_nz = e.set_x = e.set_y = e.set_c = e.set_v = true;
    return e;
}

void abi_load_font(void)
{
    card_exit_apply(load_font());
}

enum {
    COLOR_READ_BACK = 0x01, STUB_TIA = 0xE3, STUB_RTS = 0x60, STUB_SOURCE = 1, STUB_DESTINATION = 3,
    STUB_LENGTH_LOW = 5, STUB_LENGTH_HIGH = 6, STUB_RETURN = 7, COLOR_STUB_HEAD = 3, SPRITE_PALETTES = 1,
    COLOR_SET_COUNT_AT = 2, COLOR_SET_SHIFT_LOW = 4, STUB_PORT_HIGH = 4, VCE_PORT_LOW = 4
};

/* One set of palettes, to the VCE or back, as the BIOS's stub does it: the registers the block moves push are
 * A (the length's low byte), X and Y as they came. */
static CardExit move_color_set(uint16_t set, uint8_t palette_high, bool read_back, uint16_t ret_stub,
                               uint8_t x, uint8_t y, bool *moved_any)
{
    CardExit e = { 0 };
    uint8_t count = rd8((uint16_t)(set + COLOR_SET_COUNT_AT));
    uint8_t carry_low;
    uint8_t carry_high;
    uint8_t value;
    uint8_t length_low;
    uint8_t length_high;
    uint8_t memory_low;
    uint8_t memory_high;
    uint16_t memory;

    e.a = e.nz = count;
    e.set_a = e.set_nz = true;
    if (count == 0) {
        return e;
    }
    wr8(STUB(STUB_LENGTH_HIGH), 0);
    carry_low = (count >> 4) & 1;
    value = rd8(STUB(STUB_LENGTH_HIGH));
    length_high = (uint8_t)((value << 1) | carry_low);
    wr8(STUB(STUB_LENGTH_HIGH), length_high);
    carry_high = (count >> 3) & 1;
    length_low = (uint8_t)(count << 5);
    value = rd8(STUB(STUB_LENGTH_HIGH));
    length_high = (uint8_t)((value << 1) | carry_high);
    wr8(STUB(STUB_LENGTH_HIGH), length_high);
    wr8(STUB(STUB_LENGTH_LOW), length_low);
    push8(y);
    push8(length_low);
    push8(x);
    memory_low = rd8(set);
    wr8(STUB(read_back ? STUB_DESTINATION : STUB_SOURCE), memory_low);
    memory_high = rd8((uint16_t)(set + 1));
    wr8(STUB((read_back ? STUB_DESTINATION : STUB_SOURCE) + 1), memory_high);
    (void)pull8();
    (void)pull8();
    (void)pull8();
    WR8(CVCE->addr_lo, 0);
    WR8(CVCE->addr_hi, palette_high);
    memory = (uint16_t)(memory_low | (memory_high << 8));
    jsr_push(ret_stub);
    if (read_back) {
        push8(y);
        push8(length_low);
        push8(x);
        for (uint16_t i = 0; i < (uint16_t)(length_low | (length_high << 8)); i++) {
            uint8_t byte = rd8((uint16_t)(0x0404 + (i & 1)));

            wr8((uint16_t)(memory + i), byte);
        }
        (void)pull8();
        (void)pull8();
        (void)pull8();
    } else {
        block_move_tia(memory, VceDataLo, (uint16_t)(length_low | (length_high << 8)), length_low, x, y);
    }
    jsr_pop();
    e.c = false;
    e.set_c = true;
    if (palette_high != 0) {
        e.a = palette_high;
        e.nz = palette_high;
    }
    *moved_any = true;
    return e;
}

/*
 * @card 00:E509 ex_colorcmd
 * WHAT IT DOES: Does what CardColorCommand asks of the palettes, if anything: writes the
 * background and sprite colours to the VCE, or with bit 0 reads them back into memory.
 * HOW: Writes a block move in RAM at CardBlockStub (TIA to the colour data port, or TAI from it)
 * and runs it once for each set whose count is not zero: the background's at colour 0, the
 * sprites' at colour $100, 32 bytes a 16-colour palette. Then it clears the command.
 * WAITS: no.
 * Entry: CardColorCommand, CardColorBackground, CardColorSprites; X, Y (pushed by the block moves). Exit, no
 * command: A 0, N 0, Z 1. Sprites moved: A 1, the VCE address's high byte it last loaded, N 0, Z 0, C 0. Sprites
 * skipped: A 0, N 0, Z 1, C 0 if the background moved, else as it was. X, Y, V kept.
 */
CardExit ex_colorcmd(uint8_t x, uint8_t y)
{
    CardExit e = { 0 };
    CardExit step;
    uint8_t command = RD8(CRAM->color_command);
    bool read_back;
    bool moved = false;

    e.a = e.nz = command;
    e.set_a = e.set_nz = true;
    if (command == 0) {
        return e;
    }
    read_back = (command & COLOR_READ_BACK) != 0;
    if (!read_back) {
        wr8(STUB(0), STUB_TIA);
        wr8(STUB(STUB_DESTINATION), VCE_PORT_LOW);
        wr8(STUB(STUB_DESTINATION + 1), STUB_PORT_HIGH);
    } else {
        push8(y);
        push8(command & COLOR_READ_BACK);
        push8(x);
        for (unsigned i = 0; i < COLOR_STUB_HEAD; i++) {
            uint8_t byte = rd8((uint16_t)(ColorReadStub + i));

            wr8(STUB(i), byte);
        }
        (void)pull8();
        (void)pull8();
        (void)pull8();
    }
    wr8(STUB(STUB_RETURN), STUB_RTS);
    step = move_color_set(CardColorBackground, 0, read_back, read_back ? 0xE5AB : 0xE54D, x, y, &moved);
    e = card_exit_over(e, step);
    step = move_color_set((uint16_t)CardColorSprites, SPRITE_PALETTES, read_back, read_back ? 0xE5D3 : 0xE575, x, y,
                          &moved);
    e = card_exit_over(e, step);
    WR8(CRAM->color_command, 0);
    return e;
}

void abi_ex_colorcmd(void)
{
    card_exit_apply(ex_colorcmd(cpu.x, cpu.y));
}
