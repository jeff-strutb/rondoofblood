/*
 * The stage overlay's start, entry loader, scroll step, object set-up, first spawn pair and frame step in the nine
 * copies of the overlay no read the game makes can load (sectors 026E, 031E, 043E, 064E, 10DE, 122E, 1A2E, 1B8E and
 * 1CEE, the code at offset $A2 of the sector, $4A bytes before 024E's at $EC). Each is 024E's routine with other
 * operands: the same bodies (stage_init.c and the others, which take their return addresses and tables as a layout)
 * run with this copy's layout, read from the copy's bytes. Differences from 024E that the layouts carry: the first
 * spawn pair starts list channel 4 (024E: 2), the health bar sounds are $0E and $4A (024E: $0F and $54), and the copies
 * at 031E, 043E and 10DE call the Long spawn and VRAM address routines.
 * StepScrollAndRunObjects ($8A05 here), the object spawn routines between SetUpStageObjects and the spawn pair, and the
 * rest of the frame step are not filed here.
 */
#include "machine.h"
#include "ops6280.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"
#include "stage_census.h"

void lines_exit_shim(Sum count);   /* object/stream.c */

/* As the 024E wrappers' exits: the scroll step leaves Y 1, V from the first bank pair's add, and the stream walk's. */
static void scroll_step_exit_shim(StageScrollEnd end)
{
    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/* The set-up ends with the stream walk's count and a JMP to the frame step: the C of the routine loaded there. */
static void set_up_exit_shim(SetUpEnd end, uint16_t jump_run, uint16_t tail_run)
{
    cpu.x = end.x;
    lines_exit_shim(end.count);
    cpu.x = end.x;
    run_routine_from(jump_run, tail_run);
}

/* What the pair leaves: A, X and Y (the second spawn's slot); N and Z from A. C and V are as the spawn left them. */
static void pair_exit_shim(StageSpawnEnd end)
{
    if (!end.ran) {
        return;
    }
    cpu.a = end.a;
    cpu.x = end.x;
    cpu.y = end.y;
    abi_nz(end.a);
}

static const InitStageLayout INIT_026E = { 0x88AF, 0x88D1, 0x88D7, 0x88DA, 0x88F4, 0x88FA,
    0x8889, 0xA8EC, 0xA94D };
static const ScrollStepLayout SCROLL_026E = { 0xADCE, 0x8A04 };
static const SetUpLayout SETUP_026E = { 0x9D23, 0xADCE, 0x8A18, 0x8A1B, 0x8A57 };
static const SpawnPairLayout SPAWN_026E = { 0x8B0D, 0x8B11, 0, 0, 4 };
static const StepFrameLayout FRAME_026E = {
    0x8B3A, 0x8B3F, 0x8B42, 0x8B45, 0x8B48, 0x8B77, 0x8B7E, 0x8B93, 0x8B9A, 0x8B8D, 0x8BA8, 0x8BAB, 0x8BBD, 0x8BC8,
    0x8BCF, 0x8C2A, 0x0E, 0x4A };

/*
 * @routine 026E:0A2 InitStage026E
 * @run 6B:88A2
 * @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 026E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
 * object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
 * loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
 * HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
 * WAITS: no.
 * Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
 */
StageExit InitStage026E(uint8_t x, uint8_t y)
{
    return InitStageBody(x, y, &INIT_026E);
}

void abi_InitStage026E(void)
{
    stage_exit_shim(InitStage026E(cpu.x, cpu.y));
}

/*
 * @routine 026E:108 LoadStageEntry026E
 * @run 6B:8908
 * @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 026E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
 * limits, the player's start position and the raster effect.
 * HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9C93, its character offsets at
 * $89E8.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
 * zTemp00-01.
 */
StageExit LoadStageEntry026E(void)
{
    return LoadStageEntryBody(0x9C93, 0x89E8);
}

void abi_LoadStageEntry026E(void)
{
    stage_exit_shim(LoadStageEntry026E());
}

/*
 * @routine 026E:1EC StageScrollStep026E
 * @run 6B:89EC
 * @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 026E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
 * offset the overlay keeps (the byte at $ADCE), into MPR5 and MPR6, then steps the stream with the scroll.
 * HOW: ScrollStepBody (stage_scroll.c).
 * WAITS: no.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStep026E(uint8_t x)
{
    return ScrollStepBody(x, &SCROLL_026E);
}

void abi_StageScrollStep026E(void)
{
    scroll_step_exit_shim(StageScrollStep026E(cpu.x));
}

/*
 * @routine 026E:215 SetUpStageObjects026E
 * @run 6B:8A15
 * @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 026E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
 * section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
 * and runs the first frame of objects.
 * HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9D23; the tail jump goes to $8A05.
 * WAITS: no.
 * Entry: X the caller's. Exit: as RunObjects.
 */
SetUpEnd SetUpStageObjects026E(uint8_t x)
{
    return SetUpObjectsBody(x, &SETUP_026E);
}

void abi_SetUpStageObjects026E(void)
{
    set_up_exit_shim(SetUpStageObjects026E(cpu.x), 0x8A57, 0x8A05);
}

/*
 * @routine 026E:309 SpawnStageObjectsRows01026E
 * @run 6B:8B09
 * @unreachable 6B:8B09-6B:8B21 no read the game makes loads disc sector 026E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
 * bit 5 of zField8E and starts list channel 4 with list $65.
 * HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObject's.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
 */
StageSpawnEnd SpawnStageObjectsRows01026E(void)
{
    return SpawnFirstPairBody(SpawnStageObject, &SPAWN_026E);
}

void abi_SpawnStageObjectsRows01026E(void)
{
    pair_exit_shim(SpawnStageObjectsRows01026E());
}

/*
 * @routine 026E:323 StepStageFrame026E
 * @run 6B:8B23
 * @stop 8BCF
 * @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 026E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
 * number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
 * the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
 * changed.
 * HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
 * SetVramAddresses.
 * WAITS: no.
 * Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
 */
StageExit StepStageFrame026E(uint8_t x, uint8_t y, bool v)
{
    return StepStageFrameBody(x, y, v, SetVramAddresses, &FRAME_026E);
}

void abi_StepStageFrame026E(void)
{
    stage_exit_shim(StepStageFrame026E(cpu.x, cpu.y, (cpu.p & P_V) != 0));
}

static const InitStageLayout INIT_031E = { 0x88AF, 0x88D1, 0x88D7, 0x88DA, 0x88F4, 0x88FA,
    0x8889, 0xA8EC, 0xA94D };
static const ScrollStepLayout SCROLL_031E = { 0xAD12, 0x8A04 };
static const SetUpLayout SETUP_031E = { 0x9E4F, 0xAD12, 0x8A18, 0x8A1B, 0x8A57 };
static const SpawnPairLayout SPAWN_031E = { 0x8B0D, 0x8B11, 0, 0, 4 };
static const StepFrameLayout FRAME_031E = {
    0x8B3A, 0x8B3F, 0x8B42, 0x8B45, 0x8B48, 0x8B77, 0x8B7E, 0x8B93, 0x8B9A, 0x8B8D, 0x8BA8, 0x8BAB, 0x8BBD, 0x8BC8,
    0x8BCF, 0x8C2A, 0x0E, 0x4A };

/*
 * @routine 031E:0A2 InitStage031E
 * @run 6B:88A2
 * @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 031E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
 * object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
 * loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
 * HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
 * WAITS: no.
 * Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
 */
StageExit InitStage031E(uint8_t x, uint8_t y)
{
    return InitStageBody(x, y, &INIT_031E);
}

void abi_InitStage031E(void)
{
    stage_exit_shim(InitStage031E(cpu.x, cpu.y));
}

/*
 * @routine 031E:108 LoadStageEntry031E
 * @run 6B:8908
 * @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 031E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
 * limits, the player's start position and the raster effect.
 * HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9E3F, its character offsets at
 * $89E8.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
 * zTemp00-01.
 */
StageExit LoadStageEntry031E(void)
{
    return LoadStageEntryBody(0x9E3F, 0x89E8);
}

void abi_LoadStageEntry031E(void)
{
    stage_exit_shim(LoadStageEntry031E());
}

/*
 * @routine 031E:1EC StageScrollStep031E
 * @run 6B:89EC
 * @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 031E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
 * offset the overlay keeps (the byte at $AD12), into MPR5 and MPR6, then steps the stream with the scroll.
 * HOW: ScrollStepBody (stage_scroll.c).
 * WAITS: no.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStep031E(uint8_t x)
{
    return ScrollStepBody(x, &SCROLL_031E);
}

void abi_StageScrollStep031E(void)
{
    scroll_step_exit_shim(StageScrollStep031E(cpu.x));
}

/*
 * @routine 031E:215 SetUpStageObjects031E
 * @run 6B:8A15
 * @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 031E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
 * section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
 * and runs the first frame of objects.
 * HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9E4F; the tail jump goes to $8A05.
 * WAITS: no.
 * Entry: X the caller's. Exit: as RunObjects.
 */
SetUpEnd SetUpStageObjects031E(uint8_t x)
{
    return SetUpObjectsBody(x, &SETUP_031E);
}

void abi_SetUpStageObjects031E(void)
{
    set_up_exit_shim(SetUpStageObjects031E(cpu.x), 0x8A57, 0x8A05);
}

/*
 * @routine 031E:309 SpawnStageObjectsRows01031E
 * @run 6B:8B09
 * @unreachable 6B:8B09-6B:8B21 no read the game makes loads disc sector 031E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
 * bit 5 of zField8E and starts list channel 4 with list $65.
 * HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObjectLong's.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
 */
StageSpawnEnd SpawnStageObjectsRows01031E(void)
{
    return SpawnFirstPairBody(SpawnStageObjectLong, &SPAWN_031E);
}

void abi_SpawnStageObjectsRows01031E(void)
{
    pair_exit_shim(SpawnStageObjectsRows01031E());
}

/*
 * @routine 031E:323 StepStageFrame031E
 * @run 6B:8B23
 * @stop 8BCF
 * @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 031E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
 * number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
 * the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
 * changed.
 * HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
 * SetVramAddressesLong.
 * WAITS: no.
 * Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
 */
StageExit StepStageFrame031E(uint8_t x, uint8_t y, bool v)
{
    return StepStageFrameBody(x, y, v, SetVramAddressesLong, &FRAME_031E);
}

void abi_StepStageFrame031E(void)
{
    stage_exit_shim(StepStageFrame031E(cpu.x, cpu.y, (cpu.p & P_V) != 0));
}

static const InitStageLayout INIT_043E = { 0x88AF, 0x88D1, 0x88D7, 0x88DA, 0x88F4, 0x88FA,
    0x8889, 0xA8EC, 0xA94D };
static const ScrollStepLayout SCROLL_043E = { 0xAD1A, 0x8A04 };
static const SetUpLayout SETUP_043E = { 0x9F22, 0xAD1A, 0x8A18, 0x8A1B, 0x8A57 };
static const SpawnPairLayout SPAWN_043E = { 0x8B0D, 0x8B11, 0, 0, 4 };
static const StepFrameLayout FRAME_043E = {
    0x8B3A, 0x8B3F, 0x8B42, 0x8B45, 0x8B48, 0x8B77, 0x8B7E, 0x8B93, 0x8B9A, 0x8B8D, 0x8BA8, 0x8BAB, 0x8BBD, 0x8BC8,
    0x8BCF, 0x8C2A, 0x0E, 0x4A };

/*
 * @routine 043E:0A2 InitStage043E
 * @run 6B:88A2
 * @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 043E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
 * object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
 * loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
 * HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
 * WAITS: no.
 * Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
 */
StageExit InitStage043E(uint8_t x, uint8_t y)
{
    return InitStageBody(x, y, &INIT_043E);
}

void abi_InitStage043E(void)
{
    stage_exit_shim(InitStage043E(cpu.x, cpu.y));
}

/*
 * @routine 043E:108 LoadStageEntry043E
 * @run 6B:8908
 * @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 043E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
 * limits, the player's start position and the raster effect.
 * HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9F02, its character offsets at
 * $89E8.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
 * zTemp00-01.
 */
StageExit LoadStageEntry043E(void)
{
    return LoadStageEntryBody(0x9F02, 0x89E8);
}

void abi_LoadStageEntry043E(void)
{
    stage_exit_shim(LoadStageEntry043E());
}

/*
 * @routine 043E:1EC StageScrollStep043E
 * @run 6B:89EC
 * @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 043E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
 * offset the overlay keeps (the byte at $AD1A), into MPR5 and MPR6, then steps the stream with the scroll.
 * HOW: ScrollStepBody (stage_scroll.c).
 * WAITS: no.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStep043E(uint8_t x)
{
    return ScrollStepBody(x, &SCROLL_043E);
}

void abi_StageScrollStep043E(void)
{
    scroll_step_exit_shim(StageScrollStep043E(cpu.x));
}

/*
 * @routine 043E:215 SetUpStageObjects043E
 * @run 6B:8A15
 * @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 043E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
 * section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
 * and runs the first frame of objects.
 * HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9F22; the tail jump goes to $8A05.
 * WAITS: no.
 * Entry: X the caller's. Exit: as RunObjects.
 */
SetUpEnd SetUpStageObjects043E(uint8_t x)
{
    return SetUpObjectsBody(x, &SETUP_043E);
}

void abi_SetUpStageObjects043E(void)
{
    set_up_exit_shim(SetUpStageObjects043E(cpu.x), 0x8A57, 0x8A05);
}

/*
 * @routine 043E:309 SpawnStageObjectsRows01043E
 * @run 6B:8B09
 * @unreachable 6B:8B09-6B:8B21 no read the game makes loads disc sector 043E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
 * bit 5 of zField8E and starts list channel 4 with list $65.
 * HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObjectLong's.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
 */
StageSpawnEnd SpawnStageObjectsRows01043E(void)
{
    return SpawnFirstPairBody(SpawnStageObjectLong, &SPAWN_043E);
}

void abi_SpawnStageObjectsRows01043E(void)
{
    pair_exit_shim(SpawnStageObjectsRows01043E());
}

/*
 * @routine 043E:323 StepStageFrame043E
 * @run 6B:8B23
 * @stop 8BCF
 * @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 043E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
 * number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
 * the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
 * changed.
 * HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
 * SetVramAddressesLong.
 * WAITS: no.
 * Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
 */
StageExit StepStageFrame043E(uint8_t x, uint8_t y, bool v)
{
    return StepStageFrameBody(x, y, v, SetVramAddressesLong, &FRAME_043E);
}

void abi_StepStageFrame043E(void)
{
    stage_exit_shim(StepStageFrame043E(cpu.x, cpu.y, (cpu.p & P_V) != 0));
}

static const InitStageLayout INIT_064E = { 0x88AF, 0x88D1, 0x88D7, 0x88DA, 0x88F4, 0x88FA,
    0x8889, 0xA8EC, 0xA94D };
static const ScrollStepLayout SCROLL_064E = { 0xAD82, 0x8A04 };
static const SetUpLayout SETUP_064E = { 0x9E22, 0xAD82, 0x8A18, 0x8A1B, 0x8A57 };
static const SpawnPairLayout SPAWN_064E = { 0x8B0D, 0x8B11, 0, 0, 4 };
static const StepFrameLayout FRAME_064E = {
    0x8B3A, 0x8B3F, 0x8B42, 0x8B45, 0x8B48, 0x8B77, 0x8B7E, 0x8B93, 0x8B9A, 0x8B8D, 0x8BA8, 0x8BAB, 0x8BBD, 0x8BC8,
    0x8BCF, 0x8C2A, 0x0E, 0x4A };

/*
 * @routine 064E:0A2 InitStage064E
 * @run 6B:88A2
 * @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 064E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
 * object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
 * loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
 * HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
 * WAITS: no.
 * Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
 */
StageExit InitStage064E(uint8_t x, uint8_t y)
{
    return InitStageBody(x, y, &INIT_064E);
}

void abi_InitStage064E(void)
{
    stage_exit_shim(InitStage064E(cpu.x, cpu.y));
}

/*
 * @routine 064E:108 LoadStageEntry064E
 * @run 6B:8908
 * @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 064E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
 * limits, the player's start position and the raster effect.
 * HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9D8E, its character offsets at
 * $89E8.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
 * zTemp00-01.
 */
StageExit LoadStageEntry064E(void)
{
    return LoadStageEntryBody(0x9D8E, 0x89E8);
}

void abi_LoadStageEntry064E(void)
{
    stage_exit_shim(LoadStageEntry064E());
}

/*
 * @routine 064E:1EC StageScrollStep064E
 * @run 6B:89EC
 * @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 064E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
 * offset the overlay keeps (the byte at $AD82), into MPR5 and MPR6, then steps the stream with the scroll.
 * HOW: ScrollStepBody (stage_scroll.c).
 * WAITS: no.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStep064E(uint8_t x)
{
    return ScrollStepBody(x, &SCROLL_064E);
}

void abi_StageScrollStep064E(void)
{
    scroll_step_exit_shim(StageScrollStep064E(cpu.x));
}

/*
 * @routine 064E:215 SetUpStageObjects064E
 * @run 6B:8A15
 * @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 064E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
 * section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
 * and runs the first frame of objects.
 * HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9E22; the tail jump goes to $8A05.
 * WAITS: no.
 * Entry: X the caller's. Exit: as RunObjects.
 */
SetUpEnd SetUpStageObjects064E(uint8_t x)
{
    return SetUpObjectsBody(x, &SETUP_064E);
}

void abi_SetUpStageObjects064E(void)
{
    set_up_exit_shim(SetUpStageObjects064E(cpu.x), 0x8A57, 0x8A05);
}

/*
 * @routine 064E:309 SpawnStageObjectsRows01064E
 * @run 6B:8B09
 * @unreachable 6B:8B09-6B:8B21 no read the game makes loads disc sector 064E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
 * bit 5 of zField8E and starts list channel 4 with list $65.
 * HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObject's.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
 */
StageSpawnEnd SpawnStageObjectsRows01064E(void)
{
    return SpawnFirstPairBody(SpawnStageObject, &SPAWN_064E);
}

void abi_SpawnStageObjectsRows01064E(void)
{
    pair_exit_shim(SpawnStageObjectsRows01064E());
}

/*
 * @routine 064E:323 StepStageFrame064E
 * @run 6B:8B23
 * @stop 8BCF
 * @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 064E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
 * number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
 * the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
 * changed.
 * HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
 * SetVramAddresses.
 * WAITS: no.
 * Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
 */
StageExit StepStageFrame064E(uint8_t x, uint8_t y, bool v)
{
    return StepStageFrameBody(x, y, v, SetVramAddresses, &FRAME_064E);
}

void abi_StepStageFrame064E(void)
{
    stage_exit_shim(StepStageFrame064E(cpu.x, cpu.y, (cpu.p & P_V) != 0));
}

static const InitStageLayout INIT_10DE = { 0x88AF, 0x88D1, 0x88D7, 0x88DA, 0x88F4, 0x88FA,
    0x8889, 0xA8EC, 0xA94D };
static const ScrollStepLayout SCROLL_10DE = { 0xAE5B, 0x8A04 };
static const SetUpLayout SETUP_10DE = { 0x9F10, 0xAE5B, 0x8A18, 0x8A1B, 0x8A57 };
static const SpawnPairLayout SPAWN_10DE = { 0x8B0D, 0x8B11, 0, 0, 4 };
static const StepFrameLayout FRAME_10DE = {
    0x8B3A, 0x8B3F, 0x8B42, 0x8B45, 0x8B48, 0x8B77, 0x8B7E, 0x8B93, 0x8B9A, 0x8B8D, 0x8BA8, 0x8BAB, 0x8BBD, 0x8BC8,
    0x8BCF, 0x8C2A, 0x0E, 0x4A };

/*
 * @routine 10DE:0A2 InitStage10DE
 * @run 6B:88A2
 * @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 10DE, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
 * object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
 * loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
 * HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
 * WAITS: no.
 * Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
 */
StageExit InitStage10DE(uint8_t x, uint8_t y)
{
    return InitStageBody(x, y, &INIT_10DE);
}

void abi_InitStage10DE(void)
{
    stage_exit_shim(InitStage10DE(cpu.x, cpu.y));
}

/*
 * @routine 10DE:108 LoadStageEntry10DE
 * @run 6B:8908
 * @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 10DE, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
 * limits, the player's start position and the raster effect.
 * HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9EF0, its character offsets at
 * $89E8.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
 * zTemp00-01.
 */
StageExit LoadStageEntry10DE(void)
{
    return LoadStageEntryBody(0x9EF0, 0x89E8);
}

void abi_LoadStageEntry10DE(void)
{
    stage_exit_shim(LoadStageEntry10DE());
}

/*
 * @routine 10DE:1EC StageScrollStep10DE
 * @run 6B:89EC
 * @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 10DE, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
 * offset the overlay keeps (the byte at $AE5B), into MPR5 and MPR6, then steps the stream with the scroll.
 * HOW: ScrollStepBody (stage_scroll.c).
 * WAITS: no.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStep10DE(uint8_t x)
{
    return ScrollStepBody(x, &SCROLL_10DE);
}

void abi_StageScrollStep10DE(void)
{
    scroll_step_exit_shim(StageScrollStep10DE(cpu.x));
}

/*
 * @routine 10DE:215 SetUpStageObjects10DE
 * @run 6B:8A15
 * @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 10DE, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
 * section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
 * and runs the first frame of objects.
 * HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9F10; the tail jump goes to $8A05.
 * WAITS: no.
 * Entry: X the caller's. Exit: as RunObjects.
 */
SetUpEnd SetUpStageObjects10DE(uint8_t x)
{
    return SetUpObjectsBody(x, &SETUP_10DE);
}

void abi_SetUpStageObjects10DE(void)
{
    set_up_exit_shim(SetUpStageObjects10DE(cpu.x), 0x8A57, 0x8A05);
}

/*
 * @routine 10DE:309 SpawnStageObjectsRows0110DE
 * @run 6B:8B09
 * @unreachable 6B:8B09-6B:8B21 no read the game makes loads disc sector 10DE, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
 * bit 5 of zField8E and starts list channel 4 with list $65.
 * HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObjectLong's.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
 */
StageSpawnEnd SpawnStageObjectsRows0110DE(void)
{
    return SpawnFirstPairBody(SpawnStageObjectLong, &SPAWN_10DE);
}

void abi_SpawnStageObjectsRows0110DE(void)
{
    pair_exit_shim(SpawnStageObjectsRows0110DE());
}

/*
 * @routine 10DE:323 StepStageFrame10DE
 * @run 6B:8B23
 * @stop 8BCF
 * @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 10DE, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
 * number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
 * the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
 * changed.
 * HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
 * SetVramAddressesLong.
 * WAITS: no.
 * Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
 */
StageExit StepStageFrame10DE(uint8_t x, uint8_t y, bool v)
{
    return StepStageFrameBody(x, y, v, SetVramAddressesLong, &FRAME_10DE);
}

void abi_StepStageFrame10DE(void)
{
    stage_exit_shim(StepStageFrame10DE(cpu.x, cpu.y, (cpu.p & P_V) != 0));
}

static const InitStageLayout INIT_122E = { 0x88AF, 0x88D1, 0x88D7, 0x88DA, 0x88F4, 0x88FA,
    0x8889, 0xA8EC, 0xA94D };
static const ScrollStepLayout SCROLL_122E = { 0xAE33, 0x8A04 };
static const SetUpLayout SETUP_122E = { 0x9E48, 0xAE33, 0x8A18, 0x8A1B, 0x8A57 };
static const SpawnPairLayout SPAWN_122E = { 0x8B0D, 0x8B11, 0, 0, 4 };
static const StepFrameLayout FRAME_122E = {
    0x8B3A, 0x8B3F, 0x8B42, 0x8B45, 0x8B48, 0x8B77, 0x8B7E, 0x8B93, 0x8B9A, 0x8B8D, 0x8BA8, 0x8BAB, 0x8BBD, 0x8BC8,
    0x8BCF, 0x8C2A, 0x0E, 0x4A };

/*
 * @routine 122E:0A2 InitStage122E
 * @run 6B:88A2
 * @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 122E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
 * object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
 * loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
 * HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
 * WAITS: no.
 * Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
 */
StageExit InitStage122E(uint8_t x, uint8_t y)
{
    return InitStageBody(x, y, &INIT_122E);
}

void abi_InitStage122E(void)
{
    stage_exit_shim(InitStage122E(cpu.x, cpu.y));
}

/*
 * @routine 122E:108 LoadStageEntry122E
 * @run 6B:8908
 * @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 122E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
 * limits, the player's start position and the raster effect.
 * HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9D08, its character offsets at
 * $89E8.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
 * zTemp00-01.
 */
StageExit LoadStageEntry122E(void)
{
    return LoadStageEntryBody(0x9D08, 0x89E8);
}

void abi_LoadStageEntry122E(void)
{
    stage_exit_shim(LoadStageEntry122E());
}

/*
 * @routine 122E:1EC StageScrollStep122E
 * @run 6B:89EC
 * @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 122E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
 * offset the overlay keeps (the byte at $AE33), into MPR5 and MPR6, then steps the stream with the scroll.
 * HOW: ScrollStepBody (stage_scroll.c).
 * WAITS: no.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStep122E(uint8_t x)
{
    return ScrollStepBody(x, &SCROLL_122E);
}

void abi_StageScrollStep122E(void)
{
    scroll_step_exit_shim(StageScrollStep122E(cpu.x));
}

/*
 * @routine 122E:215 SetUpStageObjects122E
 * @run 6B:8A15
 * @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 122E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
 * section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
 * and runs the first frame of objects.
 * HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9E48; the tail jump goes to $8A05.
 * WAITS: no.
 * Entry: X the caller's. Exit: as RunObjects.
 */
SetUpEnd SetUpStageObjects122E(uint8_t x)
{
    return SetUpObjectsBody(x, &SETUP_122E);
}

void abi_SetUpStageObjects122E(void)
{
    set_up_exit_shim(SetUpStageObjects122E(cpu.x), 0x8A57, 0x8A05);
}

/*
 * @routine 122E:309 SpawnStageObjectsRows01122E
 * @run 6B:8B09
 * @unreachable 6B:8B09-6B:8B21 no read the game makes loads disc sector 122E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
 * bit 5 of zField8E and starts list channel 4 with list $65.
 * HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObject's.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
 */
StageSpawnEnd SpawnStageObjectsRows01122E(void)
{
    return SpawnFirstPairBody(SpawnStageObject, &SPAWN_122E);
}

void abi_SpawnStageObjectsRows01122E(void)
{
    pair_exit_shim(SpawnStageObjectsRows01122E());
}

/*
 * @routine 122E:323 StepStageFrame122E
 * @run 6B:8B23
 * @stop 8BCF
 * @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 122E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
 * number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
 * the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
 * changed.
 * HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
 * SetVramAddresses.
 * WAITS: no.
 * Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
 */
StageExit StepStageFrame122E(uint8_t x, uint8_t y, bool v)
{
    return StepStageFrameBody(x, y, v, SetVramAddresses, &FRAME_122E);
}

void abi_StepStageFrame122E(void)
{
    stage_exit_shim(StepStageFrame122E(cpu.x, cpu.y, (cpu.p & P_V) != 0));
}

static const InitStageLayout INIT_1A2E = { 0x88AF, 0x88D1, 0x88D7, 0x88DA, 0x88F4, 0x88FA,
    0x8889, 0xA8EC, 0xA94D };
static const ScrollStepLayout SCROLL_1A2E = { 0xADC9, 0x8A04 };
static const SetUpLayout SETUP_1A2E = { 0x9E12, 0xADC9, 0x8A18, 0x8A1B, 0x8A57 };
static const SpawnPairLayout SPAWN_1A2E = { 0x8B0D, 0x8B11, 0, 0, 4 };
static const StepFrameLayout FRAME_1A2E = {
    0x8B3A, 0x8B3F, 0x8B42, 0x8B45, 0x8B48, 0x8B77, 0x8B7E, 0x8B93, 0x8B9A, 0x8B8D, 0x8BA8, 0x8BAB, 0x8BBD, 0x8BC8,
    0x8BCF, 0x8C2A, 0x0E, 0x4A };

/*
 * @routine 1A2E:0A2 InitStage1A2E
 * @run 6B:88A2
 * @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 1A2E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
 * object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
 * loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
 * HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
 * WAITS: no.
 * Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
 */
StageExit InitStage1A2E(uint8_t x, uint8_t y)
{
    return InitStageBody(x, y, &INIT_1A2E);
}

void abi_InitStage1A2E(void)
{
    stage_exit_shim(InitStage1A2E(cpu.x, cpu.y));
}

/*
 * @routine 1A2E:108 LoadStageEntry1A2E
 * @run 6B:8908
 * @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 1A2E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
 * limits, the player's start position and the raster effect.
 * HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9D52, its character offsets at
 * $89E8.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
 * zTemp00-01.
 */
StageExit LoadStageEntry1A2E(void)
{
    return LoadStageEntryBody(0x9D52, 0x89E8);
}

void abi_LoadStageEntry1A2E(void)
{
    stage_exit_shim(LoadStageEntry1A2E());
}

/*
 * @routine 1A2E:1EC StageScrollStep1A2E
 * @run 6B:89EC
 * @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 1A2E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
 * offset the overlay keeps (the byte at $ADC9), into MPR5 and MPR6, then steps the stream with the scroll.
 * HOW: ScrollStepBody (stage_scroll.c).
 * WAITS: no.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStep1A2E(uint8_t x)
{
    return ScrollStepBody(x, &SCROLL_1A2E);
}

void abi_StageScrollStep1A2E(void)
{
    scroll_step_exit_shim(StageScrollStep1A2E(cpu.x));
}

/*
 * @routine 1A2E:215 SetUpStageObjects1A2E
 * @run 6B:8A15
 * @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 1A2E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
 * section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
 * and runs the first frame of objects.
 * HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9E12; the tail jump goes to $8A05.
 * WAITS: no.
 * Entry: X the caller's. Exit: as RunObjects.
 */
SetUpEnd SetUpStageObjects1A2E(uint8_t x)
{
    return SetUpObjectsBody(x, &SETUP_1A2E);
}

void abi_SetUpStageObjects1A2E(void)
{
    set_up_exit_shim(SetUpStageObjects1A2E(cpu.x), 0x8A57, 0x8A05);
}

/*
 * @routine 1A2E:309 SpawnStageObjectsRows011A2E
 * @run 6B:8B09
 * @unreachable 6B:8B09-6B:8B21 no read the game makes loads disc sector 1A2E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
 * bit 5 of zField8E and starts list channel 4 with list $65.
 * HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObject's.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
 */
StageSpawnEnd SpawnStageObjectsRows011A2E(void)
{
    return SpawnFirstPairBody(SpawnStageObject, &SPAWN_1A2E);
}

void abi_SpawnStageObjectsRows011A2E(void)
{
    pair_exit_shim(SpawnStageObjectsRows011A2E());
}

/*
 * @routine 1A2E:323 StepStageFrame1A2E
 * @run 6B:8B23
 * @stop 8BCF
 * @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 1A2E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
 * number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
 * the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
 * changed.
 * HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
 * SetVramAddresses.
 * WAITS: no.
 * Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
 */
StageExit StepStageFrame1A2E(uint8_t x, uint8_t y, bool v)
{
    return StepStageFrameBody(x, y, v, SetVramAddresses, &FRAME_1A2E);
}

void abi_StepStageFrame1A2E(void)
{
    stage_exit_shim(StepStageFrame1A2E(cpu.x, cpu.y, (cpu.p & P_V) != 0));
}

static const InitStageLayout INIT_1B8E = { 0x88AF, 0x88D1, 0x88D7, 0x88DA, 0x88F4, 0x88FA,
    0x8889, 0xA8EC, 0xA94D };
static const ScrollStepLayout SCROLL_1B8E = { 0xAE25, 0x8A04 };
static const SetUpLayout SETUP_1B8E = { 0x9EC3, 0xAE25, 0x8A18, 0x8A1B, 0x8A57 };
static const SpawnPairLayout SPAWN_1B8E = { 0x8B0D, 0x8B11, 0, 0, 4 };
static const StepFrameLayout FRAME_1B8E = {
    0x8B3A, 0x8B3F, 0x8B42, 0x8B45, 0x8B48, 0x8B77, 0x8B7E, 0x8B93, 0x8B9A, 0x8B8D, 0x8BA8, 0x8BAB, 0x8BBD, 0x8BC8,
    0x8BCF, 0x8C2A, 0x0E, 0x4A };

/*
 * @routine 1B8E:0A2 InitStage1B8E
 * @run 6B:88A2
 * @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 1B8E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
 * object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
 * loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
 * HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
 * WAITS: no.
 * Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
 */
StageExit InitStage1B8E(uint8_t x, uint8_t y)
{
    return InitStageBody(x, y, &INIT_1B8E);
}

void abi_InitStage1B8E(void)
{
    stage_exit_shim(InitStage1B8E(cpu.x, cpu.y));
}

/*
 * @routine 1B8E:108 LoadStageEntry1B8E
 * @run 6B:8908
 * @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 1B8E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
 * limits, the player's start position and the raster effect.
 * HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9D63, its character offsets at
 * $89E8.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
 * zTemp00-01.
 */
StageExit LoadStageEntry1B8E(void)
{
    return LoadStageEntryBody(0x9D63, 0x89E8);
}

void abi_LoadStageEntry1B8E(void)
{
    stage_exit_shim(LoadStageEntry1B8E());
}

/*
 * @routine 1B8E:1EC StageScrollStep1B8E
 * @run 6B:89EC
 * @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 1B8E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
 * offset the overlay keeps (the byte at $AE25), into MPR5 and MPR6, then steps the stream with the scroll.
 * HOW: ScrollStepBody (stage_scroll.c).
 * WAITS: no.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStep1B8E(uint8_t x)
{
    return ScrollStepBody(x, &SCROLL_1B8E);
}

void abi_StageScrollStep1B8E(void)
{
    scroll_step_exit_shim(StageScrollStep1B8E(cpu.x));
}

/*
 * @routine 1B8E:215 SetUpStageObjects1B8E
 * @run 6B:8A15
 * @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 1B8E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
 * section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
 * and runs the first frame of objects.
 * HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9EC3; the tail jump goes to $8A05.
 * WAITS: no.
 * Entry: X the caller's. Exit: as RunObjects.
 */
SetUpEnd SetUpStageObjects1B8E(uint8_t x)
{
    return SetUpObjectsBody(x, &SETUP_1B8E);
}

void abi_SetUpStageObjects1B8E(void)
{
    set_up_exit_shim(SetUpStageObjects1B8E(cpu.x), 0x8A57, 0x8A05);
}

/*
 * @routine 1B8E:309 SpawnStageObjectsRows011B8E
 * @run 6B:8B09
 * @unreachable 6B:8B09-6B:8B21 no read the game makes loads disc sector 1B8E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
 * bit 5 of zField8E and starts list channel 4 with list $65.
 * HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObject's.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
 */
StageSpawnEnd SpawnStageObjectsRows011B8E(void)
{
    return SpawnFirstPairBody(SpawnStageObject, &SPAWN_1B8E);
}

void abi_SpawnStageObjectsRows011B8E(void)
{
    pair_exit_shim(SpawnStageObjectsRows011B8E());
}

/*
 * @routine 1B8E:323 StepStageFrame1B8E
 * @run 6B:8B23
 * @stop 8BCF
 * @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 1B8E, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
 * number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
 * the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
 * changed.
 * HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
 * SetVramAddresses.
 * WAITS: no.
 * Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
 */
StageExit StepStageFrame1B8E(uint8_t x, uint8_t y, bool v)
{
    return StepStageFrameBody(x, y, v, SetVramAddresses, &FRAME_1B8E);
}

void abi_StepStageFrame1B8E(void)
{
    stage_exit_shim(StepStageFrame1B8E(cpu.x, cpu.y, (cpu.p & P_V) != 0));
}

static const InitStageLayout INIT_1CEE = { 0x88AF, 0x88D1, 0x88D7, 0x88DA, 0x88F4, 0x88FA,
    0x8889, 0xA8EC, 0xA94D };
static const ScrollStepLayout SCROLL_1CEE = { 0xAE38, 0x8A04 };
static const SetUpLayout SETUP_1CEE = { 0x9D76, 0xAE38, 0x8A18, 0x8A1B, 0x8A57 };
static const SpawnPairLayout SPAWN_1CEE = { 0x8B0D, 0x8B11, 0, 0, 4 };
static const StepFrameLayout FRAME_1CEE = {
    0x8B3A, 0x8B3F, 0x8B42, 0x8B45, 0x8B48, 0x8B77, 0x8B7E, 0x8B93, 0x8B9A, 0x8B8D, 0x8BA8, 0x8BAB, 0x8BBD, 0x8BC8,
    0x8BCF, 0x8C2A, 0x0E, 0x4A };

/*
 * @routine 1CEE:0A2 InitStage1CEE
 * @run 6B:88A2
 * @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 1CEE, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
 * object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
 * loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
 * HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
 * WAITS: no.
 * Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
 */
StageExit InitStage1CEE(uint8_t x, uint8_t y)
{
    return InitStageBody(x, y, &INIT_1CEE);
}

void abi_InitStage1CEE(void)
{
    stage_exit_shim(InitStage1CEE(cpu.x, cpu.y));
}

/*
 * @routine 1CEE:108 LoadStageEntry1CEE
 * @run 6B:8908
 * @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 1CEE, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
 * limits, the player's start position and the raster effect.
 * HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9CF6, its character offsets at
 * $89E8.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
 * zTemp00-01.
 */
StageExit LoadStageEntry1CEE(void)
{
    return LoadStageEntryBody(0x9CF6, 0x89E8);
}

void abi_LoadStageEntry1CEE(void)
{
    stage_exit_shim(LoadStageEntry1CEE());
}

/*
 * @routine 1CEE:1EC StageScrollStep1CEE
 * @run 6B:89EC
 * @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 1CEE, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
 * offset the overlay keeps (the byte at $AE38), into MPR5 and MPR6, then steps the stream with the scroll.
 * HOW: ScrollStepBody (stage_scroll.c).
 * WAITS: no.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStep1CEE(uint8_t x)
{
    return ScrollStepBody(x, &SCROLL_1CEE);
}

void abi_StageScrollStep1CEE(void)
{
    scroll_step_exit_shim(StageScrollStep1CEE(cpu.x));
}

/*
 * @routine 1CEE:215 SetUpStageObjects1CEE
 * @run 6B:8A15
 * @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 1CEE, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
 * section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
 * and runs the first frame of objects.
 * HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9D76; the tail jump goes to $8A05.
 * WAITS: no.
 * Entry: X the caller's. Exit: as RunObjects.
 */
SetUpEnd SetUpStageObjects1CEE(uint8_t x)
{
    return SetUpObjectsBody(x, &SETUP_1CEE);
}

void abi_SetUpStageObjects1CEE(void)
{
    set_up_exit_shim(SetUpStageObjects1CEE(cpu.x), 0x8A57, 0x8A05);
}

/*
 * @routine 1CEE:309 SpawnStageObjectsRows011CEE
 * @run 6B:8B09
 * @unreachable 6B:8B09-6B:8B21 no read the game makes loads disc sector 1CEE, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
 * bit 5 of zField8E and starts list channel 4 with list $65.
 * HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObject's.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
 */
StageSpawnEnd SpawnStageObjectsRows011CEE(void)
{
    return SpawnFirstPairBody(SpawnStageObject, &SPAWN_1CEE);
}

void abi_SpawnStageObjectsRows011CEE(void)
{
    pair_exit_shim(SpawnStageObjectsRows011CEE());
}

/*
 * @routine 1CEE:323 StepStageFrame1CEE
 * @run 6B:8B23
 * @stop 8BCF
 * @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 1CEE, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
 * number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
 * the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
 * changed.
 * HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
 * SetVramAddresses.
 * WAITS: no.
 * Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
 */
StageExit StepStageFrame1CEE(uint8_t x, uint8_t y, bool v)
{
    return StepStageFrameBody(x, y, v, SetVramAddresses, &FRAME_1CEE);
}

void abi_StepStageFrame1CEE(void)
{
    stage_exit_shim(StepStageFrame1CEE(cpu.x, cpu.y, (cpu.p & P_V) != 0));
}

