/*
 * The machine as the C decompilation sees it.
 *
 * Bus: rd8 and wr8 take a logical address and go through the MPRs, as the
 * CPU does. Access order is behaviour: every routine touches each byte as
 * often and in the order its assembly does, so two reads are never put in
 * one expression.
 *
 * Registers: `cpu` is touched only by abi_ wrappers, the one place that
 * meets the assembly's calling convention, and by call_original, which hands
 * registers to code that has no C yet. A routine's body takes arguments and
 * returns values.
 *
 * Alu is the value of an addition or subtraction with what it carried out.
 * It is what lets a multi-byte sum be written byte by byte, in the bus order
 * the original has, and what a wrapper gives its caller as flags.
 */
#ifndef RONDO_MACHINE_H
#define RONDO_MACHINE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t a, x, y, s, p;
} Cpu;

extern Cpu cpu;

enum {
    P_C = 0x01, P_Z = 0x02, P_I = 0x04, P_D = 0x08,
    P_B = 0x10, P_T = 0x20, P_V = 0x40, P_N = 0x80
};

/* The bus and the instructions that have no C form. */
uint8_t rd8(uint16_t addr);
void wr8(uint16_t addr, uint8_t value);
void st0(uint8_t value);                  /* ST0: VDC address port, past the MPRs */
void st1(uint8_t value);                  /* ST1: VDC data low */
void st2(uint8_t value);                  /* ST2: VDC data high */
uint8_t tma(uint8_t mpr_bit);             /* TMA: the bank in one MPR */
void tam(uint8_t mpr_bits, uint8_t bank); /* TAM: set a bank in the given MPRs */
/* TAI: block move, source alternating between src and src + 1. */
void tai(uint16_t src, uint16_t dst, uint16_t len);
/* Runs original code, with the registers as they stand in cpu. */
void call_original(uint16_t addr);
void cli(void);                           /* CLI: interrupts on */
/* Returns once an interrupt has run: a wait the original spins on. */
void wait_interrupt(void);

/* The result of an operation, as the flags and A leave it. */
typedef struct {
    uint8_t a;    /* the value */
    uint8_t nz;   /* the value N and Z were last set from */
    bool c;       /* carry out */
    bool v;       /* overflow, when v_set */
    bool v_set;   /* false: nothing here set V; the caller's stands */
    bool c_kept;  /* true: nothing here set C; the caller's stands */
    bool nz_kept; /* true: N and Z are as cpu holds them */
    bool n_on;    /* TST: N set although nz is zero, as Z and N can both be */
    bool from_cpu;/* came back from original code: cpu holds the flags */
} Alu;

static inline Alu adc8(uint8_t a, uint8_t b, bool carry)
{
    unsigned sum = (unsigned)a + b + (carry ? 1u : 0u);
    Alu r;

    r.a = (uint8_t)sum;
    r.nz = r.a;
    r.c = sum > 0xFF;
    r.v = (((a ^ b) & 0x80) == 0) && (((a ^ sum) & 0x80) != 0);
    r.v_set = true;
    r.c_kept = false;
    r.nz_kept = r.n_on = r.from_cpu = false;
    return r;
}

static inline Alu sbc8(uint8_t a, uint8_t b, bool carry)
{
    unsigned diff = (unsigned)a - b - (carry ? 0u : 1u);
    Alu r;

    r.a = (uint8_t)diff;
    r.nz = r.a;
    r.c = diff <= 0xFF;
    r.v = (((a ^ b) & (a ^ diff)) & 0x80) != 0;
    r.v_set = true;
    r.c_kept = false;
    r.nz_kept = r.n_on = r.from_cpu = false;
    return r;
}

/* ADC with D set, as the HuC6280 does it: V is left alone. */
static inline Alu adc8_bcd(uint8_t a, uint8_t b, bool carry)
{
    unsigned sum = (a & 0x0Fu) + (b & 0x0Fu) + (carry ? 1u : 0u);
    Alu r;

    if (sum >= 0x0A)
        sum += 0x06;
    sum += (a & 0xF0u) + (b & 0xF0u);
    if (sum >= 0xA0)
        sum += 0x60;
    r.a = (uint8_t)sum;
    r.nz = r.a;
    r.c = (sum & 0xFF00u) != 0;
    r.v = false;
    r.v_set = false;
    r.c_kept = false;
    r.nz_kept = r.n_on = r.from_cpu = false;
    return r;
}

/* Nothing has happened yet: every flag is the caller's. */
static inline Alu alu_none(void)
{
    Alu r = { 0 };

    r.c_kept = true;
    return r;
}

/* LDA and the like: a value in A, N and Z from it; C and V stand. */
static inline void alu_load(Alu *r, uint8_t value)
{
    r->a = value;
    r->nz = value;
    r->nz_kept = false;
    r->n_on = false;
}

/* LDX, LDY: N and Z from a value that is not going to A. */
static inline void alu_nz(Alu *r, uint8_t value)
{
    r->nz = value;
    r->nz_kept = false;
    r->n_on = false;
}

/* CMP: A kept; N, Z and C from the compare; V stands. */
static inline void alu_compare(Alu *r, uint8_t a, uint8_t b)
{
    r->a = a;
    r->nz = (uint8_t)(a - b);
    r->nz_kept = false;
    r->n_on = false;
    r->c = a >= b;
    r->c_kept = false;
}

/* TST #mask, memory: A and C kept; Z from the AND; N and V bits 7 and 6. */
static inline void alu_tst(Alu *r, uint8_t mask, uint8_t memory)
{
    bool zero = (mask & memory) == 0;

    r->nz = zero ? 0 : (uint8_t)((memory & 0x80) | 0x01);
    r->n_on = zero && (memory & 0x80) != 0;
    r->nz_kept = false;
    r->v = (memory & 0x40) != 0;
    r->v_set = true;
}

/* What a call left, after what came before it: flags it did not set stand. */
static inline Alu alu_then(Alu before, Alu after)
{
    if (after.from_cpu)
        return after;
    if (after.c_kept) {
        after.c = before.c;
        after.c_kept = before.c_kept;
    }
    if (!after.v_set) {
        after.v = before.v;
        after.v_set = before.v_set;
    }
    return after;
}

/* Reads through a zero-page pointer: LDA [zp] and LDA [zp],Y. */
static inline uint8_t rd8_zp(uint16_t pointer)
{
    uint8_t lo = rd8(pointer);
    uint8_t hi = rd8((uint16_t)(pointer + 1));

    return rd8((uint16_t)(lo | (hi << 8)));
}

static inline uint8_t rd8_zp_y(uint16_t pointer, uint8_t y)
{
    uint8_t lo = rd8(pointer);
    uint8_t hi = rd8((uint16_t)(pointer + 1));

    return rd8((uint16_t)((lo | (hi << 8)) + y));
}

/* Writes through a zero-page pointer: STA [zp],Y. */
static inline void wr8_zp_y(uint16_t pointer, uint8_t y, uint8_t value)
{
    uint8_t lo = rd8(pointer);
    uint8_t hi = rd8((uint16_t)(pointer + 1));

    wr8((uint16_t)((lo | (hi << 8)) + y), value);
}

/* Memory shifts: the bit shifted out. */
static inline bool lsr_mem(uint16_t addr)
{
    uint8_t v = rd8(addr);

    wr8(addr, (uint8_t)(v >> 1));
    return (v & 0x01) != 0;
}

static inline bool ror_mem(uint16_t addr, bool in)
{
    uint8_t v = rd8(addr);

    wr8(addr, (uint8_t)((v >> 1) | (in ? 0x80 : 0)));
    return (v & 0x01) != 0;
}

static inline bool asl_mem(uint16_t addr)
{
    uint8_t v = rd8(addr);

    wr8(addr, (uint8_t)(v << 1));
    return (v & 0x80) != 0;
}

static inline bool rol_mem(uint16_t addr, bool in)
{
    uint8_t v = rd8(addr);

    wr8(addr, (uint8_t)((v << 1) | (in ? 0x01 : 0)));
    return (v & 0x80) != 0;
}

/* Wrappers only: exit flags. */
static inline void abi_nz(uint8_t v)
{
    cpu.p = (uint8_t)(cpu.p & ~(P_N | P_Z));
    cpu.p |= v & P_N;
    if (v == 0)
        cpu.p |= P_Z;
}

static inline void abi_flag(uint8_t flag, bool on)
{
    cpu.p = (uint8_t)(on ? (cpu.p | flag) : (cpu.p & ~flag));
}

/* C and V alone, for code about to call original code that may keep them. */
static inline void abi_alu_flags(Alu *r)
{
    if (!r->c_kept)
        abi_flag(P_C, r->c);
    if (r->v_set)
        abi_flag(P_V, r->v);
    r->c_kept = true;
    r->v_set = false;
}

static inline void abi_alu(Alu r)
{
    cpu.a = r.a;
    if (!r.nz_kept) {
        abi_nz(r.nz);
        if (r.n_on)
            abi_flag(P_N, true);
    }
    if (!r.c_kept)
        abi_flag(P_C, r.c);
    if (r.v_set)
        abi_flag(P_V, r.v);
}

/* A and the flags as cpu holds them: at entry, and after original code. */
static inline Alu alu_from_cpu(void)
{
    Alu r = { 0 };

    r.a = cpu.a;
    r.nz = (cpu.p & P_Z) ? 0 : ((cpu.p & P_N) ? 0x80 : 0x01);
    r.nz_kept = true;
    r.c = (cpu.p & P_C) != 0;
    r.c_kept = true;
    r.v = (cpu.p & P_V) != 0;
    r.from_cpu = true;
    return r;
}

/* Runs original code with A and the flags as r leaves them, and gives what it left. */
static inline Alu call_with(uint16_t addr, Alu r)
{
    abi_alu(r);
    call_original(addr);
    return alu_from_cpu();
}

/* Runs a routine through its wrapper, for one that meets the assembly's registers. */
static inline Alu call_abi(void (*abi)(void), Alu r)
{
    abi_alu(r);
    abi();
    return alu_from_cpu();
}

#endif
