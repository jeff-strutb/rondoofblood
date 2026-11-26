/*
 * The first 24 bytes of the ending's effect overlay in the seven versions no read the game makes can load (sectors
 * 0441, 0651, 1231, 1A31, 1B91, 25F7, 1CF1): three entry jumps at $8000, $8003 and $8006 and RunAllListChannels at
 * $8009, which runs every list channel with a kind through RunListChannel (list_channels_census.c). The bytes are the
 * same in all seven; only the RunListChannel and EffectStep they reach are each version's own.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "vram_update.h"
#include "list_channels_census.h"
#include "effect_slots_census.h"
#include "effect_entry_census.h"

typedef ChannelExit (*ChannelRunner)(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);

/* What a run over the channels leaves, set on the registers (as list_channels_census.c's own). */
static void channel_exit_shim(ChannelExit exit)
{
    cpu.x = exit.x;
    if (exit.left) {
        abi_nz(exit.x);
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

/*
 * Every channel from 7 down to 0 whose kind is not zero goes through the version's RunListChannel, the channel kept
 * on the stack across the call. What it leaves: X $FF, N set, Z clear; A the last kind read or what the last channel
 * left; Y, C and V as the last channel that set them left them, else as they were.
 */
static ChannelExit run_all_channels(ChannelRunner run, uint8_t y, bool c, bool v)
{
    ChannelExit exit = { 0 };
    ChannelExit last;
    uint8_t slot, kind, a = 0;
    bool y_set = false, c_set = false, v_set = false;

    for (slot = 7; (int8_t)slot >= 0; slot--) {
        kind = RD8(LCH->kind[slot]);
        a = kind;
        if (kind != 0) {
            push8(slot);
            jsr_push(0x8013);
            last = run(kind, slot, y, c, v);
            jsr_pop();
            slot = pull8();
            a = last.a;
            if (last.y_set) {
                y = last.y;
                y_set = true;
            }
            if (last.c_set) {
                c = last.c;
                c_set = true;
            }
            if (last.v_set) {
                v = last.v;
                v_set = true;
            }
        }
    }
    exit.a = a;
    exit.x = 0xFF;
    exit.nz = 0xFF;
    exit.y = y;
    exit.y_set = y_set;
    exit.c = c;
    exit.c_set = c_set;
    exit.v = v;
    exit.v_set = v_set;
    return exit;
}

/*
 * @routine 0441:000 ListEntry0441
 * @run 6C:8000
 * @unreachable 6C:8000-6C:8002 no read the game makes loads disc sector 0441, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8000: goes to RunAllListChannels.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as RunAllListChannels.
 */
ChannelExit ListEntry0441(uint8_t y, bool c, bool v)
{
    return run_all_channels(RunListChannel0441, y, c, v);
}

void abi_ListEntry0441(void)
{
    channel_exit_shim(ListEntry0441(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0441:003 EffectEntry0441
 * @run 6C:8003
 * @unreachable 6C:8003-6C:8005 no read the game makes loads disc sector 0441, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8003: goes to EffectStep, the palette effects' frame.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as EffectStep.
 */
Exit EffectEntry0441(uint8_t y)
{
    return EffectStep0441(y);
}

void abi_EffectEntry0441(void)
{
    exit_shim(EffectEntry0441(cpu.y));
}

/*
 * @routine 0441:006 VramEntry0441
 * @run 6C:8006
 * @unreachable 6C:8006-6C:8008 no read the game makes loads disc sector 0441, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as VramListRun.
 */
VramExit VramEntry0441(uint8_t x, uint8_t y, bool carry, bool overflow)
{
    return VramListRun(x, y, carry, overflow);
}

void abi_VramEntry0441(void)
{
    abi_VramListRun();
}

/*
 * @routine 0441:009 RunAllListChannels0441
 * @run 6C:8009
 * @unreachable 6C:8009-6C:8017 no read the game makes loads disc sector 0441, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs the list channels: every channel from 7 down to 0 whose kind is not zero goes through
 * RunListChannel.
 * HOW: The kind read at $3C80 by X; the channel kept on the stack across the call.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last channel left; Y, C and V as the
 * last channel left them.
 */
ChannelExit RunAllListChannels0441(uint8_t y, bool c, bool v)
{
    return run_all_channels(RunListChannel0441, y, c, v);
}

void abi_RunAllListChannels0441(void)
{
    channel_exit_shim(RunAllListChannels0441(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0651:000 ListEntry0651
 * @run 6C:8000
 * @unreachable 6C:8000-6C:8002 no read the game makes loads disc sector 0651, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8000: goes to RunAllListChannels.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as RunAllListChannels.
 */
ChannelExit ListEntry0651(uint8_t y, bool c, bool v)
{
    return run_all_channels(RunListChannel0651, y, c, v);
}

void abi_ListEntry0651(void)
{
    channel_exit_shim(ListEntry0651(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0651:003 EffectEntry0651
 * @run 6C:8003
 * @unreachable 6C:8003-6C:8005 no read the game makes loads disc sector 0651, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8003: goes to EffectStep, the palette effects' frame.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as EffectStep.
 */
Exit EffectEntry0651(uint8_t y)
{
    return EffectStep0651(y);
}

void abi_EffectEntry0651(void)
{
    exit_shim(EffectEntry0651(cpu.y));
}

/*
 * @routine 0651:006 VramEntry0651
 * @run 6C:8006
 * @unreachable 6C:8006-6C:8008 no read the game makes loads disc sector 0651, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as VramListRun.
 */
VramExit VramEntry0651(uint8_t x, uint8_t y, bool carry, bool overflow)
{
    return VramListRun(x, y, carry, overflow);
}

void abi_VramEntry0651(void)
{
    abi_VramListRun();
}

/*
 * @routine 0651:009 RunAllListChannels0651
 * @run 6C:8009
 * @unreachable 6C:8009-6C:8017 no read the game makes loads disc sector 0651, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs the list channels: every channel from 7 down to 0 whose kind is not zero goes through
 * RunListChannel.
 * HOW: The kind read at $3C80 by X; the channel kept on the stack across the call.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last channel left; Y, C and V as the
 * last channel left them.
 */
ChannelExit RunAllListChannels0651(uint8_t y, bool c, bool v)
{
    return run_all_channels(RunListChannel0651, y, c, v);
}

void abi_RunAllListChannels0651(void)
{
    channel_exit_shim(RunAllListChannels0651(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 1231:000 ListEntry1231
 * @run 6C:8000
 * @unreachable 6C:8000-6C:8002 no read the game makes loads disc sector 1231, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8000: goes to RunAllListChannels.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as RunAllListChannels.
 */
ChannelExit ListEntry1231(uint8_t y, bool c, bool v)
{
    return run_all_channels(RunListChannel1231, y, c, v);
}

void abi_ListEntry1231(void)
{
    channel_exit_shim(ListEntry1231(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 1231:003 EffectEntry1231
 * @run 6C:8003
 * @unreachable 6C:8003-6C:8005 no read the game makes loads disc sector 1231, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8003: goes to EffectStep, the palette effects' frame.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as EffectStep.
 */
Exit EffectEntry1231(uint8_t y)
{
    return EffectStep1231(y);
}

void abi_EffectEntry1231(void)
{
    exit_shim(EffectEntry1231(cpu.y));
}

/*
 * @routine 1231:006 VramEntry1231
 * @run 6C:8006
 * @unreachable 6C:8006-6C:8008 no read the game makes loads disc sector 1231, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as VramListRun.
 */
VramExit VramEntry1231(uint8_t x, uint8_t y, bool carry, bool overflow)
{
    return VramListRun(x, y, carry, overflow);
}

void abi_VramEntry1231(void)
{
    abi_VramListRun();
}

/*
 * @routine 1231:009 RunAllListChannels1231
 * @run 6C:8009
 * @unreachable 6C:8009-6C:8017 no read the game makes loads disc sector 1231, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs the list channels: every channel from 7 down to 0 whose kind is not zero goes through
 * RunListChannel.
 * HOW: The kind read at $3C80 by X; the channel kept on the stack across the call.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last channel left; Y, C and V as the
 * last channel left them.
 */
ChannelExit RunAllListChannels1231(uint8_t y, bool c, bool v)
{
    return run_all_channels(RunListChannel1231, y, c, v);
}

void abi_RunAllListChannels1231(void)
{
    channel_exit_shim(RunAllListChannels1231(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 1A31:000 ListEntry1A31
 * @run 6C:8000
 * @unreachable 6C:8000-6C:8002 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8000: goes to RunAllListChannels.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as RunAllListChannels.
 */
ChannelExit ListEntry1A31(uint8_t y, bool c, bool v)
{
    return run_all_channels(RunListChannel1A31, y, c, v);
}

void abi_ListEntry1A31(void)
{
    channel_exit_shim(ListEntry1A31(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 1A31:003 EffectEntry1A31
 * @run 6C:8003
 * @unreachable 6C:8003-6C:8005 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8003: goes to EffectStep, the palette effects' frame.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as EffectStep.
 */
Exit EffectEntry1A31(uint8_t y)
{
    return EffectStep1A31(y);
}

void abi_EffectEntry1A31(void)
{
    exit_shim(EffectEntry1A31(cpu.y));
}

/*
 * @routine 1A31:006 VramEntry1A31
 * @run 6C:8006
 * @unreachable 6C:8006-6C:8008 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as VramListRun.
 */
VramExit VramEntry1A31(uint8_t x, uint8_t y, bool carry, bool overflow)
{
    return VramListRun(x, y, carry, overflow);
}

void abi_VramEntry1A31(void)
{
    abi_VramListRun();
}

/*
 * @routine 1A31:009 RunAllListChannels1A31
 * @run 6C:8009
 * @unreachable 6C:8009-6C:8017 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs the list channels: every channel from 7 down to 0 whose kind is not zero goes through
 * RunListChannel.
 * HOW: The kind read at $3C80 by X; the channel kept on the stack across the call.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last channel left; Y, C and V as the
 * last channel left them.
 */
ChannelExit RunAllListChannels1A31(uint8_t y, bool c, bool v)
{
    return run_all_channels(RunListChannel1A31, y, c, v);
}

void abi_RunAllListChannels1A31(void)
{
    channel_exit_shim(RunAllListChannels1A31(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 1B91:000 ListEntry1B91
 * @run 6C:8000
 * @unreachable 6C:8000-6C:8002 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8000: goes to RunAllListChannels.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as RunAllListChannels.
 */
ChannelExit ListEntry1B91(uint8_t y, bool c, bool v)
{
    return run_all_channels(RunListChannel1B91, y, c, v);
}

void abi_ListEntry1B91(void)
{
    channel_exit_shim(ListEntry1B91(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 1B91:003 EffectEntry1B91
 * @run 6C:8003
 * @unreachable 6C:8003-6C:8005 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8003: goes to EffectStep, the palette effects' frame.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as EffectStep.
 */
Exit EffectEntry1B91(uint8_t y)
{
    return EffectStep1B91(y);
}

void abi_EffectEntry1B91(void)
{
    exit_shim(EffectEntry1B91(cpu.y));
}

/*
 * @routine 1B91:006 VramEntry1B91
 * @run 6C:8006
 * @unreachable 6C:8006-6C:8008 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as VramListRun.
 */
VramExit VramEntry1B91(uint8_t x, uint8_t y, bool carry, bool overflow)
{
    return VramListRun(x, y, carry, overflow);
}

void abi_VramEntry1B91(void)
{
    abi_VramListRun();
}

/*
 * @routine 1B91:009 RunAllListChannels1B91
 * @run 6C:8009
 * @unreachable 6C:8009-6C:8017 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs the list channels: every channel from 7 down to 0 whose kind is not zero goes through
 * RunListChannel.
 * HOW: The kind read at $3C80 by X; the channel kept on the stack across the call.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last channel left; Y, C and V as the
 * last channel left them.
 */
ChannelExit RunAllListChannels1B91(uint8_t y, bool c, bool v)
{
    return run_all_channels(RunListChannel1B91, y, c, v);
}

void abi_RunAllListChannels1B91(void)
{
    channel_exit_shim(RunAllListChannels1B91(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 25F7:000 ListEntry25F7
 * @run 6C:8000
 * @unreachable 6C:8000-6C:8002 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8000: goes to RunAllListChannels.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as RunAllListChannels.
 */
ChannelExit ListEntry25F7(uint8_t y, bool c, bool v)
{
    return run_all_channels(RunListChannel25F7, y, c, v);
}

void abi_ListEntry25F7(void)
{
    channel_exit_shim(ListEntry25F7(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 25F7:003 EffectEntry25F7
 * @run 6C:8003
 * @unreachable 6C:8003-6C:8005 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8003: goes to EffectStep, the palette effects' frame.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as EffectStep.
 */
Exit EffectEntry25F7(uint8_t y)
{
    return EffectStep25F7(y);
}

void abi_EffectEntry25F7(void)
{
    exit_shim(EffectEntry25F7(cpu.y));
}

/*
 * @routine 25F7:006 VramEntry25F7
 * @run 6C:8006
 * @unreachable 6C:8006-6C:8008 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as VramListRun.
 */
VramExit VramEntry25F7(uint8_t x, uint8_t y, bool carry, bool overflow)
{
    return VramListRun(x, y, carry, overflow);
}

void abi_VramEntry25F7(void)
{
    abi_VramListRun();
}

/*
 * @routine 25F7:009 RunAllListChannels25F7
 * @run 6C:8009
 * @unreachable 6C:8009-6C:8017 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs the list channels: every channel from 7 down to 0 whose kind is not zero goes through
 * RunListChannel.
 * HOW: The kind read at $3C80 by X; the channel kept on the stack across the call.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last channel left; Y, C and V as the
 * last channel left them.
 */
ChannelExit RunAllListChannels25F7(uint8_t y, bool c, bool v)
{
    return run_all_channels(RunListChannel25F7, y, c, v);
}

void abi_RunAllListChannels25F7(void)
{
    channel_exit_shim(RunAllListChannels25F7(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 1CF1:000 ListEntry1CF1
 * @run 6C:8000
 * @unreachable 6C:8000-6C:8002 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8000: goes to RunAllListChannels.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as RunAllListChannels.
 */
ChannelExit ListEntry1CF1(uint8_t y, bool c, bool v)
{
    return run_all_channels(RunListChannel1CF1, y, c, v);
}

void abi_ListEntry1CF1(void)
{
    channel_exit_shim(ListEntry1CF1(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 1CF1:003 EffectEntry1CF1
 * @run 6C:8003
 * @unreachable 6C:8003-6C:8005 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8003: goes to EffectStep, the palette effects' frame.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as EffectStep.
 */
Exit EffectEntry1CF1(uint8_t y)
{
    return EffectStep1CF1(y);
}

void abi_EffectEntry1CF1(void)
{
    exit_shim(EffectEntry1CF1(cpu.y));
}

/*
 * @routine 1CF1:006 VramEntry1CF1
 * @run 6C:8006
 * @unreachable 6C:8006-6C:8008 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as VramListRun.
 */
VramExit VramEntry1CF1(uint8_t x, uint8_t y, bool carry, bool overflow)
{
    return VramListRun(x, y, carry, overflow);
}

void abi_VramEntry1CF1(void)
{
    abi_VramListRun();
}

/*
 * @routine 1CF1:009 RunAllListChannels1CF1
 * @run 6C:8009
 * @unreachable 6C:8009-6C:8017 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs the list channels: every channel from 7 down to 0 whose kind is not zero goes through
 * RunListChannel.
 * HOW: The kind read at $3C80 by X; the channel kept on the stack across the call.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last channel left; Y, C and V as the
 * last channel left them.
 */
ChannelExit RunAllListChannels1CF1(uint8_t y, bool c, bool v)
{
    return run_all_channels(RunListChannel1CF1, y, c, v);
}

void abi_RunAllListChannels1CF1(void)
{
    channel_exit_shim(RunAllListChannels1CF1(cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

