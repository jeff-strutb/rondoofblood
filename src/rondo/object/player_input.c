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

enum {
    PAD_LEFT_OR_RIGHT = 0xA0, PAD_RIGHT_ONLY = 0x20, PAD_TURBO_BIT = 0x01, PAD_ORDINARY_BITS = 0xFE,
    PAD_SELECT_BIT = 0x04, ANIMATION_STOPPED = 0xFF, ANIMATION_LOOPS = 0xFE,
    ITEM_READY = 0x0F, ITEM_REQUEST = 0x04, ITEM_FIRST_SLOT = 3, ITEM_SLOT_END = 0x10,
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
void RichterClearPadOverrides(void)
{
    for (uint16_t i = 0; i < 4; i++)
        wr8((uint16_t)(PadOverrides + i), 0);
}

void abi_RichterClearPadOverrides(void)
{
    RichterClearPadOverrides();
}

/* The exit state of an animation routine as the assembly leaves it. */
static AnimationStep animation_exit(uint8_t a, uint8_t nz, bool c, bool y_set, uint8_t y)
{
    AnimationStep r = { FRAME_STOPPED, false, false, 0, 0, true, a, nz, c, y_set, y };

    return r;
}

/*
 * The part of the animation code the others call (01BB:69C, no tag of its own): points zTemp00-01 at the list of the
 * animation in play and gives Y twice the step. A is that doubled step and C its bit 7 when it returns.
 */
static uint8_t animation_list_and_step(bool *carry)
{
    uint8_t animation = RD8(OBJ->fields_2ee0[0]);
    uint8_t doubled = (uint8_t)(animation << 1);
    uint8_t step;

    WR8(ZP->temp00, rd8((uint16_t)(AnimationLists + doubled)));
    WR8(ZP->temp01, rd8((uint16_t)(AnimationLists + 1 + doubled)));
    step = RD8(OBJ->flags[0]);
    *carry = (step & 0x80) != 0;
    return (uint8_t)(step << 1);
}

/* The entry of the step in play: how long, then which frame; a mark of $FE starts the list over. */
static AnimationStep animation_read_entry(void)
{
    AnimationStep result;
    bool carry;
    uint8_t y, time, frame;

    for (;;) {
        jsr_push(0xB683);
        y = animation_list_and_step(&carry);
        jsr_pop();
        time = RD8_PTR_Y(ZP->temp00, y);
        WR8(OBJ->f2f90[0], time);
        if (time == ANIMATION_STOPPED) {
            result = animation_exit(ANIMATION_STOPPED, 0, true, true, y);
            result.how = FRAME_END_MARK;
            result.step = (uint8_t)(y >> 1);
            return result;
        }
        if (time != ANIMATION_LOOPS) {
            break;
        }
        WR8(OBJ->flags[0], 0);
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
AnimationWaitEnd RichterAnimationWait(void)
{
    AnimationWaitEnd r = { { FRAME_STOPPED }, false, 0 };

    jsr_push(0xB65B);
    r.step = RichterAnimationRunning();
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
AnimationStep RichterAnimationRunning(void)
{
    AnimationStep tick;
    uint8_t time;

    jsr_push(0xB663);
    tick = RichterAnimationTick();
    jsr_pop();
    time = RD8(OBJ->f2f90[0]);
    return animation_exit(time, (uint8_t)(time - ANIMATION_STOPPED), time >= ANIMATION_STOPPED, tick.y_set, tick.y);
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
 * HOW: The timer in OBJ_2F90, the step in OBJ_FLAGS_2F38, the frame shown in OBJ_2A68.
 * WAITS: no.
 * Entry: nothing. Exit: A the frame or $FF; Y twice the step, plus one with a frame; C and N, Z
 * from what was read. X and V kept. Clobbers zTemp00-01.
 */
AnimationStep RichterAnimationTick(void)
{
    AnimationStep result;
    uint8_t time = RD8(OBJ->f2f90[0]);
    uint8_t left, y, frame;
    bool carry;

    if (time == ANIMATION_STOPPED) {
        return animation_exit(ANIMATION_STOPPED, 0, true, false, 0);
    }
    left = DEC8(OBJ->f2f90[0]);
    if (left == 0) {
        INC8(OBJ->flags[0]);
        return animation_read_entry();
    }
    jsr_push(0xB695);
    y = animation_list_and_step(&carry);
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
 * HOW: The animation into OBJ_FIELDS_2EE0, step zero, and its first entry read.
 * WAITS: no.
 * Entry: A animation. Exit: as RichterAnimationTick. Clobbers zTemp00-01.
 */
AnimationStep RichterSetAnimation(uint8_t animation)
{
    WR8(OBJ->fields_2ee0[0], animation);
    WR8(OBJ->flags[0], 0);
    return animation_read_entry();
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
 * WAITS: no.
 * Entry: nothing. Exit: A the last read, N and Z from it; X the state if it went that far. Y,
 * C and V kept.
 */
TurnWindow RichterCheckTurnWindow(void)
{
    TurnWindow r = { ended_kept(), false, 0 };
    uint8_t value, facing, x_lo;

    wr8(PlayerByte3BD7, 0);
    value = RD8(OBJ->f2800[0]);
    if (value != 0) {
        r.end = ended_load(value);
        return r;
    }
    r.x_set = true;
    r.x = RD8(OBJ->state[0]);
    value = rd8((uint16_t)(RichterTurnStates + r.x));
    wr8(PlayerByte3BD7, value);
    r.end = ended_load(value);
    facing = RD8(OBJ->facing_left[0]);
    x_lo = RD8(OBJ->x_lo[0]);
    r.end = ended_load(x_lo);
    if ((facing == 0) != ((x_lo & 0x80) != 0)) {
        wr8(PlayerByte3BD7, 0);
    }
    return r;
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
 * @routine 01BB:7CA RichterFaceByPad
 * @run 6D:B7CA
 * WHAT IT DOES: Turns him to face the way the pad holds, left or right; with neither, as he is.
 * HOW: Bits 7 and 5 of zPadHeld, and facing in OBJ_2AC0: 0 for right, 1 for left.
 * WAITS: no.
 * Entry: nothing. Exit: A the bits looked at last; N and Z from them, or from the facing when it
 * turns left. X, Y, C and V kept.
 */
Ended RichterFaceByPad(void)
{
    uint8_t held = RD8(ZP->pad_held);
    uint8_t turn = (uint8_t)(held & PAD_LEFT_OR_RIGHT);

    if (turn == 0) {
        return ended_load(turn);
    }
    WR8(OBJ->facing_left[0], 0);
    turn = (uint8_t)(turn & PAD_RIGHT_ONLY);
    if (turn != 0) {
        return ended_load(turn);
    }
    return ended_with_a(ended_nz(INC8(OBJ->facing_left[0])), 0);
}

void abi_RichterFaceByPad(void)
{
    abi_ended(RichterFaceByPad());
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
EndedTst RichterNoteItemPress(void)
{
    uint8_t pressed = RD8(ZP->pad_pressed);
    EndedTst r;

    if ((pressed & PAD_SELECT_BIT) == 0) {
        return ended_tst(ended_kept(), pressed, PAD_SELECT_BIT);
    }
    wr8(PadForcedPress, ITEM_REQUEST);
    r = ended_tst(ended_load(ITEM_REQUEST), pressed, PAD_SELECT_BIT);
    r.v_only = true;
    return r;
}

void abi_RichterNoteItemPress(void)
{
    abi_ended_tst(RichterNoteItemPress());
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
EndedX RichterCheckItemHearts(void)
{
    EndedX r;
    uint8_t item, hearts, cost;

    wr8(RichterItemReady, 0);
    item = RD8(ZP->field93);
    hearts = RD8(ZP->field94);
    cost = rd8((uint16_t)(RichterItemCosts + item));
    r.x = item;
    r.end = ended_compare(hearts, cost);
    if (hearts >= cost) {
        wr8(RichterItemReady, ITEM_READY);
        r.end = ended_load(ITEM_READY);
        r.end = ended_with_carry(r.end, true);
    }
    return r;
}

void abi_RichterCheckItemHearts(void)
{
    EndedX r = RichterCheckItemHearts();

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
 * HOW: Bit 2 of zPadPressed; OBJ_2750 of slots 3 to 15 all empty; RichterItemCosts in BCD, then
 * RichterItemCrashLists and RichterItemCrashScripts by item, 0 for none, on channel and slot 6.
 * WAITS: no.
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
        for (uint8_t slot = ITEM_SLOT_END - 1; slot >= ITEM_FIRST_SLOT; slot--) {
            jsr_push(0xB811);
            ClearObject(slot);
            jsr_pop();
        }
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
    if (list != 0) {
        jsr_push(0xB830);
        StartListChannelKind2(list, ITEM_CRASH_CHANNEL);
        jsr_pop();
    }
    wr8(PlayerByte3BD6, 0);
    crash.script = rd8((uint16_t)(RichterItemCrashScripts + crash.item));
    if (crash.script != 0) {
        jsr_push(0xB83F);
        StartScriptSlotKind1(crash.script, ITEM_CRASH_SLOT);
        jsr_pop();
    }
    crash.hearts = bcd_subtract(rd8(zField94), crash.cost);
    wr8(zField94, crash.hearts);
    return crash;
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
 * @routine 01BC:129 RichterUseItem
 * @run 6D:B929
 * WHAT IT DOES: Using the item in hand: on the use press with the use direction held and the
 * hearts the item costs, goes to the item's own handler, which throws it or not. Says whether
 * to attack, and with what in PlayerByte3BD5: 0 the whip, or the item's attack.
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
    uint16_t entry;

    if (!(rd8(zPadPressed) & ITEM_USE_PRESS))
        return use;
    if (!(rd8(zPadHeld) & ITEM_USE_HELD))
        return item_not_used(use, USE_NOT_HELD);
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

void abi_RichterUseItem(void)
{
    item_use_exit_shim(RichterUseItem());
}

/* Not used: attack with the whip. */
static ItemUse item_not_used(ItemUse use, ItemUseHow how)
{
    use.how = how;
    wr8(PlayerByte3BD5, 0);
    return use;
}

/* Used: the item's attack, and its hearts paid. */
static ItemUse item_used(ItemUse use)
{
    use.how = USE_STARTED;
    wr8(PlayerByte3BD5, rd8((uint16_t)(RichterItemAttacks + use.item)));
    use.hearts = bcd_subtract(rd8(zField94), rd8((uint16_t)(RichterItemUseCosts + use.item)));
    wr8(zField94, use.hearts);
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
static bool item_out(ItemUse *use)
{
    ItemScan scan = RichterFindItemOut();

    use->scanned = true;
    use->slot = scan.slot;
    use->seen = scan.seen;
    return scan.found;
}

/* A start of the item's sound and script on channel and slot 6, as the first of its throws. */
static void start_item(ItemUse *use, uint8_t list, uint8_t script)
{
    StartListChannelKind2(list, ITEM_CRASH_CHANNEL);
    StartScriptSlotKind1(script, ITEM_CRASH_SLOT);
    use->script = script;
    use->script_started = true;
}

/*
 * @routine 01BC:15B RichterUseThrownItem
 * @run 6D:B95B
 * WHAT IT DOES: Items 2 to 5, which can be thrown up to three at a time: the first throw of a
 * use starts the item's sound and script, unless one is out already; a second or third throw
 * needs more hearts, and a fourth is refused. Then the item's attack, and its hearts paid.
 * HOW: PlayerByte3BD6 marks the first throw. The items fly in slots 5, 8 and 11; with one out
 * the hearts must reach RichterItemSecondCosts, with two RichterItemThirdCosts.
 * WAITS: no.
 * Entry: X twice the item. Exit: as RichterUseItem.
 */
ItemUse RichterUseThrownItem(uint8_t item)
{
    ItemUse use = item_handler(item);

    if (rd8(PlayerByte3BD6) != 0) {
        if (item_out(&use))
            return item_not_used(use, USE_ITEM_OUT);
        StartListChannelKind2(rd8((uint16_t)(RichterItemUseLists + use.item)), ITEM_CRASH_CHANNEL);
        wr8(PlayerByte3BD6, 0);
        use.script = rd8((uint16_t)(RichterItemUseScripts + use.item));
        StartScriptSlotKind1(use.script, ITEM_CRASH_SLOT);
        use.script_started = true;
    }
    use.by_index = false;
    if (rd8(OBJ_2750 + ITEM_SLOT_A) != 0) {
        use.hearts = rd8(zField94);
        use.cost = rd8((uint16_t)(RichterItemSecondCosts + use.item));
        if (use.hearts < use.cost)
            return item_not_used(use, USE_TOO_FEW_HEARTS);
        if (rd8(OBJ_2750 + ITEM_SLOT_B) != 0) {
            use.hearts = rd8(zField94);
            use.cost = rd8((uint16_t)(RichterItemThirdCosts + use.item));
            if (use.hearts < use.cost)
                return item_not_used(use, USE_TOO_FEW_HEARTS);
            use.seen = rd8(OBJ_2750 + ITEM_SLOT_C);
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
 * HOW: RichterFindItemOut; PlayerByte3BD6 marks the first use.
 * WAITS: no.
 * Entry: X twice the item. Exit: as RichterUseItem.
 */
ItemUse RichterUseItemSix(uint8_t item)
{
    ItemUse use = item_handler(item);

    if (item_out(&use))
        return item_not_used(use, USE_ITEM_OUT);
    if (rd8(PlayerByte3BD6) != 0) {
        start_item(&use, ITEM_SIX_LIST, ITEM_SIX_SCRIPT);
        wr8(PlayerByte3BD6, 0);
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
 * HOW: OBJ_2750 of each slot in turn.
 * WAITS: no.
 * Entry: X twice the item. Exit: as RichterUseItem.
 */
ItemUse RichterUseItemSeven(uint8_t item)
{
    ItemUse use = item_handler(item);

    if (rd8(OBJ_2750 + ITEM_SLOT_A) == 0 || rd8(OBJ_2750 + ITEM_SLOT_B) == 0)
        return item_used(use);
    use.seen = rd8(OBJ_2750 + ITEM_SLOT_C);
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
 * HOW: OBJ_2750 of each.
 * WAITS: no.
 * Entry: nothing. Exit: Y the slot, or $10; A its OBJ_2750, or 0. Found: N and Z from A, C kept
 * if it was slot 5, else clear. None: Z and C set, N clear. X and V kept.
 */
ItemScan RichterFindItemOut(void)
{
    ItemScan scan = { false, ITEM_SLOT_A, 0 };

    for (; scan.slot < ITEM_SLOT_END; scan.slot++) {
        scan.seen = rd8((uint16_t)(OBJ_2750 + scan.slot));
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
 * HOW: RichterFindItemOut; PlayerByte3BD6 marks the first use.
 * WAITS: no.
 * Entry: X twice the item. Exit: as RichterUseItem.
 */
ItemUse RichterUseItemEight(uint8_t item)
{
    ItemUse use = item_handler(item);

    if (item_out(&use))
        return item_not_used(use, USE_ITEM_OUT);
    if (rd8(PlayerByte3BD6) != 0) {
        start_item(&use, ITEM_EIGHT_LIST, ITEM_EIGHT_SCRIPT);
        wr8(PlayerByte3BD6, 0);
    }
    return item_used(use);
}

void abi_RichterUseItemEight(void)
{
    item_use_exit_shim(RichterUseItemEight((uint8_t)(cpu.x >> 1)));
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
    uint8_t latched, pressed, held;

    pressed = RD8(ZP->pad_pressed);
    pressed = (uint8_t)(pressed & ITEM_USE_PRESS);
    jsr_push(0xB90F);
    held = rd8(PadForcedPressB);
    latched = (uint8_t)(held | pressed);
    wr8(PadForcedPressB, latched);
    jsr_pop();
    pressed = RD8(ZP->pad_pressed);
    pressed = (uint8_t)(pressed & PAD_TURBO_BIT);
    held = rd8(PadForcedPressB);
    latched = (uint8_t)(held | pressed);
    wr8(PadForcedPressB, latched);
    return latched;
}

void abi_RichterLatchButtons(void)
{
    cpu.a = RichterLatchButtons();
    abi_nz(cpu.a);
}
