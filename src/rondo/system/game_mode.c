/*
 * The game's modes: FrameModeDispatch runs the handler for zGameMode each frame, and
 * most handlers run an overlay's frame until it sets zModeDone, then load what the
 * next mode needs and set it. Every handler ends with what the overlay, the BIOS or
 * the loaders left in A and the flags, so each carries them in an Alu, and X and Y go
 * straight to cpu where the assembly loads them.
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
    STAGE_BONUS = 0x8A, EVENTS_BANK_OFFSET = 0x0D,
    CD_FADE_OUT = 0x0C,
    STAGES_ATTRACT = 5, JOY_RUN = 0x08
};

/* Reads zModeDone; true when the overlay has finished. */
static bool mode_done(Alu *r)
{
    uint8_t byte = rd8(zModeDone);

    alu_load(r, byte);
    return byte != 0;
}

/* An overlay's frame, then whether it has finished. */
static Alu overlay_frame(uint16_t entry, bool *done)
{
    Alu r = call_with(entry, alu_from_cpu());

    *done = mode_done(&r);
    return r;
}

static Alu load_scene(Alu r, uint8_t scene)
{
    alu_load(&r, scene);
    return LoadSceneScript(r);
}

static Alu set_mode(Alu r, uint8_t mode)
{
    alu_load(&r, mode);
    return SetGameMode(r);
}

/* The bit of StageClearBit, ORed into a table byte at the index it gave in X. */
static Alu mark_stage(Alu r, uint16_t table)
{
    uint8_t byte;

    r = StageClearBit(r);
    byte = rd8((uint16_t)(table + cpu.x));
    alu_load(&r, (uint8_t)(r.a | byte));
    wr8((uint16_t)(table + cpu.x), r.a);
    return r;
}

/*
 * @routine 0045:105 FrameModeDispatch
 * @run 69:5105
 * WHAT IT DOES: Runs this frame's game mode.
 * HOW: Counts zFrameTick, then jumps through the table by zGameMode.
 * Entry: nothing. Exit: as the handler leaves them.
 */
Alu FrameModeDispatch(void)
{
    uint8_t byte = rd8(zFrameTick);
    uint8_t lo;
    uint8_t hi;
    uint16_t handler;
    Alu r = alu_from_cpu();

    wr8(zFrameTick, (uint8_t)(byte + 1));
    byte = rd8(zGameMode);
    r.a = (uint8_t)(byte << 1);
    alu_nz(&r, r.a);
    r.c = (byte & 0x80) != 0;
    r.c_kept = false;
    cpu.x = r.a;
    lo = rd8((uint16_t)(GameModeHandlers + cpu.x));
    hi = rd8((uint16_t)(GameModeHandlers + cpu.x + 1));
    handler = (uint16_t)(lo | (hi << 8));
    abi_alu(r);
    switch (handler) {
    case 0x5124: return ModeBoot();
    case 0x514F: return ModeLogo();
    case 0x515E: return ModeAttract();
    case 0x51B6: return ModeAttractPlay();
    case 0x51DD: return ModeStageLoad();
    case 0x5265: return ModeStage();
    case 0x52FE: return ModeStageCleared();
    case 0x534E: return ModeDead();
    case 0x5432: return ModeGameOverWait();
    case 0x544E: return ModeScreen();
    case 0x549A: return ModeEnding();
    default:
        call_original(handler);
        return alu_from_cpu();
    }
}

void abi_FrameModeDispatch(void)
{
    abi_alu(FrameModeDispatch());
}

/*
 * @routine 0045:124 ModeBoot
 * @run 69:5124
 * WHAT IT DOES: Mode 0: the boot overlay's frame, then the display off, the screen size
 * set and on to the title; a negative zModeDone goes on to the title's sub-mode 3.
 * HOW: Overlay frame, WaitVblankDisplayOff, the BIOS's screen size, SetGameMode.
 * Entry: nothing. Exit: as SetGameMode, or with zGameSubMode 3 and A 3.
 */
Alu ModeBoot(void)
{
    bool done;
    Alu r = overlay_frame(FrameOverlay8000, &done);
    uint8_t value;

    if (!done)
        return r;
    r = call_abi(abi_WaitVblankDisplayOff, r);
    alu_load(&r, 4);
    wr8(zDisplayDelay, 4);
    alu_load(&r, 5);
    r = call_with(BiosExScrSiz, r);
    wr8(zStage, 0);
    alu_load(&r, 0xFF);
    wr8(zCharacterLoaded, 0xFF);
    wr8(zCharacter, 0);
    value = rd8(zModeDone);
    cpu.x = value;
    r = set_mode(r, MODE_TITLE);
    alu_load(&r, cpu.x);
    if (r.a & 0x80) {
        alu_load(&r, 3);
        wr8(zGameSubMode, 3);
    }
    return r;
}

void abi_ModeBoot(void)
{
    abi_alu(ModeBoot());
}

/*
 * @routine 0045:14F ModeLogo
 * @run 69:514F
 * WHAT IT DOES: Mode 1: the overlay's frame, then the stage's load.
 * HOW: Overlay frame at its second entry; zField80 cleared; SetGameMode.
 * Entry: nothing. Exit: as SetGameMode.
 */
Alu ModeLogo(void)
{
    bool done;
    Alu r = overlay_frame(FrameOverlay8003, &done);

    if (!done)
        return r;
    wr8(zField80, 0);
    return set_mode(r, MODE_STAGE_LOAD);
}

void abi_ModeLogo(void)
{
    abi_alu(ModeLogo());
}

/*
 * @routine 0045:15E ModeAttract
 * @run 69:515E
 * WHAT IT DOES: Mode 2: the title's frame. The overlay's answer picks what comes next:
 * zero runs the overlay's other entry; positive restarts the attract's load; negative
 * starts the next demo stage, or on the first pass loads the title's scenes.
 * HOW: The overlay at $8006, then zAttractStep; the five demo stages are a table.
 * Entry: nothing. Exit: as what ran last.
 */
Alu ModeAttract(void)
{
    Alu r = call_with(FrameOverlay8006, alu_from_cpu());
    uint8_t byte = rd8(zModeDone);

    alu_load(&r, byte);
    if (!(byte & 0x80)) {
        if (byte == 0)
            return call_with(FrameOverlay8009, r);
        return set_mode(r, MODE_ATTRACT_LOAD);
    }
    byte = rd8(zAttractStep);
    alu_load(&r, byte);
    if (byte == 0) {
        byte = rd8(zAttractStep);
        wr8(zAttractStep, (uint8_t)(byte + 1));
        wr8(zMusicWanted, 0);
        r = call_abi(abi_ResetGameState, r);
        r = load_scene(r, 7);
        r = load_scene(r, 8);
        r = load_scene(r, 9);
        r = AdpcmLoadFixed(r);
        return NextGameMode(r);
    }
    r = ResetScoreCounters(r);
    wr8(zCharacter, 0);
    r = load_scene(r, SCENE_COMMON);
    byte = rd8(zAttractStep);
    cpu.y = byte;
    byte = rd8((uint16_t)(AttractStages + cpu.y));
    alu_load(&r, byte);
    wr8(zStage, byte);
    r = StageEnter(r);
    byte = rd8(zAttractStep);
    wr8(zAttractStep, (uint8_t)(byte + 1));
    byte = rd8(zAttractStep);
    alu_compare(&r, byte, STAGES_ATTRACT);
    if (byte == STAGES_ATTRACT)
        wr8(zAttractStep, 0);
    byte = rd8(zGameRunning);
    wr8(zGameRunning, (uint8_t)(byte + 1));
    return NextGameMode(r);
}

void abi_ModeAttract(void)
{
    abi_alu(ModeAttract());
}

/*
 * @routine 0045:1B6 ModeAttractPlay
 * @run 69:51B6
 * WHAT IT DOES: Mode 3: a demo's title frames; Run or the overlay's end stops the demo,
 * fades the CD and goes to mode 8.
 * HOW: The BIOS's joypad byte, the overlay's frame, a sound fade, the CD fade.
 * Entry: nothing. Exit: as what ran last.
 */
Alu ModeAttractPlay(void)
{
    uint8_t byte = rd8(BiosJoyTrigger);
    Alu r = alu_from_cpu();

    alu_load(&r, (uint8_t)(byte & JOY_RUN));
    if (r.a == 0) {
        bool done;

        r = call_with(FrameOverlay8000, r);
        done = mode_done(&r);
        if (!done)
            return r;
    }
    wr8(zGameRunning, 0);
    alu_load(&r, SOUND_FADE);
    r = call_abi(abi_SoundCommand, r);
    alu_load(&r, CD_FADE_OUT);
    r = call_with(BiosCdFade, r);
    alu_load(&r, 0x40);
    wr8(zWaitTimer, 0x40);
    r = call_abi(abi_ResetGameState, r);
    return set_mode(r, MODE_GAME_OVER);
}

void abi_ModeAttractPlay(void)
{
    abi_alu(ModeAttractPlay());
}

/*
 * @routine 0045:1DD ModeStageLoad
 * @run 69:51DD
 * WHAT IT DOES: Mode 4: the menu overlay's frame, then what zModeEvents asks for: the
 * name entry, a new game from the menu, or the first stage.
 * HOW: The overlay's fifth entry, then bits 3 and 4 of zModeEvents.
 * Entry: nothing. Exit: as what ran last.
 */
Alu ModeStageLoad(void)
{
    bool done;
    Alu r = overlay_frame(OverlayTerrainLookup, &done);

    if (!done)
        return r;
    if (rd8(zModeEvents) & 0x08)
        return ModeStageLoadScreens(r);
    if (rd8(zModeEvents) & 0x10)
        return ModeStageLoadContinue(r);
    r = load_scene(r, SCENE_COMMON);
    return StageEnterNext(r);
}

void abi_ModeStageLoad(void)
{
    abi_alu(ModeStageLoad());
}

/*
 * @routine 0045:1F0 StageEnterNext
 * @run 69:51F0
 * WHAT IT DOES: Enters the stage in zStage with a clean score count and goes on a mode.
 * HOW: StageEnter, ResetScoreCounters, NextGameMode.
 * Entry: nothing. Exit: as NextGameMode.
 */
Alu StageEnterNext(Alu r)
{
    r = StageEnter(r);
    r = ResetScoreCounters(r);
    return NextGameMode(r);
}

void abi_StageEnterNext(void)
{
    abi_alu(StageEnterNext(alu_from_cpu()));
}

/*
 * @routine 0045:1F9 ModeStageLoadScreens
 * @run 69:51F9
 * WHAT IT DOES: The path of ModeStageLoad for bit 3 of zModeEvents: loads the screens'
 * scenes and goes to mode 10.
 * HOW: Clears the bit and the wanted ADPCM, ResetGameState, three scenes, the fixed ADPCM.
 * Entry: from ModeStageLoad. Exit: as SetGameMode.
 */
Alu ModeStageLoadScreens(Alu r)
{
    uint8_t byte = rd8(zModeEvents);

    wr8(zModeEvents, (uint8_t)(byte & ~0x08));
    wr8(zMusicWanted, 0);
    r = call_abi(abi_ResetGameState, r);
    r = load_scene(r, 7);
    r = load_scene(r, 8);
    r = load_scene(r, 9);
    r = AdpcmLoadFixed(r);
    return set_mode(r, MODE_ENDING);
}

void abi_ModeStageLoadScreens(void)
{
    abi_alu(ModeStageLoadScreens(alu_from_cpu()));
}

/*
 * @routine 0045:217 ModeStageLoadContinue
 * @run 69:5217
 * WHAT IT DOES: The path of ModeStageLoad for bit 4 of zModeEvents: enters the stage and
 * goes to the intro mode.
 * HOW: ResetScoreCounters, a scene, StageEnter, zGameRunning counted, SetGameMode.
 * Entry: from ModeStageLoad. Exit: as SetGameMode.
 */
Alu ModeStageLoadContinue(Alu r)
{
    uint8_t byte;

    r = ResetScoreCounters(r);
    r = load_scene(r, SCENE_COMMON);
    r = StageEnter(r);
    byte = rd8(zGameRunning);
    wr8(zGameRunning, (uint8_t)(byte + 1));
    return set_mode(r, MODE_INTRO);
}

void abi_ModeStageLoadContinue(void)
{
    abi_alu(ModeStageLoadContinue(alu_from_cpu()));
}

/*
 * @routine 0045:229 ModeStageDone
 * @run 69:5229
 * WHAT IT DOES: The stage's overlay finished with a positive answer: goes on a mode,
 * except on the bonus stage, which marks it and, unless the save slot's flag is set,
 * loads the ending's scenes and goes to mode 9.
 * HOW: Compares zStage; StageClearBit in StagesVisited; the save's flag at $7D25.
 * Entry: from ModeStage. Exit: as what ran last.
 */
Alu ModeStageDone(Alu r)
{
    uint8_t byte = rd8(zStage);

    alu_compare(&r, byte, STAGE_BONUS);
    if (byte != STAGE_BONUS)
        return NextGameMode(r);
    r = mark_stage(r, StagesVisited);
    r = call_abi(abi_ResetGameState, r);
    r = call_abi(abi_SoundStopAll, r);
    byte = rd8(zSaveSlot);
    alu_load(&r, byte);
    if (!(byte & 0x80)) {
        r = SaveProgress(r);
        cpu.y = rd8(zSaveSlot);
        cpu.x = rd8((uint16_t)(SaveSlotOffsets + cpu.y));
        byte = rd8((uint16_t)(0x7D25 + cpu.x));
        alu_load(&r, byte);
        if (byte != 0)
            return NextGameMode(r);
    }
    alu_load(&r, 0xFF);
    wr8(zCharacterLoaded, 0xFF);
    r = load_scene(r, 0x5C);
    r = load_scene(r, 0x5D);
    return set_mode(r, MODE_SCREEN);
}

void abi_ModeStageDone(void)
{
    abi_alu(ModeStageDone(alu_from_cpu()));
}

/*
 * @routine 0045:265 ModeStage
 * @run 69:5265
 * WHAT IT DOES: Mode 5: a stage's frame. A negative answer goes to mode 7, a positive one
 * to ModeStageDone. At zero, bits of zModeEvents ask for a boss's ADPCM and scene, a
 * screen's scenes, or the stage's clear with the score saved.
 * HOW: Bits 0, 1 and 2 of zModeEvents, each cleared as it is taken.
 * Entry: nothing. Exit: as what ran last.
 */
Alu ModeStage(void)
{
    Alu r = call_with(FrameOverlay8000, alu_from_cpu());
    uint8_t byte = rd8(zModeDone);
    uint8_t high;

    alu_load(&r, byte);
    if (byte & 0x80)
        return set_mode(r, MODE_DEAD);
    if (byte != 0)
        return ModeStageDone(r);
    if (rd8(zModeEvents) & 0x01) {
        byte = rd8(zModeEvents);
        wr8(zModeEvents, (uint8_t)(byte & ~0x01));
        byte = rd8(zCharacter);
        r = adc8(0x22, byte, false);
        r = AdpcmLoad(r);
        return load_scene(r, 0x5E);
    }
    if (rd8(zModeEvents) & 0x02) {
        byte = rd8(zModeEvents);
        wr8(zModeEvents, (uint8_t)(byte & ~0x02));
        r = call_abi(abi_ResetGameState, r);
        alu_load(&r, 0xFF);
        wr8(zMusicWanted, 0xFF);
        wr8(zCharacterLoaded, 0xFF);
        byte = rd8(zStage);
        alu_compare(&r, byte, 4);
        if (byte == 4) {
            high = 2;
        } else {
            alu_compare(&r, byte, 7);
            if (byte == 7) {
                high = 0x0A;
            } else {
                alu_compare(&r, byte, 9);
                high = byte == 9 ? 0x0C : 0x0E;
            }
        }
        r = load_scene(r, high);
        r = load_scene(r, (uint8_t)(high + 1));
        return set_mode(r, MODE_SCREEN);
    }
    if (rd8(zModeEvents) & 0x04) {
        byte = rd8(zModeEvents);
        wr8(zModeEvents, (uint8_t)(byte & ~0x04));
        r = mark_stage(r, StagesVisited);
        byte = rd8(zSaveSlot);
        alu_load(&r, byte);
        if (!(byte & 0x80))
            r = SaveScore(r);
        byte = rd8(zCharacter);
        r.c = (byte & 0x80) != 0;
        r.c_kept = false;
        r = adc8((uint8_t)(byte << 1), 0x58, r.c);
        r = LoadSceneScript(r);
        byte = rd8(zCharacter);
        r.c = (byte & 0x80) != 0;
        r.c_kept = false;
        r = adc8((uint8_t)(byte << 1), 0x59, r.c);
        r = LoadSceneScript(r);
        return set_mode(r, MODE_SCREEN);
    }
    return r;
}

void abi_ModeStage(void)
{
    abi_alu(ModeStage());
}

/*
 * @routine 0045:2FE ModeStageCleared
 * @run 69:52FE
 * WHAT IT DOES: Mode 6: marks the stage visited and runs into StageClearedNext.
 * HOW: StageClearBit into StagesVisited.
 * Entry: nothing. Exit: as StageClearedNext.
 */
Alu ModeStageCleared(void)
{
    return StageClearedNext(mark_stage(alu_from_cpu(), StagesVisited));
}

void abi_ModeStageCleared(void)
{
    abi_alu(ModeStageCleared());
}

/*
 * @routine 0045:306 StageClearedNext
 * @run 69:5306
 * WHAT IT DOES: Enters the next stage, marks it and saves, and goes to the stage mode; if
 * there is no next stage, back to the title's mode 0.
 * HOW: StageEnter; zStage zero is the end. StageClearBit into StagesReached.
 * Entry: nothing. Exit: as SetGameMode.
 */
Alu StageClearedNext(Alu r)
{
    uint8_t byte;

    r = StageEnter(r);
    byte = rd8(zStage);
    alu_load(&r, byte);
    if (byte == 0) {
        r.a = 0;
        return SetGameMode(r);
    }
    r = mark_stage(r, StagesReached);
    byte = rd8(zSaveSlot);
    alu_load(&r, byte);
    if (!(byte & 0x80))
        r = SaveProgress(r);
    return set_mode(r, MODE_STAGE);
}

void abi_StageClearedNext(void)
{
    abi_alu(StageClearedNext(alu_from_cpu()));
}

/*
 * @routine 0045:325 StageClearBit
 * @run 69:5325
 * WHAT IT DOES: Finds zStage's bit in the visited tables: the byte in X, the bit in A.
 * HOW: Bits 2-4 of the stage give the byte, five further on for the second character;
 * bits 0-1 the bit; a stage with bit 7 set takes the high nibble.
 * Entry: nothing. Exit: A the bit, X the byte, Y zStage, N and Z from Y or the last shift;
 * C from the last shift or the add; V from the add. Clobbers zTemp00.
 */
Alu StageClearBit(Alu r)
{
    uint8_t value = 0;
    uint8_t stage;
    uint8_t shifts;
    uint8_t byte;

    cpu.x = rd8(zCharacter);
    if (cpu.x != 0)
        value = 5;
    wr8(zTemp00, value);
    stage = rd8(zStage);
    byte = rd8(zTemp00);
    r = adc8((uint8_t)((stage >> 2) & 0x07), byte, false);
    cpu.x = r.a;
    stage = rd8(zStage);
    shifts = stage & 0x03;
    value = 1;
    for (uint8_t i = 0; i < shifts; i++) {
        r.c = (value & 0x80) != 0;
        value = (uint8_t)(value << 1);
    }
    stage = rd8(zStage);
    cpu.y = stage;
    alu_nz(&r, stage);
    if (stage & 0x80) {
        for (unsigned i = 0; i < 4; i++) {
            r.c = (value & 0x80) != 0;
            value = (uint8_t)(value << 1);
        }
        alu_nz(&r, value);
    }
    r.a = value;
    return r;
}

void abi_StageClearBit(void)
{
    abi_alu(StageClearBit(alu_from_cpu()));
}

/*
 * @routine 0045:34E ModeDead
 * @run 69:534E
 * WHAT IT DOES: Mode 7: the continue screen's frame. Then counts a continue used, in
 * BCD up to 999, saves, and goes back into the stage, or at the last continue to the
 * title. A stage with bit 7 set restarts from zStageRequested.
 * HOW: Maps the overlay's bank; the continues at $3F41-$3F42; $3F24 picks what follows.
 * Entry: nothing. Exit: as what ran last.
 */
Alu ModeDead(void)
{
    Alu r;
    uint8_t base;
    uint8_t byte;
    bool done;

    base = rd8(zRamBaseBank);
    r = adc8(EVENTS_BANK_OFFSET, base, false);
    tam(MPR4, r.a);
    r = call_with(FrameOverlay8000, r);
    done = mode_done(&r);
    if (!done)
        return r;
    byte = rd8(ContinuesLo);
    r = adc8_bcd(byte, 1, false);
    wr8(ContinuesLo, r.a);
    if (r.c) {
        byte = rd8(ContinuesHi);
        alu_nz(&r, (uint8_t)(byte + 1));
        wr8(ContinuesHi, (uint8_t)(byte + 1));
    }
    byte = rd8(ContinuesHi);
    alu_compare(&r, byte, 0x0A);
    if (byte == 0x0A) {
        alu_load(&r, 0x99);
        wr8(ContinuesLo, 0x99);
        alu_load(&r, 0x09);
        wr8(ContinuesHi, 0x09);
    }
    byte = rd8(zSaveSlot);
    alu_load(&r, byte);
    if (!(byte & 0x80))
        r = SaveScore(r);
    byte = rd8(ContinueChoice);
    alu_load(&r, byte);
    if (byte != 0) {
        byte = (uint8_t)(byte - 1);
        alu_load(&r, byte);
        if (byte == 0) {
            r = call_abi(abi_ResetGameState, r);
            return ModeBackToTitle(r);
        }
        byte = rd8(zCharacter);
        byte ^= 0x01;
        alu_load(&r, byte);
        wr8(zCharacter, byte);
    }
    byte = rd8(zStage);
    alu_load(&r, byte);
    if (byte & 0x80) {
        byte = rd8(zStageRequested);
        byte &= 0x7F;
        alu_load(&r, byte);
    }
    byte |= 0x40;
    alu_load(&r, byte);
    wr8(zStage, byte);
    r = ResetScoreCounters(r);
    byte = rd8(zStage);
    alu_compare(&r, byte, 0x43);
    if (byte == 0x43) {
        alu_load(&r, 0x42);
        wr8(zStage, 0x42);
    }
    r = set_mode(r, MODE_STAGE_LOAD);
    return StageEnterNext(r);
}

void abi_ModeDead(void)
{
    abi_alu(ModeDead());
}

/*
 * @routine 0045:3B7 ModeBackToTitle
 * @run 69:53B7
 * WHAT IT DOES: Goes back to the title: forgets what is loaded, loads the menu's scenes
 * and ADPCM, and sets mode 0.
 * HOW: Two scenes, an ADPCM, SetGameMode.
 * Entry: nothing. Exit: as SetGameMode.
 */
Alu ModeBackToTitle(Alu r)
{
    alu_load(&r, 0xFF);
    wr8(zMusicWanted, 0xFF);
    wr8(zCharacterLoaded, 0xFF);
    alu_load(&r, 0);
    wr8(zStage, 0);
    r = load_scene(r, SCENE_MENU_A);
    r = load_scene(r, SCENE_MENU_B);
    alu_load(&r, ADPCM_MENU);
    r = AdpcmLoad(r);
    r.a = 0;
    return SetGameMode(r);
}

void abi_ModeBackToTitle(void)
{
    abi_alu(ModeBackToTitle(alu_from_cpu()));
}

/* The save slot's copy of the character's progress, from the working bytes. */
static Alu save_progress_tail(Alu r)
{
    uint8_t byte = rd8(ProgressCharacter);

    alu_load(&r, byte);
    wr8((uint16_t)(SaveBuffer + cpu.x), byte);
    byte = rd8(ContinuesLo);
    alu_load(&r, byte);
    wr8((uint16_t)(SaveContinues + cpu.x), byte);
    byte = rd8(ContinuesHi);
    alu_load(&r, byte);
    wr8((uint16_t)(SaveContinues + 1 + cpu.x), byte);
    cpu.y = 0;
    do {
        byte = rd8((uint16_t)(StagesReached + cpu.y));
        wr8((uint16_t)(SaveBuffer + 1 + cpu.x), byte);
        cpu.x++;
        cpu.y++;
    } while (cpu.y != 0x14);
    alu_compare(&r, cpu.y, 0x14);
    r.a = byte;
    return BackupWrite(r);
}

/*
 * @routine 0045:3D7 SaveProgress
 * @run 69:53D7
 * WHAT IT DOES: Writes the character's progress to the save slot in zSaveSlot.
 * HOW: Reads the backup file to RAM, copies into the slot, writes it back.
 * Entry: nothing. Exit: as BackupWrite.
 */
Alu SaveProgress(Alu r)
{
    r = BackupRead(r);
    cpu.y = rd8(zSaveSlot);
    cpu.x = rd8((uint16_t)(SaveSlotOffsets + cpu.y));
    return save_progress_tail(r);
}

void abi_SaveProgress(void)
{
    abi_alu(SaveProgress(alu_from_cpu()));
}

/*
 * @routine 0045:3E1 SaveScore
 * @run 69:53E1
 * WHAT IT DOES: As SaveProgress, and first adds the stage's score to the slot's total, in
 * BCD, stopping at 999999.
 * HOW: A 24-bit BCD add from zScoreAddLo-Hi.
 * Entry: nothing. Exit: as BackupWrite.
 */
Alu SaveScore(Alu r)
{
    uint8_t a;
    uint8_t b;

    r = BackupRead(r);
    cpu.y = rd8(zSaveSlot);
    cpu.x = rd8((uint16_t)(SaveSlotOffsets + cpu.y));
    a = rd8(zScoreAddLo);
    b = rd8((uint16_t)(SaveScoreTotal + cpu.x));
    r = adc8_bcd(a, b, false);
    wr8((uint16_t)(SaveScoreTotal + cpu.x), r.a);
    a = rd8(zScoreAddHi);
    b = rd8((uint16_t)(SaveScoreTotal + 1 + cpu.x));
    r = adc8_bcd(a, b, r.c);
    wr8((uint16_t)(SaveScoreTotal + 1 + cpu.x), r.a);
    b = rd8((uint16_t)(SaveScoreTotal + 2 + cpu.x));
    r = adc8_bcd(0, b, r.c);
    wr8((uint16_t)(SaveScoreTotal + 2 + cpu.x), r.a);
    if (r.c) {
        alu_load(&r, 0x99);
        wr8((uint16_t)(SaveScoreTotal + cpu.x), 0x99);
        wr8((uint16_t)(SaveScoreTotal + 1 + cpu.x), 0x99);
        wr8((uint16_t)(SaveScoreTotal + 2 + cpu.x), 0x99);
    }
    return save_progress_tail(r);
}

void abi_SaveScore(void)
{
    abi_alu(SaveScore(alu_from_cpu()));
}

/*
 * @routine 0045:432 ModeGameOverWait
 * @run 69:5432
 * WHAT IT DOES: Mode 8: waits out zWaitTimer, stops the sound, and goes back to the title;
 * with bit 4 of zModeEvents, to the stage's load from the start.
 * HOW: Counts down; SoundStopAll; ModeBackToTitle.
 * Entry: nothing. Exit: on waiting, A kept, N and Z from the count; else as what ran last.
 */
Alu ModeGameOverWait(void)
{
    uint8_t byte = rd8(zWaitTimer);
    Alu r = alu_from_cpu();

    byte = (uint8_t)(byte - 1);
    wr8(zWaitTimer, byte);
    alu_nz(&r, byte);
    if (byte != 0)
        return r;
    r = call_abi(abi_SoundStopAll, r);
    if (!(rd8(zModeEvents) & 0x10))
        return ModeBackToTitle(r);
    byte = rd8(zModeEvents);
    wr8(zModeEvents, (uint8_t)(byte & ~0x10));
    r = ModeBackToTitle(r);
    wr8(zCharacter, 0);
    wr8(zField80, 0);
    return set_mode(r, MODE_STAGE_LOAD);
}

void abi_ModeGameOverWait(void)
{
    abi_alu(ModeGameOverWait());
}

/*
 * @routine 0045:44E ModeScreen
 * @run 69:544E
 * WHAT IT DOES: Mode 9: a screen's frame between stages, then what follows it: back to
 * the title from stage $10; from the bonus stage the save's flag and the next stage;
 * otherwise the stage's load again with the player's stock restored.
 * HOW: Overlay frame, then zStage.
 * Entry: nothing. Exit: as what ran last.
 */
Alu ModeScreen(void)
{
    bool done;
    Alu r = overlay_frame(FrameOverlay8000, &done);
    uint8_t byte;

    if (!done)
        return r;
    r = call_abi(abi_ResetGameState, r);
    r = load_scene(r, SCENE_COMMON);
    byte = rd8(zStage);
    alu_compare(&r, byte, 0x10);
    if (byte == 0x10)
        return ModeBackToTitle(r);
    alu_compare(&r, byte, STAGE_BONUS);
    if (byte == STAGE_BONUS) {
        byte = rd8(zSaveSlot);
        alu_load(&r, byte);
        if (!(byte & 0x80)) {
            r = BackupRead(r);
            cpu.y = rd8(zSaveSlot);
            cpu.x = rd8((uint16_t)(SaveSlotOffsets + cpu.y));
            alu_load(&r, 1);
            wr8((uint16_t)(0x7D25 + cpu.x), 1);
            r = BackupWrite(r);
        }
        return StageClearedNext(r);
    }
    r = StageLoad(r);
    wr8(zField99, 0);
    wr8(zField95, 5);
    wr8(zField8A, 3);
    wr8(zField8E, 1);
    byte = rd8(zFieldA3);
    wr8(zField83, byte);
    return set_mode(r, MODE_STAGE);
}

void abi_ModeScreen(void)
{
    abi_alu(ModeScreen());
}

/*
 * @routine 0045:49A ModeEnding
 * @run 69:549A
 * WHAT IT DOES: Mode 10: the ending's frame, then the stage in zStage.
 * HOW: Overlay frame, ResetGameState, a scene, StageEnter, ResetScoreCounters, SetGameMode.
 * Entry: nothing. Exit: as what ran last.
 */
Alu ModeEnding(void)
{
    bool done;
    Alu r = overlay_frame(FrameOverlay8000, &done);

    if (!done)
        return r;
    wr8(zGameRunning, 0);
    r = call_abi(abi_ResetGameState, r);
    r = load_scene(r, SCENE_COMMON);
    r = StageEnter(r);
    r = ResetScoreCounters(r);
    return set_mode(r, MODE_STAGE);
}

void abi_ModeEnding(void)
{
    abi_alu(ModeEnding());
}

/*
 * @routine 0045:4B7 NextGameMode
 * @run 69:54B7
 * WHAT IT DOES: Goes on to the next game mode.
 * HOW: zGameMode plus one, into SetGameMode.
 * Entry: nothing. Exit: as SetGameMode, N and Z from the new mode.
 */
Alu NextGameMode(Alu r)
{
    uint8_t byte = rd8(zGameMode);

    alu_load(&r, (uint8_t)(byte + 1));
    return SetGameMode(r);
}

void abi_NextGameMode(void)
{
    abi_alu(NextGameMode(alu_from_cpu()));
}

/*
 * @routine 0045:4BA SetGameMode
 * @run 69:54BA
 * WHAT IT DOES: Sets the game mode, with its done flag and its sub-mode cleared.
 * HOW: Writes zGameMode, clears zModeDone, then zGameSubMode and zSubStep.
 * Entry: A mode. Exit: A 0. N, Z, C, V, X and Y kept.
 */
Alu SetGameMode(Alu r)
{
    wr8(zGameMode, r.a);
    wr8(zModeDone, 0);
    r.a = 0;
    wr8(zGameSubMode, 0);
    wr8(zSubStep, 0);
    return r;
}

void abi_SetGameMode(void)
{
    abi_alu(SetGameMode(alu_from_cpu()));
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
Alu ResetScoreCounters(Alu r)
{
    wr8(zScoreAddLo, 0);
    wr8(zScoreAddHi, 0);
    wr8(zScore0, 0);
    wr8(zScore1, 0);
    wr8(zScore2, 0);
    alu_load(&r, 3);
    wr8(zStock, 3);
    wr8(zField93, 0);
    return r;
}

void abi_ResetScoreCounters(void)
{
    abi_alu(ResetScoreCounters(alu_from_cpu()));
}

/*
 * @routine 0045:587 FadeAndStopSound
 * @run 69:5587
 * WHAT IT DOES: Fades the CD out, unless zField8E says a stage's track goes on, and stops
 * the sound.
 * HOW: Bits 3 and 4 of zField8E; the BIOS's fade; SoundCommand.
 * Entry: nothing. Exit: as SoundCommand.
 */
void abi_FadeAndStopSound(void)
{
    uint8_t byte = rd8(zField8E);
    Alu r = alu_from_cpu();

    alu_load(&r, (uint8_t)(byte & 0x18));
    if (r.a == 0) {
        alu_load(&r, CD_FADE_OUT);
        r = call_with(BiosCdFade, r);
    }
    alu_load(&r, SOUND_STOP);
    abi_alu(r);
    abi_SoundCommand();
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
