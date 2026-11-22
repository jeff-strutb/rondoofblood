/*
 * check.c: the oracles. The C decompilation against the running original.
 *
 * Every counted access of a routine is judged in the original's order and number. Counted:
 * work RAM reads and writes (the stack page and CD RAM included), hardware reads and writes,
 * and changes of the MPRs and the CPU clock. Not counted (build/rondo/checker/DESIGN.md, 3.2):
 * instruction and operand fetches, reads of ROM, the pulls an RTS or RTI makes of the return
 * address, and everything between an interrupt's entry and its return (the handler is judged as
 * calls of its own).
 *
 * The original runs untouched and writes one stream of events while a tagged routine is open:
 * OPEN and CLOSE of each tagged call (JSR, BSR, JMP or an interrupt entry), CALL and RET of every
 * entry, INT and IRET around an interrupt, and every counted access. A call opens when its first
 * instruction is reached with the disc's bytes resident at its run address.
 *
 * A5 (--a5 FILE). The stream is kept in memory. At each call's return RAM is rewound to its entry,
 * the C runs on the entry registers against that RAM, and each access it makes is matched to the
 * next counted event of the call. Hardware reads are answered from the stream. An interrupt the
 * call had is passed where the original took it: the handler's RAM writes are applied then. At
 * the end registers, flags, MPRs, CPU clock, the stack and the return target, and RAM must be
 * the original's.
 *
 * A7 (--record FILE, then --hybrid FILE, run at the same time over a FIFO or one after the other
 * over a file). The record writes the stream. The hybrid runs the C in place of every tagged
 * call, as a coroutine the emulated CPU can run under: an interrupt is dispatched inside the C at
 * the place the original took it (its handler then runs on the live machine and sees the RAM and
 * stack the original had), code with no C is run when the C calls it, and a frame that ends in the
 * middle of a call ends there. Every frame's hash must equal the record's.
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <ucontext.h>

#include "machine.h"

typedef struct {
    uint16_t run;
    uint16_t len;
    uint32_t disc;
    const char *name;
    void (*abi)(void);
    int swappable;
    uint16_t ends;   /* nonzero: judged on reaching this address at the entry's S */
    int rom;         /* nonzero: disc is an offset in the System Card's ROM */
} Tag;

extern const Tag rondo_tags[];
extern const unsigned rondo_tag_count;

typedef struct { uint8_t a, x, y, s, p; uint16_t pc; } Regs;

/* The core's exports (patches 0004, 0005, 0006). */
void (*chk_get_regs)(uint8_t *a, uint8_t *x, uint8_t *y, uint8_t *s, uint8_t *p, uint16_t *pc);
void (*chk_set_regs)(uint8_t a, uint8_t x, uint8_t y, uint8_t s, uint8_t p, uint16_t pc);
unsigned (*chk_mpr)(unsigned);
void (*chk_set_mpr)(unsigned, uint8_t);
uint32_t (*chk_ts)(void);
uint64_t (*chk_ts_abs)(void);
uint8_t (*chk_bus_rd)(uint16_t);
void (*chk_bus_wr)(uint16_t, uint8_t);
void (*chk_bus_st)(unsigned, uint8_t);
const uint8_t *(*chk_bank)(unsigned);
void (*chk_charge)(int);
uint8_t (*chk_get_speed)(void);
void (*chk_set_speed)(uint8_t);

Cpu cpu;

enum { MODE_OFF, MODE_A5, MODE_RECORD, MODE_HYBRID };

/* Events. Accesses first; the rest are markers with a payload. */
enum {
    EV_R, EV_W, EV_ROMR, EV_HWR, EV_HWW, EV_RTPULL, EV_STATE,
    EV_OPEN, EV_CLOSE, EV_CALL, EV_RET, EV_INT, EV_IRET, EV_FRAME, EV_END
};
enum { K_JSR, K_IRQ, K_JMP };          /* OPEN, CALL: how the entry was made */
enum { CL_OK, CL_DROP, CL_LEFT };      /* CLOSE: how it ended */

typedef struct {
    uint8_t type, val, old, kind;
    uint32_t addr;                     /* physical address; for OPEN, CLOSE the tag index */
    uint64_t clock;                    /* absolute master clock: markers, hardware accesses */
    uint64_t hash2;                    /* FRAME: the picture's hash; the RAM hash is in clock */
    uint16_t pc, ret;
    uint8_t a, x, y, s, p, speed;
    uint8_t mpr[8];
} Ev;

static int is_marker(int t) { return t >= EV_STATE; }
/* A poke of the script is a write the C does not make, by something outside the program: kind 1 on a W event.
 * It is in the stream so that rewinding RAM and putting it forward again keeps it, and is never counted. */
static int is_poke(const Ev *e) { return e->type == EV_W && e->kind == 1; }
static int is_counted(const Ev *e)
{
    int t = e->type;

    return (t == EV_R || t == EV_W || t == EV_HWR || t == EV_HWW || t == EV_STATE) && !is_poke(e);
}
static int is_noise(const Ev *e) { return e->type == EV_ROMR || e->type == EV_RTPULL || is_poke(e); }

#define PFLAGS 0xCF
#define PREV_IRQ 0x100
#define PREV_UNIT 0x101
#define FRAMES 512
#define LOG_START (1u << 22)
static uint64_t log_hard_events = 1u << 24;      /* RONDO_LOG_HARD overrides, for finding a fault */
#define LOG_HARD log_hard_events
#define C_MAX (1u << 20)
#define UNIT_MARK 0x0001

enum { D_MOVED, D_EXTRA, D_MISSING, D_VALUE, D_STACK, D_STATE, D_REGS, D_CALL, D_INT, NKIND };
static const char *kind_name[NKIND] = {
    "MOVED", "EXTRA", "MISSING", "VALUE", "STACK", "STATE", "REGS", "CALL", "INT"
};

typedef struct {
    uint64_t calls, identical, divergent, unverified, order, printed, printed_v;
    uint64_t kind[NKIND];
    uint64_t ints, originals, waits;   /* hybrid: interrupts dispatched inside its C, calls into original code, waits */
    /* Calls not judged, by cause (each also counted in `unverified`, except the three arrivals): */
    uint64_t unv_left;       /* left by a longer route than RTS or RTI */
    uint64_t unv_log;        /* the stream outgrew LOG_HARD events */
    uint64_t unv_depth;      /* nested deeper than FRAMES */
    uint64_t unv_computed;   /* a tagged address entered by RTS: a computed dispatch, not opened */
    uint64_t arrive_branch;  /* a tagged address reached by a branch, fall-through or other non-entry (not opened) */
    uint64_t arrive_nonres;  /* a tagged address entered by JSR or JMP while its bytes are not resident (not opened) */
    uint64_t runaway;        /* the C ran past its access or write budget (counted divergent) */
    uint64_t prefix;         /* open at the script's end: its accesses judged as far as the log goes */
    uint64_t noreturn;       /* A5: a call the original's record shows did not return inside the call: judged up to it, its exit not compared */
    uint64_t kept;           /* hybrid: entered by the original's code, not swapped (a diagnostic run) */
} Stat;

static int mode;
static const char *check_path;
static FILE *report;
static uint8_t *user;
static Stat *stats;
static int16_t *tag_at;     /* run address to the first tag index there, or -1 */
static int16_t *tag_next;   /* the next tag with the same run address (another overlay's bytes), or -1 */
static int tag_resident(const Tag *t);

/* The tag at a run address: the first whose bytes are resident, else the first. */
static int tag_pick(unsigned pc)
{
    int first = tag_at[pc & 0xFFFF];

    if (first < 0 || tag_next[first] < 0)
        return first;
    for (int i = first; i >= 0; i = tag_next[i])
        if (tag_resident(&rondo_tags[i]))
            return i;
    return first;
}
static uint8_t *ram_bank[256];
static uint8_t *rom_bank[256];
static int prev_op = -1;
static unsigned long cur_frame;
static const char *script_where = "";      /* the script line being run, for reports */

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

static int is_ram(uint32_t phys) { return ram_bank[phys >> 13] != NULL; }
static int is_rom(uint32_t phys) { return rom_bank[phys >> 13] != NULL && ram_bank[phys >> 13] == NULL; }

/* ------------------------------------------------------------------------------------------ */
/* The stream: the record's side.                                                              */

static FILE *out;                       /* the record's stream */
static uint64_t emitted;

static void wire_write(const Ev *e)
{
    uint8_t head[8] = { e->type, e->val, e->old, e->kind, (uint8_t)e->addr, (uint8_t)(e->addr >> 8),
                        (uint8_t)(e->addr >> 16), (uint8_t)(e->addr >> 24) };

    fwrite(head, 1, 8, out);
    if (e->type == EV_HWR || e->type == EV_HWW) {
        fwrite(&e->clock, 8, 1, out);
    } else if (is_marker(e->type)) {
        fwrite(&e->clock, 8, 1, out);
        fwrite(&e->hash2, 8, 1, out);
        fwrite(&e->pc, 2, 1, out);
        fwrite(&e->ret, 2, 1, out);
        fwrite(&e->a, 1, 6, out);
        fwrite(e->mpr, 1, 8, out);
    }
    emitted++;
}

static int wire_read(FILE *f, Ev *e)
{
    uint8_t head[8];

    if (fread(head, 1, 8, f) != 8)
        return 0;
    memset(e, 0, sizeof *e);
    e->type = head[0];
    e->val = head[1];
    e->old = head[2];
    e->kind = head[3];
    e->addr = head[4] | (head[5] << 8) | (head[6] << 16) | ((uint32_t)head[7] << 24);
    if (e->type == EV_HWR || e->type == EV_HWW) {
        if (fread(&e->clock, 8, 1, f) != 1)
            return 0;
    } else if (is_marker(e->type)) {
        if (fread(&e->clock, 8, 1, f) != 1 || fread(&e->hash2, 8, 1, f) != 1 ||
            fread(&e->pc, 2, 1, f) != 1 || fread(&e->ret, 2, 1, f) != 1 ||
            fread(&e->a, 1, 6, f) != 6 || fread(e->mpr, 1, 8, f) != 8)
            return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------------------------------ */
/* The event store: A5 keeps the stream in an array it fills; the hybrid reads it from the     */
/* record through a window.                                                                     */

static Ev *win;
static uint64_t wbase, wn, wcap;
static int end_seen;                    /* the record's closing marker: what it opened, how many frames, how many left open */
static uint64_t end_opens, end_frames, end_left;
static int at_eof;
static FILE *in;

/* Frames the hybrid has read ahead of its own: the record's hashes. */
static struct { uint64_t ram, pic; } *fq;
static uint64_t fq_head, fq_tail, fq_cap;

static void store_push(const Ev *e)
{
    if (wn == wcap) {
        wcap = wcap ? wcap * 2 : (1u << 16);
        win = realloc(win, wcap * sizeof *win);
        if (!win) {
            fprintf(stderr, "check: out of memory in the event window\n");
            exit(5);
        }
    }
    win[wn++] = *e;
}

/* Reads one event from the record's stream into the window. */
static int fetch_one(void)
{
    Ev e;

    if (at_eof)
        return 0;
    if (!wire_read(in, &e)) {
        at_eof = 1;
        return 0;
    }
    if (e.type == EV_END) {
        end_seen = 1;
        end_opens = e.clock;
        end_frames = e.hash2;
        end_left = e.addr;
    }
    if (e.type == EV_FRAME) {
        if (fq_tail == fq_cap) {
            fq_cap = fq_cap ? fq_cap * 2 : 1024;
            fq = realloc(fq, fq_cap * sizeof *fq);
        }
        fq[fq_tail].ram = e.clock;
        fq[fq_tail].pic = e.hash2;
        fq_tail++;
    }
    store_push(&e);
    return 1;
}

/* The event at an absolute index, reading ahead as far as it takes; NULL past the end. */
static Ev *evp(uint64_t i)
{
    if (mode == MODE_A5)
        return i < wn ? &win[i] : NULL;
    if (i < wbase) {
        fprintf(stderr, "check: event %llu was trimmed (window starts at %llu)\n", (unsigned long long)i,
                (unsigned long long)wbase);
        exit(5);
    }
    while (i >= wbase + wn)
        if (!fetch_one())
            return NULL;
    return &win[i - wbase];
}

/* ------------------------------------------------------------------------------------------ */
/* Spans: the part of the stream one call's C is matched against.                              */

typedef struct {
    uint64_t start, cur, end;          /* end: the CLOSE's index; unknown (UINT64_MAX) until the scan reaches it */
    int tag;
    int nest;                          /* OPENs minus CLOSEs between start and cur (hybrid) */
    uint64_t defects;
} Span;

#define SPAN_OPEN UINT64_MAX

static uint16_t last_pc, before_pc;   /* the PC of the last instruction the hook saw, and the one before the arrival */

static Ev *evp(uint64_t i);

/*
 * The event at j for a scan of a span: NULL at the span's end (its own CLOSE, which sets end) or
 * where the record stops. *nest counts the frames the scan has gone into.
 */
static Ev *sev(Span *s, uint64_t j, int *nest)
{
    Ev *e;

    if (j >= s->end || !(e = evp(j)))
        return NULL;
    if (mode != MODE_A5) {
        if (e->type == EV_OPEN) {
            (*nest)++;
        } else if (e->type == EV_CLOSE) {
            if (*nest == 0) {
                s->end = j;
                return NULL;
            }
            (*nest)--;
        }
    }
    return e;
}

static Span *sp;                       /* the span whose C is running */

/* The record ends inside this call: at its end, or at the CLOSE that flushes a call the script left open. */
static int record_ends_here(const Span *s)
{
    Ev *c;

    if (at_eof)
        return 1;
    return s->end != SPAN_OPEN && (c = evp(s->end)) && c->type == EV_CLOSE && c->kind == CL_LEFT;
}
enum { ABORT_NONE, ABORT_DIVERGED, ABORT_RUNAWAY, ABORT_END };
static int prefix_mode;                /* A5: judging a call open at the script's end: the log ending is not a defect */
static jmp_buf c_abort;
static int blame_tag = -1;             /* the innermost tagged routine whose C is running */

static char why[256];

/* A defect: counted, and the first few of a routine printed. */
static void defect(int kind, const char *fmt, ...)
{
    char text[300];
    va_list ap;
    Stat *s;
    /* The defect is the innermost tagged routine's whose C made the access: that is the routine a writer
     * opens. A call whose C lacks a frame shows up in its callee; the call judged (or swapped) is named in
     * the text when it is another. The judged call's own verdict counts it too (sp->defects). */
    int tag = blame_tag >= 0 ? blame_tag : (sp ? sp->tag : 0);
    size_t used;

    va_start(ap, fmt);
    used = (size_t)vsnprintf(text, sizeof text, fmt, ap);
    va_end(ap);
    if (sp && sp->tag != tag && used < sizeof text)
        snprintf(text + used, sizeof text - used, " [in the call of %s]", rondo_tags[sp->tag].name);
    s = &stats[tag];
    s->kind[kind]++;
    if (sp)
        sp->defects++;
    if (s->printed < 6) {
        s->printed++;
        fprintf(report, "%s %s call %llu frame %lu%s%s: %s\n", rondo_tags[tag].name, kind_name[kind],
                (unsigned long long)s->calls, cur_frame, script_where[0] ? " at " : "", script_where, text);
    }
}

void check_finish(void);
static void stop_context(uint64_t from);
static void park_forever(void);
static unsigned long extra_run;
static const char *stopped_why;

/* The hybrid can not go on in step: the report and the totals so far are written and the run ends. */
static void stop_run(int code, const char *fmt, ...)
{
    static char text[300];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(text, sizeof text, fmt, ap);
    va_end(ap);
    fprintf(report, "stopped at frame %lu: %s\n", cur_frame, text);
    stopped_why = text;
    fflush(report);
    check_finish();
    exit(code);
}

/* The access an event stands for, for a message. */
static const char *describe(const Ev *e, char *buf, size_t n)
{
    switch (e->type) {
    case EV_R: snprintf(buf, n, "read %06X (%02X)", e->addr, e->val); break;
    case EV_W: snprintf(buf, n, "write %06X (%02X)", e->addr, e->val); break;
    case EV_HWR: snprintf(buf, n, "hardware read %06X (%02X)", e->addr, e->val); break;
    case EV_HWW: snprintf(buf, n, "hardware write %06X (%02X)", e->addr, e->val); break;
    case EV_STATE: snprintf(buf, n, "MPR or clock change"); break;
    default: snprintf(buf, n, "event %d", e->type); break;
    }
    return buf;
}

static const char *describe_c(int type, uint32_t phys, char *buf, size_t n)
{
    if (type == EV_STATE)
        snprintf(buf, n, "MPR or clock change");
    else
        snprintf(buf, n, "%s %06X",
                 type == EV_R ? "read" : type == EV_W ? "write" : type == EV_HWR ? "hardware read" : "hardware write",
                 phys);
    return buf;
}

/* ------------------------------------------------------------------------------------------ */
/* Placement: one C access against the stream.                                                  */

static void (*int_pass)(Span *s, uint64_t int_idx, uint64_t iret_idx);
static void do_pass(Span *s, uint64_t i, uint64_t r);
static void frame_pass(Span *s, uint64_t idx, unsigned pulls);
static unsigned long place_from = (unsigned long)-1, place_to;   /* RONDO_PLACE=lo,hi, frames */
static unsigned long skip_frame = (unsigned long)-1;

#define NO_RETURN UINT64_MAX
#define PASSES 256

/* The IRET that closes the interrupt at index i; NO_RETURN if the handler does not return in this call. */
static uint64_t section_end(Span *s, uint64_t i)
{
    int depth = 1, open = 0;

    for (uint64_t k = i + 1; k < s->end; k++) {
        Ev *e = evp(k);

        if (!e)
            return k;           /* the record ends inside the handler (the script ended there): it never returns */
        if (e->type == EV_INT)
            depth++;
        else if (e->type == EV_IRET && --depth == 0)
            return k;
        else if (e->type == EV_OPEN)
            open++;
        else if (e->type == EV_CLOSE && open-- == 0 && mode != MODE_A5)
            return e->kind == CL_LEFT ? k : NO_RETURN;    /* a call left open at the record's end closes there */
    }
    /* A5, judging a call still open at the end of the log: the handler had not returned either, and what the
     * call did so far is all that is judged. */
    return mode == MODE_A5 && prefix_mode ? s->end : NO_RETURN;
}

static int same_state(const Ev *e, const uint8_t *mpr, uint8_t speed)
{
    return !memcmp(e->mpr, mpr, 8) && e->speed == speed;
}

/*
 * A5: a poke of the script that landed while the call was open is a write by something outside the program, at a
 * place in the original's sequence. The C's memory takes it when the C's accesses pass that place, so a byte the
 * C wrote before it and the script then overwrote ends as the original's did. It is no write of either side: it
 * is not placed, not counted, and not undone as the C's.
 */
static void apply_pokes(uint64_t from, uint64_t to)
{
    if (mode != MODE_A5)
        return;
    for (uint64_t k = from; k < to; k++) {
        Ev *e = evp(k);

        if (e && is_poke(e))
            ram_bank[e->addr >> 13][e->addr & 0x1FFF] = e->val;
    }
}

/*
 * Places a counted access of the C: the index of the event it matches, or -1 when the original made
 * no such access before a barrier. Interrupts passed on the way are delivered before it returns.
 * A match that is not the next counted event is a MOVED defect; none at all is EXTRA.
 */
static int64_t place(Span *s, int type, uint32_t phys, const uint8_t *mpr, uint8_t speed)
{
    uint64_t j = s->cur;
    int nest = s->nest;
    uint64_t ints[PASSES], irets[PASSES];
    unsigned nints = 0;
    int64_t found = -1, first = -1;
    unsigned skipped = 0;
    int hw = type == EV_HWR || type == EV_HWW;
    uint64_t from = s->cur;
    char a[96], b[96];
    int watching = place_from != (unsigned long)-1 && cur_frame >= place_from && cur_frame <= place_to;

    int hit_end = 0;

    for (;; j++) {
        Ev *e = sev(s, j, &nest);

        if (!e) {
            hit_end = 1;
            break;
        }
        if (is_noise(e))
            continue;
        if (e->type == EV_INT) {
            uint64_t r = section_end(s, j);

            if (r == NO_RETURN) {
                defect(D_INT, "an interrupt the call took does not return in the call");
                break;
            }
            if (nints == PASSES) {
                defect(D_INT, "more than %d interrupts or frame ends passed in one placement", PASSES);
                break;
            }
            ints[nints] = j;
            irets[nints++] = r;
            j = r;
            continue;
        }
        if (e->type == EV_FRAME && mode == MODE_HYBRID) {
            if (nints == PASSES) {
                defect(D_INT, "more than %d interrupts or frame ends passed in one placement", PASSES);
                break;
            }
            ints[nints] = j;
            irets[nints++] = j;
            continue;
        }
        if (e->type == EV_CALL && e->kind == K_JSR && tag_pick(e->pc) < 0 && !hw) {
            /* The original called code the C has not: the C must say so (call_original) first. */
            break;
        }
        if (!is_counted(e))
            continue;
        if (first < 0)
            first = (int64_t)j;
        if (e->type == type && (type == EV_STATE ? same_state(e, mpr, speed) : e->addr == phys)) {
            found = (int64_t)j;
            break;
        }
        skipped++;
        if (!hw && type != EV_STATE && (e->type == EV_HWR || e->type == EV_HWW || e->type == EV_STATE))
            break;
    }
    if (found < 0 && hit_end && first < 0 && mode == MODE_HYBRID && sp == s && record_ends_here(s))
        park_forever();
    if (found >= 0) {
        extra_run = 0;
    } else if (mode == MODE_HYBRID && ++extra_run > 200000) {
        /* A C that keeps making accesses the original did not (a spin the record has no iterations of) never
         * lets time pass: the run is stopped with a report, not left to spin. */
        stop_context(s->cur);
        stop_run(3, "the C made %lu accesses in a row that the original did not (a spin or a runaway)", extra_run);
    }
    if (found < 0 && hit_end && first < 0 && mode == MODE_A5 && prefix_mode)
        longjmp(c_abort, ABORT_END);          /* the call was still open when the script ended: its accesses so far held */
    if (found < 0) {
        if (first >= 0)
            defect(D_EXTRA, "C %s; the original's next is %s at access %llu of the call",
                   describe_c(type, phys, a, sizeof a), describe(evp((uint64_t)first), b, sizeof b),
                   (unsigned long long)((uint64_t)first - s->start));
        else
            defect(D_EXTRA, "C %s; the original made no more accesses in the call", describe_c(type, phys, a, sizeof a));
        if (watching)
            fprintf(report, "PLACE %lu %s %06X %llu -> %llu\n", cur_frame, type == EV_R || type == EV_HWR ? "r" : "w",
                    phys, (unsigned long long)from, (unsigned long long)from);
        return -1;
    }
    if (found != first)
        defect(D_MOVED, "C %s placed at access %lld, past %u of the original's accesses it has not made first: %s at %lld "
               "(a read the C skips is reported here, on the access after it)",
               describe_c(type, phys, a, sizeof a), (long long)(found - (int64_t)s->start), skipped,
               describe(evp((uint64_t)first), b, sizeof b), (long long)(first - (int64_t)s->start));
    if (watching)
        fprintf(report, "PLACE %lu %s %06X %llu -> %llu\n", cur_frame, type == EV_R || type == EV_HWR ? "r" : "w", phys,
                (unsigned long long)from, (unsigned long long)found + 1);
    if (nints && found != first && skip_frame != (unsigned long)-1 && cur_frame >= skip_frame) {
        fprintf(report, "SKIP frame %lu in %s: %s %06X placed at %lld from %llu, past an interrupt at %llu\n", cur_frame,
                rondo_tags[blame_tag >= 0 ? blame_tag : s->tag].name, type == EV_R || type == EV_HWR ? "read" : "write",
                phys, (long long)found, (unsigned long long)from, (unsigned long long)ints[0]);
        skip_frame = (unsigned long)-1;
    }
    apply_pokes(from, (uint64_t)found);
    s->cur = (uint64_t)found + 1;
    s->nest = nest;
    for (unsigned i = 0; i < nints; i++) {
        uint64_t keep = s->cur;

        do_pass(s, ints[i], irets[i]);
        s->cur = keep;
    }
    return found;
}

/* A call's end: what the original did that the C did not is missing; interrupts left are delivered. */
static void span_finish(Span *s)
{
    int reported = 0;
    int nest = s->nest;
    unsigned pulls = 0;

    for (uint64_t j = s->cur;; j++) {
        Ev *e = sev(s, j, &nest);
        char b[96];

        if (!e)
            break;
        if (e->type == EV_RTPULL)
            pulls++;
        if (e->type == EV_INT) {
            uint64_t r = section_end(s, j);

            if (r == NO_RETURN) {
                defect(D_INT, "an interrupt the call took does not return in the call");
                break;
            }
            int_pass(s, j, r);
            j = r;
        } else if (e->type == EV_FRAME && mode == MODE_HYBRID) {
            frame_pass(s, j, pulls);
        } else if (is_counted(e) && !reported) {
            defect(D_MISSING, "the original's %s at access %llu of the call is not made", describe(e, b, sizeof b),
                   (unsigned long long)(j - s->start));
            reported = 1;
        }
    }
    if (s->end == SPAN_OPEN)
        s->end = wbase + wn;
    s->cur = s->end;
}

/* ------------------------------------------------------------------------------------------ */
/* A5's machine.                                                                                */

typedef struct {
    const Tag *tag;
    int tag_index;
    int kind;              /* K_JSR or K_IRQ */
    uint64_t start;
    uint16_t ret;
    Regs regs;
    uint8_t mpr[8];
    uint8_t speed;
} Frame;

static Frame frames[FRAMES];
static int depth;

static uint8_t mshadow[8];
static uint8_t c_speed;
static struct { uint32_t phys; uint8_t old; } cw[C_MAX];
static uint32_t cwn;
static uint64_t c_budget;


static const Regs *exit_regs_a5;

static void stop_c(int reason, const char *fmt, uint32_t a, uint32_t b, uint32_t c)
{
    snprintf(why, sizeof why, fmt, a, b, c);
    longjmp(c_abort, reason);
}

static uint32_t a5_phys(uint16_t addr)
{
    return canon(((uint32_t)mshadow[addr >> 13] << 13) | (addr & 0x1FFF));
}

static void ram_put(uint32_t p, uint8_t value)
{
    if (cwn >= C_MAX)
        stop_c(ABORT_RUNAWAY, "C wrote past its record", 0, 0, 0);
    cw[cwn].phys = p;
    cw[cwn++].old = ram_bank[p >> 13][p & 0x1FFF];
    ram_bank[p >> 13][p & 0x1FFF] = value;
}

/* A5: the original's interrupt handler wrote this; put it where the C will read it. */
static void a5_int_pass(Span *s, uint64_t i, uint64_t r)
{
    for (uint64_t k = i + 1; k < r && k < s->end; k++) {
        Ev *e = evp(k);

        if (e && e->type == EV_W)
            ram_bank[e->addr >> 13][e->addr & 0x1FFF] = e->val;
    }
}

/* ------------------------------------------------------------------------------------------ */
/* The hybrid's coroutines.                                                                     */

#define CO_STACK (512u * 1024u)
#define FS_MAX 96

typedef struct Ctx {
    ucontext_t uc;
    void *stack;
    Span span;
    int tag;
    int kind;
    uint8_t entry_s;
    uint64_t open_i, close_i;
    Cpu saved_cpu;
    int parked, done, abandoned, natural;
    uint16_t resume_pc;
    uint8_t resume_s;
    unsigned long wait_frame;      /* parked until the core has ended this frame */
    uint64_t wait_clock;
    int fs[FS_MAX], fsn;
    struct Ctx *below;
} Ctx;

static ucontext_t main_uc;
static Ctx *top;
static Ctx *free_ctx;
static uint64_t scan;                      /* where the next tagged entry's OPEN is looked for */
static int in_swap = -1;
static uint64_t *ran_inside;
static int *cur_fs;
static int *cur_fsn;

static uint64_t hybrid_clock_now(void) { return chk_ts_abs(); }

/* Brings the hybrid's clock to the original's at an access of hardware. */
static void pace_to(uint64_t want)
{
    uint64_t now = hybrid_clock_now();

    if (want > now) {
        uint64_t d = want - now;

        chk_charge((int)(d > 0x7FFFFFFF ? 0x7FFFFFFF : d));
    }
}

/*
 * The one primitive: park the C, with the emulated CPU set up (by the caller) to run what must
 * run. The hook resumes the C when the CPU reaches resume_pc with S resume_s after an RTS or RTI.
 */
static void cpu_run_until(uint16_t resume_pc, uint8_t resume_s)
{
    Ctx *c = top;

    c->parked = 1;
    c->resume_pc = resume_pc;
    c->resume_s = resume_s;
    c->saved_cpu = cpu;
    swapcontext(&c->uc, &main_uc);
    cpu = c->saved_cpu;
}

/* Dispatches the interrupt the original took at event i: charge to its clock, hand the handler
 * the original's registers, and run the machine until it returns here. */
static void hy_int_pass(Span *s, uint64_t i, uint64_t r)
{
    Ev *e = evp(i);
    Ev ev;
    uint64_t entry_idx = i + 1;
    uint16_t handler = 0;
    uint8_t mpr[8];

    if (!e)
        return;
    if (top && top->natural) {
        /* The core took this interrupt itself, at the instruction the C was waiting to resume at. */
        top->natural--;
        return;
    }
    ev = *e;
    for (uint64_t k = i + 1; k < r; k++) {
        Ev *x = evp(k);

        if (x && x->type == EV_CALL && x->kind == K_IRQ) {
            handler = x->pc;
            entry_idx = k;
            break;
        }
    }
    stats[s->tag].ints++;
    pace_to(ev.clock);
    /* The interrupt's own eight cycles (seven, and the one that samples the next), which the core spends before the
     * handler's first instruction: 3 master clocks a cycle at the fast clock, 12 at the slow. */
    chk_charge(8 * (ev.speed ? 3 : 12));
    for (unsigned k = 0; k < 8; k++)
        mpr[k] = (uint8_t)chk_mpr(k);
    if (memcmp(mpr, ev.mpr, 8) || chk_get_speed() != ev.speed)
        defect(D_INT, "at the interrupt the MPRs or the CPU clock differ from the original's");
    if (cpu.s != ev.s)
        defect(D_STACK, "at the interrupt S is %02X, the original's %02X", cpu.s, ev.s);
    /* The interrupt's own pushes on the live stack, as the CPU makes them. */
    chk_bus_wr((uint16_t)(0x2100 | ev.s), (uint8_t)(ev.pc >> 8));
    chk_bus_wr((uint16_t)(0x2100 | (uint8_t)(ev.s - 1)), (uint8_t)ev.pc);
    chk_bus_wr((uint16_t)(0x2100 | (uint8_t)(ev.s - 2)), (uint8_t)(ev.p & ~P_B));
    chk_set_regs(ev.a, ev.x, ev.y, (uint8_t)(ev.s - 3), (uint8_t)((ev.p | P_I) & ~(P_T | P_D)), handler);
    prev_op = PREV_IRQ;
    last_pc = 0xFFFF;
    scan = entry_idx + 1;
    cpu_run_until(ev.pc, ev.s);
    scan = r + 1;
}

/*
 * The original's frame ended here, between two accesses of the call: park the C until the core has ended
 * its frame too, with the machine's S and the clock where the original's were, so the frame's hash is
 * taken with the call half done, as the record's was. The interrupts are masked meanwhile: the one the
 * record took after the frame end is dispatched by the C at its place, not by the core at this PC.
 */
static void frame_pass(Span *s, uint64_t idx, unsigned pulls)
{
    Ev *e = evp(idx);
    Ctx *c = top;
    uint8_t a, x, y, st, p;
    uint16_t pc;

    (void)s;
    if (!e || !c || cur_frame >= e->addr)
        return;
    memcpy(&c->wait_clock, e->mpr, 8);
    c->wait_frame = e->addr;
    chk_get_regs(&a, &x, &y, &st, &p, &pc);
    /* pulls: the bytes of its return address the original's RTS or RTI had taken when the frame ended. */
    chk_set_regs(cpu.a, cpu.x, cpu.y, (uint8_t)(cpu.s + pulls), (uint8_t)(cpu.p | P_I), pc);
    c->parked = 1;
    c->saved_cpu = cpu;
    swapcontext(&c->uc, &main_uc);
    cpu = c->saved_cpu;
}

/* Passes the interrupt or frame end at index i of the stream. */
static void do_pass(Span *s, uint64_t i, uint64_t r)
{
    Ev *e = evp(i);

    if (e && e->type == EV_FRAME)
        frame_pass(s, i, 0);
    else
        int_pass(s, i, r);
}

/* ------------------------------------------------------------------------------------------ */
/* The C's machine.                                                                             */

static unsigned hy_bank_phys(uint16_t addr, uint32_t *phys)
{
    *phys = canon(((uint32_t)chk_mpr(addr >> 13) << 13) | (addr & 0x1FFF));
    return *phys >> 13;
}

uint8_t rd8(uint16_t addr)
{
    uint32_t p;

    if (mode == MODE_HYBRID) {
        Ev *e;
        int64_t j;
        uint8_t v;

        hy_bank_phys(addr, &p);
        if (!sp || is_rom(p))
            return chk_bus_rd(addr);
        j = place(sp, is_ram(p) ? EV_R : EV_HWR, p, NULL, 0);
        e = j >= 0 ? evp((uint64_t)j) : NULL;
        if (e && !is_ram(p))
            pace_to(e->clock);
        v = chk_bus_rd(addr);
        if (e && e->val != v) {
            char b[96];

            defect(D_VALUE, "C read %06X gave %02X, the original's %s", p, v, describe(e, b, sizeof b));
        }
        return v;
    }
    if (++c_budget > 4000000)
        stop_c(ABORT_RUNAWAY, "C ran past its access budget", 0, 0, 0);
    p = a5_phys(addr);
    if (is_rom(p))
        return rom_bank[p >> 13][p & 0x1FFF];
    if (is_ram(p)) {
        int64_t j = place(sp, EV_R, p, NULL, 0);
        uint8_t v = ram_bank[p >> 13][p & 0x1FFF];

        if (j >= 0) {
            Ev *e = evp((uint64_t)j);

            if (e->val != v) {
                char b[96];

                defect(D_VALUE, "C read %06X gave %02X, the original's %s", p, v, describe(e, b, sizeof b));
            }
        }
        return v;
    }
    {
        int64_t j = place(sp, EV_HWR, p, NULL, 0);

        if (j < 0)
            stop_c(ABORT_DIVERGED, "C reads hardware %06X; the original made no such read here", p, 0, 0);
        return evp((uint64_t)j)->val;
    }
}

void wr8(uint16_t addr, uint8_t value)
{
    uint32_t p;

    if (mode == MODE_HYBRID) {
        int64_t j;
        Ev *e;

        hy_bank_phys(addr, &p);
        if (!sp || is_rom(p)) {
            chk_bus_wr(addr, value);
            return;
        }
        j = place(sp, is_ram(p) ? EV_W : EV_HWW, p, NULL, 0);
        e = j >= 0 ? evp((uint64_t)j) : NULL;
        if (e && !is_ram(p))
            pace_to(e->clock);
        if (e && e->val != value) {
            char b[96];

            defect(D_VALUE, "C wrote %06X with %02X, the original's %s", p, value, describe(e, b, sizeof b));
        }
        chk_bus_wr(addr, value);
        return;
    }
    if (++c_budget > 4000000)
        stop_c(ABORT_RUNAWAY, "C ran past its access budget", 0, 0, 0);
    p = a5_phys(addr);
    if (is_ram(p)) {
        int64_t j = place(sp, EV_W, p, NULL, 0);

        if (j >= 0) {
            Ev *e = evp((uint64_t)j);

            if (e->val != value) {
                char b[96];

                defect(D_VALUE, "C wrote %06X with %02X, the original's %s", p, value, describe(e, b, sizeof b));
            }
        }
        ram_put(p, value);
    } else if (!is_rom(p)) {
        int64_t j = place(sp, EV_HWW, p, NULL, 0);
        Ev *e;

        if (j < 0)
            stop_c(ABORT_DIVERGED, "C writes hardware %06X; the original made no such write here", p, 0, 0);
        e = evp((uint64_t)j);
        if (e->val != value)
            stop_c(ABORT_DIVERGED, "hardware write %06X: C %02X, original %02X", p, value, e->val);
    }
}

static void st_reg(unsigned which, uint32_t phys, uint8_t value)
{
    int64_t j = sp ? place(sp, EV_HWW, phys, NULL, 0) : -1;
    Ev *e = j >= 0 ? evp((uint64_t)j) : NULL;

    if (mode == MODE_HYBRID) {
        if (e)
            pace_to(e->clock);
        if (e && e->val != value)
            defect(D_VALUE, "C wrote hardware %06X with %02X, the original %02X", phys, value, e->val);
        chk_bus_st(which, value);
        return;
    }
    if (!e)
        stop_c(ABORT_DIVERGED, "C writes hardware %06X; the original made no such write here", phys, 0, 0);
    if (e->val != value)
        stop_c(ABORT_DIVERGED, "hardware write %06X: C %02X, original %02X", phys, value, e->val);
}

void st0(uint8_t value) { st_reg(0, 0x1FE000, value); }
void st1(uint8_t value) { st_reg(2, 0x1FE002, value); }
void st2(uint8_t value) { st_reg(3, 0x1FE003, value); }

static void mprs_now(uint8_t *v)
{
    if (mode == MODE_HYBRID)
        for (unsigned i = 0; i < 8; i++)
            v[i] = (uint8_t)chk_mpr(i);
    else
        memcpy(v, mshadow, 8);
}

uint8_t tma(uint8_t mpr_bit)
{
    uint8_t v[8];
    uint8_t a = 0;

    mprs_now(v);
    /* TMA with several bits set: the core loads A from each in turn, so the highest set bit's bank is the result
     * (huc6280.cpp, the TMA macro); no bit set leaves A as it was, which the C's caller sees as 0 here. */
    for (unsigned i = 0; i < 8; i++)
        if (mpr_bit & (1u << i))
            a = v[i];
    return a;
}

void tam(uint8_t mpr_bits, uint8_t bank)
{
    uint8_t v[8];

    mprs_now(v);
    for (unsigned i = 0; i < 8; i++)
        if (mpr_bits & (1u << i))
            v[i] = bank;
    if (sp)
        place(sp, EV_STATE, 0, v, mode == MODE_HYBRID ? chk_get_speed() : c_speed);
    for (unsigned i = 0; i < 8; i++)
        if (mpr_bits & (1u << i)) {
            if (mode == MODE_HYBRID)
                chk_set_mpr(i, bank);
            else
                mshadow[i] = bank;
        }
}

/*
 * TAI: a block move pushes Y, A and X, moves the bytes (a read of the source, alternating between its two
 * bytes, and a write of the destination, each), and pulls X, A and Y. Those six accesses are the core's
 * (huc6280.cpp, LD_BMT) and the original makes them, so the C does: the registers the C holds at the call are
 * the ones pushed.
 */
void tai(uint16_t src, uint16_t dst, uint16_t len)
{
    push8(cpu.y);
    push8(cpu.a);
    push8(cpu.x);
    for (unsigned i = 0; i < len; i++) {
        uint8_t v = rd8((uint16_t)(src + (i & 1)));

        wr8((uint16_t)(dst + i), v);
    }
    cpu.x = pull8();
    cpu.a = pull8();
    cpu.y = pull8();
}

void cli(void)
{
    cpu.p &= (uint8_t)~P_I;
}

static void set_speed(uint8_t fast)
{
    uint8_t v[8];

    mprs_now(v);
    if (sp)
        place(sp, EV_STATE, 0, v, fast);
    if (mode == MODE_HYBRID)
        chk_set_speed(fast);
    else
        c_speed = fast;
}

void csl(void) { set_speed(0); }
void csh(void) { set_speed(1); }

/*
 * wait_interrupt: the original spins with no data access (a jump or branch to itself) until an
 * interrupt. The next counted event of the span must be that interrupt's; it is delivered here.
 */
/*
 * The record ends inside this call (the script ended there): the C parks and is never resumed, the script's last
 * frames going on with it parked as the original's did.
 */
static void park_forever(void)
{
    Ctx *c = top;
    uint8_t a, x, y, st, p;
    uint16_t pc;

    c->wait_frame = (unsigned long)-1;
    c->wait_clock = hybrid_clock_now() + 1;
    chk_get_regs(&a, &x, &y, &st, &p, &pc);
    chk_set_regs(cpu.a, cpu.x, cpu.y, cpu.s, (uint8_t)(cpu.p | P_I), pc);
    c->parked = 1;
    c->saved_cpu = cpu;
    swapcontext(&c->uc, &main_uc);
    for (;;)
        ;
}

void wait_interrupt(void)
{
    uint64_t j;
    Ev *e = NULL;
    int nest;

    if (!sp) {
        stop_run(3, "wait_interrupt outside a swapped call");
    }
    stats[sp->tag].waits++;
    nest = sp->nest;
    for (j = sp->cur; (e = sev(sp, j, &nest)) && !is_counted(e) && e->type != EV_INT; j++)
        if (e->type == EV_FRAME && mode == MODE_HYBRID)
            frame_pass(sp, j, 0);
    if (!e && mode == MODE_HYBRID && record_ends_here(sp))
        park_forever();
    if (!e || e->type != EV_INT) {
        if (mode == MODE_A5) {
            if (!e && prefix_mode)
                longjmp(c_abort, ABORT_END);
            snprintf(why, sizeof why, "the C waits for an interrupt; the original's next event is not one");
            longjmp(c_abort, ABORT_DIVERGED);
        }
        {
            char b[96];

            defect(D_CALL, "the C waits for an interrupt; the original's next is %s, at access %llu of the call",
                   e ? describe(e, b, sizeof b) : "the end of the record", (unsigned long long)(e ? j - sp->start : 0));
        }
        stop_context(sp->cur);
        stop_run(3, "a wait with no interrupt in the record");
    }
    {
        uint64_t r = section_end(sp, j);

        if (r == NO_RETURN) {
            if (mode == MODE_A5) {
                snprintf(why, sizeof why, "the interrupt the C waits for does not return in the call");
                longjmp(c_abort, ABORT_DIVERGED);
            }
            defect(D_INT, "an interrupt the call took does not return in the call");
            stop_run(3, "a wait whose interrupt does not return in the call");
        }
        sp->cur = r + 1;
        int_pass(sp, j, r);
        sp->cur = r + 1;
    }
}

/*
 * call_original(addr): run code that has no C yet, with the registers as cpu holds them. The
 * original made the call at a CALL event of the call's stream; the counted events before it that
 * the C has not made (the JSR's pushes among them) are missing, and the C states them with call_orig.
 */
static void original_at(uint16_t ret, uint16_t addr)
{
    uint64_t call = sp->cur, retidx;
    Ev ec;
    int found = 0;
    Span *span = sp;
    int nest = span->nest;
    int own = blame_tag < 0 || blame_tag == span->tag;     /* the routine whose call this is makes the call itself */
    int noreturn;

    for (uint64_t j = sp->cur;; j++) {
        Ev *x = sev(sp, j, &nest);

        if (!x)
            break;
        if (x->type == EV_INT) {
            j = section_end(sp, j);
            if (j == NO_RETURN)
                break;
            continue;
        }
        if (x->type == EV_CALL && x->pc == addr && x->kind != K_IRQ) {
            ec = *x;            /* a copy: delivering an interrupt below can move the window */
            found = 1;
            call = j;
            break;
        }
    }
    if (!found) {
        if (mode == MODE_A5)
            stop_c(ABORT_DIVERGED, "C calls %04X; the original made no such call", addr, 0, 0);
        defect(D_CALL, "C calls %04X; the original made no such call", addr);
        stop_run(3, "a call into original code the record does not have");
    }
    /* Everything counted before the call the C has not made. */
    for (uint64_t k = span->cur; k < call; k++) {
        Ev *x = evp(k);
        char b[96];

        if (x->type == EV_INT) {
            uint64_t r = section_end(span, k);

            if (r == NO_RETURN)
                break;
            int_pass(span, k, r);
            k = r;
            continue;
        }
        if (x->type == EV_FRAME && mode == MODE_HYBRID) {
            frame_pass(span, k, 0);
            continue;
        }
        if (is_counted(x)) {
            defect(D_MISSING, "before the call to %04X the original's %s is not made", addr, describe(x, b, sizeof b));
            break;
        }
    }
    if (ec.a != cpu.a || ec.x != cpu.x || ec.y != cpu.y || ((ec.p ^ cpu.p) & PFLAGS)) {
        if (mode == MODE_A5) {
            snprintf(why, sizeof why, "to %04X: C A%02X X%02X Y%02X P%02X, original A%02X X%02X Y%02X P%02X", addr,
                     cpu.a, cpu.x, cpu.y, cpu.p, ec.a, ec.x, ec.y, ec.p);
            longjmp(c_abort, ABORT_DIVERGED);
        }
        defect(D_CALL, "to %04X: C A%02X X%02X Y%02X P%02X, original A%02X X%02X Y%02X P%02X", addr, cpu.a, cpu.x,
               cpu.y, cpu.p, ec.a, ec.x, ec.y, ec.p);
    }
    /* Where it comes back: the RET that pops the callee's frame. A tail jump pushes none, and its code returns
     * to the caller of the routine that jumped. */
    for (retidx = call + 1;; retidx++) {
        Ev *x = sev(span, retidx, &nest);

        if (!x || (x->type == EV_RET && x->s == (uint8_t)(ec.s + 2) && (!ret || x->pc == ret)))
            break;
    }
    noreturn = retidx >= span->end || !evp(retidx);
    if (own && (ec.kind == K_JMP || noreturn)) {
        /* The routine's own tail jump, or code that does not come back in the call: the routine ends there.
         * (A routine called by C from another's C that jumps this way is a call and a return: below.) */
        if (mode == MODE_A5) {
            const Regs *x = exit_regs_a5;

            if (prefix_mode)
                longjmp(c_abort, ABORT_END);     /* still open at the script's end: nothing of its exit to compare */
            for (uint64_t k = call + 1; k < span->end; k++)
                if (evp(k)->type == EV_W)
                    ram_put(evp(k)->addr, evp(k)->val);
            cpu.a = x->a;
            cpu.x = x->x;
            cpu.y = x->y;
            cpu.p = x->p;
            for (unsigned i = 0; i < 8; i++)
                mshadow[i] = (uint8_t)chk_mpr(i);
            c_speed = chk_get_speed();
            span->cur = span->end;
            return;
        }
        {
            Ctx *c = top;

            stats[span->tag].originals++;
            pace_to(ec.clock);
            chk_set_regs(cpu.a, cpu.x, cpu.y, cpu.s, cpu.p, addr);
            if (ret) {      /* a call the original never came back from: the C's code after it is not the original's */
                stats[span->tag].noreturn++;
                longjmp(c_abort, ABORT_END);
            }
            prev_op = ec.kind == K_JMP ? 0x4C : 0x20;
            last_pc = rondo_tags[span->tag].run;     /* the jump comes from this routine, not from the code run before */
            scan = call + 1;
            c->abandoned = 1;
            swapcontext(&c->uc, &main_uc);
        }
    }
    if (noreturn) {
        /* A routine called from another's C jumps to code that never returns here: the C after it is wrong. */
        if (mode == MODE_A5)
            stop_c(ABORT_DIVERGED, "code called at %04X does not come back in this call, from a routine the call did not enter", addr, 0, 0);
        defect(D_CALL, "the code called at %04X does not come back in this call", addr);
        stop_run(3, "original code does not return");
    }
    if (mode == MODE_A5) {
        Ev *r = evp(retidx);

        for (uint64_t k = call + 1; k < retidx; k++) {
            Ev *x = evp(k);

            if (x->type == EV_W)
                ram_put(x->addr, x->val);
        }
        cpu.a = r->a;
        cpu.x = r->x;
        cpu.y = r->y;
        cpu.p = r->p;
        memcpy(mshadow, r->mpr, 8);
        c_speed = r->speed;
        span->cur = retidx + 1;
        span->nest = nest;
        return;
    }
    /* The hybrid: run the code on the live machine and come back where it returns. */
    {
        uint8_t s0 = cpu.s;
        uint16_t back = ret;

        pace_to(ec.clock);
        if (!ret) {
            /* No frame was stated: the code needs a return address on the stack all the same. */
            uint16_t r2 = ec.kind == K_JSR ? ec.ret : 0;

            chk_bus_wr((uint16_t)(0x2100 | s0), (uint8_t)((uint16_t)(r2 - 1) >> 8));
            chk_bus_wr((uint16_t)(0x2100 | (uint8_t)(s0 - 1)), (uint8_t)(r2 - 1));
            back = r2;
            s0 = (uint8_t)(s0 - 2);
        }
        stats[span->tag].originals++;
        chk_set_regs(cpu.a, cpu.x, cpu.y, s0, cpu.p, addr);
        prev_op = 0x20;
        last_pc = rondo_tags[span->tag].run;
        scan = call + 1;
        cpu_run_until(back, (uint8_t)(s0 + 2));
        {
            uint8_t a, x, y, s, p;
            uint16_t pc;

            chk_get_regs(&a, &x, &y, &s, &p, &pc);
            cpu.a = a;
            cpu.x = x;
            cpu.y = y;
            cpu.p = p;
        }
        span->cur = retidx + 1;
        span->nest = nest;
        scan = retidx + 1;
    }
}

/*
 * hand_over(addr): a drafted routine reached bytes no script ran (a branch arm or a fall-through). The C
 * ends here and the original runs on from addr with the registers as cpu holds them and S as the C's pushes
 * and pulls left it (so the original sees the stack it would have had); its RTS returns to the routine's
 * caller. A5 has nothing to compare it with: the call is DIVERGENT, naming the address. The hybrid hands
 * the machine over as a tail jump does and counts it.
 */
void hand_over(uint16_t addr)
{
    if (!sp)
        stop_run(3, "hand_over outside a swapped call");
    if (mode == MODE_A5) {
        snprintf(why, sizeof why, "the C reached %04X, bytes no script ran", addr);
        longjmp(c_abort, ABORT_DIVERGED);
    }
    {
        Ctx *c = top;

        stats[sp->tag].originals++;
        chk_set_regs(cpu.a, cpu.x, cpu.y, cpu.s, cpu.p, addr);
        prev_op = 0;
        scan = sp->cur;
        c->abandoned = 1;
        swapcontext(&c->uc, &main_uc);
    }
}

void call_original(uint16_t addr)
{
    original_at(0, addr);
}

void call_orig(uint16_t ret, uint16_t addr)
{
    push8((uint8_t)((uint16_t)(ret - 1) >> 8));
    push8((uint8_t)(ret - 1));
    original_at(ret, addr);
    cpu.s = (uint8_t)(cpu.s + 2);
}

/* ------------------------------------------------------------------------------------------ */
/* Instrumentation: which routine's C is running, for blame.                                    */

static struct { void *fn; int tag; } fn_cache[1 << 14];

static int tag_of_fn(void *fn)
{
    unsigned h = (unsigned)(((uintptr_t)fn >> 4) & ((1u << 14) - 1));
    static unsigned used;

    while (fn_cache[h].fn && fn_cache[h].fn != fn)
        h = (h + 1) & ((1u << 14) - 1);
    if (!fn_cache[h].fn) {
        if (used >= (1u << 14) - 64)
            return -1;      /* the table is full: an unnamed function is blamed on its caller */
        used++;
        Dl_info info;

        fn_cache[h].fn = fn;
        fn_cache[h].tag = -1;
        if (dladdr(fn, &info) && info.dli_sname) {
            const char *n = info.dli_sname;

            if (!strncmp(n, "abi_", 4))
                n += 4;
            for (unsigned i = 0; i < rondo_tag_count; i++)
                if (!strcmp(rondo_tags[i].name, n)) {
                    fn_cache[h].tag = (int)i;
                    break;
                }
        }
    }
    return fn_cache[h].tag;
}

void __cyg_profile_func_enter(void *fn, void *site)
{
    int tag;

    (void)site;
    if (!cur_fs)
        return;
    tag = tag_of_fn(fn);
    if (in_swap >= 0 && tag >= 0 && tag != in_swap) {
        if (!ran_inside)
            ran_inside = calloc(rondo_tag_count, sizeof *ran_inside);
        ran_inside[tag]++;
    }
    {
        int n = *cur_fsn;
        int below = n ? cur_fs[(n < FS_MAX ? n : FS_MAX) - 1] : -1;

        /* Deeper than FS_MAX the depth is still counted so the pops match; the name stays the last kept. */
        if (n < FS_MAX)
            cur_fs[n] = tag >= 0 ? tag : below;
        (*cur_fsn)++;
        blame_tag = tag >= 0 ? tag : below;
    }
}

void __cyg_profile_func_exit(void *fn, void *site)
{
    (void)fn;
    (void)site;
    if (!cur_fs)
        return;
    if (*cur_fsn > 0)
        (*cur_fsn)--;
    if (*cur_fsn > 0)
        blame_tag = cur_fs[(*cur_fsn < FS_MAX ? *cur_fsn : FS_MAX) - 1];
}

/* ------------------------------------------------------------------------------------------ */
/* The record's side of the bookkeeping, shared by the record run and A5.                       */

static uint8_t last_mpr[8];
static uint8_t last_speed;
static uint64_t cur_clock(void) { return chk_ts_abs(); }

static void judge(Frame *f, const Regs *r);

/*
 * The log is full (LOG_HARD events) with calls still open: a call that has stayed open for millions of accesses, as
 * the kernel's start does for the whole run. Its accesses so far are judged now, as far as the log goes (counted
 * under the routine as a prefix, and as unv_log when the outermost call is the only one), then the log from the next
 * call on is kept and the run goes on. Nothing is dropped without a count.
 */
static void trim_log(void)
{
    if (getenv("RONDO_TRIM_TRACE"))
        fprintf(stderr, "TRIM frame %lu wn %llu depth %d top %s\n", cur_frame, (unsigned long long)wn, depth, rondo_tags[frames[0].tag_index].name);
    uint64_t base;

    if (!depth) {
        wn = 0;
        return;
    }
    if (!getenv("RONDO_TRIM_NOJUDGE"))
        judge(&frames[0], NULL);
    stats[frames[0].tag_index].unv_log++;
    base = depth > 1 ? frames[1].start : wn;
    memmove(win, win + base, (wn - base) * sizeof *win);
    wn -= base;
    memmove(frames, frames + 1, (size_t)(depth - 1) * sizeof frames[0]);
    depth--;
    for (int i = 0; i < depth; i++)
        frames[i].start -= base;
}

static void log_event(const Ev *e)
{
    if (mode == MODE_A5) {
        if (wn == wcap && wcap >= LOG_HARD)
            trim_log();
        if (wn == wcap) {
            wcap *= 2;
            win = realloc(win, wcap * sizeof *win);
            if (!win) {
                fprintf(stderr, "check: out of memory for the A5 log\n");
                exit(5);
            }
        }
        win[wn++] = *e;
    } else if (mode == MODE_RECORD) {
        wire_write(e);
    }
}

static void marker(int type, const Regs *r, int kind, uint32_t addr, uint16_t ret)
{
    Ev e;

    memset(&e, 0, sizeof e);
    e.type = (uint8_t)type;
    e.kind = (uint8_t)kind;
    e.addr = addr;
    e.clock = cur_clock();
    e.pc = r->pc;
    e.ret = ret;
    e.a = r->a;
    e.x = r->x;
    e.y = r->y;
    e.s = r->s;
    e.p = r->p;
    e.speed = chk_get_speed();
    for (unsigned i = 0; i < 8; i++)
        e.mpr[i] = (uint8_t)chk_mpr(i);
    log_event(&e);
}

static void sync_state(const Regs *r)
{
    uint8_t m[8];
    uint8_t now = chk_get_speed();
    int changed = now != last_speed;

    for (unsigned i = 0; i < 8; i++) {
        m[i] = (uint8_t)chk_mpr(i);
        changed |= m[i] != last_mpr[i];
    }
    if (changed) {
        if (depth)
            marker(EV_STATE, r, 0, 0, 0);
        memcpy(last_mpr, m, 8);
        last_speed = now;
    }
}

void check_rd(uint32_t phys, uint8_t v, int kind)
{
    Ev e;

    if (kind == 2) {
        prev_op = v;
        return;
    }
    if (kind != 0 || !depth || (mode != MODE_A5 && mode != MODE_RECORD))
        return;
    phys = canon(phys);
    memset(&e, 0, sizeof e);
    e.addr = phys;
    e.val = v;
    if (is_ram(phys)) {
        e.type = (prev_op == 0x60 || prev_op == 0x40) && is_stack(phys) ? EV_RTPULL : EV_R;
    } else if (is_rom(phys)) {
        e.type = EV_ROMR;
    } else {
        e.type = EV_HWR;
        e.clock = cur_clock();
    }
    log_event(&e);
}

void check_wr_pre(uint32_t phys, uint8_t v)
{
    Ev e;

    if (!depth || (mode != MODE_A5 && mode != MODE_RECORD))
        return;
    phys = canon(phys);
    memset(&e, 0, sizeof e);
    e.addr = phys;
    e.val = v;
    if (is_ram(phys)) {
        e.type = EV_W;
        e.old = ram_bank[phys >> 13][phys & 0x1FFF];
    } else if (is_rom(phys)) {
        return;         /* a write to ROM does nothing: no program state, no access to place */
    } else {
        e.type = EV_HWW;
        e.clock = cur_clock();
    }
    log_event(&e);
}

/* Interrupts taken while a call is open. The pushes follow the marker. */
static struct { uint16_t resume; uint8_t s; } ints[FRAMES];
static int nints;

/*
 * Interrupts entered and not yet returned from, by the S the handler starts with (the three pushed bytes
 * above it). A routine entered by a JMP at that same S, as the card's interrupt handler jumps through the
 * game's vector, leaves by RTI: its frame is an interrupt's.
 */
static uint8_t ichain[FRAMES];
static int nchain;

/*
 * The interrupt can come between a JSR and the first instruction of its callee, or between an RTS and the
 * instruction after it: the hook for that instruction then sees the RTI that ended the handler. What the
 * interrupted instruction was is kept with the PC and S the handler returns to, and put back when it returns,
 * so the call is opened and the return marked as if the interrupt had not come between.
 */
static struct { uint16_t resume; uint8_t s; int prev; uint16_t from; } ipend[FRAMES];
static int nipend;

static uint16_t stack_return(uint8_t s)
{
    const uint8_t *base = ram_bank[0xF8];
    unsigned lo = base[0x100 + (uint8_t)(s + 1)];
    unsigned hi = base[0x100 + (uint8_t)(s + 2)];

    return (uint16_t)(((hi << 8) | lo) + 1);
}

static uint16_t irq_return(uint8_t s)
{
    const uint8_t *base = ram_bank[0xF8];

    return (uint16_t)(base[0x100 + (uint8_t)(s + 2)] | (base[0x100 + (uint8_t)(s + 3)] << 8));
}

static void boundary(const Regs *r);

void check_irq(void)
{
    Regs r;

    chk_get_regs(&r.a, &r.x, &r.y, &r.s, &r.p, &r.pc);
    if (nipend == FRAMES) {
        memmove(ipend, ipend + 1, (FRAMES - 1) * sizeof ipend[0]);
        nipend--;
    }
    ipend[nipend].resume = r.pc;
    ipend[nipend].s = r.s;
    if (getenv("RONDO_TRACE_ENTRY") && mode == MODE_HYBRID)
        fprintf(stderr, "IRQ frame %lu pc %04X S%02X prev_op %X top %s\n", cur_frame, r.pc, r.s, prev_op, top ? rondo_tags[top->tag].name : "-");
    ipend[nipend].from = last_pc;
    ipend[nipend++].prev = prev_op;
    if (mode == MODE_HYBRID && top && top->parked && !top->wait_frame && r.pc == top->resume_pc &&
        r.s == top->resume_s)
        top->natural++;      /* taken at the place the C waits for: the record's next interrupt is this one */
    if (mode == MODE_A5 || mode == MODE_RECORD) {
        /* The instruction just run may have been the call's RTS: the interrupt comes before the
         * hook for the instruction after it, so the call is closed here, outside the interrupt. */
        boundary(&r);
        if (depth) {
            sync_state(&r);
            marker(EV_INT, &r, 0, 0, 0);
            if (nints < FRAMES) {
                ints[nints].resume = r.pc;
                ints[nints].s = r.s;
                nints++;
            }
        }
    }
    prev_op = PREV_IRQ;
}

/*
 * A computed dispatch: the code pushes a routine's address less one and RTSes to it. The RTS lands on a tagged
 * routine's first byte without having been a return: no JSR left that address on the stack where it was popped
 * from (jsrmark). Such an arrival is a call, with the dispatcher's own return address below it.
 */
static uint16_t jsrmark[256];
static int computed_entry;
/*
 * Code can also run into a tagged routine without a call instruction: a branch or fall-through from other code (a
 * routine that ends by running on into the next), or the return of a call that lands on the next routine's first
 * byte. That is a jump into it, as a JMP is: no frame is pushed and the routine returns to the caller of the code it
 * came from. A branch back to a routine's own first byte (a loop) is not an entry.
 */
static int tail_entry;
static int self_jump;      /* a JMP from inside the routine to its own first byte: a loop, not a call */

static int in_routine(int index, uint16_t pc)
{
    return index >= 0 && (uint16_t)(pc - rondo_tags[index].run) < rondo_tags[index].len;
}

static int prev_is_entry(void)
{
    if (self_jump)
        return 0;
    return prev_op == 0x20 || prev_op == 0x44 || prev_op == 0x4C || prev_op == 0x6C || prev_op == 0x7C ||
           prev_op == PREV_UNIT || computed_entry || tail_entry;
}

static int entry_kind(void)
{
    if (prev_op == PREV_IRQ)
        return K_IRQ;
    if (tail_entry)
        return K_JMP;
    return prev_op == 0x20 || prev_op == 0x44 || prev_op == PREV_UNIT || computed_entry ? K_JSR : K_JMP;
}

/* The arrival at tagged index at pc, other than by an entry instruction: is it a jump into the routine? */
static void classify_arrival(int index, uint16_t pc)
{
    int plain = prev_op == 0x20 || prev_op == 0x44 || prev_op == 0x4C || prev_op == 0x6C || prev_op == 0x7C ||
                prev_op == PREV_UNIT || prev_op == PREV_IRQ;

    (void)pc;
    tail_entry = 0;
    self_jump = (prev_op == 0x4C || prev_op == 0x6C || prev_op == 0x7C) && in_routine(index, before_pc);
    if (index < 0 || plain || computed_entry)
        return;
    tail_entry = prev_op == 0x60 || !in_routine(index, before_pc);
}

static void judge(Frame *f, const Regs *r);

static void close_event(const Frame *f, const Regs *r, int how)
{
    Ev e;

    memset(&e, 0, sizeof e);
    e.type = EV_CLOSE;
    e.kind = (uint8_t)how;
    e.addr = (uint32_t)f->tag_index;
    e.clock = cur_clock();
    e.pc = r->pc;
    e.a = r->a;
    e.x = r->x;
    e.y = r->y;
    e.s = r->s;
    e.p = r->p;
    e.speed = chk_get_speed();
    for (unsigned i = 0; i < 8; i++)
        e.mpr[i] = (uint8_t)chk_mpr(i);
    log_event(&e);
}

static void close_frames(const Regs *r)
{
    while (depth) {
        Frame *f = &frames[depth - 1];
        uint8_t up = (uint8_t)(r->s - f->regs.s);
        int how = -1;

        if (f->tag->ends) {
            if (r->pc == f->tag->ends && r->s == f->regs.s)
                how = CL_OK;
        } else if (f->kind == K_IRQ) {
            if (up == 3 && r->pc == f->ret && prev_op == 0x40)
                how = CL_OK;
            else if (up > 3 && up < 0x80)
                how = CL_LEFT;
        } else {
            if (up == 2 && r->pc == f->ret && prev_op == 0x60)
                how = CL_OK;
            else if (up > 2 && up < 0x80 && prev_op == 0x60)
                how = CL_DROP;
            else if (up > 2 && up < 0x80)
                how = CL_LEFT;
        }
        if (how < 0)
            break;
        if (how == CL_LEFT && getenv("RONDO_DEBUG_LEFT") && !strcmp(f->tag->name, getenv("RONDO_DEBUG_LEFT")))
            fprintf(stderr, "LEFT %s kind %d entry S %02X now S %02X pc %04X ret %04X prev_op %X frame %lu\n",
                    f->tag->name, f->kind, f->regs.s, r->s, r->pc, f->ret, prev_op, cur_frame);
        if (mode == MODE_A5) {
            if (how == CL_LEFT) {
                stats[f->tag_index].calls++;
                stats[f->tag_index].unverified++;
                stats[f->tag_index].unv_left++;
            } else {
                judge(f, r);
            }
        } else if (mode == MODE_RECORD) {
            close_event(f, r, how);
        }
        depth--;
    }
    if (!depth) {
        wn = 0;
        nints = 0;
    }
}

static uint64_t opens_emitted;

static void open_frame(int index, const Regs *r, int kind)
{
    Frame *f;
    Ev e;

    opens_emitted++;

    if (depth == FRAMES) {
        while (depth) {
            Regs z = { 0 };

            if (mode == MODE_RECORD)
                close_event(&frames[depth - 1], &z, CL_LEFT);
            else {
                Stat *s = &stats[frames[depth - 1].tag_index];

                s->calls++;
                s->unverified++;
                s->unv_depth++;
            }
            depth--;
        }
        depth = 0;
        wn = 0;
        nints = 0;
    }
    f = &frames[depth++];
    f->tag = &rondo_tags[index];
    f->tag_index = index;
    f->kind = kind;
    f->ret = kind == K_IRQ ? irq_return(r->s) : stack_return(r->s);
    f->regs = *r;
    for (unsigned i = 0; i < 8; i++)
        f->mpr[i] = (uint8_t)chk_mpr(i);
    f->speed = chk_get_speed();
    memset(&e, 0, sizeof e);
    e.type = EV_OPEN;
    e.kind = (uint8_t)kind;
    e.addr = (uint32_t)index;
    e.clock = cur_clock();
    e.pc = r->pc;
    e.ret = f->ret;
    e.a = r->a;
    e.x = r->x;
    e.y = r->y;
    e.s = r->s;
    e.p = r->p;
    e.speed = f->speed;
    memcpy(e.mpr, f->mpr, 8);
    log_event(&e);
    f->start = wn;
}

/* ------------------------------------------------------------------------------------------ */
/* A5: judging a call.                                                                          */

static void say(const Tag *t, Stat *s, const char *verdict, const char *text)
{
    if (s->printed_v < 4) {
        s->printed_v++;
        fprintf(report, "%s %s call %llu: %s\n", t->name, verdict, (unsigned long long)s->calls, text);
    }
}

static uint8_t *final_val;
static uint32_t *final_stamp;
static uint32_t stamp;

static void judge(Frame *f, const Regs *r)
{
    const int prefix = r == NULL;           /* open at the script's end */

    if (getenv("RONDO_NOJUDGE")) {          /* a bisect switch for finding a fault of the checker's own: nothing is run */
        stats[f->tag_index].calls++;
        return;
    }
    Stat *s = &stats[f->tag_index];
    static uint8_t *snap;
    int reason;
    char diff[400] = "";
    size_t dl = 0;
    Span span;
    uint64_t end = wn;
    int fs0[FS_MAX] = { f->tag_index };
    int fsn0 = 1;

    s->calls++;
    if (prefix && getenv("RONDO_JUDGE_CHECK")) {
        if (!snap)
            snap = malloc(33u * 8192);
        for (unsigned b = 0x68; b <= 0x87; b++)
            if (ram_bank[b])
                memcpy(snap + (b - 0x68) * 8192, ram_bank[b], 8192);
        memcpy(snap + 32u * 8192, ram_bank[0xF8], 8192);
    }
    if (!final_val) {
        final_val = malloc(1u << 21);
        final_stamp = calloc(1u << 21, sizeof *final_stamp);
    }
    stamp++;
    span.start = span.cur = f->start;
    span.end = end;
    span.nest = 0;
    span.tag = f->tag_index;
    span.defects = 0;
    sp = &span;
    blame_tag = f->tag_index;
    cur_fs = fs0;
    cur_fsn = &fsn0;
    for (uint64_t i = end; i-- > f->start;) {
        if (win[i].type == EV_W) {
            ram_bank[win[i].addr >> 13][win[i].addr & 0x1FFF] = win[i].old;
            final_stamp[win[i].addr] = stamp;
        }
    }
    for (uint64_t i = f->start; i < end; i++)
        if (win[i].type == EV_W)
            final_val[win[i].addr] = win[i].val;
    memcpy(mshadow, f->mpr, 8);
    c_speed = f->speed;
    cpu.a = f->regs.a;
    cpu.x = f->regs.x;
    cpu.y = f->regs.y;
    cpu.s = f->regs.s;
    cpu.p = f->regs.p;
    cwn = 0;
    c_budget = 0;
    why[0] = 0;
    exit_regs_a5 = r;
    prefix_mode = prefix;
    reason = setjmp(c_abort);
    if (reason == 0) {
        f->tag->abi();
        apply_pokes(span.cur, end);
        if (!prefix)
            span_finish(&span);
    }
    prefix_mode = 0;
    if (reason == ABORT_NONE && !prefix) {
        uint8_t exit_s;
        uint16_t exit_pc;
        const uint8_t *stack = ram_bank[0xF8] + 0x100;

        /* A routine that ends on a shared exit is judged on what it leaves for that code: registers and flags
         * included. Anything a header proves dead is for the header to say, with the reason. */
        if (cpu.a != r->a)
            dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " A C %02X orig %02X;", cpu.a, r->a);
        if (cpu.x != r->x)
            dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " X C %02X orig %02X;", cpu.x, r->x);
        if (cpu.y != r->y)
            dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " Y C %02X orig %02X;", cpu.y, r->y);
        if (f->kind == K_IRQ) {
            exit_s = (uint8_t)(cpu.s + 3);
            exit_pc = (uint16_t)(stack[(uint8_t)(cpu.s + 2)] | (stack[(uint8_t)(cpu.s + 3)] << 8));
            if (((stack[(uint8_t)(cpu.s + 1)] ^ r->p) & PFLAGS))
                dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " P at RTI C %02X orig %02X;",
                                       stack[(uint8_t)(cpu.s + 1)], r->p);
        } else if (f->tag->ends) {
            exit_s = cpu.s;
            exit_pc = f->tag->ends;
        } else {
            exit_s = (uint8_t)(cpu.s + 2);
            exit_pc = (uint16_t)((stack[(uint8_t)(cpu.s + 1)] | (stack[(uint8_t)(cpu.s + 2)] << 8)) + 1);
            if ((cpu.p ^ r->p) & PFLAGS)
                dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " P C %02X orig %02X (entry %02X);", cpu.p, r->p,
                                       f->regs.p);
        }
        if (f->tag->ends && ((cpu.p ^ r->p) & PFLAGS))
            dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " P C %02X orig %02X (entry %02X);", cpu.p, r->p,
                                   f->regs.p);
        if (exit_s != r->s || exit_pc != r->pc)
            defect(D_STACK, "the call ends with S %02X and returns to %04X, the original S %02X and %04X", exit_s,
                   exit_pc, r->s, r->pc);
        if (c_speed != chk_get_speed())
            dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " CPU clock C %s orig %s;", c_speed ? "fast" : "slow",
                                   chk_get_speed() ? "fast" : "slow");
        for (unsigned i = 0; i < 8; i++)
            if (mshadow[i] != (uint8_t)chk_mpr(i))
                dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " MPR%u C %02X orig %02X;", i, mshadow[i],
                                       chk_mpr(i));
        /* RAM: everything either side wrote; the stack at and below S is dead. */
        for (uint64_t i = f->start; i < end && dl < 300; i++) {
            if (win[i].type == EV_W) {
                uint32_t p = win[i].addr;
                uint8_t c = ram_bank[p >> 13][p & 0x1FFF];

                if (is_stack(p) && (uint8_t)(p & 0xFF) <= exit_s)
                    continue;
                if (c != final_val[p])
                    dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " %06X C %02X orig %02X;", p, c, final_val[p]);
            }
        }
        for (uint32_t i = 0; i < cwn && dl < 300; i++) {
            uint32_t p = cw[i].phys;

            if (is_stack(p) && (uint8_t)(p & 0xFF) <= exit_s)
                continue;
            if (final_stamp[p] != stamp && ram_bank[p >> 13][p & 0x1FFF] != cw[i].old)
                dl += (size_t)snprintf(diff + dl, sizeof diff - dl, " %06X C wrote %02X, orig left %02X;", p,
                                       ram_bank[p >> 13][p & 0x1FFF], cw[i].old);
        }
    }
    /* Put RAM back: undo the C, redo the original. */
    while (cwn) {
        cwn--;
        ram_bank[cw[cwn].phys >> 13][cw[cwn].phys & 0x1FFF] = cw[cwn].old;
    }
    for (uint64_t i = f->start; i < end; i++)
        if (win[i].type == EV_W)
            ram_bank[win[i].addr >> 13][win[i].addr & 0x1FFF] = win[i].val;
    if (prefix && getenv("RONDO_JUDGE_CHECK")) {
        unsigned shown = 0;

        for (unsigned b = 0x68; b <= 0x88 && shown < 6; b++) {
            const uint8_t *now = b == 0x88 ? ram_bank[0xF8] : ram_bank[b];

            if (now)
                for (unsigned i = 0; i < 8192 && shown < 6; i++)
                    if (now[i] != snap[(b == 0x88 ? 32u : b - 0x68) * 8192 + i]) {
                        fprintf(stderr, "JUDGE CHANGED RAM frame %lu %s%s bank %02X off %04X was %02X now %02X (log %llu..%llu, cw %u)\n",
                                cur_frame, f->tag->name, prefix ? " prefix" : "", b == 0x88 ? 0xF8 : b, i,
                                snap[(b == 0x88 ? 32u : b - 0x68) * 8192 + i], now[i], (unsigned long long)f->start,
                                (unsigned long long)end, cwn);
                        shown++;
                    }
        }
    }
    sp = NULL;
    cur_fs = NULL;
    cur_fsn = NULL;
    blame_tag = -1;
    if (reason == ABORT_RUNAWAY) {
        /* The C itself ran away (an endless loop, a write count past the call's): its fault, not the checker's. */
        s->divergent++;
        s->runaway++;
        say(f->tag, s, "DIVERGENT", why);
    } else if (reason == ABORT_DIVERGED) {
        s->divergent++;
        say(f->tag, s, "DIVERGENT", why);
    } else if (reason == ABORT_END || prefix) {
        s->prefix++;
        if (span.defects)
            s->order++;
        else
            s->identical++;
    } else if (dl) {
        s->divergent++;
        say(f->tag, s, "DIVERGENT", diff);
    } else if (span.defects) {
        s->order++;
    } else {
        s->identical++;
    }
    (void)fsn0;
}

/* ------------------------------------------------------------------------------------------ */
/* A7: the hybrid's calls.                                                                      */

static uint64_t swapped;
static unsigned long hy_frames_ok, hy_frames_total, hy_first_bad = (unsigned long)-1;
static uint64_t nf_record;
static unsigned long dump_frame;

static int kept_by_name(int index)
{
    const char *keep = getenv("RONDO_KEEP");
    const char *only = getenv("RONDO_ONLY");
    size_t n = strlen(rondo_tags[index].name);

    /* RONDO_ONLY=name,...: only those routines are swapped; every other call runs the original. */
    if (only) {
        for (const char *at = only; (at = strstr(at, rondo_tags[index].name)) != NULL; at += n)
            if ((at == only || at[-1] == ',') && (at[n] == ',' || at[n] == 0))
                return 0;
        return 1;
    }
    if (keep && !strcmp(keep, "all"))
        return 1;
    /* RONDO_KEEP_WAITS=1 (a diagnostic): routines whose C reaches wait_interrupt run as original code. */
    if (getenv("RONDO_KEEP_WAITS") && !rondo_tags[index].swappable)
        return 1;
    for (const char *at = keep; at && (at = strstr(at, rondo_tags[index].name)) != NULL; at += n)
        if ((at == keep || at[-1] == ',') && (at[n] == ',' || at[n] == 0))
            return 1;
    return 0;
}

static int tag_resident(const Tag *t)
{
    /* A card routine is in ROM: it is resident when its bank is mapped at its address. */
    if (t->rom) {
        for (unsigned i = 0; i < t->len; i++) {
            uint16_t logical = (uint16_t)(t->run + i);

            if (chk_mpr(logical >> 13) != (t->disc + i) >> 13)
                return 0;
        }
        return 1;
    }
    for (unsigned i = 0; i < t->len; i++) {
        uint16_t logical = (uint16_t)(t->run + i);
        uint32_t p = canon(((uint32_t)chk_mpr(logical >> 13) << 13) | (logical & 0x1FFF));
        const uint8_t *b = rom_bank[p >> 13];

        if (!b || b[p & 0x1FFF] != user[t->disc + i])
            return 0;
    }
    return 1;
}

static Ctx *ctx_new(void)
{
    Ctx *c = free_ctx;

    if (c) {
        free_ctx = c->below;
    } else {
        c = calloc(1, sizeof *c);
        c->stack = mmap(NULL, CO_STACK, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (c->stack == MAP_FAILED) {
            fprintf(stderr, "check: no stack for a coroutine\n");
            exit(5);
        }
    }
    return c;
}

/* Drops the events no open call or scan can need any more. */
static void hy_trim(void)
{
    uint64_t keep = scan;

    for (Ctx *c = top; c; c = c->below)
        if (c->span.cur < keep)
            keep = c->span.cur;
    if (keep > wbase + wn)
        keep = wbase + wn;
    if (keep > wbase + (1u << 17) && keep - wbase > wn / 2) {
        uint64_t drop = keep - wbase;

        memmove(win, win + drop, (wn - drop) * sizeof *win);
        wn -= drop;
        wbase = keep;
    }
}

static void co_main(void)
{
    Ctx *c = top;

    sp = &c->span;
    cur_fs = c->fs;
    cur_fsn = &c->fsn;
    c->fsn = 1;
    c->fs[0] = c->tag;
    blame_tag = c->tag;
    in_swap = c->tag;
    rondo_tags[c->tag].abi();
    span_finish(&c->span);
    in_swap = -1;
    c->done = 1;
    swapcontext(&c->uc, &main_uc);
}

static void hy_switch_in(Ctx *c)
{
    c->parked = 0;
    sp = &c->span;
    cur_fs = c->fs;
    cur_fsn = &c->fsn;
    blame_tag = c->fsn ? c->fs[c->fsn - 1] : c->tag;
    swapcontext(&main_uc, &c->uc);
}

static int hy_finish(Ctx *c)
{
    const Tag *t = &rondo_tags[c->tag];
    Ev *close = evp(c->span.end);
    uint8_t s = cpu.s;
    uint16_t pc;
    uint8_t ns;
    uint8_t p = cpu.p;
    int64_t owed;

    if (c->kind == K_IRQ) {
        p = chk_bus_rd((uint16_t)(0x2100 | (uint8_t)(s + 1)));
        pc = (uint16_t)(chk_bus_rd((uint16_t)(0x2100 | (uint8_t)(s + 2))) |
                        (chk_bus_rd((uint16_t)(0x2100 | (uint8_t)(s + 3))) << 8));
        ns = (uint8_t)(s + 3);
    } else if (t->ends) {
        pc = t->ends;
        ns = s;
    } else {
        pc = (uint16_t)((chk_bus_rd((uint16_t)(0x2100 | (uint8_t)(s + 1))) |
                         (chk_bus_rd((uint16_t)(0x2100 | (uint8_t)(s + 2))) << 8)) + 1);
        ns = (uint8_t)(s + 2);
    }
    if (close && close->type == EV_CLOSE) {
        blame_tag = c->tag;
        if (ns != close->s || pc != close->pc)
            defect(D_STACK, "the call ends with S %02X and returns to %04X, the original S %02X and %04X", ns, pc,
                   close->s, close->pc);
        else if (cpu.a != close->a || cpu.x != close->x || cpu.y != close->y || ((p ^ close->p) & PFLAGS))
            defect(D_REGS, "the call leaves A%02X X%02X Y%02X P%02X, the original A%02X X%02X Y%02X P%02X", cpu.a,
                   cpu.x, cpu.y, p, close->a, close->x, close->y, close->p);
        blame_tag = -1;
        owed = (int64_t)close->clock - (int64_t)hybrid_clock_now();
    } else {
        owed = 1;
    }
    chk_set_regs(cpu.a, cpu.x, cpu.y, ns, p, pc);
    prev_op = c->kind == K_IRQ ? 0x40 : 0x60;
    last_pc = 0xFFFF;
    scan = c->span.end + 1;
    return owed > 0 ? (int)(owed > 0x7FFFFFFF ? 0x7FFFFFFF : owed) : 1;
}

static int hook_again(void);

/* A coroutine ran or parked; the hook's answer: 0 to go on running the CPU, else clocks owed. */
static int hy_after(void)
{
    Ctx *c = top;
    int ret = 0;

    sp = NULL;
    cur_fs = NULL;
    cur_fsn = NULL;
    blame_tag = -1;
    if (c && c->parked && c->wait_frame) {
        int64_t owed = (int64_t)c->wait_clock - (int64_t)hybrid_clock_now();

        return owed > 0 ? (int)(owed > 0x7FFFFFFF ? 0x7FFFFFFF : owed) : 1;
    }
    if (c && c->abandoned) {
        top = c->below;
        c->abandoned = 0;
        c->below = free_ctx;
        free_ctx = c;
        hy_trim();
        return hook_again();     /* the hook must see the first instruction of the code jumped to */
    }
    if (c && c->done) {
        top = c->below;
        ret = hy_finish(c);
        c->below = free_ctx;
        free_ctx = c;
        c->done = 0;
        hy_trim();
    } else if (c && c->parked) {
        ret = hook_again();      /* the CPU is set to run code: the hook sees its first instruction, now, at no cost in time */
    }
    return ret;
}

static int recent_tags[16];
static unsigned recent_n;

/* For a stop: the calls swapped last, and what the record has next. */
static void stop_context(uint64_t from)
{
    unsigned shown = 0;

    fprintf(report, "  swapped last:");
    for (unsigned i = recent_n > 12 ? recent_n - 12 : 0; i < recent_n; i++)
        fprintf(report, " %s", rondo_tags[recent_tags[i % 16]].name);
    fprintf(report, "\n  the record's next calls:");
    for (uint64_t i = from; shown < 8; i++) {
        Ev *e = evp(i);

        if (!e)
            break;
        if (e->type == EV_OPEN) {
            fprintf(report, " %s(S%02X)", rondo_tags[e->addr].name, e->s);
            shown++;
        } else if (e->type == EV_CALL) {
            fprintf(report, " >%04X", e->pc);
        } else if (e->type == EV_CLOSE) {
            fprintf(report, " <%s", rondo_tags[e->addr].name);
        } else if (e->type == EV_INT) {
            fprintf(report, " INT");
        }
    }
    fprintf(report, "\n");
}

static int hy_enter(int index, const Regs *r, int kind, int keep)
{
    uint64_t i = scan;
    Ev *e = NULL;
    Ctx *c;

    for (;; i++) {
        e = evp(i);
        if (!e || e->type == EV_OPEN)
            break;
    }
    if (!e && keep) {
        scan = i;
        return 0;
    }
    if (!e) {
        stop_run(4, "the call of %s is past the record's end", rondo_tags[index].name);
    }
    if ((int)e->addr != index) {
        stop_context(i > 24 ? i - 24 : 0);
        stop_run(4, "the hybrid calls %s (S%02X); the record's next is %s (S%02X)", rondo_tags[index].name, r->s,
                 rondo_tags[e->addr].name, e->s);
    }
    if (keep) {
        stats[index].kept++;
        scan = i + 1;
        return 0;
    }
    if (e->a != r->a || e->x != r->x || e->y != r->y || e->s != r->s || ((e->p ^ r->p) & PFLAGS)) {
        Span none = { 0 };

        none.tag = index;
        sp = &none;
        blame_tag = index;
        defect(D_REGS, "entry A%02X X%02X Y%02X S%02X P%02X, the record's A%02X X%02X Y%02X S%02X P%02X", r->a, r->x,
               r->y, r->s, r->p, e->a, e->x, e->y, e->s, e->p);
        sp = NULL;
        blame_tag = -1;
    }
    c = ctx_new();
    memset(&c->span, 0, sizeof c->span);
    c->span.start = c->span.cur = i + 1;
    c->span.end = SPAN_OPEN;
    c->span.nest = 0;
    c->span.tag = index;
    c->tag = index;
    c->kind = e->kind;
    (void)kind;
    c->open_i = i;
    c->entry_s = r->s;
    recent_tags[recent_n++ % 16] = index;
    c->done = c->parked = c->abandoned = 0;
    c->fsn = 0;
    c->below = top;
    top = c;
    swapped++;
    stats[index].calls++;
    cpu.a = r->a;
    cpu.x = r->x;
    cpu.y = r->y;
    cpu.s = r->s;
    cpu.p = r->p;
    scan = i + 1;
    getcontext(&c->uc);
    c->uc.uc_stack.ss_sp = c->stack;
    c->uc.uc_stack.ss_size = CO_STACK;
    c->uc.uc_link = &main_uc;
    makecontext(&c->uc, co_main, 0);
    hy_switch_in(c);
    return hy_after();
}

/* ------------------------------------------------------------------------------------------ */
/* Unit calls: a call at a routine's own entry, made by the script.                             */

static struct {
    int active, entered;
    unsigned long count;
    uint16_t entry;
    Regs saved;
    uint8_t s;
} unit;

/* NULL on success; otherwise why not. Called between frames, in every run alike. */
const char *check_unit_call(uint16_t addr, int has_a, uint8_t a, int has_x, uint8_t x, int has_y, uint8_t y,
                            int has_p, uint8_t p)
{
    Regs r;
    int index = tag_pick(addr);

    if (unit.active)
        return "a unit call is still running";
    if (index < 0)
        return "the address is not a routine's own entry";
    if (!tag_resident(&rondo_tags[index]))
        return "the routine's bytes are not resident at its address";
    chk_get_regs(&r.a, &r.x, &r.y, &r.s, &r.p, &r.pc);
    unit.saved = r;
    chk_bus_wr((uint16_t)(0x2100 | r.s), (uint8_t)((uint16_t)(UNIT_MARK - 1) >> 8));
    chk_bus_wr((uint16_t)(0x2100 | (uint8_t)(r.s - 1)), (uint8_t)(UNIT_MARK - 1));
    unit.s = (uint8_t)(r.s - 2);
    unit.entry = addr;
    unit.entered = 0;
    unit.count = 0;
    unit.active = 1;
    chk_set_regs(has_a ? a : r.a, has_x ? x : r.x, has_y ? y : r.y, unit.s, has_p ? p : r.p, addr);
    return NULL;
}

/*
 * Every arrival at a tagged routine's first byte that does not open a call is counted under the routine, never
 * dropped quietly: a branch or fall-through (information), an entry while the bytes there are another overlay's
 * (information), and an RTS landing there (a computed dispatch: a call the checker cannot judge, counted as
 * unverified, so the routine is not IDENTICAL on the strength of other calls). `judge_calls`: A5 and the record
 * count the calls; the hybrid counts the arrivals only.
 */
static void count_arrival(int index, int judge_calls)
{
    Stat *s = &stats[index];

    (void)judge_calls;
    if (self_jump)
        return;                 /* a JMP to the routine's own first byte: the idle loop, not an arrival */
    if (prev_is_entry() || prev_op == PREV_IRQ) {
        if (!tag_resident(&rondo_tags[index]))
            s->arrive_nonres++;
        return;
    }
    s->arrive_branch++;      /* a branch back into the routine's own first byte */
}

/* What happens between two instructions, for the record and A5: state changes, calls that ended, interrupts that returned. */
static void boundary(const Regs *r)
{
    if (prev_op == 0x53 || prev_op == 0x54 || prev_op == 0xD4 || prev_op == PREV_IRQ) {
        if (depth) {
            sync_state(r);
        } else {
            for (unsigned i = 0; i < 8; i++)
                last_mpr[i] = (uint8_t)chk_mpr(i);
            last_speed = chk_get_speed();
        }
    }
    close_frames(r);
    if (prev_op == 0x40 && nchain && r->s == (uint8_t)(ichain[nchain - 1] + 3))
        nchain--;
    if (nints && depth && prev_op == 0x40 && r->pc == ints[nints - 1].resume && r->s == ints[nints - 1].s) {
        Ev e;

        memset(&e, 0, sizeof e);
        e.type = EV_IRET;
        e.clock = cur_clock();
        log_event(&e);
        nints--;
    }
}

/*
 * The script wrote a byte of RAM (poke, or the M= of a unit call) between frames. If a call is open the write is in
 * the stream, so a call that stays open across it can be rewound and put forward without losing it.
 */
void check_poke(uint32_t phys, uint8_t old, uint8_t value)
{
    Ev e;

    if (!depth || (mode != MODE_A5 && mode != MODE_RECORD) || !is_ram(canon(phys)))
        return;
    memset(&e, 0, sizeof e);
    e.type = EV_W;
    e.kind = 1;
    e.addr = canon(phys);
    e.val = value;
    e.old = old;
    log_event(&e);
}

/* Is a unit call still on its way (between its entry and its return)? */
int check_unit_active(void)
{
    return unit.active;
}

/* The run address of a routine by name, or -1. */
int check_tag_run(const char *name)
{
    for (unsigned i = 0; i < rondo_tag_count; i++)
        if (!strcmp(rondo_tags[i].name, name))
            return rondo_tags[i].run;
    return -1;
}

/* ------------------------------------------------------------------------------------------ */
/* The hook before every instruction.                                                           */

int check_exec(uint32_t pc);

/* The CPU was set to run code at its registers' PC: what the hook does for that instruction. */
static int hook_again(void)
{
    uint8_t a, x, y, s, p;
    uint16_t pc;

    chk_get_regs(&a, &x, &y, &s, &p, &pc);
    return check_exec(pc);
}

int check_exec(uint32_t pc)
{
    Regs r;
    int index;

    chk_get_regs(&r.a, &r.x, &r.y, &r.s, &r.p, &r.pc);
    r.pc = (uint16_t)pc;
    if (unit.active && !unit.entered && pc == unit.entry && r.s == unit.s) {
        unit.entered = 1;
        prev_op = PREV_UNIT;
    }
    if (prev_op == 0x20 || prev_op == 0x44)
        jsrmark[(uint8_t)(r.s + 1)] = stack_return(r.s);
    before_pc = last_pc;
    last_pc = (uint16_t)pc;
    computed_entry = prev_op == 0x60 && tag_pick(pc & 0xFFFF) >= 0 && jsrmark[(uint8_t)(r.s - 1)] != (uint16_t)pc;
    classify_arrival(tag_pick(pc & 0xFFFF), (uint16_t)pc);
    if (getenv("RONDO_TRACE_PC") && pc == strtoul(getenv("RONDO_TRACE_PC"), NULL, 16))
        fprintf(stderr, "PC %04X frame %lu S%02X prev_op %X before %04X mpr2 %02X tag %d resident %d\n", (unsigned)pc,
                cur_frame, r.s, prev_op, before_pc, chk_mpr(2), tag_pick(pc & 0xFFFF),
                tag_pick(pc & 0xFFFF) >= 0 ? tag_resident(&rondo_tags[tag_pick(pc & 0xFFFF)]) : -1);
    if (getenv("RONDO_TRACE_ENTRY") && tag_pick(pc & 0xFFFF) >= 0 &&
        strstr(getenv("RONDO_TRACE_ENTRY"), rondo_tags[tag_pick(pc & 0xFFFF)].name) && mode != MODE_HYBRID)
        fprintf(stderr, "ENTRY frame %lu %s pc %04X S%02X prev_op %X before %04X computed %d tail %d self %d depth %d\n",
                cur_frame, rondo_tags[tag_pick(pc & 0xFFFF)].name, (unsigned)pc, r.s, prev_op, before_pc, computed_entry,
                tail_entry, self_jump, depth);
    if (unit.active && unit.entered && ++unit.count > 2000000) {
        /* A crafted state that runs data as code or loops for ever is not a valid script. */
        fprintf(stderr, "check: a unit call ran over 2,000,000 instructions: not a valid script\n");
        if (mode == MODE_HYBRID)
            stop_run(6, "a unit call ran over 2,000,000 instructions");
        exit(6);
    }
    if (mode == MODE_HYBRID) {
        if (top && top->parked && top->wait_frame) {
            if (cur_frame < top->wait_frame)
                return 1;                    /* the frame has not ended: the core goes on, running nothing */
            top->wait_frame = 0;
            hy_switch_in(top);
            return hy_after();
        }
        if (top && top->parked && (prev_op == 0x60 || prev_op == 0x40) && pc == top->resume_pc &&
            r.s == top->resume_s) {
            if (prev_op == 0x40 && nipend && ipend[nipend - 1].resume == pc && ipend[nipend - 1].s == r.s)
                nipend--;
            hy_switch_in(top);
            return hy_after();
        }
        if (prev_op == 0x40 && nipend && pc == ipend[nipend - 1].resume && r.s == ipend[nipend - 1].s) {
            before_pc = ipend[nipend - 1].from;
            before_pc = ipend[nipend - 1].from;
        prev_op = ipend[--nipend].prev;
            computed_entry = prev_op == 0x60 && tag_pick(pc & 0xFFFF) >= 0 && jsrmark[(uint8_t)(r.s - 1)] != (uint16_t)pc;
            classify_arrival(tag_pick(pc & 0xFFFF), (uint16_t)pc);
        }
        if (unit.active && unit.entered && pc == UNIT_MARK && r.s == (uint8_t)(unit.s + 2)) {
            unit.active = 0;
            chk_set_regs(unit.saved.a, unit.saved.x, unit.saved.y, unit.saved.s, unit.saved.p, unit.saved.pc);
            prev_op = -1;
            return 0;
        }
        index = tag_pick(pc & 0xFFFF);
        if (getenv("RONDO_TRACE_ENTRY") && index >= 0 && strstr(getenv("RONDO_TRACE_ENTRY"), rondo_tags[index].name))
            fprintf(stderr, "ENTRY frame %lu %s pc %04X S%02X prev_op %X before %04X self %d tail %d comp %d entry %d resident %d kept %d scan %llu top %s\n",
                    cur_frame, rondo_tags[index].name, (unsigned)pc, r.s, prev_op, before_pc, self_jump, tail_entry, computed_entry, prev_is_entry(),
                    tag_resident(&rondo_tags[index]), kept_by_name(index), (unsigned long long)scan,
                    top ? rondo_tags[top->tag].name : "-");
        if (index >= 0)
            count_arrival(index, 0);
        if (index < 0 || !(prev_is_entry() || prev_op == PREV_IRQ) || !tag_resident(&rondo_tags[index]))
            return 0;
        return hy_enter(index, &r, prev_op == PREV_IRQ ? K_IRQ : K_JSR, kept_by_name(index));
    }
    if (mode != MODE_A5 && mode != MODE_RECORD) {
        if (unit.active && unit.entered && pc == UNIT_MARK && r.s == (uint8_t)(unit.s + 2)) {
            unit.active = 0;
            chk_set_regs(unit.saved.a, unit.saved.x, unit.saved.y, unit.saved.s, unit.saved.p, unit.saved.pc);
        }
        return 0;
    }
    boundary(&r);
    if (prev_op == 0x40 && nipend && pc == ipend[nipend - 1].resume && r.s == ipend[nipend - 1].s) {
        before_pc = ipend[nipend - 1].from;
        prev_op = ipend[--nipend].prev;
        computed_entry = prev_op == 0x60 && tag_pick(pc & 0xFFFF) >= 0 && jsrmark[(uint8_t)(r.s - 1)] != (uint16_t)pc;
        classify_arrival(tag_pick(pc & 0xFFFF), (uint16_t)pc);
    }
    if (unit.active && unit.entered && pc == UNIT_MARK && r.s == (uint8_t)(unit.s + 2)) {
        unit.active = 0;
        chk_set_regs(unit.saved.a, unit.saved.x, unit.saved.y, unit.saved.s, unit.saved.p, unit.saved.pc);
        prev_op = -1;
        return 0;
    }
    if (prev_op == PREV_IRQ && nchain < FRAMES)
        ichain[nchain++] = r.s;
    {
        int irq_kind = prev_op == PREV_IRQ || (prev_is_entry() && entry_kind() == K_JMP && nchain && r.s == ichain[nchain - 1]);

        if (depth) {
            /* A return that lands on a tagged routine's first byte is a return and an entry: the RET is logged
             * before the CALL, or the call that returned there has no RET and looks as if it never came back. */
            if (prev_op == 0x60 && tail_entry && jsrmark[(uint8_t)(r.s - 1)] == (uint16_t)pc)
                marker(EV_RET, &r, 0, 0, 0);
            if (prev_is_entry() || prev_op == PREV_IRQ)
                marker(EV_CALL, &r, prev_op == PREV_IRQ ? K_IRQ : entry_kind(), 0,
                       irq_kind ? irq_return(r.s) : stack_return(r.s));
            else if (prev_op == 0x60)
                marker(EV_RET, &r, 0, 0, 0);
        }
        index = tag_pick(pc & 0xFFFF);
        if (index >= 0)
            count_arrival(index, 1);
        if (index >= 0 && (prev_is_entry() || prev_op == PREV_IRQ) && tag_resident(&rondo_tags[index]))
            open_frame(index, &r, irq_kind ? K_IRQ : K_JSR);
    }
    return 0;
}

/* ------------------------------------------------------------------------------------------ */
/* Frames.                                                                                      */

static uint64_t hash_bytes(uint64_t h, const uint8_t *p, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        h ^= p[i];
        h *= 0x100000001B3ull;
    }
    return h;
}

/* RAM and the picture; the stack at and below S is not state. */
void check_frame(unsigned long frame, const uint32_t *pixels, unsigned w, unsigned h)
{
    uint64_t hash = 0xCBF29CE484222325ull;
    uint64_t picture = 0xCBF29CE484222325ull;
    uint8_t a, x, y, s, p;
    uint16_t pc;

    cur_frame = frame;
    if (mode != MODE_RECORD && mode != MODE_HYBRID && !(mode == MODE_A5 && dump_frame && frame == dump_frame))
        return;
    if (dump_frame && frame == dump_frame) {
        char name[4200];
        FILE *d;

        snprintf(name, sizeof name, "%s.%s.ram", check_path,
                 mode == MODE_RECORD ? "record" : mode == MODE_A5 ? "a5" : "hybrid");
        d = fopen(name, "wb");
        for (unsigned b = 0x68; b <= 0x87 && d; b++)
            if (ram_bank[b])
                fwrite(ram_bank[b], 1, 8192, d);
        if (d) {
            fwrite(ram_bank[0xF8], 1, 8192, d);
            fclose(d);
        }
    }
    if (mode == MODE_A5)
        return;
    chk_get_regs(&a, &x, &y, &s, &p, &pc);
    if (dump_frame && frame == dump_frame) {
        fprintf(stderr, "FRAME %lu %s: PC %04X A%02X X%02X Y%02X S%02X P%02X", frame,
                mode == MODE_RECORD ? "record" : "hybrid", pc, a, x, y, s, p);
        if (mode == MODE_RECORD) {
            for (int i = 0; i < depth; i++)
                fprintf(stderr, " open:%s(kind %d,S%02X)", frames[i].tag->name, frames[i].kind, frames[i].regs.s);
        } else {
            for (Ctx *c = top; c; c = c->below)
                fprintf(stderr, " ctx:%s(parked %d)", rondo_tags[c->tag].name, c->parked);
        }
        fprintf(stderr, "\n");
    }
    for (unsigned b = 0x68; b <= 0x87; b++)
        if (ram_bank[b])
            hash = hash_bytes(hash, ram_bank[b], 8192);
    hash = hash_bytes(hash, ram_bank[0xF8], 0x100);
    hash = hash_bytes(hash, ram_bank[0xF8] + 0x100 + (size_t)s + 1, 0xFF - (size_t)s);
    hash = hash_bytes(hash, ram_bank[0xF8] + 0x200, 8192 - 0x200);
    picture = hash_bytes(picture, (const uint8_t *)pixels, (size_t)w * h * 4);
    if (mode == MODE_RECORD) {
        Ev e;

        memset(&e, 0, sizeof e);
        e.type = EV_FRAME;
        e.addr = (uint32_t)frame;
        e.clock = hash;
        e.hash2 = picture;
        {
            uint64_t when = cur_clock();

            memcpy(e.mpr, &when, 8);
        }
        wire_write(&e);
        nf_record++;
        fflush(out);
        if (ferror(out))
            exit(0);         /* the hybrid stopped reading: its report says why */
    } else {
        hy_trim();
        /* The record's hashes for this frame: read ahead until they are there. */
        while (fq_head == fq_tail && fetch_one())
            ;
        if (fq_head < fq_tail) {
            hy_frames_total++;
            if (fq[fq_head].ram != hash || fq[fq_head].pic != picture) {
                if (hy_first_bad == (unsigned long)-1) {
                    hy_first_bad = frame;
                    fprintf(report, "first differing frame %lu: RAM %s, picture %s\n", frame,
                            fq[fq_head].ram != hash ? "differs" : "same", fq[fq_head].pic != picture ? "differs" : "same");
                }
            } else {
                hy_frames_ok++;
            }
            fq_head++;
        }
    }
}

/* ------------------------------------------------------------------------------------------ */
/* Setup and the end.                                                                           */

void check_where(const char *text)
{
    script_where = text;
}

int check_init(int which, const char *path, const char *user_path)
{
    FILE *f = fopen(user_path, "rb");

    mode = which;
    check_path = path;
    if (getenv("RONDO_DUMP_FRAME"))
        dump_frame = strtoul(getenv("RONDO_DUMP_FRAME"), NULL, 10);
    if (getenv("RONDO_LOG_HARD"))
        log_hard_events = strtoull(getenv("RONDO_LOG_HARD"), NULL, 10);
    if (getenv("RONDO_PLACE")) {
        unsigned long lo = 0, hi = 0;

        sscanf(getenv("RONDO_PLACE"), "%lu,%lu", &lo, &hi);
        place_from = lo;
        place_to = hi;
    }
    if (getenv("RONDO_SKIPS"))
        skip_frame = strtoul(getenv("RONDO_SKIPS"), NULL, 10);
    user = malloc(20785152);
    tag_at = malloc(65536 * sizeof *tag_at);
    if (!f || !user || !tag_at || fread(user, 1, 20785152, f) != 20785152) {
        fprintf(stderr, "check: cannot load %s\n", user_path);
        return -1;
    }
    fclose(f);
    memset(tag_at, 0xFF, 65536 * sizeof *tag_at);
    tag_next = malloc(rondo_tag_count * sizeof *tag_next + 2);
    for (unsigned i = rondo_tag_count; i-- > 0;) {
        tag_next[i] = tag_at[rondo_tags[i].run];
        tag_at[rondo_tags[i].run] = (int16_t)i;
    }
    stats = calloc(rondo_tag_count, sizeof *stats);
    for (unsigned b = 0; b < 256; b++) {
        const uint8_t *p = chk_bank(b);

        rom_bank[b] = (uint8_t *)p;
        if (p && ((b >= 0x68 && b <= 0x87) || (b >= 0xF8 && b <= 0xFB)))
            ram_bank[b] = (uint8_t *)p;
    }
    for (unsigned i = 0; i < 8; i++)
        last_mpr[i] = (uint8_t)chk_mpr(i);
    last_speed = chk_get_speed();
    if (mode == MODE_A5) {
        win = malloc(LOG_START * sizeof *win);
        wcap = LOG_START;
        report = fopen(path, "w");
        int_pass = a5_int_pass;
    } else if (mode == MODE_RECORD) {
        signal(SIGPIPE, SIG_IGN);
        out = fopen(path, "wb");
        if (!out) {
            fprintf(stderr, "check: cannot write the record %s\n", path);
            return -1;
        }
        setvbuf(out, NULL, _IOFBF, 1u << 22);
    } else if (mode == MODE_HYBRID) {
        char name[4200];

        in = fopen(path, "rb");
        if (!in) {
            fprintf(stderr, "check: cannot read the record %s\n", path);
            return -1;
        }
        setvbuf(in, NULL, _IOFBF, 1u << 22);
        snprintf(name, sizeof name, "%s.hybrid.txt", path);
        report = fopen(name, "w");
        int_pass = hy_int_pass;
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
        /* Calls open at the script's end are judged as far as their log goes, innermost first. */
        while (depth) {
            Frame *f = &frames[depth - 1];

            judge(f, NULL);
            depth--;
        }
        for (unsigned i = 0; i < rondo_tag_count; i++) {
            Stat *s = &stats[i];

            fprintf(report, "TAG %s %llu %llu %llu %llu %llu", rondo_tags[i].name, (unsigned long long)s->calls,
                    (unsigned long long)s->identical, (unsigned long long)s->divergent,
                    (unsigned long long)s->unverified, (unsigned long long)s->order);
            for (int k = 0; k < NKIND; k++)
                fprintf(report, " %llu", (unsigned long long)s->kind[k]);
            /* Causes: left, log, depth, computed, arrive_branch, arrive_nonres, runaway, prefix, noreturn */
            fprintf(report, " %llu %llu %llu %llu %llu %llu %llu %llu %llu\n", (unsigned long long)s->unv_left,
                    (unsigned long long)s->unv_log, (unsigned long long)s->unv_depth,
                    (unsigned long long)s->unv_computed, (unsigned long long)s->arrive_branch,
                    (unsigned long long)s->arrive_nonres, (unsigned long long)s->runaway,
                    (unsigned long long)s->prefix, (unsigned long long)s->noreturn);
        }
        fclose(report);
    } else if (mode == MODE_RECORD) {
        Ev e;
        uint32_t left = (uint32_t)depth;

        while (depth) {
            Regs z = { 0 };

            close_event(&frames[depth - 1], &z, CL_LEFT);
            depth--;
        }
        memset(&e, 0, sizeof e);
        e.type = EV_END;
        e.addr = left;
        e.clock = opens_emitted;
        e.hash2 = nf_record;
        wire_write(&e);
        fflush(out);
        fclose(out);
        printf("record: %llu events, %llu frames\n", (unsigned long long)emitted, (unsigned long long)nf_record);
    } else if (mode == MODE_HYBRID) {
        uint64_t defects = 0;
        long first;

        uint64_t expected = 0, parked_n = 0, kept_n = 0, unreached = 0, unopened = 0;
        int known = 0, diagnostic;

        for (unsigned i = 0; i < rondo_tag_count; i++) {
            for (int k = 0; k < NKIND; k++)
                defects += stats[i].kind[k];
            kept_n += stats[i].kept;
            unopened += stats[i].arrive_branch + stats[i].arrive_nonres + stats[i].unv_computed;
        }
        for (Ctx *c = top; c; c = c->below)
            parked_n++;
        if (!stopped_why) {
            /* The rest of the record: the frames and calls this run did not reach. */
            while (fetch_one())
                ;
            known = end_seen;
            expected = end_frames;
            for (uint64_t k = scan > wbase ? scan : wbase; k < wbase + wn; k++)
                if (win[k - wbase].type == EV_OPEN)
                    unreached++;
        }
        diagnostic = getenv("RONDO_KEEP") || getenv("RONDO_ONLY") || getenv("RONDO_KEEP_WAITS") ||
                     getenv("RONDO_PLACE") || getenv("RONDO_SKIPS") || getenv("RONDO_DUMP_FRAME");
        /* Identical only when everything was compared and nothing is left over. A diagnostic run, a run that
         * stopped, one that swapped nothing, or one that left calls or frames unreached has no verdict. */
        if (hy_first_bad != (unsigned long)-1)
            first = (long)hy_first_bad;
        else if (stopped_why)
            first = -3L;
        else if (diagnostic)
            first = -4L;
        else if (defects)
            first = -2L;
        else if (!swapped || !known || hy_frames_total != expected || hy_frames_ok != hy_frames_total ||
                 parked_n > end_left || unreached)
            first = -5L;
        else
            first = -1L;
        if (stopped_why)
            printf("STOPPED %s\n", stopped_why);
        fprintf(report, "calls swapped %llu; access defects %llu; parked %llu, kept %llu, unreached %lld\n"
                "frames compared %llu of %lld, identical %lu, first differing %ld\n", (unsigned long long)swapped,
                (unsigned long long)defects, (unsigned long long)parked_n, (unsigned long long)kept_n,
                stopped_why ? -1LL : (long long)unreached, (unsigned long long)hy_frames_total,
                known ? (long long)expected : -1LL, hy_frames_ok, first);
        fclose(report);
        for (unsigned i = 0; i < rondo_tag_count; i++) {
            if (stats[i].calls)
                printf("SWAPPED %s %llu\n", rondo_tags[i].name, (unsigned long long)stats[i].calls);
            if (stats[i].ints || stats[i].originals || stats[i].waits)
                printf("UNDER %s %llu %llu %llu\n", rondo_tags[i].name, (unsigned long long)stats[i].ints,
                       (unsigned long long)stats[i].originals, (unsigned long long)stats[i].waits);
            for (int k = 0; k < NKIND; k++)
                if (stats[i].kind[k])
                    printf("DEFECT %s %s %llu\n", rondo_tags[i].name, kind_name[k], (unsigned long long)stats[i].kind[k]);
        }
        for (unsigned i = 0; ran_inside && i < rondo_tag_count; i++)
            if (ran_inside[i])
                printf("RAN %s %llu\n", rondo_tags[i].name, (unsigned long long)ran_inside[i]);
        {
            uint64_t ni = 0, no = 0, nw = 0;

            for (unsigned i = 0; i < rondo_tag_count; i++) {
                ni += stats[i].ints;
                no += stats[i].originals;
                nw += stats[i].waits;
            }
            printf("hybrid: %llu swapped, frames compared %llu, identical %lu, first differing %ld, access defects %llu, "
                   "interrupts inside %llu, original calls %llu, waits %llu, frames expected %lld, parked %llu, "
                   "kept %llu, unreached %lld, unopened arrivals %llu, diagnostic %d\n",
                   (unsigned long long)swapped, (unsigned long long)hy_frames_total, hy_frames_ok, first,
                   (unsigned long long)defects, (unsigned long long)ni, (unsigned long long)no, (unsigned long long)nw,
                   known ? (long long)expected : -1LL, (unsigned long long)parked_n, (unsigned long long)kept_n,
                   stopped_why ? -1LL : (long long)unreached, (unsigned long long)unopened, diagnostic ? 1 : 0);
        }
    }
}
