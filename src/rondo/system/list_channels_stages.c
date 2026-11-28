/* RunListChannel in the overlays of stages 00, 02 and 06 to 08: the same code as RunListChannel07D1 but for kind 3's
 * table of copy lists. */
#include "machine.h"
#include "routines.h"
#include "list_channels_census.h"
#include "list_channels.h"

/*
 * @routine 0251:018 RunListChannel0251
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $901F), any other runs one VRAM list and
 * frees the channel.
 * HOW: RunListChannelBody with the table at $901F, Stage 00's overlay (sector 0251).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel0251(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x901F, kind, slot, y, c, v);
}

void abi_RunListChannel0251(void)
{
    ChannelExitShim(RunListChannel0251(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0511:018 RunListChannel0511
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $9349), any other runs one VRAM list and
 * frees the channel.
 * HOW: RunListChannelBody with the table at $9349, Stage 02's overlay (sector 0511).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel0511(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x9349, kind, slot, y, c, v);
}

void abi_RunListChannel0511(void)
{
    ChannelExitShim(RunListChannel0511(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0A91:018 RunListChannel0A91
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8E25), any other runs one VRAM list and
 * frees the channel.
 * HOW: RunListChannelBody with the table at $8E25, Stage 06's overlay (sector 0A91).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel0A91(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8E25, kind, slot, y, c, v);
}

void abi_RunListChannel0A91(void)
{
    ChannelExitShim(RunListChannel0A91(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0BF1:018 RunListChannel0BF1
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8CA7), any other runs one VRAM list and
 * frees the channel.
 * HOW: RunListChannelBody with the table at $8CA7, Stage 07's overlay (sector 0BF1).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel0BF1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8CA7, kind, slot, y, c, v);
}

void abi_RunListChannel0BF1(void)
{
    ChannelExitShim(RunListChannel0BF1(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0D51:018 RunListChannel0D51
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8D0C), any other runs one VRAM list and
 * frees the channel.
 * HOW: RunListChannelBody with the table at $8D0C, Stage 08's overlay (sector 0D51).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel0D51(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8D0C, kind, slot, y, c, v);
}

void abi_RunListChannel0D51(void)
{
    ChannelExitShim(RunListChannel0D51(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}
