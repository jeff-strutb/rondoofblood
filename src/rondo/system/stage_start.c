/* Starting a stage's lists: the list channel and the script slot its section names. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum { SCRIPT_SLOT = 6, LIST_RETURN_1 = 0x88B4, LIST_RETURN_2 = 0x88BC, LIST_RETURN_3 = 0x88BF };

/*
 * Clears the list channels, starts channel 0 as kind 2 with the list the overlay names for the
 * section, expands the section's stage lists, and starts script slot 6 with the script the
 * overlay names for the section and the character playing (two scripts a section).
 */
/* The registers the list channel start leaves: the callee's block move saves them on the stack. */
static void channel_registers_shim(uint8_t a, uint8_t x)
{
    cpu.a = a;
    cpu.x = x;
}

static StageStartEnd start_stage_lists(uint16_t lists_at, uint16_t scripts_at, StageListEnd (*expand)(void))
{
    StageStartEnd end;
    uint8_t x;
    uint8_t list;
    uint8_t script;
    uint8_t section;

    jsr_push(LIST_RETURN_1);
    ClearListChannels();
    jsr_pop();
    x = RD8(ZP->field82);
    list = rd8((uint16_t)(lists_at + x));
    jsr_push(LIST_RETURN_2);
    channel_registers_shim(StartListChannel0Kind2(list), 0);
    jsr_pop();
    jsr_push(LIST_RETURN_3);
    expand();
    jsr_pop();
    section = RD8(ZP->field82);
    end.index = add_bytes((uint8_t)(section << 1), RD8(ZP->character), false);
    script = rd8((uint16_t)(scripts_at + end.index.value));
    end.script = StartScriptSlotKind1(script, SCRIPT_SLOT);
    return end;
}

static void start_exit_shim(StageStartEnd end)
{
    script_slot_exit_shim(end.script);
    cpu.x = SCRIPT_SLOT;
    abi_flag(P_C, end.index.carry);
    abi_flag(P_V, end.index.overflow);
}

/*
 * @routine 024E:0B1 StartStageLists
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageLists(void)
{
    return start_stage_lists(0x9B86, 0x9B8E, ExpandStageLists);
}

void abi_StartStageLists(void)
{
    start_exit_shim(StartStageLists());
}

/*
 * @routine 050E:0B1 StartStageListsB
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9C12 and $9C1A.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsB(void)
{
    return start_stage_lists(0x9C12, 0x9C1A, ExpandStageListsB);
}

void abi_StartStageListsB(void)
{
    start_exit_shim(StartStageListsB());
}

/*
 * @routine 05BE:0B1 StartStageListsC
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9E6D and $9E75.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsC(void)
{
    return start_stage_lists(0x9E6D, 0x9E75, ExpandStageListsC);
}

void abi_StartStageListsC(void)
{
    start_exit_shim(StartStageListsC());
}

/*
 * @routine 071E:0B1 StartStageListsD
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9F31 and $9F39.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsD(void)
{
    return start_stage_lists(0x9F31, 0x9F39, ExpandStageListsD);
}

void abi_StartStageListsD(void)
{
    start_exit_shim(StartStageListsD());
}

/*
 * @routine 07CE:0B1 StartStageListsE
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9CC6 and $9CCA.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsE(void)
{
    return start_stage_lists(0x9CC6, 0x9CCA, ExpandStageListsE);
}

void abi_StartStageListsE(void)
{
    start_exit_shim(StartStageListsE());
}

/*
 * @routine 087E:0B1 StartStageListsF
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9E61 and $9E62.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsF(void)
{
    return start_stage_lists(0x9E61, 0x9E62, ExpandStageListsF);
}

void abi_StartStageListsF(void)
{
    start_exit_shim(StartStageListsF());
}

/*
 * @routine 092E:0B1 StartStageListsG
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9CC3 and $9CCB.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsG(void)
{
    return start_stage_lists(0x9CC3, 0x9CCB, ExpandStageListsG);
}

void abi_StartStageListsG(void)
{
    start_exit_shim(StartStageListsG());
}

/*
 * @routine 09DE:0B1 StartStageListsH
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9EA8 and $9EA9.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsH(void)
{
    return start_stage_lists(0x9EA8, 0x9EA9, ExpandStageListsH);
}

void abi_StartStageListsH(void)
{
    start_exit_shim(StartStageListsH());
}

/*
 * @routine 0A8E:0B1 StartStageListsI
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9C9C and $9CA4.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsI(void)
{
    return start_stage_lists(0x9C9C, 0x9CA4, ExpandStageListsI);
}

void abi_StartStageListsI(void)
{
    start_exit_shim(StartStageListsI());
}

/*
 * @routine 0B3E:0B1 StartStageListsJ
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9E73 and $9E74.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsJ(void)
{
    return start_stage_lists(0x9E73, 0x9E74, ExpandStageListsJ);
}

void abi_StartStageListsJ(void)
{
    start_exit_shim(StartStageListsJ());
}

/*
 * @routine 0BEE:0B1 StartStageListsK
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9CBB and $9CBE.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsK(void)
{
    return start_stage_lists(0x9CBB, 0x9CBE, ExpandStageListsK);
}

void abi_StartStageListsK(void)
{
    start_exit_shim(StartStageListsK());
}

/*
 * @routine 0C9E:0B1 StartStageListsL
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9E76 and $9E77.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsL(void)
{
    return start_stage_lists(0x9E76, 0x9E77, ExpandStageListsL);
}

void abi_StartStageListsL(void)
{
    start_exit_shim(StartStageListsL());
}

/*
 * @routine 0D4E:0B1 StartStageListsM
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9CDB and $9CDF.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsM(void)
{
    return start_stage_lists(0x9CDB, 0x9CDF, ExpandStageListsM);
}

void abi_StartStageListsM(void)
{
    start_exit_shim(StartStageListsM());
}

/*
 * @routine 0DFE:0B1 StartStageListsN
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9EA4 and $9EA5.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsN(void)
{
    return start_stage_lists(0x9EA4, 0x9EA5, ExpandStageListsN);
}

void abi_StartStageListsN(void)
{
    start_exit_shim(StartStageListsN());
}

/*
 * @routine 0EAE:0B1 StartStageListsO
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9DEA and $9DEE.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsO(void)
{
    return start_stage_lists(0x9DEA, 0x9DEE, ExpandStageListsO);
}

void abi_StartStageListsO(void)
{
    start_exit_shim(StartStageListsO());
}

/*
 * @routine 0F5E:0B1 StartStageListsP
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9F19 and $9F1A.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsP(void)
{
    return start_stage_lists(0x9F19, 0x9F1A, ExpandStageListsP);
}

void abi_StartStageListsP(void)
{
    start_exit_shim(StartStageListsP());
}

/*
 * @routine 100E:0B1 StartStageListsQ
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9C6C and $9C6F.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsQ(void)
{
    return start_stage_lists(0x9C6C, 0x9C6F, ExpandStageListsQ);
}

void abi_StartStageListsQ(void)
{
    start_exit_shim(StartStageListsQ());
}

/*
 * @routine 10BE:0B1 StartStageListsR
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9F03 and $9F04.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsR(void)
{
    return start_stage_lists(0x9F03, 0x9F04, ExpandStageListsR);
}

void abi_StartStageListsR(void)
{
    start_exit_shim(StartStageListsR());
}

/*
 * @routine 116E:0B1 StartStageListsS
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9C65 and $9C6B.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsS(void)
{
    return start_stage_lists(0x9C65, 0x9C6B, ExpandStageListsS);
}

void abi_StartStageListsS(void)
{
    start_exit_shim(StartStageListsS());
}

/*
 * @routine 12CE:0B1 StartStageListsT
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9F27 and $9F2F.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsT(void)
{
    return start_stage_lists(0x9F27, 0x9F2F, ExpandStageListsT);
}

void abi_StartStageListsT(void)
{
    start_exit_shim(StartStageListsT());
}

/*
 * @routine 160E:0B1 StartStageListsU
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9C9C and $9C9F.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsU(void)
{
    return start_stage_lists(0x9C9C, 0x9C9F, ExpandStageListsU);
}

void abi_StartStageListsU(void)
{
    start_exit_shim(StartStageListsU());
}

/*
 * @routine 16BE:0B1 StartStageListsV
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9F3F and $9F40.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsV(void)
{
    return start_stage_lists(0x9F3F, 0x9F40, ExpandStageListsD);
}

void abi_StartStageListsV(void)
{
    start_exit_shim(StartStageListsV());
}

/*
 * @routine 18CE:0B1 StartStageListsW
 * @run 6B:88B1
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
 * lists built into the list buffer, and the section's script in script slot 6.
 * HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
 * names for zField82, builds the section's lists, then starts slot 6 with the script from the
 * overlay's second table at twice zField82 plus the character.
 * The same code as StartStageLists; this overlay's tables are at $9EA0 and $9EA1.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
 * index add.
 */
StageStartEnd StartStageListsW(void)
{
    return start_stage_lists(0x9EA0, 0x9EA1, ExpandStageListsV);
}

void abi_StartStageListsW(void)
{
    start_exit_shim(StartStageListsW());
}
