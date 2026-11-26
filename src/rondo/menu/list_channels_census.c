/*
 * The list channels of the ending's effect overlay, in the versions of it no read the game makes can load (sectors
 * 0441, 0651, 1231, 1A31 and 1B91). Eight channels (ListChannels, $3C80 up) each hold a kind: kind 1 plays a list of
 * VRAM lists with a hold timer between them, kind 3 steps a list of VRAM copies, any other kind runs one VRAM list and
 * frees the channel. RunListChannel is the same code in every version but for the table of kind 3's lists, so it is
 * one body here with the table as its argument. AddToListWord at $8133 is the same bytes in every version as the 00C5
 * overlay's, whose C (layers.c) it calls.
 */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "vram_update.h"
#include "layers.h"
#include "list_channels_census.h"

enum {
    KIND_LIST = 1, KIND_COPIES = 3, LIST_END = 0xFF, LIST_RESTART = 0xFE,
    COPY_HOLD = 0, COPY_SHIFTED = 1, COPY_RESTART = 2, COPY_PLAIN_STEP = 6, COPY_SHIFTED_STEP = 5,
    LIST_WORD_STEP = 2, PAUSED_BIT = 0x01, CHANNEL_LISTS = 0x9800
};


static ChannelExit channel_exit(uint8_t a, uint8_t x, uint8_t nz)
{
    ChannelExit exit = { 0 };

    exit.a = a;
    exit.x = x;
    exit.nz = nz;
    return exit;
}

static void channel_exit_shim(ChannelExit exit)
{
    cpu.x = exit.x;
    if (exit.left) {
        abi_nz(exit.x);         /* the PLX after the list; the STZs that follow leave the flags */
        return;
    }
    cpu.a = exit.a;
    abi_nz(exit.nz);
    if (exit.y_set) {
        cpu.y = exit.y;
    }
    if (exit.c_set) {
        abi_flag(P_C, exit.c);
    }
    if (exit.v_set) {
        abi_flag(P_V, exit.v);
    }
}

/* A VRAM list's exit as a channel's. */
static ChannelExit from_vram(VramExit out, uint8_t slot)
{
    ChannelExit exit = channel_exit(out.a, out.x, 0);

    (void)slot;
    exit.left = out.left;
    exit.y = out.y;
    exit.c = out.carry;
    exit.v = out.overflow;
    exit.y_set = exit.c_set = exit.v_set = true;
    return exit;
}

/* The channel's four bytes zeroed: the end of a list, and of a one-list kind. */
static void free_channel(uint8_t slot)
{
    WR8(LCH->kind[slot], 0);
    WR8(LCH->list[slot], 0);
    WR8(LCH->hold[slot], 0);
    WR8(LCH->place[slot], 0);
}

/* The hold timer counted down: A the timer read, N and Z from the decrement (DEC abs,X). */
static ChannelExit hold_down(uint8_t timer, uint8_t slot)
{
    uint8_t left = DEC8(LCH->hold[slot]);

    return channel_exit(timer, slot, left);
}

/* Kind 3's list word, from the table of copy lists, indexed by the channel's list number doubled; the ASL and TAY
 * leave Y that index and C the number's bit 7. */
typedef struct { uint8_t y; bool carry; } CopyListLoad;

static CopyListLoad load_copy_list(uint16_t table, uint8_t slot)
{
    uint8_t list = RD8(LCH->list[slot]);
    CopyListLoad load = { (uint8_t)(list << 1), (list & 0x80) != 0 };

    WR8(LCH->word_lo[slot], rd8((uint16_t)(table + load.y)));
    WR8(LCH->word_hi[slot], rd8((uint16_t)(table + 1 + load.y)));
    return load;
}

/* The VDC's selects before a copy: the control register's high byte cleared, then the read address. */
static void select_copy_read(uint16_t clear_ret, uint16_t read_ret)
{
    jsr_push(clear_ret);
    ClearVdcControlHigh();
    jsr_pop();
    jsr_push(read_ret);
    SelectVramReadAddress();
    jsr_pop();
}

/* COUNT words read from the VRAM's read port and written back through the write port, the count in the list at Y. */
static void copy_words(uint8_t y)
{
    uint8_t count = RD8_PTR_Y(ZP->temp00, y);
    uint8_t value;

    do {
        value = RD8(VDC->data_lo);
        WR8(VDC->data_lo, value);
        value = RD8(VDC->data_hi);
        WR8(VDC->data_hi, value);
        count--;
    } while (count != 0);
}

/* Kind 3: the list of VRAM copies at the channel's list word, one entry after another until a hold or the end. */
static ChannelExit run_copies(uint16_t table, uint8_t slot, uint8_t y, bool c, bool v)
{
    ChannelExit exit;
    Sum high;
    CopyListLoad load;
    uint8_t op, value, step, moved;

    if ((RD8(ZP->paused) & PAUSED_BIT) != 0) {
        exit = channel_exit(0, slot, 0);
        exit.y = y;
        return exit;
    }
    if (RD8(LCH->place[slot]) == 0) {
        INC8(LCH->place[slot]);
        load = load_copy_list(table, slot);
        y = load.y;
        c = load.carry;
    } else {
        value = RD8(LCH->hold[slot]);
        if (value != 0) {
            return hold_down(value, slot);
        }
    }
    for (;;) {
        WR8(ZP->temp00, RD8(LCH->word_lo[slot]));
        WR8(ZP->temp01, RD8(LCH->word_hi[slot]));
        op = RD8_PTR(ZP->temp00);
        if ((op & 0x80) != 0) {
            free_channel(slot);
            exit = channel_exit(op, slot, op);
            exit.y = y;
            exit.y_set = true;
            exit.c = c;
            exit.c_set = true;
            exit.v = v;
            exit.v_set = true;
            return exit;
        }
        if (op == COPY_HOLD) {
            y = 1;
            WR8(LCH->hold[slot], RD8_PTR_Y(ZP->temp00, y));
            high = AddToListWord(LIST_WORD_STEP, slot);     /* falls through into it: no frame */
            exit = channel_exit(high.value, slot, high.value);
            exit.y = y;
            exit.c = high.carry;
            exit.v = high.overflow;
            exit.y_set = exit.c_set = exit.v_set = true;
            return exit;
        }
        if (op == COPY_RESTART) {
            load = load_copy_list(table, slot);
            y = load.y;
            c = load.carry;
            continue;
        }
        if (op == COPY_SHIFTED) {
            select_copy_read(0x80E2, 0x80E5);
            y = 3;
            value = RD8_PTR_Y(ZP->temp00, y);
            push8(value);
            WR8(VDC->data_lo, (uint8_t)(value & 0xF0));
            y = 1;
            WR8(VDC->data_hi, RD8_PTR_Y(ZP->temp00, y));
            jsr_push(0x80F9);
            SelectVramWriteAddress();
            jsr_pop();
            value = pull8();
            WR8(VDC->data_lo, (uint8_t)((value & 0x0F) << 4));
            y++;
            WR8(VDC->data_hi, RD8_PTR_Y(ZP->temp00, y));
            jsr_push(0x810C);
            SelectVramData();
            jsr_pop();
            push8(slot);
            step = COPY_SHIFTED_STEP;
            y = 4;
        } else {
            select_copy_read(0x80B7, 0x80BA);
            y = 1;
            WR8(VDC->data_lo, RD8_PTR_Y(ZP->temp00, y));
            y++;
            WR8(VDC->data_hi, RD8_PTR_Y(ZP->temp00, y));
            jsr_push(0x80CA);
            SelectVramWriteAddress();
            jsr_pop();
            y++;
            WR8(VDC->data_lo, RD8_PTR_Y(ZP->temp00, y));
            y++;
            WR8(VDC->data_hi, RD8_PTR_Y(ZP->temp00, y));
            jsr_push(0x80D9);
            SelectVramData();
            jsr_pop();
            y++;
            push8(slot);
            step = COPY_PLAIN_STEP;
        }
        copy_words(y);
        y = 0;
        moved = pull8();
        slot = moved;
        jsr_push(0x8127);
        high = AddToListWord(step, slot);
        jsr_pop();
        c = high.carry;
        v = high.overflow;
    }
}

/* Kind 1: while the hold lasts it counts down; else the next VRAM list of the channel's list (through the overlay's
 * table of lists at $9800), the hold set from it; the end frees the channel, and $FE starts it again. */
static ChannelExit run_lists(uint8_t slot, uint8_t y_in, bool c, bool v, bool *again)
{
    ChannelExit exit;
    uint8_t value, y;

    *again = false;
    value = RD8(LCH->hold[slot]);
    if (value != 0) {
        exit = hold_down(value, slot);
        exit.y = y_in;
        return exit;
    }
    WR8(ZP->bios_a.lo, rd8(CHANNEL_LISTS));
    WR8(ZP->bios_a.hi, rd8(CHANNEL_LISTS + 1));
    y = (uint8_t)(RD8(LCH->list[slot]) << 1);
    WR8(ZP->temp00, RD8_PTR_Y(ZP->bios_a.lo, y));
    y++;
    WR8(ZP->temp01, RD8_PTR_Y(ZP->bios_a.lo, y));
    y = (uint8_t)(RD8(LCH->place[slot]) << 1);
    value = RD8_PTR_Y(ZP->temp00, y);
    if (value == LIST_END) {
        free_channel(slot);
        exit = channel_exit(value, slot, 0);
        exit.y = y;
        exit.c = true;
        exit.y_set = exit.c_set = true;
        return exit;
    }
    if (value == LIST_RESTART) {
        WR8(LCH->hold[slot], 0);
        WR8(LCH->place[slot], 0);
        *again = true;
        exit = channel_exit(value, slot, 0);
        return exit;
    }
    WR8(LCH->hold[slot], value);
    INC8(LCH->place[slot]);
    y++;
    value = RD8_PTR_Y(ZP->temp00, y);
    (void)c;
    return from_vram(VramListFromIndex(value, slot, (value & 0x80) != 0, false, v), slot);
}

/* Any other kind: the channel's VRAM list, then the channel freed. */
static ChannelExit run_once(uint8_t slot, bool c, bool v)
{
    ChannelExit exit;
    uint8_t list = RD8(LCH->list[slot]);

    push8(slot);
    jsr_push(0x8026);
    exit = from_vram(VramListFromIndex(list, slot, (list & 0x80) != 0, c, v), slot);
    jsr_pop();
    slot = pull8();
    free_channel(slot);
    exit.x = slot;
    exit.nz = slot;             /* the PLX's, kept by the STZs */
    return exit;
}

/* RunListChannel's body: the kind in A picks the path; a $FE in a kind 1 list runs the routine again from its top
 * with A $FE, which takes the other-kind path. */
static ChannelExit run_list_channel(uint16_t table, uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    ChannelExit exit;
    bool again;

    for (;;) {
        if (kind == KIND_LIST) {
            exit = run_lists(slot, y, c, v, &again);
            if (!again) {
                if (!exit.c_set) {
                    exit.c = c;
                    exit.c_set = true;
                }
                return exit;
            }
            kind = LIST_RESTART;
            c = true;           /* the CMP #$FE that matched */
            continue;
        }
        if (kind == KIND_COPIES) {
            exit = run_copies(table, slot, y, c, v);
            return exit;
        }
        return run_once(slot, c, v);
    }
}

static ChannelExit abi_run_list_channel(uint16_t table)
{
    return run_list_channel(table, cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0);
}

/*
 * @routine 0441:018 RunListChannel0441
 * @run 6C:8018
 * @stop 8133
 * @unreachable 6C:8018-6C:8132 no read the game makes loads disc sector 0441, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8018 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs one list channel by its kind: 1 plays a list of VRAM lists with a hold between them, 3 steps a
 * list of VRAM copies (its lists from the table at $8E6A), any other runs one VRAM list and frees the channel.
 * HOW: The kind counted down to pick the path. Kind 1 reads the overlay's table of channel lists at $9800, the hold
 * from the list and the VRAM list after it (VramListFromIndex, a tail jump); $FF ends it, $FE starts it again and runs
 * the routine from its top with A $FE. Kind 3 waits while the game is paused; each entry of its list is a hold (0),
 * a restart (2), or a VRAM copy (words read from the read port and written back, from a plain address pair or, for 1,
 * a pair packed in a byte); AddToListWord steps the list word on. Other kinds: VramListFromIndex, then the channel freed.
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel0441(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return run_list_channel(0x8E6A, kind, slot, y, c, v);
}

void abi_RunListChannel0441(void)
{
    channel_exit_shim(abi_run_list_channel(0x8E6A));
}

/*
 * @routine 0651:018 RunListChannel0651
 * @run 6C:8018
 * @stop 8133
 * @unreachable 6C:8018-6C:8132 no read the game makes loads disc sector 0651, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8018 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs one list channel by its kind: 1 plays a list of VRAM lists with a hold between them, 3 steps a
 * list of VRAM copies (its lists from the table at $8DC1), any other runs one VRAM list and frees the channel.
 * HOW: The kind counted down to pick the path. Kind 1 reads the overlay's table of channel lists at $9800, the hold
 * from the list and the VRAM list after it (VramListFromIndex, a tail jump); $FF ends it, $FE starts it again and runs
 * the routine from its top with A $FE. Kind 3 waits while the game is paused; each entry of its list is a hold (0),
 * a restart (2), or a VRAM copy (words read from the read port and written back, from a plain address pair or, for 1,
 * a pair packed in a byte); AddToListWord steps the list word on. Other kinds: VramListFromIndex, then the channel freed.
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel0651(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return run_list_channel(0x8DC1, kind, slot, y, c, v);
}

void abi_RunListChannel0651(void)
{
    channel_exit_shim(abi_run_list_channel(0x8DC1));
}

/*
 * @routine 1231:018 RunListChannel1231
 * @run 6C:8018
 * @stop 8133
 * @unreachable 6C:8018-6C:8132 no read the game makes loads disc sector 1231, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8018 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs one list channel by its kind: 1 plays a list of VRAM lists with a hold between them, 3 steps a
 * list of VRAM copies (its lists from the table at $8EA0), any other runs one VRAM list and frees the channel.
 * HOW: The kind counted down to pick the path. Kind 1 reads the overlay's table of channel lists at $9800, the hold
 * from the list and the VRAM list after it (VramListFromIndex, a tail jump); $FF ends it, $FE starts it again and runs
 * the routine from its top with A $FE. Kind 3 waits while the game is paused; each entry of its list is a hold (0),
 * a restart (2), or a VRAM copy (words read from the read port and written back, from a plain address pair or, for 1,
 * a pair packed in a byte); AddToListWord steps the list word on. Other kinds: VramListFromIndex, then the channel freed.
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel1231(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return run_list_channel(0x8EA0, kind, slot, y, c, v);
}

void abi_RunListChannel1231(void)
{
    channel_exit_shim(abi_run_list_channel(0x8EA0));
}

/*
 * @routine 1A31:018 RunListChannel1A31
 * @run 6C:8018
 * @stop 8133
 * @unreachable 6C:8018-6C:8132 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8018 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs one list channel by its kind: 1 plays a list of VRAM lists with a hold between them, 3 steps a
 * list of VRAM copies (its lists from the table at $8E9A), any other runs one VRAM list and frees the channel.
 * HOW: The kind counted down to pick the path. Kind 1 reads the overlay's table of channel lists at $9800, the hold
 * from the list and the VRAM list after it (VramListFromIndex, a tail jump); $FF ends it, $FE starts it again and runs
 * the routine from its top with A $FE. Kind 3 waits while the game is paused; each entry of its list is a hold (0),
 * a restart (2), or a VRAM copy (words read from the read port and written back, from a plain address pair or, for 1,
 * a pair packed in a byte); AddToListWord steps the list word on. Other kinds: VramListFromIndex, then the channel freed.
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel1A31(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return run_list_channel(0x8E9A, kind, slot, y, c, v);
}

void abi_RunListChannel1A31(void)
{
    channel_exit_shim(abi_run_list_channel(0x8E9A));
}

/*
 * @routine 1B91:018 RunListChannel1B91
 * @run 6C:8018
 * @stop 8133
 * @unreachable 6C:8018-6C:8132 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8018 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs one list channel by its kind: 1 plays a list of VRAM lists with a hold between them, 3 steps a
 * list of VRAM copies (its lists from the table at $8CF2), any other runs one VRAM list and frees the channel.
 * HOW: The kind counted down to pick the path. Kind 1 reads the overlay's table of channel lists at $9800, the hold
 * from the list and the VRAM list after it (VramListFromIndex, a tail jump); $FF ends it, $FE starts it again and runs
 * the routine from its top with A $FE. Kind 3 waits while the game is paused; each entry of its list is a hold (0),
 * a restart (2), or a VRAM copy (words read from the read port and written back, from a plain address pair or, for 1,
 * a pair packed in a byte); AddToListWord steps the list word on. Other kinds: VramListFromIndex, then the channel freed.
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel1B91(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return run_list_channel(0x8CF2, kind, slot, y, c, v);
}

void abi_RunListChannel1B91(void)
{
    channel_exit_shim(abi_run_list_channel(0x8CF2));
}

/*
 * @routine 25F7:018 RunListChannel25F7
 * @run 6C:8018
 * @stop 8133
 * @unreachable 6C:8018-6C:8132 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8018 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs one list channel by its kind: 1 plays a list of VRAM lists with a hold between them, 3 steps a
 * list of VRAM copies (its lists from the table at $829F), any other runs one VRAM list and frees the channel.
 * HOW: The kind counted down to pick the path. Kind 1 reads the overlay's table of channel lists at $9800, the hold
 * from the list and the VRAM list after it (VramListFromIndex, a tail jump); $FF ends it, $FE starts it again and runs
 * the routine from its top with A $FE. Kind 3 waits while the game is paused; each entry of its list is a hold (0),
 * a restart (2), or a VRAM copy (words read from the read port and written back, from a plain address pair or, for 1,
 * a pair packed in a byte); AddToListWord steps the list word on. Other kinds: VramListFromIndex, then the channel freed.
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel25F7(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return run_list_channel(0x829F, kind, slot, y, c, v);
}

void abi_RunListChannel25F7(void)
{
    channel_exit_shim(abi_run_list_channel(0x829F));
}

/*
 * @routine 1CF1:018 RunListChannel1CF1
 * @run 6C:8018
 * @stop 8133
 * @unreachable 6C:8018-6C:8132 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8018 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs one list channel by its kind: 1 plays a list of VRAM lists with a hold between them, 3 steps a
 * list of VRAM copies (its lists from the table at $9049), any other runs one VRAM list and frees the channel.
 * HOW: The kind counted down to pick the path. Kind 1 reads the overlay's table of channel lists at $9800, the hold
 * from the list and the VRAM list after it (VramListFromIndex, a tail jump); $FF ends it, $FE starts it again and runs
 * the routine from its top with A $FE. Kind 3 waits while the game is paused; each entry of its list is a hold (0),
 * a restart (2), or a VRAM copy (words read from the read port and written back, from a plain address pair or, for 1,
 * a pair packed in a byte); AddToListWord steps the list word on. Other kinds: VramListFromIndex, then the channel freed.
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel1CF1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return run_list_channel(0x9049, kind, slot, y, c, v);
}

void abi_RunListChannel1CF1(void)
{
    channel_exit_shim(abi_run_list_channel(0x9049));
}
