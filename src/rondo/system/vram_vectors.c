/* The VRAM overlay's entry at $8006. */
#include "machine.h"
#include "vram_update.h"

/*
 * @routine 00C5:006 VecVramListRun
 * @run 6C:8006
 * WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
 * HOW: One jump, no frame of its own: the caller's JSR frame is the one VramListRun returns through.
 * WAITS: no.
 * Entry, exit: as VramListRun.
 */
VramExit VecVramListRun(uint8_t x, uint8_t y, bool carry, bool overflow)
{
    return VramListRun(x, y, carry, overflow);
}

void abi_VecVramListRun(void)
{
    abi_VramListRun();
}
