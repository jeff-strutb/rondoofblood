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

enum {
    PAD_LEFT_OR_RIGHT = 0xA0, PAD_RIGHT_ONLY = 0x20, PAD_TURBO_BIT = 0x01, PAD_ORDINARY_BITS = 0xFE,
    PAD_SELECT_BIT = 0x04, ANIMATION_STOPPED = 0xFF, ANIMATION_LOOPS = 0xFE,
    ITEM_READY = 0x0F, ITEM_REQUEST = 0x04, ITEM_FIRST_SLOT = 3, ITEM_SLOT_END = 0x10,
    ITEM_CRASH_CHANNEL = 6, ITEM_CRASH_SLOT = 6
};

/*
 * @routine 01BB:611 RichterReadPad
 * @run 6D:B611
 * WHAT IT DOES: Reads the pad for the frame: what is held, and what is newly pressed. Bit 0
 * counts as pressed only when held for three frames after being up, which steadies it.
 * HOW: The last four frames of the pad are kept in PadHistory.
 * Entry: nothing. Exit: A the new presses, N and Z from them. X, Y, C and V kept. Clobbers
 * zTemp00.
 */
uint8_t RichterReadPad(void)
{
    uint8_t before = rd8(zPadHeld);
    uint8_t oldest = rd8(PadHistory + 2);
    uint8_t second = rd8(PadHistory + 1);
    uint8_t third = rd8(PadHistory);
    uint8_t held = rd8(BiosPadState);
    uint8_t steady;
    uint8_t pressed;

    wr8(zPadPrevious, before);
    wr8(PadHistory + 3, oldest);
    wr8(PadHistory + 2, second);
    wr8(PadHistory + 1, third);
    wr8(PadHistory, held);
    wr8(zPadHeld, held);
    steady = (uint8_t)((held ^ oldest) & second & third & held & PAD_TURBO_BIT);
    wr8(zTemp00, steady);
    pressed = (uint8_t)(((held ^ before) & held & PAD_ORDINARY_BITS) | steady);
    wr8(zPadPressed, pressed);
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
 * Entry: nothing. Exit: all registers and flags kept.
 */
void RichterClearPadOverrides(void)
{
    for (uint16_t i = 0; i < 4; i++)
        wr8((uint16_t)(PadOverrides + i), 0);
}

void abi_RichterClearPadOverrides(void)
{
    RichterClearPadOverrides();
}

/*
 * @routine 01BB:659 RichterAnimationWait
 * @run 6D:B659
 * WHAT IT DOES: Steps the animation and, unless it has ended, returns to its caller's caller, so
 * the caller's own work waits for the end.
 * HOW: RichterAnimationRunning; the assembly drops the return address.
 * Entry: nothing. Exit, ended: as RichterAnimationRunning. Otherwise the caller's return address
 * is gone, A its high byte, N and Z from it.
 */
static void drop_return_shim(void)
{
    cpu.a = rd8((uint16_t)(0x2100 + (uint8_t)(cpu.s + 2)));
    cpu.s = (uint8_t)(cpu.s + 2);
    abi_nz(cpu.a);
}

void abi_RichterAnimationWait(void)
{
    AnimationStep step = RichterAnimationRunning();

    animation_running_exit_shim(step);
    if (rd8(OBJ_2F90) != ANIMATION_STOPPED)
        drop_return_shim();
}

/*
 * @routine 01BB:660 RichterAnimationRunning
 * @run 6D:B660
 * WHAT IT DOES: Steps the animation; a caller looks at whether it has ended.
 * HOW: RichterAnimationTick, then the timer against $FF.
 * Entry: nothing. Exit: A the timer, N, Z and C from comparing it with $FF. Y as the tick. X and V
 * kept. Clobbers zTemp00-01.
 */
AnimationStep RichterAnimationRunning(void)
{
    return RichterAnimationTick();
}

void animation_running_exit_shim(AnimationStep step)
{
    uint8_t time = rd8(OBJ_2F90);

    animation_exit_shim(step);
    cpu.a = time;
    abi_nz((uint8_t)(time - ANIMATION_STOPPED));
    abi_flag(P_C, time == ANIMATION_STOPPED);
}

void abi_RichterAnimationRunning(void)
{
    animation_running_exit_shim(RichterAnimationRunning());
}

/* The list of the animation in play, with zTemp00-01 left pointing at it as the original does. */
static uint16_t animation_list(void)
{
    uint16_t entry = (uint16_t)(AnimationLists + (uint8_t)(rd8(OBJ_FIELDS_2EE0) << 1));
    uint16_t list = (uint16_t)(rd8(entry) | (rd8((uint16_t)(entry + 1)) << 8));

    wr8(zTemp00, (uint8_t)list);
    wr8(zTemp01, (uint8_t)(list >> 8));
    return list;
}

/* The list's entry at this step: how long, then which frame. A mark of $FE starts it over. */
static AnimationStep animation_read_entry(AnimationStep result)
{
    for (;;) {
        uint16_t list = animation_list();
        uint8_t step = rd8(OBJ_FLAGS_2F38);
        uint8_t time;

        result.step = step;
        time = rd8((uint16_t)(list + (uint8_t)(step << 1)));
        wr8(OBJ_2F90, time);
        if (time == ANIMATION_STOPPED) {
            result.how = FRAME_END_MARK;
            return result;
        }
        if (time != ANIMATION_LOOPS) {
            result.frame = rd8((uint16_t)(list + (uint8_t)((step << 1) + 1)));
            wr8(OBJ_2A68, result.frame);
            result.how = FRAME_SHOWN;
            return result;
        }
        wr8(OBJ_FLAGS_2F38, 0);
        result.looped = true;
    }
}

/*
 * @routine 01BB:669 RichterAnimationTick
 * @run 6D:B669
 * WHAT IT DOES: Counts down the frame on show; at zero moves to the next step of the list.
 * A timer of $FF means the animation has stopped.
 * HOW: The timer in OBJ_2F90, the step in OBJ_FLAGS_2F38, the frame shown in OBJ_2A68.
 * Entry: nothing. Exit: A the frame or $FF; Y twice the step, plus one with a frame; C and N, Z
 * from what was read. X and V kept. Clobbers zTemp00-01.
 */
AnimationStep RichterAnimationTick(void)
{
    AnimationStep result = { FRAME_STOPPED };
    uint8_t time = rd8(OBJ_2F90);

    if (time == ANIMATION_STOPPED)
        return result;
    wr8(OBJ_2F90, (uint8_t)(time - 1));
    if (time == 1) {
        wr8(OBJ_FLAGS_2F38, (uint8_t)(rd8(OBJ_FLAGS_2F38) + 1));
        return animation_read_entry(result);
    }
    {
        uint16_t list = animation_list();
        uint8_t step = rd8(OBJ_FLAGS_2F38);

        result.step = step;
        result.same_frame = true;
        result.frame = rd8((uint16_t)(list + (uint8_t)((step << 1) + 1)));
        wr8(OBJ_2A68, result.frame);
        result.how = FRAME_SHOWN;
        return result;
    }
}

void animation_exit_shim(AnimationStep result)
{
    uint8_t doubled = (uint8_t)(result.step << 1);

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
 * HOW: The animation into OBJ_FIELDS_2EE0, step zero, and its first entry read.
 * Entry: A animation. Exit: as RichterAnimationTick. Clobbers zTemp00-01.
 */
AnimationStep RichterSetAnimation(uint8_t animation)
{
    AnimationStep result = { FRAME_STOPPED };

    wr8(OBJ_FIELDS_2EE0, animation);
    wr8(OBJ_FLAGS_2F38, 0);
    return animation_read_entry(result);
}

void abi_RichterSetAnimation(void)
{
    animation_exit_shim(RichterSetAnimation(cpu.a));
}

/*
 * @routine 01BB:704 RichterCheckTurnWindow
 * @run 6D:B704
 * WHAT IT DOES: Says whether the state in play lets him turn at the screen's middle: a table
 * by state, in the first step only, and only on the side of the middle he faces away from.
 * HOW: PlayerByte3BD7 is the answer.
 * Entry: nothing. Exit: A the last read, N and Z from it; X the state if it went that far. Y,
 * C and V kept.
 */
void RichterCheckTurnWindow(void)
{
    wr8(PlayerByte3BD7, 0);
    if (rd8(OBJ_2800) == 0) {
        bool faces_left = rd8(OBJ_2AC0) != 0;
        bool on_right = (rd8(OBJ_X_LO) & 0x80) != 0;

        wr8(PlayerByte3BD7, rd8((uint16_t)(RichterTurnStates + rd8(OBJ_27A8))));
        if (faces_left == on_right)
            wr8(PlayerByte3BD7, 0);
    }
}

void abi_RichterCheckTurnWindow(void)
{
    RichterCheckTurnWindow();
    if (rd8(OBJ_2800) == 0) {
        cpu.x = rd8(OBJ_27A8);
        cpu.a = rd8(OBJ_X_LO);
    } else {
        cpu.a = rd8(OBJ_2800);
    }
    abi_nz(cpu.a);
}

/*
 * @routine 01BB:7CA RichterFaceByPad
 * @run 6D:B7CA
 * WHAT IT DOES: Turns him to face the way the pad holds, left or right; with neither, as he is.
 * HOW: Bits 7 and 5 of zPadHeld, and facing in OBJ_2AC0: 0 for right, 1 for left.
 * Entry: nothing. Exit: A the bits looked at last; N and Z from them, or from the facing when it
 * turns left. X, Y, C and V kept.
 */
void RichterFaceByPad(void)
{
    uint8_t held = rd8(zPadHeld);

    if (held & PAD_LEFT_OR_RIGHT)
        wr8(OBJ_2AC0, (held & PAD_RIGHT_ONLY) ? 0 : 1);
}

/* Facing left ends on the INC that sets the facing to 1: N and Z from that, A still 0. */
static void face_exit_shim(uint8_t held)
{
    bool turned_left = (held & PAD_LEFT_OR_RIGHT) != 0 && !(held & PAD_RIGHT_ONLY);

    cpu.a = (held & PAD_LEFT_OR_RIGHT) ? (uint8_t)(held & PAD_RIGHT_ONLY) : 0;
    abi_nz(turned_left ? 1 : cpu.a);
}

void abi_RichterFaceByPad(void)
{
    uint8_t held = rd8(zPadHeld);

    RichterFaceByPad();
    face_exit_shim(held);
}

/*
 * @routine 01BB:7DD RichterNoteItemPress
 * @run 6D:B7DD
 * WHAT IT DOES: Notes that the item button was pressed this frame.
 * HOW: Bit 2 of zPadPressed sets RichterItemRequest to 4.
 * Entry: nothing. Exit: Z from the test, N and V bits 7 and 6 of the presses; A 4 if pressed.
 * X, Y and C kept.
 */
void RichterNoteItemPress(void)
{
    if (rd8(zPadPressed) & PAD_SELECT_BIT)
        wr8(PadForcedPress, ITEM_REQUEST);
}

void abi_RichterNoteItemPress(void)
{
    uint8_t pressed = rd8(zPadPressed);

    RichterNoteItemPress();
    abi_flag(P_V, (pressed & 0x40) != 0);
    if (pressed & PAD_SELECT_BIT) {
        cpu.a = ITEM_REQUEST;
        abi_nz(ITEM_REQUEST);
    } else {
        abi_nz(0);
        abi_flag(P_N, (pressed & 0x80) != 0);
    }
}

/*
 * @routine 01BB:7E9 RichterCheckItemHearts
 * @run 6D:B7E9
 * WHAT IT DOES: Says whether he has the hearts the item in hand costs.
 * HOW: zHearts against a table by item, in BCD; RichterItemReady is $0F if so, else 0.
 * Entry: nothing. Exit: X the item; A the hearts, N, Z and C from the compare, or A $0F. Y and
 * V kept.
 */
void RichterCheckItemHearts(void)
{
    uint8_t cost = rd8((uint16_t)(RichterItemCosts + rd8(zField93)));

    wr8(RichterItemReady, rd8(zField94) >= cost ? ITEM_READY : 0);
}

void abi_RichterCheckItemHearts(void)
{
    uint8_t item = rd8(zField93);
    uint8_t hearts = rd8(zField94);
    uint8_t cost = rd8((uint16_t)(RichterItemCosts + item));

    RichterCheckItemHearts();
    cpu.x = item;
    abi_flag(P_C, hearts >= cost);
    if (hearts >= cost) {
        cpu.a = ITEM_READY;
        abi_nz(ITEM_READY);
    } else {
        cpu.a = hearts;
        abi_nz((uint8_t)(hearts - cost));
    }
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
 * HOW: Bit 2 of zPadPressed; OBJ_2750 of slots 3 to 15 all empty; RichterItemCosts in BCD, then
 * RichterItemCrashLists and RichterItemCrashScripts by item, 0 for none, on channel and slot 6.
 * Entry: nothing. Exit: V bit 6 of the presses. Started: C set, A the hearts left, N and Z from
 * them, X the item, Y the script if one started. Not started, C clear and: no press, N bit 7 of
 * the presses, Z set, A, X, Y kept; an item out, A its OBJ_2750, N and Z from it, X its slot;
 * too few hearts, A the hearts, N and Z from comparing them with the cost, X the item. Y kept.
 */
ItemCrash RichterTryItemCrash(void)
{
    ItemCrash crash = { CRASH_NO_PRESS };
    uint8_t list;

    crash.pressed = rd8(zPadPressed);
    if (crash.pressed & PAD_SELECT_BIT) {
        for (crash.slot = ITEM_FIRST_SLOT; crash.slot < ITEM_SLOT_END; crash.slot++) {
            crash.out = rd8((uint16_t)(OBJ_2750 + crash.slot));
            if (crash.out != 0) {
                crash.how = CRASH_ITEM_OUT;
                break;
            }
        }
    }
    if (crash.pressed & PAD_SELECT_BIT && crash.how != CRASH_ITEM_OUT) {
        for (uint8_t slot = ITEM_SLOT_END - 1; slot >= ITEM_FIRST_SLOT; slot--)
            ClearObject(slot);
        crash.item = rd8(zField93);
        crash.hearts = rd8(zField94);
        crash.cost = rd8((uint16_t)(RichterItemCosts + crash.item));
        crash.how = crash.hearts >= crash.cost ? CRASH_STARTED : CRASH_TOO_FEW_HEARTS;
    }
    if (crash.how != CRASH_STARTED) {
        wr8(PadForcedPress, 0);
        return crash;
    }
    list = rd8((uint16_t)(RichterItemCrashLists + crash.item));
    if (list != 0)
        StartListChannelKind2(list, ITEM_CRASH_CHANNEL);
    wr8(PlayerByte3BD6, 0);
    crash.script = rd8((uint16_t)(RichterItemCrashScripts + crash.item));
    if (crash.script != 0)
        StartScriptSlotKind1(crash.script, ITEM_CRASH_SLOT);
    crash.hearts = bcd_subtract(rd8(zField94), crash.cost);
    wr8(zField94, crash.hearts);
    return crash;
}

void abi_RichterTryItemCrash(void)
{
    ItemCrash crash = RichterTryItemCrash();

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

/*
 * @routine 01BB:909 RichterLatchButtons
 * @run 6D:B909
 * WHAT IT DOES: Keeps the jump and attack presses until a state acts on them.
 * HOW: Bits 1 and 0 of zPadPressed ORed into RichterButtonsLatched.
 * Entry: nothing. Exit: A what is latched, N and Z from it. X, Y, C and V kept.
 */
uint8_t RichterLatchButtons(void)
{
    uint8_t latched = (uint8_t)(rd8(PadForcedPressB) | (rd8(zPadPressed) & 3));

    wr8(PadForcedPressB, latched);
    return latched;
}

void abi_RichterLatchButtons(void)
{
    cpu.a = RichterLatchButtons();
    abi_nz(cpu.a);
}
