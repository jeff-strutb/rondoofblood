/*
 * trace.c: what the game executes and reads, folded onto the disc.
 *
 * The game runs from RAM it filled from the disc, and overlays reuse that
 * RAM, so a run address does not say which disc byte ran. Instead every
 * RAM byte carries a provenance: the disc byte it is a copy of, or none.
 * A byte the CPU reads from the CD data bus carries the disc position the
 * drive read it from (core patch 0003); loads, stores, pushes, pulls,
 * register transfers and block moves carry provenance along (opclass.h,
 * generated from huc.py); any other write clears it. An executed byte is
 * credited to the disc byte its provenance names.
 *
 * Maps written at the end (one byte of flags per byte, 1 opcode,
 * 2 operand, 4 data read):
 *   discmap.bin   track 02's user data, 20,785,152 B
 *   cardmap.bin   the System Card, 262,144 B
 *   noprov.bin    the 2 MB physical space: RAM executed with no provenance
 *
 * And edges.csv: every distinct transfer of control the catalog starts routines at,
 * each end where it ran: kind (the opcode: 20 JSR, 44 BSR, 4C JMP, 6C JMP (abs),
 * 7C JMP (abs,X); 100 an interrupt's entry, from the instruction it interrupted),
 * then from and to, each a space (D the disc's track 02 user data, C the System Card,
 * R RAM with no disc origin, or I/O) and an offset in it.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opclass.h"

#define NONE 0xFFFFFFFFu
#define PHYS (1u << 21)
#define USER_LBA 3890u                    /* track 02's first user sector */
#define USER_BYTES (10149u * 2048u)
#define F_OPC 1
#define F_OPD 2
#define F_DATA 4

static uint32_t *prov;                    /* per physical byte: absolute disc byte, or NONE */
static uint8_t *discmap, *cardmap, *noprov;
static uint8_t *user;                     /* track 02's user data, for the invariant */
static uint32_t pa = NONE, px = NONE, py = NONE;
static unsigned cls;
static uint32_t last_read = NONE, block_read = NONE;
static int plant;
static uint64_t outside;                  /* provenance outside track 02's user data */
static uint32_t (*cd_origin)(void);

/* Edges: a set of (kind, from, to), each end a location: 2 bits of space, 25 of offset. */
#define EDGE_BITS 21
#define EDGE_SLOTS (1u << EDGE_BITS)
#define EDGE_EMPTY 0xFFFFFFFFFFFFFFFFull
#define SP_DISC 0u
#define SP_CARD 1u
#define SP_RAM 2u
#define SP_IO 3u
static uint64_t *edges;
static uint64_t nedges, edges_lost;
static uint32_t prev_loc = NONE;          /* the last instruction's opcode, as a location */
static uint8_t prev_opc;
static int irq_pending;

/* --watch: which instructions read or write these physical bytes. */
#define WATCH_MAX 8
#define SEEN_MAX 256
static uint32_t watch[WATCH_MAX];
static int nwatch;
static uint32_t op_phys = NONE;           /* the current instruction's opcode */
static struct { uint32_t addr, op, origin; char rw; } seen[SEEN_MAX];
static int nseen;

static void watched(uint32_t p, char rw)
{
    for (int i = 0; i < nwatch; i++) {
        if (watch[i] != p)
            continue;
        for (int k = 0; k < nseen; k++)
            if (seen[k].addr == p && seen[k].op == op_phys && seen[k].rw == rw)
                return;
        if (nseen < SEEN_MAX) {
            seen[nseen].addr = p;
            seen[nseen].op = op_phys;
            seen[nseen].origin = op_phys == NONE ? NONE : prov[op_phys];
            seen[nseen++].rw = rw;
        }
        return;
    }
}

void trace_watch(uint32_t phys)
{
    if (nwatch < WATCH_MAX)
        watch[nwatch++] = phys;
}

static int is_ram(uint32_t bank)
{
    return (bank >= 0x68 && bank <= 0x87) || bank == 0xF8;
}

static uint32_t canon(uint32_t phys)
{
    uint32_t bank = phys >> 13;
    if (bank >= 0xF9 && bank <= 0xFB)     /* base RAM's mirrors */
        return (0xF8u << 13) | (phys & 0x1FFF);
    return phys;
}

static void credit(uint32_t origin, uint8_t flag)
{
    uint32_t lo = USER_LBA * 2048u;
    if (origin == NONE)
        return;
    if (origin >= lo && origin - lo < USER_BYTES)
        discmap[origin - lo] |= flag;
    else
        outside++;
}

static void finish(void)
{
    uint32_t t;
    switch (cls) {
    case OC_LDA: case OC_PLA: pa = last_read; break;
    case OC_LDX: case OC_PLX: px = last_read; break;
    case OC_LDY: case OC_PLY: py = last_read; break;
    case OC_TAX: px = pa; break;
    case OC_TXA: pa = px; break;
    case OC_TAY: py = pa; break;
    case OC_TYA: pa = py; break;
    case OC_SAX: t = pa; pa = px; px = t; break;
    case OC_SAY: t = pa; pa = py; py = t; break;
    case OC_SXY: t = px; px = py; py = t; break;
    case OC_A_NONE: pa = NONE; break;
    case OC_X_NONE: px = NONE; break;
    case OC_Y_NONE: py = NONE; break;
    }
    cls = OC_OTHER;
}

static void exec(uint32_t p, uint8_t flag)
{
    uint32_t bank = p >> 13;
    if (bank < 0x20) {
        cardmap[p] |= flag;
    } else if (is_ram(bank)) {
        if (prov[p] != NONE)
            credit(prov[p], flag);
        else
            noprov[p] |= flag;
    }
}

/* Where the byte at physical p came from, as a location: disc, card, RAM with no origin, I/O. */
static uint32_t loc_of(uint32_t p)
{
    uint32_t bank = p >> 13, lo = USER_LBA * 2048u;
    if (bank < 0x20)
        return SP_CARD << 25 | p;
    if (is_ram(bank)) {
        uint32_t o = prov[p];
        if (o != NONE && o >= lo && o - lo < USER_BYTES)
            return SP_DISC << 25 | (o - lo);
        return SP_RAM << 25 | p;
    }
    return SP_IO << 25 | p;
}

static void edge(uint32_t kind, uint32_t from, uint32_t to)
{
    uint64_t key = (uint64_t)kind << 54 | (uint64_t)from << 27 | to;
    uint64_t h = key * 0x9E3779B97F4A7C15ull;
    for (uint32_t i = (uint32_t)(h >> (64 - EDGE_BITS)), n = 0; n < EDGE_SLOTS; i = (i + 1) & (EDGE_SLOTS - 1), n++) {
        if (edges[i] == key)
            return;
        if (edges[i] == EDGE_EMPTY) {
            if (nedges * 2 >= EDGE_SLOTS) {   /* half full: count what is lost, never fail quietly */
                edges_lost++;
                return;
            }
            edges[i] = key;
            nedges++;
            return;
        }
    }
}

static uint32_t origin_of(uint32_t p)
{
    uint32_t bank = p >> 13, off = p & 0x1FFF;
    if (is_ram(bank))
        return prov[p];
    if (bank == 0xFF && (off & 0x1C00) == 0x1800 && (off & 0x1E00) != 0x1A00 &&
        (off & 0x18C0) != 0x18C0 && ((off & 0xF) == 1 || (off & 0xF) == 8)) {
        uint32_t o = cd_origin();
        return o == NONE ? NONE : o + (uint32_t)plant;
    }
    return NONE;
}

void trace_rd(uint32_t phys, uint8_t v, int kind)
{
    uint32_t p = canon(phys);
    if (kind == 2) {
        finish();
        cls = opclass[v];
        last_read = block_read = NONE;
        op_phys = p;
        exec(p, F_OPC);
        uint32_t here = loc_of(p);
        if (irq_pending && prev_loc != NONE)
            edge(0x100, prev_loc, here);
        else if (!irq_pending && prev_loc != NONE && (prev_opc == 0x20 || prev_opc == 0x44 || prev_opc == 0x4C ||
                                                      prev_opc == 0x6C || prev_opc == 0x7C))
            edge(prev_opc, prev_loc, here);
        irq_pending = 0;
        prev_loc = here;
        prev_opc = v;
    } else if (kind == 1) {
        exec(p, F_OPD);
    } else {
        uint32_t o = origin_of(p);
        last_read = block_read = o;
        if (nwatch)
            watched(p, 'r');
        if (p >> 13 < 0x20)
            cardmap[p] |= F_DATA;
        else
            credit(o, F_DATA);
    }
}

void trace_wr(uint32_t phys, uint8_t v)
{
    uint32_t p = canon(phys);
    (void)v;
    if (nwatch)
        watched(p, 'w');
    if (!is_ram(p >> 13))
        return;
    switch (cls) {
    case OC_STA: case OC_PHA: prov[p] = pa; break;
    case OC_STX: case OC_PHX: prov[p] = px; break;
    case OC_STY: case OC_PHY: prov[p] = py; break;
    case OC_BLOCK: prov[p] = block_read; break;
    default: prov[p] = NONE; break;
    }
}

void trace_irq(void)
{
    irq_pending = 1;
    finish();
    last_read = block_read = NONE;
}

int trace_init(const char *user_path, int plant_offset, uint32_t (*origin)(void))
{
    FILE *f = fopen(user_path, "rb");
    prov = malloc(PHYS * sizeof *prov);
    discmap = calloc(USER_BYTES, 1);
    cardmap = calloc(1u << 18, 1);
    noprov = calloc(PHYS, 1);
    user = malloc(USER_BYTES);
    edges = malloc(EDGE_SLOTS * sizeof *edges);
    if (!f || !prov || !discmap || !cardmap || !noprov || !user || !edges ||
        fread(user, 1, USER_BYTES, f) != USER_BYTES) {
        fprintf(stderr, "pcebox: cannot load %s\n", user_path);
        return -1;
    }
    fclose(f);
    memset(prov, 0xFF, PHYS * sizeof *prov);
    memset(edges, 0xFF, EDGE_SLOTS * sizeof *edges);
    plant = plant_offset;
    cd_origin = origin;
    return 0;
}

/* The invariant: every RAM byte with provenance holds that disc byte. */
void trace_check(const char *name, const uint8_t *(*bank)(unsigned))
{
    uint64_t held = 0, wrong = 0, off_track = 0;
    for (uint32_t p = 0; p < PHYS; p++) {
        uint32_t o = prov[p], lo = USER_LBA * 2048u;
        if (o == NONE)
            continue;
        held++;
        if (o < lo || o - lo >= USER_BYTES) {
            off_track++;
            continue;
        }
        const uint8_t *b = bank(p >> 13);
        if (!b || b[p & 0x1FFF] != user[o - lo])
            wrong++;
    }
    printf("trace %s: %llu RAM bytes hold disc bytes, %llu differ from the disc, %llu from outside track 02\n",
           name, (unsigned long long)held, (unsigned long long)wrong, (unsigned long long)off_track);
}

static void write_map(const char *dir, const char *name, const uint8_t *m, size_t n)
{
    char path[4096];
    snprintf(path, sizeof path, "%s/%s", dir, name);
    FILE *f = fopen(path, "wb");
    if (!f || fwrite(m, 1, n, f) != n) {
        perror(path);
        exit(1);
    }
    fclose(f);
}

void trace_finish(const char *dir)
{
    for (int k = 0; k < nseen; k++) {
        uint32_t o = seen[k].origin, lo = USER_LBA * 2048u;
        printf("watch %06X %c by the instruction at %02X:%04X", seen[k].addr, seen[k].rw,
               seen[k].op >> 13, seen[k].op & 0x1FFF);
        if (o != NONE && o >= lo && o - lo < USER_BYTES)
            printf(", disc %04X:%03X\n", (o - lo) / 2048, (o - lo) % 2048);
        else
            printf("%s\n", seen[k].op >> 13 < 0x20 ? ", System Card" : ", no disc origin");
    }
    uint64_t code = 0, data = 0, card = 0, np = 0;
    for (uint32_t i = 0; i < USER_BYTES; i++) {
        code += (discmap[i] & (F_OPC | F_OPD)) != 0;
        data += discmap[i] == F_DATA;
    }
    for (uint32_t i = 0; i < (1u << 18); i++)
        card += (cardmap[i] & (F_OPC | F_OPD)) != 0;
    for (uint32_t i = 0; i < PHYS; i++)
        np += (noprov[i] & (F_OPC | F_OPD)) != 0;
    printf("trace: disc code %llu B, disc data only %llu B, card code %llu B, "
           "RAM code with no disc origin %llu B, accesses outside track 02 %llu\n",
           (unsigned long long)code, (unsigned long long)data, (unsigned long long)card,
           (unsigned long long)np, (unsigned long long)outside);
    write_map(dir, "discmap.bin", discmap, USER_BYTES);
    write_map(dir, "cardmap.bin", cardmap, 1u << 18);
    write_map(dir, "noprov.bin", noprov, PHYS);
    static const char space[] = "DCRI";
    char path[4096];
    snprintf(path, sizeof path, "%s/edges.csv", dir);
    FILE *f = fopen(path, "w");
    if (!f) {
        perror(path);
        exit(1);
    }
    fprintf(f, "kind,from_space,from,to_space,to\n");
    for (uint32_t i = 0; i < EDGE_SLOTS; i++) {
        uint64_t k = edges[i];
        if (k == EDGE_EMPTY)
            continue;
        uint32_t kind = (uint32_t)(k >> 54), from = (uint32_t)(k >> 27) & 0x7FFFFFF, to = (uint32_t)k & 0x7FFFFFF;
        fprintf(f, "%X,%c,%X,%c,%X\n", kind, space[from >> 25], from & 0x1FFFFFF, space[to >> 25], to & 0x1FFFFFF);
    }
    fclose(f);
    printf("trace: %llu edges, %llu lost to a full table\n", (unsigned long long)nedges,
           (unsigned long long)edges_lost);
    if (edges_lost) {
        fprintf(stderr, "pcebox: the edge table is full: %llu edges lost\n", (unsigned long long)edges_lost);
        exit(1);
    }
}
