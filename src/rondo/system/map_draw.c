/* Map drawing: the VDC's data port as a word to mask in place. */
#include "machine.h"
#include "access.h"
#include "stage_map.h"

/*
 * @routine 024F:392 AndVramData
 * @run 6B:9392
 * WHAT IT DOES: Clears bits of the VRAM word under the VDC's address: the word the caller
 * has selected for reading and writing at once is ANDed with a mask in place.
 * HOW: Each byte of the data port is read, ANDed with its mask byte, and written back, low
 * byte first, then high byte. The caller has selected the port's register.
 * WAITS: no.
 * Entry: X low mask, Y high mask. Exit: A the high byte written, N and Z from it. X and Y kept.
 */
uint8_t AndVramData(uint8_t lo_mask, uint8_t hi_mask)
{
    uint8_t byte;

    byte = RD8(VDC->data_lo);
    byte = (uint8_t)(lo_mask & byte);
    WR8(VDC->data_lo, byte);
    byte = RD8(VDC->data_hi);
    byte = (uint8_t)(hi_mask & byte);
    WR8(VDC->data_hi, byte);
    return byte;
}

void abi_AndVramData(void)
{
    cpu.a = AndVramData(cpu.x, cpu.y);
    abi_nz(cpu.a);
}

/*
 * @routine 024F:3A1 OrVramData
 * @run 6B:93A1
 * WHAT IT DOES: Sets bits of the VRAM word under the VDC's address: the word the caller
 * has selected for reading and writing at once is ORed with a mask in place.
 * HOW: As AndVramData, with OR.
 * WAITS: no.
 * Entry: X low mask, Y high mask. Exit: A the high byte written, N and Z from it. X and Y kept.
 */
uint8_t OrVramData(uint8_t lo_mask, uint8_t hi_mask)
{
    uint8_t byte;

    byte = RD8(VDC->data_lo);
    byte = (uint8_t)(lo_mask | byte);
    WR8(VDC->data_lo, byte);
    byte = RD8(VDC->data_hi);
    byte = (uint8_t)(hi_mask | byte);
    WR8(VDC->data_hi, byte);
    return byte;
}

void abi_OrVramData(void)
{
    cpu.a = OrVramData(cpu.x, cpu.y);
    abi_nz(cpu.a);
}

/*
 * @routine 05BF:6A4 AndVramDataLong
 * @run 6B:96A4
 * WHAT IT DOES: AndVramData, as the stages whose overlay runs it at $96A4 have it.
 * HOW: The same bytes as AndVramData at another run address. Its C is AndVramData.
 * WAITS: no.
 * Entry: X low mask, Y high mask. Exit: as AndVramData.
 */
void abi_AndVramDataLong(void)
{
    abi_AndVramData();
}

/*
 * @routine 05BF:6B3 OrVramDataLong
 * @run 6B:96B3
 * WHAT IT DOES: OrVramData, as the stages whose overlay runs it at $96B3 have it.
 * HOW: The same bytes as OrVramData at another run address. Its C is OrVramData.
 * WAITS: no.
 * Entry: X low mask, Y high mask. Exit: as OrVramData.
 */
void abi_OrVramDataLong(void)
{
    abi_OrVramData();
}
