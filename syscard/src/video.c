/* The BIOS's video setters. */
#include "card.h"
#include "syscard.h"

enum {
    VDC_REG_MWR = 0x09,
    MWR_SIZE_BITS = 0x70, MWR_DOT_BITS = 0x8F, SCREEN_SIZE_MASK = 0x07
};

/* Sets the bits of the VDC's memory width register, and keeps a copy. */
static uint8_t set_memory_width(uint8_t keep, uint8_t bits)
{
    uint8_t width = (uint8_t)((rd8(CardVdcMwr) & keep) | bits);

    wr8(CardVdcMwr, width);
    wr8(CardVdcReg, VDC_REG_MWR);
    wr8(VdcAddress, VDC_REG_MWR);
    wr8(VdcDataLo, width);
    return width;
}

static uint8_t size_bits(uint8_t size)
{
    return (uint8_t)((size & SCREEN_SIZE_MASK) << 4);
}

/*
 * @card 00:E267 ex_scrsiz
 * WHAT IT DOES: Sets the size of the background map.
 * HOW: The size's three bits go to bits 4-6 of the memory width register.
 * Entry: A size. Exit: A the register, N and Z from it, Y the bits, C clear. X and V kept.
 */
uint8_t ex_scrsiz(uint8_t size)
{
    return set_memory_width(MWR_DOT_BITS, size_bits(size));
}

void abi_ex_scrsiz(void)
{
    uint8_t bits = size_bits(cpu.a);
    uint8_t width = ex_scrsiz(cpu.a);

    cpu.a = width;
    cpu.y = bits;
    abi_nz(width);
    abi_flag(P_C, false);
}

/*
 * @card 00:E272 ex_dotmod
 * WHAT IT DOES: Sets the VRAM dot width and sprite width of the memory width register.
 * HOW: The mode's bits outside the map size go to the register.
 * Entry: A mode. Exit: A the register, N and Z from it, Y the bits. X, C, V kept.
 */
uint8_t ex_dotmod(uint8_t mode)
{
    return set_memory_width(MWR_SIZE_BITS, mode & MWR_DOT_BITS);
}

void abi_ex_dotmod(void)
{
    uint8_t bits = cpu.a & MWR_DOT_BITS;
    uint8_t width = ex_dotmod(cpu.a);

    cpu.a = width;
    cpu.y = bits;
    abi_nz(width);
}
