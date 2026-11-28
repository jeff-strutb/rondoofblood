/*
 * The cutscene overlay of sector $224B (bank $6B at $8000; scenes $02 and $03): the shared script engine
 * (scene_engine.c) at this overlay's places, $8 past the $5C overlay's, and this overlay's own begin and
 * object handlers.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "scene_engine.h"
#include "scene_02.h"


static void scene_entry_shim(uint8_t high, uint8_t object, uint8_t index)
{
    cpu.a = high;
    cpu.x = object;
    cpu.y = index;
    abi_nz(high);
}

/*
 * @routine 224B:00B Scene02Begin
 * @run 6B:800B
 * WHAT IT DOES: Starts the scene of $02 and $03: bit 0 of ProgressCharacter, the script at $8383.
 * HOW: ResetGameState; an ORA into ProgressCharacter; the script pointer, the frame counter $FFFF, the sub-mode up.
 * WAITS: yes, through ResetGameState (the vertical blank).
 * Entry: X the doubled sub-mode and Y kept. Exit: A $FF, X 0, N and Z from the new state; Y and C from the character
 * where the script is picked by it, else Y $FF and C kept.
 */
Exit Scene02Begin(uint8_t kept_x, uint8_t kept_y)
{
    Exit exit = { 0 };

    jsr_push(0x800E);
    ResetGameState(wait_regs_xy_shim(kept_x, kept_y));
    jsr_pop();
    wr8(ProgressCharacter, (uint8_t)(rd8(ProgressCharacter) | 0x01));
    WR8(ZP->script_ptr, 0x83);
    WR8(ZP->script_ptr_hi, 0x83);
    WR8(ZP->wait_timer3, 0xFF);
    WR8(ZP->wait_timer4, 0xFF);
    exit.a = 0xFF;
    exit.x = 0;
    exit.y = 0xFF;
    exit.c_kept = true;
    exit.nz = INC8(ZP->game_sub_mode);
    return exit;
}

void abi_Scene02Begin(void)
{
    exit_shim(Scene02Begin(cpu.x, cpu.y));
}

/*
 * @routine 224B:23E Scene02Descend
 * @run 6B:823E
 * WHAT IT DOES: Lowers objects 0 and 2's Y by one a frame (16 bits); at object 2's Y low $B1 object $10 goes on a state, at $91 object 2's frame goes up, at $71 the object goes on.
 * HOW: DECs with the borrow by compare with $FF.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene02Descend(uint8_t object)
{
    uint8_t value;
    ObjExit result;

    DEC8(OBJ->y_lo[0]);
    value = RD8(OBJ->y_lo[0]);
    if (value == 0xFF) {
        DEC8(OBJ->y_hi[0]);
    }
    DEC8(OBJ->y_lo[2]);
    value = RD8(OBJ->y_lo[2]);
    if (value == 0xFF) {
        DEC8(OBJ->y_hi[2]);
    }
    value = RD8(OBJ->y_lo[2]);
    result = scene_kept(value, 0);
    result.exit.c_kept = false;
    result.exit.c = value >= 0xB1;
    result.exit.nz = (uint8_t)(value - 0xB1);
    if (value == 0xB1) {
        result.exit.nz = INC8(OBJ->state[0x10]);
        return result;
    }
    result.exit.c = value >= 0x91;
    result.exit.nz = (uint8_t)(value - 0x91);
    if (value == 0x91) {
        result.exit.nz = INC8(OBJ->frame[2]);
        return result;
    }
    result.exit.c = value >= 0x71;
    result.exit.nz = (uint8_t)(value - 0x71);
    if (value == 0x71) {
        result.exit.nz = INC8(OBJ->state[object]);
    }
    return result;
}

void abi_Scene02Descend(void)
{
    scene_obj_shim(Scene02Descend(cpu.x));
}

/*
 * @routine 224B:290 Scene02ScrollSet
 * @run 6B:8290
 * WHAT IT DOES: Puts the scroll's Y low at $FC and goes on.
 * HOW: Two stores.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene02ScrollSet(uint8_t object)
{
    WR8(SCROLL->y_lo[0], 0xFC);
    return scene_kept(0xFC, INC8(OBJ->state[object]));
}

void abi_Scene02ScrollSet(void)
{
    scene_obj_shim(Scene02ScrollSet(cpu.x));
}

/*
 * @routine 224B:299 Scene02ScrollUp
 * @run 6B:8299
 * WHAT IT DOES: Moves the scroll's Y low down by four a frame until 0, then goes on.
 * HOW: SEC, SBC #4.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene02ScrollUp(uint8_t object)
{
    uint8_t value = RD8(SCROLL->y_lo[0]);
    uint8_t diff = (uint8_t)(value - 4);
    ObjExit result;

    WR8(SCROLL->y_lo[0], diff);
    result = scene_kept(diff, diff);
    result.exit.c = value >= 4;
    result.exit.c_kept = false;
    result.exit.v = ((value ^ 4) & (value ^ diff) & 0x80) != 0;
    result.exit.v_set = true;
    if (diff == 0) {
        result.exit.nz = INC8(OBJ->state[object]);
    }
    return result;
}

void abi_Scene02ScrollUp(void)
{
    scene_obj_shim(Scene02ScrollUp(cpu.x));
}

/*
 * @routine 224B:2A5 Scene02RaiseSecond
 * @run 6B:82A5
 * WHAT IT DOES: Raises object 1's Y (16 bits) until its low byte is $50.
 * HOW: INC with the carry, CMP.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene02RaiseSecond(uint8_t object)
{
    uint8_t value;
    ObjExit result;

    if (INC8(OBJ->y_lo[1]) == 0) {
        INC8(OBJ->y_hi[1]);
    }
    value = RD8(OBJ->y_lo[1]);
    result = scene_kept(value, (uint8_t)(value - 0x50));
    result.exit.c = value >= 0x50;
    result.exit.c_kept = false;
    if (value == 0x50) {
        result.exit.nz = INC8(OBJ->state[object]);
    }
    return result;
}

void abi_Scene02RaiseSecond(void)
{
    scene_obj_shim(Scene02RaiseSecond(cpu.x));
}

/*
 * @routine 224B:2B5 Scene02Bob
 * @run 6B:82B5
 * WHAT IT DOES: Bobs object 1 on a sine of 15 around Y $50.
 * HOW: ScaleBySine of 15 by work_d doubled; the product added to $50.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene02Bob(uint8_t object)
{
    ScaleResult wave;
    uint8_t angle, low, high, product;
    unsigned total;
    ObjExit result;

    WR8(ZP->temp00, 0x0F);
    WR8(ZP->temp01, 0x00);
    INC8(OBJ->work_d[object]);
    angle = (uint8_t)(RD8(OBJ->work_d[object]) << 1);
    jsr_push(0x82C5);
    wave = ScaleBySine(angle);
    jsr_pop();
    (void)wave;
    product = RD8(ZP->product.mid);
    total = 0x50u + product;
    low = (uint8_t)total;
    WR8(OBJ->y_lo[1], low);
    high = (uint8_t)(RD8(ZP->product.hi) + (total > 0xFF ? 1 : 0));
    WR8(OBJ->y_hi[1], high);
    result = scene_kept(high, high);
    result.exit.c = total > 0xFF && high == 0;
    result.exit.c_kept = false;
    result.exit.v = total > 0xFF && high == 0x80;       /* CLA then ADC: overflow only from $7F plus the carry */
    result.exit.v_set = true;
    result.exit.y = 0;
    result.exit.y_kept = false;
    return result;
}

void abi_Scene02Bob(void)
{
    scene_obj_shim(Scene02Bob(cpu.x));
}

/*
 * @routine 224B:2DE Scene02PlayListOnce
 * @run 6B:82DE
 * WHAT IT DOES: Plays the start argument's frame list once.
 * HOW: ScenePlayListOnce with this overlay's lists.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene02PlayListOnce(uint8_t object)
{
    return ScenePlayListOnce(0x8347, 0x82E9, object);
}

void abi_Scene02PlayListOnce(void)
{
    scene_obj_shim(Scene02PlayListOnce(cpu.x));
}

/*
 * @routine 224B:2F4 Scene02SlideForCount
 * @run 6B:82F4
 * WHAT IT DOES: As the $5C overlay's slide, with this overlay's speed tables.
 * HOW: SceneSlide.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene02SlideForCount(uint8_t object)
{
    static const uint16_t speeds[4] = { 0x8324, 0x8329, 0x832E, 0x8333 };

    return SceneSlide(speeds, 0x831E, object);
}

void abi_Scene02SlideForCount(void)
{
    scene_obj_shim(Scene02SlideForCount(cpu.x));
}

/*
 * @routine 224B:338 Scene02FrameList
 * @run 6B:8338
 * WHAT IT DOES: Steps an object through the frame list this overlay's table names for A.
 * HOW: ObjectFrameListAt.
 * WAITS: no.
 * Entry: A the list, X the object. Exit: as StepObjectFrames.
 */
ObjExit Scene02FrameList(uint8_t list, uint8_t object)
{
    return ObjectFrameListAt(list, object, 0x8347);
}

void abi_Scene02FrameList(void)
{
    scene_obj_shim(Scene02FrameList(cpu.a, cpu.x));
}

/*
 * @routine 224B:226 Scene02StepsA
 * @run 6B:8226
 * @stop 8238
 * WHAT IT DOES: Goes to the step its state names.
 * HOW: The state doubled indexes the table; the word through zTemp00 and zTemp01.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene02StepsA(uint8_t object)
{
    uint8_t index, high;
    uint16_t target = SceneStateJump(0x8238, object, &index, &high);
    ObjExit result;

    switch (target) {
    case 0x823E: result = Scene02Descend(object); break;
    case 0x8273: result = scene_kept(0, 0); result.exit.a_kept = true; break;
    default:
        scene_entry_shim(high, object, index);
        run_routine(target);
        result = scene_kept(0, 0);
        result.exit.from_cpu = true;
        return result;
    }
    if (result.exit.a_kept) {
        result.exit.a = high;
        result.exit.a_kept = false;
    }
    if (result.exit.y_kept) {
        result.exit.y = index;
        result.exit.y_kept = false;
    }
    return result;
}

void abi_Scene02StepsA(void)
{
    scene_obj_shim(Scene02StepsA(cpu.x));
}

/*
 * @routine 224B:274 Scene02StepsB
 * @run 6B:8274
 * @stop 8286
 * WHAT IT DOES: Goes to the step its state names.
 * HOW: The state doubled indexes the table; the word through zTemp00 and zTemp01.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene02StepsB(uint8_t object)
{
    uint8_t index, high;
    uint16_t target = SceneStateJump(0x8286, object, &index, &high);
    ObjExit result;

    switch (target) {
    case 0x8290: result = Scene02ScrollSet(object); break;
    case 0x8299: result = Scene02ScrollUp(object); break;
    case 0x82A5: result = Scene02RaiseSecond(object); break;
    case 0x82B5: result = Scene02Bob(object); break;
    default:
        scene_entry_shim(high, object, index);
        run_routine(target);
        result = scene_kept(0, 0);
        result.exit.from_cpu = true;
        return result;
    }
    if (result.exit.a_kept) {
        result.exit.a = high;
        result.exit.a_kept = false;
    }
    if (result.exit.y_kept) {
        result.exit.y = index;
        result.exit.y_kept = false;
    }
    return result;
}

void abi_Scene02StepsB(void)
{
    scene_obj_shim(Scene02StepsB(cpu.x));
}

/* This overlay's object handlers by their addresses, for the engine's dispatch. */
static bool scene_02_handler(uint16_t target, uint8_t object, ObjExit *out)
{
    switch (target) {
    case 0x82DE: *out = Scene02PlayListOnce(object); return true;
    case 0x82F4: *out = Scene02SlideForCount(object); return true;
    case 0x8274: *out = Scene02StepsB(object); return true;
    case 0x8226: *out = Scene02StepsA(object); return true;
    default: return false;
    }
}

static const SceneOverlay SCENE_02 = { 8, scene_02_handler };

/*
 * @routine 224B:027 Scene02Frame
 * @run 6B:8027
 * WHAT IT DOES: As Scene5CFrame (25D3:01F), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CFrame.
 * Entry, exit: as Scene5CFrame.
 */
Exit Scene02Frame(void)
{
    return SceneFrame(&SCENE_02);
}

void abi_Scene02Frame(void)
{
    exit_shim(Scene02Frame());
}

/*
 * @routine 224B:058 Scene02ScriptStep
 * @run 6B:8058
 * WHAT IT DOES: As Scene5CScriptStep (25D3:050), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CScriptStep.
 * Entry, exit: as Scene5CScriptStep.
 */
Exit Scene02ScriptStep(void)
{
    return SceneScriptStep(&SCENE_02);
}

void abi_Scene02ScriptStep(void)
{
    exit_shim(Scene02ScriptStep());
}

/*
 * @routine 224B:0AD Scene02CommandStartListAndClear
 * @run 6B:80AD
 * WHAT IT DOES: As Scene5CCommandStartListAndClear (25D3:0A5), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CCommandStartListAndClear.
 * Entry, exit: as Scene5CCommandStartListAndClear.
 */
Exit Scene02CommandStartListAndClear(void)
{
    return SceneCommandStartListAndClear(&SCENE_02);
}

void abi_Scene02CommandStartListAndClear(void)
{
    exit_shim(Scene02CommandStartListAndClear());
}

/*
 * @routine 224B:0C8 Scene02CommandStartScript1
 * @run 6B:80C8
 * WHAT IT DOES: As Scene5CCommandStartScript1 (25D3:0C0), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CCommandStartScript1.
 * Entry, exit: as Scene5CCommandStartScript1.
 */
Exit Scene02CommandStartScript1(void)
{
    return SceneCommandStartScript1(&SCENE_02);
}

void abi_Scene02CommandStartScript1(void)
{
    exit_shim(Scene02CommandStartScript1());
}

/*
 * @routine 224B:0D0 Scene02CommandStartScript2
 * @run 6B:80D0
 * WHAT IT DOES: As Scene5CCommandStartScript2 (25D3:0C8), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CCommandStartScript2.
 * Entry, exit: as Scene5CCommandStartScript2.
 */
Exit Scene02CommandStartScript2(void)
{
    return SceneCommandStartScript2(&SCENE_02);
}

void abi_Scene02CommandStartScript2(void)
{
    exit_shim(Scene02CommandStartScript2());
}

/*
 * @routine 224B:0D8 Scene02CommandStartScript3
 * @run 6B:80D8
 * WHAT IT DOES: As Scene5CCommandStartScript3 (25D3:0D0), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CCommandStartScript3.
 * Entry, exit: as Scene5CCommandStartScript3.
 */
Exit Scene02CommandStartScript3(void)
{
    return SceneCommandStartScript3(&SCENE_02);
}

void abi_Scene02CommandStartScript3(void)
{
    exit_shim(Scene02CommandStartScript3());
}

/*
 * @routine 224B:0E0 Scene02CommandPlayAdpcm
 * @run 6B:80E0
 * WHAT IT DOES: As Scene5CCommandPlayAdpcm (25D3:0D8), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CCommandPlayAdpcm.
 * Entry, exit: as Scene5CCommandPlayAdpcm.
 */
Exit Scene02CommandPlayAdpcm(void)
{
    return SceneCommandPlayAdpcm(&SCENE_02);
}

void abi_Scene02CommandPlayAdpcm(void)
{
    exit_shim(Scene02CommandPlayAdpcm());
}

/*
 * @routine 224B:0E8 Scene02CommandCdPlay
 * @run 6B:80E8
 * WHAT IT DOES: As Scene5CCommandCdPlay (25D3:0E0), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CCommandCdPlay.
 * Entry, exit: as Scene5CCommandCdPlay.
 */
Exit Scene02CommandCdPlay(uint8_t index)
{
    return SceneCommandCdPlay(&SCENE_02, index);
}

void abi_Scene02CommandCdPlay(void)
{
    exit_shim(Scene02CommandCdPlay(cpu.x));
}

/*
 * @routine 224B:0F0 Scene02CommandPlaceKind
 * @run 6B:80F0
 * WHAT IT DOES: As Scene5CCommandPlaceKind (25D3:0E8), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CCommandPlaceKind.
 * Entry, exit: as Scene5CCommandPlaceKind.
 */
Exit Scene02CommandPlaceKind(void)
{
    return SceneCommandPlaceKind(&SCENE_02);
}

void abi_Scene02CommandPlaceKind(void)
{
    exit_shim(Scene02CommandPlaceKind());
}

/*
 * @routine 224B:0FF Scene02CommandPlaceNext
 * @run 6B:80FF
 * WHAT IT DOES: As Scene5CCommandPlaceNext (25D3:0F7), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CCommandPlaceNext.
 * Entry, exit: as Scene5CCommandPlaceNext.
 */
Exit Scene02CommandPlaceNext(void)
{
    return SceneCommandPlaceNext(&SCENE_02);
}

void abi_Scene02CommandPlaceNext(void)
{
    exit_shim(Scene02CommandPlaceNext());
}

/*
 * @routine 224B:119 Scene02CommandPlaceFrame
 * @run 6B:8119
 * WHAT IT DOES: As Scene5CCommandPlaceFrame (25D3:111), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CCommandPlaceFrame.
 * Entry, exit: as Scene5CCommandPlaceFrame.
 */
Exit Scene02CommandPlaceFrame(void)
{
    return SceneCommandPlaceFrame(&SCENE_02);
}

void abi_Scene02CommandPlaceFrame(void)
{
    exit_shim(Scene02CommandPlaceFrame());
}

/*
 * @routine 224B:12E Scene02CommandPlaceTimed
 * @run 6B:812E
 * WHAT IT DOES: As Scene5CCommandPlaceTimed (25D3:126), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CCommandPlaceTimed.
 * Entry, exit: as Scene5CCommandPlaceTimed.
 */
Exit Scene02CommandPlaceTimed(void)
{
    return SceneCommandPlaceTimed(&SCENE_02);
}

void abi_Scene02CommandPlaceTimed(void)
{
    exit_shim(Scene02CommandPlaceTimed());
}

/*
 * @routine 224B:17A Scene02CommandPlaceTitleObjectA
 * @run 6B:817A
 * WHAT IT DOES: As Scene5CCommandPlaceTitleObjectA (25D3:172), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CCommandPlaceTitleObjectA.
 * Entry, exit: as Scene5CCommandPlaceTitleObjectA.
 */
Exit Scene02CommandPlaceTitleObjectA(void)
{
    return SceneCommandPlaceTitleObjectA(&SCENE_02);
}

void abi_Scene02CommandPlaceTitleObjectA(void)
{
    exit_shim(Scene02CommandPlaceTitleObjectA());
}

/*
 * @routine 224B:1A1 Scene02CommandPlaceTitleObjectB
 * @run 6B:81A1
 * WHAT IT DOES: As Scene5CCommandPlaceTitleObjectB (25D3:199), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CCommandPlaceTitleObjectB.
 * Entry, exit: as Scene5CCommandPlaceTitleObjectB.
 */
Exit Scene02CommandPlaceTitleObjectB(void)
{
    return SceneCommandPlaceTitleObjectB(&SCENE_02);
}

void abi_Scene02CommandPlaceTitleObjectB(void)
{
    exit_shim(Scene02CommandPlaceTitleObjectB());
}

/*
 * @routine 224B:1C8 Scene02CommandLoadBanks
 * @run 6B:81C8
 * WHAT IT DOES: As Scene5CCommandLoadBanks (25D3:1C0), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CCommandLoadBanks.
 * Entry, exit: as Scene5CCommandLoadBanks.
 */
Exit Scene02CommandLoadBanks(void)
{
    return SceneCommandLoadBanks(&SCENE_02);
}

void abi_Scene02CommandLoadBanks(void)
{
    exit_shim(Scene02CommandLoadBanks());
}

/*
 * @routine 224B:1FD Scene02ObjectsStep
 * @run 6B:81FD
 * WHAT IT DOES: As Scene5CObjectsStep (25D3:1F5), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CObjectsStep.
 * Entry, exit: as Scene5CObjectsStep.
 */
Exit Scene02ObjectsStep(void)
{
    return SceneObjectsStep(&SCENE_02);
}

void abi_Scene02ObjectsStep(void)
{
    exit_shim(Scene02ObjectsStep());
}

/*
 * @routine 224B:20C Scene02DispatchKind
 * @run 6B:820C
 * WHAT IT DOES: As Scene5CDispatchKind (25D3:204), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($8).
 * WAITS: as Scene5CDispatchKind.
 * Entry, exit: as Scene5CDispatchKind.
 */
ObjExit Scene02DispatchKind(uint8_t kind, uint8_t object)
{
    return SceneDispatchKind(&SCENE_02, kind, object);
}

void abi_Scene02DispatchKind(void)
{
    scene_obj_shim(Scene02DispatchKind(cpu.a, cpu.x));
}
