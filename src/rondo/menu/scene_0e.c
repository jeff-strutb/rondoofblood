/*
 * The cutscene overlay of sector $1E61 (bank $6B at $8000; scenes $0E and $0F): the shared script engine
 * (scene_engine.c) at this overlay's places, $12 past the $5C overlay's, and this overlay's own begin and
 * object handlers.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "scene_engine.h"
#include "scene_0e.h"
#include "scene_0a.h"


static void scene_entry_shim(uint8_t high, uint8_t object, uint8_t index)
{
    cpu.a = high;
    cpu.x = object;
    cpu.y = index;
    abi_nz(high);
}

/*
 * @routine 1E61:236 Scene0ERiseScroll
 * @run 6B:8236
 * WHAT IT DOES: Waits a frame, then scrolls: X low up one and Y (16.8) up half a dot a frame until X low wraps.
 * HOW: INC; ADC of $80 to the Y fraction and 0 to Y low.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0ERiseScroll(uint8_t object)
{
    uint8_t state, value, frac, low;
    unsigned total;
    ObjExit result;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        return scene_kept(0, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return scene_kept(state, state);
    }
    INC8(SCROLL->x_lo[0]);
    frac = RD8(SCROLL->y_frac[0]);
    total = 0x80u + frac;
    WR8(SCROLL->y_frac[0], (uint8_t)total);
    low = RD8(SCROLL->y_lo[0]);
    value = (uint8_t)(low + (total > 0xFF ? 1 : 0));
    WR8(SCROLL->y_lo[0], value);
    result = scene_kept(0, 0);
    result.exit.c = total > 0xFF && value == 0;
    result.exit.c_kept = false;
    result.exit.v = (low == 0x7F) && total > 0xFF;
    result.exit.v_set = true;
    value = (uint8_t)(RD8(SCROLL->x_lo[0]) + 1);
    result.exit.a = value;
    result.exit.nz = value;
    if (value == 0) {
        result.exit.nz = INC8(OBJ->state[object]);
    }
    return result;
}

void abi_Scene0ERiseScroll(void)
{
    scene_obj_shim(Scene0ERiseScroll(cpu.x));
}

/*
 * @routine 1E61:277 Scene0EPieceStart
 * @run 6B:8277
 * WHAT IT DOES: Sets the work count $50 and goes on.
 * HOW: Two stores.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0EPieceStart(uint8_t object)
{
    WR8(OBJ->work_d[object], 0x50);
    return scene_kept(0x50, INC8(OBJ->state[object]));
}

void abi_Scene0EPieceStart(void)
{
    scene_obj_shim(Scene0EPieceStart(cpu.x));
}

/*
 * @routine 1E61:280 Scene0EPieceSwingA
 * @run 6B:8280
 * WHAT IT DOES: Counts down; above the limit the two pieces swing on a sine and move apart.
 * HOW: DEC work_d; ScaleBySine of 2 by the count times 8 plus $C0; Y places; X of object 0 up, object 1 down.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0EPieceSwingA(uint8_t object)
{
    ScaleResult wave;
    uint8_t value, angle, y;
    unsigned total;
    ObjExit result;

    DEC8(OBJ->work_d[object]);
    value = RD8(OBJ->work_d[object]);
    if (value == 0xFF) {                                  /* BEQ back to the INC of the state */
        result = scene_kept(value, INC8(OBJ->state[object]));
        result.exit.c = true;
        result.exit.c_kept = false;
        return result;
    }
    result = scene_kept(value, (uint8_t)(value - 0x10));
    result.exit.c = value >= 0x10;
    result.exit.c_kept = false;
    if (value >= 0x10) {
        WR8(ZP->temp00, 0x02);
        WR8(ZP->temp01, 0x00);
        value = RD8(OBJ->work_d[object]);
        angle = (uint8_t)((uint8_t)(value << 3) + 0xC0);
        jsr_push(0x82A0);
        wave = ScaleBySine(angle);
        jsr_pop();
        (void)wave;
        total = 0x43u + RD8(ZP->product.mid);
        y = (uint8_t)total;
        WR8(OBJ->y_lo[0], y);
        total = (unsigned)y + 0x28;
        y = (uint8_t)total;
        WR8(OBJ->y_lo[1], y);
        result.exit.a = y;
        result.exit.c = total > 0xFF;
        result.exit.y = 0;
        result.exit.y_kept = false;
        result.exit.v = (((uint8_t)(y - 0x28) ^ y) & (0x28 ^ y) & 0x80) != 0;
        result.exit.v_set = true;
        value = INC8(OBJ->x_lo[0]);
        result.exit.nz = value;
        if (value == 0) {
            result.exit.nz = INC8(OBJ->x_hi[0]);
        }
    }
    result.exit.nz = DEC8(OBJ->x_lo[1]);
    return result;
}

void abi_Scene0EPieceSwingA(void)
{
    scene_obj_shim(Scene0EPieceSwingA(cpu.x));
}

/*
 * @routine 1E61:2DB Scene0EPieceSwingB
 * @run 6B:82DB
 * WHAT IT DOES: Counts down; above the limit the two pieces swing on a sine and move apart.
 * HOW: DEC work_d; ScaleBySine of 2 by the count times 8 plus $C0; Y places; X of object 0 up, object 1 down.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0EPieceSwingB(uint8_t object)
{
    ScaleResult wave;
    uint8_t value, angle, y;
    unsigned total;
    ObjExit result;

    DEC8(OBJ->work_d[object]);
    value = RD8(OBJ->work_d[object]);
    if (value == 0xFF) {                                  /* BEQ back to the INC of the state */
        result = scene_kept(value, INC8(OBJ->state[object]));
        result.exit.c = true;
        result.exit.c_kept = false;
        return result;
    }
    result = scene_kept(value, (uint8_t)(value - 0x08));
    result.exit.c = value >= 0x08;
    result.exit.c_kept = false;
    if (value >= 0x08) {
        WR8(ZP->temp00, 0x02);
        WR8(ZP->temp01, 0x00);
        value = RD8(OBJ->work_d[object]);
        angle = (uint8_t)((uint8_t)(value << 3) + 0xC0);
        jsr_push(0x82FB);
        wave = ScaleBySine(angle);
        jsr_pop();
        (void)wave;
        total = 0x53u + RD8(ZP->product.mid);
        y = (uint8_t)total;
        WR8(OBJ->y_lo[0], y);
        WR8(OBJ->y_lo[1], y);
        result.exit.a = y;
        result.exit.c = total > 0xFF;
        result.exit.y = 0;
        result.exit.y_kept = false;
        result.exit.v = ((0x53 ^ y) & ((uint8_t)(y - 0x53) ^ y) & 0x80) != 0;
        result.exit.v_set = true;
        value = INC8(OBJ->x_lo[0]);
        result.exit.nz = value;
        if (value == 0) {
            result.exit.nz = INC8(OBJ->x_hi[0]);
        }
    }
    result.exit.nz = DEC8(OBJ->x_lo[1]);
    return result;
}

void abi_Scene0EPieceSwingB(void)
{
    scene_obj_shim(Scene0EPieceSwingB(cpu.x));
}

/*
 * @routine 1E61:2D2 Scene0EPieceStartB
 * @run 6B:82D2
 * WHAT IT DOES: Sets the work count $60 and goes on.
 * HOW: Two stores.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0EPieceStartB(uint8_t object)
{
    WR8(OBJ->work_d[object], 0x60);
    return scene_kept(0x60, INC8(OBJ->state[object]));
}

void abi_Scene0EPieceStartB(void)
{
    scene_obj_shim(Scene0EPieceStartB(cpu.x));
}

/*
 * @routine 1E61:312 Scene0EBandsA
 * @run 6B:8312
 * WHAT IT DOES: Starts raster effect $13, then after a count pulls two raster bands apart by half a dot a frame until the lower reaches $BF.
 * HOW: State 0 sets up; state 1 counts work_d and work_e; state 2 the 16.8 band scrolls.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0EBandsA(uint8_t object)
{
    uint8_t state, value, hi;
    unsigned total;
    ObjExit result;
    bool carry;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(ZP->raster_effect, 0x13);
        INC8(SCROLL->y_hi[0]);
        WR8(SCROLL->y_lo[5], 0x47);
        WR8(SCROLL->y_lo[6], 0x97);
        INC8(SCROLL->x_hi[2]);
        WR8(OBJ->work_e[object], 0);
        WR8(OBJ->work_d[object], 0xFF);
        return scene_kept(0xFF, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        value = DEC8(OBJ->work_d[object]);
        if (value != 0) {
            return scene_kept(0, value);
        }
        value = DEC8(OBJ->work_e[object]);
        if (!(value & 0x80)) {
            return scene_kept(0, value);
        }
        return scene_kept(0, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return scene_kept(state, state);
    }
    value = RD8(SCROLL->y_lo[5]);
    carry = value >= 0x2F;
    if (value != 0x2F) {
        value = RD8(SCROLL->y_frac[5]);
        carry = value >= 0x80;
        WR8(SCROLL->y_frac[5], (uint8_t)(value - 0x80));
        value = RD8(SCROLL->y_lo[5]);
        hi = (uint8_t)(value - (carry ? 0 : 1));
        WR8(SCROLL->y_lo[5], hi);
    }
    value = RD8(SCROLL->y_frac[6]);
    total = (unsigned)value + 0x80;
    WR8(SCROLL->y_frac[6], (uint8_t)total);
    value = RD8(SCROLL->y_lo[6]);
    hi = (uint8_t)(value + (total > 0xFF ? 1 : 0));
    WR8(SCROLL->y_lo[6], hi);
    result = scene_kept(hi, (uint8_t)(hi - 0xBF));
    result.exit.c = hi >= 0xBF;
    result.exit.c_kept = false;
    result.exit.v = (value == 0x7F) && total > 0xFF;
    result.exit.v_set = true;
    if (hi == 0xBF) {
        result.exit.nz = INC8(OBJ->state[object]);
    }
    return result;
}

void abi_Scene0EBandsA(void)
{
    scene_obj_shim(Scene0EBandsA(cpu.x));
}

/*
 * @routine 1E61:37E Scene0EBandsB
 * @run 6B:837E
 * WHAT IT DOES: As Scene0EBandsA with other starts and ends ($27, $C7) and a count of $170.
 * HOW: As Scene0EBandsA.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0EBandsB(uint8_t object)
{
    uint8_t state, value, hi;
    unsigned total;
    ObjExit result;
    bool carry;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(ZP->raster_effect, 0x13);
        INC8(SCROLL->y_hi[0]);
        WR8(SCROLL->y_lo[5], 0x3F);
        WR8(SCROLL->y_lo[6], 0x8F);
        INC8(SCROLL->x_hi[2]);
        WR8(OBJ->work_e[object], 1);
        WR8(OBJ->work_d[object], 0x70);
        return scene_kept(0x70, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        value = DEC8(OBJ->work_d[object]);
        if (value != 0) {
            return scene_kept(0, value);
        }
        value = DEC8(OBJ->work_e[object]);
        if (!(value & 0x80)) {
            return scene_kept(0, value);
        }
        return scene_kept(0, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return scene_kept(state, state);
    }
    value = RD8(SCROLL->y_lo[5]);
    carry = value >= 0x27;
    if (value != 0x27) {
        value = RD8(SCROLL->y_frac[5]);
        carry = value >= 0x80;
        WR8(SCROLL->y_frac[5], (uint8_t)(value - 0x80));
        value = RD8(SCROLL->y_lo[5]);
        hi = (uint8_t)(value - (carry ? 0 : 1));
        WR8(SCROLL->y_lo[5], hi);
    }
    value = RD8(SCROLL->y_frac[6]);
    total = (unsigned)value + 0x80;
    WR8(SCROLL->y_frac[6], (uint8_t)total);
    value = RD8(SCROLL->y_lo[6]);
    hi = (uint8_t)(value + (total > 0xFF ? 1 : 0));
    WR8(SCROLL->y_lo[6], hi);
    result = scene_kept(hi, (uint8_t)(hi - 0xC7));
    result.exit.c = hi >= 0xC7;
    result.exit.c_kept = false;
    result.exit.v = (value == 0x7F) && total > 0xFF;
    result.exit.v_set = true;
    if (hi == 0xC7) {
        result.exit.nz = INC8(OBJ->state[object]);
    }
    return result;
}

void abi_Scene0EBandsB(void)
{
    scene_obj_shim(Scene0EBandsB(cpu.x));
}

/*
 * @routine 1E61:3F4 Scene0EPlayListOnce
 * @run 6B:83F4
 * WHAT IT DOES: Plays the start argument's frame list once.
 * HOW: ScenePlayListOnce with this overlay's lists.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0EPlayListOnce(uint8_t object)
{
    return ScenePlayListOnce(0x845D, 0x83FF, object);
}

void abi_Scene0EPlayListOnce(void)
{
    scene_obj_shim(Scene0EPlayListOnce(cpu.x));
}

/*
 * @routine 1E61:40A Scene0ESlideForCount
 * @run 6B:840A
 * WHAT IT DOES: As the $5C overlay's slide, with this overlay's speed tables.
 * HOW: SceneSlide.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0ESlideForCount(uint8_t object)
{
    static const uint16_t speeds[4] = { 0x843A, 0x843F, 0x8444, 0x8449 };

    return SceneSlide(speeds, 0x8434, object);
}

void abi_Scene0ESlideForCount(void)
{
    scene_obj_shim(Scene0ESlideForCount(cpu.x));
}

/*
 * @routine 1E61:44E Scene0EFrameList
 * @run 6B:844E
 * WHAT IT DOES: Steps an object through the frame list this overlay's table names for A.
 * HOW: ObjectFrameListAt.
 * WAITS: no.
 * Entry: A the list, X the object. Exit: as StepObjectFrames.
 */
ObjExit Scene0EFrameList(uint8_t list, uint8_t object)
{
    return ObjectFrameListAt(list, object, 0x845D);
}

void abi_Scene0EFrameList(void)
{
    scene_obj_shim(Scene0EFrameList(cpu.a, cpu.x));
}

/*
 * @routine 1E61:25F Scene0EStepsA
 * @run 6B:825F
 * @stop 8271
 * WHAT IT DOES: Goes to the step its state names.
 * HOW: The state doubled indexes the table; the word through zTemp00 and zTemp01.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0EStepsA(uint8_t object)
{
    uint8_t index, high;
    uint16_t target = SceneStateJump(0x8271, object, &index, &high);
    ObjExit result;

    switch (target) {
    case 0x8277: result = Scene0EPieceStart(object); break;
    case 0x8280: result = Scene0EPieceSwingA(object); break;
    case 0x82B9: result = scene_kept(0, 0); result.exit.a_kept = true; break;
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

void abi_Scene0EStepsA(void)
{
    scene_obj_shim(Scene0EStepsA(cpu.x));
}

/*
 * @routine 1E61:2BA Scene0EStepsB
 * @run 6B:82BA
 * @stop 82CC
 * WHAT IT DOES: Goes to the step its state names.
 * HOW: The state doubled indexes the table; the word through zTemp00 and zTemp01.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test or store left them.
 */
ObjExit Scene0EStepsB(uint8_t object)
{
    uint8_t index, high;
    uint16_t target = SceneStateJump(0x82CC, object, &index, &high);
    ObjExit result;

    switch (target) {
    case 0x82D2: result = Scene0EPieceStartB(object); break;
    case 0x82DB: result = Scene0EPieceSwingB(object); break;
    case 0x8311: result = scene_kept(0, 0); result.exit.a_kept = true; break;
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

void abi_Scene0EStepsB(void)
{
    scene_obj_shim(Scene0EStepsB(cpu.x));
}

/* This overlay's object handlers by their addresses, for the engine's dispatch. */
static bool scene_0e_handler(uint16_t target, uint8_t object, ObjExit *out)
{
    /* The $0A overlay's engine bytes are this one's: whichever is loaded gives its handlers. */
    if (resident_at(0x800B) == DISC(0x23F6, 0x00B)) {
        return scene_0a_handler(target, object, out);
    }
    switch (target) {
    case 0x83F4: *out = Scene0EPlayListOnce(object); return true;
    case 0x840A: *out = Scene0ESlideForCount(object); return true;
    case 0x837E: *out = Scene0EBandsB(object); return true;
    case 0x8312: *out = Scene0EBandsA(object); return true;
    case 0x82BA: *out = Scene0EStepsB(object); return true;
    case 0x8236: *out = Scene0ERiseScroll(object); return true;
    case 0x825F: *out = Scene0EStepsA(object); return true;
    default: return false;
    }
}

static const SceneOverlay SCENE_0E = { 18, scene_0e_handler };

/*
 * @routine 1E61:031 Scene0EFrame
 * @run 6B:8031
 * WHAT IT DOES: As Scene5CFrame (25D3:01F), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CFrame.
 * Entry, exit: as Scene5CFrame.
 */
Exit Scene0EFrame(void)
{
    return SceneFrame(&SCENE_0E);
}

void abi_Scene0EFrame(void)
{
    exit_shim(Scene0EFrame());
}

/*
 * @routine 1E61:062 Scene0EScriptStep
 * @run 6B:8062
 * WHAT IT DOES: As Scene5CScriptStep (25D3:050), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CScriptStep.
 * Entry, exit: as Scene5CScriptStep.
 */
Exit Scene0EScriptStep(void)
{
    return SceneScriptStep(&SCENE_0E);
}

void abi_Scene0EScriptStep(void)
{
    exit_shim(Scene0EScriptStep());
}

/*
 * @routine 1E61:0B7 Scene0ECommandStartListAndClear
 * @run 6B:80B7
 * WHAT IT DOES: As Scene5CCommandStartListAndClear (25D3:0A5), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CCommandStartListAndClear.
 * Entry, exit: as Scene5CCommandStartListAndClear.
 */
Exit Scene0ECommandStartListAndClear(void)
{
    return SceneCommandStartListAndClear(&SCENE_0E);
}

void abi_Scene0ECommandStartListAndClear(void)
{
    exit_shim(Scene0ECommandStartListAndClear());
}

/*
 * @routine 1E61:0D2 Scene0ECommandStartScript1
 * @run 6B:80D2
 * WHAT IT DOES: As Scene5CCommandStartScript1 (25D3:0C0), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CCommandStartScript1.
 * Entry, exit: as Scene5CCommandStartScript1.
 */
Exit Scene0ECommandStartScript1(void)
{
    return SceneCommandStartScript1(&SCENE_0E);
}

void abi_Scene0ECommandStartScript1(void)
{
    exit_shim(Scene0ECommandStartScript1());
}

/*
 * @routine 1E61:0DA Scene0ECommandStartScript2
 * @run 6B:80DA
 * WHAT IT DOES: As Scene5CCommandStartScript2 (25D3:0C8), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CCommandStartScript2.
 * Entry, exit: as Scene5CCommandStartScript2.
 */
Exit Scene0ECommandStartScript2(void)
{
    return SceneCommandStartScript2(&SCENE_0E);
}

void abi_Scene0ECommandStartScript2(void)
{
    exit_shim(Scene0ECommandStartScript2());
}

/*
 * @routine 1E61:0E2 Scene0ECommandStartScript3
 * @run 6B:80E2
 * WHAT IT DOES: As Scene5CCommandStartScript3 (25D3:0D0), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CCommandStartScript3.
 * Entry, exit: as Scene5CCommandStartScript3.
 */
Exit Scene0ECommandStartScript3(void)
{
    return SceneCommandStartScript3(&SCENE_0E);
}

void abi_Scene0ECommandStartScript3(void)
{
    exit_shim(Scene0ECommandStartScript3());
}

/*
 * @routine 1E61:0EA Scene0ECommandPlayAdpcm
 * @run 6B:80EA
 * WHAT IT DOES: As Scene5CCommandPlayAdpcm (25D3:0D8), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CCommandPlayAdpcm.
 * Entry, exit: as Scene5CCommandPlayAdpcm.
 */
Exit Scene0ECommandPlayAdpcm(void)
{
    return SceneCommandPlayAdpcm(&SCENE_0E);
}

void abi_Scene0ECommandPlayAdpcm(void)
{
    exit_shim(Scene0ECommandPlayAdpcm());
}

/*
 * @routine 1E61:0F2 Scene0ECommandCdPlay
 * @run 6B:80F2
 * WHAT IT DOES: As Scene5CCommandCdPlay (25D3:0E0), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CCommandCdPlay.
 * Entry, exit: as Scene5CCommandCdPlay.
 */
Exit Scene0ECommandCdPlay(uint8_t index)
{
    return SceneCommandCdPlay(&SCENE_0E, index);
}

void abi_Scene0ECommandCdPlay(void)
{
    exit_shim(Scene0ECommandCdPlay(cpu.x));
}

/*
 * @routine 1E61:0FA Scene0ECommandPlaceKind
 * @run 6B:80FA
 * WHAT IT DOES: As Scene5CCommandPlaceKind (25D3:0E8), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CCommandPlaceKind.
 * Entry, exit: as Scene5CCommandPlaceKind.
 */
Exit Scene0ECommandPlaceKind(void)
{
    return SceneCommandPlaceKind(&SCENE_0E);
}

void abi_Scene0ECommandPlaceKind(void)
{
    exit_shim(Scene0ECommandPlaceKind());
}

/*
 * @routine 1E61:109 Scene0ECommandPlaceNext
 * @run 6B:8109
 * WHAT IT DOES: As Scene5CCommandPlaceNext (25D3:0F7), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CCommandPlaceNext.
 * Entry, exit: as Scene5CCommandPlaceNext.
 */
Exit Scene0ECommandPlaceNext(void)
{
    return SceneCommandPlaceNext(&SCENE_0E);
}

void abi_Scene0ECommandPlaceNext(void)
{
    exit_shim(Scene0ECommandPlaceNext());
}

/*
 * @routine 1E61:123 Scene0ECommandPlaceFrame
 * @run 6B:8123
 * WHAT IT DOES: As Scene5CCommandPlaceFrame (25D3:111), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CCommandPlaceFrame.
 * Entry, exit: as Scene5CCommandPlaceFrame.
 */
Exit Scene0ECommandPlaceFrame(void)
{
    return SceneCommandPlaceFrame(&SCENE_0E);
}

void abi_Scene0ECommandPlaceFrame(void)
{
    exit_shim(Scene0ECommandPlaceFrame());
}

/*
 * @routine 1E61:138 Scene0ECommandPlaceTimed
 * @run 6B:8138
 * WHAT IT DOES: As Scene5CCommandPlaceTimed (25D3:126), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CCommandPlaceTimed.
 * Entry, exit: as Scene5CCommandPlaceTimed.
 */
Exit Scene0ECommandPlaceTimed(void)
{
    return SceneCommandPlaceTimed(&SCENE_0E);
}

void abi_Scene0ECommandPlaceTimed(void)
{
    exit_shim(Scene0ECommandPlaceTimed());
}

/*
 * @routine 1E61:184 Scene0ECommandPlaceTitleObjectA
 * @run 6B:8184
 * WHAT IT DOES: As Scene5CCommandPlaceTitleObjectA (25D3:172), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CCommandPlaceTitleObjectA.
 * Entry, exit: as Scene5CCommandPlaceTitleObjectA.
 */
Exit Scene0ECommandPlaceTitleObjectA(void)
{
    return SceneCommandPlaceTitleObjectA(&SCENE_0E);
}

void abi_Scene0ECommandPlaceTitleObjectA(void)
{
    exit_shim(Scene0ECommandPlaceTitleObjectA());
}

/*
 * @routine 1E61:1AB Scene0ECommandPlaceTitleObjectB
 * @run 6B:81AB
 * WHAT IT DOES: As Scene5CCommandPlaceTitleObjectB (25D3:199), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CCommandPlaceTitleObjectB.
 * Entry, exit: as Scene5CCommandPlaceTitleObjectB.
 */
Exit Scene0ECommandPlaceTitleObjectB(void)
{
    return SceneCommandPlaceTitleObjectB(&SCENE_0E);
}

void abi_Scene0ECommandPlaceTitleObjectB(void)
{
    exit_shim(Scene0ECommandPlaceTitleObjectB());
}

/*
 * @routine 1E61:1D2 Scene0ECommandLoadBanks
 * @run 6B:81D2
 * WHAT IT DOES: As Scene5CCommandLoadBanks (25D3:1C0), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CCommandLoadBanks.
 * Entry, exit: as Scene5CCommandLoadBanks.
 */
Exit Scene0ECommandLoadBanks(void)
{
    return SceneCommandLoadBanks(&SCENE_0E);
}

void abi_Scene0ECommandLoadBanks(void)
{
    exit_shim(Scene0ECommandLoadBanks());
}

/*
 * @routine 1E61:207 Scene0EObjectsStep
 * @run 6B:8207
 * WHAT IT DOES: As Scene5CObjectsStep (25D3:1F5), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CObjectsStep.
 * Entry, exit: as Scene5CObjectsStep.
 */
Exit Scene0EObjectsStep(void)
{
    return SceneObjectsStep(&SCENE_0E);
}

void abi_Scene0EObjectsStep(void)
{
    exit_shim(Scene0EObjectsStep());
}

/*
 * @routine 1E61:216 Scene0EDispatchKind
 * @run 6B:8216
 * WHAT IT DOES: As Scene5CDispatchKind (25D3:204), at this overlay's place in the shared engine.
 * HOW: The shared body of scene_engine.c with this overlay's shift ($12).
 * WAITS: as Scene5CDispatchKind.
 * Entry, exit: as Scene5CDispatchKind.
 */
ObjExit Scene0EDispatchKind(uint8_t kind, uint8_t object)
{
    return SceneDispatchKind(&SCENE_0E, kind, object);
}

void abi_Scene0EDispatchKind(void)
{
    scene_obj_shim(Scene0EDispatchKind(cpu.a, cpu.x));
}
