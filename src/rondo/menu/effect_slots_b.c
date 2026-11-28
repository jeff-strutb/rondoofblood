/*
 * The palette effects' kind 2 and kind 3 handlers in the overlays of stages $09 to $0E and the secret stages: the same
 * code as version 1A31's (effect_slots_census.c) but for the table of lists a negative list number picks.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "effect_slots_census.h"
#include "effect_slots_b.h"

/*
 * @routine 0721:362 EffectFade0721
 * @run 6C:8362
 * WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
 * overlay's table of lists for a negative list number ($89E7), Secret stage 03's overlay.
 * HOW: EffectFade1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectFade1A31.
 */
Exit EffectFade0721(uint8_t slot)
{
    return EffectFadeWithTable(0x89E7, slot);
}

void abi_EffectFade0721(void)
{
    exit_shim(EffectFade0721(cpu.x));
}

/*
 * @routine 0881:362 EffectFade0881
 * @run 6C:8362
 * WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
 * overlay's table of lists for a negative list number ($89E9), Secret stage 04's overlay.
 * HOW: EffectFade1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectFade1A31.
 */
Exit EffectFade0881(uint8_t slot)
{
    return EffectFadeWithTable(0x89E9, slot);
}

void abi_EffectFade0881(void)
{
    exit_shim(EffectFade0881(cpu.x));
}

/*
 * @routine 0B41:362 EffectFade0B41
 * @run 6C:8362
 * WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
 * overlay's table of lists for a negative list number ($8A01), Secret stage 06's overlay.
 * HOW: EffectFade1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectFade1A31.
 */
Exit EffectFade0B41(uint8_t slot)
{
    return EffectFadeWithTable(0x8A01, slot);
}

void abi_EffectFade0B41(void)
{
    exit_shim(EffectFade0B41(cpu.x));
}

/*
 * @routine 0EB1:362 EffectFade0EB1
 * @run 6C:8362
 * WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
 * overlay's table of lists for a negative list number ($89DF), Stage 09's overlay.
 * HOW: EffectFade1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectFade1A31.
 */
Exit EffectFade0EB1(uint8_t slot)
{
    return EffectFadeWithTable(0x89DF, slot);
}

void abi_EffectFade0EB1(void)
{
    exit_shim(EffectFade0EB1(cpu.x));
}

/*
 * @routine 10C1:362 EffectFade10C1
 * @run 6C:8362
 * WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
 * overlay's table of lists for a negative list number ($8A25), Secret stage 0A's overlay.
 * HOW: EffectFade1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectFade1A31.
 */
Exit EffectFade10C1(uint8_t slot)
{
    return EffectFadeWithTable(0x8A25, slot);
}

void abi_EffectFade10C1(void)
{
    exit_shim(EffectFade10C1(cpu.x));
}

/*
 * @routine 12D1:362 EffectFade12D1
 * @run 6C:8362
 * WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
 * overlay's table of lists for a negative list number ($8A0B), Stage 0C's overlay.
 * HOW: EffectFade1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectFade1A31.
 */
Exit EffectFade12D1(uint8_t slot)
{
    return EffectFadeWithTable(0x8A0B, slot);
}

void abi_EffectFade12D1(void)
{
    exit_shim(EffectFade12D1(cpu.x));
}

/*
 * @routine 1611:362 EffectFade1611
 * @run 6C:8362
 * WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
 * overlay's table of lists for a negative list number ($89E1), Stage 0E's overlay.
 * HOW: EffectFade1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectFade1A31.
 */
Exit EffectFade1611(uint8_t slot)
{
    return EffectFadeWithTable(0x89E1, slot);
}

void abi_EffectFade1611(void)
{
    exit_shim(EffectFade1611(cpu.x));
}

/*
 * @routine 16C1:362 EffectFade16C1
 * @run 6C:8362
 * WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
 * overlay's table of lists for a negative list number ($89F3), Secret stage 0E's overlay.
 * HOW: EffectFade1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectFade1A31.
 */
Exit EffectFade16C1(uint8_t slot)
{
    return EffectFadeWithTable(0x89F3, slot);
}

void abi_EffectFade16C1(void)
{
    exit_shim(EffectFade16C1(cpu.x));
}

/*
 * @routine 05C1:4FF EffectColour05C1
 * @run 6C:84FF
 * WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
 * negative list number ($89E5), Secret stage 02's overlay.
 * HOW: EffectColour1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectColour1A31.
 */
Exit EffectColour05C1(uint8_t slot)
{
    return EffectColourWithTable(0x89E5, slot);
}

void abi_EffectColour05C1(void)
{
    exit_shim(EffectColour05C1(cpu.x));
}

/*
 * @routine 0721:4FF EffectColour0721
 * @run 6C:84FF
 * WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
 * negative list number ($89E7), Secret stage 03's overlay.
 * HOW: EffectColour1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectColour1A31.
 */
Exit EffectColour0721(uint8_t slot)
{
    return EffectColourWithTable(0x89E7, slot);
}

void abi_EffectColour0721(void)
{
    exit_shim(EffectColour0721(cpu.x));
}

/*
 * @routine 0881:4FF EffectColour0881
 * @run 6C:84FF
 * WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
 * negative list number ($89E9), Secret stage 04's overlay.
 * HOW: EffectColour1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectColour1A31.
 */
Exit EffectColour0881(uint8_t slot)
{
    return EffectColourWithTable(0x89E9, slot);
}

void abi_EffectColour0881(void)
{
    exit_shim(EffectColour0881(cpu.x));
}

/*
 * @routine 0B41:4FF EffectColour0B41
 * @run 6C:84FF
 * WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
 * negative list number ($8A01), Secret stage 06's overlay.
 * HOW: EffectColour1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectColour1A31.
 */
Exit EffectColour0B41(uint8_t slot)
{
    return EffectColourWithTable(0x8A01, slot);
}

void abi_EffectColour0B41(void)
{
    exit_shim(EffectColour0B41(cpu.x));
}

/*
 * @routine 0EB1:4FF EffectColour0EB1
 * @run 6C:84FF
 * WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
 * negative list number ($89DF), Stage 09's overlay.
 * HOW: EffectColour1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectColour1A31.
 */
Exit EffectColour0EB1(uint8_t slot)
{
    return EffectColourWithTable(0x89DF, slot);
}

void abi_EffectColour0EB1(void)
{
    exit_shim(EffectColour0EB1(cpu.x));
}

/*
 * @routine 10C1:4FF EffectColour10C1
 * @run 6C:84FF
 * WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
 * negative list number ($8A25), Secret stage 0A's overlay.
 * HOW: EffectColour1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectColour1A31.
 */
Exit EffectColour10C1(uint8_t slot)
{
    return EffectColourWithTable(0x8A25, slot);
}

void abi_EffectColour10C1(void)
{
    exit_shim(EffectColour10C1(cpu.x));
}

/*
 * @routine 12D1:4FF EffectColour12D1
 * @run 6C:84FF
 * WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
 * negative list number ($8A0B), Stage 0C's overlay.
 * HOW: EffectColour1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectColour1A31.
 */
Exit EffectColour12D1(uint8_t slot)
{
    return EffectColourWithTable(0x8A0B, slot);
}

void abi_EffectColour12D1(void)
{
    exit_shim(EffectColour12D1(cpu.x));
}

/*
 * @routine 1611:4FF EffectColour1611
 * @run 6C:84FF
 * WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
 * negative list number ($89E1), Stage 0E's overlay.
 * HOW: EffectColour1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectColour1A31.
 */
Exit EffectColour1611(uint8_t slot)
{
    return EffectColourWithTable(0x89E1, slot);
}

void abi_EffectColour1611(void)
{
    exit_shim(EffectColour1611(cpu.x));
}

/*
 * @routine 16C1:4FF EffectColour16C1
 * @run 6C:84FF
 * WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
 * negative list number ($89F3), Secret stage 0E's overlay.
 * HOW: EffectColour1A31's body.
 * WAITS: no.
 * Entry: X the slot. Exit: as EffectColour1A31.
 */
Exit EffectColour16C1(uint8_t slot)
{
    return EffectColourWithTable(0x89F3, slot);
}

void abi_EffectColour16C1(void)
{
    exit_shim(EffectColour16C1(cpu.x));
}
