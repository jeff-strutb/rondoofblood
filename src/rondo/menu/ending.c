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
