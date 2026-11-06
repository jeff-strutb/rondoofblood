/*
 * check.c: the oracles. The C decompilation against the running original.
 *
 * A5 (--a5 FILE). The original runs untouched. A call of a routine with C opens
 * a frame when its first instruction is reached by JSR, BSR or a jump, with the
 * disc's bytes resident at its run address. While a frame is open, every data
 * access goes to a log. At the return (RTS with S back and PC where the stack
 * said) RAM is rewound to the call's entry by undoing the log, the C runs on the
 * entry registers against that RAM with hardware reads replayed from the log, and
 * registers, flags, MPRs, RAM written and hardware writes must be the original's.
 * RAM is then put forward again. A call an interrupt overlapped is reported as
 * unverified, never judged; so is one that waits or leaves its frame. The order of
 * accesses is checked as well and reported on its own: a state match with a
 * different order is flagged. Stack bytes are not compared: the C keeps those in
 * locals. Code the C calls by address is replayed from the log: its entry registers
 * are compared, its writes applied and its exit registers taken.
 *
 * A7 (--record FILE, then --hybrid FILE). A recording run of the original logs
 * every call's cost in master clocks, and a hash of RAM and the picture for each
 * frame. The hybrid run swaps in the C at every call the original made, where the
 * call had no interrupt in it, did not cross a frame, and its C runs no original
 * code. Each of its hardware accesses is made at the clock the original made it,
 * and at the end the machine is charged the rest of what the original took. Every
 * frame must hash the same. The stack below S is not hashed.
 */
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "machine.h"

typedef struct {
    uint16_t run;
    uint16_t len;
    uint32_t disc;
    const char *name;
    void (*abi)(void);
    int swappable;
    uint16_t ends;   /* nonzero: judged on reaching this address at the entry's S */
} Tag;

extern const Tag rondo_tags[];
extern const unsigned rondo_tag_count;

typedef struct { uint8_t a, x, y, s, p; uint16_t pc; } Regs;

/* The core's exports (patch 0004). */
void (*chk_get_regs)(uint8_t *a, uint8_t *x, uint8_t *y, uint8_t *s, uint8_t *p, uint16_t *pc);
void (*chk_set_regs)(uint8_t a, uint8_t x, uint8_t y, uint8_t s, uint8_t p, uint16_t pc);
unsigned (*chk_mpr)(unsigned);
void (*chk_set_mpr)(unsigned, uint8_t);
uint32_t (*chk_ts)(void);
uint8_t (*chk_bus_rd)(uint16_t);
void (*chk_bus_wr)(uint16_t, uint8_t);
void (*chk_bus_st)(unsigned, uint8_t);
const uint8_t *(*chk_bank)(unsigned);
void (*chk_charge)(int);

Cpu cpu;

enum { MODE_OFF, MODE_A5, MODE_RECORD, MODE_HYBRID };
enum { E_READ, E_WRITE, E_HWREAD, E_HWWRITE, E_CALL, E_RET };

typedef struct {
    uint32_t phys;
    uint8_t type, val, old;
    uint8_t a, x, y, s, p;
    uint16_t pc;
    uint8_t mpr[8];
} LogE;

#define LOG_MAX (1u << 22)
#define FRAMES 64
#define C_MAX (1u << 20)
#define PFLAGS 0xCF
#define PREV_IRQ 0x100

typedef struct {
    const Tag *tag;
    int tag_index;
    uint32_t start;
    uint32_t record;
    uint32_t ts;
    uint16_t ret;
    Regs regs;
    uint8_t mpr[8];
    int overlapped;
} Frame;

static int mode;
static const char *check_path;
static FILE *report;
static uint8_t *user;
static LogE *logb;
static uint32_t logn;
static Frame frames[FRAMES];
static int depth;
static int prev_op = -1;
static int16_t *tag_at;     /* run address to tag index, or -1 */
static uint8_t *ram_bank[256];
static uint8_t *rom_bank[256];

/* Results by tag. */
typedef struct { uint64_t calls, identical, divergent, unverified, order, printed; } Stat;
static Stat *stats;
static uint64_t judged;

/* The machine, for the C under A5: where it stands in the log. */
static uint8_t mshadow[8];
static uint32_t cursor, hw_cursor, seg_end, seg_start;
static struct { uint32_t phys; uint8_t old; } cw[C_MAX];
static uint32_t cwn;
static jmp_buf c_abort;
static char why[256];
static int order_bad;
static char order_why[160];
static uint64_t c_budget;

enum { ABORT_NONE, ABORT_DIVERGED, ABORT_UNVERIFIED };

static uint32_t canon(uint32_t phys)
{
    uint32_t bank = phys >> 13;

    if (bank >= 0xF9 && bank <= 0xFB)
        return (0xF8u << 13) | (phys & 0x1FFF);
    return phys;
}

static int is_stack(uint32_t phys)
{
    return (phys >> 13) == 0xF8 && (phys & 0x1F00) == 0x0100;
}

/*
 * An access whose order is behaviour: hardware only. C is behaviour exact and the ASM bit
 * exact, so the order and count of plain RAM accesses are the ASM's to keep, and RAM is
 * judged by what it holds when the routine ends.
 */
static int is_access(const LogE *e)
{
    return e->type == E_HWREAD || e->type == E_HWWRITE;
}

static void stop_c(int reason, const char *fmt, uint32_t a, uint32_t b, uint32_t c)
{
    snprintf(why, sizeof why, fmt, a, b, c);
    longjmp(c_abort, reason);
}

/* The next access of the original's record, for the order check. */
static void order_step(int type, uint32_t phys)
{
    if (order_bad)
        return;
    while (cursor < seg_end && !is_access(&logb[cursor]))
        cursor++;
    if (cursor >= seg_end) {
        order_bad = 1;
        snprintf(order_why, sizeof order_why, "C %s %06X past the original's last access",
                 type == E_READ || type == E_HWREAD ? "read" : "write", phys);
        return;
    }
    if (logb[cursor].phys != phys || logb[cursor].type != type) {
        order_bad = 1;
        snprintf(order_why, sizeof order_why, "C %s %06X, original %s %06X, access %u",
                 type == E_READ || type == E_HWREAD ? "read" : "write", phys,
                 logb[cursor].type == E_READ || logb[cursor].type == E_HWREAD ? "read" : "write",
                 logb[cursor].phys, cursor - seg_start);
        return;
    }
    cursor++;
}

static LogE *next_hw(void)
{
    while (hw_cursor < seg_end && logb[hw_cursor].type != E_HWREAD && logb[hw_cursor].type != E_HWWRITE)
        hw_cursor++;
    return hw_cursor < seg_end ? &logb[hw_cursor] : NULL;
}

static uint32_t a5_phys(uint16_t addr)
{
    return canon(((uint32_t)mshadow[addr >> 13] << 13) | (addr & 0x1FFF));
}

static void pace_hw(void);

static unsigned hy_bank(uint16_t addr)
{
    return canon((uint32_t)chk_mpr(addr >> 13) << 13) >> 13;
}

uint8_t rd8(uint16_t addr)
{
    uint32_t p;

    if (mode == MODE_HYBRID) {
        if (!rom_bank[hy_bank(addr)])
            pace_hw();
        return chk_bus_rd(addr);
    }
    if (++c_budget > 4000000)
        stop_c(ABORT_UNVERIFIED, "C ran past its access budget", 0, 0, 0);
    p = a5_phys(addr);
    if (rom_bank[p >> 13])
        return rom_bank[p >> 13][p & 0x1FFF];
    {
        LogE *e = next_hw();

        if (!e || e->type != E_HWREAD || e->phys != p)
            stop_c(ABORT_DIVERGED, "C reads hardware %06X; original's next %06X type %u", p,
                   e ? e->phys : 0, e ? e->type : 99);
        order_step(E_HWREAD, p);
        hw_cursor++;
        return e->val;
    }
}

static void hw_write(uint32_t p, uint8_t value)
{
    LogE *e = next_hw();

    if (!e || e->type != E_HWWRITE || e->phys != p)
        stop_c(ABORT_DIVERGED, "C writes hardware %06X; original's next %06X type %u", p,
               e ? e->phys : 0, e ? e->type : 99);
    if (e->val != value)
        stop_c(ABORT_DIVERGED, "hardware write %06X: C %02X, original %02X", p, value, e->val);
    order_step(E_HWWRITE, p);
    hw_cursor++;
}

static void ram_put(uint32_t p, uint8_t value)
{
    if (cwn >= C_MAX)
        stop_c(ABORT_UNVERIFIED, "C wrote past its record", 0, 0, 0);
    cw[cwn].phys = p;
    cw[cwn++].old = ram_bank[p >> 13][p & 0x1FFF];
    ram_bank[p >> 13][p & 0x1FFF] = value;
}

void wr8(uint16_t addr, uint8_t value)
{
    uint32_t p;

    if (mode == MODE_HYBRID) {
        if (!ram_bank[hy_bank(addr)])
            pace_hw();
        chk_bus_wr(addr, value);
        return;
    }
    if (++c_budget > 4000000)
        stop_c(ABORT_UNVERIFIED, "C ran past its access budget", 0, 0, 0);
    p = a5_phys(addr);
    if (ram_bank[p >> 13]) {
        ram_put(p, value);
    } else {
        hw_write(p, value);
    }
}

void st0(uint8_t value)
{
    if (mode == MODE_HYBRID) {
        pace_hw();
        chk_bus_st(0, value);
    } else
        hw_write(0x1FE000, value);
}

void st1(uint8_t value)
{
    if (mode == MODE_HYBRID) {
        pace_hw();
        chk_bus_st(2, value);
    } else
        hw_write(0x1FE002, value);
}

void st2(uint8_t value)
{
    if (mode == MODE_HYBRID) {
        pace_hw();
        chk_bus_st(3, value);
    } else
        hw_write(0x1FE003, value);
}

uint8_t tma(uint8_t mpr_bit)
{
    for (unsigned i = 0; i < 8; i++)
        if (mpr_bit & (1u << i))
            return mode == MODE_HYBRID ? (uint8_t)chk_mpr(i) : mshadow[i];
    return 0;
}

void tam(uint8_t mpr_bits, uint8_t bank)
{
    for (unsigned i = 0; i < 8; i++)
        if (mpr_bits & (1u << i)) {
            if (mode == MODE_HYBRID)
                chk_set_mpr(i, bank);
            else
                mshadow[i] = bank;
        }
}

void tai(uint16_t src, uint16_t dst, uint16_t len)
{
    for (unsigned i = 0; i < len; i++) {
        uint8_t v = rd8((uint16_t)(src + (i & 1)));

        wr8((uint16_t)(dst + i), v);
    }
}

void cli(void)
{
    cpu.p &= (uint8_t)~P_I;
}

void wait_interrupt(void)
{
    if (mode == MODE_A5)
        stop_c(ABORT_UNVERIFIED, "waits on an interrupt", 0, 0, 0);
    fprintf(stderr, "check: a swapped routine waited on an interrupt\n");
    exit(3);
}

void call_original(uint16_t addr)
{
    uint32_t call = hw_cursor > cursor ? cursor : hw_cursor;
    uint32_t ret;
    LogE *e = NULL;

    if (mode != MODE_A5) {
        fprintf(stderr, "check: a swapped routine called original code at %04X\n", addr);
        exit(3);
    }
    /* The first entry to that address after where the C has reached. */
    for (uint32_t i = call; i < seg_end; i++)
        if (logb[i].type == E_CALL && logb[i].pc == addr) {
            e = &logb[i];
            call = i;
            break;
        }
    if (!e)
        stop_c(ABORT_DIVERGED, "C calls %04X; the original made no such call", addr, 0, 0);
    {
        LogE *h = next_hw();

        if (h && hw_cursor < call)
            stop_c(ABORT_DIVERGED, "C skipped hardware access %06X before calling %04X", h->phys, addr, 0);
    }
    if (e->a != cpu.a || e->x != cpu.x || e->y != cpu.y || ((e->p ^ cpu.p) & PFLAGS))
        stop_c(ABORT_DIVERGED, "to %04X: C A%02X X%02X Y%02X/P%02X", addr,
               (uint32_t)((cpu.a << 16) | (cpu.x << 8) | cpu.y), cpu.p);
    for (ret = call + 1; ret < seg_end; ret++)
        if (logb[ret].type == E_RET && logb[ret].s == (uint8_t)(e->s + 2))
            break;
    if (ret >= seg_end)
        stop_c(ABORT_UNVERIFIED, "code called at %04X does not come back in this call", addr, 0, 0);
    for (uint32_t i = call + 1; i < ret; i++)
        if (logb[i].type == E_WRITE)
            ram_put(logb[i].phys, logb[i].val);
    cpu.a = logb[ret].a;
    cpu.x = logb[ret].x;
    cpu.y = logb[ret].y;
    cpu.p = logb[ret].p;
    memcpy(mshadow, logb[ret].mpr, 8);
    if (!order_bad)
        cursor = ret + 1;
    hw_cursor = ret + 1;
}

static void note_hw_clock(void);
static void pace_hw(void);

/* The original's side of the log. */
static void log_push(int type, uint32_t phys, uint8_t val, uint8_t old)
{
    LogE *e;

    if (logn >= LOG_MAX) {
        /* Too long to judge: every open call goes down as unverified. */
        for (int i = 0; i < depth; i++) {
            stats[frames[i].tag_index].calls++;
            stats[frames[i].tag_index].unverified++;
        }
        depth = 0;
        logn = 0;
        return;
    }
    e = &logb[logn++];
    e->type = (uint8_t)type;
    e->phys = phys;
    e->val = val;
    e->old = old;
}

void check_rd(uint32_t phys, uint8_t v, int kind)
{
    if (kind == 2) {
        prev_op = v;
        return;
    }
    if (kind != 0 || !depth)
        return;
    phys = canon(phys);
    if (mode == MODE_RECORD) {
        if (!rom_bank[phys >> 13])
            note_hw_clock();
        return;
    }
    if (mode != MODE_A5)
        return;
    log_push(rom_bank[phys >> 13] ? E_READ : E_HWREAD, phys, v, 0);
}

void check_wr_pre(uint32_t phys, uint8_t v)
{
    if (!depth)
        return;
    phys = canon(phys);
    if (mode == MODE_RECORD) {
        if (!ram_bank[phys >> 13])
            note_hw_clock();
        return;
    }
    if (mode != MODE_A5)
        return;
    if (ram_bank[phys >> 13])
        log_push(E_WRITE, phys, v, ram_bank[phys >> 13][phys & 0x1FFF]);
    else
        log_push(E_HWWRITE, phys, v, 0);
}

void check_irq(void)
{
    prev_op = PREV_IRQ;
    for (int i = 0; i < depth; i++)
        frames[i].overlapped = 1;
}

static void log_marker(int type, const Regs *r)
{
    if (logn >= LOG_MAX)
        return;
    LogE *e = &logb[logn++];

    e->type = (uint8_t)type;
    e->phys = 0;
    e->a = r->a;
    e->x = r->x;
    e->y = r->y;
    e->s = r->s;
    e->p = r->p;
    e->pc = r->pc;
    for (unsigned i = 0; i < 8; i++)
        e->mpr[i] = (uint8_t)chk_mpr(i);
}

static void say(const Tag *t, Stat *s, const char *verdict, const char *text)
{
    if (s->printed < 6) {
        s->printed++;
        fprintf(report, "%s %s call %llu: %s\n", t->name, verdict,
                (unsigned long long)s->calls, text);
    }
}

/* Rewinds, runs the C, compares, puts RAM forward. The frame is on top. */
static void judge(Frame *f, const Regs *r)
{
    Stat *s = &stats[f->tag_index];
    static uint8_t *final_val;
    static uint32_t *final_stamp;
    static uint32_t stamp;
    int reason;
    char diff[400] = "";
    size_t dl = 0;

    s->calls++;
    judged++;
    if (f->overlapped) {
        s->unverified++;
        return;
    }
    if (!final_val) {
        final_val = malloc(1u << 21);
        final_stamp = calloc(1u << 21, sizeof *final_stamp);
    }
    stamp++;
    seg_start = cursor = hw_cursor = f->start;
    seg_end = logn;
    for (uint32_t i = logn; i-- > f->start;) {
        if (logb[i].type == E_WRITE) {
            ram_bank[logb[i].phys >> 13][logb[i].phys & 0x1FFF] = logb[i].old;
            final_stamp[logb[i].phys] = stamp;
        }
    }
    for (uint32_t i = f->start; i < logn; i++)
        if (logb[i].type == E_WRITE)
            final_val[logb[i].phys] = logb[i].val;
    memcpy(mshadow, f->mpr, 8);
    cpu.a = f->regs.a;
    cpu.x = f->regs.x;
    cpu.y = f->regs.y;
    cpu.s = f->regs.s;
    cpu.p = f->regs.p;
    cwn = 0;
    order_bad = 0;
    c_budget = 0;
    why[0] = 0;
    reason = setjmp(c_abort);
    if (reason == 0)
        f->tag->abi();
    if (reason == ABORT_NONE) {
        /* Where a routine ends on a shared exit, registers and flags are dead. */
        if (f->tag->ends)
            ;
        else if (cpu.a != r->a)
            dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " A C %02X orig %02X;", cpu.a, r->a);
        if (!f->tag->ends && cpu.x != r->x)
            dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " X C %02X orig %02X;", cpu.x, r->x);
        if (!f->tag->ends && cpu.y != r->y)
            dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " Y C %02X orig %02X;", cpu.y, r->y);
        if (!f->tag->ends && ((cpu.p ^ r->p) & PFLAGS))
            dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " P C %02X orig %02X (entry %02X);",
                                   cpu.p, r->p, f->regs.p);
        for (unsigned i = 0; i < 8; i++)
            if (mshadow[i] != (uint8_t)chk_mpr(i))
                dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " MPR%u C %02X orig %02X;", i,
                                       mshadow[i], chk_mpr(i));
        /* RAM: everything either side wrote. */
        for (uint32_t i = f->start; i < logn && dl < 300; i++) {
            if (logb[i].type == E_WRITE && !is_stack(logb[i].phys)) {
                uint32_t p = logb[i].phys;
                uint8_t c = ram_bank[p >> 13][p & 0x1FFF];

                if (c != final_val[p] && dl < 300)
                    dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " %06X C %02X orig %02X;",
                                           p, c, final_val[p]);
            }
        }
        for (uint32_t i = 0; i < cwn && dl < 300; i++) {
            uint32_t p = cw[i].phys;

            if (final_stamp[p] != stamp &&
                ram_bank[p >> 13][p & 0x1FFF] != cw[i].old)
                dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " %06X C wrote %02X, orig left %02X;",
                                       p, ram_bank[p >> 13][p & 0x1FFF], cw[i].old);
        }
        {
            LogE *h = next_hw();

            if (h && dl < 300)
                dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " original's hardware access %06X not made;",
                                       h->phys);
        }
    }
    /* Put RAM back: undo the C, redo the original. */
    while (cwn) {
        cwn--;
        ram_bank[cw[cwn].phys >> 13][cw[cwn].phys & 0x1FFF] = cw[cwn].old;
    }
    for (uint32_t i = f->start; i < logn; i++)
        if (logb[i].type == E_WRITE)
            ram_bank[logb[i].phys >> 13][logb[i].phys & 0x1FFF] = logb[i].val;
    if (reason == ABORT_UNVERIFIED) {
        s->unverified++;
        say(f->tag, s, "UNVERIFIED", why);
    } else if (reason == ABORT_DIVERGED) {
        s->divergent++;
        say(f->tag, s, "DIVERGENT", why);
    } else if (dl) {
        s->divergent++;
        say(f->tag, s, "DIVERGENT", diff);
    } else {
        s->identical++;
    }
    if (reason == ABORT_NONE && !dl) {
        if (!order_bad) {
            while (cursor < seg_end && !is_access(&logb[cursor]))
                cursor++;
            if (cursor < seg_end) {
                order_bad = 1;
                snprintf(order_why, sizeof order_why, "original's access %06X not made", logb[cursor].phys);
            }
        }
        if (order_bad) {
            s->order++;
            say(f->tag, s, "ORDER", order_why);
        }
    }
}

/* A7: the record. */
typedef struct {
    int32_t tag;
    uint32_t delta;
    uint32_t end;
    uint32_t flags;
    uint32_t start_ts;
    uint64_t hw_first;     /* its first entry in the clocks of hardware accesses */
} Rec;
enum { REC_OVERLAP = 1, REC_CROSSED = 2, REC_UNCLOSED = 4 };

static Rec *recs;
static uint64_t nrec, recmax;
static uint32_t *hw_clock;
static uint64_t nhw, hwmax, hw_replay;
static uint64_t rec_cursor;
static int run_frame_stamp;
static uint64_t swapped, kept_interrupt, kept_crossed, kept_original;
static uint64_t *fhash;
static uint64_t nf, fhmax;
static unsigned long hy_frames_ok, hy_first_bad = (unsigned long)-1;

/* RONDO_DUMP_FRAME=N: at frame N, RAM to <record>.<mode>.ram; and the hybrid's last
 * swapped calls, to find what wrote a byte that differs. */
static unsigned long dump_frame;
#define RECENT 256
static struct { int tag; uint8_t a, x, y; unsigned long frame; int owed, used; uint32_t delta; } recent[RECENT];
static unsigned recent_n;
static int drift_reported;

static void note_hw_clock(void)
{
    if (nhw == hwmax) {
        hwmax = hwmax ? hwmax * 2 : (1u << 20);
        hw_clock = realloc(hw_clock, hwmax * sizeof *hw_clock);
    }
    hw_clock[nhw++] = chk_ts();
}

/* Up to the clock the original made this access at. */
static void pace_hw(void)
{
    if (hw_replay < hwmax) {
        uint32_t want = hw_clock[hw_replay++];
        uint32_t now = chk_ts();

        if (want > now)
            chk_charge((int)(want - now));
    }
}

static void rec_new(int tag_index)
{
    if (nrec == recmax) {
        recmax = recmax ? recmax * 2 : (1u << 20);
        recs = realloc(recs, recmax * sizeof *recs);
    }
    recs[nrec].tag = tag_index;
    recs[nrec].delta = 0;
    recs[nrec].end = 0;
    recs[nrec].flags = REC_UNCLOSED;
    recs[nrec].start_ts = chk_ts();
    recs[nrec].hw_first = nhw;
    nrec++;
}

static int tag_resident(const Tag *t)
{
    for (unsigned i = 0; i < t->len; i++) {
        uint16_t logical = (uint16_t)(t->run + i);
        uint32_t p = canon(((uint32_t)chk_mpr(logical >> 13) << 13) | (logical & 0x1FFF));
        const uint8_t *b = rom_bank[p >> 13];

        if (!b || b[p & 0x1FFF] != user[t->disc + i])
            return 0;
    }
    return 1;
}

static uint16_t stack_return(uint8_t s)
{
    const uint8_t *base = ram_bank[0xF8];
    unsigned lo = base[0x100 + (uint8_t)(s + 1)];
    unsigned hi = base[0x100 + (uint8_t)(s + 2)];

    return (uint16_t)(((hi << 8) | lo) + 1);
}

static int prev_is_entry(void)
{
    return prev_op == 0x20 || prev_op == 0x44 || prev_op == 0x4C || prev_op == 0x6C || prev_op == 0x7C;
}

static void close_frames(const Regs *r)
{
    while (depth) {
        Frame *f = &frames[depth - 1];
        uint8_t up = (uint8_t)(r->s - f->regs.s);

        if (f->tag->ends ? (r->pc == f->tag->ends && r->s == f->regs.s)
                         : (up == 2 && r->pc == f->ret && prev_op == 0x60)) {
            if (mode == MODE_A5) {
                judge(f, r);
            } else {
                Rec *c = &recs[f->record];

                /* The core resets its clock inside a frame's run, so a clock
                 * that went backward means the call crossed that reset. */
                if (chk_ts() < f->ts)
                    c->flags |= REC_CROSSED;
                c->delta = chk_ts() - f->ts;
                c->end = (uint32_t)nrec;
                c->flags &= ~(uint32_t)REC_UNCLOSED;
                if (f->overlapped)
                    c->flags |= REC_OVERLAP;
            }
            depth--;
            continue;
        }
        if (up > 2 && up < 0x80) {
            if (mode == MODE_A5) {
                stats[f->tag_index].calls++;
                stats[f->tag_index].unverified++;
            }
            depth--;
            continue;
        }
        break;
    }
    if (!depth)
        logn = 0;
}

static void open_frame(int index, const Regs *r)
{
    Frame *f;

    if (depth == FRAMES) {
        depth = 0;
        logn = 0;
    }
    f = &frames[depth++];
    f->tag = &rondo_tags[index];
    f->tag_index = index;
    f->start = logn;
    f->ret = stack_return(r->s);
    f->regs = *r;
    f->overlapped = 0;
    for (unsigned i = 0; i < 8; i++)
        f->mpr[i] = (uint8_t)chk_mpr(i);
    if (mode == MODE_RECORD) {
        f->record = (uint32_t)nrec;
        f->ts = chk_ts();
        rec_new(index);
    }
}

/* Runs before each instruction. Nonzero: the C ran; the machine owes those clocks. */
int check_exec(uint32_t pc)
{
    Regs r;
    int index;

    chk_get_regs(&r.a, &r.x, &r.y, &r.s, &r.p, &r.pc);
    r.pc = (uint16_t)pc;
    if (mode == MODE_HYBRID) {
        index = tag_at[pc & 0xFFFF];
        if (index < 0 || !prev_is_entry() || !tag_resident(&rondo_tags[index]))
            return 0;
        if (rec_cursor >= nrec) {
            fprintf(stderr, "check: call of %s past the record's end\n", rondo_tags[index].name);
            exit(4);
        }
        Rec *c = &recs[rec_cursor];

        if (c->tag != index) {
            fprintf(stderr, "check: call of %s at frame %d; the record's next is %s\n",
                    rondo_tags[index].name, run_frame_stamp, rondo_tags[c->tag].name);
            exit(4);
        }
        /* The first call the clocks disagree at, with what ran before it. */
        if (!drift_reported && c->start_ts != chk_ts()) {
            drift_reported = 1;
            fprintf(report, "first clock drift at frame %d, %s: original ts %u, hybrid ts %u (record %llu, flags %u)\n",
                    run_frame_stamp, rondo_tags[index].name, c->start_ts, chk_ts(),
                    (unsigned long long)rec_cursor, c->flags);
            for (unsigned i = recent_n > 16 ? recent_n - 16 : 0; i < recent_n; i++)
                fprintf(report, "  before: frame %lu %s A%02X X%02X Y%02X owed %d used %d rec %u\n",
                        recent[i % RECENT].frame, rondo_tags[recent[i % RECENT].tag].name,
                        recent[i % RECENT].a, recent[i % RECENT].x, recent[i % RECENT].y,
                        recent[i % RECENT].owed, recent[i % RECENT].used, recent[i % RECENT].delta);
            fflush(report);
        }
        if (c->flags & REC_OVERLAP)
            kept_interrupt++;
        if (c->flags & (REC_CROSSED | REC_UNCLOSED))
            kept_crossed++;
        if (!rondo_tags[index].swappable)
            kept_original++;
        if (c->flags || !rondo_tags[index].swappable) {
            rec_cursor++;
            return 0;
        }
        rec_cursor = c->end;
        swapped++;
        recent[recent_n % RECENT].tag = index;
        recent[recent_n % RECENT].a = r.a;
        recent[recent_n % RECENT].x = r.x;
        recent[recent_n % RECENT].y = r.y;
        recent[recent_n % RECENT].frame = (unsigned long)run_frame_stamp;
        recent_n++;
        cpu.a = r.a;
        cpu.x = r.x;
        cpu.y = r.y;
        cpu.s = r.s;
        cpu.p = r.p;
        {
            uint32_t ts0 = chk_ts();
            const Tag *t = &rondo_tags[index];
            uint16_t ret = t->ends ? t->ends : stack_return(r.s);
            int32_t owed;

            hw_replay = c->hw_first;
            rondo_tags[index].abi();
            owed = (int32_t)c->delta - (int32_t)(chk_ts() - ts0);
            recent[(recent_n - 1) % RECENT].owed = owed;
            recent[(recent_n - 1) % RECENT].used = (int)(chk_ts() - ts0);
            recent[(recent_n - 1) % RECENT].delta = c->delta;
            chk_set_regs(cpu.a, cpu.x, cpu.y, t->ends ? r.s : (uint8_t)(r.s + 2), cpu.p, ret);
            prev_op = 0x60;
            return owed > 0 ? owed : 1;
        }
    }
    close_frames(&r);
    if (depth && mode == MODE_A5) {
        if (prev_is_entry() || prev_op == PREV_IRQ)
            log_marker(E_CALL, &r);
        else if (prev_op == 0x60)
            log_marker(E_RET, &r);
    }
    index = tag_at[pc & 0xFFFF];
    if (index >= 0 && prev_is_entry() && tag_resident(&rondo_tags[index]))
        open_frame(index, &r);
    return 0;
}

static uint64_t hash_bytes(uint64_t h, const uint8_t *p, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        h ^= p[i];
        h *= 0x100000001B3ull;
    }
    return h;
}

/* RAM and the picture; the stack below S is not state. */
void check_frame(unsigned long frame, const uint32_t *pixels, unsigned w, unsigned h)
{
    uint64_t hash = 0xCBF29CE484222325ull;
    uint64_t picture = 0xCBF29CE484222325ull;
    uint8_t a, x, y, s, p;
    uint16_t pc;

    run_frame_stamp = (int)frame;
    if (mode != MODE_RECORD && mode != MODE_HYBRID)
        return;
    if (dump_frame && frame == dump_frame) {
        char name[4200];
        FILE *d;

        snprintf(name, sizeof name, "%s.%s.ram", check_path, mode == MODE_RECORD ? "record" : "hybrid");
        d = fopen(name, "wb");
        for (unsigned b = 0x68; b <= 0x87 && d; b++)
            if (ram_bank[b])
                fwrite(ram_bank[b], 1, 8192, d);
        if (d) {
            fwrite(ram_bank[0xF8], 1, 8192, d);
            fclose(d);
        }
        if (mode == MODE_HYBRID && report) {
            for (unsigned i = recent_n > RECENT ? recent_n - RECENT : 0; i < recent_n; i++)
                fprintf(report, "swapped frame %lu %s A%02X X%02X Y%02X\n", recent[i % RECENT].frame,
                        rondo_tags[recent[i % RECENT].tag].name, recent[i % RECENT].a,
                        recent[i % RECENT].x, recent[i % RECENT].y);
            fflush(report);
        }
    }
    chk_get_regs(&a, &x, &y, &s, &p, &pc);
    for (unsigned b = 0x68; b <= 0x87; b++)
        if (ram_bank[b])
            hash = hash_bytes(hash, ram_bank[b], 8192);
    hash = hash_bytes(hash, ram_bank[0xF8], 0x100);
    hash = hash_bytes(hash, ram_bank[0xF8] + 0x100 + (size_t)s + 1, 0xFF - (size_t)s);
    hash = hash_bytes(hash, ram_bank[0xF8] + 0x200, 8192 - 0x200);
    picture = hash_bytes(picture, (const uint8_t *)pixels, (size_t)w * h * 4);
    if (mode == MODE_RECORD) {
        if (nf + 2 > fhmax) {
            fhmax = fhmax ? fhmax * 2 : 65536;
            fhash = realloc(fhash, fhmax * sizeof *fhash);
        }
        fhash[nf++] = hash;
        fhash[nf++] = picture;
        for (int i = 0; i < depth; i++)
            recs[frames[i].record].flags |= REC_CROSSED;
    } else if (nf + 2 <= fhmax) {
        if (fhash[nf] != hash || fhash[nf + 1] != picture) {
            if (hy_first_bad == (unsigned long)-1) {
                hy_first_bad = frame;
                fprintf(report, "first differing frame %lu: RAM %s, picture %s\n", frame,
                        fhash[nf] != hash ? "differs" : "same",
                        fhash[nf + 1] != picture ? "differs" : "same");
            }
        } else {
            hy_frames_ok++;
        }
        nf += 2;
    }
}

int check_init(int which, const char *path, const char *user_path)
{
    FILE *f = fopen(user_path, "rb");

    mode = which;
    check_path = path;
    if (getenv("RONDO_DUMP_FRAME"))
        dump_frame = strtoul(getenv("RONDO_DUMP_FRAME"), NULL, 10);
    user = malloc(20785152);
    tag_at = malloc(65536 * sizeof *tag_at);
    if (!f || !user || !tag_at || fread(user, 1, 20785152, f) != 20785152) {
        fprintf(stderr, "check: cannot load %s\n", user_path);
        return -1;
    }
    fclose(f);
    memset(tag_at, 0xFF, 65536 * sizeof *tag_at);
    for (unsigned i = 0; i < rondo_tag_count; i++)
        /* These two never return, so a frame for them would never close. */
        if (strcmp(rondo_tags[i].name, "KernelStart") && strcmp(rondo_tags[i].name, "KernelIdle"))
            tag_at[rondo_tags[i].run] = (int16_t)i;
    stats = calloc(rondo_tag_count, sizeof *stats);
    if (mode == MODE_A5)
        logb = malloc(LOG_MAX * sizeof *logb);
    for (unsigned b = 0; b < 256; b++) {
        const uint8_t *p = chk_bank(b);
        rom_bank[b] = (uint8_t *)p;
        if (p && ((b >= 0x68 && b <= 0x87) || (b >= 0xF8 && b <= 0xFB)))
            ram_bank[b] = (uint8_t *)p;
    }
    if (mode == MODE_HYBRID) {
        FILE *r = fopen(path, "rb");
        uint64_t head[3];

        if (!r || fread(head, sizeof head, 1, r) != 1) {
            fprintf(stderr, "check: cannot read the record %s\n", path);
            return -1;
        }
        nrec = head[0];
        fhmax = head[1];
        hwmax = head[2];
        hw_clock = malloc((hwmax + 1) * sizeof *hw_clock);
        recs = malloc(nrec * sizeof *recs);
        fhash = malloc(fhmax * sizeof *fhash);
        if (fread(recs, sizeof *recs, nrec, r) != nrec || fread(fhash, sizeof *fhash, fhmax, r) != fhmax ||
            fread(hw_clock, sizeof *hw_clock, hwmax, r) != hwmax) {
            fprintf(stderr, "check: short record %s\n", path);
            return -1;
        }
        fclose(r);
    } else if (mode == MODE_A5) {
        report = fopen(path, "w");
    }
    if (mode == MODE_HYBRID) {
        char name[4200];

        snprintf(name, sizeof name, "%s.hybrid.txt", path);
        report = fopen(name, "w");
    }
    if ((mode == MODE_A5 || mode == MODE_HYBRID) && !report) {
        fprintf(stderr, "check: cannot write the report for %s\n", path);
        return -1;
    }
    return 0;
}

void check_finish(void)
{
    if (mode == MODE_A5) {
        for (unsigned i = 0; i < rondo_tag_count; i++) {
            Stat *s = &stats[i];

            fprintf(report, "TAG %s %llu %llu %llu %llu %llu\n", rondo_tags[i].name,
                    (unsigned long long)s->calls, (unsigned long long)s->identical,
                    (unsigned long long)s->divergent, (unsigned long long)s->unverified,
                    (unsigned long long)s->order);
        }
        fclose(report);
    } else if (mode == MODE_RECORD) {
        FILE *r = fopen(check_path, "wb");
        uint64_t head[3] = { nrec, nf, nhw };

        if (!r || fwrite(head, sizeof head, 1, r) != 1 ||
            fwrite(recs, sizeof *recs, nrec, r) != nrec || fwrite(fhash, sizeof *fhash, nf, r) != nf ||
            fwrite(hw_clock, sizeof *hw_clock, nhw, r) != nhw) {
            perror(check_path);
            exit(1);
        }
        fclose(r);
        printf("record: %llu calls, %llu frames\n", (unsigned long long)nrec, (unsigned long long)(nf / 2));
    } else if (mode == MODE_HYBRID) {
        fprintf(report, "calls swapped %llu; left to the original: interrupt %llu, crossed a frame %llu, "
                "runs original code %llu\nframes compared %llu, identical %lu, first differing %ld\n",
                (unsigned long long)swapped, (unsigned long long)kept_interrupt,
                (unsigned long long)kept_crossed, (unsigned long long)kept_original,
                (unsigned long long)(nf / 2), hy_frames_ok,
                hy_first_bad == (unsigned long)-1 ? -1L : (long)hy_first_bad);
        fclose(report);
        printf("hybrid: %llu swapped, frames compared %llu, identical %lu, first differing %ld\n",
               (unsigned long long)swapped, (unsigned long long)(nf / 2), hy_frames_ok,
               hy_first_bad == (unsigned long)-1 ? -1L : (long)hy_first_bad);
    }
}
