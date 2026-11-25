/*
 * ops6280.h: every HuC6280 instruction as a C step on the register file `cpu`, for the C that
 * tools/rondo/cdraft.py drafts from the assembly (a stepping stone: never counted as decompiled).
 *
 * Each step does what the core does (tools/toolchains/src/pce/mednafen/pce/huc6280_ops.inc and
 * huc6280.cpp), in the core's order and with its flags, and every counted access goes through
 * rd8/wr8/st0-st2/tam/tma/push8/pull8: pointer low byte, pointer high byte, then the data;
 * a read-modify-write is one read then one write; BBR and BBS read the zero page byte; a block
 * move pushes Y, A, X, reads a source byte and writes a destination byte for each count, then
 * pulls X, A, Y. Instruction and operand fetches are not accesses here (the checker does not
 * count them).
 *
 * Where the core differs from the manual the core is followed, because it is the original the
 * oracles run: TSB sets Z when (m | A) == 0 and TRB when (m & ~A) == 0, BIT #imm sets N and V
 * from the immediate, ADC and SBC in decimal mode leave V alone, PHP pushes P with B set and T
 * clear, the T flag is cleared after every instruction but SET, PLP and RTI (the drafter
 * emits SET and the instruction after it together, as the _t forms), a zero page pointer
 * wraps inside page zero, JMP (abs) does not wrap its page.
 */
#ifndef RONDO_OPS6280_H
#define RONDO_OPS6280_H

#include "machine.h"

/* ---- flags ---- */

static inline uint8_t zn_bits(uint8_t v)
{
    return v == 0 ? P_Z : (uint8_t)(v & P_N);
}

/* X_ZN: N and Z from a value. */
static inline void op_zn(uint8_t v)
{
    cpu.p = (uint8_t)((cpu.p & ~(P_Z | P_N)) | zn_bits(v));
}

/* ---- effective addresses (pointer reads are counted reads) ---- */

static inline uint16_t ea_zp(uint8_t o) { return (uint16_t)(0x2000 | o); }
static inline uint16_t ea_zpx(uint8_t o) { return (uint16_t)(0x2000 | (uint8_t)(cpu.x + o)); }
static inline uint16_t ea_zpy(uint8_t o) { return (uint16_t)(0x2000 | (uint8_t)(cpu.y + o)); }
static inline uint16_t ea_abx(uint16_t a) { return (uint16_t)(a + cpu.x); }
static inline uint16_t ea_aby(uint16_t a) { return (uint16_t)(a + cpu.y); }

static inline uint16_t ea_ind(uint8_t o)
{
    uint8_t lo = rd8((uint16_t)(0x2000 | o));
    uint8_t hi = rd8((uint16_t)(0x2000 | (uint8_t)(o + 1)));

    return (uint16_t)(lo | (hi << 8));
}

static inline uint16_t ea_izx(uint8_t o)
{
    return ea_ind((uint8_t)(o + cpu.x));
}

static inline uint16_t ea_izy(uint8_t o)
{
    return (uint16_t)(ea_ind(o) + cpu.y);
}

/* ---- loads, stores, transfers ---- */

static inline void op_lda(uint8_t v) { cpu.a = v; op_zn(v); }
static inline void op_ldx(uint8_t v) { cpu.x = v; op_zn(v); }
static inline void op_ldy(uint8_t v) { cpu.y = v; op_zn(v); }
static inline void op_sta(uint16_t ea) { wr8(ea, cpu.a); }
static inline void op_stx(uint16_t ea) { wr8(ea, cpu.x); }
static inline void op_sty(uint16_t ea) { wr8(ea, cpu.y); }
static inline void op_stz(uint16_t ea) { wr8(ea, 0); }
static inline void op_tax(void) { cpu.x = cpu.a; op_zn(cpu.a); }
static inline void op_txa(void) { cpu.a = cpu.x; op_zn(cpu.a); }
static inline void op_tay(void) { cpu.y = cpu.a; op_zn(cpu.a); }
static inline void op_tya(void) { cpu.a = cpu.y; op_zn(cpu.a); }
static inline void op_tsx(void) { cpu.x = cpu.s; op_zn(cpu.x); }
static inline void op_txs(void) { cpu.s = cpu.x; }
static inline void op_inx(void) { cpu.x++; op_zn(cpu.x); }
static inline void op_iny(void) { cpu.y++; op_zn(cpu.y); }
static inline void op_dex(void) { cpu.x--; op_zn(cpu.x); }
static inline void op_dey(void) { cpu.y--; op_zn(cpu.y); }
static inline void op_cla(void) { cpu.a = 0; }
static inline void op_clx(void) { cpu.x = 0; }
static inline void op_cly(void) { cpu.y = 0; }
static inline void op_sax(void) { uint8_t t = cpu.x; cpu.x = cpu.a; cpu.a = t; }
static inline void op_say(void) { uint8_t t = cpu.y; cpu.y = cpu.a; cpu.a = t; }
static inline void op_sxy(void) { uint8_t t = cpu.x; cpu.x = cpu.y; cpu.y = t; }

/* ---- flag instructions ---- */

static inline void op_clc(void) { cpu.p &= (uint8_t)~P_C; }
static inline void op_sec(void) { cpu.p |= P_C; }
static inline void op_cld(void) { cpu.p &= (uint8_t)~P_D; }
static inline void op_sed(void) { cpu.p |= P_D; }
static inline void op_cli(void) { cpu.p &= (uint8_t)~P_I; }
static inline void op_sei(void) { cpu.p |= P_I; }
static inline void op_clv(void) { cpu.p &= (uint8_t)~P_V; }
static inline void op_set(void) { cpu.p |= P_T; }
static inline void op_clear_t(void) { cpu.p &= (uint8_t)~P_T; }

/* ---- logic and arithmetic on a value; the _t forms are the same after SET ---- */

/* T mode: the operand has been read; A is zero page [X] for the operation, and goes back to it. */
static inline uint8_t t_begin(void)
{
    uint8_t saved = cpu.a;

    cpu.a = rd8((uint16_t)(0x2000 + cpu.x));
    return saved;
}

static inline void t_end(uint8_t saved)
{
    wr8((uint16_t)(0x2000 + cpu.x), cpu.a);
    cpu.a = saved;
    cpu.p &= (uint8_t)~P_T;
}

static inline void op_and(uint8_t v) { cpu.a &= v; op_zn(cpu.a); }
static inline void op_ora(uint8_t v) { cpu.a |= v; op_zn(cpu.a); }
static inline void op_eor(uint8_t v) { cpu.a ^= v; op_zn(cpu.a); }
static inline void op_and_t(uint8_t v) { uint8_t s = t_begin(); op_and(v); t_end(s); }
static inline void op_ora_t(uint8_t v) { uint8_t s = t_begin(); op_ora(v); t_end(s); }
static inline void op_eor_t(uint8_t v) { uint8_t s = t_begin(); op_eor(v); t_end(s); }

static inline void op_adc(uint8_t x)
{
    uint8_t a = cpu.a;

    if (cpu.p & P_D) {
        unsigned tmp = (a & 0x0Fu) + (x & 0x0Fu) + (cpu.p & 1u);

        if (tmp >= 0x0A)
            tmp += 0x06;
        tmp += (a & 0xF0u) + (x & 0xF0u);
        if (tmp >= 0xA0)
            tmp += 0x60;
        cpu.p &= (uint8_t)~(P_Z | P_N | P_C);
        if (tmp & 0xFF00u)
            cpu.p |= P_C;
        cpu.a = (uint8_t)tmp;
        cpu.p |= zn_bits(cpu.a);
    } else {
        unsigned l = (unsigned)a + x + (cpu.p & 1u);

        cpu.p &= (uint8_t)~(P_Z | P_C | P_N | P_V);
        cpu.p |= (uint8_t)((((a ^ x) & 0x80) ^ 0x80) & ((a ^ l) & 0x80)) >> 1;
        cpu.p |= (uint8_t)((l >> 8) & P_C);
        cpu.a = (uint8_t)l;
        cpu.p |= zn_bits(cpu.a);
    }
}

static inline void op_adc_t(uint8_t v) { uint8_t s = t_begin(); op_adc(v); t_end(s); }

static inline void op_sbc(uint8_t x)
{
    uint8_t a = cpu.a;

    if (cpu.p & P_D) {
        const uint8_t m = (uint8_t)((a & 0xF) - (x & 0xF) - ((cpu.p & 1) ^ 1));
        const uint8_t n = (uint8_t)((a >> 4) - (x >> 4) - ((m >> 4) & 1));
        uint8_t res = (uint8_t)((n << 4) | (m & 0xF));

        cpu.p &= (uint8_t)~(P_Z | P_N | P_C);
        if (m & 0x10)
            res = (uint8_t)(res - 0x06);
        if (n & 0x10)
            res = (uint8_t)(res - 0x60);
        cpu.a = res;
        cpu.p |= (uint8_t)(((n >> 4) & 0x1) ^ 1);
        cpu.p |= zn_bits(cpu.a);
    } else {
        unsigned l = (unsigned)a - x - ((cpu.p & 1u) ^ 1u);

        cpu.p &= (uint8_t)~(P_Z | P_C | P_N | P_V);
        cpu.p |= (uint8_t)(((a ^ l) & (a ^ x) & 0x80) >> 1);
        cpu.p |= (uint8_t)(((l >> 8) & P_C) ^ P_C);
        cpu.a = (uint8_t)l;
        cpu.p |= zn_bits(cpu.a);
    }
}

static inline void op_cmp_r(uint8_t r, uint8_t m)
{
    unsigned t = (unsigned)r - m;

    op_zn((uint8_t)t);
    cpu.p &= (uint8_t)~P_C;
    cpu.p |= (uint8_t)(((t >> 8) & P_C) ^ P_C);
}

static inline void op_cmp(uint8_t m) { op_cmp_r(cpu.a, m); }
static inline void op_cpx(uint8_t m) { op_cmp_r(cpu.x, m); }
static inline void op_cpy(uint8_t m) { op_cmp_r(cpu.y, m); }

/* BIT: Z from A & m, N and V from m (the immediate form too, in this core). */
static inline void op_bit(uint8_t m)
{
    cpu.p &= (uint8_t)~(P_Z | P_V | P_N);
    cpu.p |= (uint8_t)(zn_bits((uint8_t)(m & cpu.a)) & P_Z);
    cpu.p |= (uint8_t)(m & (P_V | P_N));
}

/* TST #imm, mem: Z from imm & m, N and V from m. The memory read is the caller's. */
static inline void op_tst(uint8_t imm, uint8_t m)
{
    cpu.p &= (uint8_t)~(P_Z | P_V | P_N);
    cpu.p |= (m & imm) ? 0 : P_Z;
    cpu.p |= (uint8_t)(m & (P_V | P_N));
}

/* ---- shifts and steps on a value: the result; flags as the core sets them ---- */

static inline uint8_t f_asl(uint8_t x)
{
    cpu.p &= (uint8_t)~P_C;
    cpu.p |= (uint8_t)(x >> 7);
    x = (uint8_t)(x << 1);
    op_zn(x);
    return x;
}

static inline uint8_t f_lsr(uint8_t x)
{
    cpu.p &= (uint8_t)~(P_C | P_N | P_Z);
    cpu.p |= (uint8_t)(x & 1);
    x >>= 1;
    cpu.p |= zn_bits(x);
    return x;
}

static inline uint8_t f_rol(uint8_t x)
{
    uint8_t l = (uint8_t)(x >> 7);

    x = (uint8_t)((x << 1) | (cpu.p & P_C));
    cpu.p &= (uint8_t)~(P_Z | P_N | P_C);
    cpu.p |= l;
    cpu.p |= zn_bits(x);
    return x;
}

static inline uint8_t f_ror(uint8_t x)
{
    uint8_t l = (uint8_t)(x & 1);

    x = (uint8_t)((x >> 1) | ((cpu.p & P_C) << 7));
    cpu.p &= (uint8_t)~(P_Z | P_N | P_C);
    cpu.p |= l;
    cpu.p |= zn_bits(x);
    return x;
}

static inline uint8_t f_inc(uint8_t x) { x++; op_zn(x); return x; }
static inline uint8_t f_dec(uint8_t x) { x--; op_zn(x); return x; }

static inline uint8_t f_tsb(uint8_t x)
{
    cpu.p &= (uint8_t)~(P_Z | P_V | P_N);
    cpu.p |= (x | cpu.a) ? 0 : P_Z;
    cpu.p |= (uint8_t)(x & (P_N | P_V));
    return (uint8_t)(x | cpu.a);
}

static inline uint8_t f_trb(uint8_t x)
{
    cpu.p &= (uint8_t)~(P_Z | P_V | P_N);
    cpu.p |= (x & (uint8_t)~cpu.a) ? 0 : P_Z;
    cpu.p |= (uint8_t)(x & (P_N | P_V));
    return (uint8_t)(x & (uint8_t)~cpu.a);
}

/* On the accumulator and on memory (one read, one write). */
#define OP_ACC(f) do { cpu.a = f(cpu.a); } while (0)
#define OP_MEM(f, ea) do { uint16_t e_ = (ea); uint8_t v_ = rd8(e_); wr8(e_, f(v_)); } while (0)

static inline void op_rmb(uint8_t bit, uint8_t zp)
{
    uint16_t e = ea_zp(zp);

    wr8(e, (uint8_t)(rd8(e) & ~(1u << bit)));
}

static inline void op_smb(uint8_t bit, uint8_t zp)
{
    uint16_t e = ea_zp(zp);

    wr8(e, (uint8_t)(rd8(e) | (1u << bit)));
}

/* BBR and BBS: the zero page read; true when the branch is taken. */
static inline bool op_bbr(uint8_t bit, uint8_t zp) { return !(rd8(ea_zp(zp)) & (1u << bit)); }
static inline bool op_bbs(uint8_t bit, uint8_t zp) { return (rd8(ea_zp(zp)) & (1u << bit)) != 0; }

/* ---- stack ---- */

static inline void op_pha(void) { push8(cpu.a); }
static inline void op_phx(void) { push8(cpu.x); }
static inline void op_phy(void) { push8(cpu.y); }
static inline void op_php(void) { cpu.p &= (uint8_t)~P_T; push8((uint8_t)(cpu.p | P_B)); }
static inline void op_pla(void) { cpu.a = pull8(); op_zn(cpu.a); }
static inline void op_plx(void) { cpu.x = pull8(); op_zn(cpu.x); }
static inline void op_ply(void) { cpu.y = pull8(); op_zn(cpu.y); }
static inline void op_plp(void) { cpu.p = pull8(); }

/* ---- the MPRs, the speed, the VDC ports ---- */

static inline void op_tam(uint8_t mask) { tam(mask, cpu.a); }
static inline void op_tma(uint8_t mask) { cpu.a = tma(mask); }
static inline void op_csl(void) { csl(); }
static inline void op_csh(void) { csh(); }
static inline void op_st0(uint8_t v) { st0(v); }
static inline void op_st1(uint8_t v) { st1(v); }
static inline void op_st2(uint8_t v) { st2(v); }

/* ---- block moves: push Y A X, the byte moves, pull X A Y ---- */

static inline void bm_begin(void) { push8(cpu.y); push8(cpu.a); push8(cpu.x); }
static inline void bm_end(void) { cpu.x = pull8(); cpu.a = pull8(); cpu.y = pull8(); }

/* A count of 0 moves 65,536 bytes on the chip; the core's loop would not end, so a drafted
 * block move states its count and the drafter refuses 0. */
static inline void op_tii(uint16_t src, uint16_t dst, uint16_t n)
{
    bm_begin();
    for (unsigned i = 0; i < n; i++)
        wr8((uint16_t)(dst + i), rd8((uint16_t)(src + i)));
    bm_end();
}

static inline void op_tdd(uint16_t src, uint16_t dst, uint16_t n)
{
    bm_begin();
    for (unsigned i = 0; i < n; i++)
        wr8((uint16_t)(dst - i), rd8((uint16_t)(src - i)));
    bm_end();
}

static inline void op_tin(uint16_t src, uint16_t dst, uint16_t n)
{
    bm_begin();
    for (unsigned i = 0; i < n; i++)
        wr8(dst, rd8((uint16_t)(src + i)));
    bm_end();
}

static inline void op_tia(uint16_t src, uint16_t dst, uint16_t n)
{
    bm_begin();
    for (unsigned i = 0; i < n; i++)
        wr8((uint16_t)(dst + (i & 1)), rd8((uint16_t)(src + i)));
    bm_end();
}

static inline void op_tai(uint16_t src, uint16_t dst, uint16_t n)
{
    bm_begin();
    for (unsigned i = 0; i < n; i++)
        wr8((uint16_t)(dst + i), rd8((uint16_t)(src + (i & 1))));
    bm_end();
}

/* A jump through a pointer in memory: the two reads, low byte first. JMP (abs) and JMP (abs,X). */
static inline uint16_t op_jmp_ptr(uint16_t a)
{
    uint8_t lo = rd8(a);
    uint8_t hi = rd8((uint16_t)(a + 1));

    return (uint16_t)(lo | (hi << 8));
}

/* The address of the instruction the draft is at, set before each one: the checker hands the rest of a call to the
 * original from here when the original's call ended (left by the stack) before the C's did. */
extern uint16_t rondo_pc;

/*
 * A branch or fall-through into bytes no script ran: the draft has no code for them. Defined by
 * the drafter's support file (counts, reports, ends the run).
 */
void untraced(uint16_t addr);
void hand_over(uint16_t addr);
/* The routine left its frame (it set S outright): in lockstep the original runs on from addr; in A5 it returns. */
void leave_original(uint16_t addr);
/* A tail jump (a JMP, a branch or a fall-through out of the routine) made by the instruction at from. */
void call_original_at(uint16_t from, uint16_t addr);

#endif
