/*
 * The machine as the C decompilation sees it.
 *
 * Bus: rd8 and wr8 take a logical address and go through the MPRs, as the CPU does. Every
 * work RAM read and write, every hardware access, every push and pull, and every TAM, CSL
 * and CSH the original makes, the C makes: where the assembly makes it, in its order and
 * number. No value is kept in a C variable across a point where the original reads memory
 * again (docs/rondo/PLAN.md, section 1). The checker judges the order of these accesses, A5
 * per call and A7 in lockstep with interrupts delivered inside the C where the original took
 * them. What it does not count: instruction and operand fetches, reads of ROM, and the pulls
 * an RTS or RTI makes of its return address.
 *
 * Registers: `cpu` is touched only by abi_ wrappers, the one place that meets the assembly's
 * calling convention, by call_original and the stack helpers below. A routine's body takes
 * arguments and returns values. cpu.s is the emulated stack pointer: at a routine's end it
 * holds S as it stood just before the routine's RTS.
 *
 * The stack: a call the original makes with JSR or BSR is made with call_frame (C) or
 * call_orig (code with no C): they push the return address where the original pushes it and
 * pop it after. PHA, PHX, PHY and PHP are push8, and the pulls pull8.
 *
 * Alu is the value of an addition or subtraction with what it carried out. It is what lets a
 * multi-byte sum be written byte by byte, in the bus order the original has, and what a
 * wrapper gives its caller as flags.
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
/* TAI: a block move, source alternating between src and src + 1; it pushes and pulls Y, A and X as the CPU does. */
void tai(uint16_t src, uint16_t dst, uint16_t len);
/*
 * Runs original code, with the registers as they stand in cpu. call_original makes no frame
 * (the checker reports the original's JSR pushes as missing); call_orig is the JSR: ret is the
 * address after the JSR's last byte.
 */
void call_original(uint16_t addr);
void call_orig(uint16_t ret, uint16_t addr);
void cli(void);                           /* CLI: interrupts on */
/*
 * resident_at(run): which routine is loaded at a run address. Returns the disc location (DISC(sector, offset)) of the
 * tagged routine whose bytes are resident there under the current mapping, 0 if none. It is not an access of the
 * original and is never placed or counted: it exists so that C can name an overlay's routine (a callee that is one of
 * several variants at the same address) and call it as an ordinary typed call. A port sets it when an overlay loads.
 */
#define DISC(sector, offset) ((uint32_t)(sector) * 2048u + (uint32_t)(offset))
uint32_t resident_at(uint16_t run_address);
/* A wait the original spins on with no data access (a jump or branch to itself): returns once the
 * interrupt the original took there has run. A spin that reads memory is a loop of reads in the C. */
void wait_interrupt(void);
void csl(void);                           /* CSL: the CPU's slow clock */
void csh(void);                           /* CSH: the fast clock */

/* The emulated stack, at $2100 + S. */
static inline void push8(uint8_t value)
{
    wr8((uint16_t)(0x2100 + cpu.s), value);
    cpu.s = (uint8_t)(cpu.s - 1);
}

static inline uint8_t pull8(void)
{
    uint8_t s = (uint8_t)(cpu.s + 1);
    uint8_t v = rd8((uint16_t)(0x2100 + s));      /* the access first: the checker may hand the call to the original here */

    cpu.s = s;
    return v;
}

/*
 * JSR or BSR to a routine written as typed C: jsr_push(ret) before the call, jsr_pop() after. ret
 * is the address after the JSR's last byte; the pushed bytes are judged against the original's.
 * The callee's own pushes then land where the original's do.
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

/*
 * JSR or BSR to C: pushes the address of the call's last byte, runs the routine, pops. Returns
 * nonzero when the callee dropped the frame (it ended with S two higher than it should): the
 * caller then ends at once, as the original's caller never went on.
 */
static inline int call_frame(uint16_t ret, void (*abi)(void))
{
    uint8_t expect;

    push8((uint8_t)((uint16_t)(ret - 1) >> 8));
    push8((uint8_t)(ret - 1));
    expect = cpu.s;
    abi();
    if (cpu.s != expect)
        return 1;
    cpu.s = (uint8_t)(cpu.s + 2);
    return 0;
}

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

/* LDA #value, STA addr. */
static inline void alu_put(Alu *r, uint16_t addr, uint8_t value)
{
    alu_load(r, value);
    wr8(addr, value);
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
