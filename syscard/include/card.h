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

typedef struct {
    uint8_t a, x, y, s, p;
} Cpu;

extern Cpu cpu;

enum { P_C = 0x01, P_Z = 0x02, P_V = 0x40, P_N = 0x80 };

uint8_t rd8(uint16_t addr);
void wr8(uint16_t addr, uint8_t value);
uint8_t tma(uint8_t mpr_bit);
void tam(uint8_t mpr_bits, uint8_t bank);

static inline void abi_flag(uint8_t flag, bool on)
{
    cpu.p = (uint8_t)(on ? (cpu.p | flag) : (cpu.p & ~flag));
}

static inline void abi_nz(uint8_t v)
{
    abi_flag(P_N, (v & 0x80) != 0);
    abi_flag(P_Z, v == 0);
}

#define MPR6 0x40
#define CardArgs CardAl /* _AX to _DX, eight bytes */
#define CARD_ARG_BYTES 8

#endif
