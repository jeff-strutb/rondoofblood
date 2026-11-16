/* The backup memory: 2 KB of battery RAM in a bank of its own, opened by a port. */
#include "card.h"
#include "syscard.h"

enum {
    BACKUP_BANK = 0xF7, BACKUP_MPR = 0x10, BACKUP_OPEN = 0x80, MPR_COUNT = 8,
    BACKUP_SIGNATURE_BYTES = 4, BACKUP_PASSWORD_BYTES = 11, BACKUP_HEADER_BYTES = 0x12,
    BACKUP_START_HI = 0x80, BACKUP_FIRST_FREE = 0x8010, BACKUP_MAX_PAGES = 0x20,
    BM_BAD_PASSWORD = 1, BM_NOT_FORMATTED = 0xFF,
    WINDOW_MPR_LOW = 0x20, WINDOW_MPR_HIGH = 0x40, WINDOW_OFFSET_BITS = 0x1F, WINDOW_BASE_HI = 0xA0,
    FIRST_FILE_ADDRESS = 0x8010, FILE_SUM_AT = 2, FILE_NAME_AT = 4, FILE_NAME_BYTES = 12,
    FILE_HEADER_BYTES = 0x10, FILE_END_BYTES = 2, BM_DONE = 0, BM_NO_FILE = 1, BM_BAD_SUM = 2,
    VDC_REG_CR = 5, VDC_CR_VBLANK_IRQ = 0x08, VDC_STATUS_VBLANK = 0x20
};

/* Keeps every MPR, and maps the backup memory's bank at $8000. */
static void map_backup_bank_shim(void)
{
    for (unsigned i = 0; i < MPR_COUNT; i++)
        wr8((uint16_t)(CardSavedMprs + i), tma((uint8_t)(1u << i)));
    tam(BACKUP_MPR, BACKUP_BANK);
}

/* Puts back every MPR. */
static void unmap_backup_bank_shim(void)
{
    for (unsigned i = 0; i < MPR_COUNT; i++)
        tam((uint8_t)(1u << i), rd8((uint16_t)(CardSavedMprs + i)));
}

/* Waits out a vertical blank with its interrupt on, so that none was lost. */
static void wait_vblank_with_interrupt_shim(void)
{
    uint8_t control = rd8(CardVdcCrLo);

    wr8(VdcAddress, VDC_REG_CR);
    wr8(VdcDataLo, control | VDC_CR_VBLANK_IRQ);
    while (rd8(VdcAddress) & VDC_STATUS_VBLANK)
        ;
    while (!(rd8(VdcAddress) & VDC_STATUS_VBLANK))
        ;
    wr8(VdcDataLo, control);
    wr8(VdcAddress, rd8(CardVdcReg));
}

/* Bytes that take a zero and give it back: the memory's size, stopping at the first that will not. */
static BackupSize measure_backup_memory(void)
{
    BackupSize size = { BACKUP_START_HI, 0, BACKUP_MAX_PAGES, true };

    while (size.pages_left != 0 && size.whole) {
        size.offset = 0;
        do {
            uint16_t at = (uint16_t)((size.page << 8) + size.offset);

            wr8(at, 0);
            size.whole = rd8(at) == 0;
            if (size.whole)
                size.offset++;
        } while (size.whole && size.offset != 0);
        if (size.whole) {
            size.page++;
            size.pages_left--;
        }
    }
    return size;
}

/* A new header: the signature, where the memory ends, and no files yet. */
static void write_backup_header(BackupSize size)
{
    wr8(CardPointerLo, 0);
    wr8(CardPointerHi, size.page);
    wr8(BackupEnd, size.offset);
    wr8((uint16_t)(BackupEnd + 1), size.page);
    for (unsigned i = 0; i < BACKUP_SIGNATURE_BYTES; i++)
        wr8((uint16_t)(BackupHeader + i), rd8((uint16_t)(BackupSignature + i)));
    wr8(BackupNextFree, (uint8_t)BACKUP_FIRST_FREE);
    wr8((uint16_t)(BackupNextFree + 1), (uint8_t)(BACKUP_FIRST_FREE >> 8));
    wr8(BackupFirstFile, 0);
    wr8((uint16_t)(BackupFirstFile + 1), 0);
}

/*
 * @card 00:FC49 bm_open
 * WHAT IT DOES: Opens the backup memory at $8000, keeping every MPR.
 * HOW: The eight MPRs go to CardSavedMprs, its bank to MPR4, slow speed, interrupts off, and
 * the unlock port.
 * Entry: nothing. Exit: A kept, N and Z from it, I set. X, Y, C, V kept.
 */
void bm_open(void)
{
    map_backup_bank_shim();
    wr8(BackupUnlock, BACKUP_OPEN);
}

void abi_bm_open(void)
{
    bm_open();
    abi_nz(cpu.a);
    abi_flag(P_I, true);
}

/*
 * @card 00:FC7F bm_close
 * WHAT IT DOES: Closes the backup memory and puts back every MPR.
 * HOW: The eight MPRs back; a read of the status port locks it; full speed; then waits out a
 * vertical blank with its interrupt on, so that none was lost, and puts the VDC back.
 * Entry: nothing. Exit: A kept, N and Z from it, I clear. X, Y, C, V kept.
 */
void bm_close(void)
{
    unmap_backup_bank_shim();
    rd8(CdIrqStatus);
    wait_vblank_with_interrupt_shim();
}

void abi_bm_close(void)
{
    bm_close();
    abi_nz(cpu.a);
    abi_flag(P_I, false);
}

/*
 * @card 00:FC32 bm_open_formatted
 * WHAT IT DOES: Opens the backup memory if it has been formatted; if not, closes it and has
 * its caller return $FF.
 * HOW: Compares the header's first four bytes with the signature, from the last.
 * Entry: nothing. Exit, formatted: A the first byte, N 1, Z 0, C 1, X $FF. Exit, not: its
 * caller's caller is next, A $FF, N 1, Z 0, X where it differed, C from that compare.
 */
BackupCheck bm_open_formatted(void)
{
    BackupCheck check = { true, 0, 0, 0 };

    bm_open();
    for (int i = BACKUP_SIGNATURE_BYTES - 1; i >= 0 && check.formatted; i--) {
        check.read = rd8((uint16_t)(BackupHeader + i));
        check.expected = rd8((uint16_t)(BackupSignature + i));
        check.where = (uint8_t)i;
        check.formatted = check.read == check.expected;
    }
    if (!check.formatted)
        bm_close();
    return check;
}

void abi_bm_open_formatted(void)
{
    BackupCheck check = bm_open_formatted();

    abi_flag(P_C, check.read >= check.expected);
    abi_flag(P_I, check.formatted);
    if (check.formatted) {
        cpu.a = check.read;
        cpu.x = 0xFF;
        abi_nz(0xFF);
    } else {
        cpu.a = BM_NOT_FORMATTED;
        cpu.x = check.where;
        abi_nz(BM_NOT_FORMATTED);
    }
}

static uint16_t backup_word(uint16_t lo)
{
    uint8_t low = rd8(lo);

    return (uint16_t)(low | (rd8((uint16_t)(lo + 1)) << 8));
}

/*
 * The backup memory is not RAM to the oracles: its bytes are read and written in the order and
 * number the original reads and writes them, a byte at a time.
 */

/* One word less another, read as a subtract reads them: both low bytes, then both high. */
static uint16_t backup_difference(uint16_t from, uint16_t less)
{
    uint8_t from_low = rd8(from);
    uint8_t less_low = rd8(less);
    uint8_t from_high = rd8((uint16_t)(from + 1));
    uint8_t less_high = rd8((uint16_t)(less + 1));

    return (uint16_t)(((from_high << 8) | from_low) - ((less_high << 8) | less_low));
}

/* A word changed in place: its low byte read and written, then its high byte. */
static void backup_word_add(uint16_t at, uint16_t value)
{
    uint8_t low = rd8(at);
    unsigned carry = (low + (value & 0xFF)) >> 8;

    wr8(at, (uint8_t)(low + value));
    wr8((uint16_t)(at + 1), (uint8_t)(rd8((uint16_t)(at + 1)) + (value >> 8) + carry));
}

/* A word less the word at another address, in place, each byte read as it is needed. */
static void backup_word_take(uint16_t at, uint16_t less_at)
{
    uint8_t low = rd8(at);
    uint8_t less_low = rd8(less_at);
    unsigned borrow = low < less_low;

    wr8(at, (uint8_t)(low - less_low));
    low = rd8((uint16_t)(at + 1));
    wr8((uint16_t)(at + 1), (uint8_t)(low - rd8((uint16_t)(less_at + 1)) - borrow));
}

/* The answer goes in _CX. */
static void give_free_bytes(uint16_t bytes)
{
    wr8(CardCl, (uint8_t)bytes);
    wr8(CardCh, (uint8_t)(bytes >> 8));
}

/* Maps the two banks a caller's own pages were at into $A000-$DFFF. */
static void map_window_banks_shim(uint8_t window)
{
    tam(WINDOW_MPR_LOW, rd8((uint16_t)(CardSavedMprs + window)));
    tam(WINDOW_MPR_HIGH, rd8((uint16_t)(CardSavedMprs + 1 + window)));
}

/*
 * @card 00:FD7B bm_map_name_window
 * WHAT IT DOES: Makes the pointer in CardCopyLo reach what it pointed to before the backup
 * memory was opened.
 * HOW: Its top three bits choose the saved MPR; the banks there and after it go at $A000, and
 * the pointer is rebased there.
 * Entry: CardCopyLo-Hi. Exit: A the pointer's new high byte, N and Z from it, X the saved MPR,
 * C bit 4 of the old high byte. Y, V kept. Gives the saved MPR.
 */
uint8_t bm_map_name_window(void)
{
    uint8_t high = rd8(CardCopyHi);
    uint8_t window = high >> 5;

    map_window_banks_shim(window);
    wr8(CardCopyHi, (uint8_t)((high & WINDOW_OFFSET_BITS) | WINDOW_BASE_HI));
    return window;
}

void abi_bm_map_name_window(void)
{
    uint8_t high = rd8(CardCopyHi);

    cpu.x = bm_map_name_window();
    cpu.a = rd8(CardCopyHi);
    abi_nz(cpu.a);
    abi_flag(P_C, (high & 0x10) != 0);
}

/*
 * @card 00:FD97 bm_count_down
 * WHAT IT DOES: Takes one from the count of bytes left.
 * HOW: A 16-bit decrement of CardByteCount.
 * Entry: CardByteCount. Exit: A its two bytes ORed, N and Z from that. X, Y, C, V kept. Gives
 * what is left.
 */
uint16_t bm_count_down(void)
{
    uint16_t left = (uint16_t)(backup_word(CardByteCount) - 1);

    wr8(CardByteCount, (uint8_t)left);
    wr8((uint16_t)(CardByteCount + 1), (uint8_t)(left >> 8));
    return left;
}

void abi_bm_count_down(void)
{
    uint16_t left = bm_count_down();

    cpu.a = (uint8_t)left | (uint8_t)(left >> 8);
    abi_nz(cpu.a);
}

/* Whether the twelve bytes of a file's name at its offset are the name asked for. */
static bool file_name_matches(uint16_t file, uint16_t name)
{
    for (unsigned i = 0; i < FILE_NAME_BYTES; i++)
        if (rd8((uint16_t)(name + i)) != rd8((uint16_t)(file + FILE_NAME_AT + i)))
            return false;
    return true;
}

/* V as bm_find_file leaves it: from taking the name's offset off again, or from its last step. */
static void abi_find_v_shim(BackupFind find)
{
    uint16_t compared_at = (uint16_t)(find.file + FILE_NAME_AT);
    uint16_t reached = (uint16_t)(find.last_file + find.last_length);

    if (find.found)
        abi_flag(P_V, (compared_at >> 8) == 0x80 && (compared_at & 0xFF) < FILE_NAME_AT);
    else if (find.stepped)
        abi_flag(P_V, ((find.last_file ^ reached) & (find.last_length ^ reached) & 0x8000) != 0);
}

/*
 * @card 00:FB96 bm_find_file
 * WHAT IT DOES: Looks through the backup memory's files for the name at _AX.
 * HOW: Each file starts with its length; a length of zero ends the list. The name is compared
 * through the window, and the file reached is left in CardFileLo.
 * Entry: _AX the name's address, backup memory open. Exit, found: C clear, A the file's high
 * byte, N and Z from it, Y $0C, V from taking the name's offset off again. Exit, not found: C
 * set, A 0, N 0, Z 1, Y 1, V from the last step, or kept if there were no files. X the saved MPR.
 */
BackupFind bm_find_file(void)
{
    BackupFind find = { false, FIRST_FILE_ADDRESS, 0, false, 0, 0 };
    uint16_t name;

    wr8(CardCopyLo, rd8(CardAl));
    wr8(CardCopyHi, rd8((uint16_t)(CardAl + 1)));
    find.window = bm_map_name_window();
    name = backup_word(CardCopyLo);
    for (;;) {
        uint16_t length = backup_word(find.file);

        if (length == 0)
            break;
        find.found = file_name_matches(find.file, name);
        if (find.found)
            break;
        find.stepped = true;
        find.last_file = find.file;
        find.last_length = length;
        find.file = (uint16_t)(find.file + length);
    }
    wr8(CardFileLo, (uint8_t)find.file);
    wr8(CardFileHi, (uint8_t)(find.file >> 8));
    return find;
}

void abi_bm_find_file(void)
{
    BackupFind find = bm_find_file();

    cpu.x = find.window;
    abi_flag(P_C, !find.found);
    if (find.found) {
        cpu.a = (uint8_t)(find.file >> 8);
        cpu.y = FILE_NAME_BYTES;
        abi_nz(cpu.a);
    } else {
        cpu.a = 0;
        cpu.y = 1;
        abi_nz(0);
    }
    abi_find_v_shim(find);
}

/*
 * @card 00:F858 bm_format
 * WHAT IT DOES: Formats the backup memory, given the password that guards it.
 * HOW: Compares eleven bytes with the password; measures how much RAM answers; writes the
 * header: signature, end, next free byte, and no files.
 * Entry: _AX the password's address. Exit, wrong password: A 1, N 0, Z 0, Y where it differed,
 * C from that compare. Exit: A 0, N 0, Z 1, X $FF, Y the bytes short of a page. V kept.
 */
BackupFormat bm_format(void)
{
    BackupFormat format = { true, 0, 0, 0, { 0, 0, 0, false } };
    uint16_t given = backup_word(CardAl);

    for (int i = BACKUP_PASSWORD_BYTES - 1; i >= 0 && format.accepted; i--) {
        format.expected = rd8((uint16_t)(BackupFormatPassword + i));
        format.read = rd8((uint16_t)(given + i));
        format.where = (uint8_t)i;
        format.accepted = format.expected == format.read;
    }
    if (!format.accepted)
        return format;
    bm_open();
    format.size = measure_backup_memory();
    write_backup_header(format.size);
    bm_close();
    return format;
}

void abi_bm_format(void)
{
    BackupFormat format = bm_format();

    if (!format.accepted) {
        cpu.a = BM_BAD_PASSWORD;
        cpu.y = format.where;
        abi_nz(BM_BAD_PASSWORD);
        abi_flag(P_C, format.expected >= format.read);
        return;
    }
    cpu.a = 0;
    cpu.x = 0xFF;
    cpu.y = format.size.offset;
    abi_nz(0);
    abi_flag(P_C, format.size.whole);
    abi_flag(P_I, false);
}

/*
 * @card 00:F8B8 bm_free
 * WHAT IT DOES: Gives how many bytes of the backup memory are free for another file.
 * HOW: Its end less its next free byte, less a file's header of $12 bytes, and zero if that
 * is below zero. The answer goes in _CX.
 * Entry: nothing. Exit: A 0, N 0, Z 1, X $FF, C and V from the subtract's high byte; or $FF as
 * bm_open_formatted if not formatted. Y kept.
 */
BackupFree bm_free(void)
{
    BackupFree free_space = { 0 };

    free_space.check = bm_open_formatted();
    if (!free_space.check.formatted)
        return free_space;
    free_space.room = backup_difference(BackupEnd, BackupNextFree);
    free_space.after_header = (uint16_t)(free_space.room - BACKUP_HEADER_BYTES);
    give_free_bytes((int16_t)free_space.after_header < 0 ? 0 : free_space.after_header);
    bm_close();
    return free_space;
}

void abi_bm_free(void)
{
    BackupFree free_space = bm_free();

    abi_flag(P_I, false);
    if (!free_space.check.formatted) {
        abi_flag(P_C, free_space.check.read >= free_space.check.expected);
        cpu.a = BM_NOT_FORMATTED;
        cpu.x = free_space.check.where;
        abi_nz(BM_NOT_FORMATTED);
        return;
    }
    cpu.a = 0;
    cpu.x = 0xFF;
    abi_nz(0);
    abi_flag(P_C, free_space.room >= BACKUP_HEADER_BYTES);
    abi_flag(P_V, ((free_space.room ^ BACKUP_HEADER_BYTES) & (free_space.room ^ free_space.after_header) & 0x8000) != 0);
}

static void put_word(uint16_t lo, uint16_t value)
{
    wr8(lo, (uint8_t)value);
    wr8((uint16_t)(lo + 1), (uint8_t)(value >> 8));
}

/* Whether an 8-bit add or subtract with its carry in leaves the signed range. */
static bool add_overflows(uint8_t a, uint8_t b, bool carry)
{
    int r = (int8_t)a + (int8_t)b + (carry ? 1 : 0);

    return r < -128 || r > 127;
}

static bool sub_overflows(uint8_t a, uint8_t b, bool borrow)
{
    int r = (int8_t)a - (int8_t)b - (borrow ? 1 : 0);

    return r < -128 || r > 127;
}

/* The exit of a call that found the memory unformatted: as bm_open_formatted leaves it. */
static void abi_not_formatted_shim(BackupCheck check)
{
    abi_flag(P_I, false);
    abi_flag(P_C, check.read >= check.expected);
    cpu.a = BM_NOT_FORMATTED;
    cpu.x = check.where;
    abi_nz(BM_NOT_FORMATTED);
}

/*
 * @card 00:FBFC bm_sum_file
 * WHAT IT DOES: Sums the name and data of the file at CardFileLo, for its checksum.
 * HOW: Adds every byte after the length and checksum words into a 16-bit sum in CardCopyLo,
 * counting the file's length less four down in CardByteCount. A length of four sums $10000
 * bytes.
 * Entry: CardFileLo. Exit: A 0, N 0, Z 1, Y the low byte of four plus the bytes summed, C and V
 * from the last add's high byte. X kept. CardPointerLo-Hi the file, its high byte moved on by
 * the pages crossed.
 */
BackupSum bm_sum_file(void)
{
    uint16_t file = backup_word(CardFileLo);
    uint16_t count = (uint16_t)(backup_word(file) - FILE_NAME_AT);
    BackupSum sum = { 0, 0, 0, count ? count : 0x10000u };

    for (uint32_t i = 0; i < sum.bytes; i++) {
        sum.last_byte = rd8((uint16_t)(file + FILE_NAME_AT + i));
        sum.before_last = sum.sum;
        sum.sum = (uint16_t)(sum.sum + sum.last_byte);
    }
    wr8(CardPointerLo, (uint8_t)file);
    wr8(CardPointerHi, (uint8_t)((file >> 8) + ((FILE_NAME_AT + sum.bytes) >> 8)));
    put_word(CardCopyLo, sum.sum);
    put_word(CardByteCount, 0);
    return sum;
}

void abi_bm_sum_file(void)
{
    BackupSum sum = bm_sum_file();

    cpu.a = 0;
    cpu.y = (uint8_t)(FILE_NAME_AT + sum.bytes);
    abi_nz(0);
    abi_flag(P_C, (uint32_t)sum.before_last + sum.last_byte > 0xFFFF);
    abi_flag(P_V, sum.before_last < 0x8000 && sum.before_last + sum.last_byte >= 0x8000);
}

static void abi_span_flags_shim(BackupSpan span)
{
    bool low_carry = (span.offset & 0xFF) + (span.data & 0xFF) > 0xFF;

    abi_flag(P_C, (uint32_t)span.offset + span.data > 0xFFFF);
    abi_flag(P_V, add_overflows((uint8_t)(span.offset >> 8), (uint8_t)(span.data >> 8), low_carry));
}

/*
 * @card 00:FB46 bm_place_span
 * WHAT IT DOES: Places a read or write of _CX bytes at offset _DX in the file at CardFileLo.
 * HOW: How far it runs past the file's end goes to CardCopyLo (negative when it fits), the
 * address in the file it starts at to CardPointerLo, and _CX to CardByteCount.
 * Entry: _CX, _DX, CardFileLo. Exit: A the high byte of the overrun, N and Z from it, Y 1, C and V
 * from adding _DX to the file's data address. X kept.
 */
BackupSpan bm_place_span(void)
{
    uint16_t file = backup_word(CardFileLo);
    uint16_t count = backup_word(CardCl);
    BackupSpan span;

    span.offset = backup_word(CardDl);
    span.data = (uint16_t)(file + FILE_HEADER_BYTES);
    span.overrun = (uint16_t)(span.offset + count + FILE_HEADER_BYTES - backup_word(file));
    put_word(CardCopyLo, span.overrun);
    put_word(CardPointerLo, (uint16_t)(span.data + span.offset));
    put_word(CardByteCount, count);
    return span;
}

void abi_bm_place_span(void)
{
    BackupSpan span = bm_place_span();

    cpu.a = (uint8_t)(span.overrun >> 8);
    cpu.y = 1;
    abi_nz(cpu.a);
    abi_span_flags_shim(span);
}

static void abi_move_way_flags_shim(BackupMoveWay way)
{
    if (way.forward) {
        abi_flag(P_C, true);
        abi_flag(P_V, sub_overflows((uint8_t)(way.source >> 8), (uint8_t)(way.destination >> 8),
                                    (way.source & 0xFF) < (way.destination & 0xFF)));
    } else {
        uint16_t last = (uint16_t)(way.count - 1);

        abi_flag(P_C, (uint32_t)way.destination + last > 0xFFFF);
        abi_flag(P_V, add_overflows((uint8_t)(way.destination >> 8), (uint8_t)(last >> 8),
                                    (way.destination & 0xFF) + (last & 0xFF) > 0xFF));
    }
}

/*
 * @card 00:FD41 bm_move_direction
 * WHAT IT DOES: Chooses which way to move bytes inside the backup memory, so that a move onto
 * itself is not spoiled.
 * HOW: A source at or above its destination goes first byte first. One below goes last byte
 * first: both pointers are moved on to their last byte.
 * Entry: CardPointerLo the source, CardCopyLo the destination, CardByteCount the bytes.
 * Exit, forward: X 2, A the high bytes' difference, N 0, Z 0, C 1, V from that subtract.
 * Exit, backward: X 0, A the destination's new high byte, N 0, Z 1, C and V from moving it on.
 * Y kept.
 */
BackupMoveWay bm_move_direction(void)
{
    BackupMoveWay way;
    uint16_t last;

    way.source = backup_word(CardPointerLo);
    way.destination = backup_word(CardCopyLo);
    way.count = backup_word(CardByteCount);
    way.forward = way.source >= way.destination;
    if (way.forward)
        return way;
    last = (uint16_t)(way.count - 1);
    put_word(CardPointerLo, (uint16_t)(way.source + last));
    put_word(CardCopyLo, (uint16_t)(way.destination + last));
    return way;
}

void abi_bm_move_direction(void)
{
    BackupMoveWay way = bm_move_direction();
    uint16_t borrowed = (way.source & 0xFF) < (way.destination & 0xFF);

    if (way.forward) {
        cpu.x = 2;
        cpu.a = (uint8_t)((way.source >> 8) - (way.destination >> 8) - borrowed);
        abi_nz(2);
    } else {
        cpu.x = 0;
        cpu.a = rd8(CardCopyHi);
        abi_nz(0);
    }
    abi_move_way_flags_shim(way);
}

/*
 * @card 00:FCD3 bm_move
 * WHAT IT DOES: Moves CardByteCount bytes inside the backup memory, then marks the end of the
 * file list at its next free byte.
 * HOW: bm_move_direction picks the way; bytes go one at a time, each pointer moved a page at a
 * time; then a zero length is written at the next free byte.
 * Entry: CardPointerLo the source, CardCopyLo the destination, CardByteCount. Exit: A 0, N 0,
 * Z 1, Y 1, CardPointerLo the next free byte. Nothing to move: X, C, V kept. Else X, C and V
 * as bm_move_direction leaves them.
 */
BackupMove bm_move(void)
{
    BackupMove move = { false, { false, 0, 0, 0 } };
    uint16_t count = backup_word(CardByteCount);
    uint16_t next_free;

    if (count != 0) {
        uint16_t from, to;
        uint8_t pages;

        move.moved = true;
        move.way = bm_move_direction();
        from = backup_word(CardPointerLo);
        to = backup_word(CardCopyLo);
        if (move.way.forward) {
            for (uint16_t i = 0; i < count; i++)
                wr8((uint16_t)(to + i), rd8((uint16_t)(from + i)));
            pages = (uint8_t)(count >> 8);
            wr8(CardCopyHi, (uint8_t)((to >> 8) + pages));
        } else {
            for (uint16_t i = 0; i < count; i++)
                wr8((uint16_t)(to - i), rd8((uint16_t)(from - i)));
            pages = (uint8_t)(((count - 1) >> 8) + 1);
            wr8(CardCopyHi, (uint8_t)((to >> 8) - pages));
        }
        put_word(CardByteCount, 0);
    }
    next_free = backup_word(BackupNextFree);
    put_word(CardPointerLo, next_free);
    put_word(next_free, 0);
    return move;
}

void abi_bm_move(void)
{
    BackupMove move = bm_move();

    cpu.a = 0;
    cpu.y = 1;
    abi_nz(0);
    if (move.moved) {
        cpu.x = move.way.forward ? 2 : 0;
        abi_move_way_flags_shim(move.way);
    }
}

/* Copies CardByteCount bytes between the backup memory and the caller's memory, through the window. */
static BackupCopy copy_through_window(bool to_caller)
{
    BackupCopy copy;
    uint16_t count = backup_word(CardByteCount);
    uint16_t backup, caller;

    copy.old_high = rd8(CardCopyHi);
    copy.window = bm_map_name_window();
    copy.bytes = count ? count : 0x10000u;
    backup = backup_word(CardPointerLo);
    caller = backup_word(CardCopyLo);
    for (uint32_t i = 0; i < copy.bytes; i++) {
        if (to_caller)
            wr8((uint16_t)(caller + i), rd8((uint16_t)(backup + i)));
        else
            wr8((uint16_t)(backup + i), rd8((uint16_t)(caller + i)));
    }
    wr8(CardPointerHi, (uint8_t)((backup >> 8) + (copy.bytes >> 8)));
    wr8(CardCopyHi, (uint8_t)((caller >> 8) + (copy.bytes >> 8)));
    put_word(CardByteCount, 0);
    return copy;
}

static void abi_copy_exit_shim(BackupCopy copy)
{
    cpu.a = 0;
    cpu.x = copy.window;
    cpu.y = (uint8_t)copy.bytes;
    abi_nz(0);
    abi_flag(P_C, (copy.old_high & 0x10) != 0);
}

/*
 * @card 00:FD17 bm_copy_out
 * WHAT IT DOES: Copies bytes from the backup memory to the caller's memory.
 * HOW: The caller's pointer in CardCopyLo is put through the window, then CardByteCount bytes go
 * one at a time from CardPointerLo; a count of zero copies $10000.
 * Entry: CardPointerLo, CardCopyLo, CardByteCount. Exit: A 0, N 0, Z 1, X the saved MPR, Y the
 * low byte of the count, C bit 4 of the caller's old high byte. V kept.
 */
BackupCopy bm_copy_out(void)
{
    return copy_through_window(true);
}

void abi_bm_copy_out(void)
{
    abi_copy_exit_shim(bm_copy_out());
}

/*
 * @card 00:FD2C bm_copy_in
 * WHAT IT DOES: Copies bytes from the caller's memory into the backup memory.
 * HOW: As bm_copy_out, the other way.
 * Entry: CardPointerLo, CardCopyLo, CardByteCount. Exit: as bm_copy_out.
 */
BackupCopy bm_copy_in(void)
{
    return copy_through_window(false);
}

void abi_bm_copy_in(void)
{
    abi_copy_exit_shim(bm_copy_in());
}

/*
 * @card 00:FAE2 bm_new_file
 * WHAT IT DOES: Starts a new, empty file named by _AX at the backup memory's next free byte.
 * HOW: Works out the room left into CardPointerLo, but tests CardCopyLo-Hi against $10 instead,
 * which the name's pointer always passes. Then copies the twelve bytes of the name through the
 * window, moves the next free byte on by a header, and gives the file a length of $10.
 * Entry: _AX the name, CardCopyLo-Hi. Exit, made: C clear, A 0, N 0, Z 1, Y 1, X the saved MPR, V
 * from moving the next free byte on. Exit, refused: C set, A the high byte of the test, N and Z
 * from it, V from it. X and Y kept.
 */
BackupNewFile bm_new_file(void)
{
    BackupNewFile made = { false, 0, 0, { 0, 0, 0 } };
    uint16_t name_at, name;

    made.next_free = backup_word(BackupNextFree);
    put_word(CardFileLo, made.next_free);
    put_word(CardPointerLo, (uint16_t)(backup_word(BackupEnd) - made.next_free));
    made.pointer = backup_word(CardCopyLo);
    made.made = made.pointer >= FILE_HEADER_BYTES;
    if (!made.made)
        return made;
    name_at = (uint16_t)(made.next_free + FILE_NAME_AT);
    put_word(CardPointerLo, name_at);
    wr8(CardCopyLo, rd8(CardAl));
    wr8(CardCopyHi, rd8(CardAh));
    made.copy.old_high = rd8(CardAh);
    made.copy.window = bm_map_name_window();
    name = backup_word(CardCopyLo);
    for (unsigned i = 0; i < FILE_NAME_BYTES; i++)
        wr8((uint16_t)(name_at + i), rd8((uint16_t)(name + i)));
    backup_word_add(BackupNextFree, FILE_HEADER_BYTES);
    put_word(made.next_free, FILE_HEADER_BYTES);
    return made;
}

/* The exit of bm_new_file's refusal, which bm_write passes on. */
static void abi_new_file_refused_shim(BackupNewFile made)
{
    bool borrowed = (made.pointer & 0xFF) < FILE_HEADER_BYTES;

    cpu.a = (uint8_t)((made.pointer >> 8) - borrowed);
    abi_nz(cpu.a);
    abi_flag(P_C, true);
    abi_flag(P_V, sub_overflows((uint8_t)(made.pointer >> 8), 0, borrowed));
}

void abi_bm_new_file(void)
{
    BackupNewFile made = bm_new_file();

    if (!made.made) {
        abi_new_file_refused_shim(made);
        return;
    }
    cpu.a = 0;
    cpu.x = made.copy.window;
    cpu.y = 1;
    abi_nz(0);
    abi_flag(P_C, false);
    abi_flag(P_V, add_overflows((uint8_t)(made.next_free >> 8), 0,
                                (made.next_free & 0xFF) + FILE_HEADER_BYTES > 0xFF));
}

/* Whether _DX is at or past a file's length: its high byte is compared first, its low only if needed. */
static bool offset_past_end(uint16_t file)
{
    uint8_t length_high = rd8((uint16_t)(file + 1));

    if (rd8(CardDh) != length_high)
        return rd8(CardDh) > length_high;
    return rd8(CardDl) >= rd8(file);
}

/* Every exit closes the memory: I clear, A the result, N and Z from it. */
static void abi_access_result_shim(BackupAccess access)
{
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
 * Entry: _AX, _BX, _CX, _DX. Exit: I clear, A the result, N and Z from it; not formatted, as
 * bm_open_formatted. No file: X the saved MPR, Y 1, C 1, V as bm_find_file. Offset past the end:
 * _CX 0, the same with A 0. Read: bytes copied, X the saved MPR, Y the count's low byte, C bit 4
 * of _BH; nothing copied, X as bm_find_file, Y 1, C from the cut or the place; V from the cut or
 * the place.
 */
BackupAccess bm_read(void)
{
    BackupAccess read = { 0 };
    bool bad_sum;

    read.check = bm_open_formatted();
    if (!read.check.formatted) {
        read.ending = BM_ENDED_NOT_FORMATTED;
        read.result = BM_NOT_FORMATTED;
        return read;
    }
    read.find = bm_find_file();
    if (!read.find.found) {
        read.ending = BM_ENDED_NOT_FOUND;
        read.result = BM_NO_FILE;
        bm_close();
        return read;
    }
    if (offset_past_end(read.find.file)) {
        put_word(CardCl, 0);
        read.ending = BM_ENDED_PAST_END;
        read.result = BM_DONE;
        bm_close();
        return read;
    }
    read.sum = bm_sum_file();
    bad_sum = (uint16_t)(read.sum.sum + backup_word((uint16_t)(read.find.file + FILE_SUM_AT))) != 0;
    read.span = bm_place_span();
    read.count_before = backup_word(CardByteCount);
    read.count = read.count_before;
    read.truncated = (read.span.overrun & 0x8000) == 0;
    if (read.truncated) {
        read.count = (uint16_t)(read.count_before - read.span.overrun);
        put_word(CardByteCount, read.count);
    }
    put_word(CardCl, read.count);
    if (read.count != 0) {
        put_word(CardCopyLo, backup_word(CardBl));
        read.copy = bm_copy_out();
    }
    read.ending = BM_ENDED_DONE;
    read.result = bad_sum ? BM_BAD_SUM : BM_DONE;
    bm_close();
    return read;
}

/* C and V from cutting a read at the file's end, or from placing it. */
static void abi_cut_flags_shim(BackupAccess read)
{
    if (!read.truncated) {
        abi_span_flags_shim(read.span);
        return;
    }
    bool borrowed = (read.count_before & 0xFF) < (read.span.overrun & 0xFF);

    abi_flag(P_C, read.count_before >= read.span.overrun);
    abi_flag(P_V, sub_overflows((uint8_t)(read.count_before >> 8), (uint8_t)(read.span.overrun >> 8),
                                borrowed));
}

void abi_bm_read(void)
{
    BackupAccess read = bm_read();

    if (read.ending == BM_ENDED_NOT_FORMATTED) {
        abi_not_formatted_shim(read.check);
        return;
    }
    abi_access_result_shim(read);
    if (read.ending != BM_ENDED_DONE) {
        cpu.x = read.find.window;
        cpu.y = 1;
        abi_flag(P_C, true);
        abi_find_v_shim(read.find);
        return;
    }
    abi_cut_flags_shim(read);
    if (read.count != 0) {
        cpu.x = read.copy.window;
        cpu.y = (uint8_t)read.copy.bytes;
        abi_flag(P_C, (read.copy.old_high & 0x10) != 0);
    } else {
        cpu.x = read.find.window;
        cpu.y = 1;
    }
}

/*
 * @card 00:F955 bm_write
 * WHAT IT DOES: Writes _CX bytes from _BX at offset _DX of the backup file named by _AX, making
 * the file if there is none, and growing it if the write runs past its end.
 * HOW: A file that must grow moves every file after it up by the growth, if the memory has room
 * for that and a closing zero length; then the bytes are copied in through the window and the
 * file's checksum is made the negative of its sum.
 * Entry: _AX, _BX, _CX, _DX. Exit: I clear, A the result, N and Z from it; not formatted, as
 * bm_open_formatted. No room for a header: A 1, X as bm_find_file, Y 1, C, V as bm_new_file. No
 * room to grow: A 1, X the saved MPR, Y 1, C 0, V from that compare. Written: A 0, X the saved
 * MPR, Y 3, C and V from negating the sum's high byte.
 */
BackupAccess bm_write(void)
{
    BackupAccess write = { 0 };
    uint16_t file, sum;

    write.check = bm_open_formatted();
    if (!write.check.formatted) {
        write.ending = BM_ENDED_NOT_FORMATTED;
        write.result = BM_NOT_FORMATTED;
        return write;
    }
    write.find = bm_find_file();
    if (!write.find.found) {
        write.new_file = bm_new_file();
        if (!write.new_file.made) {
            write.ending = BM_ENDED_NOT_FOUND;
            write.result = BM_NO_FILE;
            bm_close();
            return write;
        }
    }
    file = backup_word(CardFileLo);
    write.span = bm_place_span();
    if ((write.span.overrun & 0x8000) == 0) {
        uint16_t growth = write.span.overrun;
        uint16_t next_free = (uint16_t)(backup_word(BackupNextFree) + growth);
        uint16_t file_end;

        put_word(CardByteCount, growth);
        put_word(CardCopyLo, next_free);
        write.end = backup_word(BackupEnd);
        write.needed = (uint16_t)(next_free + FILE_END_BYTES);
        put_word(CardPointerLo, write.needed);
        if (write.end < write.needed) {
            write.ending = BM_ENDED_NO_ROOM;
            write.result = BM_NO_FILE;
            bm_close();
            return write;
        }
        put_word(BackupNextFree, next_free);
        file_end = (uint16_t)(file + backup_word(file));
        backup_word_add(file, growth);
        put_word(CardPointerLo, file_end);
        put_word(CardCopyLo, (uint16_t)(file_end + growth));
        put_word(CardByteCount, (uint16_t)(backup_word(BackupNextFree) - file_end));
        write.move = bm_move();
        write.span = bm_place_span();
    }
    put_word(CardCopyLo, backup_word(CardBl));
    write.copy = bm_copy_in();
    write.sum = bm_sum_file();
    sum = write.sum.sum;
    put_word((uint16_t)(file + FILE_SUM_AT), (uint16_t)(0 - sum));
    write.ending = BM_ENDED_DONE;
    write.result = BM_DONE;
    bm_close();
    return write;
}

void abi_bm_write(void)
{
    BackupAccess write = bm_write();
    uint16_t sum = write.sum.sum;

    if (write.ending == BM_ENDED_NOT_FORMATTED) {
        abi_not_formatted_shim(write.check);
        return;
    }
    if (write.ending == BM_ENDED_NOT_FOUND) {
        abi_new_file_refused_shim(write.new_file);
        cpu.x = write.find.window;
        cpu.y = 1;
        abi_access_result_shim(write);
        return;
    }
    abi_access_result_shim(write);
    if (write.ending == BM_ENDED_NO_ROOM) {
        cpu.x = write.find.found ? write.find.window : write.new_file.copy.window;
        cpu.y = 1;
        abi_flag(P_C, false);
        abi_flag(P_V, sub_overflows((uint8_t)(write.end >> 8), (uint8_t)(write.needed >> 8),
                                    (write.end & 0xFF) < (write.needed & 0xFF)));
        return;
    }
    cpu.x = write.copy.window;
    cpu.y = 3;
    abi_flag(P_C, sum == 0);
    abi_flag(P_V, sub_overflows(0, (uint8_t)(sum >> 8), (sum & 0xFF) != 0));
}

/*
 * @card 00:FA1A bm_delete
 * WHAT IT DOES: Deletes the backup file named by _AX.
 * HOW: Moves every file after it down over it, takes its length off the next free byte, and
 * marks the list's new end.
 * Entry: _AX. Exit: I clear, A the result, N and Z from it; not formatted, as bm_open_formatted.
 * No file: A 1, X the saved MPR, Y 1, C 1, V as bm_find_file. Deleted: A 0, Y 1; X, C and V as
 * bm_move leaves them, or, with nothing after it, X the saved MPR and C and V from taking the
 * length off.
 */
BackupAccess bm_delete(void)
{
    BackupAccess removal = { 0 };
    uint16_t following;

    removal.check = bm_open_formatted();
    if (!removal.check.formatted) {
        removal.ending = BM_ENDED_NOT_FORMATTED;
        removal.result = BM_NOT_FORMATTED;
        return removal;
    }
    removal.find = bm_find_file();
    if (!removal.find.found) {
        removal.ending = BM_ENDED_NOT_FOUND;
        removal.result = BM_NO_FILE;
        bm_close();
        return removal;
    }
    removal.length = backup_word(removal.find.file);
    removal.next_free = backup_word(BackupNextFree);
    following = (uint16_t)(removal.find.file + removal.length);
    put_word(CardCopyLo, removal.find.file);
    put_word(CardPointerLo, following);
    put_word(CardByteCount, (uint16_t)(removal.next_free - following));
    backup_word_take(BackupNextFree, removal.find.file);
    removal.move = bm_move();
    removal.ending = BM_ENDED_DONE;
    removal.result = BM_DONE;
    bm_close();
    return removal;
}

void abi_bm_delete(void)
{
    BackupAccess removal = bm_delete();

    if (removal.ending == BM_ENDED_NOT_FORMATTED) {
        abi_not_formatted_shim(removal.check);
        return;
    }
    abi_access_result_shim(removal);
    cpu.y = 1;
    if (removal.ending == BM_ENDED_NOT_FOUND) {
        cpu.x = removal.find.window;
        abi_flag(P_C, true);
        abi_find_v_shim(removal.find);
    } else if (removal.move.moved) {
        cpu.x = removal.move.way.forward ? 2 : 0;
        abi_move_way_flags_shim(removal.move.way);
    } else {
        cpu.x = removal.find.window;
        abi_flag(P_C, removal.next_free >= removal.length);
        abi_flag(P_V, sub_overflows((uint8_t)(removal.next_free >> 8), (uint8_t)(removal.length >> 8),
                                    (removal.next_free & 0xFF) < (removal.length & 0xFF)));
    }
}
