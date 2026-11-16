/* The BIOS's video setters. */
#include "card.h"
#include "syscard.h"

enum {
    VDC_REG_MWR = 0x09,
    MWR_SIZE_BITS = 0x70, MWR_DOT_BITS = 0x8F, SCREEN_SIZE_MASK = 0x07
};

/* Sets the bits of the VDC's memory width register, and keeps a copy. */
static uint8_t set_memory_width(uint8_t keep, uint8_t bits)
{
    uint8_t width = (uint8_t)((rd8(CardVdcMwr) & keep) | bits);

    wr8(CardVdcMwr, width);
    wr8(CardVdcReg, VDC_REG_MWR);
    wr8(VdcAddress, VDC_REG_MWR);
    wr8(VdcDataLo, width);
    return width;
}

static uint8_t size_bits(uint8_t size)
{
    return (uint8_t)((size & SCREEN_SIZE_MASK) << 4);
}

/*
 * @card 00:E267 ex_scrsiz
 * WHAT IT DOES: Sets the size of the background map.
 * HOW: The size's three bits go to bits 4-6 of the memory width register.
 * Entry: A size. Exit: A the register, N and Z from it, Y the bits, C clear. X and V kept.
 */
uint8_t ex_scrsiz(uint8_t size)
{
    return set_memory_width(MWR_DOT_BITS, size_bits(size));
}

void abi_ex_scrsiz(void)
{
    uint8_t given = cpu.a;

    ex_scrsiz_exit_shim(given, ex_scrsiz(given));
}

/* ex_scrsiz's exit, for its wrapper and for a caller's that ends in it: the size given, and what it returned. */
void ex_scrsiz_exit_shim(uint8_t given, uint8_t width)
{
    cpu.a = width;
    cpu.y = size_bits(given);
    abi_nz(width);
    abi_flag(P_C, false);
}

/*
 * @card 00:E272 ex_dotmod
 * WHAT IT DOES: Sets the VRAM dot width and sprite width of the memory width register.
 * HOW: The mode's bits outside the map size go to the register.
 * Entry: A mode. Exit: A the register, N and Z from it, Y the bits. X, C, V kept.
 */
uint8_t ex_dotmod(uint8_t mode)
{
    return set_memory_width(MWR_SIZE_BITS, mode & MWR_DOT_BITS);
}

void abi_ex_dotmod(void)
{
    uint8_t bits = cpu.a & MWR_DOT_BITS;
    uint8_t width = ex_dotmod(cpu.a);

    cpu.a = width;
    cpu.y = bits;
    abi_nz(width);
}

enum {
    VDC_REG_MAWR = 0x00, VDC_REG_VWR = 0x02,
    VDC_REG_HSR = 0x0A, VDC_REG_HDR = 0x0B, VDC_REG_VSR = 0x0C, VDC_REG_VDR = 0x0D,
    VDC_REG_VCR = 0x0E,
    SCREEN_MODE_BITS = 0x05, SCREEN_WIDE = 0x01, VCE_CLOCK_FLIP = 0x04,
    HSYNC_WIDTH_NARROW = 0x02, HSYNC_WIDTH_WIDE = 0x03,
    HSTART_NARROW = 0x13, HSTART_WIDE = 0x19, HEND_NARROW = 0x27, HEND_WIDE = 0x35,
    DOTS_NARROW = 0x00, DOTS_WIDE = 0x0A,
    VSYNC_WIDTH = 0x02, VSTART_BASE = 0x87, VEND_BASE = 0x7E,
    ROWS_MIN = 0x0A, ROWS_LIMIT = 0x1F,
    COLUMNS_MIN_NARROW = 0x0A, COLUMNS_LIMIT_NARROW = 0x23,
    COLUMNS_MIN_WIDE = 0x10, COLUMNS_LIMIT_WIDE = 0x2D,
    FALLBACK_COLUMNS = 0x20, FALLBACK_ROWS = 0x1E, SCRMOD_FAILED = 0x01,
    SAT_WORDS = 0x100
};

/* Selects a VDC register, keeping a copy of the choice. */
static void select_vdc_register(uint8_t reg)
{
    wr8(CardVdcReg, reg);
    wr8(VdcAddress, reg);
}

static void set_vdc_register(uint8_t reg, uint8_t low, uint8_t high)
{
    select_vdc_register(reg);
    wr8(VdcDataLo, low);
    wr8(VdcDataHi, high);
}

/*
 * @card 00:E341 set_screen_rows
 * WHAT IT DOES: Sets the VDC's vertical display for CardScreenRows rows of tiles, and the VCE's
 * dot clock from CardScreenMode.
 * HOW: Vertical sync two lines wide, the display starting $87 less half the rows' lines, the
 * display rows times eight lines tall, its end at $7E plus half the lines (the shift's carry is
 * the one, as the lines are always a multiple of eight); then the mode with
 * bit 2 flipped goes to the VCE.
 * Entry: CardScreenRows, CardScreenMode. Exit: A what went to the VCE, N and Z from it, X the
 * lines less one, C and V from adding the end's base. Y kept.
 */
ScreenRows set_screen_rows(void)
{
    ScreenRows set;
    uint8_t lines_less_one, half;

    set.rows_dots = (uint8_t)(rd8(CardScreenRows) << 3);
    lines_less_one = (uint8_t)(set.rows_dots - 1);
    set_vdc_register(VDC_REG_VSR, VSYNC_WIDTH, (uint8_t)(VSTART_BASE - (set.rows_dots >> 1)));
    set_vdc_register(VDC_REG_VDR, lines_less_one, 0);
    half = (uint8_t)((uint8_t)(set.rows_dots + 1) >> 1);
    set_vdc_register(VDC_REG_VCR, (uint8_t)(half + VEND_BASE), 0);
    set.vce = rd8(CardScreenMode) ^ VCE_CLOCK_FLIP;
    wr8(VceControl, set.vce);
    return set;
}

void abi_set_screen_rows(void)
{
    ScreenRows set = set_screen_rows();
    uint8_t half = (uint8_t)((uint8_t)(set.rows_dots + 1) >> 1);
    int end = (int8_t)half + VEND_BASE;

    cpu.a = set.vce;
    cpu.x = (uint8_t)(set.rows_dots - 1);
    abi_nz(set.vce);
    abi_flag(P_C, half + VEND_BASE > 0xFF);
    abi_flag(P_V, end > 127);
}

/*
 * @card 00:E29D ex_scrmod
 * WHAT IT DOES: Sets the screen's size and dot clock: A the mode (bit 0 wide, bit 2 the dot
 * clock), X the columns of tiles, Y the rows.
 * HOW: Keeps the three; refuses rows outside $0A-$1E, or columns outside $0A-$22 ($10-$2C
 * wide), by setting the fallback mode instead; sets the vertical display, then the horizontal
 * sync, start, width and end, and the memory width register's dot width.
 * Entry: A, X, Y. Exit, set: C clear, A the memory width register, N and Z from it, X the
 * columns less one, Y the dot bits, V from adding the end's base. Refused: as ex_scrmod_fallback.
 */
ScreenMode ex_scrmod(uint8_t mode, uint8_t columns, uint8_t rows)
{
    ScreenMode set = { true, columns, 0, DOTS_NARROW, 0, HEND_NARROW };
    bool wide = (mode & SCREEN_WIDE) != 0;
    uint8_t start;

    wr8(CardScreenMode, mode & SCREEN_MODE_BITS);
    wr8(CardScreenColumns, columns);
    wr8(CardScreenRows, rows);
    if (rows < ROWS_MIN || rows >= ROWS_LIMIT)
        return ex_scrmod_fallback();
    if (wide ? columns < COLUMNS_MIN_WIDE || columns >= COLUMNS_LIMIT_WIDE
             : columns < COLUMNS_MIN_NARROW || columns >= COLUMNS_LIMIT_NARROW)
        return ex_scrmod_fallback();
    set_screen_rows();
    start = (uint8_t)((wide ? HSTART_WIDE : HSTART_NARROW) - 1 - (columns >> 1));
    set_vdc_register(VDC_REG_HSR, wide ? HSYNC_WIDTH_WIDE : HSYNC_WIDTH_NARROW, start);
    set.end_inverse = (uint8_t)~(uint8_t)(start + columns);
    set.end_base = wide ? HEND_WIDE : HEND_NARROW;
    set_vdc_register(VDC_REG_HDR, (uint8_t)(columns - 1), (uint8_t)(set.end_inverse + set.end_base));
    set.dot_bits = wide ? DOTS_WIDE : DOTS_NARROW;
    set.width_register = ex_dotmod(set.dot_bits);
    return set;
}

static void abi_scrmod_exit_shim(ScreenMode set)
{
    int sum = (int8_t)set.end_inverse + (int8_t)set.end_base;

    cpu.x = (uint8_t)(set.columns - 1);
    cpu.y = set.dot_bits;
    abi_flag(P_V, sum < -128 || sum > 127);
}

void abi_ex_scrmod(void)
{
    ScreenMode set = ex_scrmod(cpu.a, cpu.x, cpu.y);

    abi_scrmod_exit_shim(set);
    if (set.accepted) {
        cpu.a = set.width_register;
        abi_nz(set.width_register);
        abi_flag(P_C, false);
    } else {
        cpu.a = SCRMOD_FAILED;
        abi_nz(SCRMOD_FAILED);
        abi_flag(P_C, true);
    }
}

/*
 * @card 00:E292 ex_scrmod_fallback
 * WHAT IT DOES: Where ex_scrmod refuses a size: sets 32 columns by 30 rows, narrow, and fails.
 * HOW: Calls ex_scrmod with mode 0, $20 columns and $1E rows, then gives 1 with C set.
 * Entry: nothing. Exit: A 1, N 0, Z 0, C set, X $1F, Y 0, V as ex_scrmod leaves it.
 */
ScreenMode ex_scrmod_fallback(void)
{
    ScreenMode set = ex_scrmod(0, FALLBACK_COLUMNS, FALLBACK_ROWS);

    set.accepted = false;
    return set;
}

void abi_ex_scrmod_fallback(void)
{
    ScreenMode set = ex_scrmod_fallback();

    abi_scrmod_exit_shim(set);
    cpu.a = SCRMOD_FAILED;
    abi_nz(SCRMOD_FAILED);
    abi_flag(P_C, true);
}

/*
 * @card 00:E5DA ex_satclr
 * WHAT IT DOES: Clears the sprite table in VRAM.
 * HOW: Sets the VDC's write address to CardSatAddress and writes 256 words of zero, then
 * clears CardSatFlag.
 * Entry: CardSatAddress. Exit: A 2, X 0, N 0, Z 1. Y, C, V kept.
 */
void ex_satclr(void)
{
    set_vdc_register(VDC_REG_MAWR, rd8(CardSatAddress), rd8((uint16_t)(CardSatAddress + 1)));
    select_vdc_register(VDC_REG_VWR);
    for (unsigned i = 0; i < SAT_WORDS; i++) {
        wr8(VdcDataLo, 0);
        wr8(VdcDataHi, 0);
    }
    wr8(CardSatFlag, 0);
}

void abi_ex_satclr(void)
{
    ex_satclr();
    cpu.a = VDC_REG_VWR;
    cpu.x = 0;
    abi_nz(0);
}

enum {
    VDC_REG_CONTROL = 0x05, VDC_REG_RCR = 0x06, VDC_REG_MARR = 0x01, VDC_REG_DCR = 0x0F,
    VDC_REG_SATB = 0x13, VDC_REG_SOUR = 0x10, VDC_REG_DESR = 0x11, VDC_REG_LENR = 0x12,
    CR_HIGH_INCREMENT = 0x18, CR_HIGH_INCREMENT_32 = 0x08, CR_HIGH_INCREMENT_64 = 0x10,
    MWR_HEIGHT_BITS = 0x30,
    CR_RASTER_IRQ = 0x04, CR_VBLANK_IRQ = 0x08, CR_SPRITES = 0x40, CR_BACKGROUND = 0x80,
    DCR_BITS = 0x1F, IRQ_NO_VSYNC_WAIT = 0x02
};

/* The control register's high byte from its copy. */
static uint8_t write_control_high(void)
{
    uint8_t high;

    select_vdc_register(VDC_REG_CONTROL);
    high = rd8(CardVdcCrHi);
    wr8(VdcDataHi, high);
    return high;
}

/* The control register's low byte from its copy. */
static uint8_t write_control_low(void)
{
    uint8_t low;

    select_vdc_register(VDC_REG_CONTROL);
    low = rd8(CardVdcCrLo);
    wr8(VdcDataLo, low);
    return low;
}

static void set_increment(uint8_t bits)
{
    wr8(CardVdcCrHi, (uint8_t)((rd8(CardVdcCrHi) & ~CR_HIGH_INCREMENT) | bits));
}

static void change_control_low(uint8_t set, uint8_t clear)
{
    wr8(CardVdcCrLo, (uint8_t)((rd8(CardVdcCrLo) & ~clear) | set));
}

/* A or N and Z from what was written: the exit of the setters that write the register. */
static void abi_written_shim(uint8_t written)
{
    cpu.a = written;
    abi_nz(written);
}

/*
 * @card 00:E382 ex_imode
 * WHAT IT DOES: Sets how far the VDC's VRAM address steps after each access: A 0 to 3 for 1,
 * 32, 64 or 128 words.
 * HOW: The two bits go to bits 3 and 4 of the control register's high byte, kept in
 * CardVdcCrHi, which is then written.
 * Entry: A. Exit: A the high byte, N and Z from it. X, Y, C, V kept.
 */
uint8_t ex_imode(uint8_t mode)
{
    set_increment((uint8_t)((mode << 3) & CR_HIGH_INCREMENT));
    return write_control_high();
}

void abi_ex_imode(void)
{
    abi_written_shim(ex_imode(cpu.a));
}

/*
 * @card 00:E391 ex_vmode
 * WHAT IT DOES: Steps the VRAM address by one column of the background map: 32 words if the map
 * is 32 tiles wide, else 64.
 * HOW: Reads the map's width from CardVdcMwr. (Its test of bit 0 after keeping bits 4 and 5 can
 * never be true.)
 * Entry: nothing. Exit: as ex_imode.
 */
uint8_t ex_vmode(void)
{
    set_increment((rd8(CardVdcMwr) & MWR_HEIGHT_BITS) ? CR_HIGH_INCREMENT_64 : CR_HIGH_INCREMENT_32);
    return write_control_high();
}

void abi_ex_vmode(void)
{
    abi_written_shim(ex_vmode());
}

/*
 * @card 00:E3A4 ex_hmode
 * WHAT IT DOES: Steps the VRAM address by one word after each access.
 * HOW: Clears both step bits, then writes the high byte.
 * Entry: nothing. Exit: as ex_imode.
 */
uint8_t ex_hmode(void)
{
    set_increment(0);
    return write_control_high();
}

void abi_ex_hmode(void)
{
    abi_written_shim(ex_hmode());
}

/*
 * @card 00:E3B5 ex_vsync
 * WHAT IT DOES: Waits for the next vertical blank, unless bit 1 of CardIrqFlags says not to.
 * HOW: Enables interrupts, turns on the VDC's vertical blank interrupt if it was off, then
 * waits for CardFrameTick to change.
 * Entry: nothing. Exit, no wait: everything kept. Waited: I clear, A the count it waited on, Z 0,
 * N and C from comparing it with the new one. X, Y, V kept.
 */
VsyncWait ex_vsync(void)
{
    VsyncWait wait = { false, 0, 0 };

    if (rd8(CardIrqFlags) & IRQ_NO_VSYNC_WAIT)
        return wait;
    cli();
    if (!(rd8(CardVdcCrLo) & CR_VBLANK_IRQ))
        ex_irqon();
    wait.waited = true;
    wait.before = rd8(CardFrameTick);
    do
        wait.after = rd8(CardFrameTick);
    while (wait.after == wait.before);
    return wait;
}

void abi_ex_vsync(void)
{
    VsyncWait wait = ex_vsync();

    if (!wait.waited)
        return;
    cpu.a = wait.before;
    abi_nz((uint8_t)(wait.before - wait.after));
    abi_flag(P_C, wait.before >= wait.after);
    abi_flag(P_I, false);
}

/*
 * @card 00:E3C7 ex_rcron
 * WHAT IT DOES: Turns on the VDC's raster compare interrupt.
 * HOW: Sets bit 2 of CardVdcCrLo and writes the control register's low byte.
 * Entry: nothing. Exit: A the low byte, N and Z from it. X, Y, C, V kept.
 */
uint8_t ex_rcron(void)
{
    change_control_low(CR_RASTER_IRQ, 0);
    return write_control_low();
}

void abi_ex_rcron(void)
{
    abi_written_shim(ex_rcron());
}

/*
 * @card 00:E3CB ex_rcroff
 * WHAT IT DOES: Turns off the VDC's raster compare interrupt.
 * HOW: As ex_rcron, clearing the bit.
 * Entry: nothing. Exit: as ex_rcron.
 */
uint8_t ex_rcroff(void)
{
    change_control_low(0, CR_RASTER_IRQ);
    return write_control_low();
}

void abi_ex_rcroff(void)
{
    abi_written_shim(ex_rcroff());
}

/*
 * @card 00:E3CF ex_irqon
 * WHAT IT DOES: Turns on the VDC's vertical blank interrupt.
 * HOW: As ex_rcron, with bit 3.
 * Entry: nothing. Exit: as ex_rcron.
 */
uint8_t ex_irqon(void)
{
    change_control_low(CR_VBLANK_IRQ, 0);
    return write_control_low();
}

void abi_ex_irqon(void)
{
    abi_written_shim(ex_irqon());
}

/*
 * @card 00:E3D3 ex_irqoff
 * WHAT IT DOES: Turns off the VDC's vertical blank interrupt.
 * HOW: As ex_rcroff, with bit 3.
 * Entry: nothing. Exit: as ex_rcron.
 */
uint8_t ex_irqoff(void)
{
    change_control_low(0, CR_VBLANK_IRQ);
    return write_control_low();
}

void abi_ex_irqoff(void)
{
    abi_written_shim(ex_irqoff());
}

/*
 * @card 00:E3E2 ex_bgon
 * WHAT IT DOES: Asks for the background to be shown, from the next time the control register's
 * low byte is written.
 * HOW: Sets bit 7 of CardVdcCrLo.
 * Entry: nothing. Exit: everything kept.
 */
void ex_bgon(void)
{
    change_control_low(CR_BACKGROUND, 0);
}

void abi_ex_bgon(void)
{
    ex_bgon();
}

/*
 * @card 00:E3E5 ex_bgoff
 * WHAT IT DOES: Asks for the background to be hidden, as ex_bgon.
 * HOW: Clears bit 7 of CardVdcCrLo.
 * Entry: nothing. Exit: everything kept.
 */
void ex_bgoff(void)
{
    change_control_low(0, CR_BACKGROUND);
}

void abi_ex_bgoff(void)
{
    ex_bgoff();
}

/*
 * @card 00:E3E8 ex_spron
 * WHAT IT DOES: Asks for sprites to be shown, as ex_bgon.
 * HOW: Sets bit 6 of CardVdcCrLo.
 * Entry: nothing. Exit: everything kept.
 */
void ex_spron(void)
{
    change_control_low(CR_SPRITES, 0);
}

void abi_ex_spron(void)
{
    ex_spron();
}

/*
 * @card 00:E3EB ex_sproff
 * WHAT IT DOES: Asks for sprites to be hidden, as ex_bgon.
 * HOW: Clears bit 6 of CardVdcCrLo.
 * Entry: nothing. Exit: everything kept.
 */
void ex_sproff(void)
{
    change_control_low(0, CR_SPRITES);
}

void abi_ex_sproff(void)
{
    ex_sproff();
}

/*
 * @card 00:E3EE ex_dspon
 * WHAT IT DOES: Asks for the background and sprites to be shown, as ex_bgon.
 * HOW: Sets bits 6 and 7 of CardVdcCrLo.
 * Entry: nothing. Exit: everything kept.
 */
void ex_dspon(void)
{
    change_control_low(CR_SPRITES | CR_BACKGROUND, 0);
}

void abi_ex_dspon(void)
{
    ex_dspon();
}

/*
 * @card 00:E3F3 ex_dspoff
 * WHAT IT DOES: Asks for the background and sprites to be hidden, as ex_bgon.
 * HOW: Clears bits 6 and 7 of CardVdcCrLo.
 * Entry: nothing. Exit: everything kept.
 */
void ex_dspoff(void)
{
    change_control_low(0, CR_SPRITES | CR_BACKGROUND);
}

void abi_ex_dspoff(void)
{
    ex_dspoff();
}

/*
 * @card 00:E3F8 ex_dmamod
 * WHAT IT DOES: Sets the VDC's DMA control register.
 * HOW: Its five bits of A to CardVdcDcr, which goes to the register's low byte.
 * Entry: A. Exit: A the bits, N and Z from them. X, Y, C, V kept.
 */
uint8_t ex_dmamod(uint8_t mode)
{
    wr8(CardVdcDcr, mode & DCR_BITS);
    select_vdc_register(VDC_REG_DCR);
    wr8(VdcDataLo, rd8(CardVdcDcr));
    return rd8(CardVdcDcr);
}

void abi_ex_dmamod(void)
{
    abi_written_shim(ex_dmamod(cpu.a));
}

/*
 * @card 00:E40B ex_sprdma
 * WHAT IT DOES: Has the VDC copy the sprite table from VRAM at the next vertical blank.
 * HOW: CardSatAddress to the VDC's sprite table address register.
 * Entry: CardSatAddress. Exit: A its high byte, N and Z from it. X, Y, C, V kept.
 */
uint8_t ex_sprdma(void)
{
    set_vdc_register(VDC_REG_SATB, rd8(CardSatAddress), rd8((uint16_t)(CardSatAddress + 1)));
    return rd8((uint16_t)(CardSatAddress + 1));
}

void abi_ex_sprdma(void)
{
    abi_written_shim(ex_sprdma());
}

/*
 * @card 00:E41F ex_setrcr
 * WHAT IT DOES: Sets the raster line the VDC interrupts at: A low, X high.
 * HOW: The two bytes to the raster compare register.
 * Entry: A, X. Exit: N and Z from A. A, X, Y, C, V kept.
 */
void ex_setrcr(uint8_t low, uint8_t high)
{
    set_vdc_register(VDC_REG_RCR, low, high);
}

void abi_ex_setrcr(void)
{
    ex_setrcr(cpu.a, cpu.x);
    abi_nz(cpu.a);
}

/*
 * @card 00:E42F ex_setred
 * WHAT IT DOES: Sets the VRAM address to read from, A low, X high, and selects the data port.
 * HOW: The address to the read address register, then the data register selected.
 * Entry: A, X. Exit: A 2, N 0, Z 0. X, Y, C, V kept.
 */
void ex_setred(uint8_t low, uint8_t high)
{
    set_vdc_register(VDC_REG_MARR, low, high);
    select_vdc_register(VDC_REG_VWR);
}

void abi_ex_setred(void)
{
    ex_setred(cpu.a, cpu.x);
    abi_written_shim(VDC_REG_VWR);
}

/*
 * @card 00:E446 ex_setwrt
 * WHAT IT DOES: Sets the VRAM address to write to, A low, X high, and selects the data port.
 * HOW: As ex_setred, with the write address register.
 * Entry: A, X. Exit: as ex_setred.
 */
void ex_setwrt(uint8_t low, uint8_t high)
{
    set_vdc_register(VDC_REG_MAWR, low, high);
    select_vdc_register(VDC_REG_VWR);
}

void abi_ex_setwrt(void)
{
    ex_setwrt(cpu.a, cpu.x);
    abi_written_shim(VDC_REG_VWR);
}

/*
 * @card 00:E45D ex_setdma
 * WHAT IT DOES: Sets up a VRAM to VRAM copy: _BX the source, _DX the destination, _CX the length.
 * HOW: The three to the VDC's DMA registers, then the register that was selected is selected
 * again.
 * Entry: _BX, _CX, _DX. Exit: A the register selected, N and Z from it. X, Y, C, V kept.
 */
uint8_t ex_setdma(void)
{
    uint8_t selected = rd8(CardVdcReg);

    set_vdc_register(VDC_REG_SOUR, rd8(CardBl), rd8(CardBh));
    set_vdc_register(VDC_REG_DESR, rd8(CardDl), rd8(CardDh));
    set_vdc_register(VDC_REG_LENR, rd8(CardCl), rd8(CardCh));
    select_vdc_register(selected);
    return selected;
}

void abi_ex_setdma(void)
{
    abi_written_shim(ex_setdma());
}

enum {
    CLEAR_TILE = 0x0200, CLEAR_WORDS = 0x1000, FONT_VRAM = 0x2000, FONT_GLYPHS = 0x100,
    FONT_ROWS = 8, FONT_PASSES = 2, FONT_GLYPH_ROM = 0xC000,
    PICTURE_BANK = 0x16, PICTURE_AT = 0x8000, PICTURE_BYTES = 0x2000, MPR4 = 0x10, MPR5 = 0x20
};

/*
 * @card 01:CBF0 clear_vram
 * WHAT IT DOES: Fills the first 4K words of VRAM, where the background map lies, with tile $200.
 * HOW: The write address to 0, then $0200 written 4,096 times.
 * Entry: nothing. Exit: A 2, X 0, Y 0, N 0, Z 1. C, V kept.
 */
void clear_vram(void)
{
    set_vdc_register(VDC_REG_MAWR, 0, 0);
    select_vdc_register(VDC_REG_VWR);
    for (unsigned i = 0; i < CLEAR_WORDS; i++) {
        wr8(VdcDataLo, (uint8_t)CLEAR_TILE);
        wr8(VdcDataHi, (uint8_t)(CLEAR_TILE >> 8));
    }
}

void abi_clear_vram(void)
{
    clear_vram();
    cpu.a = (uint8_t)(CLEAR_TILE >> 8);
    cpu.x = 0;
    cpu.y = 0;
    abi_nz(0);
}

/*
 * @card 01:CC9E load_font
 * WHAT IT DOES: Loads the card's font and its title picture's tiles into VRAM from $2000.
 * HOW: 256 one-plane glyphs of eight rows from $C000 in bank 1, each row written as both bytes
 * of a word, twice over, for 16 words a tile; then banks $16 and $17 mapped at $8000 for a block
 * move of $2000 bytes to the data port, and the MPRs put back.
 * Entry: bank 1 at $C000. Exit: A what MPR4 held, N and Z from it, X 0, Y 8, C 0, V 0.
 * CardPointerLo-Hi $C800, CardCopyLo 0.
 */
uint8_t load_font(void)
{
    uint16_t glyph = FONT_GLYPH_ROM;
    uint8_t saved4, saved5;

    set_vdc_register(VDC_REG_MAWR, (uint8_t)FONT_VRAM, (uint8_t)(FONT_VRAM >> 8));
    select_vdc_register(VDC_REG_VWR);
    for (unsigned g = 0; g < FONT_GLYPHS; g++, glyph = (uint16_t)(glyph + FONT_ROWS)) {
        for (unsigned pass = 0; pass < FONT_PASSES; pass++) {
            for (unsigned row = 0; row < FONT_ROWS; row++) {
                uint8_t bits = rd8((uint16_t)(glyph + row));

                wr8(VdcDataLo, bits);
                wr8(VdcDataHi, bits);
            }
        }
    }
    wr8(CardPointerLo, (uint8_t)glyph);
    wr8(CardPointerHi, (uint8_t)(glyph >> 8));
    wr8(CardCopyLo, 0);
    saved4 = tma(MPR4);
    saved5 = tma(MPR5);
    tam(MPR4, PICTURE_BANK);
    tam(MPR5, PICTURE_BANK + 1);
    for (unsigned i = 0; i < PICTURE_BYTES; i++)
        wr8((i & 1) ? VdcDataHi : VdcDataLo, rd8((uint16_t)(PICTURE_AT + i)));
    tam(MPR5, saved5);
    tam(MPR4, saved4);
    return saved4;
}

void abi_load_font(void)
{
    cpu.a = load_font();
    cpu.x = 0;
    cpu.y = FONT_ROWS;
    abi_nz(cpu.a);
    abi_flag(P_C, false);
    abi_flag(P_V, false);
}

enum {
    COLOR_READ_BACK = 0x01, COLOR_SET_BYTES_SHIFT = 5, SPRITE_PALETTE_HIGH = 0x01,
    STUB_TIA = 0xE3, STUB_RTS = 0x60, STUB_SOURCE = 1, STUB_DESTINATION = 3, STUB_LENGTH = 5,
    STUB_RETURN = 7, COLOR_STUB_HEAD = 3
};

static void put_stub_word(unsigned at, uint16_t value)
{
    wr8((uint16_t)(CardBlockStub + at), (uint8_t)value);
    wr8((uint16_t)(CardBlockStub + at + 1), (uint8_t)(value >> 8));
}

static uint16_t word_at(uint16_t at)
{
    return (uint16_t)(rd8(at) | (rd8((uint16_t)(at + 1)) << 8));
}

/* One set of palettes, to the VCE or back: the length into the stub, the VCE's address, the move. */
static void move_colors(uint16_t set, uint8_t palette_high, bool read_back)
{
    uint16_t bytes = (uint16_t)(rd8((uint16_t)(set + 2)) << COLOR_SET_BYTES_SHIFT);
    uint16_t memory = word_at(set);

    put_stub_word(STUB_LENGTH, bytes);
    put_stub_word(read_back ? STUB_DESTINATION : STUB_SOURCE, memory);
    wr8(VceAddressLo, 0);
    wr8(VceAddressHi, palette_high);
    for (uint16_t i = 0; i < bytes; i++) {
        uint16_t port = (i & 1) ? VceDataHi : VceDataLo;

        if (read_back)
            wr8((uint16_t)(memory + i), rd8(port));
        else
            wr8(port, rd8((uint16_t)(memory + i)));
    }
}

/*
 * @card 00:E509 ex_colorcmd
 * WHAT IT DOES: Does what CardColorCommand asks of the palettes, if anything: writes the
 * background and sprite colours to the VCE, or with bit 0 reads them back into memory.
 * HOW: Writes a block move in RAM at CardBlockStub (TIA to the colour data port, or TAI from it)
 * and runs it once for each set whose count is not zero: the background's at colour 0, the
 * sprites' at colour $100, 32 bytes a 16-colour palette. The C makes the moves itself. Then it
 * clears the command.
 * Entry: CardColorCommand, CardColorBackground, CardColorSprites. Exit, no command: A 0, N 0, Z 1.
 * Sprites moved: A 1, the VCE address's high byte it last loaded, N 0, Z 0, C 0. Sprites skipped:
 * A 0, N 0, Z 1, C 0 if the background moved, else as it was. X, Y, V kept.
 */
ColorCommand ex_colorcmd(void)
{
    ColorCommand done;
    bool read_back;

    done.command = rd8(CardColorCommand);
    done.background_sets = rd8((uint16_t)(CardColorBackground + 2));
    done.sprite_sets = rd8((uint16_t)(CardColorSprites + 2));
    if (done.command == 0)
        return done;
    read_back = (done.command & COLOR_READ_BACK) != 0;
    if (read_back) {
        for (unsigned i = 0; i < COLOR_STUB_HEAD; i++)
            wr8((uint16_t)(CardBlockStub + i), rd8((uint16_t)(ColorReadStub + i)));
    } else {
        wr8(CardBlockStub, STUB_TIA);
        put_stub_word(STUB_DESTINATION, VceDataLo);
    }
    wr8((uint16_t)(CardBlockStub + STUB_RETURN), STUB_RTS);
    if (done.background_sets != 0)
        move_colors(CardColorBackground, 0, read_back);
    if (done.sprite_sets != 0)
        move_colors(CardColorSprites, SPRITE_PALETTE_HIGH, read_back);
    wr8(CardColorCommand, 0);
    return done;
}

void abi_ex_colorcmd(void)
{
    ColorCommand done = ex_colorcmd();

    if (done.command == 0 || done.sprite_sets == 0) {
        cpu.a = 0;
        abi_nz(0);
        if (done.command != 0 && done.background_sets != 0)
            abi_flag(P_C, false);
        return;
    }
    cpu.a = SPRITE_PALETTE_HIGH;
    abi_nz(SPRITE_PALETTE_HIGH);
    abi_flag(P_C, false);
}
