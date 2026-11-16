/*
 * Loading from the disc through the System Card's BIOS, and the backup file. A record
 * holds a disc address as the BIOS wants it, with the disc's offset to the track taken
 * off ($000F32). The BIOS answers in A and is asked again until it answers zero; every
 * call sees A, X, Y and the flags, so they are kept in an Alu and in cpu.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

enum {
    MPR4 = 0x10, MPR5 = 0x20, MPR6 = 0x40, CD_TRACK_ENTRY = 6, CD_PLAY_FROM_MSF = 0x40,
    CD_PLAY_ONCE = 0x42, CD_PLAY_REPEAT = 0x41,
    CD_STATUS_IDLE = 0,
    BACKUP_NAME_LO = 0x9A, BACKUP_NAME_HI = 0x6C,
    BACKUP_BLANK_LO = 0x8F, BACKUP_BLANK_HI = 0x6C,
    BACKUP_BUFFER_HI = 0x7D, BACKUP_SIZE = 0x90, BACKUP_FREE_NEEDED = 0x92,
    OP_BANKS = 0, OP_GOTO = 0xC0, OP_CALL_BY_SCENE = 0x82, OP_COPY_TILES = 0x81,
    OP_CALL_TWO = 0x80, OP_CALL_ONE = 0x83
};

/* The three bytes of a record's disc address, at Y 0 to 2, less the track's start. */
static Alu record_address(Alu r)
{
    uint8_t byte;

    cpu.y = 0;
    byte = rd8_zp_y(zTemp00, cpu.y);
    r = sbc8(byte, 0x32, true);
    wr8(zBiosDl, r.a);
    cpu.y = 1;
    byte = rd8_zp_y(zTemp00, cpu.y);
    r = sbc8(byte, 0x0F, r.c);
    wr8(zBiosCh, r.a);
    cpu.y = 2;
    byte = rd8_zp_y(zTemp00, cpu.y);
    r = sbc8(byte, 0x00, r.c);
    wr8(zBiosCl, r.a);
    return r;
}

/* Waits for the drive: asks its status, and pauses it until it will stop. */
static Alu wait_drive_idle(Alu r)
{
    for (;;) {
        r.a = 0;
        r = call_with(BiosCdStat, r);
        alu_compare(&r, r.a, CD_STATUS_IDLE);
        if (r.a == CD_STATUS_IDLE)
            return r;
        do {
            r = call_with(BiosCdPause, r);
            alu_compare(&r, r.a, CD_STATUS_IDLE);
        } while (r.a != CD_STATUS_IDLE);
    }
}

/* index * size + table, with the BIOS's 8-bit multiply, into zTemp00-01. */
static Alu record_pointer(Alu r, uint8_t index, uint8_t size, uint8_t table_lo, uint8_t table_hi)
{
    uint8_t byte;

    wr8(zBiosAl, index);
    alu_load(&r, size);
    wr8(zBiosBl, size);
    r = call_with(BiosMaMul8u, r);
    byte = rd8(zBiosCl);
    r = adc8(table_lo, byte, false);
    wr8(zTemp00, r.a);
    byte = rd8(zBiosCh);
    r = adc8(table_hi, byte, r.c);
    wr8(zTemp01, r.a);
    return r;
}

/*
 * @routine 0048:424 BackupRead
 * @run 6A:6C24
 * WHAT IT DOES: Reads the game's backup file to RAM at $7D00.
 * HOW: The card's bm_read, with the file's name at $6C9A and $90 bytes.
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
    return bm_read();
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
 * HOW: The BIOS's delete, its free count compared with $0092, its write.
 * Entry: nothing. Exit, no room: A 1, N and Z from it. Exit: as the BIOS leaves them.
 */
Alu BackupWrite(Alu r)
{
    uint8_t byte;

    alu_load(&r, BACKUP_NAME_LO);
    wr8(zBiosAl, BACKUP_NAME_LO);
    alu_load(&r, BACKUP_NAME_HI);
    wr8(zBiosAh, BACKUP_NAME_HI);
    r = call_with(BiosBmDelete, r);
    r = call_with(BiosBmFree, r);
    byte = rd8(zBiosCl);
    alu_compare(&r, byte, BACKUP_FREE_NEEDED);
    byte = rd8(zBiosCh);
    r = sbc8(byte, 0, r.c);
    if (r.a & 0x80) {
        alu_load(&r, 1);
        return r;
    }
    alu_load(&r, BACKUP_NAME_LO);
    wr8(zBiosAl, BACKUP_NAME_LO);
    alu_load(&r, BACKUP_NAME_HI);
    wr8(zBiosAh, BACKUP_NAME_HI);
    alu_load(&r, 0);
    wr8(zBiosBl, 0);
    alu_load(&r, BACKUP_BUFFER_HI);
    wr8(zBiosBh, BACKUP_BUFFER_HI);
    alu_load(&r, BACKUP_SIZE);
    wr8(zBiosCl, BACKUP_SIZE);
    alu_load(&r, 0);
    wr8(zBiosCh, 0);
    wr8(zBiosDl, 0);
    wr8(zBiosDh, 0);
    return call_with(BiosBmWrite, r);
}

void abi_BackupWrite(void)
{
    abi_alu(BackupWrite(alu_from_cpu()));
}

/*
 * @routine 0048:47A BackupFormat
 * @run 6A:6C7A
 * WHAT IT DOES: Formats the backup memory; on failure the game stops at BackupFailed.
 * HOW: The BIOS's format; its answer goes to X.
 * Entry: nothing. Exit: as the BIOS leaves them, with X its answer, N and Z from it.
 */
void abi_BackupFormat(void)
{
    Alu r = alu_from_cpu();

    alu_load(&r, BACKUP_BLANK_LO);
    wr8(zBiosAl, BACKUP_BLANK_LO);
    alu_load(&r, BACKUP_BLANK_HI);
    wr8(zBiosAh, BACKUP_BLANK_HI);
    r = call_with(BiosBmFormat, r);
    cpu.x = r.a;
    alu_nz(&r, r.a);
    abi_alu(r);
    if (r.a != 0)
        abi_BackupFailed();
}

/*
 * @routine 0048:48C BackupFailed
 * @run 6A:6C8C
 * WHAT IT DOES: Where the game stops when the backup memory cannot be formatted.
 * HOW: Jumps to itself; interrupts go on.
 * Entry: anything. Exit: none.
 */
void abi_BackupFailed(void)
{
    for (;;)
        wait_interrupt();
}

/*
 * @routine 0048:643 AdpcmLoad
 * @run 6A:6E43
 * WHAT IT DOES: Loads a sound into ADPCM memory from the disc, unless it is already there.
 * HOW: Stops the ADPCM and waits on its status, waits for the drive, then the BIOS's ADPCM
 * transfer from the record for it, until it answers zero.
 * Entry: A sound. Exit, already there: A kept, N, Z, C from the compare. Exit: A zero,
 * N 0, Z 1, C 1; Y 3; V as the BIOS leaves it.
 */
Alu AdpcmLoad(Alu r)
{
    uint8_t sound = r.a;
    uint8_t byte;

    byte = rd8(zMusicWanted);
    alu_compare(&r, sound, byte);
    if (sound == byte)
        return r;
    wr8(zMusicWanted, sound);
    r = call_with(BiosAdStop, r);
    do {
        r = call_with(BiosAdStat, r);
        cpu.y = r.a;
        alu_nz(&r, r.a);
    } while (r.a != 0);
    alu_load(&r, sound);
    r.a = (uint8_t)(sound << 2);
    r = adc8(r.a, 0x35, false);
    wr8(zTemp00, r.a);
    r = adc8(0, 0x71, r.c);
    wr8(zTemp01, r.a);
    do {
        r = wait_drive_idle(r);
        r = record_address(r);
        cpu.y = 3;
        byte = rd8_zp_y(zTemp00, cpu.y);
        alu_load(&r, byte);
        wr8(zBiosAl, byte);
        wr8(zBiosDh, 0);
        wr8(zBiosBh, 0);
        wr8(zBiosBl, 0);
        r = call_with(BiosAdTrans, r);
        alu_compare(&r, r.a, 0);
    } while (r.a != 0);
    wr8(BiosAdpcmBusy, 0);
    return r;
}

void abi_AdpcmLoad(void)
{
    abi_alu(AdpcmLoad(alu_from_cpu()));
}

/*
 * @routine 0048:69D AdpcmLoadFixed
 * @run 6A:6E9D
 * WHAT IT DOES: Loads $20 sectors of ADPCM from $20 past disc address $0010AB, with the
 * track's start taken off as a record's is.
 * HOW: Waits for the drive; the address worked out from its constants; ADPCM transfer.
 * Entry: nothing. Exit: A zero, N 0, Z 1, C 1; Y 3; V as the BIOS leaves it.
 */
Alu AdpcmLoadFixed(Alu r)
{
    uint8_t byte;

    do {
        r = wait_drive_idle(r);
        cpu.y = 0;
        r = sbc8(0xAB, 0x32, true);
        wr8(zBiosDl, r.a);
        cpu.y = 1;
        r = sbc8(0x10, 0x0F, r.c);
        wr8(zBiosCh, r.a);
        cpu.y = 2;
        r = sbc8(0x00, 0x00, r.c);
        wr8(zBiosCl, r.a);
        byte = rd8(zBiosDl);
        r = adc8(byte, 0x20, false);
        wr8(zBiosDl, r.a);
        byte = rd8(zBiosCh);
        r = adc8(0, byte, r.c);
        wr8(zBiosCh, r.a);
        byte = rd8(zBiosCl);
        r = adc8(0, byte, r.c);
        wr8(zBiosCl, r.a);
        cpu.y = 3;
        alu_load(&r, 0x20);
        wr8(zBiosAl, 0x20);
        wr8(zBiosDh, 0);
        wr8(zBiosBh, 0);
        wr8(zBiosBl, 0);
        r = call_with(BiosAdTrans, r);
        alu_compare(&r, r.a, 0);
    } while (r.a != 0);
    wr8(BiosAdpcmBusy, 0);
    return r;
}

void abi_AdpcmLoadFixed(void)
{
    abi_alu(AdpcmLoadFixed(alu_from_cpu()));
}

/*
 * @routine 0048:6EB AdpcmPlayFromDisc
 * @run 6A:6EEB
 * WHAT IT DOES: Plays a sound straight from the disc as ADPCM.
 * HOW: The record for it, five bytes each from $720D; waits for the drive; the BIOS's
 * ADPCM play with mode $0E, until it answers zero.
 * Entry: A sound. Exit: A zero, N 0, Z 1, C 1; Y 4; V as the BIOS leaves it.
 */
void abi_AdpcmPlayFromDisc(void)
{
    Alu r = record_pointer(alu_from_cpu(), cpu.a, 5, 0x0D, 0x72);
    uint8_t byte;

    do {
        r = wait_drive_idle(r);
        r = record_address(r);
        cpu.y = 3;
        byte = rd8_zp_y(zTemp00, cpu.y);
        alu_load(&r, byte);
        wr8(zBiosAl, byte);
        cpu.y = 4;
        byte = rd8_zp_y(zTemp00, cpu.y);
        wr8(zBiosAh, byte);
        wr8(zBiosBl, 0);
        alu_load(&r, 0x0E);
        wr8(zBiosDh, 0x0E);
        r = call_with(BiosAdCplay, r);
        alu_compare(&r, r.a, 0);
    } while (r.a != 0);
    wr8(BiosAdpcmBusy, 0);
    abi_alu(r);
}

/* The record for a track, six bytes each from $7250, as the BIOS's play and search want. */
static Alu track_record_start(Alu r)
{
    uint8_t byte;

    alu_load(&r, 0x40);
    wr8(zBiosBh, 0x40);
    byte = rd8_zp(zTemp00);
    alu_load(&r, byte);
    wr8(zBiosAl, byte);
    cpu.y = 1;
    byte = rd8_zp_y(zTemp00, cpu.y);
    alu_load(&r, byte);
    wr8(zBiosAh, byte);
    cpu.y = 2;
    byte = rd8_zp_y(zTemp00, cpu.y);
    alu_load(&r, byte);
    wr8(zBiosBl, byte);
    return r;
}

/* The play of a track, whichever way the mode came. A in r is the track. */
/* The track's entry in CdTrackTable, through zTemp00 and 01, from the BIOS's multiply. */
static TrackEntry track_entry(uint8_t track)
{
    TrackEntry entry;
    Sum low;

    wr8(zBiosAl, track);
    wr8(zBiosBl, CD_TRACK_ENTRY);
    entry.product = ma_mul8u(track, CD_TRACK_ENTRY);
    low = add_bytes((uint8_t)CdTrackTable, rd8(zBiosCl), false);
    wr8(zTemp00, low.value);
    entry.high = add_bytes((uint8_t)(CdTrackTable >> 8), rd8(zBiosCh), low.carry);
    wr8(zTemp01, entry.high.value);
    return entry;
}

/* A track played in the mode given until the BIOS's play succeeds, then the CD's fade off. */
static TrackPlay play_track(uint8_t track, uint8_t mode)
{
    TrackPlay play;
    uint8_t result;

    wr8(zCdPlayMode, mode);
    wr8(zCdTrack, track);
    play.track = track;
    play.mode = mode;
    play.entry = track_entry(track);
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
        tried = cd_play();
        result = cd_play_result(tried);
        play.results[play.tries % CD_PLAY_KEPT] = result;
        play.attempts[play.tries % CD_PLAY_KEPT] = tried;
        play.tries++;
    } while (result != 0);
    cd_fade(0);
    return play;
}

/* As the BIOS's calls leave the registers, then X and Y back as they came, N and Z from X. */
static void play_track_exit_shim(TrackPlay play)
{
    uint8_t x = cpu.x;
    uint8_t y = cpu.y;
    unsigned first = play.tries > CD_PLAY_KEPT ? play.tries - CD_PLAY_KEPT : 0;

    ma_mul8u_exit_shim(play.track, CD_TRACK_ENTRY, play.entry.product);
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
 * Entry: A track. Exit: A 0 from the BIOS's fade, C and V as its play left them, X and Y kept, N
 * and Z from X. The BIOS's play is tried until it succeeds; the C replays the last 32 tries' exits.
 */
TrackPlay CdPlayOnce(uint8_t track)
{
    return play_track(track, CD_PLAY_ONCE);
}

void abi_CdPlayOnce(void)
{
    play_track_exit_shim(CdPlayOnce(cpu.a));
}

/*
 * @routine 0048:74B CdPlayAgain
 * @run 6A:6F4B
 * WHAT IT DOES: Plays the last track again, repeating.
 * HOW: zCdTrack into CdPlayRepeat.
 * Entry: nothing. Exit: as CdPlayRepeat.
 */
TrackPlay CdPlayAgain(void)
{
    return play_track(rd8(zCdTrack), CD_PLAY_REPEAT);
}

void abi_CdPlayAgain(void)
{
    play_track_exit_shim(CdPlayAgain());
}

/*
 * @routine 0048:74D CdPlayRepeat
 * @run 6A:6F4D
 * WHAT IT DOES: Plays a CD track, repeating.
 * HOW: Mode $41 for the BIOS's play.
 * Entry: A track. Exit: as CdPlayOnce.
 */
TrackPlay CdPlayRepeat(uint8_t track)
{
    return play_track(track, CD_PLAY_REPEAT);
}

void abi_CdPlayRepeat(void)
{
    play_track_exit_shim(CdPlayRepeat(cpu.a));
}

/*
 * @routine 0048:79E CdSearchTrack
 * @run 6A:6F9E
 * WHAT IT DOES: Seeks to a CD track, so that its play will start at once. $FF does
 * nothing.
 * HOW: The record for it; the BIOS's search, until it answers zero.
 * Entry: A track. Exit, $FF: A kept, N 0, Z 1, C 1. Exit: A zero, X and Y kept, N and Z
 * from X; C and V as the BIOS leaves them.
 */
Alu CdSearchTrack(Alu r)
{
    uint8_t track = r.a;
    uint8_t saved_x = cpu.x;
    uint8_t saved_y = cpu.y;

    alu_compare(&r, track, 0xFF);
    if (track == 0xFF)
        return r;
    wr8(zCdTrack, track);
    r = record_pointer(r, track, 6, 0x50, 0x72);
    do {
        r = track_record_start(r);
        r = call_with(BiosCdSearch, r);
        cpu.x = r.a;
        alu_nz(&r, r.a);
    } while (r.a != 0);
    cpu.y = saved_y;
    cpu.x = saved_x;
    alu_nz(&r, saved_x);
    return r;
}

void abi_CdSearchTrack(void)
{
    abi_alu(CdSearchTrack(alu_from_cpu()));
}

/*
 * @routine 0049:100 SceneCallOverlay
 * @run 6A:7100
 * WHAT IT DOES: Maps the scene overlay and runs its entry at $8006 with a pointer from the
 * table at $72C8.
 * HOW: The pointer goes to zProduct1-2, where that entry takes it.
 * Entry: X the table's offset. Exit: as the overlay leaves them.
 */
Alu SceneCallOverlay(Alu r)
{
    uint8_t byte = rd8(zRamBaseBank);

    r = adc8(0x04, byte, false);
    tam(MPR4, r.a);
    byte = rd8((uint16_t)(SceneCallTable + cpu.x));
    alu_load(&r, byte);
    wr8(zProduct1, byte);
    byte = rd8((uint16_t)(SceneCallTable + 1 + cpu.x));
    alu_load(&r, byte);
    wr8(zProduct2, byte);
    cpu.y = 0;
    return call_with(FrameOverlay8006, r);
}

void abi_SceneCallOverlay(void)
{
    abi_alu(SceneCallOverlay(alu_from_cpu()));
}

/* The scene's pieces, one op at a time from the list at zTemp08-09. */
static Alu scene_ops(Alu r);

/* The list moves on by the op's length. */
static Alu scene_next(Alu r, uint8_t length)
{
    uint8_t byte = rd8(zTemp08);

    r = adc8(length, byte, false);
    wr8(zTemp08, r.a);
    byte = rd8(zTemp09);
    r = adc8(0, byte, r.c);
    wr8(zTemp09, r.a);
    return r;
}

/* Which overlay pointer an op $82 takes, by the scene. */
static Alu scene_call_by_scene(Alu r)
{
    static const struct { uint8_t scene; uint8_t offset; } cases[] = {
        { 0x02, 0x0C }, { 0x58, 0x0C }, { 0x5A, 0x0C }, { 0x5C, 0x0C },
        { 0x0E, 0x06 }, { 0x0C, 0x0A }, { 0x0A, 0x08 }, { 0x08, 0x0E }
    };
    uint8_t scene = rd8(zTemp02);
    uint8_t offset = 0x10;

    for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        alu_compare(&r, scene, cases[i].scene);
        if (scene == cases[i].scene) {
            offset = cases[i].offset;
            break;
        }
    }
    cpu.x = offset;
    alu_nz(&r, offset);
    return SceneCallOverlay(r);
}

/*
 * @routine 0049:0B1 SceneReadSectors
 * @run 6A:70B1
 * WHAT IT DOES: A scene op from 1 to 3: reads sectors from the disc to the place in RAM
 * the op gives, at an offset from the scene's own disc address.
 * HOW: The op indexes the table of destinations at $70FA; the op's second byte is the
 * sector offset and its third the count; the BIOS's read, asked again until it answers
 * zero, and each time the table is indexed by what A then holds.
 * Entry: A the op. Exit: the list moved on, and the ops go on.
 */
Alu SceneReadSectors(Alu r)
{
    uint8_t byte;

    do {
        cpu.y = (uint8_t)(r.a << 1);
        r.c = (r.a & 0x80) != 0;
        r.c_kept = false;
        byte = rd8((uint16_t)(SceneReadDestinations + cpu.y));
        alu_load(&r, byte);
        wr8(zBiosBl, byte);
        byte = rd8((uint16_t)(SceneReadDestinations + 1 + cpu.y));
        wr8(zBiosBh, byte);
        r = record_address(r);
        alu_load(&r, 1);
        wr8(zBiosDh, 1);
        cpu.y = 1;
        byte = rd8_zp_y(zTemp08, cpu.y);
        r = adc8(byte, rd8(zBiosDl), false);
        wr8(zBiosDl, r.a);
        byte = rd8(zBiosCh);
        r = adc8(0, byte, r.c);
        wr8(zBiosCh, r.a);
        byte = rd8(zBiosCl);
        r = adc8(0, byte, r.c);
        wr8(zBiosCl, r.a);
        cpu.y = 2;
        byte = rd8_zp_y(zTemp08, cpu.y);
        alu_load(&r, byte);
        wr8(zBiosAl, byte);
        r = call_with(BiosCdRead, r);
        cpu.x = r.a;
        alu_nz(&r, r.a);
    } while (r.a != 0);
    r = scene_next(r, 3);
    return scene_ops(r);
}

void abi_SceneReadSectors(void)
{
    abi_alu(SceneReadSectors(alu_from_cpu()));
}

static Alu scene_ops(Alu r)
{
    uint8_t op;
    uint8_t byte;
    uint8_t base;

    for (;;) {
        op = rd8_zp(zTemp08);
        alu_load(&r, op);
        if (op == OP_BANKS) {
            static const uint8_t mprs[] = { MPR4, MPR5, MPR6 };

            for (uint8_t i = 0; i < 3; i++) {
                cpu.y = (uint8_t)(i + 1);
                byte = rd8_zp_y(zTemp08, cpu.y);
                base = rd8(zRamBaseBank);
                r = adc8(byte, base, false);
                tam(mprs[i], r.a);
            }
            r = scene_next(r, 4);
            continue;
        }
        if (!(op & 0x80)) {
            r = SceneReadSectors(r);
            return r;
        }
        alu_compare(&r, op, OP_GOTO);
        if (op == OP_GOTO) {
            uint8_t lo;

            cpu.y = 1;
            lo = rd8_zp_y(zTemp08, cpu.y);
            cpu.x = lo;
            cpu.y = 2;
            byte = rd8_zp_y(zTemp08, cpu.y);
            alu_load(&r, byte);
            wr8(zTemp08, lo);
            wr8(zTemp09, byte);
            continue;
        }
        alu_compare(&r, op, OP_CALL_ONE);
        if (op == OP_CALL_ONE) {
            cpu.x = 2;
            alu_nz(&r, 2);
            r = SceneCallOverlay(r);
            r = scene_next(r, 1);
            continue;
        }
        alu_compare(&r, op, OP_CALL_BY_SCENE);
        if (op == OP_CALL_BY_SCENE) {
            r = scene_call_by_scene(r);
            r = scene_next(r, 1);
            continue;
        }
        alu_compare(&r, op, OP_COPY_TILES);
        if (op == OP_COPY_TILES) {
            base = rd8(zRamBaseBank);
            r = adc8(0x04, base, false);
            tam(MPR5, r.a);
            for (uint16_t i = 0; i < 0x1400; i++) {
                byte = rd8((uint16_t)(0xC000 + i));
                wr8((uint16_t)(0xA000 + i), byte);
            }
            r = scene_next(r, 1);
            continue;
        }
        alu_compare(&r, op, OP_CALL_TWO);
        if (op == OP_CALL_TWO) {
            cpu.x = 0;
            r = SceneCallOverlay(r);
            cpu.x = 4;
            alu_nz(&r, 4);
            r = SceneCallOverlay(r);
            r = scene_next(r, 1);
            continue;
        }
        return r;
    }
}

/*
 * @routine 0048:7D8 LoadSceneScript
 * @run 6A:6FD8
 * WHAT IT DOES: Loads a scene: runs its list of ops, which map banks, read sectors from
 * the disc, and call the scene overlay, until an op it does not know ends it.
 * HOW: The scene's record, five bytes each from $74D1, holds the disc address and at Y 3
 * and 4 the list.
 * Entry: A scene. Exit: A the last op, N, Z and C from comparing it. Clobbers zTemp00-02,
 * zTemp08-09.
 */
Alu LoadSceneScript(Alu r)
{
    uint8_t scene = r.a;
    uint8_t byte;

    wr8(zTemp02, scene);
    r = record_pointer(r, scene, 5, 0xD1, 0x74);
    cpu.y = 3;
    byte = rd8_zp_y(zTemp00, cpu.y);
    wr8(zTemp08, byte);
    cpu.y = 4;
    byte = rd8_zp_y(zTemp00, cpu.y);
    wr8(zTemp09, byte);
    return scene_ops(r);
}

void abi_LoadSceneScript(void)
{
    abi_alu(LoadSceneScript(alu_from_cpu()));
}
