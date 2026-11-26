/*
 * The card's check of the disc at start: the first data track found, its first two sectors read to $2800 and compared with
 * the card's own copy of the title, and the program in them run.
 *
 * No record yet (row request to the typed access session): the byte at $2269 that keeps the data track's number.
 */
#include "card.h"
#include "card_access.h"
#include "syscard.h"

enum {
    DISC_INFO_TRACK = 0x22, DISC_INFO_BUFFER_LOW = 0x56, DISC_INFO_BUFFER_HIGH = 0x22, DISC_INFO_WHAT = 2,
    TRACK_DATA_BIT = 0x04, DATA_TRACK_AT = 0x2269, SAVED_SECTOR_FROM = 0x20F8, READ_BUFFER_LOW = 0x00, READ_BUFFER_HIGH = 0x28,
    READ_MODE_SECTORS = 1, READ_SECTORS = 2, TITLE_MPR = 0x40, TITLE_BANK = 0x1F, TITLE_AT_LOW = 0xE1, TITLE_AT_HIGH = 0xDA,
    TITLE_BYTES_LOW = 0xD5, TITLE_BYTES_HIGH = 0x04, ZERO_BYTES = 0x4A, BOOT_PROGRAM = 0x2B26, BOOT_PROGRAM_RET = 0xE209
};

/* ADC #1 with the decimal flag set, as the track number counts on. */
static uint8_t next_track(uint8_t track)
{
    uint8_t low = (uint8_t)((track & 0x0F) + 1);
    uint8_t high = (uint8_t)(track >> 4);

    if (low > 9) {
        low = (uint8_t)(low - 10);
        high++;
    }
    if (high > 9)
        high = (uint8_t)(high - 10);
    return (uint8_t)((high << 4) | low);
}

/* TII: Y, A and X pushed, the bytes moved up one at a time, X, A and Y pulled. */
static void block_move_tii(uint16_t src, uint16_t dst, uint16_t length, uint8_t a, uint8_t x, uint8_t y)
{
    push8(y);
    push8(a);
    push8(x);
    for (uint16_t i = 0; i < length; i++) {
        uint8_t byte = rd8((uint16_t)(src + i));

        wr8((uint16_t)(dst + i), byte);
    }
    (void)pull8();
    (void)pull8();
    (void)pull8();
}

/* INC pointer, INC pointer+1 if it wrapped: a pair of zero page bytes. */
static void step_pointer(void)
{
    if (INC8(CZP->pointer.lo) == 0)
        (void)INC8(CZP->pointer.hi);
}

static void step_copy(void)
{
    if (INC8(CZP->copy.lo) == 0)
        (void)INC8(CZP->copy.hi);
}

/* The count in _CX less one, and what is left (LDA, ORA). */
static uint8_t count_down_cx(void)
{
    uint8_t low = RD8(CZP->c.lo);
    uint8_t high;

    if (low == 0)
        (void)DEC8(CZP->c.hi);
    (void)DEC8(CZP->c.lo);
    low = RD8(CZP->c.lo);
    high = RD8(CZP->c.hi);
    return (uint8_t)(low | high);
}

/* The jump into the program the disc's first sectors hold: original code with no C, its registers as the routine has them. */
static void boot_program_shim(uint8_t x, uint8_t y)
{
    cpu.a = 0;
    cpu.x = x;
    cpu.y = y;
    abi_nz(0);
    call_orig(BOOT_PROGRAM_RET, BOOT_PROGRAM);
}

/*
 * @card 00:E14C check_boot_disc
 * WHAT IT DOES: Finds the disc's first data track, reads its first two sectors to $2800, checks them against the card's own
 * copy of the title and that the next $4A bytes are zero, and runs the program there.
 * HOW: cd_dinfo for each track from CardDiscFirstTrack on, up to the last, until one's answer has the data bit; its number
 * is kept and msf_to_lba makes its start a sector, which is saved twice in the base table; cd_read two sectors to $2800 with the
 * card's MPR 6 bank $1F mapped; compares $4D5 bytes, then $4A zero ones, then JSR $2B26.
 * WAITS: through cd_dinfo and cd_read, and in the program it runs.
 * Entry: A, X, Y (pushed by the block moves). Exit: no data track: C 0, A the last track, N, Z from comparing it with the
 * last, V bit 6 of the answer's last control byte. A byte differs: C 1, A that byte, N and Z from the difference. Else C 1
 * after the program returns.
 */
CardExit check_boot_disc(uint8_t entry_a, uint8_t entry_x, uint8_t entry_y)
{
    CardExit e = { 0 };
    CardExit step;
    uint8_t a;
    uint8_t x = entry_x;
    uint8_t y = entry_y;
    uint8_t value;
    uint8_t last;
    uint8_t control;
    uint8_t other;

    a = rd8(CardDiscFirstTrack);
    for (;;) {
        push8(a);
        WR8(CZP->a.hi, a);
        WR8(CZP->a.lo, DISC_INFO_WHAT);
        WR8(CZP->b.lo, DISC_INFO_BUFFER_LOW);
        WR8(CZP->b.hi, DISC_INFO_BUFFER_HIGH);
        jsr_push(0xE161);
        step = cd_dinfo(DISC_INFO_TRACK, x);
        jsr_pop();
        e = card_exit_over(e, step);
        if (step.set_x)
            x = step.x;
        if (step.set_y)
            y = step.y;
        a = pull8();
        control = RD8_IDX(COL(CRAM->cd_result), 3);
        if ((control & TRACK_DATA_BIT) != 0)
            break;
        last = rd8((uint16_t)(CardDiscFirstTrack + 1));
        if (a >= last) {
            e.a = a;
            e.nz = (uint8_t)(a - last);
            e.c = false;
            e.v = (control & 0x40) != 0;
            e.set_a = e.set_nz = e.set_c = e.set_v = true;
            return e;
        }
        a = next_track(a);
    }
    wr8(DATA_TRACK_AT, a);
    jsr_push(0xE17C);
    step = msf_to_lba(x);
    jsr_pop();
    e = card_exit_over(e, step);
    if (step.set_a)
        a = step.a;
    block_move_tii(SAVED_SECTOR_FROM, (uint16_t)(CardCdBases), 3, a, x, y);
    block_move_tii(SAVED_SECTOR_FROM, (uint16_t)(CardCdBases + 3), 3, a, x, y);
    WR8(CZP->c.lo, 0);
    WR8(CZP->c.hi, 0);
    WR8(CZP->d.lo, 0);
    WR8(CZP->b.lo, READ_BUFFER_LOW);
    WR8(CZP->b.hi, READ_BUFFER_HIGH);
    WR8(CZP->d.hi, READ_MODE_SECTORS);
    WR8(CZP->a.lo, READ_SECTORS);
    WR8(CZP->a.hi, 0);
    jsr_push(0xE1A4);
    step = cd_read(READ_SECTORS, x);
    jsr_pop();
    e = card_exit_over(e, step);
    if (step.set_x)
        x = step.x;
    if (step.set_y)
        y = step.y;
    tam(TITLE_MPR, TITLE_BANK);
    WR8(CZP->pointer.lo, READ_BUFFER_LOW);
    WR8(CZP->pointer.hi, READ_BUFFER_HIGH);
    WR8(CZP->copy.lo, TITLE_AT_LOW);
    WR8(CZP->copy.hi, TITLE_AT_HIGH);
    WR8(CZP->c.lo, TITLE_BYTES_LOW);
    WR8(CZP->c.hi, TITLE_BYTES_HIGH);
    for (;;) {
        value = RD8_PTR(CZP->pointer.lo);
        other = RD8_PTR(CZP->copy.lo);
        if (value != other) {
            e.a = value;
            e.nz = (uint8_t)(value - other);
            e.set_a = e.set_nz = true;
            e.c = true;
            e.set_c = true;
            return e;
        }
        step_pointer();
        step_copy();
        if (count_down_cx() == 0)
            break;
    }
    WR8(CZP->c.lo, ZERO_BYTES);
    WR8(CZP->c.hi, 0);
    for (;;) {
        value = RD8_PTR(CZP->pointer.lo);
        if (value != 0) {
            e.a = e.nz = value;
            e.set_a = e.set_nz = true;
            e.c = true;
            e.set_c = true;
            return e;
        }
        step_pointer();
        step_copy();
        if (count_down_cx() == 0)
            break;
    }
    boot_program_shim(x, y);
    e.c = true;
    e.set_c = true;
    return e;
}

void abi_check_boot_disc(void)
{
    card_exit_apply(check_boot_disc(cpu.a, cpu.x, cpu.y));
}
