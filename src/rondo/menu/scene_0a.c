/*
 * The cutscene overlay of sector $23F6 (bank $6B at $8000; scenes $0A and $0B): the shared script engine
 * (scene_engine.c) at this overlay's places, $12 past the $5C overlay's, and this overlay's own begin and
 * object handlers.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "scene_engine.h"
#include "scene_0a.h"

/* SCENE0A HANDLERS */

static void scene_entry_shim(uint8_t high, uint8_t object, uint8_t index)
{
    cpu.a = high;
    cpu.x = object;
    cpu.y = index;
    abi_nz(high);
}

/* The swoop's place ($82D5 and $832C): the velocity of angle work_d, then X $90 plus its high byte, Y $FF plus the Y
 * velocity's high byte as a signed 16-bit place; C and V from the high byte's add. */
static ObjExit swoop_place(uint8_t object, uint16_t ret)
{
    uint8_t angle, base, sign, high;
    unsigned total;
    ObjExit result;

    angle = RD8(OBJ->work_d[object]);
    jsr_push(ret);
    (void)SetVelocityFromAngle(angle, object);
    jsr_pop();
    base = RD8(OBJ->vx_hi[object]);
    WR8(OBJ->x_lo[object], (uint8_t)(0x90 + base));
    WR8(ZP->temp00, 0);
    if (RD8(OBJ->vy_hi[object]) & 0x80) {
        DEC8(ZP->temp00);
    }
    base = RD8(OBJ->vy_hi[object]);
    total = 0xFFu + base;
    WR8(OBJ->y_lo[object], (uint8_t)total);
    sign = RD8(ZP->temp00);
    high = (uint8_t)(sign + (total > 0xFF ? 1 : 0));
    WR8(OBJ->y_hi[object], high);
    result = scene_kept(0, 0);
    result.exit.c = sign == 0xFF && total > 0xFF;
    result.exit.c_kept = false;
    result.exit.v = sign == 0x7F && total > 0xFF;
    result.exit.v_set = true;
    return result;
}

/*
 * @routine 23F6:00B Scene0ABegin
 * @run 6B:800B
 * WHAT IT DOES: Starts the scene of $0A and $0B: bit 1 of ProgressCharacter, the script of the character from $802D.
 * HOW: ResetGameState; an ORA into ProgressCharacter; the script pointer, the frame counter $FFFF, the sub-mode up.
 * WAITS: yes, through ResetGameState (the vertical blank).
 * Entry: X the doubled sub-mode and Y kept. Exit: A $FF, X 0, N and Z from the new state; Y and C from the character
 * where the script is picked by it, else Y $FF and C kept.
 */
Exit Scene0ABegin(uint8_t kept_x, uint8_t kept_y)
{
    Exit exit = { 0 };
    uint8_t character, row;

    jsr_push(0x800E);
    ResetGameState(wait_regs_xy_shim(kept_x, kept_y));
    jsr_pop();
    wr8(ProgressCharacter, (uint8_t)(rd8(ProgressCharacter) | 0x02));
    character = rd8(zCharacter);
    row = (uint8_t)(character << 1);
    WR8(ZP->script_ptr, rd8((uint16_t)(0x802D + row)));
    WR8(ZP->script_ptr_hi, rd8((uint16_t)(0x802E + row)));
    WR8(ZP->wait_timer3, 0xFF);
    WR8(ZP->wait_timer4, 0xFF);
    exit.a = 0xFF;
    exit.x = 0;
    exit.y = row;
    exit.c = (character & 0x80) != 0;
    exit.nz = INC8(ZP->game_sub_mode);
    return exit;
}

void abi_Scene0ABegin(void)
{
    exit_shim(Scene0ABegin(cpu.x, cpu.y));
}

/*
 * @routine 23F6:234 Scene0AOrbit
 * @run 6B:8234
 * WHAT IT DOES: Shows an object, swings it on a growing circle from ($CE, $B1), then after pauses flies it off.
 * HOW: States 0 to 3 on SetVelocityFromAngle and ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0AOrbit(uint8_t object)
{
    uint8_t state, value, angle, base;
    unsigned total;
    AxisMove move;
    ObjExit result;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->sprite_set[object], 0x03);
        WR8(OBJ->frame[object], 0x0D);
        WR8(OBJ->work_d[object], 0x60);
        return scene_kept(0x60, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        WR8(ZP->temp01, 0x5F);
        WR8(ZP->temp00, 0x00);
        INC8(OBJ->work_d[object]);
        angle = RD8(OBJ->work_d[object]);
        jsr_push(0x825E);
        (void)SetVelocityFromAngle(angle, object);
        jsr_pop();
        base = RD8(OBJ->vx_hi[object]);
        WR8(OBJ->x_lo[object], (uint8_t)(0xCE + base));
        base = RD8(OBJ->vy_hi[object]);
        total = 0xB1u + base;
        WR8(OBJ->y_lo[object], (uint8_t)total);
        value = RD8(OBJ->work_d[object]);
        result = scene_kept(value, value);
        result.exit.c = total > 0xFF;
        result.exit.c_kept = false;
        result.exit.v = ((0xB1 ^ total) & (base ^ total) & 0x80) != 0;
        result.exit.v_set = true;
        if (!(value & 0x80)) {
            return result;
        }
        WR8(OBJ->work_d[object], 0x08);
        result.exit.a = 0x08;
        result.exit.nz = INC8(OBJ->state[object]);
        return result;
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        value = DEC8(OBJ->work_d[object]);
        if (value != 0) {
            return scene_kept(0, value);
        }
        WR8(OBJ->work_d[object], 0x08);
        WR8(OBJ->vx_lo[object], 0);
        WR8(OBJ->vy_lo[object], 0);
        WR8(OBJ->vx_hi[object], 0xFC);
        WR8(OBJ->vy_hi[object], 0x02);
        return scene_kept(0x02, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return scene_kept(state, state);
    }
    jsr_push(0x82A5);
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

void abi_Scene0AOrbit(void)
{
    scene_obj_shim(Scene0AOrbit(cpu.x));
}

/*
 * @routine 23F6:2AE Scene0ASwoop
 * @run 6B:82AE
 * WHAT IT DOES: Shows an object and swoops it on a circle from ($90, $FF) one way, then, after a pause, back the other, until its angle count is $98.
 * HOW: States 0 to 3 on SetVelocityFromAngle; the Y place sixteen bits signed.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0ASwoop(uint8_t object)
{
    uint8_t state, value;
    ObjExit result;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->sprite_set[object], 0x03);
        WR8(OBJ->frame[object], 0x12);
        WR8(OBJ->work_d[object], 0x80);
        return scene_kept(0x80, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        WR8(ZP->temp01, 0x4F);
        WR8(ZP->temp00, 0x00);
        INC8(OBJ->work_d[object]);
        result = swoop_place(object, 0x82D8);
        value = RD8(OBJ->work_d[object]);
        result.exit.a = value;
        result.exit.nz = (uint8_t)(value - 0xB0);
        result.exit.c = value >= 0xB0;
        if (value != 0xB0) {
            return result;
        }
        WR8(OBJ->work_d[object], 0x01);
        result.exit.a = 0x01;
        result.exit.nz = INC8(OBJ->state[object]);
        return result;
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        value = DEC8(OBJ->work_d[object]);
        if (value != 0) {
            return scene_kept(0, value);
        }
        WR8(OBJ->work_d[object], 0xB0);
        return scene_kept(0xB0, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return scene_kept(state, state);
    }
    WR8(ZP->temp01, 0x4F);
    WR8(ZP->temp00, 0x00);
    DEC8(OBJ->work_d[object]);
    DEC8(OBJ->work_d[object]);
    DEC8(OBJ->work_d[object]);
    result = swoop_place(object, 0x8332);
    value = RD8(OBJ->work_d[object]);
    result.exit.a = value;
    result.exit.nz = (uint8_t)(value - 0x98);
    result.exit.c = value >= 0x98;
    if (value != 0x98) {
        return result;
    }
    result.exit.nz = INC8(OBJ->state[object]);
    return result;
}

void abi_Scene0ASwoop(void)
{
    scene_obj_shim(Scene0ASwoop(cpu.x));
}

/*
 * @routine 23F6:376 Scene0AScrollStart
 * @run 6B:8376
 * WHAT IT DOES: Puts the scroll's X low at $80 and goes on.
 * HOW: Two stores.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0AScrollStart(uint8_t object)
{
    WR8(SCROLL->x_lo[0], 0x80);
    return scene_kept(0x80, INC8(OBJ->state[object]));
}

void abi_Scene0AScrollStart(void)
{
    scene_obj_shim(Scene0AScrollStart(cpu.x));
}

/*
 * @routine 23F6:37F Scene0AScrollRight
 * @run 6B:837F
 * WHAT IT DOES: Scrolls X right by a dot and a half a frame; on the carry out of the low byte the low byte goes to zero, the high byte up, and the object goes on.
 * HOW: ADC of $80 to the fraction and 1 to the low byte.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0AScrollRight(uint8_t object)
{
    uint8_t value, low;
    unsigned total;
    bool carry;
    ObjExit result;

    value = RD8(SCROLL->x_frac[0]);
    total = 0x80u + value;
    WR8(SCROLL->x_frac[0], (uint8_t)total);
    low = RD8(SCROLL->x_lo[0]);
    total = 1u + low + (total > 0xFF ? 1 : 0);
    carry = total > 0xFF;
    WR8(SCROLL->x_lo[0], (uint8_t)total);
    result = scene_kept((uint8_t)total, (uint8_t)total);
    result.exit.c = carry;
    result.exit.c_kept = false;
    result.exit.v = ((1 ^ total) & (low ^ total) & 0x80) != 0;
    result.exit.v_set = true;
    if (!carry) {
        return result;
    }
    WR8(SCROLL->x_lo[0], 0);
    INC8(SCROLL->x_hi[0]);
    result.exit.nz = INC8(OBJ->state[object]);
    return result;
}

void abi_Scene0AScrollRight(void)
{
    scene_obj_shim(Scene0AScrollRight(cpu.x));
}

/*
 * @routine 23F6:3A6 Scene0APlayListOnce
 * @run 6B:83A6
 * WHAT IT DOES: Plays the start argument's frame list once.
 * HOW: ScenePlayListOnce with this overlay's lists.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0APlayListOnce(uint8_t object)
{
    return ScenePlayListOnce(0x840F, 0x83B1, object);
}

void abi_Scene0APlayListOnce(void)
{
    scene_obj_shim(Scene0APlayListOnce(cpu.x));
}

/*
 * @routine 23F6:3BC Scene0ASlideForCount
 * @run 6B:83BC
 * WHAT IT DOES: As the $5C overlay's slide, with this overlay's speed tables.
 * HOW: SceneSlide.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0ASlideForCount(uint8_t object)
{
    static const uint16_t speeds[4] = { 0x83EC, 0x83F1, 0x83F6, 0x83FB };

    return SceneSlide(speeds, 0x83E6, object);
}

void abi_Scene0ASlideForCount(void)
{
    scene_obj_shim(Scene0ASlideForCount(cpu.x));
}

/*
 * @routine 23F6:400 Scene0AFrameList
 * @run 6B:8400
 * WHAT IT DOES: Steps an object through the frame list this overlay's table names for A.
 * HOW: ObjectFrameListAt.
 * WAITS: no.
 * Entry: A the list, X the object. Exit: as StepObjectFrames.
 */
ObjExit Scene0AFrameList(uint8_t list, uint8_t object)
{
    return ObjectFrameListAt(list, object, 0x840F);
}

void abi_Scene0AFrameList(void)
{
    scene_obj_shim(Scene0AFrameList(cpu.a, cpu.x));
}

/*
 * @routine 23F6:35E Scene0ASteps
 * @run 6B:835E
 * @stop 8370
 * WHAT IT DOES: Goes to the step its state names.
 * HOW: The state doubled indexes the table; the word through zTemp00 and zTemp01.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0ASteps(uint8_t object)
{
    uint8_t index, high;
    uint16_t target = SceneStateJump(0x8370, object, &index, &high);
    ObjExit result;

    switch (target) {
    case 0x8376: result = Scene0AScrollStart(object); break;
    case 0x837F: result = Scene0AScrollRight(object); break;
    case 0x839B: result = scene_kept(0, 0); result.exit.a_kept = true; break;
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

void abi_Scene0ASteps(void)
{
    scene_obj_shim(Scene0ASteps(cpu.x));
}

/* This overlay's object handlers by their addresses, for the engine's dispatch. */
bool scene_0a_handler(uint16_t target, uint8_t object, ObjExit *out)
{
    switch (target) {
    case 0x83A6: *out = Scene0APlayListOnce(object); return true;
    case 0x83BC: *out = Scene0ASlideForCount(object); return true;
    case 0x835E: *out = Scene0ASteps(object); return true;
    case 0x8234: *out = Scene0AOrbit(object); return true;
    case 0x82AE: *out = Scene0ASwoop(object); return true;
    default: return false;
    }
}
