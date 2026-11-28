/*
 * The cutscene overlay of sector $20BC (bank $6B at $8000; scenes $0C and $0D): the shared script engine
 * (scene_engine.c) at this overlay's places, $16 past the $5C overlay's, and this overlay's own begin and
 * object handlers.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "scene_engine.h"
#include "scene_0c.h"
#include "scene_5a.h"


static void scene_entry_shim(uint8_t high, uint8_t object, uint8_t index)
{
    cpu.a = high;
    cpu.x = object;
    cpu.y = index;
    abi_nz(high);
}

/*
 * @routine 20BC:00B Scene0CBegin
 * @run 6B:800B
 * WHAT IT DOES: Starts the scene of $0C and $0D: bit 2 of ProgressCharacter, the player's health $5C, the script of the character from $8031.
 * HOW: ResetGameState; an ORA into ProgressCharacter; the script pointer, the frame counter $FFFF, the sub-mode up.
 * WAITS: yes, through ResetGameState (the vertical blank).
 * Entry: X the doubled sub-mode and Y kept. Exit: A $FF, X 0, N and Z from the new state; Y and C from the character
 * where the script is picked by it, else Y $FF and C kept.
 */
Exit Scene0CBegin(uint8_t kept_x, uint8_t kept_y)
{
    Exit exit = { 0 };
    uint8_t character, row;

    jsr_push(0x800E);
    ResetGameState(wait_regs_xy_shim(kept_x, kept_y));
    jsr_pop();
    wr8(ProgressCharacter, (uint8_t)(rd8(ProgressCharacter) | 0x04));
    WR8(ZP->player_health, 0x5C);
    character = rd8(zCharacter);
    row = (uint8_t)(character << 1);
    WR8(ZP->script_ptr, rd8((uint16_t)(0x8031 + row)));
    WR8(ZP->script_ptr_hi, rd8((uint16_t)(0x8032 + row)));
    WR8(ZP->wait_timer3, 0xFF);
    WR8(ZP->wait_timer4, 0xFF);
    exit.a = 0xFF;
    exit.x = 0;
    exit.y = row;
    exit.c = (character & 0x80) != 0;
    exit.nz = INC8(ZP->game_sub_mode);
    return exit;
}

void abi_Scene0CBegin(void)
{
    exit_shim(Scene0CBegin(cpu.x, cpu.y));
}

/*
 * @routine 20BC:232 Scene0CDriftThree
 * @run 6B:8232
 * WHAT IT DOES: Waits 33 frames, then moves objects $11, 2 and 3 left by two and up by one a frame.
 * HOW: state 0 sets work_d; state 1 counts it down, reloading below zero; three 16-bit SBCs and DECs.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0CDriftThree(uint8_t object)
{
    uint8_t state, value, diff;
    ObjExit result;
    bool carry;
    static const uint8_t movers[3] = { 0x11, 2, 3 };
    int i;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->work_d[object], 0x20);
        return scene_kept(0x20, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return scene_kept(state, state);
    }
    if (DEC8(OBJ->work_d[object]) & 0x80) {
        return scene_kept(0, INC8(OBJ->state[object]));
    }
    result = scene_kept(0, 0);
    for (i = 0; i < 3; i++) {
        uint8_t slot = movers[i];

        value = RD8(OBJ->x_lo[slot]);
        carry = value >= 2;
        diff = (uint8_t)(value - 2);
        WR8(OBJ->x_lo[slot], diff);
        value = RD8(OBJ->x_hi[slot]);
        diff = (uint8_t)(value - (carry ? 0 : 1));
        result.exit.c = value >= (carry ? 0u : 1u);
        result.exit.v = ((value ^ 0) & (value ^ diff) & 0x80) != 0;
        WR8(OBJ->x_hi[slot], diff);
        result.exit.a = diff;
        result.exit.nz = DEC8(OBJ->y_lo[slot]);
    }
    result.exit.c_kept = false;
    result.exit.v_set = true;
    return result;
}

void abi_Scene0CDriftThree(void)
{
    scene_obj_shim(Scene0CDriftThree(cpu.x));
}

/*
 * @routine 20BC:28F Scene0CPlayListOnce
 * @run 6B:828F
 * WHAT IT DOES: Plays the start argument's frame list once.
 * HOW: ScenePlayListOnce with this overlay's lists.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0CPlayListOnce(uint8_t object)
{
    return ScenePlayListOnce(0x82F8, 0x829A, object);
}

void abi_Scene0CPlayListOnce(void)
{
    scene_obj_shim(Scene0CPlayListOnce(cpu.x));
}

/*
 * @routine 20BC:2A5 Scene0CSlideForCount
 * @run 6B:82A5
 * WHAT IT DOES: As the $5C overlay's slide, with this overlay's speed tables.
 * HOW: SceneSlide.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0CSlideForCount(uint8_t object)
{
    static const uint16_t speeds[4] = { 0x82D5, 0x82DA, 0x82DF, 0x82E4 };

    return SceneSlide(speeds, 0x82CF, object);
}

void abi_Scene0CSlideForCount(void)
{
    scene_obj_shim(Scene0CSlideForCount(cpu.x));
}

/*
 * @routine 20BC:2E9 Scene0CFrameList
 * @run 6B:82E9
 * WHAT IT DOES: Steps an object through the frame list this overlay's table names for A.
 * HOW: ObjectFrameListAt.
 * WAITS: no.
 * Entry: A the list, X the object. Exit: as StepObjectFrames.
 */
ObjExit Scene0CFrameList(uint8_t list, uint8_t object)
{
    return ObjectFrameListAt(list, object, 0x82F8);
}

void abi_Scene0CFrameList(void)
{
    scene_obj_shim(Scene0CFrameList(cpu.a, cpu.x));
}

/* This overlay's object handlers by their addresses, for the engine's dispatch. */
static bool scene_0c_handler(uint16_t target, uint8_t object, ObjExit *out)
{
    /* The $5A overlay holds this overlay's objects' loop and dispatch bytes: whichever is loaded gives its handlers. */
    if (resident_at(0x820B) == DISC(0x2703, 0x20B)) {
        return scene_5a_handler(target, object, out);
    }
    switch (target) {
    case 0x828F: *out = Scene0CPlayListOnce(object); return true;
    case 0x82A5: *out = Scene0CSlideForCount(object); return true;
    case 0x8232: *out = Scene0CDriftThree(object); return true;
    default: return false;
    }
}

static const SceneOverlay SCENE_0C = { 22, scene_0c_handler };

/*
 * @routine 20BC:035 Scene0CFrame
 * @run 6B:8035
 * WHAT IT DOES: As Scene5CFrame (25D3:01F), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CFrame.
 * Entry, exit: as Scene5CFrame.
 */
Exit Scene0CFrame(void)
{
    return SceneFrame(&SCENE_0C);
}

void abi_Scene0CFrame(void)
{
    exit_shim(Scene0CFrame());
}

/*
 * @routine 20BC:066 Scene0CScriptStep
 * @run 6B:8066
 * WHAT IT DOES: As Scene5CScriptStep (25D3:050), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CScriptStep.
 * Entry, exit: as Scene5CScriptStep.
 */
Exit Scene0CScriptStep(void)
{
    return SceneScriptStep(&SCENE_0C);
}

void abi_Scene0CScriptStep(void)
{
    exit_shim(Scene0CScriptStep());
}

/*
 * @routine 20BC:0BB Scene0CCommandStartListAndClear
 * @run 6B:80BB
 * WHAT IT DOES: As Scene5CCommandStartListAndClear (25D3:0A5), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CCommandStartListAndClear.
 * Entry, exit: as Scene5CCommandStartListAndClear.
 */
Exit Scene0CCommandStartListAndClear(void)
{
    return SceneCommandStartListAndClear(&SCENE_0C);
}

void abi_Scene0CCommandStartListAndClear(void)
{
    exit_shim(Scene0CCommandStartListAndClear());
}

/*
 * @routine 20BC:0D6 Scene0CCommandStartScript1
 * @run 6B:80D6
 * WHAT IT DOES: As Scene5CCommandStartScript1 (25D3:0C0), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CCommandStartScript1.
 * Entry, exit: as Scene5CCommandStartScript1.
 */
Exit Scene0CCommandStartScript1(void)
{
    return SceneCommandStartScript1(&SCENE_0C);
}

void abi_Scene0CCommandStartScript1(void)
{
    exit_shim(Scene0CCommandStartScript1());
}

/*
 * @routine 20BC:0DE Scene0CCommandStartScript2
 * @run 6B:80DE
 * WHAT IT DOES: As Scene5CCommandStartScript2 (25D3:0C8), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CCommandStartScript2.
 * Entry, exit: as Scene5CCommandStartScript2.
 */
Exit Scene0CCommandStartScript2(void)
{
    return SceneCommandStartScript2(&SCENE_0C);
}

void abi_Scene0CCommandStartScript2(void)
{
    exit_shim(Scene0CCommandStartScript2());
}

/*
 * @routine 20BC:0E6 Scene0CCommandStartScript3
 * @run 6B:80E6
 * WHAT IT DOES: As Scene5CCommandStartScript3 (25D3:0D0), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CCommandStartScript3.
 * Entry, exit: as Scene5CCommandStartScript3.
 */
Exit Scene0CCommandStartScript3(void)
{
    return SceneCommandStartScript3(&SCENE_0C);
}

void abi_Scene0CCommandStartScript3(void)
{
    exit_shim(Scene0CCommandStartScript3());
}

/*
 * @routine 20BC:0EE Scene0CCommandPlayAdpcm
 * @run 6B:80EE
 * WHAT IT DOES: As Scene5CCommandPlayAdpcm (25D3:0D8), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CCommandPlayAdpcm.
 * Entry, exit: as Scene5CCommandPlayAdpcm.
 */
Exit Scene0CCommandPlayAdpcm(void)
{
    return SceneCommandPlayAdpcm(&SCENE_0C);
}

void abi_Scene0CCommandPlayAdpcm(void)
{
    exit_shim(Scene0CCommandPlayAdpcm());
}

/*
 * @routine 20BC:0F6 Scene0CCommandCdPlay
 * @run 6B:80F6
 * WHAT IT DOES: As Scene5CCommandCdPlay (25D3:0E0), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CCommandCdPlay.
 * Entry, exit: as Scene5CCommandCdPlay.
 */
Exit Scene0CCommandCdPlay(uint8_t index)
{
    return SceneCommandCdPlay(&SCENE_0C, index);
}

void abi_Scene0CCommandCdPlay(void)
{
    exit_shim(Scene0CCommandCdPlay(cpu.x));
}

/*
 * @routine 20BC:0FE Scene0CCommandPlaceKind
 * @run 6B:80FE
 * WHAT IT DOES: As Scene5CCommandPlaceKind (25D3:0E8), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CCommandPlaceKind.
 * Entry, exit: as Scene5CCommandPlaceKind.
 */
Exit Scene0CCommandPlaceKind(void)
{
    return SceneCommandPlaceKind(&SCENE_0C);
}

void abi_Scene0CCommandPlaceKind(void)
{
    exit_shim(Scene0CCommandPlaceKind());
}

/*
 * @routine 20BC:10D Scene0CCommandPlaceNext
 * @run 6B:810D
 * WHAT IT DOES: As Scene5CCommandPlaceNext (25D3:0F7), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CCommandPlaceNext.
 * Entry, exit: as Scene5CCommandPlaceNext.
 */
Exit Scene0CCommandPlaceNext(void)
{
    return SceneCommandPlaceNext(&SCENE_0C);
}

void abi_Scene0CCommandPlaceNext(void)
{
    exit_shim(Scene0CCommandPlaceNext());
}

/*
 * @routine 20BC:127 Scene0CCommandPlaceFrame
 * @run 6B:8127
 * WHAT IT DOES: As Scene5CCommandPlaceFrame (25D3:111), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CCommandPlaceFrame.
 * Entry, exit: as Scene5CCommandPlaceFrame.
 */
Exit Scene0CCommandPlaceFrame(void)
{
    return SceneCommandPlaceFrame(&SCENE_0C);
}

void abi_Scene0CCommandPlaceFrame(void)
{
    exit_shim(Scene0CCommandPlaceFrame());
}

/*
 * @routine 20BC:13C Scene0CCommandPlaceTimed
 * @run 6B:813C
 * WHAT IT DOES: As Scene5CCommandPlaceTimed (25D3:126), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CCommandPlaceTimed.
 * Entry, exit: as Scene5CCommandPlaceTimed.
 */
Exit Scene0CCommandPlaceTimed(void)
{
    return SceneCommandPlaceTimed(&SCENE_0C);
}

void abi_Scene0CCommandPlaceTimed(void)
{
    exit_shim(Scene0CCommandPlaceTimed());
}

/*
 * @routine 20BC:188 Scene0CCommandPlaceTitleObjectA
 * @run 6B:8188
 * WHAT IT DOES: As Scene5CCommandPlaceTitleObjectA (25D3:172), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CCommandPlaceTitleObjectA.
 * Entry, exit: as Scene5CCommandPlaceTitleObjectA.
 */
Exit Scene0CCommandPlaceTitleObjectA(void)
{
    return SceneCommandPlaceTitleObjectA(&SCENE_0C);
}

void abi_Scene0CCommandPlaceTitleObjectA(void)
{
    exit_shim(Scene0CCommandPlaceTitleObjectA());
}

/*
 * @routine 20BC:1AF Scene0CCommandPlaceTitleObjectB
 * @run 6B:81AF
 * WHAT IT DOES: As Scene5CCommandPlaceTitleObjectB (25D3:199), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CCommandPlaceTitleObjectB.
 * Entry, exit: as Scene5CCommandPlaceTitleObjectB.
 */
Exit Scene0CCommandPlaceTitleObjectB(void)
{
    return SceneCommandPlaceTitleObjectB(&SCENE_0C);
}

void abi_Scene0CCommandPlaceTitleObjectB(void)
{
    exit_shim(Scene0CCommandPlaceTitleObjectB());
}

/*
 * @routine 20BC:1D6 Scene0CCommandLoadBanks
 * @run 6B:81D6
 * WHAT IT DOES: As Scene5CCommandLoadBanks (25D3:1C0), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CCommandLoadBanks.
 * Entry, exit: as Scene5CCommandLoadBanks.
 */
Exit Scene0CCommandLoadBanks(void)
{
    return SceneCommandLoadBanks(&SCENE_0C);
}

void abi_Scene0CCommandLoadBanks(void)
{
    exit_shim(Scene0CCommandLoadBanks());
}

/*
 * @routine 20BC:20B Scene0CObjectsStep
 * @run 6B:820B
 * WHAT IT DOES: As Scene5CObjectsStep (25D3:1F5), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CObjectsStep.
 * Entry, exit: as Scene5CObjectsStep.
 */
Exit Scene0CObjectsStep(void)
{
    return SceneObjectsStep(&SCENE_0C);
}

void abi_Scene0CObjectsStep(void)
{
    exit_shim(Scene0CObjectsStep());
}

/*
 * @routine 20BC:21A Scene0CDispatchKind
 * @run 6B:821A
 * WHAT IT DOES: As Scene5CDispatchKind (25D3:204), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($16).
 * WAITS: as Scene5CDispatchKind.
 * Entry, exit: as Scene5CDispatchKind.
 */
ObjExit Scene0CDispatchKind(uint8_t kind, uint8_t object)
{
    return SceneDispatchKind(&SCENE_0C, kind, object);
}

void abi_Scene0CDispatchKind(void)
{
    scene_obj_shim(Scene0CDispatchKind(cpu.a, cpu.x));
}
