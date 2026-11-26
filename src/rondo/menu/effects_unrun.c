/*
 * The ending's palette fade (EndingEffectFade, 0111:371) in the versions of the overlay whose copy differs from
 * 0111's in the table of lists for a negative list number: sectors 1E65 (table $9412), 20C0 ($940E), 224F ($9410,
 * the same bytes at 25D7) and 23FA ($9416). A script loads each and no script runs it, so
 * tools/rondo/scripts/unrun_effects_0E.txt, _0C, _02 and _0A calls each at its entry. The body is ending.c's EndingEffectFade with the
 * table as its argument; the few helpers it needs are copied from ending.c, where they are file-local.
 *
 * The colour buffer at $3C60 (SavedColoursLo) is a symbol, not a record yet: it is read through rd8 and wr8 at
 * EFFECT_COLOURS, as ending.c does (a row request for the owner of the record lists).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "effects_unrun.h"

enum {
    EFFECT_COLOURS = 0x3C60, EFFECT_LIST_POINTER = 0x9402, EFFECT_COLOUR_BYTES = 0x20, EFFECT_ROUNDS = 8,
    LIST_END = 0xFF
};

/* The slot given back: every field of it cleared, the kind first. */
static void clear_effect_slot(uint8_t slot)
{
    WR8(SLOTS->kind[slot], 0);
    WR8(SLOTS->script[slot], 0);
    WR8(SLOTS->f18[slot], 0);
    WR8(SLOTS->f24[slot], 0);
    WR8(SLOTS->timer[slot], 0);
    WR8(SLOTS->f3c[slot], 0);
    WR8(SLOTS->f48[slot], 0);
    WR8(SLOTS->f54[slot], 0);
}

/* A plain Exit that keeps what it does not name. */
static Exit effect_exit(uint8_t a, uint8_t nz)
{
    Exit exit = { 0 };

    exit.a = a;
    exit.nz = nz;
    exit.x_kept = exit.y_kept = exit.c_kept = true;
    return exit;
}

/* The slot's list address in zTemp00 and zTemp01: from the table for a negative list number, else through the
 * pointer at $9402. */
static void select_effect_list(uint8_t list, uint16_t negative_table)
{
    uint8_t y = (uint8_t)(list << 1);

    if ((list & 0x80) != 0) {
        WR8(ZP->temp00, rd8((uint16_t)(negative_table + y)));
        WR8(ZP->temp01, rd8((uint16_t)(negative_table + 1 + y)));
        return;
    }
    WR8(ZP->bios_a.lo, rd8(EFFECT_LIST_POINTER));
    WR8(ZP->bios_a.hi, rd8(EFFECT_LIST_POINTER + 1));
    WR8(ZP->temp00, RD8_PTR_Y(ZP->bios_a.lo, y));
    y++;
    WR8(ZP->temp01, RD8_PTR_Y(ZP->bios_a.lo, y));
}

/* The V the last SBC of a colour left (set false: none ran). */
typedef struct { bool set; bool v; } FadeV;

/* One colour word of the fade: each of its three 3-bit components counted down by one unless it is zero; the V of
 * the last subtract it ran. */
static FadeV fade_colour(uint8_t x)
{
    uint8_t low, part, blue, high, carry_out, red;
    FadeV last = { false, false };

    low = rd8((uint16_t)(EFFECT_COLOURS + x));
    part = (uint8_t)(low & 0x07);
    if (part != 0) {
        part--;
    }
    WR8(ZP->temp02, part);
    low = rd8((uint16_t)(EFFECT_COLOURS + x));
    blue = (uint8_t)(low & 0x38);
    if (blue != 0) {
        last.v = (((blue ^ 0x08) & (blue ^ (uint8_t)(blue - 0x08))) & 0x80) != 0;
        last.set = true;
        blue = (uint8_t)(blue - 0x08);
    }
    part = (uint8_t)(blue | RD8(ZP->temp02));
    WR8(ZP->temp02, part);
    low = rd8((uint16_t)(EFFECT_COLOURS + x));
    high = rd8((uint16_t)(EFFECT_COLOURS + 1 + x));
    carry_out = (uint8_t)(high & 1);
    wr8((uint16_t)(EFFECT_COLOURS + 1 + x), (uint8_t)(high >> 1));
    low = (uint8_t)((low >> 1) | (carry_out << 7));
    red = (uint8_t)(low & 0xE0);
    if (red != 0) {
        last.v = (((red ^ 0x20) & (red ^ (uint8_t)(red - 0x20))) & 0x80) != 0;
        last.set = true;
        red = (uint8_t)(red - 0x20);
    }
    carry_out = (uint8_t)(red >> 7);
    red = (uint8_t)(red << 1);
    high = rd8((uint16_t)(EFFECT_COLOURS + 1 + x));
    wr8((uint16_t)(EFFECT_COLOURS + 1 + x), (uint8_t)((high << 1) | carry_out));
    part = RD8(ZP->temp02);
    wr8((uint16_t)(EFFECT_COLOURS + x), (uint8_t)(red | part));
    return last;
}

/* The fade of any version: TABLE is that version's table of lists for a negative list number. */
static Exit effect_fade(uint8_t slot, uint16_t table)
{
    Exit exit;
    uint8_t timer, count, y, value, high, x;
    FadeV seen, v = { false, false };

    timer = RD8(SLOTS->timer[slot]);
    if (timer != 0) {
        return effect_exit(timer, DEC8(SLOTS->timer[slot]));
    }
    INC8(SLOTS->f24[slot]);
    count = RD8(SLOTS->f24[slot]);
    if (count >= EFFECT_ROUNDS) {
        clear_effect_slot(slot);
        exit = effect_exit(count, (uint8_t)(count - EFFECT_ROUNDS));
        exit.c = true;
        exit.c_kept = false;
        return exit;
    }
    select_effect_list(RD8(SLOTS->script[slot]), table);
    y = 0;
    value = RD8_PTR_Y(ZP->temp00, y);
    WR8(SLOTS->timer[slot], value);
    x = slot;
    y++;
    for (;;) {
        value = RD8_PTR_Y(ZP->temp00, y);
        if (value == LIST_END) {
            exit = effect_exit(LIST_END, 0);
            exit.c = true;
            exit.c_kept = false;
            exit.x = x;
            exit.x_kept = false;
            exit.y = y;
            exit.y_kept = false;
            exit.v = v.v;
            exit.v_set = v.set;
            return exit;
        }
        WR8(ZP->temp08, value);
        WR8(VCE->addr_lo, value);
        y++;
        high = RD8_PTR_Y(ZP->temp00, y);
        WR8(ZP->temp09, high);
        WR8(VCE->addr_hi, high);
        y++;
        block_move_tai(ADDR_OF8(VCE->data_lo), EFFECT_COLOURS, EFFECT_COLOUR_BYTES, high, x, y);
        for (x = 0; x < EFFECT_COLOUR_BYTES; x = (uint8_t)(x + 2)) {
            seen = fade_colour(x);
            if (seen.set) {
                v = seen;
            }
        }
        WR8(VCE->addr_lo, RD8(ZP->temp08));
        high = RD8(ZP->temp09);
        WR8(VCE->addr_hi, high);
        block_move_tia(EFFECT_COLOURS, ADDR_OF8(VCE->data_lo), EFFECT_COLOUR_BYTES, high, x, y);
    }
}

/*
 * @routine 1E65:371 EndingEffectFadeA
 * @run 6C:8371
 * WHAT IT DOES: EndingEffectFade (0111:371), as the overlay of sector 1E65 has it: its table of lists for a negative list
 * number is at $9412.
 * HOW: effect_fade.
 * WAITS: no.
 * Entry, exit: as EndingEffectFade.
 */
Exit EndingEffectFadeA(uint8_t slot)
{
    return effect_fade(slot, 0x9412);
}

void abi_EndingEffectFadeA(void)
{
    exit_shim(EndingEffectFadeA(cpu.x));
}

/*
 * @routine 20C0:371 EndingEffectFadeB
 * @run 6C:8371
 * WHAT IT DOES: EndingEffectFade (0111:371), as the overlay of sector 20C0 has it: its table of lists for a negative list
 * number is at $940E.
 * HOW: effect_fade.
 * WAITS: no.
 * Entry, exit: as EndingEffectFade.
 */
Exit EndingEffectFadeB(uint8_t slot)
{
    return effect_fade(slot, 0x940E);
}

void abi_EndingEffectFadeB(void)
{
    exit_shim(EndingEffectFadeB(cpu.x));
}

/*
 * @routine 224F:371 EndingEffectFadeC
 * @run 6C:8371
 * WHAT IT DOES: EndingEffectFade (0111:371), as the overlay of sector 224F has it: its table of lists for a negative list
 * number is at $9410. The same bytes are at 25D7:371.
 * HOW: effect_fade.
 * WAITS: no.
 * Entry, exit: as EndingEffectFade.
 */
Exit EndingEffectFadeC(uint8_t slot)
{
    return effect_fade(slot, 0x9410);
}

void abi_EndingEffectFadeC(void)
{
    exit_shim(EndingEffectFadeC(cpu.x));
}

/*
 * @routine 23FA:371 EndingEffectFadeD
 * @run 6C:8371
 * WHAT IT DOES: EndingEffectFade (0111:371), as the overlay of sector 23FA has it: its table of lists for a negative list
 * number is at $9416.
 * HOW: effect_fade.
 * WAITS: no.
 * Entry, exit: as EndingEffectFade.
 */
Exit EndingEffectFadeD(uint8_t slot)
{
    return effect_fade(slot, 0x9416);
}

void abi_EndingEffectFadeD(void)
{
    exit_shim(EndingEffectFadeD(cpu.x));
}
