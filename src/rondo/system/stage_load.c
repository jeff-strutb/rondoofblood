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
    STAGE_TABLE_SECRET_LO = 0xC2, STAGE_TABLE_LO = 0xB1, STAGE_TABLE_HI = 0x74,
    VRAM_COPY_ROWS = 8, VRAM_COPY_ROW_BYTES = 0x80, VRAM_COPY_ADDR_HI = 0x7E,
    VRAM_COPY_RAM_HI = 0x9C
};

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
Alu StageEnter(Alu r)
{
    uint8_t byte;
    bool finished = false;

    wr8(zField80, 0);
    wr8(zField89, 0);
    wr8(zField85, 0);
    wr8(zField82, 0);
    wr8(zField8F, 0);
    wr8(zField90, 0);
    wr8(zField91, 0);
    wr8(zField92, 0);
    wr8(zField99, 0);
    alu_load(&r, 5);
    wr8(zField95, 5);
    byte = rd8(zField8C);
    alu_load(&r, (uint8_t)(byte & 0x7F));
    wr8(zField8C, r.a);
    r = call_abi(abi_ResetGameState, r);
    r = call_abi(abi_SoundStopAll, r);
    byte = rd8(zStage);
    wr8(zStageRequested, byte);
    alu_load(&r, (uint8_t)(byte & STAGE_RESTART_BIT));
    if (r.a != 0) {
        byte = rd8(zStage);
        alu_load(&r, (uint8_t)(byte & 0xBF));
        wr8(zStage, r.a);
        wr8(zField8A, 0);
    } else {
        byte = rd8(zStage);
        alu_load(&r, byte);
        if (!(byte & STAGE_SECRET_BIT)) {
            byte = (uint8_t)((byte | STAGE_SECRET_BIT) & ~STAGE_ROUTE_BIT);
            alu_load(&r, byte);
            cpu.x = rd8(zField8C);
            alu_compare(&r, cpu.x, 0x05);
            r.a = byte;
            if (cpu.x >= 0x05) {
                byte |= STAGE_ROUTE_BIT;
                alu_load(&r, byte);
            }
            wr8(zStage, byte);
            alu_load(&r, 3);
            wr8(zField8A, 3);
        } else {
            byte = (uint8_t)((byte & STAGE_INDEX_MASK) + 2);
            alu_load(&r, byte);
            wr8(zStage, byte);
            alu_load(&r, 1);
            wr8(zField8A, 1);
        }
        wr8(zField8C, 0);
    }
    byte = rd8(zStageRequested);
    alu_load(&r, (uint8_t)(byte & STAGE_RESTART_BIT));
    if (r.a == 0) {
        byte = rd8(zStage);
        alu_load(&r, byte);
        alu_compare(&r, byte, 0x80);
        if (byte == 0x80) {
            alu_load(&r, 1);
            wr8(zField8A, 1);
            alu_load(&r, 2);
            wr8(zStage, 2);
        } else if (alu_compare(&r, byte, 0x02), byte == 0x02) {
            alu_load(&r, 0);
            wr8(zStage, 0);
            alu_load(&r, SCENE_MENU_A);
            r = LoadSceneScript(r);
            alu_load(&r, SCENE_MENU_B);
            r = LoadSceneScript(r);
            alu_load(&r, ADPCM_MENU);
            r = AdpcmLoad(r);
            finished = true;
        } else if (alu_compare(&r, byte, 0x85), byte == 0x85) {
            cpu.y = rd8(zField83);
            alu_compare(&r, cpu.y, 0x0A);
            r.a = byte;
            if (cpu.y == 0x0A) {
                alu_load(&r, 1);
                wr8(zField8A, 1);
                alu_load(&r, 7);
            }
            wr8(zStage, r.a);
        } else if (alu_compare(&r, byte, 0x0B), byte == 0x0B) {
            uint8_t first = rd8(0x3F51);
            uint8_t second = rd8(0x3F56);

            alu_load(&r, (uint8_t)((first | second) & 0x01));
            if (r.a == 0) {
                alu_load(&r, 0x4A);
                wr8(zStageRequested, 0x4A);
                alu_load(&r, 0x0A);
            } else {
                alu_load(&r, 0x0B);
            }
            wr8(zStage, r.a);
        } else if (alu_compare(&r, byte, 0x8B), byte == 0x8B) {
            alu_load(&r, 1);
            wr8(zField8A, 1);
            alu_load(&r, 0x4C);
            wr8(zStageRequested, 0x4C);
            alu_load(&r, 0x0C);
            wr8(zStage, 0x0C);
        } else if (alu_compare(&r, byte, 0x8C), byte == 0x8C) {
            alu_load(&r, 1);
            wr8(zField8A, 1);
            alu_load(&r, 0x4E);
            wr8(zStageRequested, 0x4E);
            alu_load(&r, 0x0E);
            wr8(zStage, 0x0E);
        }
    }
    if (finished)
        return r;
    wr8(zField83, 0);
    return StageLoad(r);
}

void abi_StageEnter(void)
{
    abi_alu(StageEnter(alu_from_cpu()));
}

/*
 * @routine 0048:57C StageLoad
 * @run 6A:6D7C
 * WHAT IT DOES: Loads the stage in zStage: the character's scene if another is loaded,
 * the stage's two scenes, its ADPCM and a seek to its CD track, so the stage can start
 * with its music at once. Secret stages and stages $10 and $0C first copy VRAM to RAM.
 * HOW: A table at $74B1 or, for bit 7, at $74C2 gives the scene by stage; tables at $7230
 * and $7115, or $7241 and $7126, give the ADPCM and the track.
 * Entry: nothing. Exit: as CdSearchTrack. Clobbers zTemp0A-0B.
 */
Alu StageLoad(Alu r)
{
    uint8_t stage = rd8(zStage);
    uint8_t byte;
    uint16_t adpcm_table;
    uint16_t track_table;

    alu_load(&r, stage);
    if (stage & 0x80) {
        alu_load(&r, STAGE_TABLE_SECRET_LO);
        wr8(zTemp0A, STAGE_TABLE_SECRET_LO);
        alu_load(&r, STAGE_TABLE_HI);
        wr8(zTemp0B, STAGE_TABLE_HI);
        alu_load(&r, 9);
        wr8(zField8E, 9);
    } else {
        alu_load(&r, STAGE_TABLE_LO);
        wr8(zTemp0A, STAGE_TABLE_LO);
        alu_load(&r, STAGE_TABLE_HI);
        wr8(zTemp0B, STAGE_TABLE_HI);
        wr8(zField8E, 0);
        stage = rd8(zStage);
        alu_load(&r, stage);
        if (stage == 0) {
            byte = rd8(zField8E);
            alu_nz(&r, (uint8_t)(byte + 1));
            wr8(zField8E, (uint8_t)(byte + 1));
        }
    }
    stage = rd8(zStage);
    byte = rd8(zTemp0A);
    r = adc8((uint8_t)(stage & STAGE_INDEX_MASK), byte, false);
    wr8(zTemp0A, r.a);
    byte = rd8(zTemp0B);
    r = adc8(0, byte, r.c);
    wr8(zTemp0B, r.a);
    byte = rd8(zCharacter);
    {
        uint8_t loaded = rd8(zCharacterLoaded);

        alu_compare(&r, byte, loaded);
        if (byte != loaded) {
            wr8(zCharacterLoaded, byte);
            r = LoadSceneScript(r);
        }
    }
    byte = rd8_zp(zTemp0A);
    r = adc8(byte, rd8(zCharacter), false);
    r = LoadSceneScript(r);
    byte = rd8_zp(zTemp0A);
    alu_load(&r, (uint8_t)(byte + 2));
    r = LoadSceneScript(r);
    stage = rd8(zStage);
    alu_load(&r, stage);
    if (stage & 0x80) {
        r = SaveVramToRam(r);
        adpcm_table = StageAdpcmSecret;
        track_table = StageTrackSecret;
    } else {
        alu_compare(&r, stage, 0x10);
        if (stage == 0x10) {
            r = SaveVramToRam(r);
        } else {
            alu_compare(&r, stage, 0x0C);
            if (stage == 0x0C)
                r = SaveVramToRam(r);
        }
        adpcm_table = StageAdpcm;
        track_table = StageTrack;
    }
    stage = rd8(zStage);
    cpu.x = (uint8_t)(stage & STAGE_INDEX_MASK);
    byte = rd8((uint16_t)(adpcm_table + cpu.x));
    r = adc8(byte, rd8(zCharacter), false);
    r = AdpcmLoad(r);
    stage = rd8(zStage);
    cpu.x = (uint8_t)(stage & STAGE_INDEX_MASK);
    byte = rd8((uint16_t)(track_table + cpu.x));
    alu_load(&r, byte);
    return CdSearchTrack(r);
}

void abi_StageLoad(void)
{
    abi_alu(StageLoad(alu_from_cpu()));
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
Alu SaveVramToRam(Alu r)
{
    uint8_t base = rd8(zRamBaseBank);
    uint8_t byte;

    r = adc8(0, base, false);
    tam(MPR4, r.a);
    r = call_abi(abi_SelectVramWriteAddress, r);
    st1(0x00);
    st2(VRAM_COPY_ADDR_HI);
    r = call_abi(abi_SelectVramData, r);
    wr8(zBiosAl, 0);
    alu_load(&r, VRAM_COPY_RAM_HI);
    wr8(zBiosAh, VRAM_COPY_RAM_HI);
    for (uint8_t row = VRAM_COPY_ROWS; row != 0; row--) {
        cpu.x = row;
        cpu.y = 0;
        do {
            byte = rd8(VDC_DATA_LO);
            wr8_zp_y(zBiosAl, cpu.y, byte);
            cpu.y++;
            byte = rd8(VDC_DATA_HI);
            wr8_zp_y(zBiosAl, cpu.y, byte);
            cpu.y++;
        } while (cpu.y != VRAM_COPY_ROW_BYTES);
        byte = rd8(zBiosAl);
        r = adc8(byte, VRAM_COPY_ROW_BYTES, false);
        wr8(zBiosAl, r.a);
        if (r.c) {
            byte = rd8(zBiosAh);
            wr8(zBiosAh, (uint8_t)(byte + 1));
        }
    }
    cpu.x = 0;
    alu_nz(&r, 0);
    return r;
}

void abi_SaveVramToRam(void)
{
    abi_alu(SaveVramToRam(alu_from_cpu()));
}
