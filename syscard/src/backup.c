/*
 * The backup memory: 2 KB of battery RAM in a bank of its own, opened by a port.
 *
 * Every byte is a field of a record: the zero page, the card's RAM, the backup memory and the ROM tables.
 */
#include "card.h"
#include "card_access.h"
#include "syscard.h"

enum {
    BACKUP_BANK = 0xF7, BACKUP_MPR = 0x10, BACKUP_OPEN = 0x80, MPR_COUNT = 8,
    BACKUP_SIGNATURE_BYTES = 4, BACKUP_PASSWORD_BYTES = 11, BACKUP_HEADER_BYTES = 0x12,
    BACKUP_START_HI = 0x80, BACKUP_FIRST_FILE_LO = 0x10, BACKUP_FIRST_FILE_HI = 0x80, BACKUP_MAX_PAGES = 0x20,
    BM_NOT_FORMATTED = 0xFF, BM_BAD_PASSWORD = 1, WINDOW_MPR_LOW = 0x20, WINDOW_MPR_HIGH = 0x40,
    WINDOW_OFFSET_BITS = 0x1F, WINDOW_BASE_HI = 0xA0, FILE_SUM_AT = 2, FILE_NAME_AT = 4, FILE_NAME_BYTES = 12,
    FILE_HEADER_BYTES = 0x10, BM_DONE = 0, BM_NO_FILE = 1, BM_BAD_SUM = 2,
    VDC_REG_CR = 5, VDC_CR_VBLANK_IRQ = 0x08, VDC_STATUS_VBLANK = 0x20, SUM_START = 4
};

/* What a later step leaves over what an earlier one left: the registers and flags it sets win. */
static CardExit exit_over(CardExit base, CardExit top)
{
    if (top.set_a) {
        base.a = top.a;
        base.set_a = true;
    }
    if (top.set_x) {
        base.x = top.x;
        base.set_x = true;
    }
    if (top.set_y) {
        base.y = top.y;
        base.set_y = true;
    }
    if (top.set_nz) {
        base.nz = top.nz;
        base.set_nz = true;
    }
    if (top.set_c) {
        base.c = top.c;
        base.set_c = true;
    }
    if (top.set_v) {
        base.v = top.v;
        base.set_v = true;
    }
    return base;
}

/*
 * @card 00:FC49 bm_open
 * WHAT IT DOES: Opens the backup memory at $8000, keeping every MPR.
 * HOW: A is pushed, the eight MPRs go to CardSavedMprs, its bank to MPR4, slow speed, interrupts
 * off, the unlock port, and A is pulled.
 * WAITS: no.
 * Entry: A (only kept). Exit: A as given, N and Z from it, I set, the CPU on its slow clock. X, Y,
 * C, V kept. Returns the A it pulled.
 */
uint8_t bm_open(uint8_t a)
{
    push8(a);
    for (unsigned i = 0; i < MPR_COUNT; i++) {
        WR8_IDX(COL(CRAM->saved_mprs), i, tma((uint8_t)(1u << i)));
    }
    tam(BACKUP_MPR, BACKUP_BANK);
    csl();
    sei();
    WR8(CCDU->subcode, BACKUP_OPEN);
    return pull8();
}

void abi_bm_open(void)
{
    cpu.a = bm_open(cpu.a);
    abi_nz(cpu.a);
}

/*
 * @card 00:FC7F bm_close
 * WHAT IT DOES: Closes the backup memory and puts back every MPR.
 * HOW: A is pushed; the eight MPRs back; a read of the status port locks it; full speed; then waits
 * out a vertical blank with its interrupt on, so that none was lost, and puts the VDC back; A is
 * pulled and interrupts are allowed.
 * WAITS: yes, for the vertical blank, polling the VDC's status with interrupts still off; the stack
 * there is the caller's frames and A.
 * Entry: A (only kept). Exit: A as given, N and Z from it, I clear, the CPU on its fast clock. X, Y,
 * C, V kept. Returns the A it pulled.
 */
uint8_t bm_close(uint8_t a)
{
    uint8_t control;

    push8(a);
    for (unsigned i = 0; i < MPR_COUNT; i++) {
        uint8_t bank = RD8_IDX(COL(CRAM->saved_mprs), i);

        tam((uint8_t)(1u << i), bank);
    }
    (void)RD8(CCDU->irq_status);
    csh();
    WR8(CVDC->addr, VDC_REG_CR);
    control = RD8(CZP->vdc_cr.lo);
    WR8(CVDC->data_lo, (uint8_t)(control | VDC_CR_VBLANK_IRQ));
    while (RD8(CVDC->addr) & VDC_STATUS_VBLANK) {
    }
    while (!(RD8(CVDC->addr) & VDC_STATUS_VBLANK)) {
    }
    control = RD8(CZP->vdc_cr.lo);
    WR8(CVDC->data_lo, control);
    control = RD8(CZP->vdc_reg);
    WR8(CVDC->addr, control);
    a = pull8();
    cli();
    return a;
}

void abi_bm_close(void)
{
    cpu.a = bm_close(cpu.a);
    abi_nz(cpu.a);
}

/*
 * @card 00:FC32 bm_open_formatted
 * WHAT IT DOES: Opens the backup memory if it has been formatted; if not, closes it and has
 * its caller return $FF.
 * HOW: Opens, then compares the header's first four bytes with the signature, from the last. On a
 * mismatch it closes and drops its own return address with two pulls, so the RTS after them returns
 * to its caller's caller: its caller must return at once and not pop the frame it pushed.
 * WAITS: only through bm_close, on the not formatted path.
 * Entry: A (only kept). Exit, formatted: A the first byte, N 1, Z 0, C 1, X $FF, I set. Exit, not: its
 * caller's caller is next, A $FF, N 1, Z 0, X where it differed, C from that compare, I clear.
 * Not run by any script: the not formatted path.
 */
BackupCheck bm_open_formatted(uint8_t a)
{
    BackupCheck check = { true, { 0 } };
    uint8_t read = 0;
    uint8_t expected = 0;
    int x = BACKUP_SIGNATURE_BYTES - 1;

    jsr_push(0xFC34);
    (void)bm_open(a);
    jsr_pop();
    do {
        read = RD8_IDX(COL(CBM->header), x);
        expected = RD8_IDX(COL(CROMT->signature), x);
        if (read != expected) {
            jsr_push(0xFC44);
            (void)bm_close(read);
            jsr_pop();
            (void)pull8();
            (void)pull8();
            check.formatted = false;
            check.exit.a = BM_NOT_FORMATTED;
            check.exit.x = (uint8_t)x;
            check.exit.nz = BM_NOT_FORMATTED;
            check.exit.c = read >= expected;
            check.exit.set_a = check.exit.set_x = check.exit.set_nz = check.exit.set_c = true;
            return check;
        }
        x--;
    } while (x >= 0);
    check.exit.a = read;
    check.exit.x = 0xFF;
    check.exit.nz = 0xFF;
    check.exit.c = true;
    check.exit.set_a = check.exit.set_x = check.exit.set_nz = check.exit.set_c = true;
    return check;
}

void abi_bm_open_formatted(void)
{
    BackupCheck check = bm_open_formatted(cpu.a);

    card_exit_apply(check.exit);
    abi_flag(P_I, check.formatted);
}

/* Maps the two banks a caller's own pages were at into $A000-$DFFF, and moves its pointer there. */
typedef struct {
    uint8_t window;     /* the saved MPR chosen: the pointer's top three bits */
    uint8_t high;       /* the pointer's new high byte */
    bool bit4;          /* bit 4 of the pointer's old high byte */
} BackupWindow;

/*
 * @card 00:FD7B bm_map_name_window
 * WHAT IT DOES: Makes the pointer in CardCopyLo reach what it pointed to before the backup
 * memory was opened.
 * HOW: Its top three bits choose the saved MPR; the banks there and after it go at $A000, and
 * the pointer is rebased there.
 * WAITS: no.
 * Entry: CardCopyLo-Hi. Exit: A the pointer's new high byte, N and Z from it, X the saved MPR,
 * C bit 4 of the old high byte. Y, V kept.
 */
static BackupWindow bm_map_name_window(void)
{
    BackupWindow w;
    uint8_t high = RD8(CZP->copy.hi);
    uint8_t bank;

    w.bit4 = (high & 0x10) != 0;
    w.window = (uint8_t)(high >> 5);
    bank = RD8_IDX(COL(CRAM->saved_mprs), w.window);
    tam(WINDOW_MPR_LOW, bank);
    bank = RD8_IDX(COL(CRAM->saved_mprs), 1 + w.window);
    tam(WINDOW_MPR_HIGH, bank);
    high = RD8(CZP->copy.hi);
    w.high = (uint8_t)((high & WINDOW_OFFSET_BITS) | WINDOW_BASE_HI);
    WR8(CZP->copy.hi, w.high);
    return w;
}

void abi_bm_map_name_window(void)
{
    BackupWindow w = bm_map_name_window();

    cpu.x = w.window;
    cpu.a = w.high;
    abi_nz(w.high);
    abi_flag(P_C, w.bit4);
}

/* The exit of a routine that ends having mapped a window: A, N, Z from the pointer, X, C. */
static CardExit window_exit(BackupWindow w)
{
    CardExit e = { 0 };

    e.a = e.nz = w.high;
    e.x = w.window;
    e.c = w.bit4;
    e.set_a = e.set_nz = e.set_x = e.set_c = true;
    return e;
}

/*
 * @card 00:FD97 bm_count_down
 * WHAT IT DOES: Takes one from the count of bytes left.
 * HOW: A 16-bit decrement of CardByteCount.
 * WAITS: no.
 * Entry: CardByteCount. Exit: A its two bytes ORed, N and Z from that. X, Y, C, V kept. Returns A.
 * Not run by any script: a low byte of zero, which takes one from the high byte.
 */
uint8_t bm_count_down(void)
{
    uint8_t low = RD8(CRAM->byte_count.lo);
    uint8_t high;

    if (low == 0) {
        (void)DEC8(CRAM->byte_count.hi);
    }
    (void)DEC8(CRAM->byte_count.lo);
    low = RD8(CRAM->byte_count.lo);
    high = RD8(CRAM->byte_count.hi);
    return (uint8_t)(low | high);
}

void abi_bm_count_down(void)
{
    cpu.a = bm_count_down();
    abi_nz(cpu.a);
}

/*
 * @card 00:FB96 bm_find_file
 * WHAT IT DOES: Looks through the backup memory's files for the name at _AX.
 * HOW: Each file starts with its length; a length of zero ends the list. The name is compared
 * through the window, and the file reached is left in CardFileLo.
 * WAITS: no.
 * Entry: _AX the name's address, backup memory open. Exit, found: C clear, A the file's high
 * byte, N and Z from it, Y $0C, V from taking the name's offset off again. Exit, not found: C
 * set, A 0, N 0, Z 1, Y 1, V from the last step, or kept if there were no files. X the saved MPR.
 * Not run by any script: a name that does not match a file.
 */
CardExit bm_find_file(void)
{
    CardExit e = { 0 };
    BackupWindow w;
    uint8_t value;

    value = RD8(CZP->a.lo);
    WR8(CZP->copy.lo, value);
    value = RD8(CZP->a.hi);
    WR8(CZP->copy.hi, value);
    jsr_push(0xFBA1);
    w = bm_map_name_window();
    jsr_pop();
    WR8(CZP->file.lo, BACKUP_FIRST_FILE_LO);
    WR8(CZP->file.hi, BACKUP_FIRST_FILE_HI);
    e.x = w.window;
    e.set_x = true;
    for (;;) {
        uint8_t length_low = RD8_PTR(CZP->file.lo);
        uint8_t length_high = RD8_PTR_Y(CZP->file.lo, 1);
        Sum low;
        Sum high;
        uint8_t y = 0;
        bool same = true;

        if ((uint8_t)(length_low | length_high) == 0) {
            e.a = 0;
            e.nz = 0;
            e.y = 1;
            e.c = true;
            e.set_a = e.set_nz = e.set_y = e.set_c = true;
            return e;
        }
        value = RD8(CZP->file.lo);
        low = add8(value, FILE_NAME_AT, false);
        WR8(CZP->file.lo, low.value);
        value = RD8(CZP->file.hi);
        high = add8(value, 0, low.carry);
        WR8(CZP->file.hi, high.value);
        do {
            uint8_t wanted = RD8_PTR_Y(CZP->copy.lo, y);
            uint8_t found = RD8_PTR_Y(CZP->file.lo, y);

            if (wanted != found) {
                same = false;
                break;
            }
            y++;
        } while (y < FILE_NAME_BYTES);
        value = RD8(CZP->file.lo);
        low = sub8(value, FILE_NAME_AT, true);
        WR8(CZP->file.lo, low.value);
        value = RD8(CZP->file.hi);
        high = sub8(value, 0, low.carry);
        WR8(CZP->file.hi, high.value);
        if (same) {
            e.a = e.nz = high.value;
            e.y = y;
            e.c = false;
            e.v = high.overflow;
            e.set_a = e.set_nz = e.set_y = e.set_c = e.set_v = true;
            return e;
        }
        length_high = RD8_PTR_Y(CZP->file.lo, 1);
        push8(length_high);
        length_low = RD8_PTR(CZP->file.lo);
        value = RD8(CZP->file.lo);
        low = add8(length_low, value, false);
        WR8(CZP->file.lo, low.value);
        length_high = pull8();
        value = RD8(CZP->file.hi);
        high = add8(length_high, value, low.carry);
        WR8(CZP->file.hi, high.value);
        e.v = high.overflow;
        e.set_v = true;
    }
}

void abi_bm_find_file(void)
{
    card_exit_apply(bm_find_file());
}

/* Where the memory ended, as bm_format measured it. */
typedef struct {
    uint8_t page;
    uint8_t offset;
    uint8_t pages_left;
    bool whole;
} BackupSize;

/*
 * @card 00:F858 bm_format
 * WHAT IT DOES: Formats the backup memory, given the password that guards it.
 * HOW: Compares eleven bytes with the password; measures how much RAM answers; writes the
 * header: signature, end, next free byte, and no files.
 * WAITS: only through bm_close.
 * Entry: _AX the password's address; A (only kept; opened with the password's first byte). Exit,
 * wrong password: A 1, N 0, Z 0, Y where it differed, C from that compare. Exit: A 0, N 0, Z 1, X $FF,
 * Y the bytes short of a page, C whether all of it answered, I clear. V kept.
 * Not run by any script: the whole routine.
 */
BackupAccess bm_format(uint8_t a)
{
    BackupAccess format = { BM_ENDED_DONE, 0, { 0 } };
    BackupSize size = { BACKUP_START_HI, 0, BACKUP_MAX_PAGES, true };
    int y = BACKUP_PASSWORD_BYTES - 1;
    uint8_t wanted = a;
    uint8_t given;
    uint8_t value;

    do {
        wanted = RD8_IDX(COL(CROMT->format_password), y);
        given = RD8_PTR_Y(CZP->a.lo, (uint8_t)y);
        if (wanted != given) {
            format.ending = BM_ENDED_NOT_FOUND;
            format.result = BM_BAD_PASSWORD;
            format.exit.y = (uint8_t)y;
            format.exit.c = wanted >= given;
            format.exit.set_y = format.exit.set_c = true;
            return format;
        }
        y--;
    } while (y >= 0);
    jsr_push(0xF867);
    (void)bm_open(wanted);
    jsr_pop();
    WR8(CZP->pointer.hi, BACKUP_START_HI);
    WR8(CZP->pointer.lo, 0);
    do {
        size.offset = 0;
        do {
            uint8_t check;

            WR8_PTR_Y(CZP->pointer.lo, size.offset, 0);
            check = RD8_PTR_Y(CZP->pointer.lo, size.offset);
            if (check != 0) {
                size.whole = false;
                break;
            }
            size.offset++;
        } while (size.offset != 0);
        if (!size.whole) {
            break;
        }
        (void)INC8(CZP->pointer.hi);
        size.pages_left--;
    } while (size.pages_left != 0);
    WR8(CBM->end.lo, size.offset);
    size.page = RD8(CZP->pointer.hi);
    WR8(CBM->end.hi, size.page);
    for (int x = BACKUP_SIGNATURE_BYTES - 1; x >= 0; x--) {
        value = RD8_IDX(COL(CROMT->signature), x);
        WR8_IDX(COL(CBM->header), x, value);
    }
    WR8(CBM->next_free.lo, BACKUP_FIRST_FILE_LO);
    WR8(CBM->next_free.hi, BACKUP_FIRST_FILE_HI);
    WR8(CBM->first_file.lo, 0);
    WR8(CBM->first_file.hi, 0);
    (void)bm_close(BM_DONE);
    format.exit.x = 0xFF;
    format.exit.y = size.offset;
    format.exit.c = size.whole;
    format.exit.set_x = format.exit.set_y = format.exit.set_c = true;
    return format;
}

/*
 * @card 00:F8B8 bm_free
 * WHAT IT DOES: Gives how many bytes of the backup memory are free for another file.
 * HOW: Its end less its next free byte, less a file's header of $12 bytes, and zero if that
 * is below zero. The answer goes in _CX.
 * WAITS: only through bm_close.
 * Entry: A (only kept). Exit: A 0, N 0, Z 1, X $FF, C and V from the subtract's high byte, I clear;
 * or $FF as bm_open_formatted if not formatted. Y kept.
 * Not run by any script: a result below zero, which puts zero in _CX.
 */
BackupAccess bm_free(uint8_t a)
{
    BackupAccess free_space = { BM_ENDED_DONE, 0, { 0 } };
    BackupCheck check;
    Sum low;
    Sum high;
    uint8_t value;
    uint8_t other;

    jsr_push(0xF8BB);
    check = bm_open_formatted(a);
    if (!check.formatted) {
        free_space.ending = BM_ENDED_NOT_FORMATTED;
        free_space.result = BM_NOT_FORMATTED;
        free_space.exit = check.exit;
        return free_space;
    }
    jsr_pop();
    value = RD8(CBM->end.lo);
    other = RD8(CBM->next_free.lo);
    low = sub8(value, other, true);
    WR8(CZP->c.lo, low.value);
    value = RD8(CBM->end.hi);
    other = RD8(CBM->next_free.hi);
    high = sub8(value, other, low.carry);
    WR8(CZP->c.hi, high.value);
    value = RD8(CZP->c.lo);
    low = sub8(value, BACKUP_HEADER_BYTES, true);
    WR8(CZP->c.lo, low.value);
    value = RD8(CZP->c.hi);
    high = sub8(value, 0, low.carry);
    WR8(CZP->c.hi, high.value);
    if (high.value & 0x80) {
        WR8(CZP->c.lo, 0);
        WR8(CZP->c.hi, 0);
    }
    (void)bm_close(BM_DONE);
    free_space.exit = check.exit;
    free_space.exit.c = high.carry;
    free_space.exit.v = high.overflow;
    free_space.exit.set_v = true;
    return free_space;
}

/*
 * @card 00:FBFC bm_sum_file
 * WHAT IT DOES: Sums the name and data of the file at CardFileLo, for its checksum.
 * HOW: Adds every byte after the length and checksum words into a 16-bit sum in CardCopyLo,
 * counting the file's length less four down in CardByteCount. A length of four sums $10000
 * bytes.
 * WAITS: no.
 * Entry: CardFileLo. Exit: A 0, N 0, Z 1, Y the low byte of four plus the bytes summed, C and V
 * from the last add's high byte. X kept. CardPointerLo-Hi the file, its high byte moved on by
 * the pages crossed.
 * Not run by any script: a file crossing a page of the memory.
 */
CardExit bm_sum_file(void)
{
    CardExit e = { 0 };
    Sum low;
    Sum high;
    uint8_t value;
    uint8_t y;
    uint8_t left;

    value = RD8(CZP->file.lo);
    WR8(CZP->pointer.lo, value);
    value = RD8(CZP->file.hi);
    WR8(CZP->pointer.hi, value);
    value = RD8_PTR(CZP->pointer.lo);
    low = sub8(value, SUM_START, true);
    WR8(CRAM->byte_count.lo, low.value);
    value = RD8_PTR_Y(CZP->pointer.lo, 1);
    high = sub8(value, 0, low.carry);
    WR8(CRAM->byte_count.hi, high.value);
    WR8(CZP->copy.lo, 0);
    WR8(CZP->copy.hi, 0);
    y = SUM_START;
    do {
        uint8_t byte = RD8_PTR_Y(CZP->pointer.lo, y);

        value = RD8(CZP->copy.lo);
        low = add8(byte, value, false);
        WR8(CZP->copy.lo, low.value);
        value = RD8(CZP->copy.hi);
        high = add8(0, value, low.carry);
        WR8(CZP->copy.hi, high.value);
        y++;
        if (y == 0) {
            (void)INC8(CZP->pointer.hi);
        }
        jsr_push(0xFC2F);
        left = bm_count_down();
        jsr_pop();
    } while (left != 0);
    e.a = 0;
    e.nz = 0;
    e.y = y;
    e.c = high.carry;
    e.v = high.overflow;
    e.set_a = e.set_nz = e.set_y = e.set_c = e.set_v = true;
    return e;
}

void abi_bm_sum_file(void)
{
    card_exit_apply(bm_sum_file());
}

/*
 * @card 00:FB46 bm_place_span
 * WHAT IT DOES: Places a read or write of _CX bytes at offset _DX in the file at CardFileLo.
 * HOW: How far it runs past the file's end goes to CardCopyLo (negative when it fits), the
 * address in the file it starts at to CardPointerLo, and _CX to CardByteCount.
 * WAITS: no.
 * Entry: _CX, _DX, CardFileLo. Exit: A the high byte of the overrun, N and Z from it, Y 1, C and V
 * from adding _DX to the file's data address. X kept.
 */
CardExit bm_place_span(void)
{
    CardExit e = { 0 };
    Sum low;
    Sum high;
    uint8_t value;
    uint8_t other;

    value = RD8(CZP->d.lo);
    other = RD8(CZP->c.lo);
    low = add8(value, other, false);
    WR8(CZP->copy.lo, low.value);
    value = RD8(CZP->d.hi);
    other = RD8(CZP->c.hi);
    high = add8(value, other, low.carry);
    WR8(CZP->copy.hi, high.value);
    value = RD8(CZP->copy.lo);
    low = add8(value, FILE_HEADER_BYTES, false);
    WR8(CZP->copy.lo, low.value);
    value = RD8(CZP->copy.hi);
    high = add8(value, 0, low.carry);
    WR8(CZP->copy.hi, high.value);
    value = RD8(CZP->copy.lo);
    other = RD8_PTR(CZP->file.lo);
    low = sub8(value, other, true);
    WR8(CZP->copy.lo, low.value);
    value = RD8(CZP->copy.hi);
    other = RD8_PTR_Y(CZP->file.lo, 1);
    high = sub8(value, other, low.carry);
    WR8(CZP->copy.hi, high.value);
    value = RD8(CZP->file.lo);
    low = add8(value, FILE_HEADER_BYTES, false);
    WR8(CZP->pointer.lo, low.value);
    value = RD8(CZP->file.hi);
    high = add8(value, 0, low.carry);
    WR8(CZP->pointer.hi, high.value);
    value = RD8(CZP->d.lo);
    other = RD8(CZP->pointer.lo);
    low = add8(value, other, false);
    WR8(CZP->pointer.lo, low.value);
    value = RD8(CZP->d.hi);
    other = RD8(CZP->pointer.hi);
    high = add8(value, other, low.carry);
    WR8(CZP->pointer.hi, high.value);
    value = RD8(CZP->c.lo);
    WR8(CRAM->byte_count.lo, value);
    value = RD8(CZP->c.hi);
    WR8(CRAM->byte_count.hi, value);
    value = RD8(CZP->copy.hi);
    e.a = e.nz = value;
    e.y = 1;
    e.c = high.carry;
    e.v = high.overflow;
    e.set_a = e.set_nz = e.set_y = e.set_c = e.set_v = true;
    return e;
}

void abi_bm_place_span(void)
{
    card_exit_apply(bm_place_span());
}

/*
 * @card 00:FD41 bm_move_direction
 * WHAT IT DOES: Chooses which way to move bytes inside the backup memory, so that a move onto
 * itself is not spoiled.
 * HOW: A source at or above its destination goes first byte first. One below goes last byte
 * first: both pointers are moved on to their last byte.
 * WAITS: no.
 * Entry: CardPointerLo the source, CardCopyLo the destination, CardByteCount the bytes.
 * Exit, forward: X 2, A the high bytes' difference, N 0, Z 0, C 1, V from that subtract.
 * Exit, backward: X 0, A the destination's new high byte, N 0, Z 1, C and V from moving it on.
 * Y kept.
 * Not run by any script: the forward way.
 */
CardExit bm_move_direction(void)
{
    CardExit e = { 0 };
    Sum low;
    Sum high;
    uint8_t value;
    uint8_t other;

    value = RD8(CZP->pointer.lo);
    other = RD8(CZP->copy.lo);
    low = sub8(value, other, true);
    value = RD8(CZP->pointer.hi);
    other = RD8(CZP->copy.hi);
    high = sub8(value, other, low.carry);
    if (high.carry) {
        e.a = high.value;
        e.x = 2;
        e.nz = 2;
        e.c = true;
        e.v = high.overflow;
        e.set_a = e.set_x = e.set_nz = e.set_c = e.set_v = true;
        return e;
    }
    jsr_push(0xFD4F);
    (void)bm_count_down();
    jsr_pop();
    value = RD8(CZP->pointer.lo);
    other = RD8(CRAM->byte_count.lo);
    low = add8(value, other, false);
    WR8(CZP->pointer.lo, low.value);
    value = RD8(CZP->pointer.hi);
    other = RD8(CRAM->byte_count.hi);
    high = add8(value, other, low.carry);
    WR8(CZP->pointer.hi, high.value);
    value = RD8(CZP->copy.lo);
    other = RD8(CRAM->byte_count.lo);
    low = add8(value, other, false);
    WR8(CZP->copy.lo, low.value);
    value = RD8(CZP->copy.hi);
    other = RD8(CRAM->byte_count.hi);
    high = add8(value, other, low.carry);
    WR8(CZP->copy.hi, high.value);
    if (INC8(CRAM->byte_count.lo) == 0) {
        (void)INC8(CRAM->byte_count.hi);
    }
    e.a = high.value;
    e.x = 0;
    e.nz = 0;
    e.c = high.carry;
    e.v = high.overflow;
    e.set_a = e.set_x = e.set_nz = e.set_c = e.set_v = true;
    return e;
}

void abi_bm_move_direction(void)
{
    card_exit_apply(bm_move_direction());
}

/*
 * @card 00:FCD3 bm_move
 * WHAT IT DOES: Moves CardByteCount bytes inside the backup memory, then marks the end of the
 * file list at its next free byte.
 * HOW: bm_move_direction picks the way; bytes go one at a time, each pointer moved a page at a
 * time; then a zero length is written at the next free byte.
 * WAITS: no.
 * Entry: CardPointerLo the source, CardCopyLo the destination, CardByteCount. Exit: A 0, Y 1, N
 * and Z from loading Y (CLA leaves them), CardPointerLo the next free byte. Nothing to
 * move: X, C, V kept. Else X, C and V as bm_move_direction leaves them.
 * Not run by any script: the forward move.
 */
CardExit bm_move(void)
{
    CardExit e = { 0 };
    uint8_t low = RD8(CRAM->byte_count.lo);
    uint8_t high = RD8(CRAM->byte_count.hi);
    uint8_t value;

    if ((uint8_t)(low | high) != 0) {
        CardExit way;
        uint8_t y = 0;
        uint8_t left;

        jsr_push(0xFCDE);
        way = bm_move_direction();
        jsr_pop();
        e.x = way.x;
        e.c = way.c;
        e.v = way.v;
        e.set_x = e.set_c = e.set_v = true;
        if (way.x == 0) {
            do {
                value = RD8_PTR_Y(CZP->pointer.lo, y);
                WR8_PTR_Y(CZP->copy.lo, y, value);
                if (y == 0) {
                    (void)DEC8(CZP->pointer.hi);
                    (void)DEC8(CZP->copy.hi);
                }
                y--;
                jsr_push(0xFCF1);
                left = bm_count_down();
                jsr_pop();
            } while (left != 0);
        } else {
            do {
                value = RD8_PTR_Y(CZP->pointer.lo, y);
                WR8_PTR_Y(CZP->copy.lo, y, value);
                y++;
                if (y == 0) {
                    (void)INC8(CZP->pointer.hi);
                    (void)INC8(CZP->copy.hi);
                }
                jsr_push(0xFD03);
                left = bm_count_down();
                jsr_pop();
            } while (left != 0);
        }
    }
    value = RD8(CBM->next_free.lo);
    WR8(CZP->pointer.lo, value);
    value = RD8(CBM->next_free.hi);
    WR8(CZP->pointer.hi, value);
    WR8_PTR(CZP->pointer.lo, 0);
    WR8_PTR_Y(CZP->pointer.lo, 1, 0);
    e.a = 0;
    e.y = 1;
    e.nz = 1;
    e.set_a = e.set_y = e.set_nz = true;
    return e;
}

void abi_bm_move(void)
{
    card_exit_apply(bm_move());
}

/*
 * @card 00:FD17 bm_copy_out
 * WHAT IT DOES: Copies bytes from the backup memory to the caller's memory.
 * HOW: The caller's pointer in CardCopyLo is put through the window, then CardByteCount bytes go
 * one at a time from CardPointerLo; a count of zero copies $10000.
 * WAITS: no.
 * Entry: CardPointerLo, CardCopyLo, CardByteCount. Exit: A 0, N 0, Z 1, X the saved MPR, Y the
 * low byte of the count, C bit 4 of the caller's old high byte. V kept.
 * Not run by any script: a copy crossing a page.
 */
CardExit bm_copy_out(void)
{
    CardExit e;
    BackupWindow w;
    uint8_t y = 0;
    uint8_t left;
    uint8_t value;

    jsr_push(0xFD1A);
    w = bm_map_name_window();
    jsr_pop();
    do {
        value = RD8_PTR_Y(CZP->pointer.lo, y);
        WR8_PTR_Y(CZP->copy.lo, y, value);
        y++;
        if (y == 0) {
            (void)INC8(CZP->pointer.hi);
            (void)INC8(CZP->copy.hi);
        }
        jsr_push(0xFD29);
        left = bm_count_down();
        jsr_pop();
    } while (left != 0);
    e = window_exit(w);
    e.a = 0;
    e.nz = 0;
    e.y = y;
    e.set_y = true;
    return e;
}

void abi_bm_copy_out(void)
{
    card_exit_apply(bm_copy_out());
}

/*
 * @card 00:FD2C bm_copy_in
 * WHAT IT DOES: Copies bytes from the caller's memory into the backup memory.
 * HOW: As bm_copy_out, the other way.
 * WAITS: no.
 * Entry: CardPointerLo, CardCopyLo, CardByteCount. Exit: as bm_copy_out.
 * Not run by any script: a copy crossing a page.
 */
CardExit bm_copy_in(void)
{
    CardExit e;
    BackupWindow w;
    uint8_t y = 0;
    uint8_t left;
    uint8_t value;

    jsr_push(0xFD2F);
    w = bm_map_name_window();
    jsr_pop();
    do {
        value = RD8_PTR_Y(CZP->copy.lo, y);
        WR8_PTR_Y(CZP->pointer.lo, y, value);
        y++;
        if (y == 0) {
            (void)INC8(CZP->pointer.hi);
            (void)INC8(CZP->copy.hi);
        }
        jsr_push(0xFD3E);
        left = bm_count_down();
        jsr_pop();
    } while (left != 0);
    e = window_exit(w);
    e.a = 0;
    e.nz = 0;
    e.y = y;
    e.set_y = true;
    return e;
}

void abi_bm_copy_in(void)
{
    card_exit_apply(bm_copy_in());
}

/*
 * @card 00:FAE2 bm_new_file
 * WHAT IT DOES: Starts a new, empty file named by _AX at the backup memory's next free byte.
 * HOW: Works out the room left into CardPointerLo, but tests CardCopyLo-Hi against $10 instead,
 * which the name's pointer always passes. Then copies the twelve bytes of the name through the
 * window, moves the next free byte on by a header, and gives the file a length of $10.
 * WAITS: no.
 * Entry: _AX the name, CardCopyLo-Hi. Exit, made: C clear, A 0, N 0, Z 1, Y 1, X the saved MPR, V
 * from moving the next free byte on. Exit, refused: C set, A the high byte of the test, N and Z
 * from it, V from it. X and Y kept.
 * Not run by any script: the refused exit (no room).
 */
CardExit bm_new_file(void)
{
    CardExit e = { 0 };
    Sum low;
    Sum high;
    BackupWindow w;
    uint8_t value;
    uint8_t other;
    uint8_t y = 0;

    value = RD8(CBM->next_free.lo);
    WR8(CZP->file.lo, value);
    value = RD8(CBM->next_free.hi);
    WR8(CZP->file.hi, value);
    value = RD8(CBM->end.lo);
    other = RD8(CZP->file.lo);
    low = sub8(value, other, true);
    WR8(CZP->pointer.lo, low.value);
    value = RD8(CBM->end.hi);
    other = RD8(CZP->file.hi);
    high = sub8(value, other, low.carry);
    WR8(CZP->pointer.hi, high.value);
    value = RD8(CZP->copy.lo);
    low = sub8(value, FILE_HEADER_BYTES, true);
    value = RD8(CZP->copy.hi);
    high = sub8(value, 0, low.carry);
    if (!high.carry) {
        e.a = e.nz = high.value;
        e.c = true;
        e.v = high.overflow;
        e.set_a = e.set_nz = e.set_c = e.set_v = true;
        return e;
    }
    value = RD8(CZP->file.lo);
    low = add8(value, FILE_NAME_AT, false);
    WR8(CZP->pointer.lo, low.value);
    value = RD8(CZP->file.hi);
    high = add8(value, 0, low.carry);
    WR8(CZP->pointer.hi, high.value);
    value = RD8(CZP->a.lo);
    WR8(CZP->copy.lo, value);
    value = RD8(CZP->a.hi);
    WR8(CZP->copy.hi, value);
    jsr_push(0xFB1D);
    w = bm_map_name_window();
    jsr_pop();
    do {
        value = RD8_PTR_Y(CZP->copy.lo, y);
        WR8_PTR_Y(CZP->pointer.lo, y, value);
        y++;
    } while (y != FILE_NAME_BYTES);
    value = RD8(CBM->next_free.lo);
    low = add8(value, FILE_HEADER_BYTES, false);
    WR8(CBM->next_free.lo, low.value);
    value = RD8(CBM->next_free.hi);
    high = add8(value, 0, low.carry);
    WR8(CBM->next_free.hi, high.value);
    WR8_PTR(CZP->file.lo, FILE_HEADER_BYTES);
    WR8_PTR_Y(CZP->file.lo, 1, 0);
    e.a = 0;
    e.nz = 0;
    e.x = w.window;
    e.y = 1;
    e.c = false;
    e.v = high.overflow;
    e.set_a = e.set_nz = e.set_x = e.set_y = e.set_c = e.set_v = true;
    return e;
}

void abi_bm_new_file(void)
{
    card_exit_apply(bm_new_file());
}

/* The exit every public routine shares: A the result, N and Z from it, I clear, and what its last steps left. */
static void access_exit(BackupAccess access)
{
    card_exit_apply(access.exit);
    abi_flag(P_I, false);
    cpu.a = access.result;
    abi_nz(access.result);
}

/*
 * @card 00:F8E3 bm_read
 * WHAT IT DOES: Reads _CX bytes at offset _DX of the backup file named by _AX into _BX.
 * HOW: Finds the file; refuses an offset at or past its end; sums the file against its
 * checksum; cuts the count at the file's end and gives it back in _CX; copies through the
 * window. A is 0, 1 for no such file, or 2 for a bad checksum (the bytes are still copied).
 * WAITS: only through bm_close.
 * Entry: _AX, _BX, _CX, _DX; A (only kept). Exit: I clear, A the result, N and Z from it; not
 * formatted, as bm_open_formatted. No file: X the saved MPR, Y 1, C 1, V as bm_find_file. Offset
 * past the end: _CX 0, A 0, Y 1, C 1, X and V as bm_find_file. Read: bytes copied, X the saved MPR,
 * Y the count's low byte, C bit 4 of _BH; nothing copied, X as bm_find_file, Y 1, C from the cut or
 * the place; V from the cut or the place.
 * Not run by any script: an offset past the end, a bad checksum.
 */
BackupAccess bm_read(uint8_t a)
{
    BackupAccess read = { BM_ENDED_DONE, 0, { 0 } };
    BackupCheck check;
    CardExit find;
    CardExit span;
    CardExit copied;
    Sum low;
    Sum high;
    Sum cut_low;
    Sum cut_high = { 0, false, false };
    bool cut = false;
    bool past_end;
    uint8_t value;
    uint8_t other;
    uint8_t folded;
    uint8_t length_high;

    jsr_push(0xF8E6);
    check = bm_open_formatted(a);
    if (!check.formatted) {
        read.ending = BM_ENDED_NOT_FORMATTED;
        read.result = BM_NOT_FORMATTED;
        read.exit = check.exit;
        return read;
    }
    jsr_pop();
    jsr_push(0xF8E9);
    find = bm_find_file();
    jsr_pop();
    if (find.c) {
        (void)bm_close(BM_NO_FILE);
        read.ending = BM_ENDED_NOT_FOUND;
        read.result = BM_NO_FILE;
        read.exit = find;
        return read;
    }
    value = RD8(CZP->d.hi);
    length_high = RD8_PTR_Y(CZP->file.lo, 1);
    past_end = value >= length_high;
    if (value == length_high) {
        value = RD8(CZP->d.lo);
        other = RD8_PTR(CZP->file.lo);
        past_end = value >= other;
    }
    if (past_end) {
        WR8(CZP->c.hi, 0);
        WR8(CZP->c.lo, 0);
        (void)bm_close(BM_DONE);
        read.ending = BM_ENDED_PAST_END;
        read.result = BM_DONE;
        read.exit = find;
        read.exit.y = 1;
        read.exit.c = true;
        read.exit.set_y = read.exit.set_c = true;
        return read;
    }
    jsr_push(0xF8FC);
    (void)bm_sum_file();
    jsr_pop();
    value = RD8_PTR_Y(CZP->file.lo, FILE_SUM_AT);
    other = RD8(CZP->copy.lo);
    low = add8(value, other, false);
    WR8(CZP->copy.lo, low.value);
    value = RD8_PTR_Y(CZP->file.lo, FILE_SUM_AT + 1);
    other = RD8(CZP->copy.hi);
    high = add8(value, other, low.carry);
    other = RD8(CZP->copy.lo);
    folded = (uint8_t)(high.value | other);
    push8(folded);
    jsr_push(0xF910);
    span = bm_place_span();
    jsr_pop();
    if (!(span.nz & 0x80)) {
        value = RD8(CRAM->byte_count.lo);
        other = RD8(CZP->copy.lo);
        cut_low = sub8(value, other, true);
        WR8(CRAM->byte_count.lo, cut_low.value);
        value = RD8(CRAM->byte_count.hi);
        other = RD8(CZP->copy.hi);
        cut_high = sub8(value, other, cut_low.carry);
        WR8(CRAM->byte_count.hi, cut_high.value);
        cut = true;
    }
    value = RD8(CRAM->byte_count.lo);
    WR8(CZP->c.lo, value);
    value = RD8(CRAM->byte_count.hi);
    WR8(CZP->c.hi, value);
    other = RD8(CRAM->byte_count.lo);
    folded = (uint8_t)(value | other);
    read.exit = find;
    read.exit.c = cut ? cut_high.carry : span.c;
    read.exit.v = cut ? cut_high.overflow : span.v;
    read.exit.set_c = read.exit.set_v = true;
    if (folded != 0) {
        value = RD8(CZP->b.lo);
        WR8(CZP->copy.lo, value);
        value = RD8(CZP->b.hi);
        WR8(CZP->copy.hi, value);
        jsr_push(0xF93D);
        copied = bm_copy_out();
        jsr_pop();
        read.exit.x = copied.x;
        read.exit.y = copied.y;
        read.exit.c = copied.c;
        read.exit.set_x = read.exit.set_y = true;
    } else {
        read.exit.y = 1;
        read.exit.set_y = true;
    }
    folded = pull8();
    read.result = folded != 0 ? BM_BAD_SUM : BM_DONE;
    (void)bm_close(read.result);
    return read;
}

/* The A bm_read leaves: 0, 1 for no such file, 2 for a bad checksum, $FF not formatted. */
uint8_t bm_read_result(BackupAccess read)
{
    return read.result;
}

/* The A bm_write leaves: 0, 1 for no room, $FF not formatted. */
uint8_t bm_write_result(BackupAccess write)
{
    return write.result;
}

/* bm_read's exit, for its wrapper and for a caller's that ends in it. */
void bm_read_exit_shim(BackupAccess read)
{
    access_exit(read);
}

void abi_bm_read(void)
{
    bm_read_exit_shim(bm_read(cpu.a));
}

/*
 * @card 00:F955 bm_write
 * WHAT IT DOES: Writes _CX bytes from _BX at offset _DX of the backup file named by _AX, making
 * the file if there is none, and growing it if the write runs past its end.
 * HOW: A file that must grow moves every file after it up by the growth, if the memory has room
 * for that and a closing zero length; then the bytes are copied in through the window and the
 * file's checksum is made the negative of its sum.
 * WAITS: only through bm_close.
 * Entry: _AX, _BX, _CX, _DX; A (only kept). Exit: I clear, A the result, N and Z from it; not
 * formatted, as bm_open_formatted. No room for a header: A 1, X as bm_find_file, Y 1, C, V as
 * bm_new_file. No room to grow: A 1, X the saved MPR, Y 1, C 0, V from that compare. Written: A 0,
 * X the saved MPR, Y 3, C and V from negating the sum's high byte.
 * Not run by any script: a write that fits the file, a refused header, no room to grow.
 */
BackupAccess bm_write(uint8_t a)
{
    BackupAccess write = { BM_ENDED_DONE, 0, { 0 } };
    BackupCheck check;
    CardExit find;
    CardExit span;
    CardExit made;
    CardExit copied;
    Sum low;
    Sum high;
    uint8_t value;
    uint8_t other;
    uint8_t window;

    jsr_push(0xF958);
    check = bm_open_formatted(a);
    if (!check.formatted) {
        write.ending = BM_ENDED_NOT_FORMATTED;
        write.result = BM_NOT_FORMATTED;
        write.exit = check.exit;
        return write;
    }
    jsr_pop();
    jsr_push(0xF95B);
    find = bm_find_file();
    jsr_pop();
    window = find.x;
    if (find.c) {
        jsr_push(0xF960);
        made = bm_new_file();
        jsr_pop();
        if (made.c) {
            write.exit = find;
            write.exit.c = true;
            write.exit.v = made.v;
            write.exit.set_c = write.exit.set_v = true;
            write.ending = BM_ENDED_NOT_FOUND;
            write.result = BM_NO_FILE;
            (void)bm_close(BM_NO_FILE);
            return write;
        }
        window = made.x;
    }
    jsr_push(0xF968);
    span = bm_place_span();
    jsr_pop();
    if (!(span.nz & 0x80)) {
        value = RD8(CZP->copy.lo);
        WR8(CRAM->byte_count.lo, value);
        value = RD8(CZP->copy.hi);
        WR8(CRAM->byte_count.hi, value);
        value = RD8(CBM->next_free.lo);
        other = RD8(CRAM->byte_count.lo);
        low = add8(value, other, false);
        WR8(CZP->copy.lo, low.value);
        value = RD8(CBM->next_free.hi);
        other = RD8(CRAM->byte_count.hi);
        high = add8(value, other, low.carry);
        WR8(CZP->copy.hi, high.value);
        value = RD8(CZP->copy.lo);
        low = add8(value, 2, false);
        WR8(CZP->pointer.lo, low.value);
        value = RD8(CZP->copy.hi);
        high = add8(value, 0, low.carry);
        WR8(CZP->pointer.hi, high.value);
        value = RD8(CBM->end.lo);
        other = RD8(CZP->pointer.lo);
        low = sub8(value, other, true);
        value = RD8(CBM->end.hi);
        other = RD8(CZP->pointer.hi);
        high = sub8(value, other, low.carry);
        if (!high.carry) {
            write.exit = span;
            write.exit.x = window;
            write.exit.c = false;
            write.exit.v = high.overflow;
            write.exit.set_x = write.exit.set_c = write.exit.set_v = true;
            write.ending = BM_ENDED_NO_ROOM;
            write.result = BM_NO_FILE;
            (void)bm_close(BM_NO_FILE);
            return write;
        }
        value = RD8(CZP->copy.lo);
        WR8(CBM->next_free.lo, value);
        value = RD8(CZP->copy.hi);
        WR8(CBM->next_free.hi, value);
        value = RD8_PTR(CZP->file.lo);
        other = RD8(CZP->file.lo);
        low = add8(value, other, false);
        WR8(CZP->copy.lo, low.value);
        WR8(CZP->pointer.lo, low.value);
        value = RD8_PTR_Y(CZP->file.lo, 1);
        other = RD8(CZP->file.hi);
        high = add8(value, other, low.carry);
        WR8(CZP->copy.hi, high.value);
        WR8(CZP->pointer.hi, high.value);
        value = RD8(CRAM->byte_count.lo);
        other = RD8_PTR(CZP->file.lo);
        low = add8(value, other, false);
        WR8_PTR(CZP->file.lo, low.value);
        value = RD8(CRAM->byte_count.hi);
        other = RD8_PTR_Y(CZP->file.lo, 1);
        high = add8(value, other, low.carry);
        WR8_PTR_Y(CZP->file.lo, 1, high.value);
        value = RD8(CRAM->byte_count.lo);
        other = RD8(CZP->copy.lo);
        low = add8(value, other, false);
        WR8(CZP->copy.lo, low.value);
        value = RD8(CRAM->byte_count.hi);
        other = RD8(CZP->copy.hi);
        high = add8(value, other, low.carry);
        WR8(CZP->copy.hi, high.value);
        value = RD8(CBM->next_free.lo);
        other = RD8(CZP->pointer.lo);
        low = sub8(value, other, true);
        WR8(CRAM->byte_count.lo, low.value);
        value = RD8(CBM->next_free.hi);
        other = RD8(CZP->pointer.hi);
        high = sub8(value, other, low.carry);
        WR8(CRAM->byte_count.hi, high.value);
        jsr_push(0xF9F2);
        (void)bm_move();
        jsr_pop();
        jsr_push(0xF9F5);
        (void)bm_place_span();
        jsr_pop();
    }
    value = RD8(CZP->b.lo);
    WR8(CZP->copy.lo, value);
    value = RD8(CZP->b.hi);
    WR8(CZP->copy.hi, value);
    jsr_push(0xFA00);
    copied = bm_copy_in();
    jsr_pop();
    jsr_push(0xFA03);
    (void)bm_sum_file();
    jsr_pop();
    value = RD8(CZP->copy.lo);
    low = sub8(0, value, true);
    WR8_PTR_Y(CZP->file.lo, FILE_SUM_AT, low.value);
    value = RD8(CZP->copy.hi);
    high = sub8(0, value, low.carry);
    WR8_PTR_Y(CZP->file.lo, FILE_SUM_AT + 1, high.value);
    (void)bm_close(BM_DONE);
    write.exit = copied;
    write.exit.y = FILE_SUM_AT + 1;
    write.exit.c = high.carry;
    write.exit.v = high.overflow;
    write.exit.set_c = write.exit.set_v = true;
    write.result = BM_DONE;
    return write;
}

/* bm_write's exit, for its wrapper and for a caller's that ends in it. */
void bm_write_exit_shim(BackupAccess write)
{
    access_exit(write);
}

void abi_bm_write(void)
{
    bm_write_exit_shim(bm_write(cpu.a));
}

/*
 * @card 00:FA1A bm_delete
 * WHAT IT DOES: Deletes the backup file named by _AX.
 * HOW: Moves every file after it down over it, takes its length off the next free byte, and
 * marks the list's new end.
 * WAITS: only through bm_close.
 * Entry: _AX; A (only kept). Exit: I clear, A the result, N and Z from it; not formatted, as
 * bm_open_formatted. No file: A 1, X the saved MPR, Y 1, C 1, V as bm_find_file. Deleted: A 0, Y 1;
 * X, C and V as bm_move leaves them, or, with nothing after it, X the saved MPR and C and V from
 * taking the length off.
 */
BackupAccess bm_delete(uint8_t a)
{
    BackupAccess removal = { BM_ENDED_DONE, 0, { 0 } };
    BackupCheck check;
    CardExit find;
    CardExit moved;
    Sum low;
    Sum high;
    uint8_t value;
    uint8_t other;

    jsr_push(0xFA1D);
    check = bm_open_formatted(a);
    if (!check.formatted) {
        removal.ending = BM_ENDED_NOT_FORMATTED;
        removal.result = BM_NOT_FORMATTED;
        removal.exit = check.exit;
        return removal;
    }
    jsr_pop();
    jsr_push(0xFA20);
    find = bm_find_file();
    jsr_pop();
    if (find.c) {
        (void)bm_close(BM_NO_FILE);
        removal.ending = BM_ENDED_NOT_FOUND;
        removal.result = BM_NO_FILE;
        removal.exit = find;
        return removal;
    }
    value = RD8(CZP->file.lo);
    WR8(CZP->copy.lo, value);
    value = RD8(CZP->file.hi);
    WR8(CZP->copy.hi, value);
    value = RD8_PTR(CZP->file.lo);
    WR8(CRAM->byte_count.lo, value);
    value = RD8_PTR_Y(CZP->file.lo, 1);
    WR8(CRAM->byte_count.hi, value);
    value = RD8(CZP->file.lo);
    other = RD8(CRAM->byte_count.lo);
    low = add8(value, other, false);
    WR8(CZP->pointer.lo, low.value);
    value = RD8(CZP->file.hi);
    other = RD8(CRAM->byte_count.hi);
    high = add8(value, other, low.carry);
    WR8(CZP->pointer.hi, high.value);
    value = RD8(CBM->next_free.lo);
    other = RD8(CZP->pointer.lo);
    low = sub8(value, other, true);
    WR8(CRAM->byte_count.lo, low.value);
    value = RD8(CBM->next_free.hi);
    other = RD8(CZP->pointer.hi);
    high = sub8(value, other, low.carry);
    WR8(CRAM->byte_count.hi, high.value);
    value = RD8(CBM->next_free.lo);
    other = RD8_PTR(CZP->file.lo);
    low = sub8(value, other, true);
    WR8(CBM->next_free.lo, low.value);
    value = RD8(CBM->next_free.hi);
    other = RD8_PTR_Y(CZP->file.lo, 1);
    high = sub8(value, other, low.carry);
    WR8(CBM->next_free.hi, high.value);
    removal.exit = find;
    removal.exit.c = high.carry;
    removal.exit.v = high.overflow;
    removal.exit.set_c = removal.exit.set_v = true;
    jsr_push(0xFA6A);
    moved = bm_move();
    jsr_pop();
    (void)bm_close(BM_DONE);
    removal.exit = exit_over(removal.exit, moved);
    removal.result = BM_DONE;
    return removal;
}

/* bm_delete's exit, for its wrapper and for a caller's that ends in it. */
void bm_delete_exit_shim(BackupAccess removal)
{
    access_exit(removal);
}

uint8_t bm_delete_result(BackupAccess removal)
{
    return removal.result;
}

void abi_bm_delete(void)
{
    bm_delete_exit_shim(bm_delete(cpu.a));
}

/*
 * @card 00:FA72 bm_files
 * WHAT IT DOES: Gives the name and data size of the backup memory's file number _AL, counting
 * from 1, in the fourteen bytes at _BX.
 * HOW: Walks the file list from $8010 by each file's length, counting the files in _AL and down
 * in CardByteCount; a length of zero ends the list. The file's twelve-byte name goes to _BX
 * through bm_map_name_window, then its length less its $10-byte header, low byte first.
 * WAITS: only through bm_close.
 * Entry: _AL the file's number (0 walks to the 256th), _BX the buffer; A (only kept). Exit: I clear,
 * A the result, N and Z from it; not formatted, as bm_open_formatted. Named: A 0, X the saved MPR,
 * Y $0D, C 1, V from moving CardFileLo on by 4 to the name. No such file: A 1, X $FF, Y 1, _AL
 * the files counted, C and V from the high byte of the last step past a file, or C 1 and V kept
 * if there was none.
 * Not run by any script: everything after the check of the header.
 */
BackupAccess bm_files(uint8_t a)
{
    BackupAccess files = { BM_ENDED_DONE, 0, { 0 } };
    BackupCheck check;
    BackupWindow w;
    Sum low;
    Sum high;
    uint8_t value;
    uint8_t y;
    uint8_t name_high;

    jsr_push(0xFA75);
    check = bm_open_formatted(a);
    if (!check.formatted) {
        files.ending = BM_ENDED_NOT_FORMATTED;
        files.result = BM_NOT_FORMATTED;
        files.exit = check.exit;
        return files;
    }
    jsr_pop();
    files.exit = check.exit;
    WR8(CZP->file.lo, BACKUP_FIRST_FILE_LO);
    WR8(CZP->file.hi, BACKUP_FIRST_FILE_HI);
    value = RD8(CZP->a.lo);
    WR8(CRAM->byte_count.lo, value);
    WR8(CZP->a.lo, 0);
    files.exit.y = 1;
    files.exit.set_y = true;
    for (;;) {
        uint8_t length_low = RD8_PTR(CZP->file.lo);
        uint8_t length_high = RD8_PTR_Y(CZP->file.lo, 1);

        if ((uint8_t)(length_low | length_high) == 0) {
            (void)bm_close(BM_NO_FILE);
            files.ending = BM_ENDED_NOT_FOUND;
            files.result = BM_NO_FILE;
            return files;
        }
        (void)INC8(CZP->a.lo);
        if (DEC8(CRAM->byte_count.lo) == 0) {
            break;
        }
        length_high = RD8_PTR_Y(CZP->file.lo, 1);
        push8(length_high);
        length_low = RD8_PTR(CZP->file.lo);
        value = RD8(CZP->file.lo);
        low = add8(length_low, value, false);
        WR8(CZP->file.lo, low.value);
        length_high = pull8();
        value = RD8(CZP->file.hi);
        high = add8(length_high, value, low.carry);
        WR8(CZP->file.hi, high.value);
        files.exit.c = high.carry;
        files.exit.v = high.overflow;
        files.exit.set_c = files.exit.set_v = true;
    }
    value = RD8_PTR(CZP->file.lo);
    low = sub8(value, FILE_HEADER_BYTES, true);
    name_high = RD8_PTR_Y(CZP->file.lo, 1);
    high = sub8(name_high, 0, low.carry);
    push8(high.value);
    push8(low.value);
    value = RD8(CZP->file.lo);
    low = add8(value, FILE_NAME_AT, false);
    WR8(CZP->file.lo, low.value);
    value = RD8(CZP->file.hi);
    high = add8(value, 0, low.carry);
    WR8(CZP->file.hi, high.value);
    value = RD8(CZP->b.lo);
    WR8(CZP->copy.lo, value);
    value = RD8(CZP->b.hi);
    WR8(CZP->copy.hi, value);
    jsr_push(0xFAC8);
    w = bm_map_name_window();
    jsr_pop();
    y = 0;
    do {
        value = RD8_PTR_Y(CZP->file.lo, y);
        WR8_PTR_Y(CZP->copy.lo, y, value);
        y++;
    } while (y < FILE_NAME_BYTES);
    value = pull8();
    WR8_PTR_Y(CZP->copy.lo, y, value);
    y++;
    value = pull8();
    WR8_PTR_Y(CZP->copy.lo, y, value);
    (void)bm_close(BM_DONE);
    files.exit.x = w.window;
    files.exit.y = y;
    files.exit.c = true;
    files.exit.v = high.overflow;
    files.exit.set_x = files.exit.set_y = files.exit.set_c = files.exit.set_v = true;
    files.result = BM_DONE;
    return files;
}

void bm_files_exit_shim(BackupFiles files)
{
    access_exit(files);
}

uint8_t bm_files_result(BackupFiles files)
{
    return files.result;
}

void abi_bm_files(void)
{
    bm_files_exit_shim(bm_files(cpu.a));
}

/* bm_free's and bm_format's exits and results: A 0 or $FF not formatted (free); 0 or 1, a wrong password (format). */
void bm_free_exit_shim(BackupFree free_space)
{
    access_exit(free_space);
}

uint8_t bm_free_result(BackupFree free_space)
{
    return free_space.result;
}

void abi_bm_free(void)
{
    bm_free_exit_shim(bm_free(cpu.a));
}

void bm_format_exit_shim(BackupFormat format)
{
    card_exit_apply(format.exit);
    if (format.ending == BM_ENDED_DONE) {
        abi_flag(P_I, false);
    }
    cpu.a = format.result;
    abi_nz(format.result);
}

uint8_t bm_format_result(BackupFormat format)
{
    return format.result;
}

void abi_bm_format(void)
{
    bm_format_exit_shim(bm_format(cpu.a));
}
