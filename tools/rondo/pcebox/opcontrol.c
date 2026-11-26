/*
 * opcontrol.c: the control for ops6280.h. Each case is one instruction (or SET and the one after
 * it) run on the core with chosen registers, flags and memory, and the same instruction as the
 * drafter writes it run on a model bus. Registers, P, S, the MPRs, the speed and the access list
 * (kind, physical address, value, order) must be equal.
 *
 * The core runs a HuCard image made by opcontrol.py: bank 0 holds a spin at $E000; the harness's
 * exec hook starts each case there by writing the instruction into RAM at $2200 and setting the
 * registers, and ends it at the next instruction. Windows 2 to 5 map ROM banks 1 to 4 (reads of
 * known data), window 1 is RAM (bank $F8, zero page and stack), window 7 the code.
 *
 *   opcontrol --core LIB --rom FILE
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libretro.h"
#include "machine.h"
#include "ops6280.h"
#include "cases.h"

Cpu cpu;

/* ---- the model bus ---- */

static uint8_t mpr[8];
static uint8_t mram[8192];
static uint8_t mspeed;

typedef struct { uint8_t w; uint32_t key; uint8_t v; } Acc;
static Acc macc[256], cacc[256];
static unsigned nm, nc;
static int logging;

uint8_t pat_rom(unsigned bank, unsigned off)
{
    if (bank == 0 && off == 0) return 0xEA;
    if (bank == 0 && off == 1) return 0x4C;
    if (bank == 0 && off == 2) return 0x00;
    if (bank == 0 && off == 3) return 0xE0;
    if (bank == 0 && off >= 0x1FF0) return 0x00;
    return (uint8_t)(bank * 37 + off * 13 + (off >> 8) * 7 + 5);
}

static uint8_t pat_ram(unsigned off) { return (uint8_t)(off * 29 + (off >> 8) * 3 + 0x11); }

static uint32_t key(uint16_t a) { return ((uint32_t)mpr[a >> 13] << 13) | (a & 0x1FFF); }

int enters(uint16_t a, uint16_t f, int j) { (void)a; (void)f; (void)j; return 1; }

uint8_t code8(uint16_t a)
{
    uint32_t k = key(a);
    unsigned bank = k >> 13;

    return bank >= 0xF8 && bank <= 0xFB ? mram[k & 0x1FFF] : bank < 0x80 ? pat_rom(bank, k & 0x1FFF) : 0xFF;
}

uint8_t rd8(uint16_t a)
{
    uint32_t k = key(a);
    unsigned bank = k >> 13;
    uint8_t v = bank >= 0xF8 && bank <= 0xFB ? mram[k & 0x1FFF] : bank < 0x80 ? pat_rom(bank, k & 0x1FFF) : 0xFF;

    macc[nm++] = (Acc){ 0, k, v };
    return v;
}

void wr8(uint16_t a, uint8_t v)
{
    uint32_t k = key(a);
    unsigned bank = k >> 13;

    macc[nm++] = (Acc){ 1, k, v };
    if (bank >= 0xF8 && bank <= 0xFB)
        mram[k & 0x1FFF] = v;
}

void st0(uint8_t v) { macc[nm++] = (Acc){ 1, 0x1FE000, v }; }
void st1(uint8_t v) { macc[nm++] = (Acc){ 1, 0x1FE002, v }; }
void st2(uint8_t v) { macc[nm++] = (Acc){ 1, 0x1FE003, v }; }
uint8_t tma(uint8_t bit)
{
    for (unsigned i = 0; i < 8; i++)
        if (bit & (1u << i))
            return mpr[i];
    return 0;
}
void tam(uint8_t bits, uint8_t bank)
{
    for (unsigned i = 0; i < 8; i++)
        if (bits & (1u << i))
            mpr[i] = bank;
}
void csl(void) { mspeed = 0; }
void csh(void) { mspeed = 1; }
void cli(void) { cpu.p &= (uint8_t)~P_I; }
void tai(uint16_t s, uint16_t d, uint16_t n) { (void)s; (void)d; (void)n; }
void call_original(uint16_t a) { (void)a; }
void call_orig(uint16_t r, uint16_t a) { (void)r; (void)a; }
void call_orig_framed(uint16_t r, uint16_t a) { (void)r; (void)a; }
void wait_interrupt(void) {}
void untraced(uint16_t a) { (void)a; }

/* ---- the core ---- */

static void (*core_run)(void);
static void (*g_get)(uint8_t *, uint8_t *, uint8_t *, uint8_t *, uint8_t *, uint16_t *);
static void (*g_set)(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t);
static unsigned (*g_mpr)(unsigned);
static void (*g_setmpr)(unsigned, uint8_t);
static void (*g_wr)(uint16_t, uint8_t);
static uint8_t (*g_rd)(uint16_t);
static uint8_t (*g_getspeed)(void);
static void (*g_setspeed)(uint8_t);

static void hook_rd(uint32_t phys, uint8_t v, int kind)
{
    if (logging && kind == 0 && nc < 256)
        cacc[nc++] = (Acc){ 0, phys, v };
}

static void hook_wr(uint32_t phys, uint8_t v)
{
    if (logging && nc < 256)
        cacc[nc++] = (Acc){ 1, phys, v };
}

static unsigned cur = 0, bad = 0, started = 0, done = 0;
static const Case *cs;
static unsigned long total_acc;
static const uint8_t DEFMPR[8] = { 0xFF, 0xF8, 1, 2, 3, 4, 5, 0 };

static void core_put(uint16_t a, uint8_t v)
{
    logging = 0;
    g_wr(a, v);
}

static void report(const Case *c, const char *why)
{
    bad++;
    if (bad <= 12)
        printf("MISMATCH case %u %s: %s\n", (unsigned)(c - cases), c->text, why);
}

static void finish_case(void)
{
    const Case *c = &cases[cur];
    uint8_t a, x, y, s, p;
    uint16_t pc;
    char why[200] = "";

    logging = 0;
    g_get(&a, &x, &y, &s, &p, &pc);
    /* the model */
    cpu = (Cpu){ c->a, c->x, c->y, c->s, c->p };
    nm = 0;
    int taken = 0;
    if (c->branch)
        taken = c->br();
    else
        c->fn();
    if (c->branch && taken != (pc != (uint16_t)(0x2200 + c->len)))
        snprintf(why, sizeof why, "branch taken %d, core pc %04X", taken, pc);
    else if (a != cpu.a || x != cpu.x || y != cpu.y || s != cpu.s || p != cpu.p)
        snprintf(why, sizeof why, "core A%02X X%02X Y%02X S%02X P%02X, C A%02X X%02X Y%02X S%02X P%02X", a, x, y, s, p,
                 cpu.a, cpu.x, cpu.y, cpu.s, cpu.p);
    else if (nm != nc)
        snprintf(why, sizeof why, "%u accesses on the core, %u in the C", nc, nm);
    else {
        for (unsigned i = 0; i < nm; i++)
            if (macc[i].w != cacc[i].w || macc[i].key != cacc[i].key || macc[i].v != cacc[i].v) {
                snprintf(why, sizeof why, "access %u: core %c %06X %02X, C %c %06X %02X", i, cacc[i].w ? 'w' : 'r',
                         cacc[i].key, cacc[i].v, macc[i].w ? 'w' : 'r', macc[i].key, macc[i].v);
                break;
            }
    }
    if (!why[0]) {
        for (unsigned i = 0; i < 8; i++)
            if (g_mpr(i) != mpr[i])
                snprintf(why, sizeof why, "MPR%u core %02X, C %02X", i, g_mpr(i), mpr[i]);
        if (!why[0] && g_getspeed() != mspeed)
            snprintf(why, sizeof why, "speed core %u, C %u", g_getspeed(), mspeed);
    }
    if (why[0])
        report(c, why);
    total_acc += nm;
    /* back to the pattern: RAM the case wrote, the MPRs, the speed */
    for (unsigned i = 0; i < nc; i++)
        if (cacc[i].w && (cacc[i].key >> 13) == 0xF8)
            core_put((uint16_t)(0x2000 | (cacc[i].key & 0x1FFF)), pat_ram(cacc[i].key & 0x1FFF));
    for (unsigned i = 0; i < c->nmem; i++)
        core_put(c->mem[i].addr, pat_ram(c->mem[i].addr & 0x1FFF));
    for (unsigned i = 0; i < 8; i++)
        g_setmpr(i, DEFMPR[i]);
    g_setspeed(0);
    for (unsigned i = 0; i < 4; i++)
        core_put((uint16_t)(0x2200 + i), pat_ram((0x200 + i) & 0x1FFF));
    memcpy(mpr, DEFMPR, 8);
    mspeed = 0;
    for (unsigned i = 0; i < 0x2000; i++)
        mram[i] = pat_ram(i);
}

static void start_case(void)
{
    const Case *c = &cases[cur];

    for (unsigned i = 0; i < c->nmem; i++) {
        core_put(c->mem[i].addr, c->mem[i].v);
        mram[c->mem[i].addr & 0x1FFF] = c->mem[i].v;
    }
    for (unsigned i = 0; i < c->len; i++)
        core_put((uint16_t)(0x2200 + i), c->ins[i]);
    nc = 0;
    logging = 1;
    g_set(c->a, c->x, c->y, c->s, c->p, 0x2200);
}

static unsigned remaining;

static int exec_hook(uint32_t pc)
{
    if (!started) {
        if (pc != 0xE000)
            return 0;
        started = 1;
        for (unsigned i = 0; i < 8; i++)
            g_setmpr(i, DEFMPR[i]);
        for (unsigned i = 0; i < 0x2000; i++)
            core_put((uint16_t)(0x2000 + i), pat_ram(i));
        memcpy(mpr, DEFMPR, 8);
        for (unsigned i = 0; i < 0x2000; i++)
            mram[i] = pat_ram(i);
        start_case();
        remaining = cases[cur].n;
    } else if (remaining == 0) {
        finish_case();
        if (++cur >= ncases) {
            printf("opcontrol: %u cases, %lu accesses compared, %u mismatches\n", ncases, total_acc, bad);
            fflush(stdout);
            exit(bad ? 1 : 0);
        }
        start_case();
        remaining = cases[cur].n;
    }
    remaining--;
    return 0;
}

static bool environment(unsigned cmd, void *data)
{
    switch (cmd) {
    case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
        *(const char **)data = ".";
        return true;
    case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
        return true;
    case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
        *(bool *)data = false;
        return true;
    case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
        *(unsigned *)data = 0;
        return true;
    case RETRO_ENVIRONMENT_SET_VARIABLES:
        return true;
    default:
        return false;
    }
}

static void video(const void *d, unsigned w, unsigned h, size_t p) { (void)d; (void)w; (void)h; (void)p; }
static void audio(int16_t l, int16_t r) { (void)l; (void)r; }
static size_t audio_batch(const int16_t *d, size_t n) { (void)d; return n; }
static void input_poll(void) {}
static int16_t input_state(unsigned a, unsigned b, unsigned c, unsigned d) { (void)a; (void)b; (void)c; (void)d; return 0; }

static void *sym(void *lib, const char *name)
{
    void *p = dlsym(lib, name);

    if (!p) {
        fprintf(stderr, "opcontrol: core has no %s\n", name);
        exit(1);
    }
    return p;
}

int main(int argc, char **argv)
{
    const char *core = NULL, *rom = NULL;

    for (int i = 1; i + 1 < argc; i += 2) {
        if (!strcmp(argv[i], "--core")) core = argv[i + 1];
        else if (!strcmp(argv[i], "--rom")) rom = argv[i + 1];
    }
    if (!core || !rom)
        return 2;
    void *lib = dlopen(core, RTLD_NOW | RTLD_LOCAL);
    if (!lib) {
        fprintf(stderr, "opcontrol: %s\n", dlerror());
        return 1;
    }
    ((void (*)(retro_environment_t))sym(lib, "retro_set_environment"))(environment);
    ((void (*)(retro_video_refresh_t))sym(lib, "retro_set_video_refresh"))(video);
    ((void (*)(retro_audio_sample_t))sym(lib, "retro_set_audio_sample"))(audio);
    ((void (*)(retro_audio_sample_batch_t))sym(lib, "retro_set_audio_sample_batch"))(audio_batch);
    ((void (*)(retro_input_poll_t))sym(lib, "retro_set_input_poll"))(input_poll);
    ((void (*)(retro_input_state_t))sym(lib, "retro_set_input_state"))(input_state);
    core_run = (void (*)(void))sym(lib, "retro_run");
    g_get = sym(lib, "rondo_get_regs");
    g_set = sym(lib, "rondo_set_regs");
    g_mpr = sym(lib, "rondo_mpr");
    g_setmpr = sym(lib, "rondo_set_mpr");
    g_wr = sym(lib, "rondo_bus_wr");
    g_rd = sym(lib, "rondo_bus_rd");
    g_getspeed = sym(lib, "rondo_get_speed");
    g_setspeed = sym(lib, "rondo_set_speed");
    ((void (*)(void))sym(lib, "retro_init"))();
    struct retro_game_info game = { rom, NULL, 0, NULL };
    if (!((bool (*)(const struct retro_game_info *))sym(lib, "retro_load_game"))(&game)) {
        fprintf(stderr, "opcontrol: the core refused %s\n", rom);
        return 1;
    }
    ((void (*)(unsigned, unsigned))sym(lib, "retro_set_controller_port_device"))(0, RETRO_DEVICE_JOYPAD);
    ((void (*)(void *, void *, void *))sym(lib, "rondo_set_hooks"))((void *)hook_rd, (void *)hook_wr, NULL);
    ((void (*)(void *, void *))sym(lib, "rondo_set_check_hooks"))((void *)exec_hook, NULL);
    cs = cases;
    (void)g_rd;
    for (unsigned frame = 0; frame < 100000; frame++)
        core_run();
    fprintf(stderr, "opcontrol: the cases did not finish (case %u)\n", cur);
    return 1;
}
