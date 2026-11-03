/*
 * pcebox: the original game run headless on the pinned PC Engine core.
 *
 *   pcebox --core LIB --cue CUE --system DIR --script FILE --out DIR [--card jp|us]
 *
 * The core is Mednafen's accurate PC Engine (Beetle PCE). Every option it
 * asks for that bears on emulation is answered explicitly, and every one
 * left at its default is logged, so no run silently takes a default.
 *
 * Script, one command per line ('#' starts a comment):
 *
 *   wait N              run N frames, pad released
 *   press B+B N         hold the buttons N frames, then release
 *   shot NAME           the last frame as OUT/NAME.ppm
 *   dump NAME           OUT/NAME.ram: frame number, MPR0-7 and PC, then
 *                       Super System Card RAM ($68-$7F), CD RAM ($80-$87)
 *                       and base RAM ($F8), 8 KB a bank in that order
 *
 * Buttons: UP DOWN LEFT RIGHT I II SELECT RUN.
 */
#include <dlfcn.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libretro.h"

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
        return *(enum retro_pixel_format *)data == RETRO_PIXEL_FORMAT_XRGB8888;
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
    for (unsigned y = 0; y < h; y++)
        memcpy(frame_px + y * w, (const uint8_t *)data + y * pitch, w * 4);
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
    }
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
    for (int i = 0; i < ndefaulted; i++)
        fprintf(stderr, "pcebox: option %s left at the core's default\n", defaulted[i]);

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
        } else if (!strcmp(cmd, "press") && k == 3) {
            pad = buttons(a1);
            run(n);
            pad = 0;
        } else if (!strcmp(cmd, "shot") && k == 2) {
            shot(a1);
        } else if (!strcmp(cmd, "dump") && k == 2) {
            dump(a1);
        } else {
            fprintf(stderr, "pcebox: %s:%d: bad command\n", script, ln);
            return 2;
        }
    }
    fclose(s);
    printf("pcebox: %lu frames\n", frames);
    ((void (*)(void))sym(lib, "retro_unload_game"))();
    ((void (*)(void))sym(lib, "retro_deinit"))();
    return 0;
}
