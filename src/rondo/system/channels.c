/*
 * Starting the overlay's two sets of per-frame channels. Code in the stage overlay
 * runs them: 8 list channels at $3C80 (a kind, a list, a position, a hold timer;
 * lists are hold and item pairs, $FF loops, $FE ends) and 12 script slots at $3C00
 * (a kind, a script, and six more fields).
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

enum { LIST_CHANNEL_BYTES = 0x7F, SCRIPT_SLOT_BUSY_CHECK = 3, KIND_SOLE_SCRIPT = 5 };

/*
 * @routine 0045:506 ClearListChannels
 * @run 69:5506
 * WHAT IT DOES: Stops every list channel and everything kept after them.
 * HOW: Block-fills $3C80 up with zero.
 * Entry: nothing. Exit: all registers and flags kept.
 */
void ClearListChannels(void)
{
    tai(KernelZeros, ListChannelKind, LIST_CHANNEL_BYTES);
}

void abi_ClearListChannels(void)
{
    ClearListChannels();
}

/* Sets a channel going from the start of its list. */
static void start_list_channel(uint8_t kind, uint8_t channel)
{
    wr8((uint16_t)(ListChannelKind + channel), kind);
    wr8((uint16_t)(ListChannelPlace + channel), 0);
    wr8((uint16_t)(ListChannelHold + channel), 0);
}

static void abi_exit_list_channel(uint8_t kind)
{
    cpu.a = kind;
    abi_nz(kind);
}

/*
 * @routine 0045:50E StartListChannel0Kind1
 * @run 69:550E
 * WHAT IT DOES: Starts list channel 0 as kind 1.
 * HOW: Sets X to 0 and runs into StartListChannelKind1.
 * Entry: A list. Exit: A 1, N 0, Z 0, X 0. Y kept.
 */
void abi_StartListChannel0Kind1(void)
{
    cpu.x = 0;
    wr8(ListChannelList, cpu.a);
    start_list_channel(1, 0);
    abi_exit_list_channel(1);
}

/*
 * @routine 0045:50F StartListChannelKind1
 * @run 69:550F
 * WHAT IT DOES: Starts a list channel as kind 1.
 * HOW: Stores the list, then the kind, and clears its place and hold.
 * Entry: A list, X channel. Exit: A 1, N 0, Z 0. X and Y kept.
 */
void StartListChannelKind1(uint8_t list, uint8_t channel)
{
    wr8((uint16_t)(ListChannelList + channel), list);
    start_list_channel(1, channel);
}

void abi_StartListChannelKind1(void)
{
    StartListChannelKind1(cpu.a, cpu.x);
    abi_exit_list_channel(1);
}

/*
 * @routine 0045:51E StartListChannel0Kind2
 * @run 69:551E
 * WHAT IT DOES: Starts list channel 0 as kind 2.
 * HOW: Sets X to 0 and runs into StartListChannelKind2.
 * Entry: A list. Exit: A 2, N 0, Z 0, X 0. Y kept.
 */
void abi_StartListChannel0Kind2(void)
{
    cpu.x = 0;
    wr8(ListChannelList, cpu.a);
    start_list_channel(2, 0);
    abi_exit_list_channel(2);
}

/*
 * @routine 0045:51F StartListChannelKind2
 * @run 69:551F
 * WHAT IT DOES: Starts a list channel as kind 2.
 * HOW: Stores the list, then the kind, and clears its place and hold.
 * Entry: A list, X channel. Exit: A 2, N 0, Z 0. X and Y kept.
 */
void StartListChannelKind2(uint8_t list, uint8_t channel)
{
    wr8((uint16_t)(ListChannelList + channel), list);
    start_list_channel(2, channel);
}

void abi_StartListChannelKind2(void)
{
    StartListChannelKind2(cpu.a, cpu.x);
    abi_exit_list_channel(2);
}

/*
 * @routine 0045:526 StartListChannelKind3
 * @run 69:5526
 * WHAT IT DOES: Starts a list channel as kind 3.
 * HOW: Stores the list, then the kind, and clears its place and hold.
 * Entry: A list, X channel. Exit: A 3, N 0, Z 0. X and Y kept.
 */
void StartListChannelKind3(uint8_t list, uint8_t channel)
{
    wr8((uint16_t)(ListChannelList + channel), list);
    start_list_channel(3, channel);
}

void abi_StartListChannelKind3(void)
{
    StartListChannelKind3(cpu.a, cpu.x);
    abi_exit_list_channel(3);
}

/*
 * @routine 0045:56E ClearScriptSlot
 * @run 69:556E
 * WHAT IT DOES: Empties a script slot.
 * HOW: Zeroes its first eight fields, 12 bytes apart.
 * Entry: X slot. Exit: all registers and flags kept.
 */
void ClearScriptSlot(uint8_t slot)
{
    for (uint16_t field = ScriptSlotKind; field <= ScriptSlotKind + 0x54; field += 0x0C)
        wr8((uint16_t)(field + slot), 0);
}

void abi_ClearScriptSlot(void)
{
    ClearScriptSlot(cpu.x);
}

/* Empties a slot and starts it. The assembly keeps the script in Y across this. */
static void start_script_slot(uint8_t kind, uint8_t script, uint8_t slot)
{
    ClearScriptSlot(slot);
    wr8((uint16_t)(ScriptSlotKind + slot), kind);
    wr8((uint16_t)(ScriptSlotScript + slot), script);
}

static void abi_exit_script_slot(uint8_t script)
{
    cpu.a = script;
    cpu.y = script;
    abi_nz(script);
}

/*
 * @routine 0045:52D StartScriptSlot0Kind1
 * @run 69:552D
 * WHAT IT DOES: Starts script slot 0 as kind 1.
 * HOW: Sets X to 0 and runs into StartScriptSlotKind1.
 * Entry: A script. Exit: A and Y the script, X 0, N and Z from it.
 */
void abi_StartScriptSlot0Kind1(void)
{
    cpu.x = 0;
    start_script_slot(1, cpu.a, 0);
    abi_exit_script_slot(cpu.a);
}

/*
 * @routine 0045:52E StartScriptSlotKind1
 * @run 69:552E
 * WHAT IT DOES: Starts a script slot as kind 1.
 * HOW: Empties it, stores the kind and the script.
 * Entry: A script, X slot. Exit: A and Y the script, N and Z from it. X kept.
 */
void StartScriptSlotKind1(uint8_t script, uint8_t slot)
{
    start_script_slot(1, script, slot);
}

void abi_StartScriptSlotKind1(void)
{
    StartScriptSlotKind1(cpu.a, cpu.x);
    abi_exit_script_slot(cpu.a);
}

/*
 * @routine 0045:53B StartScriptSlot0Kind2
 * @run 69:553B
 * WHAT IT DOES: Starts script slot 0 as kind 2.
 * HOW: Sets X to 0 and runs into StartScriptSlotKind2.
 * Entry: A script. Exit: A and Y the script, X 0, N and Z from it.
 */
void abi_StartScriptSlot0Kind2(void)
{
    cpu.x = 0;
    start_script_slot(2, cpu.a, 0);
    abi_exit_script_slot(cpu.a);
}

/*
 * @routine 0045:53C StartScriptSlotKind2
 * @run 69:553C
 * WHAT IT DOES: Starts a script slot as kind 2.
 * HOW: Empties it, stores the kind and the script.
 * Entry: A script, X slot. Exit: A and Y the script, N and Z from it. X kept.
 */
void abi_StartScriptSlotKind2(void)
{
    start_script_slot(2, cpu.a, cpu.x);
    abi_exit_script_slot(cpu.a);
}

/*
 * @routine 0045:543 StartScriptSlot0Kind3
 * @run 69:5543
 * WHAT IT DOES: Starts script slot 0 as kind 3.
 * HOW: Sets X to 0 and runs into StartScriptSlotKind3.
 * Entry: A script. Exit: A and Y the script, X 0, N and Z from it.
 */
void abi_StartScriptSlot0Kind3(void)
{
    cpu.x = 0;
    start_script_slot(3, cpu.a, 0);
    abi_exit_script_slot(cpu.a);
}

/*
 * @routine 0045:544 StartScriptSlotKind3
 * @run 69:5544
 * WHAT IT DOES: Starts a script slot as kind 3.
 * HOW: Empties it, stores the kind and the script.
 * Entry: A script, X slot. Exit: A and Y the script, N and Z from it. X kept.
 */
void abi_StartScriptSlotKind3(void)
{
    start_script_slot(3, cpu.a, cpu.x);
    abi_exit_script_slot(cpu.a);
}

/*
 * @routine 0045:54B StartScriptSlotKind4
 * @run 69:554B
 * WHAT IT DOES: Starts a script slot as kind 4.
 * HOW: Empties it, stores the kind and the script.
 * Entry: A script, X slot. Exit: A and Y the script, N and Z from it. X kept.
 */
void abi_StartScriptSlotKind4(void)
{
    start_script_slot(4, cpu.a, cpu.x);
    abi_exit_script_slot(cpu.a);
}

/*
 * @routine 0045:552 StartSoleScriptSlot
 * @run 69:5552
 * WHAT IT DOES: Starts a script slot as kind 5, unless slot 3 is in use: says which.
 * HOW: Slot 3's kind nonzero refuses. Otherwise empties the slot and starts it.
 * Entry: A script, X slot. Exit, started: A the script, N and Z from it, C 1.
 * Refused: A slot 3's kind, N and Z from it, C 0. Y the script, X kept, either way.
 */
bool StartSoleScriptSlot(uint8_t script, uint8_t slot, uint8_t *busy_kind)
{
    *busy_kind = rd8(ScriptSlotKind + SCRIPT_SLOT_BUSY_CHECK);
    if (*busy_kind != 0)
        return false;
    start_script_slot(KIND_SOLE_SCRIPT, script, slot);
    return true;
}

void abi_StartSoleScriptSlot(void)
{
    uint8_t script = cpu.a;
    uint8_t busy_kind;
    bool started = StartSoleScriptSlot(script, cpu.x, &busy_kind);

    cpu.y = script;
    cpu.a = started ? script : busy_kind;
    abi_nz(cpu.a);
    abi_flag(P_C, started);
}

/*
 * @routine 0045:562 StartScriptSlotKind6
 * @run 69:5562
 * WHAT IT DOES: Starts a script slot as kind 6, keeping Y.
 * HOW: Empties it, then the kind and, from what it finds in Y, the script.
 * Entry: X slot, Y script. Exit: A and Y the script, N and Z from it. X kept.
 */
void abi_StartScriptSlotKind6(void)
{
    start_script_slot(6, cpu.y, cpu.x);
    abi_exit_script_slot(cpu.y);
}

/*
 * @routine 0045:568 StartScriptSlotKind7
 * @run 69:5568
 * WHAT IT DOES: Starts a script slot as kind 7, keeping Y.
 * HOW: Empties it, then the kind and, from what it finds in Y, the script.
 * Entry: X slot, Y script. Exit: A and Y the script, N and Z from it. X kept.
 */
void abi_StartScriptSlotKind7(void)
{
    start_script_slot(7, cpu.y, cpu.x);
    abi_exit_script_slot(cpu.y);
}
