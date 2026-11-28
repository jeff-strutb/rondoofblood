/*
 * The cutscene overlay of sectors $267B to $267E (bank $6B at $8000; scenes $58 and $59, after a stage): the script
 * engine of scene_engine_b.c at this overlay's places (lengths $8043, commands $8082, object kinds $826F), and this
 * overlay's own object handlers.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "scene_engine.h"
#include "scene_engine_b.h"
#include "scene_58.h"

static bool scene_58_command(uint16_t target, uint8_t index, bool carry, Exit *out);
bool scene_58_handler(uint16_t target, uint8_t object, ObjExit *out);

static const SceneLayoutB LAYOUT = {
    0x8065, 0x8250, Scene58ObjectsStep, 0x8043, 0x8082, 0x826F, 0x818E, 0x8198, scene_58_command, scene_58_handler
};

/*
 * @routine 267B:000 Scene58Step
 * @run 6B:8000
 * @stop 800E
 * WHAT IT DOES: One call of the scene: bank zRamBaseBank + $0A mapped at $A000, then the handler of the sub-mode in
 * the table at $800E (0 begins, 1 runs a frame, 2 waits for RUN).
 * HOW: ADC and TAM to MPR5; zGameSubMode doubled indexes the table and is jumped through, to the routine loaded
 * there: the $5A overlay holds these bytes with its own handlers at the same places.
 * WAITS: yes, through the begin handler.
 * Entry: nothing. Exit: as the handler leaves them; it is entered with A and X the doubled sub-mode, N and Z from it,
 * C from the doubling and V from the bank's add.
 */
Exit Scene58Step(uint8_t kept_y)
{
    Exit exit = { 0 };
    uint8_t base, bank, state, index, low, high;
    uint16_t target;
    bool overflow;

    base = RD8(ZP->ram_base_bank);
    bank = (uint8_t)(0x0A + base);
    overflow = ((0x0A ^ bank) & (base ^ bank) & 0x80) != 0;
    tam(0x20, bank);
    state = RD8(ZP->game_sub_mode);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0x800E + index));
    high = rd8((uint16_t)(0x800F + index));
    target = (uint16_t)(low | (high << 8));
    cpu.a = index;
    cpu.x = index;
    cpu.y = kept_y;
    abi_nz(index);
    abi_flag(P_C, (state & 0x80) != 0);
    abi_flag(P_V, overflow);
    run_routine(target);
    exit.from_cpu = true;
    return exit;
}

void abi_Scene58Step(void)
{
    exit_shim(Scene58Step(cpu.y));
}

/* This overlay's script commands by their addresses, for the engine's step. */
static bool scene_58_command(uint16_t target, uint8_t index, bool carry, Exit *out)
{
    switch (target) {
    case 0x80C0: *out = Scene58CommandStartListAndClear(); return true;
    case 0x80DC: *out = Scene58CommandStartScript1(); return true;
    case 0x807D: *out = Scene58CommandEnd(index, carry); return true;
    case 0x8104: *out = Scene58CommandPlaceKind(); return true;
    case 0x812D: *out = Scene58CommandPlaceFrame(); return true;
    case 0x80F4: *out = Scene58CommandPlayAdpcm(); return true;
    case 0x80E4: *out = Scene58CommandStartScript2(); return true;
    case 0x80EC: *out = Scene58CommandStartScript3(); return true;
    case 0x80FC: *out = Scene58CommandCdPlay(index); return true;
    case 0x81B4: *out = Scene58CommandPlaceTitleObjectALow(); return true;
    case 0x81DE: *out = Scene58CommandPlaceTitleObjectB(); return true;
    case 0x80A8: *out = Scene58CommandSubModeTwo(); return true;
    case 0x80D4: *out = Scene58CommandStartList(); return true;
    case 0x8113: *out = Scene58CommandPlaceNext(); return true;
    case 0x8142: *out = Scene58CommandPlaceTimed(); return true;
    case 0x821B: *out = Scene58CommandLoadBanks(); return true;
    case 0x80AE: *out = Scene58CommandSetScroll(); return true;
    case 0x8168: *out = Scene58CommandPlaceWorked(); return true;
    case 0x8205: *out = Scene58CommandBranchOnProgress(); return true;
    default: return false;
    }
}

/* This overlay's object handlers by their addresses, for the engine's dispatch. */
bool scene_58_handler(uint16_t target, uint8_t object, ObjExit *out)
{
    switch (target) {
    case 0x8574: *out = Scene58ScrollLayers(object); return true;
    case 0x85B3: *out = Scene58Sparkles(object); return true;
    case 0x861F: *out = Scene58SparkleFall(object); return true;
    case 0x8389: *out = Scene58LoadPatterns(object, 0x18); return true;   /* kind 13: Y the dispatch's index $18 */
    case 0x828B: *out = Scene58SpawnCycleA(object); return true;
    case 0x83B9: *out = Scene58Swing(object); return true;
    case 0x8468: *out = Scene58ShowObjectsAndClear(object); return true;
    case 0x8476: *out = Scene58SpawnCycleB(object); return true;
    case 0x863D: *out = Scene58CycleLists(object); return true;
    case 0x866B: *out = Scene58Shake(object); return true;
    case 0x8698: *out = Scene58SlideThenList(object); return true;
    case 0x86A1: *out = Scene58PlayListOnce(object); return true;
    case 0x86B7: *out = Scene58Slide(object); return true;
    default: return false;
    }
}

/*
 * @routine 267B:014 Scene58Begin
 * @run 6B:8014
 * WHAT IT DOES: Sub-mode 0: starts the scene: the game state wiped, the script from $8E8A, its frame counter $FFFF, the sub-mode on.
 * HOW: ResetGameState, then Scene5CBegin's stores with this script.
 * WAITS: yes, through ResetGameState (the vertical blank).
 * Entry: X the doubled sub-mode, Y kept. Exit: A $FF, X 0, Y $FF, N and Z from the new sub-mode; C kept.
 */
Exit Scene58Begin(uint8_t kept_x, uint8_t kept_y)
{
    return SceneBBegin(0x8014, 0x8A, 0x8E, kept_x, kept_y);
}

void abi_Scene58Begin(void)
{
    exit_shim(Scene58Begin(cpu.x, cpu.y));
}

/*
 * @routine 267B:028 Scene58Frame
 * @run 6B:8028
 * WHAT IT DOES: Sub-mode 1: one frame of the scene: the frame counted, the script's command run if due, every object run.
 * HOW: The copy of EndingFrameTick at $803C, the script step, the objects' loop.
 * WAITS: no.
 * Entry: nothing. Exit: as the objects' loop, Y and V the step's where the loop leaves them.
 */
Exit Scene58Frame(void)
{
    return SceneBFrame(&LAYOUT, 0x8028);
}

void abi_Scene58Frame(void)
{
    exit_shim(Scene58Frame());
}

/*
 * @routine 267B:032 Scene58WaitForRun
 * @run 6B:8032
 * WHAT IT DOES: Sub-mode 2, after the script ends: the mode is done when RUN is newly pressed.
 * HOW: BiosJoyTrigger's RUN bit; INC of zModeDone.
 * WAITS: no.
 * Entry: nothing. Exit: A the RUN bit, N and Z from it or from zModeDone; X, Y, C and V kept.
 */
Exit Scene58WaitForRun(void)
{
    return SceneBWaitForRun();
}

void abi_Scene58WaitForRun(void)
{
    exit_shim(Scene58WaitForRun());
}

/*
 * @routine 267B:056 Scene58ScriptAdvance
 * @run 6B:8056
 * WHAT IT DOES: Steps the script past its command and runs on into the script step.
 * HOW: The command's length from this overlay's table at $8043 added to the pointer.
 * WAITS: no.
 * Entry: nothing. Exit: as the script step, X the command stepped past when it left X alone, V from the add when it set none.
 */
Exit Scene58ScriptAdvance(void)
{
    return SceneBScriptAdvance(&LAYOUT);
}

void abi_Scene58ScriptAdvance(void)
{
    exit_shim(Scene58ScriptAdvance());
}

/*
 * @routine 267B:065 Scene58ScriptStep
 * @run 6B:8065
 * WHAT IT DOES: Runs the script's command when its two time bytes equal the frame counter.
 * HOW: The times compared; the command doubled indexes this overlay's table at $8082, entered with X the index and Y 3.
 * WAITS: no.
 * Entry: nothing. Exit: a time byte differs: that byte, N, Z and C from comparing it, Y 1 or 2, X kept; else as the command.
 */
Exit Scene58ScriptStep(void)
{
    return SceneBScriptStep(&LAYOUT);
}

void abi_Scene58ScriptStep(void)
{
    exit_shim(Scene58ScriptStep());
}

/*
 * @routine 267B:07D Scene58CommandEnd
 * @run 6B:807D
 * WHAT IT DOES: A script command: ends the script, the sub-mode set to 2 (wait for RUN); the script is not stepped.
 * HOW: A store; RTS.
 * WAITS: no.
 * Entry: from the script step: X the doubled command, Y 3. Exit: A 2, N and Z from it; X, Y and C kept.
 */
Exit Scene58CommandEnd(uint8_t index, bool carry)
{
    return SceneBCommandEnd(index, carry);
}

void abi_Scene58CommandEnd(void)
{
    exit_shim(Scene58CommandEnd(cpu.x, (cpu.p & P_C) != 0));
}

/*
 * @routine 267B:0A8 Scene58CommandSubModeTwo
 * @run 6B:80A8
 * WHAT IT DOES: A script command: the sub-mode set to 2, and the script steps on.
 * HOW: A store; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandSubModeTwo(void)
{
    return SceneBCommandSubModeTwo(&LAYOUT);
}

void abi_Scene58CommandSubModeTwo(void)
{
    exit_shim(Scene58CommandSubModeTwo());
}

/*
 * @routine 267B:0AE Scene58CommandSetScroll
 * @run 6B:80AE
 * WHAT IT DOES: A script command: the screen's scroll X high from the operand's high four bits, Y high from its low four.
 * HOW: PHA, four LSR, PLA, AND; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandSetScroll(void)
{
    return SceneBCommandSetScroll(&LAYOUT);
}

void abi_Scene58CommandSetScroll(void)
{
    exit_shim(Scene58CommandSetScroll());
}

/*
 * @routine 267B:0C0 Scene58CommandStartListAndClear
 * @run 6B:80C0
 * WHAT IT DOES: A script command: list channel 0 started with the operand's list, the screen's state, every object and the scroll block ($EE bytes from ScrollXFrac) cleared.
 * HOW: StartListChannel0Kind2, ClearScreenState, the copy of EndingClearObjects at $818E, a TAI of $EE zeros to ScrollXFrac; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandStartListAndClear(void)
{
    return SceneBCommandStartListAndClear(&LAYOUT, 0x80C0);
}

void abi_Scene58CommandStartListAndClear(void)
{
    exit_shim(Scene58CommandStartListAndClear());
}

/*
 * @routine 267B:0D4 Scene58CommandStartList
 * @run 6B:80D4
 * WHAT IT DOES: A script command: list channel 0 started with the operand's list.
 * HOW: StartListChannel0Kind2; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandStartList(void)
{
    return SceneBCommandStartList(&LAYOUT, 0x80D4);
}

void abi_Scene58CommandStartList(void)
{
    exit_shim(Scene58CommandStartList());
}

/*
 * @routine 267B:0DC Scene58CommandStartScript1
 * @run 6B:80DC
 * WHAT IT DOES: A script command: script slot 0 started with the operand, kind 1.
 * HOW: StartScriptSlot0Kind1; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandStartScript1(void)
{
    return SceneBCommandStartScript1(&LAYOUT, 0x80DC);
}

void abi_Scene58CommandStartScript1(void)
{
    exit_shim(Scene58CommandStartScript1());
}

/*
 * @routine 267B:0E4 Scene58CommandStartScript2
 * @run 6B:80E4
 * WHAT IT DOES: A script command: script slot 0 started with the operand, kind 2.
 * HOW: StartScriptSlot0Kind2; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandStartScript2(void)
{
    return SceneBCommandStartScript2(&LAYOUT, 0x80E4);
}

void abi_Scene58CommandStartScript2(void)
{
    exit_shim(Scene58CommandStartScript2());
}

/*
 * @routine 267B:0EC Scene58CommandStartScript3
 * @run 6B:80EC
 * WHAT IT DOES: A script command: script slot 0 started with the operand, kind 3.
 * HOW: StartScriptSlot0Kind3; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandStartScript3(void)
{
    return SceneBCommandStartScript3(&LAYOUT, 0x80EC);
}

void abi_Scene58CommandStartScript3(void)
{
    exit_shim(Scene58CommandStartScript3());
}

/*
 * @routine 267B:0F4 Scene58CommandPlayAdpcm
 * @run 6B:80F4
 * WHAT IT DOES: A script command: the ADPCM sound the operand names played from the disc.
 * HOW: AdpcmPlayFromDisc; the advance.
 * WAITS: yes, on the drive and the ADPCM.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandPlayAdpcm(void)
{
    return SceneBCommandPlayAdpcm(&LAYOUT, 0x80F4);
}

void abi_Scene58CommandPlayAdpcm(void)
{
    exit_shim(Scene58CommandPlayAdpcm());
}

/*
 * @routine 267B:0FC Scene58CommandCdPlay
 * @run 6B:80FC
 * WHAT IT DOES: A script command: the CD track the operand names played once.
 * HOW: CdPlayOnce with X the doubled command; the advance.
 * WAITS: yes, on the drive.
 * Entry: from the script step: X the doubled command, Y 3. Exit: as the advance.
 */
Exit Scene58CommandCdPlay(uint8_t index)
{
    return SceneBCommandCdPlay(&LAYOUT, 0x80FC, index);
}

void abi_Scene58CommandCdPlay(void)
{
    exit_shim(Scene58CommandCdPlay(cpu.x));
}

/*
 * @routine 267B:104 Scene58CommandPlaceKind
 * @run 6B:8104
 * WHAT IT DOES: A script command: an object placed at the script's X and Y and given the kind that follows.
 * HOW: The copy of EndingPlaceObject, then the kind; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandPlaceKind(void)
{
    return SceneBCommandPlaceKind(&LAYOUT, 0x8104);
}

void abi_Scene58CommandPlaceKind(void)
{
    exit_shim(Scene58CommandPlaceKind());
}

/*
 * @routine 267B:113 Scene58CommandPlaceNext
 * @run 6B:8113
 * WHAT IT DOES: A script command: an object placed, its start argument and sprite set from the script, kind 2.
 * HOW: The copy of EndingPlaceObject, then the stores; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandPlaceNext(void)
{
    return SceneBCommandPlaceNext(&LAYOUT, 0x8113);
}

void abi_Scene58CommandPlaceNext(void)
{
    exit_shim(Scene58CommandPlaceNext());
}

/*
 * @routine 267B:12D Scene58CommandPlaceFrame
 * @run 6B:812D
 * WHAT IT DOES: A script command: an object placed, its frame and sprite set from the script; its kind is left 0.
 * HOW: The copy of EndingPlaceObject, then the stores; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandPlaceFrame(void)
{
    return SceneBCommandPlaceFrame(&LAYOUT, 0x812D);
}

void abi_Scene58CommandPlaceFrame(void)
{
    exit_shim(Scene58CommandPlaceFrame());
}

/*
 * @routine 267B:142 Scene58CommandPlaceTimed
 * @run 6B:8142
 * WHAT IT DOES: A script command: an object placed, its frame, sprite set, start argument and work_d from the script, kind 3.
 * HOW: The copy of EndingPlaceObject, then the stores; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandPlaceTimed(void)
{
    return SceneBCommandPlaceTimed(&LAYOUT, 0x8142);
}

void abi_Scene58CommandPlaceTimed(void)
{
    exit_shim(Scene58CommandPlaceTimed());
}

/*
 * @routine 267B:168 Scene58CommandPlaceWorked
 * @run 6B:8168
 * WHAT IT DOES: A script command: an object placed, its work_e, sprite set, start argument and work_d from the script, kind 4.
 * HOW: The copy of EndingPlaceObject, then the stores; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandPlaceWorked(void)
{
    return SceneBCommandPlaceWorked(&LAYOUT, 0x8168);
}

void abi_Scene58CommandPlaceWorked(void)
{
    exit_shim(Scene58CommandPlaceWorked());
}

/*
 * @routine 267B:1B4 Scene58CommandPlaceTitleObjectALow
 * @run 6B:81B4
 * WHAT IT DOES: A script command: slot $2E cleared and started (kind 1) with the four operands as start argument and work_d to work_f, Y low $40 and X high stepped down one.
 * HOW: ClearObject, INC of the kind, the stores, DEC of X high; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandPlaceTitleObjectALow(void)
{
    return SceneBCommandPlaceTitleObjectALow(&LAYOUT, 0x81B4);
}

void abi_Scene58CommandPlaceTitleObjectALow(void)
{
    exit_shim(Scene58CommandPlaceTitleObjectALow());
}

/*
 * @routine 267B:1DE Scene58CommandPlaceTitleObjectB
 * @run 6B:81DE
 * WHAT IT DOES: A script command: slot $2F cleared and started (kind 1) with the four operands as start argument and work_d to work_f, Y low $D8.
 * HOW: ClearObject, INC of the kind, the stores; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandPlaceTitleObjectB(void)
{
    return SceneBCommandPlaceTitleObjectB(&LAYOUT, 0x81DE);
}

void abi_Scene58CommandPlaceTitleObjectB(void)
{
    exit_shim(Scene58CommandPlaceTitleObjectB());
}

/*
 * @routine 267B:205 Scene58CommandBranchOnProgress
 * @run 6B:8205
 * WHAT IT DOES: A script command: when the low four bits of ProgressCharacter are all set, the script goes on at $9225 and zRasterStage is cleared.
 * HOW: AND and CMP of $0F; the pointer set and STZ; the advance either way (the step past the command at the pointer).
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandBranchOnProgress(void)
{
    return SceneBCommandBranchOnProgress(&LAYOUT);
}

void abi_Scene58CommandBranchOnProgress(void)
{
    exit_shim(Scene58CommandBranchOnProgress());
}

/*
 * @routine 267B:21B Scene58CommandLoadBanks
 * @run 6B:821B
 * WHAT IT DOES: A script command: reads four banks of the ADPCM memory into the banks from zRamBaseBank + $10 on, two at a time at MPR5 and MPR6.
 * HOW: As EndingCommandLoadBanks: for zTemp00 0 to 3, TAM of the two banks, the BIOS registers set, ad_read; the advance.
 * WAITS: yes, through ad_read, which waits for the ADPCM to be idle.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene58CommandLoadBanks(void)
{
    return SceneBCommandLoadBanks(&LAYOUT, 0x821B);
}

void abi_Scene58CommandLoadBanks(void)
{
    exit_shim(Scene58CommandLoadBanks());
}

/*
 * @routine 267B:250 Scene58ObjectsStep
 * @run 6B:8250
 * WHAT IT DOES: Runs every object: each slot with a kind through this overlay's handler for it.
 * HOW: X 0 to $57; the dispatch for each kind not 0.
 * WAITS: no.
 * Entry: nothing. Exit: A the last kind or handler's A, X $58, N and Z from the CPX, C set; Y and V as the last handler left them.
 */
Exit Scene58ObjectsStep(void)
{
    return SceneBObjectsStep(&LAYOUT);
}

void abi_Scene58ObjectsStep(void)
{
    exit_shim(Scene58ObjectsStep());
}

/*
 * @routine 267B:25F Scene58DispatchKind
 * @run 6B:825F
 * @stop 826F
 * WHAT IT DOES: Jumps to the handler of the object's kind, from this overlay's table at $826F.
 * HOW: The kind less one, doubled, indexes the table; the word through zTemp00 and zTemp01.
 * WAITS: no.
 * Entry: A the kind (not 0), X the object. Exit: as the handler, entered with A the handler's high byte and Y the index.
 */
ObjExit Scene58DispatchKind(uint8_t kind, uint8_t object)
{
    return SceneBDispatchKind(&LAYOUT, kind, object);
}

void abi_Scene58DispatchKind(void)
{
    scene_obj_shim(Scene58DispatchKind(cpu.a, cpu.x));
}

/* ------------------------------------------------------------------ object handlers */

/*
 * @routine 267B:63D Scene58CycleLists
 * @run 6B:863D
 * WHAT IT DOES: An object handler: starts list channel 0 on lists $0C, $0D and $0E in turn, one every nine frames.
 * HOW: SceneBCycleLists: work_e the list, work_d the count; StartListChannel0Kind2.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them; X the object, C from the list's compare after a start.
 */
ObjExit Scene58CycleLists(uint8_t object)
{
    return SceneBCycleLists(0x8668, 0x09, 0x0F, object);
}

void abi_Scene58CycleLists(void)
{
    scene_obj_shim(Scene58CycleLists(cpu.x));
}

/*
 * @routine 267B:66B Scene58Shake
 * @run 6B:866B
 * @stop 8686
 * WHAT IT DOES: An object handler: shakes the screen, every eighth frame setting the scroll's X high and Y high from
 * the tables at $8686 and $868A by bits 3 and 4 of zFrameTick.
 * HOW: SceneBShake.
 * WAITS: no.
 * Entry: X the object. Exit: A the last value, N and Z from it; Y the index and C clear after a store; X kept.
 */
ObjExit Scene58Shake(uint8_t object)
{
    return SceneBShake(0x8686, 0x868A, object);
}

void abi_Scene58Shake(void)
{
    scene_obj_shim(Scene58Shake(cpu.x));
}

/*
 * @routine 267B:698 Scene58SlideThenList
 * @run 6B:8698
 * WHAT IT DOES: An object handler: Scene58Slide, then the frame list work_e names.
 * HOW: JSR Scene58Slide; JMP Scene58FrameList with A work_e.
 * WAITS: no.
 * Entry: X the object. Exit: as Scene58FrameList, V as the slide left it.
 */
ObjExit Scene58SlideThenList(uint8_t object)
{
    ObjExit slide, list;
    uint8_t number;

    jsr_push(0x869B);
    slide = Scene58Slide(object);
    jsr_pop();
    number = RD8(OBJ->work_e[object]);
    list = Scene58FrameList(number, object);
    return SceneBAfter(list, slide);
}

void abi_Scene58SlideThenList(void)
{
    scene_obj_shim(Scene58SlideThenList(cpu.x));
}

/*
 * @routine 267B:6A1 Scene58PlayListOnce
 * @run 6B:86A1
 * WHAT IT DOES: An object handler: plays the start argument's frame list once, then moves on a state unless the
 * argument is negative.
 * HOW: ScenePlayListOnce with this overlay's lists ($8722).
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene58PlayListOnce(uint8_t object)
{
    return ScenePlayListOnce(0x8722, 0x86AC, object);
}

void abi_Scene58PlayListOnce(void)
{
    scene_obj_shim(Scene58PlayListOnce(cpu.x));
}

/*
 * @routine 267B:6B7 Scene58Slide
 * @run 6B:86B7
 * @stop 86E7
 * WHAT IT DOES: An object handler: takes the speeds its start argument names from the tables at $86E7, $86F2,
 * $86FD and $8708, then moves for work_d frames and moves on a state.
 * HOW: SceneSlide.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene58Slide(uint8_t object)
{
    static const uint16_t speeds[4] = { 0x86E7, 0x86F2, 0x86FD, 0x8708 };

    return SceneSlide(speeds, 0x86E1, object);
}

void abi_Scene58Slide(void)
{
    scene_obj_shim(Scene58Slide(cpu.x));
}

/*
 * @routine 267B:713 Scene58FrameList
 * @run 6B:8713
 * @stop 8722
 * WHAT IT DOES: Steps an object through the frame list this overlay's table at $8722 names for A.
 * HOW: ObjectFrameListAt.
 * WAITS: no.
 * Entry: A the list, X the object. Exit: as StepObjectFrames.
 */
ObjExit Scene58FrameList(uint8_t list, uint8_t object)
{
    return ObjectFrameListAt(list, object, 0x8722);
}

void abi_Scene58FrameList(void)
{
    scene_obj_shim(Scene58FrameList(cpu.a, cpu.x));
}

/*
 * @routine 267B:28B Scene58SpawnCycleA
 * @run 6B:828B
 * @stop 82E8
 * WHAT IT DOES: An object handler: spawns objects in a cycle of six slots (the table at $82E8), each as kind 1 at Y
 * $170, their work_d and the waits between them from the step table at $82EE; at the table's end it clears itself.
 * HOW: SceneBSpawnCycle: work_e the wait, work_f the slot, work_d the step; ClearObjectInY.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene58SpawnCycleA(uint8_t object)
{
    return SceneBSpawnCycle(0x82E8, 0x82EE, 0x82B2, object);
}

void abi_Scene58SpawnCycleA(void)
{
    scene_obj_shim(Scene58SpawnCycleA(cpu.x));
}

/*
 * @routine 267B:389 Scene58LoadPatterns
 * @run 6B:8389
 * WHAT IT DOES: An object handler: copies five rows of $80 bytes from $B980, $B380, $B200, $B680 and $B180 (the bank
 * Scene58Step maps at $A000) to VRAM from $7000, then clears itself.
 * HOW: SelectVramWriteAddress, ST1 and ST2, SelectVramData, five TIA to the VDC's data port; ClearObject.
 * WAITS: no.
 * Entry: X the object, Y the dispatch's index ($18: the handler of kind 13). Exit: A 2, N and Z from it; X, Y and C
 * kept.
 */
ObjExit Scene58LoadPatterns(uint8_t object, uint8_t y)
{
    return SceneBLoadPatterns(0x8389, y, object);
}

void abi_Scene58LoadPatterns(void)
{
    scene_obj_shim(Scene58LoadPatterns(cpu.x, cpu.y));
}

/*
 * @routine 267B:3B9 Scene58Swing
 * @run 6B:83B9
 * WHAT IT DOES: An object handler: swings object $10 left, right, left, right and left (X speed -$10 and +$10, turning
 * below X low $40 and at $80), then sends it off left at -$60.
 * HOW: SceneBSwing; ApplyVelocityX.
 * WAITS: no.
 * Entry: X the object. Exit: as the last compare left them, V from the move; from state 5 on, A and C from the move.
 */
ObjExit Scene58Swing(uint8_t object)
{
    static const uint8_t lo[6] = { 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0xA0 };
    static const uint8_t hi[6] = { 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF };

    return SceneBSwing(0x83B9, lo, hi, object);
}

void abi_Scene58Swing(void)
{
    scene_obj_shim(Scene58Swing(cpu.x));
}

/*
 * @routine 267B:468 Scene58ShowObjectsAndClear
 * @run 6B:8468
 * WHAT IT DOES: An object handler: gives objects $13, $11 and $0F sprite attribute $10, then clears itself.
 * HOW: Three stores; ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A $10, N and Z from it; X, Y and C kept.
 */
ObjExit Scene58ShowObjectsAndClear(uint8_t object)
{
    WR8(OBJ->sprite_attr[0x13], 0x10);
    WR8(OBJ->sprite_attr[0x11], 0x10);
    WR8(OBJ->sprite_attr[0x0F], 0x10);
    ClearObject(object);                /* the tail JMP: A, X, Y and the flags kept */
    return scene_kept(0x10, 0x10);
}

void abi_Scene58ShowObjectsAndClear(void)
{
    scene_obj_shim(Scene58ShowObjectsAndClear(cpu.x));
}

/*
 * @routine 267B:476 Scene58SpawnCycleB
 * @run 6B:8476
 * @stop 84D3
 * WHAT IT DOES: An object handler: as Scene58SpawnCycleA with the slots at $84D3 and the steps at $84D9.
 * HOW: SceneBSpawnCycle.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene58SpawnCycleB(uint8_t object)
{
    return SceneBSpawnCycle(0x84D3, 0x84D9, 0x849D, object);
}

void abi_Scene58SpawnCycleB(void)
{
    scene_obj_shim(Scene58SpawnCycleB(cpu.x));
}

/*
 * @routine 267B:574 Scene58ScrollLayers
 * @run 6B:8574
 * @stop 85A7
 * WHAT IT DOES: An object handler: sets zRasterEffect to $1E, then every frame scrolls layers 1 to 4 by their 24-bit speeds (the tables at $85A6, $85AA and $85AE by layer).
 * HOW: SceneBScrollLayers: three ADC a layer to ScrollXFrac, ScrollXLo and ScrollXHi.
 * WAITS: no.
 * Entry: X the object. Exit: state 0: A $1E, N and Z from the state. Then: A the last high byte, N and Z from X, C set, V from the last add; Y kept.
 */
ObjExit Scene58ScrollLayers(uint8_t object)
{
    return SceneBScrollLayers(0x85A6, 0x85AA, 0x85AE, 0x1E, 5, object);
}

void abi_Scene58ScrollLayers(void)
{
    scene_obj_shim(Scene58ScrollLayers(cpu.x));
}

/*
 * @routine 267B:5B3 Scene58Sparkles
 * @run 6B:85B3
 * WHAT IT DOES: An object handler: every eight frames starts a sparkle (Scene58SpawnSparkle); when the slot it picks is taken it tries again the next frame.
 * HOW: work_d the count; the spawn's carry.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them; after a try, C, V and Y the spawn's.
 */
ObjExit Scene58Sparkles(uint8_t object)
{
    return SceneBSparkles(0x85B3, 0x85CF, 0x860F, 0x27, object);
}

void abi_Scene58Sparkles(void)
{
    scene_obj_shim(Scene58Sparkles(cpu.x));
}

/*
 * @routine 267B:5CF Scene58SpawnSparkle
 * @run 6B:85CF
 * @stop 860F
 * WHAT IT DOES: Starts a sparkle in one of sixteen slots picked at random (the table at $860F) when it is free: X low at random, Y $FF00 plus 0 to $7F at random, frame $28 to $36 at random, sprite set 3, kind 8.
 * HOW: NextRandom four times; the stores.
 * WAITS: no.
 * Entry: X the object (kept). Exit: started: A 8, C set, V from the frame's add; taken: A the slot's kind, C clear, V NextRandom's; Y the slot, N and Z from A.
 */
ObjExit Scene58SpawnSparkle(uint8_t object)
{
    return SceneBSpawnSparkle(0x85CF, 0x860F, 0x27, object);
}

void abi_Scene58SpawnSparkle(void)
{
    scene_obj_shim(Scene58SpawnSparkle(cpu.x));
}

/*
 * @routine 267B:61F Scene58SparkleFall
 * @run 6B:861F
 * WHAT IT DOES: An object handler (the sparkle's): falls, $40 more Y speed a frame, and is cleared once its Y high is 1 or more.
 * HOW: ADC to the Y speed; ApplyVelocityY; ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A its Y high, N and Z from it, C and V from the move; X and Y kept.
 */
ObjExit Scene58SparkleFall(uint8_t object)
{
    return SceneBSparkleFall(0x861F, object);
}

void abi_Scene58SparkleFall(void)
{
    scene_obj_shim(Scene58SparkleFall(cpu.x));
}
