/*
 * The script engine of the cutscene overlays (bank $6B at $8000): scenes $02-$03, $0A-$0B, $0C-$0D, $0E-$0F and
 * $5C-$5D hold the same engine, from the frame ($801F in the $5C overlay) to the object dispatch, each at its own
 * shift from the $5C overlay's addresses (0, 8, $12, $16, $12). Each routine here is the shared body; an overlay's
 * routine calls it with that overlay's SceneOverlay, so every call, table and return address is that overlay's own.
 * Their sub-mode steps, begin routines and object handlers are their own.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "vram_update.h"
#include "scene_engine.h"

enum { SCENE_OBJECTS = 0x58, VDC_REG_VRAM_A = 2 };

void scene_obj_shim(ObjExit result)
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

static void cpu_command_shim(uint8_t index, uint8_t command)
{
    cpu.a = index;
    cpu.x = index;
    cpu.y = 3;
    abi_nz(3);
    abi_flag(P_C, (command & 0x80) != 0);
}

static void scene_entry_shim(uint8_t high, uint8_t object, uint8_t index, bool carry)
{
    cpu.a = high;
    cpu.x = object;
    cpu.y = index;
    abi_nz(high);
    abi_flag(P_C, carry);
}

static ObjExit kept_obj(uint8_t a, uint8_t nz)
{
    ObjExit result = { 0 };

    result.exit.a = a;
    result.exit.nz = nz;
    result.exit.x_kept = result.exit.y_kept = result.exit.c_kept = true;
    return result;
}


Exit SceneScriptStep(const SceneOverlay *ov);
ObjExit SceneDispatchKind(const SceneOverlay *ov, uint8_t kind, uint8_t object);
Exit SceneFrame(const SceneOverlay *ov);
Exit SceneScriptAdvance(const SceneOverlay *ov);
Exit SceneCommandSubModeTwo(const SceneOverlay *ov);
Exit SceneCommandSetScroll(const SceneOverlay *ov);
Exit SceneCommandStartListAndClear(const SceneOverlay *ov);
Exit SceneCommandStartList(const SceneOverlay *ov);
Exit SceneCommandStartScript1(const SceneOverlay *ov);
Exit SceneCommandStartScript2(const SceneOverlay *ov);
Exit SceneCommandStartScript3(const SceneOverlay *ov);
Exit SceneCommandPlayAdpcm(const SceneOverlay *ov);
Exit SceneCommandCdPlay(const SceneOverlay *ov, uint8_t index);
Exit SceneCommandPlaceKind(const SceneOverlay *ov);
Exit SceneCommandPlaceNext(const SceneOverlay *ov);
Exit SceneCommandPlaceFrame(const SceneOverlay *ov);
Exit SceneCommandPlaceTimed(const SceneOverlay *ov);
void SceneClearObjects(const SceneOverlay *ov);
Exit ScenePlaceObject(const SceneOverlay *ov, uint8_t object, uint8_t y);
Exit SceneCommandPlaceTitleObjectA(const SceneOverlay *ov);
Exit SceneCommandPlaceTitleObjectB(const SceneOverlay *ov);
Exit SceneCommandLoadBanks(const SceneOverlay *ov);
Exit SceneObjectsStep(const SceneOverlay *ov);

/* One frame counted on the scene's 16-bit counter (the bytes of EndingFrameTick). */
uint8_t SceneFrameTick(void)
{
    uint8_t low = INC8(ZP->wait_timer3);

    return low != 0 ? low : INC8(ZP->wait_timer4);
}

/* The objects' loop ($81F5), with what the registers held before it. */
static Exit scene_objects(const SceneOverlay *ov, Exit prior)
{
    Exit exit = { 0 };
    ObjExit result;
    uint8_t slot, kind, a = 0, y = 0;
    bool y_set = false, v_set = false, v = false, via_cpu = false;

    for (slot = 0; slot != SCENE_OBJECTS; slot++) {
        kind = RD8(OBJ->kind[slot]);
        a = kind;
        if (kind == 0) {
            continue;
        }
        jsr_push((uint16_t)(0x81FE + ov->shift));
        result = SceneDispatchKind(ov, kind, slot);
        jsr_pop();
        if (result.exit.from_cpu) {
            via_cpu = true;
            continue;
        }
        via_cpu = false;
        a = result.exit.a;
        if (result.moved) {
            if (result.exit.a_kept) {
                a = scene_move_high(result.move);
            }
            if (!result.exit.v_set) {
                result.exit.v = scene_move_v(result.move);
                result.exit.v_set = true;
            }
        }
        if (!result.exit.x_kept) {
            slot = result.exit.x;
        }
        if (!result.exit.y_kept) {
            y = result.exit.y;
            y_set = true;
        }
        if (result.exit.v_set) {
            v = result.exit.v;
            v_set = true;
        }
    }
    if (via_cpu) {
        exit.from_cpu = true;
        cpu.a = a;
        cpu.x = SCENE_OBJECTS;
        abi_nz(0);
        abi_flag(P_C, true);
        return exit;
    }
    exit.a = a;
    exit.x = SCENE_OBJECTS;
    exit.y = y_set ? y : prior.y;
    exit.y_kept = y_set ? false : prior.y_kept;
    exit.nz = 0;
    exit.c = true;
    exit.v = v_set ? v : prior.v;
    exit.v_set = v_set ? true : prior.v_set;
    return exit;
}

Exit SceneFrame(const SceneOverlay *ov)
{
    Exit exit;

    jsr_push((uint16_t)(0x8022 + ov->shift));
    (void)SceneFrameTick();
    jsr_pop();
    jsr_push((uint16_t)(0x8025 + ov->shift));
    exit = SceneScriptStep(ov);
    jsr_pop();
    jsr_push((uint16_t)(0x8028 + ov->shift));
    exit = scene_objects(ov, exit);
    jsr_pop();
    return exit;
}

Exit SceneScriptAdvance(const SceneOverlay *ov)
{
    Exit exit;
    uint8_t command, length, low, sum;
    unsigned total;
    bool overflow;

    command = RD8_PTR(ZP->script_ptr);
    length = rd8((uint16_t)(0x8030 + ov->shift + command));
    low = RD8(ZP->script_ptr);
    total = (unsigned)length + low;
    sum = (uint8_t)total;
    overflow = ((length ^ sum) & (low ^ sum) & 0x80) != 0;
    WR8(ZP->script_ptr, sum);
    if (total > 0xFF) {
        INC8(ZP->script_ptr_hi);
    }
    exit = SceneScriptStep(ov);
    if (exit.x_kept) {
        exit.x = command;
        exit.x_kept = false;
    }
    if (!exit.v_set) {
        exit.v = overflow;
        exit.v_set = true;
    }
    return exit;
}

Exit SceneScriptStep(const SceneOverlay *ov)
{
    Exit exit = { 0 };
    uint8_t time, counter, command, index, low, high;
    uint16_t target;

    time = RD8_PTR_Y(ZP->script_ptr, 1);
    counter = RD8(ZP->wait_timer3);
    if (time != counter) {
        exit.a = time;
        exit.y = 1;
        exit.nz = (uint8_t)(time - counter);
        exit.c = time >= counter;
        exit.x_kept = true;
        return exit;
    }
    time = RD8_PTR_Y(ZP->script_ptr, 2);
    counter = RD8(ZP->wait_timer4);
    if (time != counter) {
        exit.a = time;
        exit.y = 2;
        exit.nz = (uint8_t)(time - counter);
        exit.c = time >= counter;
        exit.x_kept = true;
        return exit;
    }
    command = RD8_PTR(ZP->script_ptr);
    index = (uint8_t)(command << 1);
    low = rd8((uint16_t)(0x806B + ov->shift + index));
    high = rd8((uint16_t)(0x806B + ov->shift + 1 + index));
    target = (uint16_t)(low | (high << 8));
    switch ((uint16_t)(target - ov->shift)) {
    case 0x80A5: return SceneCommandStartListAndClear(ov);
    case 0x80C0: return SceneCommandStartScript1(ov);
    case 0x80E8: return SceneCommandPlaceKind(ov);
    case 0x8111: return SceneCommandPlaceFrame(ov);
    case 0x80D8: return SceneCommandPlayAdpcm(ov);
    case 0x80C8: return SceneCommandStartScript2(ov);
    case 0x80D0: return SceneCommandStartScript3(ov);
    case 0x80E0: return SceneCommandCdPlay(ov, index);
    case 0x8172: return SceneCommandPlaceTitleObjectA(ov);
    case 0x8199: return SceneCommandPlaceTitleObjectB(ov);
    case 0x808D: return SceneCommandSubModeTwo(ov);
    case 0x80B9: return SceneCommandStartList(ov);
    case 0x80F7: return SceneCommandPlaceNext(ov);
    case 0x8126: return SceneCommandPlaceTimed(ov);
    case 0x81C0: return SceneCommandLoadBanks(ov);
    case 0x8093: return SceneCommandSetScroll(ov);
    case 0x8068:
        exit.a = index;
        exit.x = index;
        exit.y = 3;
        exit.nz = DEC8(ZP->mode_done);
        exit.c = (command & 0x80) != 0;
        return exit;
    default:
        break;
    }
    cpu_command_shim(index, command);
    run_routine(target);
    exit.from_cpu = true;
    return exit;
}

Exit SceneCommandSubModeTwo(const SceneOverlay *ov)
{
    WR8(ZP->game_sub_mode, 2);
    return SceneScriptAdvance(ov);
}

Exit SceneCommandSetScroll(const SceneOverlay *ov)
{
    uint8_t operand = RD8_PTR_Y(ZP->script_ptr, 3);

    push8(operand);
    WR8(SCROLL->x_hi[0], (uint8_t)(operand >> 4));
    operand = pull8();
    WR8(SCROLL->y_hi[0], (uint8_t)(operand & 0x0F));
    return SceneScriptAdvance(ov);
}

Exit SceneCommandStartListAndClear(const SceneOverlay *ov)
{
    uint8_t list = RD8_PTR_Y(ZP->script_ptr, 3);

    jsr_push((uint16_t)(0x80AA + ov->shift));
    StartListChannel0Kind2(list);
    jsr_pop();
    jsr_push((uint16_t)(0x80AD + ov->shift));
    ClearScreenState(wait_regs_xy_shim(0, 3));
    jsr_pop();
    jsr_push((uint16_t)(0x80B0 + ov->shift));
    SceneClearObjects(ov);
    jsr_pop();
    block_move_tai(KernelZeros, ScrollXFrac, 0x00EE, VDC_REG_VRAM_A, SCENE_OBJECTS, 0xFF);
    return SceneScriptAdvance(ov);
}

Exit SceneCommandStartList(const SceneOverlay *ov)
{
    uint8_t list = RD8_PTR_Y(ZP->script_ptr, 3);

    jsr_push((uint16_t)(0x80BE + ov->shift));
    StartListChannel0Kind2(list);
    jsr_pop();
    return SceneScriptAdvance(ov);
}

Exit SceneCommandStartScript1(const SceneOverlay *ov)
{
    uint8_t script = RD8_PTR_Y(ZP->script_ptr, 3);

    jsr_push((uint16_t)(0x80C5 + ov->shift));
    (void)StartScriptSlot0Kind1(script);
    jsr_pop();
    return SceneScriptAdvance(ov);
}

Exit SceneCommandStartScript2(const SceneOverlay *ov)
{
    uint8_t script = RD8_PTR_Y(ZP->script_ptr, 3);

    jsr_push((uint16_t)(0x80CD + ov->shift));
    (void)StartScriptSlot0Kind2(script);
    jsr_pop();
    return SceneScriptAdvance(ov);
}

Exit SceneCommandStartScript3(const SceneOverlay *ov)
{
    uint8_t script = RD8_PTR_Y(ZP->script_ptr, 3);

    jsr_push((uint16_t)(0x80D5 + ov->shift));
    (void)StartScriptSlot0Kind3(script);
    jsr_pop();
    return SceneScriptAdvance(ov);
}

Exit SceneCommandPlayAdpcm(const SceneOverlay *ov)
{
    uint8_t sound = RD8_PTR_Y(ZP->script_ptr, 3);

    jsr_push((uint16_t)(0x80DD + ov->shift));
    (void)AdpcmPlayFromDisc(sound);
    jsr_pop();
    return SceneScriptAdvance(ov);
}

Exit SceneCommandCdPlay(const SceneOverlay *ov, uint8_t index)
{
    uint8_t track = RD8_PTR_Y(ZP->script_ptr, 3);

    jsr_push((uint16_t)(0x80E5 + ov->shift));
    (void)CdPlayOnce(track, index, 3);
    jsr_pop();
    return SceneScriptAdvance(ov);
}

Exit SceneCommandPlaceKind(const SceneOverlay *ov)
{
    uint8_t object, y, value;

    object = RD8_PTR_Y(ZP->script_ptr, 3);
    jsr_push((uint16_t)(0x80EE + ov->shift));
    (void)ScenePlaceObject(ov, object, 3);
    jsr_pop();
    y = 7;
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->kind[object], value);
    return SceneScriptAdvance(ov);
}

Exit SceneCommandPlaceNext(const SceneOverlay *ov)
{
    uint8_t object, y, value;

    object = RD8_PTR_Y(ZP->script_ptr, 3);
    jsr_push((uint16_t)(0x80FD + ov->shift));
    (void)ScenePlaceObject(ov, object, 3);
    jsr_pop();
    y = 7;
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->start_arg[object], value);
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->sprite_set[object], value);
    WR8(OBJ->kind[object], 2);
    return SceneScriptAdvance(ov);
}

Exit SceneCommandPlaceFrame(const SceneOverlay *ov)
{
    uint8_t object, y, value;

    object = RD8_PTR_Y(ZP->script_ptr, 3);
    jsr_push((uint16_t)(0x8117 + ov->shift));
    (void)ScenePlaceObject(ov, object, 3);
    jsr_pop();
    y = 7;
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->frame[object], value);
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->sprite_set[object], value);
    return SceneScriptAdvance(ov);
}

Exit SceneCommandPlaceTimed(const SceneOverlay *ov)
{
    uint8_t object, y, value;

    object = RD8_PTR_Y(ZP->script_ptr, 3);
    jsr_push((uint16_t)(0x812C + ov->shift));
    (void)ScenePlaceObject(ov, object, 3);
    jsr_pop();
    y = 7;
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->frame[object], value);
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->sprite_set[object], value);
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->start_arg[object], value);
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->work_d[object], value);
    WR8(OBJ->kind[object], 3);
    return SceneScriptAdvance(ov);
}

void SceneClearObjects(const SceneOverlay *ov)
{
    uint8_t slot;

    for (slot = 0; slot != SCENE_OBJECTS; slot++) {
        jsr_push((uint16_t)(0x8150 + ov->shift));
        ClearObject(slot);
        jsr_pop();
    }
}

Exit ScenePlaceObject(const SceneOverlay *ov, uint8_t object, uint8_t y)
{
    Exit exit = { 0 };
    uint8_t value;

    jsr_push((uint16_t)(0x8159 + ov->shift));
    ClearObject(object);
    jsr_pop();
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->x_lo[object], value);
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->x_hi[object], value);
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->y_lo[object], value);
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->y_hi[object], value);
    exit.a = value;
    exit.y = y;
    exit.nz = value;
    exit.x_kept = true;
    exit.c_kept = true;
    return exit;
}

Exit SceneCommandPlaceTitleObjectA(const SceneOverlay *ov)
{
    uint8_t value;

    jsr_push((uint16_t)(0x8177 + ov->shift));
    ClearObject(0x2E);
    jsr_pop();
    INC8(OBJ->kind[0x2E]);
    value = RD8_PTR_Y(ZP->script_ptr, 3);
    WR8(OBJ->start_arg[0x2E], value);
    value = RD8_PTR_Y(ZP->script_ptr, 4);
    WR8(OBJ->work_d[0x2E], value);
    value = RD8_PTR_Y(ZP->script_ptr, 5);
    WR8(OBJ->work_e[0x2E], value);
    value = RD8_PTR_Y(ZP->script_ptr, 6);
    WR8(OBJ->work_f[0x2E], value);
    WR8(OBJ->y_lo[0x2E], 0xC8);
    return SceneScriptAdvance(ov);
}

Exit SceneCommandPlaceTitleObjectB(const SceneOverlay *ov)
{
    uint8_t value;

    jsr_push((uint16_t)(0x819E + ov->shift));
    ClearObject(0x2F);
    jsr_pop();
    INC8(OBJ->kind[0x2F]);
    value = RD8_PTR_Y(ZP->script_ptr, 3);
    WR8(OBJ->start_arg[0x2F], value);
    value = RD8_PTR_Y(ZP->script_ptr, 4);
    WR8(OBJ->work_d[0x2F], value);
    value = RD8_PTR_Y(ZP->script_ptr, 5);
    WR8(OBJ->work_e[0x2F], value);
    value = RD8_PTR_Y(ZP->script_ptr, 6);
    WR8(OBJ->work_f[0x2F], value);
    WR8(OBJ->y_lo[0x2F], 0xD8);
    return SceneScriptAdvance(ov);
}

Exit SceneCommandLoadBanks(const SceneOverlay *ov)
{
    uint8_t i, bank, rotated, carry, out, round;

    WR8(ZP->temp00, 0);
    do {
        i = RD8(ZP->temp00);
        bank = (uint8_t)((uint8_t)(i << 1) + 0x10 + RD8(ZP->ram_base_bank));
        tam(0x20, bank);
        tam(0x40, (uint8_t)(bank + 1));
        WR8(ZP->bios_c.lo, 0);
        rotated = RD8(ZP->temp00);
        carry = 0;
        for (round = 0; round < 3; round++) {
            out = (uint8_t)(rotated & 1);
            rotated = (uint8_t)((rotated >> 1) | (carry << 7));
            carry = out;
        }
        WR8(ZP->bios_c.hi, rotated);
        WR8(ZP->bios_d.hi, 0);
        WR8(ZP->bios_b.lo, 0);
        WR8(ZP->bios_b.hi, 0xA0);
        WR8(ZP->bios_a.lo, 0);
        WR8(ZP->bios_a.hi, 0x40);
        jsr_push((uint16_t)(0x81EA + ov->shift));
        (void)vec_ad_read();
        jsr_pop();
        INC8(ZP->temp00);
    } while (RD8(ZP->temp00) != 4);
    return SceneScriptAdvance(ov);
}

Exit SceneObjectsStep(const SceneOverlay *ov)
{
    Exit none = { 0 };

    none.y_kept = true;
    return scene_objects(ov, none);
}

ObjExit SceneDispatchKind(const SceneOverlay *ov, uint8_t kind, uint8_t object)
{
    ObjExit result = { 0 };
    uint8_t less, index, low, high, high_read;
    uint16_t target;
    bool carry;

    less = (uint8_t)(kind - 1);
    index = (uint8_t)(less << 1);
    carry = (less & 0x80) != 0;
    low = rd8((uint16_t)(0x8214 + ov->shift + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0x8214 + ov->shift + 1 + index));
    WR8(ZP->temp01, high);
    low = RD8(ZP->temp00);
    high_read = RD8(ZP->temp01);
    target = (uint16_t)(low | (high_read << 8));
    if (!ov->handler(target, object, &result)) {
        scene_entry_shim(high, object, index, carry);
        run_routine(target);
        result.exit.from_cpu = true;
        return result;
    }
    if (result.exit.a_kept && !result.moved) {
        result.exit.a = high;
        result.exit.a_kept = false;
    }
    if (result.exit.y_kept) {
        result.exit.y = index;
        result.exit.y_kept = false;
    }
    if (result.exit.c_kept && !result.moved) {
        result.exit.c = carry;
        result.exit.c_kept = false;
    }
    return result;
}


/* An object handler's exit that keeps what it does not name. */
ObjExit scene_kept(uint8_t a, uint8_t nz)
{
    ObjExit result = { 0 };

    result.exit.a = a;
    result.exit.nz = nz;
    result.exit.x_kept = result.exit.y_kept = result.exit.c_kept = true;
    return result;
}

/* The handler that plays the frame list the start argument names until it ends, then moves on a state unless the
 * argument is negative (the 5C overlay's $8293): lists the overlay's frame list table, ret the JSR's return. */
ObjExit ScenePlayListOnce(uint16_t lists, uint16_t ret, uint8_t object)
{
    ObjExit result;
    uint8_t state, argument;

    state = RD8(OBJ->state[object]);
    if (state != 0) {
        return scene_kept(state, state);
    }
    argument = RD8(OBJ->start_arg[object]);
    jsr_push(ret);
    result = ObjectFrameListAt(argument, object, lists);
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

/* The handler that gives the object the speeds its start argument names (four tables: X low, X high, Y low, Y high),
 * then moves it for work_d frames (the 5C overlay's $82A9): ret the return of its JSR to ApplyVelocity. */
ObjExit SceneSlide(const uint16_t speeds[4], uint16_t ret, uint8_t object)
{
    ObjExit result;
    uint8_t state, argument, value, less;
    AxisMove move;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        argument = RD8(OBJ->start_arg[object]);
        value = rd8((uint16_t)(speeds[0] + argument));
        WR8(OBJ->vx_lo[object], value);
        value = rd8((uint16_t)(speeds[1] + argument));
        WR8(OBJ->vx_hi[object], value);
        value = rd8((uint16_t)(speeds[2] + argument));
        WR8(OBJ->vy_lo[object], value);
        value = rd8((uint16_t)(speeds[3] + argument));
        WR8(OBJ->vy_hi[object], value);
        result = scene_kept(value, INC8(OBJ->state[object]));
        result.exit.y = argument;
        result.exit.y_kept = false;
        return result;
    }
    less = (uint8_t)(state - 1);
    if (less != 0) {
        return scene_kept(less, less);
    }
    jsr_push(ret);
    move = ApplyVelocity(object);
    jsr_pop();
    result = scene_kept(0, DEC8(OBJ->work_d[object]));
    result.exit.a_kept = true;
    result.move = move;
    result.moved = true;
    if (result.exit.nz != 0) {
        return result;
    }
    result.exit.nz = INC8(OBJ->state[object]);
    return result;
}

/* A jump by the object's state through a table of words (the $5C overlay has none): the state doubled in Y, the word
 * through zTemp00 and zTemp01; the target. */
uint16_t SceneStateJump(uint16_t table, uint8_t object, uint8_t *index, uint8_t *high)
{
    uint8_t state, low;

    state = RD8(OBJ->state[object]);
    *index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(table + *index));
    WR8(ZP->temp00, low);
    *high = rd8((uint16_t)(table + 1 + *index));
    WR8(ZP->temp01, *high);
    low = RD8(ZP->temp00);
    return (uint16_t)(low | (RD8(ZP->temp01) << 8));
}
