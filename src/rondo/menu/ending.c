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
 * Entry: X the object, the caller's frame on the stack. Exit: N and Z from the timer, Z set when it has run out;
 * A, X, Y and C kept.
 */
bool EndingHoldTimer(uint8_t object)
{
    uint8_t left = DEC8(OBJ->life_timer[object]);

    if (left != 0) {
        (void)pull8();
        (void)pull8();
        return false;
    }
    return true;
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
 * @run 6B:80EF
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
 * @run 6B:81F6
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
