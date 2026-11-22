/* The BIOS's math entries: bank 0 maps bank 3, where the routines lie, at $C000. */
#include "card.h"
#include "card_access.h"
#include "syscard.h"

enum { MATH_BANK = 3 };

/*
 * @card 00:FDA9 map_math_bank
 * WHAT IT DOES: Maps the math routines at $C000, keeping what was there.
 * HOW: A is pushed, MPR6 goes to CardSavedMpr6, bank 3 to MPR6, A is pulled.
 * WAITS: no.
 * Entry: A (only kept: it goes on the stack and comes back). Exit: A as given, N and Z from it. X, Y,
 * C, V kept. Returns the byte pulled, which is A.
 */
static uint8_t map_math_bank(uint8_t a)
{
    push8(a);
    WR8(CRAM->saved_mpr6, tma(MPR6));
    tam(MPR6, MATH_BANK);
    return pull8();
}

void abi_map_math_bank(void)
{
    cpu.a = map_math_bank(cpu.a);
    abi_nz(cpu.a);
}

/* What unmap_math_bank leaves in A and the flags: the bytes it pulled. */
typedef struct {
    uint8_t a;
    uint8_t flags;
} KeptRegisters;

/*
 * @card 00:FDB5 unmap_math_bank
 * WHAT IT DOES: Puts back what map_math_bank kept.
 * HOW: The flags and A are pushed, CardSavedMpr6 goes to MPR6, A and the flags are pulled.
 * WAITS: no.
 * Entry: A and the flags, which are only kept. Exit: all registers kept, the flags as pushed (T clear,
 * B set).
 */
static KeptRegisters unmap_math_bank(uint8_t a, uint8_t flags)
{
    KeptRegisters kept;
    uint8_t saved;

    push8(php_byte(flags));
    push8(a);
    saved = RD8(CRAM->saved_mpr6);
    tam(MPR6, saved);
    kept.a = pull8();
    kept.flags = pull8();
    return kept;
}

void abi_unmap_math_bank(void)
{
    KeptRegisters kept = unmap_math_bank(cpu.a, cpu.p);

    cpu.a = kept.a;
    cpu.p = kept.flags;
}

/* What mul_u8 made, and what its adds left in V and C. */
typedef struct {
    uint16_t product;
    bool added;     /* an add ran: V is its overflow */
    bool overflow;  /* the last add's V */
    bool carry;     /* C at the end: the last round's add carried */
} Mul8;

enum { MUL_ROUNDS = 8 };

/*
 * @card 03:CD3A mul_u8
 * WHAT IT DOES: Multiplies two unsigned bytes for a 16-bit product in _CX.
 * HOW: Shift and add over eight rounds, the multiplier in the high register moving out as the product
 * moves in. _CL takes the multiplicand first, is read again at every add, and then takes the product's
 * high and low bytes. Decimal mode clear.
 * WAITS: no.
 * Entry: _AL, _BL. Exit: A the low byte, X the high, Y 0, N 0, Z 1; C from the last round's add, V from
 * the last add (kept if there was none).
 */
static Mul8 mul_u8(void)
{
    Mul8 m = { 0, false, false, false };
    uint8_t al = RD8(CZP->a.lo);
    uint8_t bl = RD8(CZP->b.lo);
    uint8_t low = 0;
    uint8_t high = bl;

    WR8(CZP->c.lo, al);
    for (unsigned round = 0; round < MUL_ROUNDS; round++) {
        bool shifted = (low & 0x80) != 0;
        bool bit;

        low = (uint8_t)(low << 1);
        bit = (high & 0x80) != 0;
        high = (uint8_t)((high << 1) | (shifted ? 1 : 0));
        m.carry = false;
        if (bit) {
            uint8_t addend = RD8(CZP->c.lo);
            unsigned sum = (unsigned)low + addend;

            m.added = true;
            m.overflow = (~(low ^ addend) & (low ^ sum) & 0x80) != 0;
            m.carry = sum > 0xFF;
            low = (uint8_t)sum;
            if (m.carry) {
                high++;
            }
        }
    }
    WR8(CZP->c.hi, high);
    WR8(CZP->c.lo, low);
    m.product = (uint16_t)(low | (high << 8));
    return m;
}

/* The flags mul_u8 leaves, from the ones it was entered with: N clear, Z set, C and V as it made them. */
static uint8_t mul_flags(uint8_t flags, Mul8 m)
{
    uint8_t p = (uint8_t)((flags & ~(P_N | P_C)) | P_Z);

    if (m.carry) {
        p |= P_C;
    }
    if (m.added) {
        p = (uint8_t)((p & ~P_V) | (m.overflow ? P_V : 0));
    }
    return p;
}

void abi_mul_u8(void)
{
    Mul8 m = mul_u8();

    cpu.a = (uint8_t)m.product;
    cpu.x = (uint8_t)(m.product >> 8);
    cpu.y = 0;
    cpu.p = mul_flags(cpu.p, m);
}

/*
 * @card 00:FDC6 ma_mul8u
 * WHAT IT DOES: The BIOS's unsigned byte multiply.
 * HOW: Maps the math bank, mul_u8, puts it back (a tail jump to the unmap).
 * WAITS: no.
 * Entry: _AL, _BL; A and the flags are only kept (A goes on the stack in the map). Exit: _CX the product,
 * A its low byte, X its high, Y 0, and the flags as mul_u8 leaves them, as pushed (T clear, B set).
 */
MaMul8u ma_mul8u(uint8_t a, uint8_t flags)
{
    MaMul8u out;
    Mul8 m;
    KeptRegisters kept;

    jsr_push(0xFDC8);
    (void)map_math_bank(a);
    jsr_pop();
    jsr_push(0xFDCB);
    m = mul_u8();
    jsr_pop();
    kept = unmap_math_bank((uint8_t)m.product, mul_flags(flags, m));
    out.product = m.product;
    out.a = kept.a;
    out.flags = kept.flags;
    return out;
}

/* ma_mul8u's exit: A, X, Y and the flags it leaves. Also the exit of a caller that ends as the callee left it. */
void ma_mul8u_exit_shim(MaMul8u result)
{
    cpu.a = result.a;
    cpu.x = (uint8_t)(result.product >> 8);
    cpu.y = 0;
    cpu.p = result.flags;
}

void abi_ma_mul8u(void)
{
    ma_mul8u_exit_shim(ma_mul8u(cpu.a, cpu.p));
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
