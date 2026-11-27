/*
 * The boot program: sector 2, which the boot block (user sector 1) reads to $3000 and runs with MPR2 to MPR6 $80 to
 * $84 (docs/rondo/HANDBOOK.md, Code). It clears the VRAM, asks the System Card its version, and either reads the
 * old-card program (20 sectors from $23 to $4000, system/old_card.c) or, with card 3.0 and its RAM, reads the game's
 * kernel by the load table at $312F and jumps to it. $3000 is base RAM, bank $F8 under MPR1.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "old_card.h"
#include "boot_program.h"

void abi_VecKernelStart(void);   /* system/vectors.c: the kernel's entry at $4000 */

enum {
    MPR2 = 0x04, MPR3 = 0x08, MPR4 = 0x10, MPR5 = 0x20, MPR6 = 0x40,
    CARD_CD_BASE = 0xE006, CARD_CD_READ = 0xE009, CARD_EX_GETVER = 0xE05A, CARD_MA_MUL8U = 0xE0BD,
    CARD_EX_MEMOPEN = 0xE0DE,
    CARD_RAM_BASE = 0xFFF5,            /* the card's byte naming its first CD RAM bank */
    CARD_VERSION_SUPER = 0x03,         /* ex_getver's X from the Super System Card 3.0 on */
    LOAD_TABLE = 0x312F,               /* five bytes a record: the first sector (three bytes, from the disc's
                                        * start), then the address of its list of ops */
    LOAD_RECORD = 0x05,
    TRACK_START = 0x000F32,            /* the data track's first sector, taken off the record's sector */
    OP_BANKS = 0x00,                   /* op 0: MPR4 to MPR6 from the next three bytes (above zRamBaseBank) */
    VDC_MAWR = 0x00, VDC_VWR = 0x02, VDC_CR = 0x05
};

/* Register shims for the card's entries, named for the instructions before each JSR. */
static void lda_shim(uint8_t a)
{
    cpu.a = a;
    abi_nz(a);
}

static void lda_y_shim(uint8_t a, uint8_t y)  /* INY then LDA: N and Z from the load */
{
    cpu.y = y;
    cpu.a = a;
    abi_nz(a);
}

/* Before LoadBootBlocks's cd_read: INY then LDA (zTemp08),Y; C and V stand from the CLA, ADC zBiosCl before them. */
static void cd_read_entry_shim(uint8_t a, Sum last)
{
    lda_y_shim(a, 2);
    abi_flag(P_C, last.carry);
    abi_flag(P_V, last.overflow);
}

/* After cd_read: the Y the card leaves, which nothing after it in LoadBootBlocks changes until the next op. */
static uint8_t cd_read_y_shim(void)
{
    return cpu.y;
}

/* After cd_read: its answer by TAX into X, N and Z from it. */
static uint8_t tax_answer_shim(void)
{
    cpu.x = cpu.a;
    abi_nz(cpu.a);
    return cpu.a;
}

/* After ex_getver: the version in X, compared with 3 (CPX), the carry the answer. */
static bool card_version_shim(void)
{
    abi_nz((uint8_t)(cpu.x - CARD_VERSION_SUPER));
    abi_flag(P_C, cpu.x >= CARD_VERSION_SUPER);
    return cpu.x >= CARD_VERSION_SUPER;
}

/* After ex_memopen: carry set when the card has no RAM to give; A its first bank otherwise. */
static bool memopen_failed_shim(uint8_t *bank)
{
    *bank = cpu.a;
    return (cpu.p & P_C) != 0;
}

/* What the VRAM fill's loops leave: X 0 (the INX's wrap) and Y $FF (the DEY's). */
static void fill_loops_shim(void)
{
    cpu.x = 0x00;
    cpu.y = 0xFF;
}

/* CLA before the JSR to LoadBootBlocks: A 0, flags kept. */
static void cla_shim(void)
{
    cpu.a = 0x00;
}

/* The registers at the JMP to the kernel: A the bank above the base plus one (INC A), N and Z from it, C and V from
 * the ADC #1. */
static void kernel_jump_shim(Sum sum)
{
    cpu.a = (uint8_t)(sum.value + 1);
    abi_nz(cpu.a);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

/* The VDC's address register then the two data ports, straight by ST0, ST1 and ST2. */
static void vdc_select(uint8_t reg)
{
    st0(reg);
}

static void vdc_word(uint8_t low, uint8_t high)
{
    st1(low);
    st2(high);
}

/* The banks the card names at $FFF5 into MPR2 to MPR5, and into zRamBaseBank; A the last bank, N and Z from it. */
static void map_card_banks(void)
{
    uint8_t bank = rd8(CARD_RAM_BASE);

    WR8(ZP->ram_base_bank, bank);
    tam(MPR2, bank);
    tam(MPR3, (uint8_t)(bank + 1));
    tam(MPR4, (uint8_t)(bank + 2));
    tam(MPR5, (uint8_t)(bank + 3));
    lda_shim((uint8_t)(bank + 3));
}

/*
 * @routine 0002:000 BootStart
 * @run F8:3000
 * WHAT IT DOES: The boot program: writes zeros to VRAM $1100 to $110F and the VDC's control register's high byte,
 * fills VRAM from 0 with 4096 words of $0110, asks the card's cd_base twice ($81 and $82 with 2 and $22, 1 and 2),
 * then the card's version. Below 3.0 it maps the CD RAM banks from $FFF5, reads 20 sectors from $23 to $4000 until
 * cd_read answers zero, and jumps to the old-card program at $4000. From 3.0, ex_memopen; when it has no RAM to give
 * the old-card path is taken all the same; otherwise its bank goes to zRamBaseBank, LoadBootBlocks reads the
 * kernel by the first record of the load table, the bank above it goes to MPR2 and the next to MPR3, and it jumps to
 * the kernel's entry at $4000.
 * HOW: ST0, ST1 and ST2 to the VDC; the card's entries with the registers its code is handed, built by shims.
 * WAITS: yes, in the card's entries (cd_read until it answers zero).
 * Entry: from the boot block. Exit: never: a jump to $4000.
 */
void BootStart(void)
{
    uint8_t count, bank, answer;
    Sum sum;

    vdc_select(VDC_MAWR);
    vdc_word(0x00, 0x11);
    vdc_select(VDC_VWR);
    for (count = 0x10; count != 0; count--) {
        vdc_word(0x00, 0x00);
    }
    vdc_select(VDC_CR);
    st2(0x00);
    vdc_select(VDC_MAWR);
    vdc_word(0x00, 0x00);
    vdc_select(VDC_VWR);
    for (int row = 0; row < 0x10; row++) {
        for (int column = 0; column < 0x100; column++) {
            vdc_word(0x10, 0x01);
        }
    }
    fill_loops_shim();
    wr8(zBiosBh, 0x81);
    wr8(zBiosAl, 0x02);
    wr8(zBiosCl, 0x01);
    lda_shim(0x01);
    call_orig(0x3039, CARD_CD_BASE);
    wr8(zBiosBh, 0x82);
    wr8(zBiosAl, 0x22);
    wr8(zBiosCl, 0x02);
    lda_shim(0x02);
    call_orig(0x3048, CARD_CD_BASE);
    call_orig(0x304B, CARD_EX_GETVER);
    if (card_version_shim()) {
        call_orig(0x3087, CARD_EX_MEMOPEN);
        if (!memopen_failed_shim(&bank)) {
            WR8(ZP->ram_base_bank, bank);
            cla_shim();
            if (call_frame(0x308F, abi_LoadBootBlocks))
                return;
            bank = RD8(ZP->ram_base_bank);
            sum = add_bytes(bank, 0x01, false);
            tam(MPR2, sum.value);
            tam(MPR3, (uint8_t)(sum.value + 1));
            kernel_jump_shim(sum);
            abi_VecKernelStart();                  /* JMP $4000 */
            return;
        }
    }
    map_card_banks();
    do {
        wr8(zBiosCl, 0x00);
        wr8(zBiosCh, 0x00);
        wr8(zBiosDl, 0x23);
        wr8(zBiosDh, 0x01);
        wr8(zBiosBl, 0x00);
        wr8(zBiosBh, 0x40);
        wr8(zBiosAl, 0x14);
        lda_shim(0x14);
        call_orig(0x307E, CARD_CD_READ);
        answer = tax_answer_shim();
    } while (answer != 0);
    OldCardMain();                                 /* JMP $3099, JMP $4000 */
}

void abi_BootStart(void)
{
    BootStart();
}

/*
 * @routine 0002:09C LoadBootBlocks
 * @run F8:309C
 * WHAT IT DOES: Reads the blocks of the load table's record A (five bytes a record at $312F: the first sector, three
 * bytes from the disc's start, then the address of the record's ops). The ops, until one with bit 7 set: 0 sets
 * MPR4 to MPR6 from the next three bytes above zRamBaseBank (four bytes); any other reads, by the card's cd_read
 * until it answers zero, the count of sectors in its third byte from the record's sector plus its second byte, to
 * $8000 under those banks (three bytes).
 * HOW: ma_mul8u for the record's offset (A times 5), zTemp00-01 the record, zTemp08-09 the op; the sector less
 * $F32 into zBiosCl, Ch and Dl, zBiosDh 1, zBiosB $8000, zBiosAl the count.
 * WAITS: yes, in the card's cd_read.
 * Entry: A the record. Exit: A the op that ended the list, N and Z from it; Y as the last op left it; C and V from
 * the last pointer add; X as the card's last call left it (0 after a read).
 */
BootLoad LoadBootBlocks(uint8_t record)
{
    BootLoad r = { 0 };
    uint8_t op, value, answer;
    Sum sum;

    wr8(zBiosAl, record);
    wr8(zBiosBl, LOAD_RECORD);
    lda_shim(LOAD_RECORD);
    call_orig(0x30A5, CARD_MA_MUL8U);
    sum = add_bytes((uint8_t)LOAD_TABLE, rd8(zBiosCl), false);
    WR8(ZP->temp00, sum.value);
    sum = add_bytes((uint8_t)(LOAD_TABLE >> 8), rd8(zBiosCh), sum.carry);
    WR8(ZP->temp01, sum.value);
    r.last = sum;
    value = RD8_PTR_Y(ZP->temp00, 3);
    WR8(ZP->temp08, value);
    value = RD8_PTR_Y(ZP->temp00, 4);
    WR8(ZP->temp09, value);
    r.y = 4;
    for (;;) {
        uint8_t step;

        op = RD8_PTR(ZP->temp08);
        if ((op & 0x80) != 0) {
            r.end = op;
            return r;
        }
        if (op == OP_BANKS) {
            value = RD8_PTR_Y(ZP->temp08, 1);
            sum = add_bytes(value, RD8(ZP->ram_base_bank), false);
            tam(MPR4, sum.value);
            value = RD8_PTR_Y(ZP->temp08, 2);
            sum = add_bytes(value, RD8(ZP->ram_base_bank), false);
            tam(MPR5, sum.value);
            value = RD8_PTR_Y(ZP->temp08, 3);
            sum = add_bytes(value, RD8(ZP->ram_base_bank), false);
            tam(MPR6, sum.value);
            r.y = 3;
            step = 4;
        } else {
            do {
                value = RD8_PTR_Y(ZP->temp00, 0);
                sum = sub_bytes(value, (uint8_t)TRACK_START, true);
                wr8(zBiosDl, sum.value);
                value = RD8_PTR_Y(ZP->temp00, 1);
                sum = sub_bytes(value, (uint8_t)(TRACK_START >> 8), sum.carry);
                wr8(zBiosCh, sum.value);
                value = RD8_PTR_Y(ZP->temp00, 2);
                sum = sub_bytes(value, (uint8_t)(TRACK_START >> 16), sum.carry);
                wr8(zBiosCl, sum.value);
                wr8(zBiosDh, 0x01);
                wr8(zBiosBl, 0x00);
                wr8(zBiosBh, 0x80);
                value = RD8_PTR_Y(ZP->temp08, 1);
                sum = add_bytes(value, rd8(zBiosDl), false);
                wr8(zBiosDl, sum.value);
                sum = add_bytes(0, rd8(zBiosCh), sum.carry);
                wr8(zBiosCh, sum.value);
                sum = add_bytes(0, rd8(zBiosCl), sum.carry);
                wr8(zBiosCl, sum.value);
                value = RD8_PTR_Y(ZP->temp08, 2);
                wr8(zBiosAl, value);
                cd_read_entry_shim(value, sum);
                call_orig(0x3128, CARD_CD_READ);
                answer = tax_answer_shim();
                r.y = cd_read_y_shim();
            } while (answer != 0);
            step = 3;
        }
        sum = add_bytes(step, RD8(ZP->temp08), false);
        WR8(ZP->temp08, sum.value);
        sum = add_bytes(0, RD8(ZP->temp09), sum.carry);
        WR8(ZP->temp09, sum.value);
        r.last = sum;
    }
}

void abi_LoadBootBlocks(void)
{
    BootLoad r = LoadBootBlocks(cpu.a);

    cpu.a = r.end;
    cpu.y = r.y;
    abi_nz(r.end);
    abi_flag(P_C, r.last.carry);
    abi_flag(P_V, r.last.overflow);
}
