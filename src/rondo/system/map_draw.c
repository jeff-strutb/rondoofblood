/* Map drawing: the VDC's data port as a word to mask in place. */
#include "machine.h"
#include "access.h"
#include "routines.h"
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

/* Both VRAM addresses, read then write, set to one word; the data port selected. */
static void set_vram_addresses(uint8_t lo, uint8_t hi, uint16_t first_ret, uint16_t second_ret)
{
    jsr_push(first_ret);
    SelectVramReadAddress();
    jsr_pop();
    WR8(VDC->data_lo, lo);
    WR8(VDC->data_hi, hi);
    jsr_push(second_ret);
    SelectVramWriteAddress();
    jsr_pop();
    WR8(VDC->data_lo, lo);
    WR8(VDC->data_hi, hi);
    SelectVramData();
}

/*
 * @routine 024F:37D SetVramAddresses
 * @run 6B:937D
 * WHAT IT DOES: Points both of the VDC's VRAM addresses, the one it reads from and the one it
 * writes to, at the same word, and selects the data port, so a word can be read and written
 * back in place (AndVramData, OrVramData).
 * HOW: Read address register, then X and Y into the data port; the write address register the
 * same; then the data port register, which this routine runs on into with no return of its own.
 * WAITS: no.
 * Entry: X low byte, Y high byte of the VRAM address. Exit: A 2, N 0, Z 0. X and Y kept.
 */
void SetVramAddresses(uint8_t lo, uint8_t hi)
{
    set_vram_addresses(lo, hi, 0x9380, 0x9389);
}

void abi_SetVramAddresses(void)
{
    SetVramAddresses(cpu.x, cpu.y);
    cpu.a = 2;
    abi_nz(2);
}

/*
 * @routine 05BF:68F SetVramAddressesLong
 * @run 6B:968F
 * WHAT IT DOES: SetVramAddresses, as the stages whose overlay runs it at $968F have it.
 * HOW: The same code at another run address, so its two calls return to $9692 and $969B.
 * WAITS: no.
 * Entry: X low byte, Y high byte of the VRAM address. Exit: as SetVramAddresses.
 */
void SetVramAddressesLong(uint8_t lo, uint8_t hi)
{
    set_vram_addresses(lo, hi, 0x9692, 0x969B);
}

void abi_SetVramAddressesLong(void)
{
    SetVramAddressesLong(cpu.x, cpu.y);
    cpu.a = 2;
    abi_nz(2);
}

/* The VRAM address the tile row step zTemp04 (rounded down to even) names: its two bytes, low then high. */
typedef struct { uint8_t lo, hi; } VramAddress;

static VramAddress stage_tile_address(void)
{
    VramAddress address;
    uint8_t step = RD8(ZP->temp04);
    uint8_t index = (uint8_t)(step & 0xFE);

    address.lo = rd8((uint16_t)(StageTileVramAddress + index));
    address.hi = rd8((uint16_t)(StageTileVramAddress + 1 + index));
    return address;
}

/*
 * @routine 024E:687 SetStageTileVramAddress
 * @run 6B:8E87
 * WHAT IT DOES: Points the VDC's VRAM addresses at the tile row the stage's tile step names:
 * one of the eight words of StageTileVramAddress, picked by zTemp04 rounded down to even.
 * HOW: The word into SetVramAddresses, which this routine jumps to with no frame of its own.
 * WAITS: no.
 * Entry: zTemp04 the step. Exit: as SetVramAddresses. X and Y the address bytes.
 */
static VramAddress set_stage_tile(void)
{
    VramAddress address = stage_tile_address();

    SetVramAddresses(address.lo, address.hi);
    return address;
}

void SetStageTileVramAddress(void)
{
    set_stage_tile();
}

void abi_SetStageTileVramAddress(void)
{
    VramAddress address = set_stage_tile();

    cpu.x = address.lo;
    cpu.y = address.hi;
    cpu.a = 2;
    abi_nz(2);
}

/*
 * @routine 05BE:687 SetStageTileVramAddressLong
 * @run 6B:8E87
 * WHAT IT DOES: SetStageTileVramAddress for the stages whose overlay has its SetVramAddresses at
 * $968F.
 * HOW: The same code, jumping to SetVramAddressesLong.
 * WAITS: no.
 * Entry: zTemp04 the step. Exit: as SetVramAddressesLong. X and Y the address bytes.
 */
static VramAddress set_stage_tileLong(void)
{
    VramAddress address = stage_tile_address();

    SetVramAddressesLong(address.lo, address.hi);
    return address;
}

void SetStageTileVramAddressLong(void)
{
    set_stage_tileLong();
}

void abi_SetStageTileVramAddressLong(void)
{
    VramAddress address = set_stage_tileLong();

    cpu.x = address.lo;
    cpu.y = address.hi;
    cpu.a = 2;
    abi_nz(2);
}
