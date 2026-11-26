/*
 * Starting the overlay's two sets of per-frame channels. Code in the stage overlay
 * runs them: 8 list channels at $3C80 (a kind, a list, a position, a hold timer;
 * lists are hold and item pairs, $FF loops, $FE ends) and 12 script slots at $3C00
 * (a kind, a script, and six more fields).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"

enum { LIST_CHANNEL_BYTES = 0x7F, SCRIPT_SLOT_BUSY_CHECK = 3, KIND_SOLE_SCRIPT = 5 };

/* The exit of every list channel start: A the kind, N and Z from it. */
void list_channel_exit_shim(uint8_t kind)
{
    cpu.a = kind;
    abi_nz(kind);
}

/* The exit of every script slot start: A and Y the script, N and Z from it. */
void script_slot_exit_shim(uint8_t script)
{
    cpu.a = script;
    cpu.y = script;
    abi_nz(script);
}

/*
 * @routine 0045:506 ClearListChannels
 * @run 69:5506
 * WHAT IT DOES: Stops every list channel and everything kept after them.
 * HOW: Block-fills $3C80 up with zero.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 */
void ClearListChannels(void)
{
    tai(KernelZeros, ADDR_OF(LCH->kind[0]), LIST_CHANNEL_BYTES);
}

void abi_ClearListChannels(void)
{
    ClearListChannels();
}

/* The end the list channel starts share: the kind, then the place and the hold cleared. */
static uint8_t list_channel_kind(uint8_t kind, uint8_t channel)
{
    WR8(LCH->kind[channel], kind);
    WR8(LCH->place[channel], 0);
    WR8(LCH->hold[channel], 0);
    return kind;
}

/*
 * @routine 0045:50E StartListChannel0Kind1
 * @run 69:550E
 * WHAT IT DOES: Starts list channel 0 as kind 1.
 * HOW: Sets X to 0 and runs into StartListChannelKind1.
 * WAITS: no.
 * Entry: A list. Exit: A 1, N 0, Z 0, X 0. Y kept.
 */
uint8_t StartListChannel0Kind1(uint8_t list)
{
    return StartListChannelKind1(list, 0);
}

void abi_StartListChannel0Kind1(void)
{
    list_channel_exit_shim(StartListChannel0Kind1(cpu.a));
    cpu.x = 0;
}

/*
 * @routine 0045:50F StartListChannelKind1
 * @run 69:550F
 * WHAT IT DOES: Starts a list channel as kind 1.
 * HOW: Stores the list, then the kind, and clears its place and hold.
 * WAITS: no.
 * Entry: A list, X channel. Exit: A 1, N 0, Z 0. X and Y kept.
 */
uint8_t StartListChannelKind1(uint8_t list, uint8_t channel)
{
    WR8(LCH->list[channel], list);
    return list_channel_kind(1, channel);
}

void abi_StartListChannelKind1(void)
{
    list_channel_exit_shim(StartListChannelKind1(cpu.a, cpu.x));
}

/*
 * @routine 0045:51E StartListChannel0Kind2
 * @run 69:551E
 * WHAT IT DOES: Starts list channel 0 as kind 2.
 * HOW: Sets X to 0 and runs into StartListChannelKind2.
 * WAITS: no.
 * Entry: A list. Exit: A 2, N 0, Z 0, X 0. Y kept.
 */
uint8_t StartListChannel0Kind2(uint8_t list)
{
    return StartListChannelKind2(list, 0);
}

void abi_StartListChannel0Kind2(void)
{
    list_channel_exit_shim(StartListChannel0Kind2(cpu.a));
    cpu.x = 0;
}

/*
 * @routine 0045:51F StartListChannelKind2
 * @run 69:551F
 * WHAT IT DOES: Starts a list channel as kind 2.
 * HOW: Stores the list, then the kind, and clears its place and hold.
 * WAITS: no.
 * Entry: A list, X channel. Exit: A 2, N 0, Z 0. X and Y kept.
 */
uint8_t StartListChannelKind2(uint8_t list, uint8_t channel)
{
    WR8(LCH->list[channel], list);
    return list_channel_kind(2, channel);
}

void abi_StartListChannelKind2(void)
{
    list_channel_exit_shim(StartListChannelKind2(cpu.a, cpu.x));
}

/*
 * @routine 0045:526 StartListChannelKind3
 * @run 69:5526
 * WHAT IT DOES: Starts a list channel as kind 3.
 * HOW: Stores the list, then the kind, and clears its place and hold.
 * WAITS: no.
 * Entry: A list, X channel. Exit: A 3, N 0, Z 0. X and Y kept.
 */
uint8_t StartListChannelKind3(uint8_t list, uint8_t channel)
{
    WR8(LCH->list[channel], list);
    return list_channel_kind(3, channel);
}

void abi_StartListChannelKind3(void)
{
    list_channel_exit_shim(StartListChannelKind3(cpu.a, cpu.x));
}

/*
 * @routine 0045:56E ClearScriptSlot
 * @run 69:556E
 * WHAT IT DOES: Empties a script slot.
 * HOW: Zeroes its first eight fields, 12 bytes apart.
 * WAITS: no.
 * Entry: X slot. Exit: all registers and flags kept.
 */
void ClearScriptSlot(uint8_t slot)
{
    WR8(SLOTS->kind[slot], 0);
    WR8(SLOTS->script[slot], 0);
    WR8(SLOTS->f18[slot], 0);
    WR8(SLOTS->f24[slot], 0);
    WR8(SLOTS->timer[slot], 0);
    WR8(SLOTS->f3c[slot], 0);
    WR8(SLOTS->f48[slot], 0);
    WR8(SLOTS->f54[slot], 0);
}

void abi_ClearScriptSlot(void)
{
    ClearScriptSlot(cpu.x);
}

/* The end the script slot starts share: the kind, and the script that Y kept. */
static void script_slot_kind(uint8_t kind, uint8_t script, uint8_t slot)
{
    WR8(SLOTS->kind[slot], kind);
    WR8(SLOTS->script[slot], script);
}

/*
 * @routine 0045:52D StartScriptSlot0Kind1
 * @run 69:552D
 * WHAT IT DOES: Starts script slot 0 as kind 1.
 * HOW: Sets X to 0 and runs into StartScriptSlotKind1.
 * WAITS: no.
 * Entry: A script. Exit: A and Y the script, X 0, N and Z from it.
 */
uint8_t StartScriptSlot0Kind1(uint8_t script)
{
    return StartScriptSlotKind1(script, 0);
}

void abi_StartScriptSlot0Kind1(void)
{
    script_slot_exit_shim(StartScriptSlot0Kind1(cpu.a));
    cpu.x = 0;
}

/*
 * @routine 0045:52E StartScriptSlotKind1
 * @run 69:552E
 * WHAT IT DOES: Starts a script slot as kind 1.
 * HOW: Keeps the script in Y, empties the slot, stores the kind and the script.
 * WAITS: no.
 * Entry: A script, X slot. Exit: A and Y the script, N and Z from it. X kept.
 */
uint8_t StartScriptSlotKind1(uint8_t script, uint8_t slot)
{
    jsr_push(0x5531);
    ClearScriptSlot(slot);
    jsr_pop();
    script_slot_kind(1, script, slot);
    return script;
}

void abi_StartScriptSlotKind1(void)
{
    script_slot_exit_shim(StartScriptSlotKind1(cpu.a, cpu.x));
}

/*
 * @routine 0045:53B StartScriptSlot0Kind2
 * @run 69:553B
 * WHAT IT DOES: Starts script slot 0 as kind 2.
 * HOW: Sets X to 0 and runs into StartScriptSlotKind2.
 * WAITS: no.
 * Entry: A script. Exit: A and Y the script, X 0, N and Z from it.
 */
uint8_t StartScriptSlot0Kind2(uint8_t script)
{
    return StartScriptSlotKind2(script, 0);
}

void abi_StartScriptSlot0Kind2(void)
{
    script_slot_exit_shim(StartScriptSlot0Kind2(cpu.a));
    cpu.x = 0;
}

/*
 * @routine 0045:53C StartScriptSlotKind2
 * @run 69:553C
 * WHAT IT DOES: Starts a script slot as kind 2.
 * HOW: Keeps the script in Y, empties the slot, stores the kind and the script.
 * WAITS: no.
 * Entry: A script, X slot. Exit: A and Y the script, N and Z from it. X kept.
 */
uint8_t StartScriptSlotKind2(uint8_t script, uint8_t slot)
{
    jsr_push(0x553F);
    ClearScriptSlot(slot);
    jsr_pop();
    script_slot_kind(2, script, slot);
    return script;
}

void abi_StartScriptSlotKind2(void)
{
    script_slot_exit_shim(StartScriptSlotKind2(cpu.a, cpu.x));
}

/*
 * @routine 0045:543 StartScriptSlot0Kind3
 * @run 69:5543
 * WHAT IT DOES: Starts script slot 0 as kind 3.
 * HOW: Sets X to 0 and runs into StartScriptSlotKind3.
 * WAITS: no.
 * Entry: A script. Exit: A and Y the script, X 0, N and Z from it.
 */
uint8_t StartScriptSlot0Kind3(uint8_t script)
{
    return StartScriptSlotKind3(script, 0);
}

void abi_StartScriptSlot0Kind3(void)
{
    script_slot_exit_shim(StartScriptSlot0Kind3(cpu.a));
    cpu.x = 0;
}

/*
 * @routine 0045:544 StartScriptSlotKind3
 * @run 69:5544
 * WHAT IT DOES: Starts a script slot as kind 3.
 * HOW: Keeps the script in Y, empties the slot, stores the kind and the script.
 * WAITS: no.
 * Entry: A script, X slot. Exit: A and Y the script, N and Z from it. X kept.
 */
uint8_t StartScriptSlotKind3(uint8_t script, uint8_t slot)
{
    jsr_push(0x5547);
    ClearScriptSlot(slot);
    jsr_pop();
    script_slot_kind(3, script, slot);
    return script;
}

void abi_StartScriptSlotKind3(void)
{
    script_slot_exit_shim(StartScriptSlotKind3(cpu.a, cpu.x));
}

/*
 * @routine 0045:54B StartScriptSlotKind4
 * @run 69:554B
 * WHAT IT DOES: Starts a script slot as kind 4.
 * HOW: Keeps the script in Y, empties the slot, stores the kind and the script.
 * WAITS: no.
 * Entry: A script, X slot. Exit: A and Y the script, N and Z from it. X kept.
 */
uint8_t StartScriptSlotKind4(uint8_t script, uint8_t slot)
{
    jsr_push(0x554E);
    ClearScriptSlot(slot);
    jsr_pop();
    script_slot_kind(4, script, slot);
    return script;
}

void abi_StartScriptSlotKind4(void)
{
    script_slot_exit_shim(StartScriptSlotKind4(cpu.a, cpu.x));
}

/*
 * @routine 0045:533 SetScriptSlotKindAndScript
 * @run 69:5533
 * WHAT IT DOES: Stores a script slot's kind and its script: the end that StartScriptSlotKind1 runs into and that the sole slot
 * start calls.
 * HOW: Two stores.
 * WAITS: no.
 * Entry: A kind, Y script, X slot. Exit: A the script, N and Z from it. X and Y kept.
 */
uint8_t SetScriptSlotKindAndScript(uint8_t kind, uint8_t script, uint8_t slot)
{
    script_slot_kind(kind, script, slot);
    return script;
}

void abi_SetScriptSlotKindAndScript(void)
{
    script_slot_exit_shim(SetScriptSlotKindAndScript(cpu.a, cpu.y, cpu.x));
}

/*
 * @routine 0045:552 StartSoleScriptSlot
 * @run 69:5552
 * WHAT IT DOES: Starts a script slot as kind 5, unless slot 3 is in use: says which.
 * HOW: Keeps the script in Y. Slot 3's kind nonzero refuses. Otherwise empties the slot and
 * calls the end that stores the kind and the script (the draft stubs that call, since the end is
 * the middle of StartScriptSlotKind1 and has no routine of its own, so liftcmp cannot compare
 * past it: its first difference is the draft's CALL to $5533 against the end's two writes).
 * WAITS: no.
 * Entry: A script, X slot. Exit, started: A the script, N and Z from it, C 1.
 * Refused: A slot 3's kind, N and Z from it, C 0. Y the script, X kept, either way.
 */
SoleSlot StartSoleScriptSlot(uint8_t script, uint8_t slot)
{
    SoleSlot result;

    result.script = script;
    result.busy_kind = RD8(SLOTS->kind[SCRIPT_SLOT_BUSY_CHECK]);
    result.started = result.busy_kind == 0;
    if (!result.started) {
        return result;
    }
    jsr_push(0x555A);
    ClearScriptSlot(slot);
    jsr_pop();
    jsr_push(0x555E);
    (void)SetScriptSlotKindAndScript(KIND_SOLE_SCRIPT, script, slot);
    jsr_pop();
    return result;
}

void sole_slot_exit_shim(SoleSlot result)
{
    cpu.y = result.script;
    cpu.a = result.started ? result.script : result.busy_kind;
    abi_nz(cpu.a);
    abi_flag(P_C, result.started);
}

void abi_StartSoleScriptSlot(void)
{
    sole_slot_exit_shim(StartSoleScriptSlot(cpu.a, cpu.x));
}

/*
 * @routine 0045:562 StartScriptSlotKind6
 * @run 69:5562
 * WHAT IT DOES: Starts a script slot as kind 6, keeping Y.
 * HOW: Empties it, then the kind and, from what it finds in Y, the script.
 * WAITS: no.
 * Entry: X slot, Y script. Exit: A and Y the script, N and Z from it. X kept.
 */
uint8_t StartScriptSlotKind6(uint8_t script, uint8_t slot)
{
    jsr_push(0x5564);
    ClearScriptSlot(slot);
    jsr_pop();
    script_slot_kind(6, script, slot);
    return script;
}

void abi_StartScriptSlotKind6(void)
{
    script_slot_exit_shim(StartScriptSlotKind6(cpu.y, cpu.x));
}

/*
 * @routine 0045:568 StartScriptSlotKind7
 * @run 69:5568
 * WHAT IT DOES: Starts a script slot as kind 7, keeping Y.
 * HOW: Empties it, then the kind and, from what it finds in Y, the script.
 * WAITS: no.
 * Entry: X slot, Y script. Exit: A and Y the script, N and Z from it. X kept.
 */
uint8_t StartScriptSlotKind7(uint8_t script, uint8_t slot)
{
    jsr_push(0x556A);
    ClearScriptSlot(slot);
    jsr_pop();
    script_slot_kind(7, script, slot);
    return script;
}

void abi_StartScriptSlotKind7(void)
{
    script_slot_exit_shim(StartScriptSlotKind7(cpu.y, cpu.x));
}
