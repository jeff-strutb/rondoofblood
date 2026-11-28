/*
 * RunListChannel ($8018 to $8132) in the stage overlays of stages $09, $0A, $0B and $10 and of the secret stages:
 * the same code as RunListChannel07D1 (list_channels.c) but for kind 3's table of copy lists, so each runs
 * RunListChannelBody with its own table. AddToListWord's bytes at $8133 are their own routine, hence @stop 8133.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "list_channels_census.h"
#include "list_channels_b.h"

/*
 * @routine 05C1:018 RunListChannel05C1
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8E4B), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $8E4B, Secret stage 02's overlay (sector 05C1).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel05C1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8E4B, kind, slot, y, c, v);
}

void abi_RunListChannel05C1(void)
{
    ChannelExitShim(RunListChannel05C1(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0721:018 RunListChannel0721
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8E95), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $8E95, Secret stage 03's overlay (sector 0721).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel0721(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8E95, kind, slot, y, c, v);
}

void abi_RunListChannel0721(void)
{
    ChannelExitShim(RunListChannel0721(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0881:018 RunListChannel0881
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $901E), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $901E, Secret stage 04's overlay (sector 0881).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel0881(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x901E, kind, slot, y, c, v);
}

void abi_RunListChannel0881(void)
{
    ChannelExitShim(RunListChannel0881(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0E01:018 RunListChannel0E01
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8D2E), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $8D2E, Secret stage 08's overlay (sector 0E01).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel0E01(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8D2E, kind, slot, y, c, v);
}

void abi_RunListChannel0E01(void)
{
    ChannelExitShim(RunListChannel0E01(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0EB1:018 RunListChannel0EB1
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8FBB), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $8FBB, Stage 09's overlay (sector 0EB1).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel0EB1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8FBB, kind, slot, y, c, v);
}

void abi_RunListChannel0EB1(void)
{
    ChannelExitShim(RunListChannel0EB1(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0F61:018 RunListChannel0F61
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8D2E), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $8D2E, Secret stage 09's overlay (sector 0F61).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel0F61(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8D2E, kind, slot, y, c, v);
}

void abi_RunListChannel0F61(void)
{
    ChannelExitShim(RunListChannel0F61(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 1011:018 RunListChannel1011
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8F58), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $8F58, Stage 0A's overlay (sector 1011).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel1011(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8F58, kind, slot, y, c, v);
}

void abi_RunListChannel1011(void)
{
    ChannelExitShim(RunListChannel1011(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 10C1:018 RunListChannel10C1
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8E56), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $8E56, Secret stage 0A's overlay (sector 10C1).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel10C1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8E56, kind, slot, y, c, v);
}

void abi_RunListChannel10C1(void)
{
    ChannelExitShim(RunListChannel10C1(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 1171:018 RunListChannel1171
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $9049), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $9049, Stage 0B's overlay (sector 1171).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel1171(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x9049, kind, slot, y, c, v);
}

void abi_RunListChannel1171(void)
{
    ChannelExitShim(RunListChannel1171(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 16C1:018 RunListChannel16C1
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8FEB), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $8FEB, Secret stage 0E's overlay (sector 16C1).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel16C1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8FEB, kind, slot, y, c, v);
}

void abi_RunListChannel16C1(void)
{
    ChannelExitShim(RunListChannel16C1(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 18D1:018 RunListChannel18D1
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $92F6), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $92F6, Stage 10's overlay (sector 18D1).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel18D1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x92F6, kind, slot, y, c, v);
}

void abi_RunListChannel18D1(void)
{
    ChannelExitShim(RunListChannel18D1(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}
