/*
 * RunListChannel ($8018 to $8132) in the overlays a scene loads: the same code as the census versions
 * (menu/list_channels_census.c) but for kind 3's table of copy lists, so each runs RunListChannelBody with its
 * own table. Seven versions; the overlays at 0111 and 13F1 hold the same bytes as 00C5 and 12D1 (0111's copy is filed in
 * menu/front_end_copies.c). AddToListWord's bytes at $8133 are its own routine (layers.c), hence @stop 8133.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "list_channels_census.h"
#include "list_channels.h"

/*
 * @routine 00C5:018 RunListChannel00C5
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $829F), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $829F, the stage overlay at sector 00C5 (0111, the ending's, holds the
 * same bytes).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel00C5(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x829F, kind, slot, y, c, v);
}

void abi_RunListChannel00C5(void)
{
    ChannelExitShim(RunListChannel00C5(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 07D1:018 RunListChannel07D1
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $91F2), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $91F2, sector 07D1's overlay.
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel07D1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x91F2, kind, slot, y, c, v);
}

void abi_RunListChannel07D1(void)
{
    ChannelExitShim(RunListChannel07D1(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0931:018 RunListChannel0931
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8DEC), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $8DEC, sector 0931's overlay.
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel0931(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8DEC, kind, slot, y, c, v);
}

void abi_RunListChannel0931(void)
{
    ChannelExitShim(RunListChannel0931(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 09E1:018 RunListChannel09E1
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8FEA), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $8FEA, sector 09E1's overlay.
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel09E1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8FEA, kind, slot, y, c, v);
}

void abi_RunListChannel09E1(void)
{
    ChannelExitShim(RunListChannel09E1(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 0B41:018 RunListChannel0B41
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8DD8), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $8DD8, sector 0B41's overlay.
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel0B41(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8DD8, kind, slot, y, c, v);
}

void abi_RunListChannel0B41(void)
{
    ChannelExitShim(RunListChannel0B41(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 1611:018 RunListChannel1611
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8F06), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $8F06, sector 1611's overlay.
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel1611(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8F06, kind, slot, y, c, v);
}

void abi_RunListChannel1611(void)
{
    ChannelExitShim(RunListChannel1611(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}

/*
 * @routine 12D1:018 RunListChannel12D1
 * @run 6C:8018
 * @stop 8133
 * WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
 * between them, 3 steps a list of VRAM copies (its lists from the table at $8F7B), any other runs one VRAM list
 * and frees the channel.
 * HOW: RunListChannelBody with the table at $8F7B, sector 12D1 (13F1 holds the same bytes).
 * WAITS: no.
 * Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
 */
ChannelExit RunListChannel12D1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannelBody(0x8F7B, kind, slot, y, c, v);
}

void abi_RunListChannel12D1(void)
{
    ChannelExitShim(RunListChannel12D1(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0));
}
