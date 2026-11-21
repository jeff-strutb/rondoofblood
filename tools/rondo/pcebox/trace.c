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
 * Three more outputs say what each disc byte is, and where it ran, so code and data can be told
 * apart and every executed byte has its run address and bank (the core exports the logical
 * address of the access in the hook, patch 0007):
 *   discmap2.bin  one byte per disc byte of track 02's user data: 1 loaded through the CD port,
 *                 2 loaded into a register by code from RAM, 4 sent to the VDC, 8 to the VCE
 *                 palette, 16 to the PSG, 32 to the ADPCM port, 64 to another I/O port,
 *                 128 moved by a block move. discmap.bin's bit 4 stays "1 or 2 or 128".
 *   runmap.bin    12-byte records, sorted and distinct: disc offset u32, logical address u16,
 *                 physical bank u8, kind u8 (1 opcode, 2 operand), mapper configuration u16, 0 u16:
 *                 where each executed byte ran, and what MPR2-6 held then (mprcfg.csv, by id).
 *   reads.csv     disc_start,length,reader,reader_run,data_logical,data_bank: runs of consecutive
 *                 disc bytes one instruction (D:disc offset, C:card offset, or R) loaded from RAM.
 * And edges.csv: every distinct transfer of control the catalog starts routines at,
 * each end where it ran: kind (the opcode: 20 JSR, 44 BSR, 4C JMP, 6C JMP (abs),
 * 7C JMP (abs,X); 100 an interrupt's entry, from the instruction it interrupted),
 * then from and to, each a space (D the disc's track 02 user data, C the System Card,
 * R RAM with no disc origin, or I/O) and an offset in it.
 */
#define _GNU_SOURCE
#include <dlfcn.h>
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
static unsigned (*get_logical)(void);   /* the core's logical address of the access in the hook */
static unsigned (*get_mpr)(unsigned);   /* the core's MPR i */
#define CFG_MAX 4096
static uint8_t cfgs[CFG_MAX][5];        /* every distinct MPR2-6 seen at an instruction fetch */
static int ncfg, cfg_cur, cfg_dirty = 1;
static uint16_t *last_cfg;

/* discmap2 flags. */
#define G_LOAD 1
#define G_READ 2
#define G_VDC 4
#define G_VCE 8
#define G_PSG 16
#define G_ADPCM 32
#define G_IO 64
#define G_MOVE 128
static uint8_t *map2;
static uint16_t *last_logical;            /* per physical byte: where it last ran */
static uint32_t *last_origin;             /*   and the disc byte it held then */
typedef struct { uint32_t disc; uint16_t logical; uint8_t bank; uint8_t kind; uint16_t cfg; uint16_t pad; } RunRec;
static RunRec *runs;
static uint64_t nruns, runs_cap;
#define RUN_MAX (24u << 20)
#define READ_BITS 22
#define READ_SLOTS (1u << READ_BITS)
static uint64_t *read_keys;               /* (reader << 32 | origin), or all ones */
static uint64_t *read_info;               /* reader_run << 24 | data_logical << 8 | data_bank */
static uint64_t nreads, reads_lost;
static uint32_t cur_reader = 0xFFFFFFFFu; /* the current instruction: D disc offset, 0x80000000 | card offset, or none */
static uint16_t cur_run;

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

/* A disc byte's offset in track 02's user data, or NONE. */
static uint32_t uoff(uint32_t origin)
{
    uint32_t lo = USER_LBA * 2048u;
    return origin != NONE && origin >= lo && origin - lo < USER_BYTES ? origin - lo : NONE;
}

static void mark2(uint32_t origin, uint8_t flag)
{
    uint32_t u = uoff(origin);
    if (u != NONE)
        map2[u] |= flag;
}

static void run_record(uint32_t u, uint16_t logical, uint8_t bank, uint8_t kind, uint16_t cfg)
{
    if (nruns == runs_cap) {
        if (runs_cap >= RUN_MAX) {
            fprintf(stderr, "pcebox: the run map is full\n");
            exit(1);
        }
        runs_cap = runs_cap ? runs_cap * 2 : 1u << 20;
        runs = realloc(runs, runs_cap * sizeof *runs);
        if (!runs) {
            fprintf(stderr, "pcebox: out of memory for the run map\n");
            exit(1);
        }
    }
    runs[nruns++] = (RunRec){ u, logical, bank, kind, cfg, 0 };
}

/* One (reader, data byte) pair, once: where a table is read, and by whom. */
static void read_note(uint32_t reader, uint32_t u, uint16_t logical, uint8_t bank)
{
    uint64_t key = (uint64_t)reader << 32 | u;
    uint64_t h = key * 0x9E3779B97F4A7C15ull;
    for (uint32_t i = (uint32_t)(h >> (64 - READ_BITS)), n = 0; n < READ_SLOTS; i = (i + 1) & (READ_SLOTS - 1), n++) {
        if (read_keys[i] == key)
            return;
        if (read_keys[i] == EDGE_EMPTY) {
            if (nreads * 2 >= READ_SLOTS) {
                reads_lost++;
                return;
            }
            read_keys[i] = key;
            read_info[i] = (uint64_t)cur_run << 24 | (uint64_t)logical << 8 | bank;
            nreads++;
            return;
        }
    }
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
        if (prov[p] != NONE) {
            credit(prov[p], flag);
            uint16_t logical = (uint16_t)get_logical();
            if (last_logical[p] != logical || last_origin[p] != prov[p] || last_cfg[p] != (uint16_t)cfg_cur) {
                last_logical[p] = logical;
                last_origin[p] = prov[p];
                last_cfg[p] = (uint16_t)cfg_cur;
                uint32_t u = uoff(prov[p]);
                if (u != NONE)
                    run_record(u, logical, (uint8_t)bank, flag == F_OPC ? 1 : 2, (uint16_t)cfg_cur);
            }
        } else
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
        if (cfg_dirty) {                  /* MPR2-6 as of this instruction: they change only by TAM */
            uint8_t now[5];
            for (int i = 0; i < 5; i++)
                now[i] = (uint8_t)get_mpr(2 + i);
            int k = 0;
            while (k < ncfg && memcmp(cfgs[k], now, 5))
                k++;
            if (k == ncfg) {
                if (ncfg == CFG_MAX) {
                    fprintf(stderr, "pcebox: more than %d mapper configurations\n", CFG_MAX);
                    exit(1);
                }
                memcpy(cfgs[ncfg++], now, 5);
            }
            cfg_cur = k;
            cfg_dirty = 0;
        }
        if (v == 0x53)
            cfg_dirty = 1;
        exec(p, F_OPC);
        cur_run = (uint16_t)get_logical();
        if (p >> 13 < 0x20)
            cur_reader = 0x80000000u | p;
        else {
            uint32_t u = is_ram(p >> 13) ? uoff(prov[p]) : NONE;
            cur_reader = u;
        }
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
        else {
            credit(o, F_DATA);
            if (o != NONE) {
                if (is_ram(p >> 13)) {
                    mark2(o, cls == OC_BLOCK ? G_MOVE : G_READ);
                    uint32_t u = uoff(o);
                    if (u != NONE)
                        read_note(cur_reader, u, (uint16_t)get_logical(), (uint8_t)(p >> 13));
                } else
                    mark2(o, G_LOAD);
            }
        }
    }
}

void trace_wr(uint32_t phys, uint8_t v)
{
    uint32_t p = canon(phys);
    (void)v;
    if (nwatch)
        watched(p, 'w');
    if (p >> 13 == 0xFF) {                /* a port: a disc byte that gets here is data */
        uint32_t o = NONE, off = p & 0x1FFF;
        switch (cls) {
        case OC_STA: o = pa; break;
        case OC_STX: o = px; break;
        case OC_STY: o = py; break;
        case OC_BLOCK: o = block_read; break;
        default: break;
        }
        if (o != NONE) {
            uint8_t g = G_IO;
            if (off < 0x400)
                g = (off & 3) >= 2 ? G_VDC : G_IO;
            else if (off < 0x800)
                g = (off & 7) == 4 || (off & 7) == 5 ? G_VCE : G_IO;
            else if (off < 0xC00)
                g = G_PSG;
            else if (off == 0x180A)
                g = G_ADPCM;
            mark2(o, g);
        }
        return;
    }
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

static int run_cmp(const void *a, const void *b)
{
    const RunRec *x = a, *y = b;
    if (x->disc != y->disc) return x->disc < y->disc ? -1 : 1;
    if (x->logical != y->logical) return x->logical < y->logical ? -1 : 1;
    if (x->bank != y->bank) return x->bank < y->bank ? -1 : 1;
    return (int)x->kind - (int)y->kind;
}

typedef struct { uint32_t reader, u; uint64_t info; } ReadRec;

static int read_cmp(const void *a, const void *b)
{
    const ReadRec *x = a, *y = b;
    if (x->reader != y->reader) return x->reader < y->reader ? -1 : 1;
    return x->u < y->u ? -1 : x->u > y->u;
}

static void write_map(const char *dir, const char *name, const uint8_t *m, size_t n);

/* discmap2.bin, runmap.bin and reads.csv. */
static void write_more(const char *dir)
{
    char path[4096];
    write_map(dir, "discmap2.bin", map2, USER_BYTES);
    qsort(runs, nruns, sizeof *runs, run_cmp);
    snprintf(path, sizeof path, "%s/runmap.bin", dir);
    FILE *f = fopen(path, "wb");
    if (!f) {
        perror(path);
        exit(1);
    }
    uint64_t kept = 0;
    for (uint64_t i = 0; i < nruns; i++) {
        if (i && !run_cmp(&runs[i], &runs[i - 1]))
            continue;
        fwrite(&runs[i], sizeof *runs, 1, f);
        kept++;
    }
    fclose(f);
    snprintf(path, sizeof path, "%s/mprcfg.csv", dir);
    f = fopen(path, "w");
    if (!f) {
        perror(path);
        exit(1);
    }
    fprintf(f, "id,mpr2,mpr3,mpr4,mpr5,mpr6\n");
    for (int k = 0; k < ncfg; k++)
        fprintf(f, "%d,%02X,%02X,%02X,%02X,%02X\n", k, cfgs[k][0], cfgs[k][1], cfgs[k][2], cfgs[k][3], cfgs[k][4]);
    fclose(f);
    ReadRec *rr = malloc((nreads + 1) * sizeof *rr);
    uint64_t n = 0;
    for (uint32_t i = 0; i < READ_SLOTS; i++)
        if (read_keys[i] != EDGE_EMPTY)
            rr[n++] = (ReadRec){ (uint32_t)(read_keys[i] >> 32), (uint32_t)read_keys[i], read_info[i] };
    qsort(rr, n, sizeof *rr, read_cmp);
    snprintf(path, sizeof path, "%s/reads.csv", dir);
    f = fopen(path, "w");
    if (!f) {
        perror(path);
        exit(1);
    }
    fprintf(f, "disc_start,length,reader,reader_run,data_logical,data_bank\n");
    uint64_t rows = 0;
    for (uint64_t i = 0; i < n;) {
        uint64_t j = i + 1;
        uint32_t lg = (uint32_t)(rr[i].info >> 8) & 0xFFFF;
        while (j < n && rr[j].reader == rr[i].reader && rr[j].u == rr[j - 1].u + 1 &&
               (rr[j].info & 0xFF) == (rr[i].info & 0xFF) &&
               (rr[j].info >> 24) == (rr[i].info >> 24) &&
               ((uint32_t)(rr[j].info >> 8) & 0xFFFF) == ((lg + (uint32_t)(j - i)) & 0xFFFF))
            j++;
        char who[32];
        if (rr[i].reader == 0xFFFFFFFFu) snprintf(who, sizeof who, "R");
        else if (rr[i].reader & 0x80000000u) snprintf(who, sizeof who, "C:%X", rr[i].reader & 0x7FFFFFFFu);
        else snprintf(who, sizeof who, "D:%X", rr[i].reader);
        fprintf(f, "%X,%llu,%s,%llX,%X,%llX\n", rr[i].u, (unsigned long long)(j - i), who,
                (unsigned long long)((rr[i].info >> 24) & 0xFFFF), lg, (unsigned long long)(rr[i].info & 0xFF));
        rows++;
        i = j;
    }
    fclose(f);
    free(rr);
    uint64_t counts[8] = {0};
    for (uint32_t i = 0; i < USER_BYTES; i++)
        for (int b = 0; b < 8; b++)
            counts[b] += (map2[i] >> b) & 1;
    printf("trace2: loaded %llu, read %llu, vdc %llu, vce %llu, psg %llu, adpcm %llu, other io %llu, moved %llu; "
           "%llu run records; %llu read pairs in %llu rows, %llu lost to a full table\n",
           (unsigned long long)counts[0], (unsigned long long)counts[1], (unsigned long long)counts[2],
           (unsigned long long)counts[3], (unsigned long long)counts[4], (unsigned long long)counts[5],
           (unsigned long long)counts[6], (unsigned long long)counts[7], (unsigned long long)kept,
           (unsigned long long)n, (unsigned long long)rows, (unsigned long long)reads_lost);
    if (reads_lost) {
        fprintf(stderr, "pcebox: the read table is full: %llu pairs lost\n", (unsigned long long)reads_lost);
        exit(1);
    }
}

/* A script's poke: the byte now holds no disc byte. */
void trace_poke(uint32_t phys)
{
    if (prov)
        prov[canon(phys)] = NONE;
}

void trace_irq(void)
{
    irq_pending = 1;
    finish();
    last_read = block_read = NONE;
}

int trace_init(const char *user_path, int plant_offset, uint32_t (*origin)(void))
{
    /* The core is a library loaded with its symbols local: find it from the address of one of its functions. */
    Dl_info info;
    void *lib = dladdr((void *)origin, &info) ? dlopen(info.dli_fname, RTLD_NOW | RTLD_NOLOAD) : NULL;
    get_logical = lib ? (unsigned (*)(void))dlsym(lib, "rondo_logical") : NULL;
    get_mpr = lib ? (unsigned (*)(unsigned))dlsym(lib, "rondo_mpr") : NULL;
    if (!get_logical || !get_mpr) {
        fprintf(stderr, "pcebox: the core does not export rondo_logical (patch 0007)\n");
        return -1;
    }
    FILE *f = fopen(user_path, "rb");
    prov = malloc(PHYS * sizeof *prov);
    discmap = calloc(USER_BYTES, 1);
    cardmap = calloc(1u << 18, 1);
    noprov = calloc(PHYS, 1);
    user = malloc(USER_BYTES);
    edges = malloc(EDGE_SLOTS * sizeof *edges);
    map2 = calloc(USER_BYTES, 1);
    last_logical = calloc(PHYS, sizeof *last_logical);
    last_origin = malloc(PHYS * sizeof *last_origin);
    last_cfg = calloc(PHYS, sizeof *last_cfg);
    read_keys = malloc(READ_SLOTS * sizeof *read_keys);
    read_info = malloc(READ_SLOTS * sizeof *read_info);
    if (!map2 || !last_logical || !last_origin || !last_cfg || !read_keys || !read_info) {
        fprintf(stderr, "pcebox: out of memory for the trace\n");
        return -1;
    }
    memset(last_origin, 0xFF, PHYS * sizeof *last_origin);
    memset(read_keys, 0xFF, READ_SLOTS * sizeof *read_keys);
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
    write_more(dir);
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
