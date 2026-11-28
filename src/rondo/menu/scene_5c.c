/*
 * The cutscene overlay of sectors $25D3 to $25D6 (bank $6B at $8000; scenes $5C and $5D, loaded after the bonus
 * stage): the ending's script engine at this overlay's own tables (lengths $8030, commands $806B, object kinds
 * $8214), and its own object handlers.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "vram_update.h"
#include "scene_5c.h"
#include "front_end_copies.h"
#include "scene_engine.h"

enum {
    SCENE_DRIFTERS = 0x8284,
    SCENE_SPEEDS_XL = 0x82D9, SCENE_SPEEDS_XH = 0x82DE, SCENE_SPEEDS_YL = 0x82E3, SCENE_SPEEDS_YH = 0x82E8,
    SCENE_FRAME_LISTS = 0x82FC, SCENE_OBJECTS = 0x58
};

static void scene_entry_shim(uint8_t high, uint8_t object, uint8_t index, bool carry)
{
    cpu.a = high;
    cpu.x = object;
    cpu.y = index;
    abi_nz(high);
    abi_flag(P_C, carry);
}

static bool scene_5c_handler(uint16_t target, uint8_t object, ObjExit *out)
{
    switch (target) {
    case 0x8293: *out = Scene5CPlayListOnce(object); return true;
    case 0x82A9: *out = Scene5CSlideForCount(object); return true;
    case 0x8249: *out = Scene5CDriftFiveObjects(object); return true;
    case 0x8226: *out = Scene5CScrollUpAndClear(object); return true;
    case 0x8220: *out = Scene5CShowFirstObject(object); return true;
    default: return false;
    }
}

static const SceneOverlay SCENE_5C = { 0, scene_5c_handler };

static ObjExit kept_obj(uint8_t a, uint8_t nz)
{
    ObjExit result = { 0 };

    result.exit.a = a;
    result.exit.nz = nz;
    result.exit.x_kept = result.exit.y_kept = result.exit.c_kept = true;
    return result;
}

/*
 * @routine 25D3:00B Scene5CBegin
 * @run 6B:800B
 * WHAT IT DOES: Starts the scene: the game state wiped, the script from $838D, its frame counter $FFFF, the state goes on.
 * HOW: As EndingBegin, with this scene's script.
 * WAITS: yes, through ResetGameState (the vertical blank).
 * Entry: X the doubled sub-mode and Y kept. Exit: A $FF, X 0, Y $FF, N and Z from the new state; C kept.
 */
Exit Scene5CBegin(uint8_t kept_x, uint8_t kept_y)
{
    Exit exit = { 0 };

    jsr_push(0x800E);
    ResetGameState(wait_regs_xy_shim(kept_x, kept_y));
    jsr_pop();
    WR8(ZP->script_ptr, 0x8D);
    WR8(ZP->script_ptr_hi, 0x83);
    WR8(ZP->wait_timer3, 0xFF);
    WR8(ZP->wait_timer4, 0xFF);
    exit.a = 0xFF;
    exit.x = 0;
    exit.y = 0xFF;
    exit.nz = INC8(ZP->game_sub_mode);
    exit.c_kept = true;
    return exit;
}

void abi_Scene5CBegin(void)
{
    exit_shim(Scene5CBegin(cpu.x, cpu.y));
}

/*
 * @routine 25D3:01F Scene5CFrame
 * @run 6B:801F
 * WHAT IT DOES: One frame of the scene: the frame counted, the script's command run if due, every object run.
 * HOW: Scene5CFrameTick, Scene5CScriptStep, Scene5CObjectsStep.
 * WAITS: no.
 * Entry: nothing. Exit: as Scene5CObjectsStep.
 */
Exit Scene5CFrame(void)
{
    return SceneFrame(&SCENE_5C);
}

void abi_Scene5CFrame(void)
{
    exit_shim(Scene5CFrame());
}

/*
 * @routine 25D3:041 Scene5CScriptAdvance
 * @run 6B:8041
 * WHAT IT DOES: Steps the script past its command and runs on into Scene5CScriptStep.
 * HOW: The command's length from $8030 added to the pointer.
 * WAITS: no.
 * Entry: nothing. Exit: as Scene5CScriptStep, X the command stepped past when it left X alone.
 */
Exit Scene5CScriptAdvance(void)
{
    return SceneScriptAdvance(&SCENE_5C);
}

void abi_Scene5CScriptAdvance(void)
{
    exit_shim(Scene5CScriptAdvance());
}

/*
 * @routine 25D3:050 Scene5CScriptStep
 * @run 6B:8050
 * WHAT IT DOES: Runs the script's command when its two time bytes equal the frame counter.
 * HOW: The times compared; the command doubled indexes the table at $806B, entered with X the index and Y 3.
 * WAITS: no.
 * Entry: nothing. Exit: a time byte differs: that byte, N, Z and C from comparing it, Y 1 or 2, X kept; else as the command.
 */
Exit Scene5CScriptStep(void)
{
    return SceneScriptStep(&SCENE_5C);
}

void abi_Scene5CScriptStep(void)
{
    exit_shim(Scene5CScriptStep());
}

/*
 * @routine 25D3:08D Scene5CCommandSubModeTwo
 * @run 6B:808D
 * WHAT IT DOES: A script command: sub-mode 2.
 * HOW: LDA #2, STA zGameSubMode.
 * WAITS: no.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandSubModeTwo(void)
{
    return SceneCommandSubModeTwo(&SCENE_5C);
}

void abi_Scene5CCommandSubModeTwo(void)
{
    exit_shim(Scene5CCommandSubModeTwo());
}

/*
 * @routine 25D3:093 Scene5CCommandSetScroll
 * @run 6B:8093
 * WHAT IT DOES: A script command: sets the scroll's high bytes from the operand's two nibbles.
 * HOW: The high nibble to ScrollXHi, the low to ScrollYHi, with PHA and PLA.
 * WAITS: no.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandSetScroll(void)
{
    return SceneCommandSetScroll(&SCENE_5C);
}

void abi_Scene5CCommandSetScroll(void)
{
    exit_shim(Scene5CCommandSetScroll());
}

/*
 * @routine 25D3:0A5 Scene5CCommandStartListAndClear
 * @run 6B:80A5
 * WHAT IT DOES: A script command: starts the operand's list, clears the screen's state, the object slots and the scroll block.
 * HOW: StartListChannel0Kind2, ClearScreenState, Scene5CClearObjects, a TAI to $3E00.
 * WAITS: no.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandStartListAndClear(void)
{
    return SceneCommandStartListAndClear(&SCENE_5C);
}

void abi_Scene5CCommandStartListAndClear(void)
{
    exit_shim(Scene5CCommandStartListAndClear());
}

/*
 * @routine 25D3:0B9 Scene5CCommandStartList
 * @run 6B:80B9
 * WHAT IT DOES: A script command: starts list channel 0 with the operand's list.
 * HOW: StartListChannel0Kind2.
 * WAITS: no.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandStartList(void)
{
    return SceneCommandStartList(&SCENE_5C);
}

void abi_Scene5CCommandStartList(void)
{
    exit_shim(Scene5CCommandStartList());
}

/*
 * @routine 25D3:0C0 Scene5CCommandStartScript1
 * @run 6B:80C0
 * WHAT IT DOES: A script command: starts script slot 0 as kind 1 with the operand's script.
 * HOW: StartScriptSlot0Kind1.
 * WAITS: no.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandStartScript1(void)
{
    return SceneCommandStartScript1(&SCENE_5C);
}

void abi_Scene5CCommandStartScript1(void)
{
    exit_shim(Scene5CCommandStartScript1());
}

/*
 * @routine 25D3:0C8 Scene5CCommandStartScript2
 * @run 6B:80C8
 * WHAT IT DOES: A script command: starts script slot 0 as kind 2 with the operand's script.
 * HOW: StartScriptSlot0Kind2.
 * WAITS: no.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandStartScript2(void)
{
    return SceneCommandStartScript2(&SCENE_5C);
}

void abi_Scene5CCommandStartScript2(void)
{
    exit_shim(Scene5CCommandStartScript2());
}

/*
 * @routine 25D3:0D0 Scene5CCommandStartScript3
 * @run 6B:80D0
 * WHAT IT DOES: A script command: starts script slot 0 as kind 3 with the operand's script.
 * HOW: StartScriptSlot0Kind3.
 * WAITS: no.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandStartScript3(void)
{
    return SceneCommandStartScript3(&SCENE_5C);
}

void abi_Scene5CCommandStartScript3(void)
{
    exit_shim(Scene5CCommandStartScript3());
}

/*
 * @routine 25D3:0D8 Scene5CCommandPlayAdpcm
 * @run 6B:80D8
 * WHAT IT DOES: A script command: plays the operand's sound from the disc as ADPCM.
 * HOW: AdpcmPlayFromDisc.
 * WAITS: yes, on the drive and the ADPCM.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandPlayAdpcm(void)
{
    return SceneCommandPlayAdpcm(&SCENE_5C);
}

void abi_Scene5CCommandPlayAdpcm(void)
{
    exit_shim(Scene5CCommandPlayAdpcm());
}

/*
 * @routine 25D3:0E0 Scene5CCommandCdPlay
 * @run 6B:80E0
 * WHAT IT DOES: A script command: plays the operand's CD track once.
 * HOW: CdPlayOnce.
 * WAITS: yes, on the drive.
 * Entry: from Scene5CScriptStep, X the doubled command, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandCdPlay(uint8_t index)
{
    return SceneCommandCdPlay(&SCENE_5C, index);
}

void abi_Scene5CCommandCdPlay(void)
{
    exit_shim(Scene5CCommandCdPlay(cpu.x));
}

/*
 * @routine 25D3:0E8 Scene5CCommandPlaceKind
 * @run 6B:80E8
 * WHAT IT DOES: A script command: places an object and sets its kind.
 * HOW: Scene5CPlaceObject, then the operand bytes.
 * WAITS: no.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandPlaceKind(void)
{
    return SceneCommandPlaceKind(&SCENE_5C);
}

void abi_Scene5CCommandPlaceKind(void)
{
    exit_shim(Scene5CCommandPlaceKind());
}

/*
 * @routine 25D3:0F7 Scene5CCommandPlaceNext
 * @run 6B:80F7
 * WHAT IT DOES: A script command: places an object, sets its start argument and picture set, and its kind 2.
 * HOW: Scene5CPlaceObject, then the operand bytes.
 * WAITS: no.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandPlaceNext(void)
{
    return SceneCommandPlaceNext(&SCENE_5C);
}

void abi_Scene5CCommandPlaceNext(void)
{
    exit_shim(Scene5CCommandPlaceNext());
}

/*
 * @routine 25D3:111 Scene5CCommandPlaceFrame
 * @run 6B:8111
 * WHAT IT DOES: A script command: places an object and sets its frame and picture set.
 * HOW: Scene5CPlaceObject, then the operand bytes.
 * WAITS: no.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandPlaceFrame(void)
{
    return SceneCommandPlaceFrame(&SCENE_5C);
}

void abi_Scene5CCommandPlaceFrame(void)
{
    exit_shim(Scene5CCommandPlaceFrame());
}

/*
 * @routine 25D3:126 Scene5CCommandPlaceTimed
 * @run 6B:8126
 * WHAT IT DOES: A script command: places an object, sets its frame, picture set, start argument and work byte D, and its kind 3.
 * HOW: Scene5CPlaceObject, then the operand bytes.
 * WAITS: no.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandPlaceTimed(void)
{
    return SceneCommandPlaceTimed(&SCENE_5C);
}

void abi_Scene5CCommandPlaceTimed(void)
{
    exit_shim(Scene5CCommandPlaceTimed());
}

/*
 * @routine 25D3:14C Scene5CClearObjects
 * @run 6B:814C
 * WHAT IT DOES: Empties all 88 object slots.
 * HOW: A loop of ClearObject.
 * WAITS: no.
 * Entry: nothing. Exit: X $58, C and Z set, N clear; A and Y kept.
 */
void Scene5CClearObjects(void)
{
    SceneClearObjects(&SCENE_5C);
}

void abi_Scene5CClearObjects(void)
{
    Scene5CClearObjects();
    cpu.x = SCENE_OBJECTS;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 25D3:156 Scene5CPlaceObject
 * @run 6B:8156
 * WHAT IT DOES: Empties an object and gives it the place four script bytes name.
 * HOW: ClearObject; X low and high, Y low and high from the bytes at Y plus 1 to 4.
 * WAITS: no.
 * Entry: X the object, Y the byte before the four. Exit: A the last byte, Y plus 4, N and Z from A; X and C kept.
 */
Exit Scene5CPlaceObject(uint8_t object, uint8_t y)
{
    return ScenePlaceObject(&SCENE_5C, object, y);
}

void abi_Scene5CPlaceObject(void)
{
    exit_shim(Scene5CPlaceObject(cpu.x, cpu.y));
}

/*
 * @routine 25D3:172 Scene5CCommandPlaceTitleObjectA
 * @run 6B:8172
 * WHAT IT DOES: A script command: object $2E with four operands and its Y at $C8.
 * HOW: ClearObject, one added to the kind, four operands.
 * WAITS: no.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandPlaceTitleObjectA(void)
{
    return SceneCommandPlaceTitleObjectA(&SCENE_5C);
}

void abi_Scene5CCommandPlaceTitleObjectA(void)
{
    exit_shim(Scene5CCommandPlaceTitleObjectA());
}

/*
 * @routine 25D3:199 Scene5CCommandPlaceTitleObjectB
 * @run 6B:8199
 * WHAT IT DOES: A script command: object $2F with four operands and its Y at $D8.
 * HOW: ClearObject, one added to the kind, four operands.
 * WAITS: no.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandPlaceTitleObjectB(void)
{
    return SceneCommandPlaceTitleObjectB(&SCENE_5C);
}

void abi_Scene5CCommandPlaceTitleObjectB(void)
{
    exit_shim(Scene5CCommandPlaceTitleObjectB());
}

/*
 * @routine 25D3:1C0 Scene5CCommandLoadBanks
 * @run 6B:81C0
 * WHAT IT DOES: A script command: reads four banks of the ADPCM memory into the banks at MPR5 and MPR6.
 * HOW: As EndingCommandLoadBanks.
 * WAITS: yes, through ad_read, which waits for the ADPCM to be idle.
 * Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
 */
Exit Scene5CCommandLoadBanks(void)
{
    return SceneCommandLoadBanks(&SCENE_5C);
}

void abi_Scene5CCommandLoadBanks(void)
{
    exit_shim(Scene5CCommandLoadBanks());
}

/*
 * @routine 25D3:1F5 Scene5CObjectsStep
 * @run 6B:81F5
 * WHAT IT DOES: Runs every object with a kind.
 * HOW: A loop over the 88 slots through Scene5CDispatchKind.
 * WAITS: no.
 * Entry: nothing. Exit: X $58, Z and C set; A the last kind or what its handler left; Y and V as the last handler left them.
 */
Exit Scene5CObjectsStep(void)
{
    return SceneObjectsStep(&SCENE_5C);
}

void abi_Scene5CObjectsStep(void)
{
    exit_shim(Scene5CObjectsStep());
}

/*
 * @routine 25D3:204 Scene5CDispatchKind
 * @run 6B:8204
 * WHAT IT DOES: Sends an object to the handler its kind names: the table at $8214, kind 1 first.
 * HOW: Kind less one doubled; the word through zTemp00 and zTemp01.
 * WAITS: no.
 * Entry: A the kind, X the object. Exit: as the handler leaves them.
 */
ObjExit Scene5CDispatchKind(uint8_t kind, uint8_t object)
{
    return SceneDispatchKind(&SCENE_5C, kind, object);
}

void abi_Scene5CDispatchKind(void)
{
    scene_obj_shim(Scene5CDispatchKind(cpu.a, cpu.x));
}

/*
 * @routine 25D3:220 Scene5CShowFirstObject
 * @run 6B:8220
 * WHAT IT DOES: Gives object 0 attribute $10.
 * HOW: One store.
 * WAITS: no.
 * Entry: X the object. Exit: A $10, N and Z from it; X, Y and C kept.
 */
ObjExit Scene5CShowFirstObject(uint8_t object)
{
    (void)object;
    WR8(OBJ->sprite_attr[0], 0x10);
    return kept_obj(0x10, 0x10);
}

void abi_Scene5CShowFirstObject(void)
{
    scene_obj_shim(Scene5CShowFirstObject(cpu.x));
}

/*
 * @routine 25D3:226 Scene5CScrollUpAndClear
 * @run 6B:8226
 * WHAT IT DOES: Scrolls the screen's X by eight a frame for 32 frames, then puts the scroll's high byte back and clears the object.
 * HOW: OBJ->state; ScrollXLo; work_d counted down; ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit Scene5CScrollUpAndClear(uint8_t object)
{
    ObjExit result;
    uint8_t state, value, count;
    unsigned total;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->work_d[object], 0x20);
        result = kept_obj(0x20, INC8(OBJ->state[object]));
        return result;
    }
    value = RD8(SCROLL->x_lo[0]);
    total = (unsigned)value + 8;
    WR8(SCROLL->x_lo[0], (uint8_t)total);
    count = DEC8(OBJ->work_d[object]);
    result = kept_obj((uint8_t)total, count);
    result.exit.c = total > 0xFF;
    result.exit.c_kept = false;
    result.exit.v = ((value ^ total) & (8 ^ total) & 0x80) != 0;
    result.exit.v_set = true;
    if (count != 0) {
        return result;
    }
    WR8(SCROLL->x_hi[0], 0);
    ClearObject(object);
    return result;
}

void abi_Scene5CScrollUpAndClear(void)
{
    scene_obj_shim(Scene5CScrollUpAndClear(cpu.x));
}

/*
 * @routine 25D3:249 Scene5CDriftFiveObjects
 * @run 6B:8249
 * WHAT IT DOES: Drifts objects 0 to 3 and 15 left at a quarter dot over a scrolling screen for 96 frames, then puts the scroll back and clears itself.
 * HOW: OBJ->state; ScrollXLo; the five slots from $8284 given their X speed and moved by ApplyVelocityX; work_d.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit Scene5CDriftFiveObjects(uint8_t object)
{
    ObjExit result;
    uint8_t state, y, slot, count;
    AxisMove move = { 0 };

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(SCROLL->x_lo[0], 0xA0);
        WR8(OBJ->work_d[object], 0x60);
        return kept_obj(0x60, INC8(OBJ->state[object]));
    }
    INC8(SCROLL->x_lo[0]);
    push8(object);
    y = 0;
    do {
        slot = rd8((uint16_t)(SCENE_DRIFTERS + y));
        WR8(OBJ->vx_lo[slot], 0xA0);
        WR8(OBJ->vx_hi[slot], 0xFF);
        jsr_push(0x8272);
        move = ApplyVelocityX(slot);
        jsr_pop();
        y++;
    } while (y != 5);
    object = pull8();
    count = DEC8(OBJ->work_d[object]);
    result = kept_obj(0, count);
    result.exit.a_kept = true;
    result.exit.y = y;
    result.exit.y_kept = false;
    result.exit.c = true;
    result.exit.c_kept = false;
    result.move = move;
    result.moved = true;
    if (count != 0) {
        return result;
    }
    WR8(SCROLL->x_hi[0], 0);
    ClearObject(object);
    return result;
}

void abi_Scene5CDriftFiveObjects(void)
{
    scene_obj_shim(Scene5CDriftFiveObjects(cpu.x));
}

/*
 * @routine 25D3:293 Scene5CPlayListOnce
 * @run 6B:8293
 * WHAT IT DOES: Plays the object's start argument's frame list until it ends, then moves it on a state unless the argument is negative.
 * HOW: As PlayFrameListOnce, with this overlay's lists.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit Scene5CPlayListOnce(uint8_t object)
{
    ObjExit result;
    uint8_t state, argument;

    state = RD8(OBJ->state[object]);
    if (state != 0) {
        return kept_obj(state, state);
    }
    argument = RD8(OBJ->start_arg[object]);
    jsr_push(0x829E);
    result = Scene5CFrameList(argument, object);
    jsr_pop();
    if (result.exit.nz != 0) {
        return result;
    }
    argument = RD8(OBJ->start_arg[object]);
    result.exit.a = argument;
    result.exit.nz = argument;
    if (argument & 0x80) {
        return result;
    }
    result.exit.nz = INC8(OBJ->state[object]);
    return result;
}

void abi_Scene5CPlayListOnce(void)
{
    scene_obj_shim(Scene5CPlayListOnce(cpu.x));
}

/*
 * @routine 25D3:2A9 Scene5CSlideForCount
 * @run 6B:82A9
 * WHAT IT DOES: As SlideForCount, with the speeds from this overlay's four tables by the start argument.
 * HOW: OBJ->state; tables at $82D9, $82DE, $82E3, $82E8; ApplyVelocity; work_d.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit Scene5CSlideForCount(uint8_t object)
{
    ObjExit result;
    uint8_t state, argument, value, less;
    AxisMove move;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        argument = RD8(OBJ->start_arg[object]);
        value = rd8((uint16_t)(SCENE_SPEEDS_XL + argument));
        WR8(OBJ->vx_lo[object], value);
        value = rd8((uint16_t)(SCENE_SPEEDS_XH + argument));
        WR8(OBJ->vx_hi[object], value);
        value = rd8((uint16_t)(SCENE_SPEEDS_YL + argument));
        WR8(OBJ->vy_lo[object], value);
        value = rd8((uint16_t)(SCENE_SPEEDS_YH + argument));
        WR8(OBJ->vy_hi[object], value);
        result = kept_obj(value, INC8(OBJ->state[object]));
        result.exit.y = argument;
        result.exit.y_kept = false;
        return result;
    }
    less = (uint8_t)(state - 1);
    if (less != 0) {
        return kept_obj(less, less);
    }
    jsr_push(0x82D3);
    move = ApplyVelocity(object);
    jsr_pop();
    result = kept_obj(0, DEC8(OBJ->work_d[object]));
    result.exit.a_kept = true;
    result.move = move;
    result.moved = true;
    if (result.exit.nz != 0) {
        return result;
    }
    result.exit.nz = INC8(OBJ->state[object]);
    return result;
}

void abi_Scene5CSlideForCount(void)
{
    scene_obj_shim(Scene5CSlideForCount(cpu.x));
}

/*
 * @routine 25D3:2ED Scene5CFrameList
 * @run 6B:82ED
 * WHAT IT DOES: Steps an object through the frame list the table at $82FC names for A.
 * HOW: As ObjectFrameList with this table.
 * WAITS: no.
 * Entry: A the list, X the object. Exit: as StepObjectFrames.
 */
ObjExit Scene5CFrameList(uint8_t list, uint8_t object)
{
    return ObjectFrameListAt(list, object, SCENE_FRAME_LISTS);
}

void abi_Scene5CFrameList(void)
{
    scene_obj_shim(Scene5CFrameList(cpu.a, cpu.x));
}

/*
 * @routine 25D3:000 Scene5CStep
 * @run 6B:8000
 * @stop 8007
 * WHAT IT DOES: One call of the scene: the handler of the sub-mode in the table at $8007 (0 begins, 1 runs a frame).
 * HOW: zGameSubMode doubled indexes the table and is jumped through, to the routine loaded there: the cutscene
 * overlays hold these bytes with their own tables.
 * WAITS: yes, through Scene5CBegin.
 * Entry: nothing. Exit: as the handler leaves them.
 */
Exit Scene5CStep(uint8_t kept_y)
{
    Exit exit = { 0 };
    uint8_t state, index, low, high;
    uint16_t target;

    state = RD8(ZP->game_sub_mode);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0x8007 + index));
    high = rd8((uint16_t)(0x8008 + index));
    target = (uint16_t)(low | (high << 8));
    scene_entry_shim(index, index, kept_y, (state & 0x80) != 0);
    run_routine(target);
    exit.from_cpu = true;
    return exit;
}

void abi_Scene5CStep(void)
{
    exit_shim(Scene5CStep(cpu.y));
}
