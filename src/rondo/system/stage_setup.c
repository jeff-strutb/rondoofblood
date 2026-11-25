/* Stage setup: the state bytes a stage keeps in the last slots of the object table's columns. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum {
    MPR5 = 0x20, MPR6 = 0x40, STAGE_BANK_OFFSET = 0x10
};

/*
 * @routine 024F:19F ClearStageStateBytes
 * @run 6B:919F
 * WHAT IT DOES: Zeroes four of the stage's state bytes, the ones a stage restart or its
 * start-of-stage handler clears together.
 * HOW: Four stores of zero, into the stage overlay's state bytes (StageVar350E, StageVar3566,
 * StageVar35BE and the blend step).
 * WAITS: no.
 * Entry: nothing. Exit: A, X, Y and flags kept.
 */
void ClearStageStateBytes(void)
{
    WR8(STAGEVARS->state_350e[0], 0);
    WR8(STAGEVARS->state_3566[0], 0);
    WR8(STAGEVARS->state_35be[0], 0);
    WR8(STAGEVARS->blend_step[0], 0);
}

void abi_ClearStageStateBytes(void)
{
    ClearStageStateBytes();
}

/* Maps the two banks and returns the add's result, whose carry and overflow the wrapper shows. */
static Sum map_stage_banks(void)
{
    uint8_t base = RD8(ZP->ram_base_bank);
    Sum sum = add_bytes(STAGE_BANK_OFFSET, base, false);

    tam(MPR5, sum.value);
    tam(MPR6, (uint8_t)(sum.value + 1));
    return sum;
}

static void map_exit_shim(Sum sum)
{
    cpu.a = (uint8_t)(sum.value + 1);
    abi_nz(cpu.a);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

/*
 * @routine 024F:349 MapStageBanks
 * @run 6B:9349
 * WHAT IT DOES: Maps the stage's two Super RAM banks into MPR5 and MPR6 ($A000 and $C000):
 * the banks $10 and $11 above the card's first Super RAM bank.
 * HOW: $10 plus zRamBaseBank into MPR5, that plus one into MPR6.
 * WAITS: no.
 * Entry: nothing. Exit: A the MPR6 bank, flags from the add and the increment (C and V from
 * the add, N and Z from A). X and Y kept.
 */
void MapStageBanks(void)
{
    map_stage_banks();
}

void abi_MapStageBanks(void)
{
    map_exit_shim(map_stage_banks());
}

/*
 * @routine 024F:33F SaveAndMapStageBanks
 * @run 6B:933F
 * WHAT IT DOES: Keeps the banks in MPR5 and MPR6 and then maps the stage's two banks there.
 * HOW: Each MPR into its saved byte (the second bytes of the blend step's and StageVar371E's columns), then MapStageBanks, which this routine runs on into with no
 * return of its own. RestoreStageBanks puts the saved ones back.
 * WAITS: no.
 * Entry: nothing. Exit: as MapStageBanks.
 */
static Sum save_and_map_stage_banks(void)
{
    uint8_t bank;

    bank = tma(MPR5);
    WR8(STAGEVARS->blend_step[1], bank);
    bank = tma(MPR6);
    WR8(STAGEVARS->state_371e[1], bank);
    return map_stage_banks();
}

Sum SaveAndMapStageBanks(void)
{
    return save_and_map_stage_banks();
}

void abi_SaveAndMapStageBanks(void)
{
    map_exit_shim(save_and_map_stage_banks());
}

/*
 * @routine 024F:354 RestoreStageBanks
 * @run 6B:9354
 * WHAT IT DOES: Puts back the banks SaveAndMapStageBanks kept in MPR5 and MPR6.
 * HOW: MPR6 from the byte at the second byte of the StageVar371E column, then MPR5 from the blend step's second byte.
 * WAITS: no.
 * Entry: nothing. Exit: A the MPR5 bank, N and Z from it. X and Y kept.
 */
uint8_t RestoreStageBanks(void)
{
    uint8_t bank;

    bank = RD8(STAGEVARS->state_371e[1]);
    tam(MPR6, bank);
    bank = RD8(STAGEVARS->blend_step[1]);
    tam(MPR5, bank);
    return bank;
}

void abi_RestoreStageBanks(void)
{
    cpu.a = RestoreStageBanks();
    abi_nz(cpu.a);
}

/*
 * @routine 05BF:651 SaveAndMapStageBanksLong
 * @run 6B:9651
 * WHAT IT DOES: SaveAndMapStageBanks, as the stages whose overlay runs it at $9651 have it.
 * HOW: The same bytes as SaveAndMapStageBanks at another run address; runs on into
 * MapStageBanksLong. Its C is SaveAndMapStageBanks.
 * WAITS: no.
 * Entry: nothing. Exit: as MapStageBanks.
 */
void abi_SaveAndMapStageBanksLong(void)
{
    abi_SaveAndMapStageBanks();
}

/*
 * @routine 05BF:65B MapStageBanksLong
 * @run 6B:965B
 * WHAT IT DOES: MapStageBanks, as the stages whose overlay runs it at $965B have it.
 * HOW: The same bytes as MapStageBanks at another run address. Its C is MapStageBanks.
 * WAITS: no.
 * Entry: nothing. Exit: as MapStageBanks.
 */
void abi_MapStageBanksLong(void)
{
    abi_MapStageBanks();
}

/*
 * @routine 05BF:666 RestoreStageBanksLong
 * @run 6B:9666
 * WHAT IT DOES: RestoreStageBanks, as the stages whose overlay runs it at $9666 have it.
 * HOW: The same bytes as RestoreStageBanks at another run address. Its C is RestoreStageBanks.
 * WAITS: no.
 * Entry: nothing. Exit: as RestoreStageBanks.
 */
void abi_RestoreStageBanksLong(void)
{
    abi_RestoreStageBanks();
}

typedef struct { uint16_t slot, frame, kind, x_lo, y_lo; } SpawnTables;

/* A row of a stage's spawn tables: the object it names is emptied and given its frame, kind and position low bytes. */
static SpawnedObject spawn_from_row(uint8_t row, const SpawnTables *t, uint16_t ret)
{
    SpawnedObject out;
    uint8_t byte;

    out.slot = rd8((uint16_t)(t->slot + row));
    jsr_push(ret);
    ClearObjectInY(out.slot);
    jsr_pop();
    byte = rd8((uint16_t)(t->frame + row));
    WR8(OBJ->frame[out.slot], byte);
    byte = rd8((uint16_t)(t->kind + row));
    WR8(OBJ->f2a10[out.slot], byte);
    byte = rd8((uint16_t)(t->x_lo + row));
    WR8(OBJ->x_lo[out.slot], byte);
    byte = rd8((uint16_t)(t->y_lo + row));
    WR8(OBJ->y_lo[out.slot], byte);
    out.y_lo = byte;
    return out;
}

static const SpawnTables spawn_a = { StageSpawnSlot, StageSpawnFrame, StageSpawnKind, StageSpawnXLo, StageSpawnYLo };
static const SpawnTables spawn_b = { StageSpawnSlotB, StageSpawnFrameB, StageSpawnKindB, StageSpawnXLoB, StageSpawnYLoB };

/*
 * @routine 024F:3B0 SpawnStageObject
 * @run 6B:93B0
 * WHAT IT DOES: Puts a new object in the slot a row of the stage's spawn tables names: the
 * slot is emptied, then given the row's frame, kind and X and Y low bytes.
 * HOW: The five tables are read by the row, X; the slot comes from the first. The other stage
 * group's copy, SpawnStageObjectLong, reads its own tables.
 * WAITS: no.
 * Entry: X the row, 0 to 7. Exit: A the Y low byte given, N and Z from it, Y the slot, X kept.
 */
SpawnedObject SpawnStageObject(uint8_t row)
{
    return spawn_from_row(row, &spawn_a, 0x93B7);
}

void abi_SpawnStageObject(void)
{
    SpawnedObject out = SpawnStageObject(cpu.x);

    cpu.a = out.y_lo;
    cpu.y = out.slot;
    abi_nz(cpu.a);
}

/*
 * @routine 05BF:6C2 SpawnStageObjectLong
 * @run 6B:96C2
 * WHAT IT DOES: SpawnStageObject for the other stage group, which keeps its spawn tables at
 * $96E2 to $9702.
 * HOW: The same code with its own tables.
 * WAITS: no.
 * Entry: X the row, 0 to 7. Exit: as SpawnStageObject.
 */
SpawnedObject SpawnStageObjectLong(uint8_t row)
{
    return spawn_from_row(row, &spawn_b, 0x96C9);
}

void abi_SpawnStageObjectLong(void)
{
    SpawnedObject out = SpawnStageObjectLong(cpu.x);

    cpu.a = out.y_lo;
    cpu.y = out.slot;
    abi_nz(cpu.a);
}
