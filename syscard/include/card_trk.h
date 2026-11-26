/* Instruction by instruction tracking of the registers and flags a routine leaves, for routines whose exit depends on the path. */
#ifndef CARD_TRK_H
#define CARD_TRK_H

#include "card.h"
#include "syscard.h"

/* The registers and flags a handler leaves, as it works them out instruction by instruction. */
typedef struct {
    uint8_t a, x, y;
    bool n, z, v, c;
    bool a_set, x_set, y_set, v_set, c_set;
} Trk;

/* LDX, LDY, TAX or TAY: the register and N, Z from it. */
static inline void t_xy_nz(Trk *t, uint8_t value)
{
    t->n = (value & 0x80) != 0;
    t->z = value == 0;
}

static inline void t_nz(Trk *t, uint8_t value)
{
    t->n = (value & 0x80) != 0;
    t->z = value == 0;
}

/* LDA, AND or ORA: A the value, N and Z from it. */
static inline void t_a(Trk *t, uint8_t value)
{
    t->a = value;
    t->a_set = true;
    t_nz(t, value);
}

/* TST #mask, memory: N and V from the byte, Z from the mask AND it. */
static inline void t_tst(Trk *t, uint8_t mask, uint8_t memory)
{
    t->n = (memory & 0x80) != 0;
    t->v = (memory & 0x40) != 0;
    t->v_set = true;
    t->z = (mask & memory) == 0;
}

/* TSB or TRB with A the mask just loaded: N and V from the old byte, Z from A AND it. */
static inline void t_tb(Trk *t, uint8_t old)
{
    t_tst(t, t->a, old);
}

static inline void t_cmp(Trk *t, uint8_t value, uint8_t operand)
{
    t_nz(t, (uint8_t)(value - operand));
    t->c = value >= operand;
    t->c_set = true;
}

/* What a routine called leaves. */
static inline void t_take(Trk *t, CardExit e)
{
    if (e.set_a) {
        t->a = e.a;
        t->a_set = true;
    }
    if (e.set_x) {
        t->x = e.x;
        t->x_set = true;
    }
    if (e.set_y) {
        t->y = e.y;
        t->y_set = true;
    }
    if (e.set_nz)
        t_nz(t, e.nz);
    if (e.set_n)
        t->n = e.n;
    if (e.set_z)
        t->z = e.z;
    if (e.set_c) {
        t->c = e.c;
        t->c_set = true;
    }
    if (e.set_v) {
        t->v = e.v;
        t->v_set = true;
    }
}

static inline CardExit t_exit(const Trk *t)
{
    CardExit e = { 0 };

    e.a = t->a;
    e.x = t->x;
    e.y = t->y;
    e.n = t->n;
    e.z = t->z;
    e.v = t->v;
    e.c = t->c;
    e.set_a = t->a_set;
    e.set_x = t->x_set;
    e.set_y = t->y_set;
    e.set_n = e.set_z = true;
    e.set_v = t->v_set;
    e.set_c = t->c_set;
    return e;
}

/* LDA #mask; TRB or TSB $1802 (or the port named): the old byte is what the flags come from. */
static inline void t_mask(Trk *t, uint8_t mask, uint8_t old)
{
    t_a(t, mask);
    t_tb(t, old);
}


/* The instructions of the tracked A. */
static inline void t_adc(Trk *t, uint8_t operand)
{
    Sum sum = add8(t->a, operand, t->c);

    t_a(t, sum.value);
    t->c = sum.carry;
    t->v = sum.overflow;
    t->c_set = t->v_set = true;
}

static inline void t_sbc(Trk *t, uint8_t operand)
{
    Sum sum = sub8(t->a, operand, t->c);

    t_a(t, sum.value);
    t->c = sum.carry;
    t->v = sum.overflow;
    t->c_set = t->v_set = true;
}

static inline void t_clc(Trk *t)
{
    t->c = false;
    t->c_set = true;
}

static inline void t_sec(Trk *t)
{
    t->c = true;
    t->c_set = true;
}

static inline void t_rol(Trk *t)
{
    bool out = (t->a & 0x80) != 0;

    t_a(t, (uint8_t)((t->a << 1) | (t->c ? 1 : 0)));
    t->c = out;
    t->c_set = true;
}

static inline void t_lsr(Trk *t)
{
    bool out = (t->a & 1) != 0;

    t_a(t, (uint8_t)(t->a >> 1));
    t->c = out;
    t->c_set = true;
}

static inline void t_asl(Trk *t)
{
    bool out = (t->a & 0x80) != 0;

    t_a(t, (uint8_t)(t->a << 1));
    t->c = out;
    t->c_set = true;
}

/* INC A and DEC A: N and Z only. */
static inline void t_inc(Trk *t)
{
    t->a = (uint8_t)(t->a + 1);
    t_nz(t, t->a);
}

#endif
