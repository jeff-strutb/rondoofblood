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
    ROW_MAP = 0x3A00, TALLY_CELLS = 0x3980, BOSS_BAR = 0x2745, ROW_BUFFER = 0x397E, SCREEN_3EB4 = 0x3EB4, SCREEN_3EC4 = 0x3EC4
};

typedef StageRegs Regs;

static Regs loaded(Regs r, uint8_t value);

/* A JSR to one of this file's routines: the frame the original pushes, the routine, the pop. */
#define CALL(ret, expr) do { jsr_push(ret); r = (expr); jsr_pop(); } while (0)

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

/* TAI, which pushes and pulls Y, A and X: the registers as they stand put in the machine for it. */
static void tai_shim(Regs r, uint16_t src, uint16_t dst, uint16_t len)
{
    stage_regs_load_shim(r);
    tai(src, dst, len);
}

/* The scroll's bytes at $3E00 (48) and $3E54 (120) cleared from KernelZeros. */
static void clear_scroll_bytes(Regs r)
{
    tai_shim(r, KernelZeros, ADDR_OF8(SCROLL->x_frac[0]), 0x0030);
    tai_shim(r, KernelZeros, ADDR_OF8(BANDS->bgy_lo[0]), 0x0078);
}

/* JSR $9415, SetVramAddresses152D. */
static Regs set_vram_addresses(Regs r, uint16_t ret)
{
    CALL(ret, SetVramAddresses152D(r));
    return r;
}

/* JSR to the census duplicates of map_draw.c's AndVramData and OrVramData, at $942A and $9439, through their C. */

static Regs and_vram(Regs r, uint16_t ret)
{
    uint8_t high;

    jsr_push(ret);
    high = AndVramData(r.x, r.y);
    jsr_pop();
    return loaded(r, high);
}

static Regs or_vram(Regs r, uint16_t ret)
{
    uint8_t high;

    jsr_push(ret);
    high = OrVramData(r.x, r.y);
    jsr_pop();
    return loaded(r, high);
}

/* PHP: the flags as they stand pushed, B set and T clear. */
static void php_shim(Regs r)
{
    stage_regs_load_shim(r);
    cpu.p &= (uint8_t)~P_T;
    push8((uint8_t)(cpu.p | P_B));
}

/* PLP: the flags pulled; C, V, N and Z taken from them. */
static Regs plp_shim(Regs r)
{
    uint8_t p = pull8();

    cpu.p = p;
    r.c = (p & P_C) != 0;
    r.v = (p & P_V) != 0;
    r.nz = (p & P_Z) != 0 ? 0 : ((p & P_N) != 0 ? 0x80 : 0x01);
    return r;
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

/*
 * @routine 152D:000 StageEntryA152D
 * @run 6B:8000
 * @unreachable 6B:8000-6B:8002 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8000 runs whatever overlay another scene
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
 * @unreachable 6B:8003-6B:8005 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8003 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8003: goes to ContinueStep152D, the continue screen's frame by zGameSubMode.
 * HOW: One jump, no frame of its own.
 * WAITS: as the handler.
 * Entry, exit: as ContinueStep152D.
 */
StageRegs StageEntryB152D(StageRegs r)
{
    return ContinueStep152D(r);
}

void abi_StageEntryB152D(void)
{
    regs_exit_shim(StageEntryB152D(regs_entry_shim()));
}

/*
 * @routine 152D:006 StageEntryC152D
 * @run 6B:8006
 * @unreachable 6B:8006-6B:8008 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8006 runs whatever overlay another scene
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
 * @unreachable 6B:8009-6B:800B no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8009 runs whatever overlay another scene
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
 * @unreachable 6B:800C-6B:8012 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $800C runs whatever overlay another scene
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
        return StageSubModeTally152D(r);
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
 * @unreachable 6B:8034-6B:806E no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8034 runs whatever overlay another scene
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
 * @unreachable 6B:806F-6B:8099 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $806F runs whatever overlay another scene
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
 * @unreachable 6B:809A-6B:80B0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $809A runs whatever overlay another scene
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
 * @unreachable 6B:80B1-6B:80D6 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $80B1 runs whatever overlay another scene
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
 * @unreachable 6B:80D7-6B:8111 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $80D7 runs whatever overlay another scene
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
 * @unreachable 6B:8120-6B:8120 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8120 runs whatever overlay another scene
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
 * @unreachable 6B:8121-6B:8121 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8121 runs whatever overlay another scene
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
 * @unreachable 6B:8122-6B:8147 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8122 runs whatever overlay another scene
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
 * @unreachable 6B:8148-6B:817A no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8148 runs whatever overlay another scene
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
 * @unreachable 6B:817B-6B:8189 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $817B runs whatever overlay another scene
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
 * @unreachable 6B:818A-6B:819A no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $818A runs whatever overlay another scene
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
 * @unreachable 6B:819B-6B:81C8 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $819B runs whatever overlay another scene
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
 * @unreachable 6B:81C9-6B:81E0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $81C9 runs whatever overlay another scene
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
 * @unreachable 6B:81E1-6B:81E7 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $81E1 runs whatever overlay another scene
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
 * @unreachable 6B:81F0-6B:8208 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $81F0 runs whatever overlay another scene
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
 * @unreachable 6B:820B-6B:8223 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $820B runs whatever overlay another scene
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
 * @unreachable 6B:8224-6B:8235 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8224 runs whatever overlay another scene
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
        return StageDeathFallBackLeft152D(r);
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
 * @unreachable 6B:8236-6B:8270 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8236 runs whatever overlay another scene
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
 * @routine 152D:25F StageDeathFallBackLeft152D
 * @run 6B:825F
 * @unreachable 6B:825F-6B:8270 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $825F runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The left fall's last step, reached from StageDeathFallLeft152D: the player put back
 * (StageResetPlayer152D), sub-mode 1, the count at $3310 $20 and the step at $3358 $20.
 * HOW: StageResetPlayer152D; stores.
 * WAITS: no.
 * Entry: nothing. Exit: A $20, N and Z from it; X, Y, C and V as StageResetPlayer152D left them.
 */
StageRegs StageDeathFallBackLeft152D(StageRegs r)
{
    return fall_ends(r, 0x8262, 0x20);
}

void abi_StageDeathFallBackLeft152D(void)
{
    regs_exit_shim(StageDeathFallBackLeft152D(regs_entry_shim()));
}

/*
 * @routine 152D:271 StageDeathFallRight152D
 * @run 6B:8271
 * @unreachable 6B:8271-6B:82B3 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8271 runs whatever overlay another scene
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
 * @unreachable 6B:82B4-6B:82BD no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $82B4 runs whatever overlay another scene
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
 * @unreachable 6B:82C6-6B:82EF no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $82C6 runs whatever overlay another scene
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
 * @unreachable 6B:82FC-6B:8302 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $82FC runs whatever overlay another scene
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
 * @unreachable 6B:830D-6B:832D no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $830D runs whatever overlay another scene
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
 * @unreachable 6B:832E-6B:8370 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $832E runs whatever overlay another scene
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
 * @unreachable 6B:8371-6B:837D no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8371 runs whatever overlay another scene
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
 * @unreachable 6B:837E-6B:83C0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $837E runs whatever overlay another scene
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
 * @unreachable 6B:83C1-6B:8435 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $83C1 runs whatever overlay another scene
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
    clear_scroll_bytes(r);
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
 * @unreachable 6B:8436-6B:843F no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8436 runs whatever overlay another scene
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
 * @unreachable 6B:8446-6B:847B no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8446 runs whatever overlay another scene
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
 * @unreachable 6B:847C-6B:8498 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $847C runs whatever overlay another scene
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
 * @unreachable 6B:8499-6B:84D7 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8499 runs whatever overlay another scene
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
 * @unreachable 6B:84D8-6B:84DE no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $84D8 runs whatever overlay another scene
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
 * @unreachable 6B:84E7-6B:84E9 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $84E7 runs whatever overlay another scene
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
 * @unreachable 6B:84EA-6B:852B no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $84EA runs whatever overlay another scene
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
 * @unreachable 6B:852C-6B:8541 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $852C runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-mode 4, the fade out after the death or the stage: the events step and the rows, then the step at
 * zField80 from the seven at $8542 (the fade's start, its wait, the CD's stop and the stock taken, the waits, the
 * restart).
 * HOW: StageModeEventsStep152D, StageRows152D; a compare that branches to the next instruction either way;
 * MapOverlayBanks5And6; zField80 doubled indexes the table, whose word is jumped through. Step 3, at $85A1, is the
 * bytes of StageWaitTimerStep (stage_mode.c, 024E:023), a census duplicate run through its C.
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
        r.nz = StageWaitTimerStep();
        return r;
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
 * @unreachable 6B:8550-6B:8558 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8550 runs whatever overlay another scene
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
 * @unreachable 6B:8559-6B:8570 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8559 runs whatever overlay another scene
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

/* A decimal add of 1 to A with C clear (SED CLC ADC #1): the sum, N and Z from it, C the carry out; V kept. */
static Regs bcd_plus_one(Regs r)
{
    unsigned sum = (r.a & 0x0Fu) + 1u;

    if (sum >= 0x0A) {
        sum += 0x06;
    }
    sum += r.a & 0xF0u;
    if (sum >= 0xA0) {
        sum += 0x60;
    }
    r.c = (sum & 0xFF00u) != 0;
    return loaded(r, (uint8_t)sum);
}

/* A decimal subtract of 1 from A with C set (SED SEC SBC #1): the difference, N and Z from it, C no borrow; V kept. */
static Regs bcd_less_one(Regs r)
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
    return loaded(r, value);
}

/* A life taken from the stock in A and written to zStock. */
static Regs take_stock(Regs r)
{
    r = bcd_less_one(r);
    WR8(ZP->stock, r.a);
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
 * @unreachable 6B:8571-6B:857F no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8571 runs whatever overlay another scene
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
 * @unreachable 6B:8580-6B:8596 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8580 runs whatever overlay another scene
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
 * @unreachable 6B:8597-6B:85A0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8597 runs whatever overlay another scene
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
 * @routine 152D:5A9 StageFadeRestart152D
 * @run 6B:85A9
 * @unreachable 6B:85A9-6B:85C3 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $85A9 runs whatever overlay another scene
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
 * @unreachable 6B:85C4-6B:85E9 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $85C4 runs whatever overlay another scene
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

/*
 * @routine 152D:5EA StageLoadRowMap152D
 * @run 6B:85EA
 * @unreachable 6B:85EA-6B:8618 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $85EA runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Fills the stage's row map at $3A00: cleared from KernelZeros (255 bytes), then unpacked from the route's
 * list (the table at $9A68 by zField82): a byte that starts a run gives the map's index, and each byte after it is a
 * row's value in its low seven bits, its bit 7 saying that the next byte starts a new run; $FF ends the list.
 * HOW: TAI; the list's address in zTemp00-01; two loops, the run bit rotated into zTemp02.
 * WAITS: no.
 * Entry: nothing. Exit: A $FF, Z set, C set; X the map index reached, Y the list offset after the end. V kept.
 * Clobbers zTemp00-02.
 */
StageRegs StageLoadRowMap152D(StageRegs r)
{
    uint8_t route, index, old;

    tai_shim(r, KernelZeros, ROW_MAP, 0x00FF);
    route = RD8(ZP->field82);
    index = (uint8_t)(route << 1);
    r.c = (route & 0x80) != 0;
    r = loaded(r, index);
    r.y = index;
    r = loaded(r, rd8((uint16_t)(0x9A68 + r.y)));
    WR8(ZP->temp00, r.a);
    r = loaded(r, rd8((uint16_t)(0x9A69 + r.y)));
    WR8(ZP->temp01, r.a);
    r.y = 0;
    for (;;) {
        r = loaded(r, RD8_PTR_Y(ZP->temp00, r.y));
        r.x = r.a;
        r.y++;
        for (;;) {
            r = loaded(r, RD8_PTR_Y(ZP->temp00, r.y));
            r.y++;
            r = compared(r, 0xFF);
            if (r.nz == 0) {
                return r;
            }
            r.c = (r.a & 0x80) != 0;
            old = RD8(ZP->temp02);
            WR8(ZP->temp02, (uint8_t)((old << 1) | (r.c ? 1 : 0)));
            r = loaded(r, (uint8_t)(r.a & 0x7F));
            r.c = false;
            wr8((uint16_t)(ROW_MAP + r.x), r.a);
            r.x++;
            r.nz = r.x;
            if ((RD8(ZP->temp02) & 0x01) != 0) {
                break;
            }
        }
    }
}

void abi_StageLoadRowMap152D(void)
{
    regs_exit_shim(StageLoadRowMap152D(regs_entry_shim()));
}

/*
 * @routine 152D:619 StageRouteSetUpA152D
 * @run 6B:8619
 * @unreachable 6B:8619-6B:8626 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8619 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The kernel's pieces at $403F and at $400C (with A the route's byte from the table at $9A59 by
 * zField82), then the route's list unpacked into SaveBuffer (StageUnpackList152D).
 * HOW: Two calls into the kernel; a jump to StageUnpackList152D.
 * WAITS: where the kernel's pieces wait.
 * Entry: nothing. Exit: as StageUnpackList152D.
 */
StageRegs StageRouteSetUpA152D(StageRegs r)
{
    r = orig_shim(r, 0x861C, 0x403F);
    r.x = RD8(ZP->field82);
    r.nz = r.x;
    r = loaded(r, rd8((uint16_t)(0x9A59 + r.x)));
    r = orig_shim(r, 0x8624, 0x400C);
    return StageUnpackList152D(r);
}

void abi_StageRouteSetUpA152D(void)
{
    regs_exit_shim(StageRouteSetUpA152D(regs_entry_shim()));
}

/*
 * @routine 152D:627 StageRouteSetUpB152D
 * @run 6B:8627
 * @unreachable 6B:8627-6B:8630 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8627 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The kernel's piece at $4015 with A the route's byte from the table at $9A5C by zField82 and X 7.
 * HOW: A table read; a jump into the kernel.
 * WAITS: where the kernel's piece waits.
 * Entry: nothing. Exit: as $4015 leaves them.
 */
StageRegs StageRouteSetUpB152D(StageRegs r)
{
    r.x = RD8(ZP->field82);
    r = loaded(r, rd8((uint16_t)(0x9A5C + r.x)));
    r.x = 7;
    r.nz = 7;
    return orig_tail_shim(r, 0x4015);
}

void abi_StageRouteSetUpB152D(void)
{
    regs_exit_shim(StageRouteSetUpB152D(regs_entry_shim()));
}

/* StageLoadScreen152D called by BSR with zField8E kept on the stack across it and put back. */
static Regs load_screen_keeping_flags(Regs r, uint16_t ret)
{
    uint8_t flags;

    r = loaded(r, RD8(ZP->field8e));
    push8(r.a);
    CALL(ret, StageLoadScreen152D(r));
    flags = pull8();
    r = loaded(r, flags);
    WR8(ZP->field8e, flags);
    return r;
}

/*
 * @routine 152D:631 StageResetScroll152D
 * @run 6B:8631
 * @unreachable 6B:8631-6B:8684 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8631 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sets the stage's screen up: the kernel's piece at $4009, the scroll cleared and loaded for the stage
 * (zField8E kept), the overlay at $A000's piece at $A5EB; for the raster effects 3, 5 and 6 the screen is loaded a
 * second time and $A626 runs; then the scroll is cleared and it runs on into StageLoadScreen152D.
 * HOW: $4009; TAI twice and StageLoadScreen152D by BSR; MapOverlayBank7, $A5EB; compares of zRasterEffect.
 * WAITS: where the kernel's piece waits.
 * Entry: nothing. Exit: as StageLoadScreen152D.
 */
StageRegs StageResetScroll152D(StageRegs r)
{
    r = orig_shim(r, 0x8634, KERNEL_4009);
    clear_scroll_bytes(r);
    r = load_screen_keeping_flags(r, 0x8647);
    r = map_bank_7(r, 0x864D);
    r = orig_shim(r, 0x8650, 0xA5EB);
    r = loaded(r, RD8(ZP->raster_effect));
    r = compared(r, 0x05);
    if (r.nz != 0) {
        r = compared(r, 0x06);
    }
    if (r.nz != 0) {
        r = compared(r, 0x03);
    }
    if (r.nz == 0) {
        clear_scroll_bytes(r);
        r = load_screen_keeping_flags(r, 0x8671);
        r = orig_shim(r, 0x8677, 0xA626);
    }
    clear_scroll_bytes(r);
    return StageLoadScreen152D(r);
}

void abi_StageResetScroll152D(void)
{
    regs_exit_shim(StageResetScroll152D(regs_entry_shim()));
}

/* The next byte of the screen's record through zTemp00-01, Y counted past it. */
static Regs screen_byte(Regs r)
{
    r = loaded(r, RD8_PTR_Y(ZP->temp00, r.y));
    r.y++;
    return r;
}

/*
 * @routine 152D:685 StageLoadScreen152D
 * @run 6B:8685
 * @unreachable 6B:8685-6B:872F no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8685 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Loads the stage's screen from its record (the table at $9A90 by zField83): the scroll's pages, X and Y,
 * steps, speeds and limits; unless zField8E's bit 1 asks to keep the player (it is then cleared), the player's X and
 * the two bytes at $2C50 and $2C08 and its page; for a character other than 0 the player's X is moved by the table at
 * $8730 by the byte at $2818 less 5; then zRasterEffect (zRasterArg cleared) and the byte at $2AA0.
 * HOW: The record's address in zTemp00-01; a byte at a time through it, each stored where it goes.
 * WAITS: no.
 * Entry: nothing. Exit: A the record's last byte, N and Z from it, Y past it; X, C and V as the moves of the player left
 * them. Clobbers zTemp00-01.
 */
StageRegs StageLoadScreen152D(StageRegs r)
{
    Sum sum;
    uint8_t stage, index, value, move;

    WR8(ZP->raster_stage, 0);
    stage = RD8(ZP->field83);
    index = (uint8_t)(stage << 1);
    r.c = (stage & 0x80) != 0;
    r = loaded(r, index);
    r.y = index;
    r = loaded(r, rd8((uint16_t)(0x9A90 + r.y)));
    WR8(ZP->temp00, r.a);
    r = loaded(r, rd8((uint16_t)(0x9A91 + r.y)));
    WR8(ZP->temp01, r.a);
    r.y = 0;
    r = screen_byte(r);
    WR8(SCROLLE->step_x_lo[6], r.a);
    WR8(SCROLLE->step_x_lo[7], r.a);
    WR8(SCROLLE->step_x_hi[0], r.a);
    WR8(SCROLLE->step_x_hi[1], r.a);
    r = loaded(r, (uint8_t)(r.a & 0x01));
    WR8(SCROLL->x_hi[0], r.a);
    WR8(SCROLL->x_hi[1], r.a);
    WR8(SCROLL->x_hi[2], r.a);
    WR8(SCROLL->x_hi[3], r.a);
    r = screen_byte(r);
    WR8(SCROLLE->step_y_hi[6], r.a);
    r = loaded(r, (uint8_t)(r.a & 0x01));
    WR8(SCROLL->y_hi[0], r.a);
    r = screen_byte(r);
    WR8(SCROLL->x_lo[0], r.a);
    r = screen_byte(r);
    WR8(SCROLL->y_lo[0], r.a);
    r = screen_byte(r);
    WR8(SCROLLE->speed_x[6], r.a);
    r = screen_byte(r);
    WR8(BANDS->bgy_lo[0], r.a);
    r = screen_byte(r);
    WR8(SCROLLE->speed_y[6], r.a);
    r = screen_byte(r);
    wr8(SCREEN_3EB4, r.a);
    r = screen_byte(r);
    WR8(SCROLLLIM->raster_water_line, r.a);
    r = screen_byte(r);
    wr8(SCREEN_3EC4, r.a);
    if ((RD8(ZP->field8e) & 0x02) != 0) {
        RMW8(ZP->field8e, old, old & (uint8_t)~0x02);
        r.y = (uint8_t)(r.y + 2);
    } else {
        r = screen_byte(r);
        WR8(OBJ->x_hi[8], r.a);
        r = screen_byte(r);
        WR8(OBJ->x_frac[48], r.a);
        WR8(OBJ->x_lo[64], 0);
        WR8(OBJ->flash_flags[24], 0);
        r = loaded(r, RD8(ZP->character));
        if (r.a != 0) {
            r = loaded(r, RD8(OBJ->f2800[24]));
            r = compared(r, 0x05);
            if (r.c) {
                sum = sub_bytes(r.a, 0x05, true);
                r = loaded(r, sum.value);
                r.c = sum.carry;
                r.v = sum.overflow;
                r.x = sum.value;
                value = RD8(OBJ->x_hi[8]);
                move = rd8((uint16_t)(0x8730 + r.x));
                sum = add_bytes(value, move, false);
                r = loaded(r, sum.value);
                r.c = sum.carry;
                r.v = sum.overflow;
                WR8(OBJ->x_hi[8], sum.value);
            }
        }
    }
    r = screen_byte(r);
    WR8(ZP->raster_effect, r.a);
    WR8(ZP->raster_arg, 0);
    r = screen_byte(r);
    WR8(OBJ->frame[56], r.a);
    return r;
}

void abi_StageLoadScreen152D(void)
{
    regs_exit_shim(StageLoadScreen152D(regs_entry_shim()));
}

/*
 * @routine 152D:734 StageOverlayFrame152D
 * @run 6B:8734
 * @unreachable 6B:8734-6B:8747 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8734 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The frame of the overlays above: the piece at $AE47 under bank 7, then the piece at $A000 under banks
 * 8 and 9 above zRamBaseBank.
 * HOW: MapOverlayBank7, $AE47; the add and two TAM; $A000.
 * WAITS: where the pieces wait.
 * Entry: nothing. Exit: as $A000 leaves them.
 */
StageRegs StageOverlayFrame152D(StageRegs r)
{
    Sum sum;

    r = map_bank_7(r, 0x8737);
    r = orig_shim(r, 0x873A, 0xAE47);
    sum = add_bytes(0x08, RD8(ZP->ram_base_bank), false);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    tam(0x20, r.a);
    r = loaded(r, (uint8_t)(r.a + 1));
    tam(0x40, r.a);
    return orig_shim(r, 0x8747, 0xA000);
}

void abi_StageOverlayFrame152D(void)
{
    regs_exit_shim(StageOverlayFrame152D(regs_entry_shim()));
}

/*
 * @routine 152D:748 StageLevelStart152D
 * @run 6B:8748
 * @unreachable 6B:8748-6B:877A no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8748 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Starts the stage's level: the kernel's piece at $4057 under bank 7, the level's stream and axis from the
 * four bytes at $9B20 by zField83, the 32 event flags cleared when the fourth byte is not 0, then the overlay at
 * $A000's piece at $ADAF.
 * HOW: MapOverlayBank7, $4057; table reads; a loop of stores with Y kept on the stack; $ADAF.
 * WAITS: where the pieces wait.
 * Entry: nothing. Exit: as $ADAF leaves them.
 */
StageRegs StageLevelStart152D(StageRegs r)
{
    uint8_t stage, index;

    r = map_bank_7(r, 0x874B);
    r = orig_shim(r, 0x874E, 0x4057);
    stage = RD8(ZP->field83);
    index = (uint8_t)(stage << 2);
    r.c = (stage & 0x40) != 0;
    r = loaded(r, index);
    r.y = index;
    r = loaded(r, rd8((uint16_t)(0x9B20 + r.y)));
    WR8(LEVEL->level_stream.lo, r.a);
    r = loaded(r, rd8((uint16_t)(0x9B21 + r.y)));
    WR8(LEVEL->level_stream.hi, r.a);
    r = loaded(r, rd8((uint16_t)(0x9B22 + r.y)));
    WR8(LEVEL->level_axis, r.a);
    r = loaded(r, rd8((uint16_t)(0x9B23 + r.y)));
    if (r.a != 0) {
        push8(r.y);
        for (r.y = 0; r.y < 0x20; r.y++) {
            r = loaded(r, 0);
            WR8(LEVEL->event_flags[r.y], 0);
        }
        r.c = true;
        r.y = pull8();
        r.nz = r.y;
    }
    return orig_shim(r, 0x877A, 0xADAF);
}

void abi_StageLevelStart152D(void)
{
    regs_exit_shim(StageLevelStart152D(regs_entry_shim()));
}

/* The handlers' piece for an object whose byte at $2818 is 0: $A36C, then the object's byte at $2F20 unpacked, its low
 * nibble into $3310, the next two bits into $3358 (and the two above into $32C8 when both are kept). */
static Regs unpack_door(Regs r, uint16_t ret, bool third)
{
    uint8_t slot, packed, rolled;
    bool carry;

    r = orig_shim(r, ret, 0xA36C);
    slot = r.x;
    packed = RD8(OBJ->fields_2ee0[64 + slot]);
    r = loaded(r, packed);
    r.y = packed;
    r = loaded(r, (uint8_t)(packed & 0x0F));
    WR8(OBJ->f3300[16 + slot], r.a);
    rolled = r.y;
    carry = r.c;
    for (int i = 0; i < 3; i++) {
        bool out = (rolled & 0x80) != 0;

        rolled = (uint8_t)((rolled << 1) | (carry ? 1 : 0));
        carry = out;
    }
    r.c = carry;
    if (third) {
        r.y = rolled;
    }
    r = loaded(r, (uint8_t)(rolled & 0x03));
    WR8(OBJ->f3358[slot], r.a);
    if (!third) {
        return r;
    }
    rolled = r.y;
    for (int i = 0; i < 2; i++) {
        bool out = (rolled & 0x80) != 0;

        rolled = (uint8_t)((rolled << 1) | (carry ? 1 : 0));
        carry = out;
    }
    r.c = carry;
    r = loaded(r, (uint8_t)(rolled & 0x03));
    WR8(OBJ->f32a8[32 + slot], r.a);
    return r;
}

/* A door taken when its gate is open: the object's $3790 cleared; with the gate and the object's state at $31F0 from 1
 * to 4 the stage goes to the door's room (zField83 from $3310, zField87 from $3358, zSubStep 2), else the state is
 * cleared. */
static Regs take_door(Regs r, bool by_pad)
{
    uint8_t slot = r.x;

    WR8(OBJ->f3778[24 + slot], 0);
    if (by_pad) {
        r.y = RD8(OBJ->f32a8[32 + slot]);
        r.nz = r.y;
        r = loaded(r, rd8((uint16_t)(PadOverrides + r.y)));
    } else {
        r = loaded(r, rd8(PlayerByte3BD7));
    }
    if (r.a != 0) {
        r = loaded(r, RD8(OBJ->status_flags[80 + slot]));
        if (r.a != 0) {
            r = compared(r, 0x05);
            if (!r.c) {
                r = loaded(r, RD8(OBJ->f3300[16 + slot]));
                WR8(ZP->field83, r.a);
                r = loaded(r, RD8(OBJ->f3358[slot]));
                WR8(ZP->field87, r.a);
                r = loaded(r, 0x02);
                WR8(ZP->sub_step, 0x02);
                return r;
            }
        }
    }
    WR8(OBJ->status_flags[80 + slot], 0);
    return r;
}

/*
 * @routine 152D:77B DoorHandler152D
 * @run 6B:877B
 * @unreachable 6B:877B-6B:87C5 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $877B runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: An object handler for a door the pad's override opens. Not yet placed (its byte at $2818 0): $A36C, and
 * the object's packed byte at $2F20 unpacked into its room ($3310), the room's entry ($3358) and the pad override's
 * index ($32C8). Placed: the byte at $3790 cleared, and when the override at PadOverrides by that index is set and the
 * object's state at $31F0 is 1 to 4, the stage goes through the door (zField83 the room, zField87 the entry, zSubStep
 * 2); otherwise the state is cleared.
 * HOW: Loads, shifts through the carry and stores; the tests and the stores of the door.
 * WAITS: no.
 * Entry: X the object. Exit: A the last value loaded or masked, N and Z from it, C from the shifts or the compare, Y
 * the packed byte shifted or the override's index; X kept.
 */
StageRegs DoorHandler152D(StageRegs r)
{
    r = loaded(r, RD8(OBJ->f2800[24 + r.x]));
    if (r.a == 0) {
        return unpack_door(r, 0x8783, true);
    }
    return take_door(r, true);
}

void abi_DoorHandler152D(void)
{
    regs_exit_shim(DoorHandler152D(regs_entry_shim()));
}

/*
 * @routine 152D:7C6 DoorHandlerB152D
 * @run 6B:87C6
 * @unreachable 6B:87C6-6B:8800 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $87C6 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: An object handler for a door PlayerByte3BD7 opens: as DoorHandler152D, with no pad override (the
 * packed byte gives the room and the entry only), the gate PlayerByte3BD7. Its fall-back that clears the state is
 * DoorHandler152D's bytes at $87C2.
 * HOW: Loads, shifts through the carry and stores; the tests and the stores of the door.
 * WAITS: no.
 * Entry: X the object. Exit: as DoorHandler152D, Y the packed byte when it was unpacked.
 */
StageRegs DoorHandlerB152D(StageRegs r)
{
    r = loaded(r, RD8(OBJ->f2800[24 + r.x]));
    if (r.a == 0) {
        return unpack_door(r, 0x87CE, false);
    }
    return take_door(r, false);
}

void abi_DoorHandlerB152D(void)
{
    regs_exit_shim(DoorHandlerB152D(regs_entry_shim()));
}

/*
 * @routine 152E:001 StageHudStart152D
 * @run 6B:8801
 * @unreachable 6B:8801-6B:8810 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8801 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Draws the stage's display at its start: the pieces 0 and 1 of StageDrawPiece152D, the stock and the
 * points to add (StageDrawStock152D, StageDrawScoreAdd152D); zField8E's bit 5 cleared; then runs on into StageHudStep152D.
 * HOW: Calls with X 0 and 1; a bit cleared.
 * WAITS: no.
 * Entry: nothing. Exit: as StageHudStep152D.
 */
StageRegs StageHudStart152D(StageRegs r)
{
    r.x = 0;
    CALL(0x8805, StageDrawPiece152D(r));
    r.x++;
    r.nz = r.x;
    CALL(0x8809, StageDrawPiece152D(r));
    CALL(0x880C, StageDrawStock152D(r));
    CALL(0x880F, StageDrawScoreAdd152D(r));
    RMW8(ZP->field8e, old, old & (uint8_t)~0x20);
    return StageHudStep152D(r);
}

void abi_StageHudStart152D(void)
{
    regs_exit_shim(StageHudStart152D(regs_entry_shim()));
}

/*
 * @routine 152E:011 StageHudStep152D
 * @run 6B:8811
 * @unreachable 6B:8811-6B:8877 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8811 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The stage's display each frame: the item sound's step, the item's frame at $2A9E from the table at
 * $8878 by zField93; the health bar moved one step toward zPlayerHealth (zField99 the shown health: down, the bar's
 * piece cleared; up, set, with sound $0B once when the bar was full and zField8E's bit 6 not yet set), sound $2C when it
 * reaches it after rising; the hearts' two digits when zField94 changed; and while zField8E's bit 2 is set the boss's
 * bar (StageBossBar152D).
 * HOW: StageHudSound152D; compares of zPlayerHealth with zField99; StageHealthBarAddress152D by BSR with AndVramData
 * or OrVramData (census duplicates at $942A and $9439, through map_draw.c's C); the sound hook; StageDrawDigit152D for each digit; a jump to StageBossBar152D.
 * WAITS: no.
 * Entry: nothing. Exit: as the last piece that ran left them.
 */
StageRegs StageHudStep152D(StageRegs r)
{
    uint8_t health, shown, hearts, before;
    bool rose = false;

    CALL(0x8814, StageHudSound152D(r));
    r.x = RD8(ZP->field93);
    r.nz = r.x;
    r = loaded(r, rd8((uint16_t)(0x8878 + r.x)));
    WR8(OBJ->frame[54], r.a);
    health = RD8(ZP->player_health);
    r = loaded(r, health);
    shown = RD8(ZP->field99);
    r = compared(r, shown);
    if (r.nz == 0) {
        RMW8(ZP->field8e, old, old | 0x20);
    } else if (!r.c) {
        CALL(0x8826, StageHealthBarAddress152D(r));
        r.x = 0xFF;
        r.y = 0xF0;
        r.nz = 0xF0;
        r = and_vram(r, 0x882D);
        r.nz = DEC8(ZP->field99);
    } else {
        if ((RD8(ZP->field8e) & 0x20) != 0 && (RD8(ZP->field8e) & 0x40) == 0) {
            r = loaded(r, 0x0B);
            r = orig_shim(r, 0x883C, SOUND_HOOK);
            RMW8(ZP->field8e, old, old | 0x40);
        }
        r.nz = INC8(ZP->field99);
        CALL(0x8842, StageHealthBarAddress152D(r));
        r.x = 0;
        r.y = 0x0F;
        r.nz = 0x0F;
        r = or_vram(r, 0x8849);
        rose = true;
    }
    if (!rose && (RD8(ZP->field8e) & 0x40) != 0) {
        RMW8(ZP->field8e, old, old & (uint8_t)~0x40);
        r = loaded(r, 0x2C);
        r = orig_shim(r, 0x8857, SOUND_HOOK);
    }
    hearts = RD8(ZP->field94);
    r = loaded(r, hearts);
    before = RD8(ZP->field95);
    r = compared(r, before);
    if (r.nz != 0) {
        WR8(ZP->field95, r.a);
        r = loaded(r, (uint8_t)(r.a & 0x0F));
        r.x = 8;
        r.nz = 8;
        CALL(0x8866, StageDrawDigit152D(r));
        r = loaded(r, RD8(ZP->field94));
        r.c = (r.a & 0x08) != 0;
        r = loaded(r, (uint8_t)(r.a >> 4));
        r.x = 9;
        r.nz = 9;
        CALL(0x8871, StageDrawDigit152D(r));
    }
    if ((RD8(ZP->field8e) & 0x04) == 0) {
        return r;
    }
    return StageBossBar152D(r);
}

void abi_StageHudStep152D(void)
{
    regs_exit_shim(StageHudStep152D(regs_entry_shim()));
}

/*
 * @routine 152E:081 StageHudSound152D
 * @run 6B:8881
 * @unreachable 6B:8881-6B:8890 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8881 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The item's glow: the kernel's piece at $4018 with X 1 and A the colour of the step the wave counter's low
 * three bits (masked by RichterItemReady) pick from the eight at $8891.
 * HOW: Two ANDs and a table read; a jump into the kernel.
 * WAITS: no.
 * Entry: nothing. Exit: as $4018 leaves them.
 */
StageRegs StageHudSound152D(StageRegs r)
{
    uint8_t step = (uint8_t)(RD8(ZP->wave_counter) & 0x07);

    r.x = (uint8_t)(step & rd8(RichterItemReady));
    r = loaded(r, rd8((uint16_t)(0x8891 + r.x)));
    r.x = 1;
    r.nz = 1;
    return orig_tail_shim(r, 0x4018);
}

void abi_StageHudSound152D(void)
{
    regs_exit_shim(StageHudSound152D(regs_entry_shim()));
}

/* The VRAM address of a bar's piece $5E less COUNT on: the low nibble in zTemp00, the high nibble times eight carried
 * into zTemp01; the address in X (low) and Y (high), the high byte HIGH plus the carry and zTemp01. */
static Regs bar_address(Regs r, uint8_t count, uint8_t low_add, uint8_t high)
{
    Sum sum;
    uint8_t piece, old, temp;

    sum = sub_bytes(0x5E, count, true);
    r.c = sum.carry;
    r.v = sum.overflow;
    piece = sum.value;
    push8(piece);
    r = loaded(r, (uint8_t)(piece & 0x0F));
    if (low_add != 0) {
        sum = add_bytes(r.a, low_add, false);
        r = loaded(r, sum.value);
    }
    WR8(ZP->temp00, r.a);
    piece = pull8();
    r = loaded(r, (uint8_t)(piece & 0xF0));
    WR8(ZP->temp01, 0);
    for (int i = 0; i < 3; i++) {
        bool out = (r.a & 0x80) != 0;

        r.a = (uint8_t)(r.a << 1);
        old = RD8(ZP->temp01);
        WR8(ZP->temp01, (uint8_t)((old << 1) | (out ? 1 : 0)));
        r.c = (old & 0x80) != 0;
    }
    temp = RD8(ZP->temp00);
    sum = add_bytes(r.a, temp, false);
    r.a = sum.value;
    r.c = sum.carry;
    r.v = sum.overflow;
    if (high == 0) {
        return r;
    }
    r.x = r.a;
    r = loaded(r, RD8(ZP->temp01));
    sum = add_bytes(r.a, high, r.c);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    r.y = r.a;
    return r;
}

/*
 * @routine 152E:099 StageHealthBarAddress152D
 * @run 6B:8899
 * @unreachable 6B:8899-6B:88C1 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8899 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Points the VDC at the health bar's piece for the shown health zField99 ($5E less it, in the rows from
 * $6500), unless zField99 is 0.
 * HOW: A subtract, nibbles and shifts into zTemp00-01, an add; SetVramAddresses152D with the address in X and
 * Y.
 * WAITS: no.
 * Entry: nothing. Exit, zField99 0: A 0, N and Z from it. Else: A 2, N and Z clear; X and Y the address, C from the
 * add, V from it. Clobbers zTemp00-01.
 */
StageRegs StageHealthBarAddress152D(StageRegs r)
{
    uint8_t shown;

    r = loaded(r, RD8(ZP->field99));
    if (r.a == 0) {
        return r;
    }
    shown = RD8(ZP->field99);
    r = bar_address(r, shown, 0, 0x65);
    return set_vram_addresses(r, 0x88C1);
}

void abi_StageHealthBarAddress152D(void)
{
    regs_exit_shim(StageHealthBarAddress152D(regs_entry_shim()));
}

/*
 * @routine 152E:0C2 StageBossBarReset152D
 * @run 6B:88C2
 * @unreachable 6B:88C2-6B:88CB no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $88C2 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The boss's bar drawn anew (StageDrawPiece152D 2) and its length at $2745 set to $5C; then runs on into
 * StageBossBar152D. Reached from StageBossBar152D while the boss's frame at $2A9D is 0.
 * HOW: A call with X 2; a store.
 * WAITS: no.
 * Entry: nothing. Exit: as StageBossBar152D.
 */
StageRegs StageBossBarReset152D(StageRegs r)
{
    r.x = 2;
    r.nz = 2;
    CALL(0x88C7, StageDrawPiece152D(r));
    r = loaded(r, 0x5C);
    wr8(BOSS_BAR, 0x5C);
    return StageBossBar152D(r);
}

void abi_StageBossBarReset152D(void)
{
    regs_exit_shim(StageBossBarReset152D(regs_entry_shim()));
}

/*
 * @routine 152E:0CC StageBossBar152D
 * @run 6B:88CC
 * @unreachable 6B:88CC-6B:88DA no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $88CC runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The boss's bar each frame: while the boss's frame at $2A9D is 0 the bar is drawn anew
 * (StageBossBarReset152D); the boss's X at $2BBD is moved left until it reaches $D8; then StageBossBarStep152D.
 * HOW: Tests; a decrement; a branch or run into StageBossBarStep152D.
 * WAITS: no.
 * Entry: nothing. Exit: as StageBossBarStep152D.
 */
StageRegs StageBossBar152D(StageRegs r)
{
    r = loaded(r, RD8(OBJ->frame[53]));
    if (r.a == 0) {
        return StageBossBarReset152D(r);
    }
    r = loaded(r, RD8(OBJ->x_hi[77]));
    r = compared(r, 0xD8);
    if (r.nz != 0) {
        r.nz = DEC8(OBJ->x_hi[77]);
    }
    return StageBossBarStep152D(r);
}

void abi_StageBossBar152D(void)
{
    regs_exit_shim(StageBossBar152D(regs_entry_shim()));
}

/*
 * @routine 152E:0DB StageBossBarStep152D
 * @run 6B:88DB
 * @unreachable 6B:88DB-6B:891B no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $88DB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Moves the boss's bar at $2745 one step toward the boss's health at $3128: down, the piece cleared; up,
 * set. A negative health is the boss's end: the bar is stepped with a health of 0 (by a call of this routine) and the
 * health put back; once the bar is empty, the count at $330D takes the health. While the count at $330D is not 0 the
 * bar flashes instead (StageBossBarFlash152D).
 * HOW: Compares; StageBossBarAddress152D by BSR with AndVramData or OrVramData (map_draw.c's C, census duplicates at
 * $942A and $9439); the health kept on the stack across the call of itself.
 * WAITS: no.
 * Entry: nothing. Exit: as the last piece that ran left them; at the bar, A the health, N, Z and C from the compare.
 */
StageRegs StageBossBarStep152D(StageRegs r)
{
    uint8_t health, bar;

    r = loaded(r, RD8(OBJ->f3300[13]));
    if (r.a != 0) {
        return StageBossBarFlash152D(r);
    }
    health = RD8(OBJ->f30f0[56]);
    r = loaded(r, health);
    if ((health & 0x80) != 0) {
        push8(r.a);
        WR8(OBJ->f30f0[56], 0);
        CALL(0x8902, StageBossBarStep152D(r));
        health = pull8();
        r = loaded(r, health);
        WR8(OBJ->f30f0[56], health);
        r.y = rd8(BOSS_BAR);
        r.nz = r.y;
        if (r.y != 0) {
            return r;
        }
        WR8(OBJ->f3300[13], r.a);
        return r;
    }
    bar = rd8(BOSS_BAR);
    r = compared(r, bar);
    if (r.nz == 0) {
        return r;
    }
    if (r.c) {
        r.nz = (uint8_t)(rd8(BOSS_BAR) + 1);
        wr8(BOSS_BAR, r.nz);
        CALL(0x8914, StageBossBarAddress152D(r));
        r.x = 0xF0;
        r.y = 0;
        r.nz = 0;
        return or_vram(r, 0x891B);
    }
    CALL(0x88F1, StageBossBarAddress152D(r));
    r.x = 0x0F;
    r.y = 0xFF;
    r.nz = 0xFF;
    r = and_vram(r, 0x88F8);
    r.nz = (uint8_t)(rd8(BOSS_BAR) - 1);
    wr8(BOSS_BAR, r.nz);
    return r;
}

void abi_StageBossBarStep152D(void)
{
    regs_exit_shim(StageBossBarStep152D(regs_entry_shim()));
}

/*
 * @routine 152E:11C StageBossBarAddress152D
 * @run 6B:891C
 * @unreachable 6B:891C-6B:8964 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $891C runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Points the VDC at the boss's bar's piece for the length at $2745 ($5E less it, from $5D40, a page on
 * at $3F and more), unless the length is 0.
 * HOW: The page in zTemp08-09; a subtract, nibbles and shifts into zTemp00-01, adds; SetVramAddresses152D with the address in X and Y.
 * WAITS: no.
 * Entry: nothing. Exit, length 0: A 0, N and Z from it. Else: A 2, N and Z clear; X and Y the address, C and V from the
 * last add. Clobbers zTemp00-01 and zTemp08-09.
 */
StageRegs StageBossBarAddress152D(StageRegs r)
{
    Sum sum;
    uint8_t length, value;

    r = loaded(r, rd8(BOSS_BAR));
    if (r.a == 0) {
        return r;
    }
    WR8(ZP->temp08, 0);
    WR8(ZP->temp09, 0);
    r = compared(r, 0x3F);
    if (r.c) {
        r = loaded(r, 0xC0);
        WR8(ZP->temp08, 0xC0);
        r.nz = INC8(ZP->temp09);
    }
    length = rd8(BOSS_BAR);
    r = bar_address(r, length, 0x40, 0);
    WR8(ZP->temp00, r.a);
    value = RD8(ZP->temp01);
    sum = add_bytes(value, 0x5D, r.c);
    WR8(ZP->temp01, sum.value);
    value = RD8(ZP->temp08);
    sum = add_bytes(value, RD8(ZP->temp00), false);
    r.x = sum.value;
    value = RD8(ZP->temp09);
    sum = add_bytes(value, RD8(ZP->temp01), sum.carry);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    r.y = sum.value;
    return set_vram_addresses(r, 0x8964);
}

void abi_StageBossBarAddress152D(void)
{
    regs_exit_shim(StageBossBarAddress152D(regs_entry_shim()));
}

/*
 * @routine 152E:17D StageBossBarFlash152D
 * @run 6B:897D
 * @unreachable 6B:897D-6B:89BD no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $897D runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The boss's bar at its end: every other piece of the four (by the wave counter's bit 0, from the table
 * of addresses at $8965) blanked by StageBarErase152D; the flash's step at $339D counted; the boss's speed at $2DFD and
 * $2DB5 raised by $0C; the kernel's piece at $4045 with X $45; once the boss's page at $2C4D is 0 the boss's frame at
 * $2A9D and zField8E's bit 2 are cleared.
 * HOW: A loop of four in zTemp01; adds; the kernel; stores.
 * WAITS: no.
 * Entry: nothing. Exit: A the boss's page, N and Z from it, the rest as $4045 left them.
 */
StageRegs StageBossBarFlash152D(StageRegs r)
{
    Sum sum;
    uint8_t index, value;

    r = loaded(r, (uint8_t)(RD8(ZP->wave_counter) & 0x01));
    WR8(ZP->temp01, r.a);
    do {
        value = RD8(ZP->temp01);
        index = (uint8_t)(value << 1);
        r.c = (value & 0x80) != 0;
        r.x = index;
        r.y = rd8((uint16_t)(0x8966 + index));
        r = loaded(r, rd8((uint16_t)(0x8965 + index)));
        r.x = r.a;
        CALL(0x8991, StageBarErase152D(r));
        r.nz = INC8(ZP->temp01);
        r.nz = INC8(ZP->temp01);
        r = loaded(r, RD8(ZP->temp01));
        r = compared(r, 0x08);
    } while (!r.c);
    r.nz = INC8(OBJ->f3358[69]);
    value = RD8(OBJ->vx_lo[37]);
    sum = add_bytes(value, 0x0C, false);
    WR8(OBJ->vx_lo[37], sum.value);
    value = RD8(OBJ->vx_hi[53]);
    sum = add_bytes(0, value, sum.carry);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(OBJ->vx_hi[53], sum.value);
    r.x = 0x45;
    r.nz = 0x45;
    r = orig_shim(r, 0x89B3, 0x4045);
    r = loaded(r, RD8(OBJ->x_frac[45]));
    if (r.a == 0) {
        return r;
    }
    WR8(OBJ->frame[53], 0);
    RMW8(ZP->field8e, old, old & (uint8_t)~0x04);
    return r;
}

void abi_StageBossBarFlash152D(void)
{
    regs_exit_shim(StageBossBarFlash152D(regs_entry_shim()));
}

/*
 * @routine 152E:1BE StageBarErase152D
 * @run 6B:89BE
 * @unreachable 6B:89BE-6B:89E8 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $89BE runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Blanks a bar's piece in VRAM at the address in X and Y, in 64 words: each word ANDed with a mask of
 * the kernel's random piece at $4051, zRandom2 and zRandom1, and the flash step's row of the table at $8975.
 * HOW: SetVramAddresses152D; a loop down X of reads and writes of the VDC's
 * data port.
 * WAITS: no.
 * Entry: X, Y the VRAM address. Exit: X 0, N and Z from it; A the word's high byte, Y the flash step's row; C and V
 * as $4051 left them. Clobbers zTemp00.
 */
StageRegs StageBarErase152D(StageRegs r)
{
    uint8_t mask, value;

    r = set_vram_addresses(r, 0x89C1);
    for (r.x = 0x40; r.x != 0; r.x--) {
        r.y = (uint8_t)(RD8(OBJ->f3358[69]) & 0x07);
        r = orig_shim(r, 0x89CC, 0x4051);
        mask = (uint8_t)(r.a | RD8(ZP->random2));
        mask = (uint8_t)(mask | RD8(ZP->random1));
        mask = (uint8_t)(mask | rd8((uint16_t)(0x8975 + r.y)));
        WR8(ZP->temp00, mask);
        value = RD8(VDC->data_lo);
        value = (uint8_t)(value & RD8(ZP->temp00));
        WR8(VDC->data_lo, value);
        value = RD8(VDC->data_hi);
        value = (uint8_t)(value & RD8(ZP->temp00));
        r = loaded(r, value);
        WR8(VDC->data_hi, value);
    }
    r.nz = 0;
    return r;
}

void abi_StageBarErase152D(void)
{
    regs_exit_shim(StageBarErase152D(regs_entry_shim()));
}

/* One row of a digit's five: its two bits at SHIFT placed into the VDC's word with the mask the side keeps. */
static uint8_t digit_row(uint8_t y)
{
    return RD8_PTR_Y(ZP->temp00, y);
}

/*
 * @routine 152E:1E9 StageDrawDigit152D
 * @run 6B:89E9
 * @unreachable 6B:89E9-6B:8A68 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $89E9 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Draws the digit in A into the display at the place in X (StageDigitAddress152D): the digit's five rows
 * (five bytes from $8A84 up) laid into the VRAM words' bit planes; an even place takes bits 4 to 7 of the low byte and
 * bits 0 and 1 of the high, an odd place bits 4 to 7 of the high byte, then the next place's low two bits.
 * HOW: The glyph's address in zTemp00-01; StageDigitAddress152D by BSR; loops of five reads and writes of the VDC's
 * data port with shifts through zTemp02-03.
 * WAITS: no.
 * Entry: A the digit, X the place. Exit: Y 5, Z set from the compare, C set; A the last high byte written. Clobbers
 * zTemp00-04.
 */
StageRegs StageDrawDigit152D(StageRegs r)
{
    Sum sum;
    uint8_t digit, value, old;

    WR8(ZP->temp04, r.x);
    digit = r.a;
    WR8(ZP->temp00, digit);
    r.c = (digit & 0x40) != 0;
    value = RD8(ZP->temp00);
    sum = add_bytes((uint8_t)(digit << 2), value, r.c);
    sum = add_bytes(sum.value, 0x84, sum.carry);
    WR8(ZP->temp00, sum.value);
    sum = add_bytes(0, 0x8A, sum.carry);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp01, sum.value);
    CALL(0x89FC, StageDigitAddress152D(r));
    r = loaded(r, RD8(ZP->temp04));
    r.c = (r.a & 0x01) != 0;
    r = loaded(r, (uint8_t)(r.a >> 1));
    if (!r.c) {
        for (r.y = 0; r.y != 5; r.y++) {
            value = digit_row(r.y);
            WR8(ZP->temp03, 0);
            for (int i = 0; i < 2; i++) {
                bool out = (value & 0x80) != 0;

                value = (uint8_t)(value << 1);
                old = RD8(ZP->temp03);
                WR8(ZP->temp03, (uint8_t)((old << 1) | (out ? 1 : 0)));
            }
            WR8(ZP->temp02, value);
            value = (uint8_t)(RD8(VDC->data_lo) & 0x0F);
            value = (uint8_t)(value | RD8(ZP->temp02));
            WR8(VDC->data_lo, value);
            value = (uint8_t)(RD8(VDC->data_hi) & 0xFC);
            value = (uint8_t)(value | RD8(ZP->temp03));
            r = loaded(r, value);
            WR8(VDC->data_hi, value);
        }
        r.c = true;
        r.nz = 0;
        return r;
    }
    for (r.y = 0; r.y != 5; r.y++) {
        value = (uint8_t)(digit_row(r.y) << 2);
        WR8(ZP->temp02, value);
        value = RD8(VDC->data_lo);
        WR8(VDC->data_lo, value);
        value = (uint8_t)(RD8(VDC->data_hi) & 0x0F);
        value = (uint8_t)(value | RD8(ZP->temp02));
        WR8(VDC->data_hi, value);
    }
    r.nz = INC8(ZP->temp04);
    CALL(0x8A48, StageDigitAddress152D(r));
    for (r.y = 0; r.y != 5; r.y++) {
        value = digit_row(r.y);
        WR8(ZP->temp02, 0);
        for (int i = 0; i < 2; i++) {
            bool out = (value & 0x80) != 0;

            value = (uint8_t)(value << 1);
            old = RD8(ZP->temp02);
            WR8(ZP->temp02, (uint8_t)((old << 1) | (out ? 1 : 0)));
        }
        value = (uint8_t)(RD8(VDC->data_lo) & 0xFC);
        value = (uint8_t)(value | RD8(ZP->temp02));
        WR8(VDC->data_lo, value);
        value = RD8(VDC->data_hi);
        r = loaded(r, value);
        WR8(VDC->data_hi, value);
    }
    r.c = true;
    r.nz = 0;
    return r;
}

void abi_StageDrawDigit152D(void)
{
    regs_exit_shim(StageDrawDigit152D(regs_entry_shim()));
}

/*
 * @routine 152E:269 StageDigitAddress152D
 * @run 6B:8A69
 * @unreachable 6B:8A69-6B:8A77 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8A69 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Points the VDC at the display's place zTemp04 (its even pair) from the table of addresses at $8A78.
 * HOW: Two table reads; a jump to SetVramAddresses152D.
 * WAITS: no.
 * Entry: zTemp04 the place. Exit: A 2, N and Z clear; X and Y the address.
 */
StageRegs StageDigitAddress152D(StageRegs r)
{
    uint8_t index = (uint8_t)(RD8(ZP->temp04) & 0xFE);

    r.x = index;
    r.a = rd8((uint16_t)(0x8A78 + index));
    r.y = rd8((uint16_t)(0x8A79 + index));
    r.x = r.a;
    return SetVramAddresses152D(r);
}

void abi_StageDigitAddress152D(void)
{
    regs_exit_shim(StageDigitAddress152D(regs_entry_shim()));
}

/*
 * @routine 152E:2CC StagePausedSelect152D
 * @run 6B:8ACC
 * @unreachable 6B:8ACC-6B:8AF0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8ACC runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The pause screen's choice, reached from StagePausedFrame152D: on the first pad's left or right (bits 4
 * and 6 of BiosJoyTrigger) the sub-weapon at zField93 moves to the next or the one before (the table at $8AC0, two on
 * for right), its frame at $2A9E from the table at $8878, PlayerByte3BD6 1 and zField94 $99.
 * HOW: Pad bits; table reads; stores.
 * WAITS: no.
 * Entry: nothing. Exit, no press: A 0, N and Z from it. Else: A $99, N and Z from it, X the new choice. Y, C and V
 * kept.
 */
StageRegs StagePausedSelect152D(StageRegs r)
{
    uint8_t choice;

    r = loaded(r, (uint8_t)(rd8(BiosJoyTrigger) & 0x50));
    if (r.a == 0) {
        return r;
    }
    r.x = RD8(ZP->field93);
    r = loaded(r, (uint8_t)(r.a & 0x40));
    if (r.a != 0) {
        r.x = (uint8_t)(r.x + 2);
    }
    choice = rd8((uint16_t)(0x8AC0 + r.x));
    WR8(ZP->field93, choice);
    r.x = choice;
    r = loaded(r, rd8((uint16_t)(0x8878 + choice)));
    WR8(OBJ->frame[54], r.a);
    r = loaded(r, 1);
    wr8(PlayerByte3BD6, 1);
    r = loaded(r, 0x99);
    WR8(ZP->field94, 0x99);
    return r;
}

void abi_StagePausedSelect152D(void)
{
    regs_exit_shim(StagePausedSelect152D(regs_entry_shim()));
}

/* The pause's menu step: the kernel's piece at $4018 twice (X 2 and 3, A from the row's first two bytes at $8B70), the
 * row's frame into $2A9F, then the kernel's piece at $400F with X 4 for the row's sound unless it is the one zPaused's
 * bit 2 (after its shift into zTemp01) already has. */
static Regs pause_menu_row(Regs r, uint8_t row)
{
    WR8(ZP->temp00, row);
    r = loaded(r, rd8((uint16_t)(0x8B70 + row)));
    r.x = 2;
    r.nz = 2;
    r = orig_shim(r, 0x8B2A, 0x4018);
    r.x = RD8(ZP->temp00);
    r = loaded(r, rd8((uint16_t)(0x8B71 + r.x)));
    r.x = 3;
    r.nz = 3;
    r = orig_shim(r, 0x8B34, 0x4018);
    r.x = RD8(ZP->temp00);
    r = loaded(r, rd8((uint16_t)(0x8B73 + r.x)));
    WR8(OBJ->frame[55], r.a);
    r = loaded(r, RD8(ZP->paused));
    WR8(ZP->temp01, r.a);
    r.x = RD8(ZP->temp00);
    r = loaded(r, rd8((uint16_t)(0x8B72 + r.x)));
    if (r.a == 0) {
        return r;
    }
    if (LSR8(ZP->temp01)) {
        r = compared(r, 0x65);
    } else {
        r = compared(r, 0x64);
    }
    if (r.nz == 0) {
        return r;
    }
    r.x = 4;
    r.nz = 4;
    return orig_tail_shim(r, 0x400F);
}

/*
 * @routine 152E:2F1 StagePausedFrame152D
 * @run 6B:8AF1
 * @unreachable 6B:8AF1-6B:8B6F no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8AF1 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The paused frame. zPaused's bit 1 clear: the choice (StagePausedSelect152D). Else zPaused counts up by
 * 4 a frame (its bits 2 up the menu's step, bit 0 the side it scrolls from) through the menu's rows (the table of four
 * bytes a row at $8B70, from $19 down or up to $1A); past them only bit 0 is kept. Steps $25 and $27 at the end of the
 * scroll redraw the score, or the stock and the points to add.
 * HOW: Shifts of zPaused through zTemp00; a subtract; the row's step through the kernel (pause_menu_row); jumps to
 * StageDrawScore152D or StageDrawScoreAdd152D, a call of StageDrawStock152D.
 * WAITS: no.
 * Entry: nothing. Exit: as the step that ran left them.
 */
StageRegs StagePausedFrame152D(StageRegs r)
{
    Sum sum;
    uint8_t paused, old, step;
    bool carry, down;

    paused = RD8(ZP->paused);
    carry = (paused & 0x01) != 0;
    old = RD8(ZP->temp00);
    WR8(ZP->temp00, (uint8_t)((carry ? 0x80 : 0) | (old >> 1)));
    step = (uint8_t)(paused >> 2);
    r = loaded(r, step);
    r.c = (paused & 0x02) != 0;
    if (!r.c) {
        return StagePausedSelect152D(r);
    }
    old = RD8(ZP->temp00);
    WR8(ZP->temp00, (uint8_t)((old << 1) | 1));
    down = (old & 0x80) != 0;
    r.c = down;
    if (down) {
        r = compared(r, 0x1A);
        if (r.c) {
            r = loaded(r, (uint8_t)(RD8(ZP->paused) & 0x01));
            WR8(ZP->paused, r.a);
            return r;
        }
    } else {
        WR8(ZP->temp00, r.a);
        sum = sub_bytes(0x19, RD8(ZP->temp00), true);
        r = loaded(r, sum.value);
        r.c = sum.carry;
        r.v = sum.overflow;
        if (!r.c) {
            r = loaded(r, (uint8_t)(RD8(ZP->paused) & 0x01));
            WR8(ZP->paused, r.a);
            return r;
        }
    }
    r.c = (r.a & 0x01) != 0;
    r = loaded(r, (uint8_t)(r.a >> 1));
    php_shim(r);
    step = (uint8_t)(r.a << 2);
    r.c = (r.a & 0x40) != 0;
    r = loaded(r, step);
    r.x = step;
    sum = add_bytes(RD8(ZP->paused), 0x04, false);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->paused, sum.value);
    r = plp_shim(r);
    if (!r.c) {
        return pause_menu_row(r, r.x);
    }
    r = loaded(r, RD8(ZP->paused));
    r.c = (r.a & 0x01) != 0;
    r = loaded(r, (uint8_t)(r.a >> 1));
    if (r.c) {
        r = compared(r, 0x25);
        if (r.nz != 0) {
            return r;
        }
        return StageDrawScore152D(r);
    }
    r = compared(r, 0x27);
    if (r.nz != 0) {
        return r;
    }
    CALL(0x8B6D, StageDrawStock152D(r));
    return StageDrawScoreAdd152D(r);
}

void abi_StagePausedFrame152D(void)
{
    regs_exit_shim(StagePausedFrame152D(regs_entry_shim()));
}

/*
 * @routine 152E:3A4 StageOpeningStart152D
 * @run 6B:8BA4
 * @unreachable 6B:8BA4-6B:8BD4 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8BA4 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Starts the stage's opening when zField8E's bit 0 is clear: the display's pieces 3 and 4
 * (StageDrawPiece152D), the opening's objects placed (pages at $2B74 and $2C4D one less, speeds at $2D24 and $2DB5 4,
 * counts at $3354 and $3355 $20), PlayerByte3BD8 2, sound $3B, the CD's wait at $33E4 cleared.
 * HOW: A bit test; calls with X 3 and 4; stores; the sound hook.
 * WAITS: no.
 * Entry: nothing. Exit, bit 0 set: everything kept. Else: as the sound hook left them.
 */
StageRegs StageOpeningStart152D(StageRegs r)
{
    if ((RD8(ZP->field8e) & 0x01) != 0) {
        return r;
    }
    r.x = 3;
    r.nz = 3;
    CALL(0x8BAD, StageDrawPiece152D(r));
    r.x++;
    r.nz = r.x;
    CALL(0x8BB1, StageDrawPiece152D(r));
    r.nz = DEC8(OBJ->x_hi[4]);
    r.nz = DEC8(OBJ->x_frac[45]);
    r = loaded(r, 0x04);
    WR8(OBJ->y_lo[84], 0x04);
    WR8(OBJ->vx_hi[53], 0x04);
    r = loaded(r, 0x20);
    WR8(OBJ->f3300[84], 0x20);
    WR8(OBJ->f3300[85], 0x20);
    r = loaded(r, 0x02);
    wr8(PlayerByte3BD8, 0x02);
    r = loaded(r, 0x3B);
    r = orig_shim(r, 0x8BD1, SOUND_HOOK);
    WR8(OBJ->hit_taken[52], 0);
    return r;
}

void abi_StageOpeningStart152D(void)
{
    regs_exit_shim(StageOpeningStart152D(regs_entry_shim()));
}

/* A count of the opening's motion run out: the next of its five steps from the tables at $8C86 (the speed, halved into
 * HIGH and its low bit into the top of LOW) and $8C8C (the count). */
static Regs opening_next_step(Regs r, uint16_t high, uint16_t low, uint16_t count)
{
    uint8_t value = rd8((uint16_t)(0x8C86 + r.y));

    wr8(high, (uint8_t)(value >> 1));
    wr8(low, (uint8_t)((value & 0x01) != 0 ? 0x80 : 0x00));
    r = loaded(r, rd8((uint16_t)(0x8C8C + r.y)));
    r.c = false;
    wr8(count, r.a);
    return r;
}

/* A 24-bit position moved by a 16-bit speed: the low and middle bytes added with the carry, the carry into the top. */
static Regs move_by_speed(Regs r, uint16_t speed_lo, uint16_t pos_lo, uint16_t speed_hi, uint16_t pos_mid,
                          uint16_t pos_hi)
{
    Sum sum;
    uint8_t a, b;

    a = rd8(speed_lo);
    b = rd8(pos_lo);
    sum = add_bytes(a, b, false);
    wr8(pos_lo, sum.value);
    a = rd8(speed_hi);
    b = rd8(pos_mid);
    sum = add_bytes(a, b, sum.carry);
    wr8(pos_mid, sum.value);
    b = rd8(pos_hi);
    sum = add_bytes(0, b, sum.carry);
    wr8(pos_hi, sum.value);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    return r;
}

/*
 * @routine 152E:3D5 StageOpeningStep152D
 * @run 6B:8BD5
 * @stop 8C87
 * @unreachable 6B:8BD5-6B:8C86 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8BD5 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One frame of the stage's opening: once the ADPCM is idle the kernel's piece at $407B runs once ($33E4
 * set). Two objects move in steps: each step's count ($3355, $3354) runs down, then the step's number ($330D, $330C)
 * counts on and its speed and count come from the tables at $8C86 and $8C8C; at the first's step 7 the opening ends
 * (the boss's frame at $2A9D cleared, zField8E's bit 0 set, $407B if not yet run). At the first's step 6 the count is
 * kept at $339C ($2A9C cleared at $1F); at the second's step 6 the screen's row fade runs with it
 * (StageOpeningFade152D).
 * HOW: ad_stat; decrements and table reads; 24-bit adds of the speeds into the positions.
 * WAITS: no.
 * Entry: nothing. Exit: as the last piece that ran left them.
 */
StageRegs StageOpeningStep152D(StageRegs r)
{
    AdStat stat;

    r = loaded(r, RD8(OBJ->hit_taken[52]));
    if (r.a == 0) {
        jsr_push(0x8BDD);
        stat = vec_ad_stat();
        jsr_pop();
        r.x = stat.playing;
        r = loaded(r, stat.playing);
        if (r.a == 0) {
            r = orig_shim(r, 0x8BE3, KERNEL_407B);
            r.nz = INC8(OBJ->hit_taken[52]);
        }
    }
    r.nz = DEC8(OBJ->f3300[85]);
    if (r.nz == 0) {
        r.nz = INC8(OBJ->f3300[13]);
        r.y = RD8(OBJ->f3300[13]);
        r.c = r.y >= 0x07;
        r.nz = (uint8_t)(r.y - 0x07);
        if (r.nz == 0) {
            WR8(OBJ->frame[53], 0);
            RMW8(ZP->field8e, old, old | 0x01);
            r = loaded(r, RD8(OBJ->hit_taken[52]));
            if (r.a != 0) {
                return r;
            }
            return orig_shim(r, 0x8C02, KERNEL_407B);
        }
        r = opening_next_step(r, ADDR_OF8(OBJ->vx_hi[53]), ADDR_OF8(OBJ->vx_lo[37]), ADDR_OF8(OBJ->f3300[85]));
    }
    r = loaded(r, RD8(OBJ->f3300[13]));
    r = compared(r, 0x06);
    if (r.nz == 0) {
        r = loaded(r, RD8(OBJ->f3300[85]));
        r = compared(r, 0x1F);
        if (r.nz == 0) {
            WR8(OBJ->frame[52], 0);
        }
        WR8(OBJ->f3358[68], r.a);
    }
    r = move_by_speed(r, ADDR_OF8(OBJ->vx_lo[37]), ADDR_OF8(OBJ->y_lo[13]), ADDR_OF8(OBJ->vx_hi[53]),
                      ADDR_OF8(OBJ->y_hi[29]), ADDR_OF8(OBJ->x_frac[45]));
    r.nz = DEC8(OBJ->f3300[84]);
    if (r.nz == 0) {
        r.nz = INC8(OBJ->f3300[12]);
        r.y = RD8(OBJ->f3300[12]);
        r = opening_next_step(r, ADDR_OF8(OBJ->y_lo[84]), ADDR_OF8(OBJ->y_frac[68]), ADDR_OF8(OBJ->f3300[84]));
    }
    r = move_by_speed(r, ADDR_OF8(OBJ->y_frac[68]), ADDR_OF8(OBJ->x_lo[60]), ADDR_OF8(OBJ->y_lo[84]),
                      ADDR_OF8(OBJ->x_hi[76]), ADDR_OF8(OBJ->x_hi[4]));
    r = loaded(r, RD8(OBJ->f3300[12]));
    r = compared(r, 0x06);
    if (r.nz != 0) {
        return r;
    }
    r = loaded(r, RD8(OBJ->f3358[68]));
    return StageOpeningFade152D(r);
}

void abi_StageOpeningStep152D(void)
{
    regs_exit_shim(StageOpeningStep152D(regs_entry_shim()));
}

/*
 * @routine 152E:4A3 StageOpeningFade152D
 * @run 6B:8CA3
 * @unreachable 6B:8CA3-6B:8CEE no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8CA3 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The opening's fade of the screen's rows, with A the count kept at $339C: from it the rows' fade level
 * (zMultiplierHi, $10 less the low nibble) and the group of rows (the table of four bytes a group at $8C93: the first
 * row's VRAM address, the count of rows, the step between them); each row is faded by StageOpeningCopyRow152D after
 * the kernel's piece at $4051.
 * HOW: Masks, a subtract and table reads into zTemp10-12 and zProduct0; a loop of rows with a 16-bit add.
 * WAITS: no.
 * Entry: A the count. Exit: N and Z from the last row count's decrement (Z set); A the address's high byte, C and V
 * from its add; X and Y as StageOpeningCopyRow152D left them.
 */
StageRegs StageOpeningFade152D(StageRegs r)
{
    Sum sum;
    uint8_t count, level, value;

    WR8(ZP->multiplier.hi, r.a);
    r = compared(r, 0x20);
    if (r.c) {
        r = loaded(r, (uint8_t)(r.a & 0x1F));
        r.c = (r.a & 0x01) != 0;
        r = loaded(r, (uint8_t)(r.a >> 1));
        if (r.c) {
            sum = add_bytes(r.a, 0x10, false);
            r = loaded(r, sum.value);
            r.c = sum.carry;
            r.v = sum.overflow;
        }
        sum = add_bytes(r.a, 0x20, r.c);
        r = loaded(r, sum.value);
        r.c = sum.carry;
        r.v = sum.overflow;
    }
    count = r.a;
    push8(count);
    r = loaded(r, (uint8_t)(count & 0x0F));
    WR8(ZP->temp10, r.a);
    level = RD8(ZP->temp10);
    sum = sub_bytes(0x10, level, true);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->multiplier.hi, sum.value);
    count = pull8();
    r.x = (uint8_t)((count >> 2) & 0x0C);
    r.c = (count & 0x02) != 0;
    r = loaded(r, rd8((uint16_t)(0x8C93 + r.x)));
    WR8(ZP->temp10, r.a);
    r = loaded(r, rd8((uint16_t)(0x8C94 + r.x)));
    WR8(ZP->temp11, r.a);
    r = loaded(r, rd8((uint16_t)(0x8C95 + r.x)));
    WR8(ZP->temp12, r.a);
    r = loaded(r, rd8((uint16_t)(0x8C96 + r.x)));
    WR8(ZP->product.lo, r.a);
    do {
        r = orig_shim(r, 0x8CDC, 0x4051);
        CALL(0x8CDE, StageOpeningCopyRow152D(r));
        value = RD8(ZP->temp10);
        sum = add_bytes(value, RD8(ZP->product.lo), false);
        WR8(ZP->temp10, sum.value);
        value = RD8(ZP->temp11);
        sum = add_bytes(0, value, sum.carry);
        r = loaded(r, sum.value);
        r.c = sum.carry;
        r.v = sum.overflow;
        WR8(ZP->temp11, sum.value);
        r.nz = DEC8(ZP->temp12);
    } while (r.nz != 0);
    return r;
}

void abi_StageOpeningFade152D(void)
{
    regs_exit_shim(StageOpeningFade152D(regs_entry_shim()));
}

/* One pass of the row's fade: each of the row's words in the buffer at $3980 ANDed with the mask, from the word itself
 * or from the next word (NEXT 2), the four planes a pass (offsets 0, $20, $40 and $60). */
static void fade_row_pass(uint8_t x, uint8_t next)
{
    static const uint8_t planes[4] = { 0x00, 0x20, 0x40, 0x60 };
    uint8_t value;

    for (int i = 0; i < 4; i++) {
        value = rd8((uint16_t)(ROW_BUFFER + 2 + planes[i] + next + x));
        value = (uint8_t)(value & RD8(ZP->temp00));
        wr8((uint16_t)(ROW_BUFFER + 2 + planes[i] + x), value);
        value = rd8((uint16_t)(ROW_BUFFER + 3 + planes[i] + next + x));
        value = (uint8_t)(value & RD8(ZP->temp00));
        wr8((uint16_t)(ROW_BUFFER + 3 + planes[i] + x), value);
    }
}

/*
 * @routine 152E:4EF StageOpeningCopyRow152D
 * @run 6B:8CEF
 * @unreachable 6B:8CEF-6B:8DB7 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8CEF runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Fades one row of the screen in VRAM: the row's 64 words (at the address in zTemp10-11) read into the
 * buffer at $397E; at a level of $10 or more cleared; else, for each level step below the last, every word ANDed with
 * the next word under a random mask (zRandom2 rotated with bit 7 set, ORed with itself), then every word ANDed with
 * zRandom2 OR zRandom3; then runs on into StageOpeningWriteRow152D, which writes the buffer back.
 * HOW: SetVramAddresses152D; loops of the VDC's data port and of the buffer.
 * WAITS: no.
 * Entry: zTemp10-11 the row's address, zMultiplierHi the level. Exit: as StageOpeningWriteRow152D. Clobbers zTemp00.
 */
StageRegs StageOpeningCopyRow152D(StageRegs r)
{
    uint8_t value, mask;

    r.x = RD8(ZP->temp10);
    r.y = RD8(ZP->temp11);
    r = set_vram_addresses(r, 0x8CF6);
    for (r.x = 0x80; r.x != 0; r.x = (uint8_t)(r.x - 2)) {
        value = RD8(VDC->data_lo);
        wr8((uint16_t)(ROW_BUFFER + 1 + r.x), value);
        value = RD8(VDC->data_hi);
        r = loaded(r, value);
        wr8((uint16_t)(ROW_BUFFER + r.x), value);
    }
    r.y = RD8(ZP->multiplier.hi);
    r.c = r.y >= 0x10;
    if (r.c) {
        for (r.x = 0x7F; (r.x & 0x80) == 0; r.x--) {
            wr8((uint16_t)(ROW_BUFFER + 2 + r.x), 0);
        }
        r.nz = r.x;
        return StageOpeningWriteRow152D(r);
    }
    r.y--;
    if (r.y != 0) {
        r.x = 0;
        do {
            value = RD8(ZP->random2);
            mask = (uint8_t)(0x80 | (value >> 1));
            r.c = (value & 0x01) != 0;
            mask = (uint8_t)(mask | RD8(ZP->random2));
            WR8(ZP->temp00, mask);
            fade_row_pass(r.x, 2);
            r.x = (uint8_t)(r.x + 2);
            r.y--;
        } while (r.y != 0);
    }
    r.y = RD8(ZP->multiplier.hi);
    r.x = 0;
    do {
        mask = RD8(ZP->random2);
        mask = (uint8_t)(mask | RD8(ZP->random3));
        WR8(ZP->temp00, mask);
        fade_row_pass(r.x, 0);
        r.x = (uint8_t)(r.x + 2);
        r.y--;
    } while (r.y != 0);
    return StageOpeningWriteRow152D(r);
}

void abi_StageOpeningCopyRow152D(void)
{
    regs_exit_shim(StageOpeningCopyRow152D(regs_entry_shim()));
}

/*
 * @routine 152E:5B8 StageOpeningWriteRow152D
 * @run 6B:8DB8
 * @unreachable 6B:8DB8-6B:8DCA no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8DB8 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Writes the row's 64 words from the buffer at $397E back through the VDC's data port.
 * HOW: A loop down X by two.
 * WAITS: no.
 * Entry: the VDC pointed at the row. Exit: X 0, Z set; A the last high byte written. Y, C and V kept.
 */
StageRegs StageOpeningWriteRow152D(StageRegs r)
{
    uint8_t value;

    for (r.x = 0x80; r.x != 0; r.x = (uint8_t)(r.x - 2)) {
        value = rd8((uint16_t)(ROW_BUFFER + 1 + r.x));
        WR8(VDC->data_lo, value);
        value = rd8((uint16_t)(ROW_BUFFER + r.x));
        r.a = value;
        WR8(VDC->data_hi, value);
    }
    r.nz = 0;
    return r;
}

void abi_StageOpeningWriteRow152D(void)
{
    regs_exit_shim(StageOpeningWriteRow152D(regs_entry_shim()));
}

/*
 * @routine 152E:5CB StageDrawStock152D
 * @run 6B:8DCB
 * @unreachable 6B:8DCB-6B:8DD3 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8DCB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Draws the stock zStock (two digits at places 0 and 1).
 * HOW: StageDrawNumber152D with X 0, X kept on the stack across it.
 * WAITS: no.
 * Entry: nothing. Exit: X kept, N and Z from it; A, Y, C and V as StageDrawNumber152D left them.
 */
StageRegs StageDrawStock152D(StageRegs r)
{
    push8(r.x);
    r = loaded(r, RD8(ZP->stock));
    r.x = 0;
    CALL(0x8DD2, StageDrawNumber152D(r));
    r.x = pull8();
    r.nz = r.x;
    return r;
}

void abi_StageDrawStock152D(void)
{
    regs_exit_shim(StageDrawStock152D(regs_entry_shim()));
}

/*
 * @routine 152E:5D4 StageDrawScoreAdd152D
 * @run 6B:8DD4
 * @unreachable 6B:8DD4-6B:8DE4 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8DD4 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Draws the points to add, zScoreAddLo and zScoreAddHi (four digits from place 3).
 * HOW: StageDrawNumber152D with X 3 and 5, X kept on the stack across them.
 * WAITS: no.
 * Entry: nothing. Exit: X kept, N and Z from it; A, Y, C and V as StageDrawNumber152D left them.
 */
StageRegs StageDrawScoreAdd152D(StageRegs r)
{
    push8(r.x);
    r = loaded(r, RD8(ZP->score_add.lo));
    r.x = 3;
    r.nz = 3;
    CALL(0x8DDC, StageDrawNumber152D(r));
    r = loaded(r, RD8(ZP->score_add.hi));
    r.x = 5;
    r.nz = 5;
    CALL(0x8DE3, StageDrawNumber152D(r));
    r.x = pull8();
    r.nz = r.x;
    return r;
}

void abi_StageDrawScoreAdd152D(void)
{
    regs_exit_shim(StageDrawScoreAdd152D(regs_entry_shim()));
}

/*
 * @routine 152E:5E5 StageDrawScore152D
 * @run 6B:8DE5
 * @unreachable 6B:8DE5-6B:8DFC no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8DE5 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Draws the score zScore0-2 (six digits from place 1).
 * HOW: StageDrawNumber152D with X 1, 3 and 5, X kept on the stack across them.
 * WAITS: no.
 * Entry: nothing. Exit: X kept, N and Z from it; A, Y, C and V as StageDrawNumber152D left them.
 */
StageRegs StageDrawScore152D(StageRegs r)
{
    push8(r.x);
    r = loaded(r, RD8(ZP->score.lo));
    r.x = 1;
    r.nz = 1;
    CALL(0x8DED, StageDrawNumber152D(r));
    r = loaded(r, RD8(ZP->score.mid));
    r.x = 3;
    r.nz = 3;
    CALL(0x8DF4, StageDrawNumber152D(r));
    r = loaded(r, RD8(ZP->score.hi));
    r.x = 5;
    r.nz = 5;
    CALL(0x8DFB, StageDrawNumber152D(r));
    r.x = pull8();
    r.nz = r.x;
    return r;
}

void abi_StageDrawScore152D(void)
{
    regs_exit_shim(StageDrawScore152D(regs_entry_shim()));
}

/*
 * @routine 152E:5FD StageDrawNumber152D
 * @run 6B:8DFD
 * @unreachable 6B:8DFD-6B:8E0D no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8DFD runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Draws the two decimal digits of A at the places X and X + 1, the low digit first.
 * HOW: StageDrawDigit152D with the low nibble, X and A kept on the stack; then a jump to it with the high nibble.
 * WAITS: no.
 * Entry: A the number, X the place. Exit: as StageDrawDigit152D.
 */
StageRegs StageDrawNumber152D(StageRegs r)
{
    uint8_t number;

    push8(r.x);
    push8(r.a);
    r = loaded(r, (uint8_t)(r.a & 0x0F));
    CALL(0x8E04, StageDrawDigit152D(r));
    number = pull8();
    r.c = (number & 0x08) != 0;
    r = loaded(r, (uint8_t)(number >> 4));
    r.x = pull8();
    r.x++;
    r.nz = r.x;
    return StageDrawDigit152D(r);
}

void abi_StageDrawNumber152D(void)
{
    regs_exit_shim(StageDrawNumber152D(regs_entry_shim()));
}

/* The list's pointer in zTemp00-01 moved on by A bytes. */
static Regs list_advance(Regs r)
{
    Sum sum;
    uint8_t value;

    value = RD8(ZP->temp00);
    sum = add_bytes(r.a, value, false);
    WR8(ZP->temp00, sum.value);
    value = RD8(ZP->temp01);
    sum = add_bytes(0, value, sum.carry);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp01, sum.value);
    return r;
}

/*
 * @routine 152E:60E StageUnpackList152D
 * @run 6B:8E0E
 * @unreachable 6B:8E0E-6B:8E71 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8E0E runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Unpacks the route's list (the table at $9A62 by zField82) into memory: SaveBuffer's 767 bytes cleared
 * from KernelZeros and its first 8 taken from the list; then records: 0 sets the target address (the next two bytes),
 * $FF ends, any other count fills that many bytes from the target with the next byte and moves the target on.
 * HOW: TAI; the list in zTemp00-01, the target in zTemp02-03, the count in zTemp04; loops of reads and writes
 * through the pointers.
 * WAITS: no.
 * Entry: nothing. Exit: A $FF, Z set, C set; X 0 or as it was, Y as the last record left it. Clobbers zTemp00-04.
 */
StageRegs StageUnpackList152D(StageRegs r)
{
    uint8_t route, count, value;

    tai_shim(r, KernelZeros, SaveBuffer, 0x02FF);
    route = RD8(ZP->field82);
    r.c = (route & 0x80) != 0;
    r = loaded(r, (uint8_t)(route << 1));
    r.y = r.a;
    r = loaded(r, rd8((uint16_t)(0x9A62 + r.y)));
    WR8(ZP->temp00, r.a);
    r = loaded(r, rd8((uint16_t)(0x9A63 + r.y)));
    WR8(ZP->temp01, r.a);
    for (r.y = 0; r.y != 8; r.y++) {
        value = RD8_PTR_Y(ZP->temp00, r.y);
        wr8((uint16_t)(SaveBuffer + r.y), value);
    }
    r = loaded(r, 8);
    for (;;) {
        r = list_advance(r);
        r = loaded(r, RD8_PTR(ZP->temp00));
        if (r.a == 0) {
            r.y = 1;
            r = loaded(r, RD8_PTR_Y(ZP->temp00, r.y));
            WR8(ZP->temp02, r.a);
            r.y++;
            r = loaded(r, RD8_PTR_Y(ZP->temp00, r.y));
            WR8(ZP->temp03, r.a);
            r = loaded(r, 3);
            continue;
        }
        r = compared(r, 0xFF);
        if (r.nz == 0) {
            return r;
        }
        WR8(ZP->temp04, r.a);
        count = r.a;
        r.x = count;
        r.y = 1;
        r = loaded(r, RD8_PTR_Y(ZP->temp00, r.y));
        for (r.y = 0; r.x != 0; r.x--) {
            WR8_PTR_Y(ZP->temp02, r.y, r.a);
            r.y++;
        }
        r = loaded(r, RD8(ZP->temp04));
        value = RD8(ZP->temp02);
        {
            Sum sum = add_bytes(r.a, value, false);

            WR8(ZP->temp02, sum.value);
            value = RD8(ZP->temp03);
            sum = add_bytes(0, value, sum.carry);
            WR8(ZP->temp03, sum.value);
        }
        r = loaded(r, 2);
    }
}

void abi_StageUnpackList152D(void)
{
    regs_exit_shim(StageUnpackList152D(regs_entry_shim()));
}

/*
 * @routine 152E:672 ClearBitRowsFour152D
 * @run 6B:8E72
 * @unreachable 6B:8E72-6B:8E77 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8E72 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Runs StageClearBitRows152D four times: three calls and the run into it.
 * HOW: Three BSR and the bytes of StageClearBitRows152D after them.
 * WAITS: no.
 * Entry, exit: as StageClearBitRows152D.
 */
StageRegs ClearBitRowsFour152D(StageRegs r)
{
    CALL(0x8E74, StageClearBitRows152D(r));
    CALL(0x8E76, StageClearBitRows152D(r));
    CALL(0x8E78, StageClearBitRows152D(r));
    return StageClearBitRows152D(r);
}

void abi_ClearBitRowsFour152D(void)
{
    regs_exit_shim(ClearBitRowsFour152D(regs_entry_shim()));
}

/*
 * @routine 152E:678 StageClearBitRows152D
 * @run 6B:8E78
 * @unreachable 6B:8E78-6B:8ED1 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8E78 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Clears one bit of four VRAM words $10 apart, for the map entry zTemp0F names: its low nibble picks the
 * bit (the masks at $8ED2 and $8EDA), its high nibble is added to the base in zTemp08-09; then zTemp0F moves on by
 * $23. Each word is pointed at by the kernel's pieces at $4024, $4021 and $4027 and ANDed in place.
 * HOW: The address in zTemp0A-0B, the count in zTemp0E; the VDC's data port.
 * WAITS: no.
 * Entry: zTemp08-09 the base, zTemp0F the entry. Exit: A the new zTemp0F, N, Z, C and V from its add; X as $4027
 * left it; Y the bit.
 */
StageRegs StageClearBitRows152D(StageRegs r)
{
    Sum sum;
    uint8_t entry, value;

    r.y = (uint8_t)(RD8(ZP->temp0f) & 0x0F);
    entry = RD8(ZP->temp0f);
    value = RD8(ZP->temp08);
    sum = add_bytes((uint8_t)(entry >> 4), value, false);
    WR8(ZP->temp0a, sum.value);
    WR8(ZP->temp0b, RD8(ZP->temp09));
    WR8(ZP->temp0e, 0x04);
    r = loaded(r, 0x04);
    r.c = sum.carry;
    r.v = sum.overflow;
    do {
        r = orig_shim(r, 0x8E93, 0x4024);
        WR8(VDC->data_lo, RD8(ZP->temp0a));
        r = loaded(r, RD8(ZP->temp0b));
        WR8(VDC->data_hi, r.a);
        r = orig_shim(r, 0x8EA0, 0x4021);
        WR8(VDC->data_lo, RD8(ZP->temp0a));
        r = loaded(r, RD8(ZP->temp0b));
        WR8(VDC->data_hi, r.a);
        r = orig_shim(r, 0x8EAD, 0x4027);
        value = RD8(VDC->data_lo);
        value = (uint8_t)(value & rd8((uint16_t)(0x8ED2 + r.y)));
        WR8(VDC->data_lo, value);
        value = RD8(VDC->data_hi);
        value = (uint8_t)(value & rd8((uint16_t)(0x8EDA + r.y)));
        WR8(VDC->data_hi, value);
        value = RD8(ZP->temp0a);
        sum = add_bytes(value, 0x10, false);
        r = loaded(r, sum.value);
        r.c = sum.carry;
        r.v = sum.overflow;
        WR8(ZP->temp0a, sum.value);
        r.nz = DEC8(ZP->temp0e);
    } while (r.nz != 0);
    value = RD8(ZP->temp0f);
    sum = add_bytes(value, 0x23, false);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp0f, sum.value);
    return r;
}

void abi_StageClearBitRows152D(void)
{
    regs_exit_shim(StageClearBitRows152D(regs_entry_shim()));
}

/*
 * @routine 152E:6EA ContinueStep152D
 * @run 6B:8EEA
 * @stop 8EF1
 * @unreachable 6B:8EEA-6B:8EF0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8EEA runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The continue screen's frame: the step zGameSubMode holds, from the four at $8EF1 (the start, the
 * reveal, the menu's start, the menu).
 * HOW: zGameSubMode doubled indexes the table, whose word is jumped through.
 * WAITS: as the step.
 * Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice zGameSubMode, N and Z from it, C its
 * bit 7.
 */
StageRegs ContinueStep152D(StageRegs r)
{
    uint8_t mode = RD8(ZP->game_sub_mode);
    uint8_t twice = (uint8_t)(mode << 1);
    uint16_t handler;

    r.c = (mode & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(0x8EF1, twice);
    switch (handler) {
    case 0x8EF9:
        return ContinueStart152D(r);
    case 0x8F5B:
        return ContinueReveal152D(r);
    case 0x9042:
        return ContinueMenuStart152D(r);
    case 0x908B:
        return ContinueMenu152D(r);
    default:
        return orig_tail_shim(r, handler);
    }
}

void abi_ContinueStep152D(void)
{
    regs_exit_shim(ContinueStep152D(regs_entry_shim()));
}

/* A row's VRAM address from $6E00: the row (in A) times $40, into zTemp00-01. */
static Regs continue_row_address(Regs r, uint8_t row)
{
    Sum sum;
    uint8_t old, value;

    WR8(ZP->temp00, 0);
    r.c = (row & 0x01) != 0;
    old = RD8(ZP->temp00);
    WR8(ZP->temp00, (uint8_t)((r.c ? 0x80 : 0) | (old >> 1)));
    r.c = (row & 0x02) != 0;
    old = RD8(ZP->temp00);
    WR8(ZP->temp00, (uint8_t)((r.c ? 0x80 : 0) | (old >> 1)));
    value = (uint8_t)(row >> 2);
    r = loaded(r, value);
    WR8(ZP->temp01, value);
    return r;
}

/*
 * @routine 152E:6F9 ContinueStart152D
 * @run 6B:8EF9
 * @unreachable 6B:8EF9-6B:8F5A no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8EF9 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The continue screen's start: sound $36, zRasterStage cleared; for each of the eight rows (7 down to 0)
 * its 64 VRAM words at $6E00 + row x $40 cleared, the kernel's piece at $4054 with X the row, and the row's object
 * placed (its count at $32C8 twice the row, sprite $0D up at $2A58, set 5, X $48, $2C50 $59); then the kernel's piece
 * at $4012 with A $0D, and the next step.
 * HOW: The row in zTemp10; the address through zTemp00-01 and the kernel's pieces at $4021 and $4027; ST1 and ST2
 * in a loop.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from zGameSubMode's increment; A, X, Y, C and V as $4012 left them.
 */
StageRegs ContinueStart152D(StageRegs r)
{
    Sum sum;
    uint8_t row, value;

    r = loaded(r, 0x36);
    r = orig_shim(r, 0x8EFE, SOUND_HOOK);
    WR8(ZP->raster_stage, 0);
    r = loaded(r, 0x07);
    WR8(ZP->temp10, 0x07);
    do {
        row = RD8(ZP->temp10);
        r = continue_row_address(r, row);
        r = orig_shim(r, 0x8F13, 0x4021);
        value = RD8(ZP->temp00);
        sum = add_bytes(0x00, value, false);
        WR8(VDC->data_lo, sum.value);
        value = RD8(ZP->temp01);
        sum = add_bytes(0x6E, value, sum.carry);
        r = loaded(r, sum.value);
        r.c = sum.carry;
        r.v = sum.overflow;
        WR8(VDC->data_hi, sum.value);
        r = orig_shim(r, 0x8F25, 0x4027);
        for (r.y = 0; r.y != 0x40; r.y++) {
            st1(0x00);
            st2(0x00);
        }
        r.c = true;
        r.x = RD8(ZP->temp10);
        r = orig_shim(r, 0x8F34, 0x4054);
        WR8(OBJ->f32a8[32 + r.x], (uint8_t)(r.x << 1));
        sum = add_bytes(r.x, 0x0D, false);
        WR8(OBJ->sprite_set[72 + r.x], sum.value);
        WR8(OBJ->sprite_set[r.x], 0x05);
        WR8(OBJ->x_hi[8 + r.x], 0x48);
        r = loaded(r, 0x59);
        r.c = sum.carry;
        r.v = sum.overflow;
        WR8(OBJ->x_frac[48 + r.x], 0x59);
        r.nz = DEC8(ZP->temp10);
    } while ((r.nz & 0x80) == 0);
    r = loaded(r, 0x0D);
    r = orig_shim(r, 0x8F58, 0x4012);
    r.nz = INC8(ZP->game_sub_mode);
    return r;
}

void abi_ContinueStart152D(void)
{
    regs_exit_shim(ContinueStart152D(regs_entry_shim()));
}

/*
 * @routine 152E:75B ContinueReveal152D
 * @run 6B:8F5B
 * @unreachable 6B:8F5B-6B:8F7D no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8F5B runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The continue screen's reveal: Run pressed once the last row's count at $32CF is $11 ends it; else each
 * row from 7 down steps (ContinueRowStep152D), and the last row's count is moved from $10 to $11.
 * HOW: A pad bit and a compare; BSR in a loop down X.
 * WAITS: no.
 * Entry: nothing. Exit: as the last piece that ran left them, X $FF after the rows; A the count, N, Z and C from its
 * compare.
 */
StageRegs ContinueReveal152D(StageRegs r)
{
    r = loaded(r, (uint8_t)(rd8(BiosJoyTrigger) & 0x08));
    if (r.a != 0) {
        r = loaded(r, RD8(OBJ->f32a8[39]));
        r = compared(r, 0x11);
        if (r.nz == 0) {
            r.nz = INC8(ZP->game_sub_mode);
            return r;
        }
    }
    r.x = 7;
    do {
        CALL(0x8F70, ContinueRowStep152D(r));
        r.x--;
        r.nz = r.x;
    } while ((r.x & 0x80) == 0);
    r = loaded(r, RD8(OBJ->f32a8[39]));
    r = compared(r, 0x10);
    if (r.nz != 0) {
        return r;
    }
    r.nz = INC8(OBJ->f32a8[39]);
    return r;
}

void abi_ContinueReveal152D(void)
{
    regs_exit_shim(ContinueReveal152D(regs_entry_shim()));
}

/*
 * @routine 152E:77E ContinueDeadFade152D
 * @run 6B:8F7E
 * @unreachable 6B:8F7E-6B:8F82 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8F7E runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The card's CD fade of type $0C. No code of the overlay reaches these bytes (they follow the RTS of
 * ContinueReveal152D); the C is the bytes' own.
 * HOW: A load and a jump to cd_fade.
 * WAITS: no.
 * Entry: nothing. Exit: as cd_fade: A the nibble, N and Z from it; X, Y, C and V kept.
 */
StageRegs ContinueDeadFade152D(StageRegs r)
{
    r = loaded(r, 0x0C);
    return fade_tail(r, 0x0C);
}

void abi_ContinueDeadFade152D(void)
{
    regs_exit_shim(ContinueDeadFade152D(regs_entry_shim()));
}

/*
 * @routine 152E:783 ContinueRowStep152D
 * @run 6B:8F83
 * @unreachable 6B:8F83-6B:9014 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8F83 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One row of the continue screen's reveal, X the row. While its byte at $2818 is 0 its object slides
 * right by 8 a frame as its count at $32C8 runs down, and then the byte is set. After that, while the count is below
 * $10: the row's word at $6E0E less the count is cleared (not at $0F), the stage's banks $10 and $11 mapped, four
 * columns of the row's picture (from $B29E, a row's 128 bytes) drawn by ContinueRowDraw152D, $2C50 and the count
 * counted.
 * HOW: Tests and stores in the row's columns; the address in zTemp00-01, the picture in zTemp08-09; the kernel's
 * pieces at $4021 and $4027; TAM.
 * WAITS: no.
 * Entry: X the row. Exit: as the last instruction that ran left them; X kept.
 */
StageRegs ContinueRowStep152D(StageRegs r)
{
    Sum sum;
    uint8_t row = r.x, value, old;

    r = loaded(r, RD8(OBJ->f2800[24 + row]));
    if (r.a == 0) {
        r = loaded(r, RD8(OBJ->f32a8[32 + row]));
        if (r.a == 0) {
            r.nz = INC8(OBJ->f2800[24 + row]);
            return r;
        }
        value = RD8(OBJ->x_hi[8 + row]);
        sum = add_bytes(value, 0x08, false);
        r = loaded(r, sum.value);
        r.c = sum.carry;
        r.v = sum.overflow;
        WR8(OBJ->x_hi[8 + row], sum.value);
        r.nz = DEC8(OBJ->f32a8[32 + row]);
        return r;
    }
    r = loaded(r, RD8(OBJ->f32a8[32 + row]));
    r = compared(r, 0x10);
    if (r.c) {
        return r;
    }
    r = continue_row_address(r, row);
    value = RD8(ZP->temp00);
    sum = add_bytes(0x00, value, false);
    WR8(ZP->temp00, sum.value);
    value = RD8(ZP->temp01);
    sum = add_bytes(0x6E, value, sum.carry);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp01, sum.value);
    r = loaded(r, RD8(OBJ->f32a8[32 + row]));
    r = compared(r, 0x0F);
    if (r.nz != 0) {
        r = orig_shim(r, 0x8FC7, 0x4021);
        value = RD8(ZP->temp00);
        sum = add_bytes(value, 0x0E, false);
        value = RD8(OBJ->f32a8[32 + r.x]);
        sum = sub_bytes(sum.value, value, true);
        WR8(VDC->data_lo, sum.value);
        r = loaded(r, RD8(ZP->temp01));
        r.c = sum.carry;
        r.v = sum.overflow;
        WR8(VDC->data_hi, r.a);
        r = orig_shim(r, 0x8FDB, 0x4027);
        st1(0x00);
        st2(0x00);
    }
    sum = add_bytes(0x10, RD8(ZP->ram_base_bank), false);
    tam(0x20, sum.value);
    tam(0x40, (uint8_t)(sum.value + 1));
    WR8(ZP->temp08, 0);
    r.c = (r.x & 0x01) != 0;
    old = RD8(ZP->temp08);
    WR8(ZP->temp08, (uint8_t)((r.c ? 0x80 : 0) | (old >> 1)));
    WR8(ZP->temp09, (uint8_t)(r.x >> 1));
    value = RD8(ZP->temp08);
    sum = add_bytes(0x9E, value, false);
    WR8(ZP->temp08, sum.value);
    value = RD8(ZP->temp09);
    sum = add_bytes(0xB2, value, sum.carry);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp09, sum.value);
    r = loaded(r, 0x0F);
    CALL(0x9002, ContinueRowDraw152D(r));
    r = loaded(r, 0x1F);
    CALL(0x9006, ContinueRowDraw152D(r));
    r = loaded(r, 0x2F);
    CALL(0x900A, ContinueRowDraw152D(r));
    r = loaded(r, 0x3F);
    CALL(0x900E, ContinueRowDraw152D(r));
    r.nz = INC8(OBJ->x_frac[48 + r.x]);
    r.nz = INC8(OBJ->f32a8[32 + r.x]);
    return r;
}

void abi_ContinueRowStep152D(void)
{
    regs_exit_shim(ContinueRowStep152D(regs_entry_shim()));
}

/*
 * @routine 152F:015 ContinueRowDraw152D
 * @run 6B:9015
 * @unreachable 6B:9015-6B:9041 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $9015 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: One word of a continue row's picture: the VDC pointed at the row's word A less its count, and the
 * picture's word there (from zTemp08-09, two bytes a word) written.
 * HOW: The column in zTemp10; the kernel's pieces at $4021 and $4027; reads through the picture's pointer.
 * WAITS: no.
 * Entry: A the column, X the row, zTemp00-01 the row's address, zTemp08-09 the picture. Exit: A the word's high byte,
 * N and Z from it, Y its offset; C and V from the subtract. X as $4027 left it.
 */
StageRegs ContinueRowDraw152D(StageRegs r)
{
    Sum sum;
    uint8_t value;

    WR8(ZP->temp10, r.a);
    r = orig_shim(r, 0x901A, 0x4021);
    value = RD8(ZP->temp00);
    sum = add_bytes(value, RD8(ZP->temp10), false);
    value = RD8(OBJ->f32a8[32 + r.x]);
    sum = sub_bytes(sum.value, value, true);
    WR8(VDC->data_lo, sum.value);
    r = loaded(r, RD8(ZP->temp01));
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(VDC->data_hi, r.a);
    r = orig_shim(r, 0x902E, 0x4027);
    value = RD8(ZP->temp10);
    sum = sub_bytes(value, RD8(OBJ->f32a8[32 + r.x]), true);
    r.c = (sum.value & 0x80) != 0;
    r.v = sum.overflow;
    r.y = (uint8_t)(sum.value << 1);
    r = loaded(r, RD8_PTR_Y(ZP->temp08, r.y));
    WR8(VDC->data_lo, r.a);
    r.y++;
    r = loaded(r, RD8_PTR_Y(ZP->temp08, r.y));
    WR8(VDC->data_hi, r.a);
    return r;
}

void abi_ContinueRowDraw152D(void)
{
    regs_exit_shim(ContinueRowDraw152D(regs_entry_shim()));
}

/* The menu's pointer object: its X at $2C58 from the table at $90CD by ContinueChoice. */
static Regs continue_pointer(Regs r)
{
    r.x = rd8(ContinueChoice);
    r = loaded(r, rd8((uint16_t)(0x90CD + r.x)));
    WR8(OBJ->x_frac[56], r.a);
    return r;
}

/*
 * @routine 152F:042 ContinueMenuStart152D
 * @run 6B:9042
 * @unreachable 6B:9042-6B:908A no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $9042 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The continue menu's start: the kernel's piece at $4057, zField90, zField91, zRasterEffect and the
 * scroll's X and Y cleared; the kernel's piece at $4054 for object 8 (its loop ends after one) and for $44 to $47;
 * the next step; the kernel's pieces at $4012 (A $0E) and $400C (A $6C); the menu's objects (sprites 4 and 3, X $4F)
 * and the choice ContinueChoice 0, the pointer placed.
 * HOW: Stores; two loops of the kernel's piece; a branch into ContinueMenu152D's pointer code at $90B6.
 * WAITS: no.
 * Entry: nothing. Exit: A the pointer's X, N and Z from it, X the choice; Y, C and V as $400C left them.
 */
StageRegs ContinueMenuStart152D(StageRegs r)
{
    r = orig_shim(r, 0x9045, 0x4057);
    WR8(ZP->field90, 0);
    WR8(ZP->field91, 0);
    WR8(ZP->raster_effect, 0);
    WR8(SCROLL->x_lo[0], 0);
    WR8(SCROLL->x_hi[0], 0);
    WR8(SCROLL->y_lo[0], 0);
    WR8(SCROLL->y_hi[0], 0);
    r.x = 0x08;
    do {
        r = orig_shim(r, 0x905C, 0x4054);
        r.x++;
        r.c = r.x >= 0x10;
        r.nz = (uint8_t)(r.x - 0x10);
    } while ((r.nz & 0x80) == 0);
    r.x = 0x44;
    do {
        r = orig_shim(r, 0x9066, 0x4054);
        r.x++;
        r.c = r.x >= 0x48;
        r.nz = (uint8_t)(r.x - 0x48);
    } while (r.nz != 0);
    r.nz = INC8(ZP->game_sub_mode);
    r = loaded(r, 0x0E);
    r = orig_shim(r, 0x9072, 0x4012);
    r = loaded(r, 0x6C);
    r = orig_shim(r, 0x9077, 0x400C);
    r = loaded(r, 0x04);
    WR8(OBJ->sprite_set[80], 0x04);
    r = loaded(r, 0x03);
    WR8(OBJ->sprite_set[8], 0x03);
    r = loaded(r, 0x4F);
    WR8(OBJ->x_hi[16], 0x4F);
    wr8(ContinueChoice, 0);
    return continue_pointer(r);
}

void abi_ContinueMenuStart152D(void)
{
    regs_exit_shim(ContinueMenuStart152D(regs_entry_shim()));
}

/*
 * @routine 152F:08B ContinueMenu152D
 * @run 6B:908B
 * @unreachable 6B:908B-6B:90C7 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $908B runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The continue menu: the cursor's blink; button I or Run takes the choice (sound $FF, zModeDone
 * counted); up, down or Select moves ContinueChoice (the table at $90C8, two on for down), with the kernel's piece at
 * $4012 for the choice's text and sound 1, and the pointer moved.
 * HOW: ContinueBlink152D; pad bits; a table read and an add; the kernel and the sound hook.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction that ran left them.
 */
StageRegs ContinueMenu152D(StageRegs r)
{
    Sum sum;
    uint8_t choice;

    CALL(0x908E, ContinueBlink152D(r));
    r = loaded(r, (uint8_t)(rd8(BiosJoyTrigger) & 0x09));
    if (r.a != 0) {
        r = loaded(r, 0xFF);
        r = orig_shim(r, 0x90C5, SOUND_HOOK);
        r.nz = INC8(ZP->mode_done);
        return r;
    }
    r = loaded(r, (uint8_t)(rd8(BiosJoyTrigger) & 0x54));
    if (r.a == 0) {
        return r;
    }
    r.x = rd8(ContinueChoice);
    r = loaded(r, (uint8_t)(r.a & 0x10));
    if (r.a == 0) {
        r.x = (uint8_t)(r.x + 2);
    }
    choice = rd8((uint16_t)(0x90C8 + r.x));
    wr8(ContinueChoice, choice);
    sum = add_bytes(choice, 0x0E, false);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    r = orig_shim(r, 0x90B1, 0x4012);
    r = loaded(r, 0x01);
    r = orig_shim(r, 0x90B6, SOUND_HOOK);
    return continue_pointer(r);
}

void abi_ContinueMenu152D(void)
{
    regs_exit_shim(ContinueMenu152D(regs_entry_shim()));
}

/*
 * @routine 152F:0D0 ContinueBlink152D
 * @run 6B:90D0
 * @unreachable 6B:90D0-6B:90ED no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $90D0 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Every fourth frame the kernel's piece at $4018 with X 0 and A the next of the ten blink colours at
 * $90EE (the step at $32D0, back to 0 after 9).
 * HOW: zFrameTick's low two bits; a table read; the kernel; an increment and a compare.
 * WAITS: no.
 * Entry: nothing. Exit: A the step or zFrameTick's bits, N, Z and C from the last test; X and Y as $4018 left them.
 */
StageRegs ContinueBlink152D(StageRegs r)
{
    r = loaded(r, (uint8_t)(RD8(ZP->frame_tick) & 0x03));
    if (r.a != 0) {
        return r;
    }
    r.y = RD8(OBJ->f32a8[40]);
    r = loaded(r, rd8((uint16_t)(0x90EE + r.y)));
    r.x = 0;
    r = orig_shim(r, 0x90E0, 0x4018);
    r.nz = INC8(OBJ->f32a8[40]);
    r = loaded(r, RD8(OBJ->f32a8[40]));
    r = compared(r, 0x0A);
    if (!r.c) {
        return r;
    }
    WR8(OBJ->f32a8[40], 0);
    return r;
}

void abi_ContinueBlink152D(void)
{
    regs_exit_shim(ContinueBlink152D(regs_entry_shim()));
}

/*
 * @routine 1530:0F8 StageSubModeTally152D
 * @run 6B:90F8
 * @stop 9108
 * @unreachable 6B:90F8-6B:9107 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $90F8 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-mode 6, the tally after the boss: the events step, the display, the overlays' frame, then the
 * tally's step at zField80 from the eight at $9108 (the start, the board, the board's reveal, a pause, the health and
 * hearts counted into the score, a wait, the stock bonus, the end).
 * HOW: StageModeEventsStep152D, StageHudStep152D, StageOverlayFrame152D; zField80 doubled indexes the table, whose
 * word is jumped through.
 * WAITS: as the step.
 * Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice zField80, N and Z from it, C its
 * bit 7.
 */
StageRegs StageSubModeTally152D(StageRegs r)
{
    uint8_t step, twice;
    uint16_t handler;

    CALL(0x90FB, StageModeEventsStep152D(r));
    CALL(0x90FE, StageHudStep152D(r));
    CALL(0x9101, StageOverlayFrame152D(r));
    step = RD8(ZP->field80);
    twice = (uint8_t)(step << 1);
    r.c = (step & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(0x9108, twice);
    switch (handler) {
    case 0x9118:
        return TallyStart152D(r);
    case 0x9126:
        return TallyBoard152D(r);
    case 0x9198:
        return TallyReveal152D(r);
    case 0x92B1:
        return TallyPause152D(r);
    case 0x92BC:
        return TallyCount152D(r);
    case 0x9312:
        return TallyWaitPress152D(r);
    case 0x9324:
        return TallyStockBonus152D(r);
    case 0x9342:
        return TallyEnd152D(r);
    default:
        return orig_tail_shim(r, handler);
    }
}

void abi_StageSubModeTally152D(void)
{
    regs_exit_shim(StageSubModeTally152D(regs_entry_shim()));
}

/*
 * @routine 1530:34D TallyWait152D
 * @run 6B:934D
 * @unreachable 6B:934D-6B:9357 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $934D runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The tally's wait: while the count at $3310 is not 0 it is counted down and the caller's step ends here,
 * this routine dropping its caller's return address so its RTS ends the caller too.
 * HOW: A test; a decrement; two PLA.
 * WAITS: no.
 * Entry: nothing. Exit, count 0: A 0, N and Z from it, the rest kept. Else: the frame dropped, A the return address's
 * high byte less one, N and Z from it.
 */
TallyWaitEnd TallyWait152D(StageRegs r)
{
    TallyWaitEnd end;
    uint8_t byte;

    r = loaded(r, RD8(OBJ->f3300[16]));
    end.dropped = false;
    if (r.a != 0) {
        r.nz = DEC8(OBJ->f3300[16]);
        byte = pull8();
        r = loaded(r, byte);
        byte = pull8();
        r = loaded(r, byte);
        end.dropped = true;
    }
    end.r = r;
    return end;
}

void abi_TallyWait152D(void)
{
    regs_exit_shim(TallyWait152D(regs_entry_shim()).r);
}

/* JSR $934D: the frame pushed; popped only when TallyWait152D did not drop it. */
static TallyWaitEnd tally_wait(Regs r, uint16_t ret)
{
    TallyWaitEnd end;

    jsr_push(ret);
    end = TallyWait152D(r);
    if (!end.dropped) {
        jsr_pop();
    }
    return end;
}

/*
 * @routine 1530:118 TallyStart152D
 * @run 6B:9118
 * @unreachable 6B:9118-6B:9122 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $9118 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The tally's first step, after the wait: the kernel's piece at $4087, sound $32, and the next step
 * (runs on into TallyNext152D).
 * HOW: TallyWait152D; $4087; the sound hook.
 * WAITS: no.
 * Entry: nothing. Exit: as TallyWait152D when it ends the step; else as TallyNext152D.
 */
StageRegs TallyStart152D(StageRegs r)
{
    TallyWaitEnd wait = tally_wait(r, 0x911B);

    r = wait.r;
    if (wait.dropped) {
        return r;
    }
    r = orig_shim(r, 0x911E, 0x4087);
    r = loaded(r, 0x32);
    r = orig_shim(r, 0x9123, SOUND_HOOK);
    return TallyNext152D(r);
}

void abi_TallyStart152D(void)
{
    regs_exit_shim(TallyStart152D(regs_entry_shim()));
}

/*
 * @routine 1530:123 TallyNext152D
 * @run 6B:9123
 * @unreachable 6B:9123-6B:9125 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $9123 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The tally's next step: zField80 counted.
 * HOW: An increment.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the increment; A, X, Y, C and V kept.
 */
StageRegs TallyNext152D(StageRegs r)
{
    r.nz = INC8(ZP->field80);
    return r;
}

void abi_TallyNext152D(void)
{
    regs_exit_shim(TallyNext152D(regs_entry_shim()));
}

/*
 * @routine 1530:126 TallyBoard152D
 * @run 6B:9126
 * @unreachable 6B:9126-6B:916F no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $9126 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The tally's board: the kernel's piece at $4012 (A $16), the display's pieces 5 to 7
 * (StageDrawPiece152D), the hearts kept at $330D, the full-health mark ($2A9B $18 at zPlayerHealth $5C), three
 * blocks of VRAM words cleared (StageClearWords152D: $0E at $5000, $0C at $5400, $0A at $5D80), the board's cells
 * at $3980 from the 62 at $93B9; the next step (TallyNext152D), then runs on into TallyDigits152D.
 * HOW: Calls; a copy loop.
 * WAITS: no.
 * Entry: nothing. Exit: as TallyDigits152D.
 */
StageRegs TallyBoard152D(StageRegs r)
{
    r = loaded(r, 0x16);
    r = orig_shim(r, 0x912B, 0x4012);
    r.x = 5;
    r.nz = 5;
    CALL(0x9130, StageDrawPiece152D(r));
    r.x++;
    r.nz = r.x;
    CALL(0x9134, StageDrawPiece152D(r));
    r.x++;
    r.nz = r.x;
    CALL(0x9138, StageDrawPiece152D(r));
    r = loaded(r, RD8(ZP->field94));
    WR8(OBJ->f3300[13], r.a);
    r = loaded(r, RD8(ZP->player_health));
    r = compared(r, 0x5C);
    if (r.nz == 0) {
        r = loaded(r, 0x18);
        WR8(OBJ->frame[51], 0x18);
    }
    r.x = 0;
    r.y = 0x50;
    r = loaded(r, 0x0E);
    CALL(0x9150, StageClearWords152D(r));
    r.x = 0;
    r.y = 0x54;
    r = loaded(r, 0x0C);
    CALL(0x9158, StageClearWords152D(r));
    r.x = 0x80;
    r.y = 0x5D;
    r = loaded(r, 0x0A);
    CALL(0x9161, StageClearWords152D(r));
    for (r.x = 0; r.x != 0x3E; r.x++) {
        r = loaded(r, rd8((uint16_t)(0x93B9 + r.x)));
        wr8((uint16_t)(TALLY_CELLS + r.x), r.a);
    }
    r.c = true;
    r.nz = 0;
    CALL(0x9170, TallyNext152D(r));
    return TallyDigits152D(r);
}

void abi_TallyBoard152D(void)
{
    regs_exit_shim(TallyBoard152D(regs_entry_shim()));
}

/*
 * @routine 1530:170 TallyDigits152D
 * @run 6B:9170
 * @unreachable 6B:9170-6B:9197 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $9170 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Puts the health and the hearts on the board as digits: zPlayerHealth's tens and units (StageSplitTens152D)
 * at $3991 and $3992, zField94's two BCD digits at $39A1 and $39A2, each one more than the digit; a leading zero is
 * left out.
 * HOW: StageSplitTens152D; shifts and masks; stores.
 * WAITS: no.
 * Entry: nothing. Exit: A the hearts' last cell, N and Z from its increment; X the health's tens cell; C from the
 * compare, V from the split's add. Y kept.
 */
StageRegs TallyDigits152D(StageRegs r)
{
    uint8_t hearts;

    r = loaded(r, RD8(ZP->player_health));
    CALL(0x9175, StageSplitTens152D(r));
    r.x++;
    r.c = r.x >= 0x01;
    r.nz = (uint8_t)(r.x - 0x01);
    if (r.nz != 0) {
        wr8((uint16_t)(TALLY_CELLS + 0x11), r.x);
    }
    r = loaded(r, (uint8_t)(r.a + 1));
    wr8((uint16_t)(TALLY_CELLS + 0x12), r.a);
    hearts = RD8(ZP->field94);
    r.c = (hearts & 0x08) != 0;
    r = loaded(r, (uint8_t)((hearts >> 4) + 1));
    r = compared(r, 0x01);
    if (r.nz != 0) {
        wr8((uint16_t)(TALLY_CELLS + 0x21), r.a);
    }
    hearts = RD8(ZP->field94);
    r = loaded(r, (uint8_t)((hearts & 0x0F) + 1));
    wr8((uint16_t)(TALLY_CELLS + 0x22), r.a);
    return r;
}

void abi_TallyDigits152D(void)
{
    regs_exit_shim(TallyDigits152D(regs_entry_shim()));
}

/* The stage's Super RAM banks $10 and $11 above zRamBaseBank in MPR5 and MPR6. */
static Regs map_stage_banks(Regs r)
{
    Sum sum = add_bytes(0x10, RD8(ZP->ram_base_bank), false);

    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    tam(0x20, r.a);
    r = loaded(r, (uint8_t)(r.a + 1));
    tam(0x40, r.a);
    return r;
}

/*
 * @routine 1530:198 TallyReveal152D
 * @run 6B:9198
 * @unreachable 6B:9198-6B:922B no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $9198 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The board's reveal, under the stage's banks: once all 62 cells at $3980 are 0 the reveal is done
 * (zPaused's bit 1 set, the wait at $3310 $40, the next step). Else a random cell that is not yet 0 is stepped by $40
 * (cleared when it carries), and drawn: a cell of the first ten columns as its 16-word glyph from the picture at $B77E
 * (rows from $5D80); the others by TallyCell152D.
 * HOW: MapStageBanks; a loop down X; the kernel's random piece at $4051 until it gives a live cell; shifts into
 * zTemp08-09, the glyph's address in zTemp10-11; SetVramWriteAddress152D; a loop of 16 words with SXY.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction that ran left them.
 */
StageRegs TallyReveal152D(StageRegs r)
{
    Sum sum;
    uint8_t cell, value, old, tens;

    r = map_stage_banks(r);
    for (r.x = 0x3D; (r.x & 0x80) == 0; r.x--) {
        r = loaded(r, rd8((uint16_t)(TALLY_CELLS + r.x)));
        if (r.a != 0) {
            break;
        }
    }
    if ((r.x & 0x80) != 0) {
        r.nz = r.x;
        RMW8(ZP->paused, old, old | 0x02);
        r = loaded(r, 0x40);
        WR8(OBJ->f3300[16], 0x40);
        return TallyNext152D(r);
    }
    for (;;) {
        r = orig_shim(r, 0x91B9, 0x4051);
        r = loaded(r, (uint8_t)(r.a & 0x3F));
        r = compared(r, 0x3E);
        if (r.c) {
            continue;
        }
        r.x = r.a;
        r = loaded(r, rd8((uint16_t)(TALLY_CELLS + r.x)));
        if (r.a != 0) {
            break;
        }
    }
    WR8(ZP->temp00, r.a);
    sum = add_bytes(r.a, 0x40, r.c);
    wr8((uint16_t)(TALLY_CELLS + r.x), sum.value);
    if (sum.carry) {
        wr8((uint16_t)(TALLY_CELLS + r.x), 0);
    }
    r.c = sum.carry;
    r.v = sum.overflow;
    cell = (uint8_t)(r.x & 0x3F);
    r = loaded(r, cell);
    r = compared(r, 0x0A);
    if (r.c) {
        return TallyCell152D(r);
    }
    WR8(ZP->temp08, 0);
    old = RD8(ZP->temp08);
    WR8(ZP->temp08, (uint8_t)(((cell & 0x01) != 0 ? 0x80 : 0) | (old >> 1)));
    old = RD8(ZP->temp08);
    WR8(ZP->temp08, (uint8_t)(((cell & 0x02) != 0 ? 0x80 : 0) | (old >> 1)));
    WR8(ZP->temp09, (uint8_t)(cell >> 2));
    value = RD8(ZP->temp00);
    tens = (uint8_t)((value >> 2) & 0x30);
    sum = add_bytes(tens, 0x80, false);
    value = RD8(ZP->temp08);
    sum = add_bytes(sum.value, value, false);
    r.x = sum.value;
    value = RD8(ZP->temp09);
    sum = add_bytes(0x5D, value, sum.carry);
    r.y = sum.value;
    value = RD8(ZP->temp00);
    cell = (uint8_t)((uint8_t)(value - 1) & 0x3F);
    WR8(ZP->temp08, 0);
    for (int i = 0; i < 3; i++) {
        old = RD8(ZP->temp08);
        WR8(ZP->temp08, (uint8_t)(((cell & 0x01) != 0 ? 0x80 : 0) | (old >> 1)));
        cell = (uint8_t)(cell >> 1);
    }
    WR8(ZP->temp09, cell);
    value = RD8(ZP->temp08);
    sum = add_bytes(0x7E, value, false);
    WR8(ZP->temp10, sum.value);
    value = RD8(ZP->temp09);
    sum = add_bytes(0xB7, value, sum.carry);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp11, sum.value);
    CALL(0x9216, SetVramWriteAddress152D(r));
    r.y = 0;
    r.x = 0x10;
    do {
        r = loaded(r, RD8_PTR_Y(ZP->temp10, r.x));
        WR8(VDC->data_lo, r.a);
        r = loaded(r, RD8_PTR_Y(ZP->temp10, r.y));
        WR8(VDC->data_hi, r.a);
        r.y++;
        r.x++;
        r.c = r.y >= 0x10;
        r.nz = (uint8_t)(r.y - 0x10);
    } while (r.nz != 0);
    return r;
}

void abi_TallyReveal152D(void)
{
    regs_exit_shim(TallyReveal152D(regs_entry_shim()));
}

/*
 * @routine 1530:22C TallyCell152D
 * @run 6B:922C
 * @unreachable 6B:922C-6B:92B0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $922C runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Draws one of the board's cells past the first ten columns: A the cell's index, zTemp00 its value
 * (its top two bits the shade, its low six the glyph). The cell's place gives the VRAM word (rows of eight from $5008,
 * odd and even halves) and which of the word's bytes it fills; eight bytes of the glyph from the picture at $B69E
 * (eight a glyph) go into that byte, the other byte written back as it was.
 * HOW: A subtract and shifts into zTemp01-02 and zTemp08-09, the glyph's address in zTemp10-11; SetVramAddresses152D;
 * a loop of eight reads and writes of the VDC's data port.
 * WAITS: no.
 * Entry: A the cell's index ($0A up), C set; zTemp00 the cell's value. Exit: Y 8, Z set, C set; A the last byte read
 * or written; X the VRAM address's low byte. Clobbers zTemp01-02, zTemp08-11.
 */
StageRegs TallyCell152D(StageRegs r)
{
    Sum sum;
    uint8_t cell, value, old, place;

    sum = sub_bytes(r.a, 0x0A, true);
    cell = sum.value;
    r.x = cell;
    WR8(ZP->temp01, cell);
    value = (uint8_t)((cell >> 1) & 0x1C);
    if ((RD8(ZP->temp01) & 0x04) != 0) {
        value = (uint8_t)(value | 0x20);
    }
    WR8(ZP->temp02, value);
    place = (uint8_t)(RD8(ZP->temp01) & 0x03);
    place = (uint8_t)(place | RD8(ZP->temp02));
    WR8(ZP->temp01, place);
    WR8(ZP->temp08, 0);
    (void)LSR8(ZP->temp08);
    place = (uint8_t)(place >> 1);
    for (int i = 0; i < 2; i++) {
        old = RD8(ZP->temp08);
        WR8(ZP->temp08, (uint8_t)(((place & 0x01) != 0 ? 0x80 : 0) | (old >> 1)));
        place = (uint8_t)(place >> 1);
    }
    WR8(ZP->temp09, place);
    value = RD8(ZP->temp00);
    sum = add_bytes((uint8_t)((value >> 2) & 0x30), 0x08, false);
    value = RD8(ZP->temp08);
    sum = add_bytes(sum.value, value, false);
    r.x = sum.value;
    value = RD8(ZP->temp09);
    sum = add_bytes(0x50, value, sum.carry);
    r.y = sum.value;
    value = RD8(ZP->temp00);
    value = (uint8_t)((uint8_t)(value - 1) & 0x3F);
    WR8(ZP->temp09, 0);
    for (int i = 0; i < 3; i++) {
        bool out = (value & 0x80) != 0;

        value = (uint8_t)(value << 1);
        old = RD8(ZP->temp09);
        WR8(ZP->temp09, (uint8_t)((old << 1) | (out ? 1 : 0)));
    }
    WR8(ZP->temp08, value);
    value = RD8(ZP->temp08);
    sum = add_bytes(0x9E, value, false);
    WR8(ZP->temp10, sum.value);
    value = RD8(ZP->temp09);
    sum = add_bytes(0xB6, value, sum.carry);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp11, sum.value);
    r = set_vram_addresses(r, 0x9285);
    r.y = 0;
    if ((RD8(ZP->temp01) & 0x01) == 0) {
        do {
            value = RD8(VDC->data_lo);
            WR8(VDC->data_lo, value);
            (void)RD8(VDC->data_hi);
            r = loaded(r, RD8_PTR_Y(ZP->temp10, r.y));
            WR8(VDC->data_hi, r.a);
            r.y++;
        } while (r.y != 0x08);
    } else {
        do {
            (void)RD8(VDC->data_lo);
            r = loaded(r, RD8_PTR_Y(ZP->temp10, r.y));
            WR8(VDC->data_lo, r.a);
            value = RD8(VDC->data_hi);
            r = loaded(r, value);
            WR8(VDC->data_hi, value);
            r.y++;
        } while (r.y != 0x08);
    }
    r.c = true;
    r.nz = 0;
    return r;
}

void abi_TallyCell152D(void)
{
    regs_exit_shim(TallyCell152D(regs_entry_shim()));
}

/*
 * @routine 1530:2B1 TallyPause152D
 * @run 6B:92B1
 * @unreachable 6B:92B1-6B:92BB no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $92B1 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A pause of the tally: after the wait, the next wait at $3310 set to $40 and the next step.
 * HOW: TallyWait152D; a store; a jump to TallyNext152D.
 * WAITS: no.
 * Entry: nothing. Exit: as TallyWait152D when it ends the step; else as TallyNext152D, A $40.
 */
StageRegs TallyPause152D(StageRegs r)
{
    TallyWaitEnd wait = tally_wait(r, 0x92B4);

    r = wait.r;
    if (wait.dropped) {
        return r;
    }
    r = loaded(r, 0x40);
    WR8(OBJ->f3300[16], 0x40);
    return TallyNext152D(r);
}

void abi_TallyPause152D(void)
{
    regs_exit_shim(TallyPause152D(regs_entry_shim()));
}

/*
 * @routine 1530:2BC TallyCount152D
 * @run 6B:92BC
 * @unreachable 6B:92BC-6B:9311 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $92BC runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The tally's count: the paused frame's menu step with zPaused's bit 0 set for it; while zPaused's bit 1
 * is clear the health is counted into the score (zPlayerHealth down by one, its cells redrawn by TallyHealth152D, 1
 * point by the kernel's piece at $408A), then after the wait the hearts (zField94 down by one in BCD, TallyHearts152D,
 * $10 points); the score redrawn and sound 1 every fourth event. With both at 0 the tally goes on two steps, or, when
 * the full-health mark at $2A9B is set, stays with zPaused's bit 1 set and the wait at $3310 $40.
 * HOW: Bit changes; StagePausedFrame152D; decrements; $408A; StageDrawScore152D; the sound hook.
 * WAITS: no.
 * Entry: nothing. Exit: as the last piece that ran left them.
 */
StageRegs TallyCount152D(StageRegs r)
{
    TallyWaitEnd wait;
    uint8_t old;

    RMW8(ZP->paused, old, old | 0x01);
    CALL(0x92C1, StagePausedFrame152D(r));
    RMW8(ZP->paused, old, old & (uint8_t)~0x01);
    if ((RD8(ZP->paused) & 0x02) != 0) {
        return r;
    }
    r = loaded(r, RD8(ZP->player_health));
    if (r.a != 0) {
        r.nz = DEC8(ZP->player_health);
        CALL(0x92CF, TallyHealth152D(r));
        r = loaded(r, 0x01);
    } else {
        wait = tally_wait(r, 0x92D6);
        r = wait.r;
        if (wait.dropped) {
            return r;
        }
        r = loaded(r, RD8(ZP->field94));
        if (r.a == 0) {
            r.nz = DEC8(OBJ->f3300[16]);
            r.nz = INC8(ZP->field80);
            r.nz = INC8(ZP->field80);
            r = loaded(r, RD8(OBJ->frame[51]));
            if (r.a != 0) {
                r.nz = DEC8(ZP->field80);
                r.nz = DEC8(ZP->field80);
                RMW8(ZP->paused, old, old | 0x02);
                r = loaded(r, 0x40);
                WR8(OBJ->f3300[16], 0x40);
            }
            return TallyNext152D(r);
        }
        r = bcd_less_one(r);
        WR8(ZP->field94, r.a);
        CALL(0x92E4, TallyHearts152D(r));
        r = loaded(r, 0x10);
    }
    r = orig_shim(r, 0x92E9, 0x408A);
    CALL(0x92EC, StageDrawScore152D(r));
    r = loaded(r, (uint8_t)(RD8(ZP->mode_events) & 0x03));
    if (r.a != 0) {
        return r;
    }
    r = loaded(r, 0x01);
    return orig_shim(r, 0x92F7, SOUND_HOOK);
}

void abi_TallyCount152D(void)
{
    regs_exit_shim(TallyCount152D(regs_entry_shim()));
}

/*
 * @routine 1530:312 TallyWaitPress152D
 * @run 6B:9312
 * @unreachable 6B:9312-6B:9323 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $9312 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A wait of the tally: after the wait, the paused frame's menu step; once zPaused's bit 1 is clear the
 * wait at $3310 is $20 and the next step.
 * HOW: TallyWait152D; StagePausedFrame152D; a bit test; a store and a jump to TallyNext152D.
 * WAITS: no.
 * Entry: nothing. Exit: as the last piece that ran left them.
 */
StageRegs TallyWaitPress152D(StageRegs r)
{
    TallyWaitEnd wait = tally_wait(r, 0x9315);

    r = wait.r;
    if (wait.dropped) {
        return r;
    }
    CALL(0x9318, StagePausedFrame152D(r));
    if ((RD8(ZP->paused) & 0x02) != 0) {
        return r;
    }
    r = loaded(r, 0x20);
    WR8(OBJ->f3300[16], 0x20);
    return TallyNext152D(r);
}

void abi_TallyWaitPress152D(void)
{
    regs_exit_shim(TallyWaitPress152D(regs_entry_shim()));
}

/*
 * @routine 1530:324 TallyStockBonus152D
 * @run 6B:9324
 * @unreachable 6B:9324-6B:9341 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $9324 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The stock bonus: after the wait, zStock up by one in BCD (kept at 99), redrawn (StageDrawStock152D),
 * sound $2A, the wait at $3310 one less and the next step.
 * HOW: TallyWait152D; SED, ADC, CLD; StageDrawStock152D; the sound hook; a decrement; a jump to TallyNext152D.
 * WAITS: no.
 * Entry: D clear, as the game keeps it. Exit: as TallyNext152D.
 */
StageRegs TallyStockBonus152D(StageRegs r)
{
    TallyWaitEnd wait = tally_wait(r, 0x9327);

    r = wait.r;
    if (wait.dropped) {
        return r;
    }
    r = loaded(r, RD8(ZP->stock));
    r = bcd_plus_one(r);
    if (r.c) {
        r = loaded(r, 0x99);
    }
    WR8(ZP->stock, r.a);
    CALL(0x9337, StageDrawStock152D(r));
    r = loaded(r, 0x2A);
    r = orig_shim(r, 0x933C, SOUND_HOOK);
    r.nz = DEC8(OBJ->f3300[16]);
    return TallyNext152D(r);
}

void abi_TallyStockBonus152D(void)
{
    regs_exit_shim(TallyStockBonus152D(regs_entry_shim()));
}

/*
 * @routine 1530:342 TallyEnd152D
 * @run 6B:9342
 * @unreachable 6B:9342-6B:934C no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $9342 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The tally's end: after the wait, zField94 back from the hearts kept at $330D and zModeDone counted.
 * HOW: TallyWait152D; a store; an increment.
 * WAITS: no.
 * Entry: nothing. Exit: A the hearts, N and Z from zModeDone's increment.
 */
StageRegs TallyEnd152D(StageRegs r)
{
    TallyWaitEnd wait = tally_wait(r, 0x9345);

    r = wait.r;
    if (wait.dropped) {
        return r;
    }
    r = loaded(r, RD8(OBJ->f3300[13]));
    WR8(ZP->field94, r.a);
    r.nz = INC8(ZP->mode_done);
    return r;
}

void abi_TallyEnd152D(void)
{
    regs_exit_shim(TallyEnd152D(regs_entry_shim()));
}

/* The last cells of a count drawn: below LIMIT the word block at $5440 or $5540 cleared (StageClearWords152D, one row),
 * else the next cell stepped (TallyDigitCell152D). */
static Regs tally_count_cells(Regs r, uint8_t value, uint8_t limit, uint8_t cell, uint8_t row_hi)
{
    r = loaded(r, value);
    r = compared(r, limit);
    if (r.c) {
        r.x = cell;
        r.nz = cell;
        return TallyDigitCell152D(r);
    }
    r.x = 0x40;
    r.y = row_hi;
    r = loaded(r, 0x01);
    return StageClearWords152D(r);
}

/*
 * @routine 1530:358 TallyHealth152D
 * @run 6B:9358
 * @unreachable 6B:9358-6B:937B no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $9358 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Redraws the board's health: the stage's banks mapped, the digits put in (TallyDigits152D), the units'
 * cell $12 stepped (TallyDigitCell152D); from 10 up the tens' cell $11 too, below it the tens' place cleared.
 * HOW: MapStageBanks; TallyDigits152D; TallyDigitCell152D by BSR; a compare; a branch to TallyDigitCell152D or a jump
 * to StageClearWords152D.
 * WAITS: no.
 * Entry: nothing. Exit: as TallyDigitCell152D or StageClearWords152D.
 */
StageRegs TallyHealth152D(StageRegs r)
{
    r = map_stage_banks(r);
    CALL(0x9365, TallyDigits152D(r));
    r.x = 0x12;
    r.nz = 0x12;
    CALL(0x9369, TallyDigitCell152D(r));
    return tally_count_cells(r, RD8(ZP->player_health), 0x0A, 0x11, 0x54);
}

void abi_TallyHealth152D(void)
{
    regs_exit_shim(TallyHealth152D(regs_entry_shim()));
}

/*
 * @routine 1530:37C TallyHearts152D
 * @run 6B:937C
 * @unreachable 6B:937C-6B:939F no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $937C runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Redraws the board's hearts as TallyHealth152D does the health: the units' cell $22, and from $10 up
 * (BCD) the tens' cell $21, below it the tens' place cleared.
 * HOW: MapStageBanks; TallyDigits152D; TallyDigitCell152D by BSR; a compare; a branch to TallyDigitCell152D or a jump
 * to StageClearWords152D.
 * WAITS: no.
 * Entry: nothing. Exit: as TallyDigitCell152D or StageClearWords152D.
 */
StageRegs TallyHearts152D(StageRegs r)
{
    r = map_stage_banks(r);
    CALL(0x9389, TallyDigits152D(r));
    r.x = 0x22;
    r.nz = 0x22;
    CALL(0x938D, TallyDigitCell152D(r));
    return tally_count_cells(r, RD8(ZP->field94), 0x10, 0x21, 0x55);
}

void abi_TallyHearts152D(void)
{
    regs_exit_shim(TallyHearts152D(regs_entry_shim()));
}

/*
 * @routine 1530:3A0 TallyDigitCell152D
 * @run 6B:93A0
 * @unreachable 6B:93A0-6B:93B8 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $93A0 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Draws the board's cell X at every shade from its value up: TallyCell152D for the cell, then the cell
 * stepped by $40 until it carries.
 * HOW: X kept on the stack across TallyCell152D; an add; a loop.
 * WAITS: no.
 * Entry: X the cell. Exit: A the cell plus $40, C set, N and Z from it; X the cell; Y 8.
 */
StageRegs TallyDigitCell152D(StageRegs r)
{
    Sum sum;
    uint8_t value;

    for (;;) {
        push8(r.x);
        r = loaded(r, rd8((uint16_t)(TALLY_CELLS + r.x)));
        WR8(ZP->temp00, r.a);
        r = loaded(r, r.x);
        CALL(0x93AA, TallyCell152D(r));
        r.x = pull8();
        r.nz = r.x;
        value = rd8((uint16_t)(TALLY_CELLS + r.x));
        sum = add_bytes(value, 0x40, false);
        r = loaded(r, sum.value);
        r.c = sum.carry;
        r.v = sum.overflow;
        if (r.c) {
            return r;
        }
        wr8((uint16_t)(TALLY_CELLS + r.x), sum.value);
    }
}

void abi_TallyDigitCell152D(void)
{
    regs_exit_shim(TallyDigitCell152D(regs_entry_shim()));
}

/*
 * @routine 1530:3F7 StageClearWords152D
 * @run 6B:93F7
 * @unreachable 6B:93F7-6B:9409 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $93F7 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Clears A rows of 64 VRAM words from the address in X and Y.
 * HOW: The count in zTemp00; SetVramWriteAddress152D; ST1 and ST2 in loops.
 * WAITS: no.
 * Entry: A the rows, X and Y the address. Exit: X 0, A as SetVramWriteAddress152D left it, N and Z from the count's
 * decrement (Z set). Y, C and V as SetVramWriteAddress152D left them. Clobbers zTemp00.
 */
StageRegs StageClearWords152D(StageRegs r)
{
    WR8(ZP->temp00, r.a);
    CALL(0x93FC, SetVramWriteAddress152D(r));
    do {
        for (r.x = 0x40; r.x != 0; r.x--) {
            st1(0x00);
            st2(0x00);
        }
        r.nz = DEC8(ZP->temp00);
    } while (r.nz != 0);
    return r;
}

void abi_StageClearWords152D(void)
{
    regs_exit_shim(StageClearWords152D(regs_entry_shim()));
}

/*
 * @routine 1530:40A StageSplitTens152D
 * @run 6B:940A
 * @unreachable 6B:940A-6B:9414 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $940A runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Splits A into its tens (X) and its units (A) by repeated subtracts of ten.
 * HOW: A loop of SBC #$0A, X counted; the last subtract undone.
 * WAITS: no.
 * Entry: A the number. Exit: A the units, X the tens, N and Z from X's decrement; C and V from the add that undid the
 * last subtract. Y kept.
 */
StageRegs StageSplitTens152D(StageRegs r)
{
    Sum sum;

    r.x = 0;
    do {
        r.x++;
        sum = sub_bytes(r.a, 0x0A, true);
        r.a = sum.value;
    } while (sum.carry);
    sum = add_bytes(r.a, 0x0A, false);
    r.a = sum.value;
    r.c = sum.carry;
    r.v = sum.overflow;
    r.x--;
    r.nz = r.x;
    return r;
}

void abi_StageSplitTens152D(void)
{
    regs_exit_shim(StageSplitTens152D(regs_entry_shim()));
}

/*
 * @routine 1530:415 SetVramAddresses152D
 * @run 6B:9415
 * @unreachable 6B:9415-6B:941D no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $9415 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Points both of the VDC's VRAM addresses at the word in X and Y, then runs on into
 * SetVramWriteAddress152D. The bytes are those of SetVramAddresses (map_draw.c) at this overlay's address, where
 * $941E is an entry of its own.
 * HOW: The kernel's piece at $4024 (the read address), X and Y into the data port; then SetVramWriteAddress152D.
 * WAITS: no.
 * Entry: X low byte, Y high byte of the VRAM address. Exit: as SetVramWriteAddress152D.
 */
StageRegs SetVramAddresses152D(StageRegs r)
{
    r = orig_shim(r, 0x9418, 0x4024);
    WR8(VDC->data_lo, r.x);
    WR8(VDC->data_hi, r.y);
    return SetVramWriteAddress152D(r);
}

void abi_SetVramAddresses152D(void)
{
    regs_exit_shim(SetVramAddresses152D(regs_entry_shim()));
}

/*
 * @routine 1530:41E SetVramWriteAddress152D
 * @run 6B:941E
 * @unreachable 6B:941E-6B:9429 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $941E runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Points the VDC's VRAM write address at the word in X and Y and selects the data port.
 * HOW: The kernel's piece at $4021, X and Y into the data port; a jump to the kernel's piece at $4027.
 * WAITS: no.
 * Entry: X low byte, Y high byte of the VRAM address. Exit: as $4027 leaves them.
 */
StageRegs SetVramWriteAddress152D(StageRegs r)
{
    r = orig_shim(r, 0x9421, 0x4021);
    WR8(VDC->data_lo, r.x);
    WR8(VDC->data_hi, r.y);
    return orig_tail_shim(r, 0x4027);
}

void abi_SetVramWriteAddress152D(void)
{
    regs_exit_shim(SetVramWriteAddress152D(regs_entry_shim()));
}

/*
 * @routine 1530:448 StageDrawPiece152D
 * @run 6B:9448
 * @unreachable 6B:9448-6B:9467 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $9448 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Places one of the display's pieces, X from 0 to 7: the kernel's piece at $405D gives a slot for the
 * object the table at $9468 names, and the slot takes its sprite, set, X and Y from the tables at $9470, $9478,
 * $9480 and $9488.
 * HOW: Table reads by X; $405D; stores by the slot in Y.
 * WAITS: no.
 * Entry: X the piece. Exit: A the Y written, N and Z from it; X and Y as $405D left them; C and V as it left them.
 */
StageRegs StageDrawPiece152D(StageRegs r)
{
    r = loaded(r, rd8((uint16_t)(0x9468 + r.x)));
    r.y = r.a;
    r = orig_shim(r, 0x944F, 0x405D);
    r = loaded(r, rd8((uint16_t)(0x9470 + r.x)));
    WR8(OBJ->sprite_set[72 + r.y], r.a);
    r = loaded(r, rd8((uint16_t)(0x9478 + r.x)));
    WR8(OBJ->sprite_set[r.y], r.a);
    r = loaded(r, rd8((uint16_t)(0x9480 + r.x)));
    WR8(OBJ->x_hi[8 + r.y], r.a);
    r = loaded(r, rd8((uint16_t)(0x9488 + r.x)));
    WR8(OBJ->x_frac[48 + r.y], r.a);
    return r;
}

void abi_StageDrawPiece152D(void)
{
    regs_exit_shim(StageDrawPiece152D(regs_entry_shim()));
}
