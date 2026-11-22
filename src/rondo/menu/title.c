/*
 * The title: the overlay read from sectors $C1 to $C5 into bank $6B and run at $8000 for mode 0.
 * Its frame steps through sub-modes by zGameSubMode: the screen set up, a scroll in, blinks, a
 * raster wipe, the logo's palette fades and cycles, and scripts; Run skips it once it has been
 * seen and the backup file read. A sub-mode is entered by a jump through a table, so the Y it is
 * entered with (kept_y below) is whatever the mode loop had.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"

enum {
    PALETTE_BYTES = 0x20, PALETTE_ROTATE_END = 0x22, TILE_HIGH = 0x31, TILE_FIRST = 0x96,
    BLINK_WAITS = 3, BLINK_PALETTE = 0x32, WIPE_STEP = 2, WIPE_END = 0xE0, LOGO_WAITS = 0x0A,
    LOGO_SOUND = 0x55, LOGO_PALETTE_A = 0x16, LOGO_PALETTE_B = 0x26, LOGO_PALETTE_C = 0x05,
    CYCLE_ADD = 0x49, CYCLE_END = 7, END_WAIT = 0x20, LAYER = 1
};

/*
 * @routine 00C1:189 TitlePaletteRotate
 * @run 6B:8189
 * WHAT IT DOES: Rotates part of a 16-colour palette by one: from the colour given to the last,
 * each takes the next one's place and the first goes to the end.
 * HOW: The palette read from the VCE into PaletteWork, the first colour kept in PaletteWorkSpare,
 * the rest moved down, then the 32 bytes written back.
 * WAITS: no.
 * Entry: X the palette in its high nibble, the first colour in its low. Exit: A the last byte
 * moved, X $22, Z and C set, N clear. Y and V kept.
 */
Exit TitlePaletteRotate(uint8_t which, uint8_t kept_y)
{
    Exit exit = { 0 };
    uint8_t palette, at, moved, spare_lo, spare_hi;

    palette = (uint8_t)(which & 0xF0);
    wr8(VCE_ADDR_LO, palette);
    wr8(VCE_ADDR_HI, 0);
    block_move_tai(VCE_DATA_LO, PaletteWork, PALETTE_BYTES, palette, which, kept_y);
    wr8(VCE_ADDR_LO, palette);
    wr8(VCE_ADDR_HI, 0);
    at = (uint8_t)((which & 0x0F) << 1);
    spare_lo = rd8((uint16_t)(PaletteWork + at));
    wr8(PaletteWorkSpare, spare_lo);
    spare_hi = rd8((uint16_t)(PaletteWork + 1 + at));
    wr8(PaletteWorkSpare + 1, spare_hi);
    moved = 0;
    do {
        moved = rd8((uint16_t)(PaletteWork + 2 + at));
        wr8((uint16_t)(PaletteWork + at), moved);
        moved = rd8((uint16_t)(PaletteWork + 3 + at));
        wr8((uint16_t)(PaletteWork + 1 + at), moved);
        at = (uint8_t)(at + 2);
    } while (at < PALETTE_ROTATE_END);
    block_move_tia(PaletteWork, VCE_DATA_LO, PALETTE_BYTES, moved, at, kept_y);
    exit.a = moved;
    exit.x = at;
    exit.nz = 0;
    exit.c = true;
    exit.y_kept = true;
    return exit;
}

void abi_TitlePaletteRotate(void)
{
    exit_shim(TitlePaletteRotate(cpu.x, cpu.y));
}

/*
 * @routine 00C1:1EE TitleTileWrite
 * @run 6B:81EE
 * WHAT IT DOES: Writes the next tile of the title's cycling pattern to VRAM.
 * HOW: zTemp00 is the tile, counted on; its high byte $31.
 * WAITS: no.
 * Entry: nothing. Exit: A the tile written, N and Z from the next. X, Y, C and V kept.
 */
Exit TitleTileWrite(void)
{
    Exit exit = { 0 };
    uint8_t tile;

    tile = RD8(ZP->temp00);
    WR8(VDC->data_lo, tile);
    exit.nz = INC8(ZP->temp00);
    st2(TILE_HIGH);
    exit.a = tile;
    exit.x_kept = true;
    exit.y_kept = true;
    exit.c_kept = true;
    return exit;
}

void abi_TitleTileWrite(void)
{
    exit_shim(TitleTileWrite());
}

/*
 * @routine 00C1:1CA TitleTileRows
 * @run 6B:81CA
 * WHAT IT DOES: Writes the title's two rows of cycling tiles, $96 on by the frame count's bits 1
 * and 2: two tiles at VRAM $0060, two at $00E0.
 * HOW: SelectVramWriteAddress, SelectVramData, TitleTileWrite.
 * WAITS: no.
 * Entry: nothing. Exit: as TitleTileWrite; C and V clear from the first tile's add. X and Y kept.
 */
Exit TitleTileRows(void)
{
    Exit exit;
    uint8_t tick, low;

    tick = RD8(ZP->frame_tick);
    low = (uint8_t)(((tick << 1) & 0x0C) + TILE_FIRST);
    WR8(ZP->temp00, low);
    jsr_push(0x81D7);
    SelectVramWriteAddress();
    jsr_pop();
    st1(0x60);
    st2(0x00);
    jsr_push(0x81DE);
    SelectVramData();
    jsr_pop();
    jsr_push(0x81E0);
    TitleTileWrite();
    jsr_pop();
    jsr_push(0x81E2);
    TitleTileWrite();
    jsr_pop();
    jsr_push(0x81E5);
    SelectVramWriteAddress();
    jsr_pop();
    st1(0xE0);
    st2(0x00);
    jsr_push(0x81EC);
    SelectVramData();
    jsr_pop();
    jsr_push(0x81EE);
    TitleTileWrite();
    jsr_pop();
    exit = TitleTileWrite();
    exit.c = false;
    exit.c_kept = false;
    exit.v = false;
    exit.v_set = true;
    return exit;
}

void abi_TitleTileRows(void)
{
    exit_shim(TitleTileRows());
}

/* The next sub-mode; its value is what the INC leaves N and Z from. */
static uint8_t next_step(void)
{
    return INC8(ZP->game_sub_mode);
}

/*
 * @routine 00C1:182 TitleWait
 * @run 6B:8182
 * WHAT IT DOES: Waits out zWaitTimer4, then the next sub-mode.
 * HOW: Counted down a frame.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the timer, or from the new sub-mode. A, X, Y, C, V kept.
 */
uint8_t TitleWait(void)
{
    uint8_t left;

    left = DEC8(ZP->wait_timer4);
    if (left != 0) {
        return left;
    }
    return next_step();
}

void abi_TitleWait(void)
{
    abi_nz(TitleWait());
}

/*
 * @routine 00C1:17A TitleEnd
 * @run 6B:817A
 * WHAT IT DOES: The title's end: marks it seen, so Run skips it next time, and finishes the mode.
 * HOW: TitleSeen 1, zModeDone counted up.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, N and Z from zModeDone. X, Y, C and V kept.
 */
uint8_t TitleEnd(void)
{
    wr8(TitleSeen, 1);
    return INC8(ZP->mode_done);
}

void abi_TitleEnd(void)
{
    uint8_t done = TitleEnd();

    cpu.a = 1;
    abi_nz(done);
}

/*
 * @routine 00C1:170 TitleLastScript
 * @run 6B:8170
 * WHAT IT DOES: Starts script 0 on slot 0 as kind 2, with $20 frames to wait.
 * HOW: zWaitTimer4; the next sub-mode; StartScriptSlotKind2.
 * WAITS: no.
 * Entry: nothing. Exit: as StartScriptSlot0Kind2 with script 0: A, X and Y 0, Z set.
 */
void TitleLastScript(void)
{
    WR8(ZP->wait_timer4, END_WAIT);
    next_step();
    StartScriptSlot0Kind2(0);
}

void abi_TitleLastScript(void)
{
    TitleLastScript();
    script_slot_exit_shim(0);
    cpu.x = 0;
}

/*
 * @routine 00C1:084 TitleScrollIn
 * @run 6B:8084
 * WHAT IT DOES: Scrolls the title in from the right, 8 pixels a frame, with the cycling tiles;
 * at the end, three blinks to come.
 * HOW: TitleTileRows; layer 1's X scroll, its high byte 2; zWaitTimer2 3 and the next sub-mode.
 * WAITS: no.
 * Entry: nothing. Exit: A the scroll's low byte, N and Z from it, C and V from its subtract; at
 * the end, A 3, N and Z from the new sub-mode. X and Y kept.
 */
Exit TitleScrollIn(void)
{
    Exit exit;
    uint8_t low, diff;

    jsr_push(0x8087);
    exit = TitleTileRows();
    jsr_pop();
    WR8(SCROLL->x_hi[LAYER], 2);
    low = RD8(SCROLL->x_lo[LAYER]);
    diff = (uint8_t)(low - 8);
    exit.c = low >= 8;
    exit.c_kept = false;
    exit.v = ((low ^ 8) & (low ^ diff) & 0x80) != 0;
    exit.v_set = true;
    WR8(SCROLL->x_lo[LAYER], diff);
    exit.a = diff;
    exit.nz = diff;
    if (diff == 0) {
        WR8(ZP->wait_timer2, BLINK_WAITS);
        exit.nz = next_step();
        exit.a = BLINK_WAITS;
    }
    return exit;
}

void abi_TitleScrollIn(void)
{
    exit_shim(TitleScrollIn());
}

/*
 * @routine 00C1:09E TitleBlink
 * @run 6B:809E
 * WHAT IT DOES: Every fourth frame, rotates palette 3 from colour 2, counting the blinks down to
 * the next sub-mode.
 * HOW: Bits 0 and 1 of zFrameTick; TitlePaletteRotate; zWaitTimer2.
 * WAITS: no.
 * Entry: nothing. Exit: between, N and V the frame count's bits 7 and 6, Z clear. Otherwise as
 * TitlePaletteRotate, with N and Z from the count, or from the new sub-mode.
 */
Exit TitleBlink(uint8_t kept_y)
{
    Exit exit = { 0 };
    uint8_t tick, left;

    tick = RD8(ZP->frame_tick);
    exit.v = (tick & 0x40) != 0;                /* TST sets N and V from the byte and Z from the mask */
    exit.v_set = true;
    if ((tick & 0x03) != 0) {
        exit.a_kept = true;
        exit.x_kept = true;
        exit.y_kept = true;
        exit.c_kept = true;
        exit.nz = (uint8_t)((tick & 0x80) | 0x01);
        return exit;
    }
    jsr_push(0x80A8);
    exit = TitlePaletteRotate(BLINK_PALETTE, kept_y);
    jsr_pop();
    exit.v = (tick & 0x40) != 0;
    exit.v_set = true;
    left = DEC8(ZP->wait_timer2);
    exit.nz = left;
    if (left == 0) {
        exit.nz = next_step();
    }
    return exit;
}

void abi_TitleBlink(void)
{
    exit_shim(TitleBlink(cpu.y));
}

/*
 * @routine 00C1:0AF TitleWipe
 * @run 6B:80AF
 * WHAT IT DOES: Moves the raster wipe's line down 2 a frame; past $E0 the effect is off, and on
 * reaching 0 a frame's wait and the next sub-mode.
 * HOW: zRasterArg; zRasterEffect cleared; zWaitTimer4.
 * WAITS: no.
 * Entry: nothing. Exit: A the line, N, Z and C from comparing it with $E0; at 0, A 1, N and Z from
 * the new sub-mode, C set. V from the add. X and Y kept.
 */
Exit TitleWipe(void)
{
    Exit exit = { 0 };
    uint8_t line, sum;
    unsigned total;

    line = RD8(ZP->raster_arg);
    total = (unsigned)line + WIPE_STEP;
    sum = (uint8_t)total;
    exit.c = total > 0xFF;
    exit.v = ((line ^ sum) & (WIPE_STEP ^ sum) & 0x80) != 0;
    exit.v_set = true;
    WR8(ZP->raster_arg, sum);
    exit.x_kept = true;
    exit.y_kept = true;
    if (sum == 0) {
        WR8(ZP->wait_timer4, 1);
        exit.nz = next_step();
        exit.a = 1;
        return exit;
    }
    exit.a = sum;
    exit.nz = (uint8_t)(sum - WIPE_END);
    exit.c = sum >= WIPE_END;
    if (exit.c) {
        WR8(ZP->raster_effect, 0);
    }
    return exit;
}

void abi_TitleWipe(void)
{
    exit_shim(TitleWipe());
}

/*
 * @routine 00C1:0C6 TitleLogoStart
 * @run 6B:80C6
 * WHAT IT DOES: Starts the logo: three waits of 10, script 1 on slot 0, and music $55.
 * HOW: zWaitTimer, zWaitTimer2 and zWaitTimer3; StartScriptSlotKind1; SoundCommand.
 * WAITS: no.
 * Entry: nothing. Exit: as SoundCommand after the script's start, with N and Z from the new
 * sub-mode.
 */
LogoStart TitleLogoStart(void)
{
    LogoStart start;

    WR8(ZP->wait_timer, LOGO_WAITS);
    WR8(ZP->wait_timer2, LOGO_WAITS);
    WR8(ZP->wait_timer3, LOGO_WAITS);
    jsr_push(0x80D3);
    StartScriptSlot0Kind1(1);
    jsr_pop();
    jsr_push(0x80D8);
    start.sound = SoundCommand(LOGO_SOUND, 0, 1);   /* X 0 and Y 1 as StartScriptSlot0Kind1 leaves them */
    jsr_pop();
    start.step = next_step();
    return start;
}

void abi_TitleLogoStart(void)
{
    LogoStart start = TitleLogoStart();

    cpu.a = LOGO_SOUND;
    cpu.x = 0;
    cpu.y = 1;
    sound_command_exit_shim(LOGO_SOUND, start.sound);
    abi_nz(start.step);
}

/*
 * @routine 00C1:0DB TitleLogoFade
 * @run 6B:80DB
 * WHAT IT DOES: The logo's palettes: while zWaitTimer lasts, palette 1 rotates from colour 6;
 * while zWaitTimer2 is not negative, palette 2 from colour 6 and three frames' hold; after a hold,
 * script 2 on slot 0; then every other frame palette 0 from colour 5, until zWaitTimer3 goes
 * negative, a frame's wait and the next sub-mode.
 * HOW: TitlePaletteRotate by the waits; zWaitTimer4 the hold.
 * WAITS: no.
 * Entry: nothing. Exit: as the last of those steps left the registers.
 */
Exit TitleLogoFade(uint8_t kept_y)
{
    Exit exit = { 0 };
    uint8_t wait, second, hold, count, tick, left;
    bool rotated = false;

    exit.a_kept = true;
    exit.x_kept = true;
    exit.y_kept = true;
    exit.c_kept = true;
    wait = RD8(ZP->wait_timer);
    if (wait != 0) {
        DEC8(ZP->wait_timer);
        jsr_push(0x80E6);
        exit = TitlePaletteRotate(LOGO_PALETTE_A, kept_y);
        jsr_pop();
        rotated = true;
    }
    second = RD8(ZP->wait_timer2);
    if (!(second & 0x80)) {
        jsr_push(0x80EF);
        exit = TitlePaletteRotate(LOGO_PALETTE_B, kept_y);
        jsr_pop();
        DEC8(ZP->wait_timer2);
        WR8(ZP->wait_timer4, 3);
        exit.a = 3;
        exit.a_kept = false;
        exit.nz = 3;
        return exit;
    }
    (void)rotated;
    hold = RD8(ZP->wait_timer4);
    if (hold != 0) {
        left = DEC8(ZP->wait_timer4);
        exit.a = hold;
        exit.a_kept = false;
        exit.nz = left;
        if (left != 0) {
            return exit;
        }
        StartScriptSlot0Kind1(2);
        exit.a = 2;
        exit.a_kept = false;
        exit.x = 0;
        exit.x_kept = false;
        exit.y = 2;
        exit.y_kept = false;
        exit.nz = 2;
        return exit;
    }
    count = RD8(ZP->wait_timer3);
    if (count & 0x80) {
        WR8(ZP->wait_timer4, 1);
        exit.a = 1;
        exit.a_kept = false;
        exit.nz = next_step();
        return exit;
    }
    tick = RD8(ZP->frame_tick);
    if (tick & 0x01) {
        exit.a = 0;
        exit.a_kept = false;
        exit.nz = 0;
        return exit;
    }
    DEC8(ZP->wait_timer3);
    jsr_push(0x8110);
    exit = TitlePaletteRotate(LOGO_PALETTE_C, kept_y);
    jsr_pop();
    if (exit.a_kept) {
        exit.a = 0;
        exit.a_kept = false;
    }
    return exit;
}

void abi_TitleLogoFade(void)
{
    exit_shim(TitleLogoFade(cpu.y));
}

/*
 * @routine 00C1:111 TitleCycle
 * @run 6B:8111
 * WHAT IT DOES: Every fourth frame, moves colour 0 of palettes 0, 1 and 2 on by $49 together,
 * until its low three bits are all set; then zWaitTimer4 $FF and the next sub-mode.
 * HOW: zWaitTimer4 counts the frames; the colour read from the VCE, summed in zTemp00 and 01, and
 * written to colours 0, $10 and $20.
 * WAITS: no.
 * Entry: nothing. Exit: between, N and Z from the count's low bits, A them. Otherwise A the
 * colour's low three bits, N, Z and C from comparing them with 7, V from the high byte's add; at
 * the end, A $FF, N and Z from the new sub-mode, C set. X and Y kept.
 */
Exit TitleCycle(void)
{
    Exit exit = { 0 };
    uint8_t count, low, high, value, hi_value, palette, bits;
    unsigned total;
    bool carry;

    exit.x_kept = true;
    exit.y_kept = true;
    INC8(ZP->wait_timer4);
    count = RD8(ZP->wait_timer4);
    bits = (uint8_t)(count & 0x03);
    exit.a = bits;
    exit.nz = bits;
    exit.c_kept = true;
    if (bits != 0) {
        return exit;
    }
    wr8(VCE_ADDR_LO, 0);
    wr8(VCE_ADDR_HI, 0);
    low = rd8(VCE_DATA_LO);
    total = (unsigned)low + CYCLE_ADD;
    value = (uint8_t)total;
    carry = total > 0xFF;
    WR8(ZP->temp00, value);
    high = rd8(VCE_DATA_HI);
    total = (unsigned)high + (carry ? 1 : 0);
    hi_value = (uint8_t)total;
    exit.v = ((high ^ hi_value) & (0 ^ hi_value) & 0x80) != 0;
    exit.v_set = true;
    WR8(ZP->temp01, hi_value);
    for (palette = 0; palette < 0x30; palette = (uint8_t)(palette + 0x10)) {
        wr8(VCE_ADDR_LO, palette);
        wr8(VCE_ADDR_HI, 0);
        value = RD8(ZP->temp00);
        wr8(VCE_DATA_LO, value);
        hi_value = RD8(ZP->temp01);
        wr8(VCE_DATA_HI, hi_value);
    }
    value = RD8(ZP->temp00);
    bits = (uint8_t)(value & 0x07);
    exit.a = bits;
    exit.nz = (uint8_t)(bits - CYCLE_END);
    exit.c = bits >= CYCLE_END;
    exit.c_kept = false;
    if (bits == CYCLE_END) {
        WR8(ZP->wait_timer4, 0xFF);
        exit.a = 0xFF;
        exit.nz = next_step();
    }
    return exit;
}

void abi_TitleCycle(void)
{
    exit_shim(TitleCycle());
}

