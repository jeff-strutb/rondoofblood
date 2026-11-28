/*
 * The stage overlay of disc sectors 03CD to 03D0 (8 K at $8000, bank $6B as the stage overlays run), which no read
 * the game makes can load; sectors 134D, 14DD, 154D, 15BD, 162D and 177D hold the same bytes. It is a later build of
 * the overlay of sectors 152D to 1530 (stage_b_census.c), with a demo recorder (zWaitTimer's byte at $78 its mode,
 * the pad's bytes kept from $A000 under bank $40) and its own frame by zGameSubMode. Routines whose bytes are those of
 * the 152D build, or of filed code, and run the same at this address are not filed again; their C serves here: $802D
 * MapOverlayBanks5And6, $8044 MapOverlayBank7, $81D3, $825E, $825F and $95FB (an RTS,
 * StageSubStepNone152D), $87B7 StageFadeTakeStock152D, $87CE StageFadeWaitLast152D, $87D8 StageWaitTimerStep, $8FA1
 * StageOpeningWriteRow152D, $9128 TallyNext152D, $9405 StageSplitTens152D, $9425 AndVramData, $9434 OrVramData.
 * Four byte copies of 152D routines are filed here as well, at this origin, their C the 152D routine's: $81AD
 * StageModeEventsStep3CD, $934B TallyWait3CD, $9638 HitKindSpawn3CD and $967A HitKindState3CD (the end of the file).
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
#include "player_objects.h"
#include "syscard.h"
#include "stage_b_census.h"
#include "stage_c_census.h"
#include "stage_programs_census.h"
#include "front_end.h"

enum {
    FADE_ACTIVE = 0x203C, SUB3_STEP_TABLE = 0x2084, SUB3_STEP = 0x2085, FALL_STEP = 0x2088, FADE_COUNT = 0x209B,
    FADE_SOUND = 0x209D, STAGE_FLAG_2743 = 0x2743, DEMO_HELD = 0x207C, DEMO_COUNT = 0x207D, DEMO_POINTER = 0x207E,
    DEMO_PAGE = 0x207F, PAD_HELD = 0x2229, PAD_NEW = 0x222E, DEMO_BUFFER = 0xA000,
    
    HIT_X = 0x2713, HIT_Y = 0x2723, HIT_HI = 0x2733, TEST_X = 0x2702, TEST_Y = 0x2703, REACH_X = 0x2704,
    REACH_Y = 0x2705, PROBE_HIT = 0x26F6, PROBE_X = 0x26FA, PROBE_Y = 0x26FE,
    ROW_MAP = 0x3A00, TALLY_CELLS = 0x3980, ROW_BUFFER = 0x397E, SCREEN_3EB4 = 0x3EB4,
    SCREEN_3EC4 = 0x3EC4, CENTER_GATE = 0x2860, PROBE_LISTS_3CD = 0xD756, GATE_BUSY = 0x2746
};

typedef StageRegs Regs;

/* 03CD's tables. */
static const StageRouteBuild route_3cd = { 0x9D63, 0x9D69, 0x9D57, 0x9D5A, 0x9D91, 0x9E21, 0x9D60, PROBE_LISTS_3CD };

void abi_VecAddScoreByte(void);   /* system/vectors.c */
void abi_VecApplyVelocity(void);   /* system/vectors.c */
void abi_VecApplyVelocityX(void);   /* system/vectors.c */
void abi_VecApplyVelocityY(void);   /* system/vectors.c */
void abi_VecCdPlayAgain(void);   /* system/vectors.c */
void abi_VecClearEnemySlots(void);   /* system/vectors.c */
void abi_VecClearListChannels(void);   /* system/vectors.c */
void abi_VecClearObjectInY(void);   /* system/vectors.c */
void abi_VecClearScreenState(void);   /* system/vectors.c */
void abi_VecLoadObjectRecord(void);   /* system/vectors.c */
void abi_VecNextRandom(void);   /* system/vectors.c */
void abi_VecPlayerAndCameraFrame(void);   /* system/vectors.c */
void abi_VecResetGameState(void);   /* system/vectors.c */
void abi_VecSelectVramData(void);   /* system/vectors.c */
void abi_VecSelectVramReadAddress(void);   /* system/vectors.c */
void abi_VecSelectVramWriteAddress(void);   /* system/vectors.c */
void abi_VecSoundCommand(void);   /* system/vectors.c */
void abi_VecStartListChannel0Kind2(void);   /* system/vectors.c */
void abi_VecStartListChannelKind2(void);   /* system/vectors.c */
void abi_VecStartScriptSlot0Kind1(void);   /* system/vectors.c */
void abi_VecStartScriptSlotKind1(void);   /* system/vectors.c */
void abi_VecStartScriptSlotKind4(void);   /* system/vectors.c */
void abi_VecStopSound(void);   /* system/vectors.c */
void abi_VecStreamFullWalk(void);   /* system/vectors.c */
void abi_VecStreamScrollStep(void);   /* system/vectors.c */

static Regs loaded(Regs r, uint8_t value);

/* A JSR to one of this file's routines: the frame the original pushes, the routine, the pop. */
#define CALL(ret, expr) do { jsr_push(ret); r = (expr); jsr_pop(); } while (0)


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

/* A JSR to a kernel jump-table entry: its C between the frame, the registers as they stand, and what it leaves. */
static Regs vector_call(Regs r, uint16_t ret, void (*vector)(void))
{
    stage_regs_load_shim(r);
    jsr_push(ret);
    vector();
    jsr_pop();
    return stage_regs_from_cpu_shim(r);
}

/* A tail JMP to a kernel jump-table entry: it returns for this routine. */
static Regs vector_tail(Regs r, void (*vector)(void))
{
    stage_regs_load_shim(r);
    vector();
    return stage_regs_from_cpu_shim(r);
}

/* A JSR into the overlay at $A000, or through a word of a table: the C of the routine loaded there (routine_at). */
static Regs resident_call(Regs r, uint16_t ret, uint16_t run)
{
    stage_regs_load_shim(r);
    call_routine(ret, run);
    return stage_regs_from_cpu_shim(r);
}

/* A tail JMP the same way. */
static Regs resident_tail(Regs r, uint16_t run)
{
    stage_regs_load_shim(r);
    run_routine(run);
    return stage_regs_from_cpu_shim(r);
}

/* TAI, which pushes and pulls Y, A and X: the registers as they stand put in the machine for it. */
static void tai_shim(Regs r, uint16_t src, uint16_t dst, uint16_t len)
{
    stage_regs_load_shim(r);
    tai(src, dst, len);
}

/* JSR $9410, SetVramAddresses3CD. */
static Regs set_vram_addresses_call(Regs r, uint16_t ret)
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
    r = after_ended(r, map_overlay_banks_exit_shim(MapOverlayBanks5And6()));
    jsr_pop();
    return r;
}

/* JSR $8044: the bytes of MapOverlayBank7. */
static Regs map_bank_7(Regs r, uint16_t ret)
{
    jsr_push(ret);
    r = after_ended(r, map_overlay_bank7_exit_shim(MapOverlayBank7()));
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
 * @unreachable 6B:8000-6B:8002 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8000 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:8003-6B:8005 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8003 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $8003: a jump to $0000, the hardware page, where no code is (the entry of this
 * build that has no routine).
 * HOW: One jump, no frame of its own.
 * WAITS: as what runs there.
 * Entry, exit: as the code at $0000.
 */
StageRegs StageEntryB3CD(StageRegs r)
{
    return resident_tail(r, 0x0000);
}

void abi_StageEntryB3CD(void)
{
    regs_exit_shim(StageEntryB3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:006 StageEntryC3CD
 * @run 6B:8006
 * @unreachable 6B:8006-6B:8008 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8006 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:8009-6B:800B no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8009 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:800C-6B:800E no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $800C runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $800C: goes to StageProbePoints3CD at $968D.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as StageProbePoints3CD.
 */
StageRegs StageEntryE3CD(StageRegs r)
{
    return resident_tail(r, 0x968D);
}

void abi_StageEntryE3CD(void)
{
    regs_exit_shim(StageEntryE3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:00F StageEntryF3CD
 * @run 6B:800F
 * @unreachable 6B:800F-6B:8011 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $800F runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The overlay's entry at $800F: goes to StageProbeAgain3CD at $96DB.
 * HOW: One jump, no frame of its own.
 * WAITS: no.
 * Entry, exit: as StageProbeAgain3CD.
 */
StageRegs StageEntryF3CD(StageRegs r)
{
    return resident_tail(r, 0x96DB);
}

void abi_StageEntryF3CD(void)
{
    regs_exit_shim(StageEntryF3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:018 StageSubModeStep3CD
 * @run 6B:8018
 * @stop 801F
 * @unreachable 6B:8018-6B:801E no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8018 runs whatever overlay another scene left there, not
 * these bytes.
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
        return resident_tail(r, handler);
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
    return resident_tail(r, run);
}

/*
 * @routine 03CD:038 StageFrameA0003CD
 * @run 6B:8038
 * @unreachable 6B:8038-6B:803D no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8038 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:803E-6B:8043 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $803E runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:804C-6B:8051 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $804C runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:8052-6B:8057 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8052 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:8058-6B:8090 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8058 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:80D6-6B:80EE no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $80D6 runs whatever overlay another scene left there, not
 * these bytes.
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
        CALL(0x80D4, DemoAdvance3CD(r));
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
 * @unreachable 6B:8091-6B:80D5 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8091 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:80EF-6B:8124 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $80EF runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:8125-6B:8167 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8125 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Sub-mode 0, the stage's start: as StageSubModeStart152D (the kernel's piece at $4006, the route's
 * pieces, the row map, the screen, the overlay at $A000's start, the display, the opening, the level), the pause and
 * the fade cleared, sub-mode 1, zField86 zField83; the demo recorder started (DemoStart3CD). zField8E's bit 0 clear
 * sends the stage to sub-mode 5 with the count at $3310 one less and PlayerStartPose 1; else $407B runs unless zStage is
 * negative.
 * HOW: Calls in turn $4006, StageRouteSetUpA3CD, StageRouteSetUpB3CD, StageLoadRowMap3CD, StageResetScroll3CD,
 * MapOverlayBanks5And6, $A003, StageHudStart3CD, StageOpeningStart3CD, StageLevelStart3CD and DemoStart3CD.
 * WAITS: where the pieces it calls wait.
 * Entry: nothing. Exit, sub-mode 5: A 1, N and Z from it. Else: A zStage, N and Z from it, or as $407B leaves them.
 */
StageRegs StageSubModeStart3CD(StageRegs r)
{
    uint8_t value;

    r = vector_call(r, 0x8128, abi_VecResetGameState);
    r = resident_call(r, 0x812B, 0x884D);
    r = resident_call(r, 0x812E, 0x885B);
    r = resident_call(r, 0x8131, 0x881E);
    r = resident_call(r, 0x8134, 0x8878);
    r = map_banks_5_6(r, 0x8137);
    r = resident_call(r, 0x813A, 0xA003);
    CALL(0x813D, StageHudStart3CD(r));
    CALL(0x8140, StageOpeningStart3CD(r));
    r = resident_call(r, 0x8143, 0x89B2);
    WR8(ZP->paused, 0);
    wr8(FADE_ACTIVE, 0);
    r.nz = INC8(ZP->game_sub_mode);
    r = loaded(r, RD8(ZP->field83));
    WR8(ZP->field86, r.a);
    CALL(0x8150, DemoStart3CD(r));
    if ((RD8(ZP->field8e) & 0x01) == 0) {
        r = loaded(r, 5);
        WR8(ZP->game_sub_mode, 5);
        r.nz = DEC8(OBJ->work_b[16]);
        r = loaded(r, 1);
        wr8(PlayerStartPose, 1);
        return r;
    }
    value = RD8(ZP->stage);
    r = loaded(r, value);
    if ((value & 0x80) != 0) {
        return r;
    }
    return vector_call(r, 0x815A, abi_VecCdPlayAgain);
}

void abi_StageSubModeStart3CD(void)
{
    regs_exit_shim(StageSubModeStart3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:168 StageFadeTimer3CD
 * @run 6B:8168
 * @unreachable 6B:8168-6B:8191 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8168 runs whatever overlay another scene left there, not
 * these bytes.
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
        return vector_tail(r, abi_VecSoundCommand);
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
 * @unreachable 6B:8192-6B:81AC no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8192 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:81D4-6B:81DD no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $81D4 runs whatever overlay another scene left there, not
 * these bytes.
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
    *r = vector_call(*r, 0x8229, abi_VecPlayerAndCameraFrame);
    return true;
}

/*
 * @routine 03CD:1DE StageSubModePlay3CD
 * @run 6B:81DE
 * @stop 8250
 * @unreachable 6B:81DE-6B:824F no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $81DE runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Sub-mode 1, the stage in play: the demo recorder, the fade and the pause; paused, the paused frame.
 * Else the events step; the stage's timer PlayerSlowTimerLo-3BE9 counted down (at its end the kernel's piece at $4015
 * with A 9 and X 7), and while it runs the scroll's steps cleared and, on odd counts, the overlay's frame skipped;
 * unless a sub-step is asked for the kernel's piece at $40D8; then for a stage with zField8E's bit 7 the scroll by
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
    r = loaded(r, rd8(PlayerSlowTimerLo));
    low = rd8(PlayerSlowTimerHi);
    r = loaded(r, (uint8_t)(r.a | low));
    if (r.a != 0) {
        r.nz = (uint8_t)(rd8(PlayerSlowTimerLo) - 1);
        wr8(PlayerSlowTimerLo, r.nz);
        if (r.nz == 0) {
            r.nz = (uint8_t)(rd8(PlayerSlowTimerHi) - 1);
            wr8(PlayerSlowTimerHi, r.nz);
            if (r.nz == 0) {
                r = loaded(r, 0x09);
                r.x = 0x07;
                r.nz = 0x07;
                r = vector_call(r, 0x820D, abi_VecStartScriptSlotKind1);
            }
        }
        WR8(SCROLLE->step_x_hi[0], 0);
        WR8(SCROLLE->step_x_lo[0], 0);
        WR8(SCROLLE->step_y_hi[0], 0);
        WR8(SCROLLE->step_y_lo[0], 0);
        r = tested(r, 0x01, PlayerSlowTimerLo);
        timed = r.nz != 0;
    }
    if (timed || frame_and_sub_step_clear(&r)) {
        r = loaded(r, RD8(ZP->field8e));
        if ((r.a & 0x80) != 0) {
            CALL(0x8230, StageCenterPlayer3CD(r));
        } else {
            CALL(0x8235, StageFrameA08D3CD(r));
            r = tested(r, 0x01, PlayerSlowTimerLo);
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
        return resident_tail(r, handler);
    }
}

void abi_StageSubModePlay3CD(void)
{
    regs_exit_shim(StageSubModePlay3CD(regs_entry_shim()));
}

/* The fade restarted when TimeSlowMode is set: cleared, $9B 2, and the fade's countdown run on. */
static Regs restart_fade(Regs r)
{
    r = loaded(r, rd8(TimeSlowMode));
    if (r.a == 0) {
        return r;
    }
    wr8(TimeSlowMode, 0);
    r = loaded(r, 2);
    wr8(FADE_COUNT, 2);
    return StageFadeTimer3CD(r);
}

/*
 * @routine 03CD:260 StageSubStepDeath3CD
 * @run 6B:8260
 * @unreachable 6B:8260-6B:8282 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8260 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Sub-step 2, the player died: sub-mode 2 with zField80, zField89 and the fall's step cleared, the
 * overlay at $A000's piece at $A067 (StageFrameA0673CD), the forced presses cleared; when TimeSlowMode is set it is
 * cleared and the fade starts again from 2.
 * HOW: Stores; StageFrameA0673CD; a jump into StageFadeTimer3CD.
 * WAITS: no.
 * Entry: nothing. Exit: A TimeSlowMode (0), N and Z from it, the rest as $A067 left them; else as StageFadeTimer3CD.
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
 * @unreachable 6B:8283-6B:82BC no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8283 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Sub-step 3, the stage cleared: sub-mode 3 with its step 0 and its table at $84 picked by zField8C
 * (none: 2; negative: 4; 3 to 6 or $0F up: 3; else 1, with $2743 set to 1); the overlay at $A000's piece at $A067, the
 * forced presses and TimeSlowMode cleared.
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
    wr8(TimeSlowMode, 0);
    return r;
}

void abi_StageSubStepCleared3CD(void)
{
    regs_exit_shim(StageSubStepCleared3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:2BD StageSubStepFadeOut3CD
 * @run 6B:82BD
 * @unreachable 6B:82BD-6B:82C7 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $82BD runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:82C8-6B:82D4 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $82C8 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:82D5-6B:82FF no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $82D5 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Sub-step 6: sub-mode 6 with zField80, zField89 and the fall's step cleared, the overlay at $A000's
 * piece at $A067, the forced presses cleared, the player's step at $3358 0 and its count at $3310 $40; when
 * TimeSlowMode is set it is cleared and the fade starts again from 2.
 * HOW: Stores; StageFrameA0673CD; a jump into StageFadeTimer3CD.
 * WAITS: no.
 * Entry: nothing. Exit: A TimeSlowMode (0), N and Z from it, the rest as $A067 left them; else as StageFadeTimer3CD.
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
    WR8(OBJ->work_c[0], 0);
    r = loaded(r, 0x40);
    WR8(OBJ->work_b[16], 0x40);
    return restart_fade(r);
}

void abi_StageSubStepTally3CD(void)
{
    regs_exit_shim(StageSubStepTally3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:300 StageSubModeDeath3CD
 * @run 6B:8300
 * @unreachable 6B:8300-6B:8317 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8300 runs whatever overlay another scene left there, not
 * these bytes.
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
    r = vector_call(r, 0x8305, abi_VecSoundCommand);
    CALL(0x8308, StageDeathPhase3CD(r));
    r = loaded(r, RD8(ZP->game_sub_mode));
    r = compared(r, 0x01);
    if (r.nz != 0) {
        return r;
    }
    r = map_banks_5_6(r, 0x8312);
    r = resident_call(r, 0x8315, 0xA015);
    return resident_tail(r, 0x89B2);
}

void abi_StageSubModeDeath3CD(void)
{
    regs_exit_shim(StageSubModeDeath3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:318 StageDeathPhase3CD
 * @run 6B:8318
 * @stop 831F
 * @unreachable 6B:8318-6B:831E no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8318 runs whatever overlay another scene left there, not
 * these bytes.
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
        return resident_tail(r, handler);
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

    r = resident_call(r, ret, 0x8878);
    r = loaded(r, 1);
    WR8(ZP->game_sub_mode, 1);
    r = loaded(r, RD8(ZP->field8c));
    if (r.a != 0) {
        WR8(ZP->field8c, 0);
        r = loaded(r, 0x18);
        WR8(OBJ->work_b[16], 0x18);
    }
    facing = (uint8_t)(RD8(OBJ->substate[24]) & 0x01);
    r.x = facing;
    r = loaded(r, rd8((uint16_t)(0x8346 + facing)));
    WR8(OBJ->work_c[0], r.a);
    return r;
}

/*
 * @routine 03CD:327 StageDeathRespawn3CD
 * @run 6B:8327
 * @unreachable 6B:8327-6B:8345 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8327 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:8348-6B:8366 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8348 runs whatever overlay another scene left there, not
 * these bytes.
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

/* The fall's last step: the player put back, sub-mode 1, zField8C cleared, the count at $3310 $20, the step at $3358.
 * */
static Regs fall_ends(Regs r, uint16_t ret, uint8_t step)
{
    r = resident_call(r, ret, 0x8878);
    r = loaded(r, 1);
    WR8(ZP->game_sub_mode, 1);
    WR8(ZP->field8c, 0);
    r = loaded(r, 0x20);
    WR8(OBJ->work_b[16], 0x20);
    r = loaded(r, step);
    WR8(OBJ->work_c[0], step);
    return r;
}

/* The fall's middle step, as in stage_b_census.c: $A01B and $A012; while the player is shown and short of the edge the
 * step is counted, else $A01E runs with X 0. */
static Regs fall_moves(Regs r, uint16_t at, bool right)
{
    r = map_banks_5_6(r, at);
    r = resident_call(r, (uint16_t)(at + 3), 0xA01B);
    r = resident_call(r, (uint16_t)(at + 6), 0xA012);
    r = loaded(r, RD8(OBJ->sprite_attr[24]));
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
    return resident_tail(r, 0xA01E);
}

/*
 * @routine 03CD:367 StageDeathFallLeft3CD
 * @run 6B:8367
 * @unreachable 6B:8367-6B:8378 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8367 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:8379-6B:83A1 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8379 runs whatever overlay another scene left there, not
 * these bytes.
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
    r = resident_call(r, 0x8381, 0xA018);
    r = resident_call(r, 0x8384, 0xA012);
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
 * @unreachable 6B:83A2-6B:83B5 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $83A2 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:83B6-6B:83FA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $83B6 runs whatever overlay another scene left there, not
 * these bytes.
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
    return resident_tail(r, handler);
}

/*
 * @routine 03CD:3FB StageSubModeCleared3CD
 * @run 6B:83FB
 * @stop 8405
 * @unreachable 6B:83FB-6B:8404 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $83FB runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:840F-6B:8438 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $840F runs whatever overlay another scene left there, not
 * these bytes.
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
    r = vector_call(r, 0x8412, abi_VecClearScreenState);
    r = resident_call(r, 0x8415, 0x884D);
    r = resident_call(r, 0x8418, 0x885B);
    r = resident_call(r, 0x841B, 0x881E);
    r = resident_call(r, 0x841E, 0x8878);
    CALL(0x8421, StageHudStart3CD(r));
    r = loaded(r, 2);
    WR8(ZP->field8a, 2);
    r = map_banks_5_6(r, 0x8428);
    r = resident_call(r, 0x842B, 0xA003);
    r = resident_call(r, 0x842E, 0x89B2);
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
 * @unreachable 6B:8445-6B:8452 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8445 runs whatever overlay another scene left there, not
 * these bytes.
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
    return resident_tail(r, 0xA012);
}

void abi_StageWalkSprite3CD(void)
{
    regs_exit_shim(StageWalkSprite3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:455 StageClearedWalkOut3CD
 * @run 6B:8455
 * @stop 8462
 * @unreachable 6B:8455-6B:8461 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8455 runs whatever overlay another scene left there, not
 * these bytes.
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
        { 0x84CE, StageClearedWalkBack3CD }
    };

    r = map_bank_7(r, 0x8458);
    r = vector_call(r, 0x845B, abi_VecStreamScrollStep);
    return step_by_table(r, rd8(SUB3_STEP), 0x8462, steps, 4);
}

void abi_StageClearedWalkOut3CD(void)
{
    regs_exit_shim(StageClearedWalkOut3CD(regs_entry_shim()));
}

/* 03CD's wait for the stage's objects. */
static const WaitObjectsBuild wait_objects_3cd = {
    0x846C, { 0x8445, 0x803E, 0x804C, 0x899E }, ADDR_OF8(OBJ->state[40]), 0x40, STAGE_FLAG_2743
};

/*
 * @routine 03CD:46C StageClearedWaitObjects3CD
 * @run 6B:846C
 * @unreachable 6B:846C-6B:8489 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $846C runs whatever overlay another scene left there, not
 * these bytes.
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
    return StageClearedWaitObjects3CDAt(r, &wait_objects_3cd);
}

/* StageClearedWaitObjects3CD's body, for its builds elsewhere. */
StageRegs StageClearedWaitObjects3CDAt(StageRegs r, const WaitObjectsBuild *build)
{
    uint8_t slot;

    r = resident_call(r, (uint16_t)(build->at + 0x3), build->calls[0]);
    r = resident_call(r, (uint16_t)(build->at + 0x6), build->calls[1]);
    r = resident_call(r, (uint16_t)(build->at + 0x9), build->calls[2]);
    r = resident_call(r, (uint16_t)(build->at + 0xC), build->calls[3]);
    for (slot = build->count; slot != 0; slot--) {
        r = loaded(r, RD8_IDX(build->slots, slot));
        r.x = slot;
        if ((r.a & 0x80) == 0 && r.a != 0) {
            return r;
        }
    }
    r.x = 0;
    wr8(build->flag, 0);
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
 * @unreachable 6B:848A-6B:84C3 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $848A runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:84C4-6B:84CD no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $84C4 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:84CE-6B:8510 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $84CE runs whatever overlay another scene left there, not
 * these bytes.
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
    WR8(OBJ->work_b[16], 0xE0);
    side = walk_side();
    r.y = side;
    r = loaded(r, rd8((uint16_t)(0x8441 + side)));
    WR8(OBJ->work_c[0], r.a);
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
    WR8(OBJ->work_b[16], 0);
    WR8(OBJ->work_c[0], 0);
    r = loaded(r, 2);
    WR8(ZP->field8a, 2);
    r = resident_call(r, 0x850B, 0xA003);
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
 * @unreachable 6B:8511-6B:85A2 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8511 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The cleared stage's last step: the player walks on with the scroll until ScrollXLo is 0; then the next
 * stage: below zField8C 7 the route's next stage from the table at $9D60 (zField82 counted, into zField83 and
 * zField86), else zField83 counted with zField8E's bit 4 set and sound $FF; zField8E's bit 1 set; the route's pieces
 * below 7, the row map, the scroll's bytes at $3E00 (48) and $3E66 (136) cleared, the screen loaded (with zField8E
 * kept, and again under bank 7 after $A74E for camera mode $0D); bit 1 cleared; the level started unless zField8C is 7
 * or 8; zField8C and the pause cleared, sub-mode 1.
 * HOW: As StageClearedWalk3CD for the walk; calls, TAI, bit changes.
 * WAITS: where the pieces it calls wait.
 * Entry: nothing. Exit, walking: A ScrollXLo, N and Z from it. Else: A 1, N and Z from it.
 */
StageRegs StageClearedWalkOn3CD(StageRegs r)
{
    return StageClearedWalkOn3CDAt(r, &route_3cd);
}

/* StageClearedWalkOn3CD's body, for the builds of it with their own tables. */
StageRegs StageClearedWalkOn3CDAt(StageRegs r, const StageRouteBuild *build)
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
        r = loaded(r, rd8((uint16_t)(build->next_stage + r.y)));
        WR8(ZP->field83, r.a);
        WR8(ZP->field86, r.a);
    } else {
        r.nz = INC8(ZP->field83);
        RMW8(ZP->field8e, old, old | 0x10);
        r = loaded(r, 0xFF);
        r = vector_call(r, 0x8553, abi_VecSoundCommand);
    }
    RMW8(ZP->field8e, old, old | 0x02);
    r = loaded(r, RD8(ZP->field8c));
    r = compared(r, 0x07);
    if (!r.c) {
        CALL(0x855E, StageRouteSetUpA3CDAt(r, build));
        CALL(0x8561, StageRouteSetUpB3CDAt(r, build));
    }
    CALL(0x8564, StageLoadRowMap3CDAt(r, build));
    tai_shim(r, KernelZeros, ADDR_OF8(SCROLL->x_frac[0]), 0x0030);
    tai_shim(r, KernelZeros, ADDR_OF8(SCROLLE->f3e66[0]), 0x0088);
    r = loaded(r, RD8(ZP->field8e));
    push8(r.a);
    CALL(0x8578, StageLoadScreen3CDAt(r, build));
    flags = pull8();
    r = loaded(r, flags);
    WR8(ZP->field8e, flags);
    r = loaded(r, RD8(SCROLLLIM->camera_mode));
    r = compared(r, 0x0D);
    if (r.nz == 0) {
        r = map_bank_7(r, 0x8585);
        r = resident_call(r, 0x8588, 0xA74E);
        CALL(0x858B, StageLoadScreen3CDAt(r, build));
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
        CALL(0x859A, StageLevelStart3CDAt(r, build));
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
 * @unreachable 6B:85A3-6B:85AC no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $85A3 runs whatever overlay another scene left there, not
 * these bytes.
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
        { 0x85B3, StageBossScrollToDoor3CD }, { 0x85E3, StageBossWaitDoor3CD }
    };

    CALL(0x85A6, StageFadeTimer3CD(r));
    return step_by_table(r, rd8(SUB3_STEP), 0x85AD, steps, 2);
}

void abi_StageClearedBossWalk3CD(void)
{
    regs_exit_shim(StageClearedBossWalk3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:5B3 StageBossScrollToDoor3CD
 * @run 6B:85B3
 * @unreachable 6B:85B3-6B:85E2 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $85B3 runs whatever overlay another scene left there, not
 * these bytes.
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
    r = vector_call(r, 0x85E0, abi_VecSoundCommand);
    r.nz = (uint8_t)(rd8(SUB3_STEP) + 1);
    wr8(SUB3_STEP, r.nz);
    return r;
}

void abi_StageBossScrollToDoor3CD(void)
{
    regs_exit_shim(StageBossScrollToDoor3CD(regs_entry_shim()));
}

/* 03CD's wait at the boss's door. */
static const BossDoorBuild boss_door_3cd = {
    0x85E3, 0x803E, 0x899E, ADDR_OF8(OBJ->substate[58]), ADDR_OF8(OBJ->anim_step[2]), ADDR_OF8(OBJ->work_c[0])
};

/*
 * @routine 03CD:5E3 StageBossWaitDoor3CD
 * @run 6B:85E3
 * @unreachable 6B:85E3-6B:85FC no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $85E3 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: A step after the boss: the overlay at $A000's piece at $A009 and the overlays' frame; once the door's
 * byte at $283A is 4 and its animation step at $340A is 0, the player's step at $3358 is $20 and the step counted.
 * HOW: StageFrameA0093CD, StageOverlayFrame3CD; two tests; a store and an increment.
 * WAITS: no.
 * Entry: nothing. Exit: A the last byte tested, N and Z from its test, or $20 with N and Z from the increment.
 */
StageRegs StageBossWaitDoor3CD(StageRegs r)
{
    return StageBossWaitDoor3CDAt(r, &boss_door_3cd);
}

/* StageBossWaitDoor3CD's body, for its builds elsewhere. */
StageRegs StageBossWaitDoor3CDAt(StageRegs r, const BossDoorBuild *build)
{
    r = resident_call(r, (uint16_t)(build->at + 3), build->frame);
    r = resident_call(r, (uint16_t)(build->at + 6), build->overlay);
    r = loaded(r, rd8(build->ready));
    r = compared(r, 0x04);
    if (r.nz != 0) {
        return r;
    }
    r = loaded(r, rd8(build->busy));
    if (r.a != 0) {
        return r;
    }
    r = loaded(r, 0x20);
    wr8(build->timer, 0x20);
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
 * @unreachable 6B:85FD-6B:8618 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $85FD runs whatever overlay another scene left there, not
 * these bytes.
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
    return StageBossWalkOut3CDAt(r, &route_3cd);
}

/* StageBossWalkOut3CD's body, for the builds of it with their own tables. */
StageRegs StageBossWalkOut3CDAt(StageRegs r, const StageRouteBuild *build)
{
    r = loaded(r, 0xB8);
    WR8(OBJ->work_b[16], 0xB8);
    CALL(0x8605, StageFrameA0003CD(r));
    r = vector_call(r, 0x8608, abi_VecPlayerAndCameraFrame);
    r = resident_call(r, 0x860B, 0xA009);
    CALL(0x860E, StageFrameA08D3CD(r));
    CALL(0x8611, StageOverlayFrame3CD(r));
    r = loaded(r, RD8(OBJ->x_hi[8]));
    r = compared(r, 0xF0);
    if (r.nz != 0) {
        return r;
    }
    return StageNextRouteStage3CDAt(r, build);
}

void abi_StageBossWalkOut3CD(void)
{
    regs_exit_shim(StageBossWalkOut3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:619 StageNextRouteStage3CD
 * @run 6B:8619
 * @unreachable 6B:8619-6B:8636 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8619 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The route's next stage: zDisplayDelay 2, the player stopped, $2743 cleared, zField82 counted and the
 * stage from the table at $9D60 into zField83, $84 and the byte at $2A58 cleared.
 * HOW: Stores; an increment; a table read.
 * WAITS: no.
 * Entry: nothing. Exit: A 0 (CLA keeps the flags), N and Z from the table's load, Y zField82.
 */
StageRegs StageNextRouteStage3CD(StageRegs r)
{
    return StageNextRouteStage3CDAt(r, &route_3cd);
}

/* StageNextRouteStage3CD's body, for the builds of it with their own tables. */
StageRegs StageNextRouteStage3CDAt(StageRegs r, const StageRouteBuild *build)
{
    r = loaded(r, 0x02);
    WR8(ZP->display_delay, 0x02);
    WR8(OBJ->work_b[16], 0);
    wr8(STAGE_FLAG_2743, 0);
    WR8(OBJ->work_c[0], 0);
    r.nz = INC8(ZP->field82);
    r = loaded(r, RD8(ZP->field82));
    r.y = r.a;
    r = loaded(r, rd8((uint16_t)(build->next_stage + r.y)));
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
 * @unreachable 6B:8637-6B:8643 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8637 runs whatever overlay another scene left there, not
 * these bytes.
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
    r = vector_call(r, 0x863D, abi_VecStreamScrollStep);
    return step_by_table(r, rd8(SUB3_STEP), 0x8644, steps, 5);
}

void abi_StageClearedLastWalk3CD(void)
{
    regs_exit_shim(StageClearedLastWalk3CD(regs_entry_shim()));
}

/*
 * @routine 03CD:64E StageClearedFrame3CD
 * @run 6B:864E
 * @unreachable 6B:864E-6B:8653 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $864E runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:8654-6B:865F no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8654 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:8660-6B:86BA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8660 runs whatever overlay another scene left there, not
 * these bytes.
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
    WR8(OBJ->work_c[0], r.a);
    WR8(OBJ->work_b[16], r.a);
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
        value = RD8(OBJ->sprite_attr[24]);
        sum = add_bytes(step, value, false);
        r = loaded(r, sum.value);
        r.c = sum.carry;
        r.v = sum.overflow;
        WR8(OBJ->sprite_attr[24], sum.value);
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
 * @unreachable 6B:86BB-6B:86D5 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $86BB runs whatever overlay another scene left there, not
 * these bytes.
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
    r = vector_call(r, 0x86C0, abi_VecSoundCommand);
    r = map_banks_5_6(r, 0x86C3);
    r = resident_call(r, 0x86C6, 0xA015);
    r = loaded(r, (uint8_t)(RD8(ZP->field8c) & 0x7F));
    if (r.a == 0) {
        WR8(ZP->field8c, 0);
        return resident_tail(r, 0x8619);
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

/* The centring done: zField8E's bit 7, the count at $3310 and PlayerStartPose cleared. */
static Regs center_done(Regs r)
{
    RMW8(ZP->field8e, old, old & (uint8_t)~0x80);
    WR8(OBJ->work_b[16], 0);
    wr8(PlayerStartPose, 0);
    return r;
}

/*
 * @routine 03CD:6D6 StageCenterPlayer3CD
 * @run 6B:86D6
 * @unreachable 6B:86D6-6B:874A no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $86D6 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The play frame's scroll while zField8E's bit 7 is set: the player's count at $3310 $80 and its step
 * cleared, the overlay at $A000's piece at $A009. Unless the byte at $2860 is set or the player's state at $2818 is 2
 * or more, PlayerStartPose is 1 and the player is walked to the screen's middle ($80) by 4 a frame with the scroll
 * moving the other way and the overlay's piece at $A08D; there the centring ends.
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
        r = loaded(r, RD8(OBJ->substate[24]));
        r = compared(r, 0x02);
        free_to_center = !r.c;
    }
    r = loaded(r, 0x80);
    WR8(OBJ->work_b[16], 0x80);
    WR8(OBJ->work_c[0], 0);
    if (!free_to_center) {
        CALL(0x86ED, StageFrameA0093CD(r));
        return r;
    }
    r = loaded(r, 0x01);
    wr8(PlayerStartPose, 0x01);
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
 * @unreachable 6B:874B-6B:8778 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $874B runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Sub-mode 4, the fade out: the events step and the display; unless the fade is at its step 3, the
 * overlay's piece at $A08D for the raster effects $0C and $0D, the overlays' frame on every fourth frame, and
 * TimeSlowMode cleared; then the step at zField80 from the seven at $8779, under banks 5 and 6.
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
        wr8(TimeSlowMode, 0);
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
 * @unreachable 6B:8787-6B:878F no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8787 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The fade's first step: the overlay at $A000's pieces at $A00C and $A012, and the step counted.
 * HOW: Two calls and an increment of zField80.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the increment; A, X, Y, C and V as $A012 left them.
 */
StageRegs StageFadeStart3CD(StageRegs r)
{
    return StageFadeStart3CDAt(r, 0x8787);
}

/* StageFadeStart3CD's body, for its copies at other addresses. */
StageRegs StageFadeStart3CDAt(StageRegs r, uint16_t at)
{
    r = resident_call(r, (uint16_t)(at + 3), 0xA00C);
    r = resident_call(r, (uint16_t)(at + 6), 0xA012);
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
 * @unreachable 6B:8790-6B:87A7 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8790 runs whatever overlay another scene left there, not
 * these bytes.
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
    r = resident_call(r, 0x8797, 0xA00F);
    before = pull8();
    r = loaded(r, before);
    r = compared(r, RD8(OBJ->sprite_set[72]));
    if (r.nz != 0) {
        r = resident_call(r, 0x87A0, 0xA012);
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
 * @stop 87B7
 * @unreachable 6B:87A8-6B:87B6 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $87A8 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:87E0-6B:87FA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $87E0 runs whatever overlay another scene left there, not
 * these bytes.
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
 * @unreachable 6B:87FB-6B:881D no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $87FB runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Sub-mode 5, the stage's opening: the events step, the display, the overlay at $A000's frame and its
 * piece at $A08D, the overlays' frame, the opening's step; once zField8E's bit 0 is set TimeSlowMode is cleared,
 * sub-mode 1 (the player's count at $3310 1) and PlayerStartPose cleared.
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
    wr8(TimeSlowMode, 0);
    r = loaded(r, 1);
    WR8(ZP->game_sub_mode, 1);
    WR8(OBJ->work_b[16], 1);
    wr8(PlayerStartPose, 0);
    return r;
}

void abi_StageSubModeOpening3CD(void)
{
    regs_exit_shim(StageSubModeOpening3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:01E StageLoadRowMap3CD
 * @run 6B:881E
 * @unreachable 6B:881E-6B:884C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $881E runs whatever overlay another scene left there, not
 * these bytes.
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
    return StageLoadRowMap3CDAt(r, &route_3cd);
}

/* StageLoadRowMap3CD's body, for the builds of it with their own tables. */
StageRegs StageLoadRowMap3CDAt(StageRegs r, const StageRouteBuild *build)
{
    uint8_t route, index, old;

    tai_shim(r, KernelZeros, ROW_MAP, 0x00FF);
    route = RD8(ZP->field82);
    index = (uint8_t)(route << 1);
    r.c = (route & 0x80) != 0;
    r = loaded(r, index);
    r.y = index;
    r = loaded(r, rd8((uint16_t)(build->row_maps + r.y)));
    WR8(ZP->temp00, r.a);
    r = loaded(r, rd8((uint16_t)(build->row_maps + 1 + r.y)));
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
 * @routine 03CE:04D StageRouteSetUpA3CD
 * @run 6B:884D
 * @unreachable 6B:884D-6B:885A no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $884D runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The kernel's pieces at $403F and at $400C (A the route's byte from the table at $9D57 by zField82),
 * then the route's list unpacked into SaveBuffer (StageUnpackList3CD).
 * HOW: Two calls into the kernel; a jump to StageUnpackList3CD.
 * WAITS: where the kernel's pieces wait.
 * Entry: nothing. Exit: as StageUnpackList3CD.
 */
StageRegs StageRouteSetUpA3CD(StageRegs r)
{
    return StageRouteSetUpA3CDAt(r, &route_3cd);
}

/* StageRouteSetUpA3CD's body, for the builds of it with their own tables. */
StageRegs StageRouteSetUpA3CDAt(StageRegs r, const StageRouteBuild *build)
{
    r = vector_call(r, 0x8850, abi_VecClearListChannels);
    r.x = RD8(ZP->field82);
    r.nz = r.x;
    r = loaded(r, rd8((uint16_t)(build->route_a + r.x)));
    r = vector_call(r, 0x8858, abi_VecStartListChannel0Kind2);
    return StageUnpackList3CDAt(r, build);
}

void abi_StageRouteSetUpA3CD(void)
{
    regs_exit_shim(StageRouteSetUpA3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:05B StageRouteSetUpB3CD
 * @run 6B:885B
 * @unreachable 6B:885B-6B:8869 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $885B runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The kernel's piece at $4015 with X 7 and A the route's byte from the table at $9D5A by zField82 twice
 * plus the character.
 * HOW: A shift and an add; a table read; a jump into the kernel.
 * WAITS: where the kernel's piece waits.
 * Entry: nothing. Exit: as $4015 leaves them.
 */
StageRegs StageRouteSetUpB3CD(StageRegs r)
{
    return StageRouteSetUpB3CDAt(r, &route_3cd);
}

/* StageRouteSetUpB3CD's body, for the builds of it with their own tables. */
StageRegs StageRouteSetUpB3CDAt(StageRegs r, const StageRouteBuild *build)
{
    Sum sum;
    uint8_t route = RD8(ZP->field82);

    sum = add_bytes((uint8_t)(route << 1), RD8(ZP->character), false);
    r.v = sum.overflow;
    r.x = sum.value;
    r = loaded(r, rd8((uint16_t)(build->route_b + r.x)));
    r.c = sum.carry;
    r.x = 7;
    r.nz = 7;
    return vector_tail(r, abi_VecStartScriptSlotKind1);
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

/* The build's StageLoadScreen3CD called by BSR with zField8E kept on the stack across it and put back. */
static Regs load_screen_keeping_flags(Regs r, uint16_t ret, const StageRouteBuild *build)
{
    uint8_t flags;

    r = loaded(r, RD8(ZP->field8e));
    push8(r.a);
    CALL(ret, StageLoadScreen3CDAt(r, build));
    flags = pull8();
    r = loaded(r, flags);
    WR8(ZP->field8e, flags);
    return r;
}

/*
 * @routine 03CE:078 StageResetScroll3CD
 * @run 6B:8878
 * @unreachable 6B:8878-6B:88D1 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8878 runs whatever overlay another scene left there, not
 * these bytes.
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
    return StageResetScroll3CDAt(r, &route_3cd);
}

/* StageResetScroll3CD's body, for the builds of it with their own tables. */
StageRegs StageResetScroll3CDAt(StageRegs r, const StageRouteBuild *build)
{
    r = loaded(r, RD8(SCROLLE->x_hi_screen[0]));
    WR8(ZP->bios_a.lo, r.a);
    r = loaded(r, RD8(SCROLL->x_lo[0]));
    WR8(ZP->bios_a.hi, r.a);
    r = vector_call(r, 0x8885, abi_VecClearScreenState);
    clear_scroll_bytes_3cd(r);
    WR8(OBJ->vy_lo[80], 0);
    r = load_screen_keeping_flags(r, 0x889B, build);
    r = map_bank_7(r, 0x88A1);
    r = resident_call(r, 0x88A4, 0xA74E);
    r.y = RD8(ZP->raster_effect);
    r = loaded(r, rd8((uint16_t)(0x886A + r.y)));
    if (r.a != 0) {
        clear_scroll_bytes_3cd(r);
        r = load_screen_keeping_flags(r, 0x88BE, build);
        r = resident_call(r, 0x88C4, 0xA79D);
    }
    clear_scroll_bytes_3cd(r);
    return StageLoadScreen3CDAt(r, build);
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
 * @routine 03CE:0D2 StageLoadScreen3CD
 * @run 6B:88D2
 * @unreachable 6B:88D2-6B:8999 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $88D2 runs whatever overlay another scene left there, not
 * these bytes.
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
    return StageLoadScreen3CDAt(r, &route_3cd);
}

/* StageLoadScreen3CD's body, for the builds of it with their own tables. */
StageRegs StageLoadScreen3CDAt(StageRegs r, const StageRouteBuild *build)
{
    Sum sum;
    uint8_t stage, index, value, move;

    WR8(ZP->raster_stage, 0);
    stage = RD8(ZP->field83);
    index = (uint8_t)(stage << 1);
    r.c = (stage & 0x80) != 0;
    r = loaded(r, index);
    r.y = index;
    r = loaded(r, rd8((uint16_t)(build->screens + r.y)));
    WR8(ZP->temp00, r.a);
    r = loaded(r, rd8((uint16_t)(build->screens + 1 + r.y)));
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
        WR8(OBJ->sprite_attr[24], 0);
        r = loaded(r, RD8(ZP->character));
        if (r.a != 0) {
            r = loaded(r, RD8(OBJ->substate[24]));
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
 * @routine 03CE:19E StageOverlayFrame3CD
 * @run 6B:899E
 * @unreachable 6B:899E-6B:89B1 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $899E runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The frame of the overlays above: the kernel's piece at $4096 under bank 7, then the piece at $A000
 * under banks 8 and 9 above zRamBaseBank.
 * HOW: MapOverlayBank7, $4096; the add and two TAM; $A000.
 * WAITS: where the pieces wait.
 * Entry: nothing. Exit: as $A000 leaves them.
 */
StageRegs StageOverlayFrame3CD(StageRegs r)
{
    Sum sum;

    r = map_bank_7(r, 0x89A1);
    r = vector_call(r, 0x89A4, abi_VecStreamScrollStep);
    sum = add_bytes(0x08, RD8(ZP->ram_base_bank), false);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    tam(0x20, r.a);
    r = loaded(r, (uint8_t)(r.a + 1));
    tam(0x40, r.a);
    return resident_call(r, 0x89B1, 0xA000);
}

void abi_StageOverlayFrame3CD(void)
{
    regs_exit_shim(StageOverlayFrame3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:1B2 StageLevelStart3CD
 * @run 6B:89B2
 * @unreachable 6B:89B2-6B:89E7 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $89B2 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Starts the stage's level: the kernel's piece at $4057, then under bank 7 the level's stream and axis
 * from the four bytes at $9E21 by zField83, the 32 event flags cleared when the fourth byte is not 0, the kernel's
 * piece at $4093 and the overlays' frame.
 * HOW: $4057, MapOverlayBank7; table reads; a loop of stores with Y kept on the stack; $4093; StageOverlayFrame3CD.
 * WAITS: where the pieces wait.
 * Entry: nothing. Exit: as StageOverlayFrame3CD.
 */
StageRegs StageLevelStart3CD(StageRegs r)
{
    return StageLevelStart3CDAt(r, &route_3cd);
}

/* StageLevelStart3CD's body, for the builds of it with their own tables. */
StageRegs StageLevelStart3CDAt(StageRegs r, const StageRouteBuild *build)
{
    uint8_t stage, index;

    r = vector_call(r, 0x89B5, abi_VecClearEnemySlots);
    r = map_bank_7(r, 0x89B8);
    stage = RD8(ZP->field83);
    index = (uint8_t)(stage << 2);
    r.c = (stage & 0x40) != 0;
    r = loaded(r, index);
    r.y = index;
    r = loaded(r, rd8((uint16_t)(build->levels + r.y)));
    WR8(LEVEL->level_stream.lo, r.a);
    r = loaded(r, rd8((uint16_t)(build->levels + 1 + r.y)));
    WR8(LEVEL->level_stream.hi, r.a);
    r = loaded(r, rd8((uint16_t)(build->levels + 2 + r.y)));
    WR8(LEVEL->level_axis, r.a);
    r = loaded(r, rd8((uint16_t)(build->levels + 3 + r.y)));
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
    r = vector_call(r, 0x89E4, abi_VecStreamFullWalk);
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

    r = vector_call(r, ret, abi_VecLoadObjectRecord);
    slot = r.x;
    packed = RD8(OBJ->work_a[64 + slot]);
    r = loaded(r, packed);
    r.y = packed;
    r = loaded(r, (uint8_t)(packed & mask));
    WR8(OBJ->work_b[16 + slot], r.a);
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
    WR8(OBJ->work_c[slot], r.a);
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
    WR8(OBJ->health[32 + slot], r.a);
    return r;
}

/* The door's state cleared (the fall-back at $8A33). */
static Regs door_closed(Regs r)
{
    WR8(OBJ->hit_status[80 + r.x], 0);
    return r;
}

/* The door taken: zField83 the room, zField87 the entry, zSubStep 2. */
static Regs door_taken(Regs r)
{
    r = loaded(r, RD8(OBJ->work_b[16 + r.x]));
    WR8(ZP->field83, r.a);
    r = loaded(r, RD8(OBJ->work_c[r.x]));
    WR8(ZP->field87, r.a);
    return r;
}

/*
 * @routine 03CE:1E8 DoorHandler3CD
 * @run 6B:89E8
 * @unreachable 6B:89E8-6B:8A36 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $89E8 runs whatever overlay another scene left there, not
 * these bytes.
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

    r = loaded(r, RD8(OBJ->substate[24 + slot]));
    if (r.a == 0) {
        return unpack_door(r, 0x89F0, 0x0F, true);
    }
    WR8(OBJ->work_l[24 + slot], 0);
    r.y = RD8(OBJ->health[32 + slot]);
    r.nz = r.y;
    r = loaded(r, rd8((uint16_t)(PadOverrides + r.y)));
    if (r.a != 0) {
        r = loaded(r, RD8(OBJ->hit_status[80 + slot]));
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
 * @routine 03CE:237 DoorHandlerB3CD
 * @run 6B:8A37
 * @unreachable 6B:8A37-6B:8A84 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8A37 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: An object handler for a door: not yet placed, $40D2 and the packed byte unpacked (its room in five
 * bits). Placed: the byte at $3790 cleared; a door whose packed byte has bit 7 needs PlayerDoorReady; with the byte at
 * $3BD3 other than 1 and the object's state at $31F0 from 1 to 4 the stage goes through it (zField83 the room,
 * zField87 the entry, zField8C 0, zSubStep 2, the player's Y bytes at $2CE0 and $2D28 cleared); else the state is
 * cleared (at $3BD3 1 nothing is done).
 * HOW: Loads, shifts and stores; the tests and the stores of the door.
 * WAITS: no.
 * Entry: X the object. Exit: A the last value loaded or computed, N and Z from it; X kept.
 */
StageRegs DoorHandlerB3CD(StageRegs r)
{
    uint8_t slot = r.x;

    r = loaded(r, RD8(OBJ->substate[24 + slot]));
    if (r.a == 0) {
        return unpack_door(r, 0x8A3F, 0x1F, false);
    }
    WR8(OBJ->work_l[24 + slot], 0);
    r = loaded(r, RD8(OBJ->work_a[64 + slot]));
    if ((r.a & 0x80) != 0) {
        r = loaded(r, rd8(PlayerDoorReady));
        if (r.a == 0) {
            return door_closed(r);
        }
    }
    r = loaded(r, (uint8_t)(rd8(PlayerMayLeaveTop) - 1));
    if (r.a == 0) {
        return r;
    }
    r = loaded(r, RD8(OBJ->hit_status[80 + slot]));
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

/*
 * @routine 03CE:285 StageHudStart3CD
 * @run 6B:8A85
 * @unreachable 6B:8A85-6B:8A94 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8A85 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Draws the stage's display at its start: the pieces 0 and 1 (StageDrawPiece3CD), the stock and the
 * points to add; zField8E's bit 5 cleared; then runs on into StageHudStep3CD.
 * HOW: Calls with X 0 and 1; a bit cleared.
 * WAITS: no.
 * Entry: nothing. Exit: as StageHudStep3CD.
 */
StageRegs StageHudStart3CD(StageRegs r)
{
    r.x = 0;
    CALL(0x8A89, StageDrawPiece3CD(r));
    r.x++;
    r.nz = r.x;
    CALL(0x8A8D, StageDrawPiece3CD(r));
    CALL(0x8A90, StageDrawStock3CD(r));
    CALL(0x8A93, StageDrawScoreAdd3CD(r));
    RMW8(ZP->field8e, old, old & (uint8_t)~0x20);
    return StageHudStep3CD(r);
}

void abi_StageHudStart3CD(void)
{
    regs_exit_shim(StageHudStart3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:295 StageHudStep3CD
 * @run 6B:8A95
 * @stop 8B18
 * @unreachable 6B:8A95-6B:8B17 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8A95 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The stage's display each frame, as StageHudStep152D: the item's glow; the item's frame at $2A9E from
 * the table at $8B18 by zField93 (for the characters, frames $72 for item $0D and $71 for $11, with $2C96 $28 instead
 * of $24); the health bar a step toward zPlayerHealth (sounds $0E and $40); the hearts' digits; the boss's bar while
 * zField8E's bit 2 is set.
 * HOW: StageHudSound3CD; compares; StageHealthBarAddress3CD by BSR with AndVramData or OrVramData (census duplicates
 * at $9425 and $9434, through map_draw.c's C); the sound hook; StageDrawDigit3CD; a jump to StageBossBar3CD.
 * WAITS: no.
 * Entry: nothing. Exit: as the last piece that ran left them.
 */
StageRegs StageHudStep3CD(StageRegs r)
{
    uint8_t shown, before;
    bool rose = false;

    CALL(0x8A98, StageHudSound3CD(r));
    r = loaded(r, 0x24);
    WR8(OBJ->y_hi[30], 0x24);
    r.x = RD8(ZP->field93);
    r = loaded(r, rd8((uint16_t)(0x8B18 + r.x)));
    r.x = RD8(ZP->character);
    if (r.x != 0) {
        r = compared(r, 0x0D);
        if (r.nz == 0) {
            r = loaded(r, 0x72);
        } else {
            r = compared(r, 0x11);
            if (r.nz == 0) {
                r.x = 0x28;
                WR8(OBJ->y_hi[30], 0x28);
                r = loaded(r, 0x71);
            }
        }
    }
    WR8(OBJ->frame[54], r.a);
    r = loaded(r, RD8(ZP->player_health));
    shown = RD8(ZP->field99);
    r = compared(r, shown);
    if (r.nz == 0) {
        RMW8(ZP->field8e, old, old | 0x20);
    } else if (!r.c) {
        CALL(0x8AC6, StageHealthBarAddress3CD(r));
        r.x = 0xFF;
        r.y = 0xF0;
        r.nz = 0xF0;
        r = and_vram(r, 0x8ACD);
        r.nz = DEC8(ZP->field99);
    } else {
        if ((RD8(ZP->field8e) & 0x20) != 0 && (RD8(ZP->field8e) & 0x40) == 0) {
            r = loaded(r, 0x0E);
            r = vector_call(r, 0x8ADC, abi_VecSoundCommand);
            RMW8(ZP->field8e, old, old | 0x40);
        }
        r.nz = INC8(ZP->field99);
        CALL(0x8AE2, StageHealthBarAddress3CD(r));
        r.x = 0;
        r.y = 0x0F;
        r.nz = 0x0F;
        r = or_vram(r, 0x8AE9);
        rose = true;
    }
    if (!rose && (RD8(ZP->field8e) & 0x40) != 0) {
        RMW8(ZP->field8e, old, old & (uint8_t)~0x40);
        r = loaded(r, 0x40);
        r = vector_call(r, 0x8AF7, abi_VecSoundCommand);
    }
    r = loaded(r, RD8(ZP->field94));
    before = RD8(ZP->field95);
    r = compared(r, before);
    if (r.nz != 0) {
        WR8(ZP->field95, r.a);
        r = loaded(r, (uint8_t)(r.a & 0x0F));
        r.x = 8;
        r.nz = 8;
        CALL(0x8B06, StageDrawDigit3CD(r));
        r = loaded(r, RD8(ZP->field94));
        r.c = (r.a & 0x08) != 0;
        r = loaded(r, (uint8_t)(r.a >> 4));
        r.x = 9;
        r.nz = 9;
        CALL(0x8B11, StageDrawDigit3CD(r));
    }
    if ((RD8(ZP->field8e) & 0x04) == 0) {
        return r;
    }
    return StageBossBar3CD(r);
}

void abi_StageHudStep3CD(void)
{
    regs_exit_shim(StageHudStep3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:321 StageHudSound3CD
 * @run 6B:8B21
 * @unreachable 6B:8B21-6B:8B30 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8B21 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The item's glow: the kernel's piece at $4018 with X 1 and A the colour of the step the wave counter's
 * low three bits (masked by RichterItemReady) pick from the eight at $8B31.
 * HOW: Two ANDs and a table read; a jump into the kernel.
 * WAITS: no.
 * Entry: nothing. Exit: as $4018 leaves them.
 */
StageRegs StageHudSound3CD(StageRegs r)
{
    uint8_t step = (uint8_t)(RD8(ZP->wave_counter) & 0x07);

    r.x = (uint8_t)(step & rd8(RichterItemReady));
    r = loaded(r, rd8((uint16_t)(0x8B31 + r.x)));
    r.x = 1;
    r.nz = 1;
    return vector_tail(r, abi_VecStartScriptSlotKind4);
}

void abi_StageHudSound3CD(void)
{
    regs_exit_shim(StageHudSound3CD(regs_entry_shim()));
}

/* The VRAM address of a bar's piece $5E less COUNT on, as in stage_b_census.c. */
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
 * @routine 03CE:339 StageHealthBarAddress3CD
 * @run 6B:8B39
 * @unreachable 6B:8B39-6B:8B61 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8B39 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Points the VDC at the health bar's piece for the shown health zField99, as StageHealthBarAddress152D.
 * HOW: A subtract, nibbles and shifts into zTemp00-01, an add; SetVramAddresses3CD with the address in X and Y.
 * WAITS: no.
 * Entry: nothing. Exit, zField99 0: A 0, N and Z from it. Else: as SetVramAddresses3CD. Clobbers zTemp00-01.
 */
StageRegs StageHealthBarAddress3CD(StageRegs r)
{
    uint8_t shown;

    r = loaded(r, RD8(ZP->field99));
    if (r.a == 0) {
        return r;
    }
    shown = RD8(ZP->field99);
    r = bar_address(r, shown, 0, 0x65);
    return set_vram_addresses_call(r, 0x8B61);
}

void abi_StageHealthBarAddress3CD(void)
{
    regs_exit_shim(StageHealthBarAddress3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:362 StageBossBarReset3CD
 * @run 6B:8B62
 * @unreachable 6B:8B62-6B:8B69 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8B62 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The boss's bar drawn anew (StageDrawPiece3CD 2) and its length at $2745 cleared; then runs on into
 * StageBossBar3CD. Reached from StageBossBar3CD while the boss's frame at $2A9D is 0.
 * HOW: A call with X 2; a store.
 * WAITS: no.
 * Entry: nothing. Exit: as StageBossBar3CD.
 */
StageRegs StageBossBarReset3CD(StageRegs r)
{
    r.x = 2;
    r.nz = 2;
    CALL(0x8B67, StageDrawPiece3CD(r));
    wr8(BossBarLength, 0);
    return StageBossBar3CD(r);
}

void abi_StageBossBarReset3CD(void)
{
    regs_exit_shim(StageBossBarReset3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:36A StageBossBar3CD
 * @run 6B:8B6A
 * @unreachable 6B:8B6A-6B:8B78 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8B6A runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The boss's bar each frame, as StageBossBar152D.
 * HOW: Tests; a decrement; a branch or run into StageBossBarStep3CD.
 * WAITS: no.
 * Entry: nothing. Exit: as StageBossBarStep3CD.
 */
StageRegs StageBossBar3CD(StageRegs r)
{
    r = loaded(r, RD8(OBJ->frame[53]));
    if (r.a == 0) {
        return StageBossBarReset3CD(r);
    }
    r = loaded(r, RD8(OBJ->x_hi[77]));
    r = compared(r, 0xD8);
    if (r.nz != 0) {
        r.nz = DEC8(OBJ->x_hi[77]);
    }
    return StageBossBarStep3CD(r);
}

void abi_StageBossBar3CD(void)
{
    regs_exit_shim(StageBossBar3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:379 StageBossBarStep3CD
 * @run 6B:8B79
 * @unreachable 6B:8B79-6B:8BB9 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8B79 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Moves the boss's bar at $2745 one step toward the boss's health at $3128, as StageBossBarStep152D; the
 * flash (StageBossBarFlash3CD) while the count at $330D is not 0.
 * HOW: Compares; StageBossBarAddress3CD by BSR with AndVramData or OrVramData; the health on the stack across the call
 * of itself.
 * WAITS: no.
 * Entry: nothing. Exit: as the last piece that ran left them.
 */
StageRegs StageBossBarStep3CD(StageRegs r)
{
    uint8_t health, bar;

    r = loaded(r, RD8(OBJ->work_b[13]));
    if (r.a != 0) {
        return StageBossBarFlash3CD(r);
    }
    health = RD8(OBJ->loop_flags[56]);
    r = loaded(r, health);
    if ((health & 0x80) != 0) {
        push8(r.a);
        WR8(OBJ->loop_flags[56], 0);
        CALL(0x8BA0, StageBossBarStep3CD(r));
        health = pull8();
        r = loaded(r, health);
        WR8(OBJ->loop_flags[56], health);
        r.y = rd8(BossBarLength);
        r.nz = r.y;
        if (r.y != 0) {
            return r;
        }
        WR8(OBJ->work_b[13], r.a);
        return r;
    }
    bar = rd8(BossBarLength);
    r = compared(r, bar);
    if (r.nz == 0) {
        return r;
    }
    if (r.c) {
        r.nz = (uint8_t)(rd8(BossBarLength) + 1);
        wr8(BossBarLength, r.nz);
        CALL(0x8BB2, StageBossBarAddress3CD(r));
        r.x = 0xF0;
        r.y = 0;
        r.nz = 0;
        return or_vram(r, 0x8BB9);
    }
    CALL(0x8B8F, StageBossBarAddress3CD(r));
    r.x = 0x0F;
    r.y = 0xFF;
    r.nz = 0xFF;
    r = and_vram(r, 0x8B96);
    r.nz = (uint8_t)(rd8(BossBarLength) - 1);
    wr8(BossBarLength, r.nz);
    return r;
}

void abi_StageBossBarStep3CD(void)
{
    regs_exit_shim(StageBossBarStep3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:3BA StageBossBarAddress3CD
 * @run 6B:8BBA
 * @unreachable 6B:8BBA-6B:8C02 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8BBA runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Points the VDC at the boss's bar's piece for the length at $2745, as StageBossBarAddress152D, from
 * $7D40.
 * HOW: The page in zTemp08-09; a subtract, nibbles and shifts into zTemp00-01, adds; SetVramAddresses3CD.
 * WAITS: no.
 * Entry: nothing. Exit, length 0: A 0, N and Z from it. Else: as SetVramAddresses3CD. Clobbers zTemp00-01, zTemp08-09.
 */
StageRegs StageBossBarAddress3CD(StageRegs r)
{
    Sum sum;
    uint8_t length, value;

    r = loaded(r, rd8(BossBarLength));
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
    length = rd8(BossBarLength);
    r = bar_address(r, length, 0x40, 0);
    WR8(ZP->temp00, r.a);
    value = RD8(ZP->temp01);
    sum = add_bytes(value, 0x7D, r.c);
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
    return set_vram_addresses_call(r, 0x8C02);
}

void abi_StageBossBarAddress3CD(void)
{
    regs_exit_shim(StageBossBarAddress3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:41B StageBossBarFlash3CD
 * @run 6B:8C1B
 * @unreachable 6B:8C1B-6B:8C59 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8C1B runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The boss's bar at its end, as StageBossBarFlash152D (the addresses at $8C03, StageBarErase3CD); the
 * speed's high byte at $2DB5 counted only when its low byte carries.
 * HOW: A loop of four in zTemp01; an add and an increment; the kernel; stores.
 * WAITS: no.
 * Entry: nothing. Exit: A the boss's page, N and Z from it, the rest as $4045 left them.
 */
StageRegs StageBossBarFlash3CD(StageRegs r)
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
        r.y = rd8((uint16_t)(0x8C04 + index));
        r = loaded(r, rd8((uint16_t)(0x8C03 + index)));
        r.x = r.a;
        CALL(0x8C2F, StageBarErase3CD(r));
        r.nz = INC8(ZP->temp01);
        r.nz = INC8(ZP->temp01);
        r = loaded(r, RD8(ZP->temp01));
        r = compared(r, 0x08);
    } while (!r.c);
    r.nz = INC8(OBJ->work_c[69]);
    value = RD8(OBJ->vx_lo[37]);
    sum = add_bytes(value, 0x0C, false);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(OBJ->vx_lo[37], sum.value);
    if (sum.carry) {
        r.nz = INC8(OBJ->vx_hi[53]);
    }
    r.x = 0x45;
    r.nz = 0x45;
    r = vector_call(r, 0x8C4F, abi_VecApplyVelocity);
    r = loaded(r, RD8(OBJ->x_frac[45]));
    if (r.a == 0) {
        return r;
    }
    WR8(OBJ->frame[53], 0);
    RMW8(ZP->field8e, old, old & (uint8_t)~0x04);
    return r;
}

void abi_StageBossBarFlash3CD(void)
{
    regs_exit_shim(StageBossBarFlash3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:45A StageBarErase3CD
 * @run 6B:8C5A
 * @unreachable 6B:8C5A-6B:8C79 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8C5A runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Blanks a bar's piece in VRAM at the address in X and Y, in 64 words: each word ANDed (AndVramData,
 * through its C) with a mask of the kernel's random piece at $4051, zRandom2, zRandom1 and the flash step's row of
 * the table at $8C13, X kept on the stack.
 * HOW: SetVramAddresses3CD; a loop down X.
 * WAITS: no.
 * Entry: X, Y the VRAM address. Exit: X 0, N and Z from it; A the word's high byte, Y the mask.
 */
StageRegs StageBarErase3CD(StageRegs r)
{
    uint8_t mask;

    r = set_vram_addresses_call(r, 0x8C5D);
    for (r.x = 0x40; r.x != 0; r.x--) {
        r.y = (uint8_t)(RD8(OBJ->work_c[69]) & 0x07);
        r = vector_call(r, 0x8C68, abi_VecNextRandom);
        mask = (uint8_t)(r.a | RD8(ZP->random2));
        mask = (uint8_t)(mask | RD8(ZP->random1));
        mask = (uint8_t)(mask | rd8((uint16_t)(0x8C13 + r.y)));
        push8(r.x);
        r = loaded(r, mask);
        r.x = mask;
        r.y = mask;
        r = and_vram(r, 0x8C75);
        r.x = pull8();
    }
    r.nz = 0;
    return r;
}

void abi_StageBarErase3CD(void)
{
    regs_exit_shim(StageBarErase3CD(regs_entry_shim()));
}

/* One row of a digit's five through zTemp00-01. */
static uint8_t digit_row(uint8_t y)
{
    return RD8_PTR_Y(ZP->temp00, y);
}

/*
 * @routine 03CE:47A StageDrawDigit3CD
 * @run 6B:8C7A
 * @unreachable 6B:8C7A-6B:8CF9 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8C7A runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Draws the digit in A into the display at the place in X, as StageDrawDigit152D, from the glyphs at
 * $8D15 up (StageDigitAddress3CD).
 * HOW: The glyph's address in zTemp00-01; StageDigitAddress3CD by BSR; loops of five reads and writes of the VDC's data
 * port with shifts through zTemp02-03.
 * WAITS: no.
 * Entry: A the digit, X the place. Exit: Y 5, Z set, C set. Clobbers zTemp00-04.
 */
StageRegs StageDrawDigit3CD(StageRegs r)
{
    Sum sum;
    uint8_t digit, value, old;

    WR8(ZP->temp04, r.x);
    digit = r.a;
    WR8(ZP->temp00, digit);
    r.c = (digit & 0x40) != 0;
    value = RD8(ZP->temp00);
    sum = add_bytes((uint8_t)(digit << 2), value, r.c);
    sum = add_bytes(sum.value, 0x15, sum.carry);
    WR8(ZP->temp00, sum.value);
    sum = add_bytes(0, 0x8D, sum.carry);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp01, sum.value);
    CALL(0x8C8D, StageDigitAddress3CD(r));
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
    CALL(0x8CD9, StageDigitAddress3CD(r));
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

void abi_StageDrawDigit3CD(void)
{
    regs_exit_shim(StageDrawDigit3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:4FA StageDigitAddress3CD
 * @run 6B:8CFA
 * @unreachable 6B:8CFA-6B:8D08 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8CFA runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Points the VDC at the display's place zTemp04 (its even pair) from the table of addresses at $8D09.
 * HOW: Two table reads; a jump to SetVramAddresses3CD.
 * WAITS: no.
 * Entry: zTemp04 the place. Exit: as SetVramAddresses3CD.
 */
StageRegs StageDigitAddress3CD(StageRegs r)
{
    uint8_t index = (uint8_t)(RD8(ZP->temp04) & 0xFE);

    r.x = index;
    r.a = rd8((uint16_t)(0x8D09 + index));
    r.y = rd8((uint16_t)(0x8D0A + index));
    r.x = r.a;
    return SetVramAddresses3CD(r);
}

void abi_StageDigitAddress3CD(void)
{
    regs_exit_shim(StageDigitAddress3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:55D StagePausedSelect3CD
 * @run 6B:8D5D
 * @unreachable 6B:8D5D-6B:8D8D no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8D5D runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The pause screen's choice, reached from StagePausedFrame3CD: Select with Run (bits 7 and 2 of
 * BiosJoyTrigger together) draws the map's debug marks (StageMapDebug3CD); else as StagePausedSelect152D, the choices
 * from the table at $8D51 and the frames from $8B18.
 * HOW: Pad bits; table reads; stores; a jump to StageMapDebug3CD.
 * WAITS: no.
 * Entry: nothing. Exit, no press: A 0, N and Z from it. Else: A $99, N and Z from it, X the new choice.
 */
StageRegs StagePausedSelect3CD(StageRegs r)
{
    uint8_t choice;

    r = loaded(r, (uint8_t)(rd8(BiosJoyTrigger) & 0x84));
    r = compared(r, 0x84);
    if (r.nz == 0) {
        return StageMapDebug3CD(r);
    }
    r = loaded(r, (uint8_t)(rd8(BiosJoyTrigger) & 0x50));
    if (r.a == 0) {
        return r;
    }
    r.x = RD8(ZP->field93);
    r = loaded(r, (uint8_t)(r.a & 0x40));
    if (r.a != 0) {
        r.x = (uint8_t)(r.x + 2);
    }
    choice = rd8((uint16_t)(0x8D51 + r.x));
    WR8(ZP->field93, choice);
    r.x = choice;
    r = loaded(r, rd8((uint16_t)(0x8B18 + choice)));
    WR8(OBJ->frame[54], r.a);
    r = loaded(r, 1);
    wr8(PlayerItemFirstUse, 1);
    r = loaded(r, 0x99);
    WR8(ZP->field94, 0x99);
    return r;
}

void abi_StagePausedSelect3CD(void)
{
    regs_exit_shim(StagePausedSelect3CD(regs_entry_shim()));
}

/* The pause's menu step: the kernel's piece at $4018 twice (X 2 and 3, A from the row's first two bytes at $8E0D), the
 * row's frame into $2A9F, then the kernel's piece at $400F with X 4 for the row's sound unless it is the one zPaused's
 * bit 2 (after its shift into zTemp01) already has. */
static Regs pause_menu_row(Regs r, uint8_t row)
{
    WR8(ZP->temp00, row);
    r = loaded(r, rd8((uint16_t)(0x8E0D + row)));
    r.x = 2;
    r.nz = 2;
    r = vector_call(r, 0x8DC7, abi_VecStartScriptSlotKind4);
    r.x = RD8(ZP->temp00);
    r = loaded(r, rd8((uint16_t)(0x8E0E + r.x)));
    r.x = 3;
    r.nz = 3;
    r = vector_call(r, 0x8DD1, abi_VecStartScriptSlotKind4);
    r.x = RD8(ZP->temp00);
    r = loaded(r, rd8((uint16_t)(0x8E10 + r.x)));
    WR8(OBJ->frame[55], r.a);
    r = loaded(r, RD8(ZP->paused));
    WR8(ZP->temp01, r.a);
    r.x = RD8(ZP->temp00);
    r = loaded(r, rd8((uint16_t)(0x8E0F + r.x)));
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
    return vector_tail(r, abi_VecStartListChannelKind2);
}

/*
 * @routine 03CE:58E StagePausedFrame3CD
 * @run 6B:8D8E
 * @unreachable 6B:8D8E-6B:8E0C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8D8E runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The paused frame. zPaused's bit 1 clear: the choice (StagePausedSelect3CD). Else zPaused counts up by
 * 4 a frame (its bits 2 up the menu's step, bit 0 the side it scrolls from) through the menu's rows (the table of four
 * bytes a row at $8E0D, from $19 down or up to $1A); past them only bit 0 is kept. Steps $25 and $27 at the end of the
 * scroll redraw the score, or the stock and the points to add.
 * HOW: Shifts of zPaused through zTemp00; a subtract; the row's step through the kernel (pause_menu_row); jumps to
 * StageDrawScore3CD or StageDrawScoreAdd3CD, a call of StageDrawStock3CD.
 * WAITS: no.
 * Entry: nothing. Exit: as the step that ran left them.
 */
StageRegs StagePausedFrame3CD(StageRegs r)
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
        return StagePausedSelect3CD(r);
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
        return StageDrawScore3CD(r);
    }
    r = compared(r, 0x27);
    if (r.nz != 0) {
        return r;
    }
    CALL(0x8E0A, StageDrawStock3CD(r));
    return StageDrawScoreAdd3CD(r);
}

void abi_StagePausedFrame3CD(void)
{
    regs_exit_shim(StagePausedFrame3CD(regs_entry_shim()));
}


/*
 * @routine 03CE:641 StageOpeningStart3CD
 * @run 6B:8E41
 * @unreachable 6B:8E41-6B:8E6C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8E41 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Starts the stage's opening when zField8E's bit 0 is clear, as StageOpeningStart152D without its sound:
 * the display's pieces 3 and 4, the opening's objects placed, TimeSlowMode 2, the wait at $33E4 cleared.
 * HOW: A bit test; calls with X 3 and 4; stores.
 * WAITS: no.
 * Entry: nothing. Exit, bit 0 set: everything kept. Else: A 2, N and Z from it, X 4, Y and the flags as
 * StageDrawPiece3CD left them.
 */
StageRegs StageOpeningStart3CD(StageRegs r)
{
    if ((RD8(ZP->field8e) & 0x01) != 0) {
        return r;
    }
    r.x = 3;
    r.nz = 3;
    CALL(0x8E4A, StageDrawPiece3CD(r));
    r.x++;
    r.nz = r.x;
    CALL(0x8E4E, StageDrawPiece3CD(r));
    r.nz = DEC8(OBJ->x_hi[4]);
    r.nz = DEC8(OBJ->x_frac[45]);
    r = loaded(r, 0x04);
    WR8(OBJ->y_lo[84], 0x04);
    WR8(OBJ->vx_hi[53], 0x04);
    r = loaded(r, 0x20);
    WR8(OBJ->work_b[84], 0x20);
    WR8(OBJ->work_b[85], 0x20);
    r = loaded(r, 0x02);
    wr8(TimeSlowMode, 0x02);
    WR8(OBJ->hit_taken[52], 0);
    return r;
}

void abi_StageOpeningStart3CD(void)
{
    regs_exit_shim(StageOpeningStart3CD(regs_entry_shim()));
}

/* A count of the opening's motion run out: the next step's speed (halved into HIGH, its low bit into the top of LOW)
 * from the table at $8EEA and its count from $8EF0. */
static Regs opening_next_step(Regs r, uint16_t high, uint16_t low, uint16_t count)
{
    uint8_t value = rd8((uint16_t)(0x8EEA + r.y));

    wr8(high, (uint8_t)(value >> 1));
    wr8(low, (uint8_t)((value & 0x01) != 0 ? 0x80 : 0x00));
    r = loaded(r, rd8((uint16_t)(0x8EF0 + r.y)));
    r.c = false;
    wr8(count, r.a);
    return r;
}

/*
 * @routine 03CE:66D StageOpeningStep3CD
 * @run 6B:8E6D
 * @stop 8EEB
 * @unreachable 6B:8E6D-6B:8EEA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8E6D runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: One frame of the stage's opening: the kernel's piece at $407B once (for a stage other than 0), the two
 * objects stepped as in StageOpeningStep152D (their steps from $8EEA and $8EF0), but moved by the kernel's pieces at
 * $4063 (object $45) and $4060 (object $44); at the first's step 7 the opening ends; at the second's step 6 the
 * screen's row fade runs (StageOpeningFade3CD).
 * HOW: Decrements and table reads; the kernel; a branch to StageOpeningFade3CD.
 * WAITS: no.
 * Entry: nothing. Exit: as the last piece that ran left them.
 */
StageRegs StageOpeningStep3CD(StageRegs r)
{
    r = loaded(r, RD8(OBJ->hit_taken[52]));
    if (r.a == 0) {
        r = loaded(r, RD8(ZP->stage));
        if (r.a != 0) {
            r = vector_call(r, 0x8E79, abi_VecCdPlayAgain);
        }
        r.nz = INC8(OBJ->hit_taken[52]);
    }
    r.nz = DEC8(OBJ->work_b[85]);
    if (r.nz == 0) {
        r.nz = INC8(OBJ->work_b[13]);
        r.y = RD8(OBJ->work_b[13]);
        r.c = r.y >= 0x07;
        r.nz = (uint8_t)(r.y - 0x07);
        if (r.nz == 0) {
            WR8(OBJ->frame[53], 0);
            RMW8(ZP->field8e, old, old | 0x01);
            return r;
        }
        r = opening_next_step(r, ADDR_OF8(OBJ->vx_hi[53]), ADDR_OF8(OBJ->vx_lo[37]), ADDR_OF8(OBJ->work_b[85]));
    }
    r = loaded(r, RD8(OBJ->work_b[13]));
    r = compared(r, 0x06);
    if (r.nz == 0) {
        r = loaded(r, RD8(OBJ->work_b[85]));
        r = compared(r, 0x1F);
        if (r.nz == 0) {
            WR8(OBJ->frame[52], 0);
        }
        WR8(OBJ->work_c[68], r.a);
    }
    r.x = 0x45;
    r.nz = 0x45;
    r = vector_call(r, 0x8EBC, abi_VecApplyVelocityY);
    r.nz = DEC8(OBJ->work_b[84]);
    if (r.nz == 0) {
        r.nz = INC8(OBJ->work_b[12]);
        r.y = RD8(OBJ->work_b[12]);
        r = opening_next_step(r, ADDR_OF8(OBJ->y_lo[84]), ADDR_OF8(OBJ->y_frac[68]), ADDR_OF8(OBJ->work_b[84]));
    }
    r.x = 0x44;
    r.nz = 0x44;
    r = vector_call(r, 0x8EDE, abi_VecApplyVelocityX);
    r = loaded(r, RD8(OBJ->work_b[12]));
    r = compared(r, 0x06);
    if (r.nz != 0) {
        return r;
    }
    r = loaded(r, RD8(OBJ->work_c[68]));
    return StageOpeningFade3CD(r);
}

void abi_StageOpeningStep3CD(void)
{
    regs_exit_shim(StageOpeningStep3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:707 StageOpeningFade3CD
 * @run 6B:8F07
 * @unreachable 6B:8F07-6B:8F51 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8F07 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The opening's fade of the screen's rows with A the count, as StageOpeningFade152D: the level and the
 * group of rows from the table at $8EF7, each row faded by StageOpeningCopyRow3CD after the kernel's piece at $4051;
 * the row's address moves on with its high byte counted only on a carry.
 * HOW: Masks, a subtract and table reads into zTemp10-12 and zProduct0; a loop of rows.
 * WAITS: no.
 * Entry: A the count. Exit: N and Z from the row count's decrement (Z set); C and V from the address's add.
 */
StageRegs StageOpeningFade3CD(StageRegs r)
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
    r = loaded(r, rd8((uint16_t)(0x8EF7 + r.x)));
    WR8(ZP->temp10, r.a);
    r = loaded(r, rd8((uint16_t)(0x8EF8 + r.x)));
    WR8(ZP->temp11, r.a);
    r = loaded(r, rd8((uint16_t)(0x8EF9 + r.x)));
    WR8(ZP->temp12, r.a);
    r = loaded(r, rd8((uint16_t)(0x8EFA + r.x)));
    WR8(ZP->product.lo, r.a);
    do {
        r = vector_call(r, 0x8F40, abi_VecNextRandom);
        CALL(0x8F42, StageOpeningCopyRow3CD(r));
        value = RD8(ZP->temp10);
        sum = add_bytes(value, RD8(ZP->product.lo), false);
        r = loaded(r, sum.value);
        r.c = sum.carry;
        r.v = sum.overflow;
        WR8(ZP->temp10, sum.value);
        if (sum.carry) {
            r.nz = INC8(ZP->temp11);
        }
        r.nz = DEC8(ZP->temp12);
    } while (r.nz != 0);
    return r;
}

void abi_StageOpeningFade3CD(void)
{
    regs_exit_shim(StageOpeningFade3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:752 StageOpeningCopyRow3CD
 * @run 6B:8F52
 * @stop 8FA1
 * @unreachable 6B:8F52-6B:8FA0 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8F52 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Fades one row of the screen in VRAM, as StageOpeningCopyRow152D, the passes over the buffer at $3980 by
 * StageFadeRowNext3CD and StageFadeRowSelf3CD; then runs on into the write back (StageOpeningWriteRow152D, whose bytes
 * are at $8FA1).
 * HOW: SetVramAddresses3CD; loops of the VDC's data port and of the buffer.
 * WAITS: no.
 * Entry: zTemp10-11 the row's address, zMultiplierHi the level. Exit: as StageOpeningWriteRow152D. Clobbers zTemp00.
 */
StageRegs StageOpeningCopyRow3CD(StageRegs r)
{
    uint8_t value, mask;

    r.x = RD8(ZP->temp10);
    r.y = RD8(ZP->temp11);
    r = set_vram_addresses_call(r, 0x8F59);
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
            r = loaded(r, mask);
            WR8(ZP->temp00, mask);
            CALL(0x8F8B, StageFadeRowNext3CD(r));
            r.x = (uint8_t)(r.x + 2);
            r.y--;
        } while (r.y != 0);
    }
    r.y = RD8(ZP->multiplier.hi);
    r.x = 0;
    do {
        mask = RD8(ZP->random2);
        mask = (uint8_t)(mask | RD8(ZP->random3));
        r = loaded(r, mask);
        WR8(ZP->temp00, mask);
        CALL(0x8F9C, StageFadeRowSelf3CD(r));
        r.x = (uint8_t)(r.x + 2);
        r.y--;
    } while (r.y != 0);
    r.nz = 0;
    return StageOpeningWriteRow152D(r);
}

void abi_StageOpeningCopyRow3CD(void)
{
    regs_exit_shim(StageOpeningCopyRow3CD(regs_entry_shim()));
}

/* One pass of the row's fade by the table of eight offsets at $8FEA: each of the word's bytes ANDed with zTemp00, read
 * from NEXT bytes on, X and Y kept on the stack. */
static Regs fade_row_by_table(Regs r, uint8_t next)
{
    Sum sum;
    uint8_t value;

    WR8(ZP->temp08, r.x);
    push8(r.x);
    push8(r.y);
    for (r.x = 0x07; (r.x & 0x80) == 0; r.x--) {
        value = rd8((uint16_t)(0x8FEA + r.x));
        sum = add_bytes(value, RD8(ZP->temp08), false);
        r.c = sum.carry;
        r.v = sum.overflow;
        r.y = sum.value;
        value = rd8((uint16_t)(ROW_BUFFER + 2 + next + r.y));
        value = (uint8_t)(value & RD8(ZP->temp00));
        r.a = value;
        wr8((uint16_t)(ROW_BUFFER + 2 + r.y), value);
    }
    r.y = pull8();
    r.x = pull8();
    r.nz = r.x;
    return r;
}

/*
 * @routine 03CE:7B4 StageFadeRowNext3CD
 * @run 6B:8FB4
 * @unreachable 6B:8FB4-6B:8FCE no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8FB4 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: One word of the row's fade: its eight bytes (the four planes' pairs, offsets from $8FEA) at $3980 + X
 * each the next word's byte ANDed with the mask in zTemp00.
 * HOW: X in zTemp08; X and Y kept on the stack; a loop down X.
 * WAITS: no.
 * Entry: X the word's offset, zTemp00 the mask. Exit: X and Y kept, N and Z from X; A the last byte written; C and V
 * from the last add.
 */
StageRegs StageFadeRowNext3CD(StageRegs r)
{
    return fade_row_by_table(r, 2);
}

void abi_StageFadeRowNext3CD(void)
{
    regs_exit_shim(StageFadeRowNext3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:7CF StageFadeRowSelf3CD
 * @run 6B:8FCF
 * @unreachable 6B:8FCF-6B:8FE9 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8FCF runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: One word of the row's fade: its eight bytes ANDed in place with the mask in zTemp00.
 * HOW: X in zTemp08; X and Y kept on the stack; a loop down X.
 * WAITS: no.
 * Entry: X the word's offset, zTemp00 the mask. Exit: as StageFadeRowNext3CD.
 */
StageRegs StageFadeRowSelf3CD(StageRegs r)
{
    return fade_row_by_table(r, 0);
}

void abi_StageFadeRowSelf3CD(void)
{
    regs_exit_shim(StageFadeRowSelf3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:7F2 StageDrawStock3CD
 * @run 6B:8FF2
 * @unreachable 6B:8FF2-6B:8FFA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8FF2 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Draws the stock zStock (two digits at places 0 and 1).
 * HOW: StageDrawNumber3CD with X 0, X kept on the stack across it.
 * WAITS: no.
 * Entry: nothing. Exit: X kept, N and Z from it; A, Y, C and V as StageDrawNumber3CD left them.
 */
StageRegs StageDrawStock3CD(StageRegs r)
{
    push8(r.x);
    r = loaded(r, RD8(ZP->stock));
    r.x = 0;
    CALL(0x8FF9, StageDrawNumber3CD(r));
    r.x = pull8();
    r.nz = r.x;
    return r;
}

void abi_StageDrawStock3CD(void)
{
    regs_exit_shim(StageDrawStock3CD(regs_entry_shim()));
}

/*
 * @routine 03CE:7FB StageDrawScoreAdd3CD
 * @run 6B:8FFB
 * @unreachable 6B:8FFB-6B:900B no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $8FFB runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Draws the points to add, zScoreAddLo and zScoreAddHi (four digits from place 3).
 * HOW: StageDrawNumber3CD with X 3 and 5, X kept on the stack across them.
 * WAITS: no.
 * Entry: nothing. Exit: X kept, N and Z from it; A, Y, C and V as StageDrawNumber3CD left them.
 */
StageRegs StageDrawScoreAdd3CD(StageRegs r)
{
    push8(r.x);
    r = loaded(r, RD8(ZP->score_add.lo));
    r.x = 3;
    r.nz = 3;
    CALL(0x9003, StageDrawNumber3CD(r));
    r = loaded(r, RD8(ZP->score_add.hi));
    r.x = 5;
    r.nz = 5;
    CALL(0x900A, StageDrawNumber3CD(r));
    r.x = pull8();
    r.nz = r.x;
    return r;
}

void abi_StageDrawScoreAdd3CD(void)
{
    regs_exit_shim(StageDrawScoreAdd3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:00C StageDrawScore3CD
 * @run 6B:900C
 * @unreachable 6B:900C-6B:9023 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $900C runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Draws the score zScore0-2 (six digits from place 1).
 * HOW: StageDrawNumber3CD with X 1, 3 and 5, X kept on the stack across them.
 * WAITS: no.
 * Entry: nothing. Exit: X kept, N and Z from it; A, Y, C and V as StageDrawNumber3CD left them.
 */
StageRegs StageDrawScore3CD(StageRegs r)
{
    push8(r.x);
    r = loaded(r, RD8(ZP->score.lo));
    r.x = 1;
    r.nz = 1;
    CALL(0x9014, StageDrawNumber3CD(r));
    r = loaded(r, RD8(ZP->score.mid));
    r.x = 3;
    r.nz = 3;
    CALL(0x901B, StageDrawNumber3CD(r));
    r = loaded(r, RD8(ZP->score.hi));
    r.x = 5;
    r.nz = 5;
    CALL(0x9022, StageDrawNumber3CD(r));
    r.x = pull8();
    r.nz = r.x;
    return r;
}

void abi_StageDrawScore3CD(void)
{
    regs_exit_shim(StageDrawScore3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:024 StageDrawNumber3CD
 * @run 6B:9024
 * @unreachable 6B:9024-6B:9034 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9024 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Draws the two decimal digits of A at the places X and X + 1, the low digit first.
 * HOW: StageDrawDigit3CD with the low nibble, X and A kept on the stack; then a jump to it with the high nibble.
 * WAITS: no.
 * Entry: A the number, X the place. Exit: as StageDrawDigit3CD.
 */
StageRegs StageDrawNumber3CD(StageRegs r)
{
    uint8_t number;

    push8(r.x);
    push8(r.a);
    r = loaded(r, (uint8_t)(r.a & 0x0F));
    CALL(0x902B, StageDrawDigit3CD(r));
    number = pull8();
    r.c = (number & 0x08) != 0;
    r = loaded(r, (uint8_t)(number >> 4));
    r.x = pull8();
    r.x++;
    r.nz = r.x;
    return StageDrawDigit3CD(r);
}

void abi_StageDrawNumber3CD(void)
{
    regs_exit_shim(StageDrawNumber3CD(regs_entry_shim()));
}

/* The list's pointer in zTemp00-01 moved on by A bytes, its high byte counted on a carry. */
static Regs list_advance(Regs r)
{
    Sum sum;
    uint8_t value;

    value = RD8(ZP->temp00);
    sum = add_bytes(r.a, value, false);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp00, sum.value);
    if (sum.carry) {
        r.nz = INC8(ZP->temp01);
    }
    return r;
}

/*
 * @routine 03CF:035 StageUnpackList3CD
 * @run 6B:9035
 * @unreachable 6B:9035-6B:9096 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9035 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Unpacks the route's list (the table at $9D63 by zField82) into memory, as StageUnpackList152D; the
 * pointers' high bytes are counted only on a carry.
 * HOW: TAI; the list in zTemp00-01, the target in zTemp02-03, the count in zTemp04.
 * WAITS: no.
 * Entry: nothing. Exit: A $FF, Z set, C set. Clobbers zTemp00-04.
 */
StageRegs StageUnpackList3CD(StageRegs r)
{
    return StageUnpackList3CDAt(r, &route_3cd);
}

/* StageUnpackList3CD's body, for the builds of it with their own tables. */
StageRegs StageUnpackList3CDAt(StageRegs r, const StageRouteBuild *build)
{
    Sum sum;
    uint8_t route, count, value;

    tai_shim(r, KernelZeros, SaveBuffer, 0x02FF);
    route = RD8(ZP->field82);
    r.c = (route & 0x80) != 0;
    r = loaded(r, (uint8_t)(route << 1));
    r.y = r.a;
    r = loaded(r, rd8((uint16_t)(build->unpack_lists + r.y)));
    WR8(ZP->temp00, r.a);
    r = loaded(r, rd8((uint16_t)(build->unpack_lists + 1 + r.y)));
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
        sum = add_bytes(r.a, value, false);
        WR8(ZP->temp02, sum.value);
        if (sum.carry) {
            (void)INC8(ZP->temp03);
        }
        r = loaded(r, 2);
    }
}

void abi_StageUnpackList3CD(void)
{
    regs_exit_shim(StageUnpackList3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:097 ClearBitRowsFour3CD
 * @run 6B:9097
 * @unreachable 6B:9097-6B:909C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9097 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Runs StageClearBitRows3CD four times: three calls and the run into it.
 * HOW: Three BSR and the bytes of StageClearBitRows3CD after them.
 * WAITS: no.
 * Entry, exit: as StageClearBitRows3CD.
 */
StageRegs ClearBitRowsFour3CD(StageRegs r)
{
    CALL(0x9099, StageClearBitRows3CD(r));
    CALL(0x909B, StageClearBitRows3CD(r));
    CALL(0x909D, StageClearBitRows3CD(r));
    return StageClearBitRows3CD(r);
}

void abi_ClearBitRowsFour3CD(void)
{
    regs_exit_shim(ClearBitRowsFour3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:09D StageClearBitRows3CD
 * @run 6B:909D
 * @unreachable 6B:909D-6B:90E4 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $909D runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Clears one bit of four VRAM words $10 apart for the map entry zTemp0F names, as StageClearBitRows152D;
 * each word pointed at by SetVramAddresses3CD (X and Y kept on the stack) and ANDed with the masks at $90E5 and $90ED.
 * HOW: The address in zTemp0A-0B, the count in zTemp0E; the VDC's data port.
 * WAITS: no.
 * Entry: zTemp08-09 the base, zTemp0F the entry. Exit: A the new zTemp0F, N, Z, C and V from its add; X and Y kept.
 */
StageRegs StageClearBitRows3CD(StageRegs r)
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
        push8(r.x);
        push8(r.y);
        r.x = RD8(ZP->temp0a);
        r.y = RD8(ZP->temp0b);
        r = set_vram_addresses_call(r, 0x90BE);
        r.y = pull8();
        value = RD8(VDC->data_lo);
        value = (uint8_t)(value & rd8((uint16_t)(0x90E5 + r.y)));
        WR8(VDC->data_lo, value);
        value = RD8(VDC->data_hi);
        value = (uint8_t)(value & rd8((uint16_t)(0x90ED + r.y)));
        WR8(VDC->data_hi, value);
        r.x = pull8();
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

void abi_StageClearBitRows3CD(void)
{
    regs_exit_shim(StageClearBitRows3CD(regs_entry_shim()));
}
/*
 * @routine 03CF:0FD StageSubModeTally3CD
 * @run 6B:90FD
 * @stop 910D
 * @unreachable 6B:90FD-6B:910C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $90FD runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Sub-mode 6, the tally after the boss: the events step, the display, the overlays' frame, then the
 * tally's step at zField80 from the eight at $910D (the start, the board, the board's reveal, a pause, the health and
 * hearts counted into the score, a wait, the stock bonus, the end).
 * HOW: StageModeEventsStep152D, StageHudStep3CD, StageOverlayFrame3CD; zField80 doubled indexes the table, whose
 * word is jumped through.
 * WAITS: as the step.
 * Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice zField80, N and Z from it, C its
 * bit 7.
 */
StageRegs StageSubModeTally3CD(StageRegs r)
{
    uint8_t step, twice;
    uint16_t handler;

    CALL(0x9100, StageModeEventsStep152D(r));
    CALL(0x9103, StageHudStep3CD(r));
    CALL(0x9106, StageOverlayFrame3CD(r));
    step = RD8(ZP->field80);
    twice = (uint8_t)(step << 1);
    r.c = (step & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(0x910D, twice);
    switch (handler) {
    case 0x911D:
        return TallyStart3CD(r);
    case 0x912B:
        return TallyBoard3CD(r);
    case 0x919D:
        return TallyReveal3CD(r);
    case 0x92AF:
        return TallyPause3CD(r);
    case 0x92BA:
        return TallyCount3CD(r);
    case 0x9310:
        return TallyWaitPress3CD(r);
    case 0x9322:
        return TallyStockBonus3CD(r);
    case 0x9340:
        return TallyEnd3CD(r);
    default:
        return resident_tail(r, handler);
    }
}

void abi_StageSubModeTally3CD(void)
{
    regs_exit_shim(StageSubModeTally3CD(regs_entry_shim()));
}

/* 03CD's tally screen. */
static const TallyBuild tally_3cd = {
    TALLY_CELLS, 0x93A9, ADDR_OF8(OBJ->work_b[16]), ADDR_OF8(OBJ->work_b[13]), ADDR_OF8(OBJ->frame[51]),
    0x9175, 0x922A, 0x9390, 0x46, true,
    StageDrawPiece3CD, StageClearWords3CD, SetVramWriteAddress3CD, SetVramAddresses3CD
};

/* JSR $934B: the frame pushed; popped only when TallyWait152D did not drop it. */
static TallyWaitEnd tally_wait_at(Regs r, uint16_t ret, uint16_t wait)
{
    TallyWaitEnd end;

    jsr_push(ret);
    end = TallyWait152DAt(r, wait);
    if (!end.dropped) {
        jsr_pop();
    }
    return end;
}

/* The wait on 03CD's own byte. */
static TallyWaitEnd tally_wait(Regs r, uint16_t ret)
{
    return tally_wait_at(r, ret, ADDR_OF8(OBJ->work_b[16]));
}

/*
 * @routine 03CF:11D TallyStart3CD
 * @run 6B:911D
 * @stop 9128
 * @unreachable 6B:911D-6B:9127 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $911D runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The tally's first step, after the wait: the kernel's piece at $4087, sound $46, and the next step
 * (runs on into TallyNext152D).
 * HOW: TallyWait152D; $4087; the sound hook.
 * WAITS: no.
 * Entry: nothing. Exit: as TallyWait152D when it ends the step; else as TallyNext152D.
 */
StageRegs TallyStart3CD(StageRegs r)
{
    return TallyStart3CDAt(r, 0x911D, &tally_3cd);
}

/* TallyStart3CD's body, for the builds of it elsewhere: where the build runs and what it uses. */
StageRegs TallyStart3CDAt(StageRegs r, uint16_t at, const TallyBuild *build)
{
    TallyWaitEnd wait = tally_wait_at(r, at + 0x3, build->wait);

    r = wait.r;
    if (wait.dropped) {
        return r;
    }
    r = vector_call(r, at + 0x6, abi_VecStopSound);
    r = loaded(r, build->start_sound);
    r = vector_call(r, at + 0xB, abi_VecSoundCommand);
    return TallyNext152D(r);
}

void abi_TallyStart3CD(void)
{
    regs_exit_shim(TallyStart3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:12B TallyBoard3CD
 * @run 6B:912B
 * @unreachable 6B:912B-6B:9174 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $912B runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The tally's board: the kernel's piece at $4012 (A $16), the display's pieces 5 to 7
 * (StageDrawPiece3CD), the hearts kept at $330D, the full-health mark ($2A9B $18 at zPlayerHealth $5C), three
 * blocks of VRAM words cleared (StageClearWords3CD: $0E at $5000, $0C at $5400, $0A at $5D80), the board's cells
 * at $3980 from the 62 at $93A9; the next step (TallyNext152D), then runs on into TallyDigits3CD.
 * HOW: Calls; a copy loop.
 * WAITS: no.
 * Entry: nothing. Exit: as TallyDigits3CD.
 */
StageRegs TallyBoard3CD(StageRegs r)
{
    return TallyBoard3CDAt(r, 0x912B, &tally_3cd);
}

/* TallyBoard3CD's body, for the builds of it elsewhere: where the build runs and what it uses. */
StageRegs TallyBoard3CDAt(StageRegs r, uint16_t at, const TallyBuild *build)
{
    r = loaded(r, 0x16);
    r = vector_call(r, at + 0x5, abi_VecStartScriptSlot0Kind1);
    r.x = 5;
    r.nz = 5;
    CALL(at + 0xA, build->draw_piece(r));
    r.x++;
    r.nz = r.x;
    CALL(at + 0xE, build->draw_piece(r));
    r.x++;
    r.nz = r.x;
    CALL(at + 0x12, build->draw_piece(r));
    r = loaded(r, RD8(ZP->field94));
    wr8(build->health_kept, r.a);
    r = loaded(r, RD8(ZP->player_health));
    r = compared(r, 0x5C);
    if (r.nz == 0) {
        r = loaded(r, 0x18);
        wr8(build->full_frame, 0x18);
    }
    r.x = 0;
    r.y = 0x50;
    r = loaded(r, 0x0E);
    CALL(at + 0x2A, build->clear_words(r));
    r.x = 0;
    r.y = 0x54;
    r = loaded(r, 0x0C);
    CALL(at + 0x32, build->clear_words(r));
    r.x = 0x80;
    r.y = 0x5D;
    r = loaded(r, 0x0A);
    CALL(at + 0x3B, build->clear_words(r));
    for (r.x = 0; r.x != 0x3E; r.x++) {
        r = loaded(r, rd8((uint16_t)(build->cell_source + r.x)));
        wr8((uint16_t)(build->cells + r.x), r.a);
    }
    r.c = true;
    r.nz = 0;
    CALL(at + 0x4A, TallyNext152D(r));
    return TallyDigits3CDAt(r, build->digits_run, build);
}

void abi_TallyBoard3CD(void)
{
    regs_exit_shim(TallyBoard3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:175 TallyDigits3CD
 * @run 6B:9175
 * @unreachable 6B:9175-6B:919C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9175 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Puts the health and the hearts on the board as digits: zPlayerHealth's tens and units
 * (StageSplitTens152D) at $3991 and $3992, zField94's two BCD digits at $39A1 and $39A2, each one more than the digit;
 * a leading zero is left out.
 * HOW: StageSplitTens152D; shifts and masks; stores.
 * WAITS: no.
 * Entry: nothing. Exit: A the hearts' last cell, N and Z from its increment; X the health's tens cell; C from the
 * compare, V from the split's add. Y kept.
 */
StageRegs TallyDigits3CD(StageRegs r)
{
    return TallyDigits3CDAt(r, 0x9175, &tally_3cd);
}

/* TallyDigits3CD's body, for the builds of it elsewhere: where the build runs and what it uses. */
StageRegs TallyDigits3CDAt(StageRegs r, uint16_t at, const TallyBuild *build)
{
    uint8_t hearts;

    r = loaded(r, RD8(ZP->player_health));
    CALL(at + 0x5, StageSplitTens152D(r));
    r.x++;
    r.c = r.x >= 0x01;
    r.nz = (uint8_t)(r.x - 0x01);
    if (r.nz != 0) {
        wr8((uint16_t)(build->cells + 0x11), r.x);
    }
    r = loaded(r, (uint8_t)(r.a + 1));
    wr8((uint16_t)(build->cells + 0x12), r.a);
    hearts = RD8(ZP->field94);
    r.c = (hearts & 0x08) != 0;
    r = loaded(r, (uint8_t)((hearts >> 4) + 1));
    r = compared(r, 0x01);
    if (r.nz != 0) {
        wr8((uint16_t)(build->cells + 0x21), r.a);
    }
    hearts = RD8(ZP->field94);
    r = loaded(r, (uint8_t)((hearts & 0x0F) + 1));
    wr8((uint16_t)(build->cells + 0x22), r.a);
    return r;
}

void abi_TallyDigits3CD(void)
{
    regs_exit_shim(TallyDigits3CD(regs_entry_shim()));
}

/* JSR $93E7: the bytes of MapStageBanks (stage_setup.c, 024F:349), a census duplicate, through its C: A the MPR6
 * bank, N and Z from it, C and V from the add. */
static Regs map_stage_banks_call(Regs r, uint16_t ret)
{
    Sum sum;

    jsr_push(ret);
    sum = MapStageBanks();
    jsr_pop();
    r = loaded(r, (uint8_t)(sum.value + 1));
    r.c = sum.carry;
    r.v = sum.overflow;
    return r;
}

/*
 * @routine 03CF:19D TallyReveal3CD
 * @run 6B:919D
 * @unreachable 6B:919D-6B:9229 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $919D runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The board's reveal, under the stage's banks: once all 62 cells at $3980 are 0 the reveal is done
 * (zPaused's bit 1 set, the wait at $3310 $40, the next step). Else a random cell that is not yet 0 is stepped by $40
 * (cleared when it carries), and drawn: a cell of the first ten columns as its 16-word glyph from the picture at $C034
 * (rows from $5D80); the others by TallyCell3CD.
 * HOW: MapStageBanks (a census duplicate at $93E7); a loop down X; the kernel's random piece at $4051 until it gives a
 * live cell; shifts into zTemp08-09, the glyph's address in zTemp10-11; SetVramWriteAddress3CD; a loop of 16 words
 * with SXY.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction that ran left them.
 */
StageRegs TallyReveal3CD(StageRegs r)
{
    return TallyReveal3CDAt(r, 0x919D, &tally_3cd);
}

/* TallyReveal3CD's body, for the builds of it elsewhere. */
StageRegs TallyReveal3CDAt(StageRegs r, uint16_t at, const TallyBuild *build)
{
    uint16_t shift = build->reveal_pauses ? 0 : 2;
    Sum sum;
    uint8_t cell, value, old, tens;

    r = map_stage_banks_call(r, at + 0x3);
    for (r.x = 0x3D; (r.x & 0x80) == 0; r.x--) {
        r = loaded(r, rd8((uint16_t)(build->cells + r.x)));
        if (r.a != 0) {
            break;
        }
    }
    if ((r.x & 0x80) != 0) {
        r.nz = r.x;
        if (build->reveal_pauses) {
            RMW8(ZP->paused, old, old | 0x02);
        }
        r = loaded(r, 0x40);
        wr8(build->wait, 0x40);
        return TallyNext152D(r);
    }
    for (;;) {
        r = vector_call(r, (uint16_t)(at + 0x1A - shift), abi_VecNextRandom);
        r = loaded(r, (uint8_t)(r.a & 0x3F));
        r = compared(r, 0x3E);
        if (r.c) {
            continue;
        }
        r.x = r.a;
        r = loaded(r, rd8((uint16_t)(build->cells + r.x)));
        if (r.a != 0) {
            break;
        }
    }
    WR8(ZP->temp00, r.a);
    sum = add_bytes(r.a, 0x40, r.c);
    wr8((uint16_t)(build->cells + r.x), sum.value);
    if (sum.carry) {
        wr8((uint16_t)(build->cells + r.x), 0);
    }
    r.c = sum.carry;
    r.v = sum.overflow;
    cell = (uint8_t)(r.x & 0x3F);
    r = loaded(r, cell);
    r = compared(r, 0x0A);
    if (r.c) {
        return TallyCell3CDAt(r, build->cell_run, build);
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
    sum = add_bytes(0x34, value, false);
    WR8(ZP->temp10, sum.value);
    value = RD8(ZP->temp09);
    sum = add_bytes(0xC0, value, sum.carry);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp11, sum.value);
    CALL((uint16_t)(at + 0x77 - shift), build->write_address(r));
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

void abi_TallyReveal3CD(void)
{
    regs_exit_shim(TallyReveal3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:22A TallyCell3CD
 * @run 6B:922A
 * @unreachable 6B:922A-6B:92AE no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $922A runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Draws one of the board's cells past the first ten columns: A the cell's index, zTemp00 its value
 * (its top two bits the shade, its low six the glyph). The cell's place gives the VRAM word (rows of eight from $5008,
 * odd and even halves) and which of the word's bytes it fills; eight bytes of the glyph from the picture at $BF54
 * (eight a glyph) go into that byte, the other byte written back as it was.
 * HOW: A subtract and shifts into zTemp01-02 and zTemp08-09, the glyph's address in zTemp10-11; SetVramAddresses3CD;
 * a loop of eight reads and writes of the VDC's data port.
 * WAITS: no.
 * Entry: A the cell's index ($0A up), C set; zTemp00 the cell's value. Exit: Y 8, Z set, C set; A the last byte read
 * or written; X the VRAM address's low byte. Clobbers zTemp01-02, zTemp08-11.
 */
StageRegs TallyCell3CD(StageRegs r)
{
    return TallyCell3CDAt(r, 0x922A, &tally_3cd);
}

/* TallyCell3CD's body, for the builds of it elsewhere: where the build runs and what it uses. */
StageRegs TallyCell3CDAt(StageRegs r, uint16_t at, const TallyBuild *build)
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
    sum = add_bytes(0x54, value, false);
    WR8(ZP->temp10, sum.value);
    value = RD8(ZP->temp09);
    sum = add_bytes(0xBF, value, sum.carry);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp11, sum.value);
    CALL(at + 0x59, build->set_addresses(r));
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

void abi_TallyCell3CD(void)
{
    regs_exit_shim(TallyCell3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:2AF TallyPause3CD
 * @run 6B:92AF
 * @unreachable 6B:92AF-6B:92B9 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $92AF runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: A pause of the tally: after the wait, the next wait at $3310 set to $40 and the next step.
 * HOW: TallyWait152D; a store; a jump to TallyNext152D.
 * WAITS: no.
 * Entry: nothing. Exit: as TallyWait152D when it ends the step; else as TallyNext152D, A $40.
 */
StageRegs TallyPause3CD(StageRegs r)
{
    return TallyPause3CDAt(r, 0x92AF, &tally_3cd);
}

/* TallyPause3CD's body, for the builds of it elsewhere: where the build runs and what it uses. */
StageRegs TallyPause3CDAt(StageRegs r, uint16_t at, const TallyBuild *build)
{
    TallyWaitEnd wait = tally_wait_at(r, at + 0x3, build->wait);

    r = wait.r;
    if (wait.dropped) {
        return r;
    }
    r = loaded(r, 0x40);
    wr8(build->wait, 0x40);
    return TallyNext152D(r);
}

void abi_TallyPause3CD(void)
{
    regs_exit_shim(TallyPause3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:2BA TallyCount3CD
 * @run 6B:92BA
 * @unreachable 6B:92BA-6B:930F no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $92BA runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The tally's count: the paused frame's menu step with zPaused's bit 0 set for it; while zPaused's bit 1
 * is clear the health is counted into the score (zPlayerHealth down by one, its cells redrawn by TallyHealth3CD, 1
 * point by the kernel's piece at $408A), then after the wait the hearts (zField94 down by one in BCD, TallyHearts3CD,
 * $10 points); the score redrawn and sound 1 every fourth event. With both at 0 the tally goes on two steps, or, when
 * the full-health mark at $2A9B is set, stays with zPaused's bit 1 set and the wait at $3310 $40.
 * HOW: Bit changes; StagePausedFrame3CD; decrements; $408A; StageDrawScore3CD; the sound hook.
 * WAITS: no.
 * Entry: nothing. Exit: as the last piece that ran left them.
 */
StageRegs TallyCount3CD(StageRegs r)
{
    TallyWaitEnd wait;
    uint8_t old;

    RMW8(ZP->paused, old, old | 0x01);
    CALL(0x92BF, StagePausedFrame3CD(r));
    RMW8(ZP->paused, old, old & (uint8_t)~0x01);
    if ((RD8(ZP->paused) & 0x02) != 0) {
        return r;
    }
    r = loaded(r, RD8(ZP->player_health));
    if (r.a != 0) {
        r.nz = DEC8(ZP->player_health);
        CALL(0x92CD, TallyHealth3CD(r));
        r = loaded(r, 0x01);
    } else {
        wait = tally_wait(r, 0x92D4);
        r = wait.r;
        if (wait.dropped) {
            return r;
        }
        r = loaded(r, RD8(ZP->field94));
        if (r.a == 0) {
            r.nz = DEC8(OBJ->work_b[16]);
            r.nz = INC8(ZP->field80);
            r.nz = INC8(ZP->field80);
            r = loaded(r, RD8(OBJ->frame[51]));
            if (r.a != 0) {
                r.nz = DEC8(ZP->field80);
                r.nz = DEC8(ZP->field80);
                RMW8(ZP->paused, old, old | 0x02);
                r = loaded(r, 0x40);
                WR8(OBJ->work_b[16], 0x40);
            }
            return TallyNext152D(r);
        }
        r = bcd_less_one(r);
        WR8(ZP->field94, r.a);
        CALL(0x92E2, TallyHearts3CD(r));
        r = loaded(r, 0x10);
    }
    r = vector_call(r, 0x92E7, abi_VecAddScoreByte);
    CALL(0x92EA, StageDrawScore3CD(r));
    r = loaded(r, (uint8_t)(RD8(ZP->mode_events) & 0x03));
    if (r.a != 0) {
        return r;
    }
    r = loaded(r, 0x01);
    return vector_call(r, 0x92F5, abi_VecSoundCommand);
}

void abi_TallyCount3CD(void)
{
    regs_exit_shim(TallyCount3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:310 TallyWaitPress3CD
 * @run 6B:9310
 * @unreachable 6B:9310-6B:9321 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9310 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: A wait of the tally: after the wait, the paused frame's menu step; once zPaused's bit 1 is clear the
 * wait at $3310 is $20 and the next step.
 * HOW: TallyWait152D; StagePausedFrame3CD; a bit test; a store and a jump to TallyNext152D.
 * WAITS: no.
 * Entry: nothing. Exit: as the last piece that ran left them.
 */
StageRegs TallyWaitPress3CD(StageRegs r)
{
    TallyWaitEnd wait = tally_wait(r, 0x9313);

    r = wait.r;
    if (wait.dropped) {
        return r;
    }
    CALL(0x9316, StagePausedFrame3CD(r));
    if ((RD8(ZP->paused) & 0x02) != 0) {
        return r;
    }
    r = loaded(r, 0x20);
    WR8(OBJ->work_b[16], 0x20);
    return TallyNext152D(r);
}

void abi_TallyWaitPress3CD(void)
{
    regs_exit_shim(TallyWaitPress3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:322 TallyStockBonus3CD
 * @run 6B:9322
 * @unreachable 6B:9322-6B:933F no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9322 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The stock bonus: after the wait, zStock up by one in BCD (kept at 99), redrawn (StageDrawStock3CD),
 * sound $3E, the wait at $3310 one less and the next step.
 * HOW: TallyWait152D; SED, ADC, CLD; StageDrawStock3CD; the sound hook; a decrement; a jump to TallyNext152D.
 * WAITS: no.
 * Entry: D clear, as the game keeps it. Exit: as TallyNext152D.
 */
StageRegs TallyStockBonus3CD(StageRegs r)
{
    TallyWaitEnd wait = tally_wait(r, 0x9325);

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
    CALL(0x9335, StageDrawStock3CD(r));
    r = loaded(r, 0x3E);
    r = vector_call(r, 0x933A, abi_VecSoundCommand);
    r.nz = DEC8(OBJ->work_b[16]);
    return TallyNext152D(r);
}

void abi_TallyStockBonus3CD(void)
{
    regs_exit_shim(TallyStockBonus3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:340 TallyEnd3CD
 * @run 6B:9340
 * @unreachable 6B:9340-6B:934A no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9340 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The tally's end: after the wait, zField94 back from the hearts kept at $330D and zModeDone counted.
 * HOW: TallyWait152D; a store; an increment.
 * WAITS: no.
 * Entry: nothing. Exit: A the hearts, N and Z from zModeDone's increment.
 */
StageRegs TallyEnd3CD(StageRegs r)
{
    return TallyEnd3CDAt(r, 0x9340, &tally_3cd);
}

/* TallyEnd3CD's body, for the builds of it elsewhere: where the build runs and what it uses. */
StageRegs TallyEnd3CDAt(StageRegs r, uint16_t at, const TallyBuild *build)
{
    TallyWaitEnd wait = tally_wait_at(r, at + 0x3, build->wait);

    r = wait.r;
    if (wait.dropped) {
        return r;
    }
    r = loaded(r, rd8(build->health_kept));
    WR8(ZP->field94, r.a);
    r.nz = INC8(ZP->mode_done);
    return r;
}

void abi_TallyEnd3CD(void)
{
    regs_exit_shim(TallyEnd3CD(regs_entry_shim()));
}

/* The last cells of a count drawn: below LIMIT the word block at $5440 or $5540 cleared (StageClearWords3CD, one row),
 * else the next cell stepped (TallyDigitCell3CD). */
static Regs tally_count_cells(Regs r, uint8_t value, uint8_t limit, uint8_t cell, uint8_t row_hi, uint16_t digit_cell,
                              const TallyBuild *build)
{
    r = loaded(r, value);
    r = compared(r, limit);
    if (r.c) {
        r.x = cell;
        r.nz = cell;
        return TallyDigitCell3CDAt(r, digit_cell, build);
    }
    r.x = 0x40;
    r.y = row_hi;
    r = loaded(r, 0x01);
    return build->clear_words(r);
}

/*
 * @routine 03CF:356 TallyHealth3CD
 * @run 6B:9356
 * @unreachable 6B:9356-6B:9372 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9356 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Redraws the board's health: the stage's banks mapped, the digits put in (TallyDigits3CD), the units'
 * cell $12 stepped (TallyDigitCell3CD); from 10 up the tens' cell $11 too, below it the tens' place cleared.
 * HOW: MapStageBanks (a census duplicate at $93E7); TallyDigits3CD; TallyDigitCell3CD by BSR; a compare; a branch to
 * TallyDigitCell3CD or a jump to StageClearWords3CD.
 * WAITS: no.
 * Entry: nothing. Exit: as TallyDigitCell3CD or StageClearWords3CD.
 */
StageRegs TallyHealth3CD(StageRegs r)
{
    return TallyHealth3CDAt(r, 0x9356, &tally_3cd);
}

/* TallyHealth3CD's body, for the builds of it elsewhere: where the build runs and what it uses. */
StageRegs TallyHealth3CDAt(StageRegs r, uint16_t at, const TallyBuild *build)
{
    r = map_stage_banks_call(r, at + 0x3);
    CALL(at + 0x6, TallyDigits3CDAt(r, build->digits_run, build));
    r.x = 0x12;
    r.nz = 0x12;
    CALL(at + 0xA, TallyDigitCell3CDAt(r, build->digit_cell_run, build));
    return tally_count_cells(r, RD8(ZP->player_health), 0x0A, 0x11, 0x54, build->digit_cell_run, build);
}

void abi_TallyHealth3CD(void)
{
    regs_exit_shim(TallyHealth3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:373 TallyHearts3CD
 * @run 6B:9373
 * @unreachable 6B:9373-6B:938F no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9373 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Redraws the board's hearts as TallyHealth3CD does the health: the units' cell $22, and from $10 up
 * (BCD) the tens' cell $21, below it the tens' place cleared.
 * HOW: MapStageBanks (a census duplicate at $93E7); TallyDigits3CD; TallyDigitCell3CD by BSR; a compare; a branch to
 * TallyDigitCell3CD or a jump to StageClearWords3CD.
 * WAITS: no.
 * Entry: nothing. Exit: as TallyDigitCell3CD or StageClearWords3CD.
 */
StageRegs TallyHearts3CD(StageRegs r)
{
    return TallyHearts3CDAt(r, 0x9373, &tally_3cd);
}

/* TallyHearts3CD's body, for the builds of it elsewhere: where the build runs and what it uses. */
StageRegs TallyHearts3CDAt(StageRegs r, uint16_t at, const TallyBuild *build)
{
    r = map_stage_banks_call(r, at + 0x3);
    CALL(at + 0x6, TallyDigits3CDAt(r, build->digits_run, build));
    r.x = 0x22;
    r.nz = 0x22;
    CALL(at + 0xA, TallyDigitCell3CDAt(r, build->digit_cell_run, build));
    return tally_count_cells(r, RD8(ZP->field94), 0x10, 0x21, 0x55, build->digit_cell_run, build);
}

void abi_TallyHearts3CD(void)
{
    regs_exit_shim(TallyHearts3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:390 TallyDigitCell3CD
 * @run 6B:9390
 * @unreachable 6B:9390-6B:93A8 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9390 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Draws the board's cell X at every shade from its value up: TallyCell3CD for the cell, then the cell
 * stepped by $40 until it carries.
 * HOW: X kept on the stack across TallyCell3CD; an add; a loop.
 * WAITS: no.
 * Entry: X the cell. Exit: A the cell plus $40, C set, N and Z from it; X the cell; Y 8.
 */
StageRegs TallyDigitCell3CD(StageRegs r)
{
    return TallyDigitCell3CDAt(r, 0x9390, &tally_3cd);
}

/* TallyDigitCell3CD's body, for the builds of it elsewhere: where the build runs and what it uses. */
StageRegs TallyDigitCell3CDAt(StageRegs r, uint16_t at, const TallyBuild *build)
{
    Sum sum;
    uint8_t value;

    for (;;) {
        push8(r.x);
        r = loaded(r, rd8((uint16_t)(build->cells + r.x)));
        WR8(ZP->temp00, r.a);
        r = loaded(r, r.x);
        CALL(at + 0xA, TallyCell3CDAt(r, build->cell_run, build));
        r.x = pull8();
        r.nz = r.x;
        value = rd8((uint16_t)(build->cells + r.x));
        sum = add_bytes(value, 0x40, false);
        r = loaded(r, sum.value);
        r.c = sum.carry;
        r.v = sum.overflow;
        if (r.c) {
            return r;
        }
        wr8((uint16_t)(build->cells + r.x), sum.value);
    }
}

void abi_TallyDigitCell3CD(void)
{
    regs_exit_shim(TallyDigitCell3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:3F2 StageClearWords3CD
 * @run 6B:93F2
 * @unreachable 6B:93F2-6B:9404 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $93F2 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Clears A rows of 64 VRAM words from the address in X and Y.
 * HOW: The count in zTemp00; SetVramWriteAddress3CD; ST1 and ST2 in loops.
 * WAITS: no.
 * Entry: A the rows, X and Y the address. Exit: X 0, A as SetVramWriteAddress3CD left it, N and Z from the count's
 * decrement (Z set). Y, C and V as SetVramWriteAddress3CD left them. Clobbers zTemp00.
 */
StageRegs StageClearWords3CD(StageRegs r)
{
    WR8(ZP->temp00, r.a);
    CALL(0x93F7, SetVramWriteAddress3CD(r));
    do {
        for (r.x = 0x40; r.x != 0; r.x--) {
            st1(0x00);
            st2(0x00);
        }
        r.nz = DEC8(ZP->temp00);
    } while (r.nz != 0);
    return r;
}

void abi_StageClearWords3CD(void)
{
    regs_exit_shim(StageClearWords3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:410 SetVramAddresses3CD
 * @run 6B:9410
 * @unreachable 6B:9410-6B:9418 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9410 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Points both of the VDC's VRAM addresses at the word in X and Y, then runs on into
 * SetVramWriteAddress3CD. The bytes are those of SetVramAddresses (map_draw.c) at this overlay's address, where
 * $9419 is an entry of its own.
 * HOW: The kernel's piece at $4024 (the read address), X and Y into the data port; then SetVramWriteAddress3CD.
 * WAITS: no.
 * Entry: X low byte, Y high byte of the VRAM address. Exit: as SetVramWriteAddress3CD.
 */
StageRegs SetVramAddresses3CD(StageRegs r)
{
    r = vector_call(r, 0x9413, abi_VecSelectVramReadAddress);
    WR8(VDC->data_lo, r.x);
    WR8(VDC->data_hi, r.y);
    return SetVramWriteAddress3CD(r);
}

void abi_SetVramAddresses3CD(void)
{
    regs_exit_shim(SetVramAddresses3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:419 SetVramWriteAddress3CD
 * @run 6B:9419
 * @unreachable 6B:9419-6B:9424 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9419 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Points the VDC's VRAM write address at the word in X and Y and selects the data port.
 * HOW: The kernel's piece at $4021, X and Y into the data port; a jump to the kernel's piece at $4027.
 * WAITS: no.
 * Entry: X low byte, Y high byte of the VRAM address. Exit: as $4027 leaves them.
 */
StageRegs SetVramWriteAddress3CD(StageRegs r)
{
    r = vector_call(r, 0x941C, abi_VecSelectVramWriteAddress);
    WR8(VDC->data_lo, r.x);
    WR8(VDC->data_hi, r.y);
    return vector_tail(r, abi_VecSelectVramData);
}

void abi_SetVramWriteAddress3CD(void)
{
    regs_exit_shim(SetVramWriteAddress3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:443 StageDrawPiece3CD
 * @run 6B:9443
 * @unreachable 6B:9443-6B:9462 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9443 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Places one of the display's pieces, X from 0 to 7: the kernel's piece at $405D gives a slot for the
 * object the table at $9463 names, and the slot takes its sprite, set, X and Y from the tables at $946B, $9473,
 * $947B and $9483.
 * HOW: Table reads by X; $405D; stores by the slot in Y.
 * WAITS: no.
 * Entry: X the piece. Exit: A the Y written, N and Z from it; X and Y as $405D left them; C and V as it left them.
 */
StageRegs StageDrawPiece3CD(StageRegs r)
{
    r = loaded(r, rd8((uint16_t)(0x9463 + r.x)));
    r.y = r.a;
    r = vector_call(r, 0x944A, abi_VecClearObjectInY);
    r = loaded(r, rd8((uint16_t)(0x946B + r.x)));
    WR8(OBJ->sprite_set[72 + r.y], r.a);
    r = loaded(r, rd8((uint16_t)(0x9473 + r.x)));
    WR8(OBJ->sprite_set[r.y], r.a);
    r = loaded(r, rd8((uint16_t)(0x947B + r.x)));
    WR8(OBJ->x_hi[8 + r.y], r.a);
    r = loaded(r, rd8((uint16_t)(0x9483 + r.x)));
    WR8(OBJ->x_frac[48 + r.y], r.a);
    return r;
}

void abi_StageDrawPiece3CD(void)
{
    regs_exit_shim(StageDrawPiece3CD(regs_entry_shim()));
}


/*
 * @routine 03CF:4EA StageObjectHits3CD
 * @run 6B:94EA
 * @unreachable 6B:94EA-6B:9504 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $94EA runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The objects' hits each frame: the hit boxes of slots 0 to 15 placed (StagePlaceHitBoxes3CD), then for
 * objects $47 down to $10 the object's test against them (StageHitTestObject3CD) and the step of its hit timer at $3790
 * (while it runs the object's state at $31F0 is cleared).
 * HOW: X kept on the stack; calls in a loop down X.
 * WAITS: no.
 * Entry: nothing. Exit: X kept, N and Z from it; C clear; A, Y and V as the last object's step left them.
 */
StageRegs StageObjectHits3CD(StageRegs r)
{
    push8(r.x);
    CALL(0x94EE, StagePlaceHitBoxes3CD(r));
    r.x = 0x47;
    do {
        CALL(0x94F3, StageHitTestObject3CD(r));
        r = loaded(r, RD8(OBJ->work_l[24 + r.x]));
        if (r.a != 0) {
            WR8(OBJ->hit_status[80 + r.x], 0);
            r.nz = DEC8(OBJ->work_l[24 + r.x]);
        }
        r.x--;
        r.c = r.x >= 0x10;
        r.nz = (uint8_t)(r.x - 0x10);
    } while (r.c);
    r.x = pull8();
    r.nz = r.x;
    return r;
}

void abi_StageObjectHits3CD(void)
{
    regs_exit_shim(StageObjectHits3CD(regs_entry_shim()));
}

/* An object's X offset at $3670, negated when the object faces the other way ($2AA0 not 0), into zTemp01; zTemp00
 * its sign. Returns the offset. */
static uint8_t facing_offset(uint8_t x)
{
    uint8_t offset;

    offset = RD8(OBJ->work_i[x]);
    WR8(ZP->temp01, offset);
    if (RD8(OBJ->frame[56 + x]) != 0) {
        offset = (uint8_t)((RD8(ZP->temp01) ^ 0xFF) + 1);
        WR8(ZP->temp01, offset);
    }
    WR8(ZP->temp00, 0);
    offset = RD8(ZP->temp01);
    if ((offset & 0x80) != 0) {
        (void)DEC8(ZP->temp00);
    }
    return offset;
}

/* A signed offset read for a 16-bit add: zTemp00 cleared, the offset read, and zTemp00 $FF when it is negative. */
static uint8_t signed_offset(uint16_t addr)
{
    uint8_t offset;

    WR8(ZP->temp00, 0);
    offset = rd8(addr);
    if ((offset & 0x80) != 0) {
        (void)DEC8(ZP->temp00);
    }
    return offset;
}

/*
 * @routine 03CF:505 StagePlaceHitBoxes3CD
 * @run 6B:9505
 * @unreachable 6B:9505-6B:955E no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9505 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Places the hit boxes of slots 0 to 15: with no event a box is off the page ($2733 1); else its X (the
 * slot's X moved by its offset at $3670, negated when it faces the other way) and Y (moved by the offset at $3700) at
 * $2713 and $2723, and the two high bytes ORed into $2733.
 * HOW: A loop up X; 16-bit adds with the sign in zTemp00, the offset in zTemp01.
 * WAITS: no.
 * Entry: nothing. Exit: X $10, Z set, C set; A the last page byte.
 */
StageRegs StagePlaceHitBoxes3CD(StageRegs r)
{
    Sum sum;
    uint8_t offset, value;

    r.x = 0;
    do {
        r = loaded(r, RD8(OBJ->event_id[56 + r.x]));
        if (r.a == 0) {
            r = loaded(r, 0x01);
            wr8((uint16_t)(HIT_HI + r.x), 0x01);
        } else {
            offset = facing_offset(r.x);
            value = RD8(OBJ->x_hi[8 + r.x]);
            sum = add_bytes(offset, value, false);
            wr8((uint16_t)(HIT_X + r.x), sum.value);
            value = RD8(OBJ->sprite_attr[24 + r.x]);
            sum = add_bytes(value, RD8(ZP->temp00), sum.carry);
            wr8((uint16_t)(HIT_HI + r.x), sum.value);
            (void)signed_offset(ADDR_OF8(OBJ->work_j[56 + r.x]));
            value = RD8(OBJ->x_frac[48 + r.x]);
            offset = RD8(OBJ->work_j[56 + r.x]);
            sum = add_bytes(value, offset, false);
            wr8((uint16_t)(HIT_Y + r.x), sum.value);
            value = RD8(OBJ->x_lo[64 + r.x]);
            sum = add_bytes(value, RD8(ZP->temp00), sum.carry);
            r.v = sum.overflow;
            value = (uint8_t)(sum.value | rd8((uint16_t)(HIT_HI + r.x)));
            r = loaded(r, value);
            wr8((uint16_t)(HIT_HI + r.x), value);
        }
        r.x++;
        r.c = r.x >= 0x10;
        r.nz = (uint8_t)(r.x - 0x10);
    } while (!r.c);
    return r;
}

void abi_StagePlaceHitBoxes3CD(void)
{
    regs_exit_shim(StagePlaceHitBoxes3CD(regs_entry_shim()));
}

/* The distance test of object X and slot Y's box: true when within both reaches. */
static bool box_hit(Regs *r)
{
    Sum sum;
    uint8_t value, other;

    value = RD8(OBJ->work_i[72 + r->x]);
    other = RD8(OBJ->work_i[72 + r->y]);
    sum = add_bytes(value, other, false);
    wr8(REACH_X, sum.value);
    value = RD8(OBJ->work_k[40 + r->x]);
    other = RD8(OBJ->work_k[40 + r->y]);
    sum = add_bytes(value, other, false);
    wr8(REACH_Y, sum.value);
    value = rd8(TEST_X);
    other = rd8((uint16_t)(HIT_X + r->y));
    sum = sub_bytes(value, other, true);
    value = sum.carry ? sum.value : (uint8_t)((sum.value ^ 0xFF) + 1);
    *r = loaded(*r, value);
    *r = compared(*r, rd8(REACH_X));
    if (r->c) {
        return false;
    }
    value = rd8(TEST_Y);
    other = rd8((uint16_t)(HIT_Y + r->y));
    sum = sub_bytes(value, other, true);
    value = sum.carry ? sum.value : (uint8_t)((sum.value ^ 0xFF) + 1);
    *r = loaded(*r, value);
    *r = compared(*r, rd8(REACH_Y));
    return !r->c;
}

/*
 * @routine 03CF:55F StageHitTestObject3CD
 * @run 6B:955F
 * @stop 95FB
 * @unreachable 6B:955F-6B:95FA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $955F runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Tests object X against the hit boxes: its state at $31F0 cleared; with an event and a reach, its point
 * (its X moved by its facing offset, its Y by the offset at $3700) on the screen's page is tested against the boxes of
 * slots 15 down to 0 (only slot 0 for an event with bit 7); a box within both reaches is a hit (StageHitHandler3CD).
 * HOW: Tests; 16-bit adds with the sign in zTemp00; the point in $2702-$2703, the reaches in $2704-$2705; a loop
 * down Y; jumps to the RTS at $95FB and a branch to StageHitHandler3CD.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or the handler left them.
 */
StageRegs StageHitTestObject3CD(StageRegs r)
{
    Sum sum;
    uint8_t offset, value;

    WR8(OBJ->hit_status[80 + r.x], 0);
    r = loaded(r, RD8(OBJ->event_id[56 + r.x]));
    if (r.a == 0) {
        return StageSubStepNone152D(r);
    }
    value = RD8(OBJ->work_i[72 + r.x]);
    r = loaded(r, (uint8_t)(value | RD8(OBJ->work_k[40 + r.x])));
    if (r.a == 0) {
        return StageSubStepNone152D(r);
    }
    offset = facing_offset(r.x);
    value = RD8(OBJ->x_hi[8 + r.x]);
    sum = add_bytes(offset, value, false);
    wr8(TEST_X, sum.value);
    value = RD8(OBJ->sprite_attr[24 + r.x]);
    sum = add_bytes(value, RD8(ZP->temp00), sum.carry);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    if (r.a != 0) {
        return StageSubStepNone152D(r);
    }
    offset = signed_offset(ADDR_OF8(OBJ->work_j[56 + r.x]));
    value = RD8(OBJ->x_frac[48 + r.x]);
    sum = add_bytes(offset, value, false);
    wr8(TEST_Y, sum.value);
    value = RD8(OBJ->x_lo[64 + r.x]);
    sum = add_bytes(value, RD8(ZP->temp00), sum.carry);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    if (r.a != 0) {
        return StageSubStepNone152D(r);
    }
    r = loaded(r, RD8(OBJ->event_id[56 + r.x]));
    r.y = (r.a & 0x80) != 0 ? 0x00 : 0x0F;
    for (; (r.y & 0x80) == 0; r.y--) {
        r = loaded(r, rd8((uint16_t)(HIT_HI + r.y)));
        if (r.a != 0) {
            continue;
        }
        if (box_hit(&r)) {
            return StageHitHandler3CD(r);
        }
    }
    r.nz = r.y;
    return StageSubStepNone152D(r);
}

void abi_StageHitTestObject3CD(void)
{
    regs_exit_shim(StageHitTestObject3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:5FC StageHitHandler3CD
 * @run 6B:95FC
 * @unreachable 6B:95FC-6B:9619 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $95FC runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Object X hit slot Y's box, as StageHitHandler152D: the hitter kept at $35E0, zHitSlot the slot, the
 * slot's kind's handler from the table of 15 at $961A.
 * HOW: Stores; a table read into zTemp00-01 and a jump through it.
 * WAITS: no.
 * Entry: X the object, Y the slot. Exit: kind 0: A 0, N and Z from it. Else: as the handler leaves them.
 */
StageRegs StageHitHandler3CD(StageRegs r)
{
    uint8_t kind, index;
    uint16_t handler;

    r = loaded(r, r.x);
    wr8((uint16_t)(ADDR_OF8(OBJ->angle_lo[32]) + r.y), r.a);
    WR8(ZP->hit_slot, r.y);
    r = loaded(r, rd8((uint16_t)(ADDR_OF8(OBJ->angle_hi[48]) + r.y)));
    if (r.a == 0) {
        return r;
    }
    kind = (uint8_t)(r.a - 1);
    index = (uint8_t)(kind << 1);
    r.c = (kind & 0x80) != 0;
    r.y = index;
    r = loaded(r, rd8((uint16_t)(0x961A + index)));
    WR8(ZP->temp00, r.a);
    r = loaded(r, rd8((uint16_t)(0x961B + index)));
    WR8(ZP->temp01, r.a);
    r.y = RD8(ZP->hit_slot);
    r.nz = r.y;
    index = RD8(ZP->temp00);
    handler = (uint16_t)(index | (RD8(ZP->temp01) << 8));
    switch (handler) {
    case 0x9638:
        return HitKindSpawn152D(r);
    case 0x9655:
        return HitKindFacing3CD(r);
    case 0x9662:
        return HitKindReport3CD(r);
    case 0x967A:
        return HitKindState152D(r);
    case 0x9681:
        return HitKindSide3CD(r);
    default:
        return resident_tail(r, handler);
    }
}

void abi_StageHitHandler3CD(void)
{
    regs_exit_shim(StageHitHandler3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:662 HitKindReport3CD
 * @run 6B:9662
 * @stop 967A
 * @unreachable 6B:9662-6B:9679 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9662 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Hit kinds 5, 10, 11, 13, 14 and 15: the hit's point ($2702-$2703) and hit box 3's X and Y ($2716,
 * $2726) kept in StageHitReport's four bytes, then the object takes the slot's kind (runs on into the bytes of
 * HitKindState152D at $967A).
 * HOW: Loads and stores.
 * WAITS: no.
 * Entry: X the object, Y the slot. Exit: as HitKindState152D.
 */
StageRegs HitKindReport3CD(StageRegs r)
{
    r = loaded(r, rd8(TEST_X));
    WR8(STAGEHIT->bytes[0], r.a);
    r = loaded(r, rd8(TEST_Y));
    WR8(STAGEHIT->bytes[1], r.a);
    r = loaded(r, rd8((uint16_t)(HIT_X + 3)));
    WR8(STAGEHIT->bytes[2], r.a);
    r = loaded(r, rd8((uint16_t)(HIT_Y + 3)));
    WR8(STAGEHIT->bytes[3], r.a);
    return HitKindState152D(r);
}

void abi_HitKindReport3CD(void)
{
    regs_exit_shim(HitKindReport3CD(regs_entry_shim()));
}

/* The side the slot keeps at $3628 (1 or 2); unless it is 2 the hit is reported (HitKindReport3CD). */
static Regs hit_side_set(Regs r, uint8_t side)
{
    r = loaded(r, side);
    wr8((uint16_t)(ADDR_OF8(OBJ->work_h[16]) + r.y), side);
    r = compared(r, 0x02);
    if (r.nz == 0) {
        return r;
    }
    return HitKindReport3CD(r);
}

/*
 * @routine 03CF:655 HitKindFacing3CD
 * @run 6B:9655
 * @unreachable 6B:9655-6B:9661 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9655 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Hit kind 6: the slot's side at $3628 from the object's facing (bit 0 of $2FF8, plus one); unless it is
 * 2, runs on into HitKindReport3CD.
 * HOW: A mask and an increment; a compare.
 * WAITS: no.
 * Entry: X the object, Y the slot. Exit: side 2: A 2, Z set, C set. Else as HitKindReport3CD.
 */
StageRegs HitKindFacing3CD(StageRegs r)
{
    uint8_t side = (uint8_t)((RD8(OBJ->sprite_flip[16 + r.x]) & 0x01) + 1);

    return hit_side_set(r, side);
}

void abi_HitKindFacing3CD(void)
{
    regs_exit_shim(HitKindFacing3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:681 HitKindSide3CD
 * @run 6B:9681
 * @unreachable 6B:9681-6B:968C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9681 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Hit kind 9: the slot's side at $3628 from bit 1 of the object's $2FF8 (plus one); unless it is 2, the
 * hit is reported. The compare and what follows are HitKindFacing3CD's bytes at $965E.
 * HOW: A mask, a shift and an increment; a branch into HitKindFacing3CD's bytes.
 * WAITS: no.
 * Entry: X the object, Y the slot. Exit: as HitKindFacing3CD.
 */
StageRegs HitKindSide3CD(StageRegs r)
{
    uint8_t side = (uint8_t)(((RD8(OBJ->sprite_flip[16 + r.x]) & 0x02) >> 1) + 1);

    return hit_side_set(r, side);
}

void abi_HitKindSide3CD(void)
{
    regs_exit_shim(HitKindSide3CD(regs_entry_shim()));
}

/* A probe point: the next four bytes of the list (zTemp0C-0D, Y on) added to object X's position, into zTemp00-03. */
static Regs probe_point(Regs r)
{
    Sum sum;
    uint8_t offset, value;

    offset = RD8_PTR_Y(ZP->temp0c, r.y);
    value = RD8(OBJ->x_hi[8 + r.x]);
    sum = add_bytes(offset, value, false);
    WR8(ZP->temp00, sum.value);
    r.y++;
    offset = RD8_PTR_Y(ZP->temp0c, r.y);
    value = RD8(OBJ->sprite_attr[24 + r.x]);
    sum = add_bytes(offset, value, sum.carry);
    WR8(ZP->temp01, sum.value);
    r.y++;
    offset = RD8_PTR_Y(ZP->temp0c, r.y);
    value = RD8(OBJ->x_frac[48 + r.x]);
    sum = add_bytes(offset, value, false);
    WR8(ZP->temp02, sum.value);
    r.y++;
    offset = RD8_PTR_Y(ZP->temp0c, r.y);
    value = RD8(OBJ->x_lo[64 + r.x]);
    sum = add_bytes(offset, value, sum.carry);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp03, sum.value);
    r.y++;
    return r;
}

/* The probe list of the index in A from the table at $D756, into zTemp0C-0D; zTemp0A, the point's count, 0. */
static Regs probe_list(Regs r, uint8_t index, uint16_t lists)
{
    r.c = (index & 0x80) != 0;
    r.y = (uint8_t)(index << 1);
    r = loaded(r, rd8((uint16_t)(lists + r.y)));
    WR8(ZP->temp0c, r.a);
    r = loaded(r, rd8((uint16_t)(lists + 1 + r.y)));
    WR8(ZP->temp0d, r.a);
    r.y = 0;
    WR8(ZP->temp0a, 0);
    return r;
}

/*
 * @routine 03CF:68D StageProbePoints3CD
 * @run 6B:968D
 * @unreachable 6B:968D-6B:96DA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $968D runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Probes the map at an object's four points (the list the index in A names, from the table at $D756:
 * four offsets of X and Y a point from the object's position): for each, ProbeTile3CD's answer at $26F6 and the
 * tile's column and row at $26FA and $26FE.
 * HOW: The list in zTemp0C-0D, the count in zTemp0A; the point into zTemp00-03; Y kept on the stack across
 * ProbeTile3CD.
 * WAITS: no.
 * Entry: A the list, X the object. Exit: A 4, Z set, C set; Y past the last point; X kept as ProbeTile3CD keeps it.
 */
StageRegs StageProbePoints3CD(StageRegs r)
{
    return StageProbePoints3CDAt(r, &route_3cd);
}

/* StageProbePoints3CD's body, for the builds of it with their own tables. */
StageRegs StageProbePoints3CDAt(StageRegs r, const StageRouteBuild *build)
{
    uint8_t count;

    r = probe_list(r, r.a, build->probe_lists);
    do {
        r = probe_point(r);
        push8(r.y);
        CALL(0x96C2, ProbeTile3CD(r));
        r.y = RD8(ZP->temp0a);
        wr8((uint16_t)(PROBE_HIT + r.y), r.a);
        r = loaded(r, RD8(ZP->temp00));
        wr8((uint16_t)(PROBE_X + r.y), r.a);
        r = loaded(r, RD8(ZP->temp02));
        wr8((uint16_t)(PROBE_Y + r.y), r.a);
        r.nz = INC8(ZP->temp0a);
        r.y = pull8();
        count = RD8(ZP->temp0a);
        r = loaded(r, count);
        r = compared(r, 0x04);
    } while (r.nz != 0);
    return r;
}

void abi_StageProbePoints3CD(void)
{
    regs_exit_shim(StageProbePoints3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:6DB StageProbeAgain3CD
 * @run 6B:96DB
 * @unreachable 6B:96DB-6B:974E no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $96DB runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Probes an object's four points (StageProbePoints3CD), then each point whose answer has bit 0 set and
 * bit 7 clear again 8 pixels higher; when the second probe's answer has bit 0 set the first answer keeps only bits 1
 * to 6.
 * HOW: A kept on the stack across StageProbePoints3CD; the list again; for each point a test of its answer, the point
 * into zTemp00-03 and $FFF8 added to its Y, ProbeTile3CD with Y kept on the stack.
 * WAITS: no.
 * Entry: A the list, X the object. Exit: A 4, Z set, C set; Y past the last point.
 */
StageRegs StageProbeAgain3CD(StageRegs r)
{
    return StageProbeAgain3CDAt(r, &route_3cd);
}

/* StageProbeAgain3CD's body, for the builds of it with their own tables. */
StageRegs StageProbeAgain3CDAt(StageRegs r, const StageRouteBuild *build)
{
    Sum sum;
    uint8_t list, value, answer;

    push8(r.a);
    CALL(0x96DF, StageProbePoints3CDAt(r, build));
    list = pull8();
    r = probe_list(r, list, build->probe_lists);
    do {
        push8(r.y);
        r.y = RD8(ZP->temp0a);
        answer = rd8((uint16_t)(PROBE_HIT + r.y));
        r.y = pull8();
        r = loaded(r, (uint8_t)(answer & 0x81));
        if ((r.a & 0x80) != 0 || r.a == 0) {
            r.y = (uint8_t)(r.y + 4);
        } else {
            r = probe_point(r);
            value = RD8(ZP->temp02);
            sum = add_bytes(0xF8, value, false);
            WR8(ZP->temp02, sum.value);
            value = RD8(ZP->temp02);
            sum = add_bytes(0xFF, value, sum.carry);
            r = loaded(r, sum.value);
            r.c = sum.carry;
            r.v = sum.overflow;
            WR8(ZP->temp02, sum.value);
            push8(r.y);
            CALL(0x972F, ProbeTile3CD(r));
            r.y = RD8(ZP->temp0a);
            r = loaded(r, (uint8_t)(r.a & 0x01));
            if (r.a != 0) {
                r.y = RD8(ZP->temp0a);
                r = loaded(r, (uint8_t)(rd8((uint16_t)(PROBE_HIT + r.y)) & 0x7E));
                wr8((uint16_t)(PROBE_HIT + r.y), r.a);
            }
            r.y = pull8();
        }
        r.nz = INC8(ZP->temp0a);
        r = loaded(r, RD8(ZP->temp0a));
        r = compared(r, 0x04);
    } while (r.nz != 0);
    return r;
}

void abi_StageProbeAgain3CD(void)
{
    regs_exit_shim(StageProbeAgain3CD(regs_entry_shim()));
}


/*
 * @routine 03CF:74F ProbeTile3CD
 * @run 6B:974F
 * @unreachable 6B:974F-6B:97E5 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $974F runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Says what the map holds at a point, as ProbeTile152D up to the attribute: a page of bit 7 gives 0, an
 * attribute below $80 or from $90 up is the answer; $80 to $8F go to their handlers (the table of 16 at $97E6), with A
 * the attribute, which set the row in the tile (zTemp02) and answer $81.
 * HOW: 9-bit adds of the point and the scroll into zTemp04-07; the kernel's pieces at $4024 and $4027; the entry in
 * zTemp08-09; the handler's address in zTemp04-05 and a jump through it.
 * WAITS: no.
 * Entry: zTemp00-01 X, zTemp02-03 Y. Exit: A the answer, N and Z from it; zTemp00 the column, zTemp02 the row.
 * Clobbers zTemp04-09.
 */
StageRegs ProbeTile3CD(StageRegs r)
{
    Sum sum;
    uint8_t value, other, old, page, attribute, index;
    uint16_t handler;
    bool carry;

    value = RD8(ZP->temp00);
    other = RD8(SCROLL->x_lo[0]);
    sum = add_bytes(value, other, false);
    WR8(ZP->temp04, sum.value);
    value = RD8(ZP->temp01);
    other = RD8(SCROLL->x_hi[0]);
    sum = add_bytes(value, other, sum.carry);
    carry = (sum.value & 0x01) != 0;
    value = RD8(ZP->temp04);
    value = (uint8_t)((carry ? 0x80 : 0) | (value >> 1));
    value = (uint8_t)((value >> 2) & 0x3F);
    WR8(ZP->temp05, value);
    value = (uint8_t)(RD8(ZP->temp04) & 0x07);
    WR8(ZP->temp00, value);
    value = RD8(ZP->temp02);
    other = RD8(SCROLL->y_lo[0]);
    sum = add_bytes(value, other, false);
    WR8(ZP->temp06, sum.value);
    value = RD8(ZP->temp03);
    other = RD8(SCROLL->y_hi[0]);
    sum = add_bytes(value, other, sum.carry);
    WR8(ZP->temp07, (uint8_t)(sum.value & 0x01));
    value = RD8(ZP->temp06);
    for (int i = 0; i < 3; i++) {
        bool out = (value & 0x80) != 0;

        value = (uint8_t)(value << 1);
        old = RD8(ZP->temp07);
        WR8(ZP->temp07, (uint8_t)((old << 1) | (out ? 1 : 0)));
    }
    value = (uint8_t)(value & 0xC0);
    other = RD8(ZP->temp05);
    sum = add_bytes(value, other, false);
    WR8(ZP->temp05, sum.value);
    value = (uint8_t)(RD8(ZP->temp06) & 0x07);
    r = loaded(r, value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp02, value);
    r = vector_call(r, 0x979A, abi_VecSelectVramReadAddress);
    WR8(VDC->data_lo, RD8(ZP->temp05));
    r = loaded(r, RD8(ZP->temp07));
    WR8(VDC->data_hi, r.a);
    r = vector_call(r, 0x97A7, abi_VecSelectVramData);
    WR8(ZP->temp08, RD8(VDC->data_lo));
    page = (uint8_t)(RD8(VDC->data_hi) & 0x07);
    WR8(ZP->temp09, page);
    r.y = page;
    r = loaded(r, rd8((uint16_t)(SaveBuffer + r.y)));
    if ((r.a & 0x80) != 0) {
        return loaded(r, 0x00);
    }
    value = RD8(ZP->temp08);
    sum = add_bytes(value, 0x00, false);
    WR8(ZP->temp08, sum.value);
    value = rd8((uint16_t)(SaveBuffer + r.y));
    sum = add_bytes(value, 0x7D, sum.carry);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp09, sum.value);
    attribute = RD8_PTR(ZP->temp08);
    r = loaded(r, attribute);
    if ((attribute & 0x80) == 0) {
        return r;
    }
    r = compared(r, 0x90);
    if (r.c) {
        return r;
    }
    push8(attribute);
    sum = sub_bytes(attribute, 0x7F, false);
    r.v = sum.overflow;
    r.c = (sum.value & 0x80) != 0;
    index = (uint8_t)(sum.value << 1);
    r.y = index;
    r = loaded(r, rd8((uint16_t)(0x97E6 + index)));
    WR8(ZP->temp04, r.a);
    r = loaded(r, rd8((uint16_t)(0x97E7 + index)));
    WR8(ZP->temp05, r.a);
    r = loaded(r, pull8());
    index = RD8(ZP->temp04);
    handler = (uint16_t)(index | (RD8(ZP->temp05) << 8));
    switch (handler) {
    case 0x9806:
        return TileSlopeA3CD(r);
    case 0x9813:
        return TileSlopeB3CD(r);
    case 0x9824:
        return TileSlopeC3CD(r);
    case 0x9835:
        return TileStepRow3CD(r);
    case 0x9844:
        return TileStepPair3CD(r);
    case 0x9858:
        return TileRowDown3CD(r);
    case 0x985D:
        return TileSlopeD3CD(r);
    case 0x986A:
        return TileSlopeE3CD(r);
    case 0x987B:
        return TileSlopeF3CD(r);
    default:
        return resident_tail(r, handler);
    }
}

void abi_ProbeTile3CD(void)
{
    regs_exit_shim(ProbeTile3CD(regs_entry_shim()));
}

/* A loaded tile handler's exit (terrain.c) in these registers: Y and C and V where it set them, then A $81. */
static Regs tile_handled(Regs r, HandlerExit e)
{
    if (e.y_set) {
        r.y = e.y;
    }
    if (e.flags_set) {
        r.c = e.carry;
        r.v = e.overflow;
    }
    return loaded(r, 0x81);
}

/*
 * @routine 03D0:006 TileSlopeA3CD
 * @run 6B:9806
 * @unreachable 6B:9806-6B:9812 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9806 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Tile attribute $80: the row moved by the table at $989C by the column; answers $81.
 * HOW: TileStepAt (terrain.c), the body of 050F:752, with the table at $989C.
 * WAITS: no.
 * Entry: zTemp00 the column, zTemp02 the row. Exit: A $81, N set; Y the column; C and V from the add.
 */
StageRegs TileSlopeA3CD(StageRegs r)
{
    return tile_handled(r, TileStepAt(0x989C));
}

void abi_TileSlopeA3CD(void)
{
    regs_exit_shim(TileSlopeA3CD(regs_entry_shim()));
}

/*
 * @routine 03D0:013 TileSlopeB3CD
 * @run 6B:9813
 * @unreachable 6B:9813-6B:9823 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9813 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Tile attribute $81: the row from the 8 by 8 table at $98A4; answers $81.
 * HOW: TileRowAt (terrain.c), the body of 050F:75F, with the table at $98A4.
 * WAITS: no.
 * Entry: zTemp00 the column, zTemp02 the row. Exit: A $81, N set; Y the index; C and V from the add.
 */
StageRegs TileSlopeB3CD(StageRegs r)
{
    return tile_handled(r, TileRowAt(0x98A4));
}

void abi_TileSlopeB3CD(void)
{
    regs_exit_shim(TileSlopeB3CD(regs_entry_shim()));
}

/*
 * @routine 03D0:024 TileSlopeC3CD
 * @run 6B:9824
 * @unreachable 6B:9824-6B:9834 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9824 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Tile attribute $82: the row from the 8 by 8 table at $98E4; answers $81.
 * HOW: TileRowAt (terrain.c), the body of 050F:75F, with the table at $98E4.
 * WAITS: no.
 * Entry: zTemp00 the column, zTemp02 the row. Exit: as TileSlopeB3CD.
 */
StageRegs TileSlopeC3CD(StageRegs r)
{
    return tile_handled(r, TileRowAt(0x98E4));
}

void abi_TileSlopeC3CD(void)
{
    regs_exit_shim(TileSlopeC3CD(regs_entry_shim()));
}

/*
 * @routine 03D0:035 TileStepRow3CD
 * @run 6B:9835
 * @unreachable 6B:9835-6B:9843 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9835 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Tile attributes $87 to $8A: the row moved by the table at $988C by the attribute less $83; answers $81.
 * HOW: A subtract, a table read and an add.
 * WAITS: no.
 * Entry: A the attribute, zTemp02 the row. Exit: A $81, N set; Y the index; C and V from the add.
 */
StageRegs TileStepRow3CD(StageRegs r)
{
    Sum sum;
    uint8_t row, step;

    r.y = sub_bytes(r.a, 0x83, true).value;
    step = rd8((uint16_t)(0x988C + r.y));
    row = RD8(ZP->temp02);
    sum = add_bytes(step, row, false);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(ZP->temp02, sum.value);
    return loaded(r, 0x81);
}

void abi_TileStepRow3CD(void)
{
    regs_exit_shim(TileStepRow3CD(regs_entry_shim()));
}

/*
 * @routine 03D0:044 TileStepPair3CD
 * @run 6B:9844
 * @unreachable 6B:9844-6B:9857 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9844 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Tile attributes $8C to $8F: the row moved by one of a pair in the table at $9894 (the attribute less
 * $8C doubled, the second when the column's bit 2 is clear); answers $81.
 * HOW: TileHalfStepAt (terrain.c), the body of 050F:781, with the table at $9894, from attribute $8C, the second byte when the
 * bit is clear.
 * WAITS: no.
 * Entry: A the attribute, zTemp00 the column, zTemp02 the row. Exit: as TileStepRow3CD.
 */
StageRegs TileStepPair3CD(StageRegs r)
{
    return tile_handled(r, TileHalfStepAt(0x9894, 0x8C, r.a, false));
}

void abi_TileStepPair3CD(void)
{
    regs_exit_shim(TileStepPair3CD(regs_entry_shim()));
}

/*
 * @routine 03D0:058 TileRowDown3CD
 * @run 6B:9858
 * @unreachable 6B:9858-6B:985C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9858 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Tile attributes $86 and $8B: the row one lower; answers $81.
 * HOW: TileNudgeA (terrain.c, 050F:7A9), the same code.
 * WAITS: no.
 * Entry: zTemp02 the row. Exit: A $81, N set. X, Y, C and V kept.
 */
StageRegs TileRowDown3CD(StageRegs r)
{
    return tile_handled(r, TileNudgeA());
}

void abi_TileRowDown3CD(void)
{
    regs_exit_shim(TileRowDown3CD(regs_entry_shim()));
}

/*
 * @routine 03D0:05D TileSlopeD3CD
 * @run 6B:985D
 * @unreachable 6B:985D-6B:9869 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $985D runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Tile attribute $83: the row moved by the table at $9924 by the column; answers $81.
 * HOW: TileStepAt (terrain.c), the body of 050F:752, with the table at $9924.
 * WAITS: no.
 * Entry: zTemp00 the column, zTemp02 the row. Exit: as TileSlopeA3CD.
 */
StageRegs TileSlopeD3CD(StageRegs r)
{
    return tile_handled(r, TileStepAt(0x9924));
}

void abi_TileSlopeD3CD(void)
{
    regs_exit_shim(TileSlopeD3CD(regs_entry_shim()));
}

/*
 * @routine 03D0:06A TileSlopeE3CD
 * @run 6B:986A
 * @unreachable 6B:986A-6B:987A no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $986A runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Tile attribute $84: the row from the 8 by 8 table at $992C; answers $81.
 * HOW: TileRowAt (terrain.c), the body of 050F:75F, with the table at $992C.
 * WAITS: no.
 * Entry: zTemp00 the column, zTemp02 the row. Exit: as TileSlopeB3CD.
 */
StageRegs TileSlopeE3CD(StageRegs r)
{
    return tile_handled(r, TileRowAt(0x992C));
}

void abi_TileSlopeE3CD(void)
{
    regs_exit_shim(TileSlopeE3CD(regs_entry_shim()));
}

/*
 * @routine 03D0:07B TileSlopeF3CD
 * @run 6B:987B
 * @unreachable 6B:987B-6B:988B no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $987B runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Tile attribute $85: the row from the 8 by 8 table at $996C; answers $81.
 * HOW: TileRowAt (terrain.c), the body of 050F:75F, with the table at $996C.
 * WAITS: no.
 * Entry: zTemp00 the column, zTemp02 the row. Exit: as TileSlopeB3CD.
 */
StageRegs TileSlopeF3CD(StageRegs r)
{
    return tile_handled(r, TileRowAt(0x996C));
}

void abi_TileSlopeF3CD(void)
{
    regs_exit_shim(TileSlopeF3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:48B StageMapDebug3CD
 * @run 6B:948B
 * @unreachable 6B:948B-6B:94B3 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $948B runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The map's debug marks, from the pause screen: for each of SaveBuffer's pages 1 to 3 that is in use (its
 * byte at $7D01 to $7D03 not negative) the page's cells are marked on the screen (StageMapDebugPage3CD; page 1 from its
 * cell $10, at the rows $10, $20 and $30).
 * HOW: Three tests and calls.
 * WAITS: no.
 * Entry: nothing. Exit: as the last test or call left them.
 */
StageRegs StageMapDebug3CD(StageRegs r)
{
    r = loaded(r, rd8((uint16_t)(SaveBuffer + 1)));
    if ((r.a & 0x80) == 0) {
        r.y = 0x10;
        r = loaded(r, 0x7D);
        r.x = 0x10;
        r.nz = 0x10;
        CALL(0x9499, StageMapDebugPage3CD(r));
    }
    r = loaded(r, rd8((uint16_t)(SaveBuffer + 2)));
    if ((r.a & 0x80) == 0) {
        r.y = 0;
        r = loaded(r, 0x7E);
        r.x = 0x20;
        r.nz = 0x20;
        CALL(0x94A6, StageMapDebugPage3CD(r));
    }
    r = loaded(r, rd8((uint16_t)(SaveBuffer + 3)));
    if ((r.a & 0x80) == 0) {
        r.y = 0;
        r = loaded(r, 0x7F);
        r.x = 0x30;
        r.nz = 0x30;
        CALL(0x94B3, StageMapDebugPage3CD(r));
    }
    return r;
}

void abi_StageMapDebug3CD(void)
{
    regs_exit_shim(StageMapDebug3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:4B4 StageMapDebugPage3CD
 * @run 6B:94B4
 * @unreachable 6B:94B4-6B:94E9 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $94B4 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: Marks a page's cells on the screen: for each cell from Y to $FF of the page at A00 that is not 0, the
 * 16 VRAM words at the cell's place (Y times 8, the high byte ORed with X) filled with $55AA.
 * HOW: The page in zTemp00-01, the row in zTemp02; the kernel's pieces at $4021 and $4027; shifts into zTemp03; ST1
 * and ST2 in a loop.
 * WAITS: no.
 * Entry: A the page's high byte, X the row's high byte, Y the first cell. Exit: Y 0, Z set; A, X, C and V as the last
 * cell left them. Clobbers zTemp00-03.
 */
StageRegs StageMapDebugPage3CD(StageRegs r)
{
    uint8_t value, old, high;

    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, r.a);
    WR8(ZP->temp02, r.x);
    do {
        r = loaded(r, RD8_PTR_Y(ZP->temp00, r.y));
        if (r.a != 0) {
            r = vector_call(r, 0x94C1, abi_VecSelectVramWriteAddress);
            WR8(ZP->temp03, 0);
            value = r.y;
            for (int i = 0; i < 3; i++) {
                bool out = (value & 0x80) != 0;

                value = (uint8_t)(value << 1);
                old = RD8(ZP->temp03);
                WR8(ZP->temp03, (uint8_t)((old << 1) | (out ? 1 : 0)));
                r.c = (old & 0x80) != 0;
            }
            WR8(VDC->data_lo, value);
            high = RD8(ZP->temp03);
            r = loaded(r, (uint8_t)(high | RD8(ZP->temp02)));
            WR8(VDC->data_hi, r.a);
            r = vector_call(r, 0x94DD, abi_VecSelectVramData);
            for (r.x = 0x10; r.x != 0; r.x--) {
                st1(0xAA);
                st2(0x55);
            }
        }
        r.y++;
        r.nz = r.y;
    } while (r.y != 0);
    return r;
}

void abi_StageMapDebugPage3CD(void)
{
    regs_exit_shim(StageMapDebugPage3CD(regs_entry_shim()));
}

/*
 * @routine 03D0:1AC GateObjectStep3CD
 * @run 6B:99AC
 * @stop 99BE
 * @unreachable 6B:99AC-6B:99BD no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $99AC runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: An object handler for the stage's gate, as GateObjectStep152D: its step at $2818 from the table of
 * seven at $99BE. No code of the overlay reaches it; the census counts none of its bytes.
 * HOW: The step doubled indexes the table; its word goes through zTemp00-01 and is jumped through.
 * WAITS: no.
 * Entry: X the object. Exit: as the step leaves them.
 */
StageRegs GateObjectStep3CD(StageRegs r)
{
    uint8_t step, index;
    uint16_t handler;

    step = RD8(OBJ->substate[24 + r.x]);
    index = (uint8_t)(step << 1);
    r.c = (step & 0x80) != 0;
    r.y = index;
    r = loaded(r, rd8((uint16_t)(0x99BE + index)));
    WR8(ZP->temp00, r.a);
    r = loaded(r, rd8((uint16_t)(0x99BF + index)));
    WR8(ZP->temp01, r.a);
    index = RD8(ZP->temp00);
    handler = (uint16_t)(index | (RD8(ZP->temp01) << 8));
    switch (handler) {
    case 0x99CC:
        return GateStart3CD(r);
    case 0x99E0:
        return GateWaitPlayer3CD(r);
    case 0x9A41:
        return GateWaitStep3CD(r);
    case 0x9A4B:
        return GateOpen3CD(r);
    case 0x9A89:
        return GateWaitClear3CD(r);
    case 0x9AA6:
        return GateClose3CD(r);
    case 0x99DB:
        return StageSubStepNone152D(r);
    default:
        return resident_tail(r, handler);
    }
}

void abi_GateObjectStep3CD(void)
{
    regs_exit_shim(GateObjectStep3CD(regs_entry_shim()));
}

/*
 * @routine 03D0:1CC GateStart3CD
 * @run 6B:99CC
 * @stop 99DB
 * @unreachable 6B:99CC-6B:99DA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $99CC runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The gate's first step: the kernel's piece at $40D2 and its sprite at $2A58 from the two at $99DC by
 * bit 0 of its packed byte; then the RTS at $99DB (StageSubStepNone152D's kind, run on into).
 * HOW: $40D2; a table read.
 * WAITS: no.
 * Entry: X the object. Exit: A the sprite, N and Z from it, Y the bit.
 */
StageRegs GateStart3CD(StageRegs r)
{
    r = vector_call(r, 0x99CF, abi_VecLoadObjectRecord);
    r.y = (uint8_t)(RD8(OBJ->work_a[64 + r.x]) & 0x01);
    r = loaded(r, rd8((uint16_t)(0x99DC + r.y)));
    WR8(OBJ->sprite_set[72 + r.x], r.a);
    return StageSubStepNone152D(r);
}

void abi_GateStart3CD(void)
{
    regs_exit_shim(GateStart3CD(regs_entry_shim()));
}

/*
 * @routine 03D0:1E0 GateWaitPlayer3CD
 * @run 6B:99E0
 * @unreachable 6B:99E0-6B:9A40 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $99E0 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The gate waits for the player, as GateWaitPlayer152D, with camera mode 1 cleared when the stage is
 * cleared through it; a packed byte with a high nibble other than $F names the sound, and camera mode is cleared;
 * with no high nibble, packed bytes 2 to 5 start the card's CD fade of type $0C.
 * HOW: Tests; stores; shifts; the kernel's piece at $400F with X 6 (X on the stack); a jump to cd_fade.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test, the kernel or cd_fade left them; X kept.
 */
StageRegs GateWaitPlayer3CD(StageRegs r)
{
    uint8_t gate;

    gate = (uint8_t)(rd8(GATE_BUSY) ^ 0x01);
    r = loaded(r, (uint8_t)(gate & rd8(PlayerDoorReady)));
    if (r.a == 0) {
        return r;
    }
    r = loaded(r, RD8(OBJ->hit_status[80 + r.x]));
    if (r.a == 0) {
        return r;
    }
    r = compared(r, 0x03);
    if (r.c) {
        return r;
    }
    r = loaded(r, (uint8_t)((RD8(OBJ->work_a[64 + r.x]) & 0x0F) + 1));
    WR8(ZP->field8c, r.a);
    r = loaded(r, (uint8_t)(RD8(SCROLLLIM->camera_mode) - 1));
    if (r.a == 0) {
        WR8(SCROLLLIM->camera_mode, 0);
    }
    r = loaded(r, 0x03);
    WR8(ZP->sub_step, 0x03);
    r.nz = INC8(OBJ->substate[24 + r.x]);
    WR8(OBJ->work_b[16 + r.x], 0);
    r = loaded(r, 0x04);
    WR8(OBJ->health[32 + r.x], 0x04);
    r = loaded(r, (uint8_t)(RD8(OBJ->work_a[64 + r.x]) & 0xF0));
    if (r.a == 0) {
        r = loaded(r, RD8(OBJ->work_a[64 + r.x]));
        r = compared(r, 0x02);
        if (!r.c) {
            return r;
        }
        r = compared(r, 0x06);
        if (r.c) {
            return r;
        }
        r = loaded(r, 0x0C);
        return fade_tail(r, 0x0C);
    }
    WR8(ZP->raster_effect, 0);
    WR8(SCROLLLIM->camera_mode, 0);
    r = compared(r, 0xF0);
    if (r.nz == 0) {
        return r;
    }
    r.c = (r.a & 0x08) != 0;
    r = loaded(r, (uint8_t)((r.a >> 4) | 0x80));
    push8(r.x);
    r.x = 0x06;
    r.nz = 0x06;
    r = vector_call(r, 0x9A2F, abi_VecStartListChannelKind2);
    r.x = pull8();
    r.nz = r.x;
    return r;
}

void abi_GateWaitPlayer3CD(void)
{
    regs_exit_shim(GateWaitPlayer3CD(regs_entry_shim()));
}

/*
 * @routine 03D0:241 GateWaitStep3CD
 * @run 6B:9A41
 * @unreachable 6B:9A41-6B:9A4A no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9A41 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The gate waits for the cleared stage's step at $85 to reach 2, then goes on.
 * HOW: A compare; an increment.
 * WAITS: no.
 * Entry: X the object. Exit: A $85, N, Z and C from the compare, or N and Z from the step's increment.
 */
StageRegs GateWaitStep3CD(StageRegs r)
{
    r = loaded(r, rd8(SUB3_STEP));
    r = compared(r, 0x02);
    if (r.nz != 0) {
        return r;
    }
    r.nz = INC8(OBJ->substate[24 + r.x]);
    return r;
}

void abi_GateWaitStep3CD(void)
{
    regs_exit_shim(GateWaitStep3CD(regs_entry_shim()));
}

/* The gate's next frame, as in stage_b_census.c, from the tables at $99DC and $99DE. */
static Regs gate_frame(Regs r, uint16_t ret)
{
    Sum sum;
    uint8_t base, count, step;

    r = loaded(r, 0x04);
    WR8(OBJ->health[32 + r.x], 0x04);
    r.y = (uint8_t)(RD8(OBJ->work_a[64 + r.x]) & 0x01);
    base = rd8((uint16_t)(0x99DC + r.y));
    count = RD8(OBJ->work_b[16 + r.x]);
    sum = add_bytes(base, count, false);
    WR8(OBJ->sprite_set[72 + r.x], sum.value);
    step = rd8((uint16_t)(0x99DE + r.y));
    sum = add_bytes(sum.value, step, false);
    r = loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    push8(r.x);
    r.x = 0x05;
    r.nz = 0x05;
    r = vector_call(r, ret, abi_VecStartListChannelKind2);
    r.x = pull8();
    r.nz = r.x;
    return r;
}

/*
 * @routine 03D0:24B GateOpen3CD
 * @run 6B:9A4B
 * @unreachable 6B:9A4B-6B:9A88 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9A4B runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The gate opens, as GateOpen152D, with sound $56 at the end.
 * HOW: A countdown at $32C8; a compare; gate_frame; the sound hook by a jump.
 * WAITS: no.
 * Entry: X the object. Exit: as the last instruction, the sound hook or the kernel left them; X kept.
 */
StageRegs GateOpen3CD(StageRegs r)
{
    r.nz = DEC8(OBJ->health[32 + r.x]);
    if (r.nz != 0) {
        return r;
    }
    r.nz = INC8(OBJ->work_b[16 + r.x]);
    r = loaded(r, RD8(OBJ->work_b[16 + r.x]));
    r = compared(r, 0x04);
    if (r.nz != 0) {
        return gate_frame(r, 0x9A87);
    }
    r.nz = INC8(OBJ->substate[24 + r.x]);
    r = loaded(r, (uint8_t)(RD8(ZP->field8c) | 0x80));
    WR8(ZP->field8c, r.a);
    r = loaded(r, 0x56);
    return vector_tail(r, abi_VecSoundCommand);
}

void abi_GateOpen3CD(void)
{
    regs_exit_shim(GateOpen3CD(regs_entry_shim()));
}

/*
 * @routine 03D0:289 GateWaitClear3CD
 * @run 6B:9A89
 * @unreachable 6B:9A89-6B:9AA5 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9A89 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The gate waits until zField8C loses bit 7, then goes on with its timer 4; a gate whose packed byte is 2
 * to 5 skips its closing.
 * HOW: A test; stores; compares; a second increment.
 * WAITS: no.
 * Entry: X the object. Exit: A zField8C, N and Z from it; or A the packed byte, N, Z and C from its compare, or N and Z
 * from the second increment.
 */
StageRegs GateWaitClear3CD(StageRegs r)
{
    r = loaded(r, RD8(ZP->field8c));
    if ((r.a & 0x80) != 0) {
        return r;
    }
    r.nz = INC8(OBJ->substate[24 + r.x]);
    r = loaded(r, 0x04);
    WR8(OBJ->health[32 + r.x], 0x04);
    r = loaded(r, RD8(OBJ->work_a[64 + r.x]));
    r = compared(r, 0x02);
    if (!r.c) {
        return r;
    }
    r = compared(r, 0x06);
    if (r.c) {
        return r;
    }
    r.nz = INC8(OBJ->substate[24 + r.x]);
    return r;
}

void abi_GateWaitClear3CD(void)
{
    regs_exit_shim(GateWaitClear3CD(regs_entry_shim()));
}

/*
 * @routine 03D0:2A6 GateClose3CD
 * @run 6B:9AA6
 * @unreachable 6B:9AA6-6B:9AD8 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9AA6 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: The gate closes, as GateClose152D, with sound $56 at the end.
 * HOW: A countdown at $32C8; a decrement; gate_frame; the sound hook by a jump.
 * WAITS: no.
 * Entry: X the object. Exit: as the last instruction, the sound hook or the kernel left them; X kept.
 */
StageRegs GateClose3CD(StageRegs r)
{
    r.nz = DEC8(OBJ->health[32 + r.x]);
    if (r.nz != 0) {
        return r;
    }
    r.nz = DEC8(OBJ->work_b[16 + r.x]);
    if ((r.nz & 0x80) == 0) {
        return gate_frame(r, 0x9AD7);
    }
    r.nz = INC8(OBJ->substate[24 + r.x]);
    r = loaded(r, 0x56);
    return vector_tail(r, abi_VecSoundCommand);
}

void abi_GateClose3CD(void)
{
    regs_exit_shim(GateClose3CD(regs_entry_shim()));
}

/* The routines below are byte copies of routines of the 152D build (stage_b_census.c) that run here at another
 * address. None has an absolute operand into its own bytes, so each is the same routine and its C is the 152D
 * routine's; they are filed here so that their assembly is assembled at this origin too. */

/*
 * @routine 03CD:1AD StageModeEventsStep3CD
 * @run 6B:81AD
 * @unreachable 6B:81AD-6B:81D2 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $81AD runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: As StageModeEventsStep152D (152D:0B1), whose bytes these are.
 * HOW: The same instructions; none of its absolute operands points into its own bytes, so it runs the same here.
 * WAITS: no.
 * Entry: nothing. Exit: as StageModeEventsStep152D: A the last value masked, N and Z from the last compare or mask; C
 * from the last compare; V from the add. X and Y kept.
 */
StageRegs StageModeEventsStep3CD(StageRegs r)
{
    return StageModeEventsStep152D(r);
}

void abi_StageModeEventsStep3CD(void)
{
    regs_exit_shim(StageModeEventsStep3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:34B TallyWait3CD
 * @run 6B:934B
 * @unreachable 6B:934B-6B:9355 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $934B runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: As TallyWait152D (152F:34D), whose bytes these are.
 * HOW: The same instructions; none of its absolute operands points into its own bytes, so it runs the same here.
 * WAITS: no.
 * Entry: nothing. Exit: as TallyWait152D: count 0, A 0, N and Z from it, the rest kept; else the caller's frame
 * dropped, A the last byte pulled, N and Z from it.
 */
TallyWaitEnd TallyWait3CD(StageRegs r)
{
    return TallyWait152D(r);
}

void abi_TallyWait3CD(void)
{
    regs_exit_shim(TallyWait3CD(regs_entry_shim()).r);
}

/*
 * @routine 03CF:638 HitKindSpawn3CD
 * @run 6B:9638
 * @unreachable 6B:9638-6B:9654 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $9638 runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: As HitKindSpawn152D (152F:5C7), whose bytes these are.
 * HOW: The same instructions; none of its absolute operands points into its own bytes, so it runs the same here.
 * WAITS: no.
 * Entry: X the object, Y the slot. Exit: as HitKindSpawn152D: A the last byte loaded, N and Z from it. X, Y, C and
 * V kept.
 */
StageRegs HitKindSpawn3CD(StageRegs r)
{
    return HitKindSpawn152D(r);
}

void abi_HitKindSpawn3CD(void)
{
    regs_exit_shim(HitKindSpawn3CD(regs_entry_shim()));
}

/*
 * @routine 03CF:67A HitKindState3CD
 * @run 6B:967A
 * @unreachable 6B:967A-6B:9680 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
 * bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
 * records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
 * reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
 * script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
 * poke of work RAM can load the sectors, and a unit call at $967A runs whatever overlay another scene left there, not
 * these bytes.
 *
 * WHAT IT DOES: As HitKindState152D (152F:5F1), whose bytes these are.
 * HOW: The same instructions; none of its absolute operands points into its own bytes, so it runs the same here.
 * WAITS: no.
 * Entry: X the object, Y the slot. Exit: as HitKindState152D: A the kind, N and Z from it. X, Y, C and V kept.
 */
StageRegs HitKindState3CD(StageRegs r)
{
    return HitKindState152D(r);
}

void abi_HitKindState3CD(void)
{
    regs_exit_shim(HitKindState3CD(regs_entry_shim()));
}

/* The 031D program's tally screen: its own routines as the shared bodies call them. */
static StageRegs draw_piece_031f(StageRegs r)
{
    stage_regs_load_shim(r);
    abi_SpawnStageObject031F();
    return stage_regs_from_cpu_shim(r);
}

static StageRegs clear_words_031f(StageRegs r)
{
    return ClearVramRows031F(r);
}

static StageRegs write_address_031f(StageRegs r)
{
    SetVramAddressXYAt(r.x, r.y, 0x9628);
    return loaded(r, 0x02);
}

static StageRegs set_addresses_031f(StageRegs r)
{
    SetVramAddresses031F(r.x, r.y);
    return loaded(r, 0x02);
}

static const TallyBuild tally_031f = {
    ADDR_OF(STAGEWORK->mask_buffer[0]), 0x95A0, ADDR_OF8(OBJ->tally_wait[0]), ADDR_OF8(OBJ->work_d[85]),
    ADDR_OF8(OBJ->frame[83]), 0x9368, 0x941B, 0x9587, 0x52, false,
    draw_piece_031f, clear_words_031f, write_address_031f, set_addresses_031f
};

/*
 * @routine 031F:310 TallyStart031F
 * @run 6B:9310
 * WHAT IT DOES: The tally screen's first step: after its wait, the sound stopped, sound $52, the next step.
 * HOW: TallyStart3CDAt (stage_c_census.c), 03CD's body, with this program's build (tally_031f).
 * WAITS: no.
 * Entry: nothing.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs TallyStart031F(StageRegs r)
{
    return TallyStart3CDAt(r, 0x9310, &tally_031f);
}

void abi_TallyStart031F(void)
{
    regs_exit_shim(TallyStart031F(regs_entry_shim()));
}

/*
 * @routine 031F:31E TallyBoard031F
 * @run 6B:931E
 * WHAT IT DOES: As TallyBoard3CD: the tally board set up, then run on into its digits.
 * HOW: TallyBoard3CDAt (stage_c_census.c), 03CD's body, with this program's build (tally_031f).
 * WAITS: no.
 * Entry: nothing.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs TallyBoard031F(StageRegs r)
{
    return TallyBoard3CDAt(r, 0x931E, &tally_031f);
}

void abi_TallyBoard031F(void)
{
    regs_exit_shim(TallyBoard031F(regs_entry_shim()));
}

/*
 * @routine 031F:368 TallyDigits031F
 * @run 6B:9368
 * WHAT IT DOES: As TallyDigits3CD: the health's and the hearts' digits put in the board's cells.
 * HOW: TallyDigits3CDAt (stage_c_census.c), 03CD's body, with this program's build (tally_031f).
 * WAITS: no.
 * Entry: nothing.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs TallyDigits031F(StageRegs r)
{
    return TallyDigits3CDAt(r, 0x9368, &tally_031f);
}

void abi_TallyDigits031F(void)
{
    regs_exit_shim(TallyDigits031F(regs_entry_shim()));
}

/*
 * @routine 031F:41B TallyCell031F
 * @run 6B:941B
 * WHAT IT DOES: As TallyCell3CD: one cell of the board drawn.
 * HOW: TallyCell3CDAt (stage_c_census.c), 03CD's body, with this program's build (tally_031f).
 * WAITS: no.
 * Entry: A the cell, zTemp00 its value.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs TallyCell031F(StageRegs r)
{
    return TallyCell3CDAt(r, 0x941B, &tally_031f);
}

void abi_TallyCell031F(void)
{
    regs_exit_shim(TallyCell031F(regs_entry_shim()));
}

/*
 * @routine 031F:4A0 TallyPause031F
 * @run 6B:94A0
 * WHAT IT DOES: As TallyPause3CD: after its wait, a wait of $40 and the next step.
 * HOW: TallyPause3CDAt (stage_c_census.c), 03CD's body, with this program's build (tally_031f).
 * WAITS: no.
 * Entry: nothing.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs TallyPause031F(StageRegs r)
{
    return TallyPause3CDAt(r, 0x94A0, &tally_031f);
}

void abi_TallyPause031F(void)
{
    regs_exit_shim(TallyPause031F(regs_entry_shim()));
}

/*
 * @routine 031F:54D TallyHealth031F
 * @run 6B:954D
 * WHAT IT DOES: As TallyHealth3CD: the board's health redrawn.
 * HOW: TallyHealth3CDAt (stage_c_census.c), 03CD's body, with this program's build (tally_031f).
 * WAITS: no.
 * Entry: nothing.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs TallyHealth031F(StageRegs r)
{
    return TallyHealth3CDAt(r, 0x954D, &tally_031f);
}

void abi_TallyHealth031F(void)
{
    regs_exit_shim(TallyHealth031F(regs_entry_shim()));
}

/*
 * @routine 031F:56A TallyHearts031F
 * @run 6B:956A
 * WHAT IT DOES: As TallyHearts3CD: the board's hearts redrawn.
 * HOW: TallyHearts3CDAt (stage_c_census.c), 03CD's body, with this program's build (tally_031f).
 * WAITS: no.
 * Entry: nothing.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs TallyHearts031F(StageRegs r)
{
    return TallyHearts3CDAt(r, 0x956A, &tally_031f);
}

void abi_TallyHearts031F(void)
{
    regs_exit_shim(TallyHearts031F(regs_entry_shim()));
}

/*
 * @routine 031F:587 TallyDigitCell031F
 * @run 6B:9587
 * WHAT IT DOES: As TallyDigitCell3CD: a cell stepped to its end.
 * HOW: TallyDigitCell3CDAt (stage_c_census.c), 03CD's body, with this program's build (tally_031f).
 * WAITS: no.
 * Entry: X the cell.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs TallyDigitCell031F(StageRegs r)
{
    return TallyDigitCell3CDAt(r, 0x9587, &tally_031f);
}

void abi_TallyDigitCell031F(void)
{
    regs_exit_shim(TallyDigitCell031F(regs_entry_shim()));
}

/*
 * @routine 031F:537 TallyEnd031F
 * @run 6B:9537
 * WHAT IT DOES: As TallyEnd3CD: after its wait, the kept health to zField94, zModeDone counted.
 * HOW: TallyEnd3CDAt (stage_c_census.c), 03CD's body, with this program's build (tally_031f).
 * WAITS: no.
 * Entry: nothing.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs TallyEnd031F(StageRegs r)
{
    return TallyEnd3CDAt(r, 0x9537, &tally_031f);
}

void abi_TallyEnd031F(void)
{
    regs_exit_shim(TallyEnd031F(regs_entry_shim()));
}

/*
 * @routine 031F:390 TallyReveal031F
 * @run 6B:9390
 * WHAT IT DOES: As TallyReveal3CD, but the reveal's end does not set bit 1 of zPaused: one cell of the board revealed a call.
 * HOW: TallyReveal3CDAt (stage_c_census.c), 03CD's body, with this program's build (tally_031f).
 * WAITS: no.
 * Entry: nothing.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs TallyReveal031F(StageRegs r)
{
    return TallyReveal3CDAt(r, 0x9390, &tally_031f);
}

void abi_TallyReveal031F(void)
{
    regs_exit_shim(TallyReveal031F(regs_entry_shim()));
}

/*
 * @routine 031F:500 TallyWaitPress031F
 * @run 6B:9500
 * WHAT IT DOES: After its wait, bit 1 of zPaused set and the paused step run until it clears that bit; then a wait of $20 and the next step.
 * HOW: The wait; SMB1; the routine loaded at $8EA0; a test of the bit.
 * WAITS: no.
 * Entry: nothing.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs TallyWaitPress031F(StageRegs r)
{
    TallyWaitEnd wait = tally_wait_at(r, 0x9503, OBJ_TALLY_WAIT);

    r = wait.r;
    if (wait.dropped) {
        return r;
    }
    RMW8(ZP->paused, old, old | 0x02);
    r = resident_call(r, 0x9508, 0x8EA0);
    if ((RD8(ZP->paused) & 0x02) != 0) {
        return r;
    }
    r = loaded(r, 0x20);
    WR8(OBJ->tally_wait[0], 0x20);
    return TallyNext152D(r);
}

void abi_TallyWaitPress031F(void)
{
    regs_exit_shim(TallyWaitPress031F(regs_entry_shim()));
}

/*
 * @routine 031F:514 TallyStockBonus031F
 * @run 6B:9514
 * WHAT IT DOES: After its wait, an extra life (TallyExtraLife031F), the routine at $9103, a wait of $80 and the next step.
 * HOW: The wait; TallyExtraLife031F by BSR; the routine loaded at $9103; a store.
 * WAITS: no.
 * Entry: nothing.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs TallyStockBonus031F(StageRegs r)
{
    TallyWaitEnd wait = tally_wait_at(r, 0x9517, OBJ_TALLY_WAIT);

    r = wait.r;
    if (wait.dropped) {
        return r;
    }
    CALL(0x9519, TallyExtraLife031F(r));
    r = resident_call(r, 0x951C, 0x9103);
    r = loaded(r, 0x80);
    WR8(OBJ->tally_wait[0], 0x80);
    return TallyNext152D(r);
}

void abi_TallyStockBonus031F(void)
{
    regs_exit_shim(TallyStockBonus031F(regs_entry_shim()));
}

/*
 * @routine 031F:524 TallyExtraLife031F
 * @run 6B:9524
 * WHAT IT DOES: A life more: the player's stock one up in decimal, 99 at most, and sound $48.
 * HOW: bcd_plus_one; the sound command.
 * WAITS: no.
 * Entry: nothing.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs TallyExtraLife031F(StageRegs r)
{
    r = loaded(r, RD8(ZP->stock));
    r = bcd_plus_one(r);
    if (r.c) {
        r = loaded(r, 0x99);
    }
    WR8(ZP->stock, r.a);
    r = loaded(r, 0x48);
    return vector_call(r, 0x9536, abi_VecSoundCommand);
}

void abi_TallyExtraLife031F(void)
{
    regs_exit_shim(TallyExtraLife031F(regs_entry_shim()));
}

/*
 * @routine 031F:542 TallyWait031F
 * @run 6B:9542
 * WHAT IT DOES: The tally steps' wait: while OBJ_TALLY_WAIT is not 0, one less and the caller's return dropped.
 * HOW: TallyWait152DAt (stage_b_census.c), the body of 152D's wait, on OBJ_TALLY_WAIT.
 * WAITS: no.
 * Entry: nothing.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
TallyWaitEnd TallyWait031F(StageRegs r)
{
    return TallyWait152DAt(r, OBJ_TALLY_WAIT);
}

void abi_TallyWait031F(void)
{
    regs_exit_shim(TallyWait031F(regs_entry_shim()).r);
}

/*
 * @routine 031F:5FE ClearVramRows031F
 * @run 6B:95FE
 * WHAT IT DOES: ClearVramRows' bytes (004D:4C8) in this program.
 * HOW: ClearVramRowsAt (continue_screen.c) with the returns of its JSRs here, $9603 and $9628.
 * WAITS: no.
 * Entry: A rows, X the address low, Y high.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs ClearVramRows031F(StageRegs r)
{
    ClearVramRowsAt(r.a, r.x, r.y, 0x9603, 0x9628);
    r = loaded(r, 0x02);
    r.x = 0;
    r.nz = 0;
    return r;
}

void abi_ClearVramRows031F(void)
{
    regs_exit_shim(ClearVramRows031F(regs_entry_shim()));
}

/*
 * @routine 031F:2F1 StageSubModeTally031F
 * @run 6B:92F1
 * WHAT IT DOES: Sub-mode 6 in the 031D program, the tally after the boss: zWaveCounter counted, the stage's frame, the scroll and objects,
 * then the tally's step at zField80 from the eight at $9300.
 * HOW: An increment; JSRs to the routines loaded at $8B23 and $8A05; zField80 doubled indexes the table, whose word is jumped
 * through.
 * WAITS: no.
 * Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice zField80, N and Z from it, C its bit 7.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageSubModeTally031F(StageRegs r)
{
    uint8_t step, twice;
    uint16_t handler;

    r.nz = INC8(ZP->wave_counter);
    r = resident_call(r, 0x92F6, 0x8B23);
    r = resident_call(r, 0x92F9, 0x8A05);
    step = RD8(ZP->field80);
    twice = (uint8_t)(step << 1);
    r.c = (step & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    handler = table_word(0x9300, twice);
    switch (handler) {
    case 0x9310:
        return TallyStart031F(r);
    case 0x931E:
        return TallyBoard031F(r);
    case 0x9390:
        return TallyReveal031F(r);
    case 0x94A0:
        return TallyPause031F(r);
    case 0x94AB:
        return TallyCount031F(r);
    case 0x9500:
        return TallyWaitPress031F(r);
    case 0x9514:
        return TallyStockBonus031F(r);
    case 0x9537:
        return TallyEnd031F(r);
    default:
        return resident_tail(r, handler);
    }
}

void abi_StageSubModeTally031F(void)
{
    regs_exit_shim(StageSubModeTally031F(regs_entry_shim()));
}

/*
 * @routine 031F:4AB TallyCount031F
 * @run 6B:94AB
 * WHAT IT DOES: The tally's count: while there is health it is counted into the score (zPlayerHealth down by one, the board's health
 * redrawn, 1 point); then after the wait the hearts (zField94 down by one in BCD, redrawn, $10 points). A carry from the
 * score gives a life more and puts zStock in StageShownStock. The score's rows are blended, and sound 1 plays every fourth frame. With
 * both at 0 the tally goes on two steps with a wait of $C0, or, with the full-health mark at $2ABB set, stays with a wait
 * of $40.
 * HOW: Decrements; TallyHealth031F, TallyWait031F, TallyHearts031F; the kernel's score add; TallyExtraLife031F by BSR on a
 * carry; the routine loaded at $915E.
 * WAITS: no.
 * Entry: D clear, as the game keeps it. Exit: as the last piece that ran left them.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs TallyCount031F(StageRegs r)
{
    TallyWaitEnd wait;

    r = loaded(r, RD8(ZP->player_health));
    if (r.a != 0) {
        r.nz = DEC8(ZP->player_health);
        CALL(0x94B4, TallyHealth031F(r));
        r = loaded(r, 0x01);
    } else {
        wait = tally_wait_at(r, 0x94BB, OBJ_TALLY_WAIT);
        r = wait.r;
        if (wait.dropped) {
            return r;
        }
        r = loaded(r, RD8(ZP->field94));
        if (r.a == 0) {
            r = loaded(r, 0xC0);
            WR8(OBJ->tally_wait[0], 0xC0);
            r.nz = INC8(ZP->field80);
            r.nz = INC8(ZP->field80);
            r = loaded(r, RD8(OBJ->frame[83]));
            if (r.a != 0) {
                r.nz = DEC8(ZP->field80);
                r.nz = DEC8(ZP->field80);
                r = loaded(r, 0x40);
                WR8(OBJ->tally_wait[0], 0x40);
            }
            return TallyNext152D(r);
        }
        r = bcd_less_one(r);
        WR8(ZP->field94, r.a);
        CALL(0x94C9, TallyHearts031F(r));
        r = loaded(r, 0x10);
    }
    r = vector_call(r, 0x94CE, abi_VecAddScoreByte);
    if (r.c) {
        CALL(0x94D2, TallyExtraLife031F(r));
        r = loaded(r, RD8(ZP->stock));
        WR8(STAGEVARS->shown_stock[0], r.a);
    }
    r = resident_call(r, 0x94DA, 0x915E);
    r = loaded(r, (uint8_t)(RD8(ZP->wave_counter) & 0x03));
    if (r.a != 0) {
        return r;
    }
    r = loaded(r, 0x01);
    return vector_call(r, 0x94E5, abi_VecSoundCommand);
}

void abi_TallyCount031F(void)
{
    regs_exit_shim(TallyCount031F(regs_entry_shim()));
}
