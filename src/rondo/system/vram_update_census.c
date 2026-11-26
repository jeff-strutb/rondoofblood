/*
 * Census copies of the 15 bytes at $8142 before VramListFromIndex, in overlays no read the game makes can load
 * (versions of the ending's effect overlay at sectors 0441, 0651, 1231, 1A31, 1B91 and 25F7). As in every loaded
 * overlay (vram_update.c, VramListFromTableA to W), each copy reads its own second table of VRAM lists; the rest of
 * these overlays from $8151 to $829E is byte for byte the 00C5 overlay's, whose C (VramListRun) serves it.
 */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "vram_update.h"
#include "vram_update_census.h"

/* What the list loop leaves, set on the registers: every exit is the CMP #$FF that matched (as vram_update.c's).
 * VramListRun never goes on into original code, so its exit is always these. */
static void census_vram_exit_shim(VramExit out)
{
    cpu.a = out.a;
    cpu.x = out.x;
    cpu.y = out.y;
    abi_nz(0);
    abi_flag(P_C, out.carry);
    abi_flag(P_V, out.overflow);
}

static bool census_entry_overflow_shim(void)
{
    return (cpu.p & P_V) != 0;
}

/* The 15 bytes: A doubled indexes the table, its word goes to zp $14 and $15 (the list pointer, named zProduct1
 * and zProduct2 in the records), Y 0, and the branch joins VramListRun at $8169 with C bit 7 of A. */
static VramExit census_list_from_table(uint16_t table, uint8_t a, uint8_t x, bool overflow)
{
    uint8_t y = (uint8_t)(a << 1);
    bool carry = (a & 0x80) != 0;
    uint8_t value;

    value = rd8((uint16_t)(table + y));
    WR8(ZP->product.mid, value);
    value = rd8((uint16_t)(table + 1 + y));
    WR8(ZP->product.hi, value);
    return VramListRun(x, 0, carry, overflow);
}

/*
 * @routine 0441:142 VramListFromTable0441
 * @run 6C:8142
 * @stop 8151
 * @unreachable 6C:8142-6C:8150 no read the game can make loads disc sector 0441, so these bytes are never in
 * memory. Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites
 * included; the boot block's three reads ($43 for 10, $4D for 12, $59 for 8 sectors); the card's reads of sectors 0
 * to 2; the old-card program's 20 sectors from $23; the five cd_read calls no script runs (sectors 0 to 3 and $23,
 * from constants); LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, which go to ADPCM memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8142 runs whatever overlay another scene
 * left there, not these bytes. The C is optimistic: no script reaches it.
 *
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table
 * of lists, at $8E6A, gives its pointer.
 * HOW: A doubled indexes the table at $8E6A, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 * liftcmp: 402 of 402 cases match its census draft (census_draft.py; callees as filed wrappers; curated
 * census_vram_list_0441.txt). The exit shim's last line is not reached there: a case ends in the tail-called
 * VramListRun, as VramListFromTableC's does.
 */
VramExit VramListFromTable0441(uint8_t a, uint8_t x, bool overflow)
{
    return census_list_from_table(0x8E6A, a, x, overflow);
}

void abi_VramListFromTable0441(void)
{
    census_vram_exit_shim(VramListFromTable0441(cpu.a, cpu.x, census_entry_overflow_shim()));
}

/*
 * @routine 0651:142 VramListFromTable0651
 * @run 6C:8142
 * @stop 8151
 * @unreachable 6C:8142-6C:8150 no read the game can make loads disc sector 0651, so these bytes are never in
 * memory. Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites
 * included; the boot block's three reads ($43 for 10, $4D for 12, $59 for 8 sectors); the card's reads of sectors 0
 * to 2; the old-card program's 20 sectors from $23; the five cd_read calls no script runs (sectors 0 to 3 and $23,
 * from constants); LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, which go to ADPCM memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8142 runs whatever overlay another scene
 * left there, not these bytes. The C is optimistic: no script reaches it.
 *
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table
 * of lists, at $8E65, gives its pointer.
 * HOW: A doubled indexes the table at $8E65, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 * liftcmp: 402 of 402 cases match its census draft (census_draft.py; callees as filed wrappers; curated
 * census_vram_list_0651.txt). The exit shim's last line is not reached there: a case ends in the tail-called
 * VramListRun, as VramListFromTableC's does.
 */
VramExit VramListFromTable0651(uint8_t a, uint8_t x, bool overflow)
{
    return census_list_from_table(0x8E65, a, x, overflow);
}

void abi_VramListFromTable0651(void)
{
    census_vram_exit_shim(VramListFromTable0651(cpu.a, cpu.x, census_entry_overflow_shim()));
}

/*
 * @routine 1231:142 VramListFromTable1231
 * @run 6C:8142
 * @stop 8151
 * @unreachable 6C:8142-6C:8150 no read the game can make loads disc sector 1231, so these bytes are never in
 * memory. Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites
 * included; the boot block's three reads ($43 for 10, $4D for 12, $59 for 8 sectors); the card's reads of sectors 0
 * to 2; the old-card program's 20 sectors from $23; the five cd_read calls no script runs (sectors 0 to 3 and $23,
 * from constants); LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, which go to ADPCM memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8142 runs whatever overlay another scene
 * left there, not these bytes. The C is optimistic: no script reaches it.
 *
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table
 * of lists, at $9118, gives its pointer.
 * HOW: A doubled indexes the table at $9118, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 * liftcmp: 402 of 402 cases match its census draft (census_draft.py; callees as filed wrappers; curated
 * census_vram_list_1231.txt). The exit shim's last line is not reached there: a case ends in the tail-called
 * VramListRun, as VramListFromTableC's does.
 */
VramExit VramListFromTable1231(uint8_t a, uint8_t x, bool overflow)
{
    return census_list_from_table(0x9118, a, x, overflow);
}

void abi_VramListFromTable1231(void)
{
    census_vram_exit_shim(VramListFromTable1231(cpu.a, cpu.x, census_entry_overflow_shim()));
}

/*
 * @routine 1A31:142 VramListFromTable1A31
 * @run 6C:8142
 * @stop 8151
 * @unreachable 6C:8142-6C:8150 no read the game can make loads disc sector 1A31, so these bytes are never in
 * memory. Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites
 * included; the boot block's three reads ($43 for 10, $4D for 12, $59 for 8 sectors); the card's reads of sectors 0
 * to 2; the old-card program's 20 sectors from $23; the five cd_read calls no script runs (sectors 0 to 3 and $23,
 * from constants); LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, which go to ADPCM memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8142 runs whatever overlay another scene
 * left there, not these bytes. The C is optimistic: no script reaches it.
 *
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table
 * of lists, at $8ED2, gives its pointer.
 * HOW: A doubled indexes the table at $8ED2, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 * liftcmp: 402 of 402 cases match its census draft (census_draft.py; callees as filed wrappers; curated
 * census_vram_list_1A31.txt). The exit shim's last line is not reached there: a case ends in the tail-called
 * VramListRun, as VramListFromTableC's does.
 */
VramExit VramListFromTable1A31(uint8_t a, uint8_t x, bool overflow)
{
    return census_list_from_table(0x8ED2, a, x, overflow);
}

void abi_VramListFromTable1A31(void)
{
    census_vram_exit_shim(VramListFromTable1A31(cpu.a, cpu.x, census_entry_overflow_shim()));
}

/*
 * @routine 1B91:142 VramListFromTable1B91
 * @run 6C:8142
 * @stop 8151
 * @unreachable 6C:8142-6C:8150 no read the game can make loads disc sector 1B91, so these bytes are never in
 * memory. Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites
 * included; the boot block's three reads ($43 for 10, $4D for 12, $59 for 8 sectors); the card's reads of sectors 0
 * to 2; the old-card program's 20 sectors from $23; the five cd_read calls no script runs (sectors 0 to 3 and $23,
 * from constants); LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, which go to ADPCM memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8142 runs whatever overlay another scene
 * left there, not these bytes. The C is optimistic: no script reaches it.
 *
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table
 * of lists, at $8D82, gives its pointer.
 * HOW: A doubled indexes the table at $8D82, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 * liftcmp: 402 of 402 cases match its census draft (census_draft.py; callees as filed wrappers; curated
 * census_vram_list_1B91.txt). The exit shim's last line is not reached there: a case ends in the tail-called
 * VramListRun, as VramListFromTableC's does.
 */
VramExit VramListFromTable1B91(uint8_t a, uint8_t x, bool overflow)
{
    return census_list_from_table(0x8D82, a, x, overflow);
}

void abi_VramListFromTable1B91(void)
{
    census_vram_exit_shim(VramListFromTable1B91(cpu.a, cpu.x, census_entry_overflow_shim()));
}

/*
 * @routine 25F7:142 VramListFromTable25F7
 * @run 6C:8142
 * @stop 8151
 * @unreachable 6C:8142-6C:8150 no read the game can make loads disc sector 25F7, so these bytes are never in
 * memory. Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites
 * included; the boot block's three reads ($43 for 10, $4D for 12, $59 for 8 sectors); the card's reads of sectors 0
 * to 2; the old-card program's 20 sectors from $23; the five cd_read calls no script runs (sectors 0 to 3 and $23,
 * from constants); LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
 * SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, which go to ADPCM memory the CPU cannot
 * run. No input or poke of work RAM can load the sector, and a unit call at $8142 runs whatever overlay another scene
 * left there, not these bytes. The C is optimistic: no script reaches it.
 *
 * WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table
 * of lists, at $829F, gives its pointer.
 * HOW: A doubled indexes the table at $829F, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
 * WAITS: no.
 * Entry: A the list number ($80 or more). Exit: as VramListRun.
 * liftcmp: 402 of 402 cases match its census draft (census_draft.py; callees as filed wrappers; curated
 * census_vram_list_25F7.txt). The exit shim's last line is not reached there: a case ends in the tail-called
 * VramListRun, as VramListFromTableC's does.
 */
VramExit VramListFromTable25F7(uint8_t a, uint8_t x, bool overflow)
{
    return census_list_from_table(0x829F, a, x, overflow);
}

void abi_VramListFromTable25F7(void)
{
    census_vram_exit_shim(VramListFromTable25F7(cpu.a, cpu.x, census_entry_overflow_shim()));
}
