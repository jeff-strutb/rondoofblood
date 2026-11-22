/*
 * The attract mode's scene: the state handlers the attract mode runs one a frame (zGameSubMode
 * is the state), and the scene script's own helpers. The scene's frame counter is the word
 * zWaitTimer3 (low) and zWaitTimer4 (high).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"

enum { OBJECT_SLOTS = 0x58 };

/*
 * @routine 00C1:7CC AttractFrameTick
 * @run 6B:87CC
 * WHAT IT DOES: Counts one frame of the attract scene on its 16-bit frame counter.
 * HOW: Adds one to the low byte, and to the high byte when the low byte wrapped to zero.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the byte last incremented (the low byte, or the high byte
 * when the low one wrapped), C and the registers kept.
 */
uint8_t AttractFrameTick(void)
{
    uint8_t low;

    low = INC8(ZP->wait_timer3);
    if (low != 0) {
        return low;
    }
    return INC8(ZP->wait_timer4);
}

void abi_AttractFrameTick(void)
{
    abi_nz(AttractFrameTick());
}

/*
 * @routine 00C1:7D3 AttractEndCountdown
 * @run 6B:87D3
 * WHAT IT DOES: The attract scene's last state: counts the frame counter down, and when it
 * runs out wipes the game state and takes the scene's done mark back off.
 * HOW: Subtracts one from the low byte of the frame counter and returns while it is not zero.
 * At zero it calls ResetGameState and subtracts one from zModeDone.
 * WAITS: yes, through ResetGameState, which waits for the vertical blank with the display off;
 * the stack there is this routine's return address and the call's. The comparison with the draft
 * shows its first difference inside ResetGameState (its C does not yet place the frame of its
 * own call to the vblank wait), not in this routine's own accesses: its own are the frame, the
 * call and a decrement of zModeDone, and agree up to that point.
 * Entry: nothing. Exit: N and Z from the byte last decremented, C and the registers as
 * ResetGameState leaves them (kept when the count has not run out).
 */
AttractEnd AttractEndCountdown(void)
{
    AttractEnd end;

    end.value = DEC8(ZP->wait_timer3);
    end.reset = false;
    if (end.value != 0) {
        return end;
    }
    jsr_push(0x87DA);
    ResetGameState();
    jsr_pop();
    end.value = DEC8(ZP->mode_done);
    end.reset = true;
    return end;
}

void abi_AttractEndCountdown(void)
{
    AttractEnd end = AttractEndCountdown();

    if (end.reset) {
        reset_game_state_exit_shim();
    }
    abi_nz(end.value);
}

/*
 * @routine 00C2:0F7 ClearAllObjects
 * @run 6B:88F7
 * WHAT IT DOES: Empties all 88 object slots, the scene script's way of clearing the scene.
 * HOW: A loop over the slots from 0 up, each through ClearObject.
 * WAITS: no.
 * Entry: nothing. Exit: X $58, C and Z set, N clear (the loop's last compare); A and Y kept.
 */
void ClearAllObjects(void)
{
    uint8_t slot;

    for (slot = 0; slot != OBJECT_SLOTS; slot++) {
        jsr_push(0x88FB);
        ClearObject(slot);
        jsr_pop();
    }
}

void abi_ClearAllObjects(void)
{
    ClearAllObjects();
    cpu.x = OBJECT_SLOTS;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 00C1:71A AttractBegin
 * @run 6B:871A
 * WHAT IT DOES: Starts the attract scene: the game state is wiped, the scene's script is set to
 * run from $8F98, its frame counter to $FFFF (so that the first tick brings it to 0) and the
 * mode goes on to its next state.
 * HOW: ResetGameState; the script pointer (the two bytes at $7E) and the frame counter are
 * written; zGameSubMode goes up by one.
 * WAITS: yes, through ResetGameState, which waits for the vertical blank with the display off;
 * the stack there is this routine's return address and the call's.
 * Entry: nothing. Exit: A $FF, X and Y as ResetGameState leaves them, N and Z from the new state,
 * C as ResetGameState leaves it. The comparison with the draft shows its first difference
 * inside ResetGameState (its C does not yet place the frame of its own call to the vblank wait);
 * this routine's own accesses agree up to that call.
 */
uint8_t AttractBegin(void)
{
    jsr_push(0x871D);
    ResetGameState();
    jsr_pop();
    WR8(ZP->_gap_007C[2], 0x98);
    WR8(ZP->_gap_007C[3], 0x8F);
    WR8(ZP->wait_timer3, 0xFF);
    WR8(ZP->wait_timer4, 0xFF);
    return INC8(ZP->game_sub_mode);
}

void abi_AttractBegin(void)
{
    uint8_t state = AttractBegin();

    reset_game_state_exit_shim();
    cpu.a = 0xFF;
    abi_nz(state);
}

/*
 * @routine 00C1:265 ModeDoneAdd
 * @run 6B:8265
 * WHAT IT DOES: Adds one to zModeDone, the count the mode loop reads to learn that the mode has
 * finished.
 * HOW: One increment.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the new value, C and the registers kept. No script reaches
 * it; it is compared with the draft only.
 */
uint8_t ModeDoneAdd(void)
{
    return INC8(ZP->mode_done);
}

void abi_ModeDoneAdd(void)
{
    abi_nz(ModeDoneAdd());
}

/*
 * The scene script: a list of commands the title, attract and ending scenes run, in a scene's own
 * zero page pointer (the two bytes at $7E; named by address until the names land). Each command is a
 * byte, its length from a table, a time as two bytes (compared with the frame counter zWaitTimer3
 * and zWaitTimer4), and operands. ScriptAdvance steps past a command and ScriptStep runs the next
 * if its time is the counter's; each command ends by going back to ScriptAdvance, so a frame runs
 * every command that is due, and the exit is the last time test that failed.
 */
#define SCRIPT_PTR _gap_007C[2]
#define SCRIPT_PTR_HI _gap_007C[3]

/*
 * @routine 00C2:000 ScriptAdvance
 * @run 6B:8800
 * WHAT IT DOES: Steps the scene script past the command it is at, and runs on into ScriptStep.
 * HOW: The command byte indexes a table of lengths, which is added to the pointer (the high byte
 * goes up on a carry); then it runs into ScriptStep.
 * WAITS: no.
 * Entry: nothing. Exit: as ScriptStep, X the command that was stepped past (when ScriptStep left
 * X alone), V from the pointer's add (when it set none).
 */
Exit ScriptAdvance(void)
{
    Exit exit;
    uint8_t command, length, low, sum_value;
    bool sum_carry, sum_overflow;
    unsigned total;

    command = RD8_PTR(ZP->SCRIPT_PTR);
    length = rd8((uint16_t)(SCRIPT_LENGTHS + command));
    low = RD8(ZP->SCRIPT_PTR);
    total = (unsigned)length + low;
    sum_value = (uint8_t)total;
    sum_carry = total > 0xFF;
    sum_overflow = ((length ^ sum_value) & (low ^ sum_value) & 0x80) != 0;
    WR8(ZP->SCRIPT_PTR, sum_value);
    if (sum_carry) {
        INC8(ZP->SCRIPT_PTR_HI);
    }
    exit = ScriptStep();
    if (exit.x_kept) {
        exit.x = command;
        exit.x_kept = false;
    }
    if (!exit.v_set) {
        exit.v = sum_overflow;
        exit.v_set = true;
    }
    return exit;
}

void abi_ScriptAdvance(void)
{
    exit_shim(ScriptAdvance());
}

/*
 * @routine 00C2:00F ScriptStep
 * @run 6B:880F
 * WHAT IT DOES: Runs the scene script's next command if its time has come: the two time bytes
 * after the command byte equal the scene's frame counter.
 * HOW: Compares the first time byte with zWaitTimer3 and the second with zWaitTimer4, and
 * returns on a difference; otherwise the command, doubled, indexes a table of handlers and it
 * is jumped to with X the doubled command and Y 3.
 * WAITS: no.
 * Entry: nothing. Exit, a time byte differs: A that byte, N, Z and C from comparing it, Y 1 or 2,
 * X kept. Otherwise as the handler leaves them.
 */
/* The registers a command handler outside C is entered with: A and X the doubled command, Y 3. */
static void script_tail_shim(uint8_t index, uint8_t command)
{
    cpu.a = index;
    cpu.x = index;
    cpu.y = 3;
    abi_nz(3);                      /* LDY #3 set N and Z last; C is the doubling's */
    abi_flag(P_C, (command & 0x80) != 0);
}

Exit ScriptStep(void)
{
    Exit exit = { 0 };
    uint8_t time, counter, command, index, value, high;
    uint16_t target;

    time = RD8_PTR_Y(ZP->SCRIPT_PTR, 1);
    counter = RD8(ZP->wait_timer3);
    if (time != counter) {
        exit.a = time;
        exit.y = 1;
        exit.nz = (uint8_t)(time - counter);
        exit.c = time >= counter;
        exit.x_kept = true;
        return exit;
    }
    time = RD8_PTR_Y(ZP->SCRIPT_PTR, 2);
    counter = RD8(ZP->wait_timer4);
    if (time != counter) {
        exit.a = time;
        exit.y = 2;
        exit.nz = (uint8_t)(time - counter);
        exit.c = time >= counter;
        exit.x_kept = true;
        return exit;
    }
    command = RD8_PTR(ZP->SCRIPT_PTR);
    index = (uint8_t)(command << 1);
    value = rd8((uint16_t)(SCRIPT_COMMANDS + index));
    high = rd8((uint16_t)(SCRIPT_COMMANDS + 1 + index));
    target = (uint16_t)(value | (high << 8));
    switch (target) {
    case 0x8859: return ScriptStartListAndClear();
    case 0x886D: return ScriptStartSlotKind1();
    case 0x87DD: return AttractHoldBeforeEnd();
    case 0x8893: return ScriptPlaceObjectState();
    case 0x88BC: return ScriptPlaceObjectFrame();
    case 0x8874: return ScriptStartSlotKind2();
    case 0x887B: return ScriptStartSlotKind3();
    case 0x888B: return ScriptPlayTrack(index);
    case 0x891D: return ScriptPlaceTitleObjectA();
    case 0x8944: return ScriptPlaceTitleObjectB();
    case 0x8847: return ScriptSaveReadAndState();
    case 0x8866: return ScriptStartList();
    case 0x88A2: return ScriptPlaceObjectKind2();
    case 0x8852: return ScriptSound();
    default:
        break;
    }
    script_tail_shim(index, command);
    call_original(target);
    exit.from_cpu = true;
    return exit;
}

void abi_ScriptStep(void)
{
    exit_shim(ScriptStep());
}

/*
 * @routine 00C2:047 ScriptSaveReadAndState
 * @run 6B:8847
 * WHAT IT DOES: A script command: marks that the title has read the save ($3F83 is set to 1) and
 * puts the mode in its state 2.
 * HOW: Two stores, then back to ScriptAdvance.
 * WAITS: no.
 * Entry: from ScriptStep. Exit: as ScriptAdvance.
 */
Exit ScriptSaveReadAndState(void)
{
    wr8(TITLE_SAVE_READ, 1);
    WR8(ZP->game_sub_mode, 2);
    return ScriptAdvance();
}

void abi_ScriptSaveReadAndState(void)
{
    exit_shim(ScriptSaveReadAndState());
}

/*
 * @routine 00C2:052 ScriptSound
 * @run 6B:8852
 * WHAT IT DOES: A script command: gives the sound driver the command's operand.
 * HOW: The byte at Y 3 goes to SoundCommand; back to ScriptAdvance.
 * WAITS: no.
 * Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
 */
Exit ScriptSound(void)
{
    uint8_t sound;

    sound = RD8_PTR_Y(ZP->SCRIPT_PTR, 3);
    jsr_push(0x8857);
    SoundCommand(sound);
    jsr_pop();
    return ScriptAdvance();
}

void abi_ScriptSound(void)
{
    exit_shim(ScriptSound());
}

/*
 * @routine 00C2:059 ScriptStartListAndClear
 * @run 6B:8859
 * WHAT IT DOES: A script command: starts list channel 0 with the operand's list, clears the
 * screen's state and all the object slots.
 * HOW: StartListChannel0Kind2, ClearScreenState, ClearAllObjects; back to ScriptAdvance.
 * WAITS: no.
 * Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
 */
Exit ScriptStartListAndClear(void)
{
    uint8_t list;

    list = RD8_PTR_Y(ZP->SCRIPT_PTR, 3);
    jsr_push(0x885E);
    StartListChannel0Kind2(list);
    jsr_pop();
    jsr_push(0x8861);
    ClearScreenState();
    jsr_pop();
    jsr_push(0x8864);
    ClearAllObjects();
    jsr_pop();
    return ScriptAdvance();
}

void abi_ScriptStartListAndClear(void)
{
    exit_shim(ScriptStartListAndClear());
}

/*
 * @routine 00C2:066 ScriptStartList
 * @run 6B:8866
 * WHAT IT DOES: A script command: starts list channel 0 with the operand's list.
 * HOW: StartListChannel0Kind2; back to ScriptAdvance.
 * WAITS: no.
 * Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
 */
Exit ScriptStartList(void)
{
    uint8_t list;

    list = RD8_PTR_Y(ZP->SCRIPT_PTR, 3);
    jsr_push(0x886B);
    StartListChannel0Kind2(list);
    jsr_pop();
    return ScriptAdvance();
}

void abi_ScriptStartList(void)
{
    exit_shim(ScriptStartList());
}

/*
 * @routine 00C2:06D ScriptStartSlotKind1
 * @run 6B:886D
 * WHAT IT DOES: A script command: starts script slot 0 as kind 1 with the operand's script.
 * HOW: StartScriptSlot0Kind1; back to ScriptAdvance.
 * WAITS: no.
 * Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
 */
Exit ScriptStartSlotKind1(void)
{
    uint8_t script;

    script = RD8_PTR_Y(ZP->SCRIPT_PTR, 3);
    jsr_push(0x8872);
    StartScriptSlot0Kind1(script);
    jsr_pop();
    return ScriptAdvance();
}

void abi_ScriptStartSlotKind1(void)
{
    exit_shim(ScriptStartSlotKind1());
}

/*
 * @routine 00C2:074 ScriptStartSlotKind2
 * @run 6B:8874
 * WHAT IT DOES: A script command: starts script slot 0 as kind 2 with the operand's script.
 * HOW: StartScriptSlot0Kind2; back to ScriptAdvance.
 * WAITS: no.
 * Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
 */
Exit ScriptStartSlotKind2(void)
{
    uint8_t script;

    script = RD8_PTR_Y(ZP->SCRIPT_PTR, 3);
    jsr_push(0x8879);
    StartScriptSlot0Kind2(script);
    jsr_pop();
    return ScriptAdvance();
}

void abi_ScriptStartSlotKind2(void)
{
    exit_shim(ScriptStartSlotKind2());
}

/*
 * @routine 00C2:07B ScriptStartSlotKind3
 * @run 6B:887B
 * WHAT IT DOES: A script command: starts script slot 0 as kind 3 with the operand's script.
 * HOW: StartScriptSlot0Kind3; back to ScriptAdvance.
 * WAITS: no.
 * Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
 */
Exit ScriptStartSlotKind3(void)
{
    uint8_t script;

    script = RD8_PTR_Y(ZP->SCRIPT_PTR, 3);
    jsr_push(0x8880);
    StartScriptSlot0Kind3(script);
    jsr_pop();
    return ScriptAdvance();
}

void abi_ScriptStartSlotKind3(void)
{
    exit_shim(ScriptStartSlotKind3());
}

/*
 * @routine 00C2:08B ScriptPlayTrack
 * @run 6B:888B
 * WHAT IT DOES: A script command: plays the operand's CD track once.
 * HOW: CdPlayOnce, with X the doubled command and Y 3 as ScriptStep enters it, which it keeps on
 * the stack; back to ScriptAdvance.
 * WAITS: yes, on the drive, through CdPlayOnce; the stack there is the caller's return, the
 * call's two bytes and X and Y pushed (2).
 * Entry: from ScriptStep, X the doubled command, Y 3. Exit: as ScriptAdvance.
 */
Exit ScriptPlayTrack(uint8_t doubled_command)
{
    uint8_t track;

    track = RD8_PTR_Y(ZP->SCRIPT_PTR, 3);
    jsr_push(0x8890);
    CdPlayOnce(track, doubled_command, 3);
    jsr_pop();
    return ScriptAdvance();
}

void abi_ScriptPlayTrack(void)
{
    exit_shim(ScriptPlayTrack(cpu.x));
}

/*
 * @routine 00C2:101 ScriptPlaceObject
 * @run 6B:8901
 * WHAT IT DOES: Empties an object and gives it the place four script bytes name.
 * HOW: ClearObject; then the bytes at Y plus 1 to 4 go to the object's X low, X high, Y low and Y
 * high.
 * WAITS: no.
 * Entry: X the object, Y the script byte before the four. Exit: A the last byte read, Y plus 4,
 * N and Z from A; X and C kept.
 */
Exit ScriptPlaceObject(uint8_t object, uint8_t y)
{
    Exit exit = { 0 };
    uint8_t value;

    jsr_push(0x8904);
    ClearObject(object);
    jsr_pop();
    y++;
    value = RD8_PTR_Y(ZP->SCRIPT_PTR, y);
    WR8(OBJ->x_lo[object], value);
    y++;
    value = RD8_PTR_Y(ZP->SCRIPT_PTR, y);
    WR8(OBJ->x_hi[object], value);
    y++;
    value = RD8_PTR_Y(ZP->SCRIPT_PTR, y);
    WR8(OBJ->y_lo[object], value);
    y++;
    value = RD8_PTR_Y(ZP->SCRIPT_PTR, y);
    WR8(OBJ->y_hi[object], value);
    exit.a = value;
    exit.y = y;
    exit.nz = value;
    exit.x_kept = true;
    exit.c_kept = true;
    return exit;
}

void abi_ScriptPlaceObject(void)
{
    exit_shim(ScriptPlaceObject(cpu.x, cpu.y));
}

/*
 * @routine 00C2:093 ScriptPlaceObjectState
 * @run 6B:8893
 * WHAT IT DOES: A script command: places an object (the operand's slot) and gives it the state
 * the next operand names.
 * HOW: ScriptPlaceObject, then one more byte to the object's first state byte (OBJ->f2750); back
 * to ScriptAdvance.
 * WAITS: no.
 * Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
 */
Exit ScriptPlaceObjectState(void)
{
    uint8_t object, y, value;

    object = RD8_PTR_Y(ZP->SCRIPT_PTR, 3);
    jsr_push(0x8899);
    ScriptPlaceObject(object, 3);
    jsr_pop();
    y = 8;
    value = RD8_PTR_Y(ZP->SCRIPT_PTR, y);
    WR8(OBJ->f2750[object], value);
    return ScriptAdvance();
}

void abi_ScriptPlaceObjectState(void)
{
    exit_shim(ScriptPlaceObjectState());
}

/*
 * @routine 00C2:0A2 ScriptPlaceObjectKind2
 * @run 6B:88A2
 * WHAT IT DOES: A script command: places an object, gives it the next two operands (OBJ->f3040
 * and OBJ->f2a10) and starts it in state 2.
 * HOW: ScriptPlaceObject, two bytes, a 2 in OBJ->f2750; back to ScriptAdvance.
 * WAITS: no.
 * Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
 */
Exit ScriptPlaceObjectKind2(void)
{
    uint8_t object, y, value;

    object = RD8_PTR_Y(ZP->SCRIPT_PTR, 3);
    jsr_push(0x88A8);
    ScriptPlaceObject(object, 3);
    jsr_pop();
    y = 8;
    value = RD8_PTR_Y(ZP->SCRIPT_PTR, y);
    WR8(OBJ->f3040[object], value);
    y = 9;
    value = RD8_PTR_Y(ZP->SCRIPT_PTR, y);
    WR8(OBJ->f2a10[object], value);
    WR8(OBJ->f2750[object], 2);
    return ScriptAdvance();
}

void abi_ScriptPlaceObjectKind2(void)
{
    exit_shim(ScriptPlaceObjectKind2());
}

/*
 * @routine 00C2:0BC ScriptPlaceObjectFrame
 * @run 6B:88BC
 * WHAT IT DOES: A script command: places an object and gives it a picture frame and a kind of
 * picture (OBJ->f2a68 and OBJ->f2a10) from the next two operands.
 * HOW: ScriptPlaceObject, two bytes; back to ScriptAdvance.
 * WAITS: no.
 * Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
 */
Exit ScriptPlaceObjectFrame(void)
{
    uint8_t object, y, value;

    object = RD8_PTR_Y(ZP->SCRIPT_PTR, 3);
    jsr_push(0x88C2);
    ScriptPlaceObject(object, 3);
    jsr_pop();
    y = 8;
    value = RD8_PTR_Y(ZP->SCRIPT_PTR, y);
    WR8(OBJ->f2a68[object], value);
    y = 9;
    value = RD8_PTR_Y(ZP->SCRIPT_PTR, y);
    WR8(OBJ->f2a10[object], value);
    return ScriptAdvance();
}

void abi_ScriptPlaceObjectFrame(void)
{
    exit_shim(ScriptPlaceObjectFrame());
}

/* The two title objects of the script: ClearObject, state +1 and four operands. */
static Exit script_place_title_object(uint8_t object, uint8_t y_low, uint16_t ret)
{
    uint8_t y, value;

    jsr_push(ret);
    ClearObject(object);
    jsr_pop();
    INC8(OBJ->f2750[object]);
    y = 3;
    value = RD8_PTR_Y(ZP->SCRIPT_PTR, y);
    WR8(OBJ->f3040[object], value);
    y = 4;
    value = RD8_PTR_Y(ZP->SCRIPT_PTR, y);
    WR8(OBJ->f34b8[object], value);
    y = 5;
    value = RD8_PTR_Y(ZP->SCRIPT_PTR, y);
    WR8(OBJ->f3510[object], value);
    y = 6;
    value = RD8_PTR_Y(ZP->SCRIPT_PTR, y);
    WR8(OBJ->f3568[object], value);
    WR8(OBJ->y_lo[object], y_low);
    return ScriptAdvance();
}

/*
 * @routine 00C2:11D ScriptPlaceTitleObjectA
 * @run 6B:891D
 * WHAT IT DOES: A script command: starts object $2E (a title object) with the next four operands
 * and its Y at $C8.
 * HOW: ClearObject, one added to its state, the four bytes to OBJ->f3040, f34b8, f3510 and
 * f3568, $C8 to its Y low; back to ScriptAdvance.
 * WAITS: no.
 * Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
 */
Exit ScriptPlaceTitleObjectA(void)
{
    return script_place_title_object(0x2E, 0xC8, 0x8922);
}

void abi_ScriptPlaceTitleObjectA(void)
{
    exit_shim(ScriptPlaceTitleObjectA());
}

/*
 * @routine 00C2:144 ScriptPlaceTitleObjectB
 * @run 6B:8944
 * WHAT IT DOES: As ScriptPlaceTitleObjectA for object $2F, its Y at $D8.
 * HOW: The same body, shared through script_place_title_object.
 * WAITS: no.
 * Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
 */
Exit ScriptPlaceTitleObjectB(void)
{
    return script_place_title_object(0x2F, 0xD8, 0x8949);
}

void abi_ScriptPlaceTitleObjectB(void)
{
    exit_shim(ScriptPlaceTitleObjectB());
}

/*
 * @routine 00C1:7DD AttractHoldBeforeEnd
 * @run 6B:87DD
 * WHAT IT DOES: The attract script's closing command: the scene goes to state 4, its counter to $40,
 * two object state bytes are cleared, and script slot 0 starts script 7.
 * HOW: Stores, then StartScriptSlot0Kind2 with 7 as a tail jump.
 * WAITS: no.
 * Entry: from ScriptStep. Exit: as StartScriptSlot0Kind2: A and Y 7, X 0, N and Z clear.
 */
Exit AttractHoldBeforeEnd(void)
{
    Exit exit = { 0 };

    WR8(ZP->game_sub_mode, 4);
    WR8(ZP->wait_timer3, 0x40);
    WR8(OBJ->f2750[8], 0);
    WR8(OBJ->f2750[9], 0);
    StartScriptSlot0Kind2(7);
    exit.a = 7;
    exit.y = 7;
    exit.x = 0;
    exit.nz = 7;
    exit.c_kept = true;
    return exit;
}

void abi_AttractHoldBeforeEnd(void)
{
    exit_shim(AttractHoldBeforeEnd());
}
