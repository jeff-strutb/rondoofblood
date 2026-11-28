/*
 * Richter's pad and animation helpers, from his overlay at $A000: what the pad holds and
 * what was newly pressed, which way he faces, and stepping through an animation's frames.
 *
 * A body returns plain values; its wrapper works out what the assembly leaves in the registers
 * and flags.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"
#include "access.h"
#include "enemy_objects.h"
#include "player_objects.h"
#include "richter.h"

enum { PI_FP_NONE, PI_FP_RIGHT, PI_FP_LEFT };
enum { PI_NP_NONE, PI_NP_NOTED };
enum { PI_CH_SHORT, PI_CH_READY };
enum {
    PAD_LEFT_OR_RIGHT = 0xA0, PAD_RIGHT_ONLY = 0x20, PAD_TURBO_BIT = 0x01, PAD_ORDINARY_BITS = 0xFE,
    PAD_SELECT_BIT = 0x04, ANIMATION_STOPPED = 0xFF, ANIMATION_LOOPS = 0xFE,
    ITEM_READY = 0x0F, ITEM_REQUEST = 0x04, ITEM_FIRST_SLOT = 3, ITEM_SLOT_END = 0x10, MARIA_ITEM_SLOT_END = 0x0F,
    MARIA_ITEM_TABLES = 0x4FD,
    ITEM_CRASH_CHANNEL = 6, ITEM_CRASH_SLOT = 6, ITEM_USE_PRESS = 0x02, ITEM_USE_HELD = 0x10,
    ITEM_THROWN_FIRST = 2, ITEM_THROWN_LAST = 5, ITEM_SIX = 6, ITEM_SEVEN = 7, ITEM_EIGHT = 8,
    ITEM_SLOT_A = 5, ITEM_SLOT_B = 8, ITEM_SLOT_C = 11, ITEM_SIX_LIST = 0x54, ITEM_SIX_SCRIPT = 7,
    ITEM_EIGHT_LIST = 0x53, ITEM_EIGHT_SCRIPT = 0x15
};

static uint8_t bcd_subtract(uint8_t hearts, uint8_t cost);
static ItemUse item_not_used(ItemUse use, ItemUseHow how);

/*
 * @routine 01BB:611 RichterReadPad
 * @run 6D:B611
 * WHAT IT DOES: Reads the pad for the frame: what is held, and what is newly pressed. Bit 0
 * counts as pressed only when held for three frames after being up, which steadies it.
 * HOW: The last four frames of the pad are kept in PadHistory.
 * WAITS: no.
 * Entry: nothing. Exit: A the new presses, N and Z from them. X, Y, C and V kept. Clobbers
 * zTemp00.
 */
uint8_t RichterReadPad(void)
{
    uint8_t held, previous, pressed, history, steady;

    previous = RD8(ZP->pad_held);
    WR8(ZP->pad_previous, previous);
    history = rd8(PadHistory + 2);
    wr8(PadHistory + 3, history);
    history = rd8(PadHistory + 1);
    wr8(PadHistory + 2, history);
    history = rd8(PadHistory);
    wr8(PadHistory + 1, history);
    held = rd8(BiosPadState);
    wr8(PadHistory, held);
    WR8(ZP->pad_held, held);
    steady = (uint8_t)(held ^ rd8(PadHistory + 3));
    steady = (uint8_t)(steady & rd8(PadHistory + 2));
    steady = (uint8_t)(steady & rd8(PadHistory + 1));
    steady = (uint8_t)(steady & rd8(PadHistory));
    steady = (uint8_t)(steady & PAD_TURBO_BIT);
    WR8(ZP->temp00, steady);
    pressed = RD8(ZP->pad_held);
    pressed = (uint8_t)(pressed ^ RD8(ZP->pad_previous));
    pressed = (uint8_t)(pressed & RD8(ZP->pad_held));
    pressed = (uint8_t)(pressed & PAD_ORDINARY_BITS);
    pressed = (uint8_t)(pressed | RD8(ZP->temp00));
    WR8(ZP->pad_pressed, pressed);
    return pressed;
}

void abi_RichterReadPad(void)
{
    cpu.a = RichterReadPad();
    abi_nz(cpu.a);
}

/*
 * @routine 01BB:64C RichterClearPadOverrides
 * @run 6D:B64C
 * WHAT IT DOES: Clears the four bytes that force the pad.
 * HOW: Stores zero.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 */
static void clear_pad_overrides_body(void)
{
    for (uint16_t i = 0; i < 4; i++)
        wr8((uint16_t)(PadOverrides + i), 0);
}

void RichterClearPadOverrides(void)
{
    clear_pad_overrides_body();
}

void abi_RichterClearPadOverrides(void)
{
    RichterClearPadOverrides();
}

/*
 * @routine 01E0:173 MariaClearPadOverrides
 * @run 6D:B973
 * WHAT IT DOES: RichterClearPadOverrides in Maria's player overlay (01DD), the same instructions at her address.
 * HOW: RichterClearPadOverrides's body.
 * WAITS: no.
 * Entry, exit: as RichterClearPadOverrides.
 */
void MariaClearPadOverrides(void)
{
    clear_pad_overrides_body();
}

void abi_MariaClearPadOverrides(void)
{
    MariaClearPadOverrides();
}

/* The exit state of an animation routine as the assembly leaves it. */
static AnimationStep animation_exit(uint8_t a, uint8_t nz, bool c, bool y_set, uint8_t y)
{
    AnimationStep r = { FRAME_STOPPED, false, false, 0, 0, true, a, nz, c, y_set, y };

    return r;
}

/*
 * @routine 01BB:69C RichterAnimationPoint
 * @run 6D:B69C
 * WHAT IT DOES: Points zTemp00-01 at the list of the animation in play and gives Y twice the step in play.
 * HOW: OBJ_ANIMATION doubled indexes AnimationLists; the step in OBJ_ANIMATION_AT doubled.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y twice the step, N and Z from it, C the step's bit 7. X and V kept.
 */
static ObjectListStep animation_point(uint16_t lists)
{
    ObjectListStep r;
    uint8_t animation = RD8(OBJ->animation[0]);
    uint8_t doubled = (uint8_t)(animation << 1);
    uint8_t step;

    WR8(ZP->temp00, rd8((uint16_t)(lists + doubled)));
    WR8(ZP->temp01, rd8((uint16_t)(lists + 1 + doubled)));
    step = RD8(OBJ->animation_at[0]);
    r.y = (uint8_t)(step << 1);
    r.carry = (step & 0x80) != 0;
    return r;
}

ObjectListStep RichterAnimationPoint(void)
{
    return animation_point(AnimationLists);
}

static void animation_point_exit_shim(ObjectListStep r)
{
    cpu.a = r.y;
    cpu.y = r.y;
    abi_nz(r.y);
    abi_flag(P_C, r.carry);
}

/*
 * @routine 01E0:1C3 MariaAnimationPoint
 * @run 6D:B9C3
 * WHAT IT DOES: RichterAnimationPoint in Maria's player overlay (01DD).
 * HOW: The same instructions with her table of animation lists at $BF3D, $322 above his.
 * WAITS: no.
 * Entry, exit: as RichterAnimationPoint.
 */
ObjectListStep MariaAnimationPoint(void)
{
    return animation_point(AnimationLists + 0x322);
}

void abi_MariaAnimationPoint(void)
{
    animation_point_exit_shim(MariaAnimationPoint());
}

void abi_RichterAnimationPoint(void)
{
    ObjectListStep r = RichterAnimationPoint();

    cpu.a = r.y;
    cpu.y = r.y;
    abi_nz(r.y);
    abi_flag(P_C, r.carry);
}

/*
 * Where one character's copy of the animation routines lies: the return address each of its calls
 * pushes (RichterAnimationWait's, Running's, the entry read's and the tick's), and its list pointer.
 * Maria's overlay (01DD) holds the same instructions at $B980 to $B9C2.
 */
typedef struct {
    uint16_t wait_ret, running_ret, entry_ret, tick_ret;
    ObjectListStep (*point)(void);
} AnimationLayout;

static const AnimationLayout RICHTER_ANIMATION = { 0xB65B, 0xB663, 0xB683, 0xB695, RichterAnimationPoint };
static const AnimationLayout MARIA_ANIMATION = { 0xB982, 0xB98A, 0xB9AA, 0xB9BC, MariaAnimationPoint };

static uint8_t animation_list_and_step(const AnimationLayout *layout, bool *carry)
{
    ObjectListStep r = layout->point();

    *carry = r.carry;
    return r.y;
}

/* The entry of the step in play: how long, then which frame; a mark of $FE starts the list over. */
static AnimationStep animation_read_entry(const AnimationLayout *layout)
{
    AnimationStep result;
    bool carry;
    uint8_t y, time, frame;

    for (;;) {
        jsr_push(layout->entry_ret);
        y = animation_list_and_step(layout, &carry);
        jsr_pop();
        time = RD8_PTR_Y(ZP->temp00, y);
        WR8(OBJ->hurt_timer[0], time);
        if (time == ANIMATION_STOPPED) {
            result = animation_exit(ANIMATION_STOPPED, 0, true, true, y);
            result.how = FRAME_END_MARK;
            result.step = (uint8_t)(y >> 1);
            return result;
        }
        if (time != ANIMATION_LOOPS) {
            break;
        }
        WR8(OBJ->scroll_band[0], 0);
    }
    y++;
    frame = RD8_PTR_Y(ZP->temp00, y);
    WR8(OBJ->frame[0], frame);
    result = animation_exit(frame, frame, false, true, y);
    result.how = FRAME_SHOWN;
    result.frame = frame;
    result.step = (uint8_t)((y - 1) >> 1);
    return result;
}

/*
 * @routine 01BB:659 RichterAnimationWait
 * @run 6D:B659
 * WHAT IT DOES: Steps the animation and, unless it has ended, returns to its caller's caller, so
 * the caller's own work waits for the end.
 * HOW: RichterAnimationRunning; the assembly drops the return address with two pulls.
 * WAITS: no.
 * Entry: nothing. Exit, ended: as RichterAnimationRunning. Otherwise the caller's return address
 * is gone, A its high byte, N and Z from it; its caller then ends at once.
 */
static AnimationStep animation_running(const AnimationLayout *layout);

static AnimationWaitEnd animation_wait(const AnimationLayout *layout)
{
    AnimationWaitEnd r = { { FRAME_STOPPED }, false, 0 };

    jsr_push(layout->wait_ret);
    r.step = animation_running(layout);
    jsr_pop();
    if (r.step.nz == 0) {
        return r;
    }
    pull8();
    r.high = pull8();
    r.dropped = true;
    r.step.a = r.high;
    r.step.nz = r.high;
    return r;
}

AnimationWaitEnd RichterAnimationWait(void)
{
    return animation_wait(&RICHTER_ANIMATION);
}

void abi_RichterAnimationWait(void)
{
    AnimationWaitEnd r = RichterAnimationWait();

    animation_exit_shim(r.step);
}

/*
 * @routine 01BB:660 RichterAnimationRunning
 * @run 6D:B660
 * WHAT IT DOES: Steps the animation; a caller looks at whether it has ended.
 * HOW: RichterAnimationTick, then the timer against $FF.
 * WAITS: no.
 * Entry: nothing. Exit: A the timer, N, Z and C from comparing it with $FF. Y as the tick. X and V
 * kept. Clobbers zTemp00-01.
 */
static AnimationStep animation_tick(const AnimationLayout *layout);

static AnimationStep animation_running(const AnimationLayout *layout)
{
    AnimationStep tick;
    uint8_t time;

    jsr_push(layout->running_ret);
    tick = animation_tick(layout);
    jsr_pop();
    time = RD8(OBJ->hurt_timer[0]);
    return animation_exit(time, (uint8_t)(time - ANIMATION_STOPPED), time >= ANIMATION_STOPPED, tick.y_set, tick.y);
}

AnimationStep RichterAnimationRunning(void)
{
    return animation_running(&RICHTER_ANIMATION);
}

void animation_running_exit_shim(AnimationStep step)
{
    animation_exit_shim(step);
}

void abi_RichterAnimationRunning(void)
{
    animation_exit_shim(RichterAnimationRunning());
}

/*
 * @routine 01BB:669 RichterAnimationTick
 * @run 6D:B669
 * WHAT IT DOES: Counts down the frame on show; at zero moves to the next step of the list.
 * A timer of $FF means the animation has stopped.
 * HOW: The timer in OBJ_HURT_TIMER, the step in OBJ_ANIMATION_AT, the frame shown in OBJ_FRAME.
 * WAITS: no.
 * Entry: nothing. Exit: A the frame or $FF; Y twice the step, plus one with a frame; C and N, Z
 * from what was read. X and V kept. Clobbers zTemp00-01.
 */
static AnimationStep animation_tick(const AnimationLayout *layout)
{
    AnimationStep result;
    uint8_t time = RD8(OBJ->hurt_timer[0]);
    uint8_t left, y, frame;
    bool carry;

    if (time == ANIMATION_STOPPED) {
        return animation_exit(ANIMATION_STOPPED, 0, true, false, 0);
    }
    left = DEC8(OBJ->hurt_timer[0]);
    if (left == 0) {
        INC8(OBJ->animation_at[0]);
        return animation_read_entry(layout);
    }
    jsr_push(layout->tick_ret);
    y = animation_list_and_step(layout, &carry);
    jsr_pop();
    y++;
    frame = RD8_PTR_Y(ZP->temp00, y);
    WR8(OBJ->frame[0], frame);
    result = animation_exit(frame, frame, carry, true, y);
    result.how = FRAME_SHOWN;
    result.same_frame = true;
    result.frame = frame;
    result.step = (uint8_t)((y - 1) >> 1);
    return result;
}

AnimationStep RichterAnimationTick(void)
{
    return animation_tick(&RICHTER_ANIMATION);
}

void animation_exit_shim(AnimationStep result)
{
    uint8_t doubled = (uint8_t)(result.step << 1);

    if (result.exact) {
        cpu.a = result.a;
        abi_nz(result.nz);
        abi_flag(P_C, result.c);
        if (result.y_set) {
            cpu.y = result.y;
        }
        return;
    }
    switch (result.how) {
    case FRAME_STOPPED:
        cpu.a = ANIMATION_STOPPED;
        abi_nz(0);
        abi_flag(P_C, true);
        break;
    case FRAME_END_MARK:
        cpu.a = ANIMATION_STOPPED;
        cpu.y = doubled;
        abi_nz(0);
        abi_flag(P_C, true);
        break;
    case FRAME_SHOWN:
        cpu.a = result.frame;
        cpu.y = (uint8_t)(doubled + 1);
        abi_nz(result.frame);
        abi_flag(P_C, result.same_frame && (result.step & 0x80) != 0);
        break;
    }
}

void abi_RichterAnimationTick(void)
{
    animation_exit_shim(RichterAnimationTick());
}

/*
 * @routine 01BB:67B RichterSetAnimation
 * @run 6D:B67B
 * WHAT IT DOES: Starts an animation from its first step.
 * HOW: The animation into OBJ_ANIMATION, step zero, and its first entry read.
 * WAITS: no.
 * Entry: A animation. Exit: as RichterAnimationTick. Clobbers zTemp00-01.
 */
static AnimationStep set_animation(const AnimationLayout *layout, uint8_t animation)
{
    WR8(OBJ->animation[0], animation);
    WR8(OBJ->animation_at[0], 0);
    return animation_read_entry(layout);
}

AnimationStep RichterSetAnimation(uint8_t animation)
{
    return set_animation(&RICHTER_ANIMATION, animation);
}

void abi_RichterSetAnimation(void)
{
    animation_exit_shim(RichterSetAnimation(cpu.a));
}

/*
 * @routine 01E0:180 MariaAnimationWait
 * @run 6D:B980
 * WHAT IT DOES: RichterAnimationWait in Maria's player overlay (01DD), the same instructions.
 * HOW: RichterAnimationWait's body with her layout (MariaAnimationRunning at $B987).
 * WAITS: no.
 * Entry, exit: as RichterAnimationWait.
 */
AnimationWaitEnd MariaAnimationWait(void)
{
    return animation_wait(&MARIA_ANIMATION);
}

void abi_MariaAnimationWait(void)
{
    AnimationWaitEnd r = MariaAnimationWait();

    animation_exit_shim(r.step);
}

/*
 * @routine 01E0:187 MariaAnimationRunning
 * @run 6D:B987
 * WHAT IT DOES: RichterAnimationRunning in Maria's player overlay (01DD).
 * HOW: The same instructions with her tick, MariaAnimationTick at $B990.
 * WAITS: no.
 * Entry, exit: as RichterAnimationRunning.
 */
AnimationStep MariaAnimationRunning(void)
{
    return animation_running(&MARIA_ANIMATION);
}

void abi_MariaAnimationRunning(void)
{
    animation_exit_shim(MariaAnimationRunning());
}

/*
 * @routine 01E0:190 MariaAnimationTick
 * @run 6D:B990
 * WHAT IT DOES: RichterAnimationTick in Maria's player overlay (01DD), the same bytes.
 * HOW: RichterAnimationTick's body with her layout: its calls go to MariaAnimationPoint at $B9C3.
 * WAITS: no.
 * Entry, exit: as RichterAnimationTick.
 */
AnimationStep MariaAnimationTick(void)
{
    return animation_tick(&MARIA_ANIMATION);
}

void abi_MariaAnimationTick(void)
{
    animation_exit_shim(MariaAnimationTick());
}

/*
 * @routine 01E0:1A2 MariaSetAnimation
 * @run 6D:B9A2
 * WHAT IT DOES: RichterSetAnimation in Maria's player overlay (01DD), the same bytes.
 * HOW: RichterSetAnimation's body with her layout: its calls go to MariaAnimationPoint at $B9C3.
 * WAITS: no.
 * Entry, exit: as RichterSetAnimation.
 */
AnimationStep MariaSetAnimation(uint8_t animation)
{
    return set_animation(&MARIA_ANIMATION, animation);
}

void abi_MariaSetAnimation(void)
{
    animation_exit_shim(MariaSetAnimation(cpu.a));
}

/*
 * @routine 01BB:704 RichterCheckTurnWindow
 * @run 6D:B704
 * WHAT IT DOES: Says whether the state in play lets him turn at the screen's middle: a table
 * by state, in the first step only, and only on the side of the middle he faces away from.
 * HOW: PlayerDoorReady is the answer.
 * WAITS: no.
 * Entry: nothing. Exit: A the last read, N and Z from it; X the state if it went that far. Y,
 * C and V kept.
 */
static TurnWindow check_turn_window(uint16_t turn_states)
{
    TurnWindow r = { ended_kept(), false, 0 };
    uint8_t value, facing, x_lo;

    wr8(PlayerDoorReady, 0);
    value = RD8(OBJ->substate[0]);
    if (value != 0) {
        r.end = ended_load(value);
        return r;
    }
    r.x_set = true;
    r.x = RD8(OBJ->state[0]);
    value = rd8((uint16_t)(turn_states + r.x));
    wr8(PlayerDoorReady, value);
    r.end = ended_load(value);
    facing = RD8(OBJ->facing_left[0]);
    x_lo = RD8(OBJ->x_lo[0]);
    r.end = ended_load(x_lo);
    if ((facing == 0) != ((x_lo & 0x80) != 0)) {
        wr8(PlayerDoorReady, 0);
    }
    return r;
}

TurnWindow RichterCheckTurnWindow(void)
{
    return check_turn_window(RichterTurnStates);
}

void abi_RichterCheckTurnWindow(void)
{
    TurnWindow r = RichterCheckTurnWindow();

    abi_ended(r.end);
    if (r.x_set) {
        cpu.x = r.x;
    }
}

/*
 * @routine 01E0:22B MariaCheckTurnWindow
 * @run 6D:BA2B
 * WHAT IT DOES: RichterCheckTurnWindow in Maria's player overlay (01DD).
 * HOW: The same instructions with her table of turn states at $BA54, $327 above his.
 * WAITS: no.
 * Entry, exit: as RichterCheckTurnWindow.
 */
TurnWindow MariaCheckTurnWindow(void)
{
    return check_turn_window(RichterTurnStates + 0x327);
}

void abi_MariaCheckTurnWindow(void)
{
    TurnWindow r = MariaCheckTurnWindow();

    abi_ended(r.end);
    if (r.x_set) {
        cpu.x = r.x;
    }
}

/*
 * @routine 01BB:7CA RichterFaceByPad
 * @run 6D:B7CA
 * WHAT IT DOES: Turns him to face the way the pad holds, left or right; with neither, as he is.
 * HOW: Bits 7 and 5 of zPadHeld, and facing in OBJ_FACING_LEFT: 0 for right, 1 for left.
 * WAITS: no.
 * Entry: nothing. Exit: A the bits looked at last; N and Z from them, or from the facing when it
 * turns left. X, Y, C and V kept.
 */
ObjArm RichterFaceByPad(void)
{
    ObjArm r = { 0 };
    uint8_t held = RD8(ZP->pad_held);

    r.first = (uint8_t)(held & PAD_LEFT_OR_RIGHT);
    r.arm = PI_FP_NONE;
    if (r.first == 0) {
        return r;
    }
    WR8(OBJ->facing_left[0], 0);
    r.first = (uint8_t)(r.first & PAD_RIGHT_ONLY);
    r.arm = PI_FP_RIGHT;
    if (r.first != 0) {
        return r;
    }
    r.second = INC8(OBJ->facing_left[0]);
    r.arm = PI_FP_LEFT;
    return r;
}

static Ended richter_face_by_pad_exit_shim(ObjArm r)
{
    if (r.arm == PI_FP_LEFT) {
        return ended_with_a(ended_nz(r.second), 0);
    }
    return ended_load(r.first);
}

void abi_RichterFaceByPad(void)
{
    abi_ended(richter_face_by_pad_exit_shim(RichterFaceByPad()));
}

/*
 * @routine 01BB:7DD RichterNoteItemPress
 * @run 6D:B7DD
 * WHAT IT DOES: Notes that the item button was pressed this frame.
 * HOW: Bit 2 of zPadPressed sets RichterItemRequest to 4.
 * WAITS: no.
 * Entry: nothing. Exit: Z from the test, N and V bits 7 and 6 of the presses; A 4 if pressed.
 * X, Y and C kept.
 */
static ObjArm note_item_press_body(void)
{
    ObjArm r = { 0 };

    r.first = RD8(ZP->pad_pressed);
    r.arm = PI_NP_NONE;
    if ((r.first & PAD_SELECT_BIT) == 0) {
        return r;
    }
    wr8(PadForcedPress, ITEM_REQUEST);
    r.arm = PI_NP_NOTED;
    return r;
}

ObjArm RichterNoteItemPress(void)
{
    return note_item_press_body();
}

EndedTst richter_note_item_press_exit_shim(ObjArm r)
{
    EndedTst t;

    if (r.arm == PI_NP_NONE) {
        return ended_tst(ended_kept(), r.first, PAD_SELECT_BIT);
    }
    t = ended_tst(ended_load(ITEM_REQUEST), r.first, PAD_SELECT_BIT);
    t.v_only = true;
    return t;
}

void abi_RichterNoteItemPress(void)
{
    abi_ended_tst(richter_note_item_press_exit_shim(RichterNoteItemPress()));
}

/*
 * @routine 01E0:306 MariaNoteItemPress
 * @run 6D:BB06
 * WHAT IT DOES: RichterNoteItemPress in Maria's player overlay (01DD), the same instructions at her address.
 * HOW: RichterNoteItemPress's body.
 * WAITS: no.
 * Entry, exit: as RichterNoteItemPress.
 */
ObjArm MariaNoteItemPress(void)
{
    return note_item_press_body();
}

void abi_MariaNoteItemPress(void)
{
    abi_ended_tst(richter_note_item_press_exit_shim(MariaNoteItemPress()));
}

/*
 * @routine 01BB:7E9 RichterCheckItemHearts
 * @run 6D:B7E9
 * WHAT IT DOES: Says whether he has the hearts the item in hand costs.
 * HOW: zHearts against a table by item, in BCD; RichterItemReady is $0F if so, else 0.
 * WAITS: no.
 * Entry: nothing. Exit: X the item; A the hearts, N, Z and C from the compare, or A $0F. Y and
 * V kept.
 */
static ObjArm check_item_hearts(uint16_t costs)
{
    ObjArm r = { 0 };

    wr8(RichterItemReady, 0);
    r.first = RD8(ZP->field93);
    r.second = RD8(ZP->field94);
    r.third = rd8((uint16_t)(costs + r.first));
    r.arm = PI_CH_SHORT;
    if (r.second >= r.third) {
        wr8(RichterItemReady, ITEM_READY);
        r.arm = PI_CH_READY;
    }
    return r;
}

static EndedX richter_check_item_hearts_exit_shim(ObjArm r)
{
    EndedX x;

    x.x = r.first;
    x.end = ended_compare(r.second, r.third);
    if (r.arm == PI_CH_READY) {
        x.end = ended_load(ITEM_READY);
        x.end = ended_with_carry(x.end, true);
    }
    return x;
}

ObjArm RichterCheckItemHearts(void)
{
    return check_item_hearts(RichterItemCosts);
}

/*
 * @routine 01E0:312 MariaCheckItemHearts
 * @run 6D:BB12
 * WHAT IT DOES: RichterCheckItemHearts in Maria's player overlay (01DD).
 * HOW: The same instructions with her table of item costs at $BD4A, $4FD above his.
 * WAITS: no.
 * Entry, exit: as RichterCheckItemHearts.
 */
ObjArm MariaCheckItemHearts(void)
{
    return check_item_hearts(RichterItemCosts + 0x4FD);
}

void abi_MariaCheckItemHearts(void)
{
    EndedX r = richter_check_item_hearts_exit_shim(MariaCheckItemHearts());

    abi_ended(r.end);
    cpu.x = r.x;
}

void abi_RichterCheckItemHearts(void)
{
    EndedX r = richter_check_item_hearts_exit_shim(RichterCheckItemHearts());

    abi_ended(r.end);
    cpu.x = r.x;
}

/* Hearts less a cost, in BCD, as the processor's decimal subtract with no borrow in gives it. */
static uint8_t bcd_subtract(uint8_t hearts, uint8_t cost)
{
    uint8_t low = (uint8_t)((hearts & 0x0F) - (cost & 0x0F));
    uint8_t high = (uint8_t)((hearts >> 4) - (cost >> 4) - ((low >> 4) & 1));
    uint8_t left = (uint8_t)((high << 4) | (low & 0x0F));

    if (low & 0x10)
        left = (uint8_t)(left - 0x06);
    if (high & 0x10)
        left = (uint8_t)(left - 0x60);
    return left;
}

/*
 * @routine 01BB:7FC RichterTryItemCrash
 * @run 6D:B7FC
 * WHAT IT DOES: The item crash: on a press of the item button with no item of his out, clears
 * the item slots and, if he has the hearts the crash costs, starts its sound and its script and
 * pays the hearts. Says whether it started; when it does not, the forced press is dropped.
 * HOW: Bit 2 of zPadPressed; OBJ_KIND of slots 3 to 15 all empty; RichterItemCosts in BCD, then
 * RichterItemCrashLists and RichterItemCrashScripts by item, 0 for none, on channel and slot 6.
 * WAITS: no.
 * Entry: nothing. Exit: V bit 6 of the presses. Started: C set, A the hearts left, N and Z from
 * them, X the item, Y the script if one started. Not started, C clear and: no press, N bit 7 of
 * the presses, Z set, A, X, Y kept; an item out, A its OBJ_KIND, N and Z from it, X its slot;
 * too few hearts, A the hearts, N and Z from comparing them with the cost, X the item. Y kept.
 */
/* One character's copy of the item crash: the end of its slot scan, its three tables, its calls' returns. */
typedef struct {
    uint8_t slot_end;
    uint16_t costs, lists, scripts;
    uint16_t clear_ret, list_ret, script_ret;
} ItemCrashLayout;

static ItemCrash try_item_crash(const ItemCrashLayout *layout)
{
    ItemCrash crash = { CRASH_NO_PRESS };
    uint8_t list;

    crash.pressed = rd8(zPadPressed);
    if (crash.pressed & PAD_SELECT_BIT) {
        for (crash.slot = ITEM_FIRST_SLOT; crash.slot < layout->slot_end; crash.slot++) {
            crash.out = rd8((uint16_t)(OBJ_KIND + crash.slot));
            if (crash.out != 0) {
                crash.how = CRASH_ITEM_OUT;
                break;
            }
        }
    }
    if (crash.pressed & PAD_SELECT_BIT && crash.how != CRASH_ITEM_OUT) {
        for (uint8_t slot = (uint8_t)(layout->slot_end - 1); slot >= ITEM_FIRST_SLOT; slot--) {
            jsr_push(layout->clear_ret);
            ClearObject(slot);
            jsr_pop();
        }
        crash.item = rd8(zField93);
        crash.hearts = rd8(zField94);
        crash.cost = rd8((uint16_t)(layout->costs + crash.item));
        crash.how = crash.hearts >= crash.cost ? CRASH_STARTED : CRASH_TOO_FEW_HEARTS;
    }
    if (crash.how != CRASH_STARTED) {
        wr8(PadForcedPress, 0);
        return crash;
    }
    list = rd8((uint16_t)(layout->lists + crash.item));
    if (list != 0) {
        jsr_push(layout->list_ret);
        StartListChannelKind2(list, ITEM_CRASH_CHANNEL);
        jsr_pop();
    }
    wr8(PlayerItemFirstUse, 0);
    crash.script = rd8((uint16_t)(layout->scripts + crash.item));
    if (crash.script != 0) {
        jsr_push(layout->script_ret);
        StartScriptSlotKind1(crash.script, ITEM_CRASH_SLOT);
        jsr_pop();
    }
    crash.hearts = bcd_subtract(rd8(zField94), crash.cost);
    wr8(zField94, crash.hearts);
    return crash;
}

ItemCrash RichterTryItemCrash(void)
{
    static const ItemCrashLayout richter = {
        ITEM_SLOT_END, RichterItemCosts, RichterItemCrashLists, RichterItemCrashScripts, 0xB811, 0xB830, 0xB83F
    };

    return try_item_crash(&richter);
}

void item_crash_exit_shim(ItemCrash crash)
{
    abi_flag(P_V, (crash.pressed & 0x40) != 0);
    abi_flag(P_C, crash.how == CRASH_STARTED);
    switch (crash.how) {
    case CRASH_NO_PRESS:
        abi_flag(P_N, (crash.pressed & 0x80) != 0);
        abi_flag(P_Z, true);
        break;
    case CRASH_ITEM_OUT:
        cpu.a = crash.out;
        cpu.x = crash.slot;
        abi_nz(crash.out);
        break;
    case CRASH_TOO_FEW_HEARTS:
        cpu.a = crash.hearts;
        cpu.x = crash.item;
        abi_nz((uint8_t)(crash.hearts - crash.cost));
        break;
    case CRASH_STARTED:
        if (crash.script != 0)
            cpu.y = crash.script;
        cpu.a = crash.hearts;
        cpu.x = crash.item;
        abi_nz(crash.hearts);
        break;
    }
}

void abi_RichterTryItemCrash(void)
{
    item_crash_exit_shim(RichterTryItemCrash());
}

/*
 * @routine 01E0:4F9 MariaTryItemCrash
 * @run 6D:BCF9
 * WHAT IT DOES: RichterTryItemCrash in Maria's player overlay (01DD): her item slots are 3 to 14, not 3 to 15.
 * HOW: RichterTryItemCrash's body with her end of the scan and her tables of costs, lists and scripts at $BD4A,
 * $BD53 and $BD5C, $4FD above his.
 * Untraced: everything after a press of the item button ($BCFE to $BD1B, $BD21 to $BD49); no script pressed it.
 * WAITS: no.
 * Entry, exit: as RichterTryItemCrash.
 */
ItemCrash MariaTryItemCrash(void)
{
    static const ItemCrashLayout maria = {
        MARIA_ITEM_SLOT_END, RichterItemCosts + MARIA_ITEM_TABLES, RichterItemCrashLists + MARIA_ITEM_TABLES,
        RichterItemCrashScripts + MARIA_ITEM_TABLES, 0xBD0E, 0xBD2D, 0xBD3C
    };

    return try_item_crash(&maria);
}

void abi_MariaTryItemCrash(void)
{
    item_crash_exit_shim(MariaTryItemCrash());
}

/*
 * @routine 01BC:129 RichterUseItem
 * @run 6D:B929
 * WHAT IT DOES: Using the item in hand: on the use press with the use direction held and the
 * hearts the item costs, goes to the item's own handler, which throws it or not. Says whether
 * to attack, and with what in PlayerAttackKind: 0 the whip, or the item's attack.
 * HOW: Bit 1 of zPadPressed, bit 4 of zPadHeld, RichterItemUseCosts by item, then the jump
 * through RichterItemUseHandlers: items 0 and 1 attack with the whip. zField93 is 0 to 8; the
 * C takes anything past 8 as the whip.
 * WAITS: no.
 * Entry: nothing. Exit: no press, C clear and all else kept. Otherwise C set, V and Y kept
 * except as a handler leaves Y, and: not held, A 0, N and Z from it; too few hearts, A the
 * hearts, X the item, N and Z from comparing them; items 0 and 1, A and X twice the item, N and
 * Z from it; as the handler otherwise.
 */
ItemUse RichterUseItem(void)
{
    ItemUse use = { USE_NO_PRESS };

    if (!(rd8(zPadPressed) & ITEM_USE_PRESS))
        return use;
    if (!(rd8(zPadHeld) & ITEM_USE_HELD))
        return item_not_used(use, USE_NOT_HELD);
    return RichterUseItemPaid();
}

/*
 * @routine 01BC:134 RichterUseItemPaid
 * @run 6D:B934
 * WHAT IT DOES: RichterUseItem from the hearts on: with the hearts the item costs, goes to the item's
 * own handler, which throws it or not; too few, no attack. RichterUseItem runs on into it, and
 * RichterUseItemOrUpPress goes to it for a press of up with the use button held.
 * HOW: RichterItemUseCosts by item, then the jump through RichterItemUseHandlers: items 0 and 1 attack
 * with the whip. zField93 is 0 to 8; the C takes anything past 8 as the whip.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterUseItem past its presses.
 */
ItemUse RichterUseItemPaid(void)
{
    ItemUse use = { USE_NO_PRESS };
    uint16_t entry;

    use.item = rd8(zField93);
    use.hearts = rd8(zField94);
    use.cost = rd8((uint16_t)(RichterItemUseCosts + use.item));
    if (use.hearts < use.cost)
        return item_not_used(use, USE_TOO_FEW_HEARTS);
    /* The jump reads its entry. */
    entry = (uint16_t)(RichterItemUseHandlers + (uint8_t)(rd8(zField93) << 1));
    rd8(entry);
    rd8((uint16_t)(entry + 1));
    switch (use.item) {
    case ITEM_THROWN_FIRST:
    case ITEM_THROWN_FIRST + 1:
    case ITEM_THROWN_FIRST + 2:
    case ITEM_THROWN_LAST:
        return RichterUseThrownItem(use.item);
    case ITEM_SIX:
        return RichterUseItemSix(use.item);
    case ITEM_SEVEN:
        return RichterUseItemSeven(use.item);
    case ITEM_EIGHT:
        return RichterUseItemEight(use.item);
    default:
        use.by_index = true;
        return item_not_used(use, USE_WHIP);
    }
}

/* The exit every path of RichterUseItem and its handlers leaves. */
void item_use_exit_shim(ItemUse use)
{
    uint8_t index = (uint8_t)(use.item << 1);

    abi_flag(P_C, use.how != USE_NO_PRESS);
    switch (use.how) {
    case USE_NO_PRESS:
        break;
    case USE_NOT_HELD:
        cpu.a = 0;
        abi_nz(0);
        break;
    case USE_WHIP:
        cpu.a = index;
        cpu.x = index;
        abi_nz(index);
        break;
    case USE_TOO_FEW_HEARTS:
        cpu.a = use.hearts;
        cpu.x = use.item;
        abi_nz((uint8_t)(use.hearts - use.cost));
        break;
    case USE_ITEM_OUT:
        cpu.a = use.seen;
        cpu.x = index;
        cpu.y = use.slot;
        abi_nz(use.seen);
        break;
    case USE_SLOTS_FULL:
        cpu.a = use.seen;
        cpu.x = use.by_index ? index : use.item;
        abi_nz(use.seen);
        break;
    case USE_STARTED:
        cpu.a = use.hearts;
        cpu.x = use.item;
        if (use.script_started)
            cpu.y = use.script;
        else if (use.scanned)
            cpu.y = ITEM_SLOT_END;
        abi_nz(use.hearts);
        break;
    }
}

void abi_RichterUseItemPaid(void)
{
    item_use_exit_shim(RichterUseItemPaid());
}

/*
 * @routine 01BC:11A RichterUseItemOrUpPress
 * @run 6D:B91A
 * WHAT IT DOES: The item's use as RichterUseItem makes it; or, with no new press of the use button,
 * the use button held and up newly pressed use it all the same (RichterUseItemPaid).
 * HOW: RichterUseItem; BBR of bit 1 of zPadHeld, TST of bit 4 of zPadPressed; the CLC and RTS where
 * neither is RichterUseItem's own ($B92C); a branch to RichterUseItemPaid.
 * WAITS: no.
 * Entry: nothing. Exit: used, as RichterUseItem. Neither, C clear: held, Z set, N and V bits 7 and 6
 * of zPadPressed; the rest as RichterUseItem left them.
 */
UpPressUse RichterUseItemOrUpPress(void)
{
    UpPressUse r = { { USE_NO_PRESS }, false, false, 0 };

    jsr_push(0xB91C);
    r.use = RichterUseItem();
    jsr_pop();
    if (r.use.how != USE_NO_PRESS) {
        return r;
    }
    if (!(rd8(zPadHeld) & ITEM_USE_PRESS)) {
        return r;
    }
    r.tested = true;
    r.pressed = rd8(zPadPressed);
    if (!(r.pressed & ITEM_USE_HELD)) {
        return r;
    }
    r.paid = true;
    r.use = RichterUseItemPaid();
    return r;
}

void up_press_use_exit_shim(UpPressUse r)
{
    if (r.paid || r.use.how != USE_NO_PRESS) {
        item_use_exit_shim(r.use);
        return;
    }
    if (r.tested) {
        abi_flag(P_Z, true);
        abi_flag(P_N, (r.pressed & 0x80) != 0);
        abi_flag(P_V, (r.pressed & 0x40) != 0);
    }
    abi_flag(P_C, false);
}

void abi_RichterUseItemOrUpPress(void)
{
    up_press_use_exit_shim(RichterUseItemOrUpPress());
}

void abi_RichterUseItem(void)
{
    item_use_exit_shim(RichterUseItem());
}

/* Not used: attack with the whip. */
static ItemUse item_not_used(ItemUse use, ItemUseHow how)
{
    use.how = how;
    wr8(PlayerAttackKind, 0);
    return use;
}

/* Used: the item's attack, and its hearts paid. */
static ItemUse item_used(ItemUse use)
{
    use.how = USE_STARTED;
    use.item = RD8(ZP->field93);
    wr8(PlayerAttackKind, rd8((uint16_t)(RichterItemAttacks + use.item)));
    use.hearts = RD8(ZP->field94);
    use.cost = rd8((uint16_t)(RichterItemUseCosts + use.item));
    use.hearts = bcd_subtract(use.hearts, use.cost);
    WR8(ZP->field94, use.hearts);
    return use;
}

/* A handler's start: the item, with X still holding twice it. */
static ItemUse item_handler(uint8_t item)
{
    ItemUse use = { USE_STARTED };

    use.item = item;
    use.by_index = true;
    return use;
}

/* The scan for an item out, which a handler stops on. */
static bool item_out(ItemUse *use, uint16_t ret)
{
    ItemScan scan;

    jsr_push(ret);
    scan = RichterFindItemOut();
    jsr_pop();

    use->scanned = true;
    use->slot = scan.slot;
    use->seen = scan.seen;
    return scan.found;
}

/* A start of the item's sound and script on channel and slot 6, as the first of its throws. */
static void start_item(ItemUse *use, uint8_t list, uint8_t script, uint16_t ret_list, uint16_t ret_script)
{
    jsr_push(ret_list);
    StartListChannelKind2(list, ITEM_CRASH_CHANNEL);
    jsr_pop();
    jsr_push(ret_script);
    StartScriptSlotKind1(script, ITEM_CRASH_SLOT);
    jsr_pop();
    use->script = script;
    use->script_started = true;
}

/*
 * @routine 01BC:15B RichterUseThrownItem
 * @run 6D:B95B
 * WHAT IT DOES: Items 2 to 5, which can be thrown up to three at a time: the first throw of a
 * use starts the item's sound and script, unless one is out already; a second or third throw
 * needs more hearts, and a fourth is refused. Then the item's attack, and its hearts paid.
 * HOW: PlayerItemFirstUse marks the first throw. The items fly in slots 5, 8 and 11; with one out
 * the hearts must reach RichterItemSecondCosts, with two RichterItemThirdCosts.
 * WAITS: no.
 * Entry: X twice the item. Exit: as RichterUseItem.
 */
ItemUse RichterUseThrownItem(uint8_t item)
{
    ItemUse use = item_handler(item);

    if (rd8(PlayerItemFirstUse) != 0) {
        if (item_out(&use, 0xB963))
            return item_not_used(use, USE_ITEM_OUT);
        use.item = RD8(ZP->field93);
        jsr_push(0xB96F);
        StartListChannelKind2(rd8((uint16_t)(RichterItemUseLists + use.item)), ITEM_CRASH_CHANNEL);
        jsr_pop();
        wr8(PlayerItemFirstUse, 0);
        use.item = RD8(ZP->field93);
        use.script = rd8((uint16_t)(RichterItemUseScripts + use.item));
        jsr_push(0xB97C);
        StartScriptSlotKind1(use.script, ITEM_CRASH_SLOT);
        jsr_pop();
        use.script_started = true;
    }
    use.by_index = false;
    use.item = RD8(ZP->field93);
    if (rd8(OBJ_KIND + ITEM_SLOT_A) != 0) {
        use.item = RD8(ZP->field93);
        use.hearts = rd8(zField94);
        use.cost = rd8((uint16_t)(RichterItemSecondCosts + use.item));
        if (use.hearts < use.cost)
            return item_not_used(use, USE_TOO_FEW_HEARTS);
        if (rd8(OBJ_KIND + ITEM_SLOT_B) != 0) {
            use.hearts = rd8(zField94);
            use.cost = rd8((uint16_t)(RichterItemThirdCosts + use.item));
            if (use.hearts < use.cost)
                return item_not_used(use, USE_TOO_FEW_HEARTS);
            use.seen = rd8(OBJ_KIND + ITEM_SLOT_C);
            if (use.seen != 0)
                return item_not_used(use, USE_SLOTS_FULL);
        }
    }
    return item_used(use);
}

void abi_RichterUseThrownItem(void)
{
    item_use_exit_shim(RichterUseThrownItem((uint8_t)(cpu.x >> 1)));
}

/*
 * @routine 01BC:1B1 RichterUseItemSix
 * @run 6D:B9B1
 * WHAT IT DOES: Item 6: only with no item out; the first use starts sound list $54 and script 7.
 * Then the item's attack, and its hearts paid.
 * HOW: RichterFindItemOut; PlayerItemFirstUse marks the first use.
 * WAITS: no.
 * Entry: X twice the item. Exit: as RichterUseItem.
 */
ItemUse RichterUseItemSix(uint8_t item)
{
    ItemUse use = item_handler(item);

    if (item_out(&use, 0xB9B3))
        return item_not_used(use, USE_ITEM_OUT);
    if (rd8(PlayerItemFirstUse) != 0) {
        start_item(&use, ITEM_SIX_LIST, ITEM_SIX_SCRIPT, 0xB9C1, 0xB9C8);
        wr8(PlayerItemFirstUse, 0);
    }
    return item_used(use);
}

void abi_RichterUseItemSix(void)
{
    item_use_exit_shim(RichterUseItemSix((uint8_t)(cpu.x >> 1)));
}

/*
 * @routine 01BC:1CD RichterUseItemSeven
 * @run 6D:B9CD
 * WHAT IT DOES: Item 7: used while one of slots 5, 8 and 11 is free; then the item's attack, and
 * its hearts paid. With all three taken, the whip.
 * HOW: OBJ_KIND of each slot in turn.
 * WAITS: no.
 * Entry: X twice the item. Exit: as RichterUseItem.
 */
ItemUse RichterUseItemSeven(uint8_t item)
{
    ItemUse use = item_handler(item);

    if (rd8(OBJ_KIND + ITEM_SLOT_A) == 0 || rd8(OBJ_KIND + ITEM_SLOT_B) == 0)
        return item_used(use);
    use.seen = rd8(OBJ_KIND + ITEM_SLOT_C);
    if (use.seen == 0)
        return item_used(use);
    return item_not_used(use, USE_SLOTS_FULL);
}

void abi_RichterUseItemSeven(void)
{
    item_use_exit_shim(RichterUseItemSeven((uint8_t)(cpu.x >> 1)));
}

/*
 * @routine 01BC:1DF RichterFindItemOut
 * @run 6D:B9DF
 * WHAT IT DOES: Finds the first of slots 5 to 15 with an object in it.
 * HOW: OBJ_KIND of each.
 * WAITS: no.
 * Entry: nothing. Exit: Y the slot, or $10; A its OBJ_KIND, or 0. Found: N and Z from A, C kept
 * if it was slot 5, else clear. None: Z and C set, N clear. X and V kept.
 */
ItemScan RichterFindItemOut(void)
{
    ItemScan scan = { false, ITEM_SLOT_A, 0 };

    for (; scan.slot < ITEM_SLOT_END; scan.slot++) {
        scan.seen = rd8((uint16_t)(OBJ_KIND + scan.slot));
        if (scan.seen != 0) {
            scan.found = true;
            break;
        }
    }
    return scan;
}

void abi_RichterFindItemOut(void)
{
    ItemScan scan = RichterFindItemOut();

    cpu.y = scan.slot;
    cpu.a = scan.seen;
    abi_nz(scan.seen);
    if (!scan.found)
        abi_flag(P_C, true);
    else if (scan.slot != ITEM_SLOT_A)
        abi_flag(P_C, false);
}

/*
 * @routine 01BC:1EC RichterUseItemEight
 * @run 6D:B9EC
 * WHAT IT DOES: Item 8: only with no item out; the first use starts sound list $53 and script
 * $15. Then the item's attack, and its hearts paid.
 * HOW: RichterFindItemOut; PlayerItemFirstUse marks the first use.
 * WAITS: no.
 * Entry: X twice the item. Exit: as RichterUseItem.
 */
ItemUse RichterUseItemEight(uint8_t item)
{
    ItemUse use = item_handler(item);

    if (item_out(&use, 0xB9EE))
        return item_not_used(use, USE_ITEM_OUT);
    if (rd8(PlayerItemFirstUse) != 0) {
        start_item(&use, ITEM_EIGHT_LIST, ITEM_EIGHT_SCRIPT, 0xB9FF, 0xBA06);
        wr8(PlayerItemFirstUse, 0);
    }
    return item_used(use);
}

void abi_RichterUseItemEight(void)
{
    item_use_exit_shim(RichterUseItemEight((uint8_t)(cpu.x >> 1)));
}

/*
 * @routine 01BC:113 RichterLatchBits
 * @run 6D:B913
 * WHAT IT DOES: Adds the buttons in A to the ones latched in PadForcedPressB.
 * HOW: ORA of the byte, STA back.
 * WAITS: no.
 * Entry: A the buttons. Exit: A the latched byte, N and Z from it. X, Y, C and V kept.
 */
uint8_t RichterLatchBits(uint8_t buttons)
{
    uint8_t latched = (uint8_t)(rd8(PadForcedPressB) | buttons);

    wr8(PadForcedPressB, latched);
    return latched;
}

void abi_RichterLatchBits(void)
{
    cpu.a = RichterLatchBits(cpu.a);
    abi_nz(cpu.a);
}

/*
 * @routine 01BB:909 RichterLatchButtons
 * @run 6D:B909
 * WHAT IT DOES: Keeps the jump and attack presses until a state acts on them.
 * HOW: Bits 1 and 0 of zPadPressed ORed into RichterButtonsLatched.
 * WAITS: no.
 * Entry: nothing. Exit: A what is latched, N and Z from it. X, Y, C and V kept.
 */
uint8_t RichterLatchButtons(void)
{
    uint8_t pressed;

    pressed = RD8(ZP->pad_pressed);
    pressed = (uint8_t)(pressed & ITEM_USE_PRESS);
    jsr_push(0xB90F);
    RichterLatchBits(pressed);
    jsr_pop();
    pressed = RD8(ZP->pad_pressed);
    pressed = (uint8_t)(pressed & PAD_TURBO_BIT);
    return RichterLatchBits(pressed);
}

void abi_RichterLatchButtons(void)
{
    cpu.a = RichterLatchButtons();
    abi_nz(cpu.a);
}

/*
 * Maria's own pad and item code (her overlay 01DD): routines with no copy in Richter's overlay. Her player bytes
 * $3BE4 to $3BE7 have no symbol yet: they are named here as PlayerHeldCount plus an offset until the rows land.
 */
enum {
    MARIA_SEQUENCE_TIMER = PlayerHeldCount + 3, MARIA_SEQUENCE_STEP = PlayerHeldCount + 2,
    MARIA_SEQUENCE_DONE = PlayerHeldCount + 4, MARIA_SEQUENCE_TABLE = 0xB96B, MARIA_SEQUENCE_STEPS = 3,
    MARIA_SEQUENCE_TIME = 0x0A, PAD_DIRECTION_BITS = 0xF0,
    MARIA_ITEM_DELAY = 0x10, MARIA_ITEM_USE_PRESS = 0x02, MARIA_ITEM_USE_HELD = 0x10, MARIA_ITEM_STATES = 5,
    MARIA_ITEM_COSTS = 0xBCDE, MARIA_ITEM_SECOND_COSTS = 0xBCD5, MARIA_ITEM_LISTS = 0xBCE7, MARIA_ITEM_SCRIPTS = 0xBCF0,
    MARIA_ITEM_THROWN = 2, MARIA_ITEM_CHANNEL = 6, MARIA_LIVE_FIRST = 5, MARIA_LIVE_END = 0x0F, MARIA_KIND_IGNORED = 0x18,
    MARIA_ITEM_ALL = 0xFF, MARIA_ITEM_SLOT_A = 5, MARIA_ITEM_SLOT_B = 8
};

/*
 * @routine 01E0:124 MariaReadPad
 * @run 6D:B924
 * WHAT IT DOES: Reads the pad for the frame: what is held, and what is newly pressed.
 * HOW: The last frame's zPadHeld into zPadPrevious, BiosPadState into zPadHeld, the bits newly set into zPadPressed.
 * WAITS: no.
 * Entry: nothing. Exit: A the presses, N and Z from them. X, Y, C and V kept.
 */
uint8_t MariaReadPad(void)
{
    uint8_t held, previous, pressed;

    held = RD8(ZP->pad_held);
    WR8(ZP->pad_previous, held);
    held = rd8(BiosPadState);
    WR8(ZP->pad_held, held);
    previous = RD8(ZP->pad_previous);
    pressed = (uint8_t)(held ^ previous);
    pressed &= RD8(ZP->pad_held);
    WR8(ZP->pad_pressed, pressed);
    return pressed;
}

void abi_MariaReadPad(void)
{
    cpu.a = MariaReadPad();
    abi_nz(cpu.a);
}

/*
 * @routine 01E0:134 MariaPadSequence
 * @run 6D:B934
 * WHAT IT DOES: Follows a sequence of directions on her pad: each must come within ten frames of the last, in the
 * order her table at $B96B gives for the way she faces; the third sets the done flag.
 * HOW: A timer, the step and the done flag, three bytes after PlayerByte3BE3; the table by twice the step plus her
 * facing; a timer run out clears the step and the flag.
 * Untraced: the third step ($B95A); no script finished a sequence.
 * WAITS: no.
 * Entry: nothing. Exit: timer out or a step made, A $0A, N and Z from it; no direction, A 0, Z set; a direction not
 * the next, A the directions, N, Z and C from comparing them. X the table index unless the timer was out; C from
 * the index's add, or from comparing the step with 3; V from the index's add. Y kept.
 */
TurnWindow MariaPadSequence(void)
{
    TurnWindow r = { ended_kept(), false, 0 };
    uint8_t value, held;
    Sum index;

    value = rd8(MARIA_SEQUENCE_TIMER);
    r.end = ended_load(value);
    if (value == 0) {
        wr8(MARIA_SEQUENCE_STEP, 0);
        wr8(MARIA_SEQUENCE_DONE, 0);
    } else {
        rd8(MARIA_SEQUENCE_TIMER);
        wr8(MARIA_SEQUENCE_TIMER, (uint8_t)(value - 1));
        value = rd8(MARIA_SEQUENCE_STEP);
        index = add_bytes((uint8_t)(value << 1), RD8(OBJ->facing_left[0]), false);
        r.x_set = true;
        r.x = index.value;
        r.end = ended_of_sum(index);
        held = (uint8_t)(RD8(ZP->pad_held) & PAD_DIRECTION_BITS);
        r.end = ended_then(r.end, ended_load(held));
        if (held == 0) {
            return r;
        }
        value = rd8((uint16_t)(MARIA_SEQUENCE_TABLE + r.x));
        r.end = ended_then(r.end, ended_compare(held, value));
        if (held != value) {
            return r;
        }
        value = rd8(MARIA_SEQUENCE_STEP);
        wr8(MARIA_SEQUENCE_STEP, (uint8_t)(value + 1));
        value = rd8(MARIA_SEQUENCE_STEP);
        r.end = ended_then(r.end, ended_compare(value, MARIA_SEQUENCE_STEPS));
        if (value == MARIA_SEQUENCE_STEPS) {
            value = rd8(MARIA_SEQUENCE_DONE);
            wr8(MARIA_SEQUENCE_DONE, (uint8_t)(value + 1));
        }
    }
    wr8(MARIA_SEQUENCE_TIMER, MARIA_SEQUENCE_TIME);
    r.end = ended_then(r.end, ended_load(MARIA_SEQUENCE_TIME));
    return r;
}

void abi_MariaPadSequence(void)
{
    TurnWindow r = MariaPadSequence();

    abi_ended(r.end);
    if (r.x_set) {
        cpu.x = r.x;
    }
}

/* A search for a live object in slots 5 to 14: the slot (Y), the kind last read, and whether one was found. */
typedef struct { bool found; uint8_t y; uint8_t kind; } LiveObject;

/*
 * @routine 01E0:4C3 MariaFindLiveObject
 * @run 6D:BCC3
 * WHAT IT DOES: Looks for an object alive in slots 5 to 14, other than one of kind $18.
 * HOW: OBJ_KIND by Y.
 * Untraced: all of it; only MariaUseItem calls it, past its own untraced test.
 * WAITS: no.
 * Entry: nothing. Exit: found, Y the slot, A its kind, N, Z and C from comparing it with $18; else Y $0F, A the
 * last kind read, Z and C set, N clear. X and V kept.
 */
LiveObject MariaFindLiveObject(void)
{
    LiveObject r = { false, 0, 0 };

    for (r.y = MARIA_LIVE_FIRST; r.y != MARIA_LIVE_END; r.y++) {
        r.kind = RD8(OBJ->kind[r.y]);
        if (r.kind != 0 && r.kind != MARIA_KIND_IGNORED) {
            r.found = true;
            return r;
        }
    }
    return r;
}

static Ended live_object_exit(LiveObject r)
{
    if (r.found) {
        return ended_with_y(ended_compare(r.kind, MARIA_KIND_IGNORED), r.y);
    }
    return ended_with_y(ended_with_a(ended_compare(MARIA_LIVE_END, MARIA_LIVE_END), r.kind), r.y);
}

void abi_MariaFindLiveObject(void)
{
    abi_ended(live_object_exit(MariaFindLiveObject()));
}

/* The end every use of her item makes: the delay of $10 frames, C set. */
static Ended item_delay(Ended e)
{
    wr8(OBJ_EVENT_ID, MARIA_ITEM_DELAY);
    return ended_with_carry(ended_then(e, ended_load(MARIA_ITEM_DELAY)), true);
}

/*
 * @routine 01E0:43A MariaItemUseAll
 * @run 6D:BC3A
 * WHAT IT DOES: The end of MariaUseItem when her sequence is done: PlayerByte3BD5 $FF, then the item delay.
 * HOW: A jump into MariaUseItem's end.
 * Untraced: all of it; no script finished a sequence.
 * WAITS: no.
 * Entry: nothing. Exit: A $10, N and Z from it, C set. X, Y and V kept.
 */
Ended MariaItemUseAll(void)
{
    wr8(PlayerByte3BD5, MARIA_ITEM_ALL);
    return item_delay(ended_load(MARIA_ITEM_ALL));
}

void abi_MariaItemUseAll(void)
{
    abi_ended(MariaItemUseAll());
}

/*
 * @routine 01E0:442 MariaUseItem
 * @run 6D:BC42
 * WHAT IT DOES: Her attack button: unless the delay in OBJ_EVENT_ID runs, on a press of button II says she attacks
 * (C set) and starts the delay. With down held and the hearts her table at $BCDE asks, an item of 2 and up is
 * thrown: its list and script start once (PlayerByte3BD6) unless an object is alive in slots 5 to 14, it waits for
 * slot 5, and slot 8 past the second cost at $BCD5, and the hearts are paid; PlayerByte3BD5 then says the item is
 * in use. In states below 5 with her pad sequence done, PlayerByte3BD5 is $FF.
 * HOW: Bit 1 of zPadPressed, bit 4 of zPadHeld, zField93 the item, zField94 the hearts in BCD; MariaFindLiveObject;
 * the list on channel 6 and the script in slot 6.
 * Untraced: the sequence's arm ($BC56 taken, into MariaItemUseAll) and the throw ($BC77 to $BCC0); no script threw
 * one of her items or finished a sequence.
 * WAITS: no.
 * Entry: nothing. Exit: the delay runs, A the delay, N and Z from it, C clear; no press, A 0, Z set, C clear;
 * otherwise A $10, N and Z from it, C set, X the item once it was read, Y as the throw's last call left it. V kept.
 */
/* The throw of an item of 2 and up: false where it is not thrown (an object alive, or slot 5 or 8 not ready). */
static bool maria_throw_item(TurnWindow *r)
{
    uint8_t value, hearts;
    LiveObject live;

    value = rd8(PlayerByte3BD6);
    if (value != 0) {
        jsr_push(0xBC7F);
        live = MariaFindLiveObject();
        jsr_pop();
        r->end = ended_then(r->end, live_object_exit(live));
        if (live.found) {
            return false;
        }
        r->x = RD8(ZP->field93);
        value = rd8((uint16_t)(MARIA_ITEM_LISTS + r->x));
        if (value != 0) {
            jsr_push(0xBC8D);
            StartListChannelKind2(value, MARIA_ITEM_CHANNEL);
            jsr_pop();
        }
        wr8(PlayerByte3BD6, 0);
        r->x = RD8(ZP->field93);
        value = rd8((uint16_t)(MARIA_ITEM_SCRIPTS + r->x));
        if (value != 0) {
            jsr_push(0xBC9C);
            StartScriptSlotKind1(value, MARIA_ITEM_CHANNEL);
            jsr_pop();
            r->end = ended_with_y(r->end, value);
        }
    }
    r->x = RD8(ZP->field93);
    value = RD8(OBJ->kind[MARIA_ITEM_SLOT_A]);
    if (value != 0) {
        r->x = RD8(ZP->field93);
        hearts = RD8(ZP->field94);
        value = rd8((uint16_t)(MARIA_ITEM_SECOND_COSTS + r->x));
        if (hearts < value) {
            return false;
        }
        value = RD8(OBJ->kind[MARIA_ITEM_SLOT_B]);
        if (value != 0) {
            return false;
        }
    }
    wr8(PlayerByte3BD5, 1);
    hearts = RD8(ZP->field94);
    value = rd8((uint16_t)(MARIA_ITEM_COSTS + r->x));
    WR8(ZP->field94, bcd_subtract(hearts, value));
    r->end = item_delay(r->end);
    return true;
}

TurnWindow MariaUseItem(void)
{
    TurnWindow r = { ended_kept(), false, 0 };
    uint8_t value, hearts;

    value = rd8(OBJ_EVENT_ID);
    r.end = ended_load(value);
    if (value != 0 || !(RD8(ZP->pad_pressed) & MARIA_ITEM_USE_PRESS)) {
        r.end = ended_with_carry(r.end, false);
        return r;
    }
    value = RD8(OBJ->state[0]);
    if (value < MARIA_ITEM_STATES) {
        value = rd8(MARIA_SEQUENCE_DONE);
        if (value != 0) {
            r.end = MariaItemUseAll();
            return r;
        }
    }
    value = (uint8_t)(RD8(ZP->pad_held) & MARIA_ITEM_USE_HELD);
    if (value != 0) {
        r.x_set = true;
        r.x = RD8(ZP->field93);
        hearts = RD8(ZP->field94);
        value = rd8((uint16_t)(MARIA_ITEM_COSTS + r.x));
        if (hearts >= value) {
            value = RD8(ZP->field93);
            if (value >= MARIA_ITEM_THROWN && maria_throw_item(&r)) {
                return r;
            }
        }
    }
    wr8(PlayerByte3BD5, 0);
    r.end = item_delay(r.end);
    return r;
}

void abi_MariaUseItem(void)
{
    TurnWindow r = MariaUseItem();

    abi_ended(r.end);
    if (r.x_set) {
        cpu.x = r.x;
    }
}
