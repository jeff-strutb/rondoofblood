/*
 * The stage overlay of disc sectors 152D to 1530 (8 K at $8000, bank $6B as the stage overlays run), which no read the
 * game makes can load. It is another build of the stage overlay at sector 024D: the same frame of sub-modes at
 * zGameSubMode (start, play, the player's death, the stage's end and the rest), its own code for most of them. The
 * entries at $8000 to $8009 and the sub-mode table at $8013 lead to everything filed here; the object handlers at
 * $877B and $87C6 and the routine at $9651 are reached from no code in the overlay (another overlay's tables).
 * What lies between the routines and is not filed: the jump tables (each named in the routine that jumps through it),
 * and the data the routines read ($8209, $82F0 to $82FB, $8A79, $8AC0 and the rest), left as bytes from the disc.
 * $8021 to $8033 are the bytes of MapOverlayBanks5And6 and MapOverlayBank7 (player_leaves.c, 024D:0A0 and 024D:0B7):
 * a census duplicate, called here through their C.
 *
 * Fields with no name yet, kept in their address form until the owner of the lists adds them: zero page $3C (1 while
 * the stage's fade runs), $84, $85 (the step of sub-mode 3), $88 (the step of the death's fall), $9B (the fade
 * countdown) and $9D (the sound the fade sends); $2743; the bytes at $3E00 to $3E74 that the overlay at $A000 keeps.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "stage_map.h"
#include "enemy_objects.h"
#include "syscard.h"
#include "stage_b_census.h"

enum {
    FADE_ACTIVE = 0x203C, SUB3_STEP_TABLE = 0x2084, SUB3_STEP = 0x2085, FALL_STEP = 0x2088, FADE_COUNT = 0x209B,
    FADE_SOUND = 0x209D, STAGE_FLAG_2743 = 0x2743,
    KERNEL_4006 = 0x4006, KERNEL_4009 = 0x4009, SOUND_HOOK = 0x401B, KERNEL_407B = 0x407B,
    SUB_MODE_TABLE = 0x8013, SUB_STEP_TABLE = 0x8112, DEATH_PHASE_TABLE = 0x81E8, FACING_SPEEDS = 0x8209,
    PLAYER_X_COUNT = 0x3310, PLAYER_X_STEP = 0x3358
};

typedef StageRegs Regs;

Ended MapOverlayBanks5And6(void);
Ended MapOverlayBank7(void);

/* The registers and flags the routine is entered with: N and Z as the flags hold them. */
static Regs regs_entry_shim(void)
{
    Regs r = { cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0, 0 };

    r.nz = (cpu.p & P_Z) != 0 ? 0 : ((cpu.p & P_N) != 0 ? 0x80 : 0x01);
    return r;
}

/* What the routine leaves, set on the machine. */
static void regs_exit_shim(Regs r)
{
    stage_regs_load_shim(r);
}

/* A JSR into code with no C: the registers as they stand, and what it leaves. */
static Regs orig_shim(Regs r, uint16_t ret, uint16_t run)
{
    stage_regs_load_shim(r);
    call_orig(ret, run);
    return stage_regs_from_cpu_shim(r);
}

/* A tail JMP into code with no C: it returns for this routine. */
static Regs orig_tail_shim(Regs r, uint16_t run)
{
    stage_regs_load_shim(r);
    call_original(run);
    return stage_regs_from_cpu_shim(r);
}

/* A load into A: N and Z from the value. */
static Regs loaded(Regs r, uint8_t value)
{
    r.a = value;
    r.nz = value;
    return r;
}

/* A compare of A: C and the difference N and Z come from. */
static Regs compared(Regs r, uint8_t value)
{
    r.c = r.a >= value;
    r.nz = (uint8_t)(r.a - value);
    return r;
}

/* What one of the filed bank routines leaves, laid over the registers. */
static Regs after_ended(Regs r, Ended e)
{
    if (e.a_set) {
        r.a = e.a;
    }
    if (e.nz_set) {
        r.nz = e.nz;
    }
    if (e.c_set) {
        r.c = e.c;
    }
    if (e.v_set) {
        r.v = e.v;
    }
    if (e.y_set) {
        r.y = e.y;
    }
    return r;
}

/* JSR $8021: the bytes of MapOverlayBanks5And6. */
static Regs map_banks_5_6(Regs r, uint16_t ret)
{
    jsr_push(ret);
    r = after_ended(r, MapOverlayBanks5And6());
    jsr_pop();
    return r;
}

/* JSR $802C: the bytes of MapOverlayBank7. */
static Regs map_bank_7(Regs r, uint16_t ret)
{
    jsr_push(ret);
    r = after_ended(r, MapOverlayBank7());
    jsr_pop();
    return r;
}

/* A word of one of the overlay's jump tables, as JMP [table,X] reads it: the low byte, then the high. */
static uint16_t table_word(uint16_t table, uint8_t index)
{
    uint8_t low = rd8((uint16_t)(table + index));
    uint8_t high = rd8((uint16_t)(table + index + 1));

    return (uint16_t)(low | (high << 8));
}

/* The card's cd_fade, reached by a tail JMP: A the nibble, N and Z from it; X, Y, C and V kept. */
static Regs fade_tail(Regs r, uint8_t type)
{
    return loaded(r, vec_cd_fade(type));
}

/* A JSR to one of this file's routines: the frame the original pushes, the routine, the pop. */
#define CALL(ret, expr) do { jsr_push(ret); r = (expr); jsr_pop(); } while (0)

/*
 * @routine 152D:000 StageEntryA152D
 * @run 6B:8000
 * @unreachable 6B:8000-6B:8002 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8000: goes to StageSubModeStep152D, the stage's frame by sub-mode.
 * HOW: One jump, no frame of its own.
 * WAITS: as the sub-mode's handler.
 * Entry, exit: as StageSubModeStep152D.
 */
StageRegs StageEntryA152D(StageRegs r)
{
    return StageSubModeStep152D(r);
}

void abi_StageEntryA152D(void)
{
    regs_exit_shim(StageEntryA152D(regs_entry_shim()));
}

/*
 * @routine 152D:003 StageEntryB152D
 * @run 6B:8003
 * @unreachable 6B:8003-6B:8005 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8003: goes to StageEndStep152D, the frame of the stage's end by zGameSubMode.
 * HOW: One jump, no frame of its own.
 * WAITS: as the handler.
 * Entry, exit: as StageEndStep152D.
 */
StageRegs StageEntryB152D(StageRegs r)
{
    return StageEndStep152D(r);
}

void abi_StageEntryB152D(void)
{
    regs_exit_shim(StageEntryB152D(regs_entry_shim()));
}

/*
 * @routine 152D:006 StageEntryC152D
 * @run 6B:8006
 * @unreachable 6B:8006-6B:8008 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8006: goes to ProbeTile152D, the tile probe at $96C6.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as ProbeTile152D.
 */
StageRegs StageEntryC152D(StageRegs r)
{
    return ProbeTile152D(r);
}

void abi_StageEntryC152D(void)
{
    regs_exit_shim(StageEntryC152D(regs_entry_shim()));
}

/*
 * @routine 152D:009 StageEntryD152D
 * @run 6B:8009
 * @unreachable 6B:8009-6B:800B no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8009: goes to ClearBitRowsFour152D at $8E72.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as ClearBitRowsFour152D.
 */
StageRegs StageEntryD152D(StageRegs r)
{
    return ClearBitRowsFour152D(r);
}

void abi_StageEntryD152D(void)
{
    regs_exit_shim(StageEntryD152D(regs_entry_shim()));
}

/*
 * @routine 152D:00C StageSubModeStep152D
 * @run 6B:800C
 * @stop 8013
 * @unreachable 6B:800C-6B:8012 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $800C runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The stage's frame: runs the handler of the sub-mode zGameSubMode holds, from the table of seven at
 * $8013 (0 the start, 1 play, 2 the player's death, 3 the stage cleared, 4 the fade out, 5 and 6).
 * HOW: The sub-mode doubled indexes the table, whose word is jumped through.
 * WAITS: as the handler.
 * Entry: nothing. Exit: as the handler leaves them; it is entered with A and X twice the sub-mode, N and Z from it,
 * C the sub-mode's bit 7.
 */
StageRegs StageSubModeStep152D(StageRegs r)
{
    uint8_t mode = RD8(ZP->game_sub_mode);
    uint8_t twice = (uint8_t)(mode << 1);
    uint16_t handler;

    r.c = (mode & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(SUB_MODE_TABLE, twice);
    switch (handler) {
    case 0x8034:
        return StageSubModeStart152D(r);
    case 0x80D7:
        return StageSubModePlay152D(r);
    case 0x81C9:
        return StageSubModeDeath152D(r);
    case 0x82B4:
        return StageSubModeCleared152D(r);
    case 0x852C:
        return StageSubModeFade152D(r);
    case 0x85C4:
        return StageSubModeEnd152D(r);
    case 0x90F8:
        return StageSubModeBoss152D(r);
    default:
        return orig_tail_shim(r, handler);
    }
}

void abi_StageSubModeStep152D(void)
{
    regs_exit_shim(StageSubModeStep152D(regs_entry_shim()));
}

/*
 * @routine 152D:034 StageSubModeStart152D
 * @run 6B:8034
 * @unreachable 6B:8034-6B:806E no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8034 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-mode 0, the stage's start: sets the stage up (the kernel's piece at $4006, the overlay's tables, the
 * overlay at $A000's start, the screen), clears the pause and the fade, and goes on to sub-mode 1; zField86 takes
 * zField83. When zField8E's bit 0 is clear the stage goes straight to sub-mode 5 instead, with the count at $3310
 * one less; otherwise, unless zStage is negative, the kernel's piece at $407B runs last.
 * HOW: Calls in turn $4006, StageClearTables152D, StageClearRows152D, StageResetObjects152D, StageResetPlayer152D,
 * MapOverlayBanks5And6, $A003, StageSetUpScroll152D, StageLoadLists152D and StageObjectsStart152D.
 * WAITS: where the pieces it calls wait.
 * Entry: nothing. Exit, sub-mode 5: A 5, N and Z from the count's decrement. Else: A zStage, N and Z from it, or as
 * $407B leaves them; X, Y, C and V as the last piece left them.
 */
StageRegs StageSubModeStart152D(StageRegs r)
{
    uint8_t value;

    r = orig_shim(r, 0x8037, KERNEL_4006);
    CALL(0x803A, StageClearTables152D(r));
    CALL(0x803D, StageClearRows152D(r));
    CALL(0x8040, StageResetObjects152D(r));
    CALL(0x8043, StageResetPlayer152D(r));
    r = map_banks_5_6(r, 0x8046);
    r = orig_shim(r, 0x8049, 0xA003);
    CALL(0x804C, StageSetUpScroll152D(r));
    CALL(0x804F, StageLoadLists152D(r));
    CALL(0x8052, StageObjectsStart152D(r));
    WR8(ZP->paused, 0);
    wr8(FADE_ACTIVE, 0);
    r.nz = INC8(ZP->game_sub_mode);
    r = loaded(r, RD8(ZP->field83));
    WR8(ZP->field86, r.a);
    if ((RD8(ZP->field8e) & 0x01) == 0) {
        r.a = 5;
        WR8(ZP->game_sub_mode, 5);
        r.nz = DEC8(OBJ->f3300[16]);
        return r;
    }
    value = RD8(ZP->stage);
    r = loaded(r, value);
    if ((value & 0x80) != 0) {
        return r;
    }
    return orig_shim(r, 0x8066, KERNEL_407B);
}

void abi_StageSubModeStart152D(void)
{
    regs_exit_shim(StageSubModeStart152D(regs_entry_shim()));
}

/*
 * @routine 152D:06F StageFadeTimer152D
 * @run 6B:806F
 * @unreachable 6B:806F-6B:8099 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $806F runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Counts down the stage's fade at $9B by two a frame. On the step that reaches one it marks the fade as
 * running ($3C 1) and, unless zField8E's bits 3 or 4 are set, starts the card's CD fade of type $0C; on the steps after
 * it clears the mark and, unless bit 4 is set, sends the sound at $9D (bit 3 set) or the CD fade of type 0.
 * HOW: Two decrements of $9B with a test after each; bit tests of zField8E; the sound hook at $401B or cd_fade, reached
 * by a tail jump.
 * WAITS: no.
 * Entry: nothing. Exit: A the count first read (0: N and Z from it), or the value the last test loaded; N and Z from
 * the last decrement or test; after the sound or the fade, as they leave them.
 */
StageRegs StageFadeTimer152D(StageRegs r)
{
    uint8_t value;

    r = loaded(r, rd8(FADE_COUNT));
    if (r.a == 0) {
        return r;
    }
    value = (uint8_t)(rd8(FADE_COUNT) - 1);
    wr8(FADE_COUNT, value);
    r.nz = value;
    if (value == 0) {
        r = loaded(r, 1);
        wr8(FADE_ACTIVE, 1);
        r = loaded(r, (uint8_t)(RD8(ZP->field8e) & 0x18));
        if (r.a != 0) {
            return r;
        }
        return fade_tail(r, 0x0C);
    }
    value = (uint8_t)(rd8(FADE_COUNT) - 1);
    wr8(FADE_COUNT, value);
    r.nz = value;
    wr8(FADE_ACTIVE, 0);
    if ((RD8(ZP->field8e) & 0x10) != 0) {
        return r;
    }
    if ((RD8(ZP->field8e) & 0x08) != 0) {
        r = loaded(r, rd8(FADE_SOUND));
        return orig_tail_shim(r, SOUND_HOOK);
    }
    r.a = 0;
    return fade_tail(r, 0);
}

void abi_StageFadeTimer152D(void)
{
    regs_exit_shim(StageFadeTimer152D(regs_entry_shim()));
}

/*
 * @routine 152D:09A StagePauseToggle152D
 * @run 6B:809A
 * @unreachable 6B:809A-6B:80B0 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $809A runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Pauses or unpauses the stage when the first pad's button 8 (Run) is pressed, unless the fade is
 * running or zPaused's bit 1 holds the pause.
 * HOW: $3C XOR 1 must not be zero; BiosJoyTrigger's bit 3; zPaused's bit 1 clear; zPaused XOR 3.
 * WAITS: no.
 * Entry: nothing. Exit: A the last value loaded or computed, N and Z from it (the bit test leaves A the pad's bit).
 * X, Y, C and V kept.
 */
StageRegs StagePauseToggle152D(StageRegs r)
{
    uint8_t value;

    r = loaded(r, (uint8_t)(rd8(FADE_ACTIVE) ^ 0x01));
    if (r.a == 0) {
        return r;
    }
    r = loaded(r, (uint8_t)(rd8(BiosJoyTrigger) & 0x08));
    if (r.a == 0) {
        return r;
    }
    if ((RD8(ZP->paused) & 0x02) != 0) {
        return r;
    }
    value = (uint8_t)(RD8(ZP->paused) ^ 0x03);
    r = loaded(r, value);
    WR8(ZP->paused, value);
    return r;
}

void abi_StagePauseToggle152D(void)
{
    regs_exit_shim(StagePauseToggle152D(regs_entry_shim()));
}

/*
 * @routine 152D:0B1 StageModeEventsStep152D
 * @run 6B:80B1
 * @unreachable 6B:80B1-6B:80D6 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $80B1 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Counts the wave and steps zModeEvents: $11 added, wrapped below $50 by its low nibble; a low pair that
 * reaches 3 carries into the next pair, and a second pair that reaches 3 clears the low nibble.
 * HOW: An add and compares on the byte, written back after each change.
 * WAITS: no.
 * Entry: nothing. Exit: A the last value masked, N and Z from the last compare or mask; C from the last compare; V
 * from the add. X and Y kept.
 */
StageRegs StageModeEventsStep152D(StageRegs r)
{
    Sum sum;
    uint8_t value;

    r.nz = INC8(ZP->wave_counter);
    sum = add_bytes(RD8(ZP->mode_events), 0x11, false);
    r = loaded(r, sum.value);
    r.v = sum.overflow;
    r = compared(r, 0x50);
    if (r.c) {
        r = loaded(r, (uint8_t)(r.a & 0x0F));
    }
    WR8(ZP->mode_events, r.a);
    r = loaded(r, (uint8_t)(r.a & 0x03));
    r = compared(r, 0x03);
    if (r.nz != 0) {
        return r;
    }
    r.nz = INC8(ZP->mode_events);
    r = loaded(r, (uint8_t)(RD8(ZP->mode_events) & 0x0C));
    r = compared(r, 0x0C);
    if (r.nz != 0) {
        return r;
    }
    value = (uint8_t)(RD8(ZP->mode_events) & 0xF0);
    r = loaded(r, value);
    WR8(ZP->mode_events, value);
    return r;
}

void abi_StageModeEventsStep152D(void)
{
    regs_exit_shim(StageModeEventsStep152D(regs_entry_shim()));
}

/*
 * @routine 152D:0D7 StageSubModePlay152D
 * @run 6B:80D7
 * @stop 8112
 * @unreachable 6B:80D7-6B:8111 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $80D7 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-mode 1, the stage in play: the fade and the pause; paused, the paused frame (StagePausedFrame152D)
 * runs instead. Else the events step and the overlay at $A000's frame; unless a sub-step is asked for, the player, the
 * camera, the objects and the screen; then the sub-step zSubStep held (cleared) runs from the table at $8112.
 * HOW: Calls StageFadeTimer152D and StagePauseToggle152D; jumps to StagePausedFrame152D; calls StageModeEventsStep152D,
 * MapOverlayBanks5And6, $A000, then $A006, MapOverlayBank7, $A08D, MapOverlayBanks5And6, $A009, StageScroll152D,
 * StageHitTests152D and StageRows152D; the sub-step doubled indexes the table.
 * WAITS: where the pieces it calls wait.
 * Entry: nothing. Exit: as the sub-step's handler leaves them; it is entered with A and X twice the sub-step, N and Z
 * from it, C its bit 7. Paused: as StagePausedFrame152D.
 */
StageRegs StageSubModePlay152D(StageRegs r)
{
    uint8_t step, twice;
    uint16_t handler;

    CALL(0x80DA, StageFadeTimer152D(r));
    CALL(0x80DD, StagePauseToggle152D(r));
    r = loaded(r, RD8(ZP->paused));
    if (r.a != 0) {
        return StagePausedFrame152D(r);
    }
    CALL(0x80E7, StageModeEventsStep152D(r));
    r = map_banks_5_6(r, 0x80EA);
    r = orig_shim(r, 0x80ED, 0xA000);
    r = loaded(r, RD8(ZP->sub_step));
    if (r.a == 0) {
        r = orig_shim(r, 0x80F4, 0xA006);
        r = map_bank_7(r, 0x80F7);
        r = orig_shim(r, 0x80FA, 0xA08D);
        r = map_banks_5_6(r, 0x80FD);
        r = orig_shim(r, 0x8100, 0xA009);
        CALL(0x8103, StageScroll152D(r));
        CALL(0x8106, StageHitTests152D(r));
        CALL(0x8109, StageRows152D(r));
    }
    step = RD8(ZP->sub_step);
    WR8(ZP->sub_step, 0);
    twice = (uint8_t)(step << 1);
    r.c = (step & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(SUB_STEP_TABLE, twice);
    switch (handler) {
    case 0x8120:
        return StageSubStepNone152D(r);
    case 0x8121:
        return StageSubStepNoneB152D(r);
    case 0x8122:
        return StageSubStepDeath152D(r);
    case 0x8148:
        return StageSubStepCleared152D(r);
    case 0x817B:
        return StageSubStepFadeOut152D(r);
    case 0x818A:
        return StageSubStepFadeOutB152D(r);
    case 0x819B:
        return StageSubStepBoss152D(r);
    default:
        return orig_tail_shim(r, handler);
    }
}

void abi_StageSubModePlay152D(void)
{
    regs_exit_shim(StageSubModePlay152D(regs_entry_shim()));
}

/*
 * @routine 152D:120 StageSubStepNone152D
 * @run 6B:8120
 * @unreachable 6B:8120-6B:8120 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8120 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-step 0: nothing.
 * HOW: RTS.
 * WAITS: no.
 * Entry: nothing. Exit: everything kept.
 */
StageRegs StageSubStepNone152D(StageRegs r)
{
    return r;
}

void abi_StageSubStepNone152D(void)
{
}

/*
 * @routine 152D:121 StageSubStepNoneB152D
 * @run 6B:8121
 * @unreachable 6B:8121-6B:8121 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8121 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-step 1: nothing.
 * HOW: RTS.
 * WAITS: no.
 * Entry: nothing. Exit: everything kept.
 */
StageRegs StageSubStepNoneB152D(StageRegs r)
{
    return r;
}

void abi_StageSubStepNoneB152D(void)
{
}

/* The fade restarted when PlayerByte3BD8 is set: cleared, $9B 2, and the fade's countdown run on. */
static Regs restart_fade(Regs r)
{
    r = loaded(r, rd8(PlayerByte3BD8));
    if (r.a == 0) {
        return r;
    }
    wr8(PlayerByte3BD8, 0);
    r = loaded(r, 2);
    wr8(FADE_COUNT, 2);
    return StageFadeTimer152D(r);
}

/*
 * @routine 152D:122 StageSubStepDeath152D
 * @run 6B:8122
 * @unreachable 6B:8122-6B:8147 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8122 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-step 2, the player died: sub-mode 2 with zField80, zField89 and the fall's step cleared, the
 * overlay at $A000's piece at $A067, the forced presses cleared; when PlayerByte3BD8 is set it is cleared and the fade
 * starts again from 2.
 * HOW: Stores; MapOverlayBank7 and $A067; a jump into StageFadeTimer152D.
 * WAITS: no.
 * Entry: nothing. Exit: A PlayerByte3BD8 (0), N and Z from it, the rest as $A067 left them; else as StageFadeTimer152D.
 */
StageRegs StageSubStepDeath152D(StageRegs r)
{
    r = loaded(r, 2);
    WR8(ZP->game_sub_mode, 2);
    WR8(ZP->field80, 0);
    WR8(ZP->field89, 0);
    r = map_bank_7(r, 0x812D);
    r = orig_shim(r, 0x8130, 0xA067);
    wr8(PadForcedPress, 0);
    wr8(PadForcedPressB, 0);
    wr8(FALL_STEP, 0);
    return restart_fade(r);
}

void abi_StageSubStepDeath152D(void)
{
    regs_exit_shim(StageSubStepDeath152D(regs_entry_shim()));
}

/*
 * @routine 152D:148 StageSubStepCleared152D
 * @run 6B:8148
 * @unreachable 6B:8148-6B:817A no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8148 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-step 3, the stage cleared: sub-mode 3 with its step 0, its table at $84 picked by zField8C (none:
 * 2; 3 or 4: 3; else 1, with $2743 set to 1); the overlay at $A000's piece at $A067, the forced presses and
 * PlayerByte3BD8 cleared.
 * HOW: Compares of zField8C; stores; MapOverlayBank7 and $A067.
 * WAITS: no.
 * Entry: nothing. Exit: as $A067 left them.
 */
StageRegs StageSubStepCleared152D(StageRegs r)
{
    uint8_t table;

    r = loaded(r, RD8(ZP->field8c));
    if (r.a == 0) {
        table = 2;
    } else {
        r.x = 1;
        wr8(STAGE_FLAG_2743, 1);
        r = compared(r, 0x03);
        if (r.nz != 0) {
            r = compared(r, 0x04);
        }
        table = r.nz == 0 ? 3 : 1;
    }
    r = loaded(r, table);
    wr8(SUB3_STEP_TABLE, table);
    r = loaded(r, 3);
    WR8(ZP->game_sub_mode, 3);
    wr8(SUB3_STEP, 0);
    r = map_bank_7(r, 0x816E);
    r = orig_shim(r, 0x8171, 0xA067);
    wr8(PadForcedPress, 0);
    wr8(PadForcedPressB, 0);
    wr8(PlayerByte3BD8, 0);
    return r;
}

void abi_StageSubStepCleared152D(void)
{
    regs_exit_shim(StageSubStepCleared152D(regs_entry_shim()));
}

/*
 * @routine 152D:17B StageSubStepFadeOut152D
 * @run 6B:817B
 * @unreachable 6B:817B-6B:8189 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $817B runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-step 4: sub-mode 4 with zField80 and zField89 cleared, and the overlay at $A000's piece at $A067.
 * HOW: Stores; MapOverlayBank7 and $A067.
 * WAITS: no.
 * Entry: nothing. Exit: as $A067 left them.
 */
StageRegs StageSubStepFadeOut152D(StageRegs r)
{
    r = loaded(r, 4);
    WR8(ZP->game_sub_mode, 4);
    WR8(ZP->field80, 0);
    WR8(ZP->field89, 0);
    r = map_bank_7(r, 0x8186);
    return orig_shim(r, 0x8189, 0xA067);
}

void abi_StageSubStepFadeOut152D(void)
{
    regs_exit_shim(StageSubStepFadeOut152D(regs_entry_shim()));
}

/*
 * @routine 152D:18A StageSubStepFadeOutB152D
 * @run 6B:818A
 * @unreachable 6B:818A-6B:819A no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $818A runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-step 5: sub-mode 4 entered at its step 5 (zField80 5, zField89 cleared), and the overlay at $A000's
 * piece at $A067.
 * HOW: Stores; MapOverlayBank7 and $A067.
 * WAITS: no.
 * Entry: nothing. Exit: as $A067 left them.
 */
StageRegs StageSubStepFadeOutB152D(StageRegs r)
{
    r = loaded(r, 4);
    WR8(ZP->game_sub_mode, 4);
    r = loaded(r, 5);
    WR8(ZP->field80, 5);
    WR8(ZP->field89, 0);
    r = map_bank_7(r, 0x8197);
    return orig_shim(r, 0x819A, 0xA067);
}

void abi_StageSubStepFadeOutB152D(void)
{
    regs_exit_shim(StageSubStepFadeOutB152D(regs_entry_shim()));
}

/*
 * @routine 152D:19B StageSubStepBoss152D
 * @run 6B:819B
 * @unreachable 6B:819B-6B:81C8 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $819B runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-step 6: sub-mode 6 with zField80, zField89 and the fall's step cleared, the overlay at $A000's
 * piece at $A067, the forced presses cleared, the player's step at $3358 0 and its count at $3310 $40; when
 * PlayerByte3BD8 is set it is cleared and the fade starts again from 2.
 * HOW: Stores; MapOverlayBank7 and $A067; a jump into StageFadeTimer152D.
 * WAITS: no.
 * Entry: nothing. Exit: A PlayerByte3BD8 (0), N and Z from it, the rest as $A067 left them; else as StageFadeTimer152D.
 */
StageRegs StageSubStepBoss152D(StageRegs r)
{
    r = loaded(r, 6);
    WR8(ZP->game_sub_mode, 6);
    WR8(ZP->field80, 0);
    WR8(ZP->field89, 0);
    r = map_bank_7(r, 0x81A6);
    r = orig_shim(r, 0x81A9, 0xA067);
    wr8(PadForcedPress, 0);
    wr8(PadForcedPressB, 0);
    wr8(FALL_STEP, 0);
    WR8(OBJ->f3358[0], 0);
    r = loaded(r, 0x40);
    WR8(OBJ->f3300[16], 0x40);
    return restart_fade(r);
}

void abi_StageSubStepBoss152D(void)
{
    regs_exit_shim(StageSubStepBoss152D(regs_entry_shim()));
}

/*
 * @routine 152D:1C9 StageSubModeDeath152D
 * @run 6B:81C9
 * @unreachable 6B:81C9-6B:81E0 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $81C9 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-mode 2, the player's death: sound command $FF, then the death's phase; once the phase has put the
 * stage back in sub-mode 1, the overlay at $A000's piece at $A015 and the objects' start.
 * HOW: The sound hook at $401B; StageDeathPhase152D; a compare of zGameSubMode; MapOverlayBanks5And6, $A015 and a jump
 * to StageObjectsStart152D.
 * WAITS: where the pieces it calls wait.
 * Entry: nothing. Exit: A zGameSubMode, N, Z and C from its compare with 1; else as StageObjectsStart152D.
 */
StageRegs StageSubModeDeath152D(StageRegs r)
{
    r = loaded(r, 0xFF);
    r = orig_shim(r, 0x81CE, SOUND_HOOK);
    CALL(0x81D1, StageDeathPhase152D(r));
    r = loaded(r, RD8(ZP->game_sub_mode));
    r = compared(r, 0x01);
    if (r.nz != 0) {
        return r;
    }
    r = map_banks_5_6(r, 0x81DB);
    r = orig_shim(r, 0x81DE, 0xA015);
    return StageObjectsStart152D(r);
}

void abi_StageSubModeDeath152D(void)
{
    regs_exit_shim(StageSubModeDeath152D(regs_entry_shim()));
}

/*
 * @routine 152D:1E1 StageDeathPhase152D
 * @run 6B:81E1
 * @stop 81E8
 * @unreachable 6B:81E1-6B:81E7 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $81E1 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The death's phase by zField87, from the table of four at $81E8.
 * HOW: zField87 doubled indexes the table, whose word is jumped through.
 * WAITS: as the phase.
 * Entry: nothing. Exit: as the phase leaves them; it is entered with A and X twice zField87, N and Z from it, C its
 * bit 7.
 */
StageRegs StageDeathPhase152D(StageRegs r)
{
    uint8_t phase = RD8(ZP->field87);
    uint8_t twice = (uint8_t)(phase << 1);
    uint16_t handler;

    r.c = (phase & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(DEATH_PHASE_TABLE, twice);
    switch (handler) {
    case 0x81F0:
        return StageDeathRespawn152D(r);
    case 0x820B:
        return StageDeathRespawnB152D(r);
    case 0x8224:
        return StageDeathFallLeft152D(r);
    case 0x8271:
        return StageDeathFallRight152D(r);
    default:
        return orig_tail_shim(r, handler);
    }
}

void abi_StageDeathPhase152D(void)
{
    regs_exit_shim(StageDeathPhase152D(regs_entry_shim()));
}

/* The player put back: StageResetPlayer152D, sub-mode 1, the count at $3310 $18, the step at $3358 from the table at
 * $8209 by the facing bit of the byte at $2818. */
static Regs respawn(Regs r, uint16_t ret)
{
    uint8_t facing;

    CALL(ret, StageResetPlayer152D(r));
    r = loaded(r, 1);
    WR8(ZP->game_sub_mode, 1);
    r = loaded(r, 0x18);
    WR8(OBJ->f3300[16], 0x18);
    facing = (uint8_t)(RD8(OBJ->f2800[24]) & 0x01);
    r.x = facing;
    r = loaded(r, rd8((uint16_t)(FACING_SPEEDS + facing)));
    WR8(OBJ->f3358[0], r.a);
    return r;
}

/*
 * @routine 152D:1F0 StageDeathRespawn152D
 * @run 6B:81F0
 * @unreachable 6B:81F0-6B:8208 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $81F0 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Phase 0 of the death: the player put back and the stage in play again (sub-mode 1), the player's count
 * at $3310 $18 and its step at $3358 from the two at $8209 by the facing bit of the byte at $2818.
 * HOW: StageResetPlayer152D; stores; one table read.
 * WAITS: no.
 * Entry: nothing. Exit: A the step, X the facing bit, N and Z from A; Y, C and V as StageResetPlayer152D left them.
 */
StageRegs StageDeathRespawn152D(StageRegs r)
{
    return respawn(r, 0x81F3);
}

void abi_StageDeathRespawn152D(void)
{
    regs_exit_shim(StageDeathRespawn152D(regs_entry_shim()));
}

/*
 * @routine 152D:20B StageDeathRespawnB152D
 * @run 6B:820B
 * @unreachable 6B:820B-6B:8223 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $820B runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Phase 1 of the death: as phase 0 (the same bytes, the same table at $8209).
 * HOW: StageResetPlayer152D; stores; one table read.
 * WAITS: no.
 * Entry: nothing. Exit: as StageDeathRespawn152D.
 */
StageRegs StageDeathRespawnB152D(StageRegs r)
{
    return respawn(r, 0x820E);
}

void abi_StageDeathRespawnB152D(void)
{
    regs_exit_shim(StageDeathRespawnB152D(regs_entry_shim()));
}

/* The fall's last step: the player put back, sub-mode 1, the count at $3310 $20 and the step at $3358 as given. */
static Regs fall_ends(Regs r, uint16_t ret, uint8_t step)
{
    CALL(ret, StageResetPlayer152D(r));
    r = loaded(r, 1);
    WR8(ZP->game_sub_mode, 1);
    r = loaded(r, 0x20);
    WR8(OBJ->f3300[16], 0x20);
    r = loaded(r, step);
    WR8(OBJ->f3358[0], step);
    return r;
}

/* The fall's middle step: the overlay at $A000's pieces at $A01B and $A012; while the player is shown and short of
 * the edge the step stays, else $A01E runs with X 0. */
static Regs fall_moves(Regs r, uint16_t at, bool right)
{
    r = map_banks_5_6(r, at);
    r = orig_shim(r, (uint16_t)(at + 3), 0xA01B);
    r = orig_shim(r, (uint16_t)(at + 6), 0xA012);
    r = loaded(r, RD8(OBJ->flash_flags[24]));
    if (r.a != 0) {
        r = loaded(r, RD8(OBJ->x_hi[8]));
        r = compared(r, right ? 0xF0 : 0x10);
        if (right ? !r.c : r.c) {
            r.nz = (uint8_t)(rd8(FALL_STEP) + 1);
            wr8(FALL_STEP, r.nz);
            return r;
        }
    }
    r.x = 0;
    return orig_tail_shim(r, 0xA01E);
}

/*
 * @routine 152D:224 StageDeathFallLeft152D
 * @run 6B:8224
 * @unreachable 6B:8224-6B:8235 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8224 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Phase 2 of the death, the fall to the left, by the fall's step at $88: step 0 sets the player's Y
 * ($2D28 $40, $2CE0 1) and runs on into StageDeathFallStart152D; step 1 moves the player while it is shown and left
 * of X $10, else ends through $A01E; step 2 puts the player back (StageResetPlayer152D, sub-mode 1, the count at $3310
 * $20, the step at $3358 $20).
 * HOW: Two decrements of the step's copy in A; the pieces of the overlay at $A000 at $A01B, $A012 and $A01E.
 * WAITS: no.
 * Entry: nothing. Exit, step 0: as StageDeathFallStart152D. Step 1, the step counted: A the player's X, N and Z from
 * the increment, C from the compare; else as $A01E leaves them. Step 2: A $20, N and Z from it, the rest as
 * StageResetPlayer152D left them.
 */
StageRegs StageDeathFallLeft152D(StageRegs r)
{
    r = loaded(r, rd8(FALL_STEP));
    r = loaded(r, (uint8_t)(r.a - 1));
    if (r.a == 0) {
        return fall_moves(r, 0x8246, false);
    }
    r = loaded(r, (uint8_t)(r.a - 1));
    if (r.a == 0) {
        return fall_ends(r, 0x8262, 0x20);
    }
    r = loaded(r, 0x40);
    WR8(OBJ->y_frac[0], 0x40);
    r = loaded(r, 0x01);
    WR8(OBJ->y_lo[16], 0x01);
    return StageDeathFallStart152D(r);
}

void abi_StageDeathFallLeft152D(void)
{
    regs_exit_shim(StageDeathFallLeft152D(regs_entry_shim()));
}

/*
 * @routine 152D:236 StageDeathFallStart152D
 * @run 6B:8236
 * @unreachable 6B:8236-6B:8270 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8236 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The fall's first step: the overlay at $A000's pieces at $A018 (with A 0) and $A012, and the fall's
 * step at $88 counted.
 * HOW: MapOverlayBanks5And6, $A018, $A012; an increment of $88.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the step's increment; A, X, Y, C and V as $A012 left them.
 */
StageRegs StageDeathFallStart152D(StageRegs r)
{
    r = map_banks_5_6(r, 0x8239);
    r = loaded(r, 0);
    r = orig_shim(r, 0x823E, 0xA018);
    r = orig_shim(r, 0x8241, 0xA012);
    r.nz = (uint8_t)(rd8(FALL_STEP) + 1);
    wr8(FALL_STEP, r.nz);
    return r;
}

void abi_StageDeathFallStart152D(void)
{
    regs_exit_shim(StageDeathFallStart152D(regs_entry_shim()));
}

/*
 * @routine 152D:271 StageDeathFallRight152D
 * @run 6B:8271
 * @unreachable 6B:8271-6B:82B3 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8271 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Phase 3 of the death, the fall to the right, by the fall's step at $88: step 0 sets the player's Y
 * ($2D28 $C0, $2CE0 $FE) and calls StageDeathFallStart152D, then moves the player as step 1 does; step 1 moves the
 * player while it is shown and left of X $F0, else ends through $A01E; step 2 puts the player back (StageResetPlayer152D,
 * sub-mode 1, the count at $3310 $20, the step at $3358 $80).
 * HOW: Two decrements of the step's copy in A; StageDeathFallStart152D; the pieces of the overlay at $A000 at $A01B,
 * $A012 and $A01E.
 * WAITS: no.
 * Entry: nothing. Exit, steps 0 and 1, the step counted: A the player's X, N and Z from the increment, C from the
 * compare; else as $A01E leaves them. Step 2: A $80, N and Z from it, the rest as StageResetPlayer152D left them.
 */
StageRegs StageDeathFallRight152D(StageRegs r)
{
    r = loaded(r, rd8(FALL_STEP));
    r = loaded(r, (uint8_t)(r.a - 1));
    if (r.a == 0) {
        return fall_moves(r, 0x8289, true);
    }
    r = loaded(r, (uint8_t)(r.a - 1));
    if (r.a == 0) {
        return fall_ends(r, 0x82A5, 0x80);
    }
    r = loaded(r, 0xC0);
    WR8(OBJ->y_frac[0], 0xC0);
    r = loaded(r, 0xFE);
    WR8(OBJ->y_lo[16], 0xFE);
    CALL(0x8286, StageDeathFallStart152D(r));
    return fall_moves(r, 0x8289, true);
}

void abi_StageDeathFallRight152D(void)
{
    regs_exit_shim(StageDeathFallRight152D(regs_entry_shim()));
}

/*
 * @routine 152D:2B4 StageSubModeCleared152D
 * @run 6B:82B4
 * @stop 82BE
 * @unreachable 6B:82B4-6B:82BD no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82B4 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-mode 3, the stage cleared: the events step, then the step of the table $84 picked (the next stage's
 * start, the walk out, the boss's walk out, the walk out after the boss) from the four at $82BE.
 * HOW: StageModeEventsStep152D; $84 doubled indexes the table, whose word is jumped through.
 * WAITS: as the step.
 * Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice $84, N and Z from it, C its bit 7.
 */
StageRegs StageSubModeCleared152D(StageRegs r)
{
    uint8_t step, twice;
    uint16_t handler;

    CALL(0x82B7, StageModeEventsStep152D(r));
    step = rd8(SUB3_STEP_TABLE);
    twice = (uint8_t)(step << 1);
    r.c = (step & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(0x82BE, twice);
    switch (handler) {
    case 0x82C6:
        return StageClearedNextStage152D(r);
    case 0x82FC:
        return StageClearedWalkOut152D(r);
    case 0x8436:
        return StageClearedBossWalk152D(r);
    case 0x84D8:
        return StageClearedLastWalk152D(r);
    default:
        return orig_tail_shim(r, handler);
    }
}

void abi_StageSubModeCleared152D(void)
{
    regs_exit_shim(StageSubModeCleared152D(regs_entry_shim()));
}

/*
 * @routine 152D:2C6 StageClearedNextStage152D
 * @run 6B:82C6
 * @unreachable 6B:82C6-6B:82EF no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82C6 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The cleared stage's first table: the next stage set up as the start does (the kernel's piece at $4009,
 * the overlay's tables, the scroll, zField8A 2, the overlay at $A000's start, the objects) and put in play, sub-mode 1,
 * unpaused, zField86 zField83.
 * HOW: Calls in turn $4009, StageClearTables152D, StageClearRows152D, StageResetObjects152D, StageResetPlayer152D,
 * StageSetUpScroll152D, MapOverlayBanks5And6, $A003 and StageObjectsStart152D.
 * WAITS: where the pieces it calls wait.
 * Entry: nothing. Exit: A zField83, N and Z from it; X, Y, C and V as StageObjectsStart152D left them.
 */
StageRegs StageClearedNextStage152D(StageRegs r)
{
    r = orig_shim(r, 0x82C9, KERNEL_4009);
    CALL(0x82CC, StageClearTables152D(r));
    CALL(0x82CF, StageClearRows152D(r));
    CALL(0x82D2, StageResetObjects152D(r));
    CALL(0x82D5, StageResetPlayer152D(r));
    CALL(0x82D8, StageSetUpScroll152D(r));
    r = loaded(r, 2);
    WR8(ZP->field8a, 2);
    r = map_banks_5_6(r, 0x82DF);
    r = orig_shim(r, 0x82E2, 0xA003);
    CALL(0x82E5, StageObjectsStart152D(r));
    WR8(ZP->paused, 0);
    r = loaded(r, 1);
    WR8(ZP->game_sub_mode, 1);
    r = loaded(r, RD8(ZP->field83));
    WR8(ZP->field86, r.a);
    return r;
}

void abi_StageClearedNextStage152D(void)
{
    regs_exit_shim(StageClearedNextStage152D(regs_entry_shim()));
}

/*
 * @routine 152D:2FC StageClearedWalkOut152D
 * @run 6B:82FC
 * @stop 8303
 * @unreachable 6B:82FC-6B:8302 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $82FC runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The cleared stage's second table: its step at $85 from the five at $8303 (the wait for the objects,
 * the walk, the scroll's wait, the walk back, the walk on to the next stage).
 * HOW: $85 doubled indexes the table, whose word is jumped through.
 * WAITS: as the step.
 * Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice $85, N and Z from it, C its bit 7.
 */
StageRegs StageClearedWalkOut152D(StageRegs r)
{
    uint8_t step = rd8(SUB3_STEP);
    uint8_t twice = (uint8_t)(step << 1);
    uint16_t handler;

    r.c = (step & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(0x8303, twice);
    switch (handler) {
    case 0x830D:
        return StageClearedWaitObjects152D(r);
    case 0x832E:
        return StageClearedWalk152D(r);
    case 0x8371:
        return StageClearedWaitScroll152D(r);
    case 0x837E:
        return StageClearedWalkBack152D(r);
    case 0x83C1:
        return StageClearedWalkOn152D(r);
    default:
        return orig_tail_shim(r, handler);
    }
}

void abi_StageClearedWalkOut152D(void)
{
    regs_exit_shim(StageClearedWalkOut152D(regs_entry_shim()));
}

/*
 * @routine 152D:30D StageClearedWaitObjects152D
 * @run 6B:830D
 * @unreachable 6B:830D-6B:832D no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $830D runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step of the cleared stage: the overlay at $A000's pieces at $A009 and $A08D and the scroll, then,
 * once no object slot from $40 down to 1 holds a state from 1 to $7F in the column at $27D0, $2743 cleared and the
 * step counted.
 * HOW: MapOverlayBanks5And6, $A009, MapOverlayBank7, $A08D, StageScroll152D; a loop down X; a store and an increment.
 * WAITS: no.
 * Entry: nothing. Exit, an object still busy: A its state, X its slot, N and Z from A. Else: X 0, N and Z from the
 * step's increment; A, Y, C and V as StageScroll152D left them.
 */
StageRegs StageClearedWaitObjects152D(StageRegs r)
{
    uint8_t slot;

    r = map_banks_5_6(r, 0x8310);
    r = orig_shim(r, 0x8313, 0xA009);
    r = map_bank_7(r, 0x8316);
    r = orig_shim(r, 0x8319, 0xA08D);
    CALL(0x831C, StageScroll152D(r));
    for (slot = 0x40; slot != 0; slot--) {
        r = loaded(r, RD8(OBJ->state[40 + slot]));
        r.x = slot;
        if ((r.a & 0x80) == 0 && r.a != 0) {
            return r;
        }
    }
    r.x = 0;
    wr8(STAGE_FLAG_2743, 0);
    r.nz = (uint8_t)(rd8(SUB3_STEP) + 1);
    wr8(SUB3_STEP, r.nz);
    return r;
}

void abi_StageClearedWaitObjects152D(void)
{
    regs_exit_shim(StageClearedWaitObjects152D(regs_entry_shim()));
}

/* zField8C less one, its low bit: which of the two values of the overlay's small tables at $82F0 the walk takes. */
static uint8_t walk_side(void)
{
    return (uint8_t)((uint8_t)(RD8(ZP->field8c) - 1) & 0x01);
}

/* The walk's push: the scroll step from the table at $82F0, its fraction cleared, and the player's X moved by the
 * table at $82F2. */
static Regs walk_push(Regs r)
{
    uint8_t side = walk_side();

    r.y = side;
    r = loaded(r, rd8((uint16_t)(0x82F0 + side)));
    WR8(SCROLLE->step_y_lo[6], r.a);
    WR8(SCROLLE->f3e66[6], 0);
    return r;
}

static Regs walk_move_x(Regs r, uint8_t side)
{
    Sum sum;
    uint8_t x = RD8(OBJ->x_hi[8]);
    uint8_t step = rd8((uint16_t)(0x82F2 + side));

    sum = add_bytes(x, step, false);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(OBJ->x_hi[8], sum.value);
    return r;
}

/* The walk's frame: MapOverlayBank7, $A08D, the scroll and $A321. */
static Regs walk_frame(Regs r, uint16_t at)
{
    r = map_bank_7(r, at);
    r = orig_shim(r, (uint16_t)(at + 3), 0xA08D);
    CALL((uint16_t)(at + 6), StageScroll152D(r));
    return orig_shim(r, (uint16_t)(at + 9), 0xA321);
}

/*
 * @routine 152D:32E StageClearedWalk152D
 * @run 6B:832E
 * @unreachable 6B:832E-6B:8370 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $832E runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step of the cleared stage: the player walks the scroll ($3E74 the step from $82F0 by the side,
 * $3E6C and $3E00 cleared, its X moved by $82F2); when its X reaches the side's end ($82F4) the overlay at $A000's
 * piece at $A067 runs, $3E74 is cleared and the step counted.
 * HOW: The side from zField8C; table reads, an add; MapOverlayBank7, $A08D, StageScroll152D, $A321; a compare.
 * WAITS: no.
 * Entry: nothing. Exit, short of the end: A the X, N, Z and C from the compare, Y the side. At the end: N and Z from
 * the step's increment, the rest as $A067 left them.
 */
StageRegs StageClearedWalk152D(StageRegs r)
{
    uint8_t side;

    r = walk_push(r);
    WR8(SCROLL->x_frac[0], 0);
    r = walk_move_x(r, r.y);
    r = walk_frame(r, 0x834D);
    side = walk_side();
    r.y = side;
    r = loaded(r, RD8(OBJ->x_hi[8]));
    r = compared(r, rd8((uint16_t)(0x82F4 + side)));
    if (r.nz != 0) {
        return r;
    }
    r = map_bank_7(r, 0x8368);
    r = orig_shim(r, 0x836B, 0xA067);
    WR8(SCROLLE->step_y_lo[6], 0);
    r.nz = (uint8_t)(rd8(SUB3_STEP) + 1);
    wr8(SUB3_STEP, r.nz);
    return r;
}

void abi_StageClearedWalk152D(void)
{
    regs_exit_shim(StageClearedWalk152D(regs_entry_shim()));
}

/*
 * @routine 152D:371 StageClearedWaitScroll152D
 * @run 6B:8371
 * @unreachable 6B:8371-6B:837D no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8371 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step of the cleared stage: the scroll and the overlay at $A000's piece at $A321; once zField8C is
 * negative the step is counted.
 * HOW: StageScroll152D, $A321; a test of zField8C; an increment.
 * WAITS: no.
 * Entry: nothing. Exit: A zField8C, N and Z from it, or from the step's increment; X, Y, C and V as $A321 left them.
 */
StageRegs StageClearedWaitScroll152D(StageRegs r)
{
    CALL(0x8374, StageScroll152D(r));
    r = orig_shim(r, 0x8377, 0xA321);
    r = loaded(r, RD8(ZP->field8c));
    if ((r.a & 0x80) == 0) {
        return r;
    }
    r.nz = (uint8_t)(rd8(SUB3_STEP) + 1);
    wr8(SUB3_STEP, r.nz);
    return r;
}

void abi_StageClearedWaitScroll152D(void)
{
    regs_exit_shim(StageClearedWaitScroll152D(regs_entry_shim()));
}

/*
 * @routine 152D:37E StageClearedWalkBack152D
 * @run 6B:837E
 * @unreachable 6B:837E-6B:83C0 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $837E runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step of the cleared stage: the player walks back (its count at $3310 $E0, its step at $3358 from
 * $82F8 by the side, the overlay at $A000's frame at $A000, its X moved back by $82F2); when the X reaches the side's
 * mark ($82F6) zField8C loses bit 7, the player stops, zField8A is 2, the overlay's piece at $A003 runs and the step is
 * counted.
 * HOW: The side from zField8C; table reads; MapOverlayBanks5And6, $A000; a subtract and a compare; stores, $A003.
 * WAITS: no.
 * Entry: nothing. Exit, short of the mark: A the X, N, Z and C from the compare, V from the subtract, Y the side. At
 * the mark: N and Z from the step's increment, the rest as $A003 left them.
 */
StageRegs StageClearedWalkBack152D(StageRegs r)
{
    Sum sum;
    uint8_t side, x, step;

    r = loaded(r, 0xE0);
    WR8(OBJ->f3300[16], 0xE0);
    side = walk_side();
    r.y = side;
    r = loaded(r, rd8((uint16_t)(0x82F8 + side)));
    WR8(OBJ->f3358[0], r.a);
    r = map_banks_5_6(r, 0x8392);
    r = orig_shim(r, 0x8395, 0xA000);
    side = walk_side();
    r.y = side;
    x = RD8(OBJ->x_hi[8]);
    step = rd8((uint16_t)(0x82F2 + side));
    sum = sub_bytes(x, step, true);
    r = loaded(r, sum.value);
    r.v = sum.overflow;
    WR8(OBJ->x_hi[8], sum.value);
    r = compared(r, rd8((uint16_t)(0x82F6 + side)));
    if (r.nz != 0) {
        return r;
    }
    r = loaded(r, (uint8_t)(RD8(ZP->field8c) & 0x7F));
    WR8(ZP->field8c, r.a);
    WR8(OBJ->f3300[16], 0);
    WR8(OBJ->f3358[0], 0);
    r = loaded(r, 2);
    WR8(ZP->field8a, 2);
    r = orig_shim(r, 0x83BE, 0xA003);
    r.nz = (uint8_t)(rd8(SUB3_STEP) + 1);
    wr8(SUB3_STEP, r.nz);
    return r;
}

void abi_StageClearedWalkBack152D(void)
{
    regs_exit_shim(StageClearedWalkBack152D(regs_entry_shim()));
}

/*
 * @routine 152D:3C1 StageClearedWalkOn152D
 * @run 6B:83C1
 * @unreachable 6B:83C1-6B:8435 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $83C1 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The cleared stage's last step: the player walks on with the scroll until its X low byte at $3E08 is 0;
 * then the next stage: below zField8C 5 the next of the route's stages from the table at $9A5F (zField82 counted, into
 * zField83 and zField86), else zField83 counted; zField8E's bit 1 set; the stage's tables, rows and objects set up
 * again (the first and the last only below 5), the scroll's bytes at $3E00 and $3E54 cleared from KernelZeros, the
 * screen loaded; zField8C and the pause cleared and the stage in play, sub-mode 1.
 * HOW: The walk's push and frame as StageClearedWalk152D; StageClearTables152D, StageClearRows152D,
 * StageResetObjects152D, two TAI, StageLoadScreen152D, StageObjectsStart152D.
 * WAITS: where the pieces it calls wait.
 * Entry: nothing. Exit, walking: A $3E08, N and Z from it, the rest as $A321 left them. Else: A 1, N and Z from it;
 * X, Y, C and V as the last piece left them, C from the compare when StageObjectsStart152D did not run.
 */
StageRegs StageClearedWalkOn152D(StageRegs r)
{
    r = walk_push(r);
    r = walk_move_x(r, r.y);
    r = walk_frame(r, 0x83DD);
    r = loaded(r, RD8(SCROLL->x_lo[0]));
    if (r.a != 0) {
        return r;
    }
    r = loaded(r, RD8(ZP->field8c));
    r = compared(r, 0x05);
    if (!r.c) {
        r.nz = INC8(ZP->field82);
        r = loaded(r, RD8(ZP->field82));
        r.y = r.a;
        r = loaded(r, rd8((uint16_t)(0x9A5F + r.y)));
        WR8(ZP->field83, r.a);
        WR8(ZP->field86, r.a);
    } else {
        r.nz = INC8(ZP->field83);
    }
    RMW8(ZP->field8e, old, old | 0x02);
    r = loaded(r, RD8(ZP->field8c));
    r = compared(r, 0x05);
    if (!r.c) {
        CALL(0x840D, StageClearTables152D(r));
    }
    CALL(0x8410, StageClearRows152D(r));
    CALL(0x8413, StageResetObjects152D(r));
    tai(KernelZeros, ADDR_OF8(SCROLL->x_frac[0]), 0x0030);
    tai(KernelZeros, ADDR_OF8(BANDS->bgy_lo[0]), 0x0078);
    CALL(0x8424, StageLoadScreen152D(r));
    r = loaded(r, RD8(ZP->field8c));
    r = compared(r, 0x05);
    if (!r.c) {
        CALL(0x842D, StageObjectsStart152D(r));
    }
    WR8(ZP->field8c, 0);
    WR8(ZP->paused, 0);
    r = loaded(r, 1);
    WR8(ZP->game_sub_mode, 1);
    return r;
}

void abi_StageClearedWalkOn152D(void)
{
    regs_exit_shim(StageClearedWalkOn152D(regs_entry_shim()));
}

/*
 * @routine 152D:436 StageClearedBossWalk152D
 * @run 6B:8436
 * @stop 8440
 * @unreachable 6B:8436-6B:843F no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8436 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The cleared stage's third table, after a boss: the fade's countdown, then its step at $85 from the three
 * at $8440 (the scroll to the boss's door, the wait for the door, the walk out).
 * HOW: StageFadeTimer152D; $85 doubled indexes the table, whose word is jumped through.
 * WAITS: as the step.
 * Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice $85, N and Z from it, C its bit 7.
 */
StageRegs StageClearedBossWalk152D(StageRegs r)
{
    uint8_t step, twice;
    uint16_t handler;

    CALL(0x8439, StageFadeTimer152D(r));
    step = rd8(SUB3_STEP);
    twice = (uint8_t)(step << 1);
    r.c = (step & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(0x8440, twice);
    switch (handler) {
    case 0x8446:
        return StageBossScrollToDoor152D(r);
    case 0x847C:
        return StageBossWaitDoor152D(r);
    case 0x8499:
        return StageBossWalkOut152D(r);
    default:
        return orig_tail_shim(r, handler);
    }
}

void abi_StageClearedBossWalk152D(void)
{
    regs_exit_shim(StageClearedBossWalk152D(regs_entry_shim()));
}

/*
 * @routine 152D:446 StageBossScrollToDoor152D
 * @run 6B:8446
 * @unreachable 6B:8446-6B:847B no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8446 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step after the boss: the overlay at $A000's piece at $A009, the scroll stepped by 1 with the player's
 * X one less, the scroll frame; once ScrollXHi is 4 the overlay's piece at $A067 runs, the scroll's steps are cleared,
 * sound command $3D is sent and the step counted.
 * HOW: MapOverlayBanks5And6, $A009; stores and a decrement; MapOverlayBank7, $A08D, StageScroll152D; a compare;
 * MapOverlayBank7, $A067, the sound hook at $401B.
 * WAITS: no.
 * Entry: nothing. Exit, short of 4: A ScrollXHi, N, Z and C from the compare, the rest as StageScroll152D left them.
 * Else: N and Z from the step's increment, the rest as the sound hook left them.
 */
StageRegs StageBossScrollToDoor152D(StageRegs r)
{
    r = map_banks_5_6(r, 0x8449);
    r = orig_shim(r, 0x844C, 0xA009);
    r = loaded(r, 1);
    WR8(SCROLLE->step_y_lo[6], 1);
    WR8(SCROLLE->f3e66[6], 0);
    r.nz = DEC8(OBJ->x_hi[8]);
    r = map_bank_7(r, 0x845A);
    r = orig_shim(r, 0x845D, 0xA08D);
    CALL(0x8460, StageScroll152D(r));
    r = loaded(r, RD8(SCROLL->x_hi[0]));
    r = compared(r, 0x04);
    if (r.nz != 0) {
        return r;
    }
    r = map_bank_7(r, 0x846B);
    r = orig_shim(r, 0x846E, 0xA067);
    WR8(SCROLLE->step_y_lo[6], 0);
    WR8(SCROLLE->step_y_lo[0], 0);
    r = loaded(r, 0x3D);
    r = orig_shim(r, 0x8479, SOUND_HOOK);
    r.nz = (uint8_t)(rd8(SUB3_STEP) + 1);
    wr8(SUB3_STEP, r.nz);
    return r;
}

void abi_StageBossScrollToDoor152D(void)
{
    regs_exit_shim(StageBossScrollToDoor152D(regs_entry_shim()));
}

/*
 * @routine 152D:47C StageBossWaitDoor152D
 * @run 6B:847C
 * @unreachable 6B:847C-6B:8498 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $847C runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step after the boss: the overlay at $A000's piece at $A009 and the scroll; once the door object's
 * byte at $283A is 4 and its animation step at $340A is 0, the player's step at $3358 is $20 and the step is counted.
 * HOW: MapOverlayBanks5And6, $A009, StageScroll152D; two tests; a store and an increment.
 * WAITS: no.
 * Entry: nothing. Exit: A the last byte tested, N and Z from its test (C from the compare with 4), or $20 with N and Z
 * from the step's increment; X, Y and V as StageScroll152D left them.
 */
StageRegs StageBossWaitDoor152D(StageRegs r)
{
    r = map_banks_5_6(r, 0x847F);
    r = orig_shim(r, 0x8482, 0xA009);
    CALL(0x8485, StageScroll152D(r));
    r = loaded(r, RD8(OBJ->f2800[58]));
    r = compared(r, 0x04);
    if (r.nz != 0) {
        return r;
    }
    r = loaded(r, RD8(OBJ->anim_step[2]));
    if (r.a != 0) {
        return r;
    }
    r = loaded(r, 0x20);
    WR8(OBJ->f3358[0], 0x20);
    r.nz = (uint8_t)(rd8(SUB3_STEP) + 1);
    wr8(SUB3_STEP, r.nz);
    return r;
}

void abi_StageBossWaitDoor152D(void)
{
    regs_exit_shim(StageBossWaitDoor152D(regs_entry_shim()));
}

/*
 * @routine 152D:499 StageBossWalkOut152D
 * @run 6B:8499
 * @unreachable 6B:8499-6B:84D7 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8499 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The last step after the boss: the player walks out (its count at $3310 $B8) with the overlay at $A000's
 * frame; when its X reaches $F0 the player stops, $2743 is cleared, the next of the route's stages is taken from the
 * table at $9A5F (zField82 counted, into zField83), $84 and the byte at $2A58 are cleared and the objects start.
 * HOW: MapOverlayBanks5And6, $A000, $A006, $A009, MapOverlayBank7, $A08D, StageScroll152D; a compare; stores; a table
 * read; StageObjectsStart152D.
 * WAITS: no.
 * Entry: nothing. Exit, short of $F0: A the X, N, Z and C from the compare, the rest as StageScroll152D left them.
 * Else: as StageObjectsStart152D.
 */
StageRegs StageBossWalkOut152D(StageRegs r)
{
    r = loaded(r, 0xB8);
    WR8(OBJ->f3300[16], 0xB8);
    r = map_banks_5_6(r, 0x84A1);
    r = orig_shim(r, 0x84A4, 0xA000);
    r = orig_shim(r, 0x84A7, 0xA006);
    r = orig_shim(r, 0x84AA, 0xA009);
    r = map_bank_7(r, 0x84AD);
    r = orig_shim(r, 0x84B0, 0xA08D);
    CALL(0x84B3, StageScroll152D(r));
    r = loaded(r, RD8(OBJ->x_hi[8]));
    r = compared(r, 0xF0);
    if (r.nz != 0) {
        return r;
    }
    WR8(OBJ->f3300[16], 0);
    wr8(STAGE_FLAG_2743, 0);
    WR8(OBJ->f3358[0], 0);
    r.nz = INC8(ZP->field82);
    r = loaded(r, RD8(ZP->field82));
    r.y = r.a;
    r = loaded(r, rd8((uint16_t)(0x9A5F + r.y)));
    WR8(ZP->field83, r.a);
    r.a = 0;
    wr8(SUB3_STEP_TABLE, 0);
    WR8(OBJ->sprite_set[72], 0);
    CALL(0x84D7, StageObjectsStart152D(r));
    return r;
}

void abi_StageBossWalkOut152D(void)
{
    regs_exit_shim(StageBossWalkOut152D(regs_entry_shim()));
}

/*
 * @routine 152D:4D8 StageClearedLastWalk152D
 * @run 6B:84D8
 * @stop 84DF
 * @unreachable 6B:84D8-6B:84DE no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $84D8 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The cleared stage's fourth table: its step at $85 from the four at $84DF (the wait for the objects and
 * the scroll's wait of the walk out, a step that only counts, and the walk off the screen).
 * HOW: $85 doubled indexes the table, whose word is jumped through.
 * WAITS: as the step.
 * Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice $85, N and Z from it, C its bit 7.
 */
StageRegs StageClearedLastWalk152D(StageRegs r)
{
    uint8_t step = rd8(SUB3_STEP);
    uint8_t twice = (uint8_t)(step << 1);
    uint16_t handler;

    r.c = (step & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(0x84DF, twice);
    switch (handler) {
    case 0x830D:
        return StageClearedWaitObjects152D(r);
    case 0x84E7:
        return StageClearedCount152D(r);
    case 0x8371:
        return StageClearedWaitScroll152D(r);
    case 0x84EA:
        return StageClearedWalkOff152D(r);
    default:
        return orig_tail_shim(r, handler);
    }
}

void abi_StageClearedLastWalk152D(void)
{
    regs_exit_shim(StageClearedLastWalk152D(regs_entry_shim()));
}

/*
 * @routine 152D:4E7 StageClearedCount152D
 * @run 6B:84E7
 * @unreachable 6B:84E7-6B:84E9 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $84E7 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step of the last walk that only counts the step.
 * HOW: An increment of $85.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the increment; A, X, Y, C and V kept.
 */
StageRegs StageClearedCount152D(StageRegs r)
{
    r.nz = (uint8_t)(rd8(SUB3_STEP) + 1);
    wr8(SUB3_STEP, r.nz);
    return r;
}

void abi_StageClearedCount152D(void)
{
    regs_exit_shim(StageClearedCount152D(regs_entry_shim()));
}

/*
 * @routine 152D:4EA StageClearedWalkOff152D
 * @run 6B:84EA
 * @unreachable 6B:84EA-6B:852B no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $84EA runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The last walk off the screen: the player's step and count at $3358 and $3310 from $82F8 by the side,
 * the overlay at $A000's frame, the player's X moved by $82F0 (its page at $2B30 too when the X wraps to 0); when the
 * X reaches the side's mark at $82FA, zModeDone is counted.
 * HOW: The side from zField8C; table reads; MapOverlayBanks5And6, $A000; adds and a compare.
 * WAITS: no.
 * Entry: nothing. Exit: A the X, N, Z and C from the compare, Y the side, V from the last add; N and Z from
 * zModeDone's increment when it is counted. X as $A000 left it.
 */
StageRegs StageClearedWalkOff152D(StageRegs r)
{
    Sum sum;
    uint8_t side, step, value;

    side = walk_side();
    r.y = side;
    r = loaded(r, rd8((uint16_t)(0x82F8 + side)));
    WR8(OBJ->f3358[0], r.a);
    WR8(OBJ->f3300[16], r.a);
    r = map_banks_5_6(r, 0x84FC);
    r = orig_shim(r, 0x84FF, 0xA000);
    side = walk_side();
    r.y = side;
    step = rd8((uint16_t)(0x82F0 + side));
    value = RD8(OBJ->x_hi[8]);
    sum = add_bytes(step, value, false);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(OBJ->x_hi[8], sum.value);
    if (sum.value == 0) {
        step = rd8((uint16_t)(0x82F0 + side));
        value = RD8(OBJ->flash_flags[24]);
        sum = add_bytes(step, value, false);
        r = loaded(r, sum.value);
        r.c = sum.carry;
        r.v = sum.overflow;
        WR8(OBJ->flash_flags[24], sum.value);
    }
    side = walk_side();
    r.y = side;
    r = loaded(r, RD8(OBJ->x_hi[8]));
    r = compared(r, rd8((uint16_t)(0x82FA + side)));
    if (r.nz != 0) {
        return r;
    }
    r.nz = INC8(ZP->mode_done);
    return r;
}

void abi_StageClearedWalkOff152D(void)
{
    regs_exit_shim(StageClearedWalkOff152D(regs_entry_shim()));
}

/*
 * @routine 152D:52C StageSubModeFade152D
 * @run 6B:852C
 * @stop 8542
 * @unreachable 6B:852C-6B:8541 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $852C runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-mode 4, the fade out after the death or the stage: the events step and the rows, then the step at
 * zField80 from the seven at $8542 (the fade's start, its wait, the CD's stop and the stock taken, the waits, the
 * restart).
 * HOW: StageModeEventsStep152D, StageRows152D; a compare that branches to the next instruction either way;
 * MapOverlayBanks5And6; zField80 doubled indexes the table, whose word is jumped through.
 * WAITS: as the step.
 * Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice zField80, N and Z from it, C its
 * bit 7.
 */
StageRegs StageSubModeFade152D(StageRegs r)
{
    uint8_t step, twice;
    uint16_t handler;

    CALL(0x852F, StageModeEventsStep152D(r));
    CALL(0x8532, StageRows152D(r));
    r = loaded(r, RD8(ZP->field80));
    r = compared(r, 0x03);
    r = map_banks_5_6(r, 0x853B);
    step = RD8(ZP->field80);
    twice = (uint8_t)(step << 1);
    r.c = (step & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(0x8542, twice);
    switch (handler) {
    case 0x8550:
        return StageFadeStart152D(r);
    case 0x8559:
        return StageFadeWait152D(r);
    case 0x8571:
        return StageFadeStopCd152D(r);
    case 0x85A1:
        return StageFadeCountdown152D(r);
    case 0x85A9:
        return StageFadeRestart152D(r);
    case 0x8580:
        return StageFadeTakeStock152D(r);
    case 0x8597:
        return StageFadeWaitLast152D(r);
    default:
        return orig_tail_shim(r, handler);
    }
}

void abi_StageSubModeFade152D(void)
{
    regs_exit_shim(StageSubModeFade152D(regs_entry_shim()));
}

/*
 * @routine 152D:550 StageFadeStart152D
 * @run 6B:8550
 * @unreachable 6B:8550-6B:8558 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8550 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The fade's first step: the overlay at $A000's pieces at $A00C and $A012, and the step counted.
 * HOW: Two calls and an increment of zField80.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the increment; A, X, Y, C and V as $A012 left them.
 */
StageRegs StageFadeStart152D(StageRegs r)
{
    r = orig_shim(r, 0x8553, 0xA00C);
    r = orig_shim(r, 0x8556, 0xA012);
    r.nz = INC8(ZP->field80);
    return r;
}

void abi_StageFadeStart152D(void)
{
    regs_exit_shim(StageFadeStart152D(regs_entry_shim()));
}

/*
 * @routine 152D:559 StageFadeWait152D
 * @run 6B:8559
 * @unreachable 6B:8559-6B:8570 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8559 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The fade's second step: the overlay at $A000's piece at $A00F, and $A012 again when it changed the byte
 * at $2A58; once zField89 is set the step is counted.
 * HOW: The byte kept on the stack across $A00F and compared after it; a test of zField89; an increment.
 * WAITS: no.
 * Entry: nothing. Exit: A zField89, N and Z from it or from zField80's increment; C from the compare, or as $A012 left
 * it; X, Y and V as the last call left them.
 */
StageRegs StageFadeWait152D(StageRegs r)
{
    uint8_t before;

    r = loaded(r, RD8(OBJ->sprite_set[72]));
    push8(r.a);
    r = orig_shim(r, 0x8560, 0xA00F);
    before = pull8();
    r = loaded(r, before);
    r = compared(r, RD8(OBJ->sprite_set[72]));
    if (r.nz != 0) {
        r = orig_shim(r, 0x8569, 0xA012);
    }
    r = loaded(r, RD8(ZP->field89));
    if (r.a == 0) {
        return r;
    }
    r.nz = INC8(ZP->field80);
    return r;
}

void abi_StageFadeWait152D(void)
{
    regs_exit_shim(StageFadeWait152D(regs_entry_shim()));
}

/* What a card routine leaves, laid over the registers. N and Z on their own (a TST or BIT) are written as the value
 * that gives them; the card's cd_pause sets them from a value. */
static Regs after_card(Regs r, CardExit e)
{
    if (e.set_a) {
        r.a = e.a;
    }
    if (e.set_x) {
        r.x = e.x;
    }
    if (e.set_y) {
        r.y = e.y;
    }
    if (e.set_nz) {
        r.nz = e.nz;
    }
    if (e.set_z || e.set_n) {
        r.nz = e.z ? 0 : (e.n ? 0x80 : 0x01);
    }
    if (e.set_c) {
        r.c = e.c;
    }
    if (e.set_v) {
        r.v = e.v;
    }
    return r;
}

/* JSR $E018, the card's cd_pause, with X as it stands, until it answers 0: A into X each time. */
static Regs pause_cd(Regs r, uint16_t ret)
{
    do {
        jsr_push(ret);
        r = after_card(r, vec_cd_pause(r.x));
        jsr_pop();
        r.x = r.a;
        r.nz = r.a;
    } while (r.a != 0);
    return r;
}

/* A life taken from the stock in A (a decimal subtract of 1, SED SEC SBC CLD) and written to zStock. */
static Regs take_stock(Regs r)
{
    uint8_t stock = r.a;
    uint8_t low = (uint8_t)((stock & 0x0F) - 1);
    uint8_t high = (uint8_t)((stock >> 4) - ((low >> 4) & 1));
    uint8_t value = (uint8_t)((high << 4) | (low & 0x0F));

    if ((low & 0x10) != 0) {
        value = (uint8_t)(value - 0x06);
    }
    if ((high & 0x10) != 0) {
        value = (uint8_t)(value - 0x60);
    }
    r.c = (high & 0x10) == 0;
    r = loaded(r, value);
    WR8(ZP->stock, value);
    return r;
}

static Regs fade_wait_set(Regs r)
{
    r = loaded(r, 0x60);
    WR8(ZP->wait_timer3, 0x60);
    r.nz = INC8(ZP->field80);
    return r;
}

/*
 * @routine 152D:571 StageFadeStopCd152D
 * @run 6B:8571
 * @unreachable 6B:8571-6B:857F no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8571 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The fade's third step: the CD paused (cd_pause until it answers 0); with no stock left zModeDone is
 * $80 (the game over), else a life is taken from zStock, the wait at zWaitTimer3 set to $60 and the step counted.
 * HOW: cd_pause in a loop; a test of zStock; a decimal subtract (SED, SBC, CLD) and stores, at $8589 in
 * StageFadeTakeStock152D's bytes.
 * WAITS: in cd_pause, as the card's routine does.
 * Entry: D clear, as the game keeps it. Exit, no stock: A $80, N and Z from it. Else: A $60, N and Z from zField80's
 * increment, C from the subtract; X 0; Y and V as cd_pause left them.
 */
StageRegs StageFadeStopCd152D(StageRegs r)
{
    r = pause_cd(r, 0x8574);
    r = loaded(r, RD8(ZP->stock));
    if (r.a == 0) {
        r = loaded(r, 0x80);
        WR8(ZP->mode_done, 0x80);
        return r;
    }
    r = take_stock(r);
    return fade_wait_set(r);
}

void abi_StageFadeStopCd152D(void)
{
    regs_exit_shim(StageFadeStopCd152D(regs_entry_shim()));
}

/*
 * @routine 152D:580 StageFadeTakeStock152D
 * @run 6B:8580
 * @unreachable 6B:8580-6B:8596 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8580 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A fade step: with stock left the fade goes back to its third step (zField80 2); with none the wait at
 * zWaitTimer3 is set to $60 and the step counted. Its bytes from $8589 also take a life for StageFadeStopCd152D.
 * HOW: A test of zStock; stores and an increment.
 * WAITS: no.
 * Entry: nothing. Exit, stock left: A 2, N and Z from it. Else: A $60, N and Z from zField80's increment. X, Y, C and V
 * kept.
 */
StageRegs StageFadeTakeStock152D(StageRegs r)
{
    r = loaded(r, RD8(ZP->stock));
    if (r.a == 0) {
        return fade_wait_set(r);
    }
    r = loaded(r, 2);
    WR8(ZP->field80, 2);
    return r;
}

void abi_StageFadeTakeStock152D(void)
{
    regs_exit_shim(StageFadeTakeStock152D(regs_entry_shim()));
}

/*
 * @routine 152D:597 StageFadeWaitLast152D
 * @run 6B:8597
 * @unreachable 6B:8597-6B:85A0 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8597 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A fade step: counts zWaitTimer3 down, and when it reaches 0 the fade goes back to its third step
 * (zField80 2).
 * HOW: A decrement; a store.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the decrement, A kept; at 0, A 2 with N and Z from it. X, Y, C and V kept.
 */
StageRegs StageFadeWaitLast152D(StageRegs r)
{
    r.nz = DEC8(ZP->wait_timer3);
    if (r.nz != 0) {
        return r;
    }
    r = loaded(r, 2);
    WR8(ZP->field80, 2);
    return r;
}

void abi_StageFadeWaitLast152D(void)
{
    regs_exit_shim(StageFadeWaitLast152D(regs_entry_shim()));
}

/*
 * @routine 152D:5A1 StageFadeCountdown152D
 * @run 6B:85A1
 * @unreachable 6B:85A1-6B:85A8 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $85A1 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A fade step: counts zWaitTimer3 down, and when it reaches 0 counts the step.
 * HOW: Two decrements' worth: zWaitTimer3, then zField80 up.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the last increment or decrement; A, X, Y, C and V kept.
 */
StageRegs StageFadeCountdown152D(StageRegs r)
{
    r.nz = DEC8(ZP->wait_timer3);
    if (r.nz != 0) {
        return r;
    }
    r.nz = INC8(ZP->field80);
    return r;
}

void abi_StageFadeCountdown152D(void)
{
    regs_exit_shim(StageFadeCountdown152D(regs_entry_shim()));
}

/*
 * @routine 152D:5A9 StageFadeRestart152D
 * @run 6B:85A9
 * @unreachable 6B:85A9-6B:85C3 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $85A9 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The fade's last step: the stage starts again from sub-mode 0, zField83 back to zField86, zField8A and
 * zField8F cleared, zField8E's bit 2 cleared; when zStage is negative the CD is paused first and zField8E's bit 3 set.
 * HOW: Stores and bit changes; cd_pause in a loop.
 * WAITS: in cd_pause, as the card's routine does.
 * Entry: nothing. Exit: A 0, N and Z from it; X 0 after cd_pause; Y, C and V as cd_pause left them.
 */
StageRegs StageFadeRestart152D(StageRegs r)
{
    r = loaded(r, RD8(ZP->field86));
    WR8(ZP->field83, r.a);
    WR8(ZP->field8a, 0);
    WR8(ZP->field8f, 0);
    RMW8(ZP->field8e, old, old & (uint8_t)~0x04);
    r = loaded(r, RD8(ZP->stage));
    if ((r.a & 0x80) != 0) {
        r = pause_cd(r, 0x85BA);
        RMW8(ZP->field8e, old, old | 0x08);
    }
    r = loaded(r, 0);
    WR8(ZP->game_sub_mode, 0);
    return r;
}

void abi_StageFadeRestart152D(void)
{
    regs_exit_shim(StageFadeRestart152D(regs_entry_shim()));
}

/*
 * @routine 152D:5C4 StageSubModeOpening152D
 * @run 6B:85C4
 * @unreachable 6B:85C4-6B:85E9 no read the game makes loads disc sector 152D, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $85C4 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-mode 5, the stage's opening: the events step, the rows, the overlay at $A000's frame, the scroll,
 * the opening's piece at $8BD5; once zField8E's bit 0 is set PlayerByte3BD8 and the player's count at $3310 are
 * cleared and the stage is in play, sub-mode 1.
 * HOW: StageModeEventsStep152D, StageRows152D, MapOverlayBanks5And6, $A000, StageScroll152D, MapOverlayBank7, $A08D,
 * StageOpeningStep152D; a bit test; stores.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, N and Z from it when the stage goes into play; else as StageOpeningStep152D left them.
 */
StageRegs StageSubModeOpening152D(StageRegs r)
{
    CALL(0x85C7, StageModeEventsStep152D(r));
    CALL(0x85CA, StageRows152D(r));
    r = map_banks_5_6(r, 0x85CD);
    r = orig_shim(r, 0x85D0, 0xA000);
    CALL(0x85D3, StageScroll152D(r));
    r = map_bank_7(r, 0x85D6);
    r = orig_shim(r, 0x85D9, 0xA08D);
    CALL(0x85DC, StageOpeningStep152D(r));
    if ((RD8(ZP->field8e) & 0x01) == 0) {
        return r;
    }
    wr8(PlayerByte3BD8, 0);
    r = loaded(r, 1);
    WR8(ZP->game_sub_mode, 1);
    WR8(OBJ->f3300[16], 0);
    return r;
}

void abi_StageSubModeOpening152D(void)
{
    regs_exit_shim(StageSubModeOpening152D(regs_entry_shim()));
}
