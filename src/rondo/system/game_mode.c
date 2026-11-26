/*
 * The game's modes: FrameModeDispatch runs the handler for zGameMode each frame, and
 * most handlers run an overlay's frame until it sets zModeDone, then load what the
 * next mode needs and set it. Each handler gives back what its calls did; its wrapper
 * replays their exits in order, with the registers the game loads before each, so that
 * it leaves what the original leaves. An overlay's frame is original code: a shim runs it
 * and keeps what it left.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

enum {
    MPR4 = 0x10,
    MODE_ATTRACT_LOAD = 1, MODE_TITLE = 2, MODE_INTRO = 3, MODE_STAGE_LOAD = 4,
    MODE_STAGE = 5, MODE_DEAD = 7, MODE_GAME_OVER = 8, MODE_SCREEN = 9, MODE_ENDING = 10,
    SOUND_FADE = 0xFE, SOUND_STOP = 0xFF,
    SCENE_COMMON = 5, SCENE_MENU_A = 4, SCENE_MENU_B = 6, ADPCM_MENU = 0x24,
    SCENE_SCREENS_A = 7, SCENE_SCREENS_B = 8, SCENE_SCREENS_C = 9,
    SCENE_ENDING_A = 0x5C, SCENE_ENDING_B = 0x5D, SCENE_BOSS = 0x5E, ADPCM_BOSS = 0x22,
    SCENE_CLEAR = 0x58, STAGE_TITLE_BACK = 0x10,
    STAGE_BONUS = 0x8A, SECOND_CHARACTER_BYTES = 5, STAGE_SECRET = 0x80, EVENTS_BANK_OFFSET = 0x0D,
    CD_FADE_OUT = 0x0C, SCREEN_SIZE = 5, BOOT_DISPLAY_DELAY = 4, BOOT_SUB_MODE = 3,
    GAME_OVER_WAIT = 0x40, CONTINUES_CAP_HI = 0x0A, CONTINUES_MAX_LO = 0x99, CONTINUES_MAX_HI = 0x09,
    STAGE_RESTART = 0x40, STAGE_RETRY_FROM = 0x43, STAGE_RETRY_TO = 0x42,
    STAGES_ATTRACT = 5, JOY_RUN = 0x08, STOCK_AT_START = 3, SAVE_STAGE_BYTES = 0x14,
    EVENT_BOSS = 0x01, EVENT_SCREEN = 0x02, EVENT_CLEAR = 0x04, EVENT_SCREENS = 0x08,
    EVENT_CONTINUE = 0x10, SOUND_KEEP_TRACK = 0x18
};

/* The handlers' wrappers, which the dispatch reaches through its table. */
void abi_ModeBoot(void);
void abi_ModeLogo(void);
void abi_ModeAttract(void);
void abi_ModeAttractPlay(void);
void abi_ModeStageLoad(void);
void abi_ModeStage(void);
void abi_ModeStageCleared(void);
void abi_ModeDead(void);
void abi_ModeGameOverWait(void);
void abi_ModeScreen(void);
void abi_ModeEnding(void);

/* The save slot's flag that the bonus stage has been seen. */
#define SaveBonusSeen (SaveScoreTotal + 6)

/* A packed-decimal add, as ADC with D set: the sum and its carry. V is left alone. */
static Sum add_decimal(uint8_t a, uint8_t b, bool carry_in)
{
    Sum sum;
    unsigned low = (a & 0x0Fu) + (b & 0x0Fu) + (carry_in ? 1u : 0u);
    unsigned total;

    if (low >= 0x0A)
        low += 0x06;
    total = low + (a & 0xF0u) + (b & 0xF0u);
    if (total >= 0xA0)
        total += 0x60;
    sum.value = (uint8_t)total;
    sum.carry = total > 0xFF;
    sum.overflow = false;
    return sum;
}

/* An overlay's entry, run with the registers as the handler was entered with them. */
static Cpu overlay_shim(uint16_t entry, uint16_t ret)
{
    call_orig(ret, entry);
    return cpu;
}

/* An overlay's entry, with A just loaded: N and Z from it. */
static Cpu overlay_loaded_shim(uint16_t entry, uint8_t a)
{
    cpu.a = a;
    abi_nz(a);
    call_original(entry);
    return cpu;
}

/* An overlay called with a frame, A just loaded: N and Z from it. */
static Cpu overlay_loaded_frame_shim(uint16_t entry, uint8_t a, uint16_t ret)
{
    cpu.a = a;
    abi_nz(a);
    call_orig(ret, entry);
    return cpu;
}

/* An overlay's entry, with A the bank an add just made: N, Z, C and V from the add. */
static Cpu overlay_added_shim(uint16_t entry, Sum bank, uint16_t ret)
{
    cpu.a = bank.value;
    abi_nz(bank.value);
    abi_flag(P_C, bank.carry);
    abi_flag(P_V, bank.overflow);
    call_orig(ret, entry);
    return cpu;
}

/* The registers and flags original code left. */
static void restore_shim(Cpu after)
{
    cpu.a = after.a;
    cpu.x = after.x;
    cpu.y = after.y;
    cpu.p = after.p;
}

/* What the overlay left, then zModeDone loaded as the original loads it. */
static void overlay_done_shim(Cpu after, uint8_t done)
{
    restore_shim(after);
    cpu.a = done;
    abi_nz(done);
}

/* LDA of a value: A and N, Z. */
static void load_shim(uint8_t a)
{
    cpu.a = a;
    abi_nz(a);
}

/* LDA #mode, JSR SetGameMode: A 0, N and Z from the mode. */
static void set_mode_shim(uint8_t mode)
{
    load_shim(mode);
    cpu.a = 0;
}

/* NextGameMode's exit: A 0, N and Z from the new mode. */
static void next_mode_shim(uint8_t mode)
{
    cpu.a = 0;
    abi_nz(mode);
}

/* ResetScoreCounters' exit: A 3, N and Z from it. */
static void reset_score_shim(void)
{
    load_shim(STOCK_AT_START);
}

/* LDA #scene, JSR LoadSceneScript. */
static void scene_shim(uint8_t scene, const DiscLog *log)
{
    load_shim(scene);
    disc_log_exit_shim(log);
}

/* A compare's flags: N and Z from the difference, C from no borrow. */
static void compare_shim(uint8_t a, uint8_t b)
{
    abi_nz((uint8_t)(a - b));
    abi_flag(P_C, a >= b);
}

/* A stage's bit, ORed into a table byte at the index StageClearBit gave. */
typedef struct { StageBit found; uint8_t marked; } StageMark;

static StageMark mark_stage(uint16_t table, uint16_t ret)
{
    StageMark mark;
    uint16_t at;

    jsr_push(ret);
    mark.found = StageClearBit();
    jsr_pop();
    at = (uint16_t)(table + mark.found.index);
    mark.marked = (uint8_t)(mark.found.bit | rd8(at));
    wr8(at, mark.marked);
    return mark;
}

static void mark_stage_shim(StageMark mark)
{
    stage_bit_exit_shim(mark.found);
    load_shim(mark.marked);
}

/* The title's three scenes and the fixed ADPCM, as two paths load them. */
typedef struct { DiscLog a; DiscLog b; DiscLog c; DiscLog fixed; } ScreensLoad;

static ScreensLoad load_screens(uint16_t first_return)
{
    ScreensLoad load;

    wr8(zMusicWanted, 0);
    jsr_push(first_return);
    ResetGameState();
    jsr_pop();
    jsr_push((uint16_t)(first_return + 5));
    load.a = LoadSceneScript(SCENE_SCREENS_A);
    jsr_pop();
    jsr_push((uint16_t)(first_return + 10));
    load.b = LoadSceneScript(SCENE_SCREENS_B);
    jsr_pop();
    jsr_push((uint16_t)(first_return + 15));
    load.c = LoadSceneScript(SCENE_SCREENS_C);
    jsr_pop();
    jsr_push((uint16_t)(first_return + 18));
    load.fixed = AdpcmLoadFixed(disc_log_x(&load.c, 0));
    jsr_pop();
    return load;
}

static void screens_load_shim(const ScreensLoad *load)
{
    reset_game_state_exit_shim();
    scene_shim(SCENE_SCREENS_A, &load->a);
    scene_shim(SCENE_SCREENS_B, &load->b);
    scene_shim(SCENE_SCREENS_C, &load->c);
    disc_log_exit_shim(&load->fixed);
}

/* An overlay's frame: what it left, and the answer it gave in zModeDone. */
typedef struct { Cpu after; uint8_t done; } OverlayFrame;

static OverlayFrame overlay_frame(uint16_t entry, uint16_t ret)
{
    OverlayFrame frame;

    frame.after = overlay_shim(entry, ret);
    frame.done = rd8(zModeDone);
    return frame;
}

/*
 * @routine 0045:105 FrameModeDispatch
 * @run 69:5105
 * WHAT IT DOES: Runs this frame's game mode.
 * HOW: Counts zFrameTick, then jumps through GameModeHandlers by zGameMode.
 * Entry: nothing. Exit: as the handler leaves them; it is entered with A and X twice the mode,
 * N and Z from it, C the mode's bit 7.
 */
typedef struct { uint8_t mode; uint16_t handler; } ModeHandler;

ModeHandler FrameModeDispatch(void)
{
    ModeHandler found;
    uint8_t twice;

    wr8(zFrameTick, (uint8_t)(rd8(zFrameTick) + 1));
    found.mode = rd8(zGameMode);
    twice = (uint8_t)(found.mode << 1);
    found.handler = (uint16_t)(rd8((uint16_t)(GameModeHandlers + twice)) |
                               (rd8((uint16_t)(GameModeHandlers + twice + 1)) << 8));
    return found;
}

/* The handler at the address the table gave, through its C where it has C. */
static void mode_handler_shim(uint16_t handler)
{
    static const struct { uint16_t run; void (*abi)(void); } handlers[] = {
        { 0x5124, abi_ModeBoot }, { 0x514F, abi_ModeLogo }, { 0x515E, abi_ModeAttract },
        { 0x51B6, abi_ModeAttractPlay }, { 0x51DD, abi_ModeStageLoad }, { 0x5265, abi_ModeStage },
        { 0x52FE, abi_ModeStageCleared }, { 0x534E, abi_ModeDead },
        { 0x5432, abi_ModeGameOverWait }, { 0x544E, abi_ModeScreen }, { 0x549A, abi_ModeEnding }
    };

    for (unsigned i = 0; i < sizeof handlers / sizeof handlers[0]; i++)
        if (handlers[i].run == handler) {
            handlers[i].abi();
            return;
        }
    call_original(handler);
}

void abi_FrameModeDispatch(void)
{
    ModeHandler found = FrameModeDispatch();

    cpu.a = (uint8_t)(found.mode << 1);
    cpu.x = cpu.a;
    abi_nz(cpu.a);
    abi_flag(P_C, (found.mode & 0x80) != 0);
    mode_handler_shim(found.handler);
}

/*
 * @routine 0045:124 ModeBoot
 * @run 69:5124
 * WHAT IT DOES: Mode 0: the boot overlay's frame, then the display off, the screen size
 * set and on to the title; a negative zModeDone goes on to the title's sub-mode 3.
 * HOW: Overlay frame, WaitVblankDisplayOff, the card's ex_scrsiz, SetGameMode.
 * WAITS: yes, for the vertical blank, through WaitVblankDisplayOff; the stack there is the caller's return and this call's two bytes.
 * Entry: nothing. Exit, the overlay not done: as it leaves them, with A zModeDone, N and Z
 * from it. Exit: A zModeDone, or 3 if it was negative; N and Z from A; X zModeDone; Y and C as
 * ex_scrsiz leaves them; V as the overlay left it.
 */
typedef struct { OverlayFrame frame; uint8_t control; CardExit width; uint8_t done_again; } BootRun;

BootRun ModeBoot(void)
{
    BootRun run;

    run.frame = overlay_frame(FrameOverlay8000, 0x5127);
    if (run.frame.done == 0)
        return run;
    jsr_push(0x512F);
    run.control = WaitVblankDisplayOff();
    jsr_pop();
    wr8(zDisplayDelay, BOOT_DISPLAY_DELAY);
    jsr_push(0x5138);
    run.width = vec_ex_scrsiz(SCREEN_SIZE);
    jsr_pop();
    wr8(zStage, 0);
    wr8(zCharacterLoaded, 0xFF);
    wr8(zCharacter, 0);
    run.done_again = rd8(zModeDone);
    jsr_push(0x5147);
    SetGameMode(MODE_TITLE);
    jsr_pop();
    if (run.done_again & 0x80)
        wr8(zGameSubMode, BOOT_SUB_MODE);
    return run;
}

void abi_ModeBoot(void)
{
    BootRun run = ModeBoot();

    overlay_done_shim(run.frame.after, run.frame.done);
    if (run.frame.done == 0)
        return;
    load_shim(run.control);
    load_shim(SCREEN_SIZE);
    ex_scrsiz_exit_shim(run.width);
    cpu.x = run.done_again;
    load_shim(run.done_again & 0x80 ? BOOT_SUB_MODE : run.done_again);
}

/*
 * @routine 0045:14F ModeLogo
 * @run 69:514F
 * WHAT IT DOES: Mode 1: the overlay's frame, then the stage's load.
 * HOW: Overlay frame at its second entry; zField80 cleared; SetGameMode.
 * WAITS: no.
 * Entry: nothing. Exit: as the overlay leaves them, A zModeDone and N, Z from it; done, A 0.
 */
OverlayFrame ModeLogo(void)
{
    OverlayFrame frame = overlay_frame(FrameOverlay8003, 0x5152);

    if (frame.done == 0)
        return frame;
    wr8(zField80, 0);
    SetGameMode(MODE_STAGE_LOAD);
    return frame;
}

void abi_ModeLogo(void)
{
    OverlayFrame frame = ModeLogo();

    overlay_done_shim(frame.after, frame.done);
    if (frame.done != 0)
        set_mode_shim(MODE_STAGE_LOAD);
}

/*
 * @routine 0045:15E ModeAttract
 * @run 69:515E
 * WHAT IT DOES: Mode 2: the title's frame. The overlay's answer picks what comes next:
 * zero runs the overlay's other entry; positive restarts the attract's load; negative
 * starts the next demo stage, or on the first pass loads the title's scenes.
 * HOW: The overlay at $8006, then zAttractStep; the five demo stages are AttractStages.
 * WAITS: no.
 * Entry: nothing. Exit: as what ran last.
 */
typedef enum { ATTRACT_OTHER_ENTRY, ATTRACT_RESTART, ATTRACT_SCREENS, ATTRACT_DEMO } AttractPath;

typedef struct {
    OverlayFrame frame;
    AttractPath path;
    Cpu second;
    ScreensLoad screens;
    DiscLog scene;
    uint8_t step;
    uint8_t stage;
    StageEnterRun enter;
    uint8_t step_after;
    uint8_t mode;
} AttractRun;

AttractRun ModeAttract(void)
{
    AttractRun run;
    uint8_t step;

    run.frame = overlay_frame(FrameOverlay8006, 0x5161);
    if (!(run.frame.done & 0x80)) {
        if (run.frame.done == 0) {
            run.path = ATTRACT_OTHER_ENTRY;
            run.second = overlay_loaded_shim(FrameOverlay8009, run.frame.done);
        } else {
            run.path = ATTRACT_RESTART;
            SetGameMode(MODE_ATTRACT_LOAD);
        }
        return run;
    }
    if (rd8(zAttractStep) == 0) {
        run.path = ATTRACT_SCREENS;
        wr8(zAttractStep, (uint8_t)(rd8(zAttractStep) + 1));
        run.screens = load_screens(0x519D);
        run.mode = NextGameMode();
        return run;
    }
    run.path = ATTRACT_DEMO;
    jsr_push(0x5176);
    ResetScoreCounters();
    jsr_pop();
    wr8(zCharacter, 0);
    jsr_push(0x517D);
    run.scene = LoadSceneScript(SCENE_COMMON);
    jsr_pop();
    run.step = rd8(zAttractStep);
    run.stage = rd8((uint16_t)(AttractStages + run.step));
    wr8(zStage, run.stage);
    jsr_push(0x5187);
    run.enter = StageEnter();
    jsr_pop();
    step = (uint8_t)(rd8(zAttractStep) + 1);
    wr8(zAttractStep, step);
    run.step_after = rd8(zAttractStep);
    if (run.step_after == STAGES_ATTRACT)
        wr8(zAttractStep, 0);
    wr8(zGameRunning, (uint8_t)(rd8(zGameRunning) + 1));
    run.mode = NextGameMode();
    return run;
}

void abi_ModeAttract(void)
{
    AttractRun run = ModeAttract();

    overlay_done_shim(run.frame.after, run.frame.done);
    switch (run.path) {
    case ATTRACT_OTHER_ENTRY:
        restore_shim(run.second);
        return;
    case ATTRACT_RESTART:
        set_mode_shim(MODE_ATTRACT_LOAD);
        return;
    case ATTRACT_SCREENS:
        load_shim(0);
        screens_load_shim(&run.screens);
        next_mode_shim(run.mode);
        return;
    case ATTRACT_DEMO:
        load_shim(run.step);
        reset_score_shim();
        scene_shim(SCENE_COMMON, &run.scene);
        cpu.y = run.step;
        load_shim(run.stage);
        stage_enter_exit_shim(&run.enter);
        cpu.a = run.step_after;
        compare_shim(run.step_after, STAGES_ATTRACT);
        next_mode_shim(run.mode);
        return;
    }
}

/*
 * @routine 0045:1B6 ModeAttractPlay
 * @run 69:51B6
 * WHAT IT DOES: Mode 3: a demo's title frames; Run or the overlay's end stops the demo,
 * fades the CD and goes to mode 8.
 * HOW: The card's joypad byte, the overlay's frame, a sound fade, the card's cd_fade.
 * WAITS: yes, through ResetGameState (the vertical blank); the stack there is the mode loop's return and the call's frame.
 * Entry: X and Y are the caller's; the overlay's leave goes on to SoundCommand. Exit: as what ran last.
 */
typedef struct {
    uint8_t run_pressed;
    OverlayFrame frame;
    CommandCall sound;
} AttractPlayRun;

AttractPlayRun ModeAttractPlay(uint8_t kept_x, uint8_t kept_y)
{
    AttractPlayRun run;

    run.run_pressed = rd8(BiosJoyTrigger) & JOY_RUN;
    if (run.run_pressed == 0) {
        run.frame.after = overlay_loaded_frame_shim(FrameOverlay8000, 0, 0x51C0);
        run.frame.done = rd8(zModeDone);
        if (run.frame.done == 0)
            return run;
        kept_x = run.frame.after.x;
        kept_y = run.frame.after.y;
    }
    wr8(zGameRunning, 0);
    jsr_push(0x51CC);
    run.sound = SoundCommand(SOUND_FADE, kept_x, kept_y);
    jsr_pop();
    jsr_push(0x51D1);
    vec_cd_fade(CD_FADE_OUT);
    jsr_pop();
    wr8(zWaitTimer, GAME_OVER_WAIT);
    jsr_push(0x51D8);
    ResetGameState();
    jsr_pop();
    SetGameMode(MODE_GAME_OVER);
    return run;
}

void abi_ModeAttractPlay(void)
{
    AttractPlayRun run = ModeAttractPlay(cpu.x, cpu.y);

    if (run.run_pressed == 0) {
        overlay_done_shim(run.frame.after, run.frame.done);
        if (run.frame.done == 0)
            return;
    } else {
        load_shim(run.run_pressed);
    }
    load_shim(SOUND_FADE);
    sound_command_exit_shim(SOUND_FADE, run.sound);
    load_shim(CD_FADE_OUT);
    cd_fade_exit_shim(CD_FADE_OUT);
    load_shim(GAME_OVER_WAIT);
    reset_game_state_exit_shim();
    set_mode_shim(MODE_GAME_OVER);
}

/*
 * @routine 0045:1F0 StageEnterNext
 * @run 69:51F0
 * WHAT IT DOES: Enters the stage in zStage with a clean score count and goes on a mode.
 * HOW: StageEnter, ResetScoreCounters, NextGameMode.
 * WAITS: no.
 * Entry: nothing. Exit: as NextGameMode.
 */
typedef struct { StageEnterRun enter; uint8_t mode; } EnterNextRun;

EnterNextRun StageEnterNext(void)
{
    EnterNextRun run;

    jsr_push(0x51F3);
    run.enter = StageEnter();
    jsr_pop();
    jsr_push(0x51F6);
    ResetScoreCounters();
    jsr_pop();
    run.mode = NextGameMode();
    return run;
}

static void enter_next_exit_shim(const EnterNextRun *run)
{
    stage_enter_exit_shim(&run->enter);
    reset_score_shim();
    next_mode_shim(run->mode);
}

void abi_StageEnterNext(void)
{
    EnterNextRun run = StageEnterNext();

    enter_next_exit_shim(&run);
}

/*
 * @routine 0045:1F9 ModeStageLoadScreens
 * @run 69:51F9
 * WHAT IT DOES: The path of ModeStageLoad for bit 3 of zModeEvents: loads the screens'
 * scenes and goes to mode 10.
 * HOW: Clears the bit and the wanted ADPCM, ResetGameState, three scenes, the fixed ADPCM.
 * WAITS: no.
 * Entry: from ModeStageLoad. Exit: A 0, N 0, Z 0, from SetGameMode's mode; the rest as
 * AdpcmLoadFixed leaves them.
 */
ScreensLoad ModeStageLoadScreens(void)
{
    ScreensLoad load;

    wr8(zModeEvents, (uint8_t)(rd8(zModeEvents) & ~EVENT_SCREENS));
    load = load_screens(0x5200);
    SetGameMode(MODE_ENDING);
    return load;
}

static void load_screens_exit_shim(const ScreensLoad *load)
{
    screens_load_shim(load);
    set_mode_shim(MODE_ENDING);
}

void abi_ModeStageLoadScreens(void)
{
    ScreensLoad load = ModeStageLoadScreens();

    load_screens_exit_shim(&load);
}

/*
 * @routine 0045:217 ModeStageLoadContinue
 * @run 69:5217
 * WHAT IT DOES: The path of ModeStageLoad for bit 4 of zModeEvents: enters the stage and
 * goes to the intro mode.
 * HOW: ResetScoreCounters, a scene, StageEnter, zGameRunning counted, SetGameMode.
 * WAITS: no.
 * Entry: from ModeStageLoad. Exit: A 0, N 0, Z 0; the rest as StageEnter leaves them.
 */
typedef struct { DiscLog scene; StageEnterRun enter; } ContinueRun;

ContinueRun ModeStageLoadContinue(void)
{
    ContinueRun run;

    jsr_push(0x521A);
    ResetScoreCounters();
    jsr_pop();
    jsr_push(0x521F);
    run.scene = LoadSceneScript(SCENE_COMMON);
    jsr_pop();
    jsr_push(0x5222);
    run.enter = StageEnter();
    jsr_pop();
    wr8(zGameRunning, (uint8_t)(rd8(zGameRunning) + 1));
    SetGameMode(MODE_INTRO);
    return run;
}

static void continue_exit_shim(const ContinueRun *run)
{
    reset_score_shim();
    scene_shim(SCENE_COMMON, &run->scene);
    stage_enter_exit_shim(&run->enter);
    set_mode_shim(MODE_INTRO);
}

void abi_ModeStageLoadContinue(void)
{
    ContinueRun run = ModeStageLoadContinue();

    continue_exit_shim(&run);
}

/*
 * @routine 0045:1DD ModeStageLoad
 * @run 69:51DD
 * WHAT IT DOES: Mode 4: the menu overlay's frame, then what zModeEvents asks for: the
 * name entry, a new game from the menu, or the first stage.
 * HOW: The overlay's fifth entry, then bits 3 and 4 of zModeEvents.
 * WAITS: no.
 * Entry: nothing. Exit: as what ran last.
 */
typedef enum { STAGE_LOAD_WAIT, STAGE_LOAD_SCREENS, STAGE_LOAD_CONTINUE, STAGE_LOAD_FIRST } StageLoadPath;

typedef struct {
    OverlayFrame frame;
    StageLoadPath path;
    ScreensLoad screens;
    ContinueRun resume;
    DiscLog scene;
    EnterNextRun next;
} ModeStageLoadRun;

ModeStageLoadRun ModeStageLoad(void)
{
    ModeStageLoadRun run;

    run.frame = overlay_frame(OverlayTerrainLookup, 0x51E0);
    run.path = STAGE_LOAD_WAIT;
    if (run.frame.done == 0)
        return run;
    if (rd8(zModeEvents) & EVENT_SCREENS) {
        run.path = STAGE_LOAD_SCREENS;
        run.screens = ModeStageLoadScreens();
    } else if (rd8(zModeEvents) & EVENT_CONTINUE) {
        run.path = STAGE_LOAD_CONTINUE;
        run.resume = ModeStageLoadContinue();
    } else {
        run.path = STAGE_LOAD_FIRST;
        jsr_push(0x51F0);
        run.scene = LoadSceneScript(SCENE_COMMON);
        jsr_pop();
        run.next = StageEnterNext();
    }
    return run;
}

void abi_ModeStageLoad(void)
{
    ModeStageLoadRun run = ModeStageLoad();

    overlay_done_shim(run.frame.after, run.frame.done);
    switch (run.path) {
    case STAGE_LOAD_WAIT:
        return;
    case STAGE_LOAD_SCREENS:
        load_screens_exit_shim(&run.screens);
        return;
    case STAGE_LOAD_CONTINUE:
        continue_exit_shim(&run.resume);
        return;
    case STAGE_LOAD_FIRST:
        scene_shim(SCENE_COMMON, &run.scene);
        enter_next_exit_shim(&run.next);
        return;
    }
}

/*
 * @routine 0045:3D7 SaveProgress
 * @run 69:53D7
 * WHAT IT DOES: Writes the character's progress to the save slot in zSaveSlot.
 * HOW: Reads the backup file to RAM, copies into the slot, writes it back.
 * Entry: nothing. Exit: as BackupWrite, entered with X past the slot's stage bytes, Y $14, A
 * the last byte copied, Z and C set.
 */
typedef struct {
    BackupAccess read;
    uint8_t slot;
    uint8_t offset;
    uint8_t last;
    BackupWriteRun write;
} SaveRun;

/* The save slot's copy of the character's progress, from the working bytes. */
static void save_progress_tail(SaveRun *run)
{
    wr8((uint16_t)(SaveBuffer + run->offset), rd8(ProgressCharacter));
    wr8((uint16_t)(SaveContinues + run->offset), rd8(ContinuesLo));
    wr8((uint16_t)(SaveContinues + 1 + run->offset), rd8(ContinuesHi));
    for (uint8_t i = 0; i < SAVE_STAGE_BYTES; i++) {
        run->last = rd8((uint16_t)(StagesReached + i));
        wr8((uint16_t)(SaveBuffer + 1 + run->offset + i), run->last);
    }
    run->write = BackupWrite();
}

static SaveRun save_slot_open(void)
{
    SaveRun run;

    run.read = BackupRead();
    run.slot = rd8(zSaveSlot);
    run.offset = rd8((uint16_t)(SaveSlotOffsets + run.slot));
    return run;
}

SaveRun SaveProgress(void)
{
    SaveRun run = save_slot_open();

    save_progress_tail(&run);
    return run;
}

static void save_exit_shim(const SaveRun *run)
{
    backup_read_exit_shim(run->read);
    cpu.y = SAVE_STAGE_BYTES;
    cpu.x = (uint8_t)(run->offset + SAVE_STAGE_BYTES);
    cpu.a = run->last;
    compare_shim(SAVE_STAGE_BYTES, SAVE_STAGE_BYTES);
    backup_write_exit_shim(&run->write);
}

void abi_SaveProgress(void)
{
    SaveRun run = SaveProgress();

    save_exit_shim(&run);
}

/*
 * @routine 0045:3E1 SaveScore
 * @run 69:53E1
 * WHAT IT DOES: As SaveProgress, and first adds the stage's score to the slot's total, in
 * BCD, stopping at 999999.
 * HOW: A 24-bit BCD add from zScoreAddLo-Hi.
 * Entry: nothing. Exit: as SaveProgress.
 */
SaveRun SaveScore(void)
{
    SaveRun run = save_slot_open();
    uint16_t total = (uint16_t)(SaveScoreTotal + run.offset);
    Sum sum;

    sum = add_decimal(rd8(zScoreAddLo), rd8(total), false);
    wr8(total, sum.value);
    sum = add_decimal(rd8(zScoreAddHi), rd8((uint16_t)(total + 1)), sum.carry);
    wr8((uint16_t)(total + 1), sum.value);
    sum = add_decimal(0, rd8((uint16_t)(total + 2)), sum.carry);
    wr8((uint16_t)(total + 2), sum.value);
    if (sum.carry) {
        wr8(total, 0x99);
        wr8((uint16_t)(total + 1), 0x99);
        wr8((uint16_t)(total + 2), 0x99);
    }
    save_progress_tail(&run);
    return run;
}

void abi_SaveScore(void)
{
    SaveRun run = SaveScore();

    save_exit_shim(&run);
}

/*
 * @routine 0045:229 ModeStageDone
 * @run 69:5229
 * WHAT IT DOES: The stage's overlay finished with a positive answer: goes on a mode,
 * except on the bonus stage, which marks it and, unless the save slot's flag is set,
 * loads the ending's scenes and goes to mode 9.
 * HOW: Compares zStage; StageClearBit in StagesVisited; the save's flag at SaveBonusSeen.
 * WAITS: yes, through ResetGameState (the vertical blank), the scene loads (the drive); the stack there is the mode loop's return and the call frames.
 * Entry: from ModeStage. Exit: as what ran last.
 */
typedef struct {
    uint8_t stage;
    StageMark mark;
    StopCall stop;
    uint8_t slot;
    bool saved;
    SaveRun save;
    uint8_t save_slot;
    uint8_t offset;
    uint8_t seen;
    bool next;
    uint8_t mode;
    DiscLog ending_a;
    DiscLog ending_b;
} StageDoneRun;

StageDoneRun ModeStageDone(void)
{
    StageDoneRun run;

    run.stage = rd8(zStage);
    run.saved = false;
    run.next = run.stage != STAGE_BONUS;
    if (run.next) {
        run.mode = NextGameMode();
        return run;
    }
    run.mark = mark_stage(StagesVisited, 0x5235);
    ResetGameState();
    run.stop = SoundStopAll(0, 0xFF);    /* X and Y as ResetGameState leaves them */
    run.slot = rd8(zSaveSlot);
    if (!(run.slot & 0x80)) {
        run.saved = true;
        run.save = SaveProgress();
        run.save_slot = rd8(zSaveSlot);
        run.offset = rd8((uint16_t)(SaveSlotOffsets + run.save_slot));
        run.seen = rd8((uint16_t)(SaveBonusSeen + run.offset));
        run.next = run.seen != 0;
        if (run.next) {
            run.mode = NextGameMode();
            return run;
        }
    }
    wr8(zCharacterLoaded, 0xFF);
    run.ending_a = LoadSceneScript(SCENE_ENDING_A);
    run.ending_b = LoadSceneScript(SCENE_ENDING_B);
    SetGameMode(MODE_SCREEN);
    return run;
}

static void stage_done_exit_shim(const StageDoneRun *run)
{
    load_shim(run->stage);
    compare_shim(run->stage, STAGE_BONUS);
    if (run->stage != STAGE_BONUS) {
        next_mode_shim(run->mode);
        return;
    }
    mark_stage_shim(run->mark);
    reset_game_state_exit_shim();
    stop_call_exit_shim(run->stop);
    load_shim(run->slot);
    if (run->saved) {
        save_exit_shim(&run->save);
        cpu.y = run->save_slot;
        cpu.x = run->offset;
        load_shim(run->seen);
        if (run->next) {
            next_mode_shim(run->mode);
            return;
        }
    }
    load_shim(0xFF);
    scene_shim(SCENE_ENDING_A, &run->ending_a);
    scene_shim(SCENE_ENDING_B, &run->ending_b);
    set_mode_shim(MODE_SCREEN);
}

void abi_ModeStageDone(void)
{
    StageDoneRun run = ModeStageDone();

    stage_done_exit_shim(&run);
}

/*
 * @routine 0045:265 ModeStage
 * @run 69:5265
 * WHAT IT DOES: Mode 5: a stage's frame. A negative answer goes to mode 7, a positive one
 * to ModeStageDone. At zero, bits of zModeEvents ask for a boss's ADPCM and scene, a
 * screen's scenes, or the stage's clear with the score saved.
 * HOW: Bits 0, 1 and 2 of zModeEvents, each cleared as it is taken.
 * WAITS: no.
 * Entry: nothing. Exit: as what ran last.
 */
typedef enum {
    STAGE_PLAYING, STAGE_LOST, STAGE_DONE, STAGE_BOSS, STAGE_SCREEN, STAGE_CLEAR
} StagePath;

typedef struct {
    OverlayFrame frame;
    StagePath path;
    StageDoneRun done;
    Sum adpcm_sum;
    AdpcmLoadRun adpcm;
    uint8_t stage;
    uint8_t first;
    DiscLog scene_a;
    DiscLog scene_b;
    StageMark mark;
    uint8_t slot;
    SaveRun save;
    Sum clear_a;
    Sum clear_b;
} StageRun;

/* The two scenes of a screen between stages, by the stage that asks for it. */
static uint8_t screen_scenes(uint8_t stage)
{
    switch (stage) {
    case 0x04:
        return 0x02;
    case 0x07:
        return 0x0A;
    case 0x09:
        return 0x0C;
    default:
        return 0x0E;
    }
}

StageRun ModeStage(void)
{
    StageRun run;
    uint8_t character;

    run.frame = overlay_frame(FrameOverlay8000, 0x5268);
    if (run.frame.done & 0x80) {
        run.path = STAGE_LOST;
        SetGameMode(MODE_DEAD);
    } else if (run.frame.done != 0) {
        run.path = STAGE_DONE;
        run.done = ModeStageDone();
    } else if (rd8(zModeEvents) & EVENT_BOSS) {
        run.path = STAGE_BOSS;
        wr8(zModeEvents, (uint8_t)(rd8(zModeEvents) & ~EVENT_BOSS));
        run.adpcm_sum = add_bytes(ADPCM_BOSS, rd8(zCharacter), false);
        run.adpcm = AdpcmLoad(run.adpcm_sum.value);
        run.scene_a = LoadSceneScript(SCENE_BOSS);
    } else if (rd8(zModeEvents) & EVENT_SCREEN) {
        run.path = STAGE_SCREEN;
        wr8(zModeEvents, (uint8_t)(rd8(zModeEvents) & ~EVENT_SCREEN));
        ResetGameState();
        wr8(zMusicWanted, 0xFF);
        wr8(zCharacterLoaded, 0xFF);
        run.stage = rd8(zStage);
        run.first = screen_scenes(run.stage);
        run.scene_a = LoadSceneScript(run.first);
        run.scene_b = LoadSceneScript((uint8_t)(run.first + 1));
        SetGameMode(MODE_SCREEN);
    } else if (rd8(zModeEvents) & EVENT_CLEAR) {
        run.path = STAGE_CLEAR;
        wr8(zModeEvents, (uint8_t)(rd8(zModeEvents) & ~EVENT_CLEAR));
        run.mark = mark_stage(StagesVisited, 0x52DC);
        run.slot = rd8(zSaveSlot);
        if (!(run.slot & 0x80))
            run.save = SaveScore();
        character = rd8(zCharacter);
        run.clear_a = add_bytes((uint8_t)(character << 1), SCENE_CLEAR, (character & 0x80) != 0);
        run.scene_a = LoadSceneScript(run.clear_a.value);
        character = rd8(zCharacter);
        run.clear_b = add_bytes((uint8_t)(character << 1), SCENE_CLEAR + 1, (character & 0x80) != 0);
        run.scene_b = LoadSceneScript(run.clear_b.value);
        SetGameMode(MODE_SCREEN);
    } else {
        run.path = STAGE_PLAYING;
    }
    return run;
}

static void sum_shim(Sum sum)
{
    load_shim(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

void abi_ModeStage(void)
{
    StageRun run = ModeStage();

    overlay_done_shim(run.frame.after, run.frame.done);
    switch (run.path) {
    case STAGE_PLAYING:
        return;
    case STAGE_LOST:
        set_mode_shim(MODE_DEAD);
        return;
    case STAGE_DONE:
        stage_done_exit_shim(&run.done);
        return;
    case STAGE_BOSS:
        sum_shim(run.adpcm_sum);
        adpcm_load_exit_shim(&run.adpcm);
        scene_shim(SCENE_BOSS, &run.scene_a);
        return;
    case STAGE_SCREEN:
        reset_game_state_exit_shim();
        load_shim(run.stage);
        if (run.stage == 0x04 || run.stage == 0x07 || run.stage == 0x09)
            compare_shim(0, 0);
        else
            compare_shim(run.stage, 0x09);
        scene_shim(run.first, &run.scene_a);
        scene_shim((uint8_t)(run.first + 1), &run.scene_b);
        set_mode_shim(MODE_SCREEN);
        return;
    case STAGE_CLEAR:
        mark_stage_shim(run.mark);
        load_shim(run.slot);
        if (!(run.slot & 0x80))
            save_exit_shim(&run.save);
        sum_shim(run.clear_a);
        disc_log_exit_shim(&run.scene_a);
        sum_shim(run.clear_b);
        disc_log_exit_shim(&run.scene_b);
        set_mode_shim(MODE_SCREEN);
        return;
    }
}

/*
 * @routine 0045:306 StageClearedNext
 * @run 69:5306
 * WHAT IT DOES: Enters the next stage, marks it and saves, and goes to the stage mode; if
 * there is no next stage, back to the title's mode 0.
 * HOW: StageEnter; zStage zero is the end. StageClearBit into StagesReached.
 * WAITS: no.
 * Entry: nothing. Exit: A 0; N and Z from mode 5, or from zStage zero; the rest as what ran
 * last.
 */
typedef struct {
    StageEnterRun enter;
    uint8_t stage;
    StageMark mark;
    uint8_t slot;
    SaveRun save;
} ClearedNextRun;

ClearedNextRun StageClearedNext(void)
{
    ClearedNextRun run;

    jsr_push(0x5309);
    run.enter = StageEnter();
    jsr_pop();
    run.stage = rd8(zStage);
    if (run.stage == 0) {
        SetGameMode(0);
        return run;
    }
    run.mark = mark_stage(StagesReached, 0x530F);
    run.slot = rd8(zSaveSlot);
    if (!(run.slot & 0x80)) {
        jsr_push(0x531C);
        run.save = SaveProgress();
        jsr_pop();
    }
    SetGameMode(MODE_STAGE);
    return run;
}

static void cleared_next_exit_shim(const ClearedNextRun *run)
{
    stage_enter_exit_shim(&run->enter);
    load_shim(run->stage);
    if (run->stage == 0) {
        cpu.a = 0;
        return;
    }
    mark_stage_shim(run->mark);
    load_shim(run->slot);
    if (!(run->slot & 0x80))
        save_exit_shim(&run->save);
    set_mode_shim(MODE_STAGE);
}

void abi_StageClearedNext(void)
{
    ClearedNextRun run = StageClearedNext();

    cleared_next_exit_shim(&run);
}

/*
 * @routine 0045:2FE ModeStageCleared
 * @run 69:52FE
 * WHAT IT DOES: Mode 6: marks the stage visited and runs into StageClearedNext.
 * HOW: StageClearBit into StagesVisited.
 * WAITS: no.
 * Entry: nothing. Exit: as StageClearedNext.
 */
typedef struct { StageMark mark; ClearedNextRun next; } StageClearedRun;

StageClearedRun ModeStageCleared(void)
{
    StageClearedRun run;

    run.mark = mark_stage(StagesVisited, 0x5300);
    run.next = StageClearedNext();
    return run;
}

void abi_ModeStageCleared(void)
{
    StageClearedRun run = ModeStageCleared();

    mark_stage_shim(run.mark);
    cleared_next_exit_shim(&run.next);
}

/*
 * @routine 0045:325 StageClearBit
 * @run 69:5325
 * WHAT IT DOES: Finds zStage's bit in the visited tables: the byte in X, the bit in A.
 * HOW: Bits 2-4 of the stage give the byte, five further on for the second character;
 * bits 0-1 the bit; a stage with bit 7 set takes the high nibble.
 * WAITS: no.
 * Entry: nothing. Exit: A the bit, X the byte, Y zStage, N and Z from Y, or from the bit with
 * bit 7 set; C and V clear, the add and the shifts never carrying. Clobbers zTemp00.
 */
StageBit StageClearBit(void)
{
    StageBit found;
    uint8_t stage;
    uint8_t base = rd8(zCharacter) != 0 ? SECOND_CHARACTER_BYTES : 0;

    wr8(zTemp00, base);
    stage = rd8(zStage);
    base = rd8(zTemp00);
    found.index = (uint8_t)(((stage >> 2) & 0x07) + base);
    found.bit = (uint8_t)(1 << (rd8(zStage) & 0x03));
    found.stage = rd8(zStage);
    if (found.stage & STAGE_SECRET)
        found.bit = (uint8_t)(found.bit << 4);
    return found;
}

/* StageClearBit's exit, for its wrapper and for a caller's that replays it. */
void stage_bit_exit_shim(StageBit found)
{
    cpu.a = found.bit;
    cpu.x = found.index;
    cpu.y = found.stage;
    abi_nz((found.stage & STAGE_SECRET) ? found.bit : found.stage);
    abi_flag(P_C, false);
    abi_flag(P_V, false);
}

void abi_StageClearBit(void)
{
    stage_bit_exit_shim(StageClearBit());
}

/*
 * @routine 0045:3B7 ModeBackToTitle
 * @run 69:53B7
 * WHAT IT DOES: Goes back to the title: forgets what is loaded, loads the menu's scenes
 * and ADPCM, and sets mode 0.
 * HOW: Two scenes, an ADPCM, SetGameMode.
 * WAITS: no itself; the two scene loads and the ADPCM load wait on the disc, stack 2 deep each.
 * Entry: nothing. Exit: A 0; the rest as AdpcmLoad leaves them.
 */
typedef struct { DiscLog menu_a; DiscLog menu_b; AdpcmLoadRun adpcm; } BackToTitleRun;

BackToTitleRun ModeBackToTitle(void)
{
    BackToTitleRun run;

    wr8(zMusicWanted, 0xFF);
    wr8(zCharacterLoaded, 0xFF);
    wr8(zStage, 0);
    jsr_push(0x53C6);
    run.menu_a = LoadSceneScript(SCENE_MENU_A);
    jsr_pop();
    jsr_push(0x53CB);
    run.menu_b = LoadSceneScript(SCENE_MENU_B);
    jsr_pop();
    jsr_push(0x53D0);
    run.adpcm = AdpcmLoad(ADPCM_MENU);
    jsr_pop();
    SetGameMode(0);
    return run;
}

static void back_to_title_exit_shim(const BackToTitleRun *run)
{
    scene_shim(SCENE_MENU_A, &run->menu_a);
    scene_shim(SCENE_MENU_B, &run->menu_b);
    load_shim(ADPCM_MENU);
    adpcm_load_exit_shim(&run->adpcm);
    cpu.a = 0;
}

void abi_ModeBackToTitle(void)
{
    BackToTitleRun run = ModeBackToTitle();

    back_to_title_exit_shim(&run);
}

/*
 * @routine 0045:34E ModeDead
 * @run 69:534E
 * WHAT IT DOES: Mode 7: the continue screen's frame. Then counts a continue used, in
 * BCD up to 999, saves, and goes back into the stage, or at the last continue to the
 * title. A stage with bit 7 set restarts from zStageRequested.
 * HOW: Maps the overlay's bank; the continues at ContinuesLo-Hi; ContinueChoice picks what
 * follows.
 * WAITS: yes, through the events overlay, SaveScore, ResetGameState and the stage enter; the stack there is the mode loop's return and the call frames.
 * Entry: nothing. Exit: as what ran last.
 */
typedef struct {
    Cpu after;
    uint8_t done;
    uint8_t continues_hi;
    uint8_t slot;
    SaveRun save;
    uint8_t choice;
    BackToTitleRun title;
    uint8_t character;
    uint8_t stage;
    bool from_request;
    uint8_t requested;
    uint8_t restart;
    uint8_t retry;
    EnterNextRun next;
} DeadRun;

DeadRun ModeDead(void)
{
    DeadRun run;
    Sum bank = add_bytes(EVENTS_BANK_OFFSET, rd8(zRamBaseBank), false);
    Sum continues;

    tam(MPR4, bank.value);
    run.after = overlay_added_shim(FrameOverlay8000, bank, 0x5358);
    run.done = rd8(zModeDone);
    if (run.done == 0)
        return run;
    continues = add_decimal(rd8(ContinuesLo), 1, false);
    wr8(ContinuesLo, continues.value);
    if (continues.carry)
        wr8(ContinuesHi, (uint8_t)(rd8(ContinuesHi) + 1));
    run.continues_hi = rd8(ContinuesHi);
    if (run.continues_hi == CONTINUES_CAP_HI) {
        wr8(ContinuesLo, CONTINUES_MAX_LO);
        wr8(ContinuesHi, CONTINUES_MAX_HI);
    }
    run.slot = rd8(zSaveSlot);
    if (!(run.slot & 0x80)) {
        jsr_push(0x5385);
        run.save = SaveScore();
        jsr_pop();
    }
    run.choice = rd8(ContinueChoice);
    if (run.choice == 1) {
        jsr_push(0x53B7);
        ResetGameState();
        jsr_pop();
        run.title = ModeBackToTitle();
        return run;
    }
    if (run.choice != 0) {
        run.character = rd8(zCharacter) ^ 0x01;
        wr8(zCharacter, run.character);
    }
    run.stage = rd8(zStage);
    run.from_request = (run.stage & 0x80) != 0;
    if (run.from_request)
        run.requested = rd8(zStageRequested) & 0x7F;
    run.restart = (uint8_t)((run.from_request ? run.requested : run.stage) | STAGE_RESTART);
    wr8(zStage, run.restart);
    jsr_push(0x53A2);
    ResetScoreCounters();
    jsr_pop();
    run.retry = rd8(zStage);
    if (run.retry == STAGE_RETRY_FROM)
        wr8(zStage, STAGE_RETRY_TO);
    jsr_push(0x53B1);
    SetGameMode(MODE_STAGE_LOAD);
    jsr_pop();
    run.next = StageEnterNext();
    return run;
}

void abi_ModeDead(void)
{
    DeadRun run = ModeDead();

    overlay_done_shim(run.after, run.done);
    if (run.done == 0)
        return;
    compare_shim(run.continues_hi, CONTINUES_CAP_HI);
    load_shim(run.slot);
    if (!(run.slot & 0x80))
        save_exit_shim(&run.save);
    load_shim(run.choice);
    if (run.choice == 1) {
        abi_nz(0);
        reset_game_state_exit_shim();
        back_to_title_exit_shim(&run.title);
        return;
    }
    if (run.choice != 0)
        load_shim(run.character);
    load_shim(run.stage);
    if (run.from_request)
        load_shim(run.requested);
    load_shim(run.restart);
    reset_score_shim();
    load_shim(run.retry);
    compare_shim(run.retry, STAGE_RETRY_FROM);
    set_mode_shim(MODE_STAGE_LOAD);
    enter_next_exit_shim(&run.next);
}

/*
 * @routine 0045:432 ModeGameOverWait
 * @run 69:5432
 * WHAT IT DOES: Mode 8: waits out zWaitTimer, stops the sound, and goes back to the title;
 * with bit 4 of zModeEvents, to the stage's load from the start.
 * HOW: Counts down; SoundStopAll; ModeBackToTitle.
 * WAITS: yes, through ModeBackToTitle (ResetGameState and the title's loads); the stack there is the mode loop's return and the call frames.
 * Entry: nothing. Exit: on waiting, A kept, N and Z from the count; else as what ran last.
 */
typedef struct { uint8_t timer; StopCall stop; bool restart; BackToTitleRun title; } GameOverRun;

GameOverRun ModeGameOverWait(uint8_t x, uint8_t y)
{
    GameOverRun run;

    run.timer = (uint8_t)(rd8(zWaitTimer) - 1);
    wr8(zWaitTimer, run.timer);
    if (run.timer != 0)
        return run;
    jsr_push(0x543A);
    run.stop = SoundStopAll(x, y);
    jsr_pop();
    run.restart = (rd8(zModeEvents) & EVENT_CONTINUE) != 0;
    if (!run.restart) {
        run.title = ModeBackToTitle();
        return run;
    }
    wr8(zModeEvents, (uint8_t)(rd8(zModeEvents) & ~EVENT_CONTINUE));
    jsr_push(0x5445);
    run.title = ModeBackToTitle();
    jsr_pop();
    wr8(zCharacter, 0);
    wr8(zField80, 0);
    SetGameMode(MODE_STAGE_LOAD);
    return run;
}

void abi_ModeGameOverWait(void)
{
    GameOverRun run = ModeGameOverWait(cpu.x, cpu.y);

    abi_nz(run.timer);
    if (run.timer != 0)
        return;
    stop_call_exit_shim(run.stop);
    back_to_title_exit_shim(&run.title);
    if (run.restart)
        set_mode_shim(MODE_STAGE_LOAD);
}

/*
 * @routine 0045:44E ModeScreen
 * @run 69:544E
 * WHAT IT DOES: Mode 9: a screen's frame between stages, then what follows it: back to
 * the title from stage $10; from the bonus stage the save's flag and the next stage;
 * otherwise the stage's load again with the player's stock restored.
 * HOW: Overlay frame, then zStage.
 * WAITS: yes, through ResetGameState (the vertical blank) and the loads (the drive); the stack there is the mode loop's return and the call frames.
 * Entry: nothing. Exit: as what ran last.
 */
typedef enum { SCREEN_WAIT, SCREEN_TITLE, SCREEN_BONUS, SCREEN_STAGE } ScreenPath;

typedef struct {
    OverlayFrame frame;
    ScreenPath path;
    DiscLog scene;
    uint8_t stage;
    BackToTitleRun title;
    uint8_t slot;
    BackupAccess read;
    uint8_t save_slot;
    uint8_t offset;
    BackupWriteRun write;
    ClearedNextRun next;
    StageLoadRun load;
    uint8_t field_a3;
} ScreenRun;

ScreenRun ModeScreen(void)
{
    ScreenRun run;

    run.frame = overlay_frame(FrameOverlay8000, 0x5451);
    run.path = SCREEN_WAIT;
    if (run.frame.done == 0)
        return run;
    jsr_push(0x5459);
    ResetGameState();
    jsr_pop();
    jsr_push(0x545E);
    run.scene = LoadSceneScript(SCENE_COMMON);
    jsr_pop();
    run.stage = rd8(zStage);
    if (run.stage == STAGE_TITLE_BACK) {
        run.path = SCREEN_TITLE;
        run.title = ModeBackToTitle();
        return run;
    }
    if (run.stage == STAGE_BONUS) {
        run.path = SCREEN_BONUS;
        run.slot = rd8(zSaveSlot);
        if (!(run.slot & 0x80)) {
            run.read = BackupRead();
            run.save_slot = rd8(zSaveSlot);
            run.offset = rd8((uint16_t)(SaveSlotOffsets + run.save_slot));
            wr8((uint16_t)(SaveBonusSeen + run.offset), 1);
            run.write = BackupWrite();
        }
        run.next = StageClearedNext();
        return run;
    }
    run.path = SCREEN_STAGE;
    jsr_push(0x546B);
    run.load = StageLoad(disc_log_y(&run.scene, 0xFF)); /* Y as ResetGameState leaves it, then the scene load's */
    jsr_pop();
    wr8(zField99, 0);
    wr8(zField95, 5);
    wr8(zField8A, 3);
    wr8(zField8E, 1);
    run.field_a3 = rd8(zFieldA3);
    wr8(zField83, run.field_a3);
    SetGameMode(MODE_STAGE);
    return run;
}

void abi_ModeScreen(void)
{
    ScreenRun run = ModeScreen();

    overlay_done_shim(run.frame.after, run.frame.done);
    if (run.path == SCREEN_WAIT)
        return;
    reset_game_state_exit_shim();
    scene_shim(SCENE_COMMON, &run.scene);
    load_shim(run.stage);
    compare_shim(run.stage, STAGE_TITLE_BACK);
    if (run.path == SCREEN_TITLE) {
        back_to_title_exit_shim(&run.title);
        return;
    }
    compare_shim(run.stage, STAGE_BONUS);
    if (run.path == SCREEN_BONUS) {
        load_shim(run.slot);
        if (!(run.slot & 0x80)) {
            backup_read_exit_shim(run.read);
            cpu.y = run.save_slot;
            cpu.x = run.offset;
            load_shim(1);
            backup_write_exit_shim(&run.write);
        }
        cleared_next_exit_shim(&run.next);
        return;
    }
    stage_load_exit_shim(&run.load);
    load_shim(run.field_a3);
    set_mode_shim(MODE_STAGE);
}

/*
 * @routine 0045:49A ModeEnding
 * @run 69:549A
 * WHAT IT DOES: Mode 10: the ending's frame, then the stage in zStage.
 * HOW: Overlay frame, ResetGameState, a scene, StageEnter, ResetScoreCounters, SetGameMode.
 * WAITS: no.
 * Entry: nothing. Exit: as what ran last.
 */
typedef struct { OverlayFrame frame; DiscLog scene; StageEnterRun enter; } EndingRun;

EndingRun ModeEnding(void)
{
    EndingRun run;

    run.frame = overlay_frame(FrameOverlay8000, 0x549D);
    if (run.frame.done == 0)
        return run;
    wr8(zGameRunning, 0);
    jsr_push(0x54A7);
    ResetGameState();
    jsr_pop();
    jsr_push(0x54AC);
    run.scene = LoadSceneScript(SCENE_COMMON);
    jsr_pop();
    jsr_push(0x54AF);
    run.enter = StageEnter();
    jsr_pop();
    jsr_push(0x54B2);
    ResetScoreCounters();
    jsr_pop();
    SetGameMode(MODE_STAGE);
    return run;
}

void abi_ModeEnding(void)
{
    EndingRun run = ModeEnding();

    overlay_done_shim(run.frame.after, run.frame.done);
    if (run.frame.done == 0)
        return;
    reset_game_state_exit_shim();
    scene_shim(SCENE_COMMON, &run.scene);
    stage_enter_exit_shim(&run.enter);
    reset_score_shim();
    set_mode_shim(MODE_STAGE);
}

/*
 * @routine 0045:4B7 NextGameMode
 * @run 69:54B7
 * WHAT IT DOES: Goes on to the next game mode.
 * HOW: zGameMode plus one, into SetGameMode.
 * Entry: nothing. Exit: A 0, N and Z from the new mode. X, Y, C and V kept.
 */
uint8_t NextGameMode(void)
{
    uint8_t mode = (uint8_t)(rd8(zGameMode) + 1);

    SetGameMode(mode);
    return mode;
}

void abi_NextGameMode(void)
{
    uint8_t mode = NextGameMode();

    cpu.a = 0;
    abi_nz(mode);
}

/*
 * @routine 0045:4BA SetGameMode
 * @run 69:54BA
 * WHAT IT DOES: Sets the game mode, with its done flag and its sub-mode cleared.
 * HOW: Writes zGameMode, clears zModeDone, then zGameSubMode and zSubStep.
 * Entry: A mode. Exit: A 0. N, Z, C, V, X and Y kept.
 */
void SetGameMode(uint8_t mode)
{
    wr8(zGameMode, mode);
    wr8(zModeDone, 0);
    wr8(zGameSubMode, 0);
    wr8(zSubStep, 0);
}

void abi_SetGameMode(void)
{
    SetGameMode(cpu.a);
    cpu.a = 0;
}

/*
 * @routine 0045:4C1 NextSubMode
 * @run 69:54C1
 * WHAT IT DOES: Goes on to the next sub-mode. Nothing in the kernel reaches it; overlays
 * may.
 * HOW: zGameSubMode plus one, with zSubStep cleared.
 * Entry: nothing. Exit: A the sub-mode, N and Z from it. X, Y, C, V kept.
 */
void abi_NextSubMode(void)
{
    uint8_t byte = rd8(zGameSubMode);

    byte = (uint8_t)(byte + 1);
    wr8(zGameSubMode, byte);
    wr8(zSubStep, 0);
    cpu.a = byte;
    abi_nz(byte);
}

/*
 * @routine 0045:4C9 ResetScoreCounters
 * @run 69:54C9
 * WHAT IT DOES: Starts a run's score and stock: score zero, the stock three.
 * HOW: Clears the score to add, the score, and zField93.
 * Entry: nothing. Exit: A 3, N and Z from it. X, Y, C, V kept.
 */
void ResetScoreCounters(void)
{
    wr8(zScoreAddLo, 0);
    wr8(zScoreAddHi, 0);
    wr8(zScore0, 0);
    wr8(zScore1, 0);
    wr8(zScore2, 0);
    wr8(zStock, STOCK_AT_START);
    wr8(zField93, 0);
}

void abi_ResetScoreCounters(void)
{
    ResetScoreCounters();
    cpu.a = STOCK_AT_START;
    abi_nz(STOCK_AT_START);
}

/*
 * @routine 0045:587 FadeAndStopSound
 * @run 69:5587
 * WHAT IT DOES: Fades the CD out, unless zField8E says a stage's track goes on, and stops
 * the sound.
 * HOW: Bits 3 and 4 of zField8E; the card's cd_fade; SoundCommand.
 * WAITS: no.
 * Entry: X and Y are the caller's; cd_fade leaves them, SoundCommand keeps them. Exit: as SoundCommand.
 */
typedef struct { uint8_t keep; CommandCall sound; } FadeStop;

FadeStop FadeAndStopSound(uint8_t kept_x, uint8_t kept_y)
{
    FadeStop run;

    run.keep = rd8(zField8E) & SOUND_KEEP_TRACK;
    if (run.keep == 0) {
        jsr_push(0x5592);
        vec_cd_fade(CD_FADE_OUT);
        jsr_pop();
    }
    run.sound = SoundCommand(SOUND_STOP, kept_x, kept_y);
    return run;
}

void abi_FadeAndStopSound(void)
{
    FadeStop run = FadeAndStopSound(cpu.x, cpu.y);

    load_shim(run.keep);
    if (run.keep == 0) {
        load_shim(CD_FADE_OUT);
        cd_fade_exit_shim(CD_FADE_OUT);
    }
    load_shim(SOUND_STOP);
    sound_command_exit_shim(SOUND_STOP, run.sound);
}

/*
 * @routine 0045:592 StopSound
 * @run 69:5592
 * WHAT IT DOES: Tells the sound driver to stop.
 * HOW: Command $FF.
 * Entry: nothing. Exit: as SoundCommand.
 */
void abi_StopSound(void)
{
    cpu.a = SOUND_STOP;
    abi_nz(SOUND_STOP);
    abi_SoundCommand();
}
