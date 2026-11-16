/* The BIOS's math entries: bank 0 maps bank 3, where the routines lie, at $C000. */
#include "card.h"
#include "syscard.h"

enum { MATH_BANK = 3 };

/*
 * @card 00:FDA9 map_math_bank
 * WHAT IT DOES: Maps the math routines at $C000, keeping what was there.
 * HOW: MPR6 goes to CardSavedMpr6, then bank 3 to MPR6.
 * Entry: nothing. Exit: A kept, N and Z from it. X, Y, C, V kept.
 */
void map_math_bank(void)
{
    wr8(CardSavedMpr6, tma(MPR6));
    tam(MPR6, MATH_BANK);
}

void abi_map_math_bank(void)
{
    map_math_bank();
    abi_nz(cpu.a);
}

/*
 * @card 00:FDB5 unmap_math_bank
 * WHAT IT DOES: Puts back what map_math_bank kept.
 * HOW: CardSavedMpr6 to MPR6, with A and the flags saved around it.
 * Entry: nothing. Exit: all registers and flags kept.
 */
void unmap_math_bank(void)
{
    tam(MPR6, rd8(CardSavedMpr6));
}

void abi_unmap_math_bank(void)
{
    unmap_math_bank();
}

/*
 * @card 03:CD3A mul_u8
 * WHAT IT DOES: Multiplies two unsigned bytes for a 16-bit product in _CX.
 * HOW: Shift and add over eight rounds in X:A, the multiplier in X moving out as the
 * product moves in.
 * Entry: _AL, _BL. Exit: A the low byte, X the high, Y 0, N 0, Z 1; C and V from the add
 * of the lowest set bit of _BL, C clear if that was not bit 0, V kept if _BL is zero.
 */
uint16_t mul_u8(uint8_t al, uint8_t bl)
{
    uint16_t product = (uint16_t)(al * bl);

    wr8(CardCl, (uint8_t)product);
    wr8(CardCh, (uint8_t)(product >> 8));
    return product;
}

/* The last add was for the lowest set bit: all the bits above it had gone in before. */
static void mul_exit_shim(uint8_t al, uint8_t bl, uint16_t product)
{
    cpu.a = (uint8_t)product;
    cpu.x = (uint8_t)(product >> 8);
    cpu.y = 0;
    abi_nz(0);
    abi_flag(P_C, false);
    if (bl != 0) {
        unsigned lowest = 0;
        uint8_t before;
        uint8_t sum;

        while (!(bl & (1u << lowest)))
            lowest++;
        before = (uint8_t)(al * (bl >> (lowest + 1)) * 2);
        sum = (uint8_t)(before + al);
        abi_flag(P_V, (~(before ^ al) & (before ^ sum) & 0x80) != 0);
        abi_flag(P_C, lowest == 0 && (unsigned)before + al > 0xFF);
    }
}

void abi_mul_u8(void)
{
    uint8_t al = rd8(CardAl);
    uint8_t bl = rd8(CardBl);

    mul_exit_shim(al, bl, mul_u8(al, bl));
}

/*
 * @card 00:FDC6 ma_mul8u
 * WHAT IT DOES: The BIOS's unsigned byte multiply.
 * HOW: Maps the math bank, mul_u8, puts it back.
 * Entry: _AL, _BL. Exit: as mul_u8.
 */
uint16_t ma_mul8u(uint8_t al, uint8_t bl)
{
    uint16_t product;

    wr8(CardAl, al);
    wr8(CardBl, bl);
    map_math_bank();
    product = mul_u8(al, bl);
    unmap_math_bank();
    return product;
}

void abi_ma_mul8u(void)
{
    uint8_t al = rd8(CardAl);
    uint8_t bl = rd8(CardBl);

    mul_exit_shim(al, bl, ma_mul8u(al, bl));
}

/* ma_mul8u's exit, for a caller's wrapper that ends in it: given the two bytes and the product. */
void ma_mul8u_exit_shim(uint8_t al, uint8_t bl, uint16_t product)
{
    mul_exit_shim(al, bl, product);
}

enum { BCD_DIGIT_LIMIT = 0x0A, BCD_HIGH_LIMIT = 0xA0, BCD_LOW_DIGIT = 0x0F, BCD_HIGH_DIGIT = 0xF0,
       BINBCD_LIMIT = 100 };

/*
 * @card 00:E600 ex_bcdbin
 * WHAT IT DOES: Turns a two-digit BCD byte into binary.
 * HOW: Ten times the high digit, made as twice it plus eight times it in CardPointerLo, plus the
 * low digit. A digit of ten or more gives the byte back unchanged, with C set.
 * Entry: A. Exit: A the result, C clear, V clear; or A as given, C set, and V kept if the high
 * digit was bad, clear if the low one was. N and Z from X, which is kept.
 */
BcdConvert ex_bcdbin(uint8_t bcd)
{
    BcdConvert out = { false, bcd, 0 };

    if ((bcd & BCD_HIGH_DIGIT) >= BCD_HIGH_LIMIT)
        return out;
    out.tens = (uint8_t)(((bcd & BCD_HIGH_DIGIT) >> 1) + ((bcd & BCD_HIGH_DIGIT) >> 3));
    wr8(CardPointerLo, out.tens);
    if ((bcd & BCD_LOW_DIGIT) >= BCD_DIGIT_LIMIT)
        return out;
    out.converted = true;
    out.value = (uint8_t)(out.tens + (bcd & BCD_LOW_DIGIT));
    return out;
}

void abi_ex_bcdbin(void)
{
    uint8_t high_bad = (cpu.a & BCD_HIGH_DIGIT) >= BCD_HIGH_LIMIT;
    BcdConvert out = ex_bcdbin(cpu.a);

    cpu.a = out.value;
    abi_nz(cpu.x);
    abi_flag(P_C, !out.converted);
    if (!high_bad)
        abi_flag(P_V, false);
}

/*
 * @card 00:E621 ex_binbcd
 * WHAT IT DOES: Turns a binary byte below 100 into two BCD digits.
 * HOW: Takes tens off counting them, then puts the count in the high digit; the units go
 * through CardPointerLo. 100 or more gives the byte back with C set.
 * Entry: A. Exit: A the result, C clear, V clear, N and Z from X, which is kept. 100 or more: A
 * kept, C set, N and Z from A less 100. V kept.
 */
BcdConvert ex_binbcd(uint8_t value)
{
    BcdConvert out = { false, value, 0 };
    uint8_t units;

    if (value >= BINBCD_LIMIT)
        return out;
    out.tens = value / 10;
    units = value % 10;
    wr8(CardPointerLo, units);
    out.converted = true;
    out.value = (uint8_t)((out.tens << 4) | units);
    return out;
}

void abi_ex_binbcd(void)
{
    uint8_t given = cpu.a;
    BcdConvert out = ex_binbcd(given);

    cpu.a = out.value;
    abi_flag(P_C, !out.converted);
    if (out.converted) {
        abi_nz(cpu.x);
        abi_flag(P_V, false);
    } else {
        abi_nz((uint8_t)(given - BINBCD_LIMIT));
    }
}
