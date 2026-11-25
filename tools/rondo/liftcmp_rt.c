/*
 * liftcmp_rt.c -- the machine as tools/rondo/liftcmp.py runs a routine against it.
 *
 * Linked with one routine's entry (ENTRY, a function of no arguments entered with the register file `cpu`) and the
 * game's other C. It stands in for the bus and the chip: every rd8, wr8, push and pull, the VDC port instructions,
 * the MPR instructions and the block move are logged in order with their address and value; a call the routine makes
 * to another routine is dispatched to that routine's C when there is any (DISPATCH, written by liftcmp.py from the
 * routine table) and otherwise stubbed with seeded registers. The same file serves both sides of a comparison, so
 * the two see the same machine.
 *
 * Memory is a 64 KB image, empty at the start of each case: a byte not yet written reads as a value fixed by the
 * case's seed and the address, and a byte written reads back what was written; hardware ports (the addresses mapped to
 * the I/O bank) read a value that also depends on how many times they have been read. See rd8.
 *
 * Input: the cases file named by argv[1], one case a line:
 *     ID SEED A X Y S P M2 M3 M4 M5 M6 N [ADDR=VAL ...]
 * (hex): the registers, the banks in MPR2 to MPR6 (MPR0 is $FF, MPR1 $F8, MPR7 $00), N memory overrides. Output on stdout, per case:
 *     CASE ID, then one line per event, then END with the registers, or UNTRACED ADDR when a draft reached bytes no
 *     script ran. When argv[2] is given, the coverage of the case is appended to that file as a line
 *     `ID PC PC ...` (return addresses of the compiler's block probes).
 */
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "machine.h"

Cpu cpu;
void ENTRY(void);

typedef struct { uint16_t run; void (*abi)(void); } Dispatch;
extern const Dispatch dispatch_table[];
extern const unsigned dispatch_count;
/* what the mapper has at an address: with MPR2 to MPR6 as given, the routine whose entry (a disc location) is there */
typedef struct { uint8_t m[5]; uint16_t run; uint32_t disc; } Resident;
extern const Resident resident_tab[];
extern const unsigned resident_count;
typedef struct { uint32_t disc; void (*abi)(void); } DiscDispatch;
extern const DiscDispatch disc_table[];
extern const unsigned disc_count;
/* the draft side only: pieces of code with no C of their own (the middle of another routine, a tail a draft jumps
 * into) that the draft runs on into where the lifted C is one function */
typedef struct { uint32_t disc; uint16_t run; void (*abi)(void); } PieceDispatch;
extern const PieceDispatch piece_table[];
extern const unsigned piece_count;

static uint8_t mem[65536];
static uint8_t written[65536];
static uint8_t reads[65536];
static uint64_t seed;
static unsigned stub_calls[65536];
static jmp_buf case_end;
static unsigned events;
static int decimal_ok;
static uint8_t *card_rom; /* the System Card's 256 KB, when the harness has it: banks $00 to $1F read as its bytes */
static uint8_t mpr[8]; /* the mapper: MPR0 to MPR7, as the case sets them; tam writes them, tma reads them */
static int depth;

static uint64_t mix64(uint64_t x)
{
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

static uint8_t seeded(uint64_t a, uint64_t b, uint64_t c)
{
    uint64_t h = mix64(seed ^ mix64(a * 0x9E3779B97F4A7C15ULL + b * 0xD6E8FEB86659FD93ULL + c));

    /* three flavours by seed: any byte, a byte from the edges that decide branches, a small one */
    switch (seed % 3) {
    case 0:
        return (uint8_t)h;
    case 1: {
        static const uint8_t edge[] = { 0x00, 0x01, 0x7F, 0x80, 0xFF, 0xFE, 0x0F, 0xF0 };
        return edge[h % sizeof edge];
    }
    default:
        return (uint8_t)(h & 7);
    }
}

void rt_event(void)
{
    if (++events > 20000) {
        printf("HUNG\n");
        longjmp(case_end, 1);
    }
}

/*
 * Memory and hardware, split as the checker splits them (check.c): an address whose MPR holds the I/O bank ($FF) is
 * hardware, the rest (work RAM, the stack page, the banks of RAM and ROM) is memory.
 * Memory reads back the same byte until a write changes it: the value of an unwritten byte is fixed by the case's
 * seed and the address alone, and a byte written reads back what was written, each side seeing its own writes.
 * Hardware reads can differ from one read to the next (a status port), so an unwritten port reads a value that also
 * depends on how many times it has been read, and a write to a port is never read back.
 */
static int is_hardware(uint16_t addr) { return mpr[addr >> 13] == 0xFF; }

/* An instruction byte: the value rd8 would give, with no event and no change to the read counts. */
uint8_t code8(uint16_t addr)
{
    if (is_hardware(addr))
        return seeded(addr, reads[addr], 1);
    if (written[addr])
        return mem[addr];
    if (card_rom && mpr[addr >> 13] < 0x20)
        return card_rom[((unsigned)mpr[addr >> 13] << 13) | (addr & 0x1FFF)];
    return seeded(addr, 0, 1);
}

uint8_t rd8(uint16_t addr)
{
    uint8_t v;

    rt_event();
    if (is_hardware(addr)) {
        v = seeded(addr, reads[addr], 1);
        if (reads[addr] != 255)
            reads[addr]++;
    } else if (written[addr]) {
        v = mem[addr];
    } else if (card_rom && mpr[addr >> 13] < 0x20) {
        v = card_rom[((unsigned)mpr[addr >> 13] << 13) | (addr & 0x1FFF)];
    } else {
        v = seeded(addr, 0, 1);
    }
    printf("R %04X %02X\n", addr, v);
    return v;
}

void wr8(uint16_t addr, uint8_t value)
{
    rt_event();
    mem[addr] = value;
    written[addr] = 1;
    printf("W %04X %02X\n", addr, value);
}

void st0(uint8_t v) { printf("ST0 %02X\n", v); }
void st1(uint8_t v) { printf("ST1 %02X\n", v); }
void st2(uint8_t v) { printf("ST2 %02X\n", v); }
/*
 * What the checker judges of the mapper and the clock (check.c): one event, "MPR or clock change", placed when the MPRs
 * or the speed are not what they were; a TAM that maps what is mapped, a CSL or CSH that changes nothing, a TMA (a
 * read of the mapper) and a CLI or SEI (a flag of P, not an access) are no event on either side. The log says the same.
 */
static uint8_t speed = 1;

static void state_line(void)
{
    printf("STATE %02X %02X %02X %02X %02X %02X %02X %02X speed %u\n", mpr[0], mpr[1], mpr[2], mpr[3], mpr[4], mpr[5],
           mpr[6], mpr[7], speed);
}

void tam(uint8_t mask, uint8_t bank)
{
    int changed = 0;

    for (int i = 0; i < 8; i++)
        if ((mask & (1 << i)) && mpr[i] != bank) {
            mpr[i] = bank;
            changed = 1;
        }
    if (changed)
        state_line();
}

/* TMA with several bits set gives the highest set bit's MPR, as the core does */
uint8_t tma(uint8_t mask)
{
    uint8_t v = 0;

    for (int i = 0; i < 8; i++)
        if (mask & (1 << i))
            v = mpr[i];
    return v;
}

void cli(void) {}
void csl(void)
{
    if (speed != 0) {
        speed = 0;
        state_line();
    }
}

void csh(void)
{
    if (speed != 1) {
        speed = 1;
        state_line();
    }
}
static void end_line(void)
{
    printf("END A=%02X X=%02X Y=%02X S=%02X P=%02X\n", cpu.a, cpu.x, cpu.y, cpu.s, cpu.p);
}

/* A wait with no data access: the routine is judged up to it, as A5 does; the case ends here. */
void wait_interrupt(void)
{
    printf("WAIT\n");
    end_line();
    longjmp(case_end, 1);
}

/* A seeded byte can drive C or a draft out of range where the game never goes (an index past a table of handlers):
 * the case ends there and is counted apart. */
static void crashed(int sig)
{
    (void)sig;
    printf("CRASHED\n");
    longjmp(case_end, 1);
}

static void hung(int sig)
{
    (void)sig;
    printf("HUNG\n");
    longjmp(case_end, 1);
}

/* TAI as the chip does it: push Y, A, X; the bytes; pull X, A, Y. The count is the caller's. */
void tai(uint16_t src, uint16_t dst, uint16_t len)
{
    push8(cpu.y);
    push8(cpu.a);
    push8(cpu.x);
    for (unsigned i = 0; i < len; i++) {
        uint8_t b = rd8((uint16_t)(src + (i & 1)));

        wr8((uint16_t)(dst + i), b);
    }
    cpu.x = pull8();
    cpu.a = pull8();
    cpu.y = pull8();
}

/* The disc location of the routine resident at a run address under the present mapper, or 0xFFFFFFFF. */
static uint32_t resident_disc(uint16_t target)
{
    for (unsigned i = 0; i < resident_count; i++)
        if (resident_tab[i].run == target && memcmp(resident_tab[i].m, mpr + 2, 5) == 0)
            return resident_tab[i].disc;
    return 0xFFFFFFFFu;
}

/*
 * resident_at: the machine layer's question, not an access. The disc location (DISC(sector, offset) when machine.h has
 * that macro) of the routine with C that is resident at the run address, 0 when none; answered from the case's mapper
 * and the trace's run map, the same way the dispatch of a call finds its callee, so both sides see the same piece.
 */
uint32_t resident_at(uint16_t run_address)
{
    uint32_t d = resident_disc(run_address);
    uint32_t r = 0;

    if (d != 0xFFFFFFFFu)
        for (unsigned i = 0; i < disc_count; i++)
            if (disc_table[i].disc == d) {
#ifdef DISC
                r = DISC(d / 2048, d % 2048);
#else
                r = ((d / 2048) << 16) | (d % 2048);
#endif
                break;
            }
    printf("#RES %04X %u\n", run_address, (unsigned)(r ? d : 0));
    return r;
}

static void (*find_abi(uint16_t target))(void)
{
    uint32_t d = resident_disc(target);

    if (d != 0xFFFFFFFFu) {
        /* the case's mapper says what is there. The draft side: the draft of that disc place as it runs at this address
         * (the same bytes often run at several addresses, and a draft's baked addresses are of one of them); then the
         * routine with C; else code we have no C or draft for (a stub), never another overlay's routine at the same
         * run address */
        for (unsigned i = 0; i < piece_count; i++)
            if (piece_table[i].disc == d && piece_table[i].run == target)
                return piece_table[i].abi;
        for (unsigned i = 0; i < disc_count; i++)
            if (disc_table[i].disc == d)
                return disc_table[i].abi;
        return 0;
    }
    for (unsigned i = 0; i < dispatch_count; i++)
        if (dispatch_table[i].run == target)
            return dispatch_table[i].abi;
    return 0;
}

/* Code with no C: its registers on return are the case's seeded ones, nothing else changes. */
static void stub_routine(uint16_t target)
{
    rt_event();
    unsigned n = stub_calls[target]++;

    cpu.a = seeded(target, n, 11);
    cpu.x = seeded(target, n, 12);
    cpu.y = seeded(target, n, 13);
    cpu.p = (uint8_t)(seeded(target, n, 14) & (P_N | P_V | P_Z | P_C | (decimal_ok ? P_D : 0)));
    printf("STUB %04X -> A=%02X X=%02X Y=%02X P=%02X\n", target, cpu.a, cpu.x, cpu.y, cpu.p);
}

/* @ends: the routine's tag says where it hands on (its tail jump out of the routine into code another routine owns).
 * The lifted C stops with the registers it holds there; the draft is stopped at the same place, so the two are judged
 * up to the jump and by A, X, Y, S and P at it (the END line). */
static int ends_addr = -1;

static void enter(uint16_t target, int tail)
{
    void (*abi)(void) = find_abi(target);

    if (tail && depth == 0 && (int)target == ends_addr)
        return;

    if (abi && depth >= 512) {
        /* the checker allows nesting to 512: a chain longer than that, or a runaway, ends the case */
        printf("DEPTHLIMIT\n");
        longjmp(case_end, 1);
    }
    if (abi) {
        /* C in the tree: it runs, on both sides, and its accesses are in the log as they are made */
        printf("#IN %04X\n", target);
        depth++;
        abi();
        depth--;
        printf("#OUT %04X\n", target);
    } else {
        printf("%s %04X A=%02X X=%02X Y=%02X P=%02X S=%02X\n", tail ? "TAIL" : "CALL", target, cpu.a, cpu.x, cpu.y, cpu.p, cpu.s);
        stub_routine(target);
    }
}

/* A tail jump: no frame. */
void call_original(uint16_t target)
{
    enter(target, 1);
}

/*
 * leave_original(addr): the draft set S outright (a TXS), so the call's frame is gone and what follows is not the call's.
 * As in A5 (check.c) the C returns right after it and the call is judged up to here: nothing to do in the runtime.
 */
void leave_original(uint16_t addr) { (void)addr; }

/* The draft sets this before each instruction (the pc of the instruction it stands for); the runtime ignores it. */
uint16_t rondo_pc;

/* A tail jump that says where it came from (the checker's entry test uses the origin; the comparison does not). */
void call_original_at(uint16_t from, uint16_t addr)
{
    (void)from;
    enter(addr, 1);
}

/* JSR: the return address less one is pushed, the routine runs, the frame is popped. */
void call_orig(uint16_t ret, uint16_t target)
{
    uint8_t expect;

    push8((uint8_t)((uint16_t)(ret - 1) >> 8));
    push8((uint8_t)(ret - 1));
    expect = cpu.s;
    enter(target, 0);
    if (cpu.s != expect) {
        printf("FRAME DROPPED\n");
        return;
    }
    cpu.s = (uint8_t)(cpu.s + 2);
}

void untraced(uint16_t addr)
{
    printf("UNTRACED %04X\n", addr);
    longjmp(case_end, 1);
}

void hand_over(uint16_t addr) { (void)addr; }

/* ---- coverage: the compiler's block probes land here ---- */

#define COVBITS 16
static uintptr_t cov_tab[1u << COVBITS];
static uintptr_t cov[1u << (COVBITS - 1)];
static unsigned cov_n;

void __sanitizer_cov_trace_pc(void)
{
    uintptr_t pc = (uintptr_t)__builtin_return_address(0);
    unsigned h = (unsigned)mix64(pc) & ((1u << COVBITS) - 1);

    while (cov_tab[h]) {
        if (cov_tab[h] == pc)
            return;
        h = (h + 1) & ((1u << COVBITS) - 1);
    }
    if (cov_n < (1u << (COVBITS - 1))) {
        cov_tab[h] = pc;
        cov[cov_n++] = pc;
    }
}

int main(int argc, char **argv)
{
    FILE *in, *cf = 0;
    char line[4096];

    if (argc < 2)
        return 2;
    if (getenv("LIFTCMP_CARDROM")) {
        FILE *rf = fopen(getenv("LIFTCMP_CARDROM"), "rb");

        if (rf) {
            card_rom = malloc(262144);
            if (card_rom && fread(card_rom, 1, 262144, rf) != 262144) {
                free(card_rom);
                card_rom = 0;
            }
            fclose(rf);
        }
    }
    if (getenv("LIFTCMP_ENDS") && getenv("LIFTCMP_ENDS")[0])
        ends_addr = (int)strtol(getenv("LIFTCMP_ENDS"), 0, 16);
    decimal_ok = getenv("LIFTCMP_DECIMAL") && getenv("LIFTCMP_DECIMAL")[0] == '1';
    in = fopen(argv[1], "r");
    if (!in)
        return 2;
    if (argc > 2)
        cf = fopen(argv[2], "w");
    setvbuf(stdout, 0, _IOFBF, 1 << 16);
    while (fgets(line, sizeof line, in)) {
        unsigned id, a, x, y, s, p, n, m[5];
        unsigned long long sd;
        int off = 0, used;
        char *q = line;

        if (sscanf(q, "%x %llx %x %x %x %x %x %x %x %x %x %x %u%n", &id, &sd, &a, &x, &y, &s, &p, &m[0], &m[1], &m[2], &m[3], &m[4], &n, &used) < 13)
            continue;
        q += used;
        memset(mem, 0, sizeof mem);
        memset(written, 0, sizeof written);
        memset(reads, 0, sizeof reads);
        memset(stub_calls, 0, sizeof stub_calls);
        memset(cov_tab, 0, sizeof cov_tab);
        cov_n = 0;
        seed = sd;
        depth = 0;
        events = 0;
        for (unsigned i = 0; i < n; i++) {
            unsigned ad, val;

            if (sscanf(q, " %x=%x%n", &ad, &val, &off) == 2) {
                mem[ad & 0xFFFF] = (uint8_t)val;
                written[ad & 0xFFFF] = 1;
                q += off;
            }
        }
        mpr[0] = 0xFF;
        mpr[1] = 0xF8;
        for (int i = 0; i < 5; i++)
            mpr[2 + i] = (uint8_t)m[i];
        mpr[7] = 0x00;
        speed = 1;
        cpu.a = (uint8_t)a;
        cpu.x = (uint8_t)x;
        cpu.y = (uint8_t)y;
        cpu.s = (uint8_t)s;
        cpu.p = (uint8_t)p;
        printf("CASE %X\n", id);
        if (setjmp(case_end) == 0) {
            struct itimerval t = { { 0, 0 }, { 0, 150000 } };

            signal(SIGALRM, hung);
            {
                static char altstack[1 << 16];
                stack_t st = { altstack, 0, sizeof altstack };
                struct sigaction sa;

                sigaltstack(&st, 0);
                memset(&sa, 0, sizeof sa);
                sa.sa_handler = crashed;
                sa.sa_flags = SA_ONSTACK | SA_NODEFER;
                sigaction(SIGSEGV, &sa, 0);
                sigaction(SIGBUS, &sa, 0);
                sigaction(SIGFPE, &sa, 0);
            }
            setitimer(ITIMER_REAL, &t, 0);
            ENTRY();
            end_line();
        }
        {
            struct itimerval off = { { 0, 0 }, { 0, 0 } };

            setitimer(ITIMER_REAL, &off, 0);
        }
        if (cf) {
            fprintf(cf, "%X", id);
            for (unsigned i = 0; i < cov_n; i++)
                fprintf(cf, " %lx", (unsigned long)cov[i]);
            fprintf(cf, "\n");
        }
    }
    fflush(stdout);
    return 0;
}
