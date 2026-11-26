/*
 * The exit of a routine whose last step depends on the path: each return builds the values that path computed, from the last
 * instruction that set them (a load or logic operation for A, N and Z; a compare, add or subtract for C; an add, subtract or bit
 * test for V). The registers themselves are set only by the routine's abi_ wrapper, from what the body returns.
 */
#ifndef CARD_EXIT_H
#define CARD_EXIT_H

#include "card.h"
#include "syscard.h"

/* After LDA, AND, ORA, EOR, ASL A, LSR A, PLA, TXA or TYA: A, and N and Z from it. */
static inline CardExit exit_a(uint8_t a)
{
    CardExit e = { 0 };

    e.a = e.nz = a;
    e.set_a = e.set_nz = true;
    return e;
}

/* After CMP #operand of the A holding `a`: A, and N, Z and C from the compare. */
static inline CardExit exit_cmp(uint8_t a, uint8_t operand)
{
    CardExit e = { 0 };

    e.a = a;
    e.nz = (uint8_t)(a - operand);
    e.c = a >= operand;
    e.set_a = e.set_nz = e.set_c = true;
    return e;
}

/* After CPX or CPY: N, Z and C from the compare, no A. */
static inline CardExit exit_cmp_index(uint8_t index, uint8_t operand)
{
    CardExit e = { 0 };

    e.nz = (uint8_t)(index - operand);
    e.c = index >= operand;
    e.set_nz = e.set_c = true;
    return e;
}

/* After LDX, INX or DEX (or LDY, INY, DEY): the register, and N and Z from it; A not set. */
static inline CardExit exit_x(uint8_t x)
{
    CardExit e = { 0 };

    e.x = e.nz = x;
    e.set_x = e.set_nz = true;
    return e;
}

static inline CardExit exit_y(uint8_t y)
{
    CardExit e = { 0 };

    e.y = e.nz = y;
    e.set_y = e.set_nz = true;
    return e;
}

static inline CardExit with_v(CardExit e, bool v)
{
    e.v = v;
    e.set_v = true;
    return e;
}

static inline CardExit with_c(CardExit e, bool c)
{
    e.c = c;
    e.set_c = true;
    return e;
}

static inline CardExit with_x(CardExit e, uint8_t x)
{
    e.x = x;
    e.set_x = true;
    return e;
}

static inline CardExit with_y(CardExit e, uint8_t y)
{
    e.y = y;
    e.set_y = true;
    return e;
}

#endif
