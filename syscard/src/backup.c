/* The backup memory: 2 KB of battery RAM in a bank of its own, opened by a port. */
#include "card.h"
#include "syscard.h"

enum {
    BACKUP_BANK = 0xF7, BACKUP_MPR = 0x10, BACKUP_OPEN = 0x80, MPR_COUNT = 8,
    BACKUP_SIGNATURE_BYTES = 4, BACKUP_PASSWORD_BYTES = 11, BACKUP_HEADER_BYTES = 0x12,
    BACKUP_START_HI = 0x80, BACKUP_FIRST_FREE = 0x8010, BACKUP_MAX_PAGES = 0x20,
    BM_BAD_PASSWORD = 1, BM_NOT_FORMATTED = 0xFF,
    WINDOW_MPR_LOW = 0x20, WINDOW_MPR_HIGH = 0x40, WINDOW_OFFSET_BITS = 0x1F, WINDOW_BASE_HI = 0xA0,
    FIRST_FILE_ADDRESS = 0x8010, FILE_NAME_AT = 4, FILE_NAME_BYTES = 12,
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
    uint16_t compared_at = (uint16_t)(find.file + FILE_NAME_AT);

    cpu.x = find.window;
    abi_flag(P_C, !find.found);
    if (find.found) {
        cpu.a = (uint8_t)(find.file >> 8);
        cpu.y = FILE_NAME_BYTES;
        abi_nz(cpu.a);
        abi_flag(P_V, (compared_at >> 8) == 0x80 && (compared_at & 0xFF) < FILE_NAME_AT);
    } else {
        uint16_t reached = (uint16_t)(find.last_file + find.last_length);

        cpu.a = 0;
        cpu.y = 1;
        abi_nz(0);
        if (find.stepped)
            abi_flag(P_V, ((find.last_file ^ reached) & (find.last_length ^ reached) & 0x8000) != 0);
    }
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
    free_space.room = (uint16_t)(backup_word(BackupEnd) - backup_word(BackupNextFree));
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
