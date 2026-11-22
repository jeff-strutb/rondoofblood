/* Stage setup: the state bytes a stage keeps in the last slots of the object table's columns. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "stage_map.h"

enum {
    STAGE_STATE_SLOT = 0x56, SAVED_MPR_SLOT = 0x57,
    MPR5 = 0x20, MPR6 = 0x40, STAGE_BANK_OFFSET = 0x10
};

/*
 * @routine 024F:19F ClearStageStateBytes
 * @run 6B:919F
 * WHAT IT DOES: Zeroes four of the stage's state bytes, the ones a stage restart or its
 * start-of-stage handler clears together.
 * HOW: Four stores of zero, each into slot $56 of one column of the object table (columns
 * $34B8, $3510, $3568 and $3670). The columns are the object table's, the use is the stage
 * overlay's own; the field names are the table's until the typed layer names them.
 * WAITS: no.
 * Entry: nothing. Exit: A, X, Y and flags kept.
 */
void ClearStageStateBytes(void)
{
    WR8(OBJ->f34b8[STAGE_STATE_SLOT], 0);
    WR8(OBJ->f3510[STAGE_STATE_SLOT], 0);
    WR8(OBJ->f3568[STAGE_STATE_SLOT], 0);
    WR8(OBJ->f3670[STAGE_STATE_SLOT], 0);
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

static void show_map_exit(Sum sum)
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
    show_map_exit(map_stage_banks());
}

/*
 * @routine 024F:33F SaveAndMapStageBanks
 * @run 6B:933F
 * WHAT IT DOES: Keeps the banks in MPR5 and MPR6 and then maps the stage's two banks there.
 * HOW: Each MPR into its saved byte (slot $57 of columns $3670 and $36C8 of the object
 * table, the stage's own use), then MapStageBanks, which this routine runs on into with no
 * return of its own. RestoreStageBanks puts the saved ones back.
 * WAITS: no.
 * Entry: nothing. Exit: as MapStageBanks.
 */
static Sum save_and_map_stage_banks(void)
{
    uint8_t bank;

    bank = tma(MPR5);
    WR8(OBJ->f3670[SAVED_MPR_SLOT], bank);
    bank = tma(MPR6);
    WR8(OBJ->f36c8[SAVED_MPR_SLOT], bank);
    return map_stage_banks();
}

void SaveAndMapStageBanks(void)
{
    save_and_map_stage_banks();
}

void abi_SaveAndMapStageBanks(void)
{
    show_map_exit(save_and_map_stage_banks());
}

/*
 * @routine 024F:354 RestoreStageBanks
 * @run 6B:9354
 * WHAT IT DOES: Puts back the banks SaveAndMapStageBanks kept in MPR5 and MPR6.
 * HOW: MPR6 from the byte at slot $57 of column $36C8, then MPR5 from slot $57 of $3670.
 * WAITS: no.
 * Entry: nothing. Exit: A the MPR5 bank, N and Z from it. X and Y kept.
 */
uint8_t RestoreStageBanks(void)
{
    uint8_t bank;

    bank = RD8(OBJ->f36c8[SAVED_MPR_SLOT]);
    tam(MPR6, bank);
    bank = RD8(OBJ->f3670[SAVED_MPR_SLOT]);
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
