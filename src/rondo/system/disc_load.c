/*
 * Loading from the disc through the System Card's BIOS, and the backup file. A record
 * holds a disc address as the BIOS wants it, with the disc's offset to the track taken
 * off ($000F32). The BIOS answers in A and is asked again until it answers zero; every
 * call sees A, X, Y and the flags, so they are kept in an Alu and in cpu.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

/* The disc addresses in the records count from the disc's start; the BIOS's, from the track's. */
#define TRACK_START 0x000F32u
#define FIXED_ADPCM_ADDRESS 0x0010ABu

enum {
    MPR4 = 0x10, MPR5 = 0x20, MPR6 = 0x40, CD_TRACK_ENTRY = 6, CD_PLAY_FROM_MSF = 0x40,
    CD_PLAY_ONCE = 0x42, CD_PLAY_REPEAT = 0x41, NO_TRACK = 0xFF,
    STREAM_RECORD_BYTES = 5, CPLAY_MODE = 0x0E,
    FIXED_ADPCM_SKIP = 0x20, FIXED_ADPCM_SECTORS = 0x20,
    SCENE_RECORD_BYTES = 5, SCENE_OVERLAY_BANK = 0x04, SCENE_READ_BYTES = 3, SCENE_BANKS_BYTES = 4,
    SCENE_CALL_FIRST = 0, SCENE_CALL_ONE = 2, SCENE_CALL_SECOND = 4, SCENE_CALL_OTHER = 0x10,
    SCENE_TILE_BYTES = 0x1400, SCENE_TILES_FROM = 0xC000, SCENE_TILES_TO = 0xA000,
    CD_STATUS_IDLE = 0,
    BACKUP_NAME_LO = 0x9A, BACKUP_NAME_HI = 0x6C,
    BACKUP_BLANK_LO = 0x8F, BACKUP_BLANK_HI = 0x6C,
    BACKUP_BUFFER_HI = 0x7D, BACKUP_SIZE = 0x90, BACKUP_FREE_NEEDED = 0x92,
    OP_BANKS = 0, OP_GOTO = 0xC0, OP_CALL_BY_SCENE = 0x82, OP_COPY_TILES = 0x81,
    OP_CALL_TWO = 0x80, OP_CALL_ONE = 0x83
};

/*
 * @routine 0048:424 BackupRead
 * @run 6A:6C24
 * WHAT IT DOES: Reads the game's backup file to RAM at $7D00.
 * HOW: The card's bm_read, with the file's name at $6C9A and $90 bytes.
 * WAITS: no. The card's bm_read is a tail jump.
 * Entry: nothing. Exit: as bm_read, entered with A 0 and Z set.
 */
/* The BIOS's arguments for the game's backup file: its name, $7D00, $90 bytes, from the start. */
static void backup_file_arguments(void)
{
    wr8(zBiosAl, BACKUP_NAME_LO);
    wr8(zBiosAh, BACKUP_NAME_HI);
    wr8(zBiosBl, 0);
    wr8(zBiosBh, BACKUP_BUFFER_HI);
    wr8(zBiosCl, BACKUP_SIZE);
    wr8(zBiosCh, 0);
    wr8(zBiosDl, 0);
    wr8(zBiosDh, 0);
}

BackupAccess BackupRead(void)
{
    backup_file_arguments();
    return bm_read(0);
}

void backup_read_exit_shim(BackupAccess read)
{
    cpu.a = 0;
    abi_nz(0);
    bm_read_exit_shim(read);
}

void abi_BackupRead(void)
{
    backup_read_exit_shim(BackupRead());
}

/*
 * @routine 0048:443 BackupWrite
 * @run 6A:6C43
 * WHAT IT DOES: Writes the game's backup file from $7D00, if there is room once the old
 * one is gone.
 * HOW: The card's bm_delete, its bm_free count compared with $0092, its bm_write.
 * WAITS: no.
 * Entry: nothing. Exit, no room: A 1, N and Z from it, C and V from the compare's subtract, X
 * and Y as bm_free left them. Exit: as bm_write, entered with A 0 and Z set.
 */
BackupWriteRun BackupWrite(void)
{
    BackupWriteRun run;

    uint8_t low, high;

    wr8(zBiosAl, BACKUP_NAME_LO);
    wr8(zBiosAh, BACKUP_NAME_HI);
    jsr_push(0x6C4E);
    run.removal = bm_delete(BACKUP_NAME_HI);
    jsr_pop();
    jsr_push(0x6C51);
    run.free_space = bm_free(bm_delete_result(run.removal));
    jsr_pop();
    low = rd8(zBiosCl);
    high = rd8(zBiosCh);
    run.room = sub_bytes(high, 0, low >= BACKUP_FREE_NEEDED);
    run.fits = !(run.room.value & 0x80);
    if (!run.fits)
        return run;
    backup_file_arguments();
    run.write = bm_write(0);
    return run;
}

void backup_write_exit_shim(const BackupWriteRun *run)
{
    cpu.a = BACKUP_NAME_HI;
    abi_nz(BACKUP_NAME_HI);
    bm_delete_exit_shim(run->removal);
    bm_free_exit_shim(run->free_space);
    abi_flag(P_C, run->room.carry);
    abi_flag(P_V, run->room.overflow);
    if (!run->fits) {
        cpu.a = 1;
        abi_nz(1);
        return;
    }
    cpu.a = 0;
    abi_nz(0);
    bm_write_exit_shim(run->write);
}

void abi_BackupWrite(void)
{
    BackupWriteRun run = BackupWrite();

    backup_write_exit_shim(&run);
}

/*
 * @routine 0048:47A BackupFormat
 * @run 6A:6C7A
 * WHAT IT DOES: Formats the backup memory; on failure the game stops at BackupFailed.
 * HOW: The card's bm_format with the blank name at $6C8F; its answer goes to X.
 * WAITS: yes, only on failure: it then spins in BackupFailed with interrupts on; the stack there is the caller's return and this call's two bytes.
 * Entry: nothing. Exit: as bm_format leaves them, with X its answer, N and Z from it.
 */
BackupFormat BackupFormatMemory(void)
{
    BackupFormat format;

    wr8(zBiosAl, BACKUP_BLANK_LO);
    wr8(zBiosAh, BACKUP_BLANK_HI);
    jsr_push(0x6C85);
    format = bm_format(BACKUP_BLANK_HI);
    jsr_pop();
    if (bm_format_result(format) != 0)
        for (;;)
            wait_interrupt();
    return format;
}

void abi_BackupFormat(void)
{
    cpu.a = BACKUP_BLANK_HI;
    abi_nz(BACKUP_BLANK_HI);
    bm_format_exit_shim(BackupFormatMemory());
    cpu.x = cpu.a;
    abi_nz(cpu.a);
}

/*
 * @routine 0048:48C BackupFailed
 * @run 6A:6C8C
 * WHAT IT DOES: Where the game stops when the backup memory cannot be formatted.
 * HOW: Jumps to itself; interrupts go on.
 * WAITS: yes, for ever: a jump to itself with interrupts on; the stack is the caller's return and its call frame.
 * Entry: anything. Exit: none.
 */
void abi_BackupFailed(void)
{
    for (;;)
        wait_interrupt();
}

/* A call made, kept in the log; past DISC_CALLS_KEPT the oldest go. */
static void log_call(DiscLog *log, DiscCall call)
{
    log->calls[log->count % DISC_CALLS_KEPT] = call;
    log->count++;
}

static DiscCall disc_call(DiscCallKind kind)
{
    DiscCall call = { 0 };

    call.kind = kind;
    return call;
}

/* X as the logged calls leave it, from the X it had before the first: what the next call pushes. */
static uint8_t log_x(const DiscLog *log, uint8_t entry_x)
{
    Cpu saved = cpu;
    uint8_t x;

    cpu.x = entry_x;
    disc_log_exit_shim(log);
    x = cpu.x;
    cpu = saved;
    return x;
}

/* An address's last add, which leaves its byte in A and its carry and overflow. */
static void log_sum(DiscLog *log, Sum sum)
{
    DiscCall call = disc_call(DISC_SUM);

    call.sum = sum;
    log_call(log, call);
}

/* Waits for the drive: asks whether it is busy, and pauses it until it says it has stopped. */
static void wait_drive_idle(DiscLog *log, uint8_t entry_x, uint16_t ret_stat, uint16_t ret_pause)
{
    for (;;) {
        DiscCall stat = disc_call(DISC_CD_STAT);

        jsr_push(ret_stat);
        stat.as.cd_stat = cd_stat(0, log_x(log, entry_x));
        jsr_pop();
        log_call(log, stat);
        if (cd_stat_result(stat.as.cd_stat) == 0)
            return;
        for (;;) {
            DiscCall pause = disc_call(DISC_CD_PAUSE);

            jsr_push(ret_pause);
            pause.as.cd_pause = cd_pause(log_x(log, entry_x));
            jsr_pop();
            log_call(log, pause);
            if (cd_pause_result(pause.as.cd_pause) == 0)
                break;
        }
    }
}

/* A record's three address bytes, from the pointer in zTemp00-01, less the track's start. */
static Sum record_disc_address(void)
{
    Sum low = sub_bytes(rd8_zp_y(zTemp00, 0), (uint8_t)TRACK_START, true);
    Sum middle;
    Sum high;

    wr8(zBiosDl, low.value);
    middle = sub_bytes(rd8_zp_y(zTemp00, 1), (uint8_t)(TRACK_START >> 8), low.carry);
    wr8(zBiosCh, middle.value);
    high = sub_bytes(rd8_zp_y(zTemp00, 2), (uint8_t)(TRACK_START >> 16), middle.carry);
    wr8(zBiosCl, high.value);
    return high;
}

/*
 * The flags the game holds at its BIOS multiply: decimal and interrupt clear. The multiply keeps V only when
 * its second byte is zero, and the game's sizes are not, so no other bit of them shows in what it leaves.
 */
enum { MUL_ENTRY_FLAGS = 0 };

/* A record's address in zTemp00-01: the BIOS's multiply of index by size, plus the table. */
static void record_at(DiscLog *log, uint16_t table, uint8_t index, uint8_t size, uint16_t ret)
{
    DiscCall mul = disc_call(DISC_MUL);
    Sum low;
    Sum high;

    wr8(zBiosAl, index);
    wr8(zBiosBl, size);
    jsr_push(ret);
    mul.as.mul = ma_mul8u(size, MUL_ENTRY_FLAGS);
    jsr_pop();
    log_call(log, mul);
    low = add_bytes((uint8_t)table, rd8(zBiosCl), false);
    wr8(zTemp00, low.value);
    high = add_bytes((uint8_t)(table >> 8), rd8(zBiosCh), low.carry);
    wr8(zTemp01, high.value);
    log_sum(log, high);
}

/* The BIOS's ADPCM transfer of _AL sectors from the address in _CL, _CH, _DL to ADPCM 0. */
static AdTrans adpcm_transfer(DiscLog *log, uint8_t entry_x, uint8_t sectors, Sum last, uint16_t ret)
{
    DiscCall trans = disc_call(DISC_AD_TRANS);

    wr8(zBiosAl, sectors);
    wr8(zBiosDh, 0);
    wr8(zBiosBh, 0);
    wr8(zBiosBl, 0);
    trans.given = sectors;
    trans.summed = true;
    trans.sum = last;
    jsr_push(ret);
    trans.as.ad_trans = ad_trans(sectors, log_x(log, entry_x));
    jsr_pop();
    log_call(log, trans);
    return trans.as.ad_trans;
}

/*
 * @routine 0048:643 AdpcmLoad
 * @run 6A:6E43
 * WHAT IT DOES: Loads a sound into ADPCM memory from the disc, unless it is already there.
 * HOW: Stops the ADPCM and waits on its status, waits for the drive, then the BIOS's ADPCM
 * transfer from the sound's record in AdpcmRecords, until it answers zero.
 * WAITS: yes, on the drive: the ADPCM status and the card's drive and transfer calls are asked until they answer; the stack there is the caller's return, this call's two bytes and the pushed sound (1).
 * Entry: A sound. Exit, already there: A kept, N 0, Z 1, C 1 from the compare. Exit: A zero,
 * N 0, Z 1, C 1; X, Y and V as the BIOS's last transfer leaves them. The C replays the last 32
 * of the BIOS's calls.
 */
AdpcmLoadRun AdpcmLoad(uint8_t sound)
{
    AdpcmLoadRun run;
    DiscCall stop = disc_call(DISC_AD_STOP);
    DiscCall stat;
    Sum low;

    run.sound = sound;
    run.log.count = 0;
    run.already = sound == rd8(zMusicWanted);
    if (run.already)
        return run;
    wr8(zMusicWanted, sound);
    push8(sound);
    jsr_push(0x6E4D);
    stop.as.control = ad_stop();
    jsr_pop();
    log_call(&run.log, stop);
    do {
        stat = disc_call(DISC_AD_STAT);
        jsr_push(0x6E50);
        stat.as.ad_stat = ad_stat();
        jsr_pop();
        log_call(&run.log, stat);
    } while (stat.as.ad_stat.playing != 0);
    sound = pull8();
    low = add_bytes((uint8_t)(sound << 2), (uint8_t)AdpcmRecords, false);
    wr8(zTemp00, low.value);
    low = add_bytes(0, (uint8_t)(AdpcmRecords >> 8), low.carry);
    wr8(zTemp01, low.value);
    log_sum(&run.log, low);
    for (;;) {
        Sum last;

        wait_drive_idle(&run.log, 0, 0x6E64, 0x6E6B);
        last = record_disc_address();
        if (ad_trans_result(adpcm_transfer(&run.log, 0, rd8_zp_y(zTemp00, 3), last, 0x6E95)) == 0)
            break;
    }
    wr8(BiosAdpcmBusy, 0);
    return run;
}

void adpcm_load_exit_shim(const AdpcmLoadRun *run)
{
    if (run->already) {
        cpu.a = run->sound;
        abi_nz(0);
        abi_flag(P_C, true);
        return;
    }
    disc_log_exit_shim(&run->log);
}

void abi_AdpcmLoad(void)
{
    AdpcmLoadRun run = AdpcmLoad(cpu.a);

    adpcm_load_exit_shim(&run);
}

/*
 * @routine 0048:69D AdpcmLoadFixed
 * @run 6A:6E9D
 * WHAT IT DOES: Loads $20 sectors of ADPCM from $20 sectors past disc address $0010AB, with the
 * track's start taken off as a record's is.
 * HOW: Waits for the drive; the address worked out a byte at a time from its constants; the
 * BIOS's ADPCM transfer, until it answers zero.
 * WAITS: yes, on the drive: the card's drive and transfer calls are asked until they answer; the stack there is the caller's return and this call's two bytes.
 * Entry: nothing. Exit: A zero, N 0, Z 1, C 1; X, Y and V as the BIOS's last transfer leaves
 * them.
 */
DiscLog AdpcmLoadFixed(uint8_t entry_x)
{
    DiscLog log;

    log.count = 0;
    for (;;) {
        Sum low;
        Sum middle;
        Sum high;

        wait_drive_idle(&log, entry_x, 0x6EA1, 0x6EA8);
        low = sub_bytes((uint8_t)FIXED_ADPCM_ADDRESS, (uint8_t)TRACK_START, true);
        wr8(zBiosDl, low.value);
        middle = sub_bytes((uint8_t)(FIXED_ADPCM_ADDRESS >> 8), (uint8_t)(TRACK_START >> 8),
                           low.carry);
        wr8(zBiosCh, middle.value);
        high = sub_bytes((uint8_t)(FIXED_ADPCM_ADDRESS >> 16), (uint8_t)(TRACK_START >> 16),
                         middle.carry);
        wr8(zBiosCl, high.value);
        low = add_bytes(rd8(zBiosDl), FIXED_ADPCM_SKIP, false);
        wr8(zBiosDl, low.value);
        middle = add_bytes(0, rd8(zBiosCh), low.carry);
        wr8(zBiosCh, middle.value);
        high = add_bytes(0, rd8(zBiosCl), middle.carry);
        wr8(zBiosCl, high.value);
        if (ad_trans_result(adpcm_transfer(&log, entry_x, FIXED_ADPCM_SECTORS, high, 0x6EE3)) == 0)
            break;
    }
    wr8(BiosAdpcmBusy, 0);
    return log;
}

void abi_AdpcmLoadFixed(void)
{
    DiscLog log = AdpcmLoadFixed(cpu.x);

    disc_log_exit_shim(&log);
}

/*
 * @routine 0048:6EB AdpcmPlayFromDisc
 * @run 6A:6EEB
 * WHAT IT DOES: Plays a sound straight from the disc as ADPCM.
 * HOW: The sound's record, five bytes each in AdpcmStreamRecords; waits for the drive; the
 * BIOS's continuous ADPCM play in mode $0E, until it answers zero.
 * WAITS: yes, on the drive: the card's drive and ADPCM play calls are asked until they answer; the stack there is the caller's return and this call's two bytes.
 * Entry: A sound. Exit: A zero, N 0, Z 1, C 1; Y 4 unless the BIOS changed it; X and V as the
 * BIOS leaves them.
 */
DiscLog AdpcmPlayFromDisc(uint8_t sound)
{
    DiscLog log;

    log.count = 0;
    record_at(&log, AdpcmStreamRecords, sound, STREAM_RECORD_BYTES, 0x6EF4);
    for (;;) {
        DiscCall play = disc_call(DISC_AD_CPLAY);

        wait_drive_idle(&log, 0, 0x6F05, 0x6F0C);
        play.summed = true;
        play.sum = record_disc_address();
        wr8(zBiosAl, rd8_zp_y(zTemp00, 3));
        wr8(zBiosAh, rd8_zp_y(zTemp00, 4));
        wr8(zBiosBl, 0);
        wr8(zBiosDh, CPLAY_MODE);
        play.given = CPLAY_MODE;
        jsr_push(0x6F3B);
        play.as.ad_cplay = ad_cplay(CPLAY_MODE, log_x(&log, 0));
        jsr_pop();
        log_call(&log, play);
        if (ad_cplay_result(play.as.ad_cplay) == 0)
            break;
    }
    wr8(BiosAdpcmBusy, 0);
    return log;
}

void abi_AdpcmPlayFromDisc(void)
{
    DiscLog log = AdpcmPlayFromDisc(cpu.a);

    disc_log_exit_shim(&log);
}

/*
 * The registers and flags each call leaves, with what the game loads before it and the compare
 * or transfer it makes of the answer: the order the original runs them in.
 */
static void disc_call_exit_shim(const DiscCall *call)
{
    if (call->kind == DISC_AD_TRANS || call->kind == DISC_AD_CPLAY || call->kind == DISC_CD_SEARCH ||
        call->kind == DISC_CD_READ) {
        cpu.y = call->kind == DISC_AD_TRANS ? 3 : call->kind == DISC_AD_CPLAY ? 4 : 2;
        cpu.a = call->given;
        abi_nz(call->given);
    }
    if (call->summed) {
        abi_flag(P_C, call->sum.carry);
        abi_flag(P_V, call->sum.overflow);
    }
    switch (call->kind) {
    case DISC_AD_STOP:
        ad_stop_exit_shim(call->as.control);
        return;
    case DISC_AD_STAT:
        ad_stat_exit_shim(call->as.ad_stat);
        cpu.y = cpu.a;
        abi_nz(cpu.a);
        return;
    case DISC_SUM:
        cpu.a = call->sum.value;
        abi_nz(call->sum.value);
        abi_flag(P_C, call->sum.carry);
        abi_flag(P_V, call->sum.overflow);
        return;
    case DISC_MUL:
        ma_mul8u_exit_shim(call->as.mul);
        return;
    case DISC_CD_STAT:
        cpu.a = 0;
        cd_stat_exit_shim(call->as.cd_stat);
        break;
    case DISC_CD_PAUSE:
        cd_pause_exit_shim(call->as.cd_pause);
        break;
    case DISC_AD_TRANS:
        ad_trans_exit_shim(call->as.ad_trans);
        break;
    case DISC_AD_CPLAY:
        ad_cplay_exit_shim(call->as.ad_cplay);
        break;
    case DISC_CD_SEARCH:
        cd_search_exit_shim(call->as.cd_search);
        cpu.x = cpu.a;
        abi_nz(cpu.a);
        return;
    case DISC_CD_READ:
        cd_read_exit_shim(call->as.cd_read);
        cpu.x = cpu.a;
        abi_nz(cpu.a);
        return;
    case DISC_OVERLAY:
        cpu.a = call->as.after.a;
        cpu.x = call->as.after.x;
        cpu.y = call->as.after.y;
        cpu.p = call->as.after.p;
        return;
    case DISC_SCENE_BANKS:
        cpu.y = 3;
        return;
    case DISC_SCENE_GOTO:
        cpu.x = call->index;
        cpu.y = 2;
        cpu.a = call->given;
        abi_nz(call->given);
        return;
    case DISC_SCENE_END:
        cpu.a = call->given;
        abi_nz((uint8_t)(call->given - 0x80));
        abi_flag(P_C, true);
        return;
    }
    abi_nz(cpu.a);
    abi_flag(P_C, true);
}

/* A loader's exit: its kept calls replayed, oldest first. */
void disc_log_exit_shim(const DiscLog *log)
{
    unsigned first = log->count > DISC_CALLS_KEPT ? log->count - DISC_CALLS_KEPT : 0;

    for (unsigned i = first; i < log->count; i++)
        disc_call_exit_shim(&log->calls[i % DISC_CALLS_KEPT]);
}

/* The track's entry in CdTrackTable, through zTemp00 and 01, from the BIOS's multiply. */
static TrackEntry track_entry(uint8_t track, uint16_t ret)
{
    TrackEntry entry;
    Sum low;

    wr8(zBiosAl, track);
    wr8(zBiosBl, CD_TRACK_ENTRY);
    jsr_push(ret);
    entry.mul = ma_mul8u(CD_TRACK_ENTRY, MUL_ENTRY_FLAGS);
    jsr_pop();
    low = add_bytes((uint8_t)CdTrackTable, rd8(zBiosCl), false);
    wr8(zTemp00, low.value);
    entry.high = add_bytes((uint8_t)(CdTrackTable >> 8), rd8(zBiosCh), low.carry);
    wr8(zTemp01, entry.high.value);
    return entry;
}

/* A track played in the mode given until the BIOS's play succeeds, then the CD's fade off. */
static TrackPlay play_track(uint8_t track, uint8_t mode, uint8_t kept_x, uint8_t kept_y)
{
    TrackPlay play;
    uint8_t result;

    push8(kept_x);
    push8(kept_y);
    wr8(zCdPlayMode, mode);
    wr8(zCdTrack, track);
    play.track = track;
    play.mode = mode;
    play.entry = track_entry(track, 0x6F5E);
    play.tries = 0;
    do {
        CdPlay tried;

        wr8(zBiosBh, CD_PLAY_FROM_MSF);
        wr8(zBiosAl, rd8_zp(zTemp00));
        wr8(zBiosAh, rd8_zp_y(zTemp00, 1));
        wr8(zBiosBl, rd8_zp_y(zTemp00, 2));
        wr8(zBiosDh, rd8(zCdPlayMode));
        wr8(zBiosCl, rd8_zp_y(zTemp00, 3));
        wr8(zBiosCh, rd8_zp_y(zTemp00, 4));
        play.dl = rd8_zp_y(zTemp00, 5);
        wr8(zBiosDl, play.dl);
        jsr_push(0x6F94);
        tried = cd_play(play.dl, play.tries == 0 ? (uint8_t)(play.entry.mul.product >> 8) : play.results[(play.tries - 1) % CD_PLAY_KEPT]);
        jsr_pop();
        result = cd_play_result(tried);
        play.results[play.tries % CD_PLAY_KEPT] = result;
        play.attempts[play.tries % CD_PLAY_KEPT] = tried;
        play.tries++;
    } while (result != 0);
    jsr_push(0x6F9B);
    cd_fade(0);
    jsr_pop();
    play.y = pull8();
    play.x = pull8();
    return play;
}

/* As the BIOS's calls leave the registers, then X and Y back as they came, N and Z from X. */
static void play_track_exit_shim(TrackPlay play)
{
    uint8_t x = play.x;
    uint8_t y = play.y;
    unsigned first = play.tries > CD_PLAY_KEPT ? play.tries - CD_PLAY_KEPT : 0;

    ma_mul8u_exit_shim(play.entry.mul);
    cpu.a = play.entry.high.value;
    abi_nz(play.entry.high.value);
    abi_flag(P_C, play.entry.high.carry);
    abi_flag(P_V, play.entry.high.overflow);
    for (unsigned i = first; i < play.tries; i++) {
        cpu.x = i == 0 ? play.mode : play.results[(i - 1) % CD_PLAY_KEPT];
        cpu.a = play.dl;
        cpu.y = 5;
        abi_nz(play.dl);
        cd_play_exit_shim(play.attempts[i % CD_PLAY_KEPT]);
    }
    cpu.x = 0;
    cd_fade_exit_shim(0);
    cpu.x = x;
    cpu.y = y;
    abi_nz(x);
}

/*
 * @routine 0048:743 CdPlayOnce
 * @run 6A:6F43
 * WHAT IT DOES: Plays a CD track once.
 * HOW: Mode $42 for the BIOS's play.
 * WAITS: yes, on the drive: the card's play is asked until it answers zero; the stack there is the caller's return, this call's two bytes and X and Y pushed (2).
 * Entry: A track. Exit: A 0 from the BIOS's fade, C and V as its play left them, X and Y kept, N
 * and Z from X. The BIOS's play is tried until it succeeds; the C replays the last 32 tries' exits.
 */
TrackPlay CdPlayOnce(uint8_t track, uint8_t kept_x, uint8_t kept_y)
{
    return play_track(track, CD_PLAY_ONCE, kept_x, kept_y);
}

void abi_CdPlayOnce(void)
{
    play_track_exit_shim(CdPlayOnce(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 0048:74B CdPlayAgain
 * @run 6A:6F4B
 * WHAT IT DOES: Plays the last track again, repeating.
 * HOW: zCdTrack into CdPlayRepeat.
 * WAITS: yes, as CdPlayOnce.
 * Entry: nothing. Exit: as CdPlayRepeat.
 */
TrackPlay CdPlayAgain(uint8_t kept_x, uint8_t kept_y)
{
    uint8_t track = rd8(zCdTrack);

    return play_track(track, CD_PLAY_REPEAT, kept_x, kept_y);
}

void abi_CdPlayAgain(void)
{
    play_track_exit_shim(CdPlayAgain(cpu.x, cpu.y));
}

/*
 * @routine 0048:74D CdPlayRepeat
 * @run 6A:6F4D
 * WHAT IT DOES: Plays a CD track, repeating.
 * HOW: Mode $41 for the BIOS's play.
 * WAITS: yes, as CdPlayOnce.
 * Entry: A track. Exit: as CdPlayOnce.
 */
TrackPlay CdPlayRepeat(uint8_t track, uint8_t kept_x, uint8_t kept_y)
{
    return play_track(track, CD_PLAY_REPEAT, kept_x, kept_y);
}

void abi_CdPlayRepeat(void)
{
    play_track_exit_shim(CdPlayRepeat(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 0048:79E CdSearchTrack
 * @run 6A:6F9E
 * WHAT IT DOES: Seeks to a CD track, so that its play will start at once. $FF does
 * nothing.
 * HOW: The track's entry in CdTrackTable; the BIOS's search from its first three bytes, until it
 * answers zero.
 * WAITS: yes, on the drive: the card's search is asked until it answers zero; the stack there is the caller's return, this call's two bytes and X and Y pushed (2).
 * Entry: A track. Exit, $FF: A kept, N 0, Z 1, C 1. Exit: A zero, X and Y kept, N and Z
 * from X; C and V as the BIOS leaves them.
 */
TrackSearch CdSearchTrack(uint8_t track, uint8_t kept_x, uint8_t kept_y)
{
    TrackSearch search;

    search.log.count = 0;
    search.skipped = track == NO_TRACK;
    if (search.skipped)
        return search;
    push8(kept_x);
    push8(kept_y);
    wr8(zCdTrack, track);
    record_at(&search.log, CdTrackTable, track, CD_TRACK_ENTRY, 0x6FAF);
    for (;;) {
        DiscCall seek = disc_call(DISC_CD_SEARCH);

        wr8(zBiosBh, CD_PLAY_FROM_MSF);
        wr8(zBiosAl, rd8_zp(zTemp00));
        wr8(zBiosAh, rd8_zp_y(zTemp00, 1));
        seek.given = rd8_zp_y(zTemp00, 2);
        wr8(zBiosBl, seek.given);
        jsr_push(0x6FD2);
        seek.as.cd_search = cd_search(seek.given, log_x(&search.log, 0));
        jsr_pop();
        log_call(&search.log, seek);
        if (cd_search_result(seek.as.cd_search) == 0)
            break;
    }
    search.y = pull8();
    search.x = pull8();
    return search;
}

/* CdSearchTrack's exit, from the registers it was entered with. */
void track_search_exit_shim(const TrackSearch *search)
{
    uint8_t x = search->x;
    uint8_t y = search->y;

    if (search->skipped) {
        abi_nz(0);
        abi_flag(P_C, true);
        return;
    }
    disc_log_exit_shim(&search->log);
    cpu.x = x;
    cpu.y = y;
    abi_nz(x);
}

void abi_CdSearchTrack(void)
{
    TrackSearch search = CdSearchTrack(cpu.a, cpu.x, cpu.y);

    track_search_exit_shim(&search);
}

/* The scene overlay's entry at $8006, with the registers SceneCallOverlay gives it. */
static Cpu scene_overlay_shim(uint8_t offset, uint8_t pointer_high, Sum bank)
{
    cpu.a = pointer_high;
    cpu.x = offset;
    cpu.y = 0;
    abi_nz(pointer_high);
    abi_flag(P_C, bank.carry);
    abi_flag(P_V, bank.overflow);
    call_original(FrameOverlay8006);
    return cpu;
}

/*
 * @routine 0049:100 SceneCallOverlay
 * @run 6A:7100
 * WHAT IT DOES: Maps the scene overlay and runs its entry at $8006 with a pointer from
 * SceneCallTable.
 * HOW: The pointer goes to zProduct1-2, where that entry takes it; Y 0.
 * WAITS: depends on the overlay it runs (an original-code tail: a jump to $8006).
 * Entry: X the table's offset. Exit: as the overlay leaves them.
 */
SceneCall SceneCallOverlay(uint8_t offset)
{
    SceneCall call;

    call.bank = add_bytes(SCENE_OVERLAY_BANK, rd8(zRamBaseBank), false);
    tam(MPR4, call.bank.value);
    wr8(zProduct1, rd8((uint16_t)(SceneCallTable + offset)));
    call.pointer_high = rd8((uint16_t)(SceneCallTable + 1 + offset));
    wr8(zProduct2, call.pointer_high);
    call.after = scene_overlay_shim(offset, call.pointer_high, call.bank);
    return call;
}

void abi_SceneCallOverlay(void)
{
    cpu = SceneCallOverlay(cpu.x).after;
}

static void log_overlay(DiscLog *log, uint8_t offset, uint16_t ret)
{
    DiscCall call = disc_call(DISC_OVERLAY);

    jsr_push(ret);
    call.as.after = SceneCallOverlay(offset).after;
    jsr_pop();
    log_call(log, call);
}

/* TII: Y, A and X pushed, the bytes moved a pair of reads and writes at a time, then pulled. */
static void block_move_tii(uint16_t src, uint16_t dst, uint16_t len, uint8_t a, uint8_t x, uint8_t y)
{
    push8(y);
    push8(a);
    push8(x);
    for (uint16_t i = 0; i < len; i++) {
        uint8_t byte = rd8((uint16_t)(src + i));

        wr8((uint16_t)(dst + i), byte);
    }
    (void)pull8();
    (void)pull8();
    (void)pull8();
}

/* The list moves on by the op's length; the add's high byte stays in A. */
static void scene_next(DiscLog *log, uint8_t length)
{
    Sum low = add_bytes(length, rd8(zTemp08), false);
    Sum high;

    wr8(zTemp08, low.value);
    high = add_bytes(0, rd8(zTemp09), low.carry);
    wr8(zTemp09, high.value);
    log_sum(log, high);
}

/* Which overlay pointer an op $82 takes, by the scene. */
static uint8_t scene_call_offset(uint8_t scene)
{
    static const struct { uint8_t scene; uint8_t offset; } cases[] = {
        { 0x02, 0x0C }, { 0x58, 0x0C }, { 0x5A, 0x0C }, { 0x5C, 0x0C },
        { 0x0E, 0x06 }, { 0x0C, 0x0A }, { 0x0A, 0x08 }, { 0x08, 0x0E }
    };

    for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; i++)
        if (scene == cases[i].scene)
            return cases[i].offset;
    return SCENE_CALL_OTHER;
}

/*
 * An op from 1 to $7F: the BIOS's read of _AL sectors to the op's destination. A failed read is
 * asked again with the destination the BIOS's answer indexes, as the original's A holds it.
 */
static void scene_read(DiscLog *log, uint8_t op)
{
    uint8_t index = op;

    for (;;) {
        DiscCall read = disc_call(DISC_CD_READ);
        Sum low;
        Sum middle;
        uint8_t entry = (uint8_t)(index << 1);

        uint8_t first, other;

        wr8(zBiosBl, rd8((uint16_t)(SceneReadDestinations + entry)));
        wr8(zBiosBh, rd8((uint16_t)(SceneReadDestinations + 1 + entry)));
        record_disc_address();
        wr8(zBiosDh, 1);
        first = rd8_zp_y(zTemp08, 1);
        other = rd8(zBiosDl);
        low = add_bytes(first, other, false);
        wr8(zBiosDl, low.value);
        middle = add_bytes(0, rd8(zBiosCh), low.carry);
        wr8(zBiosCh, middle.value);
        read.sum = add_bytes(0, rd8(zBiosCl), middle.carry);
        wr8(zBiosCl, read.sum.value);
        read.summed = true;
        read.given = rd8_zp_y(zTemp08, 2);
        wr8(zBiosAl, read.given);
        jsr_push(0x70F2);
        read.as.cd_read = cd_read(read.given, log_x(log, 0));
        jsr_pop();
        log_call(log, read);
        index = cd_read_result(read.as.cd_read);
        if (index == 0)
            break;
    }
    scene_next(log, SCENE_READ_BYTES);
}

/* The scene's ops, one at a time from the list at zTemp08-09, until one it does not know. */
static void scene_ops(DiscLog *log)
{
    static const uint8_t mprs[] = { MPR4, MPR5, MPR6 };

    for (;;) {
        uint8_t op = rd8_zp(zTemp08);
        DiscCall step;

        if (op == OP_BANKS) {
            for (uint8_t i = 0; i < 3; i++) {
                uint8_t bank = rd8_zp_y(zTemp08, (uint8_t)(i + 1));

                tam(mprs[i], (uint8_t)(bank + rd8(zRamBaseBank)));
            }
            log_call(log, disc_call(DISC_SCENE_BANKS));
            scene_next(log, SCENE_BANKS_BYTES);
        } else if (!(op & 0x80)) {
            scene_read(log, op);
        } else if (op == OP_GOTO) {
            step = disc_call(DISC_SCENE_GOTO);
            step.index = rd8_zp_y(zTemp08, 1);
            step.given = rd8_zp_y(zTemp08, 2);
            wr8(zTemp08, step.index);
            wr8(zTemp09, step.given);
            log_call(log, step);
        } else if (op == OP_CALL_ONE) {
            log_overlay(log, SCENE_CALL_ONE, 0x70AC);
            scene_next(log, 1);
        } else if (op == OP_CALL_BY_SCENE) {
            log_overlay(log, scene_call_offset(rd8(zTemp02)), 0x70AC);
            scene_next(log, 1);
        } else if (op == OP_COPY_TILES) {
            uint8_t bank = (uint8_t)(SCENE_OVERLAY_BANK + rd8(zRamBaseBank));

            tam(MPR5, bank);
            disc_log_exit_shim(log);    /* X and Y as the original has them: the TII pushes them */
            block_move_tii(SCENE_TILES_FROM, SCENE_TILES_TO, SCENE_TILE_BYTES, bank, cpu.x, cpu.y);
            scene_next(log, 1);
        } else if (op == OP_CALL_TWO) {
            log_overlay(log, SCENE_CALL_FIRST, 0x7064);
            log_overlay(log, SCENE_CALL_SECOND, 0x7069);
            scene_next(log, 1);
        } else {
            step = disc_call(DISC_SCENE_END);
            step.given = op;
            log_call(log, step);
            return;
        }
    }
}

/*
 * @routine 0049:0B1 SceneReadSectors
 * @run 6A:70B1
 * WHAT IT DOES: A scene op from 1 to $7F: reads sectors from the disc to the place in RAM
 * the op gives, at an offset from the scene's own disc address; then the ops go on.
 * HOW: The op indexes SceneReadDestinations; the op's second byte is the sector offset and its
 * third the count; the BIOS's read, asked again until it answers zero, and each time the table
 * is indexed by what A then holds.
 * WAITS: yes, on the drive: the card's read is asked until it answers zero; the stack there is the caller's return and this call's two bytes.
 * Entry: A the op. Exit: as LoadSceneScript.
 */
DiscLog SceneReadSectors(uint8_t op)
{
    DiscLog log;

    log.count = 0;
    scene_read(&log, op);
    scene_ops(&log);
    return log;
}

void abi_SceneReadSectors(void)
{
    DiscLog log = SceneReadSectors(cpu.a);

    disc_log_exit_shim(&log);
}

/*
 * @routine 0048:7D8 LoadSceneScript
 * @run 6A:6FD8
 * WHAT IT DOES: Loads a scene: runs its list of ops, which map banks, read sectors from
 * the disc, and call the scene overlay, until an op it does not know ends it.
 * HOW: The scene's record, five bytes each in SceneRecords, holds the disc address and at 3
 * and 4 the list.
 * WAITS: yes, on the drive: through SceneReadSectors, and on the overlay calls; the stack there is the caller's return and this call's two bytes (and an overlay call's).
 * Entry: A scene. Exit: A the last op, N and Z from comparing it with $80, C 1; X, Y and V as
 * the last op left them. Clobbers zTemp00-02, zTemp08-09.
 */
DiscLog LoadSceneScript(uint8_t scene)
{
    DiscLog log;

    log.count = 0;
    wr8(zTemp02, scene);
    record_at(&log, SceneRecords, scene, SCENE_RECORD_BYTES, 0x6FE3);
    wr8(zTemp08, rd8_zp_y(zTemp00, 3));
    wr8(zTemp09, rd8_zp_y(zTemp00, 4));
    scene_ops(&log);
    return log;
}

void abi_LoadSceneScript(void)
{
    DiscLog log = LoadSceneScript(cpu.a);

    disc_log_exit_shim(&log);
}
