/*
 * The machine as the card's C sees it. Names of RAM and ports are in card_ram.h, generated
 * from syscard/config/symbols.csv.
 *
 * rd8 and wr8 take a logical address, as the CPU does. cpu is touched only by abi_
 * wrappers and *_shim functions, the place that meets the assembly's calling convention.
 */
#ifndef CARD_H
#define CARD_H

#include <stdbool.h>
#include <stdint.h>

#include "card_ram.h"
#include "syscard.h"

typedef struct {
    uint8_t a, x, y, s, p;
} Cpu;

extern Cpu cpu;

enum { P_C = 0x01, P_Z = 0x02, P_I = 0x04, P_D = 0x08, P_B = 0x10, P_T = 0x20, P_V = 0x40, P_N = 0x80 };

uint8_t rd8(uint16_t addr);
void wr8(uint16_t addr, uint8_t value);
uint8_t tma(uint8_t mpr_bit);
void tam(uint8_t mpr_bits, uint8_t bank);
void cli(void);
void csl(void); /* the CPU to its slow clock, as CSL; it stays so for the caller */
void csh(void); /* the CPU to its fast clock, as CSH */

/* The emulated stack, at $2100 + S. */
static inline void push8(uint8_t value)
{
    wr8((uint16_t)(0x2100 + cpu.s), value);
    cpu.s = (uint8_t)(cpu.s - 1);
}

static inline uint8_t pull8(void)
{
    cpu.s = (uint8_t)(cpu.s + 1);
    return rd8((uint16_t)(0x2100 + cpu.s));
}

/*
 * JSR or BSR to a routine written as typed C: jsr_push(ret) before the call, jsr_pop() after. ret is the
 * address after the JSR's last byte, taken from the image; the callee's own pushes then land where the
 * original's do. A tail jump gets no frame.
 */
static inline void jsr_push(uint16_t ret)
{
    push8((uint8_t)((uint16_t)(ret - 1) >> 8));
    push8((uint8_t)(ret - 1));
}

static inline void jsr_pop(void)
{
    cpu.s = (uint8_t)(cpu.s + 2);
}

/* The byte PHP pushes for the flags given: T clear, B set. */
static inline uint8_t php_byte(uint8_t flags)
{
    return (uint8_t)((flags & ~P_T) | P_B);
}

/* The I flag set, as SEI: the flag only, the CPU takes no access. */
static inline void sei(void)
{
    cpu.p |= P_I;
}

/* An add or a subtract of two bytes with its carry in, as ADC and SBC (decimal clear) leave them. */
typedef struct {
    uint8_t value;
    bool carry;     /* ADC: carry out. SBC: no borrow */
    bool overflow;
} Sum;

static inline Sum add8(uint8_t a, uint8_t b, bool carry_in)
{
    Sum s;
    unsigned total = (unsigned)a + b + (carry_in ? 1u : 0u);

    s.value = (uint8_t)total;
    s.carry = total > 0xFF;
    s.overflow = (~(a ^ b) & (a ^ s.value) & 0x80) != 0;
    return s;
}

static inline Sum sub8(uint8_t a, uint8_t b, bool carry_in)
{
    Sum s;
    int total = (int)a - b - (carry_in ? 0 : 1);

    s.value = (uint8_t)total;
    s.carry = total >= 0;
    s.overflow = ((a ^ b) & (a ^ s.value) & 0x80) != 0;
    return s;
}

/* Through a zero page pointer: LDA (zp), LDA (zp),Y and STA (zp),Y. The pointer's bytes are read, low then high. */
static inline uint8_t rd8_zp_y(uint16_t pointer, uint8_t y)
{
    uint8_t lo = rd8(pointer);
    uint8_t hi = rd8((uint16_t)(pointer + 1));

    return rd8((uint16_t)((lo | (hi << 8)) + y));
}

static inline void wr8_zp_y(uint16_t pointer, uint8_t y, uint8_t value)
{
    uint8_t lo = rd8(pointer);
    uint8_t hi = rd8((uint16_t)(pointer + 1));

    wr8((uint16_t)((lo | (hi << 8)) + y), value);
}

static inline void abi_flag(uint8_t flag, bool on)
{
    cpu.p = (uint8_t)(on ? (cpu.p | flag) : (cpu.p & ~flag));
}

static inline void abi_nz(uint8_t v)
{
    abi_flag(P_N, (v & 0x80) != 0);
    abi_flag(P_Z, v == 0);
}

/* CardExit, the registers and flags a routine leaves, is in syscard.h; this sets them for an exit shim. */
static inline void card_exit_apply(CardExit e)
{
    if (e.set_a) {
        cpu.a = e.a;
    }
    if (e.set_x) {
        cpu.x = e.x;
    }
    if (e.set_y) {
        cpu.y = e.y;
    }
    if (e.set_nz) {
        abi_flag(P_N, (e.nz & 0x80) != 0);
        abi_flag(P_Z, e.nz == 0);
    }
    if (e.set_c) {
        abi_flag(P_C, e.c);
    }
    if (e.set_v) {
        abi_flag(P_V, e.v);
    }
}

#define MPR6 0x40
#define CardArgs CardAl /* _AX to _DX, eight bytes */
#define CARD_ARG_BYTES 8

#endif
