/*
 * The stage overlay of disc sectors 03CD to 03D0 (8 K at $8000, bank $6B as the stage overlays run), which no read
 * the game makes can load; sectors 134D, 14DD, 154D, 15BD, 162D and 177D hold the same bytes. It is a later build of
 * the overlay of sectors 152D to 1530 (stage_b_census.c), with a demo recorder (zWaitTimer's byte at $78 its mode,
 * the pad's bytes kept from $A000 under bank $40) and its own frame by zGameSubMode. Routines whose bytes are those of
 * the 152D build, or of filed code, and run the same at this address are not filed again; their C serves here: $802D
 * MapOverlayBanks5And6, $8044 MapOverlayBank7, $81AD StageModeEventsStep152D, $81D3, $825E, $825F and $95FB (an RTS,
 * StageSubStepNone152D), $87B7 StageFadeTakeStock152D, $87CE StageFadeWaitLast152D, $87D8 StageWaitTimerStep, $8FA1
 * StageOpeningWriteRow152D, $9128 TallyNext152D, $934B TallyWait152D, $9405 StageSplitTens152D, $9425 AndVramData,
 * $9434 OrVramData, $9638 HitKindSpawn152D, $967A HitKindState152D.
 *
 * Fields with no name yet keep their address form, as in stage_b_census.c, and the demo recorder's pointer at zero page
 * $7E and its bytes at $7C and $7D, and the pad bytes at $2229 and $222E.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "stage_map.h"
#include "enemy_objects.h"
#include "syscard.h"
#include "stage_b_census.h"
#include "stage_c_census.h"

enum {
    FADE_ACTIVE = 0x203C, SUB3_STEP_TABLE = 0x2084, SUB3_STEP = 0x2085, FALL_STEP = 0x2088, FADE_COUNT = 0x209B,
    FADE_SOUND = 0x209D, STAGE_FLAG_2743 = 0x2743, DEMO_HELD = 0x207C, DEMO_COUNT = 0x207D, DEMO_POINTER = 0x207E,
    DEMO_PAGE = 0x207F, PAD_HELD = 0x2229, PAD_NEW = 0x222E, DEMO_BUFFER = 0xA000,
    KERNEL_4006 = 0x4006, KERNEL_4009 = 0x4009, SOUND_HOOK = 0x401B, KERNEL_407B = 0x407B,
    HIT_X = 0x2713, HIT_Y = 0x2723, HIT_HI = 0x2733, TEST_X = 0x2702, TEST_Y = 0x2703, REACH_X = 0x2704,
    REACH_Y = 0x2705, HIT_GATE = 0x2748, PROBE_HIT = 0x26F6, PROBE_X = 0x26FA, PROBE_Y = 0x26FE,
    ROW_MAP = 0x3A00, TALLY_CELLS = 0x3980, BOSS_BAR = 0x2745, ROW_BUFFER = 0x397E, SCREEN_3EB4 = 0x3EB4,
    SCREEN_3EC4 = 0x3EC4, CENTER_GATE = 0x2860
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

/* JSR $9410, SetVramAddresses3CD. */
static Regs set_vram_addresses(Regs r, uint16_t ret)
{
    CALL(ret, SetVramAddresses3CD(r));
    return r;
}

/* JSR to the census duplicates of map_draw.c's AndVramData and OrVramData, at $9425 and $9434, through their C. */

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

/* JSR $802D: the bytes of MapOverlayBanks5And6. */
static Regs map_banks_5_6(Regs r, uint16_t ret)
{
    jsr_push(ret);
    r = after_ended(r, MapOverlayBanks5And6());
    jsr_pop();
    return r;
}

/* JSR $8044: the bytes of MapOverlayBank7. */
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


/* A frame-dropping routine called by JSR: the frame pushed, popped only when it did not drop it. */
static TallyWaitEnd call_dropping(Regs r, uint16_t ret, TallyWaitEnd (*routine)(StageRegs))
{
    TallyWaitEnd end;

    jsr_push(ret);
    end = routine(r);
    if (!end.dropped) {
        jsr_pop();
    }
    return end;
}

/* TST #IMM,ADDR: Z from the AND of the immediate with the byte, N and V from the byte's bits 7 and 6. N and Z are
 * written as the value that gives them; the code branches on Z right after and sets N again before it is read. */
static Regs tested(Regs r, uint8_t imm, uint16_t addr)
{
    uint8_t byte = rd8(addr);

    r.v = (byte & 0x40) != 0;
    r.nz = (byte & imm) != 0 ? (uint8_t)((byte & 0x80) != 0 ? 0x80 : 0x01) : 0;
    return r;
}

/*
 * @routine 03CD:000 StageEntryA3CD
 * @run 6B:8000
 * @unreachable 6B:8000-6B:8002 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8000 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8000: goes to StageSubModeStep3CD, the stage's frame by sub-mode.
 * HOW: One jump, no frame of its own.
 * WAITS: as the sub-mode's handler.
 * Entry, exit: as StageSubModeStep3CD.
 */
StageRegs StageEntryA3CD(StageRegs r)
{
    return StageSubModeStep3CD(r);
}

void abi_StageEntryA3CD(void)
{
    regs_exit_shim(StageEntryA3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:003 StageEntryB3CD
 * @run 6B:8003
 * @unreachable 6B:8003-6B:8005 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8003 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8003: a jump to $0000, the hardware page, where no code is (the entry of this
 * build that has no routine).
 * HOW: One jump, no frame of its own.
 * WAITS: as what runs there.
 * Entry, exit: as the code at $0000.
 */
StageRegs StageEntryB3CD(StageRegs r)
{
    return orig_tail_shim(r, 0x0000);
}

void abi_StageEntryB3CD(void)
{
    regs_exit_shim(StageEntryB3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:006 StageEntryC3CD
 * @run 6B:8006
 * @unreachable 6B:8006-6B:8008 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8006 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8006: goes to ProbeTile3CD, the tile probe at $974F.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as ProbeTile3CD.
 */
StageRegs StageEntryC3CD(StageRegs r)
{
    return ProbeTile3CD(r);
}

void abi_StageEntryC3CD(void)
{
    regs_exit_shim(StageEntryC3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:009 StageEntryD3CD
 * @run 6B:8009
 * @unreachable 6B:8009-6B:800B no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8009 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8009: goes to ClearBitRowsFour3CD at $9097.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as ClearBitRowsFour3CD.
 */
StageRegs StageEntryD3CD(StageRegs r)
{
    return ClearBitRowsFour3CD(r);
}

void abi_StageEntryD3CD(void)
{
    regs_exit_shim(StageEntryD3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:00C StageEntryE3CD
 * @run 6B:800C
 * @unreachable 6B:800C-6B:800E no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $800C runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $800C: goes to StageProbePoints3CD at $968D.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as StageProbePoints3CD.
 */
StageRegs StageEntryE3CD(StageRegs r)
{
    return StageProbePoints3CD(r);
}

void abi_StageEntryE3CD(void)
{
    regs_exit_shim(StageEntryE3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:00F StageEntryF3CD
 * @run 6B:800F
 * @unreachable 6B:800F-6B:8011 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $800F runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $800F: goes to StageProbeAgain3CD at $96DB.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as StageProbeAgain3CD.
 */
StageRegs StageEntryF3CD(StageRegs r)
{
    return StageProbeAgain3CD(r);
}

void abi_StageEntryF3CD(void)
{
    regs_exit_shim(StageEntryF3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:018 StageSubModeStep3CD
 * @run 6B:8018
 * @stop 801F
 * @unreachable 6B:8018-6B:801E no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8018 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The stage's frame: runs the handler of the sub-mode zGameSubMode holds, from the table of seven at
 * $801F (0 the start, 1 play, 2 the player's death, 3 the stage cleared, 4 the fade out, 5 the opening, 6 the tally).
 * HOW: The sub-mode doubled indexes the table, whose word is jumped through.
 * WAITS: as the handler.
 * Entry: nothing. Exit: as the handler leaves them; it is entered with A and X twice the sub-mode, N and Z from it,
 * C the sub-mode's bit 7.
 */
StageRegs StageSubModeStep3CD(StageRegs r)
{
    uint8_t mode = RD8(ZP->game_sub_mode);
    uint8_t twice = (uint8_t)(mode << 1);
    uint16_t handler;

    r.c = (mode & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(0x801F, twice);
    switch (handler) {
    case 0x8125:
        return StageSubModeStart3CD(r);
    case 0x81DE:
        return StageSubModePlay3CD(r);
    case 0x8300:
        return StageSubModeDeath3CD(r);
    case 0x83FB:
        return StageSubModeCleared3CD(r);
    case 0x874B:
        return StageSubModeFade3CD(r);
    case 0x87FB:
        return StageSubModeOpening3CD(r);
    case 0x90FD:
        return StageSubModeTally3CD(r);
    default:
        return orig_tail_shim(r, handler);
    }
}

void abi_StageSubModeStep3CD(void)
{
    regs_exit_shim(StageSubModeStep3CD(regs_entry_shim()));
}

/* One of the four pieces that map banks and jump into the overlay at $A000: the JSR to $802D or $8044, the jump. */
static Regs map_and_jump(Regs r, bool banks_5_6, uint16_t ret, uint16_t run)
{
    r = banks_5_6 ? map_banks_5_6(r, ret) : map_bank_7(r, ret);
    return orig_tail_shim(r, run);
}

/*
 * @routine 03CD:038 StageFrameA0003CD
 * @run 6B:8038
 * @unreachable 6B:8038-6B:803D no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8038 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay at $A000's frame at $A000 under banks 5 and 6.
 * HOW: MapOverlayBanks5And6; a jump to $A000.
 * WAITS: as $A000.
 * Entry: nothing. Exit: as $A000 leaves them.
 */
StageRegs StageFrameA0003CD(StageRegs r)
{
    return map_and_jump(r, true, 0x803B, 0xA000);
}

void abi_StageFrameA0003CD(void)
{
    regs_exit_shim(StageFrameA0003CD(regs_entry_shim()));
}

/*
 * @routine 03CD:03E StageFrameA0093CD
 * @run 6B:803E
 * @unreachable 6B:803E-6B:8043 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $803E runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay at $A000's piece at $A009 under banks 5 and 6.
 * HOW: MapOverlayBanks5And6; a jump to $A009.
 * WAITS: as $A009.
 * Entry: nothing. Exit: as $A009 leaves them.
 */
StageRegs StageFrameA0093CD(StageRegs r)
{
    return map_and_jump(r, true, 0x8041, 0xA009);
}

void abi_StageFrameA0093CD(void)
{
    regs_exit_shim(StageFrameA0093CD(regs_entry_shim()));
}

/*
 * @routine 03CD:04C StageFrameA08D3CD
 * @run 6B:804C
 * @unreachable 6B:804C-6B:8051 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $804C runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay at $A000's piece at $A08D under bank 7.
 * HOW: MapOverlayBank7; a jump to $A08D.
 * WAITS: as $A08D.
 * Entry: nothing. Exit: as $A08D leaves them.
 */
StageRegs StageFrameA08D3CD(StageRegs r)
{
    return map_and_jump(r, false, 0x804F, 0xA08D);
}

void abi_StageFrameA08D3CD(void)
{
    regs_exit_shim(StageFrameA08D3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:052 StageFrameA0673CD
 * @run 6B:8052
 * @unreachable 6B:8052-6B:8057 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8052 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The overlay at $A000's piece at $A067 under bank 7.
 * HOW: MapOverlayBank7; a jump to $A067.
 * WAITS: as $A067.
 * Entry: nothing. Exit: as $A067 leaves them.
 */
StageRegs StageFrameA0673CD(StageRegs r)
{
    return map_and_jump(r, false, 0x8055, 0xA067);
}

void abi_StageFrameA0673CD(void)
{
    regs_exit_shim(StageFrameA0673CD(regs_entry_shim()));
}

/*
 * @routine 03CD:058 DemoStart3CD
 * @run 6B:8058
 * @unreachable 6B:8058-6B:8090 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8058 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Starts the demo recorder by the pad held at the stage's start: bit 4 records (mode 1, the buffer's
 * first byte at $A000 cleared under bank $40 above zRamBaseBank), bit 6 plays back (mode $FF); either way the random
 * seed, the wave counter, zModeEvents, zFrameTick and the recorder's bytes are cleared and its pointer set to $A000.
 * HOW: Pad bits at $2229; the mode in zWaitTimer; stores.
 * WAITS: no.
 * Entry: nothing. Exit: A the pad's bit or $A0, N and Z from it; C and V from the add when recording. X and Y kept.
 */
StageRegs DemoStart3CD(StageRegs r)
{
    Sum sum;

    WR8(ZP->wait_timer, 0);
    r = loaded(r, (uint8_t)(rd8(PAD_HELD) & 0x10));
    if (r.a != 0) {
        r.nz = INC8(ZP->wait_timer);
        sum = add_bytes(0x40, RD8(ZP->ram_base_bank), false);
        r = loaded(r, sum.value);
        r.c = sum.carry;
        r.v = sum.overflow;
        tam(0x20, r.a);
        wr8(DEMO_BUFFER, 0);
    } else {
        r = loaded(r, (uint8_t)(rd8(PAD_HELD) & 0x40));
        if (r.a == 0) {
            return r;
        }
        r.nz = DEC8(ZP->wait_timer);
    }
    WR8(ZP->random0, 0);
    WR8(ZP->random1, 0);
    WR8(ZP->random2, 0);
    WR8(ZP->random3, 0);
    WR8(ZP->wave_counter, 0);
    WR8(ZP->mode_events, 0);
    WR8(ZP->frame_tick, 0);
    wr8(DEMO_HELD, 0);
    wr8(DEMO_COUNT, 0);
    wr8(DEMO_POINTER, 0);
    r = loaded(r, 0xA0);
    wr8(DEMO_PAGE, 0xA0);
    return r;
}

void abi_DemoStart3CD(void)
{
    regs_exit_shim(DemoStart3CD(regs_entry_shim()));
}

/* The recorder's two bytes at its pointer cleared (the next record's count and pad). */
static Regs demo_clear_record(Regs r)
{
    r.a = 0;
    wr8_zp_y(DEMO_POINTER, 0, 0);
    r.y = 1;
    r.nz = 1;
    wr8_zp_y(DEMO_POINTER, r.y, 0);
    return r;
}

/*
 * @routine 03CD:0D6 DemoAdvance3CD
 * @run 6B:80D6
 * @unreachable 6B:80D6-6B:80EE no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $80D6 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Moves the demo recorder's pointer to the next record (two bytes); at $E000 the recording ends (mode
 * 0); else the new record is cleared.
 * HOW: A 16-bit increment of the pointer at $7E; a compare of its page.
 * WAITS: no.
 * Entry: nothing. Exit, at the end: N and Z from the page's compare, C set. Else: A 0, Y 1, N and Z from Y.
 */
StageRegs DemoAdvance3CD(StageRegs r)
{
    r.nz = (uint8_t)(rd8(DEMO_POINTER) + 1);
    wr8(DEMO_POINTER, r.nz);
    r.nz = (uint8_t)(rd8(DEMO_POINTER) + 1);
    wr8(DEMO_POINTER, r.nz);
    if (r.nz == 0) {
        r.nz = (uint8_t)(rd8(DEMO_PAGE) + 1);
        wr8(DEMO_PAGE, r.nz);
        r = loaded(r, rd8(DEMO_PAGE));
        r = compared(r, 0xE0);
        if (r.nz == 0) {
            WR8(ZP->wait_timer, 0);
            return r;
        }
    }
    return demo_clear_record(r);
}

void abi_DemoAdvance3CD(void)
{
    regs_exit_shim(DemoAdvance3CD(regs_entry_shim()));
}

/* The demo recorder's pad for the frame: a record is a count and a pad byte; the same pad held counts the record up,
 * a new pad starts the next record. With bit 2 of $222E pressed the recording ends. */
static Regs demo_record(Regs r)
{
    uint8_t pad;

    r = loaded(r, (uint8_t)(rd8(PAD_NEW) & 0x04));
    if (r.a != 0) {
        CALL(0x80D6, DemoAdvance3CD(r));
        WR8(ZP->wait_timer, 0);
        return r;
    }
    r = loaded(r, rd8_zp(DEMO_POINTER));
    if (r.a != 0) {
        r.y = 1;
        r = loaded(r, rd8(BiosPadState));
        pad = rd8_zp_y(DEMO_POINTER, r.y);
        r = compared(r, pad);
        if (r.nz == 0) {
            r = loaded(r, (uint8_t)(rd8_zp(DEMO_POINTER) + 1));
            wr8_zp_y(DEMO_POINTER, 0, r.a);
            return r;
        }
        CALL(0x80C7, DemoAdvance3CD(r));
    }
    r = loaded(r, 0x01);
    wr8_zp_y(DEMO_POINTER, 0, 0x01);
    r.y = 0x01;
    r = loaded(r, rd8(BiosPadState));
    wr8_zp_y(DEMO_POINTER, r.y, r.a);
    return r;
}

/*
 * @routine 03CD:091 DemoStep3CD
 * @run 6B:8091
 * @unreachable 6B:8091-6B:80D5 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8091 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The demo recorder's frame, under banks $40 and $41: mode 0 does nothing; a negative mode plays back
 * (DemoPlay3CD); a positive one records BiosPadState into the buffer (a full record of $FF moves on first).
 * HOW: The mode in zWaitTimer; the add and two TAM; reads and writes through the pointer at $7E; DemoAdvance3CD by BSR.
 * WAITS: no.
 * Entry: nothing. Exit: as the step that ran left them; mode 0: A 0, N and Z from it.
 */
StageRegs DemoStep3CD(StageRegs r)
{
    Sum sum;

    r = loaded(r, RD8(ZP->wait_timer));
    if (r.a == 0) {
        return r;
    }
    sum = add_bytes(0x40, RD8(ZP->ram_base_bank), false);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    tam(0x20, r.a);
    r = loaded(r, (uint8_t)(r.a + 1));
    tam(0x40, r.a);
    r = loaded(r, RD8(ZP->wait_timer));
    if ((r.a & 0x80) != 0) {
        return DemoPlay3CD(r);
    }
    r = loaded(r, rd8_zp(DEMO_POINTER));
    r = compared(r, 0xFF);
    if (r.nz == 0) {
        CALL(0x80AB, DemoAdvance3CD(r));
    }
    return demo_record(r);
}

void abi_DemoStep3CD(void)
{
    regs_exit_shim(DemoStep3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:0EF DemoPlay3CD
 * @run 6B:80EF
 * @unreachable 6B:80EF-6B:8124 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $80EF runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The demo's playback: each record's pad is put in BiosPadState for its count of frames, its new
 * presses into BiosJoyTrigger; a record with a count of 0 ends the playback, and so does the buffer's end at $E000.
 * HOW: The count at $7D, the last pad at $7C; reads through the pointer at $7E; a 16-bit increment.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction that ran left them.
 */
StageRegs DemoPlay3CD(StageRegs r)
{
    uint8_t pad;

    r = loaded(r, rd8(DEMO_COUNT));
    if (r.a == 0) {
        r = loaded(r, rd8_zp(DEMO_POINTER));
        if (r.a == 0) {
            WR8(ZP->wait_timer, 0);
            return r;
        }
        wr8(DEMO_COUNT, r.a);
    }
    r.y = 1;
    r = loaded(r, rd8_zp_y(DEMO_POINTER, r.y));
    wr8(BiosPadState, r.a);
    r = loaded(r, (uint8_t)(rd8(DEMO_HELD) ^ 0xFF));
    pad = rd8(BiosPadState);
    r = loaded(r, (uint8_t)(r.a & pad));
    wr8(BiosJoyTrigger, r.a);
    r = loaded(r, rd8(BiosPadState));
    wr8(DEMO_HELD, r.a);
    r.nz = (uint8_t)(rd8(DEMO_COUNT) - 1);
    wr8(DEMO_COUNT, r.nz);
    if (r.nz != 0) {
        return r;
    }
    r.nz = (uint8_t)(rd8(DEMO_POINTER) + 1);
    wr8(DEMO_POINTER, r.nz);
    r.nz = (uint8_t)(rd8(DEMO_POINTER) + 1);
    wr8(DEMO_POINTER, r.nz);
    if (r.nz != 0) {
        return r;
    }
    r.nz = (uint8_t)(rd8(DEMO_PAGE) + 1);
    wr8(DEMO_PAGE, r.nz);
    r = loaded(r, rd8(DEMO_PAGE));
    r = compared(r, 0xE0);
    if (r.nz != 0) {
        return r;
    }
    WR8(ZP->wait_timer, 0);
    return r;
}

void abi_DemoPlay3CD(void)
{
    regs_exit_shim(DemoPlay3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:125 StageSubModeStart3CD
 * @run 6B:8125
 * @unreachable 6B:8125-6B:8167 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8125 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-mode 0, the stage's start: as StageSubModeStart152D (the kernel's piece at $4006, the route's
 * pieces, the row map, the screen, the overlay at $A000's start, the display, the opening, the level), the pause and
 * the fade cleared, sub-mode 1, zField86 zField83; the demo recorder started (DemoStart3CD). zField8E's bit 0 clear
 * sends the stage to sub-mode 5 with the count at $3310 one less and PlayerByte3BF0 1; else $407B runs unless zStage is
 * negative.
 * HOW: Calls in turn $4006, StageRouteSetUpA3CD, StageRouteSetUpB3CD, StageLoadRowMap3CD, StageResetScroll3CD,
 * MapOverlayBanks5And6, $A003, StageHudStart3CD, StageOpeningStart3CD, StageLevelStart3CD and DemoStart3CD.
 * WAITS: where the pieces it calls wait.
 * Entry: nothing. Exit, sub-mode 5: A 1, N and Z from it. Else: A zStage, N and Z from it, or as $407B leaves them.
 */
StageRegs StageSubModeStart3CD(StageRegs r)
{
    uint8_t value;

    r = orig_shim(r, 0x8128, KERNEL_4006);
    CALL(0x812B, StageRouteSetUpA3CD(r));
    CALL(0x812E, StageRouteSetUpB3CD(r));
    CALL(0x8131, StageLoadRowMap3CD(r));
    CALL(0x8134, StageResetScroll3CD(r));
    r = map_banks_5_6(r, 0x8137);
    r = orig_shim(r, 0x813A, 0xA003);
    CALL(0x813D, StageHudStart3CD(r));
    CALL(0x8140, StageOpeningStart3CD(r));
    CALL(0x8143, StageLevelStart3CD(r));
    WR8(ZP->paused, 0);
    wr8(FADE_ACTIVE, 0);
    r.nz = INC8(ZP->game_sub_mode);
    r = loaded(r, RD8(ZP->field83));
    WR8(ZP->field86, r.a);
    CALL(0x8150, DemoStart3CD(r));
    if ((RD8(ZP->field8e) & 0x01) == 0) {
        r = loaded(r, 5);
        WR8(ZP->game_sub_mode, 5);
        r.nz = DEC8(OBJ->f3300[16]);
        r = loaded(r, 1);
        wr8(PlayerByte3BF0, 1);
        return r;
    }
    value = RD8(ZP->stage);
    r = loaded(r, value);
    if ((value & 0x80) != 0) {
        return r;
    }
    return orig_shim(r, 0x815A, KERNEL_407B);
}

void abi_StageSubModeStart3CD(void)
{
    regs_exit_shim(StageSubModeStart3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:168 StageFadeTimer3CD
 * @run 6B:8168
 * @unreachable 6B:8168-6B:8191 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8168 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The fade's countdown at $9B, as StageFadeTimer152D does it (the same code at this address).
 * HOW: Two decrements of $9B; bit tests of zField8E; the sound hook or cd_fade by a tail jump.
 * WAITS: no.
 * Entry: nothing. Exit: as StageFadeTimer152D.
 */
StageRegs StageFadeTimer3CD(StageRegs r)
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

void abi_StageFadeTimer3CD(void)
{
    regs_exit_shim(StageFadeTimer3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:192 StagePauseToggle3CD
 * @run 6B:8192
 * @unreachable 6B:8192-6B:81AC no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8192 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Pauses or unpauses on Run, as StagePauseToggle152D, and not while zDisplayDelay is set.
 * HOW: $3C XOR 1; BiosJoyTrigger's bit 3; zDisplayDelay; zPaused's bit 1; zPaused XOR 3.
 * WAITS: no.
 * Entry: nothing. Exit: A the last value loaded or computed, N and Z from it. X, Y, C and V kept.
 */
StageRegs StagePauseToggle3CD(StageRegs r)
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
    r = loaded(r, RD8(ZP->display_delay));
    if (r.a != 0) {
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

void abi_StagePauseToggle3CD(void)
{
    regs_exit_shim(StagePauseToggle3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:1D4 StageFrameSkip3CD
 * @run 6B:81D4
 * @unreachable 6B:81D4-6B:81DD no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $81D4 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Lets its caller run one frame in eight: on the others it drops its caller's return address, so its
 * RTS ends the caller too. No code of the overlay calls it.
 * HOW: zFrameTick's low three bits; two PLA.
 * WAITS: no.
 * Entry: nothing. Exit, the eighth frame: A 0, Z set. Else: the frame dropped, A the return address's high byte less
 * one, N and Z from it.
 */
TallyWaitEnd StageFrameSkip3CD(StageRegs r)
{
    TallyWaitEnd end;
    uint8_t byte;

    r = loaded(r, (uint8_t)(RD8(ZP->frame_tick) & 0x07));
    end.dropped = false;
    if (r.a != 0) {
        byte = pull8();
        r = loaded(r, byte);
        byte = pull8();
        r = loaded(r, byte);
        end.dropped = true;
    }
    end.r = r;
    return end;
}

void abi_StageFrameSkip3CD(void)
{
    regs_exit_shim(StageFrameSkip3CD(regs_entry_shim()).r);
}

/* The frame's overlay piece at $A000 and, with no sub-step asked for, the kernel's piece at $40D8: true when the
 * frame goes on to the scroll and the rest, false when a sub-step was asked for. */
static bool frame_and_sub_step_clear(Regs *r)
{
    jsr_push(0x8222);
    *r = StageFrameA0003CD(*r);
    jsr_pop();
    *r = loaded(*r, RD8(ZP->sub_step));
    if (r->a != 0) {
        return false;
    }
    *r = orig_shim(*r, 0x8229, 0x40D8);
    return true;
}

/*
 * @routine 03CD:1DE StageSubModePlay3CD
 * @run 6B:81DE
 * @stop 8250
 * @unreachable 6B:81DE-6B:824F no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $81DE runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-mode 1, the stage in play: the demo recorder, the fade and the pause; paused, the paused frame. Else
 * the events step; the stage's timer PlayerByte3BE8-3BE9 counted down (at its end the kernel's piece at $4015 with A 9
 * and X 7), and while it runs the scroll's steps cleared and, on odd counts, the overlay's frame skipped; unless a
 * sub-step is asked for the kernel's piece at $40D8; then for a stage with zField8E's bit 7 the scroll by
 * StageCenterPlayer3CD, else the overlay's piece at $A08D (and $A009 on even counts); the overlays' frame, the
 * objects' hits and the display; then the sub-step zSubStep held (cleared) from the table at $8250.
 * HOW: Calls; TST of the timer's bit 0; the sub-step doubled indexes the table, whose word is jumped through.
 * WAITS: where the pieces it calls wait.
 * Entry: nothing. Exit: as the sub-step's handler leaves them; paused: as StagePausedFrame3CD.
 */
StageRegs StageSubModePlay3CD(StageRegs r)
{
    uint8_t step, twice, low;
    uint16_t handler;
    bool timed = false;

    CALL(0x81E1, StageSubStepNone152D(r));
    CALL(0x81E4, DemoStep3CD(r));
    CALL(0x81E7, StageFadeTimer3CD(r));
    CALL(0x81EA, StagePauseToggle3CD(r));
    r = loaded(r, RD8(ZP->paused));
    if (r.a != 0) {
        return StagePausedFrame3CD(r);
    }
    CALL(0x81F4, StageModeEventsStep152D(r));
    r = loaded(r, rd8(PlayerByte3BE8));
    low = rd8(PlayerByte3BE9);
    r = loaded(r, (uint8_t)(r.a | low));
    if (r.a != 0) {
        r.nz = (uint8_t)(rd8(PlayerByte3BE8) - 1);
        wr8(PlayerByte3BE8, r.nz);
        if (r.nz == 0) {
            r.nz = (uint8_t)(rd8(PlayerByte3BE9) - 1);
            wr8(PlayerByte3BE9, r.nz);
            if (r.nz == 0) {
                r = loaded(r, 0x09);
                r.x = 0x07;
                r.nz = 0x07;
                r = orig_shim(r, 0x820D, 0x4015);
            }
        }
        WR8(SCROLLE->step_x_hi[0], 0);
        WR8(SCROLLE->step_x_lo[0], 0);
        WR8(SCROLLE->step_y_hi[0], 0);
        WR8(SCROLLE->step_y_lo[0], 0);
        r = tested(r, 0x01, PlayerByte3BE8);
        timed = r.nz != 0;
    }
    if (timed || frame_and_sub_step_clear(&r)) {
        r = loaded(r, RD8(ZP->field8e));
        if ((r.a & 0x80) != 0) {
            CALL(0x8230, StageCenterPlayer3CD(r));
        } else {
            CALL(0x8235, StageFrameA08D3CD(r));
            r = tested(r, 0x01, PlayerByte3BE8);
            if (r.nz == 0) {
                CALL(0x823E, StageFrameA0093CD(r));
            }
        }
        CALL(0x8241, StageOverlayFrame3CD(r));
        CALL(0x8244, StageObjectHits3CD(r));
        CALL(0x8247, StageHudStep3CD(r));
    }
    step = RD8(ZP->sub_step);
    WR8(ZP->sub_step, 0);
    twice = (uint8_t)(step << 1);
    r.c = (step & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(0x8250, twice);
    switch (handler) {
    case 0x825E:
    case 0x825F:
        return StageSubStepNone152D(r);
    case 0x8260:
        return StageSubStepDeath3CD(r);
    case 0x8283:
        return StageSubStepCleared3CD(r);
    case 0x82BD:
        return StageSubStepFadeOut3CD(r);
    case 0x82C8:
        return StageSubStepFadeOutB3CD(r);
    case 0x82D5:
        return StageSubStepTally3CD(r);
    default:
        return orig_tail_shim(r, handler);
    }
}

void abi_StageSubModePlay3CD(void)
{
    regs_exit_shim(StageSubModePlay3CD(regs_entry_shim()));
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
    return StageFadeTimer3CD(r);
}

/*
 * @routine 03CD:260 StageSubStepDeath3CD
 * @run 6B:8260
 * @unreachable 6B:8260-6B:8282 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8260 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-step 2, the player died: sub-mode 2 with zField80, zField89 and the fall's step cleared, the
 * overlay at $A000's piece at $A067 (StageFrameA0673CD), the forced presses cleared; when PlayerByte3BD8 is set it is
 * cleared and the fade starts again from 2.
 * HOW: Stores; StageFrameA0673CD; a jump into StageFadeTimer3CD.
 * WAITS: no.
 * Entry: nothing. Exit: A PlayerByte3BD8 (0), N and Z from it, the rest as $A067 left them; else as StageFadeTimer3CD.
 */
StageRegs StageSubStepDeath3CD(StageRegs r)
{
    r = loaded(r, 2);
    WR8(ZP->game_sub_mode, 2);
    WR8(ZP->field80, 0);
    WR8(ZP->field89, 0);
    CALL(0x826B, StageFrameA0673CD(r));
    wr8(PadForcedPress, 0);
    wr8(PadForcedPressB, 0);
    wr8(FALL_STEP, 0);
    return restart_fade(r);
}

void abi_StageSubStepDeath3CD(void)
{
    regs_exit_shim(StageSubStepDeath3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:283 StageSubStepCleared3CD
 * @run 6B:8283
 * @unreachable 6B:8283-6B:82BC no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8283 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-step 3, the stage cleared: sub-mode 3 with its step 0 and its table at $84 picked by zField8C
 * (none: 2; negative: 4; 3 to 6 or $0F up: 3; else 1, with $2743 set to 1); the overlay at $A000's piece at $A067, the
 * forced presses and PlayerByte3BD8 cleared.
 * HOW: Tests and compares of zField8C; stores; StageFrameA0673CD.
 * WAITS: no.
 * Entry: nothing. Exit: as $A067 left them.
 */
StageRegs StageSubStepCleared3CD(StageRegs r)
{
    uint8_t table;

    r = loaded(r, RD8(ZP->field8c));
    if (r.a == 0) {
        table = 2;
    } else if ((r.a & 0x80) != 0) {
        table = 4;
    } else {
        r.x = 1;
        wr8(STAGE_FLAG_2743, 1);
        r = compared(r, 0x03);
        if (!r.c) {
            table = 1;
        } else {
            r = compared(r, 0x07);
            if (!r.c) {
                table = 3;
            } else {
                r = compared(r, 0x0F);
                table = r.c ? 3 : 1;
            }
        }
    }
    r = loaded(r, table);
    wr8(SUB3_STEP_TABLE, table);
    r = loaded(r, 3);
    WR8(ZP->game_sub_mode, 3);
    wr8(SUB3_STEP, 0);
    CALL(0x82B3, StageFrameA0673CD(r));
    wr8(PadForcedPress, 0);
    wr8(PadForcedPressB, 0);
    wr8(PlayerByte3BD8, 0);
    return r;
}

void abi_StageSubStepCleared3CD(void)
{
    regs_exit_shim(StageSubStepCleared3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:2BD StageSubStepFadeOut3CD
 * @run 6B:82BD
 * @unreachable 6B:82BD-6B:82C7 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $82BD runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-step 4: sub-mode 4 with zField80 and zField89 cleared, then the overlay at $A000's piece at $A067.
 * HOW: Stores; a jump to StageFrameA0673CD.
 * WAITS: no.
 * Entry: nothing. Exit: as $A067 leaves them.
 */
StageRegs StageSubStepFadeOut3CD(StageRegs r)
{
    r = loaded(r, 4);
    WR8(ZP->game_sub_mode, 4);
    WR8(ZP->field80, 0);
    WR8(ZP->field89, 0);
    return StageFrameA0673CD(r);
}

void abi_StageSubStepFadeOut3CD(void)
{
    regs_exit_shim(StageSubStepFadeOut3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:2C8 StageSubStepFadeOutB3CD
 * @run 6B:82C8
 * @unreachable 6B:82C8-6B:82D4 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $82C8 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-step 5: sub-mode 4 entered at its step 5 (zField80 5, zField89 cleared), then the overlay at
 * $A000's piece at $A067.
 * HOW: Stores; a jump to StageFrameA0673CD.
 * WAITS: no.
 * Entry: nothing. Exit: as $A067 leaves them.
 */
StageRegs StageSubStepFadeOutB3CD(StageRegs r)
{
    r = loaded(r, 4);
    WR8(ZP->game_sub_mode, 4);
    r = loaded(r, 5);
    WR8(ZP->field80, 5);
    WR8(ZP->field89, 0);
    return StageFrameA0673CD(r);
}

void abi_StageSubStepFadeOutB3CD(void)
{
    regs_exit_shim(StageSubStepFadeOutB3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:2D5 StageSubStepTally3CD
 * @run 6B:82D5
 * @unreachable 6B:82D5-6B:82FF no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $82D5 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-step 6: sub-mode 6 with zField80, zField89 and the fall's step cleared, the overlay at $A000's piece
 * at $A067, the forced presses cleared, the player's step at $3358 0 and its count at $3310 $40; when PlayerByte3BD8
 * is set it is cleared and the fade starts again from 2.
 * HOW: Stores; StageFrameA0673CD; a jump into StageFadeTimer3CD.
 * WAITS: no.
 * Entry: nothing. Exit: A PlayerByte3BD8 (0), N and Z from it, the rest as $A067 left them; else as StageFadeTimer3CD.
 */
StageRegs StageSubStepTally3CD(StageRegs r)
{
    r = loaded(r, 6);
    WR8(ZP->game_sub_mode, 6);
    WR8(ZP->field80, 0);
    WR8(ZP->field89, 0);
    CALL(0x82E0, StageFrameA0673CD(r));
    wr8(PadForcedPress, 0);
    wr8(PadForcedPressB, 0);
    wr8(FALL_STEP, 0);
    WR8(OBJ->f3358[0], 0);
    r = loaded(r, 0x40);
    WR8(OBJ->f3300[16], 0x40);
    return restart_fade(r);
}

void abi_StageSubStepTally3CD(void)
{
    regs_exit_shim(StageSubStepTally3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:300 StageSubModeDeath3CD
 * @run 6B:8300
 * @unreachable 6B:8300-6B:8317 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8300 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-mode 2, the player's death: sound command $FF, the death's phase; once the phase has put the stage
 * back in sub-mode 1, the overlay at $A000's piece at $A015 and the level's start.
 * HOW: The sound hook; StageDeathPhase3CD; a compare; MapOverlayBanks5And6, $A015 and a jump to StageLevelStart3CD.
 * WAITS: where the pieces it calls wait.
 * Entry: nothing. Exit: A zGameSubMode, N, Z and C from its compare with 1; else as StageLevelStart3CD.
 */
StageRegs StageSubModeDeath3CD(StageRegs r)
{
    r = loaded(r, 0xFF);
    r = orig_shim(r, 0x8305, SOUND_HOOK);
    CALL(0x8308, StageDeathPhase3CD(r));
    r = loaded(r, RD8(ZP->game_sub_mode));
    r = compared(r, 0x01);
    if (r.nz != 0) {
        return r;
    }
    r = map_banks_5_6(r, 0x8312);
    r = orig_shim(r, 0x8315, 0xA015);
    return StageLevelStart3CD(r);
}

void abi_StageSubModeDeath3CD(void)
{
    regs_exit_shim(StageSubModeDeath3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:318 StageDeathPhase3CD
 * @run 6B:8318
 * @stop 831F
 * @unreachable 6B:8318-6B:831E no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8318 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The death's phase by zField87, from the table of four at $831F.
 * HOW: zField87 doubled indexes the table, whose word is jumped through.
 * WAITS: as the phase.
 * Entry: nothing. Exit: as the phase leaves them; it is entered with A and X twice zField87, N and Z from it, C its
 * bit 7.
 */
StageRegs StageDeathPhase3CD(StageRegs r)
{
    uint8_t phase = RD8(ZP->field87);
    uint8_t twice = (uint8_t)(phase << 1);
    uint16_t handler;

    r.c = (phase & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(0x831F, twice);
    switch (handler) {
    case 0x8327:
        return StageDeathRespawn3CD(r);
    case 0x8348:
        return StageDeathRespawnB3CD(r);
    case 0x8367:
        return StageDeathFallLeft3CD(r);
    case 0x83B6:
        return StageDeathFallRight3CD(r);
    default:
        return orig_tail_shim(r, handler);
    }
}

void abi_StageDeathPhase3CD(void)
{
    regs_exit_shim(StageDeathPhase3CD(regs_entry_shim()));
}

/* The player put back: StageResetScroll3CD, sub-mode 1; with zField8C set it is cleared and the count at $3310 $18; the
 * step at $3358 from the table at $8346 by the facing bit of the byte at $2818. */
static Regs respawn(Regs r, uint16_t ret)
{
    uint8_t facing;

    CALL(ret, StageResetScroll3CD(r));
    r = loaded(r, 1);
    WR8(ZP->game_sub_mode, 1);
    r = loaded(r, RD8(ZP->field8c));
    if (r.a != 0) {
        WR8(ZP->field8c, 0);
        r = loaded(r, 0x18);
        WR8(OBJ->f3300[16], 0x18);
    }
    facing = (uint8_t)(RD8(OBJ->f2800[24]) & 0x01);
    r.x = facing;
    r = loaded(r, rd8((uint16_t)(0x8346 + facing)));
    WR8(OBJ->f3358[0], r.a);
    return r;
}

/*
 * @routine 03CD:327 StageDeathRespawn3CD
 * @run 6B:8327
 * @unreachable 6B:8327-6B:8345 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8327 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Phase 0 of the death: the screen and the player put back, sub-mode 1; when zField8C is set it is
 * cleared and the player's count at $3310 is $18; the step at $3358 from the two at $8346 by the facing bit.
 * HOW: StageResetScroll3CD; stores; one table read.
 * WAITS: no.
 * Entry: nothing. Exit: A the step, X the facing bit, N and Z from A; Y, C and V as StageResetScroll3CD left them.
 */
StageRegs StageDeathRespawn3CD(StageRegs r)
{
    return respawn(r, 0x832A);
}

void abi_StageDeathRespawn3CD(void)
{
    regs_exit_shim(StageDeathRespawn3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:348 StageDeathRespawnB3CD
 * @run 6B:8348
 * @unreachable 6B:8348-6B:8366 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8348 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Phase 1 of the death: as phase 0 (the same bytes, the same table at $8346).
 * HOW: StageResetScroll3CD; stores; one table read.
 * WAITS: no.
 * Entry: nothing. Exit: as StageDeathRespawn3CD.
 */
StageRegs StageDeathRespawnB3CD(StageRegs r)
{
    return respawn(r, 0x834B);
}

void abi_StageDeathRespawnB3CD(void)
{
    regs_exit_shim(StageDeathRespawnB3CD(regs_entry_shim()));
}

/* The fall's last step: the player put back, sub-mode 1, zField8C cleared, the count at $3310 $20, the step at $3358. */
static Regs fall_ends(Regs r, uint16_t ret, uint8_t step)
{
    CALL(ret, StageResetScroll3CD(r));
    r = loaded(r, 1);
    WR8(ZP->game_sub_mode, 1);
    WR8(ZP->field8c, 0);
    r = loaded(r, 0x20);
    WR8(OBJ->f3300[16], 0x20);
    r = loaded(r, step);
    WR8(OBJ->f3358[0], step);
    return r;
}

/* The fall's middle step, as in stage_b_census.c: $A01B and $A012; while the player is shown and short of the edge the
 * step is counted, else $A01E runs with X 0. */
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
 * @routine 03CD:367 StageDeathFallLeft3CD
 * @run 6B:8367
 * @unreachable 6B:8367-6B:8378 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8367 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Phase 2 of the death, the fall to the left, as StageDeathFallLeft152D (its bytes, here running into
 * this build's StageDeathFallStart3CD): step 0 sets the player's Y and runs on into StageDeathFallStart3CD; step 1
 * moves the player; step 2 puts it back (StageDeathFallBackLeft3CD).
 * HOW: Two decrements of the step's copy in A; the pieces of the overlay at $A000.
 * WAITS: no.
 * Entry: nothing. Exit: as the step that ran left them.
 */
StageRegs StageDeathFallLeft3CD(StageRegs r)
{
    r = loaded(r, rd8(FALL_STEP));
    r = loaded(r, (uint8_t)(r.a - 1));
    if (r.a == 0) {
        return fall_moves(r, 0x8389, false);
    }
    r = loaded(r, (uint8_t)(r.a - 1));
    if (r.a == 0) {
        return StageDeathFallBackLeft3CD(r);
    }
    r = loaded(r, 0x40);
    WR8(OBJ->y_frac[0], 0x40);
    r = loaded(r, 0x01);
    WR8(OBJ->y_lo[16], 0x01);
    return StageDeathFallStart3CD(r);
}

void abi_StageDeathFallLeft3CD(void)
{
    regs_exit_shim(StageDeathFallLeft3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:379 StageDeathFallStart3CD
 * @run 6B:8379
 * @unreachable 6B:8379-6B:83A1 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8379 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The fall's first step: the overlay at $A000's pieces at $A018 (with A 0) and $A012, and the fall's step
 * at $88 counted.
 * HOW: MapOverlayBanks5And6, $A018, $A012; an increment of $88.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the step's increment; A, X, Y, C and V as $A012 left them.
 */
StageRegs StageDeathFallStart3CD(StageRegs r)
{
    r = map_banks_5_6(r, 0x837C);
    r = loaded(r, 0);
    r = orig_shim(r, 0x8381, 0xA018);
    r = orig_shim(r, 0x8384, 0xA012);
    r.nz = (uint8_t)(rd8(FALL_STEP) + 1);
    wr8(FALL_STEP, r.nz);
    return r;
}

void abi_StageDeathFallStart3CD(void)
{
    regs_exit_shim(StageDeathFallStart3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:3A2 StageDeathFallBackLeft3CD
 * @run 6B:83A2
 * @unreachable 6B:83A2-6B:83B5 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $83A2 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The left fall's last step: the screen and the player put back (StageResetScroll3CD), sub-mode 1,
 * zField8C cleared, the count at $3310 $20 and the step at $3358 $20.
 * HOW: StageResetScroll3CD; stores.
 * WAITS: no.
 * Entry: nothing. Exit: A $20, N and Z from it; X, Y, C and V as StageResetScroll3CD left them.
 */
StageRegs StageDeathFallBackLeft3CD(StageRegs r)
{
    return fall_ends(r, 0x83A5, 0x20);
}

void abi_StageDeathFallBackLeft3CD(void)
{
    regs_exit_shim(StageDeathFallBackLeft3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:3B6 StageDeathFallRight3CD
 * @run 6B:83B6
 * @unreachable 6B:83B6-6B:83FA no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $83B6 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Phase 3 of the death, the fall to the right: step 0 sets the player's Y ($C0, $FE), calls
 * StageDeathFallStart3CD and moves the player as step 1 does; step 1 moves it while it is shown and left of X $F0;
 * step 2 puts it back (StageResetScroll3CD, sub-mode 1, zField8C cleared, the count $20, the step $80).
 * HOW: Two decrements of the step's copy in A; the pieces of the overlay at $A000.
 * WAITS: no.
 * Entry: nothing. Exit: as the step that ran left them.
 */
StageRegs StageDeathFallRight3CD(StageRegs r)
{
    r = loaded(r, rd8(FALL_STEP));
    r = loaded(r, (uint8_t)(r.a - 1));
    if (r.a == 0) {
        return fall_moves(r, 0x83CE, true);
    }
    r = loaded(r, (uint8_t)(r.a - 1));
    if (r.a == 0) {
        return fall_ends(r, 0x83EA, 0x80);
    }
    r = loaded(r, 0xC0);
    WR8(OBJ->y_frac[0], 0xC0);
    r = loaded(r, 0xFE);
    WR8(OBJ->y_lo[16], 0xFE);
    CALL(0x83CB, StageDeathFallStart3CD(r));
    return fall_moves(r, 0x83CE, true);
}

void abi_StageDeathFallRight3CD(void)
{
    regs_exit_shim(StageDeathFallRight3CD(regs_entry_shim()));
}

/* A table of this build's steps by an index in RAM: the index doubled, the word, the handler's C. */
typedef StageRegs (*StepHandler)(StageRegs r);
typedef struct { uint16_t run; StepHandler c; } StepEntry;

static Regs step_by_table(Regs r, uint8_t step, uint16_t table, const StepEntry *entries, int count)
{
    uint8_t twice = (uint8_t)(step << 1);
    uint16_t handler;

    r.c = (step & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(table, twice);
    for (int i = 0; i < count; i++) {
        if (entries[i].run == handler) {
            return entries[i].c(r);
        }
    }
    return orig_tail_shim(r, handler);
}

/*
 * @routine 03CD:3FB StageSubModeCleared3CD
 * @run 6B:83FB
 * @stop 8405
 * @unreachable 6B:83FB-6B:8404 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $83FB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-mode 3, the stage cleared: the events step, then the step of the table $84 picked from the five at
 * $8405 (the next stage's start, the walk out, the walk out after the boss, the last walk, the stage's end).
 * HOW: StageModeEventsStep152D (its bytes at $81AD); $84 doubled indexes the table, whose word is jumped through.
 * WAITS: as the step.
 * Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice $84, N and Z from it, C its bit 7.
 */
StageRegs StageSubModeCleared3CD(StageRegs r)
{
    static const StepEntry steps[] = {
        { 0x840F, StageClearedNextStage3CD }, { 0x8455, StageClearedWalkOut3CD }, { 0x85A3, StageClearedBossWalk3CD },
        { 0x8637, StageClearedLastWalk3CD }, { 0x86BB, StageClearedEnding3CD }
    };

    CALL(0x83FE, StageModeEventsStep152D(r));
    return step_by_table(r, rd8(SUB3_STEP_TABLE), 0x8405, steps, 5);
}

void abi_StageSubModeCleared3CD(void)
{
    regs_exit_shim(StageSubModeCleared3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:40F StageClearedNextStage3CD
 * @run 6B:840F
 * @unreachable 6B:840F-6B:8438 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $840F runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The cleared stage's first table: the next stage set up as the start does (the kernel's piece at $4009,
 * the route's pieces, the row map, the screen, the display, zField8A 2, the overlay at $A000's start, the level) and
 * put in play, sub-mode 1, unpaused, zField86 zField83.
 * HOW: Calls in turn $4009, StageRouteSetUpA3CD, StageRouteSetUpB3CD, StageLoadRowMap3CD, StageResetScroll3CD,
 * StageHudStart3CD, MapOverlayBanks5And6, $A003 and StageLevelStart3CD.
 * WAITS: where the pieces it calls wait.
 * Entry: nothing. Exit: A zField83, N and Z from it; X, Y, C and V as StageLevelStart3CD left them.
 */
StageRegs StageClearedNextStage3CD(StageRegs r)
{
    r = orig_shim(r, 0x8412, KERNEL_4009);
    CALL(0x8415, StageRouteSetUpA3CD(r));
    CALL(0x8418, StageRouteSetUpB3CD(r));
    CALL(0x841B, StageLoadRowMap3CD(r));
    CALL(0x841E, StageResetScroll3CD(r));
    CALL(0x8421, StageHudStart3CD(r));
    r = loaded(r, 2);
    WR8(ZP->field8a, 2);
    r = map_banks_5_6(r, 0x8428);
    r = orig_shim(r, 0x842B, 0xA003);
    CALL(0x842E, StageLevelStart3CD(r));
    WR8(ZP->paused, 0);
    r = loaded(r, 1);
    WR8(ZP->game_sub_mode, 1);
    r = loaded(r, RD8(ZP->field83));
    WR8(ZP->field86, r.a);
    return r;
}

void abi_StageClearedNextStage3CD(void)
{
    regs_exit_shim(StageClearedNextStage3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:445 StageWalkSprite3CD
 * @run 6B:8445
 * @unreachable 6B:8445-6B:8452 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8445 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The walking player's sprite at $2A58 by the character (the two at $8453), then the overlay at $A000's
 * piece at $A012.
 * HOW: MapOverlayBanks5And6; a table read; a jump to $A012.
 * WAITS: no.
 * Entry: nothing. Exit: as $A012 leaves them.
 */
StageRegs StageWalkSprite3CD(StageRegs r)
{
    r = map_banks_5_6(r, 0x8448);
    r.x = RD8(ZP->character);
    r = loaded(r, rd8((uint16_t)(0x8453 + r.x)));
    WR8(OBJ->sprite_set[72], r.a);
    return orig_tail_shim(r, 0xA012);
}

void abi_StageWalkSprite3CD(void)
{
    regs_exit_shim(StageWalkSprite3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:455 StageClearedWalkOut3CD
 * @run 6B:8455
 * @stop 8462
 * @unreachable 6B:8455-6B:8461 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8455 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The cleared stage's second table: the kernel's piece at $4096 under bank 7, then its step at $85 from
 * the five at $8462 (the wait for the objects, the walk, the scroll's wait, the walk back, the walk on).
 * HOW: MapOverlayBank7; $4096; $85 doubled indexes the table, whose word is jumped through.
 * WAITS: as the step.
 * Entry: nothing. Exit: as the step leaves them.
 */
StageRegs StageClearedWalkOut3CD(StageRegs r)
{
    static const StepEntry steps[] = {
        { 0x846C, StageClearedWaitObjects3CD }, { 0x848A, StageClearedWalk3CD }, { 0x84C4, StageClearedWaitScroll3CD },
        { 0x84CE, StageClearedWalkBack3CD }, { 0x8511, StageClearedWalkOn3CD }
    };

    r = map_bank_7(r, 0x8458);
    r = orig_shim(r, 0x845B, 0x4096);
    return step_by_table(r, rd8(SUB3_STEP), 0x8462, steps, 5);
}

void abi_StageClearedWalkOut3CD(void)
{
    regs_exit_shim(StageClearedWalkOut3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:46C StageClearedWaitObjects3CD
 * @run 6B:846C
 * @unreachable 6B:846C-6B:8489 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $846C runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step of the cleared stage: the walking sprite, the overlay at $A000's pieces at $A009 and $A08D and
 * the overlays' frame; once no object slot from $40 down to 1 holds a state from 1 to $7F at $27D0, $2743 cleared and
 * the step counted.
 * HOW: StageWalkSprite3CD, StageFrameA0093CD, StageFrameA08D3CD, StageOverlayFrame3CD; a loop down X.
 * WAITS: no.
 * Entry: nothing. Exit, an object busy: A its state, X its slot, N and Z from A. Else: X 0, N and Z from the step's
 * increment.
 */
StageRegs StageClearedWaitObjects3CD(StageRegs r)
{
    uint8_t slot;

    CALL(0x846F, StageWalkSprite3CD(r));
    CALL(0x8472, StageFrameA0093CD(r));
    CALL(0x8475, StageFrameA08D3CD(r));
    CALL(0x8478, StageOverlayFrame3CD(r));
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

void abi_StageClearedWaitObjects3CD(void)
{
    regs_exit_shim(StageClearedWaitObjects3CD(regs_entry_shim()));
}

/* zField8C less one, its low bit: which of the two values of the overlay's small tables at $8439 the walk takes. */
static uint8_t walk_side(void)
{
    return (uint8_t)((uint8_t)(RD8(ZP->field8c) - 1) & 0x01);
}

/* The walk's push: the scroll's X step from the table at $8439, its fraction cleared; Y the side. */
static Regs walk_push(Regs r)
{
    uint8_t side = walk_side();

    r.y = side;
    r = loaded(r, rd8((uint16_t)(0x8439 + side)));
    WR8(SCROLLE->step_x_hi[0], r.a);
    WR8(SCROLLE->step_x_lo[0], 0);
    return r;
}

/* The player's X moved by the table at $843B by the side. */
static Regs walk_move_x(Regs r, uint8_t side)
{
    Sum sum;
    uint8_t x = RD8(OBJ->x_hi[8]);
    uint8_t step = rd8((uint16_t)(0x843B + side));

    sum = add_bytes(x, step, false);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(OBJ->x_hi[8], sum.value);
    return r;
}

/*
 * @routine 03CD:48A StageClearedWalk3CD
 * @run 6B:848A
 * @unreachable 6B:848A-6B:84C3 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $848A runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step of the cleared stage: the player walks the scroll ($3E86 the step from $8439 by the side, $3E7E
 * and $3E00 cleared, its X moved by $843B); at the side's end ($843D) the overlay's piece at $A067 runs, $3E86 is
 * cleared and the step counted.
 * HOW: The side from zField8C; table reads, an add; StageFrameA08D3CD, StageOverlayFrame3CD; a compare.
 * WAITS: no.
 * Entry: nothing. Exit, short of the end: A the X, N, Z and C from the compare, Y the side. At the end: N and Z from
 * the step's increment, the rest as $A067 left them.
 */
StageRegs StageClearedWalk3CD(StageRegs r)
{
    uint8_t side;

    r = walk_push(r);
    WR8(SCROLL->x_frac[0], 0);
    r = walk_move_x(r, r.y);
    CALL(0x84A9, StageFrameA08D3CD(r));
    CALL(0x84AC, StageOverlayFrame3CD(r));
    side = walk_side();
    r.y = side;
    r = loaded(r, RD8(OBJ->x_hi[8]));
    r = compared(r, rd8((uint16_t)(0x843D + side)));
    if (r.nz != 0) {
        return r;
    }
    CALL(0x84BE, StageFrameA0673CD(r));
    WR8(SCROLLE->step_x_hi[0], 0);
    r.nz = (uint8_t)(rd8(SUB3_STEP) + 1);
    wr8(SUB3_STEP, r.nz);
    return r;
}

void abi_StageClearedWalk3CD(void)
{
    regs_exit_shim(StageClearedWalk3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:4C4 StageClearedWaitScroll3CD
 * @run 6B:84C4
 * @unreachable 6B:84C4-6B:84CD no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $84C4 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step of the cleared stage: the overlays' frame; once zField8C is negative the step is counted.
 * HOW: StageOverlayFrame3CD; a test; an increment.
 * WAITS: no.
 * Entry: nothing. Exit: A zField8C, N and Z from it or from the step's increment.
 */
StageRegs StageClearedWaitScroll3CD(StageRegs r)
{
    CALL(0x84C7, StageOverlayFrame3CD(r));
    r = loaded(r, RD8(ZP->field8c));
    if ((r.a & 0x80) == 0) {
        return r;
    }
    r.nz = (uint8_t)(rd8(SUB3_STEP) + 1);
    wr8(SUB3_STEP, r.nz);
    return r;
}

void abi_StageClearedWaitScroll3CD(void)
{
    regs_exit_shim(StageClearedWaitScroll3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:4CE StageClearedWalkBack3CD
 * @run 6B:84CE
 * @unreachable 6B:84CE-6B:8510 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $84CE runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step of the cleared stage: the player walks back (count $E0, step from $8441 by the side, the
 * overlay at $A000's frame, its X moved back by $843B); at the side's mark ($843F) zField8C loses bit 7, the player
 * stops, zField8A is 2, the overlay's piece at $A003 and the walking sprite run and the step is counted.
 * HOW: The side from zField8C; table reads; StageFrameA0003CD; a subtract and a compare; stores; $A003,
 * StageWalkSprite3CD.
 * WAITS: no.
 * Entry: nothing. Exit, short of the mark: A the X, N, Z and C from the compare, V from the subtract, Y the side. At
 * the mark: N and Z from the step's increment.
 */
StageRegs StageClearedWalkBack3CD(StageRegs r)
{
    Sum sum;
    uint8_t side, x, step;

    r = loaded(r, 0xE0);
    WR8(OBJ->f3300[16], 0xE0);
    side = walk_side();
    r.y = side;
    r = loaded(r, rd8((uint16_t)(0x8441 + side)));
    WR8(OBJ->f3358[0], r.a);
    CALL(0x84E2, StageFrameA0003CD(r));
    side = walk_side();
    r.y = side;
    x = RD8(OBJ->x_hi[8]);
    step = rd8((uint16_t)(0x843B + side));
    sum = sub_bytes(x, step, true);
    r = loaded(r, sum.value);
    r.v = sum.overflow;
    WR8(OBJ->x_hi[8], sum.value);
    r = compared(r, rd8((uint16_t)(0x843F + side)));
    if (r.nz != 0) {
        return r;
    }
    r = loaded(r, (uint8_t)(RD8(ZP->field8c) & 0x7F));
    WR8(ZP->field8c, r.a);
    WR8(OBJ->f3300[16], 0);
    WR8(OBJ->f3358[0], 0);
    r = loaded(r, 2);
    WR8(ZP->field8a, 2);
    r = orig_shim(r, 0x850B, 0xA003);
    CALL(0x850E, StageWalkSprite3CD(r));
    r.nz = (uint8_t)(rd8(SUB3_STEP) + 1);
    wr8(SUB3_STEP, r.nz);
    return r;
}

void abi_StageClearedWalkBack3CD(void)
{
    regs_exit_shim(StageClearedWalkBack3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:511 StageClearedWalkOn3CD
 * @run 6B:8511
 * @unreachable 6B:8511-6B:85A2 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8511 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The cleared stage's last step: the player walks on with the scroll until ScrollXLo is 0; then the next
 * stage: below zField8C 7 the route's next stage from the table at $9D60 (zField82 counted, into zField83 and zField86),
 * else zField83 counted with zField8E's bit 4 set and sound $FF; zField8E's bit 1 set; the route's pieces below 7, the
 * row map, the scroll's bytes at $3E00 (48) and $3E66 (136) cleared, the screen loaded (with zField8E kept, and again
 * under bank 7 after $A74E for camera mode $0D); bit 1 cleared; the level started unless zField8C is 7 or 8; zField8C
 * and the pause cleared, sub-mode 1.
 * HOW: As StageClearedWalk3CD for the walk; calls, TAI, bit changes.
 * WAITS: where the pieces it calls wait.
 * Entry: nothing. Exit, walking: A ScrollXLo, N and Z from it. Else: A 1, N and Z from it.
 */
StageRegs StageClearedWalkOn3CD(StageRegs r)
{
    uint8_t flags;
    bool start;

    r = walk_push(r);
    r = walk_move_x(r, r.y);
    CALL(0x852D, StageFrameA08D3CD(r));
    CALL(0x8530, StageOverlayFrame3CD(r));
    r = loaded(r, RD8(SCROLL->x_lo[0]));
    if (r.a != 0) {
        return r;
    }
    r = loaded(r, RD8(ZP->field8c));
    r = compared(r, 0x07);
    if (!r.c) {
        r.nz = INC8(ZP->field82);
        r = loaded(r, RD8(ZP->field82));
        r.y = r.a;
        r = loaded(r, rd8((uint16_t)(0x9D60 + r.y)));
        WR8(ZP->field83, r.a);
        WR8(ZP->field86, r.a);
    } else {
        r.nz = INC8(ZP->field83);
        RMW8(ZP->field8e, old, old | 0x10);
        r = loaded(r, 0xFF);
        r = orig_shim(r, 0x8553, SOUND_HOOK);
    }
    RMW8(ZP->field8e, old, old | 0x02);
    r = loaded(r, RD8(ZP->field8c));
    r = compared(r, 0x07);
    if (!r.c) {
        CALL(0x855E, StageRouteSetUpA3CD(r));
        CALL(0x8561, StageRouteSetUpB3CD(r));
    }
    CALL(0x8564, StageLoadRowMap3CD(r));
    tai_shim(r, KernelZeros, ADDR_OF8(SCROLL->x_frac[0]), 0x0030);
    tai_shim(r, KernelZeros, ADDR_OF8(SCROLLE->f3e66[0]), 0x0088);
    r = loaded(r, RD8(ZP->field8e));
    push8(r.a);
    CALL(0x8578, StageLoadScreen3CD(r));
    flags = pull8();
    r = loaded(r, flags);
    WR8(ZP->field8e, flags);
    r = loaded(r, RD8(SCROLLLIM->camera_mode));
    r = compared(r, 0x0D);
    if (r.nz == 0) {
        r = map_bank_7(r, 0x8585);
        r = orig_shim(r, 0x8588, 0xA74E);
        CALL(0x858B, StageLoadScreen3CD(r));
    }
    RMW8(ZP->field8e, old, old & (uint8_t)~0x02);
    r = loaded(r, RD8(ZP->field8c));
    r = compared(r, 0x09);
    start = r.c;
    if (!start) {
        r = compared(r, 0x07);
        start = !r.c;
    }
    if (start) {
        CALL(0x859A, StageLevelStart3CD(r));
    }
    WR8(ZP->field8c, 0);
    WR8(ZP->paused, 0);
    r = loaded(r, 1);
    WR8(ZP->game_sub_mode, 1);
    return r;
}

void abi_StageClearedWalkOn3CD(void)
{
    regs_exit_shim(StageClearedWalkOn3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:5A3 StageClearedBossWalk3CD
 * @run 6B:85A3
 * @stop 85AD
 * @unreachable 6B:85A3-6B:85AC no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $85A3 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The cleared stage's third table, after a boss: the fade's countdown, then its step at $85 from the
 * three at $85AD (the scroll to the boss's door, the wait for the door, the walk out).
 * HOW: StageFadeTimer3CD; $85 doubled indexes the table, whose word is jumped through.
 * WAITS: as the step.
 * Entry: nothing. Exit: as the step leaves them.
 */
StageRegs StageClearedBossWalk3CD(StageRegs r)
{
    static const StepEntry steps[] = {
        { 0x85B3, StageBossScrollToDoor3CD }, { 0x85E3, StageBossWaitDoor3CD }, { 0x85FD, StageBossWalkOut3CD }
    };

    CALL(0x85A6, StageFadeTimer3CD(r));
    return step_by_table(r, rd8(SUB3_STEP), 0x85AD, steps, 3);
}

void abi_StageClearedBossWalk3CD(void)
{
    regs_exit_shim(StageClearedBossWalk3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:5B3 StageBossScrollToDoor3CD
 * @run 6B:85B3
 * @unreachable 6B:85B3-6B:85E2 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $85B3 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step after the boss: the walking sprite and the overlay at $A000's piece at $A009, the scroll stepped
 * by 1 with the player's X one less, the overlays' frame; once ScrollXHi is 4 the overlay's piece at $A067 runs, the
 * scroll's steps are cleared, sound $58 is sent and the step counted.
 * HOW: StageWalkSprite3CD, StageFrameA0093CD; stores and a decrement; StageFrameA08D3CD, StageOverlayFrame3CD; a
 * compare; StageFrameA0673CD, the sound hook.
 * WAITS: no.
 * Entry: nothing. Exit, short of 4: A ScrollXHi, N, Z and C from the compare. Else: N and Z from the step's increment.
 */
StageRegs StageBossScrollToDoor3CD(StageRegs r)
{
    CALL(0x85B6, StageWalkSprite3CD(r));
    CALL(0x85B9, StageFrameA0093CD(r));
    r = loaded(r, 1);
    WR8(SCROLLE->step_x_hi[0], 1);
    WR8(SCROLLE->step_x_lo[0], 0);
    r.nz = DEC8(OBJ->x_hi[8]);
    CALL(0x85C7, StageFrameA08D3CD(r));
    CALL(0x85CA, StageOverlayFrame3CD(r));
    r = loaded(r, RD8(SCROLL->x_hi[0]));
    r = compared(r, 0x04);
    if (r.nz != 0) {
        return r;
    }
    CALL(0x85D5, StageFrameA0673CD(r));
    WR8(SCROLLE->step_x_hi[0], 0);
    WR8(SCROLLE->step_x_lo[2], 0);
    r = loaded(r, 0x58);
    r = orig_shim(r, 0x85E0, SOUND_HOOK);
    r.nz = (uint8_t)(rd8(SUB3_STEP) + 1);
    wr8(SUB3_STEP, r.nz);
    return r;
}

void abi_StageBossScrollToDoor3CD(void)
{
    regs_exit_shim(StageBossScrollToDoor3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:5E3 StageBossWaitDoor3CD
 * @run 6B:85E3
 * @unreachable 6B:85E3-6B:85FC no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $85E3 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step after the boss: the overlay at $A000's piece at $A009 and the overlays' frame; once the door's
 * byte at $283A is 4 and its animation step at $340A is 0, the player's step at $3358 is $20 and the step counted.
 * HOW: StageFrameA0093CD, StageOverlayFrame3CD; two tests; a store and an increment.
 * WAITS: no.
 * Entry: nothing. Exit: A the last byte tested, N and Z from its test, or $20 with N and Z from the increment.
 */
StageRegs StageBossWaitDoor3CD(StageRegs r)
{
    CALL(0x85E6, StageFrameA0093CD(r));
    CALL(0x85E9, StageOverlayFrame3CD(r));
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

void abi_StageBossWaitDoor3CD(void)
{
    regs_exit_shim(StageBossWaitDoor3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:5FD StageBossWalkOut3CD
 * @run 6B:85FD
 * @unreachable 6B:85FD-6B:8618 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $85FD runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The last step after the boss: the player walks out (count $B8) with the overlay at $A000's frame, the
 * kernel's piece at $40D8 and the piece at $A009; at X $F0 the route's next stage (StageNextRouteStage3CD).
 * HOW: StageFrameA0003CD, $40D8, $A009, StageFrameA08D3CD, StageOverlayFrame3CD; a compare; a branch to
 * StageNextRouteStage3CD.
 * WAITS: no.
 * Entry: nothing. Exit, short of $F0: A the X, N, Z and C from the compare. Else: as StageNextRouteStage3CD.
 */
StageRegs StageBossWalkOut3CD(StageRegs r)
{
    r = loaded(r, 0xB8);
    WR8(OBJ->f3300[16], 0xB8);
    CALL(0x8602, StageFrameA0003CD(r));
    r = orig_shim(r, 0x8605, 0x40D8);
    r = orig_shim(r, 0x8608, 0xA009);
    CALL(0x860B, StageFrameA08D3CD(r));
    CALL(0x860E, StageOverlayFrame3CD(r));
    r = loaded(r, RD8(OBJ->x_hi[8]));
    r = compared(r, 0xF0);
    if (r.nz != 0) {
        return r;
    }
    return StageNextRouteStage3CD(r);
}

void abi_StageBossWalkOut3CD(void)
{
    regs_exit_shim(StageBossWalkOut3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:619 StageNextRouteStage3CD
 * @run 6B:8619
 * @unreachable 6B:8619-6B:8636 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8619 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The route's next stage: zDisplayDelay 2, the player stopped, $2743 cleared, zField82 counted and the
 * stage from the table at $9D60 into zField83, $84 and the byte at $2A58 cleared.
 * HOW: Stores; an increment; a table read.
 * WAITS: no.
 * Entry: nothing. Exit: A 0 (CLA keeps the flags), N and Z from the table's load, Y zField82.
 */
StageRegs StageNextRouteStage3CD(StageRegs r)
{
    r = loaded(r, 0x02);
    WR8(ZP->display_delay, 0x02);
    WR8(OBJ->f3300[16], 0);
    wr8(STAGE_FLAG_2743, 0);
    WR8(OBJ->f3358[0], 0);
    r.nz = INC8(ZP->field82);
    r = loaded(r, RD8(ZP->field82));
    r.y = r.a;
    r = loaded(r, rd8((uint16_t)(0x9D60 + r.y)));
    WR8(ZP->field83, r.a);
    r.a = 0;
    wr8(SUB3_STEP_TABLE, 0);
    WR8(OBJ->sprite_set[72], 0);
    return r;
}

void abi_StageNextRouteStage3CD(void)
{
    regs_exit_shim(StageNextRouteStage3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:637 StageClearedLastWalk3CD
 * @run 6B:8637
 * @stop 8644
 * @unreachable 6B:8637-6B:8643 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8637 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The cleared stage's fourth table: the kernel's piece at $4096 under bank 7, then its step at $85 from
 * the five at $8644 (the wait for the objects, a frame, the scroll's wait, zField8C's bit 7 cleared, the walk off).
 * HOW: MapOverlayBank7; $4096; $85 doubled indexes the table, whose word is jumped through.
 * WAITS: as the step.
 * Entry: nothing. Exit: as the step leaves them.
 */
StageRegs StageClearedLastWalk3CD(StageRegs r)
{
    static const StepEntry steps[] = {
        { 0x846C, StageClearedWaitObjects3CD }, { 0x864E, StageClearedFrame3CD }, { 0x84C4, StageClearedWaitScroll3CD },
        { 0x8654, StageClearedClearBit3CD }, { 0x8660, StageClearedWalkOff3CD }
    };

    r = map_bank_7(r, 0x863A);
    r = orig_shim(r, 0x863D, 0x4096);
    return step_by_table(r, rd8(SUB3_STEP), 0x8644, steps, 5);
}

void abi_StageClearedLastWalk3CD(void)
{
    regs_exit_shim(StageClearedLastWalk3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:64E StageClearedFrame3CD
 * @run 6B:864E
 * @unreachable 6B:864E-6B:8653 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $864E runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step of the last walk: the overlays' frame and the step counted.
 * HOW: StageOverlayFrame3CD; an increment.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the increment; the rest as StageOverlayFrame3CD left them.
 */
StageRegs StageClearedFrame3CD(StageRegs r)
{
    CALL(0x8651, StageOverlayFrame3CD(r));
    r.nz = (uint8_t)(rd8(SUB3_STEP) + 1);
    wr8(SUB3_STEP, r.nz);
    return r;
}

void abi_StageClearedFrame3CD(void)
{
    regs_exit_shim(StageClearedFrame3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:654 StageClearedClearBit3CD
 * @run 6B:8654
 * @unreachable 6B:8654-6B:865F no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8654 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: A step of the last walk: the overlays' frame, zField8C's bit 7 cleared and the step counted.
 * HOW: StageOverlayFrame3CD; a mask; an increment.
 * WAITS: no.
 * Entry: nothing. Exit: A zField8C, N and Z from the step's increment.
 */
StageRegs StageClearedClearBit3CD(StageRegs r)
{
    CALL(0x8657, StageOverlayFrame3CD(r));
    r = loaded(r, (uint8_t)(RD8(ZP->field8c) & 0x7F));
    WR8(ZP->field8c, r.a);
    r.nz = (uint8_t)(rd8(SUB3_STEP) + 1);
    wr8(SUB3_STEP, r.nz);
    return r;
}

void abi_StageClearedClearBit3CD(void)
{
    regs_exit_shim(StageClearedClearBit3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:660 StageClearedWalkOff3CD
 * @run 6B:8660
 * @unreachable 6B:8660-6B:86BA no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8660 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The last walk off the screen: the overlays' frame; the player's step and count from $8441 by the side,
 * the overlay at $A000's frame, its X moved by $8439 (its page at $2B30 too when it wraps); at the side's mark ($8443):
 * below zField8C $0F zModeDone is counted; from $0F up the stage goes on to the room zField8C less one over 16 and the
 * entry 2 or 3 by the side, through StageSubStepDeath3CD.
 * HOW: StageOverlayFrame3CD; table reads; StageFrameA0003CD; adds and a compare; shifts; a jump to
 * StageSubStepDeath3CD.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction or piece that ran left them.
 */
StageRegs StageClearedWalkOff3CD(StageRegs r)
{
    Sum sum;
    uint8_t side, step, value;

    CALL(0x8663, StageOverlayFrame3CD(r));
    side = walk_side();
    r.y = side;
    r = loaded(r, rd8((uint16_t)(0x8441 + side)));
    WR8(OBJ->f3358[0], r.a);
    WR8(OBJ->f3300[16], r.a);
    CALL(0x8675, StageFrameA0003CD(r));
    side = walk_side();
    r.y = side;
    step = rd8((uint16_t)(0x8439 + side));
    value = RD8(OBJ->x_hi[8]);
    sum = add_bytes(step, value, false);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(OBJ->x_hi[8], sum.value);
    if (sum.value == 0) {
        step = rd8((uint16_t)(0x8439 + side));
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
    r = compared(r, rd8((uint16_t)(0x8443 + side)));
    if (r.nz != 0) {
        return r;
    }
    r = loaded(r, RD8(ZP->field8c));
    r = compared(r, 0x0F);
    if (!r.c) {
        r.nz = INC8(ZP->mode_done);
        return r;
    }
    value = (uint8_t)(r.a - 1);
    r.c = (value & 0x08) != 0;
    r = loaded(r, (uint8_t)(value >> 4));
    WR8(ZP->field83, r.a);
    r = loaded(r, (uint8_t)(((uint8_t)(RD8(ZP->field8c) - 1) & 0x01) | 0x02));
    WR8(ZP->field87, r.a);
    return StageSubStepDeath3CD(r);
}

void abi_StageClearedWalkOff3CD(void)
{
    regs_exit_shim(StageClearedWalkOff3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:6BB StageClearedEnding3CD
 * @run 6B:86BB
 * @unreachable 6B:86BB-6B:86D5 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $86BB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The cleared stage's fifth table: sound $FF and the overlay at $A000's piece at $A015; zField8C keeps
 * only its low seven bits; when they are 0 the route goes on (StageNextRouteStage3CD), else zModeDone is counted.
 * HOW: The sound hook; MapOverlayBanks5And6, $A015; a mask; a jump to StageNextRouteStage3CD.
 * WAITS: no.
 * Entry: nothing. Exit: as StageNextRouteStage3CD, or A zField8C's bits, N and Z from zModeDone's increment.
 */
StageRegs StageClearedEnding3CD(StageRegs r)
{
    r = loaded(r, 0xFF);
    r = orig_shim(r, 0x86C0, SOUND_HOOK);
    r = map_banks_5_6(r, 0x86C3);
    r = orig_shim(r, 0x86C6, 0xA015);
    r = loaded(r, (uint8_t)(RD8(ZP->field8c) & 0x7F));
    if (r.a == 0) {
        WR8(ZP->field8c, 0);
        return StageNextRouteStage3CD(r);
    }
    WR8(ZP->field8c, r.a);
    r.nz = INC8(ZP->mode_done);
    return r;
}

void abi_StageClearedEnding3CD(void)
{
    regs_exit_shim(StageClearedEnding3CD(regs_entry_shim()));
}

/* The player's X steps toward the screen's middle: the scroll's X step at $3E86 ($FC or 4 by the side) and the X
 * moved the other way by 4. */
static Regs center_step(Regs r)
{
    Sum sum;
    uint8_t x;

    r = loaded(r, RD8(OBJ->x_hi[8]));
    if ((r.a & 0x80) == 0) {
        r = loaded(r, 0xFC);
        WR8(SCROLLE->step_x_hi[0], 0xFC);
        WR8(SCROLLE->step_x_lo[0], 0);
        x = RD8(OBJ->x_hi[8]);
        sum = add_bytes(x, 0x04, false);
    } else {
        r = loaded(r, 0x04);
        WR8(SCROLLE->step_x_hi[0], 0x04);
        WR8(SCROLLE->step_x_lo[0], 0);
        x = RD8(OBJ->x_hi[8]);
        sum = add_bytes(x, 0xFC, false);
    }
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(OBJ->x_hi[8], sum.value);
    return r;
}

/* The player's X near the middle: X plus 2 masked with MASK compared with $80. */
static Regs center_near(Regs r, uint8_t mask)
{
    Sum sum = add_bytes(RD8(OBJ->x_hi[8]), 0x02, false);

    r = loaded(r, (uint8_t)(sum.value & mask));
    r.v = sum.overflow;
    return compared(r, 0x80);
}

/* The centring done: zField8E's bit 7, the count at $3310 and PlayerByte3BF0 cleared. */
static Regs center_done(Regs r)
{
    RMW8(ZP->field8e, old, old & (uint8_t)~0x80);
    WR8(OBJ->f3300[16], 0);
    wr8(PlayerByte3BF0, 0);
    return r;
}

/*
 * @routine 03CD:6D6 StageCenterPlayer3CD
 * @run 6B:86D6
 * @unreachable 6B:86D6-6B:874A no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $86D6 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The play frame's scroll while zField8E's bit 7 is set: the player's count at $3310 $80 and its step
 * cleared, the overlay at $A000's piece at $A009. Unless the byte at $2860 is set or the player's state at $2818 is 2 or
 * more, PlayerByte3BF0 is 1 and the player is walked to the screen's middle ($80) by 4 a frame with the scroll moving
 * the other way and the overlay's piece at $A08D; there the centring ends.
 * HOW: Tests; StageFrameA0093CD; adds, masks and compares; StageFrameA08D3CD; stores and a bit cleared.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction or piece that ran left them.
 */
StageRegs StageCenterPlayer3CD(StageRegs r)
{
    bool free_to_center;

    r = loaded(r, rd8(CENTER_GATE));
    free_to_center = r.a == 0;
    if (free_to_center) {
        r = loaded(r, RD8(OBJ->f2800[24]));
        r = compared(r, 0x02);
        free_to_center = !r.c;
    }
    r = loaded(r, 0x80);
    WR8(OBJ->f3300[16], 0x80);
    WR8(OBJ->f3358[0], 0);
    if (!free_to_center) {
        CALL(0x86ED, StageFrameA0093CD(r));
        return r;
    }
    r = loaded(r, 0x01);
    wr8(PlayerByte3BF0, 0x01);
    CALL(0x86FE, StageFrameA0093CD(r));
    r = center_near(r, 0xFC);
    if (r.nz == 0) {
        return center_done(r);
    }
    r = center_step(r);
    CALL(0x8736, StageFrameA08D3CD(r));
    r = center_near(r, 0xF8);
    if (r.nz != 0) {
        return r;
    }
    return center_done(r);
}

void abi_StageCenterPlayer3CD(void)
{
    regs_exit_shim(StageCenterPlayer3CD(regs_entry_shim()));
}

/* The bytes of StageWaitTimerStep at $87D8 (stage_mode.c), a census duplicate: its C, as a step of the table. */
static StageRegs StageWaitTimerStep3CD(StageRegs r)
{
    r.nz = StageWaitTimerStep();
    return r;
}

/*
 * @routine 03CD:74B StageSubModeFade3CD
 * @run 6B:874B
 * @stop 8779
 * @unreachable 6B:874B-6B:8778 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $874B runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-mode 4, the fade out: the events step and the display; unless the fade is at its step 3, the
 * overlay's piece at $A08D for the raster effects $0C and $0D, the overlays' frame on every fourth frame, and
 * PlayerByte3BD8 cleared; then the step at zField80 from the seven at $8779, under banks 5 and 6.
 * HOW: StageModeEventsStep152D, StageHudStep3CD; compares; StageFrameA08D3CD; TST of zFrameTick;
 * StageOverlayFrame3CD; MapOverlayBanks5And6; zField80 doubled indexes the table, whose word is jumped through.
 * WAITS: as the step.
 * Entry: nothing. Exit: as the step leaves them.
 */
StageRegs StageSubModeFade3CD(StageRegs r)
{
    static const StepEntry steps[] = {
        { 0x8787, StageFadeStart3CD }, { 0x8790, StageFadeWait3CD }, { 0x87A8, StageFadeStopCd3CD },
        { 0x87D8, StageWaitTimerStep3CD }, { 0x87E0, StageFadeRestart3CD }, { 0x87B7, StageFadeTakeStock152D },
        { 0x87CE, StageFadeWaitLast152D }
    };

    CALL(0x874E, StageModeEventsStep152D(r));
    CALL(0x8751, StageHudStep3CD(r));
    r = loaded(r, RD8(ZP->field80));
    r = compared(r, 0x03);
    if (r.nz != 0) {
        r = loaded(r, RD8(ZP->raster_effect));
        r = compared(r, 0x0C);
        if (r.nz != 0) {
            r = compared(r, 0x0D);
        }
        if (r.nz == 0) {
            CALL(0x8764, StageFrameA08D3CD(r));
        }
        r = tested(r, 0x03, ADDR_OF8(ZP->frame_tick));
        if (r.nz == 0) {
            CALL(0x876C, StageOverlayFrame3CD(r));
        }
        wr8(PlayerByte3BD8, 0);
    }
    r = map_banks_5_6(r, 0x8772);
    return step_by_table(r, RD8(ZP->field80), 0x8779, steps, 7);
}

void abi_StageSubModeFade3CD(void)
{
    regs_exit_shim(StageSubModeFade3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:787 StageFadeStart3CD
 * @run 6B:8787
 * @unreachable 6B:8787-6B:878F no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8787 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The fade's first step: the overlay at $A000's pieces at $A00C and $A012, and the step counted.
 * HOW: Two calls and an increment of zField80.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the increment; A, X, Y, C and V as $A012 left them.
 */
StageRegs StageFadeStart3CD(StageRegs r)
{
    r = orig_shim(r, 0x878A, 0xA00C);
    r = orig_shim(r, 0x878D, 0xA012);
    r.nz = INC8(ZP->field80);
    return r;
}

void abi_StageFadeStart3CD(void)
{
    regs_exit_shim(StageFadeStart3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:790 StageFadeWait3CD
 * @run 6B:8790
 * @unreachable 6B:8790-6B:87A7 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8790 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The fade's second step: the overlay at $A000's piece at $A00F, and $A012 again when it changed the byte
 * at $2A58; once zField89 is set the step is counted.
 * HOW: The byte kept on the stack across $A00F and compared after it; a test; an increment.
 * WAITS: no.
 * Entry: nothing. Exit: A zField89, N and Z from it or from zField80's increment.
 */
StageRegs StageFadeWait3CD(StageRegs r)
{
    uint8_t before;

    r = loaded(r, RD8(OBJ->sprite_set[72]));
    push8(r.a);
    r = orig_shim(r, 0x8797, 0xA00F);
    before = pull8();
    r = loaded(r, before);
    r = compared(r, RD8(OBJ->sprite_set[72]));
    if (r.nz != 0) {
        r = orig_shim(r, 0x87A0, 0xA012);
    }
    r = loaded(r, RD8(ZP->field89));
    if (r.a == 0) {
        return r;
    }
    r.nz = INC8(ZP->field80);
    return r;
}

void abi_StageFadeWait3CD(void)
{
    regs_exit_shim(StageFadeWait3CD(regs_entry_shim()));
}

/* What a card routine leaves, laid over the registers (as in stage_b_census.c). */
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

/* A decimal subtract of 1 from A with C set (SED SEC SBC #1): the difference, N and Z from it, C no borrow; V kept. */
static Regs bcd_less_one(Regs r)
{
    uint8_t value8 = r.a;
    uint8_t low = (uint8_t)((value8 & 0x0F) - 1);
    uint8_t high = (uint8_t)((value8 >> 4) - ((low >> 4) & 1));
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

/*
 * @routine 03CD:7A8 StageFadeStopCd3CD
 * @run 6B:87A8
 * @unreachable 6B:87A8-6B:87B6 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $87A8 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The fade's third step: the CD paused (cd_pause until it answers 0); with no stock left zModeDone is
 * $80 (the game over), else a life is taken from zStock (at $87C0 in StageFadeTakeStock152D's bytes), the wait at
 * zWaitTimer3 set to $60 and the step counted.
 * HOW: cd_pause in a loop; a test of zStock; a decimal subtract and stores.
 * WAITS: in cd_pause, as the card's routine does.
 * Entry: D clear, as the game keeps it. Exit, no stock: A $80, N and Z from it. Else: A $60, N and Z from zField80's
 * increment, C from the subtract; X 0.
 */
StageRegs StageFadeStopCd3CD(StageRegs r)
{
    r = pause_cd(r, 0x87AB);
    r = loaded(r, RD8(ZP->stock));
    if (r.a == 0) {
        r = loaded(r, 0x80);
        WR8(ZP->mode_done, 0x80);
        return r;
    }
    r = bcd_less_one(r);
    WR8(ZP->stock, r.a);
    r = loaded(r, 0x60);
    WR8(ZP->wait_timer3, 0x60);
    r.nz = INC8(ZP->field80);
    return r;
}

void abi_StageFadeStopCd3CD(void)
{
    regs_exit_shim(StageFadeStopCd3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:7E0 StageFadeRestart3CD
 * @run 6B:87E0
 * @unreachable 6B:87E0-6B:87FA no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $87E0 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The fade's last step: the stage starts again from sub-mode 0, zField83 back to zField86, zField8A and
 * zField8F cleared, zField8E's bit 2 cleared; when zStage is negative the CD is paused first and bit 3 set.
 * HOW: Stores and bit changes; cd_pause in a loop.
 * WAITS: in cd_pause, as the card's routine does.
 * Entry: nothing. Exit: A 0, N and Z from it.
 */
StageRegs StageFadeRestart3CD(StageRegs r)
{
    r = loaded(r, RD8(ZP->field86));
    WR8(ZP->field83, r.a);
    WR8(ZP->field8a, 0);
    WR8(ZP->field8f, 0);
    RMW8(ZP->field8e, old, old & (uint8_t)~0x04);
    r = loaded(r, RD8(ZP->stage));
    if ((r.a & 0x80) != 0) {
        r = pause_cd(r, 0x87F1);
        RMW8(ZP->field8e, old, old | 0x08);
    }
    r = loaded(r, 0);
    WR8(ZP->game_sub_mode, 0);
    return r;
}

void abi_StageFadeRestart3CD(void)
{
    regs_exit_shim(StageFadeRestart3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:7FB StageSubModeOpening3CD
 * @run 6B:87FB
 * @unreachable 6B:87FB-6B:881D no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $87FB runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sub-mode 5, the stage's opening: the events step, the display, the overlay at $A000's frame and its
 * piece at $A08D, the overlays' frame, the opening's step; once zField8E's bit 0 is set PlayerByte3BD8 is cleared,
 * sub-mode 1 (the player's count at $3310 1) and PlayerByte3BF0 cleared.
 * HOW: StageModeEventsStep152D, StageHudStep3CD, StageFrameA0003CD, StageFrameA08D3CD, StageOverlayFrame3CD,
 * StageOpeningStep3CD; a bit test; stores.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, N and Z from it when the stage goes into play; else as StageOpeningStep3CD left them.
 */
StageRegs StageSubModeOpening3CD(StageRegs r)
{
    CALL(0x87FE, StageModeEventsStep152D(r));
    CALL(0x8801, StageHudStep3CD(r));
    CALL(0x8804, StageFrameA0003CD(r));
    CALL(0x8807, StageFrameA08D3CD(r));
    CALL(0x880A, StageOverlayFrame3CD(r));
    CALL(0x880D, StageOpeningStep3CD(r));
    if ((RD8(ZP->field8e) & 0x01) == 0) {
        return r;
    }
    wr8(PlayerByte3BD8, 0);
    r = loaded(r, 1);
    WR8(ZP->game_sub_mode, 1);
    WR8(OBJ->f3300[16], 1);
    wr8(PlayerByte3BF0, 0);
    return r;
}

void abi_StageSubModeOpening3CD(void)
{
    regs_exit_shim(StageSubModeOpening3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:81E StageLoadRowMap3CD
 * @run 6B:881E
 * @unreachable 6B:881E-6B:884C no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $881E runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Fills the stage's row map at $3A00 as StageLoadRowMap152D does, from the route's list in the table
 * at $9D69 by zField82.
 * HOW: TAI; the list's address in zTemp00-01; two loops, the run bit rotated into zTemp02.
 * WAITS: no.
 * Entry: nothing. Exit: A $FF, Z set, C set; X the map index reached, Y the list offset after the end. Clobbers
 * zTemp00-02.
 */
StageRegs StageLoadRowMap3CD(StageRegs r)
{
    uint8_t route, index, old;

    tai_shim(r, KernelZeros, ROW_MAP, 0x00FF);
    route = RD8(ZP->field82);
    index = (uint8_t)(route << 1);
    r.c = (route & 0x80) != 0;
    r = loaded(r, index);
    r.y = index;
    r = loaded(r, rd8((uint16_t)(0x9D69 + r.y)));
    WR8(ZP->temp00, r.a);
    r = loaded(r, rd8((uint16_t)(0x9D6A + r.y)));
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

void abi_StageLoadRowMap3CD(void)
{
    regs_exit_shim(StageLoadRowMap3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:84D StageRouteSetUpA3CD
 * @run 6B:884D
 * @unreachable 6B:884D-6B:885A no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $884D runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The kernel's pieces at $403F and at $400C (A the route's byte from the table at $9D57 by zField82),
 * then the route's list unpacked into SaveBuffer (StageUnpackList3CD).
 * HOW: Two calls into the kernel; a jump to StageUnpackList3CD.
 * WAITS: where the kernel's pieces wait.
 * Entry: nothing. Exit: as StageUnpackList3CD.
 */
StageRegs StageRouteSetUpA3CD(StageRegs r)
{
    r = orig_shim(r, 0x8850, 0x403F);
    r.x = RD8(ZP->field82);
    r.nz = r.x;
    r = loaded(r, rd8((uint16_t)(0x9D57 + r.x)));
    r = orig_shim(r, 0x8858, 0x400C);
    return StageUnpackList3CD(r);
}

void abi_StageRouteSetUpA3CD(void)
{
    regs_exit_shim(StageRouteSetUpA3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:85B StageRouteSetUpB3CD
 * @run 6B:885B
 * @unreachable 6B:885B-6B:8869 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $885B runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The kernel's piece at $4015 with X 7 and A the route's byte from the table at $9D5A by zField82 twice
 * plus the character.
 * HOW: A shift and an add; a table read; a jump into the kernel.
 * WAITS: where the kernel's piece waits.
 * Entry: nothing. Exit: as $4015 leaves them.
 */
StageRegs StageRouteSetUpB3CD(StageRegs r)
{
    Sum sum;
    uint8_t route = RD8(ZP->field82);

    sum = add_bytes((uint8_t)(route << 1), RD8(ZP->character), false);
    r.v = sum.overflow;
    r.x = sum.value;
    r = loaded(r, rd8((uint16_t)(0x9D5A + r.x)));
    r.c = sum.carry;
    r.x = 7;
    r.nz = 7;
    return orig_tail_shim(r, 0x4015);
}

void abi_StageRouteSetUpB3CD(void)
{
    regs_exit_shim(StageRouteSetUpB3CD(regs_entry_shim()));
}

/* The scroll's bytes at $3E00 (48) and $3E66 (136) cleared from KernelZeros. */
static void clear_scroll_bytes_3cd(Regs r)
{
    tai_shim(r, KernelZeros, ADDR_OF8(SCROLL->x_frac[0]), 0x0030);
    tai_shim(r, KernelZeros, ADDR_OF8(SCROLLE->f3e66[0]), 0x0088);
}

/* StageLoadScreen3CD called by BSR with zField8E kept on the stack across it and put back. */
static Regs load_screen_keeping_flags(Regs r, uint16_t ret)
{
    uint8_t flags;

    r = loaded(r, RD8(ZP->field8e));
    push8(r.a);
    CALL(ret, StageLoadScreen3CD(r));
    flags = pull8();
    r = loaded(r, flags);
    WR8(ZP->field8e, flags);
    return r;
}

/*
 * @routine 03CD:878 StageResetScroll3CD
 * @run 6B:8878
 * @unreachable 6B:8878-6B:88D1 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8878 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Sets the stage's screen up: the scroll's page and X kept in zBiosAl and zBiosAh, the kernel's piece at
 * $4009, the scroll cleared, the byte at $2ED8 cleared and the screen loaded (zField8E kept); the overlay at $A000's
 * piece at $A74E under bank 7; for a raster effect the table at $886A marks, the screen is loaded again and $A79D runs;
 * then the scroll cleared and it runs on into StageLoadScreen3CD.
 * HOW: Stores; $4009; TAI; StageLoadScreen3CD by BSR; MapOverlayBank7, $A74E; a table read by zRasterEffect.
 * WAITS: where the kernel's piece waits.
 * Entry: nothing. Exit: as StageLoadScreen3CD.
 */
StageRegs StageResetScroll3CD(StageRegs r)
{
    r = loaded(r, RD8(SCROLLE->x_hi_screen[0]));
    WR8(ZP->bios_a.lo, r.a);
    r = loaded(r, RD8(SCROLL->x_lo[0]));
    WR8(ZP->bios_a.hi, r.a);
    r = orig_shim(r, 0x8885, KERNEL_4009);
    clear_scroll_bytes_3cd(r);
    WR8(OBJ->vy_lo[80], 0);
    r = load_screen_keeping_flags(r, 0x889B);
    r = map_bank_7(r, 0x88A1);
    r = orig_shim(r, 0x88A4, 0xA74E);
    r.y = RD8(ZP->raster_effect);
    r = loaded(r, rd8((uint16_t)(0x886A + r.y)));
    if (r.a != 0) {
        clear_scroll_bytes_3cd(r);
        r = load_screen_keeping_flags(r, 0x88BE);
        r = orig_shim(r, 0x88C4, 0xA79D);
    }
    clear_scroll_bytes_3cd(r);
    return StageLoadScreen3CD(r);
}

void abi_StageResetScroll3CD(void)
{
    regs_exit_shim(StageResetScroll3CD(regs_entry_shim()));
}

/* The next byte of the screen's record through zTemp00-01, Y counted past it. */
static Regs screen_byte(Regs r)
{
    r = loaded(r, RD8_PTR_Y(ZP->temp00, r.y));
    r.y++;
    return r;
}

/*
 * @routine 03CD:8D2 StageLoadScreen3CD
 * @run 6B:88D2
 * @unreachable 6B:88D2-6B:8999 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $88D2 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Loads the stage's screen from its record (the table at $9D91 by zField83): the scroll's screen pages, X
 * and Y, the camera mode, the band's byte at $3E66 and the scroll's limits; unless zField8E's bit 1 keeps the player
 * (it is then cleared): a first byte of 0 restores the scroll's page and X kept in zBiosAl and zBiosAh, else it is the
 * player's X; then the player's Y at $2C50, the bytes at $2C08 and $2B30 cleared, and for a character other than 0 in
 * the states 5 to 8 at $2818 the player's X moved by the table at $899A; then zRasterEffect (zRasterArg cleared) and,
 * unless negative, the byte at $2AA0.
 * HOW: The record's address in zTemp00-01; a byte at a time through it, each stored where it goes.
 * WAITS: no.
 * Entry: nothing. Exit: A the record's last byte read, N and Z from it, Y past it. Clobbers zTemp00-01.
 */
StageRegs StageLoadScreen3CD(StageRegs r)
{
    Sum sum;
    uint8_t stage, index, value, move;

    WR8(ZP->raster_stage, 0);
    stage = RD8(ZP->field83);
    index = (uint8_t)(stage << 1);
    r.c = (stage & 0x80) != 0;
    r = loaded(r, index);
    r.y = index;
    r = loaded(r, rd8((uint16_t)(0x9D91 + r.y)));
    WR8(ZP->temp00, r.a);
    r = loaded(r, rd8((uint16_t)(0x9D92 + r.y)));
    WR8(ZP->temp01, r.a);
    r.y = 0;
    r = screen_byte(r);
    WR8(SCROLLE->x_hi_screen[0], r.a);
    WR8(SCROLLE->x_hi_screen[1], r.a);
    WR8(SCROLLE->x_hi_screen[2], r.a);
    WR8(SCROLLE->x_hi_screen[3], r.a);
    r = loaded(r, (uint8_t)(r.a & 0x01));
    WR8(SCROLL->x_hi[0], r.a);
    WR8(SCROLL->x_hi[1], r.a);
    WR8(SCROLL->x_hi[2], r.a);
    WR8(SCROLL->x_hi[3], r.a);
    r = screen_byte(r);
    WR8(SCROLLE->y_hi_screen[0], r.a);
    r = loaded(r, (uint8_t)(r.a & 0x01));
    WR8(SCROLL->y_hi[0], r.a);
    r = screen_byte(r);
    WR8(SCROLL->x_lo[0], r.a);
    r = screen_byte(r);
    WR8(SCROLL->y_lo[0], r.a);
    r = screen_byte(r);
    WR8(SCROLLLIM->camera_mode, r.a);
    r = screen_byte(r);
    WR8(SCROLLE->f3e66[0], r.a);
    r = screen_byte(r);
    WR8(SCROLLLIM->x_min_hi, r.a);
    r = screen_byte(r);
    wr8(ScrollXMaxHi, r.a);
    r = screen_byte(r);
    WR8(SCROLLLIM->y_min_hi, r.a);
    r = screen_byte(r);
    wr8(ScrollYMaxHi, r.a);
    if ((RD8(ZP->field8e) & 0x02) != 0) {
        RMW8(ZP->field8e, old, old & (uint8_t)~0x02);
        r.y = (uint8_t)(r.y + 2);
    } else {
        r = loaded(r, RD8_PTR_Y(ZP->temp00, r.y));
        if (r.a == 0) {
            r = loaded(r, RD8(ZP->bios_a.lo));
            WR8(SCROLLE->x_hi_screen[0], r.a);
            r = loaded(r, (uint8_t)(r.a & 0x01));
            WR8(SCROLL->x_hi[0], r.a);
            r = loaded(r, RD8(ZP->bios_a.hi));
            WR8(SCROLL->x_lo[0], r.a);
            r.y++;
        } else {
            r.y++;
            WR8(OBJ->x_hi[8], r.a);
        }
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
                r.v = sum.overflow;
                r = compared(r, 0x04);
                if (!r.c) {
                    r.x = r.a;
                    value = RD8(OBJ->x_hi[8]);
                    move = rd8((uint16_t)(0x899A + r.x));
                    sum = add_bytes(value, move, false);
                    r = loaded(r, sum.value);
                    r.c = sum.carry;
                    r.v = sum.overflow;
                    WR8(OBJ->x_hi[8], sum.value);
                }
            }
        }
    }
    r = screen_byte(r);
    WR8(ZP->raster_effect, r.a);
    WR8(ZP->raster_arg, 0);
    r = loaded(r, RD8_PTR_Y(ZP->temp00, r.y));
    if ((r.a & 0x80) == 0) {
        r.y++;
        WR8(OBJ->frame[56], r.a);
        return r;
    }
    r.y++;
    return r;
}

void abi_StageLoadScreen3CD(void)
{
    regs_exit_shim(StageLoadScreen3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:99E StageOverlayFrame3CD
 * @run 6B:899E
 * @unreachable 6B:899E-6B:89B1 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $899E runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: The frame of the overlays above: the kernel's piece at $4096 under bank 7, then the piece at $A000 under
 * banks 8 and 9 above zRamBaseBank.
 * HOW: MapOverlayBank7, $4096; the add and two TAM; $A000.
 * WAITS: where the pieces wait.
 * Entry: nothing. Exit: as $A000 leaves them.
 */
StageRegs StageOverlayFrame3CD(StageRegs r)
{
    Sum sum;

    r = map_bank_7(r, 0x89A1);
    r = orig_shim(r, 0x89A4, 0x4096);
    sum = add_bytes(0x08, RD8(ZP->ram_base_bank), false);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    tam(0x20, r.a);
    r = loaded(r, (uint8_t)(r.a + 1));
    tam(0x40, r.a);
    return orig_shim(r, 0x89B1, 0xA000);
}

void abi_StageOverlayFrame3CD(void)
{
    regs_exit_shim(StageOverlayFrame3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:9B2 StageLevelStart3CD
 * @run 6B:89B2
 * @unreachable 6B:89B2-6B:89E7 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $89B2 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: Starts the stage's level: the kernel's piece at $4057, then under bank 7 the level's stream and axis
 * from the four bytes at $9E21 by zField83, the 32 event flags cleared when the fourth byte is not 0, the kernel's piece
 * at $4093 and the overlays' frame.
 * HOW: $4057, MapOverlayBank7; table reads; a loop of stores with Y kept on the stack; $4093; StageOverlayFrame3CD.
 * WAITS: where the pieces wait.
 * Entry: nothing. Exit: as StageOverlayFrame3CD.
 */
StageRegs StageLevelStart3CD(StageRegs r)
{
    uint8_t stage, index;

    r = orig_shim(r, 0x89B5, 0x4057);
    r = map_bank_7(r, 0x89B8);
    stage = RD8(ZP->field83);
    index = (uint8_t)(stage << 2);
    r.c = (stage & 0x40) != 0;
    r = loaded(r, index);
    r.y = index;
    r = loaded(r, rd8((uint16_t)(0x9E21 + r.y)));
    WR8(LEVEL->level_stream.lo, r.a);
    r = loaded(r, rd8((uint16_t)(0x9E22 + r.y)));
    WR8(LEVEL->level_stream.hi, r.a);
    r = loaded(r, rd8((uint16_t)(0x9E23 + r.y)));
    WR8(LEVEL->level_axis, r.a);
    r = loaded(r, rd8((uint16_t)(0x9E24 + r.y)));
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
    r = orig_shim(r, 0x89E4, 0x4093);
    CALL(0x89E7, StageOverlayFrame3CD(r));
    return r;
}

void abi_StageLevelStart3CD(void)
{
    regs_exit_shim(StageLevelStart3CD(regs_entry_shim()));
}

/* The door handlers' piece for an object whose byte at $2818 is 0: the kernel's piece at $40D2, then the object's
 * packed byte at $2F20: its room (MASK of it) into $3310, the next two bits into $3358, and for the pad's door the two
 * above into $32C8. */
static Regs unpack_door(Regs r, uint16_t ret, uint8_t mask, bool third)
{
    uint8_t slot, packed, rolled;
    bool carry;

    r = orig_shim(r, ret, 0x40D2);
    slot = r.x;
    packed = RD8(OBJ->fields_2ee0[64 + slot]);
    r = loaded(r, packed);
    r.y = packed;
    r = loaded(r, (uint8_t)(packed & mask));
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

/* The door's state cleared (the fall-back at $8A33). */
static Regs door_closed(Regs r)
{
    WR8(OBJ->status_flags[80 + r.x], 0);
    return r;
}

/* The door taken: zField83 the room, zField87 the entry, zSubStep 2. */
static Regs door_taken(Regs r)
{
    r = loaded(r, RD8(OBJ->f3300[16 + r.x]));
    WR8(ZP->field83, r.a);
    r = loaded(r, RD8(OBJ->f3358[r.x]));
    WR8(ZP->field87, r.a);
    return r;
}

/*
 * @routine 03CD:9E8 DoorHandler3CD
 * @run 6B:89E8
 * @unreachable 6B:89E8-6B:8A36 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $89E8 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: An object handler for a door the pad's override opens, as DoorHandler152D with the kernel's piece at
 * $40D2 where that has $A36C, and zField8C set to 1 when the door is taken.
 * HOW: Loads, shifts through the carry and stores; the tests and the stores of the door.
 * WAITS: no.
 * Entry: X the object. Exit: A the last value loaded or masked, N and Z from it; X kept.
 */
StageRegs DoorHandler3CD(StageRegs r)
{
    uint8_t slot = r.x;

    r = loaded(r, RD8(OBJ->f2800[24 + slot]));
    if (r.a == 0) {
        return unpack_door(r, 0x89F0, 0x0F, true);
    }
    WR8(OBJ->f3778[24 + slot], 0);
    r.y = RD8(OBJ->f32a8[32 + slot]);
    r.nz = r.y;
    r = loaded(r, rd8((uint16_t)(PadOverrides + r.y)));
    if (r.a != 0) {
        r = loaded(r, RD8(OBJ->status_flags[80 + slot]));
        if (r.a != 0) {
            r = compared(r, 0x05);
            if (!r.c) {
                r = door_taken(r);
                r = loaded(r, 0x01);
                WR8(ZP->field8c, 0x01);
                r = loaded(r, 0x02);
                WR8(ZP->sub_step, 0x02);
                return r;
            }
        }
    }
    return door_closed(r);
}

void abi_DoorHandler3CD(void)
{
    regs_exit_shim(DoorHandler3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:A37 DoorHandlerB3CD
 * @run 6B:8A37
 * @unreachable 6B:8A37-6B:8A84 no read the game makes loads disc sectors 03CD to 03D0 (or their copies at 134D, 14DD, 154D, 15BD, 162D and 177D), so these bytes are never in memory.
 * Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
 * the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
 * five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
 * on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
 * run. No input or poke of work RAM can load the sectors, and a unit call at $8A37 runs whatever overlay another scene
 * left there, not these bytes.
 *
 * WHAT IT DOES: An object handler for a door: not yet placed, $40D2 and the packed byte unpacked (its room in five bits).
 * Placed: the byte at $3790 cleared; a door whose packed byte has bit 7 needs PlayerByte3BD7; with the byte at $3BD3
 * other than 1 and the object's state at $31F0 from 1 to 4 the stage goes through it (zField83 the room, zField87 the
 * entry, zField8C 0, zSubStep 2, the player's Y bytes at $2CE0 and $2D28 cleared); else the state is cleared (at $3BD3
 * 1 nothing is done).
 * HOW: Loads, shifts and stores; the tests and the stores of the door.
 * WAITS: no.
 * Entry: X the object. Exit: A the last value loaded or computed, N and Z from it; X kept.
 */
StageRegs DoorHandlerB3CD(StageRegs r)
{
    uint8_t slot = r.x;

    r = loaded(r, RD8(OBJ->f2800[24 + slot]));
    if (r.a == 0) {
        return unpack_door(r, 0x8A3F, 0x1F, false);
    }
    WR8(OBJ->f3778[24 + slot], 0);
    r = loaded(r, RD8(OBJ->fields_2ee0[64 + slot]));
    if ((r.a & 0x80) != 0) {
        r = loaded(r, rd8(PlayerByte3BD7));
        if (r.a == 0) {
            return door_closed(r);
        }
    }
    r = loaded(r, (uint8_t)(rd8(PlayerMayLeaveTop) - 1));
    if (r.a == 0) {
        return r;
    }
    r = loaded(r, RD8(OBJ->status_flags[80 + slot]));
    if (r.a == 0) {
        return door_closed(r);
    }
    r = compared(r, 0x05);
    if (r.c) {
        return door_closed(r);
    }
    r = door_taken(r);
    WR8(ZP->field8c, 0);
    r = loaded(r, 0x02);
    WR8(ZP->sub_step, 0x02);
    WR8(OBJ->y_lo[16], 0);
    WR8(OBJ->y_frac[0], 0);
    return r;
}

void abi_DoorHandlerB3CD(void)
{
    regs_exit_shim(DoorHandlerB3CD(regs_entry_shim()));
}
