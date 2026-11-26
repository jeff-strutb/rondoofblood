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
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <execinfo.h>
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
extern const unsigned rondo_ram_calls[];
extern const unsigned rondo_ram_call_count;
extern const struct Alias { const char *name; uint16_t run; } rondo_aliases[];
extern const unsigned rondo_alias_count;

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
    EV_OPEN, EV_CLOSE, EV_CALL, EV_RET, EV_INT, EV_IRET, EV_FRAME, EV_END,
    EV_UBEGIN, EV_UEND          /* the script's unit call: what the harness and the called routine do, between the two */
};
enum { K_JSR, K_IRQ, K_JMP };          /* OPEN, CALL: how the entry was made */
enum { CL_OK, CL_DROP, CL_LEFT, CL_END };   /* CLOSE: how it ended; CL_END: the script ended with the call open (the record's closing flush, not a hand-over) */

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
#define FRAMES 8192
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
    uint64_t arrive_nonres;  /* kept for the report's column: always 0 (an arrival while no row's bytes are resident is OTHER CODE) */
    uint64_t runaway;        /* the C ran past its access or write budget (counted divergent) */
    uint64_t prefix;         /* open at the script's end: its accesses judged as far as the log goes */
    uint64_t dispatch;       /* the C ended by an RTS that jumps to an address the routine pushed (the RTS-as-jump trick): judged up to there */
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
static unsigned res_mpr(unsigned i);
static uint32_t a5_phys(uint16_t addr);
static int in_routine(int index, uint16_t pc);

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
static struct { uint64_t ram, pic, when; uint16_t pc, top; uint8_t s; } *fq;
static uint64_t fq_head, fq_tail, fq_cap;

static void unit_note(uint64_t idx, int type);

static void store_push(const Ev *e)
{
    if (wn == wcap) {
        wcap = wcap ? wcap * 2 : (1u << 16);
        if (getenv("RONDO_DIAG_WIN") && wcap >= (1u << 22)) {
            void *bt[24];
            int nb = backtrace(bt, 24);

            fprintf(stderr, "WINGROW frame %lu cap %llu base %llu sizeof %zu\n", cur_frame, (unsigned long long)wcap,
                    (unsigned long long)wbase, sizeof *win);
            backtrace_symbols_fd(bt, nb, 2);
        }
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
        if (getenv("RONDO_DIAG_CTX"))
            fprintf(stderr, "ATEOF frame %lu window %llu..%llu feof %d ferror %d errno %d\n", cur_frame,
                    (unsigned long long)wbase, (unsigned long long)(wbase + wn), feof(in), ferror(in), errno);
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
        fq[fq_tail].pc = e.pc;
        fq[fq_tail].s = e.s;
        fq[fq_tail].top = e.ret;
        memcpy(&fq[fq_tail].when, e.mpr, 8);
        fq_tail++;
    }
    if (e.type == EV_UBEGIN || e.type == EV_UEND)
        unit_note(wbase + wn, e.type);
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
/*
 * A unit call is the script's own call of a routine (the `call` command), made at a frame boundary wherever the game is: its
 * accesses (the harness's mark push, the routine's own, what it calls) are in the stream between an EV_UBEGIN and an EV_UEND.
 * A call the game had open before it (the host) did not make them and its C does not: for a span that began before the
 * block they are noise, and the unit's writes are applied to A5's model as the cursor crosses them (apply_pokes). The unit
 * routine's own span, which begins inside the block, counts them as any call's.
 */
static struct { uint64_t b, e; } uranges[512];
static unsigned nur;
static int ur_open;                    /* the last range has begun and not ended */

static void unit_note(uint64_t idx, int type)
{
    if (type == EV_UBEGIN) {
        if (nur < sizeof uranges / sizeof uranges[0]) {
            uranges[nur].b = idx;
            uranges[nur].e = UINT64_MAX;
            nur++;
            ur_open = 1;
        }
    } else if (type == EV_UEND && nur && ur_open) {
        uranges[nur - 1].e = idx;
        ur_open = 0;
    }
}

static int unit_skip(const Span *s, uint64_t j)
{
    for (unsigned i = nur; i-- > 0;) {
        if (j >= uranges[i].b && j <= uranges[i].e)
            return s->start < uranges[i].b;
        if (uranges[i].e < j && uranges[i].e != UINT64_MAX)
            break;                 /* ranges are in order: the ones before this end before j */
    }
    return 0;
}

static Ev unit_noise = { .type = EV_ROMR };

static Ev *sev(Span *s, uint64_t j, int *nest)
{
    Ev *e;

    if (j >= s->end || !(e = evp(j)))
        return NULL;
    if (nur && unit_skip(s, j))
        return &unit_noise;
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

    /* at_eof says the READER reached the end of the stream: a scan's read-ahead gets there long before the hybrid
     * does. It ends this call only when the call itself has no end in what was read. */
    if (at_eof && s->end == SPAN_OPEN)
        return 1;
    return s->end != SPAN_OPEN && (c = evp(s->end)) && c->type == EV_CLOSE && (c->kind == CL_LEFT || c->kind == CL_END);
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
uint16_t rondo_pc;

/* A left hand-over at which the original took an interrupt right after (the record: the CLOSE, then the INT): the
 * original runs on from the instruction the C is at with the core's interrupts held (the I flag), until the core is at
 * the interrupted PC; there the interrupt is dispatched from the record as a C's is (hy_int_pass), at its clock. */
static struct {
    int active, tag, n;
    Ev ev;                  /* the INT event */
    uint64_t entry_idx;     /* the handler's CALL */
    uint16_t handler;
    uint8_t i_orig;         /* the I flag the C had at the hand-over */
} hold;
static uint64_t held_ints, held_not;
static uint64_t handover_ints;     /* interrupts the record took before the instruction a C handed over at: delivered by the C */
static uint64_t handover_frames;   /* frame ends the record took at the place a C handed over to the original: passed by the C */
static uint64_t retpoint_ints;
static uint64_t retpoint_frames;  /* frame ends the record had at that return place, passed by the C */
static uint64_t dispatch_held;    /* interrupts the record took at an RTS dispatch's target: held for the core's arrival there */   /* interrupts the record took at a resume place after original code that the core reached early: delivered by the C */
static unsigned hold_acc;     /* data accesses the core made under the hold */
void hold_prepare(Span *s, const Ev *closing);
int hold_arm(uint64_t end_idx, int tag);
int hold_arm_from(uint64_t k, int tag, int need, uint16_t pc, uint8_t sp_s);
void hand_over(uint16_t addr);
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
            return e->kind == CL_LEFT || e->kind == CL_END ? k : NO_RETURN;    /* a call left open at the record's end closes there */
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

        if (e && (is_poke(e) || (e->type == EV_W && sp && unit_skip(sp, k))))
            ram_bank[e->addr >> 13][e->addr & 0x1FFF] = e->val;
    }
}

/*
 * Places a counted access of the C: the index of the event it matches, or -1 when the original made
 * no such access before a barrier. Interrupts passed on the way are delivered before it returns.
 * A match that is not the next counted event is a MOVED defect; none at all is EXTRA.
 */
/* A call (JSR) into RAM code with no disc origin (the traces' provenance: rondo_ram_calls, from edges.csv) that comes back
 * to the routine that made it: its accesses are that routine's own, to be matched against its C, not a callee's. */
static int ram_call_phys(uint32_t phys)
{
    for (unsigned i = 0; i < rondo_ram_call_count; i++)
        if (rondo_ram_calls[i] == phys)
            return 1;
    return 0;
}

static int ram_call_returns(Span *s, uint64_t j, const Ev *call)
{
    uint32_t phys;
    int nest = s->nest;

    if (mode == MODE_A5)
        phys = a5_phys(call->pc);
    else
        phys = canon(((uint32_t)chk_mpr(call->pc >> 13) << 13) | (call->pc & 0x1FFF));
    if (!ram_call_phys(phys))
        return 0;
    for (uint64_t k = j + 1; k < j + 200000; k++) {
        Ev *x = sev(s, k, &nest);

        if (!x)
            return 0;
        if (x->type == EV_RET && x->s == (uint8_t)(call->s + 2))
            return 1;
    }
    return 0;
}

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
            static uint64_t last_j = UINT64_MAX;
            int passes = !getenv("RONDO_NO_RAMCALL") && ram_call_returns(s, j, e);     /* the switch is for comparing verdicts with and without the rule */

            if (getenv("RONDO_DIAG_UNTAGGED") && mode == MODE_A5 && j != last_j) {
                uint32_t tp = a5_phys(e->pc);
                uint8_t w[16];
                const uint8_t *page = is_ram(tp) ? ram_bank[tp >> 13] : is_rom(tp) ? rom_bank[tp >> 13] : NULL;
                const uint8_t *hit = NULL;

                /* Disc origin: the 16 bytes at the target are somewhere in the disc's data track, or are not (built in RAM). */
                if (page && (tp & 0x1FFF) <= 0x1FF0) {
                    memcpy(w, page + (tp & 0x1FFF), 16);
                    hit = memmem(user, 20785152, w, 16);
                }
                last_j = j;
                fprintf(stderr, "UNTAGGEDCALL routine %s target %04X phys %06X %s frame %lu %s\n", rondo_tags[s->tag].name, e->pc, tp,
                        !page ? "hw" : hit ? "disc" : "nodisc", cur_frame, passes ? "passed" : "kept");
            }
            if (passes)
                continue;      /* the stub's accesses are this routine's own: matched against the C's */
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
    if (getenv("RONDO_DIAG_CTX") && found < 0 && mode == MODE_HYBRID)
        fprintf(stderr, "PLACEFAIL frame %lu in %s: type %d phys %06X hit_end %d first %lld at_eof %d s->end %llu cur %llu rondo_pc %04X ends_here %d close %d/%d\n",
                cur_frame, rondo_tags[s->tag].name, type, phys, hit_end, (long long)first, (int)at_eof,
                (unsigned long long)s->end, (unsigned long long)s->cur, rondo_pc, record_ends_here(s),
                s->end != SPAN_OPEN && evp(s->end) ? evp(s->end)->type : -1,
                s->end != SPAN_OPEN && evp(s->end) ? evp(s->end)->kind : -1);
    if (found < 0 && getenv("RONDO_DIAG_CTX") && mode == MODE_HYBRID && s->end != SPAN_OPEN) {
        for (uint64_t k = s->end; k < s->end + 14; k++) {
            Ev *x = evp(k);

            if (!x)
                break;
            fprintf(stderr, "  AFTER %llu type %d kind %d addr %06X pc %04X s %02X clock %llu val %02X\n", (unsigned long long)k, x->type,
                    x->kind, x->addr, x->pc, x->s, (unsigned long long)x->clock, x->val);
        }
    }
    if (found < 0 && hit_end && first < 0 && mode == MODE_HYBRID && sp == s && record_ends_here(s)) {
        /* The original's call ended here (left by the stack: it pulled past its own return address) while the C goes on:
         * the C hands the machine to the original at the instruction it is at, whose access it has not made. Only the
         * script's end parks the C for good. */
        /* The call was closed as left in the record: that is the case for the hand-over, whether or not the reader has
         * since read the stream to its end (the read-ahead of a scan can: at_eof says the READER is at the end, not
         * that this call is). Only a call the record never closes, in a stream that ended, parks for good. */
        Ev *closing = s->end != SPAN_OPEN ? evp(s->end) : NULL;

        if (rondo_pc && closing && closing->type == EV_CLOSE && closing->kind == CL_LEFT) {
            hold_prepare(s, closing);
            hand_over(rondo_pc);
        }
        park_forever();
    }
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
/* Did the C end by an RTS that does not return to its caller? It pushed more than it pulled, so the address the RTS pops is
 * one it pushed itself: a jump (a table dispatch), after which the original's call goes on in other routines. */
static int ends_by_dispatch(uint8_t entry_s, int is_irq, const Tag *t)
{
    uint8_t below = (uint8_t)(entry_s - cpu.s);

    return !is_irq && !t->ends && below >= 1 && below < 0x80;
}

#define HORIZON (8u << 20)           /* hybrid: events a call's return or end is looked for ahead (8M events, 384 MB of window) */
static uint64_t horizon_calls;
static int horizon_hit;

/* A C that ends with S below its entry RTSes to an address on its own stack. That is a dispatch only if the original
 * made the same jump in this call: the arrival its RTS logs (a RET marker, or the CALL of a tagged routine entered by
 * RTS) at the address the C's stack holds, with S two above the C's, before the call's end (interrupts skipped).
 * Otherwise the C left a push or a frame on the stack where the original returned, and the call is judged whole, so
 * the S compare names it in its own call, not in the next. The match is on the arrival's address and S, not on a
 * nesting count: A5's record has no CLOSE events to count with. c_s is the C's S at its end; target the address its
 * RTS goes to. */
static int original_dispatched(Span *s, uint8_t c_s, uint16_t target)
{
    int nest = s->nest;

    for (uint64_t j = s->cur; mode != MODE_HYBRID || j - s->cur <= HORIZON; j++) {
        Ev *e = sev(s, j, &nest);

        if (!e)
            return 0;
        if (e->type == EV_INT) {
            uint64_t r = section_end(s, j);

            if (r == NO_RETURN)
                return 0;
            j = r;
            continue;
        }
        if ((e->type == EV_RET || (e->type == EV_CALL && e->kind != K_IRQ)) && e->pc == target &&
            e->s == (uint8_t)(c_s + 2))
            return 1;
    }
    return 0;
}

static int a5_handed;       /* A5: the C handed the rest of the call to the original (a tail, leave_original): what it left on the stack is the original's code's to pop */
static uint64_t not_dispatched;      /* calls whose C ended below its entry S where the original made no such jump */

/* ends_by_dispatch, confirmed against the record: counted and (RONDO_DIAG_DISPATCH) printed when the record refuses. */
static int dispatch_confirmed(Span *s, uint8_t entry_s, uint16_t target, const char *name)
{
    if (original_dispatched(s, cpu.s, target))
        return 1;
    not_dispatched++;
    if (getenv("RONDO_DIAG_DISPATCH"))
        fprintf(stderr, "NOTDISPATCH frame %lu %s: entry S%02X, the C ends at S%02X and its RTS goes to %04X; the original "
                "made no such jump: judged whole\n", cur_frame, name, entry_s, cpu.s, target);
    return 0;
}

static void span_finish_to_call(Span *s, uint16_t target, uint8_t arrival_s);

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
        if (mode == MODE_HYBRID && s->end == SPAN_OPEN && j - s->cur > HORIZON) {
            horizon_calls++;      /* a call with no end in the next HORIZON events: the rest is not read into memory here */
            break;
        }
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

/* As span_finish, for a call that ended by dispatch: what is left up to the arrival at the dispatched address (where the
 * record's next call begins) is passed or reported; the rest of the span belongs to the dispatched routines. */
static void span_finish_to_call(Span *s, uint16_t target, uint8_t arrival_s)
{
    int reported = 0;
    int nest = s->nest;
    unsigned pulls = 0;
    uint64_t j;

    for (j = s->cur;; j++) {
        Ev *e = sev(s, j, &nest);
        char b[96];

        if (!e || e->type == EV_CALL || e->type == EV_OPEN)
            break;
        /* The interrupt the original took at the dispatched address, its RTS run (S as after it): not the C's to deliver
         * before its own RTS; hy_finish holds it for the core's arrival there. */
        if (e->type == EV_INT && e->pc == target && e->s == arrival_s)
            break;
        if (e->type == EV_RTPULL)
            pulls++;
        if (e->type == EV_INT) {
            uint64_t r = section_end(s, j);

            if (r == NO_RETURN)
                break;
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
    s->cur = j;
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
    int parked, done, abandoned, natural, dispatch;
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
static const char *pace_site = "";

static void pace_to(uint64_t want)
{
    uint64_t now = hybrid_clock_now();

    if (want > now) {
        uint64_t d = want - now;

        chk_charge((int)(d > 0x7FFFFFFF ? 0x7FFFFFFF : d));
    } else if (want == 0 && getenv("RONDO_DIAG_AHEAD")) {
        static unsigned zero_n;

        if (zero_n++ < 5)
            fprintf(stderr, "PACE-TO-ZERO frame %lu after site '%s': the event has no clock\n", cur_frame, pace_site);
    } else if (want < now && getenv("RONDO_DIAG_AHEAD")) {
        /* The core is ahead of the record: the time cannot be given back. This is where a lead shows first. */
        fprintf(stderr, "AHEAD frame %lu %s: the core is %llu clocks past the record's %llu\n", cur_frame, pace_site,
                (unsigned long long)(now - want), (unsigned long long)want);
    }
}

static void pace_to_access(uint32_t phys, uint64_t stamp)
{
    uint32_t off = phys & 0x1FFF;

    if ((phys >> 13) == 0xFF && ((off & 0x1C00) == 0x0000 || (off & 0x1C00) == 0x0400))
        stamp -= chk_get_speed() ? 3 : 12;
    pace_to(stamp);
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
    pace_site = "interrupt (C)";
    pace_to(ev.clock);
    /* The interrupt's own eight cycles (seven, and the one that samples the next), which the core spends before the
     * handler's first instruction: 3 master clocks a cycle at the fast clock, 12 at the slow. */
    chk_charge(8 * (ev.speed ? 3 : 12));
    for (unsigned k = 0; k < 8; k++)
        mpr[k] = (uint8_t)chk_mpr(k);
    if (memcmp(mpr, ev.mpr, 8) || chk_get_speed() != ev.speed)
        defect(D_INT, "at the interrupt the MPRs or the CPU clock differ from the original's");
    if (cpu.s != ev.s && getenv("RONDO_DIAG_CTX")) {
        Ev *last = s->cur > s->start ? evp(s->cur - 1) : NULL;

        fprintf(stderr, "INTPASS frame %lu in %s: INT event %llu at PC %04X S%02X, the C's S%02X; the C's last placed access (event %llu) type %d addr %06X\n",
                cur_frame, rondo_tags[s->tag].name, (unsigned long long)i, ev.pc, ev.s, cpu.s, (unsigned long long)(s->cur - 1),
                last ? last->type : -1, last ? last->addr : 0u);
        for (uint64_t k = i; k < i + 13; k++) {
            Ev *x = evp(k);

            if (!x)
                break;
            fprintf(stderr, "  INTPASS +%llu type %d kind %d addr %06X pc %04X s %02X val %02X\n", (unsigned long long)(k - i), x->type, x->kind,
                    x->addr, x->pc, x->s, x->val);
        }
    }
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
    if (getenv("RONDO_DIAG_RESUME"))
        fprintf(stderr, "DIAGFRAME frame %lu event %llu wait_frame %lu in %s S%02X pulls %u\n", cur_frame,
                (unsigned long long)idx, c->wait_frame, rondo_tags[c->tag].name, cpu.s, pulls);
    if (getenv("RONDO_DIAG_CTX") && (uint8_t)(cpu.s + pulls) != e->s) {
        /* The C stands where the original's frame ended, and its S is not the original's: the C's stack differs here. */
        fprintf(stderr, "FRAMEPASS frame %lu in %s (%s): the C's S%02X (+%u pulls), the record's S%02X at PC %04X; fs:", cur_frame,
                rondo_tags[c->tag].name, blame_tag >= 0 ? rondo_tags[blame_tag].name : "-", cpu.s, pulls, e->s, e->pc);
        for (int i = 0; i < c->fsn && i < FS_MAX; i++)
            fprintf(stderr, " %s", rondo_tags[c->fs[i]].name);
        fprintf(stderr, "\n");
        for (uint64_t k = idx > 14 ? idx - 14 : 0; k < idx; k++) {
            Ev *x2 = evp(k);

            if (x2)
                fprintf(stderr, "  FRAMEPASS -%llu type %d kind %d addr %06X pc %04X s %02X val %02X\n", (unsigned long long)(idx - k), x2->type,
                        x2->kind, x2->addr, x2->pc, x2->s, x2->val);
        }
    }
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

int enters(uint16_t addr, uint16_t from, int jmp)
{
    int index = tag_pick(addr);
    int inside = index >= 0 && in_routine(index, from);
    int r = jmp ? !inside : (index >= 0 && !inside);

    if (!r && getenv("RONDO_DIAG_CTX") && mode != MODE_RECORD)
        fprintf(stderr, "ENTERS no call at %04X from %04X (%s, tag %d) frame %lu mode %d\n", addr, from, jmp ? "jmp" : "branch", index,
                cur_frame, mode);
    return r;
}

/* An instruction byte the routine patched (the operand of its own code in RAM): read as the CPU fetches it, no access. */
uint8_t code8(uint16_t addr)
{
    uint32_t p;

    if (mode == MODE_HYBRID)
        return chk_bus_rd(addr);
    p = a5_phys(addr);
    if (is_rom(p))
        return rom_bank[p >> 13][p & 0x1FFF];
    return is_ram(p) ? ram_bank[p >> 13][p & 0x1FFF] : 0xFF;
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
        uint64_t before;

        if (e && !is_ram(p)) {
            pace_site = "hardware read";
            pace_to_access(p, e->clock);
        }
        before = hybrid_clock_now();
        v = chk_bus_rd(addr);
        if (e && e->val != v) {
            char b[96];

            defect(D_VALUE, "C read %06X gave %02X, the original's %s (the core's clock before the read %llu, after %llu; the record's %llu)", p, v,
                   describe(e, b, sizeof b), (unsigned long long)before, (unsigned long long)hybrid_clock_now(),
                   (unsigned long long)e->clock);
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
        if (e && !is_ram(p)) {
            pace_site = "hardware write";
            pace_to(e->clock);
        }
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
        if (e) {
            pace_site = "st0/st1/st2";
            pace_to(e->clock);
        }
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
    uint8_t v[8], now[8];
    int changes;

    mprs_now(v);
    memcpy(now, v, 8);
    for (unsigned i = 0; i < 8; i++)
        if (mpr_bits & (1u << i))
            v[i] = bank;
    /* The record logs a mapping only when it changes (sync_state): a TAM that maps what is already mapped is no
     * event on either side. */
    changes = memcmp(now, v, 8) != 0;
    if (sp && changes)
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

/*
 * resident_at(run): the disc location (DISC(sector, offset)) of the tagged routine whose bytes are resident at that
 * run address under the mapping the caller sees (A5: the C's own, against the call's rewound memory; lockstep and the
 * record: the live core's), or 0. It reads the routine table and nothing else: no access of the original is made, so
 * none is placed or counted. It exists so that C can name an overlay's routine: where one address holds one of several
 * variants, C asks which, and calls that one. A port sets the answer when an overlay loads.
 */
uint32_t resident_at(uint16_t run)
{
    int i = tag_pick(run);

    if (i < 0 || rondo_tags[i].rom || !tag_resident(&rondo_tags[i]))
        return 0;
    return rondo_tags[i].disc;
}

void cli(void)
{
    cpu.p &= (uint8_t)~P_I;
}

static void set_speed(uint8_t fast)
{
    uint8_t v[8];

    mprs_now(v);
    /* As for TAM: a clock change that changes nothing is no event on either side. */
    if (sp && fast != (mode == MODE_HYBRID ? chk_get_speed() : c_speed))
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
    if (!e && mode == MODE_HYBRID && getenv("RONDO_DIAG_CTX"))
        fprintf(stderr, "WAITEND frame %lu in %s: at_eof %d span end %llu cur %llu j %llu ends_here %d\n", cur_frame,
                rondo_tags[sp->tag].name, (int)at_eof, (unsigned long long)sp->end, (unsigned long long)sp->cur,
                (unsigned long long)j, record_ends_here(sp));
    if (!e && mode == MODE_HYBRID && getenv("RONDO_DIAG_CTX") && sp->end != SPAN_OPEN) {
        for (uint64_t q = sp->end > 3 ? sp->end - 3 : 0; q < sp->end + 6; q++) {
            Ev *x = evp(q);

            if (x)
                fprintf(stderr, "  WAITEV %llu type %d kind %d pc %04X s %02X addr %u clock %llu\n", (unsigned long long)q,
                        x->type, x->kind, x->pc, x->s, x->addr, (unsigned long long)x->clock);
        }
    }
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
static uint16_t tail_from;      /* call_original_at: where the tail jump came from */

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
    if (!found && mode == MODE_HYBRID && sp == span && record_ends_here(span)) {
        /* The record's call has ended (the script did, or the original left it) before the call the C goes on to make:
         * as at an access past the end (place), the original takes the machine over where the C is when it left the
         * call, and a call the record never closes in a stream that ended parks the C. */
        Ev *closing = span->end != SPAN_OPEN ? evp(span->end) : NULL;

        if (rondo_pc && closing && closing->type == EV_CLOSE && closing->kind == CL_LEFT) {
            hold_prepare(span, closing);
            hand_over(rondo_pc);
        }
        park_forever();
    }
    if (!found) {
        if (getenv("RONDO_DIAG_CTX") && mode == MODE_HYBRID) {
            int ti = tag_pick(addr);

            fprintf(stderr, "  NOCALL tag at %04X: index %d %s resident %d\n", addr, ti, ti >= 0 ? rondo_tags[ti].name : "-",
                    ti >= 0 ? tag_resident(&rondo_tags[ti]) : -1);
        }
        if (getenv("RONDO_DIAG_CTX") && mode == MODE_HYBRID)
            for (uint64_t k = sp->cur; k < sp->cur + 16; k++) {
                Ev *x = evp(k);

                if (!x)
                    break;
                fprintf(stderr, "  NOCALL %llu type %d kind %d addr %06X pc %04X s %02X clock %llu val %02X (the C calls %04X from %04X, S%02X)\n",
                        (unsigned long long)k, x->type, x->kind, x->addr, x->pc, x->s, (unsigned long long)x->clock, x->val, addr,
                        rondo_pc, cpu.s);
            }
        if (mode == MODE_A5)
            stop_c(ABORT_DIVERGED, "C calls %04X; the original made no such call", addr, 0, 0);
        defect(D_CALL, "C calls %04X; the original made no such call", addr);
        stop_run(3, "a call into original code the record does not have");
    }
    /* Everything counted before the call the C has not made. */
    for (uint64_t k = span->cur; k < call; k++) {
        Ev *x = evp(k);
        char b[96];

        if (unit_skip(span, k))
            continue;

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
    /* The stack at the call: the callee's first instruction runs at the S the JSR left (a tail jump, the S it was at).
     * The C carries its own S, so a push or pop it has and the original has not shows here, at the call, and not only
     * where the call ends or an interrupt comes. */
    if (ec.s != cpu.s) {
        if (mode == MODE_A5) {
            snprintf(why, sizeof why, "to %04X: C S%02X, original S%02X", addr, cpu.s, ec.s);
            longjmp(c_abort, ABORT_DIVERGED);
        }
        defect(D_STACK, "at the call to %04X the C's S is %02X, the original's %02X", addr, cpu.s, ec.s);
    }
    /* Where it comes back: the RET that pops the callee's frame. A tail jump pushes none, and its code returns
     * to the caller of the routine that jumped. */
    for (retidx = call + 1;; retidx++) {
        Ev *x = sev(span, retidx, &nest);

        if (!x || (x->type == EV_RET && x->s == (uint8_t)(ec.s + 2) && (!ret || x->pc == ret)))
            break;
        if (mode == MODE_HYBRID && own && retidx - call > HORIZON) {
            /* A callee that has not returned within HORIZON events: the search would read the rest of the script into
             * memory (a stage's main loop never returns). The routine ends here and the original runs on; if the
             * callee did return later, what the C would have done after it is the original's, counted. */
            horizon_calls++;
            horizon_hit = 1;
            break;
        }
    }
    noreturn = horizon_hit || retidx >= span->end || !evp(retidx);
    horizon_hit = 0;
    if (own && (ec.kind == K_JMP || noreturn)) {
        /* The routine's own tail jump, or code that does not come back in the call: the routine ends there.
         * (A routine called by C from another's C that jumps this way is a call and a return: below.) */
        if (mode == MODE_A5) {
            const Regs *x = exit_regs_a5;

            a5_handed = 1;
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
            if (ret) {      /* a call the original never came back from: the C's code after it is not the original's */
                stats[span->tag].noreturn++;
                longjmp(c_abort, ABORT_END);
            }
            return;
        }
        {
            Ctx *c = top;

            stats[span->tag].originals++;
            pace_site = "tail";
            pace_to(ec.clock);
            chk_set_regs(cpu.a, cpu.x, cpu.y, cpu.s, cpu.p, addr);
            prev_op = ec.kind == K_JMP ? 0x4C : 0x20;
            last_pc = tail_from ? tail_from : rondo_tags[span->tag].run;     /* the jump comes from this routine, not from the code run before */
            tail_from = 0;
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
        int natural0;
        int took;

        pace_site = "call";
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
        natural0 = top ? top->natural : 0;
        cpu_run_until(back, (uint8_t)(s0 + 2));
        took = top ? top->natural - natural0 : 0;     /* the interrupts the core took at the return place itself */
        {
            uint8_t a, x, y, s, p;
            uint16_t pc;

            chk_get_regs(&a, &x, &y, &s, &p, &pc);
            cpu.a = a;
            cpu.x = x;
            cpu.y = y;
            cpu.p = p;
        }
        /* An interrupt the core took at the place the C waits to resume at (check_irq counted it as `natural`) can sit
         * in the record before the callee's RET: the callee's return and the interrupt at its return address come
         * in that order. It is then part of the stretch just run on the live machine, not an interrupt a later
         * access of the C will pass: take it off the count, or the next interrupt the C passes is thought to have
         * been taken already. */
        for (uint64_t k = call + 1; k < retidx && top && top->natural > 0; k++) {
            Ev *x = evp(k);

            if (x && x->type == EV_INT) {
                uint64_t r2 = section_end(span, k);

                top->natural--;
                if (r2 == NO_RETURN)
                    break;
                k = r2;
            }
        }
        /* The record's interrupt at the return place itself (the callee's RTS has run, its RET follows the handler) that
         * the core did not take there: the core arrives early when the code ran before it was C, whose instructions take
         * no time (a draft's hand-over to the original a few instructions before the RTS), so the C resumes before the
         * interrupt is due. The original took it before the instruction at the return address: it is delivered here, at
         * the record's clock, with S as it was there, as call_return_point delivers one at a C's own return. */
        /* A frame end the record has at the return place itself (the callee's RTS run, before the instruction at the
         * return address), for the same reason: the core came back before the frame ended, and the C would go on and make
         * accesses the original made in the next frame. It is passed here with the return S (frame_pass does nothing when
         * the core has ended that frame already). */
        for (uint64_t k = call + 1; top && k < retidx; k++) {
            Ev *x = evp(k);
            uint64_t r2;

            if (!x)
                break;
            if (x->type == EV_FRAME && x->pc == back && x->s == (uint8_t)(s0 + 2)) {
                cpu.s = (uint8_t)(s0 + 2);
                if (cur_frame < x->addr)
                    retpoint_frames++;
                frame_pass(span, k, 0);
                cpu.s = s0;
                continue;
            }
            if (x->type != EV_INT)
                continue;
            r2 = section_end(span, k);
            if (r2 == NO_RETURN)
                break;
            if (natural0 == 0 && took == 0 && x->pc == back && x->s == (uint8_t)(s0 + 2)) {
                cpu.s = (uint8_t)(s0 + 2);
                retpoint_ints++;
                hy_int_pass(span, k, r2);
                cpu.s = s0;
                break;
            }
            k = r2;
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

        uint8_t p = cpu.p;

        stats[sp->tag].originals++;
        /* The record's frame end at the place the C stands, with the C's S: before the original ran the instruction at
         * rondo_pc, or the branch there and before the instruction at addr (a branch makes no access, so both are this
         * place in the record). The C's instructions take no time, so the core, given the machine here, would end that
         * frame only after it ran on (out of the routine, with another S): it is passed here, as an access past it would
         * pass it. */
        {
            int nest = sp->nest;

            for (uint64_t j = sp->cur;; j++) {
                Ev *e = sev(sp, j, &nest);

                if (!e)
                    break;
                if (is_noise(e) && e->type != EV_RTPULL)
                    continue;
                if (e->type == EV_INT && e->pc == rondo_pc && e->s == cpu.s) {
                    /* The interrupt the original took before the instruction the C stands at (a branch it then took):
                     * delivered by the C, which resumes at that PC, as at a C's return point. One taken after the branch,
                     * at addr, is the hold's below. */
                    uint64_t r = section_end(sp, j);

                    if (r == NO_RETURN)
                        break;
                    handover_ints++;
                    hy_int_pass(sp, j, r);
                    sp->cur = r + 1;
                    j = r;
                    continue;
                }
                if (e->type != EV_FRAME || (e->pc != rondo_pc && e->pc != addr) || e->s != cpu.s)
                    break;
                handover_frames++;
                frame_pass(sp, j, 0);
                sp->cur = j + 1;
            }
        }
        /* The machine goes to the original here. When the record's next event past the markers is the interrupt it took at
         * this very PC and S (before it ran the instruction at addr), it is held and dispatched from the record, as after a
         * left close; else the original's own timing carries on. */
        if (!hold.active)
            hold_arm_from(sp->cur, sp->tag, 1, addr, cpu.s);
        if (hold.active) {
            hold.i_orig = p & P_I;
            p |= P_I;
        }
        chk_set_regs(cpu.a, cpu.x, cpu.y, cpu.s, p, addr);
        prev_op = 0;
        scan = sp->cur;
        c->abandoned = 1;
        swapcontext(&c->uc, &main_uc);
    }
}

/* At a left hand-over: the C's registers against the original's at the close (when the C is at the PC the original
 * left at), and the interrupt that follows the close in the record, if the next event after the close markers is one. */
int hold_arm_from(uint64_t k, int tag, int need, uint16_t pc, uint8_t sp_s)
{
    Ev *x;

    hold.active = 0;
    for (; (x = evp(k)) && (x->type == EV_CLOSE || x->type == EV_STATE); k++)
        ;
    if (!x || x->type != EV_INT || (need && (x->pc != pc || x->s != sp_s)))
        return 0;
    hold.ev = *x;
    hold.handler = 0;
    for (uint64_t j = k + 1; j < k + 64 && (x = evp(j)); j++)
        if (x->type == EV_CALL && x->kind == K_IRQ) {
            hold.handler = x->pc;
            hold.entry_idx = j;
            break;
        }
    if (!hold.handler)
        return 0;
    hold.tag = tag;
    hold.n = 0;
    hold_acc = 0;
    hold.active = 1;
    return 1;
}

/* The interrupt the record has right after the close at end_idx (past the other closes and MPR changes), if any: armed as
 * the hold, 1; else 0. */
int hold_arm(uint64_t end_idx, int tag)
{
    return hold_arm_from(end_idx + 1, tag, 0, 0, 0);
}

void hold_prepare(Span *s, const Ev *closing)
{
    hold.active = 0;
    if (closing->pc == rondo_pc && (cpu.a != closing->a || cpu.x != closing->x || cpu.y != closing->y || cpu.s != closing->s ||
                                    ((cpu.p ^ closing->p) & PFLAGS)))
        defect(D_REGS, "at the hand-over at %04X the C has A%02X X%02X Y%02X S%02X P%02X, the original A%02X X%02X Y%02X S%02X P%02X",
               rondo_pc, cpu.a, cpu.x, cpu.y, cpu.s, cpu.p, closing->a, closing->x, closing->y, closing->s, closing->p);
    /* The C is where the original was when the call was closed as left, so the core is too: at the close's clock, not the
     * clock of the last access the C paced to (the instructions the C ran since took no time). From there the original's
     * own timing takes the core to what comes next (an interrupt is taken where the original took it). */
    if (closing->pc == rondo_pc) {
        if (closing->clock >= hybrid_clock_now()) {
            pace_site = "left hand-over";
            pace_to(closing->clock);
        } else
            defect(D_INT, "at the hand-over at %04X the core's clock is %llu clocks past the original's", rondo_pc,
                   (unsigned long long)(hybrid_clock_now() - closing->clock));
    }
    if (!hold_arm(s->end, s->tag))
        held_not++;
}

/* A tail jump from the instruction at `from`: that is where the original's jump came from, and the hook judges
 * an arrival at a routine's first byte by it (a loop inside the routine, or an entry from other code). A draft
 * that runs on into another piece inline can jump from an address outside its own routine's range. */
void call_original_at(uint16_t from, uint16_t addr)
{
    tail_from = from;
    original_at(0, addr);
}

/*
 * leave_original(addr): the C set S outright (TXS): the call's frame is gone, and the code after it is not the call's.
 * The record closes the call as left. In lockstep the original runs on from addr as a tail jump does; in A5 the
 * call is judged as left by the stack (unverified) and the C returns.
 */
void leave_original(uint16_t addr)
{
    if (mode == MODE_A5) {
        a5_handed = 1;
        return;
    }
    hand_over(addr);
}

void call_original(uint16_t addr)
{
    original_at(0, addr);
}

/*
 * The C's call is about to end (jsr_pop, call_frame: just before the S update that drops the call's frame). In the record the
 * original took an interrupt before its RTS pulled the return address when the next events after the C's last placed
 * access are the interrupt and then the RTPULLs: the interrupt is delivered here, with S as the original had it (the frame
 * still on the stack), not at the next access of the caller, after the frame is gone. Hybrid only; A5 passes the
 * interrupts left at the call's end in span_finish.
 */
void call_return_point(void)
{
    Span *s = sp;
    int nest;
    uint64_t j;

    if (mode != MODE_HYBRID || !s || getenv("RONDO_NO_RETURN_POINT"))      /* the switch is for comparing runs with and without the hook */
        return;
    nest = s->nest;
    for (j = s->cur;; j++) {
        uint64_t end0 = s->end;
        Ev *e = sev(s, j, &nest);
        uint64_t r;

        if (!e && end0 == SPAN_OPEN && s->end != SPAN_OPEN && getenv("RONDO_DIAG_CTX"))
            fprintf(stderr, "RETPOINT %s frame %lu: scan from %llu found the call's end at %llu (nest at start %d)\n",
                    rondo_tags[s->tag].name, cur_frame, (unsigned long long)s->cur, (unsigned long long)s->end, s->nest);
        if (e && is_noise(e) && e->type != EV_RTPULL)
            continue;            /* a read of ROM or a poke between the C's last access and the interrupt (uncounted) */
        if (e && (e->type == EV_CLOSE || e->type == EV_RET))
            continue;            /* the end of a call nested in this one, which the C has returned from already (its own return point has passed) */
        if (e && e->type == EV_RTPULL) {
            /* The pulls of this call's own RTS (its return address at S+1 and S+2 of the stack) stop the scan: the interrupt or
             * frame end comes before them. Pulls of a nested call's RTS, already returned from, are passed. */
            uint32_t own = 0x1F0100u + (uint8_t)(cpu.s + 1);

            if (e->addr == own || e->addr == own + 1)
                break;
            continue;
        }
        if (e && e->type == EV_FRAME) {
            /* The frame ended in the original with the call's RTS not yet run (the pulls follow): the hash is taken with
             * the frame still on the stack, as the record took it. */
            s->nest = nest;      /* the nested CLOSEs passed are behind the cursor now */
            frame_pass(s, j, 0);
            s->cur = j + 1;
            continue;
        }
        if (!e || e->type != EV_INT)
            break;
        r = section_end(s, j);
        if (r == NO_RETURN)
            break;
        s->nest = nest;
        hy_int_pass(s, j, r);
        s->cur = r + 1;
        j = r;
    }
}

void call_orig_framed(uint16_t ret, uint16_t addr)
{
    original_at(ret, addr);
    cpu.s = (uint8_t)(cpu.s + 2);
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
/* RONDO_DUMP_SPAN=NAME,N: the record's events of the Nth call of routine NAME (A5 and the hybrid), one line each: the index
 * from the call's first event, type, kind, address, pc, s, val and clock. Types: 0 R, 1 W, 2 ROMR, 3 HWR, 4 HWW, 5 RTPULL,
 * 6 STATE, 7 OPEN, 8 CLOSE, 9 CALL, 10 RET, 11 INT, 12 IRET, 13 FRAME. */
static void dump_span(const char *name, uint64_t callno, uint64_t from, uint64_t to)
{
    const char *want = getenv("RONDO_DUMP_SPAN");
    char buf[128];
    unsigned long n;

    if (!want || sscanf(want, "%127[^,],%lu", buf, &n) != 2 || strcmp(buf, name) || n != callno)
        return;
    fprintf(stderr, "SPAN %s call %llu, events %llu to %llu:\n", name, (unsigned long long)callno, (unsigned long long)from,
            (unsigned long long)to);
    for (uint64_t k = from; k < to; k++) {
        Ev *x = evp(k);

        if (!x)
            break;
        fprintf(stderr, "SPAN +%llu type %d kind %d addr %06X pc %04X s %02X val %02X clock %llu\n", (unsigned long long)(k - from),
                x->type, x->kind, x->addr, x->pc, x->s, x->val, (unsigned long long)x->clock);
    }
}

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
        if (e->type == EV_UBEGIN || e->type == EV_UEND)
            unit_note(wn, e->type);
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
    if (kind == 0 && hold.active)
        hold_acc++;
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

    if (hold.active)
        hold_acc++;
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
    if (mode == MODE_HYBRID && getenv("RONDO_DIAG_CTX")) {
        uint64_t k = scan;
        Ev *x;

        while ((x = evp(k)) && (x->type == EV_CLOSE || x->type == EV_STATE || x->type == EV_RET))
            k++;
        fprintf(stderr, "IRQCORE frame %lu core pc %04X S%02X clock %llu top %s%s; record next %llu type %d pc %04X s %02X clock %llu\n", cur_frame, r.pc,
                r.s, (unsigned long long)hybrid_clock_now(), top ? rondo_tags[top->tag].name : "-",
                top && top->parked ? "(parked)" : "", (unsigned long long)k, x ? x->type : -1, x ? x->pc : 0, x ? x->s : 0,
                x ? (unsigned long long)x->clock : 0ull);
    }
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
    /* An interrupt's return to the first byte of the routine whose frame is on top at this S resumes that frame (an idle
     * loop the interrupt came in on): not a new call. Without it each interrupt left one more idle frame open and the
     * record's frame stack hit its 512 limit, which flushed every frame as left (a long script's idle loop then lost
     * the interrupts inside it). */
    if (!self_jump && index >= 0 && prev_op == 0x40 && depth && mode != MODE_HYBRID && frames[depth - 1].tag_index == index) {
        uint8_t a, x, y, st, p;
        uint16_t pc2;

        chk_get_regs(&a, &x, &y, &st, &p, &pc2);
        if (frames[depth - 1].regs.s == st)
            self_jump = 1;
    }
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
        if (how == CL_LEFT && getenv("RONDO_DEBUG_LEFT") && (!strcmp(getenv("RONDO_DEBUG_LEFT"), "all") || !strcmp(f->tag->name, getenv("RONDO_DEBUG_LEFT"))))
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
        if (getenv("RONDO_DEBUG_LEFT"))
            fprintf(stderr, "OVERFLOW frame %lu: %d frames open, the %d limit; every one is flushed as left; top %s\n", cur_frame,
                    depth, FRAMES, frames[depth - 1].tag->name);
        if (getenv("RONDO_DEBUG_LEFT")) {
            int i = 0;

            while (i < depth && i < FRAMES) {
                int j = i;

                while (j + 1 < depth && frames[j + 1].tag == frames[i].tag && frames[j + 1].regs.s == frames[i].regs.s)
                    j++;
                fprintf(stderr, "  OPEN %3d..%3d %s S=%02X\n", i, j, frames[i].tag->name, frames[i].regs.s);
                i = j + 1;
            }
        }
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

static uint64_t probe_calls, probe_bad;
static uint32_t probe_seen[2];     /* the two variants a probe saw first */

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
    dump_span(f->tag->name, s->calls, f->start, end);
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
    if (getenv("RONDO_PROBE_RESIDENT") && f->tag->run == (uint16_t)strtoul(getenv("RONDO_PROBE_RESIDENT"), NULL, 16)) {
        /* The check of resident_at in A5: at the call's entry, against its rewound memory and its mapping, it must name
         * the routine the record opened here. */
        uint32_t got = resident_at(f->tag->run);

        probe_calls++;
        if (got != (f->tag->rom ? 0 : f->tag->disc)) {
            probe_bad++;
            fprintf(stderr, "PROBE %04X: %s call: resident_at gives %u, the record's call is of %u\n", f->tag->run,
                    f->tag->name, got, f->tag->disc);
        } else if (probe_seen[0] != got && probe_seen[1] != got && probe_seen[1] == 0 && probe_seen[0] != 0) {
            probe_seen[1] = got;
        } else if (!probe_seen[0]) {
            probe_seen[0] = got;
        }
    }
    a5_handed = 0;
    reason = setjmp(c_abort);
    if (reason == 0) {
        f->tag->abi();
        apply_pokes(span.cur, end);
        if (!prefix && ends_by_dispatch(f->regs.s, f->kind == K_IRQ, f->tag) &&
            (a5_handed || dispatch_confirmed(&span, f->regs.s, (uint16_t)((ram_bank[0xF8][0x100 | (uint8_t)(cpu.s + 1)] |
                                                             (ram_bank[0xF8][0x100 | (uint8_t)(cpu.s + 2)] << 8)) + 1),
                               f->tag->name))) {
            s->dispatch++;
            longjmp(c_abort, ABORT_END);       /* the original's call goes on in the routines it jumped to: judged up to here */
        }
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
        if (getenv("RONDO_DIAG_EXIT") && !strcmp(f->tag->name, getenv("RONDO_DIAG_EXIT")))
            fprintf(stderr, "EXIT %s call %llu frame %lu: C S%02X (entry S%02X) exit S%02X pc %04X, the original S%02X pc %04X\n", f->tag->name,
                    (unsigned long long)s->calls, cur_frame, cpu.s, f->regs.s, exit_s, exit_pc, r->s, r->pc);
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

/* The mapping a residency test reads: the live core's, except while A5 runs a C, whose own mapping (after its TAMs) it is. */
static unsigned res_mpr(unsigned i)
{
    return mode == MODE_A5 && sp ? mshadow[i] : chk_mpr(i);
}

static int tag_resident(const Tag *t)
{
    /* A card routine is in ROM: it is resident when its bank is mapped at its address. */
    if (t->rom) {
        for (unsigned i = 0; i < t->len; i++) {
            uint16_t logical = (uint16_t)(t->run + i);

            if (res_mpr(logical >> 13) != (t->disc + i) >> 13)
                return 0;
        }
        return 1;
    }
    for (unsigned i = 0; i < t->len; i++) {
        uint16_t logical = (uint16_t)(t->run + i);
        uint32_t p = canon(((uint32_t)res_mpr(logical >> 13) << 13) | (logical & 0x1FFF));
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
    if (ends_by_dispatch(c->entry_s, c->kind == K_IRQ, &rondo_tags[c->tag]) &&
        dispatch_confirmed(&c->span, c->entry_s, (uint16_t)((chk_bus_rd((uint16_t)(0x2100 | (uint8_t)(cpu.s + 1))) |
                                                            (chk_bus_rd((uint16_t)(0x2100 | (uint8_t)(cpu.s + 2))) << 8)) + 1),
                           rondo_tags[c->tag].name)) {
        c->dispatch = 1;
        stats[c->tag].dispatch++;
        span_finish_to_call(&c->span, (uint16_t)((chk_bus_rd((uint16_t)(0x2100 | (uint8_t)(cpu.s + 1))) |
                                                  (chk_bus_rd((uint16_t)(0x2100 | (uint8_t)(cpu.s + 2))) << 8)) + 1),
                            (uint8_t)(cpu.s + 2));
    } else {
        span_finish(&c->span);
    }
    in_swap = -1;
    c->done = 1;
    swapcontext(&c->uc, &main_uc);
}

static void hy_switch_in(Ctx *c)
{
    if (getenv("RONDO_DIAG_CTX"))
        fprintf(stderr, "CTX frame %lu switch in %s resume %04X S%02X\n", cur_frame, rondo_tags[c->tag].name, c->resume_pc,
                c->resume_s);
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
    Ev *close = c->span.end == SPAN_OPEN ? NULL : evp(c->span.end);     /* an end not found yet is not looked for: that read the rest of the stream */
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
    if (c->dispatch) {
        /* The RTS jumps to an address the routine pushed: the original's call goes on in the routines that address
         * begins, whose arrivals the hook will match; the call is judged up to here, and time is paced to the arrival. */
        Ev *next = evp(c->span.cur);

        owed = next && (next->type == EV_CALL || next->type == EV_OPEN) && next->clock
                   ? (int64_t)next->clock - (int64_t)hybrid_clock_now() : 1;
        /* The original took an interrupt at the dispatched address before its first instruction (span_finish_to_call
         * stopped there): held, and dispatched from the record when the core stands there, as after a hand-over. */
        if (next && next->type == EV_INT && !hold.active && hold_arm_from(c->span.cur, c->tag, 1, pc, ns)) {
            hold.i_orig = p & P_I;
            p |= P_I;
            dispatch_held++;
        }
        chk_set_regs(cpu.a, cpu.x, cpu.y, ns, p, pc);
        prev_op = 0x60;
        last_pc = 0xFFFF;
        scan = c->span.cur;
        return owed > 0 ? (int)(owed > 0x7FFFFFFF ? 0x7FFFFFFF : owed) : 1;
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
        if (owed < 0 && getenv("RONDO_DIAG_AHEAD"))
            fprintf(stderr, "AHEAD frame %lu call end of %s: the core is %lld clocks past the record's %llu\n", cur_frame, rondo_tags[c->tag].name,
                    (long long)-owed, (unsigned long long)close->clock);
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
        if (getenv("RONDO_DIAG_CTX"))
            fprintf(stderr, "CTX frame %lu abandoned %s\n", cur_frame, rondo_tags[c->tag].name);
        top = c->below;
        c->abandoned = 0;
        c->below = free_ctx;
        free_ctx = c;
        hy_trim();
        return hook_again();     /* the hook must see the first instruction of the code jumped to */
    }
    if (c && c->done) {
        if (getenv("RONDO_DIAG_CTX"))
            fprintf(stderr, "CTX frame %lu done %s\n", cur_frame, rondo_tags[c->tag].name);
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

/* RONDO_DIAG_DRIFT=FROM[,TO]: the frames whose swapped calls print their entry clock against the record's. */
static int drift_in(unsigned long frame)
{
    const char *v = getenv("RONDO_DIAG_DRIFT");
    char *end;
    unsigned long from = strtoul(v, &end, 10), to = *end == ',' ? strtoul(end + 1, NULL, 10) : (unsigned long)-1;

    return frame >= from && frame <= to;
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
    if (getenv("RONDO_DIAG_DRIFT") && drift_in(cur_frame))
        fprintf(stderr, "DRIFT frame %lu enter %s call %llu pc %04X S%02X: core clock %llu, the record's %llu (%+lld)\n", cur_frame,
                rondo_tags[index].name, (unsigned long long)stats[index].calls + 1, r->pc, r->s,
                (unsigned long long)hybrid_clock_now(), (unsigned long long)e->clock,
                (long long)hybrid_clock_now() - (long long)e->clock);
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
    c->done = c->parked = c->abandoned = c->dispatch = 0;
    c->fsn = 0;
    c->below = top;
    top = c;
    swapped++;
    stats[index].calls++;
    if (getenv("RONDO_DUMP_SPAN")) {
        uint64_t k = i;
        int nest = 0;
        Ev *x;

        while ((x = evp(k)) && k < i + 200000) {
            if (x->type == EV_OPEN)
                nest++;
            else if (x->type == EV_CLOSE && --nest == 0)
                break;
            k++;
        }
        dump_span(rondo_tags[index].name, stats[index].calls, i, k + 1);
    }
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
    void *host;          /* hybrid: the C on top when the call was made (parked, often waiting for the frame end) */
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
    if (mode == MODE_HYBRID && getenv("RONDO_DIAG_CTX"))
        fprintf(stderr, "UNITCALL frame %lu %04X: core PC %04X S%02X; top %s parked %d wait_frame %lu resume %04X S%02X\n", cur_frame,
                addr, r.pc, r.s, top ? rondo_tags[top->tag].name : "-", top ? top->parked : 0, top ? top->wait_frame : 0,
                top ? top->resume_pc : 0, top ? top->resume_s : 0);
    unit.saved = r;
    if (depth && (mode == MODE_A5 || mode == MODE_RECORD))
        marker(EV_UBEGIN, &r, 0, 0, 0);
    if (mode == MODE_HYBRID) {
        /* The registers the script leaves unnamed are the machine's at the call: the original's, which the record's
         * UBEGIN holds. The hybrid's core stands where a parked C left it (its registers at its last access, I masked
         * while it waits for the frame end), so those are taken from the record; the core's own are put back after. */
        Ev *x;

        for (uint64_t k = scan; k < scan + (1u << 16) && (x = evp(k)); k++)
            if (x->type == EV_UBEGIN) {
                r.a = x->a;
                r.x = x->x;
                r.y = x->y;
                r.p = x->p;
                if (x->s != r.s) {
                    int keep = blame_tag;

                    blame_tag = index;
                    defect(D_STACK, "at the unit call S is %02X, the original's %02X", r.s, x->s);
                    blame_tag = keep;
                }
                break;
            }
    }
    chk_bus_wr((uint16_t)(0x2100 | r.s), (uint8_t)((uint16_t)(UNIT_MARK - 1) >> 8));
    chk_bus_wr((uint16_t)(0x2100 | (uint8_t)(r.s - 1)), (uint8_t)(UNIT_MARK - 1));
    unit.s = (uint8_t)(r.s - 2);
    unit.host = mode == MODE_HYBRID ? (void *)top : NULL;
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
static uint64_t other_call, other_branch;   /* arrivals at a tagged address while no row there had its bytes resident */

static void count_arrival(int index, int judge_calls)
{
    Stat *s = &stats[index];
    int entry = prev_is_entry() || prev_op == PREV_IRQ;

    (void)judge_calls;
    if (self_jump)
        return;                 /* a JMP to the routine's own first byte: the idle loop, not an arrival */
    if (!tag_resident(&rondo_tags[index])) {
        /* tag_pick found no row at this address whose bytes are there: the code that ran is another overlay's, with
         * no row. It is no routine's arrival; it is counted on its own (OTHER CODE in the report). */
        if (entry)
            other_call++;
        else
            other_branch++;
        return;
    }
    if (entry)
        return;
    s->arrive_branch++;      /* a branch back into the routine's own first byte, its bytes resident */
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

/*
 * Is the machine between work: the innermost call the game has open is the kernel's idle loop (the tag at run 4174,
 * KernelIdle, drafted r0043_174), none of the game's routines beyond it? A script `call` is made there, so its accesses are
 * not counted into a routine the game has open. The record and A5 ask their own stack of open calls (frames[]); the
 * hybrid has no original calls open while a C is parked, so it asks the record's answer carried by the frame event of the
 * last frame end (the innermost open call then), which is the same frame boundary on both sides.
 */
int check_idle(void)
{
    int idle = tag_at[0x4174];

    if (idle < 0)
        return 1;            /* no idle loop known: do not hold the call back */
    if (mode == MODE_HYBRID)
        return fq_head == 0 || fq[fq_head - 1].top == idle + 1;
    return depth > 0 && frames[depth - 1].tag_index == idle;
}

/* The run address of a routine by name, or -1. */
/* A routine's name to its run address: a row's own, else a filed routine served by the draft at its address. */
int check_tag_run(const char *name)
{
    for (unsigned i = 0; i < rondo_tag_count; i++)
        if (!strcmp(rondo_tags[i].name, name))
            return rondo_tags[i].run;
    for (unsigned i = 0; i < rondo_alias_count; i++)
        if (!strcmp(rondo_aliases[i].name, name))
            return rondo_aliases[i].run;
    return -1;
}

/* ------------------------------------------------------------------------------------------ */
/* The hook before every instruction.                                                           */

int check_exec(uint32_t pc);
static int hook_again(void);

/* The interrupt the record has after a left hand-over, at the interrupted PC: compared with the core's registers and
 * dispatched from the record, as hy_int_pass does a C's (pace to its clock, the eight cycles, the pushes, the handler). */
static int hold_dispatch(const Regs *r)
{
    Ev ev = hold.ev;
    uint8_t mpr[8];
    uint8_t p = (uint8_t)((r->p & ~P_I) | hold.i_orig);

    hold.active = 0;
    blame_tag = hold.tag;
    if (r->a != ev.a || r->x != ev.x || r->y != ev.y || r->s != ev.s || ((p ^ ev.p) & PFLAGS))
        defect(D_REGS, "at the held interrupt at %04X the core has A%02X X%02X Y%02X S%02X P%02X, the original A%02X X%02X Y%02X S%02X P%02X",
               (unsigned)r->pc, r->a, r->x, r->y, r->s, p, ev.a, ev.x, ev.y, ev.s, ev.p);
    if (hybrid_clock_now() > ev.clock)
        stop_run(3, "the original's run-on after a left hand-over reached the interrupted PC %04X after the interrupt's clock", (unsigned)r->pc);
    pace_to(ev.clock);
    chk_charge(8 * (ev.speed ? 3 : 12));
    for (unsigned k = 0; k < 8; k++)
        mpr[k] = (uint8_t)chk_mpr(k);
    if (memcmp(mpr, ev.mpr, 8) || chk_get_speed() != ev.speed)
        defect(D_INT, "at the held interrupt the MPRs or the CPU clock differ from the original's");
    blame_tag = -1;
    chk_set_regs(r->a, r->x, r->y, r->s, p, r->pc);
    check_irq();             /* the bookkeeping of an interrupt the core took: where the handler returns to */
    chk_bus_wr((uint16_t)(0x2100 | ev.s), (uint8_t)(ev.pc >> 8));
    chk_bus_wr((uint16_t)(0x2100 | (uint8_t)(ev.s - 1)), (uint8_t)ev.pc);
    chk_bus_wr((uint16_t)(0x2100 | (uint8_t)(ev.s - 2)), (uint8_t)(ev.p & ~P_B));
    chk_set_regs(ev.a, ev.x, ev.y, (uint8_t)(ev.s - 3), (uint8_t)((ev.p | P_I) & ~(P_T | P_D)), hold.handler);
    prev_op = PREV_IRQ;
    last_pc = 0xFFFF;
    scan = hold.entry_idx + 1;
    held_ints++;
    return hook_again();
}

/* Each instruction the original runs under the hold: the hold ends at the record's interrupted PC; anything else that
 * could make the run-on differ from the record's stops the run. */
static int hold_step(const Regs *r)
{
    uint8_t op;

    if (r->pc == hold.ev.pc && r->s == hold.ev.s)
        return hold_dispatch(r);
    {
        unsigned seen = hold_acc;

        op = chk_bus_rd(r->pc);       /* this look at the opcode is a read the core's hook counts: not the original's */
        hold_acc = seen;
    }
    if (op == 0x08 || op == 0x28 || op == 0x40 || op == 0x58 || op == 0x78)
        stop_run(3, "an instruction that reads or sets P (%02X at %04X) ran under the interrupt hold", op, (unsigned)r->pc);
    if (hold_acc)
        stop_run(3, "the original's run-on after a left hand-over made a data access before the interrupted PC %04X, which the record does not have",
                 (unsigned)hold.ev.pc);
    if (++hold.n > 16 || hybrid_clock_now() > hold.ev.clock)
        stop_run(3, "the original's run-on after a left hand-over did not reach the interrupted PC %04X in time (%d instructions)",
                 (unsigned)hold.ev.pc, hold.n);
    return 0;
}

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
        /* The script's unit call runs on the machine as it stood at the frame boundary: a C that was parked there (the
         * game's own call, open in the record around the unit call's block) stays parked until the call returns to
         * its mark and the machine is put back; it is not resumed by the frame end or a PC the unit's code passes. */
        int unit_over_host = unit.active && top && (void *)top == unit.host;

        if (hold.active)
            return hold_step(&r);
        if (!unit_over_host && top && top->parked && top->wait_frame) {
            if (cur_frame < top->wait_frame)
                return 1;                    /* the frame has not ended: the core goes on, running nothing */
            top->wait_frame = 0;
            hy_switch_in(top);
            return hy_after();
        }
        if (getenv("RONDO_DIAG_RESUME") && top && top->parked && pc == top->resume_pc)
            fprintf(stderr, "DIAGRESUME frame %lu %s pc %04X S%02X resume_s %02X prev_op %X wait_frame %lu\n", cur_frame,
                    rondo_tags[top->tag].name, (unsigned)pc, r.s, top->resume_s, prev_op, top->wait_frame);
        if (!unit_over_host && top && top->parked && (prev_op == 0x60 || prev_op == 0x40) && pc == top->resume_pc &&
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
        if (depth)
            marker(EV_UEND, &r, 0, 0, 0);
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
    if (getenv("RONDO_FRAME_TRACE") && frame >= strtoul(getenv("RONDO_FRAME_TRACE"), NULL, 10) && mode != MODE_A5) {
        fprintf(stderr, "FT %lu %s PC %04X A%02X X%02X Y%02X S%02X P%02X clock %llu:", frame,
                mode == MODE_RECORD ? "rec" : "hyb", pc, a, x, y, s, p, (unsigned long long)chk_ts_abs());
        if (mode == MODE_RECORD) {
            for (int i = 0; i < depth; i++)
                fprintf(stderr, " %s", frames[i].tag->name);
        } else {
            for (Ctx *c = top; c; c = c->below)
                fprintf(stderr, " %s%s", rondo_tags[c->tag].name, c->parked ? "(p)" : "");
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
        e.pc = pc;
        e.s = s;
        e.ret = (uint16_t)(depth ? frames[depth - 1].tag_index + 1 : 0);      /* the innermost open call, plus one: 0 none */
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
        if (getenv("RONDO_DIAG_WIN")) {
            const Ctx *low = NULL;
            int nctx = 0;

            for (Ctx *c = top; c; c = c->below) {
                nctx++;
                if (!low || c->span.cur < low->span.cur)
                    low = c;
            }
            fprintf(stderr, "WIN frame %lu base %llu n %llu scan %llu ctxs %d lowest %s cur %llu start %llu end %llu parked %d\n", frame,
                    (unsigned long long)wbase, (unsigned long long)wn, (unsigned long long)scan, nctx,
                    low ? rondo_tags[low->tag].name : "-", low ? (unsigned long long)low->span.cur : 0ull,
                    low ? (unsigned long long)low->span.start : 0ull, low ? (unsigned long long)low->span.end : 0ull, low ? low->parked : 0);
        }
        /* The record's hashes for this frame: read ahead until they are there. */
        while (fq_head == fq_tail && fetch_one())
            ;
        if (fq_head < fq_tail) {
            hy_frames_total++;
            if (fq[fq_head].ram != hash || fq[fq_head].pic != picture) {
                if (hy_first_bad == (unsigned long)-1) {
                    hy_first_bad = frame;
                    fprintf(report, "first differing frame %lu: RAM %s, picture %s; the core at PC %04X S%02X clock %llu%s, the original at PC %04X S%02X clock %llu\n",
                            frame, fq[fq_head].ram != hash ? "differs" : "same", fq[fq_head].pic != picture ? "differs" : "same",
                            (unsigned)pc, s, (unsigned long long)hybrid_clock_now(), top && top->parked ? " (a C is parked: the registers are the hybrid's)" : "",
                            (unsigned)fq[fq_head].pc, fq[fq_head].s, (unsigned long long)fq[fq_head].when);
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
            /* Causes: left, log, depth, computed, arrive_branch, arrive_nonres, runaway, prefix, noreturn, dispatch */
            fprintf(report, " %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n", (unsigned long long)s->unv_left,
                    (unsigned long long)s->unv_log, (unsigned long long)s->unv_depth,
                    (unsigned long long)s->unv_computed, (unsigned long long)s->arrive_branch,
                    (unsigned long long)s->arrive_nonres, (unsigned long long)s->runaway,
                    (unsigned long long)s->prefix, (unsigned long long)s->noreturn, (unsigned long long)s->dispatch);
        }
        fprintf(report, "OTHER CODE at tagged addresses (no row's bytes resident, charged to no routine): %llu by call, %llu by branch\n",
                (unsigned long long)other_call, (unsigned long long)other_branch);
        fprintf(report, "RTS below the entry S that the original did not make (judged whole, not as a dispatch): %llu calls\n",
                (unsigned long long)not_dispatched);
        if (probe_calls)
            fprintf(report, "PROBE resident_at: %llu calls, %llu differ, variants seen %u and %u\n",
                    (unsigned long long)probe_calls, (unsigned long long)probe_bad, probe_seen[0], probe_seen[1]);
        fclose(report);
    } else if (mode == MODE_RECORD) {
        Ev e;
        uint32_t left = (uint32_t)depth;

        while (depth) {
            Regs z = { 0 };

            close_event(&frames[depth - 1], &z, CL_END);
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
            /* A C still running (or parked) at the end has compared the record's events its span passed: the calls
             * opened there were reached, through the C. */
            uint64_t reached = scan;

            for (Ctx *c = top; c; c = c->below)
                if (c->span.cur > reached)
                    reached = c->span.cur;
            for (uint64_t k = reached > wbase ? reached : wbase; k < wbase + wn; k++)
                if (win[k - wbase].type == EV_OPEN) {
                    unreached++;
                    if (getenv("RONDO_DIAG_CTX")) {
                        Ev *u = &win[k - wbase];
                        fprintf(stderr, "UNREACHED %llu (scan %llu, stream end %llu): OPEN %s pc %04X s %02X clock %llu\n",
                                (unsigned long long)k, (unsigned long long)scan, (unsigned long long)(wbase + wn),
                                u->addr < rondo_tag_count ? rondo_tags[u->addr].name : "?", u->pc, u->s,
                                (unsigned long long)u->clock);
                    }
                }
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
        if (held_ints || held_not)
            fprintf(report, "left hand-overs: %llu with the interrupt that followed held and dispatched from the record, %llu with none\n",
                    (unsigned long long)held_ints, (unsigned long long)held_not);
        fprintf(report, "interrupts at the return from original code delivered from the record (the core came back before they were due): %llu\n",
                (unsigned long long)retpoint_ints);
        fprintf(report, "frame ends at the return from original code passed by the C (the core came back before the frame ended): %llu\n",
                (unsigned long long)retpoint_frames);
        fprintf(report, "interrupts at the address an RTS dispatch goes to, held for the core's arrival there: %llu\n",
                (unsigned long long)dispatch_held);
        fprintf(report, "interrupts before the instruction a C handed over at, delivered by the C: %llu\n",
                (unsigned long long)handover_ints);
        fprintf(report, "frame ends at a hand-over to the original passed by the C (the original's frame ended where the C stood): %llu\n",
                (unsigned long long)handover_frames);
        if (horizon_calls)
            fprintf(report, "horizon hand-overs %llu: calls whose return was not within %u events, the original ran on\n",
                    (unsigned long long)horizon_calls, (unsigned)HORIZON);
        fprintf(report, "OTHER CODE at tagged addresses (no row's bytes resident, charged to no routine): %llu by call, %llu by branch\n",
                (unsigned long long)other_call, (unsigned long long)other_branch);
        fprintf(report, "RTS below the entry S that the original did not make (judged whole, not as a dispatch): %llu calls\n",
                (unsigned long long)not_dispatched);
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
            if (getenv("RONDO_DIAG_CTX") && (stats[i].arrive_branch || stats[i].arrive_nonres || stats[i].unv_computed))
                fprintf(stderr, "ARRIVE %s branch %llu nonres %llu computed %llu\n", rondo_tags[i].name,
                        (unsigned long long)stats[i].arrive_branch, (unsigned long long)stats[i].arrive_nonres,
                        (unsigned long long)stats[i].unv_computed);
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
