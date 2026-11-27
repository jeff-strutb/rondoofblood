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
#include "vram_update.h"

enum { ENDING_LISTS = 0x8A05 };

/* The arms the converted handlers end on (see EndingArm in front_end.h). */
enum { FP_START, FP_POSITIVE, FP_LOW, FP_SET };
enum { DB_START, DB_FALL, DB_LANDED, DB_OTHER, DB_DONE, DB_BOUNCE };
enum { CS_START, CS_AT_TEN, CS_DROPPED, CS_ADVANCED };
enum { SS_START, SS_BACK, SS_BACK_DONE, SS_OTHER, SS_FIFTH };
enum { SQ_START, SQ_WAIT, SQ_TAIL, SQ_ODD_WAIT, SQ_MOVED };

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

/* EndingHoldTimer's early exit pulled the two bytes of the caller's return address with PLA: A is the last one (its high
 * byte), N and Z from it, and the caller's frame is gone. */
static ObjExit hold_dropped(ObjExit result, uint16_t ret)
{
    result.exit.a = (uint8_t)(ret >> 8);
    result.exit.a_kept = false;
    result.exit.nz = result.exit.a;
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

/* The registers a handler is entered with from the kind dispatch: A the table's high byte, X the object, Y the index, N and Z from A, C the doubling's. */
static void cpu_entry_shim(uint8_t high, uint8_t object, uint8_t index, bool carry)
{
    cpu.a = high;
    cpu.x = object;
    cpu.y = index;
    abi_nz(high);
    abi_flag(P_C, carry);
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
 * Entry: X the object, the caller's frame on the stack. Exit: while the hold lasts A the high byte of the caller's return
 * address (the last PLA), N and Z from it; when it has run out N and Z from the timer (zero). X, Y and C kept.
 * not checked: it pulls its caller's frame and cannot run alone; its callers compare it.
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
        cpu.a = pull8();
        abi_nz(cpu.a);
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
 * not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
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
            return hold_dropped(result, 0x8460);
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
 * not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
 */
ObjExit EndingStartList3(uint8_t object, uint8_t entry_a)
{
    ObjExit result = object_exit(entry_a, 0);

    jsr_push(0x8568);
    result.exit.nz = EndingHoldTimer(object);
    if (result.exit.nz != 0) {
        return hold_dropped(result, 0x8568);
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
 * not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
 */
ObjExit EndingStartScript18(uint8_t object, uint8_t entry_a)
{
    ObjExit result = object_exit(entry_a, 0);

    jsr_push(0x857C);
    result.exit.nz = EndingHoldTimer(object);
    if (result.exit.nz != 0) {
        return hold_dropped(result, 0x857C);
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
 * not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
 */
ObjExit EndingSplitScrolls(uint8_t object, uint8_t entry_a)
{
    ObjExit result = object_exit(entry_a, 0);

    jsr_push(0x85C2);
    result.exit.nz = EndingHoldTimer(object);
    if (result.exit.nz != 0) {
        return hold_dropped(result, 0x85C2);
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
 * not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
 */
ObjExit EndingScrollsOn(uint8_t object, uint8_t entry_a)
{
    ObjExit result = object_exit(entry_a, 0);

    jsr_push(0x85E5);
    result.exit.nz = EndingHoldTimer(object);
    if (result.exit.nz != 0) {
        return hold_dropped(result, 0x85E5);
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
 * not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
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
            return hold_dropped(result, 0x8744);
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
            return hold_dropped(result, hold_ret);
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
 * not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
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
 * not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
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
 * not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
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
            return hold_dropped(result, 0x87BC);
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
EndingArm EndingScrollSteps(uint8_t object)
{
    EndingArm r = { 0 };
    uint8_t state, low, value;
    unsigned total;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        INC8(SCROLL->x_hi[0]);
        INC8(SCROLL->y_hi[0]);
        r.arm = SS_START;
        r.third = INC8(OBJ->state[object]);
        return r;
    }
    state = (uint8_t)(state - 1);
    if (state == 0 || state == 1 || state == 2) {
        static const uint16_t RETS[] = { 0x8610, 0x8621, 0x8639 };

        jsr_push(RETS[state]);
        r.exit = EndingScrollBack8();
        jsr_pop();
        r.first = r.exit.a;
        r.arm = SS_BACK;
        if (r.first != 0xF8) {
            return r;
        }
        r.arm = SS_BACK_DONE;
        r.index = state;
        if (state == 0) {
            WR8(SCROLL->x_hi[0], 0);
        } else if (state == 1) {
            WR8(SCROLL->y_hi[0], 0);
            push8(object);
            jsr_push(0x862E);
            (void)StartListChannelKind2(0x0D, object);
            jsr_pop();
            (void)pull8();
        } else {
            INC8(SCROLL->y_hi[0]);
        }
        r.third = INC8(OBJ->state[object]);
        return r;
    }
    if (state != 3) {
        r.arm = SS_OTHER;
        r.first = (uint8_t)(state - 3);
        return r;
    }
    value = RD8(OBJ->x_lo[1]);
    total = (unsigned)value + 0x10;
    WR8(OBJ->x_lo[1], (uint8_t)total);
    if (total > 0xFF) {
        INC8(OBJ->x_hi[1]);
    }
    jsr_push(0x8658);
    r.exit = EndingScrollBack8();
    jsr_pop();
    low = r.exit.a;
    r.first = low;
    r.arm = SS_FIFTH;
    if (low == 0) {
        r.third = INC8(OBJ->state[object]);
    }
    return r;
}

/* What EndingScrollSteps leaves in the registers and flags, from the arm it ended on. */
static ObjExit scroll_steps_exit_shim(EndingArm r)
{
    ObjExit result;

    switch (r.arm) {
    case SS_START:
        return object_exit(0, r.third);
    case SS_OTHER:
        return object_exit(r.first, r.first);
    case SS_FIFTH:
        result = object_exit(r.exit.a, r.exit.nz);
        result.exit.c = r.exit.c;
        result.exit.c_kept = false;
        result.exit.v = r.exit.v;
        result.exit.v_set = true;
        if (r.first == 0) {
            result.exit.nz = r.third;
        }
        return result;
    default:
        result = with_carry(object_exit(r.first, (uint8_t)(r.first - 0xF8)), r.first >= 0xF8);
        result.exit.v = r.exit.v;
        result.exit.v_set = true;
        if (r.arm == SS_BACK_DONE) {
            if (r.index == 1) {
                result.exit.a = 2;
            }
            result.exit.nz = r.third;
        }
        return result;
    }
}

void abi_EndingScrollSteps(void)
{
    object_exit_shim(scroll_steps_exit_shim(EndingScrollSteps(cpu.x)));
}

/* The position's high byte after a move: what the move leaves in A. */
static uint8_t move_high(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;

    return (uint8_t)((move.before + added) >> 16);
}

/* The V a move leaves: the sign of the speed against the position's high byte and the sum's. */
static bool move_overflow(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;
    uint8_t sign = (uint8_t)(added >> 16);
    uint8_t high = (uint8_t)(move.before >> 16);
    uint8_t result = move_high(move);

    return (~(sign ^ high) & (sign ^ result) & 0x80) != 0;
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
 * not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
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
        return hold_dropped(result, 0x8845);
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
EndingArm EndingScrollSqueeze(uint8_t object)
{
    EndingArm r = { 0 };
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
        r.arm = SQ_START;
        r.third = INC8(OBJ->state[object]);
        return r;
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        left = DEC8(OBJ->life_timer[object]);
        r.arm = SQ_WAIT;
        r.third = left;
        if (left == 0) {
            r.third = INC8(OBJ->state[object]);
        }
        return r;
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        r.arm = SQ_TAIL;
        r.exit = EndingScrollSlots();               /* a jump, not a call: no frame */
        return r;
    }
    jsr_push(0x88C8);
    r.exit = EndingScrollSlots();
    jsr_pop();
    r.arm = SQ_ODD_WAIT;
    if ((RD8(ZP->frame_tick) & 1) == 0) {
        return r;
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
    r.arm = SQ_MOVED;
    r.first = value;
    if (value >= 0xA7) {
        WR8(SCROLL->y_lo[4], value);
    }
    return r;
}

/* What EndingScrollSqueeze leaves in the registers and flags, from the arm it ended on. */
static ObjExit scroll_squeeze_exit_shim(EndingArm r)
{
    ObjExit result;

    switch (r.arm) {
    case SQ_START:
        return object_exit(0x40, r.third);
    case SQ_WAIT:
        return object_exit(0, r.third);
    case SQ_TAIL:
        return through_exit(r.exit);
    case SQ_ODD_WAIT:
        result = object_exit(r.exit.a, r.exit.nz);
        result.exit.c = r.exit.c;
        result.exit.c_kept = false;
        result.exit.v = r.exit.v;
        result.exit.v_set = true;
        return result;
    default:
        result = with_carry(object_exit(r.first, (uint8_t)(r.first - 0xA7)), r.first >= 0xA7);
        result.exit.v = r.exit.v;
        result.exit.v_set = true;
        return result;
    }
}

void abi_EndingScrollSqueeze(void)
{
    object_exit_shim(scroll_squeeze_exit_shim(EndingScrollSqueeze(cpu.x)));
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
EndingArm EndingDropAndBounce(uint8_t object)
{
    EndingArm r = { 0 };
    uint8_t state, low, high, timer, product, value;
    unsigned total;

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
        r.arm = DB_START;
        r.third = INC8(OBJ->state[object]);
        return r;
    }
    state = (uint8_t)(state - 1);
    if (state == 0) {
        low = RD8(OBJ->vy_lo[object]);
        total = (unsigned)low + 0x60;
        WR8(OBJ->vy_lo[object], (uint8_t)total);
        high = RD8(OBJ->vy_hi[object]);
        WR8(OBJ->vy_hi[object], (uint8_t)(high + (total > 0xFF ? 1 : 0)));
        jsr_push(0x8946);
        r.move = ApplyVelocity(object);
        jsr_pop();
        r.first = RD8(OBJ->y_lo[object]);
        r.arm = DB_FALL;
        if (r.first < 0x60) {
            return r;
        }
        WR8(OBJ->y_lo[object], 0x60);
        WR8(OBJ->x_lo[object], 0x60);
        r.arm = DB_LANDED;
        r.third = INC8(OBJ->state[object]);
        return r;
    }
    state = (uint8_t)(state - 1);
    if (state != 0) {
        r.arm = DB_OTHER;
        r.first = state;
        return r;
    }
    WR8(ZP->temp00, 0xFF);
    WR8(ZP->temp01, 0x07);
    value = RD8(OBJ->life_timer[object]);
    total = (unsigned)value + 0x10;
    timer = (uint8_t)total;
    WR8(OBJ->life_timer[object], timer);
    if ((timer & 0x80) != 0) {
        WR8(OBJ->y_lo[object], 0x60);
        INC8(OBJ->state[object]);
        WR8(SCROLL->y_lo[1], 0x2F);
        WR8(SCROLL->y_lo[2], 0xAF);
        r.arm = DB_DONE;
        r.first = value;
        r.second = timer;
        r.flag = total > 0xFF;
        return r;
    }
    jsr_push(0x8970);
    (void)ScaleBySine(timer);
    jsr_pop();
    product = RD8(ZP->product.hi);
    WR8(OBJ->y_lo[object], (uint8_t)(product + 0x60));
    product = RD8(ZP->product.hi);
    WR8(SCROLL->y_lo[1], (uint8_t)(0x2F - product));
    product = RD8(ZP->product.hi);
    WR8(SCROLL->y_lo[2], (uint8_t)(0xAF - product));
    r.arm = DB_BOUNCE;
    r.first = product;
    return r;
}

/* What EndingDropAndBounce leaves in the registers and flags, from the arm it ended on. */
static ObjExit drop_and_bounce_exit_shim(EndingArm r)
{
    ObjExit result;
    uint8_t sum;

    switch (r.arm) {
    case DB_START:
        return object_exit(0xC0, r.third);
    case DB_FALL:
    case DB_LANDED:
        result = with_carry(object_exit(r.first, (uint8_t)(r.first - 0x60)), r.first >= 0x60);
        result = under_move(result, r.move);
        if (r.arm == DB_LANDED) {
            result.exit.a = 0x60;
            result.exit.nz = r.third;
        }
        return result;
    case DB_OTHER:
        return object_exit(r.first, r.first);
    case DB_DONE:
        result = with_carry(object_exit(0xAF, 0xAF), r.flag);
        result.exit.v = ((r.first ^ r.second) & (0x10 ^ r.second) & 0x80) != 0;
        result.exit.v_set = true;
        return result;
    default:
        sum = (uint8_t)(0xAF - r.first);
        result = with_carry(object_exit(sum, sum), 0xAF >= r.first);
        result.exit.v = ((0xAF ^ r.first) & (0xAF ^ sum) & 0x80) != 0;
        result.exit.v_set = true;
        result.exit.y = 0;
        result.exit.y_kept = false;
        return result;
    }
}

void abi_EndingDropAndBounce(void)
{
    object_exit_shim(drop_and_bounce_exit_shim(EndingDropAndBounce(cpu.x)));
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
 * not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
 */
EndingArm EndingCreditScrollStart(uint8_t object)
{
    EndingArm r = { 0 };
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
        r.arm = CS_START;
        r.third = INC8(OBJ->state[object]);
        return r;
    }
    r.first = state;
    r.arm = CS_AT_TEN;
    if (state == 0x0A) {
        return r;
    }
    jsr_push(0x826C);
    left = EndingHoldTimer(object);
    if (left != 0) {
        r.arm = CS_DROPPED;
        return r;
    }
    jsr_pop();
    state = RD8(OBJ->state[object]);
    WR8(SCROLL->x_hi[0], rd8((uint16_t)(0x8284 + (uint8_t)(state << 1))));
    WR8(SCROLL->y_hi[0], rd8((uint16_t)(0x8285 + (uint8_t)(state << 1))));
    WR8(OBJ->life_timer[object], 6);
    r.arm = CS_ADVANCED;
    r.second = state;
    r.third = INC8(OBJ->state[object]);
    return r;
}

/* What EndingCreditScrollStart leaves in the registers and flags, from the arm it ended on. */
static ObjExit credit_scroll_exit_shim(EndingArm r)
{
    ObjExit result;

    if (r.arm == CS_START) {
        result = with_carry(object_exit(6, r.third), false);
        result.exit.y = 0x1A;
        result.exit.y_kept = false;
    } else if (r.arm == CS_ADVANCED) {
        result = with_carry(object_exit(6, r.third), (r.second & 0x80) != 0);
        result.exit.y = (uint8_t)(r.second << 1);
        result.exit.y_kept = false;
    } else {
        result = with_carry(object_exit(r.first, (uint8_t)(r.first - 0x0A)), r.first >= 0x0A);
        if (r.arm == CS_DROPPED) {
            result = hold_dropped(result, 0x826C);
        }
    }
    result.exit.v = false;                      /* the add of the list number: no overflow */
    result.exit.v_set = true;
    return result;
}

void abi_EndingCreditScrollStart(void)
{
    object_exit_shim(credit_scroll_exit_shim(EndingCreditScrollStart(cpu.x)));
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
EndingArm EndingFallingPiece(uint8_t object)
{
    EndingArm r = { 0 };
    uint8_t state, sum, tick, index, high;
    unsigned total;

    state = RD8(OBJ->state[object]);
    if (state == 0) {
        WR8(OBJ->vy_hi[object], rd8((uint16_t)(0x82D5 + object)));
        WR8(OBJ->sprite_set[object], 3);
        total = (unsigned)object + 0x72;
        sum = (uint8_t)total;
        WR8(OBJ->frame[object], sum);
        r.arm = FP_START;
        r.first = object;
        r.second = sum;
        r.flag = total > 0xFF;
        r.third = INC8(OBJ->state[object]);
        return r;
    }
    tick = RD8(ZP->frame_tick);
    index = (uint8_t)((((uint8_t)(object + tick)) >> 2) & 1);
    WR8(OBJ->flash_flags[object], rd8((uint16_t)(0x82DA + index)));
    jsr_push(0x82C5);
    r.move = ApplyVelocity(object);
    jsr_pop();
    r.index = index;
    high = RD8(OBJ->y_hi[object]);
    r.first = high;
    r.arm = FP_POSITIVE;
    if ((high & 0x80) == 0) {
        return r;
    }
    r.second = RD8(OBJ->y_lo[object]);
    r.arm = FP_LOW;
    if ((r.second & 0x80) != 0) {
        return r;
    }
    WR8(OBJ->y_hi[object], 1);
    r.arm = FP_SET;
    return r;
}

/* What EndingFallingPiece leaves in the registers and flags, from the arm it ended on. */
static ObjExit falling_piece_exit_shim(EndingArm r)
{
    ObjExit result;

    if (r.arm == FP_START) {
        result = with_carry(object_exit(r.second, r.third), r.flag);
        result.exit.v = ((r.first ^ r.second) & (0x72 ^ r.second) & 0x80) != 0;
        result.exit.v_set = true;
        return result;
    }
    result = under_move(object_exit(r.first, r.first), r.move);
    result.exit.y = r.index;
    result.exit.y_kept = false;
    if (r.arm == FP_LOW) {
        result.exit.a = r.second;
        result.exit.nz = r.second;
    } else if (r.arm == FP_SET) {
        result.exit.a = 1;
        result.exit.nz = 1;
    }
    return result;
}

void abi_EndingFallingPiece(void)
{
    object_exit_shim(falling_piece_exit_shim(EndingFallingPiece(cpu.x)));
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

/*
 * The ending's scene script: the pointer at $7E, the frame counter zWaitTimer3 and zWaitTimer4. A command is a byte, its length from a table
 * at $8030, a time as two bytes, and operands; EndingScriptAdvance steps past a command and EndingScriptStep runs the next if its time is
 * the counter's, and each command goes back to EndingScriptAdvance.
 */
#define ENDING_PTR _gap_007C[2]
#define ENDING_PTR_HI _gap_007C[3]

enum { VDC_REG_VRAM_A = 2, ENDING_LENGTHS = 0x8030, ENDING_COMMANDS = 0x806A, ENDING_OBJECTS = 0x58 };

/* The registers a command handler outside C is entered with: A and X the doubled command, Y 3. */
static void ending_tail_shim(uint8_t index, uint8_t command, int v_in)
{
    if (v_in >= 0) {
        abi_flag(P_V, v_in != 0);
    }
    cpu.a = index;
    cpu.x = index;
    cpu.y = 3;
    abi_nz(3);
    abi_flag(P_C, (command & 0x80) != 0);
}

/*
 * @routine 010D:131 EndingClearObjects
 * @run 6B:8131
 * WHAT IT DOES: Empties all 88 object slots.
 * HOW: Sets X to 0 and runs into a loop of ClearObject.
 * WAITS: no.
 * Entry: nothing. Exit: X $58, C and Z set, N clear; A and Y kept.
 */
void EndingClearObjects(void)
{
    uint8_t slot;

    for (slot = 0; slot != ENDING_OBJECTS; slot++) {
        jsr_push(0x8135);
        ClearObject(slot);
        jsr_pop();
    }
}

void abi_EndingClearObjects(void)
{
    EndingClearObjects();
    cpu.x = ENDING_OBJECTS;
    abi_nz(0);
    abi_flag(P_C, true);
}

Exit EndingScriptStepV(int v_in);

/*
 * @routine 010D:040 EndingScriptAdvance
 * @run 6B:8040
 * WHAT IT DOES: Steps the ending's script past the command it is at, and runs on into EndingScriptStep.
 * HOW: The command byte indexes a table of lengths, which is added to the pointer (the high byte goes up on a carry).
 * WAITS: no.
 * Entry: nothing. Exit: as EndingScriptStep, X the command stepped past (when EndingScriptStep left X alone), V from the pointer's add
 * (when it set none).
 */
Exit EndingScriptAdvance(void)
{
    Exit exit;
    uint8_t command, length, low, sum_value;
    bool sum_carry, sum_overflow;
    unsigned total;

    command = RD8_PTR(ZP->ENDING_PTR);
    length = rd8((uint16_t)(ENDING_LENGTHS + command));
    low = RD8(ZP->ENDING_PTR);
    total = (unsigned)length + low;
    sum_value = (uint8_t)total;
    sum_carry = total > 0xFF;
    sum_overflow = ((length ^ sum_value) & (low ^ sum_value) & 0x80) != 0;
    WR8(ZP->ENDING_PTR, sum_value);
    if (sum_carry) {
        INC8(ZP->ENDING_PTR_HI);
    }
    exit = EndingScriptStepV(sum_overflow);
    if (exit.x_kept) {
        exit.x = command;
        exit.x_kept = false;
    }
    if (!exit.v_set) {
        exit.v = sum_overflow;
        exit.v_set = true;
    }
    return exit;
}

void abi_EndingScriptAdvance(void)
{
    exit_shim(EndingScriptAdvance());
}

/*
 * @routine 010D:090 EndingCommandStartList
 * @run 6B:8090
 * WHAT IT DOES: A script command: starts list channel 0 with the operand's list, clears the screen's state and all the object slots, and
 * clears the scroll block again.
 * HOW: StartListChannel0Kind2, ClearScreenState, EndingClearObjects, a TAI from $4003 to $3E00; back to EndingScriptAdvance.
 * WAITS: no.
 * Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
 */
Exit EndingCommandStartList(void)
{
    uint8_t list = RD8_PTR_Y(ZP->ENDING_PTR, 3);

    jsr_push(0x8095);
    StartListChannel0Kind2(list);
    jsr_pop();
    jsr_push(0x8098);
    ClearScreenState(wait_regs_xy_shim(0, 3));   /* X 0 from the list start, Y 3 from EndingScriptStep */
    jsr_pop();
    jsr_push(0x809B);
    EndingClearObjects();
    jsr_pop();
    block_move_tai(0x4003, ScrollXFrac, 0x00EE, VDC_REG_VRAM_A, ENDING_OBJECTS, 0xFF);
    return EndingScriptAdvance();
}

static Exit start_script_command(uint8_t kind)
{
    uint8_t script = RD8_PTR_Y(ZP->ENDING_PTR, 3);

    jsr_push(kind == 1 ? 0x80B0 : 0x80B7);
    if (kind == 1) {
        (void)StartScriptSlot0Kind1(script);
    } else {
        (void)StartScriptSlot0Kind2(script);
    }
    jsr_pop();
    return EndingScriptAdvance();
}

/*
 * @routine 010D:0AB EndingCommandStartScript1
 * @run 6B:80AB
 * WHAT IT DOES: A script command: starts script slot 0 as kind 1 with the operand's script.
 * HOW: StartScriptSlot0Kind1; back to EndingScriptAdvance.
 * WAITS: no.
 * Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
 */
Exit EndingCommandStartScript1(void)
{
    return start_script_command(1);
}

/*
 * @routine 010D:0B2 EndingCommandStartScript2
 * @run 6B:80B2
 * WHAT IT DOES: A script command: starts script slot 0 as kind 2 with the operand's script.
 * HOW: StartScriptSlot0Kind2; back to EndingScriptAdvance.
 * WAITS: no.
 * Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
 */
Exit EndingCommandStartScript2(void)
{
    return start_script_command(2);
}

/*
 * @routine 010D:0C5 EndingCommandCdPlay
 * @run 6B:80C5
 * WHAT IT DOES: A script command: plays the CD track the operand names, once.
 * HOW: CdPlayOnce; back to EndingScriptAdvance.
 * WAITS: no.
 * Entry: from EndingScriptStep, X the doubled command, Y 3. Exit: as EndingScriptAdvance.
 */
Exit EndingCommandCdPlay(uint8_t index)
{
    uint8_t track = RD8_PTR_Y(ZP->ENDING_PTR, 3);

    jsr_push(0x80CA);
    (void)CdPlayOnce(track, index, 3);
    jsr_pop();
    return EndingScriptAdvance();
}

/*
 * @routine 010D:13B EndingPlaceObject
 * @run 6B:813B
 * WHAT IT DOES: Empties an object and gives it the place four script bytes name (the copy of ScriptPlaceObject in this overlay).
 * HOW: ClearObject; then the bytes at Y plus 1 to 4 go to the object's X low, X high, Y low and Y high.
 * WAITS: no.
 * Entry: X the object, Y the script byte before the four. Exit: A the last byte read, Y plus 4, N and Z from A; X and C kept.
 */
Exit EndingPlaceObject(uint8_t object, uint8_t y)
{
    Exit exit = { 0 };
    uint8_t value;

    jsr_push(0x813E);
    ClearObject(object);
    jsr_pop();
    y++;
    value = RD8_PTR_Y(ZP->ENDING_PTR, y);
    WR8(OBJ->x_lo[object], value);
    y++;
    value = RD8_PTR_Y(ZP->ENDING_PTR, y);
    WR8(OBJ->x_hi[object], value);
    y++;
    value = RD8_PTR_Y(ZP->ENDING_PTR, y);
    WR8(OBJ->y_lo[object], value);
    y++;
    value = RD8_PTR_Y(ZP->ENDING_PTR, y);
    WR8(OBJ->y_hi[object], value);
    exit.a = value;
    exit.y = y;
    exit.nz = value;
    exit.x_kept = true;
    exit.c_kept = true;
    return exit;
}

void abi_EndingPlaceObject(void)
{
    exit_shim(EndingPlaceObject(cpu.x, cpu.y));
}

/* The commands that place an object: ClearObject and the four place bytes, then the fields each command names. */
static Exit place_command(uint8_t fields, uint16_t first_ret)
{
    uint8_t object, y, value;

    object = RD8_PTR_Y(ZP->ENDING_PTR, 3);
    jsr_push(first_ret);
    (void)EndingPlaceObject(object, 3);
    jsr_pop();
    y = 7;
    switch (fields) {
    case 0:
        y++;
        value = RD8_PTR_Y(ZP->ENDING_PTR, y);
        WR8(OBJ->kind[object], value);
        break;
    case 1:
        y++;
        value = RD8_PTR_Y(ZP->ENDING_PTR, y);
        WR8(OBJ->next_kind[object], value);
        y++;
        value = RD8_PTR_Y(ZP->ENDING_PTR, y);
        WR8(OBJ->sprite_set[object], value);
        WR8(OBJ->kind[object], 2);
        break;
    case 2:
        y++;
        value = RD8_PTR_Y(ZP->ENDING_PTR, y);
        WR8(OBJ->frame[object], value);
        y++;
        value = RD8_PTR_Y(ZP->ENDING_PTR, y);
        WR8(OBJ->sprite_set[object], value);
        break;
    default:
        y++;
        value = RD8_PTR_Y(ZP->ENDING_PTR, y);
        WR8(OBJ->frame[object], value);
        y++;
        value = RD8_PTR_Y(ZP->ENDING_PTR, y);
        WR8(OBJ->sprite_set[object], value);
        y++;
        value = RD8_PTR_Y(ZP->ENDING_PTR, y);
        WR8(OBJ->next_kind[object], value);
        y++;
        value = RD8_PTR_Y(ZP->ENDING_PTR, y);
        WR8(OBJ->life_timer[object], value);
        WR8(OBJ->kind[object], 3);
        break;
    }
    return EndingScriptAdvance();
}

/* The command that reads four banks of the ADPCM memory into the RAM banks at MPR5 and MPR6. */
/*
 * @routine 010D:0CD EndingCommandPlaceKind
 * @run 6B:80CD
 * WHAT IT DOES: A script command: places an object (empties it and gives it the four place bytes) and sets its kind from the byte after.
 * HOW: ScriptPlaceObject; one store; back to EndingScriptAdvance.
 * WAITS: no.
 * Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
 */
Exit EndingCommandPlaceKind(void)
{
    return place_command(0, 0x80D3);
}

/*
 * @routine 010D:0DC EndingCommandPlaceNext
 * @run 6B:80DC
 * WHAT IT DOES: A script command: places an object, sets its next kind and picture kind from the two bytes after, and its kind to 2.
 * HOW: ScriptPlaceObject; three stores; back to EndingScriptAdvance.
 * WAITS: no.
 * Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
 */
Exit EndingCommandPlaceNext(void)
{
    return place_command(1, 0x80E2);
}

/*
 * @routine 010D:0F6 EndingCommandPlaceFrame
 * @run 6B:80F6
 * WHAT IT DOES: A script command: places an object and sets its frame and picture kind from the two bytes after.
 * HOW: ScriptPlaceObject; two stores; back to EndingScriptAdvance.
 * WAITS: no.
 * Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
 */
Exit EndingCommandPlaceFrame(void)
{
    return place_command(2, 0x80FC);
}

/*
 * @routine 010D:10B EndingCommandPlaceTimed
 * @run 6B:810B
 * WHAT IT DOES: A script command: places an object and sets its frame, picture kind, next kind and hold from the four bytes after, and its kind to 3.
 * HOW: ScriptPlaceObject; five stores; back to EndingScriptAdvance.
 * WAITS: no.
 * Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
 */
Exit EndingCommandPlaceTimed(void)
{
    return place_command(3, 0x8111);
}

/*
 * @routine 010D:1A5 EndingCommandLoadBanks
 * @run 6B:81A5
 * WHAT IT DOES: A script command: reads four banks of the ADPCM memory into the RAM banks at MPR5 and MPR6, one after another.
 * HOW: For each of four numbers the two banks are mapped, the ADPCM address and the length set, and ad_read called; back to EndingScriptAdvance.
 * WAITS: yes, through ad_read, which waits for the ADPCM to be idle.
 * Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
 */
Exit EndingCommandLoadBanks(void)
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
        jsr_push(0x81CF);
        (void)vec_ad_read();
        jsr_pop();
        INC8(ZP->temp00);
    } while (RD8(ZP->temp00) != 4);
    return EndingScriptAdvance();
}

/*
 * @routine 010D:04F EndingScriptStep
 * @run 6B:804F
 * WHAT IT DOES: Runs the ending's script command if its time has come: the two time bytes after the command byte equal the frame counter.
 * HOW: Compares the first time byte with zWaitTimer3 and the second with zWaitTimer4 and returns on a difference; otherwise the
 * command, doubled, indexes the table at $806A and it is jumped to with X the doubled command and Y 3.
 * WAITS: no.
 * Entry: nothing. Exit, a time byte differs: A that byte, N, Z and C from comparing it, Y 1 or 2, X kept. Otherwise as the command leaves them.
 */
Exit EndingScriptStepV(int v_in)
{
    Exit exit = { 0 };
    uint8_t time, counter, command, index, value, high;
    uint16_t target;

    time = RD8_PTR_Y(ZP->ENDING_PTR, 1);
    counter = RD8(ZP->wait_timer3);
    if (time != counter) {
        exit.a = time;
        exit.y = 1;
        exit.nz = (uint8_t)(time - counter);
        exit.c = time >= counter;
        exit.x_kept = true;
        return exit;
    }
    time = RD8_PTR_Y(ZP->ENDING_PTR, 2);
    counter = RD8(ZP->wait_timer4);
    if (time != counter) {
        exit.a = time;
        exit.y = 2;
        exit.nz = (uint8_t)(time - counter);
        exit.c = time >= counter;
        exit.x_kept = true;
        return exit;
    }
    command = RD8_PTR(ZP->ENDING_PTR);
    index = (uint8_t)(command << 1);
    value = rd8((uint16_t)(ENDING_COMMANDS + index));
    high = rd8((uint16_t)(ENDING_COMMANDS + 1 + index));
    target = (uint16_t)(value | (high << 8));
    switch (target) {
    case 0x8090: return EndingCommandStartList();
    case 0x80AB: return EndingCommandStartScript1();
    case 0x80B2: return EndingCommandStartScript2();
    case 0x80C5: return EndingCommandCdPlay(index);
    case 0x80CD: return EndingCommandPlaceKind();
    case 0x80DC: return EndingCommandPlaceNext();
    case 0x80F6: return EndingCommandPlaceFrame();
    case 0x810B: return EndingCommandPlaceTimed();
    case 0x81A5: return EndingCommandLoadBanks();
    case 0x8067:
        exit.a = index;
        exit.x = index;
        exit.y = 3;
        exit.nz = DEC8(ZP->mode_done);
        exit.c = (command & 0x80) != 0;
        return exit;
    default:
        break;
    }
    ending_tail_shim(index, command, v_in);
    run_routine(target);
    exit.from_cpu = true;
    return exit;
}

Exit EndingScriptStep(void)
{
    return EndingScriptStepV(-1);
}

void abi_EndingScriptStep(void)
{
    exit_shim(EndingScriptStep());
}

/*
 * @routine 010D:00B EndingBegin
 * @run 6B:800B
 * WHAT IT DOES: Starts the ending: the game state is wiped, the script is set to run from $8AA3, its frame counter to $FFFF and the
 * state goes on.
 * HOW: ResetGameState; the script pointer and the counter are written; zGameSubMode counted up.
 * WAITS: yes, through ResetGameState, which waits for the vertical blank with the display off; the stack there is this routine's
 * return address and the call's.
 * Entry: X the doubled sub-mode and Y kept (ResetGameState saves them). Exit: A $FF, X 0, Y $FF, N and Z from the new state; C kept.
 */
Exit EndingBegin(uint8_t kept_x, uint8_t kept_y)
{
    Exit exit = { 0 };

    jsr_push(0x800E);
    ResetGameState(wait_regs_xy_shim(kept_x, kept_y));
    jsr_pop();
    WR8(ZP->ENDING_PTR, 0xA3);
    WR8(ZP->ENDING_PTR_HI, 0x8A);
    WR8(ZP->wait_timer3, 0xFF);
    WR8(ZP->wait_timer4, 0xFF);
    exit.a = 0xFF;
    exit.x = 0;
    exit.y = 0xFF;
    exit.nz = INC8(ZP->game_sub_mode);
    exit.c_kept = true;
    return exit;
}

void abi_EndingBegin(void)
{
    exit_shim(EndingBegin(cpu.x, cpu.y));
}

/*
 * @routine 1E61:00B EndingBegin1E61
 * @run 6B:800B
 * WHAT IT DOES: Starts the ending of the overlay at sector 1E61: the game state is wiped, bit 3 of ProgressCharacter is set,
 * the script runs from the address the table at $802D gives for the character, its frame counter is set to $FFFF and the
 * state goes on.
 * HOW: ResetGameState; an ORA into ProgressCharacter; zCharacter doubled indexes the table; the counter written;
 * zGameSubMode counted up.
 * WAITS: yes, through ResetGameState (the vertical blank, the display off); the stack there is this routine's return
 * address and the call's.
 * Entry: X the doubled sub-mode and Y kept (ResetGameState saves them). Exit: A $FF, X 0, Y the character doubled, N and
 * Z from the new state, C bit 7 of zCharacter (the ASL).
 */
Exit EndingBegin1E61(uint8_t kept_x, uint8_t kept_y)
{
    Exit exit = { 0 };
    uint8_t character, row;

    jsr_push(0x800E);
    ResetGameState(wait_regs_xy_shim(kept_x, kept_y));
    jsr_pop();
    wr8(ProgressCharacter, (uint8_t)(rd8(ProgressCharacter) | 0x08));
    character = rd8(zCharacter);
    row = (uint8_t)(character << 1);
    WR8(ZP->ENDING_PTR, rd8((uint16_t)(0x802D + row)));
    WR8(ZP->ENDING_PTR_HI, rd8((uint16_t)(0x802E + row)));
    WR8(ZP->wait_timer3, 0xFF);
    WR8(ZP->wait_timer4, 0xFF);
    exit.a = 0xFF;
    exit.x = 0;
    exit.y = row;
    exit.c = (character & 0x80) != 0;
    exit.nz = INC8(ZP->game_sub_mode);
    return exit;
}

void abi_EndingBegin1E61(void)
{
    exit_shim(EndingBegin1E61(cpu.x, cpu.y));
}

/*
 * @routine 010D:1E9 EndingDispatchKind
 * @run 6B:81E9
 * WHAT IT DOES: Sends an object to the handler its kind names: the table at $81F9 holds one address per kind, kind 1 first.
 * HOW: Kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01; the jump is made with A the word's high
 * byte, Y the index, C bit 7 of kind less one, X the object.
 * WAITS: no.
 * Entry: A the kind (not zero), X the object. Exit: as the handler leaves them.
 */
ObjExit EndingDispatchKind(uint8_t kind, uint8_t object)
{
    ObjExit result;
    Exit exit = { 0 };
    uint8_t less, index, low, high, high_read;
    uint16_t target;
    bool carry;

    less = (uint8_t)(kind - 1);
    index = (uint8_t)(less << 1);
    carry = (less & 0x80) != 0;
    low = rd8((uint16_t)(0x81F9 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0x81FA + index));
    WR8(ZP->temp01, high);
    low = RD8(ZP->temp00);
    high_read = RD8(ZP->temp01);
    target = (uint16_t)(low | (high_read << 8));
    switch (target) {
    case 0x899C: result = EndingWaitForList(object); break;
    case 0x89B2: result = EndingFlyOff(object); break;
    case 0x8902: result = drop_and_bounce_exit_shim(EndingDropAndBounce(object)); break;
    case 0x8887: result = scroll_squeeze_exit_shim(EndingScrollSqueeze(object)); break;
    case 0x8853: result = EndingLinkAnimationA(object); break;
    case 0x8876: result = EndingLinkAnimationB(object); break;
    case 0x881D: result = EndingHoldThenScript19(object); break;
    case 0x8720: result = EndingRiseFrame31(object); break;
    case 0x8759: result = EndingRiseFrame32A(object); break;
    case 0x87A0: result = EndingRiseWithList(object); break;
    case 0x8405: result = EndingScrollRunUp(object); break;
    case 0x86E3: result = EndingScrollUp(object); break;
    case 0x869A: result = EndingScrollDown(object); break;
    case 0x8668: result = EndingScrollRight6(object); break;
    case 0x85FB: result = scroll_steps_exit_shim(EndingScrollSteps(object)); break;
    case 0x850C: result = EndingPlayList0EThenClear(object); break;
    case 0x84DE: result = EndingRiseToY40(object); break;
    case 0x84BE: result = EndingPlayList0F(object); break;
    case 0x8484: result = EndingScrollWindowA(object); break;
    case 0x8438: result = EndingScrollWindowSteps(object); break;
    case 0x8418: result = EndingPlayList10(object); break;
    case 0x84A1: result = EndingScrollWindowB(object); break;
    case 0x83CE: result = EndingScrollPull(object); break;
    case 0x836A: result = EndingAimAtPlace(object); break;
    case 0x82E8: result = EndingPieceSpawn(object); break;
    case 0x82E2: result = EndingClearAfterScrollY(object); break;
    case 0x82DC: result = EndingClearAfterScrollX(object); break;
    case 0x8298: result = falling_piece_exit_shim(EndingFallingPiece(object)); break;
    case 0x8239: result = credit_scroll_exit_shim(EndingCreditScrollStart(object)); break;
    case 0x87D6: result = EndingRiseFrame32B(object); break;
    default:
        exit.from_cpu = true;
        result.exit = exit;
        result.moved = false;
        cpu_entry_shim(high, object, index, carry);
        run_routine(target);
        return result;
    }
    if (result.exit.a_kept) {
        result.exit.a = result.moved ? move_high(result.move) : high;
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

void abi_EndingDispatchKind(void)
{
    object_exit_shim(EndingDispatchKind(cpu.a, cpu.x));
}

/* After the last object: the loop's compare of X with $58, and the A the last slot left. */
static void objects_end_shim(uint8_t a)
{
    cpu.a = a;
    cpu.x = ENDING_OBJECTS;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 010D:1DA EndingObjectsStep
 * @run 6B:81DA
 * WHAT IT DOES: Runs every object of the ending scene that has a kind: a loop over the 88 slots, each with a kind sent to its handler.
 * HOW: OBJ->kind read for each slot; EndingDispatchKind for the ones that are not zero.
 * WAITS: no.
 * Entry: nothing. Exit: X $58, N and Z clear... Z set, C set (the loop's last compare); A the last slot's kind or what its handler left;
 * Y and V as the last handler left them.
 * not checked: the jump to a handler is computed and through mode cannot follow it; only the idle loop is compared (curated/ending_objects_idle.txt).
 */
static Exit objects_step(Exit prior)
{
    Exit exit = { 0 };
    ObjExit result;
    uint8_t slot, kind, a = 0, y = 0;
    bool y_set = false, v_set = false, v = false, via_cpu = false;

    for (slot = 0; slot != ENDING_OBJECTS; slot++) {
        kind = RD8(OBJ->kind[slot]);
        a = kind;
        if (kind == 0) {
            continue;
        }
        jsr_push(0x81E3);
        result = EndingDispatchKind(kind, slot);
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
        if (result.moved) {
            v = move_overflow(result.move);
            v_set = true;
        }
        if (result.exit.v_set) {
            v = result.exit.v;
            v_set = true;
        }
    }
    if (via_cpu) {
        exit.from_cpu = true;
        objects_end_shim(a);
        return exit;
    }
    exit.a = a;
    exit.x = ENDING_OBJECTS;
    exit.y = y_set ? y : prior.y;
    exit.y_kept = y_set ? false : prior.y_kept;
    exit.nz = 0;
    exit.c = true;
    exit.v = v_set ? v : prior.v;
    exit.v_set = v_set ? true : prior.v_set;
    return exit;
}

Exit EndingObjectsStep(void)
{
    Exit none = { 0 };

    none.y_kept = true;
    return objects_step(none);
}

void abi_EndingObjectsStep(void)
{
    exit_shim(EndingObjectsStep());
}

/*
 * @routine 010D:01F EndingFrame
 * @run 6B:801F
 * WHAT IT DOES: One frame of the ending: the frame counted, the script's next command run if it is due, and every object run.
 * HOW: EndingFrameTick, EndingScriptStep, EndingObjectsStep.
 * WAITS: no.
 * Entry: nothing. Exit: as EndingObjectsStep.
 * not checked: as EndingObjectsStep: only the idle loop is compared.
 */
Exit EndingFrame(void)
{
    Exit exit;

    jsr_push(0x8022);
    EndingFrameTick();
    jsr_pop();
    jsr_push(0x8025);
    exit = EndingScriptStep();
    jsr_pop();
    jsr_push(0x8028);
    exit = objects_step(exit);
    jsr_pop();
    return exit;
}

void abi_EndingFrame(void)
{
    exit_shim(EndingFrame());
}

/*
 * @routine 010D:000 EndingStep
 * @run 6B:8000
 * WHAT IT DOES: One call of the ending's mode: the handler of the sub-mode zGameSubMode names in the table at $8007 (0 begins the ending,
 * 1 runs a frame of it).
 * HOW: Reads zGameSubMode, doubles it and jumps through the table. A word this overlay does not hold runs as the original code.
 * WAITS: yes, through EndingBegin, which waits for the vertical blank.
 * Entry: nothing. Exit: as the handler leaves them.
 * not checked: one case of 400 jumps to a table word outside this overlay's C.
 */
Exit EndingStep(uint8_t kept_y)
{
    Exit exit = { 0 };
    uint8_t state, index, low, high;
    uint16_t target;

    state = RD8(ZP->game_sub_mode);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0x8007 + index));
    high = rd8((uint16_t)(0x8008 + index));
    target = (uint16_t)(low | (high << 8));
    /* The overlay loaded at $8000 decides what its table's words are: this file's 010D, or 1E61, whose $800B begins with
     * the character's script and whose $801F is inside that routine. */
    if (resident_at(0x800B) == DISC(0x1E61, 0x00B)) {
        if (target == 0x800B)
            return EndingBegin1E61(index, kept_y);
    } else if (resident_at(0x800B) == DISC(0x010D, 0x00B)) {
        switch (target) {
        case 0x800B: return EndingBegin(index, kept_y);
        case 0x801F: return EndingFrame();
        default:
            break;
        }
    }
    cpu_entry_shim(index, index, kept_y, (state & 0x80) != 0);
    run_routine(target);
    exit.from_cpu = true;
    return exit;
}

void abi_EndingStep(void)
{
    exit_shim(EndingStep(cpu.y));
}

/*
 * The ending's palette effects (overlay sector $0111, bank $6C at $8000): twelve slots at $3C00, a type byte each (0 free), a list
 * number at $3C0C, the list's step at $3C18, a count at $3C24 and a timer at $3C30; the slot's other fields at $3C3C, $3C48 and $3C54.
 */
enum {
    EFFECT_TYPE = 0x3C00, EFFECT_LIST = 0x3C0C, EFFECT_STEP = 0x3C18, EFFECT_COUNT = 0x3C24, EFFECT_TIMER = 0x3C30,
    EFFECT_F3C3C = 0x3C3C, EFFECT_F3C48 = 0x3C48, EFFECT_F3C54 = 0x3C54, EFFECT_SLOTS = 12, EFFECT_COLOURS = 0x3C60,
    EFFECT_TABLE = 0x82EA
};

/* The slot given back: every field of it cleared, the type last... first. */
static void clear_effect_slot(uint8_t slot)
{
    wr8((uint16_t)(EFFECT_TYPE + slot), 0);
    wr8((uint16_t)(EFFECT_LIST + slot), 0);
    wr8((uint16_t)(EFFECT_STEP + slot), 0);
    wr8((uint16_t)(EFFECT_COUNT + slot), 0);
    wr8((uint16_t)(EFFECT_TIMER + slot), 0);
    wr8((uint16_t)(EFFECT_F3C3C + slot), 0);
    wr8((uint16_t)(EFFECT_F3C48 + slot), 0);
    wr8((uint16_t)(EFFECT_F3C54 + slot), 0);
}

/* A plain Exit that keeps what it does not name. */
static Exit effect_exit(uint8_t a, uint8_t nz)
{
    Exit exit = { 0 };

    exit.a = a;
    exit.nz = nz;
    exit.x_kept = exit.y_kept = exit.c_kept = true;
    return exit;
}

/* What a palette load leaves in Y, C and V, laid over an exit. */
static void effect_after_load(Exit *exit, PaletteLoad load)
{
    exit->y = 9;
    exit->y_kept = false;
    exit->c = load.sum.carry;
    exit->c_kept = false;
    exit->v = load.sum.overflow;
    exit->v_set = true;
}

/* A byte at an address counted up or down, as the INC and DEC of an absolute,X operand do. */
static uint8_t inc_at(uint16_t address)
{
    uint8_t value = (uint8_t)(rd8(address) + 1);

    wr8(address, value);
    return value;
}

static uint8_t dec_at(uint16_t address)
{
    uint8_t value = (uint8_t)(rd8(address) - 1);

    wr8(address, value);
    return value;
}

/* The slot's list address in zTemp00 and zTemp01, from the table for a negative list number or the pointer at $9400. */
static void select_effect_list(uint8_t list, uint16_t negative_table, uint16_t pointer)
{
    uint8_t y = (uint8_t)(list << 1);

    if ((list & 0x80) != 0) {
        WR8(ZP->temp00, rd8((uint16_t)(negative_table + y)));
        WR8(ZP->temp01, rd8((uint16_t)(negative_table + 1 + y)));
        return;
    }
    WR8(ZP->bios_a.lo, rd8(pointer));
    WR8(ZP->bios_a.hi, rd8((uint16_t)(pointer + 1)));
    WR8(ZP->temp00, RD8_PTR_Y(ZP->bios_a.lo, y));
    y++;
    WR8(ZP->temp01, RD8_PTR_Y(ZP->bios_a.lo, y));
}

/*
 * @routine 0111:2F8 EndingEffectList
 * @run 6C:82F8
 * WHAT IT DOES: A palette effect that plays a list of palette loads: once its timer has run down it takes the list's palettes one a
 * step (each loaded by LoadPackedPaletteC), the timer set from the list's first byte; when the list ends it counts a round and starts
 * the list again, and when the rounds the list names have been played it gives the slot back.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at $89D2).
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N, Z and C from
 * comparing it with the rounds played, Y 1; X kept.
 */
Exit EndingEffectList(uint8_t slot)
{
    Exit exit;
    PaletteLoad load = { { 0, false, false }, 0 };
    uint8_t timer, y, value, rounds, count;
    bool loaded = false;

    for (;;) {
        timer = rd8((uint16_t)(EFFECT_TIMER + slot));
        if (timer != 0) {
            exit = effect_exit(timer, dec_at((uint16_t)(EFFECT_TIMER + slot)));
            if (loaded) {
                effect_after_load(&exit, load);
            }
            return exit;
        }
        select_effect_list(rd8((uint16_t)(EFFECT_LIST + slot)), 0x89D2, 0x9400);
        y = 0;
        value = RD8_PTR_Y(ZP->temp00, y);
        wr8((uint16_t)(EFFECT_TIMER + slot), value);
        for (;;) {
            y = (uint8_t)((uint8_t)(rd8((uint16_t)(EFFECT_STEP + slot)) << 2) + 2);
            value = RD8_PTR_Y(ZP->temp00, y);
            if (value != 0xFF) {
                jsr_push(0x8340);
                load = LoadPackedPaletteC(y);
                jsr_pop();
                loaded = true;
                inc_at((uint16_t)(EFFECT_STEP + slot));
                break;
            }
            inc_at((uint16_t)(EFFECT_COUNT + slot));
            wr8((uint16_t)(EFFECT_STEP + slot), 0);
            y = 1;
            value = RD8_PTR_Y(ZP->temp00, y);
            if ((value & 0x80) != 0) {
                continue;
            }
            rounds = (uint8_t)(value & 0x7F);
            count = rd8((uint16_t)(EFFECT_COUNT + slot));
            if (rounds >= count) {
                continue;
            }
            clear_effect_slot(slot);
            exit = effect_exit(rounds, (uint8_t)(rounds - count));
            exit.c = false;
            exit.c_kept = false;
            exit.y = 1;
            exit.y_kept = false;
            if (loaded) {
                exit.v = load.sum.overflow;
                exit.v_set = true;
            }
            return exit;
        }
    }
}

void abi_EndingEffectList(void)
{
    exit_shim(EndingEffectList(cpu.x));
}

/* The V the last SBC of a colour left (set false: none ran). */
typedef struct { bool set; bool v; } FadeV;

/* One colour word of the fade: each of its three 3-bit components counted down by one unless it is zero; the V of
 * the last subtract it ran. */
static FadeV fade_colour(uint8_t x)
{
    uint8_t low, part, blue, high, carry_out, red;
    FadeV last = { false, false };

    low = rd8((uint16_t)(EFFECT_COLOURS + x));
    part = (uint8_t)(low & 0x07);
    if (part != 0) {
        part--;
    }
    WR8(ZP->temp02, part);
    low = rd8((uint16_t)(EFFECT_COLOURS + x));
    blue = (uint8_t)(low & 0x38);
    if (blue != 0) {
        last.v = (((blue ^ 0x08) & (blue ^ (uint8_t)(blue - 0x08))) & 0x80) != 0;
        last.set = true;
        blue = (uint8_t)(blue - 0x08);
    }
    part = (uint8_t)(blue | RD8(ZP->temp02));
    WR8(ZP->temp02, part);
    low = rd8((uint16_t)(EFFECT_COLOURS + x));
    high = rd8((uint16_t)(EFFECT_COLOURS + 1 + x));
    carry_out = (uint8_t)(high & 1);
    wr8((uint16_t)(EFFECT_COLOURS + 1 + x), (uint8_t)(high >> 1));
    low = (uint8_t)((low >> 1) | (carry_out << 7));
    red = (uint8_t)(low & 0xE0);
    if (red != 0) {
        last.v = (((red ^ 0x20) & (red ^ (uint8_t)(red - 0x20))) & 0x80) != 0;
        last.set = true;
        red = (uint8_t)(red - 0x20);
    }
    carry_out = (uint8_t)(red >> 7);
    red = (uint8_t)(red << 1);
    high = rd8((uint16_t)(EFFECT_COLOURS + 1 + x));
    wr8((uint16_t)(EFFECT_COLOURS + 1 + x), (uint8_t)((high << 1) | carry_out));
    part = RD8(ZP->temp02);
    wr8((uint16_t)(EFFECT_COLOURS + x), (uint8_t)(red | part));
    return last;
}

/*
 * @routine 0111:371 EndingEffectFade
 * @run 6C:8371
 * WHAT IT DOES: A palette effect that fades palettes down: once its timer has run down it counts a round, gives the slot back after the
 * eighth, and otherwise takes the list's palettes (an address of the VCE and a count each) and darkens every colour of them by one step
 * in each of its three components, writing them back to the VCE.
 * HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402 (or a table at $9442); the VCE read into $3C60 by a TAI,
 * changed word by word, and written back by a TIA.
 * WAITS: no.
 * Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X $20 when a palette
 * was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
 */
Exit EndingEffectFade(uint8_t slot)
{
    Exit exit;
    uint8_t timer, y, value, count, low, high, x;
    FadeV seen, v = { false, false };

    timer = rd8((uint16_t)(EFFECT_TIMER + slot));
    if (timer != 0) {
        return effect_exit(timer, dec_at((uint16_t)(EFFECT_TIMER + slot)));
    }
    inc_at((uint16_t)(EFFECT_COUNT + slot));
    count = rd8((uint16_t)(EFFECT_COUNT + slot));
    if (count >= 8) {
        clear_effect_slot(slot);
        exit = effect_exit(count, (uint8_t)(count - 8));
        exit.c = true;
        exit.c_kept = false;
        return exit;
    }
    select_effect_list(rd8((uint16_t)(EFFECT_LIST + slot)), 0x9442, 0x9402);
    y = 0;
    value = RD8_PTR_Y(ZP->temp00, y);
    wr8((uint16_t)(EFFECT_TIMER + slot), value);
    x = slot;
    y++;
    for (;;) {
        value = RD8_PTR_Y(ZP->temp00, y);
        if (value == 0xFF) {
            exit = effect_exit(0xFF, 0);
            exit.c = true;
            exit.c_kept = false;
            exit.x = x;
            exit.x_kept = false;
            exit.y = y;
            exit.y_kept = false;
            exit.v = v.v;
            exit.v_set = v.set;
            return exit;
        }
        WR8(ZP->temp08, value);
        wr8(VCE_ADDR_LO, value);
        y++;
        high = RD8_PTR_Y(ZP->temp00, y);
        WR8(ZP->temp09, high);
        wr8(VCE_ADDR_HI, high);
        y++;
        block_move_tai(VCE_DATA_LO, EFFECT_COLOURS, 0x20, high, x, y);
        for (x = 0; x < 0x20; x = (uint8_t)(x + 2)) {
            seen = fade_colour(x);
            if (seen.set)
                v = seen;
        }
        low = RD8(ZP->temp08);
        wr8(VCE_ADDR_LO, low);
        high = RD8(ZP->temp09);
        wr8(VCE_ADDR_HI, high);
        block_move_tia(EFFECT_COLOURS, VCE_DATA_LO, 0x20, high, x, y);
    }
}

void abi_EndingEffectFade(void)
{
    exit_shim(EndingEffectFade(cpu.x));
}

/*
 * @routine 0111:2DA EndingEffectDispatch
 * @run 6C:82DA
 * WHAT IT DOES: Sends a palette effect slot to the handler its type names: the table at $82EA holds one address per type, type 1 first.
 * HOW: Type less one doubled indexes the table; the word goes through zTemp00 and zTemp01; the jump is made with A the word's high byte,
 * Y the index, C bit 7 of type less one, X the slot. A handler this overlay's C does not hold runs as the original code.
 * WAITS: no.
 * Entry: A the type (not zero), X the slot. Exit: as the handler leaves them.
 * not checked: 2 of 400 cases jump to a type outside the table.
 */
Exit EndingEffectDispatch(uint8_t type, uint8_t slot)
{
    Exit exit = { 0 };
    uint8_t less, index, low, high, high_read;
    uint16_t target;
    bool carry;

    less = (uint8_t)(type - 1);
    index = (uint8_t)(less << 1);
    carry = (less & 0x80) != 0;
    low = rd8((uint16_t)(EFFECT_TABLE + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(EFFECT_TABLE + 1 + index));
    WR8(ZP->temp01, high);
    low = RD8(ZP->temp00);
    high_read = RD8(ZP->temp01);
    target = (uint16_t)(low | (high_read << 8));
    switch (target) {
    case 0x82F8: exit = EndingEffectList(slot); break;
    case 0x8371: exit = EndingEffectFade(slot); break;
    default:
        cpu_entry_shim(high, slot, index, carry);
        call_original(target);
        exit.from_cpu = true;
        return exit;
    }
    if (exit.a_kept) {
        exit.a = high;
        exit.a_kept = false;
    }
    if (exit.y_kept) {
        exit.y = index;
        exit.y_kept = false;
    }
    if (exit.c_kept) {
        exit.c = carry;
        exit.c_kept = false;
    }
    return exit;
}

void abi_EndingEffectDispatch(void)
{
    exit_shim(EndingEffectDispatch(cpu.a, cpu.x));
}

/* The registers the type 5 effect is entered with: X 3, C the compare's. */
static void effect_type5_shim(bool carry)
{
    cpu.x = 3;
    abi_flag(P_C, carry);
}

/* The end of the loop over the slots when the last handler ran as original code: X $FF from the DEX. */
static void effect_loop_end_shim(bool a_loaded)
{
    if (a_loaded) {
        cpu.a = 0;
    }
    cpu.x = 0xFF;
    abi_nz(0xFF);
}

/*
 * @routine 0111:2AE EndingEffectStep
 * @run 6C:82AE
 * WHAT IT DOES: One frame of the ending's palette effects: while the game is paused (bit 0 of zPaused) only slots 2 and $0B are run; otherwise,
 * unless slot 3 holds type 5 (which runs as the original code with X 3), every slot from $0B down to 0 that has a type is run through
 * EndingEffectDispatch.
 * HOW: zPaused; the types at $3C00; the slot kept in zRichterObject's byte ($20D9) across each dispatch.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N set, Z clear; A the last type loaded or what the last handler left; C, Y and V as the last handler left them.
 * not checked: the paused path is untraced in the draft, and types outside the table jump to other overlays.
 */
Exit EndingEffectStep(void)
{
    Exit exit = { 0 };
    Exit last;
    uint8_t type, slot, kept;
    bool have_last = false;
    uint8_t last_slot = 0;
    bool y_set = false, c_set = false, v_set = false, v = false, c = false;
    uint8_t y = 0, a = 0;

    if ((RD8(ZP->paused) & 1) != 0) {
        type = rd8((uint16_t)(EFFECT_TYPE + 2));
        jsr_push(0x82B5);
        if (type != 0) {
            exit = EndingEffectDispatch(type, 2);
            if (exit.from_cpu) {
                jsr_pop();
                return exit;
            }
        }
        jsr_pop();
        type = rd8((uint16_t)(EFFECT_TYPE + 0x0B));
        if (type == 0) {
            exit = effect_exit(0, 0);
            exit.x = 0x0B;
            exit.x_kept = false;
            return exit;
        }
        return EndingEffectDispatch(type, 0x0B);
    }
    type = rd8((uint16_t)(EFFECT_TYPE + 3));
    c = type >= 5;
    c_set = true;
    if (type == 5) {
        effect_type5_shim(c);
        call_original(0x856F);
        exit.from_cpu = true;
        return exit;
    }
    for (slot = 0x0B; (int8_t)slot >= 0; slot--) {
        type = rd8((uint16_t)(EFFECT_TYPE + slot));
        a = type;
        if (type != 0) {
            wr8(0x20D9, slot);
            jsr_push(0x82D4);
            last = EndingEffectDispatch(type, slot);
            jsr_pop();
            if (last.from_cpu) {
                have_last = true;
                last_slot = slot;
                exit = last;
            } else {
                have_last = false;
                a = last.a;
                y = last.y;
                y_set = true;
                c = last.c;
                c_set = true;
                if (last.v_set) {
                    v = last.v;
                    v_set = true;
                }
            }
            kept = rd8(0x20D9);
            slot = kept;
        }
    }
    if (have_last) {
        effect_loop_end_shim(last_slot != 0);
        exit.from_cpu = true;
        return exit;
    }
    exit.a = a;
    exit.x = 0xFF;
    exit.nz = 0xFF;
    exit.y = y;
    exit.y_kept = !y_set;
    exit.c = c;
    exit.c_kept = !c_set;
    exit.v = v;
    exit.v_set = v_set;
    return exit;
}

void abi_EndingEffectStep(void)
{
    exit_shim(EndingEffectStep());
}

/*
 * @routine 0111:003 EndingEffectEntry
 * @run 6C:8003
 * WHAT IT DOES: The palette effects' entry for a frame: runs EndingEffectStep.
 * HOW: A jump to EndingEffectStep.
 * WAITS: no.
 * Entry: nothing. Exit: as EndingEffectStep.
 * not checked: as EndingEffectStep.
 */
Exit EndingEffectEntry(void)
{
    return EndingEffectStep();
}

void abi_EndingEffectEntry(void)
{
    exit_shim(EndingEffectEntry());
}

void abi_EndingCommandStartList(void)
{
    exit_shim(EndingCommandStartList());
}

void abi_EndingCommandStartScript1(void)
{
    exit_shim(EndingCommandStartScript1());
}

void abi_EndingCommandStartScript2(void)
{
    exit_shim(EndingCommandStartScript2());
}

void abi_EndingCommandCdPlay(void)
{
    exit_shim(EndingCommandCdPlay(cpu.x));
}

void abi_EndingCommandPlaceKind(void)
{
    exit_shim(EndingCommandPlaceKind());
}

void abi_EndingCommandPlaceNext(void)
{
    exit_shim(EndingCommandPlaceNext());
}

void abi_EndingCommandPlaceFrame(void)
{
    exit_shim(EndingCommandPlaceFrame());
}

void abi_EndingCommandPlaceTimed(void)
{
    exit_shim(EndingCommandPlaceTimed());
}

void abi_EndingCommandLoadBanks(void)
{
    exit_shim(EndingCommandLoadBanks());
}
