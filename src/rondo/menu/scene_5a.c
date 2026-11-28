/*
 * The cutscene overlay of sectors $2703 to $2706 (bank $6B at $8000; scenes $5A and $5B, after a stage): the script
 * engine of scene_engine_b.c at this overlay's places (lengths $8043, commands $8080, object kinds $822A), and this
 * overlay's own object handlers. Its $8000 step, $8032 wait for RUN and $807B end command are the bytes of the $58
 * overlay's at those places or of its end command, and its objects' loop and dispatch ($820B, $821A) the bytes of
 * the $0C overlay's: their C serves these.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "scene_engine.h"
#include "scene_engine_b.h"
#include "scene_58.h"
#include "scene_0c.h"
#include "scene_5a.h"

static bool scene_5a_command(uint16_t target, uint8_t index, bool carry, Exit *out);

static const SceneLayoutB LAYOUT = {
    0x8063, 0x820B, Scene0CObjectsStep, 0x8043, 0x8080, 0x822A, 0x8162, 0x816C, scene_5a_command, scene_5a_handler
};

/* This overlay's script commands by their addresses, for the engine's step. */
static bool scene_5a_command(uint16_t target, uint8_t index, bool carry, Exit *out)
{
    switch (target) {
    case 0x80BA: *out = Scene5ACommandStartListAndClear(); return true;
    case 0x80D6: *out = Scene5ACommandStartScript1(); return true;
    case 0x807B: *out = Scene58CommandEnd(index, carry); return true;
    case 0x80FE: *out = Scene5ACommandPlaceKind(); return true;
    case 0x8127: *out = Scene5ACommandPlaceFrame(); return true;
    case 0x80EE: *out = Scene5ACommandPlayAdpcm(); return true;
    case 0x80DE: *out = Scene5ACommandStartScript2(); return true;
    case 0x80E6: *out = Scene5ACommandStartScript3(); return true;
    case 0x80F6: *out = Scene5ACommandCdPlay(index); return true;
    case 0x8188: *out = Scene5ACommandPlaceTitleObjectA(); return true;
    case 0x81AF: *out = Scene5ACommandPlaceTitleObjectB(); return true;
    case 0x80A2: *out = Scene5ACommandSubModeTwo(); return true;
    case 0x80CE: *out = Scene5ACommandStartList(); return true;
    case 0x810D: *out = Scene5ACommandPlaceNext(); return true;
    case 0x813C: *out = Scene5ACommandPlaceTimed(); return true;
    case 0x81D6: *out = Scene5ACommandLoadBanks(); return true;
    case 0x80A8: *out = Scene5ACommandSetScroll(); return true;
    default: return false;
    }
}

/* This overlay's object handlers by their addresses, for the $0C overlay's dispatch (whose bytes this overlay holds at
 * $821A) when this overlay is the one loaded. */
bool scene_5a_handler(uint16_t target, uint8_t object, ObjExit *out)
{
    switch (target) {
    case 0x82A4: *out = SceneBLoadPatterns(0x82A4, 0x18, object); return true;   /* Scene58LoadPatterns's bytes; kind 13, Y $18 */
    case 0x82D4: *out = Scene5ASwing(object); return true;
    case 0x8391: *out = Scene5ASpawnCycle(object); return true;
    case 0x8383: *out = Scene58ShowObjectsAndClear(object); return true;
    case 0x8558: *out = Scene5ACycleLists(object); return true;
    case 0x8586: *out = Scene5AShake(object); return true;
    case 0x85B3: *out = Scene5ASlideThenList(object); return true;
    case 0x85BC: *out = Scene5APlayListOnce(object); return true;
    case 0x85D2: *out = Scene5ASlide(object); return true;
    default: return false;
    }
}

/*
 * @routine 2703:014 Scene5ABegin
 * @run 6B:8014
 * WHAT IT DOES: Sub-mode 0: starts the scene: the game state wiped, the script from $8D9C, its frame counter $FFFF, the sub-mode on.
 * HOW: ResetGameState, then Scene5CBegin's stores with this script.
 * WAITS: yes, through ResetGameState (the vertical blank).
 * Entry: X the doubled sub-mode, Y kept. Exit: A $FF, X 0, Y $FF, N and Z from the new sub-mode; C kept.
 */
Exit Scene5ABegin(uint8_t kept_x, uint8_t kept_y)
{
    return SceneBBegin(0x8014, 0x9C, 0x8D, kept_x, kept_y);
}

void abi_Scene5ABegin(void)
{
    exit_shim(Scene5ABegin(cpu.x, cpu.y));
}

/*
 * @routine 2703:028 Scene5AFrame
 * @run 6B:8028
 * WHAT IT DOES: Sub-mode 1: one frame of the scene: the frame counted, the script's command run if due, every object run.
 * HOW: The copy of EndingFrameTick at $803C, the script step, the objects' loop.
 * WAITS: no.
 * Entry: nothing. Exit: as the objects' loop, Y and V the step's where the loop leaves them.
 */
Exit Scene5AFrame(void)
{
    return SceneBFrame(&LAYOUT, 0x8028);
}

void abi_Scene5AFrame(void)
{
    exit_shim(Scene5AFrame());
}

/*
 * @routine 2703:054 Scene5AScriptAdvance
 * @run 6B:8054
 * WHAT IT DOES: Steps the script past its command and runs on into the script step.
 * HOW: The command's length from this overlay's table at $8043 added to the pointer.
 * WAITS: no.
 * Entry: nothing. Exit: as the script step, X the command stepped past when it left X alone, V from the add when it set none.
 */
Exit Scene5AScriptAdvance(void)
{
    return SceneBScriptAdvance(&LAYOUT);
}

void abi_Scene5AScriptAdvance(void)
{
    exit_shim(Scene5AScriptAdvance());
}

/*
 * @routine 2703:063 Scene5AScriptStep
 * @run 6B:8063
 * WHAT IT DOES: Runs the script's command when its two time bytes equal the frame counter.
 * HOW: The times compared; the command doubled indexes this overlay's table at $8080, entered with X the index and Y 3.
 * WAITS: no.
 * Entry: nothing. Exit: a time byte differs: that byte, N, Z and C from comparing it, Y 1 or 2, X kept; else as the command.
 */
Exit Scene5AScriptStep(void)
{
    return SceneBScriptStep(&LAYOUT);
}

void abi_Scene5AScriptStep(void)
{
    exit_shim(Scene5AScriptStep());
}

/*
 * @routine 2703:0A2 Scene5ACommandSubModeTwo
 * @run 6B:80A2
 * WHAT IT DOES: A script command: the sub-mode set to 2, and the script steps on.
 * HOW: A store; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandSubModeTwo(void)
{
    return SceneBCommandSubModeTwo(&LAYOUT);
}

void abi_Scene5ACommandSubModeTwo(void)
{
    exit_shim(Scene5ACommandSubModeTwo());
}

/*
 * @routine 2703:0A8 Scene5ACommandSetScroll
 * @run 6B:80A8
 * WHAT IT DOES: A script command: the screen's scroll X high from the operand's high four bits, Y high from its low four.
 * HOW: PHA, four LSR, PLA, AND; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandSetScroll(void)
{
    return SceneBCommandSetScroll(&LAYOUT);
}

void abi_Scene5ACommandSetScroll(void)
{
    exit_shim(Scene5ACommandSetScroll());
}

/*
 * @routine 2703:0BA Scene5ACommandStartListAndClear
 * @run 6B:80BA
 * WHAT IT DOES: A script command: list channel 0 started with the operand's list, the screen's state, every object and the scroll block ($EE bytes from ScrollXFrac) cleared.
 * HOW: StartListChannel0Kind2, ClearScreenState, the copy of EndingClearObjects at $8162, a TAI of $EE zeros to ScrollXFrac; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandStartListAndClear(void)
{
    return SceneBCommandStartListAndClear(&LAYOUT, 0x80BA);
}

void abi_Scene5ACommandStartListAndClear(void)
{
    exit_shim(Scene5ACommandStartListAndClear());
}

/*
 * @routine 2703:0CE Scene5ACommandStartList
 * @run 6B:80CE
 * WHAT IT DOES: A script command: list channel 0 started with the operand's list.
 * HOW: StartListChannel0Kind2; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandStartList(void)
{
    return SceneBCommandStartList(&LAYOUT, 0x80CE);
}

void abi_Scene5ACommandStartList(void)
{
    exit_shim(Scene5ACommandStartList());
}

/*
 * @routine 2703:0D6 Scene5ACommandStartScript1
 * @run 6B:80D6
 * WHAT IT DOES: A script command: script slot 0 started with the operand, kind 1.
 * HOW: StartScriptSlot0Kind1; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandStartScript1(void)
{
    return SceneBCommandStartScript1(&LAYOUT, 0x80D6);
}

void abi_Scene5ACommandStartScript1(void)
{
    exit_shim(Scene5ACommandStartScript1());
}

/*
 * @routine 2703:0DE Scene5ACommandStartScript2
 * @run 6B:80DE
 * WHAT IT DOES: A script command: script slot 0 started with the operand, kind 2.
 * HOW: StartScriptSlot0Kind2; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandStartScript2(void)
{
    return SceneBCommandStartScript2(&LAYOUT, 0x80DE);
}

void abi_Scene5ACommandStartScript2(void)
{
    exit_shim(Scene5ACommandStartScript2());
}

/*
 * @routine 2703:0E6 Scene5ACommandStartScript3
 * @run 6B:80E6
 * WHAT IT DOES: A script command: script slot 0 started with the operand, kind 3.
 * HOW: StartScriptSlot0Kind3; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandStartScript3(void)
{
    return SceneBCommandStartScript3(&LAYOUT, 0x80E6);
}

void abi_Scene5ACommandStartScript3(void)
{
    exit_shim(Scene5ACommandStartScript3());
}

/*
 * @routine 2703:0EE Scene5ACommandPlayAdpcm
 * @run 6B:80EE
 * WHAT IT DOES: A script command: the ADPCM sound the operand names played from the disc.
 * HOW: AdpcmPlayFromDisc; the advance.
 * WAITS: yes, on the drive and the ADPCM.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandPlayAdpcm(void)
{
    return SceneBCommandPlayAdpcm(&LAYOUT, 0x80EE);
}

void abi_Scene5ACommandPlayAdpcm(void)
{
    exit_shim(Scene5ACommandPlayAdpcm());
}

/*
 * @routine 2703:0F6 Scene5ACommandCdPlay
 * @run 6B:80F6
 * WHAT IT DOES: A script command: the CD track the operand names played once.
 * HOW: CdPlayOnce with X the doubled command; the advance.
 * WAITS: yes, on the drive.
 * Entry: from the script step: X the doubled command, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandCdPlay(uint8_t index)
{
    return SceneBCommandCdPlay(&LAYOUT, 0x80F6, index);
}

void abi_Scene5ACommandCdPlay(void)
{
    exit_shim(Scene5ACommandCdPlay(cpu.x));
}

/*
 * @routine 2703:0FE Scene5ACommandPlaceKind
 * @run 6B:80FE
 * WHAT IT DOES: A script command: an object placed at the script's X and Y and given the kind that follows.
 * HOW: The copy of EndingPlaceObject, then the kind; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandPlaceKind(void)
{
    return SceneBCommandPlaceKind(&LAYOUT, 0x80FE);
}

void abi_Scene5ACommandPlaceKind(void)
{
    exit_shim(Scene5ACommandPlaceKind());
}

/*
 * @routine 2703:10D Scene5ACommandPlaceNext
 * @run 6B:810D
 * WHAT IT DOES: A script command: an object placed, its start argument and sprite set from the script, kind 2.
 * HOW: The copy of EndingPlaceObject, then the stores; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandPlaceNext(void)
{
    return SceneBCommandPlaceNext(&LAYOUT, 0x810D);
}

void abi_Scene5ACommandPlaceNext(void)
{
    exit_shim(Scene5ACommandPlaceNext());
}

/*
 * @routine 2703:127 Scene5ACommandPlaceFrame
 * @run 6B:8127
 * WHAT IT DOES: A script command: an object placed, its frame and sprite set from the script; its kind is left 0.
 * HOW: The copy of EndingPlaceObject, then the stores; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandPlaceFrame(void)
{
    return SceneBCommandPlaceFrame(&LAYOUT, 0x8127);
}

void abi_Scene5ACommandPlaceFrame(void)
{
    exit_shim(Scene5ACommandPlaceFrame());
}

/*
 * @routine 2703:13C Scene5ACommandPlaceTimed
 * @run 6B:813C
 * WHAT IT DOES: A script command: an object placed, its frame, sprite set, start argument and work_d from the script, kind 3.
 * HOW: The copy of EndingPlaceObject, then the stores; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandPlaceTimed(void)
{
    return SceneBCommandPlaceTimed(&LAYOUT, 0x813C);
}

void abi_Scene5ACommandPlaceTimed(void)
{
    exit_shim(Scene5ACommandPlaceTimed());
}

/*
 * @routine 2703:188 Scene5ACommandPlaceTitleObjectA
 * @run 6B:8188
 * WHAT IT DOES: A script command: slot $2E cleared and started (kind 1) with the four operands as start argument and work_d to work_f, Y low $C8.
 * HOW: ClearObject, INC of the kind, the stores; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandPlaceTitleObjectA(void)
{
    return SceneBCommandPlaceTitleObjectA(&LAYOUT, 0x8188);
}

void abi_Scene5ACommandPlaceTitleObjectA(void)
{
    exit_shim(Scene5ACommandPlaceTitleObjectA());
}

/*
 * @routine 2703:1AF Scene5ACommandPlaceTitleObjectB
 * @run 6B:81AF
 * WHAT IT DOES: A script command: slot $2F cleared and started (kind 1) with the four operands as start argument and work_d to work_f, Y low $D8.
 * HOW: ClearObject, INC of the kind, the stores; the advance.
 * WAITS: no.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandPlaceTitleObjectB(void)
{
    return SceneBCommandPlaceTitleObjectB(&LAYOUT, 0x81AF);
}

void abi_Scene5ACommandPlaceTitleObjectB(void)
{
    exit_shim(Scene5ACommandPlaceTitleObjectB());
}

/*
 * @routine 2703:1D6 Scene5ACommandLoadBanks
 * @run 6B:81D6
 * WHAT IT DOES: A script command: reads four banks of the ADPCM memory into the banks from zRamBaseBank + $10 on, two at a time at MPR5 and MPR6.
 * HOW: As EndingCommandLoadBanks: for zTemp00 0 to 3, TAM of the two banks, the BIOS registers set, ad_read; the advance.
 * WAITS: yes, through ad_read, which waits for the ADPCM to be idle.
 * Entry: from the script step, Y 3. Exit: as the advance.
 */
Exit Scene5ACommandLoadBanks(void)
{
    return SceneBCommandLoadBanks(&LAYOUT, 0x81D6);
}

void abi_Scene5ACommandLoadBanks(void)
{
    exit_shim(Scene5ACommandLoadBanks());
}

/* ------------------------------------------------------------------ object handlers */

/*
 * @routine 2703:558 Scene5ACycleLists
 * @run 6B:8558
 * WHAT IT DOES: An object handler: starts list channel 0 on lists $0C to $0F in turn, one every $11 frames.
 * HOW: SceneBCycleLists: work_e the list, work_d the count; StartListChannel0Kind2.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them; X the object, C from the list's compare after a start.
 */
ObjExit Scene5ACycleLists(uint8_t object)
{
    return SceneBCycleLists(0x8583, 0x11, 0x10, object);
}

void abi_Scene5ACycleLists(void)
{
    scene_obj_shim(Scene5ACycleLists(cpu.x));
}

/*
 * @routine 2703:586 Scene5AShake
 * @run 6B:8586
 * @stop 85A1
 * WHAT IT DOES: An object handler: shakes the screen, every eighth frame setting the scroll's X high and Y high from
 * the tables at $85A1 and $85A5 by bits 3 and 4 of zFrameTick.
 * HOW: SceneBShake.
 * WAITS: no.
 * Entry: X the object. Exit: A the last value, N and Z from it; Y the index and C clear after a store; X kept.
 */
ObjExit Scene5AShake(uint8_t object)
{
    return SceneBShake(0x85A1, 0x85A5, object);
}

void abi_Scene5AShake(void)
{
    scene_obj_shim(Scene5AShake(cpu.x));
}

/*
 * @routine 2703:5B3 Scene5ASlideThenList
 * @run 6B:85B3
 * WHAT IT DOES: An object handler: Scene5ASlide, then the frame list work_e names.
 * HOW: JSR Scene5ASlide; JMP Scene5AFrameList with A work_e.
 * WAITS: no.
 * Entry: X the object. Exit: as Scene5AFrameList, V as the slide left it.
 */
ObjExit Scene5ASlideThenList(uint8_t object)
{
    ObjExit slide, list;
    uint8_t number;

    jsr_push(0x85B6);
    slide = Scene5ASlide(object);
    jsr_pop();
    number = RD8(OBJ->work_e[object]);
    list = Scene5AFrameList(number, object);
    return SceneBAfter(list, slide);
}

void abi_Scene5ASlideThenList(void)
{
    scene_obj_shim(Scene5ASlideThenList(cpu.x));
}

/*
 * @routine 2703:5BC Scene5APlayListOnce
 * @run 6B:85BC
 * WHAT IT DOES: An object handler: plays the start argument's frame list once, then moves on a state unless the
 * argument is negative.
 * HOW: ScenePlayListOnce with this overlay's lists ($863D).
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene5APlayListOnce(uint8_t object)
{
    return ScenePlayListOnce(0x863D, 0x85C7, object);
}

void abi_Scene5APlayListOnce(void)
{
    scene_obj_shim(Scene5APlayListOnce(cpu.x));
}

/*
 * @routine 2703:5D2 Scene5ASlide
 * @run 6B:85D2
 * @stop 8602
 * WHAT IT DOES: An object handler: takes the speeds its start argument names from the tables at $8602, $860D,
 * $8618 and $8623, then moves for work_d frames and moves on a state.
 * HOW: SceneSlide.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene5ASlide(uint8_t object)
{
    static const uint16_t speeds[4] = { 0x8602, 0x860D, 0x8618, 0x8623 };

    return SceneSlide(speeds, 0x85FC, object);
}

void abi_Scene5ASlide(void)
{
    scene_obj_shim(Scene5ASlide(cpu.x));
}

/*
 * @routine 2703:62E Scene5AFrameList
 * @run 6B:862E
 * @stop 863D
 * WHAT IT DOES: Steps an object through the frame list this overlay's table at $863D names for A.
 * HOW: ObjectFrameListAt.
 * WAITS: no.
 * Entry: A the list, X the object. Exit: as StepObjectFrames.
 */
ObjExit Scene5AFrameList(uint8_t list, uint8_t object)
{
    return ObjectFrameListAt(list, object, 0x863D);
}

void abi_Scene5AFrameList(void)
{
    scene_obj_shim(Scene5AFrameList(cpu.a, cpu.x));
}

/*
 * @routine 2703:2D4 Scene5ASwing
 * @run 6B:82D4
 * WHAT IT DOES: An object handler: swings object $10 along X with the X speeds -6, -$10, -8, +8 and -8 (turning below
 * X low $40 in states 0, 2 and 4 and at $80 or more in 1 and 3), then sends it off left at -$60.
 * HOW: SceneBSwing; ApplyVelocityX.
 * WAITS: no.
 * Entry: X the object. Exit: as the last compare left them, V from the move; from state 5 on, A and C from the move.
 */
ObjExit Scene5ASwing(uint8_t object)
{
    static const uint8_t lo[6] = { 0xFA, 0xF0, 0xF8, 0x08, 0xF8, 0xA0 };
    static const uint8_t hi[6] = { 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF };

    return SceneBSwing(0x82D4, lo, hi, object);
}

void abi_Scene5ASwing(void)
{
    scene_obj_shim(Scene5ASwing(cpu.x));
}

/*
 * @routine 2703:391 Scene5ASpawnCycle
 * @run 6B:8391
 * @stop 83EE
 * WHAT IT DOES: An object handler: as Scene58SpawnCycleA with the slots at $83EE and the steps at $83F4.
 * HOW: SceneBSpawnCycle.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene5ASpawnCycle(uint8_t object)
{
    return SceneBSpawnCycle(0x83EE, 0x83F4, 0x83B8, object);
}

void abi_Scene5ASpawnCycle(void)
{
    scene_obj_shim(Scene5ASpawnCycle(cpu.x));
}
