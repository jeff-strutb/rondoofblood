/*
 * pcebox: the original game run headless on the pinned PC Engine core.
 *
 *   pcebox --core LIB --cue CUE --system DIR --script FILE --out DIR [--card jp|us]
 *          [--trace USER [--plant-origin N]]
 *
 * The core is Mednafen's accurate PC Engine (Beetle PCE). Every option it
 * asks for that bears on emulation is answered explicitly, and every one
 * left at its default is logged, so no run silently takes a default.
 *
 * Script, one command per line ('#' starts a comment):
 *
 *   wait N              run N frames, pad released
 *   press B+B N         hold the buttons N frames, then release
 *   hold B+B            hold the buttons from here on (wait releases them)
 *   run N               run N frames, the pad as it is
 *   include FILE        run another script here (relative to this one)
 *   random SEED N H     N frames of seeded random play: a new pad state every
 *                       H frames, drawn from moves, jumps, attacks, the
 *                       sub-weapon and crouching (never SELECT+RUN, the
 *                       soft reset); the same seed plays the same
 *   shot NAME           the last frame as OUT/NAME.ppm
 *   dump NAME           OUT/NAME.ram: frame number, MPR0-7 and PC, then
 *                       Super System Card RAM ($68-$7F), CD RAM ($80-$87)
 *                       and base RAM ($F8), 8 KB a bank in that order
 *
 * Buttons: UP DOWN LEFT RIGHT I II SELECT RUN.
 *
 * --trace USER (track 02's user data) folds what runs onto the disc
 * (trace.c): every dump also checks that each RAM byte with a disc origin
 * holds that disc byte, and the maps are written to OUT at the end.
 * --plant-origin N shifts every CD origin by N bytes, to prove that check
 * fails. --watch PHYS (hex, repeatable) lists every instruction that reads
 * or writes that physical byte, with its disc origin.
 *
 * The oracles (check.c), each with --user (track 02's user data):
 *   --a5 FILE        judge every call of a routine with C; report to FILE
 *   --record FILE    log every call's cost and each frame's hash to FILE
 *   --hybrid FILE    run with the C swapped in against that record
 */
#include <dlfcn.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libretro.h"

int trace_init(const char *user_path, int plant_offset, uint32_t (*origin)(void));
void trace_rd(uint32_t phys, uint8_t v, int kind);
void trace_wr(uint32_t phys, uint8_t v);
void trace_irq(void);
void trace_check(const char *name, const uint8_t *(*bank)(unsigned));
void trace_finish(const char *dir);
void trace_watch(uint32_t phys);
static const char *trace_user;
static int trace_plant;

extern void (*chk_get_regs)(uint8_t *, uint8_t *, uint8_t *, uint8_t *, uint8_t *, uint16_t *);
extern void (*chk_set_regs)(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t);
extern unsigned (*chk_mpr)(unsigned);
extern void (*chk_set_mpr)(unsigned, uint8_t);
extern uint32_t (*chk_ts)(void);
extern uint8_t (*chk_bus_rd)(uint16_t);
extern void (*chk_bus_wr)(uint16_t, uint8_t);
extern void (*chk_bus_st)(unsigned, uint8_t);
extern const uint8_t *(*chk_bank)(unsigned);
int check_init(int which, const char *path, const char *user_path);
void check_finish(void);
void check_rd(uint32_t phys, uint8_t v, int kind);
void check_wr_pre(uint32_t phys, uint8_t v);
void check_irq(void);
int check_exec(uint32_t pc);
void check_frame(unsigned long frame, const uint32_t *pixels, unsigned w, unsigned h);
static int check_mode;
static const char *check_file, *check_user;

static const char *opt_card = "System Card 3";
static const char *system_dir, *out_dir;

/* Options that bear on emulation, answered as the real machine is: a
 * Super System Card 3.0 alone (no Arcade Card), the drive at its own
 * speed, one standard 2-button pad with no multitap, the VDC's sprite
 * limit, no overclock. What is left to the core's defaults shapes only
 * the host's picture and sound (palette, cropping, volumes, resampling)
 * or pads 2-5 and turbo, which a plain machine does not have. */
static const char *answer(const char *key)
{
    static const struct { const char *k, *v; } a[] = {
        { "pce_cdbios", NULL },            /* from --card */
        { "pce_arcadecard", "disabled" },
        { "pce_cdspeed", "1" },
        { "pce_cdimagecache", "disabled" },
        { "pce_multitap", "disabled" },
        { "pce_default_joypad_type_p1", "2 Buttons" },
        { "pce_Turbo_Toggling", "disabled" },
        { "pce_up_down_allowed", "disabled" },
        { "pce_disable_softreset", "disabled" },
        { "pce_psgrevision", "HuC6280" },
        { "pce_nospritelimit", "disabled" },
        { "pce_ocmultiplier", "1" },
    };
    for (size_t i = 0; i < sizeof a / sizeof a[0]; i++)
        if (!strcmp(key, a[i].k))
            return a[i].v ? a[i].v : opt_card;
    return NULL;
}

static void core_log(enum retro_log_level level, const char *fmt, ...)
{
    va_list ap;
    if (level < RETRO_LOG_WARN)
        return;
    fprintf(stderr, "core: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

static enum retro_pixel_format pixfmt = RETRO_PIXEL_FORMAT_0RGB1555;
static char defaulted[64][64];
static int ndefaulted;

static bool environment(unsigned cmd, void *data)
{
    switch (cmd) {
    case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
        *(const char **)data = system_dir;
        return true;
    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
        *(const char **)data = out_dir;
        return true;
    case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
        ((struct retro_log_callback *)data)->log = core_log;
        return true;
    case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
        /* The core is built 16-bit (RGB565) and draws so whatever the
         * answer; take what it asks for and convert. */
        pixfmt = *(enum retro_pixel_format *)data;
        return pixfmt == RETRO_PIXEL_FORMAT_XRGB8888 || pixfmt == RETRO_PIXEL_FORMAT_RGB565;
    case RETRO_ENVIRONMENT_GET_VARIABLE: {
        struct retro_variable *v = data;
        v->value = answer(v->key);
        if (!v->value) {
            int seen = 0;
            for (int i = 0; i < ndefaulted; i++)
                seen |= !strcmp(defaulted[i], v->key);
            if (!seen && ndefaulted < 64)
                snprintf(defaulted[ndefaulted++], 64, "%s", v->key);
        }
        return v->value != NULL;
    }
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

static uint32_t frame_px[1024 * 512];
static unsigned frame_w, frame_h;
static unsigned long frames;
static unsigned pad;

static void video(const void *data, unsigned w, unsigned h, size_t pitch)
{
    if (!data)
        return;
    if (w > 1024) w = 1024;
    if (h > 512) h = 512;
    for (unsigned y = 0; y < h; y++) {
        const uint8_t *row = (const uint8_t *)data + y * pitch;
        if (pixfmt == RETRO_PIXEL_FORMAT_XRGB8888) {
            memcpy(frame_px + y * w, row, w * 4);
            continue;
        }
        for (unsigned x = 0; x < w; x++) {
            uint16_t c = ((const uint16_t *)row)[x];
            uint32_t r = (c >> 11) & 31, g = (c >> 5) & 63, b = c & 31;
            frame_px[y * w + x] = ((r << 3 | r >> 2) << 16) | ((g << 2 | g >> 4) << 8) | (b << 3 | b >> 2);
        }
    }
    frame_w = w;
    frame_h = h;
}

static void audio(int16_t l, int16_t r) { (void)l; (void)r; }
static size_t audio_batch(const int16_t *d, size_t n) { (void)d; return n; }
static void input_poll(void) {}

static int16_t input_state(unsigned port, unsigned device, unsigned index, unsigned id)
{
    (void)index;
    if (port != 0 || device != RETRO_DEVICE_JOYPAD)
        return 0;
    if (id == RETRO_DEVICE_ID_JOYPAD_MASK)
        return (int16_t)pad;
    return (pad >> id) & 1;
}

static unsigned buttons(const char *s)
{
    static const struct { const char *n; unsigned id; } b[] = {
        { "UP", RETRO_DEVICE_ID_JOYPAD_UP }, { "DOWN", RETRO_DEVICE_ID_JOYPAD_DOWN },
        { "LEFT", RETRO_DEVICE_ID_JOYPAD_LEFT }, { "RIGHT", RETRO_DEVICE_ID_JOYPAD_RIGHT },
        { "I", RETRO_DEVICE_ID_JOYPAD_A }, { "II", RETRO_DEVICE_ID_JOYPAD_B },
        { "SELECT", RETRO_DEVICE_ID_JOYPAD_SELECT }, { "RUN", RETRO_DEVICE_ID_JOYPAD_START },
    };
    unsigned m = 0;
    char buf[128];
    snprintf(buf, sizeof buf, "%s", s);
    for (char *t = strtok(buf, "+"); t; t = strtok(NULL, "+")) {
        size_t i;
        for (i = 0; i < sizeof b / sizeof b[0]; i++)
            if (!strcmp(t, b[i].n))
                break;
        if (i == sizeof b / sizeof b[0]) {
            fprintf(stderr, "pcebox: unknown button %s\n", t);
            exit(2);
        }
        m |= 1u << b[i].id;
    }
    return m;
}

static void (*core_run)(void);
static const uint8_t *(*core_bank)(unsigned);
static unsigned (*core_mpr)(unsigned);
static unsigned (*core_pc)(void);

static void run(unsigned long n)
{
    while (n--) {
        core_run();
        frames++;
        if (check_mode)
            check_frame(frames, frame_px, frame_w, frame_h);
    }
}

static void random_play(uint32_t seed, unsigned long n, unsigned long hold)
{
    static const char *states[] = {
        "RIGHT", "RIGHT", "RIGHT+II", "RIGHT+I", "II", "I", "LEFT", "LEFT+II",
        "DOWN+I", "UP+I", "RIGHT+UP+I", "DOWN", "UP", "RIGHT+II", "I", "RIGHT",
    };
    uint32_t x = seed ? seed : 1;
    while (n) {
        unsigned long k = n < hold ? n : hold;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        pad = (x >> 8) % 7 == 0 ? 0 : buttons(states[x & 15]);
        run(k);
        n -= k;
    }
    pad = 0;
}

static FILE *open_out(const char *name, const char *ext)
{
    char path[4096];
    snprintf(path, sizeof path, "%s/%s.%s", out_dir, name, ext);
    FILE *f = fopen(path, "wb");
    if (!f) {
        perror(path);
        exit(1);
    }
    return f;
}

static void shot(const char *name)
{
    FILE *f = open_out(name, "ppm");
    fprintf(f, "P6\n%u %u\n255\n", frame_w, frame_h);
    for (unsigned i = 0; i < frame_w * frame_h; i++) {
        uint32_t p = frame_px[i];
        fputc((p >> 16) & 0xFF, f);
        fputc((p >> 8) & 0xFF, f);
        fputc(p & 0xFF, f);
    }
    fclose(f);
}

static void dump(const char *name)
{
    if (trace_user)
        trace_check(name, core_bank);
    static const uint8_t zero[8192];
    FILE *f = open_out(name, "ram");
    uint8_t hdr[16] = { 0 };
    for (int i = 0; i < 4; i++)
        hdr[i] = (uint8_t)(frames >> (8 * i));
    for (int i = 0; i < 8; i++)
        hdr[4 + i] = (uint8_t)core_mpr(i);
    hdr[12] = (uint8_t)core_pc();
    hdr[13] = (uint8_t)(core_pc() >> 8);
    fwrite(hdr, 1, sizeof hdr, f);
    for (unsigned b = 0x68; b <= 0x88; b++) {
        unsigned bank = b == 0x88 ? 0xF8 : b;
        const uint8_t *p = core_bank(bank);
        fwrite(p ? p : zero, 1, 8192, f);
    }
    fclose(f);
}

/* Run a script; include nests up to 8 deep. 0, or nonzero on a bad line. */
static int run_script(const char *script, int depth)
{
    FILE *s = fopen(script, "r");
    if (!s) {
        perror(script);
        return 1;
    }
    char line[512];
    int ln = 0;
    while (fgets(line, sizeof line, s)) {
        char cmd[32], a1[128];
        unsigned long n = 0;
        ln++;
        char *h = strchr(line, '#');
        if (h) *h = 0;
        int k = sscanf(line, "%31s %127s %lu", cmd, a1, &n);
        if (k <= 0)
            continue;
        if (!strcmp(cmd, "wait") && k >= 2) {
            pad = 0;
            run(strtoul(a1, NULL, 10));
        } else if (!strcmp(cmd, "random") && k == 3) {
            unsigned long hold = 8;
            char a3[32];
            if (sscanf(line, "%*s %*s %*s %31s", a3) == 1)
                hold = strtoul(a3, NULL, 10);
            random_play((uint32_t)strtoul(a1, NULL, 10), n, hold ? hold : 8);
        } else if (!strcmp(cmd, "hold") && k == 2) {
            pad = buttons(a1);
        } else if (!strcmp(cmd, "run") && k == 2) {
            run(strtoul(a1, NULL, 10));
        } else if (!strcmp(cmd, "press") && k == 3) {
            pad = buttons(a1);
            run(n);
            pad = 0;
        } else if (!strcmp(cmd, "include") && k == 2) {
            char path[4096];
            const char *slash = strrchr(script, '/');
            if (a1[0] == '/' || !slash)
                snprintf(path, sizeof path, "%s", a1);
            else
                snprintf(path, sizeof path, "%.*s/%s", (int)(slash - script), script, a1);
            if (depth > 8 || run_script(path, depth + 1)) {
                fclose(s);
                return 2;
            }
        } else if (!strcmp(cmd, "shot") && k == 2) {
            shot(a1);
        } else if (!strcmp(cmd, "dump") && k == 2) {
            dump(a1);
        } else {
            fprintf(stderr, "pcebox: %s:%d: bad command\n", script, ln);
            fclose(s);
            return 2;
        }
    }
    fclose(s);
    return 0;
}

static void *sym(void *lib, const char *name)
{
    void *p = dlsym(lib, name);
    if (!p) {
        fprintf(stderr, "pcebox: core has no %s\n", name);
        exit(1);
    }
    return p;
}

int main(int argc, char **argv)
{
    const char *core = NULL, *cue = NULL, *script = NULL;
    for (int i = 1; i + 1 < argc; i += 2) {
        if (!strcmp(argv[i], "--core")) core = argv[i + 1];
        else if (!strcmp(argv[i], "--cue")) cue = argv[i + 1];
        else if (!strcmp(argv[i], "--system")) system_dir = argv[i + 1];
        else if (!strcmp(argv[i], "--script")) script = argv[i + 1];
        else if (!strcmp(argv[i], "--out")) out_dir = argv[i + 1];
        else if (!strcmp(argv[i], "--trace")) trace_user = argv[i + 1];
        else if (!strcmp(argv[i], "--a5")) { check_mode = 1; check_file = argv[i + 1]; }
        else if (!strcmp(argv[i], "--record")) { check_mode = 2; check_file = argv[i + 1]; }
        else if (!strcmp(argv[i], "--hybrid")) { check_mode = 3; check_file = argv[i + 1]; }
        else if (!strcmp(argv[i], "--user")) check_user = argv[i + 1];
        else if (!strcmp(argv[i], "--plant-origin")) trace_plant = atoi(argv[i + 1]);
        else if (!strcmp(argv[i], "--watch")) trace_watch((uint32_t)strtoul(argv[i + 1], NULL, 16));
        else if (!strcmp(argv[i], "--card"))
            opt_card = !strcmp(argv[i + 1], "us") ? "System Card 3 US" : "System Card 3";
        else {
            fprintf(stderr, "pcebox: unknown argument %s\n", argv[i]);
            return 2;
        }
    }
    if (!core || !cue || !system_dir || !script || !out_dir) {
        fprintf(stderr, "usage: pcebox --core LIB --cue CUE --system DIR --script FILE --out DIR [--card jp|us]\n");
        return 2;
    }
    void *lib = dlopen(core, RTLD_NOW | RTLD_LOCAL);
    if (!lib) {
        fprintf(stderr, "pcebox: %s\n", dlerror());
        return 1;
    }
    ((void (*)(retro_environment_t))sym(lib, "retro_set_environment"))(environment);
    ((void (*)(retro_video_refresh_t))sym(lib, "retro_set_video_refresh"))(video);
    ((void (*)(retro_audio_sample_t))sym(lib, "retro_set_audio_sample"))(audio);
    ((void (*)(retro_audio_sample_batch_t))sym(lib, "retro_set_audio_sample_batch"))(audio_batch);
    ((void (*)(retro_input_poll_t))sym(lib, "retro_set_input_poll"))(input_poll);
    ((void (*)(retro_input_state_t))sym(lib, "retro_set_input_state"))(input_state);
    core_run = (void (*)(void))sym(lib, "retro_run");
    core_bank = (const uint8_t *(*)(unsigned))sym(lib, "rondo_bank");
    core_mpr = (unsigned (*)(unsigned))sym(lib, "rondo_mpr");
    core_pc = (unsigned (*)(void))sym(lib, "rondo_pc");
    ((void (*)(void))sym(lib, "retro_init"))();

    struct retro_game_info game = { cue, NULL, 0, NULL };
    if (!((bool (*)(const struct retro_game_info *))sym(lib, "retro_load_game"))(&game)) {
        fprintf(stderr, "pcebox: the core refused %s\n", cue);
        return 1;
    }
    ((void (*)(unsigned, unsigned))sym(lib, "retro_set_controller_port_device"))(0, RETRO_DEVICE_JOYPAD);
    if (trace_user) {
        if (trace_init(trace_user, trace_plant, (uint32_t (*)(void))sym(lib, "rondo_cd_origin")))
            return 1;
        ((void (*)(void *, void *, void *))sym(lib, "rondo_set_hooks"))(
            (void *)trace_rd, (void *)trace_wr, (void *)trace_irq);
    }
    if (check_mode) {
        chk_get_regs = (void (*)(uint8_t *, uint8_t *, uint8_t *, uint8_t *, uint8_t *, uint16_t *))sym(lib, "rondo_get_regs");
        chk_set_regs = (void (*)(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t))sym(lib, "rondo_set_regs");
        chk_mpr = core_mpr;
        chk_set_mpr = (void (*)(unsigned, uint8_t))sym(lib, "rondo_set_mpr");
        chk_ts = (uint32_t (*)(void))sym(lib, "rondo_timestamp");
        chk_bus_rd = (uint8_t (*)(uint16_t))sym(lib, "rondo_bus_rd");
        chk_bus_wr = (void (*)(uint16_t, uint8_t))sym(lib, "rondo_bus_wr");
        chk_bus_st = (void (*)(unsigned, uint8_t))sym(lib, "rondo_bus_st");
        chk_bank = core_bank;
        if (!check_user || check_init(check_mode, check_file, check_user))
            return 1;
        ((void (*)(void *, void *, void *))sym(lib, "rondo_set_hooks"))(
            (void *)check_rd, NULL, (void *)check_irq);
        ((void (*)(void *, void *))sym(lib, "rondo_set_check_hooks"))(
            (void *)check_exec, check_mode == 1 ? (void *)check_wr_pre : NULL);
    }
    for (int i = 0; i < ndefaulted; i++)
        fprintf(stderr, "pcebox: option %s left at the core's default\n", defaulted[i]);

    if (run_script(script, 0))
        return 2;
    printf("pcebox: %lu frames\n", frames);
    if (trace_user)
        trace_finish(out_dir);
    if (check_mode)
        check_finish();
    ((void (*)(void))sym(lib, "retro_unload_game"))();
    ((void (*)(void))sym(lib, "retro_deinit"))();
    return 0;
}
