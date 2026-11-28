/*
 * The ending's palette effects in the versions of its overlay no read the game makes can load (sectors 0441, 0651
 * and 1231, bank $6C at $8000). The overlay at sector 0111 is the loaded one (ending.c holds EndingEffectStep,
 * EndingEffectDispatch, EndingEffectList and EndingEffectFade); these versions carry the same engine at the run
 * addresses $829F to $85E6: twelve slots at $3C00 (SLOTS), each of a kind 1 to 5 that the dispatcher sends to
 * one of five handlers, and every frame EffectStep runs them. Kind 1 plays a list of packed palettes, kind 2 darkens
 * palettes in steps, kind 3 steps palette colours down by a growing amount, kind 4 writes one colour, kind 5 flashes
 * palettes and puts them back. The three versions differ only in the address of one table of lists (the kind 2 and
 * kind 4 lists a negative list number picks), so each routine is one body here with that table as its argument.
 * What lies between: the 64 bytes at $84BB are LoadPackedPaletteC's bytes (vram_update.c, 0111:4CE), and $8151 to
 * $829E are the 00C5 overlay's (VramListRun and the rest), which their C serves.
 *
 * Fields with no name yet and kept in their old address form until the owner of the lists adds them: the 32 bytes at
 * $3C60 are 16 colours as low and high bytes by turns (SAVCOL's two columns of 16 do not describe them as these
 * handlers use them), and zero page $3C, set while the kind 5 effect runs.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "vram_update.h"
#include "effect_slots_census.h"
#include "list_channels_census.h"
#include "list_channels.h"

enum {
    EFFECT_COLOURS = 0x3C60, EFFECT_COLOUR_BYTES = 0x20,
    EFFECT_ACTIVE = 0x203C,             /* zero page $3C: 1 while kind 5 runs, 0 when it ends */
    LIST_POINTER = 0x9400, FADE_POINTER = 0x9402, SHADE_POINTER = 0x9404,
    COLOUR_POINTER = 0x9406,
    SAVED_BACKGROUND_BYTES = 0x200, SAVED_SPRITE_BYTES = 0x100,
    MPR5 = 0x20, KIND_COUNT_MAX = 7, VCE_SAVE = 0xB000, VCE_SAVE_BYTES = 0x400, BANK_PLAIN = 0x12, BANK_IDLE = 0x30,
    NO_KIND = 0xFF, LIST_END = 0xFF, FADE_ROUNDS = 8, SHADE_ROUNDS = 8, FLASH_ROUNDS = 0x10
};

/*
 * What differs between the versions of the overlay: the tables a negative list number picks (kinds 1, 2 and 4), the
 * return address of kind 1's call of the palette loader, the handlers' addresses by kind (the dispatcher's table),
 * and where kind 5 keeps its tables and saved palettes. Versions B have two handlers more, kinds 6 and 7.
 */
typedef struct {
    uint16_t list_table, fade_table, colour_table, loader_ret;
    uint16_t handler[KIND_COUNT_MAX];
    uint8_t kinds;
    uint16_t flash_colours, flash_palettes, saved_background, saved_sprites;
    uint16_t save_ret, restore_ret;
    uint16_t type_table, step_paused_ret, step_loop_ret;
} EffectLayout;

/* A plain Exit that keeps what it does not name. */
static Exit effect_exit(uint8_t a, uint8_t nz)
{
    Exit exit = { 0 };

    exit.a = a;
    exit.nz = nz;
    exit.x_kept = exit.y_kept = exit.c_kept = true;
    return exit;
}

/* The slot given back: all eight of its fields cleared, in the order of the columns. */
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

/* What a palette load leaves in Y, C and V, laid over an exit. */
static void effect_after_load(Exit *exit, PaletteLoad load)
{
    exit->y = 9;
    exit->y_kept = false;
    exit->c = load.sum.carry;
    exit->c_kept = false;
    exit->v = load.sum.overflow;
    exit->v_set = true;
}

/* The slot's list address in zTemp00 and zTemp01: from the table for a negative list number (a word each), else
 * through the pointer the overlay keeps at POINTER (a table of the lists, a word each). */
static void select_effect_list(uint8_t list, uint16_t negative_table, uint16_t pointer)
{
    uint8_t y = (uint8_t)(list << 1);

    if ((list & 0x80) != 0) {
        WR8(ZP->temp00, rd8((uint16_t)(negative_table + y)));
        WR8(ZP->temp01, rd8((uint16_t)(negative_table + 1 + y)));
        return;
    }
    WR8(ZP->bios_a.lo, rd8(pointer));
    WR8(ZP->bios_a.hi, rd8((uint16_t)(pointer + 1)));
    WR8(ZP->temp00, RD8_PTR_Y(ZP->bios_a.lo, y));
    y++;
    WR8(ZP->temp01, RD8_PTR_Y(ZP->bios_a.lo, y));
}

/*
 * Kind 1: a palette effect that plays a list of palette loads. Once its timer has run down it takes the list's
 * palettes one a step (each loaded by the packed palette loader, the timer set from the list's first byte); when the
 * list ends it counts a round and starts the list again, and when the rounds the list names have been played it gives
 * the slot back.
 */
static Exit run_effect_list(const EffectLayout *layout, uint8_t slot)
{
    Exit exit;
    PaletteLoad load = { { 0, false, false }, 0 };
    uint8_t timer, y, value, rounds, count;
    bool loaded = false;

    for (;;) {
        timer = RD8(SLOTS->timer[slot]);
        if (timer != 0) {
            exit = effect_exit(timer, DEC8(SLOTS->timer[slot]));
            if (loaded) {
                effect_after_load(&exit, load);
            }
            return exit;
        }
        select_effect_list(RD8(SLOTS->script[slot]), layout->list_table, LIST_POINTER);
        y = 0;
        value = RD8_PTR_Y(ZP->temp00, y);
        WR8(SLOTS->timer[slot], value);
        for (;;) {
            y = (uint8_t)((uint8_t)(RD8(SLOTS->f18[slot]) << 2) + 2);
            value = RD8_PTR_Y(ZP->temp00, y);
            if (value != LIST_END) {
                jsr_push(layout->loader_ret);
                load = LoadPackedPaletteC(y);
                jsr_pop();
                loaded = true;
                INC8(SLOTS->f18[slot]);
                break;
            }
            INC8(SLOTS->f24[slot]);
            WR8(SLOTS->f18[slot], 0);
            y = 1;
            value = RD8_PTR_Y(ZP->temp00, y);
            if ((value & 0x80) != 0) {
                continue;
            }
            rounds = (uint8_t)(value & 0x7F);
            count = RD8(SLOTS->f24[slot]);
            if (rounds >= count) {
                continue;
            }
            clear_effect_slot(slot);
            exit = effect_exit(rounds, (uint8_t)(rounds - count));
            exit.c = false;
            exit.c_kept = false;
            exit.y = 1;
            exit.y_kept = false;
            if (loaded) {
                exit.v = load.sum.overflow;
                exit.v_set = true;
            }
            return exit;
        }
    }
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
        last.v = sub_bytes(blue, 0x08, true).overflow;
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
        last.v = sub_bytes(red, 0x20, true).overflow;
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

/*
 * Kind 2: a palette effect that fades palettes down. Once its timer has run down it counts a round, gives the slot
 * back after the eighth, and otherwise takes the list's palettes (an address of the VCE and a count each) and darkens
 * every colour of them by one step in each of its three components, writing them back to the VCE. The list comes
 * from the table of this version for a negative list number, else through the pointer at $9402.
 */
static Exit run_effect_fade(const EffectLayout *layout, uint8_t slot)
{
    Exit exit;
    uint8_t timer, y, value, count, low, high, x;
    FadeV seen, v = { false, false };

    timer = RD8(SLOTS->timer[slot]);
    if (timer != 0) {
        return effect_exit(timer, DEC8(SLOTS->timer[slot]));
    }
    INC8(SLOTS->f24[slot]);
    count = RD8(SLOTS->f24[slot]);
    if (count >= FADE_ROUNDS) {
        clear_effect_slot(slot);
        exit = effect_exit(count, (uint8_t)(count - FADE_ROUNDS));
        exit.c = true;
        exit.c_kept = false;
        return exit;
    }
    select_effect_list(RD8(SLOTS->script[slot]), layout->fade_table, FADE_POINTER);
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
        block_move_tai(ADDR_OF(VCE->data_lo), EFFECT_COLOURS, EFFECT_COLOUR_BYTES, high, x, y);
        for (x = 0; x < EFFECT_COLOUR_BYTES; x = (uint8_t)(x + 2)) {
            seen = fade_colour(x);
            if (seen.set) {
                v = seen;
            }
        }
        low = RD8(ZP->temp08);
        WR8(VCE->addr_lo, low);
        high = RD8(ZP->temp09);
        WR8(VCE->addr_hi, high);
        block_move_tia(EFFECT_COLOURS, ADDR_OF(VCE->data_lo), EFFECT_COLOUR_BYTES, high, x, y);
    }
}

/* A colour component taken down by an amount and not below zero: the subtract's result, or zero when it borrowed. */
static uint8_t take_down(uint8_t component, uint8_t amount)
{
    Sum sum = sub_bytes(component, amount, true);

    return sum.carry ? sum.value : 0;
}

/*
 * Kind 3: a palette effect that darkens palettes by an amount that grows with its count. Once its timer has run down
 * it counts a round (the slot is given back after the eighth), and takes the list's palettes (a VCE address and a
 * pointer to blocks of eight colours with their ninth bits in the first byte each) from the pointer at $9404; each
 * colour is written to the VCE with each of its three components taken down by 7 less the count (blue), that times
 * 8 (green) and that times 32 (red, the top two bits of the low byte and the ninth bit), not below zero.
 */
static Exit run_effect_shade(uint8_t slot)
{
    Exit exit;
    Sum sum;
    uint8_t timer, count, step, y, value, entries, ninth, a, blue, green, red, left;
    bool v = false, v_set = false, carry;

    timer = RD8(SLOTS->timer[slot]);
    if (timer != 0) {
        return effect_exit(timer, DEC8(SLOTS->timer[slot]));
    }
    INC8(SLOTS->f24[slot]);
    count = RD8(SLOTS->f24[slot]);
    if (count >= SHADE_ROUNDS) {
        clear_effect_slot(slot);
        exit = effect_exit(count, (uint8_t)(count - SHADE_ROUNDS));
        exit.c = true;
        exit.c_kept = false;
        return exit;
    }
    step = (uint8_t)(count ^ 7);
    WR8(ZP->temp08, step);
    WR8(ZP->temp09, (uint8_t)(step << 3));
    WR8(ZP->temp0a, (uint8_t)(step << 5));
    y = (uint8_t)(RD8(SLOTS->script[slot]) << 1);
    WR8(ZP->bios_a.lo, rd8(SHADE_POINTER));
    WR8(ZP->bios_a.hi, rd8(SHADE_POINTER + 1));
    WR8(ZP->temp00, RD8_PTR_Y(ZP->bios_a.lo, y));
    y++;
    WR8(ZP->temp01, RD8_PTR_Y(ZP->bios_a.lo, y));
    y = 0;
    value = RD8_PTR_Y(ZP->temp00, y);
    WR8(SLOTS->timer[slot], value);
    y++;
    for (;;) {
        value = RD8_PTR_Y(ZP->temp00, y);
        if (value == LIST_END) {
            exit = effect_exit(LIST_END, 0);
            exit.c = true;
            exit.c_kept = false;
            exit.y = y;
            exit.y_kept = false;
            exit.v = v;
            exit.v_set = v_set;
            return exit;
        }
        WR8(VCE->addr_lo, value);
        y++;
        value = RD8_PTR_Y(ZP->temp00, y);
        WR8(VCE->addr_hi, value);
        y++;
        value = RD8_PTR_Y(ZP->temp00, y);
        WR8(ZP->temp02, value);
        y++;
        value = RD8_PTR_Y(ZP->temp00, y);
        WR8(ZP->temp03, value);
        y++;
        WR8(ZP->hit_slot, y);
        entries = RD8_PTR(ZP->temp02);
        WR8(ZP->temp05, entries);
        do {
            y = 1;
            ninth = RD8_PTR_Y(ZP->temp02, y);
            WR8(ZP->temp04, ninth);
            do {
                y++;
                value = RD8_PTR_Y(ZP->temp02, y);
                blue = take_down((uint8_t)(value & 0x07), RD8(ZP->temp08));
                WR8(ZP->temp0b, blue);
                value = RD8_PTR_Y(ZP->temp02, y);
                green = take_down((uint8_t)(value & 0x38), RD8(ZP->temp09));
                WR8(ZP->temp0b, (uint8_t)(green | RD8(ZP->temp0b)));
                value = RD8_PTR_Y(ZP->temp02, y);
                carry = ASL8(ZP->temp04);
                a = (uint8_t)(((value >> 1) | (carry ? 0x80 : 0)) & 0xE0);
                red = take_down(a, RD8(ZP->temp0a));
                carry = (red & 0x80) != 0;
                WR8(VCE->data_lo, (uint8_t)((uint8_t)(red << 1) | RD8(ZP->temp0b)));
                WR8(VCE->data_hi, carry ? 1 : 0);
            } while (y < 9);
            sum = add_bytes(RD8(ZP->temp02), 9, false);
            WR8(ZP->temp02, sum.value);
            if (sum.carry) {
                INC8(ZP->temp03);
            }
            v = sum.overflow;
            v_set = true;
            left = DEC8(ZP->temp05);
        } while ((left & 0x80) == 0);
        y = RD8(ZP->hit_slot);
    }
}

/*
 * Kind 4: a palette effect that writes one colour and ends: the list number times 4 indexes a table of four bytes
 * (a VCE address, then the colour's low and high bytes), through the pointer at $9406 for a positive number, else from
 * the table of this version. The carry the shifts leave is added in, as the original's; the slot is given back.
 */
static Exit run_effect_colour(const EffectLayout *layout, uint8_t slot)
{
    Exit exit;
    Sum low, high;
    uint8_t list, a, y, value;
    bool carry;

    list = RD8(SLOTS->script[slot]);
    carry = (list & 0x40) != 0;
    a = (uint8_t)(list << 2);
    if ((list & 0x80) == 0) {
        low = add_bytes(a, rd8(COLOUR_POINTER), carry);
        WR8(ZP->temp00, low.value);
        high = add_bytes(0, rd8(COLOUR_POINTER + 1), low.carry);
        WR8(ZP->temp01, high.value);
    } else {
        low = add_bytes(a, (uint8_t)(layout->colour_table & 0xFF), carry);
        WR8(ZP->temp00, low.value);
        high = add_bytes(0, (uint8_t)(layout->colour_table >> 8), low.carry);
        WR8(ZP->temp01, high.value);
    }
    y = 0;
    value = RD8_PTR_Y(ZP->temp00, y);
    WR8(VCE->addr_lo, value);
    y++;
    value = RD8_PTR_Y(ZP->temp00, y);
    WR8(VCE->addr_hi, value);
    y++;
    value = RD8_PTR_Y(ZP->temp00, y);
    WR8(VCE->data_lo, value);
    y++;
    value = RD8_PTR_Y(ZP->temp00, y);
    WR8(VCE->data_hi, value);
    clear_effect_slot(slot);
    exit = effect_exit(value, value);
    exit.y = y;
    exit.y_kept = false;
    exit.c = high.carry;
    exit.c_kept = false;
    exit.v = high.overflow;
    exit.v_set = true;
    return exit;
}

/* The end of kind 5: the two palettes it saved are written back, the flag at zero page $3C cleared, the slot given
 * back. A is $80 at the end, N set; X, Y and C as they were (the block moves push and pull them). */
static Exit end_effect_flash(const EffectLayout *layout, uint8_t slot, uint8_t y_in)
{
    Exit exit;

    wr8(EFFECT_ACTIVE, 0);
    WR8(VCE->addr_hi, 0);
    WR8(VCE->addr_lo, 0);
    block_move_tia(layout->saved_background, ADDR_OF(VCE->data_lo), SAVED_BACKGROUND_BYTES, 0, slot, y_in);
    WR8(VCE->addr_hi, 1);
    WR8(VCE->addr_lo, 0x80);
    block_move_tia(layout->saved_sprites, ADDR_OF(VCE->data_lo), SAVED_SPRITE_BYTES, 0x80, slot, y_in);
    clear_effect_slot(slot);
    exit = effect_exit(0x80, 0x80);
    return exit;
}

/*
 * Kind 5: a palette effect that flashes. The first time (count 0) it saves the VCE's 256 background and 128 sprite
 * colours in the overlay's RAM ($8698, $8898); each round after it fills a 16 colour palette buffer with one colour
 * of its table at $85E7 (the slot's list number times 16 colours, the count times one) and writes the buffer over every
 * palette the list at $8667 names; after the sixteenth round it writes the saved colours back and gives the slot up.
 */
static Exit run_effect_flash(const EffectLayout *layout, uint8_t slot, uint8_t y_in)
{
    Exit exit;
    Sum sum;
    uint8_t count, a, y, x, value, low, high;

    wr8(EFFECT_ACTIVE, 1);
    count = RD8(SLOTS->f24[slot]);
    if (count == 0) {
        WR8(VCE->addr_hi, 0);
        WR8(VCE->addr_lo, 0);
        block_move_tai(ADDR_OF(VCE->data_lo), layout->saved_background, SAVED_BACKGROUND_BYTES, 0, slot, y_in);
        WR8(VCE->addr_hi, 1);
        WR8(VCE->addr_lo, 0x80);
        block_move_tai(ADDR_OF(VCE->data_lo), layout->saved_sprites, SAVED_SPRITE_BYTES, 0x80, slot, y_in);
    }
    INC8(SLOTS->f24[slot]);
    count = RD8(SLOTS->f24[slot]);
    if (count >= FLASH_ROUNDS) {
        exit = end_effect_flash(layout, slot, y_in);
        exit.c = true;
        exit.c_kept = false;
        return exit;
    }
    a = (uint8_t)(RD8(SLOTS->script[slot]) << 5);
    WR8(ZP->temp04, a);
    count = RD8(SLOTS->f24[slot]);
    sum = add_bytes((uint8_t)(count << 1), RD8(ZP->temp04), false);
    y = sum.value;
    WR8(ZP->temp02, rd8((uint16_t)(layout->flash_colours + y)));
    WR8(ZP->temp03, rd8((uint16_t)(layout->flash_colours + 1 + y)));
    y = 0;
    x = slot;
    for (;;) {
        value = rd8((uint16_t)(layout->flash_palettes + y));
        if (value == LIST_END) {
            exit = effect_exit(LIST_END, 0);
            exit.x = x;
            exit.x_kept = false;
            exit.y = y;
            exit.y_kept = false;
            exit.c = true;
            exit.c_kept = false;
            exit.v = sum.overflow;
            exit.v_set = true;
            return exit;
        }
        WR8(ZP->temp08, value);
        y++;
        value = rd8((uint16_t)(layout->flash_palettes + y));
        WR8(ZP->temp09, value);
        y++;
        for (x = 0; x < EFFECT_COLOUR_BYTES; x = (uint8_t)(x + 2)) {
            low = RD8(ZP->temp02);
            wr8((uint16_t)(EFFECT_COLOURS + x), low);
            high = RD8(ZP->temp03);
            wr8((uint16_t)(EFFECT_COLOURS + 1 + x), high);
        }
        low = RD8(ZP->temp08);
        WR8(VCE->addr_lo, low);
        high = RD8(ZP->temp09);
        WR8(VCE->addr_hi, high);
        block_move_tia(EFFECT_COLOURS, ADDR_OF(VCE->data_lo), EFFECT_COLOUR_BYTES, high, x, y);
    }
}

/* A handler entered by original code at an address this C does not hold: the registers as the dispatcher leaves them. */
static void effect_entry_shim(uint8_t high, uint8_t slot, uint8_t index, bool carry)
{
    cpu.a = high;
    cpu.x = slot;
    cpu.y = index;
    abi_nz(high);
    abi_flag(P_C, carry);
}

/*
 * Versions B, kinds 6 and 7, and their helper: the bank of Super RAM mapped at $A000 (MPR5) is the base bank plus $12,
 * or $30 when bit 0 of zIdleFlag is set, and the VCE's colour address is set to 0.
 */
static Exit select_saved_bank(void)
{
    Exit exit;
    Sum sum;
    uint8_t bank = BANK_PLAIN;

    if ((RD8(ZP->idle_flag) & 1) != 0) {
        bank = BANK_IDLE;
    }
    sum = add_bytes(bank, RD8(ZP->ram_base_bank), false);
    tam(MPR5, sum.value);
    WR8(VCE->addr_lo, 0);
    WR8(VCE->addr_hi, 0);
    exit = effect_exit(sum.value, sum.value);
    exit.c = sum.carry;
    exit.c_kept = false;
    exit.v = sum.overflow;
    exit.v_set = true;
    return exit;
}

/* Kind 6: all 512 colours of the VCE read into the Super RAM bank at $B000. */
static Exit run_vce_save(const EffectLayout *layout, uint8_t slot, uint8_t y)
{
    Exit exit;

    jsr_push(layout->save_ret);
    exit = select_saved_bank();
    jsr_pop();
    block_move_tai(ADDR_OF(VCE->data_lo), VCE_SAVE, VCE_SAVE_BYTES, exit.a, slot, y);
    return exit;
}

/* Kind 7: the 512 colours written back from the Super RAM bank at $B000. */
static Exit run_vce_restore(const EffectLayout *layout, uint8_t slot, uint8_t y)
{
    Exit exit;

    jsr_push(layout->restore_ret);
    exit = select_saved_bank();
    jsr_pop();
    block_move_tia(VCE_SAVE, ADDR_OF(VCE->data_lo), VCE_SAVE_BYTES, exit.a, slot, y);
    return exit;
}

/*
 * The dispatcher: the type less one, doubled, indexes the word table at $82DB (kind 1 first); the word goes through
 * zTemp00 and zTemp01 and the jump is made through them, with A the word's high byte, Y the index, C bit 7 of the type
 * less one, X the slot. A type past the table's last jumps into whatever the next bytes name.
 */
static Exit run_effect_dispatch(const EffectLayout *layout, uint8_t type, uint8_t slot)
{
    Exit exit = { 0 };
    uint8_t less, index, low, high, high_read, kind;
    uint16_t target;
    bool carry;

    less = (uint8_t)(type - 1);
    index = (uint8_t)(less << 1);
    carry = (less & 0x80) != 0;
    low = rd8((uint16_t)(layout->type_table + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(layout->type_table + 1 + index));
    WR8(ZP->temp01, high);
    low = RD8(ZP->temp00);
    high_read = RD8(ZP->temp01);
    target = (uint16_t)(low | (high_read << 8));
    for (kind = 0; kind < layout->kinds; kind++) {
        if (layout->handler[kind] == target) {
            break;
        }
    }
    if (kind == layout->kinds) {
        kind = NO_KIND;
    }
    switch (kind) {
    case 0: exit = run_effect_list(layout, slot); break;
    case 1: exit = run_effect_fade(layout, slot); break;
    case 2: exit = run_effect_shade(slot); break;
    case 3: exit = run_effect_colour(layout, slot); break;
    case 4: exit = run_effect_flash(layout, slot, index); break;
    case 5: exit = run_vce_save(layout, slot, index); break;
    case 6: exit = run_vce_restore(layout, slot, index); break;
    default:
        effect_entry_shim(high, slot, index, carry);
        run_routine(target);
        exit.from_cpu = true;
        return exit;
    }
    if (exit.a_kept) {
        exit.a = high;
        exit.a_kept = false;
    }
    if (exit.x_kept) {
        exit.x = slot;
        exit.x_kept = false;
    }
    if (exit.y_kept) {
        exit.y = index;
        exit.y_kept = false;
    }
    if (exit.c_kept) {
        exit.c = carry;
        exit.c_kept = false;
    }
    return exit;
}

/* The end of the loop over the slots when the last handler ran as original code: X $FF from the DEX. */
static void effect_loop_end_shim(bool a_loaded)
{
    if (a_loaded) {
        cpu.a = 0;
    }
    cpu.x = 0xFF;
    abi_nz(0xFF);
}

/*
 * One frame of the palette effects: while the game is paused (bit 0 of zPaused) only slots 2 and $0B are run;
 * otherwise, if slot 3 holds kind 5 only that runs, with X 3; else every slot from $0B down to 0 that has a kind is
 * run through the dispatcher, the slot kept in zRichterObject's byte across each.
 */
static Exit run_effect_step(const EffectLayout *layout, uint8_t y_in)
{
    Exit exit = { 0 };
    Exit last;
    uint8_t type, slot;
    bool have_last = false;
    uint8_t last_slot = 0;
    bool y_set = false, c_set = false, v_set = false, v = false, c = false;
    uint8_t y = 0, a = 0;

    if ((RD8(ZP->paused) & 1) != 0) {
        jsr_push(layout->step_paused_ret);
        type = RD8(SLOTS->kind[2]);
        if (type != 0) {
            exit = run_effect_dispatch(layout, type, 2);
            if (exit.from_cpu) {
                jsr_pop();
                return exit;
            }
        }
        jsr_pop();
        type = RD8(SLOTS->kind[0x0B]);
        if (type == 0) {
            exit = effect_exit(0, 0);
            exit.x = 0x0B;
            exit.x_kept = false;
            return exit;
        }
        return run_effect_dispatch(layout, type, 0x0B);
    }
    type = RD8(SLOTS->kind[3]);
    c = type >= 5;
    c_set = true;
    if (type == 5) {
        exit = run_effect_flash(layout, 3, y_in);
        if (exit.x_kept) {
            exit.x = 3;
            exit.x_kept = false;
        }
        return exit;
    }
    for (slot = 0x0B; (int8_t)slot >= 0; slot--) {
        type = RD8(SLOTS->kind[slot]);
        a = type;
        if (type != 0) {
            WR8(ZP->richter_object, slot);
            jsr_push(layout->step_loop_ret);
            last = run_effect_dispatch(layout, type, slot);
            jsr_pop();
            if (last.from_cpu) {
                have_last = true;
                last_slot = slot;
                exit = last;
            } else {
                have_last = false;
                a = last.a;
                y = last.y;
                y_set = true;
                c = last.c;
                c_set = true;
                if (last.v_set) {
                    v = last.v;
                    v_set = true;
                }
            }
            slot = RD8(ZP->richter_object);
        }
    }
    if (have_last) {
        effect_loop_end_shim(last_slot != 0);
        exit.from_cpu = true;
        return exit;
    }
    exit.a = a;
    exit.x = 0xFF;
    exit.nz = 0xFF;
    exit.y = y;
    exit.y_kept = !y_set;
    exit.c = c;
    exit.c_kept = !c_set;
    exit.v = v;
    exit.v_set = v_set;
    return exit;
}


static const EffectLayout LAYOUT_0441 = {
    0x8998, 0x89BC, 0x89BC, 0x832D,
    { 0x82E5, 0x835E, 0x8407, 0x84FB, 0x8535 }, 5,
    0x85E7, 0x8667, 0x8698, 0x8898,
    0x0000, 0x0000,
    0x82DB, 0x82A6, 0x82C5
};

/*
 * @routine 0441:29F EffectStep0441
 * @run 6C:829F
 * @unreachable 6C:829F-6C:82CA no read the game makes loads disc sector 0441, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $829F runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
 * otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
 * HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
 * 5 at slot 3 is entered with X 3.
 * WAITS: no.
 * Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
 * left; C, Y and V as the last handler left them.
 */
Exit EffectStep0441(uint8_t y)
{
    return run_effect_step(&LAYOUT_0441, y);
}

void abi_EffectStep0441(void)
{
    exit_shim(EffectStep0441(cpu.y));
}

/*
 * @routine 0441:2CB EffectDispatch0441
 * @run 6C:82CB
 * @stop 82DB
 * @unreachable 6C:82CB-6C:82DA no read the game makes loads disc sector 0441, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82CB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82DB holds one address per
 * kind, kind 1 first.
 * HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
 * with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
 * WAITS: no.
 * Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
 */
Exit EffectDispatch0441(uint8_t type, uint8_t slot)
{
    return run_effect_dispatch(&LAYOUT_0441, type, slot);
}

void abi_EffectDispatch0441(void)
{
    exit_shim(EffectDispatch0441(cpu.a, cpu.x));
}

/*
 * @routine 0441:2E5 EffectList0441
 * @run 6C:82E5
 * @unreachable 6C:82E5-6C:8344 no read the game makes loads disc sector 0441, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82E5 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
 * list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
 * again, and when the rounds the list names have been played it gives the slot back.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
 * $8998); each palette loaded by the packed palette loader at $84BB, whose bytes are LoadPackedPaletteC's.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
 * Z and C from comparing it with the rounds played, Y 1; X kept.
 */
Exit EffectList0441(uint8_t slot)
{
    return run_effect_list(&LAYOUT_0441, slot);
}

void abi_EffectList0441(void)
{
    exit_shim(EffectList0441(cpu.x));
}

/*
 * @routine 0441:345 ClearEffectSlot0441
 * @run 6C:8345
 * @unreachable 6C:8345-6C:835D no read the game makes loads disc sector 0441, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8345 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
 * HOW: Eight STZ of the slot's columns, the kind first.
 * WAITS: no.
 * Entry: X the slot. Exit: A, X, Y and the flags kept.
 */
void ClearEffectSlot0441(uint8_t slot)
{
    clear_effect_slot(slot);
}

void abi_ClearEffectSlot0441(void)
{
    ClearEffectSlot0441(cpu.x);
}

/*
 * @routine 0441:35E EffectFade0441
 * @run 6C:835E
 * @unreachable 6C:835E-6C:8406 no read the game makes loads disc sector 0441, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $835E runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
 * the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
 * every colour of them by one step in each of its three components, writing them back to the VCE.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
 * table at $89BC; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
 * $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
 */
Exit EffectFade0441(uint8_t slot)
{
    return run_effect_fade(&LAYOUT_0441, slot);
}

void abi_EffectFade0441(void)
{
    exit_shim(EffectFade0441(cpu.x));
}

/*
 * @routine 0441:407 EffectShade0441
 * @run 6C:8407
 * @stop 84BB
 * @unreachable 6C:8407-6C:84BA no read the game makes loads disc sector 0441, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8407 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 3: a palette effect that darkens palettes by an amount that grows each round: once its timer has
 * run down it counts a round (the slot is given back after the eighth) and writes the list's palettes to the VCE, each
 * colour's blue, green and red parts taken down by 7 less the count, by that times 8 and times 32, not below zero.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9404; zTemp08 to zTemp0A the three amounts;
 * the blocks (eight colours, their ninth bits first) read through zTemp02.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, Y
 * the list's place, V from the last block's pointer add (else kept).
 */
Exit EffectShade0441(uint8_t slot)
{
    return run_effect_shade(slot);
}

void abi_EffectShade0441(void)
{
    exit_shim(EffectShade0441(cpu.x));
}

/*
 * @routine 0441:4FB EffectColour0441
 * @run 6C:84FB
 * @unreachable 6C:84FB-6C:8534 no read the game makes loads disc sector 0441, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $84FB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
 * colour's two bytes) picked by the list number, the slot then given back.
 * HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
 * or for a negative number the table at $89BC; the four bytes go to the VCE ports.
 * WAITS: no.
 * Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
 * kept.
 */
Exit EffectColour0441(uint8_t slot)
{
    return run_effect_colour(&LAYOUT_0441, slot);
}

void abi_EffectColour0441(void)
{
    exit_shim(EffectColour0441(cpu.x));
}

/*
 * @routine 0441:535 EffectFlash0441
 * @run 6C:8535
 * @stop 05C0
 * @unreachable 6C:8535-6C:85BF no read the game makes loads disc sector 0441, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8535 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
 * in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
 * palette the list at $8667 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
 * HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $8698, $8898 and $3C60;
 * the colour table at $85E7 indexed by the list number times 32 and the count times 2.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
 * no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
 * with C set.
 */
Exit EffectFlash0441(uint8_t slot, uint8_t y)
{
    return run_effect_flash(&LAYOUT_0441, slot, y);
}

void abi_EffectFlash0441(void)
{
    exit_shim(EffectFlash0441(cpu.x, cpu.y));
}

/*
 * @routine 0441:5C0 EffectFlashEnd0441
 * @run 6C:85C0
 * @unreachable 6C:85C0-6C:85E6 no read the game makes loads disc sector 0441, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $85C0 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
 * the slot given back.
 * HOW: Two TIA from $8698 (256 colours) and $8898 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
 * WAITS: no.
 * Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
 */
Exit EffectFlashEnd0441(uint8_t slot, uint8_t y)
{
    return end_effect_flash(&LAYOUT_0441, slot, y);
}

void abi_EffectFlashEnd0441(void)
{
    exit_shim(EffectFlashEnd0441(cpu.x, cpu.y));
}

static const EffectLayout LAYOUT_0651 = {
    0x8998, 0x89AC, 0x89AC, 0x832D,
    { 0x82E5, 0x835E, 0x8407, 0x84FB, 0x8535 }, 5,
    0x85E7, 0x8667, 0x8698, 0x8898,
    0x0000, 0x0000,
    0x82DB, 0x82A6, 0x82C5
};

/*
 * @routine 0651:29F EffectStep0651
 * @run 6C:829F
 * @unreachable 6C:829F-6C:82CA no read the game makes loads disc sector 0651, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $829F runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
 * otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
 * HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
 * 5 at slot 3 is entered with X 3.
 * WAITS: no.
 * Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
 * left; C, Y and V as the last handler left them.
 */
Exit EffectStep0651(uint8_t y)
{
    return run_effect_step(&LAYOUT_0651, y);
}

void abi_EffectStep0651(void)
{
    exit_shim(EffectStep0651(cpu.y));
}

/*
 * @routine 0651:2CB EffectDispatch0651
 * @run 6C:82CB
 * @stop 82DB
 * @unreachable 6C:82CB-6C:82DA no read the game makes loads disc sector 0651, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82CB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82DB holds one address per
 * kind, kind 1 first.
 * HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
 * with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
 * WAITS: no.
 * Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
 */
Exit EffectDispatch0651(uint8_t type, uint8_t slot)
{
    return run_effect_dispatch(&LAYOUT_0651, type, slot);
}

void abi_EffectDispatch0651(void)
{
    exit_shim(EffectDispatch0651(cpu.a, cpu.x));
}

/*
 * @routine 0651:2E5 EffectList0651
 * @run 6C:82E5
 * @unreachable 6C:82E5-6C:8344 no read the game makes loads disc sector 0651, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82E5 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
 * list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
 * again, and when the rounds the list names have been played it gives the slot back.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
 * $8998); each palette loaded by the packed palette loader at $84BB, whose bytes are LoadPackedPaletteC's.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
 * Z and C from comparing it with the rounds played, Y 1; X kept.
 */
Exit EffectList0651(uint8_t slot)
{
    return run_effect_list(&LAYOUT_0651, slot);
}

void abi_EffectList0651(void)
{
    exit_shim(EffectList0651(cpu.x));
}

/*
 * @routine 0651:345 ClearEffectSlot0651
 * @run 6C:8345
 * @unreachable 6C:8345-6C:835D no read the game makes loads disc sector 0651, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8345 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
 * HOW: Eight STZ of the slot's columns, the kind first.
 * WAITS: no.
 * Entry: X the slot. Exit: A, X, Y and the flags kept.
 */
void ClearEffectSlot0651(uint8_t slot)
{
    clear_effect_slot(slot);
}

void abi_ClearEffectSlot0651(void)
{
    ClearEffectSlot0651(cpu.x);
}

/*
 * @routine 0651:35E EffectFade0651
 * @run 6C:835E
 * @unreachable 6C:835E-6C:8406 no read the game makes loads disc sector 0651, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $835E runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
 * the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
 * every colour of them by one step in each of its three components, writing them back to the VCE.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
 * table at $89AC; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
 * $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
 */
Exit EffectFade0651(uint8_t slot)
{
    return run_effect_fade(&LAYOUT_0651, slot);
}

void abi_EffectFade0651(void)
{
    exit_shim(EffectFade0651(cpu.x));
}

/*
 * @routine 0651:407 EffectShade0651
 * @run 6C:8407
 * @stop 84BB
 * @unreachable 6C:8407-6C:84BA no read the game makes loads disc sector 0651, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8407 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 3: a palette effect that darkens palettes by an amount that grows each round: once its timer has
 * run down it counts a round (the slot is given back after the eighth) and writes the list's palettes to the VCE, each
 * colour's blue, green and red parts taken down by 7 less the count, by that times 8 and times 32, not below zero.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9404; zTemp08 to zTemp0A the three amounts;
 * the blocks (eight colours, their ninth bits first) read through zTemp02.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, Y
 * the list's place, V from the last block's pointer add (else kept).
 */
Exit EffectShade0651(uint8_t slot)
{
    return run_effect_shade(slot);
}

void abi_EffectShade0651(void)
{
    exit_shim(EffectShade0651(cpu.x));
}

/*
 * @routine 0651:4FB EffectColour0651
 * @run 6C:84FB
 * @unreachable 6C:84FB-6C:8534 no read the game makes loads disc sector 0651, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $84FB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
 * colour's two bytes) picked by the list number, the slot then given back.
 * HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
 * or for a negative number the table at $89AC; the four bytes go to the VCE ports.
 * WAITS: no.
 * Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
 * kept.
 */
Exit EffectColour0651(uint8_t slot)
{
    return run_effect_colour(&LAYOUT_0651, slot);
}

void abi_EffectColour0651(void)
{
    exit_shim(EffectColour0651(cpu.x));
}

/*
 * @routine 0651:535 EffectFlash0651
 * @run 6C:8535
 * @stop 05C0
 * @unreachable 6C:8535-6C:85BF no read the game makes loads disc sector 0651, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8535 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
 * in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
 * palette the list at $8667 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
 * HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $8698, $8898 and $3C60;
 * the colour table at $85E7 indexed by the list number times 32 and the count times 2.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
 * no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
 * with C set.
 */
Exit EffectFlash0651(uint8_t slot, uint8_t y)
{
    return run_effect_flash(&LAYOUT_0651, slot, y);
}

void abi_EffectFlash0651(void)
{
    exit_shim(EffectFlash0651(cpu.x, cpu.y));
}

/*
 * @routine 0651:5C0 EffectFlashEnd0651
 * @run 6C:85C0
 * @unreachable 6C:85C0-6C:85E6 no read the game makes loads disc sector 0651, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $85C0 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
 * the slot given back.
 * HOW: Two TIA from $8698 (256 colours) and $8898 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
 * WAITS: no.
 * Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
 */
Exit EffectFlashEnd0651(uint8_t slot, uint8_t y)
{
    return end_effect_flash(&LAYOUT_0651, slot, y);
}

void abi_EffectFlashEnd0651(void)
{
    exit_shim(EffectFlashEnd0651(cpu.x, cpu.y));
}

static const EffectLayout LAYOUT_1231 = {
    0x8998, 0x89B2, 0x89B2, 0x832D,
    { 0x82E5, 0x835E, 0x8407, 0x84FB, 0x8535 }, 5,
    0x85E7, 0x8667, 0x8698, 0x8898,
    0x0000, 0x0000,
    0x82DB, 0x82A6, 0x82C5
};

/*
 * @routine 1231:29F EffectStep1231
 * @run 6C:829F
 * @unreachable 6C:829F-6C:82CA no read the game makes loads disc sector 1231, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $829F runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
 * otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
 * HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
 * 5 at slot 3 is entered with X 3.
 * WAITS: no.
 * Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
 * left; C, Y and V as the last handler left them.
 */
Exit EffectStep1231(uint8_t y)
{
    return run_effect_step(&LAYOUT_1231, y);
}

void abi_EffectStep1231(void)
{
    exit_shim(EffectStep1231(cpu.y));
}

/*
 * @routine 1231:2CB EffectDispatch1231
 * @run 6C:82CB
 * @stop 82DB
 * @unreachable 6C:82CB-6C:82DA no read the game makes loads disc sector 1231, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82CB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82DB holds one address per
 * kind, kind 1 first.
 * HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
 * with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
 * WAITS: no.
 * Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
 */
Exit EffectDispatch1231(uint8_t type, uint8_t slot)
{
    return run_effect_dispatch(&LAYOUT_1231, type, slot);
}

void abi_EffectDispatch1231(void)
{
    exit_shim(EffectDispatch1231(cpu.a, cpu.x));
}

/*
 * @routine 1231:2E5 EffectList1231
 * @run 6C:82E5
 * @unreachable 6C:82E5-6C:8344 no read the game makes loads disc sector 1231, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82E5 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
 * list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
 * again, and when the rounds the list names have been played it gives the slot back.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
 * $8998); each palette loaded by the packed palette loader at $84BB, whose bytes are LoadPackedPaletteC's.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
 * Z and C from comparing it with the rounds played, Y 1; X kept.
 */
Exit EffectList1231(uint8_t slot)
{
    return run_effect_list(&LAYOUT_1231, slot);
}

void abi_EffectList1231(void)
{
    exit_shim(EffectList1231(cpu.x));
}

/*
 * @routine 1231:345 ClearEffectSlot1231
 * @run 6C:8345
 * @unreachable 6C:8345-6C:835D no read the game makes loads disc sector 1231, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8345 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
 * HOW: Eight STZ of the slot's columns, the kind first.
 * WAITS: no.
 * Entry: X the slot. Exit: A, X, Y and the flags kept.
 */
void ClearEffectSlot1231(uint8_t slot)
{
    clear_effect_slot(slot);
}

void abi_ClearEffectSlot1231(void)
{
    ClearEffectSlot1231(cpu.x);
}

/*
 * @routine 1231:35E EffectFade1231
 * @run 6C:835E
 * @unreachable 6C:835E-6C:8406 no read the game makes loads disc sector 1231, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $835E runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
 * the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
 * every colour of them by one step in each of its three components, writing them back to the VCE.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
 * table at $89B2; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
 * $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
 */
Exit EffectFade1231(uint8_t slot)
{
    return run_effect_fade(&LAYOUT_1231, slot);
}

void abi_EffectFade1231(void)
{
    exit_shim(EffectFade1231(cpu.x));
}

/*
 * @routine 1231:407 EffectShade1231
 * @run 6C:8407
 * @stop 84BB
 * @unreachable 6C:8407-6C:84BA no read the game makes loads disc sector 1231, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8407 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 3: a palette effect that darkens palettes by an amount that grows each round: once its timer has
 * run down it counts a round (the slot is given back after the eighth) and writes the list's palettes to the VCE, each
 * colour's blue, green and red parts taken down by 7 less the count, by that times 8 and times 32, not below zero.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9404; zTemp08 to zTemp0A the three amounts;
 * the blocks (eight colours, their ninth bits first) read through zTemp02.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, Y
 * the list's place, V from the last block's pointer add (else kept).
 */
Exit EffectShade1231(uint8_t slot)
{
    return run_effect_shade(slot);
}

void abi_EffectShade1231(void)
{
    exit_shim(EffectShade1231(cpu.x));
}

/*
 * @routine 1231:4FB EffectColour1231
 * @run 6C:84FB
 * @unreachable 6C:84FB-6C:8534 no read the game makes loads disc sector 1231, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $84FB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
 * colour's two bytes) picked by the list number, the slot then given back.
 * HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
 * or for a negative number the table at $89B2; the four bytes go to the VCE ports.
 * WAITS: no.
 * Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
 * kept.
 */
Exit EffectColour1231(uint8_t slot)
{
    return run_effect_colour(&LAYOUT_1231, slot);
}

void abi_EffectColour1231(void)
{
    exit_shim(EffectColour1231(cpu.x));
}

/*
 * @routine 1231:535 EffectFlash1231
 * @run 6C:8535
 * @stop 05C0
 * @unreachable 6C:8535-6C:85BF no read the game makes loads disc sector 1231, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8535 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
 * in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
 * palette the list at $8667 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
 * HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $8698, $8898 and $3C60;
 * the colour table at $85E7 indexed by the list number times 32 and the count times 2.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
 * no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
 * with C set.
 */
Exit EffectFlash1231(uint8_t slot, uint8_t y)
{
    return run_effect_flash(&LAYOUT_1231, slot, y);
}

void abi_EffectFlash1231(void)
{
    exit_shim(EffectFlash1231(cpu.x, cpu.y));
}

/*
 * @routine 1231:5C0 EffectFlashEnd1231
 * @run 6C:85C0
 * @unreachable 6C:85C0-6C:85E6 no read the game makes loads disc sector 1231, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $85C0 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
 * the slot given back.
 * HOW: Two TIA from $8698 (256 colours) and $8898 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
 * WAITS: no.
 * Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
 */
Exit EffectFlashEnd1231(uint8_t slot, uint8_t y)
{
    return end_effect_flash(&LAYOUT_1231, slot, y);
}

void abi_EffectFlashEnd1231(void)
{
    exit_shim(EffectFlashEnd1231(cpu.x, cpu.y));
}

static const EffectLayout LAYOUT_1A31 = {
    0x89C3, 0x89D9, 0x89D9, 0x8331,
    { 0x82E9, 0x8362, 0x840B, 0x84FF, 0x8560, 0x8539, 0x8543 }, 7,
    0x8612, 0x8692, 0x86C3, 0x88C3,
    0x853B, 0x8545,
    0x82DB, 0x82A6, 0x82C5
};

/*
 * @routine 1A31:29F EffectStep1A31
 * @run 6C:829F
 * @unreachable 6C:829F-6C:82CA no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $829F runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
 * otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
 * HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
 * 5 at slot 3 is entered with X 3.
 * WAITS: no.
 * Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
 * left; C, Y and V as the last handler left them.
 */
Exit EffectStep1A31(uint8_t y)
{
    return run_effect_step(&LAYOUT_1A31, y);
}

void abi_EffectStep1A31(void)
{
    exit_shim(EffectStep1A31(cpu.y));
}

/*
 * @routine 1A31:2CB EffectDispatch1A31
 * @run 6C:82CB
 * @stop 82DB
 * @unreachable 6C:82CB-6C:82DA no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82CB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82DB holds one address per
 * kind, kind 1 first.
 * HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
 * with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
 * WAITS: no.
 * Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
 */
Exit EffectDispatch1A31(uint8_t type, uint8_t slot)
{
    return run_effect_dispatch(&LAYOUT_1A31, type, slot);
}

void abi_EffectDispatch1A31(void)
{
    exit_shim(EffectDispatch1A31(cpu.a, cpu.x));
}

/*
 * @routine 1A31:2E9 EffectList1A31
 * @run 6C:82E9
 * @unreachable 6C:82E9-6C:8348 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82E9 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
 * list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
 * again, and when the rounds the list names have been played it gives the slot back.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
 * $89C3); each palette loaded by the packed palette loader at $84BF, whose bytes are LoadPackedPaletteC's.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
 * Z and C from comparing it with the rounds played, Y 1; X kept.
 */
Exit EffectList1A31(uint8_t slot)
{
    return run_effect_list(&LAYOUT_1A31, slot);
}

void abi_EffectList1A31(void)
{
    exit_shim(EffectList1A31(cpu.x));
}

/*
 * @routine 1A31:349 ClearEffectSlot1A31
 * @run 6C:8349
 * @unreachable 6C:8349-6C:8361 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8349 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
 * HOW: Eight STZ of the slot's columns, the kind first.
 * WAITS: no.
 * Entry: X the slot. Exit: A, X, Y and the flags kept.
 */
void ClearEffectSlot1A31(uint8_t slot)
{
    clear_effect_slot(slot);
}

void abi_ClearEffectSlot1A31(void)
{
    ClearEffectSlot1A31(cpu.x);
}

/*
 * @routine 1A31:362 EffectFade1A31
 * @run 6C:8362
 * @unreachable 6C:8362-6C:840A no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8362 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
 * the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
 * every colour of them by one step in each of its three components, writing them back to the VCE.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
 * table at $89D9; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
 * $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
 */
Exit EffectFade1A31(uint8_t slot)
{
    return run_effect_fade(&LAYOUT_1A31, slot);
}

void abi_EffectFade1A31(void)
{
    exit_shim(EffectFade1A31(cpu.x));
}

/*
 * @routine 1A31:40B EffectShade1A31
 * @run 6C:840B
 * @stop 84BF
 * @unreachable 6C:840B-6C:84BE no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $840B runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 3: a palette effect that darkens palettes by an amount that grows each round: once its timer has
 * run down it counts a round (the slot is given back after the eighth) and writes the list's palettes to the VCE, each
 * colour's blue, green and red parts taken down by 7 less the count, by that times 8 and times 32, not below zero.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9404; zTemp08 to zTemp0A the three amounts;
 * the blocks (eight colours, their ninth bits first) read through zTemp02.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, Y
 * the list's place, V from the last block's pointer add (else kept).
 */
Exit EffectShade1A31(uint8_t slot)
{
    return run_effect_shade(slot);
}

void abi_EffectShade1A31(void)
{
    exit_shim(EffectShade1A31(cpu.x));
}

/*
 * @routine 1A31:4FF EffectColour1A31
 * @run 6C:84FF
 * @unreachable 6C:84FF-6C:8538 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $84FF runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
 * colour's two bytes) picked by the list number, the slot then given back.
 * HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
 * or for a negative number the table at $89D9; the four bytes go to the VCE ports.
 * WAITS: no.
 * Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
 * kept.
 */
Exit EffectColour1A31(uint8_t slot)
{
    return run_effect_colour(&LAYOUT_1A31, slot);
}

void abi_EffectColour1A31(void)
{
    exit_shim(EffectColour1A31(cpu.x));
}

/*
 * @routine 1A31:539 EffectVceSave1A31
 * @run 6C:8539
 * @unreachable 6C:8539-6C:8542 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8539 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 6: all 512 colours of the VCE saved in the Super RAM bank mapped at $A000, at $B000.
 * HOW: SelectSavedBank, then a TAI of $400 bytes from the VCE's data port.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
 */
Exit EffectVceSave1A31(uint8_t slot, uint8_t y)
{
    return run_vce_save(&LAYOUT_1A31, slot, y);
}

void abi_EffectVceSave1A31(void)
{
    exit_shim(EffectVceSave1A31(cpu.x, cpu.y));
}

/*
 * @routine 1A31:543 EffectVceRestore1A31
 * @run 6C:8543
 * @unreachable 6C:8543-6C:854C no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8543 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 7: the 512 colours written back to the VCE from the Super RAM bank mapped at $A000, at $B000.
 * HOW: SelectSavedBank, then a TIA of $400 bytes to the VCE's data port.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
 */
Exit EffectVceRestore1A31(uint8_t slot, uint8_t y)
{
    return run_vce_restore(&LAYOUT_1A31, slot, y);
}

void abi_EffectVceRestore1A31(void)
{
    exit_shim(EffectVceRestore1A31(cpu.x, cpu.y));
}

/*
 * @routine 1A31:54D SelectSavedBank1A31
 * @run 6C:854D
 * @unreachable 6C:854D-6C:855F no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $854D runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Maps the Super RAM bank of the saved palette at $A000 and sets the VCE's colour address to 0.
 * HOW: The bank is the base bank plus $12, or plus $30 when bit 0 of zIdleFlag is set; TAM for MPR5.
 * WAITS: no.
 * Entry: nothing. Exit: A the bank, N and Z from it, C and V from the add; X and Y kept.
 */
Exit SelectSavedBank1A31(void)
{
    return select_saved_bank();
}

void abi_SelectSavedBank1A31(void)
{
    exit_shim(SelectSavedBank1A31());
}

/*
 * @routine 1A31:560 EffectFlash1A31
 * @run 6C:8560
 * @stop 05EB
 * @unreachable 6C:8560-6C:85EA no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8560 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
 * in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
 * palette the list at $8692 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
 * HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86C3, $88C3 and $3C60;
 * the colour table at $8612 indexed by the list number times 32 and the count times 2.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
 * no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
 * with C set.
 */
Exit EffectFlash1A31(uint8_t slot, uint8_t y)
{
    return run_effect_flash(&LAYOUT_1A31, slot, y);
}

void abi_EffectFlash1A31(void)
{
    exit_shim(EffectFlash1A31(cpu.x, cpu.y));
}

/*
 * @routine 1A31:5EB EffectFlashEnd1A31
 * @run 6C:85EB
 * @unreachable 6C:85EB-6C:8611 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $85EB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
 * the slot given back.
 * HOW: Two TIA from $86C3 (256 colours) and $88C3 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
 * WAITS: no.
 * Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
 */
Exit EffectFlashEnd1A31(uint8_t slot, uint8_t y)
{
    return end_effect_flash(&LAYOUT_1A31, slot, y);
}

void abi_EffectFlashEnd1A31(void)
{
    exit_shim(EffectFlashEnd1A31(cpu.x, cpu.y));
}

static const EffectLayout LAYOUT_1B91 = {
    0x89C3, 0x89CF, 0x89CF, 0x8331,
    { 0x82E9, 0x8362, 0x840B, 0x84FF, 0x8560, 0x8539, 0x8543 }, 7,
    0x8612, 0x8692, 0x86C3, 0x88C3,
    0x853B, 0x8545,
    0x82DB, 0x82A6, 0x82C5
};

/*
 * @routine 1B91:29F EffectStep1B91
 * @run 6C:829F
 * @unreachable 6C:829F-6C:82CA no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $829F runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
 * otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
 * HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
 * 5 at slot 3 is entered with X 3.
 * WAITS: no.
 * Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
 * left; C, Y and V as the last handler left them.
 */
Exit EffectStep1B91(uint8_t y)
{
    return run_effect_step(&LAYOUT_1B91, y);
}

void abi_EffectStep1B91(void)
{
    exit_shim(EffectStep1B91(cpu.y));
}

/*
 * @routine 1B91:2CB EffectDispatch1B91
 * @run 6C:82CB
 * @stop 82DB
 * @unreachable 6C:82CB-6C:82DA no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82CB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82DB holds one address per
 * kind, kind 1 first.
 * HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
 * with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
 * WAITS: no.
 * Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
 */
Exit EffectDispatch1B91(uint8_t type, uint8_t slot)
{
    return run_effect_dispatch(&LAYOUT_1B91, type, slot);
}

void abi_EffectDispatch1B91(void)
{
    exit_shim(EffectDispatch1B91(cpu.a, cpu.x));
}

/*
 * @routine 1B91:2E9 EffectList1B91
 * @run 6C:82E9
 * @unreachable 6C:82E9-6C:8348 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82E9 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
 * list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
 * again, and when the rounds the list names have been played it gives the slot back.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
 * $89C3); each palette loaded by the packed palette loader at $84BF, whose bytes are LoadPackedPaletteC's.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
 * Z and C from comparing it with the rounds played, Y 1; X kept.
 */
Exit EffectList1B91(uint8_t slot)
{
    return run_effect_list(&LAYOUT_1B91, slot);
}

void abi_EffectList1B91(void)
{
    exit_shim(EffectList1B91(cpu.x));
}

/*
 * @routine 1B91:349 ClearEffectSlot1B91
 * @run 6C:8349
 * @unreachable 6C:8349-6C:8361 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8349 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
 * HOW: Eight STZ of the slot's columns, the kind first.
 * WAITS: no.
 * Entry: X the slot. Exit: A, X, Y and the flags kept.
 */
void ClearEffectSlot1B91(uint8_t slot)
{
    clear_effect_slot(slot);
}

void abi_ClearEffectSlot1B91(void)
{
    ClearEffectSlot1B91(cpu.x);
}

/*
 * @routine 1B91:362 EffectFade1B91
 * @run 6C:8362
 * @unreachable 6C:8362-6C:840A no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8362 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
 * the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
 * every colour of them by one step in each of its three components, writing them back to the VCE.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
 * table at $89CF; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
 * $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
 */
Exit EffectFade1B91(uint8_t slot)
{
    return run_effect_fade(&LAYOUT_1B91, slot);
}

void abi_EffectFade1B91(void)
{
    exit_shim(EffectFade1B91(cpu.x));
}

/*
 * @routine 1B91:40B EffectShade1B91
 * @run 6C:840B
 * @stop 84BF
 * @unreachable 6C:840B-6C:84BE no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $840B runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 3: a palette effect that darkens palettes by an amount that grows each round: once its timer has
 * run down it counts a round (the slot is given back after the eighth) and writes the list's palettes to the VCE, each
 * colour's blue, green and red parts taken down by 7 less the count, by that times 8 and times 32, not below zero.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9404; zTemp08 to zTemp0A the three amounts;
 * the blocks (eight colours, their ninth bits first) read through zTemp02.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, Y
 * the list's place, V from the last block's pointer add (else kept).
 */
Exit EffectShade1B91(uint8_t slot)
{
    return run_effect_shade(slot);
}

void abi_EffectShade1B91(void)
{
    exit_shim(EffectShade1B91(cpu.x));
}

/*
 * @routine 1B91:4FF EffectColour1B91
 * @run 6C:84FF
 * @unreachable 6C:84FF-6C:8538 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $84FF runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
 * colour's two bytes) picked by the list number, the slot then given back.
 * HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
 * or for a negative number the table at $89CF; the four bytes go to the VCE ports.
 * WAITS: no.
 * Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
 * kept.
 */
Exit EffectColour1B91(uint8_t slot)
{
    return run_effect_colour(&LAYOUT_1B91, slot);
}

void abi_EffectColour1B91(void)
{
    exit_shim(EffectColour1B91(cpu.x));
}

/*
 * @routine 1B91:539 EffectVceSave1B91
 * @run 6C:8539
 * @unreachable 6C:8539-6C:8542 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8539 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 6: all 512 colours of the VCE saved in the Super RAM bank mapped at $A000, at $B000.
 * HOW: SelectSavedBank, then a TAI of $400 bytes from the VCE's data port.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
 */
Exit EffectVceSave1B91(uint8_t slot, uint8_t y)
{
    return run_vce_save(&LAYOUT_1B91, slot, y);
}

void abi_EffectVceSave1B91(void)
{
    exit_shim(EffectVceSave1B91(cpu.x, cpu.y));
}

/*
 * @routine 1B91:543 EffectVceRestore1B91
 * @run 6C:8543
 * @unreachable 6C:8543-6C:854C no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8543 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 7: the 512 colours written back to the VCE from the Super RAM bank mapped at $A000, at $B000.
 * HOW: SelectSavedBank, then a TIA of $400 bytes to the VCE's data port.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
 */
Exit EffectVceRestore1B91(uint8_t slot, uint8_t y)
{
    return run_vce_restore(&LAYOUT_1B91, slot, y);
}

void abi_EffectVceRestore1B91(void)
{
    exit_shim(EffectVceRestore1B91(cpu.x, cpu.y));
}

/*
 * @routine 1B91:54D SelectSavedBank1B91
 * @run 6C:854D
 * @unreachable 6C:854D-6C:855F no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $854D runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Maps the Super RAM bank of the saved palette at $A000 and sets the VCE's colour address to 0.
 * HOW: The bank is the base bank plus $12, or plus $30 when bit 0 of zIdleFlag is set; TAM for MPR5.
 * WAITS: no.
 * Entry: nothing. Exit: A the bank, N and Z from it, C and V from the add; X and Y kept.
 */
Exit SelectSavedBank1B91(void)
{
    return select_saved_bank();
}

void abi_SelectSavedBank1B91(void)
{
    exit_shim(SelectSavedBank1B91());
}

/*
 * @routine 1B91:560 EffectFlash1B91
 * @run 6C:8560
 * @stop 05EB
 * @unreachable 6C:8560-6C:85EA no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8560 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
 * in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
 * palette the list at $8692 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
 * HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86C3, $88C3 and $3C60;
 * the colour table at $8612 indexed by the list number times 32 and the count times 2.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
 * no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
 * with C set.
 */
Exit EffectFlash1B91(uint8_t slot, uint8_t y)
{
    return run_effect_flash(&LAYOUT_1B91, slot, y);
}

void abi_EffectFlash1B91(void)
{
    exit_shim(EffectFlash1B91(cpu.x, cpu.y));
}

/*
 * @routine 1B91:5EB EffectFlashEnd1B91
 * @run 6C:85EB
 * @unreachable 6C:85EB-6C:8611 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $85EB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
 * the slot given back.
 * HOW: Two TIA from $86C3 (256 colours) and $88C3 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
 * WAITS: no.
 * Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
 */
Exit EffectFlashEnd1B91(uint8_t slot, uint8_t y)
{
    return end_effect_flash(&LAYOUT_1B91, slot, y);
}

void abi_EffectFlashEnd1B91(void)
{
    exit_shim(EffectFlashEnd1B91(cpu.x, cpu.y));
}

static const EffectLayout LAYOUT_25F7 = {
    0x89C3, 0x9410, 0x89C3, 0x8331,
    { 0x82E9, 0x8362, 0x840B, 0x84FF, 0x8560, 0x8539, 0x8543 }, 7,
    0x8612, 0x8692, 0x86C3, 0x88C3,
    0x853B, 0x8545,
    0x82DB, 0x82A6, 0x82C5
};

/*
 * @routine 25F7:29F EffectStep25F7
 * @run 6C:829F
 * @unreachable 6C:829F-6C:82CA no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $829F runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
 * otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
 * HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
 * 5 at slot 3 is entered with X 3.
 * WAITS: no.
 * Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
 * left; C, Y and V as the last handler left them.
 */
Exit EffectStep25F7(uint8_t y)
{
    return run_effect_step(&LAYOUT_25F7, y);
}

void abi_EffectStep25F7(void)
{
    exit_shim(EffectStep25F7(cpu.y));
}

/*
 * @routine 25F7:2CB EffectDispatch25F7
 * @run 6C:82CB
 * @stop 82DB
 * @unreachable 6C:82CB-6C:82DA no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82CB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82DB holds one address per
 * kind, kind 1 first.
 * HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
 * with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
 * WAITS: no.
 * Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
 */
Exit EffectDispatch25F7(uint8_t type, uint8_t slot)
{
    return run_effect_dispatch(&LAYOUT_25F7, type, slot);
}

void abi_EffectDispatch25F7(void)
{
    exit_shim(EffectDispatch25F7(cpu.a, cpu.x));
}

/*
 * @routine 25F7:2E9 EffectList25F7
 * @run 6C:82E9
 * @unreachable 6C:82E9-6C:8348 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82E9 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
 * list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
 * again, and when the rounds the list names have been played it gives the slot back.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
 * $89C3); each palette loaded by the packed palette loader at $84BF, whose bytes are LoadPackedPaletteC's.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
 * Z and C from comparing it with the rounds played, Y 1; X kept.
 */
Exit EffectList25F7(uint8_t slot)
{
    return run_effect_list(&LAYOUT_25F7, slot);
}

void abi_EffectList25F7(void)
{
    exit_shim(EffectList25F7(cpu.x));
}

/*
 * @routine 25F7:349 ClearEffectSlot25F7
 * @run 6C:8349
 * @unreachable 6C:8349-6C:8361 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8349 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
 * HOW: Eight STZ of the slot's columns, the kind first.
 * WAITS: no.
 * Entry: X the slot. Exit: A, X, Y and the flags kept.
 */
void ClearEffectSlot25F7(uint8_t slot)
{
    clear_effect_slot(slot);
}

void abi_ClearEffectSlot25F7(void)
{
    ClearEffectSlot25F7(cpu.x);
}

/*
 * @routine 25F7:362 EffectFade25F7
 * @run 6C:8362
 * @unreachable 6C:8362-6C:840A no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8362 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
 * the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
 * every colour of them by one step in each of its three components, writing them back to the VCE.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
 * table at $9410; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
 * $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
 */
Exit EffectFade25F7(uint8_t slot)
{
    return run_effect_fade(&LAYOUT_25F7, slot);
}

void abi_EffectFade25F7(void)
{
    exit_shim(EffectFade25F7(cpu.x));
}

/*
 * @routine 25F7:40B EffectShade25F7
 * @run 6C:840B
 * @stop 84BF
 * @unreachable 6C:840B-6C:84BE no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $840B runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 3: a palette effect that darkens palettes by an amount that grows each round: once its timer has
 * run down it counts a round (the slot is given back after the eighth) and writes the list's palettes to the VCE, each
 * colour's blue, green and red parts taken down by 7 less the count, by that times 8 and times 32, not below zero.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9404; zTemp08 to zTemp0A the three amounts;
 * the blocks (eight colours, their ninth bits first) read through zTemp02.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, Y
 * the list's place, V from the last block's pointer add (else kept).
 */
Exit EffectShade25F7(uint8_t slot)
{
    return run_effect_shade(slot);
}

void abi_EffectShade25F7(void)
{
    exit_shim(EffectShade25F7(cpu.x));
}

/*
 * @routine 25F7:4FF EffectColour25F7
 * @run 6C:84FF
 * @unreachable 6C:84FF-6C:8538 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $84FF runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
 * colour's two bytes) picked by the list number, the slot then given back.
 * HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
 * or for a negative number the table at $89C3; the four bytes go to the VCE ports.
 * WAITS: no.
 * Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
 * kept.
 */
Exit EffectColour25F7(uint8_t slot)
{
    return run_effect_colour(&LAYOUT_25F7, slot);
}

void abi_EffectColour25F7(void)
{
    exit_shim(EffectColour25F7(cpu.x));
}

/*
 * @routine 25F7:539 EffectVceSave25F7
 * @run 6C:8539
 * @unreachable 6C:8539-6C:8542 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8539 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 6: all 512 colours of the VCE saved in the Super RAM bank mapped at $A000, at $B000.
 * HOW: SelectSavedBank, then a TAI of $400 bytes from the VCE's data port.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
 */
Exit EffectVceSave25F7(uint8_t slot, uint8_t y)
{
    return run_vce_save(&LAYOUT_25F7, slot, y);
}

void abi_EffectVceSave25F7(void)
{
    exit_shim(EffectVceSave25F7(cpu.x, cpu.y));
}

/*
 * @routine 25F7:543 EffectVceRestore25F7
 * @run 6C:8543
 * @unreachable 6C:8543-6C:854C no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8543 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 7: the 512 colours written back to the VCE from the Super RAM bank mapped at $A000, at $B000.
 * HOW: SelectSavedBank, then a TIA of $400 bytes to the VCE's data port.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
 */
Exit EffectVceRestore25F7(uint8_t slot, uint8_t y)
{
    return run_vce_restore(&LAYOUT_25F7, slot, y);
}

void abi_EffectVceRestore25F7(void)
{
    exit_shim(EffectVceRestore25F7(cpu.x, cpu.y));
}

/*
 * @routine 25F7:54D SelectSavedBank25F7
 * @run 6C:854D
 * @unreachable 6C:854D-6C:855F no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $854D runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Maps the Super RAM bank of the saved palette at $A000 and sets the VCE's colour address to 0.
 * HOW: The bank is the base bank plus $12, or plus $30 when bit 0 of zIdleFlag is set; TAM for MPR5.
 * WAITS: no.
 * Entry: nothing. Exit: A the bank, N and Z from it, C and V from the add; X and Y kept.
 */
Exit SelectSavedBank25F7(void)
{
    return select_saved_bank();
}

void abi_SelectSavedBank25F7(void)
{
    exit_shim(SelectSavedBank25F7());
}

/*
 * @routine 25F7:560 EffectFlash25F7
 * @run 6C:8560
 * @stop 05EB
 * @unreachable 6C:8560-6C:85EA no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8560 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
 * in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
 * palette the list at $8692 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
 * HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86C3, $88C3 and $3C60;
 * the colour table at $8612 indexed by the list number times 32 and the count times 2.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
 * no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
 * with C set.
 */
Exit EffectFlash25F7(uint8_t slot, uint8_t y)
{
    return run_effect_flash(&LAYOUT_25F7, slot, y);
}

void abi_EffectFlash25F7(void)
{
    exit_shim(EffectFlash25F7(cpu.x, cpu.y));
}

/*
 * @routine 25F7:5EB EffectFlashEnd25F7
 * @run 6C:85EB
 * @unreachable 6C:85EB-6C:8611 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $85EB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
 * the slot given back.
 * HOW: Two TIA from $86C3 (256 colours) and $88C3 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
 * WAITS: no.
 * Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
 */
Exit EffectFlashEnd25F7(uint8_t slot, uint8_t y)
{
    return end_effect_flash(&LAYOUT_25F7, slot, y);
}

void abi_EffectFlashEnd25F7(void)
{
    exit_shim(EffectFlashEnd25F7(cpu.x, cpu.y));
}

static const EffectLayout LAYOUT_1CF1 = {
    0x89C3, 0x89D1, 0x89D1, 0x8331,
    { 0x82E9, 0x8362, 0x840B, 0x84FF, 0x8560, 0x8539, 0x8543 }, 7,
    0x8612, 0x8692, 0x86C3, 0x88C3,
    0x853B, 0x8545,
    0x82DB, 0x82A6, 0x82C5
};

/*
 * @routine 1CF1:29F EffectStep1CF1
 * @run 6C:829F
 * @unreachable 6C:829F-6C:82CA no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $829F runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
 * otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
 * HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
 * 5 at slot 3 is entered with X 3.
 * WAITS: no.
 * Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
 * left; C, Y and V as the last handler left them.
 */
Exit EffectStep1CF1(uint8_t y)
{
    return run_effect_step(&LAYOUT_1CF1, y);
}

void abi_EffectStep1CF1(void)
{
    exit_shim(EffectStep1CF1(cpu.y));
}

/*
 * @routine 1CF1:2CB EffectDispatch1CF1
 * @run 6C:82CB
 * @stop 82DB
 * @unreachable 6C:82CB-6C:82DA no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82CB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82DB holds one address per
 * kind, kind 1 first.
 * HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
 * with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
 * WAITS: no.
 * Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
 */
Exit EffectDispatch1CF1(uint8_t type, uint8_t slot)
{
    return run_effect_dispatch(&LAYOUT_1CF1, type, slot);
}

void abi_EffectDispatch1CF1(void)
{
    exit_shim(EffectDispatch1CF1(cpu.a, cpu.x));
}

/*
 * @routine 1CF1:2E9 EffectList1CF1
 * @run 6C:82E9
 * @unreachable 6C:82E9-6C:8348 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82E9 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
 * list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
 * again, and when the rounds the list names have been played it gives the slot back.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
 * $89C3); each palette loaded by the packed palette loader at $84BF, whose bytes are LoadPackedPaletteC's.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
 * Z and C from comparing it with the rounds played, Y 1; X kept.
 */
Exit EffectList1CF1(uint8_t slot)
{
    return run_effect_list(&LAYOUT_1CF1, slot);
}

void abi_EffectList1CF1(void)
{
    exit_shim(EffectList1CF1(cpu.x));
}

/*
 * @routine 1CF1:349 ClearEffectSlot1CF1
 * @run 6C:8349
 * @unreachable 6C:8349-6C:8361 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8349 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
 * HOW: Eight STZ of the slot's columns, the kind first.
 * WAITS: no.
 * Entry: X the slot. Exit: A, X, Y and the flags kept.
 */
void ClearEffectSlot1CF1(uint8_t slot)
{
    clear_effect_slot(slot);
}

void abi_ClearEffectSlot1CF1(void)
{
    ClearEffectSlot1CF1(cpu.x);
}

/*
 * @routine 1CF1:362 EffectFade1CF1
 * @run 6C:8362
 * @unreachable 6C:8362-6C:840A no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8362 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
 * the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
 * every colour of them by one step in each of its three components, writing them back to the VCE.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
 * table at $89D1; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
 * $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
 */
Exit EffectFade1CF1(uint8_t slot)
{
    return run_effect_fade(&LAYOUT_1CF1, slot);
}

void abi_EffectFade1CF1(void)
{
    exit_shim(EffectFade1CF1(cpu.x));
}

/*
 * @routine 1CF1:40B EffectShade1CF1
 * @run 6C:840B
 * @stop 84BF
 * @unreachable 6C:840B-6C:84BE no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $840B runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 3: a palette effect that darkens palettes by an amount that grows each round: once its timer has
 * run down it counts a round (the slot is given back after the eighth) and writes the list's palettes to the VCE, each
 * colour's blue, green and red parts taken down by 7 less the count, by that times 8 and times 32, not below zero.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9404; zTemp08 to zTemp0A the three amounts;
 * the blocks (eight colours, their ninth bits first) read through zTemp02.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, Y
 * the list's place, V from the last block's pointer add (else kept).
 */
Exit EffectShade1CF1(uint8_t slot)
{
    return run_effect_shade(slot);
}

void abi_EffectShade1CF1(void)
{
    exit_shim(EffectShade1CF1(cpu.x));
}

/*
 * @routine 1CF1:4FF EffectColour1CF1
 * @run 6C:84FF
 * @unreachable 6C:84FF-6C:8538 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $84FF runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
 * colour's two bytes) picked by the list number, the slot then given back.
 * HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
 * or for a negative number the table at $89D1; the four bytes go to the VCE ports.
 * WAITS: no.
 * Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
 * kept.
 */
Exit EffectColour1CF1(uint8_t slot)
{
    return run_effect_colour(&LAYOUT_1CF1, slot);
}

void abi_EffectColour1CF1(void)
{
    exit_shim(EffectColour1CF1(cpu.x));
}

/*
 * @routine 1CF1:539 EffectVceSave1CF1
 * @run 6C:8539
 * @unreachable 6C:8539-6C:8542 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8539 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 6: all 512 colours of the VCE saved in the Super RAM bank mapped at $A000, at $B000.
 * HOW: SelectSavedBank, then a TAI of $400 bytes from the VCE's data port.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
 */
Exit EffectVceSave1CF1(uint8_t slot, uint8_t y)
{
    return run_vce_save(&LAYOUT_1CF1, slot, y);
}

void abi_EffectVceSave1CF1(void)
{
    exit_shim(EffectVceSave1CF1(cpu.x, cpu.y));
}

/*
 * @routine 1CF1:543 EffectVceRestore1CF1
 * @run 6C:8543
 * @unreachable 6C:8543-6C:854C no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8543 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 7: the 512 colours written back to the VCE from the Super RAM bank mapped at $A000, at $B000.
 * HOW: SelectSavedBank, then a TIA of $400 bytes to the VCE's data port.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
 */
Exit EffectVceRestore1CF1(uint8_t slot, uint8_t y)
{
    return run_vce_restore(&LAYOUT_1CF1, slot, y);
}

void abi_EffectVceRestore1CF1(void)
{
    exit_shim(EffectVceRestore1CF1(cpu.x, cpu.y));
}

/*
 * @routine 1CF1:54D SelectSavedBank1CF1
 * @run 6C:854D
 * @unreachable 6C:854D-6C:855F no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $854D runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Maps the Super RAM bank of the saved palette at $A000 and sets the VCE's colour address to 0.
 * HOW: The bank is the base bank plus $12, or plus $30 when bit 0 of zIdleFlag is set; TAM for MPR5.
 * WAITS: no.
 * Entry: nothing. Exit: A the bank, N and Z from it, C and V from the add; X and Y kept.
 */
Exit SelectSavedBank1CF1(void)
{
    return select_saved_bank();
}

void abi_SelectSavedBank1CF1(void)
{
    exit_shim(SelectSavedBank1CF1());
}

/*
 * @routine 1CF1:560 EffectFlash1CF1
 * @run 6C:8560
 * @stop 05EB
 * @unreachable 6C:8560-6C:85EA no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8560 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
 * in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
 * palette the list at $8692 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
 * HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86C3, $88C3 and $3C60;
 * the colour table at $8612 indexed by the list number times 32 and the count times 2.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
 * no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
 * with C set.
 */
Exit EffectFlash1CF1(uint8_t slot, uint8_t y)
{
    return run_effect_flash(&LAYOUT_1CF1, slot, y);
}

void abi_EffectFlash1CF1(void)
{
    exit_shim(EffectFlash1CF1(cpu.x, cpu.y));
}

/*
 * @routine 1CF1:5EB EffectFlashEnd1CF1
 * @run 6C:85EB
 * @unreachable 6C:85EB-6C:8611 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $85EB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
 * the slot given back.
 * HOW: Two TIA from $86C3 (256 colours) and $88C3 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
 * WAITS: no.
 * Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
 */
Exit EffectFlashEnd1CF1(uint8_t slot, uint8_t y)
{
    return end_effect_flash(&LAYOUT_1CF1, slot, y);
}

void abi_EffectFlashEnd1CF1(void)
{
    exit_shim(EffectFlashEnd1CF1(cpu.x, cpu.y));
}

static const EffectLayout LAYOUT_1845 = {
    0x89A7, 0x9412, 0x89A7, 0x833C,
    { 0x82F4, 0x836D, 0x8416, 0x850A, 0x8544 }, 5,
    0x85F6, 0x8676, 0x86A7, 0x88A7,
    0x0000, 0x0000,
    0x82EA, 0x82B5, 0x82D4
};

/*
 * @routine 1845:2AE EffectStep1845
 * @run 6C:82AE
 * @unreachable 6C:82AE-6C:82D9 no read the game makes loads disc sector 1845, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82AE runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
 * otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
 * HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
 * 5 at slot 3 is entered with X 3.
 * WAITS: no.
 * Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
 * left; C, Y and V as the last handler left them.
 */
Exit EffectStep1845(uint8_t y)
{
    return run_effect_step(&LAYOUT_1845, y);
}

void abi_EffectStep1845(void)
{
    exit_shim(EffectStep1845(cpu.y));
}

/*
 * @routine 1845:2DA EffectDispatch1845
 * @run 6C:82DA
 * @stop 82EA
 * @unreachable 6C:82DA-6C:82E9 no read the game makes loads disc sector 1845, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82DA runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82EA holds one address per
 * kind, kind 1 first.
 * HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
 * with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
 * WAITS: no.
 * Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
 */
Exit EffectDispatch1845(uint8_t type, uint8_t slot)
{
    return run_effect_dispatch(&LAYOUT_1845, type, slot);
}

void abi_EffectDispatch1845(void)
{
    exit_shim(EffectDispatch1845(cpu.a, cpu.x));
}

/*
 * @routine 1845:2F4 EffectList1845
 * @run 6C:82F4
 * @unreachable 6C:82F4-6C:8353 no read the game makes loads disc sector 1845, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82F4 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
 * list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
 * again, and when the rounds the list names have been played it gives the slot back.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
 * $89A7); each palette loaded by the packed palette loader at $84CA, whose bytes are LoadPackedPaletteC's.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
 * Z and C from comparing it with the rounds played, Y 1; X kept.
 */
Exit EffectList1845(uint8_t slot)
{
    return run_effect_list(&LAYOUT_1845, slot);
}

void abi_EffectList1845(void)
{
    exit_shim(EffectList1845(cpu.x));
}

/*
 * @routine 1845:354 ClearEffectSlot1845
 * @run 6C:8354
 * @unreachable 6C:8354-6C:836C no read the game makes loads disc sector 1845, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8354 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
 * HOW: Eight STZ of the slot's columns, the kind first.
 * WAITS: no.
 * Entry: X the slot. Exit: A, X, Y and the flags kept.
 */
void ClearEffectSlot1845(uint8_t slot)
{
    clear_effect_slot(slot);
}

void abi_ClearEffectSlot1845(void)
{
    ClearEffectSlot1845(cpu.x);
}

/*
 * @routine 1845:36D EffectFade1845
 * @run 6C:836D
 * @unreachable 6C:836D-6C:8415 no read the game makes loads disc sector 1845, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $836D runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
 * the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
 * every colour of them by one step in each of its three components, writing them back to the VCE.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
 * table at $9412; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
 * $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
 */
Exit EffectFade1845(uint8_t slot)
{
    return run_effect_fade(&LAYOUT_1845, slot);
}

void abi_EffectFade1845(void)
{
    exit_shim(EffectFade1845(cpu.x));
}

/*
 * @routine 1845:416 EffectShade1845
 * @run 6C:8416
 * @stop 84CA
 * @unreachable 6C:8416-6C:84C9 no read the game makes loads disc sector 1845, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8416 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 3: a palette effect that darkens palettes by an amount that grows each round: once its timer has
 * run down it counts a round (the slot is given back after the eighth) and writes the list's palettes to the VCE, each
 * colour's blue, green and red parts taken down by 7 less the count, by that times 8 and times 32, not below zero.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9404; zTemp08 to zTemp0A the three amounts;
 * the blocks (eight colours, their ninth bits first) read through zTemp02.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, Y
 * the list's place, V from the last block's pointer add (else kept).
 */
Exit EffectShade1845(uint8_t slot)
{
    return run_effect_shade(slot);
}

void abi_EffectShade1845(void)
{
    exit_shim(EffectShade1845(cpu.x));
}

/*
 * @routine 1845:50A EffectColour1845
 * @run 6C:850A
 * @unreachable 6C:850A-6C:8543 no read the game makes loads disc sector 1845, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $850A runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
 * colour's two bytes) picked by the list number, the slot then given back.
 * HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
 * or for a negative number the table at $89A7; the four bytes go to the VCE ports.
 * WAITS: no.
 * Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
 * kept.
 */
Exit EffectColour1845(uint8_t slot)
{
    return run_effect_colour(&LAYOUT_1845, slot);
}

void abi_EffectColour1845(void)
{
    exit_shim(EffectColour1845(cpu.x));
}

/*
 * @routine 1845:544 EffectFlash1845
 * @run 6C:8544
 * @stop 05CF
 * @unreachable 6C:8544-6C:85CE no read the game makes loads disc sector 1845, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8544 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
 * in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
 * palette the list at $8676 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
 * HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86A7, $88A7 and $3C60;
 * the colour table at $85F6 indexed by the list number times 32 and the count times 2.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
 * no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
 * with C set.
 */
Exit EffectFlash1845(uint8_t slot, uint8_t y)
{
    return run_effect_flash(&LAYOUT_1845, slot, y);
}

void abi_EffectFlash1845(void)
{
    exit_shim(EffectFlash1845(cpu.x, cpu.y));
}

/*
 * @routine 1845:5CF EffectFlashEnd1845
 * @run 6C:85CF
 * @unreachable 6C:85CF-6C:85F5 no read the game makes loads disc sector 1845, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $85CF runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
 * the slot given back.
 * HOW: Two TIA from $86A7 (256 colours) and $88A7 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
 * WAITS: no.
 * Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
 */
Exit EffectFlashEnd1845(uint8_t slot, uint8_t y)
{
    return end_effect_flash(&LAYOUT_1845, slot, y);
}

void abi_EffectFlashEnd1845(void)
{
    exit_shim(EffectFlashEnd1845(cpu.x, cpu.y));
}

static const EffectLayout LAYOUT_1AA0 = {
    0x89A7, 0x940E, 0x89A7, 0x833C,
    { 0x82F4, 0x836D, 0x8416, 0x850A, 0x8544 }, 5,
    0x85F6, 0x8676, 0x86A7, 0x88A7,
    0x0000, 0x0000,
    0x82EA, 0x82B5, 0x82D4
};

/*
 * @routine 1AA0:2AE EffectStep1AA0
 * @run 6C:82AE
 * @unreachable 6C:82AE-6C:82D9 no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82AE runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
 * otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
 * HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
 * 5 at slot 3 is entered with X 3.
 * WAITS: no.
 * Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
 * left; C, Y and V as the last handler left them.
 */
Exit EffectStep1AA0(uint8_t y)
{
    return run_effect_step(&LAYOUT_1AA0, y);
}

void abi_EffectStep1AA0(void)
{
    exit_shim(EffectStep1AA0(cpu.y));
}

/*
 * @routine 1AA0:2DA EffectDispatch1AA0
 * @run 6C:82DA
 * @stop 82EA
 * @unreachable 6C:82DA-6C:82E9 no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82DA runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82EA holds one address per
 * kind, kind 1 first.
 * HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
 * with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
 * WAITS: no.
 * Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
 */
Exit EffectDispatch1AA0(uint8_t type, uint8_t slot)
{
    return run_effect_dispatch(&LAYOUT_1AA0, type, slot);
}

void abi_EffectDispatch1AA0(void)
{
    exit_shim(EffectDispatch1AA0(cpu.a, cpu.x));
}

/*
 * @routine 1AA0:2F4 EffectList1AA0
 * @run 6C:82F4
 * @unreachable 6C:82F4-6C:8353 no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82F4 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
 * list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
 * again, and when the rounds the list names have been played it gives the slot back.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
 * $89A7); each palette loaded by the packed palette loader at $84CA, whose bytes are LoadPackedPaletteC's.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
 * Z and C from comparing it with the rounds played, Y 1; X kept.
 */
Exit EffectList1AA0(uint8_t slot)
{
    return run_effect_list(&LAYOUT_1AA0, slot);
}

void abi_EffectList1AA0(void)
{
    exit_shim(EffectList1AA0(cpu.x));
}

/*
 * @routine 1AA0:354 ClearEffectSlot1AA0
 * @run 6C:8354
 * @unreachable 6C:8354-6C:836C no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8354 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
 * HOW: Eight STZ of the slot's columns, the kind first.
 * WAITS: no.
 * Entry: X the slot. Exit: A, X, Y and the flags kept.
 */
void ClearEffectSlot1AA0(uint8_t slot)
{
    clear_effect_slot(slot);
}

void abi_ClearEffectSlot1AA0(void)
{
    ClearEffectSlot1AA0(cpu.x);
}

/*
 * @routine 1AA0:36D EffectFade1AA0
 * @run 6C:836D
 * @unreachable 6C:836D-6C:8415 no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $836D runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
 * the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
 * every colour of them by one step in each of its three components, writing them back to the VCE.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
 * table at $940E; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
 * $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
 */
Exit EffectFade1AA0(uint8_t slot)
{
    return run_effect_fade(&LAYOUT_1AA0, slot);
}

void abi_EffectFade1AA0(void)
{
    exit_shim(EffectFade1AA0(cpu.x));
}

/*
 * @routine 1AA0:416 EffectShade1AA0
 * @run 6C:8416
 * @stop 84CA
 * @unreachable 6C:8416-6C:84C9 no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8416 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 3: a palette effect that darkens palettes by an amount that grows each round: once its timer has
 * run down it counts a round (the slot is given back after the eighth) and writes the list's palettes to the VCE, each
 * colour's blue, green and red parts taken down by 7 less the count, by that times 8 and times 32, not below zero.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9404; zTemp08 to zTemp0A the three amounts;
 * the blocks (eight colours, their ninth bits first) read through zTemp02.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, Y
 * the list's place, V from the last block's pointer add (else kept).
 */
Exit EffectShade1AA0(uint8_t slot)
{
    return run_effect_shade(slot);
}

void abi_EffectShade1AA0(void)
{
    exit_shim(EffectShade1AA0(cpu.x));
}

/*
 * @routine 1AA0:50A EffectColour1AA0
 * @run 6C:850A
 * @unreachable 6C:850A-6C:8543 no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $850A runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
 * colour's two bytes) picked by the list number, the slot then given back.
 * HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
 * or for a negative number the table at $89A7; the four bytes go to the VCE ports.
 * WAITS: no.
 * Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
 * kept.
 */
Exit EffectColour1AA0(uint8_t slot)
{
    return run_effect_colour(&LAYOUT_1AA0, slot);
}

void abi_EffectColour1AA0(void)
{
    exit_shim(EffectColour1AA0(cpu.x));
}

/*
 * @routine 1AA0:544 EffectFlash1AA0
 * @run 6C:8544
 * @stop 05CF
 * @unreachable 6C:8544-6C:85CE no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8544 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
 * in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
 * palette the list at $8676 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
 * HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86A7, $88A7 and $3C60;
 * the colour table at $85F6 indexed by the list number times 32 and the count times 2.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
 * no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
 * with C set.
 */
Exit EffectFlash1AA0(uint8_t slot, uint8_t y)
{
    return run_effect_flash(&LAYOUT_1AA0, slot, y);
}

void abi_EffectFlash1AA0(void)
{
    exit_shim(EffectFlash1AA0(cpu.x, cpu.y));
}

/*
 * @routine 1AA0:5CF EffectFlashEnd1AA0
 * @run 6C:85CF
 * @unreachable 6C:85CF-6C:85F5 no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $85CF runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
 * the slot given back.
 * HOW: Two TIA from $86A7 (256 colours) and $88A7 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
 * WAITS: no.
 * Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
 */
Exit EffectFlashEnd1AA0(uint8_t slot, uint8_t y)
{
    return end_effect_flash(&LAYOUT_1AA0, slot, y);
}

void abi_EffectFlashEnd1AA0(void)
{
    exit_shim(EffectFlashEnd1AA0(cpu.x, cpu.y));
}

static const EffectLayout LAYOUT_1C2F = {
    0x89A7, 0x9410, 0x89A7, 0x833C,
    { 0x82F4, 0x836D, 0x8416, 0x850A, 0x8544 }, 5,
    0x85F6, 0x8676, 0x86A7, 0x88A7,
    0x0000, 0x0000,
    0x82EA, 0x82B5, 0x82D4
};

/*
 * @routine 1C2F:2AE EffectStep1C2F
 * @run 6C:82AE
 * @unreachable 6C:82AE-6C:82D9 no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82AE runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
 * otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
 * HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
 * 5 at slot 3 is entered with X 3.
 * WAITS: no.
 * Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
 * left; C, Y and V as the last handler left them.
 */
Exit EffectStep1C2F(uint8_t y)
{
    return run_effect_step(&LAYOUT_1C2F, y);
}

void abi_EffectStep1C2F(void)
{
    exit_shim(EffectStep1C2F(cpu.y));
}

/*
 * @routine 1C2F:2DA EffectDispatch1C2F
 * @run 6C:82DA
 * @stop 82EA
 * @unreachable 6C:82DA-6C:82E9 no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82DA runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82EA holds one address per
 * kind, kind 1 first.
 * HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
 * with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
 * WAITS: no.
 * Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
 */
Exit EffectDispatch1C2F(uint8_t type, uint8_t slot)
{
    return run_effect_dispatch(&LAYOUT_1C2F, type, slot);
}

void abi_EffectDispatch1C2F(void)
{
    exit_shim(EffectDispatch1C2F(cpu.a, cpu.x));
}

/*
 * @routine 1C2F:2F4 EffectList1C2F
 * @run 6C:82F4
 * @unreachable 6C:82F4-6C:8353 no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82F4 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
 * list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
 * again, and when the rounds the list names have been played it gives the slot back.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
 * $89A7); each palette loaded by the packed palette loader at $84CA, whose bytes are LoadPackedPaletteC's.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
 * Z and C from comparing it with the rounds played, Y 1; X kept.
 */
Exit EffectList1C2F(uint8_t slot)
{
    return run_effect_list(&LAYOUT_1C2F, slot);
}

void abi_EffectList1C2F(void)
{
    exit_shim(EffectList1C2F(cpu.x));
}

/*
 * @routine 1C2F:354 ClearEffectSlot1C2F
 * @run 6C:8354
 * @unreachable 6C:8354-6C:836C no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8354 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
 * HOW: Eight STZ of the slot's columns, the kind first.
 * WAITS: no.
 * Entry: X the slot. Exit: A, X, Y and the flags kept.
 */
void ClearEffectSlot1C2F(uint8_t slot)
{
    clear_effect_slot(slot);
}

void abi_ClearEffectSlot1C2F(void)
{
    ClearEffectSlot1C2F(cpu.x);
}

/*
 * @routine 1C2F:36D EffectFade1C2F
 * @run 6C:836D
 * @unreachable 6C:836D-6C:8415 no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $836D runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
 * the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
 * every colour of them by one step in each of its three components, writing them back to the VCE.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
 * table at $9410; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
 * $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
 */
Exit EffectFade1C2F(uint8_t slot)
{
    return run_effect_fade(&LAYOUT_1C2F, slot);
}

void abi_EffectFade1C2F(void)
{
    exit_shim(EffectFade1C2F(cpu.x));
}

/*
 * @routine 1C2F:416 EffectShade1C2F
 * @run 6C:8416
 * @stop 84CA
 * @unreachable 6C:8416-6C:84C9 no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8416 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 3: a palette effect that darkens palettes by an amount that grows each round: once its timer has
 * run down it counts a round (the slot is given back after the eighth) and writes the list's palettes to the VCE, each
 * colour's blue, green and red parts taken down by 7 less the count, by that times 8 and times 32, not below zero.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9404; zTemp08 to zTemp0A the three amounts;
 * the blocks (eight colours, their ninth bits first) read through zTemp02.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, Y
 * the list's place, V from the last block's pointer add (else kept).
 */
Exit EffectShade1C2F(uint8_t slot)
{
    return run_effect_shade(slot);
}

void abi_EffectShade1C2F(void)
{
    exit_shim(EffectShade1C2F(cpu.x));
}

/*
 * @routine 1C2F:50A EffectColour1C2F
 * @run 6C:850A
 * @unreachable 6C:850A-6C:8543 no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $850A runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
 * colour's two bytes) picked by the list number, the slot then given back.
 * HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
 * or for a negative number the table at $89A7; the four bytes go to the VCE ports.
 * WAITS: no.
 * Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
 * kept.
 */
Exit EffectColour1C2F(uint8_t slot)
{
    return run_effect_colour(&LAYOUT_1C2F, slot);
}

void abi_EffectColour1C2F(void)
{
    exit_shim(EffectColour1C2F(cpu.x));
}

/*
 * @routine 1C2F:544 EffectFlash1C2F
 * @run 6C:8544
 * @stop 05CF
 * @unreachable 6C:8544-6C:85CE no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8544 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
 * in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
 * palette the list at $8676 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
 * HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86A7, $88A7 and $3C60;
 * the colour table at $85F6 indexed by the list number times 32 and the count times 2.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
 * no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
 * with C set.
 */
Exit EffectFlash1C2F(uint8_t slot, uint8_t y)
{
    return run_effect_flash(&LAYOUT_1C2F, slot, y);
}

void abi_EffectFlash1C2F(void)
{
    exit_shim(EffectFlash1C2F(cpu.x, cpu.y));
}

/*
 * @routine 1C2F:5CF EffectFlashEnd1C2F
 * @run 6C:85CF
 * @unreachable 6C:85CF-6C:85F5 no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $85CF runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
 * the slot given back.
 * HOW: Two TIA from $86A7 (256 colours) and $88A7 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
 * WAITS: no.
 * Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
 */
Exit EffectFlashEnd1C2F(uint8_t slot, uint8_t y)
{
    return end_effect_flash(&LAYOUT_1C2F, slot, y);
}

void abi_EffectFlashEnd1C2F(void)
{
    exit_shim(EffectFlashEnd1C2F(cpu.x, cpu.y));
}

static const EffectLayout LAYOUT_1DDA = {
    0x89A7, 0x9416, 0x89A7, 0x833C,
    { 0x82F4, 0x836D, 0x8416, 0x850A, 0x8544 }, 5,
    0x85F6, 0x8676, 0x86A7, 0x88A7,
    0x0000, 0x0000,
    0x82EA, 0x82B5, 0x82D4
};

/*
 * @routine 1DDA:2AE EffectStep1DDA
 * @run 6C:82AE
 * @unreachable 6C:82AE-6C:82D9 no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82AE runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
 * otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
 * HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
 * 5 at slot 3 is entered with X 3.
 * WAITS: no.
 * Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
 * left; C, Y and V as the last handler left them.
 */
Exit EffectStep1DDA(uint8_t y)
{
    return run_effect_step(&LAYOUT_1DDA, y);
}

void abi_EffectStep1DDA(void)
{
    exit_shim(EffectStep1DDA(cpu.y));
}

/*
 * @routine 1DDA:2DA EffectDispatch1DDA
 * @run 6C:82DA
 * @stop 82EA
 * @unreachable 6C:82DA-6C:82E9 no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82DA runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82EA holds one address per
 * kind, kind 1 first.
 * HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
 * with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
 * WAITS: no.
 * Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
 */
Exit EffectDispatch1DDA(uint8_t type, uint8_t slot)
{
    return run_effect_dispatch(&LAYOUT_1DDA, type, slot);
}

void abi_EffectDispatch1DDA(void)
{
    exit_shim(EffectDispatch1DDA(cpu.a, cpu.x));
}

/*
 * @routine 1DDA:2F4 EffectList1DDA
 * @run 6C:82F4
 * @unreachable 6C:82F4-6C:8353 no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82F4 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
 * list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
 * again, and when the rounds the list names have been played it gives the slot back.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
 * $89A7); each palette loaded by the packed palette loader at $84CA, whose bytes are LoadPackedPaletteC's.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
 * Z and C from comparing it with the rounds played, Y 1; X kept.
 */
Exit EffectList1DDA(uint8_t slot)
{
    return run_effect_list(&LAYOUT_1DDA, slot);
}

void abi_EffectList1DDA(void)
{
    exit_shim(EffectList1DDA(cpu.x));
}

/*
 * @routine 1DDA:354 ClearEffectSlot1DDA
 * @run 6C:8354
 * @unreachable 6C:8354-6C:836C no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8354 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
 * HOW: Eight STZ of the slot's columns, the kind first.
 * WAITS: no.
 * Entry: X the slot. Exit: A, X, Y and the flags kept.
 */
void ClearEffectSlot1DDA(uint8_t slot)
{
    clear_effect_slot(slot);
}

void abi_ClearEffectSlot1DDA(void)
{
    ClearEffectSlot1DDA(cpu.x);
}

/*
 * @routine 1DDA:36D EffectFade1DDA
 * @run 6C:836D
 * @unreachable 6C:836D-6C:8415 no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $836D runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
 * the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
 * every colour of them by one step in each of its three components, writing them back to the VCE.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
 * table at $9416; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
 * $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
 */
Exit EffectFade1DDA(uint8_t slot)
{
    return run_effect_fade(&LAYOUT_1DDA, slot);
}

void abi_EffectFade1DDA(void)
{
    exit_shim(EffectFade1DDA(cpu.x));
}

/*
 * @routine 1DDA:416 EffectShade1DDA
 * @run 6C:8416
 * @stop 84CA
 * @unreachable 6C:8416-6C:84C9 no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8416 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 3: a palette effect that darkens palettes by an amount that grows each round: once its timer has
 * run down it counts a round (the slot is given back after the eighth) and writes the list's palettes to the VCE, each
 * colour's blue, green and red parts taken down by 7 less the count, by that times 8 and times 32, not below zero.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9404; zTemp08 to zTemp0A the three amounts;
 * the blocks (eight colours, their ninth bits first) read through zTemp02.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, Y
 * the list's place, V from the last block's pointer add (else kept).
 */
Exit EffectShade1DDA(uint8_t slot)
{
    return run_effect_shade(slot);
}

void abi_EffectShade1DDA(void)
{
    exit_shim(EffectShade1DDA(cpu.x));
}

/*
 * @routine 1DDA:50A EffectColour1DDA
 * @run 6C:850A
 * @unreachable 6C:850A-6C:8543 no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $850A runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
 * colour's two bytes) picked by the list number, the slot then given back.
 * HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
 * or for a negative number the table at $89A7; the four bytes go to the VCE ports.
 * WAITS: no.
 * Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
 * kept.
 */
Exit EffectColour1DDA(uint8_t slot)
{
    return run_effect_colour(&LAYOUT_1DDA, slot);
}

void abi_EffectColour1DDA(void)
{
    exit_shim(EffectColour1DDA(cpu.x));
}

/*
 * @routine 1DDA:544 EffectFlash1DDA
 * @run 6C:8544
 * @stop 05CF
 * @unreachable 6C:8544-6C:85CE no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8544 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
 * in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
 * palette the list at $8676 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
 * HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86A7, $88A7 and $3C60;
 * the colour table at $85F6 indexed by the list number times 32 and the count times 2.
 * WAITS: no.
 * Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
 * no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
 * with C set.
 */
Exit EffectFlash1DDA(uint8_t slot, uint8_t y)
{
    return run_effect_flash(&LAYOUT_1DDA, slot, y);
}

void abi_EffectFlash1DDA(void)
{
    exit_shim(EffectFlash1DDA(cpu.x, cpu.y));
}

/*
 * @routine 1DDA:5CF EffectFlashEnd1DDA
 * @run 6C:85CF
 * @unreachable 6C:85CF-6C:85F5 no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $85CF runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
 * the slot given back.
 * HOW: Two TIA from $86A7 (256 colours) and $88A7 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
 * WAITS: no.
 * Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
 */
Exit EffectFlashEnd1DDA(uint8_t slot, uint8_t y)
{
    return end_effect_flash(&LAYOUT_1DDA, slot, y);
}

void abi_EffectFlashEnd1DDA(void)
{
    exit_shim(EffectFlashEnd1DDA(cpu.x, cpu.y));
}


/*
 * The kind 2 and kind 3 handlers of version 1A31 with another table of lists for a negative list number: the copies the
 * stage overlays hold differ from 1A31's only in that table's address (effect_slots_b.c).
 */
Exit EffectFadeWithTable(uint16_t table, uint8_t slot)
{
    EffectLayout layout = LAYOUT_1A31;

    layout.fade_table = table;
    return run_effect_fade(&layout, slot);
}

Exit EffectColourWithTable(uint16_t table, uint8_t slot)
{
    EffectLayout layout = LAYOUT_1A31;

    layout.colour_table = table;
    return run_effect_colour(&layout, slot);
}

/* The title's effects (overlay sector 00C5) and the ending's kinds the ending's own code does not have (sector 0111):
 * the same code as these versions, with their own tables. */
static const EffectLayout LAYOUT_00C5 = {
    0x8A84, 0x9444, 0x8A84, 0x83F2,
    { 0x83AA, 0x8423, 0x84CC, 0x85C0, 0x8621, 0x85FA, 0x8604 }, 7,
    0x86D3, 0x8753, 0x8784, 0x8984,
    0x85FC, 0x8606,
    0x839C, 0x8367, 0x8386
};
static const EffectLayout LAYOUT_0111 = {
    0x0000, 0x0000, 0x89D2, 0x0000,
    { 0x82F8, 0x8371, 0x841A, 0x850E, 0x856F, 0x8548, 0x8552 }, 7,
    0x8621, 0x86A1, 0x86D2, 0x88D2,
    0x854A, 0x8554,
    0x82EA, 0x0000, 0x0000
};

/*
 * @routine 00C5:360 EffectStep00C5
 * @run 6C:8360
 * WHAT IT DOES: As EffectStep1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectStep1A31.
 */
Exit EffectStep00C5(uint8_t y)
{
    return run_effect_step(&LAYOUT_00C5, y);
}

void abi_EffectStep00C5(void)
{
    exit_shim(EffectStep00C5(cpu.y));
}

/*
 * @routine 00C5:38C EffectDispatch00C5
 * @run 6C:838C
 * @stop 839C
 * WHAT IT DOES: As EffectDispatch1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectDispatch1A31.
 */
Exit EffectDispatch00C5(uint8_t type, uint8_t slot)
{
    return run_effect_dispatch(&LAYOUT_00C5, type, slot);
}

void abi_EffectDispatch00C5(void)
{
    exit_shim(EffectDispatch00C5(cpu.a, cpu.x));
}

/*
 * @routine 00C5:3AA EffectList00C5
 * @run 6C:83AA
 * WHAT IT DOES: As EffectList1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectList1A31.
 */
Exit EffectList00C5(uint8_t slot)
{
    return run_effect_list(&LAYOUT_00C5, slot);
}

void abi_EffectList00C5(void)
{
    exit_shim(EffectList00C5(cpu.x));
}

/*
 * @routine 00C5:40A ClearEffectSlot00C5
 * @run 6C:840A
 * WHAT IT DOES: As ClearEffectSlot1A31, whose bytes these are.
 * HOW: Eight STZ of the slot's columns.
 * WAITS: no.
 * Entry: X the slot. Exit: A, X, Y and the flags kept.
 */
void ClearEffectSlot00C5(uint8_t slot)
{
    clear_effect_slot(slot);
}

void abi_ClearEffectSlot00C5(void)
{
    ClearEffectSlot00C5(cpu.x);
}

/*
 * @routine 00C5:423 EffectFade00C5
 * @run 6C:8423
 * WHAT IT DOES: As EffectFade1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectFade1A31.
 */
Exit EffectFade00C5(uint8_t slot)
{
    return run_effect_fade(&LAYOUT_00C5, slot);
}

void abi_EffectFade00C5(void)
{
    exit_shim(EffectFade00C5(cpu.x));
}

/*
 * @routine 00C5:4CC EffectShade00C5
 * @run 6C:84CC
 * WHAT IT DOES: As EffectShade1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectShade1A31.
 */
Exit EffectShade00C5(uint8_t slot)
{
    return run_effect_shade(slot);
}

void abi_EffectShade00C5(void)
{
    exit_shim(EffectShade00C5(cpu.x));
}

/*
 * @routine 00C5:5C0 EffectColour00C5
 * @run 6C:85C0
 * WHAT IT DOES: As EffectColour1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectColour1A31.
 */
Exit EffectColour00C5(uint8_t slot)
{
    return run_effect_colour(&LAYOUT_00C5, slot);
}

void abi_EffectColour00C5(void)
{
    exit_shim(EffectColour00C5(cpu.x));
}

/*
 * @routine 00C5:5FA EffectVceSave00C5
 * @run 6C:85FA
 * WHAT IT DOES: As EffectVceSave1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectVceSave1A31.
 */
Exit EffectVceSave00C5(uint8_t slot, uint8_t y)
{
    return run_vce_save(&LAYOUT_00C5, slot, y);
}

void abi_EffectVceSave00C5(void)
{
    exit_shim(EffectVceSave00C5(cpu.x, cpu.y));
}

/*
 * @routine 00C5:604 EffectVceRestore00C5
 * @run 6C:8604
 * WHAT IT DOES: As EffectVceRestore1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectVceRestore1A31.
 */
Exit EffectVceRestore00C5(uint8_t slot, uint8_t y)
{
    return run_vce_restore(&LAYOUT_00C5, slot, y);
}

void abi_EffectVceRestore00C5(void)
{
    exit_shim(EffectVceRestore00C5(cpu.x, cpu.y));
}

/*
 * @routine 00C5:60E SelectSavedBank00C5
 * @run 6C:860E
 * WHAT IT DOES: As SelectSavedBank1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as SelectSavedBank1A31.
 */
Exit SelectSavedBank00C5(void)
{
    return select_saved_bank();
}

void abi_SelectSavedBank00C5(void)
{
    exit_shim(SelectSavedBank00C5());
}

/*
 * @routine 00C5:621 EffectFlash00C5
 * @run 6C:8621
 * WHAT IT DOES: As EffectFlash1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectFlash1A31.
 */
Exit EffectFlash00C5(uint8_t slot, uint8_t y)
{
    return run_effect_flash(&LAYOUT_00C5, slot, y);
}

void abi_EffectFlash00C5(void)
{
    exit_shim(EffectFlash00C5(cpu.x, cpu.y));
}

/*
 * @routine 00C5:6AC EffectFlashEnd00C5
 * @run 6C:86AC
 * WHAT IT DOES: As EffectFlashEnd1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectFlashEnd1A31.
 */
Exit EffectFlashEnd00C5(uint8_t slot, uint8_t y)
{
    return end_effect_flash(&LAYOUT_00C5, slot, y);
}

void abi_EffectFlashEnd00C5(void)
{
    exit_shim(EffectFlashEnd00C5(cpu.x, cpu.y));
}

/*
 * @routine 0111:41A EffectShade0111
 * @run 6C:841A
 * WHAT IT DOES: As EffectShade1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectShade1A31.
 */
Exit EffectShade0111(uint8_t slot)
{
    return run_effect_shade(slot);
}

void abi_EffectShade0111(void)
{
    exit_shim(EffectShade0111(cpu.x));
}

/*
 * @routine 0111:50E EffectColour0111
 * @run 6C:850E
 * WHAT IT DOES: As EffectColour1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectColour1A31.
 */
Exit EffectColour0111(uint8_t slot)
{
    return run_effect_colour(&LAYOUT_0111, slot);
}

void abi_EffectColour0111(void)
{
    exit_shim(EffectColour0111(cpu.x));
}

/*
 * @routine 0111:548 EffectVceSave0111
 * @run 6C:8548
 * WHAT IT DOES: As EffectVceSave1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectVceSave1A31.
 */
Exit EffectVceSave0111(uint8_t slot, uint8_t y)
{
    return run_vce_save(&LAYOUT_0111, slot, y);
}

void abi_EffectVceSave0111(void)
{
    exit_shim(EffectVceSave0111(cpu.x, cpu.y));
}

/*
 * @routine 0111:552 EffectVceRestore0111
 * @run 6C:8552
 * WHAT IT DOES: As EffectVceRestore1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectVceRestore1A31.
 */
Exit EffectVceRestore0111(uint8_t slot, uint8_t y)
{
    return run_vce_restore(&LAYOUT_0111, slot, y);
}

void abi_EffectVceRestore0111(void)
{
    exit_shim(EffectVceRestore0111(cpu.x, cpu.y));
}

/*
 * @routine 0111:55C SelectSavedBank0111
 * @run 6C:855C
 * WHAT IT DOES: As SelectSavedBank1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as SelectSavedBank1A31.
 */
Exit SelectSavedBank0111(void)
{
    return select_saved_bank();
}

void abi_SelectSavedBank0111(void)
{
    exit_shim(SelectSavedBank0111());
}

/*
 * @routine 0111:56F EffectFlash0111
 * @run 6C:856F
 * WHAT IT DOES: As EffectFlash1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectFlash1A31.
 */
Exit EffectFlash0111(uint8_t slot, uint8_t y)
{
    return run_effect_flash(&LAYOUT_0111, slot, y);
}

void abi_EffectFlash0111(void)
{
    exit_shim(EffectFlash0111(cpu.x, cpu.y));
}

/*
 * @routine 0111:5FA EffectFlashEnd0111
 * @run 6C:85FA
 * WHAT IT DOES: As EffectFlashEnd1A31, whose instructions these are with this overlay's table addresses.
 * HOW: The shared body with this overlay's layout.
 * WAITS: no.
 * Entry, exit: as EffectFlashEnd1A31.
 */
Exit EffectFlashEnd0111(uint8_t slot, uint8_t y)
{
    return end_effect_flash(&LAYOUT_0111, slot, y);
}

void abi_EffectFlashEnd0111(void)
{
    exit_shim(EffectFlashEnd0111(cpu.x, cpu.y));
}

/* The eight list channels, from the last down: each with a kind is run, X kept across it. */
static ChannelExit run_all_channels(uint8_t y, bool c, bool v)
{
    ChannelExit out = { 0 };
    uint8_t slot = 7, kind;

    out.y = y;
    out.c = c;
    out.v = v;
    do {
        kind = rd8((uint16_t)(ListChannelKind + slot));
        out.a = kind;
        out.nz = kind;
        if (kind != 0) {
            ChannelExit run;

            push8(slot);
            jsr_push(0x8013);
            run = RunListChannel00C5(kind, slot, out.y, out.c, out.v);
            jsr_pop();
            slot = pull8();
            out.a = run.a;
            if (run.y_set) {
                out.y = run.y;
                out.y_set = true;
            }
            if (run.c_set) {
                out.c = run.c;
                out.c_set = true;
            }
            if (run.v_set) {
                out.v = run.v;
                out.v_set = true;
            }
        }
        slot--;
    } while ((slot & 0x80) == 0);
    out.x = slot;
    out.nz = slot;
    return out;
}

/*
 * @routine 00C5:009 RunListChannels00C5
 * @run 6C:8009
 * WHAT IT DOES: Runs every list channel that has a kind, from channel 7 down to 0.
 * HOW: A loop with PHX, BSR to RunListChannel at $8018 and PLX.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind read or what the last channel left; Y, C, V as the last channel left them.
 */
ChannelExit RunListChannels00C5(uint8_t y, bool c, bool v)
{
    return run_all_channels(y, c, v);
}

void abi_RunListChannels00C5(void)
{
    ChannelExitShim(RunListChannels00C5(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0111:009 RunListChannels0111
 * @run 6C:8009
 * WHAT IT DOES: Runs every list channel that has a kind, from channel 7 down to 0. The same bytes as 00C5:009.
 * HOW: A loop with PHX, BSR to RunListChannel at $8018 and PLX.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind read or what the last channel left; Y, C, V as the last channel left them.
 */
ChannelExit RunListChannels0111(uint8_t y, bool c, bool v)
{
    return run_all_channels(y, c, v);
}

void abi_RunListChannels0111(void)
{
    ChannelExitShim(RunListChannels0111(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 00C5:000 VecRunListChannels00C5
 * @run 6C:8000
 * WHAT IT DOES: Goes to the overlay's routine at $8009.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as the routine it goes to.
 */
void abi_VecRunListChannels00C5(void)
{
    run_routine(0x8009);
}

/*
 * @routine 00C5:003 VecEffectStep00C5
 * @run 6C:8003
 * WHAT IT DOES: Goes to the overlay's routine at $8360.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as the routine it goes to.
 */
void abi_VecEffectStep00C5(void)
{
    run_routine(0x8360);
}
