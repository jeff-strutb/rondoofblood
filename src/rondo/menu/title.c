/*
 * The title: the overlay read from sectors $C1 to $C5 into bank $6B and run at $8000 for mode 0.
 * Its frame steps through sub-modes by zGameSubMode: the screen set up, a scroll in, blinks, a
 * raster wipe, the logo's palette fades and cycles, and scripts; Run skips it once it has been
 * seen and the backup file read.
 *
 * A body returns plain values; its wrapper works out what the assembly leaves in the registers
 * and flags.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

enum {
    PALETTE_COLOURS = 0x20, PALETTE_ROTATE_END = 0x22, TILE_HIGH = 0x31, TILE_FIRST = 0x96,
    TILE_ROW_A = 0x60, TILE_ROW_B = 0xE0, SCROLL_IN_HIGH = 2, SCROLL_IN_STEP = 8, BLINK_WAITS = 3,
    BLINK_PALETTE = 0x32, WIPE_STEP = 2, WIPE_END = 0xE0, LOGO_WAITS = 0x0A, LOGO_SCRIPT = 1,
    LOGO_SOUND = 0x55, LOGO_PALETTE_A = 0x16, LOGO_PALETTE_B = 0x26, LOGO_PALETTE_C = 0x05,
    LOGO_HOLD = 3, LOGO_SCRIPT_B = 2, CYCLE_ADD = 0x49, CYCLE_END = 7, END_WAIT = 0x20,
    LAYER = 1
};

/*
 * @routine 00C1:189 TitlePaletteRotate
 * @run 6B:8189
 * WHAT IT DOES: Rotates part of a 16-colour palette by one: from the colour given to the last,
 * each takes the next one's place and the first goes to the end.
 * HOW: The palette read from the VCE into PaletteWork, the first colour kept in PaletteWorkSpare,
 * the rest moved down, then the 32 bytes written back.
 * Entry: X the palette in its high nibble, the first colour in its low. Exit: A the last byte
 * moved, X $22, Z and C set, N clear. Y and V kept.
 */
uint8_t TitlePaletteRotate(uint8_t which)
{
    uint8_t palette = (uint8_t)(which & 0xF0);
    uint8_t at = (uint8_t)((which & 0x0F) << 1);
    uint8_t moved = 0;

    wr8(VCE_ADDR_LO, palette);
    wr8(VCE_ADDR_HI, 0);
    tai(VCE_DATA_LO, PaletteWork, PALETTE_COLOURS);
    wr8(VCE_ADDR_LO, palette);
    wr8(VCE_ADDR_HI, 0);
    wr8(PaletteWorkSpare, rd8((uint16_t)(PaletteWork + at)));
    wr8(PaletteWorkSpare + 1, rd8((uint16_t)(PaletteWork + at + 1)));
    for (; at < PALETTE_ROTATE_END; at = (uint8_t)(at + 2)) {
        wr8((uint16_t)(PaletteWork + at), rd8((uint16_t)(PaletteWork + at + 2)));
        moved = rd8((uint16_t)(PaletteWork + at + 3));
        wr8((uint16_t)(PaletteWork + at + 1), moved);
    }
    for (uint16_t i = 0; i < PALETTE_COLOURS; i++)
        wr8((uint16_t)(VCE_DATA_LO + (i & 1)), rd8((uint16_t)(PaletteWork + i)));
    return moved;
}

void palette_rotate_exit_shim(uint8_t moved)
{
    cpu.a = moved;
    cpu.x = PALETTE_ROTATE_END;
    abi_nz(0);
    abi_flag(P_C, true);
}

void abi_TitlePaletteRotate(void)
{
    palette_rotate_exit_shim(TitlePaletteRotate(cpu.x));
}

/*
 * @routine 00C1:1EE TitleTileWrite
 * @run 6B:81EE
 * WHAT IT DOES: Writes the next tile of the title's cycling pattern to VRAM.
 * HOW: zTemp00 is the tile, counted on; its high byte $31.
 * Entry: nothing. Exit: A the tile written, N and Z from the next. X, Y, C and V kept.
 */
uint8_t TitleTileWrite(void)
{
    uint8_t tile = rd8(zTemp00);

    wr8(VDC_DATA_LO, tile);
    wr8(zTemp00, (uint8_t)(tile + 1));
    st2(TILE_HIGH);
    return tile;
}

void abi_TitleTileWrite(void)
{
    uint8_t tile = TitleTileWrite();

    cpu.a = tile;
    abi_nz((uint8_t)(tile + 1));
}

/*
 * @routine 00C1:1CA TitleTileRows
 * @run 6B:81CA
 * WHAT IT DOES: Writes the title's two rows of cycling tiles, $96 on by the frame count's bits 1
 * and 2: two tiles at VRAM $0060, two at $00E0.
 * HOW: SelectVramWriteAddress, SelectVramData, TitleTileWrite.
 * Entry: nothing. Exit: as TitleTileWrite; C and V clear from the first tile's add. X and Y kept.
 */
uint8_t TitleTileRows(void)
{
    uint8_t last;

    wr8(zTemp00, add_bytes((uint8_t)((rd8(zFrameTick) << 1) & 0x0C), TILE_FIRST, false).value);
    SelectVramWriteAddress();
    st1(TILE_ROW_A);
    st2(0);
    SelectVramData();
    TitleTileWrite();
    TitleTileWrite();
    SelectVramWriteAddress();
    st1(TILE_ROW_B);
    st2(0);
    SelectVramData();
    TitleTileWrite();
    last = TitleTileWrite();
    return last;
}

void tile_rows_exit_shim(uint8_t last)
{
    cpu.a = last;
    abi_nz((uint8_t)(last + 1));
    abi_flag(P_C, false);
    abi_flag(P_V, false);
}

void abi_TitleTileRows(void)
{
    tile_rows_exit_shim(TitleTileRows());
}

/* The next sub-mode; its value is what the INC leaves N and Z from. */
static uint8_t next_step(void)
{
    uint8_t step = (uint8_t)(rd8(zGameSubMode) + 1);

    wr8(zGameSubMode, step);
    return step;
}

/*
 * @routine 00C1:182 TitleWait
 * @run 6B:8182
 * WHAT IT DOES: Waits out zWaitTimer4, then the next sub-mode.
 * HOW: Counted down a frame.
 * Entry: nothing. Exit: N and Z from the timer, or from the new sub-mode. A, X, Y, C, V kept.
 */
uint8_t TitleWait(void)
{
    uint8_t left = (uint8_t)(rd8(zWaitTimer4) - 1);

    wr8(zWaitTimer4, left);
    return left != 0 ? left : next_step();
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
 * Entry: nothing. Exit: A 1, N and Z from zModeDone. X, Y, C and V kept.
 */
uint8_t TitleEnd(void)
{
    uint8_t done = (uint8_t)(rd8(zModeDone) + 1);

    wr8(TitleSeen, 1);
    wr8(zModeDone, done);
    return done;
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
 * Entry: nothing. Exit: as StartScriptSlot0Kind2 with script 0: A, X and Y 0, Z set.
 */
void TitleLastScript(void)
{
    wr8(zWaitTimer4, END_WAIT);
    next_step();
    StartScriptSlotKind2(0, 0);
}

void abi_TitleLastScript(void)
{
    TitleLastScript();
    cpu.a = 0;
    cpu.x = 0;
    cpu.y = 0;
    abi_nz(0);
}

/*
 * @routine 00C1:084 TitleScrollIn
 * @run 6B:8084
 * WHAT IT DOES: Scrolls the title in from the right, 8 pixels a frame, with the cycling tiles;
 * at the end, three blinks to come.
 * HOW: TitleTileRows; layer 1's X scroll, its high byte 2; zWaitTimer2 3 and the next sub-mode.
 * Entry: nothing. Exit: A the scroll's low byte, N and Z from it, C and V from its subtract; at
 * the end, A 3, N and Z from the new sub-mode. X and Y kept.
 */
ScrollIn TitleScrollIn(void)
{
    ScrollIn scroll;

    scroll.tiles = TitleTileRows();
    wr8(ScrollXHi + LAYER, SCROLL_IN_HIGH);
    scroll.low = sub_bytes(rd8(ScrollXLo + LAYER), SCROLL_IN_STEP, true);
    wr8(ScrollXLo + LAYER, scroll.low.value);
    scroll.done = scroll.low.value == 0;
    if (scroll.done) {
        wr8(zWaitTimer2, BLINK_WAITS);
        scroll.step = next_step();
    }
    return scroll;
}

void abi_TitleScrollIn(void)
{
    ScrollIn scroll = TitleScrollIn();

    tile_rows_exit_shim(scroll.tiles);
    cpu.a = scroll.done ? BLINK_WAITS : scroll.low.value;
    abi_nz(scroll.done ? scroll.step : scroll.low.value);
    abi_flag(P_C, scroll.low.carry);
    abi_flag(P_V, scroll.low.overflow);
}

/*
 * @routine 00C1:09E TitleBlink
 * @run 6B:809E
 * WHAT IT DOES: Every fourth frame, rotates palette 3 from colour 2, counting the blinks down to
 * the next sub-mode.
 * HOW: Bits 0 and 1 of zFrameTick; TitlePaletteRotate; zWaitTimer2.
 * Entry: nothing. Exit: between, N and V the frame count's bits 7 and 6, Z clear. Otherwise as
 * TitlePaletteRotate, with N and Z from the count, or from the new sub-mode.
 */
Blink TitleBlink(void)
{
    Blink blink = { false, 0, 0, 0 };

    blink.tick = rd8(zFrameTick);
    if (blink.tick & 0x03)
        return blink;
    blink.rotated = true;
    blink.moved = TitlePaletteRotate(BLINK_PALETTE);
    blink.left = (uint8_t)(rd8(zWaitTimer2) - 1);
    wr8(zWaitTimer2, blink.left);
    if (blink.left == 0)
        blink.left = next_step();
    return blink;
}

void abi_TitleBlink(void)
{
    Blink blink = TitleBlink();

    if (!blink.rotated) {
        abi_flag(P_Z, false);
        abi_flag(P_N, (blink.tick & 0x80) != 0);
        abi_flag(P_V, (blink.tick & 0x40) != 0);
        return;
    }
    palette_rotate_exit_shim(blink.moved);
    abi_nz(blink.left);
}

/*
 * @routine 00C1:0AF TitleWipe
 * @run 6B:80AF
 * WHAT IT DOES: Moves the raster wipe's line down 2 a frame; past $E0 the effect is off, and on
 * reaching 0 a frame's wait and the next sub-mode.
 * HOW: zRasterArg; zRasterEffect cleared; zWaitTimer4.
 * Entry: nothing. Exit: A the line, N, Z and C from comparing it with $E0; at 0, A 1, N and Z from
 * the new sub-mode, C set. V from the add. X and Y kept.
 */
Wipe TitleWipe(void)
{
    Wipe wipe;

    wipe.line = add_bytes(rd8(zRasterArg), WIPE_STEP, false);
    wr8(zRasterArg, wipe.line.value);
    if (wipe.line.value == 0) {
        wr8(zWaitTimer4, 1);
        wipe.step = next_step();
        return wipe;
    }
    if (wipe.line.value >= WIPE_END)
        wr8(zRasterEffect, 0);
    return wipe;
}

void abi_TitleWipe(void)
{
    Wipe wipe = TitleWipe();

    abi_flag(P_V, wipe.line.overflow);
    if (wipe.line.value == 0) {
        cpu.a = 1;
        abi_nz(wipe.step);
        abi_flag(P_C, wipe.line.carry);
        return;
    }
    cpu.a = wipe.line.value;
    abi_nz((uint8_t)(wipe.line.value - WIPE_END));
    abi_flag(P_C, wipe.line.value >= WIPE_END);
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

    wr8(zWaitTimer, LOGO_WAITS);
    wr8(zWaitTimer2, LOGO_WAITS);
    wr8(zWaitTimer3, LOGO_WAITS);
    StartScriptSlotKind1(LOGO_SCRIPT, 0);
    start.sound = SoundCommand(LOGO_SOUND, 0, 1);   /* X 0 and Y 1 as StartScriptSlot0Kind1 leaves them */
    start.step = next_step();
    return start;
}

void abi_TitleLogoStart(void)
{
    LogoStart start = TitleLogoStart();

    cpu.a = LOGO_SOUND;
    cpu.x = 0;
    cpu.y = LOGO_SCRIPT;
    abi_nz(LOGO_SOUND);
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
 * Entry: nothing. Exit: as the last of those steps left the registers.
 */
LogoFade TitleLogoFade(void)
{
    LogoFade fade = { LOGO_HOLDING, false, 0, 0, 0, 0, 0 };
    uint8_t wait = rd8(zWaitTimer);

    if (wait != 0) {
        wr8(zWaitTimer, (uint8_t)(wait - 1));
        fade.first = true;
        fade.moved = TitlePaletteRotate(LOGO_PALETTE_A);
    }
    fade.value = rd8(zWaitTimer2);
    if (!(fade.value & 0x80)) {
        fade.moved = TitlePaletteRotate(LOGO_PALETTE_B);
        wr8(zWaitTimer2, (uint8_t)(rd8(zWaitTimer2) - 1));
        wr8(zWaitTimer4, LOGO_HOLD);
        fade.how = LOGO_SECOND;
        return fade;
    }
    fade.value = rd8(zWaitTimer4);
    if (fade.value != 0) {
        fade.held = fade.value;
        fade.value = (uint8_t)(fade.value - 1);
        wr8(zWaitTimer4, fade.value);
        if (fade.value != 0)
            return fade;
        StartScriptSlotKind1(LOGO_SCRIPT_B, 0);
        fade.how = LOGO_SCRIPT_STARTED;
        return fade;
    }
    fade.count = rd8(zWaitTimer3);
    if (fade.count & 0x80) {
        wr8(zWaitTimer4, 1);
        fade.value = next_step();
        fade.how = LOGO_DONE;
        return fade;
    }
    fade.tick = rd8(zFrameTick);
    if (fade.tick & 1) {
        fade.how = LOGO_ODD_FRAME;
        return fade;
    }
    wr8(zWaitTimer3, (uint8_t)(fade.count - 1));
    fade.moved = TitlePaletteRotate(LOGO_PALETTE_C);
    fade.how = LOGO_THIRD;
    return fade;
}

void abi_TitleLogoFade(void)
{
    LogoFade fade = TitleLogoFade();

    if (fade.first)
        palette_rotate_exit_shim(fade.moved);
    switch (fade.how) {
    case LOGO_SECOND:
        palette_rotate_exit_shim(fade.moved);
        cpu.a = LOGO_HOLD;
        abi_nz(LOGO_HOLD);
        break;
    case LOGO_HOLDING:
        cpu.a = fade.held;
        abi_nz(fade.value);
        break;
    case LOGO_SCRIPT_STARTED:
        cpu.a = LOGO_SCRIPT_B;
        cpu.x = 0;
        cpu.y = LOGO_SCRIPT_B;
        abi_nz(LOGO_SCRIPT_B);
        break;
    case LOGO_DONE:
        cpu.a = 1;
        abi_nz(fade.value);
        break;
    case LOGO_ODD_FRAME:
        cpu.a = 0;
        abi_nz(0);
        break;
    case LOGO_THIRD:
        palette_rotate_exit_shim(fade.moved);
        break;
    }
}

/*
 * @routine 00C1:111 TitleCycle
 * @run 6B:8111
 * WHAT IT DOES: Every fourth frame, moves colour 0 of palettes 0, 1 and 2 on by $49 together,
 * until its low three bits are all set; then zWaitTimer4 $FF and the next sub-mode.
 * HOW: zWaitTimer4 counts the frames; the colour read from the VCE, summed in zTemp00 and 01, and
 * written to colours 0, $10 and $20.
 * Entry: nothing. Exit: between, N and Z from the count's low bits, A them. Otherwise A the
 * colour's low three bits, N, Z and C from comparing them with 7, V from the high byte's add; at
 * the end, A $FF, N and Z from the new sub-mode, C set. X and Y kept.
 */
Cycle TitleCycle(void)
{
    Cycle cycle = { false, 0, { 0, false, false }, { 0, false, false }, false, 0 };
    uint8_t count = (uint8_t)(rd8(zWaitTimer4) + 1);

    wr8(zWaitTimer4, count);
    cycle.count = (uint8_t)(count & 0x03);
    if (cycle.count != 0)
        return cycle;
    cycle.stepped = true;
    wr8(VCE_ADDR_LO, 0);
    wr8(VCE_ADDR_HI, 0);
    cycle.low = add_bytes(rd8(VCE_DATA_LO), CYCLE_ADD, false);
    wr8(zTemp00, cycle.low.value);
    cycle.high = add_bytes(0, rd8(VCE_DATA_HI), cycle.low.carry);
    wr8(zTemp01, cycle.high.value);
    for (uint8_t palette = 0; palette < 0x30; palette = (uint8_t)(palette + 0x10)) {
        wr8(VCE_ADDR_LO, palette);
        wr8(VCE_ADDR_HI, 0);
        wr8(VCE_DATA_LO, rd8(zTemp00));
        wr8(VCE_DATA_HI, rd8(zTemp01));
    }
    if ((rd8(zTemp00) & 0x07) == CYCLE_END) {
        wr8(zWaitTimer4, 0xFF);
        cycle.ended = true;
        cycle.step = next_step();
    }
    return cycle;
}

void abi_TitleCycle(void)
{
    Cycle cycle = TitleCycle();
    uint8_t bits;

    if (!cycle.stepped) {
        cpu.a = cycle.count;
        abi_nz(cycle.count);
        return;
    }
    abi_flag(P_V, cycle.high.overflow);
    if (cycle.ended) {
        cpu.a = 0xFF;
        abi_nz(cycle.step);
        abi_flag(P_C, true);
        return;
    }
    bits = (uint8_t)(cycle.low.value & 0x07);
    cpu.a = bits;
    abi_nz((uint8_t)(bits - CYCLE_END));
    abi_flag(P_C, bits >= CYCLE_END);
}
