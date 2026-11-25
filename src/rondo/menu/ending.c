/*
 * The ending scene (overlay sectors $010D to $0111, bank $6B at $8000): a scene script interpreter
 * of its own (the word at $7E, the frame counter zWaitTimer3 and zWaitTimer4) and the state machines of the
 * scene's objects: each object's state byte (OBJ->state) counts its steps, its kind picks the handler.
 *
 * Fields named by their address are placeholders until their names land: the scroll slots
 * (SCROLL->x_lo[i] and the like) are used by index.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"

enum { ENDING_LISTS = 0x8A05 };

/* A handler's start: the registers it leaves alone are marked kept. */
static ObjExit object_exit(uint8_t a, uint8_t nz)
{
    ObjExit result = { 0 };

    result.exit.a = a;
    result.exit.nz = nz;
    result.exit.x_kept = true;
    result.exit.y_kept = true;
    result.exit.c_kept = true;
    return result;
}

static void object_exit_shim(ObjExit result)
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

/* A compare's carry on an exit. */
static ObjExit with_carry(ObjExit result, bool carry)
{
    result.exit.c = carry;
    result.exit.c_kept = false;
    return result;
}

/*
 * @routine 010D:029 EndingFrameTick
 * @run 6B:8029
 * WHAT IT DOES: Counts one frame of the ending scene on its 16-bit frame counter.
 * HOW: Adds one to zWaitTimer3, and to zWaitTimer4 when it wrapped to zero.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the byte last incremented, C and the registers kept.
 */
uint8_t EndingFrameTick(void)
{
    uint8_t low;

    low = INC8(ZP->wait_timer3);
    if (low != 0) {
        return low;
    }
    return INC8(ZP->wait_timer4);
}

void abi_EndingFrameTick(void)
{
    abi_nz(EndingFrameTick());
}

/*
 * @routine 010D:067 EndingModeDoneBack
 * @run 6B:8067
 * WHAT IT DOES: Takes one back off zModeDone.
 * HOW: One decrement.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the new value, C and the registers kept.
 */
uint8_t EndingModeDoneBack(void)
{
    return DEC8(ZP->mode_done);
}

void abi_EndingModeDoneBack(void)
{
    abi_nz(EndingModeDoneBack());
}

/*
 * @routine 010D:5F2 EndingNothing
 * @run 6B:85F2
 * WHAT IT DOES: Returns at once.
 * HOW: An RTS.
 * WAITS: no.
 * Entry: nothing. Exit: everything kept.
 */
void EndingNothing(void)
{
}

void abi_EndingNothing(void)
{
    EndingNothing();
}

/*
 * @routine 010D:5F3 EndingHoldTimer
 * @run 6B:85F3
 * WHAT IT DOES: Counts down an object's hold timer; while it has not run out the caller is not returned to:
 * the routine returns to the caller's caller instead.
 * HOW: OBJ->life_timer counted down; when it is not zero the return address of the call is pulled off the
 * stack (two PLA) before the RTS.
 * WAITS: no.
 * Entry: X the object, the caller's frame on the stack. Exit: N and Z from the timer (it returns the timer), Z set when
 * it has run out; A, X, Y and C kept.
 */
uint8_t EndingHoldTimer(uint8_t object)
{
    uint8_t left = DEC8(OBJ->life_timer[object]);

    if (left != 0) {
        (void)pull8();
        (void)pull8();
    }
    return left;
}

void abi_EndingHoldTimer(void)
{
    uint8_t left;

    left = DEC8(OBJ->life_timer[cpu.x]);
    abi_nz(left);
    if (left != 0) {
        (void)pull8();
        (void)pull8();
    }
}

/*
 * @routine 010D:65E EndingScrollBack8
 * @run 6B:865E
 * WHAT IT DOES: Takes 8 off the low byte of the first scroll X.
 * HOW: A subtract of 8 from SCROLL->x_lo[0].
 * WAITS: no.
 * Entry: nothing. Exit: A the new byte, N and Z from it, C and V from the subtract; X and Y kept.
 */
Exit EndingScrollBack8(void)
{
    Exit exit = { 0 };
    uint8_t low = RD8(SCROLL->x_lo[0]);
    uint8_t diff = (uint8_t)(low - 8);

    WR8(SCROLL->x_lo[0], diff);
    exit.a = diff;
    exit.nz = diff;
    exit.c = low >= 8;
    exit.v = ((low ^ 8) & (low ^ diff) & 0x80) != 0;
    exit.v_set = true;
    exit.x_kept = exit.y_kept = true;
    return exit;
}

void abi_EndingScrollBack8(void)
{
    exit_shim(EndingScrollBack8());
}

/*
 * @routine 010E:0EF EndingScrollSlots
 * @run 6B:88EF
 * WHAT IT DOES: Moves the scroll slots 3 and 4 on: 4 added to the low X of slot 3, 8 to the low X of slot 4.
 * HOW: Two adds.
 * WAITS: no.
 * Entry: nothing. Exit: A the second sum, N and Z from it, C and V from its add; X and Y kept.
 */
Exit EndingScrollSlots(void)
{
    Exit exit = { 0 };
    uint8_t value = RD8(SCROLL->x_lo[3]);

    WR8(SCROLL->x_lo[3], (uint8_t)(value + 4));
    value = RD8(SCROLL->x_lo[4]);
    exit.a = (uint8_t)(value + 8);
    WR8(SCROLL->x_lo[4], exit.a);
    exit.nz = exit.a;
    exit.c = (unsigned)value + 8 > 0xFF;
    exit.v = ((value ^ exit.a) & (8 ^ exit.a) & 0x80) != 0;
    exit.v_set = true;
    exit.x_kept = exit.y_kept = true;
    return exit;
}

void abi_EndingScrollSlots(void)
{
    exit_shim(EndingScrollSlots());
}

/*
 * @routine 010E:1F6 EndingFrameList
 * @run 6B:89F6
 * WHAT IT DOES: Steps an object's animation along the list the ending's table names: the list's address is
 * read from the table at $8A05 by A, and StepObjectFrames runs on it.
 * HOW: A doubled indexes the table; its two bytes go to zTemp10 and zTemp11; StepObjectFrames as a tail jump.
 * WAITS: no.
 * Entry: A the list, X the object. Exit: as StepObjectFrames; X kept.
 */
ObjExit EndingFrameList(uint8_t list, uint8_t object)
{
    ObjExit result = { 0 };
    uint8_t index, value, y;
    FrameStep step;

    index = (uint8_t)(list << 1);
    value = rd8((uint16_t)(ENDING_LISTS + index));
    WR8(ZP->temp10, value);
    value = rd8((uint16_t)(ENDING_LISTS + 1 + index));
    WR8(ZP->temp11, value);
    y = index;
    step = StepObjectFrames(object, &y);
    result.exit.a = (step == FRAME_LIST_ENDED) ? 0x00 : 0xFF;
    result.exit.nz = result.exit.a;
    result.exit.y = y;
    result.exit.c = step == FRAME_CHANGED;
    result.exit.x_kept = true;
    return result;
}

void abi_EndingFrameList(void)
{
    object_exit_shim(EndingFrameList(cpu.a, cpu.x));
}

/*
 * @routine 010D:2DC EndingClearAfterScrollX
 * @run 6B:82DC
 * WHAT IT DOES: An object handler: counts the first scroll's X high byte up and clears the object.
 * HOW: One increment; ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X the object. Exit: N and Z from the new byte; A, X, Y and C kept.
 */
ObjExit EndingClearAfterScrollX(uint8_t object)
{
    ObjExit result = object_exit(0, INC8(SCROLL->x_hi[0]));

    result.exit.a_kept = true;
    ClearObject(object);
    return result;
}

void abi_EndingClearAfterScrollX(void)
{
    object_exit_shim(EndingClearAfterScrollX(cpu.x));
}

/*
 * @routine 010D:2E2 EndingClearAfterScrollY
 * @run 6B:82E2
 * WHAT IT DOES: An object handler: counts the first scroll's Y high byte up and clears the object.
 * HOW: One increment; ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X the object. Exit: N and Z from the new byte; A, X, Y and C kept.
 */
ObjExit EndingClearAfterScrollY(uint8_t object)
{
    ObjExit result = object_exit(0, INC8(SCROLL->y_hi[0]));

    result.exit.a_kept = true;
    ClearObject(object);
    return result;
}

void abi_EndingClearAfterScrollY(void)
{
    object_exit_shim(EndingClearAfterScrollY(cpu.x));
}

/*
 * @routine 010D:405 EndingScrollRunUp
 * @run 6B:8405
 * WHAT IT DOES: An object handler: in its first state counts the first scroll's X low byte up each frame until
 * it reaches $80, then moves the object to its next state.
 * HOW: OBJ->state; one increment of SCROLL->x_lo[0] and a compare.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingScrollRunUp(uint8_t object)
{
    ObjExit result;
    uint8_t state, value;

    state = RD8(OBJ->state[object]);
    if (state != 0) {
        return object_exit(state, state);
    }
    INC8(SCROLL->x_lo[0]);
    value = RD8(SCROLL->x_lo[0]);
    result = with_carry(object_exit(value, (uint8_t)(value - 0x80)), value >= 0x80);
    if (value != 0x80) {
        return result;
    }
    result.exit.nz = INC8(OBJ->state[object]);
    return result;
}

void abi_EndingScrollRunUp(void)
{
    object_exit_shim(EndingScrollRunUp(cpu.x));
}

/*
 * @routine 010D:3CE EndingScrollPull
 * @run 6B:83CE
 * WHAT IT DOES: An object handler of two states: the first sets the first scroll's X low byte to $80; the second
 * takes one off the X of objects 1, 2 and 3 a frame (with the borrow into the high bytes of objects 1 and 3)
 * and counts the first scroll's X low byte down, moving on when it reaches 0.
 * HOW: OBJ->state; decrements with a test for $FF.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingScrollPull(uint8_t object)
{
    ObjExit result;
    uint8_t state, value;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(SCROLL->x_lo[0], 0x80);
        return object_exit(0x80, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return object_exit(state, state);
    }
    DEC8(OBJ->x_lo[1]);
    value = RD8(OBJ->x_lo[1]);
    if (value == 0xFF) {
        DEC8(OBJ->x_hi[1]);
    }
    DEC8(OBJ->x_lo[2]);
    DEC8(OBJ->x_lo[3]);
    value = RD8(OBJ->x_lo[3]);
    if (value == 0xFF) {
        DEC8(OBJ->x_hi[3]);
    }
    result = with_carry(object_exit(value, 0), value == 0xFF);
    result.exit.nz = DEC8(SCROLL->x_lo[0]);
    if (result.exit.nz == 0) {
        result.exit.nz = INC8(OBJ->state[object]);
    }
    return result;
}

void abi_EndingScrollPull(void)
{
    object_exit_shim(EndingScrollPull(cpu.x));
}

/* The animation handlers of two stages: the first stage shows picture kind 3 and moves on; the second waits
 * (A the state less 1 and 2 as each test leaves it); the third plays a list of the ending's table until it ends. */
static ObjExit play_list_once(uint8_t object, uint8_t list_number, uint16_t ret)
{
    ObjExit list;
    uint8_t state;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->sprite_set[object], 3);
        return object_exit(3, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        return object_exit(0, 0);
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return object_exit(state, state);
    }
    jsr_push(ret);
    list = EndingFrameList(list_number, object);
    jsr_pop();
    if (list.exit.a != 0) {
        return list;
    }
    list.exit.nz = INC8(OBJ->state[object]);
    return list;
}

/*
 * @routine 010D:418 EndingPlayList10
 * @run 6B:8418
 * WHAT IT DOES: An object handler: shows picture kind 3, waits a frame, then plays the ending's list $10 until it
 * ends and moves to the next state.
 * HOW: OBJ->state and OBJ->sprite_set; EndingFrameList.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingPlayList10(uint8_t object)
{
    return play_list_once(object, 0x10, 0x8432);
}

void abi_EndingPlayList10(void)
{
    object_exit_shim(EndingPlayList10(cpu.x));
}

/*
 * @routine 010D:4BE EndingPlayList0F
 * @run 6B:84BE
 * WHAT IT DOES: As EndingPlayList10, with the ending's list $0F.
 * HOW: OBJ->state and OBJ->sprite_set; EndingFrameList.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingPlayList0F(uint8_t object)
{
    return play_list_once(object, 0x0F, 0x84D8);
}

void abi_EndingPlayList0F(void)
{
    object_exit_shim(EndingPlayList0F(cpu.x));
}

/*
 * @routine 010D:50C EndingPlayList0EThenClear
 * @run 6B:850C
 * WHAT IT DOES: An object handler: shows picture kind 3, waits a frame, then plays the ending's list $0E until it
 * ends and clears the object.
 * HOW: OBJ->state and OBJ->sprite_set; EndingFrameList; ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingPlayList0EThenClear(uint8_t object)
{
    ObjExit list;
    uint8_t state;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->sprite_set[object], 3);
        return object_exit(3, INC8(OBJ->state[object]));
    }
    if (state == 1) {
        return object_exit(0, 0);
    }
    jsr_push(0x8523);
    list = EndingFrameList(0x0E, object);
    jsr_pop();
    if (list.exit.a != 0) {
        return list;
    }
    ClearObject(object);
    return list;
}

void abi_EndingPlayList0EThenClear(void)
{
    object_exit_shim(EndingPlayList0EThenClear(cpu.x));
}

/*
 * @routine 010D:4DE EndingRiseToY40
 * @run 6B:84DE
 * WHAT IT DOES: An object handler: shows picture kind 3, waits a frame, then shows frame $0D and moves the object up
 * a line each frame until its Y low byte is $40, then sets flash flags $8D and moves to the next state.
 * HOW: OBJ->state, OBJ->sprite_set, OBJ->frame, OBJ->y_lo and OBJ->flash_flags.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingRiseToY40(uint8_t object)
{
    ObjExit result;
    uint8_t state, low;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->sprite_set[object], 3);
        return object_exit(3, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        return object_exit(0, 0);
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return object_exit(state, state);
    }
    WR8(OBJ->frame[object], 0x0D);
    DEC8(OBJ->y_lo[object]);
    low = RD8(OBJ->y_lo[object]);
    result = with_carry(object_exit(low, (uint8_t)(low - 0x40)), low >= 0x40);
    if (low != 0x40) {
        return result;
    }
    WR8(OBJ->flash_flags[object], 0x8D);
    result.exit.a = 0x8D;
    result.exit.nz = INC8(OBJ->state[object]);
    return result;
}

void abi_EndingRiseToY40(void)
{
    object_exit_shim(EndingRiseToY40(cpu.x));
}

/* The scene's raster effect $13 and two pairs of scroll Y values, the first state of four handlers. */
static void raster_and_scroll(uint8_t first, uint8_t second)
{
    WR8(ZP->raster_effect, 0x13);
    WR8(SCROLL->y_lo[5], first);
    WR8(SCROLL->y_lo[1], first);
    WR8(SCROLL->y_lo[6], second);
    WR8(SCROLL->y_lo[2], second);
}

/* The same, the lower slot of each pair written first. */
static void raster_and_scroll_low_first(uint8_t first, uint8_t second)
{
    WR8(ZP->raster_effect, 0x13);
    WR8(SCROLL->y_lo[1], first);
    WR8(SCROLL->y_lo[5], first);
    WR8(SCROLL->y_lo[2], second);
    WR8(SCROLL->y_lo[6], second);
}

/*
 * @routine 010D:484 EndingScrollWindowA
 * @run 6B:8484
 * WHAT IT DOES: An object handler: in its first state starts the raster effect $13 and sets scroll slots 1 and 5 to Y
 * $1F and 2 and 6 to $BF, then moves to the next state.
 * HOW: OBJ->state; ZP->raster_effect; SCROLL->y_lo.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingScrollWindowA(uint8_t object)
{
    uint8_t state = RD8(OBJ->state[object]);

    if (state != 0) {
        return object_exit(state, state);
    }
    raster_and_scroll(0x1F, 0xBF);
    return object_exit(0xBF, INC8(OBJ->state[object]));
}

void abi_EndingScrollWindowA(void)
{
    object_exit_shim(EndingScrollWindowA(cpu.x));
}

/*
 * @routine 010D:4A1 EndingScrollWindowB
 * @run 6B:84A1
 * WHAT IT DOES: As EndingScrollWindowA with the Y values $27 and $C7.
 * HOW: OBJ->state; ZP->raster_effect; SCROLL->y_lo.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingScrollWindowB(uint8_t object)
{
    uint8_t state = RD8(OBJ->state[object]);

    if (state != 0) {
        return object_exit(state, state);
    }
    raster_and_scroll(0x27, 0xC7);
    return object_exit(0xC7, INC8(OBJ->state[object]));
}

void abi_EndingScrollWindowB(void)
{
    object_exit_shim(EndingScrollWindowB(cpu.x));
}

/*
 * @routine 010D:548 EndingScrollWindowHold
 * @run 6B:8548
 * WHAT IT DOES: An object handler: starts the raster effect $13, sets scroll slots 1 and 5 to Y $0F and 2 and 6 to $6F,
 * holds the object for $40 frames and moves to the next state.
 * HOW: ZP->raster_effect; SCROLL->y_lo; OBJ->life_timer.
 * WAITS: no.
 * Entry: X the object. Exit: A $40, N and Z from the new state; C and the registers kept.
 */
ObjExit EndingScrollWindowHold(uint8_t object)
{
    raster_and_scroll(0x0F, 0x6F);
    WR8(OBJ->life_timer[object], 0x40);
    return object_exit(0x40, INC8(OBJ->state[object]));
}

void abi_EndingScrollWindowHold(void)
{
    object_exit_shim(EndingScrollWindowHold(cpu.x));
}

/*
 * @routine 010D:438 EndingScrollWindowSteps
 * @run 6B:8438
 * WHAT IT DOES: An object handler of four states: the first starts the raster effect $13, sets scroll slots 1 and 5 to Y
 * $1F and 2 and 6 to $AF and holds the object $20 frames; the second waits the hold out; the third moves on; the
 * fourth moves scroll slot 1's X on by 4 a frame and, when it wraps to 0, moves the first object and scroll slot 1's
 * X high byte on and the object to its next state.
 * HOW: OBJ->state; EndingHoldTimer; SCROLL->x_lo and x_hi.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingScrollWindowSteps(uint8_t object)
{
    ObjExit result;
    uint8_t state, low, sum;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        raster_and_scroll(0x1F, 0xAF);
        WR8(OBJ->life_timer[object], 0x20);
        return object_exit(0x20, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        result = object_exit(0, 0);
        jsr_push(0x8460);
        result.exit.nz = EndingHoldTimer(object);
        if (result.exit.nz != 0) {
            return result;
        }
        jsr_pop();
        result.exit.nz = INC8(OBJ->state[object]);
        return result;
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        return object_exit(0, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return object_exit(state, state);
    }
    low = RD8(SCROLL->x_lo[1]);
    sum = (uint8_t)(low + 4);
    WR8(SCROLL->x_lo[1], sum);
    result = with_carry(object_exit(sum, sum), (unsigned)low + 4 > 0xFF);
    result.exit.v = ((low ^ sum) & (4 ^ sum) & 0x80) != 0;
    result.exit.v_set = true;
    if (sum != 0) {
        return result;
    }
    INC8(OBJ->state[0]);
    INC8(SCROLL->x_hi[1]);
    result.exit.nz = INC8(OBJ->state[object]);
    return result;
}

void abi_EndingScrollWindowSteps(void)
{
    object_exit_shim(EndingScrollWindowSteps(cpu.x));
}

/*
 * @routine 010D:565 EndingStartList3
 * @run 6B:8565
 * WHAT IT DOES: An object handler: once its hold has run out starts list 0 on channel 7 as kind 3, holds the object $20
 * frames and moves to the next state.
 * HOW: EndingHoldTimer (which returns to the dispatch while the hold lasts); StartListChannelKind3; OBJ->life_timer.
 * WAITS: no.
 * Entry: X the object, A as the dispatch left it. Exit: A $20, N and Z from the new state; C and the registers kept.
 * While the hold lasts: A kept, N and Z from the timer.
 */
ObjExit EndingStartList3(uint8_t object, uint8_t entry_a)
{
    ObjExit result = object_exit(entry_a, 0);

    jsr_push(0x8568);
    result.exit.nz = EndingHoldTimer(object);
    if (result.exit.nz != 0) {
        return result;
    }
    jsr_pop();
    push8(object);
    jsr_push(0x856F);
    (void)StartListChannelKind3(0, 7);
    jsr_pop();
    (void)pull8();
    WR8(OBJ->life_timer[object], 0x20);
    return object_exit(0x20, INC8(OBJ->state[object]));
}

void abi_EndingStartList3(void)
{
    object_exit_shim(EndingStartList3(cpu.x, cpu.a));
}

/*
 * @routine 010D:579 EndingStartScript18
 * @run 6B:8579
 * WHAT IT DOES: An object handler: once its hold has run out starts script $18 on slot 7 as kind 1 and moves object 2's frame,
 * object 3's X, scroll slot 2's X high byte and low byte, and the object's state on.
 * HOW: EndingHoldTimer; StartScriptSlotKind1; five counts.
 * WAITS: no.
 * Entry: X the object, A as the dispatch left it. Exit: A and Y $18, N and Z from the new state; C and X kept.
 * While the hold lasts: A kept, N and Z from the timer.
 */
ObjExit EndingStartScript18(uint8_t object, uint8_t entry_a)
{
    ObjExit result = object_exit(entry_a, 0);

    jsr_push(0x857C);
    result.exit.nz = EndingHoldTimer(object);
    if (result.exit.nz != 0) {
        return result;
    }
    jsr_pop();
    push8(object);
    jsr_push(0x8584);
    (void)StartScriptSlotKind1(0x18, 7);
    jsr_pop();
    (void)pull8();
    INC8(OBJ->frame[2]);
    INC8(OBJ->x_lo[3]);
    INC8(OBJ->x_lo[3]);
    INC8(SCROLL->x_hi[2]);
    DEC8(SCROLL->x_lo[2]);
    result = object_exit(0x18, INC8(OBJ->state[object]));
    result.exit.y = 0x18;
    result.exit.y_kept = false;
    return result;
}

void abi_EndingStartScript18(void)
{
    object_exit_shim(EndingStartScript18(cpu.x, cpu.a));
}

/*
 * @routine 010D:598 EndingScrollSlide
 * @run 6B:8598
 * WHAT IT DOES: An object handler: takes 4 off scroll slot 2's X low byte; while the result is $80 or more it is stored and
 * nothing else happens; below that it counts scroll slot 6's Y high byte up, holds the object $30 frames, counts object
 * 4's state, starts script $17 on slot 7 as kind 1, moves the object on and stores $80.
 * HOW: SCROLL->x_lo; StartScriptSlotKind1.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingScrollSlide(uint8_t object)
{
    ObjExit result;
    uint8_t low, diff;
    bool overflow;

    low = RD8(SCROLL->x_lo[2]);
    diff = (uint8_t)(low - 4);
    overflow = ((low ^ 4) & (low ^ diff) & 0x80) != 0;
    result = with_carry(object_exit(diff, (uint8_t)(diff - 0x80)), diff >= 0x80);
    result.exit.v = overflow;
    result.exit.v_set = true;
    if (diff >= 0x80) {
        WR8(SCROLL->x_lo[2], diff);
        return result;
    }
    INC8(SCROLL->y_hi[6]);
    WR8(OBJ->life_timer[object], 0x30);
    INC8(OBJ->state[4]);
    push8(object);
    jsr_push(0x85B5);
    (void)StartScriptSlotKind1(0x17, 7);
    jsr_pop();
    (void)pull8();
    result.exit.a = 0x80;
    result.exit.y = 0x17;
    result.exit.y_kept = false;
    result.exit.nz = INC8(OBJ->state[object]);
    WR8(SCROLL->x_lo[2], 0x80);
    return result;
}

void abi_EndingScrollSlide(void)
{
    object_exit_shim(EndingScrollSlide(cpu.x));
}

/*
 * @routine 010D:5BF EndingSplitScrolls
 * @run 6B:85BF
 * WHAT IT DOES: An object handler: once its hold has run out resets scroll slots 2 and 6 and moves slots 1 and 2's Y high bytes on,
 * sets slot 2's Y low byte to $6F, holds the object $20 frames and moves objects 1 and this one to their next states.
 * HOW: EndingHoldTimer; stores and counts on SCROLL.
 * WAITS: no.
 * Entry: X the object, A as the dispatch left it. Exit: A $20, N and Z from the new state; C and the registers kept.
 * While the hold lasts: A kept, N and Z from the timer.
 */
ObjExit EndingSplitScrolls(uint8_t object, uint8_t entry_a)
{
    ObjExit result = object_exit(entry_a, 0);

    jsr_push(0x85C2);
    result.exit.nz = EndingHoldTimer(object);
    if (result.exit.nz != 0) {
        return result;
    }
    jsr_pop();
    WR8(SCROLL->y_hi[6], 0);
    INC8(SCROLL->y_hi[1]);
    INC8(SCROLL->y_hi[2]);
    WR8(SCROLL->y_lo[2], 0x6F);
    WR8(SCROLL->x_hi[2], 0);
    WR8(SCROLL->x_lo[2], 0);
    WR8(OBJ->life_timer[object], 0x20);
    INC8(OBJ->state[1]);
    return object_exit(0x20, INC8(OBJ->state[object]));
}

void abi_EndingSplitScrolls(void)
{
    object_exit_shim(EndingSplitScrolls(cpu.x, cpu.a));
}

/*
 * @routine 010D:5E2 EndingScrollsOn
 * @run 6B:85E2
 * WHAT IT DOES: An object handler: once its hold has run out counts scroll slot 6's Y high byte and slot 2's X high byte up,
 * counts object 6's state and moves this object to its next state.
 * HOW: EndingHoldTimer; four counts.
 * WAITS: no.
 * Entry: X the object, A as the dispatch left it. Exit: A kept, N and Z from the new state; C and the registers kept.
 */
ObjExit EndingScrollsOn(uint8_t object, uint8_t entry_a)
{
    ObjExit result = object_exit(entry_a, 0);

    jsr_push(0x85E5);
    result.exit.nz = EndingHoldTimer(object);
    if (result.exit.nz != 0) {
        return result;
    }
    jsr_pop();
    INC8(SCROLL->y_hi[6]);
    INC8(SCROLL->x_hi[2]);
    INC8(OBJ->state[6]);
    result.exit.nz = INC8(OBJ->state[object]);
    return result;
}

void abi_EndingScrollsOn(void)
{
    object_exit_shim(EndingScrollsOn(cpu.x, cpu.a));
}

/* A move laid under an exit: its V (and its C where the exit keeps C) come out as the move left them. */
static ObjExit under_move(ObjExit result, AxisMove move)
{
    result.move = move;
    result.moved = true;
    return result;
}

/* The tail of three handlers that rise: after a move, the Y low byte is compared and the state moves on at the mark. */
static ObjExit rise_to(uint8_t object, AxisMove move, uint8_t mark)
{
    ObjExit result;
    uint8_t low = RD8(OBJ->y_lo[object]);

    result = with_carry(object_exit(low, (uint8_t)(low - mark)), low >= mark);
    result = under_move(result, move);
    if (low == mark) {
        result.exit.nz = INC8(OBJ->state[object]);
    }
    return result;
}

/* The first state of the rising handlers: the picture, a hold, and the speed's high byte counted down by 2. */
static void rise_start(uint8_t object)
{
    WR8(OBJ->sprite_set[object], 3);
}

/*
 * @routine 010D:720 EndingRiseFrame31
 * @run 6B:8720
 * WHAT IT DOES: An object handler of three stages: shows picture kind 3 and frame $31, slows its upward speed by 2 and holds
 * $B0 frames; then waits the hold out; then moves until its Y low byte is $48 and moves to the next state.
 * HOW: OBJ->state; EndingHoldTimer; ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingRiseFrame31(uint8_t object)
{
    ObjExit result;
    uint8_t state;
    AxisMove move;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        rise_start(object);
        WR8(OBJ->frame[object], 0x31);
        DEC8(OBJ->vy_hi[object]);
        DEC8(OBJ->vy_hi[object]);
        WR8(OBJ->life_timer[object], 0xB0);
        return object_exit(0xB0, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        result = object_exit(0, 0);
        jsr_push(0x8744);
        result.exit.nz = EndingHoldTimer(object);
        if (result.exit.nz != 0) {
            return result;
        }
        jsr_pop();
        result.exit.nz = INC8(OBJ->state[object]);
        return result;
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return object_exit(state, state);
    }
    jsr_push(0x874E);
    move = ApplyVelocity(object);
    jsr_pop();
    return rise_to(object, move, 0x48);
}

void abi_EndingRiseFrame31(void)
{
    object_exit_shim(EndingRiseFrame31(cpu.x));
}

/* The two handlers that start a fall: stage 0 sets speed and hold, stage 1 the hold and frame $32, stage 2 the move to a mark. */
static ObjExit rise_frame32(uint8_t object, uint8_t hold, uint8_t shift, uint16_t hold_ret, uint16_t move_ret)
{
    ObjExit result;
    uint8_t state, value, sum;
    AxisMove move;
    unsigned total;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        rise_start(object);
        WR8(OBJ->vy_lo[object], 0x60);
        DEC8(OBJ->vy_hi[object]);
        DEC8(OBJ->vy_hi[object]);
        WR8(OBJ->life_timer[object], hold);
        value = rd8((uint16_t)(0x345F + object));
        total = (unsigned)value + shift;
        sum = (uint8_t)total;
        wr8((uint16_t)(0x345F + object), sum);
        result = with_carry(object_exit(sum, 0), total > 0xFF);
        result.exit.v = ((value ^ sum) & (shift ^ sum) & 0x80) != 0;
        result.exit.v_set = true;
        result.exit.nz = INC8(OBJ->state[object]);
        return result;
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        result = object_exit(0, 0);
        jsr_push(hold_ret);
        result.exit.nz = EndingHoldTimer(object);
        if (result.exit.nz != 0) {
            return result;
        }
        jsr_pop();
        WR8(OBJ->frame[object], 0x32);
        return object_exit(0x32, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return object_exit(state, state);
    }
    jsr_push(move_ret);
    move = ApplyVelocity(object);
    jsr_pop();
    return rise_to(object, move, 0x60);
}

/*
 * @routine 010D:759 EndingRiseFrame32A
 * @run 6B:8759
 * WHAT IT DOES: An object handler of three stages: shows picture kind 3, sets its downward speed $60 and slows its upward speed
 * by 2, holds $90 frames and adds $10 to the byte at $345F plus its number; then waits the hold out and shows frame $32; then
 * moves until its Y low byte is $60 and moves to the next state.
 * HOW: OBJ->state; EndingHoldTimer; ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingRiseFrame32A(uint8_t object)
{
    return rise_frame32(object, 0x90, 0x10, 0x8786, 0x8795);
}

void abi_EndingRiseFrame32A(void)
{
    object_exit_shim(EndingRiseFrame32A(cpu.x));
}

/*
 * @routine 010D:7D6 EndingRiseFrame32B
 * @run 6B:87D6
 * WHAT IT DOES: As EndingRiseFrame32A, holding $A0 frames and adding $20.
 * HOW: OBJ->state; EndingHoldTimer; ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingRiseFrame32B(uint8_t object)
{
    return rise_frame32(object, 0xA0, 0x20, 0x8803, 0x8812);
}

void abi_EndingRiseFrame32B(void)
{
    object_exit_shim(EndingRiseFrame32B(cpu.x));
}

/*
 * @routine 010D:7A0 EndingRiseWithList
 * @run 6B:87A0
 * WHAT IT DOES: An object handler of three stages: shows picture kind 3, slows its upward speed by 1 and holds $80 frames; then
 * waits the hold out; then plays the ending's list 7 and moves until its Y low byte is $68 and moves to the next state.
 * HOW: OBJ->state; EndingHoldTimer; EndingFrameList; ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingRiseWithList(uint8_t object)
{
    ObjExit result, list;
    uint8_t state;
    AxisMove move;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        rise_start(object);
        DEC8(OBJ->vy_hi[object]);
        WR8(OBJ->life_timer[object], 0x80);
        return object_exit(0x80, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        result = object_exit(0, 0);
        jsr_push(0x87BC);
        result.exit.nz = EndingHoldTimer(object);
        if (result.exit.nz != 0) {
            return result;
        }
        jsr_pop();
        result.exit.nz = INC8(OBJ->state[object]);
        return result;
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return object_exit(state, state);
    }
    jsr_push(0x87C8);
    list = EndingFrameList(7, object);
    jsr_pop();
    jsr_push(0x87CB);
    move = ApplyVelocity(object);
    jsr_pop();
    result = rise_to(object, move, 0x68);
    result.exit.y = list.exit.y;
    result.exit.y_kept = false;
    return result;
}

void abi_EndingRiseWithList(void)
{
    object_exit_shim(EndingRiseWithList(cpu.x));
}

/*
 * @routine 010D:668 EndingScrollRight6
 * @run 6B:8668
 * WHAT IT DOES: An object handler of two stages: adds 6 to the first scroll's X low byte; in the first stage a carry counts
 * the first scroll's Y high byte up and moves to the next state; in the second the byte is counted up once more and a
 * carry moves the X high byte on, clears the low byte and moves to the next state.
 * HOW: OBJ->state; SCROLL->x_lo[0], x_hi[0] and y_hi[0].
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingScrollRight6(uint8_t object)
{
    ObjExit result;
    uint8_t state, low, sum;
    bool second = false;
    unsigned total;

    state = RD8(OBJ->state[object]);
    if (state != 0) {
        state = (uint8_t)(state - 1);
        if (state != 0) {
            return object_exit(state, state);
        }
        second = true;
    }
    low = RD8(SCROLL->x_lo[0]);
    total = (unsigned)low + 6;
    sum = (uint8_t)total;
    WR8(SCROLL->x_lo[0], sum);
    result = with_carry(object_exit(sum, sum), total > 0xFF);
    result.exit.v = ((low ^ sum) & (6 ^ sum) & 0x80) != 0;
    result.exit.v_set = true;
    if (second) {
        result.exit.nz = INC8(SCROLL->x_lo[0]);
    }
    if (total <= 0xFF) {
        return result;
    }
    if (!second) {
        INC8(SCROLL->y_hi[0]);
    } else {
        INC8(SCROLL->x_hi[0]);
        WR8(SCROLL->x_lo[0], 0);
    }
    result.exit.nz = INC8(OBJ->state[object]);
    return result;
}

void abi_EndingScrollRight6(void)
{
    object_exit_shim(EndingScrollRight6(cpu.x));
}

/*
 * @routine 010D:6E3 EndingScrollUp
 * @run 6B:86E3
 * WHAT IT DOES: An object handler of two stages: the first starts the raster effect $13 and sets scroll slots 1, 5 and 6 to Y
 * $0F, $2F and $BF, slot 1's Y high byte up by one and slot 2's X high byte up by one; the second counts slot 1's Y down (with
 * the borrow) until it is $2F and moves to the next state.
 * HOW: OBJ->state; ZP->raster_effect; SCROLL->y_lo and y_hi.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingScrollUp(uint8_t object)
{
    ObjExit result;
    uint8_t state, value;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(ZP->raster_effect, 0x13);
        WR8(SCROLL->y_lo[1], 0x0F);
        INC8(SCROLL->y_hi[1]);
        WR8(SCROLL->y_lo[5], 0x2F);
        WR8(SCROLL->y_lo[6], 0xBF);
        INC8(SCROLL->x_hi[2]);
        return object_exit(0xBF, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return object_exit(state, state);
    }
    DEC8(SCROLL->y_lo[1]);
    value = RD8(SCROLL->y_lo[1]);
    if (value == 0xFF) {
        DEC8(SCROLL->y_hi[1]);
    }
    value = RD8(SCROLL->y_lo[1]);
    result = with_carry(object_exit(value, (uint8_t)(value - 0x2F)), value >= 0x2F);
    if (value == 0x2F) {
        result.exit.nz = INC8(OBJ->state[object]);
    }
    return result;
}

void abi_EndingScrollUp(void)
{
    object_exit_shim(EndingScrollUp(cpu.x));
}

/*
 * @routine 010D:69A EndingScrollDown
 * @run 6B:869A
 * WHAT IT DOES: An object handler of three stages: the first starts the raster effect $13, sets scroll slots 5 and 6 to Y $1F and
 * $BF, slot 1 to $00, and counts slot 0's Y high byte and slot 2's X high byte up; the second, on odd frames, counts slot 1's Y
 * up and the Y low bytes of the two objects after this one down until slot 1's Y is $1F, then clears the frame of the object
 * two on and moves to the next state; the third plays the ending's list $0B on the next object.
 * HOW: OBJ->state; ZP->frame_tick; SCROLL; EndingFrameList.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingScrollDown(uint8_t object)
{
    ObjExit result, list;
    uint8_t state, tick, value;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(ZP->raster_effect, 0x13);
        WR8(SCROLL->y_lo[5], 0x1F);
        WR8(SCROLL->y_lo[1], 0x00);
        INC8(SCROLL->y_hi[0]);
        WR8(SCROLL->y_lo[6], 0xBF);
        INC8(SCROLL->x_hi[2]);
        return object_exit(0xBF, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        tick = RD8(ZP->frame_tick);
        result = with_carry(object_exit((uint8_t)(tick >> 1), (uint8_t)(tick >> 1)), (tick & 1) != 0);
        if ((tick & 1) == 0) {
            return result;
        }
        INC8(SCROLL->y_lo[1]);
        DEC8(OBJ->y_lo[object + 1]);
        DEC8(OBJ->y_lo[object + 2]);
        value = RD8(SCROLL->y_lo[1]);
        result = with_carry(object_exit(value, (uint8_t)(value - 0x1F)), value >= 0x1F);
        if (value == 0x1F) {
            result.exit.nz = INC8(OBJ->state[object]);
            WR8(OBJ->frame[object + 2], 0);
        }
        return result;
    }
    jsr_push(0x86E1);
    list = EndingFrameList(0x0B, (uint8_t)(object + 1));
    jsr_pop();
    list.exit.x_kept = true;
    list.exit.nz = object;
    return list;
}

void abi_EndingScrollDown(void)
{
    object_exit_shim(EndingScrollDown(cpu.x));
}

/*
 * @routine 010D:5FB EndingScrollSteps
 * @run 6B:85FB
 * WHAT IT DOES: An object handler of five stages: the first counts the first scroll's X and Y high bytes up; the next three each
 * take 8 off the first scroll's X low byte until it is $F8, then clear its X high byte, start the ending's list $0D on this channel
 * and count its Y high byte up; the fifth adds $10 to object 1's X (with the carry into its high byte) and takes 8 off the scroll's
 * X low byte until it is 0.
 * HOW: OBJ->state; EndingScrollBack8; StartListChannelKind2.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingScrollSteps(uint8_t object)
{
    ObjExit result;
    Exit back;
    uint8_t state, value;
    unsigned total;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        INC8(SCROLL->x_hi[0]);
        INC8(SCROLL->y_hi[0]);
        return object_exit(0, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state == 0 || state == 1 || state == 2) {
        static const uint16_t RETS[] = { 0x8610, 0x8621, 0x8639 };

        jsr_push(RETS[state]);
        back = EndingScrollBack8();
        jsr_pop();
        value = back.a;
        result = with_carry(object_exit(value, (uint8_t)(value - 0xF8)), value >= 0xF8);
        result.exit.v = back.v;
        result.exit.v_set = true;
        if (value != 0xF8) {
            return result;
        }
        if (state == 0) {
            WR8(SCROLL->x_hi[0], 0);
        } else if (state == 1) {
            WR8(SCROLL->y_hi[0], 0);
            push8(object);
            jsr_push(0x862E);
            (void)StartListChannelKind2(0x0D, object);
            jsr_pop();
            (void)pull8();
            result.exit.a = 2;
        } else {
            INC8(SCROLL->y_hi[0]);
        }
        result.exit.nz = INC8(OBJ->state[object]);
        return result;
    }
    if (state != 3) {
        return object_exit(state - 3, (uint8_t)(state - 3));
    }
    value = RD8(OBJ->x_lo[1]);
    total = (unsigned)value + 0x10;
    WR8(OBJ->x_lo[1], (uint8_t)total);
    if (total > 0xFF) {
        INC8(OBJ->x_hi[1]);
    }
    jsr_push(0x8658);
    back = EndingScrollBack8();
    jsr_pop();
    result = object_exit(back.a, back.nz);
    result.exit.c = back.c;
    result.exit.c_kept = false;
    result.exit.v = back.v;
    result.exit.v_set = true;
    if (back.a == 0) {
        result.exit.nz = INC8(OBJ->state[object]);
    }
    return result;
}

void abi_EndingScrollSteps(void)
{
    object_exit_shim(EndingScrollSteps(cpu.x));
}

/* The position's high byte after a move: what the move leaves in A. */
static uint8_t move_high(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;

    return (uint8_t)((move.before + added) >> 16);
}

/* A move that ends a handler: A, N, Z, C and V all from it. */
static ObjExit move_exit(AxisMove move)
{
    uint8_t high = move_high(move);
    ObjExit result = object_exit(high, high);

    return under_move(result, move);
}

/*
 * @routine 010E:01D EndingHoldThenScript19
 * @run 6B:881D
 * WHAT IT DOES: An object handler of two stages: the first starts the raster effect $13, sets scroll slots 1 and 5 to Y $2F and
 * 2 and 6 to $BF and holds the object $40 frames; the second, once the hold has run out, starts script $19 on slot 7 as kind 1
 * and moves to the next state.
 * HOW: OBJ->state; EndingHoldTimer; StartScriptSlotKind1.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingHoldThenScript19(uint8_t object)
{
    ObjExit result;
    uint8_t state;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        raster_and_scroll_low_first(0x2F, 0xBF);
        WR8(OBJ->life_timer[object], 0x40);
        return object_exit(0x40, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return object_exit(state, state);
    }
    result = object_exit(0, 0);
    jsr_push(0x8845);
    result.exit.nz = EndingHoldTimer(object);
    if (result.exit.nz != 0) {
        return result;
    }
    jsr_pop();
    push8(object);
    jsr_push(0x884D);
    (void)StartScriptSlotKind1(0x19, 7);
    jsr_pop();
    (void)pull8();
    result = object_exit(0x19, INC8(OBJ->state[object]));
    result.exit.y = 0x19;
    result.exit.y_kept = false;
    return result;
}

void abi_EndingHoldThenScript19(void)
{
    object_exit_shim(EndingHoldThenScript19(cpu.x));
}

/* The two link animations: play a list and, when a frame changed to a non-zero picture, start script $0D on slot 1. */
static ObjExit link_animation(uint8_t object, uint8_t list_number)
{
    ObjExit result;
    uint8_t state, frame;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->sprite_set[object], 3);
        INC8(OBJ->state[object]);
    }
    jsr_push(0x8865);
    result = EndingFrameList(list_number, object);
    jsr_pop();
    if (!result.exit.c) {
        return result;
    }
    frame = RD8(OBJ->frame[object]);
    result.exit.a = frame;
    result.exit.nz = frame;
    if (frame == 0) {
        return result;
    }
    push8(object);
    jsr_push(0x8874);
    (void)StartScriptSlotKind1(0x0D, 1);
    jsr_pop();
    (void)pull8();
    result.exit.a = 0x0D;
    result.exit.y = 0x0D;
    result.exit.nz = object;
    return result;
}

/*
 * @routine 010E:053 EndingLinkAnimationA
 * @run 6B:8853
 * WHAT IT DOES: An object handler: shows picture kind 3 and plays the ending's list 4; when a frame changes to a non-zero picture
 * it starts script $0D on slot 1 as kind 1.
 * HOW: OBJ->state and OBJ->sprite_set; EndingFrameList; StartScriptSlotKind1.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingLinkAnimationA(uint8_t object)
{
    return link_animation(object, 4);
}

void abi_EndingLinkAnimationA(void)
{
    object_exit_shim(EndingLinkAnimationA(cpu.x));
}

/*
 * @routine 010E:076 EndingLinkAnimationB
 * @run 6B:8876
 * WHAT IT DOES: As EndingLinkAnimationA, with the ending's list 5.
 * HOW: OBJ->state and OBJ->sprite_set; EndingFrameList; StartScriptSlotKind1.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingLinkAnimationB(uint8_t object)
{
    return link_animation(object, 5);
}

void abi_EndingLinkAnimationB(void)
{
    object_exit_shim(EndingLinkAnimationB(cpu.x));
}

/*
 * @routine 010D:36A EndingAimAtPlace
 * @run 6B:836A
 * WHAT IT DOES: An object handler of two stages: the first makes the object's speed the distance to the place (X $70, Y $58)
 * times 8 and moves to the next state; the second adds $18 to the downward speed and moves.
 * HOW: OBJ->state; subtracts and three shifts of the 16-bit speeds; ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingAimAtPlace(uint8_t object)
{
    ObjExit result;
    uint8_t state, low, high, vyh, borrow, round, value, carry_low, carry_high = 0;
    unsigned total;
    AxisMove move;
    bool overflow;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        low = RD8(OBJ->x_lo[object]);
        borrow = low < 0x70;
        WR8(OBJ->vx_lo[object], (uint8_t)(low - 0x70));
        high = RD8(OBJ->x_hi[object]);
        WR8(OBJ->vx_hi[object], (uint8_t)(high - borrow));
        low = RD8(OBJ->y_lo[object]);
        borrow = low < 0x58;
        WR8(OBJ->vy_lo[object], (uint8_t)(low - 0x58));
        high = RD8(OBJ->y_hi[object]);
        vyh = (uint8_t)(high - borrow);
        overflow = (high & (high ^ vyh) & 0x80) != 0;
        WR8(OBJ->vy_hi[object], vyh);
        for (round = 0; round < 3; round++) {
            value = RD8(OBJ->vx_lo[object]);
            carry_low = value >> 7;
            WR8(OBJ->vx_lo[object], (uint8_t)(value << 1));
            value = RD8(OBJ->vx_hi[object]);
            WR8(OBJ->vx_hi[object], (uint8_t)((value << 1) | carry_low));
            value = RD8(OBJ->vy_lo[object]);
            carry_low = value >> 7;
            WR8(OBJ->vy_lo[object], (uint8_t)(value << 1));
            value = RD8(OBJ->vy_hi[object]);
            carry_high = value >> 7;
            WR8(OBJ->vy_hi[object], (uint8_t)((value << 1) | carry_low));
        }
        result = with_carry(object_exit(vyh, INC8(OBJ->state[object])), carry_high != 0);
        result.exit.v = overflow;
        result.exit.v_set = true;
        return result;
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return object_exit(state, state);
    }
    low = RD8(OBJ->vy_lo[object]);
    total = (unsigned)low + 0x18;
    WR8(OBJ->vy_lo[object], (uint8_t)total);
    if (total > 0xFF) {
        INC8(OBJ->vy_hi[object]);
    }
    jsr_push(0x83CD);
    move = ApplyVelocity(object);
    jsr_pop();
    return move_exit(move);
}

void abi_EndingAimAtPlace(void)
{
    object_exit_shim(EndingAimAtPlace(cpu.x));
}

/* A plain Exit as a handler's result. */
static ObjExit through_exit(Exit exit)
{
    ObjExit result = { 0 };

    result.exit = exit;
    return result;
}

/*
 * @routine 010E:087 EndingScrollSqueeze
 * @run 6B:8887
 * WHAT IT DOES: An object handler of four stages: the first starts the raster effect $14 and sets the scroll's Y slots (1 and 5 to
 * $2F, 2 and 6 to $BF, 0 to $5F, 4 to $A7) and holds the object $40 frames; the second waits the hold out; the third, on odd frames,
 * moves slots 5 and 6 towards each other until slot 5 is $78; every stage but the first moves scroll slots 3 and 4 on.
 * HOW: OBJ->state; SCROLL->y_lo; EndingScrollSlots.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingScrollSqueeze(uint8_t object)
{
    ObjExit result;
    Exit slots;
    uint8_t state, value, left;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(ZP->raster_effect, 0x14);
        INC8(SCROLL->y_hi[1]);
        WR8(SCROLL->y_lo[1], 0x2F);
        WR8(SCROLL->y_lo[5], 0x2F);
        WR8(SCROLL->y_lo[2], 0xBF);
        WR8(SCROLL->y_lo[6], 0xBF);
        WR8(SCROLL->y_lo[0], 0x5F);
        WR8(SCROLL->y_lo[4], 0xA7);
        WR8(OBJ->life_timer[object], 0x40);
        return object_exit(0x40, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        left = DEC8(OBJ->life_timer[object]);
        result = object_exit(0, left);
        if (left == 0) {
            result.exit.nz = INC8(OBJ->state[object]);
        }
        return result;
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        slots = EndingScrollSlots();               /* a jump, not a call: no frame */
        return through_exit(slots);
    }
    jsr_push(0x88C8);
    slots = EndingScrollSlots();
    jsr_pop();
    result = object_exit(slots.a, slots.nz);
    result.exit.c = slots.c;
    result.exit.c_kept = false;
    result.exit.v = slots.v;
    result.exit.v_set = true;
    if ((RD8(ZP->frame_tick) & 1) == 0) {
        return result;
    }
    INC8(SCROLL->y_lo[5]);
    DEC8(SCROLL->y_lo[6]);
    value = RD8(SCROLL->y_lo[5]);
    if (value == 0x78) {
        INC8(OBJ->state[object]);
    }
    value = RD8(SCROLL->y_lo[5]);
    WR8(SCROLL->y_lo[1], value);
    value = RD8(SCROLL->y_lo[6]);
    result = with_carry(object_exit(value, (uint8_t)(value - 0xA7)), value >= 0xA7);
    result.exit.v = slots.v;
    result.exit.v_set = true;
    if (value >= 0xA7) {
        WR8(SCROLL->y_lo[4], value);
    }
    return result;
}

void abi_EndingScrollSqueeze(void)
{
    object_exit_shim(EndingScrollSqueeze(cpu.x));
}

/*
 * @routine 010E:19C EndingWaitForList
 * @run 6B:899C
 * WHAT IT DOES: An object handler: in its first state plays the ending's list its next kind names, and when the list has ended
 * and the kind is not negative moves to the next state.
 * HOW: OBJ->state and OBJ->next_kind; EndingFrameList.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingWaitForList(uint8_t object)
{
    ObjExit list;
    uint8_t state, kind;

    state = RD8(OBJ->state[object]);
    if (state != 0) {
        return object_exit(state, state);
    }
    kind = RD8(OBJ->next_kind[object]);
    jsr_push(0x89A7);
    list = EndingFrameList(kind, object);
    jsr_pop();
    if (list.exit.a != 0) {
        return list;
    }
    kind = RD8(OBJ->next_kind[object]);
    list.exit.a = kind;
    list.exit.nz = kind;
    if ((kind & 0x80) != 0) {
        return list;
    }
    list.exit.nz = INC8(OBJ->state[object]);
    return list;
}

void abi_EndingWaitForList(void)
{
    object_exit_shim(EndingWaitForList(cpu.x));
}

/*
 * @routine 010E:1B2 EndingFlyOff
 * @run 6B:89B2
 * WHAT IT DOES: An object handler of two stages: the first takes its speed from four tables at $89E2 by its next kind; the second
 * moves the object until its hold has run out.
 * HOW: OBJ->state and OBJ->next_kind; ApplyVelocity; OBJ->life_timer.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingFlyOff(uint8_t object)
{
    ObjExit result;
    uint8_t state, kind, left;
    AxisMove move;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        kind = RD8(OBJ->next_kind[object]);
        WR8(OBJ->vx_lo[object], rd8((uint16_t)(0x89E2 + kind)));
        WR8(OBJ->vx_hi[object], rd8((uint16_t)(0x89E7 + kind)));
        WR8(OBJ->vy_lo[object], rd8((uint16_t)(0x89EC + kind)));
        state = rd8((uint16_t)(0x89F1 + kind));
        WR8(OBJ->vy_hi[object], state);
        result = object_exit(state, INC8(OBJ->state[object]));
        result.exit.y = kind;
        result.exit.y_kept = false;
        return result;
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return object_exit(state, state);
    }
    jsr_push(0x89DC);
    move = ApplyVelocity(object);
    jsr_pop();
    left = DEC8(OBJ->life_timer[object]);
    result = object_exit(0, left);
    result.exit.a_kept = true;
    result = under_move(result, move);
    if (left == 0) {
        result.exit.nz = INC8(OBJ->state[object]);
    }
    return result;
}

void abi_EndingFlyOff(void)
{
    object_exit_shim(EndingFlyOff(cpu.x));
}

/*
 * @routine 010E:102 EndingDropAndBounce
 * @run 6B:8902
 * WHAT IT DOES: An object handler of three stages: the first starts the raster effect $13, sets the scroll's Y slots (1 and 5 to
 * $2F, 2 and 6 to $AF), picture kind 3, frame 1 and X speed $C0; the second adds $60 to the downward speed each frame and moves
 * the object until its Y is $60 or more, then fixes it at $60 (Y and X); the third adds $10 to the hold byte each frame and bounces
 * the object by the sine of it ($07FF scaled) while the byte is not negative, shifting scroll slots 1 and 2 against it;
 * when it goes negative it fixes the object at Y $60, the slots at $2F and $AF, and moves to the next state.
 * HOW: OBJ->state; ApplyVelocity; ScaleBySine with zTemp00 and zTemp01 set to $07FF; ZP->product.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingDropAndBounce(uint8_t object)
{
    ObjExit result;
    uint8_t state, low, high, sum, timer, product, value;
    unsigned total;
    AxisMove move;
    bool carry;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(ZP->raster_effect, 0x13);
        WR8(ZP->raster_arg, 0);
        WR8(SCROLL->y_lo[1], 0x2F);
        WR8(SCROLL->y_lo[5], 0x2F);
        WR8(SCROLL->y_lo[2], 0xAF);
        WR8(SCROLL->y_lo[6], 0xAF);
        WR8(OBJ->sprite_set[object], 3);
        WR8(OBJ->frame[object], 1);
        WR8(OBJ->vx_lo[object], 0xC0);
        return object_exit(0xC0, INC8(OBJ->state[object]));
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        low = RD8(OBJ->vy_lo[object]);
        total = (unsigned)low + 0x60;
        WR8(OBJ->vy_lo[object], (uint8_t)total);
        high = RD8(OBJ->vy_hi[object]);
        WR8(OBJ->vy_hi[object], (uint8_t)(high + (total > 0xFF ? 1 : 0)));
        jsr_push(0x8946);
        move = ApplyVelocity(object);
        jsr_pop();
        low = RD8(OBJ->y_lo[object]);
        result = with_carry(object_exit(low, (uint8_t)(low - 0x60)), low >= 0x60);
        result = under_move(result, move);
        if (low < 0x60) {
            return result;
        }
        WR8(OBJ->y_lo[object], 0x60);
        WR8(OBJ->x_lo[object], 0x60);
        result.exit.a = 0x60;
        result.exit.nz = INC8(OBJ->state[object]);
        return result;
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        return object_exit(state, state);
    }
    WR8(ZP->temp00, 0xFF);
    WR8(ZP->temp01, 0x07);
    value = RD8(OBJ->life_timer[object]);
    total = (unsigned)value + 0x10;
    timer = (uint8_t)total;
    carry = total > 0xFF;
    WR8(OBJ->life_timer[object], timer);
    if ((timer & 0x80) != 0) {
        WR8(OBJ->y_lo[object], 0x60);
        INC8(OBJ->state[object]);
        WR8(SCROLL->y_lo[1], 0x2F);
        WR8(SCROLL->y_lo[2], 0xAF);
        result = with_carry(object_exit(0xAF, 0xAF), carry);
        result.exit.v = ((value ^ timer) & (0x10 ^ timer) & 0x80) != 0;
        result.exit.v_set = true;
        return result;
    }
    jsr_push(0x8970);
    (void)ScaleBySine(timer);
    jsr_pop();
    product = RD8(ZP->product.hi);
    WR8(OBJ->y_lo[object], (uint8_t)(product + 0x60));
    product = RD8(ZP->product.hi);
    WR8(SCROLL->y_lo[1], (uint8_t)(0x2F - product));
    product = RD8(ZP->product.hi);
    sum = (uint8_t)(0xAF - product);
    WR8(SCROLL->y_lo[2], sum);
    result = with_carry(object_exit(sum, sum), 0xAF >= product);
    result.exit.v = ((0xAF ^ product) & (0xAF ^ sum) & 0x80) != 0;
    result.exit.v_set = true;
    result.exit.y = 0;
    result.exit.y_kept = false;
    return result;
}

void abi_EndingDropAndBounce(void)
{
    object_exit_shim(EndingDropAndBounce(cpu.x));
}

/*
 * @routine 010D:239 EndingCreditScrollStart
 * @run 6B:8239
 * WHAT IT DOES: An object handler: every frame starts list $18 plus the frame count's two low bits on channel 7 as kind 2. In its first
 * state it also starts script $1A on slot 0, holds the object 6 frames, moves the first scroll's X high byte and Y high byte and
 * Y low byte, and moves on. In the next states, until state $0A, once the hold has run out it takes the first scroll's X and Y high
 * bytes from a table at $8284 by state, holds 6 frames and moves on.
 * HOW: OBJ->state; StartListChannelKind2; StartScriptSlot0Kind1; EndingHoldTimer; SCROLL.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingCreditScrollStart(uint8_t object)
{
    ObjExit result;
    uint8_t state, tick, left;

    push8(object);
    tick = RD8(ZP->frame_tick);
    jsr_push(0x8246);
    (void)StartListChannelKind2((uint8_t)(0x18 + (tick & 3)), 7);
    jsr_pop();
    (void)pull8();
    state = RD8(OBJ->state[object]);
    if (state == 0) {
        push8(object);
        jsr_push(0x8252);
        (void)StartScriptSlot0Kind1(0x1A);
        jsr_pop();
        (void)pull8();
        WR8(OBJ->life_timer[object], 6);
        INC8(SCROLL->x_hi[0]);
        INC8(SCROLL->y_hi[0]);
        DEC8(SCROLL->y_lo[0]);
        result = with_carry(object_exit(6, INC8(OBJ->state[object])), false);
        result.exit.y = 0x1A;
        result.exit.y_kept = false;
        result.exit.v = false;
        result.exit.v_set = true;
        return result;
    }
    result = with_carry(object_exit(state, (uint8_t)(state - 0x0A)), state >= 0x0A);
    result.exit.v = false;                      /* the add of the list number: no overflow */
    result.exit.v_set = true;
    if (state == 0x0A) {
        return result;
    }
    jsr_push(0x826C);
    left = EndingHoldTimer(object);
    if (left != 0) {
        result.exit.nz = left;
        return result;
    }
    jsr_pop();
    state = RD8(OBJ->state[object]);
    WR8(SCROLL->x_hi[0], rd8((uint16_t)(0x8284 + (uint8_t)(state << 1))));
    WR8(SCROLL->y_hi[0], rd8((uint16_t)(0x8285 + (uint8_t)(state << 1))));
    WR8(OBJ->life_timer[object], 6);
    result = with_carry(object_exit(6, INC8(OBJ->state[object])), (state & 0x80) != 0);
    result.exit.y = (uint8_t)(state << 1);
    result.exit.y_kept = false;
    result.exit.v = false;
    result.exit.v_set = true;
    return result;
}

void abi_EndingCreditScrollStart(void)
{
    object_exit_shim(EndingCreditScrollStart(cpu.x));
}

/*
 * @routine 010D:298 EndingFallingPiece
 * @run 6B:8298
 * WHAT IT DOES: An object handler of two stages: the first takes its Y speed from a table at $82D5 by its number, shows picture kind 3
 * and frame $72 plus its number and moves on; after that each frame it sets flash flags from a table at $82DA by bit 2 of its number
 * plus the frame count, moves, and clears the object's Y high byte to 1 once it is negative with a non-negative Y low byte.
 * HOW: OBJ->state; OBJ->flash_flags; ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingFallingPiece(uint8_t object)
{
    ObjExit result;
    uint8_t state, sum, tick, index, high, low;
    unsigned total;
    AxisMove move;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->vy_hi[object], rd8((uint16_t)(0x82D5 + object)));
        WR8(OBJ->sprite_set[object], 3);
        total = (unsigned)object + 0x72;
        sum = (uint8_t)total;
        WR8(OBJ->frame[object], sum);
        result = with_carry(object_exit(sum, INC8(OBJ->state[object])), total > 0xFF);
        result.exit.v = ((object ^ sum) & (0x72 ^ sum) & 0x80) != 0;
        result.exit.v_set = true;
        return result;
    }
    tick = RD8(ZP->frame_tick);
    index = (uint8_t)((((uint8_t)(object + tick)) >> 2) & 1);
    WR8(OBJ->flash_flags[object], rd8((uint16_t)(0x82DA + index)));
    jsr_push(0x82C5);
    move = ApplyVelocity(object);
    jsr_pop();
    high = RD8(OBJ->y_hi[object]);
    result = under_move(object_exit(high, high), move);
    result.exit.y = index;
    result.exit.y_kept = false;
    if ((high & 0x80) == 0) {
        return result;
    }
    low = RD8(OBJ->y_lo[object]);
    result.exit.a = low;
    result.exit.nz = low;
    if ((low & 0x80) != 0) {
        return result;
    }
    WR8(OBJ->y_hi[object], 1);
    result.exit.a = 1;
    result.exit.nz = 1;
    return result;
}

void abi_EndingFallingPiece(void)
{
    object_exit_shim(EndingFallingPiece(cpu.x));
}

/*
 * @routine 010D:2E8 EndingPieceSpawn
 * @run 6B:82E8
 * WHAT IT DOES: An object handler of two stages: the first waits for object 1's X to be $0078 (high byte 0 and low $78) and moves on;
 * the second makes objects $10 to $2E: each cleared, shown as picture kind 3, kind $1A, frame $40 plus its number, X low and
 * Y low from tables at $831C and $833B; then clears object 1's frame and the object itself.
 * HOW: OBJ->state; ClearObject; two table reads a piece.
 * WAITS: no.
 * Entry: X the object. Exit: as the last test left them.
 */
ObjExit EndingPieceSpawn(uint8_t object)
{
    ObjExit result;
    uint8_t state, value, piece, table;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        value = RD8(OBJ->x_hi[1]);
        if (value != 0) {
            return object_exit(value, value);
        }
        value = RD8(OBJ->x_lo[1]);
        result = with_carry(object_exit(value, (uint8_t)(value - 0x78)), value >= 0x78);
        if (value == 0x78) {
            result.exit.nz = INC8(OBJ->state[object]);
        }
        return result;
    }
    push8(object);
    table = 0;
    for (piece = 0x10; piece != 0x2F; piece++) {
        jsr_push(0x8303);
        ClearObject(piece);
        jsr_pop();
        WR8(OBJ->sprite_set[piece], 3);
        WR8(OBJ->kind[piece], 0x1A);
        WR8(OBJ->frame[piece], (uint8_t)(piece + 0x40));
        WR8(OBJ->x_lo[piece], rd8((uint16_t)(0x831C + piece)));
        table = rd8((uint16_t)(0x833B + piece));
        WR8(OBJ->y_lo[piece], table);
    }
    (void)pull8();
    WR8(OBJ->frame[1], 0);
    result = with_carry(object_exit(table, object), true);
    result.exit.v = false;                      /* the last add of piece $2E and $40 */
    result.exit.v_set = true;
    ClearObject(object);
    return result;
}

void abi_EndingPieceSpawn(void)
{
    object_exit_shim(EndingPieceSpawn(cpu.x));
}
