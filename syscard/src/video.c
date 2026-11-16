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
    uint8_t bits = size_bits(cpu.a);
    uint8_t width = ex_scrsiz(cpu.a);

    cpu.a = width;
    cpu.y = bits;
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
