/*
 * The disc's own loader: the code of user sector 0 that the System Card runs from $2B26 once it has read sectors 0
 * and 1 to $2800-$37FF (docs/rondo/HANDBOOK.md, Disc). It checks the disc's signature, shows or reads what the
 * information block of sector 1 (at $3000) names, maps the program's banks, reads the program the block names and
 * jumps to it. For this disc that program is the boot program, sector 2 at $3000 (system/boot_program.c).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"

/* The information block, sector 1 as loaded, and the card's bytes the loader reads and writes. */
enum {
    IPL_PROGRAM = 0x3000,              /* eight bytes: sector (high first), count, load address, run address */
    IPL_BANKS = 0x3008,                /* the banks for MPR2 to MPR6, counted from the card's CD RAM base */
    IPL_FLAGS = 0x300D,
    IPL_DATA_SECTOR = 0x300E,          /* high, middle, low */
    IPL_DATA_MODE = 0x3011,            /* and the sector count of the third data read */
    IPL_DATA_ADDRESS = 0x3012,         /* two bytes */
    IPL_ADPCM_SECTOR = 0x3014,         /* high, middle, low */
    IPL_ADPCM_COUNT = 0x3017,
    IPL_ADPCM_RATE = 0x3018,
    IPL_SIGNATURE = 0x3020,            /* compared with the card's own copy */
    CARD_SIGNATURE = 0xDFB6,
    SIGNATURE_BYTES = 0x4A,
    CARD_RAM_BASE = 0xFFF5,
    CARD_ARG_221F = 0x221F, CARD_ARG_2220 = 0x2220, CARD_ARG_2221 = 0x2221, CARD_ARG_2222 = 0x2222,
    CARD_ARG_2223 = 0x2223, CARD_ARG_2224 = 0x2224, CARD_ARG_2225 = 0x2225,
    CARD_CD_READ = 0xE009, CARD_AD_TRANS = 0xE033, CARD_AD_CPLAY = 0xE03C, CARD_E08A = 0xE08A, CARD_E099 = 0xE099,
    STACK_COPY = 0xDFE0,               /* where the last read is run from when it would load over this code */
    STACK_COPY_BYTES = 0x15
};

/* The registers the loader holds where it calls the card or moves a block: each shim is the instructions before. */
static void ipl_regs_shim(uint8_t a, uint8_t x, uint8_t y, uint8_t nz)
{
    cpu.a = a;
    cpu.x = x;
    cpu.y = y;
    abi_nz(nz);
}

static void ipl_a_shim(uint8_t a)                /* LDA: A, N and Z */
{
    cpu.a = a;
    abi_nz(a);
}

static void ipl_carry_shim(bool carry)
{
    abi_flag(P_C, carry);
}

static void ipl_sum_shim(uint8_t a, uint8_t operand, uint8_t result, bool carry)     /* LDA, CLC or not, ADC */
{
    cpu.a = result;
    abi_nz(result);
    abi_flag(P_C, carry);
    abi_flag(P_V, ((operand ^ result) & (a ^ result) & 0x80) != 0);
}

/* TII: the CPU pushes Y, A and X, moves the bytes up from src to dst, and pulls them. */
static void ipl_tii_shim(uint16_t src, uint16_t dst, uint16_t len)
{
    uint16_t i;

    push8(cpu.y);
    push8(cpu.a);
    push8(cpu.x);
    for (i = 0; i < len; i++) {
        uint8_t byte = rd8((uint16_t)(src + i));

        wr8((uint16_t)(dst + i), byte);
    }
    (void)pull8();
    (void)pull8();
    (void)pull8();
}

/* The A the card's call left, and the compare with 0 the loader makes of it. */
static uint8_t ipl_answer_shim(void)
{
    abi_nz(cpu.a);
    abi_flag(P_C, true);
    return cpu.a;
}

/* LDX #$FF, TXS: the stack emptied. */
static void ipl_reset_stack_shim(void)
{
    cpu.x = 0xFF;
    cpu.s = 0xFF;
    abi_nz(0xFF);
}

/* One of the three adds that make the sector of a data read: the information block's byte plus the carry in. */
static bool ipl_sector_byte(uint16_t source, uint8_t add, bool carry, uint8_t *out_register)
{
    uint8_t value = rd8(source);
    unsigned total = (unsigned)value + add + (carry ? 1u : 0u);
    uint8_t result = (uint8_t)total;

    ipl_sum_shim(value, add, result, total > 0xFF);
    *out_register = result;
    return total > 0xFF;
}

/* The data sector plus `add` into the card's CL, CH and DL, low byte first. */
static void ipl_data_sector(uint8_t add)
{
    uint8_t byte;
    bool carry;

    carry = ipl_sector_byte(IPL_DATA_SECTOR + 2, add, false, &byte);
    wr8(zBiosDl, byte);
    carry = ipl_sector_byte(IPL_DATA_SECTOR + 1, 0, carry, &byte);
    wr8(zBiosCh, byte);
    (void)ipl_sector_byte(IPL_DATA_SECTOR, 0, carry, &byte);
    wr8(zBiosCl, byte);
}

/* The three data reads that bit 0 of the working flags asks for; true where the original goes on through $2BFA, which
 * clears the bit: a count out of range, or a failed first or third read. A failed second read goes on past it. */
static bool ipl_load_data(void)
{
    uint8_t mode, byte;

    call_orig(0x2B55, CARD_E099);
    mode = rd8(IPL_DATA_MODE);
    ipl_a_shim(mode);
    if (mode == 0) {
        return true;
    }
    ipl_carry_shim(mode >= 0x22);
    if (mode >= 0x22) {
        return true;
    }
    ipl_tii_shim(IPL_DATA_SECTOR, 0x2000 + 0xFC, 3);
    wr8(zBiosDh, 0);
    wr8(zBiosBl, 0);
    wr8(zBiosBh, 0x38);
    wr8(zBiosAl, 0);
    wr8(zBiosAh, 0x02);
    ipl_a_shim(0x02);
    call_orig(0x2B78, CARD_CD_READ);
    if (ipl_answer_shim() != 0) {
        return true;
    }
    wr8(CARD_ARG_2222, 0x10);
    wr8(CARD_ARG_2225, 0x10);
    wr8(CARD_ARG_2220, 0x00);
    wr8(CARD_ARG_2221, 0x38);
    wr8(CARD_ARG_2223, 0x00);
    wr8(CARD_ARG_2224, 0x39);
    wr8(CARD_ARG_221F, 0x02);
    ipl_data_sector(1);
    wr8(zBiosDh, 0xFF);
    wr8(zBiosAl, 0x01);
    wr8(zBiosBl, 0);
    wr8(zBiosBh, 0);
    ipl_a_shim(0x01);
    call_orig(0x2BC4, CARD_CD_READ);
    if (ipl_answer_shim() != 0) {
        return false;
    }
    ipl_data_sector(2);
    wr8(zBiosDh, 0xFF);
    byte = rd8(IPL_DATA_MODE);
    wr8(zBiosAl, byte);
    byte = rd8(IPL_DATA_ADDRESS);
    wr8(zBiosBl, byte);
    byte = rd8(IPL_DATA_ADDRESS + 1);
    wr8(zBiosBh, byte);
    ipl_a_shim(byte);
    call_orig(0x2BF6, CARD_CD_READ);
    return ipl_answer_shim() != 0;
}

/* The ADPCM read and play that bit 1 of the working flags asks for; it stops at the first thing that fails. */
static void ipl_load_adpcm(void)
{
    uint8_t count, doubled, byte;

    ipl_tii_shim(IPL_ADPCM_SECTOR, 0x2000 + 0xFC, 3);
    count = rd8(IPL_ADPCM_COUNT);
    ipl_a_shim(count);
    if (count == 0) {
        return;
    }
    abi_nz((uint8_t)(count - 0x21));
    ipl_carry_shim(count >= 0x21);
    if (count >= 0x21) {
        return;
    }
    wr8(zBiosAl, count);
    wr8(zBiosDh, 0);
    wr8(zBiosBl, 0);
    wr8(zBiosBh, 0);
    call_orig(0x2C1A, CARD_AD_TRANS);
    if (ipl_answer_shim() != 0) {
        return;
    }
    if (RD8(ZP->temp00) & 0x40) {
        return;
    }
    wr8(zBiosBl, 0);
    wr8(zBiosBh, 0);
    wr8(zBiosAl, 0);
    count = rd8(IPL_ADPCM_COUNT);
    doubled = (uint8_t)(count << 3);
    ipl_a_shim(doubled);
    ipl_carry_shim((count & 0x20) != 0);
    if (doubled == 0) {
        doubled = 0xFF;
        ipl_a_shim(doubled);
        wr8(zBiosAl, doubled);
    }
    wr8(zBiosAh, doubled);
    byte = rd8(IPL_ADPCM_RATE);
    wr8(zBiosDh, byte);
    byte = (uint8_t)(RD8(ZP->temp00) & 0x80);
    ipl_a_shim(byte);
    wr8(zBiosDl, byte);
    call_orig(0x2C43, CARD_AD_CPLAY);
}

/*
 * @routine 0000:326 IplStart
 * @run F8:2B26
 * WHAT IT DOES: The disc's loader. Returns to the card unless the $4A bytes at $3020 match the card's at $DFB6. The
 * working flags (zTemp00) are the information block's, or $60 when it has none. With bit 0 set it reads the block's
 * graphics data by three cd_reads (bit 0 is dropped when one fails or the count is out of range); with bit 1 set it
 * reads the block's ADPCM and starts it; with bit 5 clear and bit 0 set it calls the card's $E08A. Then it maps I/O,
 * work RAM and the block's five banks, and reads the program the block names: with no sector count it jumps to the
 * run address plus $3000; else it reads to the load address and jumps to the run address, from a copy of its last
 * $15 bytes at $DFE0 when the load address is below $3000. A failed program read returns to the card.
 * HOW: The compare loop; the flags; ipl_load_data and ipl_load_adpcm are the two optional parts of the original's
 * straight line; TAM for each bank; TII for the eight-byte block; the stack emptied and a jump through $2007.
 * WAITS: yes, in the card's calls (cd_read on the drive); the stack there is the card's return and this call's two bytes
 * (more inside TII).
 * Entry: from the card, the information block at $3000. Exit: to the program, with the stack emptied; or RTS to the
 * card (signature: A the byte, X and Y the loop's, flags from the compare; failed read: A the card's answer).
 */
void IplStart(void)
{
    uint8_t flags, value, other, y, x, byte;
    uint8_t count, load_lo, load_hi, difference;
    bool carry;
    uint16_t target, ret;

    y = 0;
    x = SIGNATURE_BYTES;
    WR8(ZP->bios_b.lo, 0x20);                       /* the pointers: zp $FA (to $3020) and $FC (to $DFB6) */
    WR8(ZP->bios_b.hi, 0x30);
    WR8(ZP->bios_c.lo, 0xB6);
    WR8(ZP->bios_c.hi, 0xDF);
    do {
        value = RD8_PTR_Y(ZP->bios_b.lo, y);
        other = RD8_PTR_Y(ZP->bios_c.lo, y);
        if (value != other) {
            ipl_regs_shim(value, x, y, (uint8_t)(value - other));
            ipl_carry_shim(value >= other);
            return;
        }
        y++;
        x--;
    } while (x != 0);
    ipl_carry_shim(true);                           /* the last compare was equal */
    flags = rd8(IPL_FLAGS);
    if (flags == 0) {
        flags = 0x60;
    }
    WR8(ZP->temp00, flags);
    ipl_regs_shim(flags, x, y, flags);
    if (RD8(ZP->temp00) & 0x01) {
        if (ipl_load_data()) {
            RMW8(ZP->temp00, old_, old_ & 0xFE);
        }
    }
    if (RD8(ZP->temp00) & 0x02) {
        ipl_load_adpcm();
    }
    if (!(RD8(ZP->temp00) & 0x20)) {
        if (RD8(ZP->temp00) & 0x01) {
            call_orig(0x2C4C, CARD_E08A);
        }
    }
    ipl_a_shim(0xFF);
    tam(0x01, 0xFF);
    ipl_a_shim(0xF8);
    tam(0x02, 0xF8);
    for (byte = 0; byte < 5; byte++) {
        uint8_t bank = rd8((uint16_t)(IPL_BANKS + byte));
        uint8_t base = rd8(CARD_RAM_BASE);
        unsigned total = (unsigned)bank + base;

        ipl_sum_shim(bank, base, (uint8_t)total, total > 0xFF);
        tam((uint8_t)(0x04 << byte), (uint8_t)total);
    }
    ipl_tii_shim(IPL_PROGRAM, 0x2001, 8);
    count = RD8(ZP->temp04);
    ipl_a_shim(count);
    ret = 0x2CCA;
    if (count == 0) {
        value = RD8(ZP->temp07);
        WR8(ZP->temp07, value);
        value = RD8(ZP->temp08);
        byte = (uint8_t)(value + 0x30);
        ipl_sum_shim(value, 0x30, byte, value >= 0xD0);
        WR8(ZP->temp08, byte);
    } else {
        ipl_tii_shim(0x2001, 0x2000 + 0xFC, 3);
        wr8(zBiosDh, 0x01);
        value = RD8(ZP->temp04);
        wr8(zBiosAl, value);
        wr8(zBiosAh, 0);
        load_lo = RD8(ZP->temp05);
        carry = 0xFF >= load_lo;
        load_hi = RD8(ZP->temp06);
        difference = (uint8_t)(0x2F - load_hi - (carry ? 0 : 1));
        ipl_a_shim(difference);
        ipl_carry_shim(0x2F >= (unsigned)load_hi + (carry ? 0u : 1u));
        abi_flag(P_V, ((0x2F ^ load_hi) & (0x2F ^ difference) & 0x80) != 0);
        if (0x2F >= (unsigned)load_hi + (carry ? 0u : 1u)) {
            ipl_tii_shim(0x2CC0, STACK_COPY, STACK_COPY_BYTES);
            ret = 0xDFEA;                           /* the same instructions, run from the copy */
        }
        ipl_tii_shim(0x2005, 0x2000 + 0xFA, 2);
        call_orig(ret, CARD_CD_READ);
        if (ipl_answer_shim() != 0) {
            return;
        }
    }
    ipl_reset_stack_shim();
    target = RD8(ZP->temp07);
    target = (uint16_t)(target | (RD8(ZP->temp08) << 8));
    run_routine(target);                            /* JMP ($2007) */
}

void abi_IplStart(void)
{
    IplStart();
}
