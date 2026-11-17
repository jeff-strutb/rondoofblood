/*
 * Entering a stage: what the requested zStage turns into, the scenes and sounds it
 * loads, and the copy of VRAM some stages keep in RAM.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

enum {
    MPR4 = 0x10,
    STAGE_RESTART_BIT = 0x40, STAGE_SECRET_BIT = 0x80, STAGE_ROUTE_BIT = 0x01,
    STAGE_INDEX_MASK = 0x7F,
    SCENE_MENU_A = 4, SCENE_MENU_B = 6, ADPCM_MENU = 0x24,
    STAGE_AFTER_LAST_SECRET = 0x80, STAGE_TITLE = 0x02, ROUTE_SECOND = 0x05,
    STAGE_KEEPS_VRAM_A = 0x10, STAGE_KEEPS_VRAM_B = 0x0C,
    VRAM_COPY_ROWS = 8, VRAM_COPY_ROW_BYTES = 0x80, VRAM_COPY_ADDR_HI = 0x7E,
    VRAM_COPY_RAM_HI = 0x9C
};

/* What a stage's request turns into, where its stage ends elsewhere; stage 2 is the title's. */
static bool stage_redirect(StageEnterRun *run, uint8_t stage)
{
    uint8_t next = stage;

    switch (stage) {
    case STAGE_AFTER_LAST_SECRET:
        wr8(zField8A, 1);
        next = 2;
        break;
    case STAGE_TITLE:
        wr8(zStage, 0);
        run->to_title = true;
        run->menu_a = LoadSceneScript(SCENE_MENU_A);
        run->menu_b = LoadSceneScript(SCENE_MENU_B);
        run->menu_adpcm = AdpcmLoad(ADPCM_MENU);
        return true;
    case 0x85:
        run->field83_read = true;
        run->field83 = rd8(zField83);
        if (run->field83 == 0x0A) {
            wr8(zField8A, 1);
            next = 0x07;
        }
        break;
    case 0x0B:
        if (((rd8((uint16_t)(StagesVisited + 4)) | rd8((uint16_t)(StagesVisited + 9))) & 0x01) == 0) {
            wr8(zStageRequested, 0x4A);
            next = 0x0A;
        }
        break;
    case 0x8B:
        wr8(zField8A, 1);
        wr8(zStageRequested, 0x4C);
        next = 0x0C;
        break;
    case 0x8C:
        wr8(zField8A, 1);
        wr8(zStageRequested, 0x4E);
        next = 0x0E;
        break;
    default:
        return false;
    }
    wr8(zStage, next);
    return false;
}

/*
 * @routine 0048:4A6 StageEnter
 * @run 6A:6CA6
 * WHAT IT DOES: Starts the stage asked for in zStage. Clears the stage's counters and
 * turns the request into the stage to run: a request with bit 6 is a restart; otherwise a
 * main stage leads to its secret version, and a secret to the next stage on. Some stages
 * go elsewhere when they end, and stage 2 goes back to the title's load. Then StageLoad.
 * HOW: Keeps the request in zStageRequested; zField8A says how the stage starts.
 * Entry: nothing. Exit: as StageLoad, or as AdpcmLoad back at the title's load.
 */
StageEnterRun StageEnter(void)
{
    StageEnterRun run;
    uint8_t stage;

    run.route_read = false;
    run.field83_read = false;
    run.to_title = false;
    wr8(zField80, 0);
    wr8(zField89, 0);
    wr8(zField85, 0);
    wr8(zField82, 0);
    wr8(zField8F, 0);
    wr8(zField90, 0);
    wr8(zField91, 0);
    wr8(zField92, 0);
    wr8(zField99, 0);
    wr8(zField95, 5);
    wr8(zField8C, (uint8_t)(rd8(zField8C) & 0x7F));
    ResetGameState();
    run.stop = SoundStopAll();
    wr8(zStageRequested, rd8(zStage));
    if (rd8(zStageRequested) & STAGE_RESTART_BIT) {
        wr8(zStage, (uint8_t)(rd8(zStage) & ~STAGE_RESTART_BIT));
        wr8(zField8A, 0);
    } else {
        stage = rd8(zStage);
        if (!(stage & STAGE_SECRET_BIT)) {
            stage = (uint8_t)((stage | STAGE_SECRET_BIT) & ~STAGE_ROUTE_BIT);
            run.route_read = true;
            run.route = rd8(zField8C);
            if (run.route >= ROUTE_SECOND)
                stage |= STAGE_ROUTE_BIT;
            wr8(zStage, stage);
            wr8(zField8A, 3);
        } else {
            wr8(zStage, (uint8_t)((stage & STAGE_INDEX_MASK) + 2));
            wr8(zField8A, 1);
        }
        wr8(zField8C, 0);
    }
    if (!(rd8(zStageRequested) & STAGE_RESTART_BIT) && stage_redirect(&run, rd8(zStage)))
        return run;
    wr8(zField83, 0);
    run.load = StageLoad();
    return run;
}

void stage_enter_exit_shim(const StageEnterRun *run)
{
    reset_game_state_exit_shim();
    stop_call_exit_shim(run->stop);
    if (run->route_read)
        cpu.x = run->route;
    if (run->field83_read)
        cpu.y = run->field83;
    if (!run->to_title) {
        stage_load_exit_shim(&run->load);
        return;
    }
    cpu.a = SCENE_MENU_A;
    abi_nz(SCENE_MENU_A);
    abi_flag(P_C, true);
    disc_log_exit_shim(&run->menu_a);
    cpu.a = SCENE_MENU_B;
    abi_nz(SCENE_MENU_B);
    disc_log_exit_shim(&run->menu_b);
    cpu.a = ADPCM_MENU;
    abi_nz(ADPCM_MENU);
    adpcm_load_exit_shim(&run->menu_adpcm);
}

void abi_StageEnter(void)
{
    StageEnterRun run = StageEnter();

    stage_enter_exit_shim(&run);
}

/*
 * @routine 0048:57C StageLoad
 * @run 6A:6D7C
 * WHAT IT DOES: Loads the stage in zStage: the character's scene if another is loaded,
 * the stage's two scenes, its ADPCM and a seek to its CD track, so the stage can start
 * with its music at once. Secret stages and stages $10 and $0C first copy VRAM to RAM.
 * HOW: StageScenes or, for bit 7, StageScenesSecret gives the scene by stage, through a pointer
 * in zTemp0A-0B; StageAdpcm and StageTrack, or their secret tables, give the ADPCM and the track.
 * Entry: nothing. Exit: as CdSearchTrack, entered with X the stage's index. Clobbers zTemp0A-0B.
 */
StageLoadRun StageLoad(void)
{
    StageLoadRun run;
    uint16_t scenes;
    Sum low;
    Sum high;
    bool secret;

    if (rd8(zStage) & STAGE_SECRET_BIT) {
        scenes = StageScenesSecret;
        wr8(zTemp0A, (uint8_t)scenes);
        wr8(zTemp0B, (uint8_t)(scenes >> 8));
        wr8(zField8E, 9);
    } else {
        scenes = StageScenes;
        wr8(zTemp0A, (uint8_t)scenes);
        wr8(zTemp0B, (uint8_t)(scenes >> 8));
        wr8(zField8E, 0);
        if (rd8(zStage) == 0)
            wr8(zField8E, (uint8_t)(rd8(zField8E) + 1));
    }
    low = add_bytes((uint8_t)(rd8(zStage) & STAGE_INDEX_MASK), rd8(zTemp0A), false);
    wr8(zTemp0A, low.value);
    high = add_bytes(0, rd8(zTemp0B), low.carry);
    wr8(zTemp0B, high.value);
    run.character = rd8(zCharacter);
    run.loaded_before = rd8(zCharacterLoaded);
    run.character_loaded = run.character != run.loaded_before;
    if (run.character_loaded) {
        wr8(zCharacterLoaded, run.character);
        run.character_scene = LoadSceneScript(run.character);
    }
    run.first_sum = add_bytes(rd8_zp(zTemp0A), rd8(zCharacter), false);
    run.first_scene = LoadSceneScript(run.first_sum.value);
    run.second = (uint8_t)(rd8_zp(zTemp0A) + 2);
    run.second_scene = LoadSceneScript(run.second);
    secret = (rd8(zStage) & STAGE_SECRET_BIT) != 0;
    run.saved = secret || rd8(zStage) == STAGE_KEEPS_VRAM_A || rd8(zStage) == STAGE_KEEPS_VRAM_B;
    if (run.saved)
        run.vram = SaveVramToRam();
    run.adpcm_index = (uint8_t)(rd8(zStage) & STAGE_INDEX_MASK);
    run.adpcm_sum = add_bytes(rd8((uint16_t)((secret ? StageAdpcmSecret : StageAdpcm) + run.adpcm_index)),
                              rd8(zCharacter), false);
    run.adpcm = AdpcmLoad(run.adpcm_sum.value);
    run.track_index = (uint8_t)(rd8(zStage) & STAGE_INDEX_MASK);
    run.track = rd8((uint16_t)((secret ? StageTrackSecret : StageTrack) + run.track_index));
    run.search = CdSearchTrack(run.track);
    return run;
}

void stage_load_exit_shim(const StageLoadRun *run)
{
    if (run->character_loaded) {
        cpu.a = run->character;
        abi_nz((uint8_t)(run->character - run->loaded_before));
        abi_flag(P_C, run->character >= run->loaded_before);
        disc_log_exit_shim(&run->character_scene);
    }
    cpu.a = run->first_sum.value;
    abi_nz(run->first_sum.value);
    abi_flag(P_C, run->first_sum.carry);
    abi_flag(P_V, run->first_sum.overflow);
    disc_log_exit_shim(&run->first_scene);
    cpu.a = run->second;
    abi_nz(run->second);
    disc_log_exit_shim(&run->second_scene);
    if (run->saved)
        save_vram_exit_shim(run->vram);
    cpu.x = run->adpcm_index;
    cpu.a = run->adpcm_sum.value;
    abi_nz(run->adpcm_sum.value);
    abi_flag(P_C, run->adpcm_sum.carry);
    abi_flag(P_V, run->adpcm_sum.overflow);
    adpcm_load_exit_shim(&run->adpcm);
    cpu.x = run->track_index;
    cpu.a = run->track;
    abi_nz(run->track);
    track_search_exit_shim(&run->search);
}

void abi_StageLoad(void)
{
    StageLoadRun run = StageLoad();

    stage_load_exit_shim(&run);
}

/*
 * @routine 0048:60A SaveVramToRam
 * @run 6A:6E0A
 * WHAT IT DOES: Copies $400 bytes of VRAM from $7E00 to RAM at $9C00, in the bank at
 * zRamBaseBank.
 * HOW: Eight rows of $80 bytes, read through the VDC's data port, through a pointer in
 * the BIOS's zBiosAl-Ah.
 * Entry: nothing. Exit: A the pointer's low byte, N and Z from X reaching zero, C from its
 * add, V too; X 0, Y $80.
 */
Sum SaveVramToRam(void)
{
    Sum low = { 0, false, false };

    tam(MPR4, rd8(zRamBaseBank));
    SelectVramReadAddress();
    st1(0x00);
    st2(VRAM_COPY_ADDR_HI);
    SelectVramData();
    wr8(zBiosAl, 0);
    wr8(zBiosAh, VRAM_COPY_RAM_HI);
    for (uint8_t row = 0; row < VRAM_COPY_ROWS; row++) {
        for (uint8_t at = 0; at < VRAM_COPY_ROW_BYTES; at += 2) {
            wr8_zp_y(zBiosAl, at, rd8(VDC_DATA_LO));
            wr8_zp_y(zBiosAl, (uint8_t)(at + 1), rd8(VDC_DATA_HI));
        }
        low = add_bytes(rd8(zBiosAl), VRAM_COPY_ROW_BYTES, false);
        wr8(zBiosAl, low.value);
        if (low.carry)
            wr8(zBiosAh, (uint8_t)(rd8(zBiosAh) + 1));
    }
    return low;
}

void save_vram_exit_shim(Sum low)
{
    cpu.a = low.value;
    cpu.x = 0;
    cpu.y = VRAM_COPY_ROW_BYTES;
    abi_nz(0);
    abi_flag(P_C, low.carry);
    abi_flag(P_V, low.overflow);
}

void abi_SaveVramToRam(void)
{
    save_vram_exit_shim(SaveVramToRam());
}
