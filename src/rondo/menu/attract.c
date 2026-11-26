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
AttractEnd AttractEndCountdown(uint8_t kept_x, uint8_t kept_y)
{
    AttractEnd end;

    end.value = DEC8(ZP->wait_timer3);
    end.reset = false;
    if (end.value != 0) {
        return end;
    }
    jsr_push(0x87DA);
    ResetGameState(wait_regs_xy_shim(kept_x, kept_y));
    jsr_pop();
    end.value = DEC8(ZP->mode_done);
    end.reset = true;
    return end;
}

void abi_AttractEndCountdown(void)
{
    AttractEnd end = AttractEndCountdown(cpu.x, cpu.y);

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
uint8_t AttractBegin(uint8_t kept_x, uint8_t kept_y)
{
    jsr_push(0x871D);
    ResetGameState(wait_regs_xy_shim(kept_x, kept_y));
    jsr_pop();
    WR8(ZP->_gap_007C[2], 0x98);
    WR8(ZP->_gap_007C[3], 0x8F);
    WR8(ZP->wait_timer3, 0xFF);
    WR8(ZP->wait_timer4, 0xFF);
    return INC8(ZP->game_sub_mode);
}

void abi_AttractBegin(void)
{
    uint8_t state = AttractBegin(cpu.x, cpu.y);

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
    case 0x8852: return ScriptSound(index);
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
 * Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
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
Exit ScriptSound(uint8_t doubled_command)
{
    uint8_t sound;

    sound = RD8_PTR_Y(ZP->SCRIPT_PTR, 3);
    jsr_push(0x8857);
    SoundCommand(sound, doubled_command, 3);
    jsr_pop();
    return ScriptAdvance();
}

void abi_ScriptSound(void)
{
    exit_shim(ScriptSound(cpu.x));
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
    ClearScreenState(wait_regs_xy_shim(0, 3));   /* X 0 from the list start, Y 3 from ScriptStep */
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

/* A scene object handler's start: the registers it leaves alone are marked kept. */
static ObjExit object_exit(uint8_t a, uint8_t nz)
{
    ObjExit result = { 0 };

    result.exit.a = a;
    result.exit.nz = nz;
    result.exit.x_kept = true;
    result.exit.y_kept = true;
    result.exit.c_kept = true;
    return result;
}

/* The exit of a handler that ended in a move: the move's C and V, then the rest. */
static void object_exit_shim(ObjExit result)
{
    uint8_t moved_a = 0;

    if (result.moved) {
        moved_a = axis_move_flags_shim(result.move);
        if (result.exit.a_kept) {
            result.exit.a = moved_a;
            result.exit.a_kept = false;
        }
    }
    exit_shim(result.exit);
}

/*
 * @routine 00C2:6DE ObjectFrameList
 * @run 6B:8EDE
 * WHAT IT DOES: Steps an object's animation along the list a scene table names: the list's
 * address is read from a table by A, and StepObjectFrames runs on it.
 * HOW: A doubled indexes the table; its two bytes go to zTemp10 and zTemp11; StepObjectFrames as a
 * tail jump.
 * WAITS: no.
 * Entry: A the list, X the object. Exit: as StepObjectFrames; X kept.
 */
FrameListEnd ObjectFrameList(uint8_t list, uint8_t object)
{
    FrameListEnd end;
    uint8_t index, value;

    index = (uint8_t)(list << 1);
    value = rd8((uint16_t)(FRAME_LISTS + index));
    WR8(ZP->temp10, value);
    value = rd8((uint16_t)(FRAME_LISTS + 1 + index));
    WR8(ZP->temp11, value);
    end.y = index;
    end.step = StepObjectFrames(object, &end.y);
    return end;
}

/* What ObjectFrameList leaves: A 0 at the list's end, else $FF, N and Z from it; Y as the step left it; C set when the picture changed. */
ObjExit frame_list_exit_shim(FrameListEnd end)
{
    ObjExit result = { 0 };

    result.exit.a = (end.step == FRAME_LIST_ENDED) ? 0x00 : 0xFF;
    result.exit.nz = result.exit.a;
    result.exit.y = end.y;
    result.exit.c = end.step == FRAME_CHANGED;
    result.exit.x_kept = true;
    return result;
}

void abi_ObjectFrameList(void)
{
    object_exit_shim(frame_list_exit_shim(ObjectFrameList(cpu.a, cpu.x)));
}

/* A scene arm that ended on a value: A it, N and Z from it, the rest kept. */
static SceneArm scene_arm(int arm, uint8_t value)
{
    SceneArm r = { 0 };

    r.arm = arm;
    r.a = value;
    r.value = value;
    return r;
}

/*
 * @routine 00C2:1B0 PlayFrameListOnce
 * @run 6B:89B0
 * WHAT IT DOES: An object handler: once the object has no state yet, plays the animation the
 * object's next kind names until it ends, then moves the object to its next state.
 * HOW: OBJ->state nonzero returns; ObjectFrameList with OBJ->next_kind; at its end (A 0), state +1.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
SceneArm PlayFrameListOnce(uint8_t object)
{
    SceneArm r;
    uint8_t state, list;

    state = RD8(OBJ->state[object]);
    if (state != 0) {
        return scene_arm(SCENE_VALUE, state);
    }
    list = RD8(OBJ->next_kind[object]);
    r = scene_arm(SCENE_LIST, 0);
    jsr_push(0x89BB);
    r.list = ObjectFrameList(list, object);
    jsr_pop();
    if (r.list.step != FRAME_LIST_ENDED) {
        return r;
    }
    r.arm = SCENE_LIST_COUNTED;
    r.value = INC8(OBJ->state[object]);
    return r;
}

/* PlayFrameListOnce's exit: the state's value, or the list's exit with N and Z from the state counted up at its end. */
ObjExit play_frame_list_once_exit_shim(SceneArm r)
{
    ObjExit result;

    if (r.arm == SCENE_VALUE) {
        return object_exit(r.value, r.value);
    }
    result = frame_list_exit_shim(r.list);
    if (r.arm == SCENE_LIST_COUNTED) {
        result.exit.nz = r.value;
    }
    return result;
}

void abi_PlayFrameListOnce(void)
{
    object_exit_shim(play_frame_list_once_exit_shim(PlayFrameListOnce(cpu.x)));
}

/* The end 1EC and 20E share: a frame list stepped, and a script slot started when the picture changes. */
static SceneArm frame_list_then_cue(uint8_t list, uint8_t object)
{
    SceneArm r = scene_arm(SCENE_LIST, 0);
    uint8_t frame, script;

    jsr_push(0x89FD);
    r.list = ObjectFrameList(list, object);
    jsr_pop();
    if (r.list.step != FRAME_CHANGED) {
        return r;
    }
    frame = RD8(OBJ->frame[object]);
    if (frame == 0) {
        r.arm = SCENE_LIST_HELD;
        r.value = frame;
        return r;
    }
    push8(object);
    script = 7;
    jsr_push(0x8A0C);
    StartScriptSlotKind1(script, 1);
    jsr_pop();
    object = pull8();
    r.arm = SCENE_CUED;
    r.value = script;
    r.object = object;
    return r;
}

/*
 * The exit of the cue handlers (frame_list_then_cue, scroll_idle_cue): a value that stopped them, the list's exit, the list's
 * exit with A and N, Z from the byte that held the cue and C set, or the cue started: A and Y the script, X and N, Z the
 * object pulled back, C set.
 */
ObjExit scene_cue_exit_shim(SceneArm r)
{
    ObjExit result;

    switch (r.arm) {
    case SCENE_VALUE:
        return object_exit(r.value, r.value);
    case SCENE_LIST_HELD:
        result = frame_list_exit_shim(r.list);
        result.exit.a = r.value;
        result.exit.nz = r.value;
        result.exit.c = true;
        result.exit.c_kept = false;
        return result;
    case SCENE_CUED:
        result = object_exit(r.value, r.object);
        result.exit.y = r.value;
        result.exit.y_kept = false;
        result.exit.x = r.object;
        result.exit.x_kept = false;
        result.exit.c = true;
        result.exit.c_kept = false;
        return result;
    default:
        return frame_list_exit_shim(r.list);
    }
}

/*
 * @routine 00C2:1EC AnimateAndCueSeven
 * @run 6B:89EC
 * WHAT IT DOES: An object handler: on its first frame gives the object picture kind 3 and state 1;
 * then steps frame list 0, and when the picture changes to a nonzero frame starts script 7 on
 * slot 1.
 * HOW: OBJ->state, OBJ->sprite_set; ObjectFrameList(0); OBJ->frame; StartScriptSlotKind1 with X and
 * A kept on the stack.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
SceneArm AnimateAndCueSeven(uint8_t object)
{
    uint8_t state;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->sprite_set[object], 3);
        INC8(OBJ->state[object]);
    }
    return frame_list_then_cue(0, object);
}

void abi_AnimateAndCueSeven(void)
{
    object_exit_shim(scene_cue_exit_shim(AnimateAndCueSeven(cpu.x)));
}

/*
 * @routine 00C2:20E AnimateList1AndCueSeven
 * @run 6B:8A0E
 * WHAT IT DOES: As AnimateAndCueSeven with frame list 1.
 * HOW: The same first-frame set-up, then frame_list_then_cue with A 1.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
SceneArm AnimateList1AndCueSeven(uint8_t object)
{
    uint8_t state;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->sprite_set[object], 3);
        INC8(OBJ->state[object]);
    }
    return frame_list_then_cue(1, object);
}

void abi_AnimateList1AndCueSeven(void)
{
    object_exit_shim(scene_cue_exit_shim(AnimateList1AndCueSeven(cpu.x)));
}

/* The first-frame set-up and the scroll test that 21F and 252 share, then the frame list and cue. */
static SceneArm scroll_idle_cue(uint8_t list, uint8_t object)
{
    SceneArm r;
    uint8_t busy, frame, kind;

    busy = RD8(SCROLL->x_hi[0]);
    busy = (uint8_t)(busy | RD8(SCROLL->y_hi[0]));
    busy = (uint8_t)(busy | RD8(SCROLL->y_lo[0]));
    if (busy != 0) {
        return scene_arm(SCENE_VALUE, busy);
    }
    r = scene_arm(SCENE_LIST, 0);
    jsr_push(0x8A3C);
    r.list = ObjectFrameList(list, object);
    jsr_pop();
    if (r.list.step != FRAME_CHANGED) {
        return r;
    }
    frame = RD8(OBJ->frame[object]);
    if (frame == 0) {
        r.arm = SCENE_LIST_HELD;
        r.value = frame;
        return r;
    }
    kind = RD8(SLOTS->kind[0]);
    if (kind != 0) {
        r.arm = SCENE_LIST_HELD;
        r.value = kind;
        return r;
    }
    push8(object);
    jsr_push(0x8A50);
    StartScriptSlotKind1(9, 1);
    jsr_pop();
    object = pull8();
    r.arm = SCENE_CUED;
    r.value = 9;
    r.object = object;
    return r;
}

/*
 * @routine 00C2:21F AnimateAndCueNine
 * @run 6B:8A1F
 * WHAT IT DOES: An object handler: on its first frame gives the object picture kind 3 and state 1;
 * then, while the scroll is still ($3E10, $3E28 and $3E20 all zero), steps frame list 8 and, when
 * the picture changes to a nonzero frame and script slot 0 is idle, starts script 9 on slot 1.
 * HOW: OBJ->state, OBJ->sprite_set; the three scroll bytes ORed; ObjectFrameList(8);
 * OBJ->frame; SLOTS->kind[0]; StartScriptSlotKind1.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
SceneArm AnimateAndCueNine(uint8_t object)
{
    uint8_t state;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->sprite_set[object], 3);
        INC8(OBJ->state[object]);
    }
    return scroll_idle_cue(8, object);
}

void abi_AnimateAndCueNine(void)
{
    object_exit_shim(scene_cue_exit_shim(AnimateAndCueNine(cpu.x)));
}

/*
 * @routine 00C2:252 AnimateList9AndCueNine
 * @run 6B:8A52
 * WHAT IT DOES: As AnimateAndCueNine with frame list 9.
 * HOW: The same body, shared through scroll_idle_cue.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
SceneArm AnimateList9AndCueNine(uint8_t object)
{
    uint8_t state;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->sprite_set[object], 3);
        INC8(OBJ->state[object]);
    }
    return scroll_idle_cue(9, object);
}

void abi_AnimateList9AndCueNine(void)
{
    object_exit_shim(scene_cue_exit_shim(AnimateList9AndCueNine(cpu.x)));
}

/*
 * @routine 00C2:26E BlinkScrollFlag
 * @run 6B:8A6E
 * WHAT IT DOES: An object handler: counts frames in OBJ->f34b8 and sets the scroll's X high byte
 * to 1 once the count's low seven bits reach $78, else 0.
 * HOW: OBJ->state set to 1 on the first frame; OBJ->f34b8 counted; the compare's carry rotated in.
 * WAITS: no.
 * Entry: X the object. Exit: A 0 or 1, N and Z from it, C clear.
 */
uint8_t BlinkScrollFlag(uint8_t object)
{
    uint8_t state, count, flag;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        INC8(OBJ->state[object]);
    }
    INC8(OBJ->f34b8[object]);
    count = RD8(OBJ->f34b8[object]);
    flag = (uint8_t)((count & 0x7F) >= 0x78 ? 1 : 0);
    WR8(SCROLL->x_hi[0], flag);
    return flag;
}

/* BlinkScrollFlag's exit: A the flag, N and Z from it, C clear (the compare's carry rotated out). */
ObjExit blink_scroll_flag_exit_shim(uint8_t flag)
{
    ObjExit result = object_exit(flag, flag);

    result.exit.c = false;
    result.exit.c_kept = false;
    return result;
}

void abi_BlinkScrollFlag(void)
{
    object_exit_shim(blink_scroll_flag_exit_shim(BlinkScrollFlag(cpu.x)));
}

/*
 * @routine 00C2:286 MirrorVramBlock
 * @run 6B:8A86
 * WHAT IT DOES: An object handler: on its second frame mirrors a block of VRAM left to right: each
 * byte of $1C blocks of $70 words (the first block $70, the rest 256) from $1170 is written reversed
 * to $2CE0 on, and the object is cleared.
 * HOW: OBJ->state counts the first frame; then the read and write addresses are set, and each
 * word's low and high bytes are read, their bits rotated into zTemp00 one by one and written
 * back; ClearObject ends it.
 * WAITS: no.
 * Entry: X the object. Exit: A the last high byte reversed, Z set, N clear, C the last bit rotated
 * out, Y 0; X kept.
 */
ObjExit MirrorVramBlock(uint8_t object)
{
    ObjExit result;
    uint8_t state, value, mirrored, blocks, y;
    bool carry = false;
    int bit;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        INC8(OBJ->state[object]);
        return object_exit(state, (uint8_t)(state + 1));
    }
    jsr_push(0x8A92);
    SelectVramReadAddress();
    jsr_pop();
    st1(0x70);
    st2(0x11);
    jsr_push(0x8A99);
    SelectVramWriteAddress();
    jsr_pop();
    st1(0xE0);
    st2(0x2C);
    jsr_push(0x8AA0);
    SelectVramData();
    jsr_pop();
    WR8(ZP->temp01, 0x1C);
    y = 0x70;
    mirrored = 0;
    for (;;) {
        value = RD8(VDC->data_lo);
        for (bit = 0; bit < 8; bit++) {
            carry = ROR8(ZP->temp00, (value & 0x80) != 0);
            value = (uint8_t)(value << 1);
        }
        mirrored = RD8(ZP->temp00);
        WR8(VDC->data_lo, mirrored);
        value = RD8(VDC->data_hi);
        for (bit = 0; bit < 8; bit++) {
            carry = ROR8(ZP->temp00, (value & 0x80) != 0);
            value = (uint8_t)(value << 1);
        }
        mirrored = RD8(ZP->temp00);
        WR8(VDC->data_hi, mirrored);
        y = (uint8_t)(y - 1);
        if (y != 0) {
            continue;
        }
        blocks = DEC8(ZP->temp01);
        if (blocks == 0) {
            break;
        }
    }
    ClearObject(object);
    result = object_exit(mirrored, 0);
    result.exit.c = carry;
    result.exit.c_kept = false;
    result.exit.y = 0;
    result.exit.y_kept = false;
    return result;
}

void abi_MirrorVramBlock(void)
{
    object_exit_shim(MirrorVramBlock(cpu.x));
}

/*
 * @routine 00C2:2F0 NudgeScrollIn
 * @run 6B:8AF0
 * WHAT IT DOES: An object handler with three states: the first frame only advances; state 1 adds one
 * to the scroll's X low byte and, when it wraps, one to the Y high byte and goes on; state 2 adds one
 * a frame until it reaches $80 and goes on.
 * HOW: OBJ->state decremented by the original's DEC A to tell the states apart; the scroll bytes by
 * INC.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit NudgeScrollIn(uint8_t object)
{
    uint8_t state, rest, value;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        return object_exit(0, INC8(OBJ->state[object]));
    }
    rest = (uint8_t)(state - 1);
    if (rest == 0) {
        value = INC8(SCROLL->x_lo[0]);
        if (value != 0) {
            return object_exit(0, value);
        }
        INC8(SCROLL->y_hi[0]);
        return object_exit(0, INC8(OBJ->state[object]));
    }
    rest = (uint8_t)(rest - 1);
    if (rest != 0) {
        return object_exit(rest, rest);
    }
    INC8(SCROLL->x_lo[0]);
    value = RD8(SCROLL->x_lo[0]);
    if (value != 0x80) {
        ObjExit result = object_exit(value, (uint8_t)(value - 0x80));
        result.exit.c = value >= 0x80;
        result.exit.c_kept = false;
        return result;
    }
    {
        ObjExit result = object_exit(value, INC8(OBJ->state[object]));
        result.exit.c = true;
        result.exit.c_kept = false;
        return result;
    }
}

void abi_NudgeScrollIn(void)
{
    object_exit_shim(NudgeScrollIn(cpu.x));
}

/*
 * @routine 00C2:319 DropAndShakeScreen
 * @run 6B:8B19
 * WHAT IT DOES: An object handler with three states: the first frame gives the object a downward
 * speed and the scroll's Y low byte $FF; state 1 speeds the object up and moves the scroll and
 * the object down until the scroll runs out, then cues script 10 and waits $10 frames; state 2
 * shakes the scroll sideways by a random amount each frame until the wait ends, then clears it.
 * HOW: OBJ->state; OBJ->vy and OBJ->y_frac; SCROLL->y_lo, x_lo and x_hi; StartScriptSlotKind1;
 * NextRandom; OBJ->f34b8 the wait.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit DropAndShakeScreen(uint8_t object)
{
    ObjExit result;
    RandomResult random;
    uint8_t state, rest, low, frac, speed, scroll, high, diff_frac, diff_scroll, shake, left;
    bool carried, borrow_low, borrow_high;
    unsigned total;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        INC8(OBJ->state[object]);
        WR8(OBJ->vy_hi[object], 2);
        WR8(SCROLL->y_lo[0], 0xFF);
        return object_exit(0xFF, 0xFF);
    }
    rest = (uint8_t)(state - 1);
    if (rest == 0) {
        low = RD8(OBJ->vy_lo[object]);
        total = (unsigned)low + 0x40;
        speed = (uint8_t)total;
        carried = total > 0xFF;
        WR8(OBJ->vy_lo[object], speed);
        if (carried) {
            INC8(OBJ->vy_hi[object]);
        }
        frac = RD8(OBJ->y_frac[object]);
        speed = RD8(OBJ->vy_lo[object]);
        diff_frac = (uint8_t)(frac - speed);
        borrow_low = frac < speed;
        WR8(OBJ->y_frac[object], diff_frac);
        scroll = RD8(SCROLL->y_lo[0]);
        high = RD8(OBJ->vy_hi[object]);
        diff_scroll = (uint8_t)(scroll - high - (borrow_low ? 1 : 0));
        borrow_high = (unsigned)scroll < (unsigned)high + (borrow_low ? 1u : 0u);
        WR8(SCROLL->y_lo[0], diff_scroll);
        result = object_exit(diff_scroll, diff_scroll);
        result.exit.c = !borrow_high;
        result.exit.c_kept = false;
        result.exit.v = ((scroll ^ high) & (scroll ^ diff_scroll) & 0x80) != 0;
        result.exit.v_set = true;
        if (!borrow_high) {
            return result;
        }
        WR8(SCROLL->y_lo[0], 0);
        push8(object);
        jsr_push(0x8B5D);
        StartScriptSlotKind1(0x0A, 1);
        jsr_pop();
        object = pull8();
        WR8(OBJ->f34b8[object], 0x10);
        result.exit.y = 0x0A;
        result.exit.y_kept = false;
        result.exit.a = 0x10;
        result.exit.nz = INC8(OBJ->state[object]);
        return result;
    }
    rest = (uint8_t)(rest - 1);
    if (rest != 0) {
        return object_exit(rest, rest);
    }
    jsr_push(0x8B6D);
    random = NextRandom();
    jsr_pop();
    shake = (uint8_t)((random.value & 0x07) << 1);
    WR8(SCROLL->x_lo[0], shake);
    WR8(SCROLL->x_hi[0], 0);
    left = DEC8(OBJ->f34b8[object]);
    result = object_exit(shake, left);
    result.exit.c = false;
    result.exit.c_kept = false;
    result.exit.v = random.overflow;
    result.exit.v_set = true;
    if (left != 0) {
        return result;
    }
    WR8(SCROLL->x_lo[0], 0);
    WR8(SCROLL->x_hi[0], 0);
    result.exit.nz = INC8(OBJ->state[object]);
    return result;
}

void abi_DropAndShakeScreen(void)
{
    object_exit_shim(DropAndShakeScreen(cpu.x));
}

/*
 * @routine 00C2:3B3 ScatterDebris
 * @run 6B:8BB3
 * WHAT IT DOES: Fills the objects from 4 to $1E with pieces flying out, each from a table of four
 * bytes: its X place and speed, its Y place and speed, a random Y fraction, a frame.
 * HOW: Per object, from $1E down to 4: ClearObject, the table's bytes into OBJ->x_lo, vx, y_lo, vy_hi,
 * the frame; NextRandom into OBJ->vy_lo; the object's picture kind 3 and kind 8.
 * WAITS: no.
 * Entry: X kept on the stack across it (a typed caller passes its own). Exit: A 8, Z clear, N
 * clear, C as the last compare left it (set), Y 4 less than the table's last index; X kept.
 */
ObjExit ScatterDebris(uint8_t kept_x)
{
    ObjExit result;
    RandomResult random = { 0 };
    uint8_t object, y, byte, speed_lo, speed_hi;

    push8(kept_x);
    object = 0x1E;
    y = 0;
    do {
        jsr_push(0x8BBA);
        ClearObject(object);
        jsr_pop();
        byte = rd8((uint16_t)(DEBRIS_TABLE + y));
        WR8(OBJ->x_lo[object], byte);
        speed_lo = (uint8_t)(byte - 0x80);
        WR8(OBJ->vx_lo[object], speed_lo);
        if (speed_lo & 0x80) {
            DEC8(OBJ->vx_hi[object]);
        }
        {
            bool bit = ASL8(OBJ->vx_lo[object]);

            ROL8(OBJ->vx_hi[object], bit);
        }
        speed_hi = 0;
        (void)speed_hi;
        y++;
        byte = rd8((uint16_t)(DEBRIS_TABLE + y));
        WR8(OBJ->y_lo[object], byte);
        y++;
        byte = rd8((uint16_t)(DEBRIS_TABLE + y));
        WR8(OBJ->vy_hi[object], byte);
        jsr_push(0x8BE2);
        random = NextRandom();
        jsr_pop();
        WR8(OBJ->vy_lo[object], random.value);
        y++;
        byte = rd8((uint16_t)(DEBRIS_TABLE + y));
        WR8(OBJ->frame[object], byte);
        y++;
        WR8(OBJ->sprite_set[object], 3);
        WR8(OBJ->kind[object], 8);
        object--;
    } while (object != 3);
    result = object_exit(8, 0);
    result.exit.a = 8;
    result.exit.nz = 0;
    result.exit.c = true;
    result.exit.c_kept = false;
    result.exit.y = y;
    result.exit.y_kept = false;
    result.exit.x = pull8();
    result.exit.x_kept = false;
    result.exit.nz = result.exit.x;
    result.exit.v = random.overflow;
    result.exit.v_set = true;
    return result;
}

void abi_ScatterDebris(void)
{
    object_exit_shim(ScatterDebris(cpu.x));
}

/* Eight frames of wait, then the next state: the end 634 and 686 share. */
static ObjExit wait_eight_then_advance(uint8_t object)
{
    WR8(OBJ->f34b8[object], 8);
    return object_exit(8, INC8(OBJ->state[object]));
}

/* A move, then what its caller reads next: the move's C and V ride on the result. */
static ObjExit moved_exit(uint8_t a, uint8_t nz, AxisMove move)
{
    ObjExit result = object_exit(a, nz);

    result.move = move;
    result.moved = true;
    return result;
}

/* A move, then flag tests that leave A as the move did. */
static ObjExit moved_keep_a(uint8_t nz, AxisMove move)
{
    ObjExit result = moved_exit(0, nz, move);

    result.exit.a_kept = true;
    return result;
}

/*
 * @routine 00C2:385 BreakApartOnFrame
 * @run 6B:8B85
 * WHAT IT DOES: An object handler: on its first frame sets picture kind 3; then steps frame list 2
 * and when the picture reaches frame $0D starts script 8 on slot 1 and scatters debris.
 * HOW: OBJ->state; ObjectFrameList(2); OBJ->frame; StartScriptSlotKind1; ScatterDebris.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit BreakApartOnFrame(uint8_t object)
{
    ObjExit result;
    uint8_t state, rest, frame;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->sprite_set[object], 3);
        return object_exit(3, INC8(OBJ->state[object]));
    }
    rest = (uint8_t)(state - 1);
    if (rest != 0) {
        return object_exit(rest, rest);
    }
    jsr_push(0x8B9B);
    result = ObjectFrameList(2, object);
    jsr_pop();
    if (result.exit.nz == 0) {
        result.exit.nz = INC8(OBJ->state[object]);
        return result;
    }
    if (!result.exit.c) {
        return result;
    }
    frame = RD8(OBJ->frame[object]);
    if (frame != 0x0D) {
        result.exit.a = frame;
        result.exit.nz = (uint8_t)(frame - 0x0D);
        result.exit.c = frame >= 0x0D;
        result.exit.c_kept = false;
        return result;
    }
    push8(object);
    jsr_push(0x8BAE);
    StartScriptSlotKind1(8, 1);
    jsr_pop();
    object = pull8();
    return ScatterDebris(object);
}

void abi_BreakApartOnFrame(void)
{
    object_exit_shim(BreakApartOnFrame(cpu.x));
}

/*
 * @routine 00C2:46A FallAndClear
 * @run 6B:8C6A
 * WHAT IT DOES: An object handler: after its first frame the object moves by its speed, its Y speed
 * grows by $20 a frame, and once it is below the screen's middle, past $80, the object is cleared.
 * HOW: OBJ->state; ApplyVelocity; OBJ->vy; OBJ->y_hi and OBJ->y_lo; ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit FallAndClear(uint8_t object)
{
    ObjExit result;
    uint8_t state, low, sum, high, ylo;
    unsigned total;
    bool carried;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        return object_exit(0, INC8(OBJ->state[object]));
    }
    jsr_push(0x8C76);
    ApplyVelocity(object);
    jsr_pop();
    low = RD8(OBJ->vy_lo[object]);
    total = (unsigned)low + 0x20;
    sum = (uint8_t)total;
    carried = total > 0xFF;
    WR8(OBJ->vy_lo[object], sum);
    if (carried) {
        INC8(OBJ->vy_hi[object]);
    }
    high = RD8(OBJ->y_hi[object]);
    result = object_exit(high, high);
    result.exit.c = carried;
    result.exit.c_kept = false;
    result.exit.v = ((low ^ sum) & (0x20 ^ sum) & 0x80) != 0;
    result.exit.v_set = true;
    if ((high & 0x80) || high == 0) {
        return result;
    }
    ylo = RD8(OBJ->y_lo[object]);
    result.exit.a = ylo;
    result.exit.nz = (uint8_t)(ylo - 0x80);
    result.exit.c = ylo >= 0x80;
    if (ylo < 0x80) {
        return result;
    }
    ClearObject(object);
    return result;
}

void abi_FallAndClear(void)
{
    object_exit_shim(FallAndClear(cpu.x));
}

/*
 * @routine 00C2:495 IntroSceneObject
 * @run 6B:8C95
 * WHAT IT DOES: An object handler of five states: it starts script 11 on slot 2 and script 6 on slot 1
 * and swaps two script slots' scripts, then rises, steps frame list 3, and eases the scroll in two
 * stages by $E steps.
 * HOW: OBJ->state; StartScriptSlotKind1 and StartScriptSlotKind4 with X on the stack;
 * ApplyVelocity; ObjectFrameList(3); SCROLL->y_lo counted down by two.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit IntroSceneObject(uint8_t object)
{
    ObjExit result;
    uint8_t state, rest, first, second, ylo, left;
    AxisMove move;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        INC8(OBJ->state[object]);
        WR8(OBJ->vy_hi[object], 0xFF);
        WR8(OBJ->frame[object], 0x2F);
        WR8(OBJ->sprite_set[object], 3);
        INC8(SCROLL->x_hi[0]);
        INC8(SCROLL->y_hi[0]);
        push8(object);
        jsr_push(0x8CBA);
        StartScriptSlotKind1(0x0B, 2);
        jsr_pop();
        jsr_push(0x8CC1);
        StartScriptSlotKind4(6, 1);
        jsr_pop();
        first = RD8(SLOTS->script[2]);
        second = RD8(SLOTS->script[0]);
        WR8(SLOTS->script[2], second);
        WR8(SLOTS->script[0], first);
        object = pull8();
        result = object_exit(first, object);
        result.exit.x = object;
        result.exit.x_kept = false;
        result.exit.y = 6;
        result.exit.y_kept = false;
        return result;
    }
    rest = (uint8_t)(state - 1);
    if (rest == 0) {
        jsr_push(0x8CD5);
        move = ApplyVelocity(object);
        jsr_pop();
        ylo = RD8(OBJ->y_lo[object]);
        if (ylo & 0x80) {
            return moved_exit(ylo, ylo, move);
        }
        return moved_exit(ylo, INC8(OBJ->state[object]), move);
    }
    rest = (uint8_t)(rest - 1);
    if (rest == 0) {
        jsr_push(0x8CE6);
        result = ObjectFrameList(3, object);
        jsr_pop();
        if (result.exit.nz != 0) {
            return result;
        }
        result.exit.nz = INC8(OBJ->state[object]);
        return result;
    }
    rest = (uint8_t)(rest - 1);
    if (rest == 0) {
        jsr_push(0x8CF2);
        move = ApplyVelocity(object);
        jsr_pop();
        WR8(SCROLL->y_hi[0], 0);
        DEC8(SCROLL->y_lo[0]);
        left = DEC8(SCROLL->y_lo[0]);
        if (left != 0) {
            return moved_keep_a(left, move);
        }
        DEC8(SCROLL->x_hi[0]);
        INC8(SCROLL->y_hi[0]);
        return moved_keep_a(INC8(OBJ->state[object]), move);
    }
    rest = (uint8_t)(rest - 1);
    if (rest != 0) {
        return object_exit(rest, rest);
    }
    jsr_push(0x8D0D);
    move = ApplyVelocity(object);
    jsr_pop();
    WR8(SCROLL->y_hi[0], 0);
    DEC8(SCROLL->y_lo[0]);
    left = DEC8(SCROLL->y_lo[0]);
    if (left != 0) {
        return moved_keep_a(left, move);
    }
    return moved_keep_a(INC8(OBJ->state[object]), move);
}

void abi_IntroSceneObject(void)
{
    object_exit_shim(IntroSceneObject(cpu.x));
}

/* What a frame-list step left in Y and C, laid over the exit of the code that ran after it. */
static ObjExit with_list_regs(ObjExit result, ObjExit list)
{
    result.exit.y = list.exit.y;
    result.exit.y_kept = list.exit.y_kept;
    result.exit.c = list.exit.c;
    result.exit.c_kept = list.exit.c_kept;
    return result;
}

/* A move laid over an exit: its C and V come out last. */
static ObjExit with_move(ObjExit result, AxisMove move)
{
    result.move = move;
    result.moved = true;
    result.exit.c_kept = true;
    return result;
}

/* The wait of 51C: the register A counts down from the state less four; it ends at 0. */
static void spin_down(uint8_t count)
{
    while (count != 0) {
        count--;
    }
}

/*
 * @routine 00C2:51C RiseAndFallObject
 * @run 6B:8D1C
 * WHAT IT DOES: An object handler of five stages: it shows picture kind 3 and moves the scroll in,
 * waits $10 frames, steps frame list 4 and then 5 (stopping to set a cue), and from then on steps
 * list 6 and moves the object until it is above the screen, when it is cleared.
 * HOW: OBJ->state; OBJ->life_timer; ObjectFrameList; ApplyVelocity; ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit RiseAndFallObject(uint8_t object)
{
    ObjExit result;
    uint8_t state, rest, high, timer;
    AxisMove move;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->sprite_set[object], 3);
        INC8(SCROLL->x_hi[0]);
        WR8(OBJ->vy_hi[object], 0xFF);
        WR8(OBJ->life_timer[object], 0x10);
        return object_exit(0x10, INC8(OBJ->state[object]));
    }
    rest = (uint8_t)(state - 1);
    if (rest == 0) {
        timer = DEC8(OBJ->life_timer[object]);
        if (timer != 0) {
            return object_exit(0, timer);
        }
        WR8(OBJ->life_timer[object], 4);
        WR8(SCROLL->x_hi[0], 0);
        return object_exit(4, INC8(OBJ->state[object]));
    }
    rest = (uint8_t)(rest - 1);
    if (rest == 0) {
        jsr_push(0x8D53);
        result = ObjectFrameList(4, object);
        jsr_pop();
        if (result.exit.nz != 0) {
            return result;
        }
        result.exit.nz = INC8(OBJ->state[object]);
        return result;
    }
    rest = (uint8_t)(rest - 1);
    if (rest == 0) {
        jsr_push(0x8D5E);
        result = ObjectFrameList(5, object);
        jsr_pop();
        if (result.exit.nz != 0) {
            return result;
        }
        result.exit.nz = DEC8(OBJ->life_timer[object]);
        if (result.exit.nz != 0) {
            return result;
        }
        WR8(OBJ->kind[0x1E], 0x0B);
        result.exit.a = 0x0B;
        result.exit.nz = INC8(OBJ->state[object]);
        return result;
    }
    spin_down((uint8_t)(rest - 1));
    jsr_push(0x8D76);
    result = ObjectFrameList(6, object);
    jsr_pop();
    jsr_push(0x8D79);
    move = ApplyVelocity(object);
    jsr_pop();
    high = RD8(OBJ->y_hi[object]);
    result.exit.a = high;
    result.exit.nz = high;
    result = with_move(result, move);
    result.exit.a_kept = false;
    if ((high & 0x80) == 0) {
        return result;
    }
    ClearObject(object);
    return result;
}

void abi_RiseAndFallObject(void)
{
    object_exit_shim(RiseAndFallObject(cpu.x));
}

/*
 * @routine 00C2:581 DriftAndClearObject
 * @run 6B:8D81
 * WHAT IT DOES: An object handler: on its first frame it sets a slow rightward speed; after that it
 * steps frame list 6 (7 once it is past $50 down), moves, slows its upward speed by 4 a frame,
 * and clears itself once above the screen.
 * HOW: OBJ->state; OBJ->y_lo; ObjectFrameList; ApplyVelocity; OBJ->vy; ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit DriftAndClearObject(uint8_t object)
{
    ObjExit result, list;
    uint8_t state, ylo, speed, high, difference;
    AxisMove move;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        INC8(OBJ->vx_hi[object]);
        WR8(OBJ->vx_lo[object], 0x40);
        return object_exit(0x40, INC8(OBJ->state[object]));
    }
    ylo = RD8(OBJ->y_lo[object]);
    jsr_push(0x8DA2);
    list = ObjectFrameList((ylo >= 0x50) ? 7 : 6, object);
    jsr_pop();
    jsr_push(0x8DA5);
    move = ApplyVelocity(object);
    jsr_pop();
    (void)move;
    speed = RD8(OBJ->vy_lo[object]);
    difference = (uint8_t)(speed - 4);
    WR8(OBJ->vy_lo[object], difference);
    if (speed < 4) {
        DEC8(OBJ->vy_hi[object]);
    }
    high = RD8(OBJ->y_hi[object]);
    result = with_list_regs(object_exit(high, high), list);
    result.exit.c = speed >= 4;
    result.exit.c_kept = false;
    result.exit.v = ((speed ^ 4) & (speed ^ difference) & 0x80) != 0;
    result.exit.v_set = true;
    if ((high & 0x80) == 0) {
        return result;
    }
    ClearObject(object);
    return result;
}

void abi_DriftAndClearObject(void)
{
    object_exit_shim(DriftAndClearObject(cpu.x));
}

/*
 * @routine 00C2:5BB SplitScreenObject
 * @run 6B:8DBB
 * WHAT IT DOES: An object handler of four states: it starts a raster effect and the two halves of
 * the scene (objects $10 and $11) drifting apart, waits for them, then moves both, counts the effect
 * and the scroll down, clears both and moves the scroll, and last counts the scroll out.
 * HOW: OBJ->state; ZP->raster_effect; SCROLL->y_lo and x_hi; ApplyVelocity and ClearObject on $10
 * and $11 with X on the stack.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit SplitScreenObject(uint8_t object)
{
    ObjExit result;
    uint8_t state, rest, effect, left;
    AxisMove move;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        INC8(OBJ->state[object]);
        DEC8(OBJ->vx_hi[0x10]);
        INC8(OBJ->vx_hi[0x11]);
        WR8(OBJ->life_timer[object], 0x40);
        WR8(ZP->raster_effect, 0x12);
        WR8(ZP->raster_arg, 0);
        WR8(SCROLL->y_lo[2], 0x38);
        INC8(SCROLL->x_hi[0]);
        WR8(SCROLL->y_lo[0], 0x68);
        return object_exit(0x68, 0x68);
    }
    rest = (uint8_t)(state - 1);
    if (rest == 0) {
        left = DEC8(OBJ->life_timer[object]);
        if (left != 0) {
            return object_exit(0, left);
        }
        return object_exit(0, INC8(OBJ->state[object]));
    }
    rest = (uint8_t)(rest - 1);
    if (rest == 0) {
        push8(object);
        jsr_push(0x8DF7);
        ApplyVelocity(0x10);
        jsr_pop();
        jsr_push(0x8DFB);
        move = ApplyVelocity(0x11);
        jsr_pop();
        object = pull8();
        effect = RD8(ZP->raster_effect);
        result = with_move(object_exit(effect, effect), move);
        result.exit.x_kept = true;
        if (effect != 0) {
            left = DEC8(SCROLL->y_lo[2]);
            result.exit.nz = left;
            if (left == 0) {
                WR8(ZP->raster_effect, 0);
            }
        }
        left = DEC8(SCROLL->y_lo[0]);
        result.exit.nz = left;
        if (left != 0) {
            return result;
        }
        push8(object);
        jsr_push(0x8E12);
        ClearObject(0x10);
        jsr_pop();
        jsr_push(0x8E16);
        ClearObject(0x11);
        jsr_pop();
        object = pull8();
        WR8(SCROLL->x_hi[0], 0);
        INC8(SCROLL->y_hi[0]);
        result.exit.nz = INC8(OBJ->state[object]);
        return result;
    }
    rest = (uint8_t)(rest - 1);
    if (rest == 0) {
        WR8(SCROLL->y_hi[0], 0);
        left = DEC8(SCROLL->y_lo[0]);
        if (left != 0) {
            return object_exit(0, left);
        }
        return object_exit(0, INC8(OBJ->state[object]));
    }
    spin_down((uint8_t)(rest - 1));
    return object_exit(0, 0);
}

void abi_SplitScreenObject(void)
{
    object_exit_shim(SplitScreenObject(cpu.x));
}

/* The step of 634 and 686: the speeds get set, then eight frames of waiting and the next state. */
static ObjExit then_wait(uint8_t object, ObjExit list)
{
    return with_list_regs(wait_eight_then_advance(object), list);
}

/* The step both end in: a frame list stepped after a move; the object waits eight frames once it ends. */
static ObjExit list_after_move(uint8_t list_number, uint16_t return_move, uint16_t return_list, uint8_t object)
{
    ObjExit list;
    AxisMove move;

    jsr_push(return_move);
    move = ApplyVelocity(object);
    jsr_pop();
    jsr_push(return_list);
    list = ObjectFrameList(list_number, object);
    jsr_pop();
    list.move = move;
    list.moved = true;
    return list;
}

/*
 * @routine 00C2:634 FlingObjectRight
 * @run 6B:8E34
 * WHAT IT DOES: An object handler: it shows picture kind 3 with a rightward speed, then moves while
 * stepping frame lists $0A and $0B in turn, flicks its speed to a hard upward one, and ends on
 * list $0D.
 * HOW: OBJ->state; ApplyVelocity; ObjectFrameList; OBJ->life_timer counted down to 0 between lists.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit FlingObjectRight(uint8_t object)
{
    ObjExit list;
    uint8_t state, rest, left;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->sprite_set[object], 3);
        WR8(OBJ->vx_lo[object], 0x70);
        return wait_eight_then_advance(object);
    }
    rest = (uint8_t)(state - 1);
    if (rest == 0) {
        list = list_after_move(0x0A, 0x8E52, 0x8E57, object);
        if (list.exit.nz != 0) {
            return list;
        }
        left = DEC8(OBJ->life_timer[object]);
        list.exit.nz = left;
        if (left != 0) {
            return list;
        }
        WR8(OBJ->vy_lo[object], 0xB0);
        DEC8(OBJ->vy_hi[object]);
        return then_wait(object, list);
    }
    rest = (uint8_t)(rest - 1);
    if (rest == 0) {
        list = list_after_move(0x0B, 0x8E65, 0x8E6A, object);
        if (list.exit.nz != 0) {
            return list;
        }
        left = DEC8(OBJ->life_timer[object]);
        list.exit.nz = left;
        if (left != 0) {
            return list;
        }
        WR8(OBJ->vx_lo[object], 1);
        WR8(OBJ->vy_lo[object], 0xFF);
        WR8(OBJ->vy_hi[object], 0xFF);
        return then_wait(object, list);
    }
    return ObjectFrameList(0x0D, object);
}

void abi_FlingObjectRight(void)
{
    object_exit_shim(FlingObjectRight(cpu.x));
}

/*
 * @routine 00C2:686 FlingObjectLeft
 * @run 6B:8E86
 * WHAT IT DOES: The mirror of FlingObjectRight: leftward, with frame lists $0A, $0C and $0D.
 * HOW: OBJ->state; ApplyVelocity; ObjectFrameList; OBJ->life_timer counted down to 0 between lists.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit FlingObjectLeft(uint8_t object)
{
    ObjExit list;
    uint8_t state, rest, left;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->sprite_set[object], 3);
        WR8(OBJ->vx_lo[object], 0x90);
        DEC8(OBJ->vx_hi[object]);
        return wait_eight_then_advance(object);
    }
    rest = (uint8_t)(state - 1);
    if (rest == 0) {
        list = list_after_move(0x0A, 0x8EB1, 0x8EB6, object);
        if (list.exit.nz != 0) {
            return list;
        }
        left = DEC8(OBJ->life_timer[object]);
        list.exit.nz = left;
        if (left != 0) {
            return list;
        }
        WR8(OBJ->vy_lo[object], 0xB0);
        DEC8(OBJ->vy_hi[object]);
        return then_wait(object, list);
    }
    rest = (uint8_t)(rest - 1);
    if (rest == 0) {
        list = list_after_move(0x0C, 0x8EC4, 0x8EC9, object);
        if (list.exit.nz != 0) {
            return list;
        }
        left = DEC8(OBJ->life_timer[object]);
        list.exit.nz = left;
        if (left != 0) {
            return list;
        }
        WR8(OBJ->vx_lo[object], 0xFF);
        WR8(OBJ->vy_lo[object], 0xFF);
        WR8(OBJ->vy_hi[object], 0xFF);
        return then_wait(object, list);
    }
    return ObjectFrameList(0x0D, object);
}

void abi_FlingObjectLeft(void)
{
    object_exit_shim(FlingObjectLeft(cpu.x));
}
