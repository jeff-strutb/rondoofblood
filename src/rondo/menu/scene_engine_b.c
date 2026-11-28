/*
 * The script engine of the cutscene overlays after a stage (bank $6B at $8000): scenes $58-$59 (sector $267B) and
 * $5A-$5B (sector $2703). It is the engine of scene_engine.c, instruction for instruction where the two share a piece,
 * with a third sub-mode (wait for RUN), a command that ends the script without stepping it, and, in $58's overlay,
 * two more commands. Its pieces do not sit at one shift from the $5C overlay's, so each overlay's layout names them,
 * and each command's body takes the address it runs at: its calls' return addresses are its own.
 * The overlays' routines (scene_58.c, scene_5a.c) call these bodies with their layout and address.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "vram_update.h"
#include "scene_engine.h"
#include "scene_engine_b.h"

enum { SCENE_OBJECTS = 0x58, VDC_REG_VRAM_A = 2, PAD_RUN = 0x08 };

/* The registers a command outside this C is entered with: A and X the doubled command, Y 3, N and Z from the LDY,
 * C from the doubling. */
static void command_entry_shim(uint8_t index, bool carry)
{
    cpu.a = index;
    cpu.x = index;
    cpu.y = 3;
    abi_nz(3);
    abi_flag(P_C, carry);
}

/* The registers an object handler outside this C is entered with: A the handler's high byte, X the object, Y the
 * doubled kind less one, N and Z from the LDA, C from the doubling. */
static void handler_entry_shim(uint8_t high, uint8_t object, uint8_t index, bool carry)
{
    cpu.a = high;
    cpu.x = object;
    cpu.y = index;
    abi_nz(high);
    abi_flag(P_C, carry);
}

/* The sub-mode 0 handler: ResetGameState, the script pointer set, the frame counter $FFFF, the sub-mode stepped
 * (Scene5CBegin's instructions with this overlay's script). at: its address. */
Exit SceneBBegin(uint16_t at, uint8_t script_lo, uint8_t script_hi, uint8_t kept_x, uint8_t kept_y)
{
    Exit exit = { 0 };

    jsr_push((uint16_t)(at + 3));
    ResetGameState(wait_regs_xy_shim(kept_x, kept_y));
    jsr_pop();
    WR8(ZP->script_ptr, script_lo);
    WR8(ZP->script_ptr_hi, script_hi);
    WR8(ZP->wait_timer3, 0xFF);
    WR8(ZP->wait_timer4, 0xFF);
    exit.a = 0xFF;
    exit.x = 0;
    exit.y = 0xFF;
    exit.nz = INC8(ZP->game_sub_mode);
    exit.c_kept = true;
    return exit;
}

/* The objects' loop: every slot with a kind run through its handler; prior the registers before the loop. */
static Exit scene_objects(const SceneLayoutB *ov, Exit prior)
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
        jsr_push((uint16_t)(ov->objects + 9));
        result = SceneBDispatchKind(ov, kind, slot);
        jsr_pop();
        if (result.exit.from_cpu) {
            via_cpu = true;
            continue;
        }
        via_cpu = false;
        a = result.exit.a;
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

/* The sub-mode 1 handler: the frame counted (the bytes of EndingFrameTick), the script stepped, the objects run by
 * the routine loaded at the overlay's objects' place. at: its address. */
Exit SceneBFrame(const SceneLayoutB *ov, uint16_t at)
{
    Exit exit, objects;

    jsr_push((uint16_t)(at + 3));
    (void)SceneFrameTick();
    jsr_pop();
    jsr_push((uint16_t)(at + 6));
    exit = SceneBScriptStep(ov);
    jsr_pop();
    jsr_push((uint16_t)(at + 9));
    objects = ov->objects_step();
    jsr_pop();
    /* What the objects' loop leaves alone is what the step left. */
    if (!objects.from_cpu) {
        if (objects.y_kept) {
            objects.y = exit.y;
            objects.y_kept = exit.y_kept;
        }
        if (!objects.v_set) {
            objects.v = exit.v;
            objects.v_set = exit.v_set;
        }
    }
    return objects;
}

/* The sub-mode 2 handler: the mode is done when RUN is newly pressed. */
Exit SceneBWaitForRun(void)
{
    Exit exit = { 0 };
    uint8_t pressed;

    pressed = (uint8_t)(rd8(BiosJoyTrigger) & PAD_RUN);
    exit.a = pressed;
    exit.nz = pressed;
    exit.x_kept = exit.y_kept = exit.c_kept = true;
    if (pressed == 0) {
        return exit;
    }
    exit.nz = INC8(ZP->mode_done);
    return exit;
}

/* The script stepped past its command (its length from the overlay's table), then the next step. */
Exit SceneBScriptAdvance(const SceneLayoutB *ov)
{
    Exit exit;
    uint8_t command, length, low, sum;
    unsigned total;
    bool overflow;

    command = RD8_PTR(ZP->script_ptr);
    length = rd8((uint16_t)(ov->lengths + command));
    low = RD8(ZP->script_ptr);
    total = (unsigned)length + low;
    sum = (uint8_t)total;
    overflow = ((length ^ sum) & (low ^ sum) & 0x80) != 0;
    WR8(ZP->script_ptr, sum);
    if (total > 0xFF) {
        INC8(ZP->script_ptr_hi);
    }
    exit = SceneBScriptStep(ov);
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

/* The script's command run when its two time bytes equal the frame counter, through the overlay's table. */
Exit SceneBScriptStep(const SceneLayoutB *ov)
{
    Exit exit = { 0 };
    uint8_t time, counter, command, index, low, high;
    uint16_t target;
    bool carry;

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
    carry = (command & 0x80) != 0;
    low = rd8((uint16_t)(ov->commands + index));
    high = rd8((uint16_t)(ov->commands + 1 + index));
    target = (uint16_t)(low | (high << 8));
    if (ov->command(target, index, carry, &exit)) {
        return exit;
    }
    command_entry_shim(index, carry);
    run_routine(target);
    exit.from_cpu = true;
    return exit;
}

/* A command that ends the script: the sub-mode set to 2 (wait for RUN), the script not stepped. Entered with X the
 * doubled command, Y 3 and C from the doubling, which it leaves. */
Exit SceneBCommandEnd(uint8_t index, bool carry)
{
    Exit exit = { 0 };

    WR8(ZP->game_sub_mode, 2);
    exit.a = 2;
    exit.x = index;
    exit.y = 3;
    exit.nz = 2;
    exit.c = carry;
    return exit;
}

Exit SceneBCommandSubModeTwo(const SceneLayoutB *ov)
{
    WR8(ZP->game_sub_mode, 2);
    return SceneBScriptAdvance(ov);
}

Exit SceneBCommandSetScroll(const SceneLayoutB *ov)
{
    uint8_t operand = RD8_PTR_Y(ZP->script_ptr, 3);

    push8(operand);
    WR8(SCROLL->x_hi[0], (uint8_t)(operand >> 4));
    operand = pull8();
    WR8(SCROLL->y_hi[0], (uint8_t)(operand & 0x0F));
    return SceneBScriptAdvance(ov);
}

/* The copy of EndingClearObjects at the overlay's place: ClearObject for every slot. */
static void scene_clear_objects(const SceneLayoutB *ov)
{
    uint8_t slot;

    for (slot = 0; slot != SCENE_OBJECTS; slot++) {
        jsr_push((uint16_t)(ov->clear + 4));
        ClearObject(slot);
        jsr_pop();
    }
}

/* The copy of EndingPlaceObject at the overlay's place: the object cleared, then its X and Y from the four script
 * bytes after Y. Leaves A the last byte, Y on it, X and C kept. */
static Exit scene_place_object(const SceneLayoutB *ov, uint8_t object, uint8_t y)
{
    Exit exit = { 0 };
    uint8_t value;

    jsr_push((uint16_t)(ov->place + 3));
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

Exit SceneBCommandStartListAndClear(const SceneLayoutB *ov, uint16_t at)
{
    uint8_t list = RD8_PTR_Y(ZP->script_ptr, 3);

    jsr_push((uint16_t)(at + 5));
    StartListChannel0Kind2(list);
    jsr_pop();
    jsr_push((uint16_t)(at + 8));
    ClearScreenState(wait_regs_xy_shim(0, 3));
    jsr_pop();
    jsr_push((uint16_t)(at + 11));
    scene_clear_objects(ov);
    jsr_pop();
    block_move_tai(KernelZeros, ScrollXFrac, 0x00EE, VDC_REG_VRAM_A, SCENE_OBJECTS, 0xFF);
    return SceneBScriptAdvance(ov);
}

Exit SceneBCommandStartList(const SceneLayoutB *ov, uint16_t at)
{
    uint8_t list = RD8_PTR_Y(ZP->script_ptr, 3);

    jsr_push((uint16_t)(at + 5));
    StartListChannel0Kind2(list);
    jsr_pop();
    return SceneBScriptAdvance(ov);
}

Exit SceneBCommandStartScript1(const SceneLayoutB *ov, uint16_t at)
{
    uint8_t script = RD8_PTR_Y(ZP->script_ptr, 3);

    jsr_push((uint16_t)(at + 5));
    (void)StartScriptSlot0Kind1(script);
    jsr_pop();
    return SceneBScriptAdvance(ov);
}

Exit SceneBCommandStartScript2(const SceneLayoutB *ov, uint16_t at)
{
    uint8_t script = RD8_PTR_Y(ZP->script_ptr, 3);

    jsr_push((uint16_t)(at + 5));
    (void)StartScriptSlot0Kind2(script);
    jsr_pop();
    return SceneBScriptAdvance(ov);
}

Exit SceneBCommandStartScript3(const SceneLayoutB *ov, uint16_t at)
{
    uint8_t script = RD8_PTR_Y(ZP->script_ptr, 3);

    jsr_push((uint16_t)(at + 5));
    (void)StartScriptSlot0Kind3(script);
    jsr_pop();
    return SceneBScriptAdvance(ov);
}

Exit SceneBCommandPlayAdpcm(const SceneLayoutB *ov, uint16_t at)
{
    uint8_t sound = RD8_PTR_Y(ZP->script_ptr, 3);

    jsr_push((uint16_t)(at + 5));
    (void)AdpcmPlayFromDisc(sound);
    jsr_pop();
    return SceneBScriptAdvance(ov);
}

Exit SceneBCommandCdPlay(const SceneLayoutB *ov, uint16_t at, uint8_t index)
{
    uint8_t track = RD8_PTR_Y(ZP->script_ptr, 3);

    jsr_push((uint16_t)(at + 5));
    (void)CdPlayOnce(track, index, 3);
    jsr_pop();
    return SceneBScriptAdvance(ov);
}

Exit SceneBCommandPlaceKind(const SceneLayoutB *ov, uint16_t at)
{
    uint8_t object, y, value;

    object = RD8_PTR_Y(ZP->script_ptr, 3);
    jsr_push((uint16_t)(at + 6));
    (void)scene_place_object(ov, object, 3);
    jsr_pop();
    y = 7;
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->kind[object], value);
    return SceneBScriptAdvance(ov);
}

Exit SceneBCommandPlaceNext(const SceneLayoutB *ov, uint16_t at)
{
    uint8_t object, y, value;

    object = RD8_PTR_Y(ZP->script_ptr, 3);
    jsr_push((uint16_t)(at + 6));
    (void)scene_place_object(ov, object, 3);
    jsr_pop();
    y = 7;
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->start_arg[object], value);
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->sprite_set[object], value);
    WR8(OBJ->kind[object], 2);
    return SceneBScriptAdvance(ov);
}

Exit SceneBCommandPlaceFrame(const SceneLayoutB *ov, uint16_t at)
{
    uint8_t object, y, value;

    object = RD8_PTR_Y(ZP->script_ptr, 3);
    jsr_push((uint16_t)(at + 6));
    (void)scene_place_object(ov, object, 3);
    jsr_pop();
    y = 7;
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->frame[object], value);
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->sprite_set[object], value);
    return SceneBScriptAdvance(ov);
}

Exit SceneBCommandPlaceTimed(const SceneLayoutB *ov, uint16_t at)
{
    uint8_t object, y, value;

    object = RD8_PTR_Y(ZP->script_ptr, 3);
    jsr_push((uint16_t)(at + 6));
    (void)scene_place_object(ov, object, 3);
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
    return SceneBScriptAdvance(ov);
}

/* The $58 overlay's command 17: an object placed with work_e, sprite set, start argument and work_d, kind 4. */
Exit SceneBCommandPlaceWorked(const SceneLayoutB *ov, uint16_t at)
{
    uint8_t object, y, value;

    object = RD8_PTR_Y(ZP->script_ptr, 3);
    jsr_push((uint16_t)(at + 6));
    (void)scene_place_object(ov, object, 3);
    jsr_pop();
    y = 7;
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->work_e[object], value);
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->sprite_set[object], value);
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->start_arg[object], value);
    y++;
    value = RD8_PTR_Y(ZP->script_ptr, y);
    WR8(OBJ->work_d[object], value);
    WR8(OBJ->kind[object], 4);
    return SceneBScriptAdvance(ov);
}

/* Slot $2E cleared and started with kind 1 and the four script bytes from Y 3 (Y stays 3 across ClearObject). */
static void title_object_a(uint16_t at)
{
    uint8_t value;

    jsr_push((uint16_t)(at + 5));
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
}

Exit SceneBCommandPlaceTitleObjectA(const SceneLayoutB *ov, uint16_t at)
{
    title_object_a(at);
    WR8(OBJ->y_lo[0x2E], 0xC8);
    return SceneBScriptAdvance(ov);
}

/* The $58 overlay's version: Y low $40, and X high decremented. */
Exit SceneBCommandPlaceTitleObjectALow(const SceneLayoutB *ov, uint16_t at)
{
    title_object_a(at);
    WR8(OBJ->y_lo[0x2E], 0x40);
    DEC8(OBJ->x_hi[0x2E]);
    return SceneBScriptAdvance(ov);
}

Exit SceneBCommandPlaceTitleObjectB(const SceneLayoutB *ov, uint16_t at)
{
    uint8_t value;

    jsr_push((uint16_t)(at + 5));
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
    return SceneBScriptAdvance(ov);
}

/* The $58 overlay's command 18: when the low four bits of ProgressCharacter are all set, the script pointer is set to
 * $9225 and zRasterStage cleared before the step (which then steps past the command at $9225); otherwise the script
 * steps on past this command. */
Exit SceneBCommandBranchOnProgress(const SceneLayoutB *ov)
{
    uint8_t bits;

    bits = (uint8_t)(rd8(ProgressCharacter) & 0x0F);
    if (bits != 0x0F) {
        return SceneBScriptAdvance(ov);
    }
    WR8(ZP->script_ptr, 0x25);
    WR8(ZP->script_ptr_hi, 0x92);
    WR8(ZP->raster_stage, 0);
    return SceneBScriptAdvance(ov);
}

Exit SceneBCommandLoadBanks(const SceneLayoutB *ov, uint16_t at)
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
        jsr_push((uint16_t)(at + 0x2A));
        (void)vec_ad_read();
        jsr_pop();
        INC8(ZP->temp00);
    } while (RD8(ZP->temp00) != 4);
    return SceneBScriptAdvance(ov);
}

Exit SceneBObjectsStep(const SceneLayoutB *ov)
{
    Exit none = { 0 };

    none.y_kept = true;
    return scene_objects(ov, none);
}

/* The object's handler from the overlay's table by kind, through zTemp00 and zTemp01. */
ObjExit SceneBDispatchKind(const SceneLayoutB *ov, uint8_t kind, uint8_t object)
{
    ObjExit result = { 0 };
    uint8_t less, index, low, high, high_read;
    uint16_t target;
    bool carry;

    less = (uint8_t)(kind - 1);
    index = (uint8_t)(less << 1);
    carry = (less & 0x80) != 0;
    low = rd8((uint16_t)(ov->kinds + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(ov->kinds + 1 + index));
    WR8(ZP->temp01, high);
    low = RD8(ZP->temp00);
    high_read = RD8(ZP->temp01);
    target = (uint16_t)(low | (high_read << 8));
    if (!ov->handler(target, object, &result)) {
        handler_entry_shim(high, object, index, carry);
        run_routine(target);
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
    if (result.exit.c_kept && !result.moved) {
        result.exit.c = carry;
        result.exit.c_kept = false;
    }
    return result;
}
