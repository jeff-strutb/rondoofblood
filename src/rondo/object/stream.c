/*
 * The level's object stream: records sorted by position along the level, walked
 * forward and back as the screen scrolls, each acted on as it comes in range. A
 * record is ten bytes: position (word), handler, four arguments, the event's flags
 * and its id, as zTemp12-13 and zTemp08-0F hold them while it is read.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

enum {
    STREAM_RECORD_BYTES = 0x0A, STREAM_END_MARK = 0xFF, SLOT_RESERVED = 0x10,
    AXIS_HORIZONTAL_LOCKED = 0x02, AXIS_VERTICAL = 0x01, OBJECT_MARKED = 0x80
};

typedef struct {
    Alu r;
    uint8_t y;
    bool nz_from_x;  /* the path ended on PLX, so N and Z are the caller's X */
} StreamExit;

static void abi_exit_stream(StreamExit e)
{
    abi_alu(e.r);
    cpu.y = e.y;
    if (e.nz_from_x)
        abi_nz(cpu.x);
}

/* The record the walk stands on: its position, into zTemp12-13. */
static void read_record_position(void)
{
    uint8_t byte = rd8_zp_y(zTemp10, 0);

    wr8(zTemp12, byte);
    byte = rd8_zp_y(zTemp10, 1);
    wr8(zProduct0, byte);
}

/*
 * @routine 0046:723 StreamDistance
 * @run 69:5F23
 * WHAT IT DOES: Gives how far the record is from the walk's position, as a word.
 * HOW: Subtracts the walk's position, and negates a negative result.
 * Entry: zTemp12, zProduct0 record position, zTemp04-05 walk position. Exit:
 * zProduct1-2 the distance; A its high byte, N, Z, C and V from the last add or
 * subtract. X and Y kept.
 */
Alu StreamDistance(void)
{
    uint8_t a = rd8(zTemp12);
    uint8_t b = rd8(zTemp04);
    Alu r = sbc8(a, b, true);

    wr8(zProduct1, r.a);
    a = rd8(zProduct0);
    b = rd8(zTemp05);
    r = sbc8(a, b, r.c);
    wr8(zProduct2, r.a);
    if (r.c)
        return r;
    r = adc8((uint8_t)~rd8(zProduct1), 1, false);
    wr8(zProduct1, r.a);
    r = adc8((uint8_t)~rd8(zProduct2), 0, r.c);
    wr8(zProduct2, r.a);
    return r;
}

void abi_StreamDistance(void)
{
    abi_alu(StreamDistance());
}

/* Is the record close enough to act on? zTemp06 is how close. */
static bool record_in_range(Alu *distance)
{
    uint8_t high;
    uint8_t low;
    uint8_t range;

    *distance = StreamDistance();
    high = rd8(zProduct2);
    if (high != 0)
        return false;
    low = rd8(zProduct1);
    range = rd8(zTemp06);
    return low < range;
}

/*
 * @routine 0047:095 RecordIfMarked
 * @run 6A:6095
 * WHAT IT DOES: The end of the record handlers: if this record's flags ask for it,
 * marks its event.
 * HOW: Bit 7 of zTemp0E, event in zTemp0F.
 * Entry: zTemp0E, zTemp0F. Exit: unmarked, A zTemp0E, N and Z from it, C, Y kept.
 * Else as RecordEvent. X and V kept.
 */
static StreamExit record_if_marked(uint8_t y)
{
    uint8_t flags = rd8(zTemp0E);
    StreamExit e = { { 0 }, y, false };

    if (!(flags & OBJECT_MARKED)) {
        e.r.a = flags;
        e.r.nz = flags;
        e.r.c_kept = true;
        return e;
    }
    flags = rd8(zTemp0F);
    e.r.a = RecordEvent(flags, &e.y);
    e.r.nz = e.r.a;
    e.r.c = (flags & 7) == 0;
    return e;
}

void abi_RecordIfMarked(void)
{
    abi_exit_stream(record_if_marked(cpu.y));
}

/* One axis of a new object's place: position less scroll, word from the bytes. */
static Alu place_axis(uint16_t pos_lo, uint16_t pos_hi, uint16_t scroll_lo,
                      uint16_t scroll_hi, uint16_t field_lo, uint16_t field_hi)
{
    uint8_t a = rd8(pos_lo);
    uint8_t b = rd8(scroll_lo);
    Alu r = sbc8(a, b, true);

    wr8(field_lo, r.a);
    a = rd8(pos_hi);
    b = rd8(scroll_hi);
    r = sbc8(a, b, r.c);
    wr8(field_hi, r.a);
    return r;
}

/* Fills a slot from the record; the slot is already known usable. */
static Alu place_new_object(uint8_t slot)
{
    uint8_t byte;
    uint8_t axis;
    Alu r;

    ClearObject(slot);
    byte = rd8(zTemp0C);
    wr8((uint16_t)(OBJ_2750 + slot), byte);
    axis = rd8(LevelAxis);
    if (axis == AXIS_VERTICAL) {
        place_axis(zTemp12, zProduct0, ScrollYLo, ScrollYHiScreen,
                   (uint16_t)(OBJ_Y_LO + slot), (uint16_t)(OBJ_Y_HI + slot));
        r = place_axis(zTemp09, zTemp0A, ScrollXLo, ScrollXHiScreen,
                       (uint16_t)(OBJ_X_LO + slot), (uint16_t)(OBJ_X_HI + slot));
    } else {
        place_axis(zTemp12, zProduct0, ScrollXLo, ScrollXHiScreen,
                   (uint16_t)(OBJ_X_LO + slot), (uint16_t)(OBJ_X_HI + slot));
        r = place_axis(zTemp09, zTemp0A, ScrollYOffset, ScrollYHiScreen,
                       (uint16_t)(OBJ_Y_LO + slot), (uint16_t)(OBJ_Y_HI + slot));
        axis = rd8(LevelAxis);
        r.c = axis >= AXIS_HORIZONTAL_LOCKED;
        if (axis != AXIS_HORIZONTAL_LOCKED) {
            uint8_t sign_byte;

            wr8(zTemp09, 0);
            sign_byte = rd8(ScrollYLo);
            if (sign_byte & 0x80) {
                sign_byte = rd8(zTemp09);
                wr8(zTemp09, (uint8_t)(sign_byte - 1));
            }
            byte = rd8((uint16_t)(OBJ_Y_LO + slot));
            sign_byte = rd8(ScrollYLo);
            r = sbc8(byte, sign_byte, true);
            wr8((uint16_t)(OBJ_Y_LO + slot), r.a);
            byte = rd8((uint16_t)(OBJ_Y_HI + slot));
            sign_byte = rd8(zTemp09);
            r = sbc8(byte, sign_byte, r.c);
            wr8((uint16_t)(OBJ_Y_HI + slot), r.a);
        }
    }
    byte = rd8(zTemp0D);
    wr8((uint16_t)(OBJ_3040 + slot), byte);
    byte = rd8(zTemp0E);
    wr8((uint16_t)(OBJ_FLAGS_2F38 + slot), byte);
    byte = rd8(zTemp0F);
    wr8((uint16_t)(OBJ_EVENT_ID + slot), byte);
    r.a = byte;
    return r;
}

/*
 * @routine 0046:7F9 StreamSpawnIfFree
 * @run 69:5FF9
 * WHAT IT DOES: Record handler 0: starts an object in the record's slot, if that slot
 * is empty.
 * HOW: The slot's type must be zero. The object is placed at the record's position
 * less the scroll, on the axis the level runs along.
 * Entry: the record in zTemp09-0F, zTemp12, zProduct0. Exit: A zTemp0F (or the
 * slot's type, if taken), X and Y kept, N and Z from X, C and V from the last step.
 */
Alu StreamSpawnIfFree(void)
{
    uint8_t slot = rd8(zTemp0B);
    uint8_t type = rd8((uint16_t)(OBJ_2750 + slot));
    Alu r = { 0 };

    if (type == 0)
        return place_new_object(slot);
    r.a = type;
    r.c_kept = true;
    return r;
}

static void abi_exit_handler(Alu r, uint8_t x)
{
    abi_alu(r);
    abi_nz(x);
}

void abi_StreamSpawnIfFree(void)
{
    abi_exit_handler(StreamSpawnIfFree(), cpu.x);
}

/*
 * @routine 0047:004 StreamPlaceObject
 * @run 6A:6004
 * WHAT IT DOES: The body of the spawning record handlers, entered with the slot in X.
 * HOW: As StreamSpawnIfFree, past its check.
 * Entry: X slot, on the stack the caller's X. Exit: as StreamSpawnIfFree.
 */
void abi_StreamPlaceObject(void)
{
    abi_alu(place_new_object(cpu.x));
}

/*
 * @routine 0047:08C StreamStartScript
 * @run 6A:608C
 * WHAT IT DOES: Record handler 1: starts a script slot, kind 1.
 * HOW: Slot and script from the record; then RecordIfMarked.
 * Entry: zTemp09 script, zTemp0A slot. Exit: as RecordIfMarked, with Y the script
 * if nothing else set it.
 */
StreamExit StreamStartScript(void)
{
    uint8_t script = rd8(zTemp09);
    uint8_t slot = rd8(zTemp0A);

    StartScriptSlotKind1(script, slot);
    return record_if_marked(script);
}

void abi_StreamStartScript(void)
{
    abi_exit_stream(StreamStartScript());
}

/*
 * @routine 0047:09F StreamStartListKind2
 * @run 6A:609F
 * WHAT IT DOES: Record handler 2: starts a list channel as kind 2, only while the
 * full-walk flag is up: that is, when a level section is first entered.
 * HOW: Tests LevelFullWalk; then list and channel from the record, and RecordIfMarked.
 * Entry: zTemp09 list, zTemp0A channel. Exit: flag down, A 0, N 0, Z 1. Else as
 * RecordIfMarked.
 */
StreamExit StreamStartListKind2(uint8_t y)
{
    StreamExit e = { { 0 }, y, false };
    uint8_t list;
    uint8_t channel;

    if (rd8(LevelFullWalk) == 0) {
        e.r.c_kept = true;
        return e;
    }
    list = rd8(zTemp09);
    channel = rd8(zTemp0A);
    StartListChannelKind2(list, channel);
    return record_if_marked(y);
}

void abi_StreamStartListKind2(void)
{
    abi_exit_stream(StreamStartListKind2(cpu.y));
}

/*
 * @routine 0047:0B0 StreamStartListKind3
 * @run 6A:60B0
 * WHAT IT DOES: Record handler 3: starts a list channel as kind 3.
 * HOW: List and channel from the record; then RecordIfMarked.
 * Entry: zTemp09 list, zTemp0A channel. Exit: as RecordIfMarked.
 */
StreamExit StreamStartListKind3(uint8_t y)
{
    uint8_t list = rd8(zTemp09);
    uint8_t channel = rd8(zTemp0A);

    StartListChannelKind3(list, channel);
    return record_if_marked(y);
}

void abi_StreamStartListKind3(void)
{
    abi_exit_stream(StreamStartListKind3(cpu.y));
}

/*
 * @routine 0047:0BB StreamClearObject
 * @run 6A:60BB
 * WHAT IT DOES: Record handler 4: empties an object slot.
 * HOW: ClearObject on the record's slot; then RecordIfMarked.
 * Entry: zTemp09 slot. Exit: as RecordIfMarked.
 */
StreamExit StreamClearObject(uint8_t y)
{
    ClearObject(rd8(zTemp09));
    return record_if_marked(y);
}

void abi_StreamClearObject(void)
{
    abi_exit_stream(StreamClearObject(cpu.y));
}

/*
 * @routine 0047:0C4 StreamSpawnIfOther
 * @run 6A:60C4
 * WHAT IT DOES: Record handler 5: starts an object in the record's slot unless that
 * slot already holds the object for this same event, so one spawned earlier stays.
 * HOW: Empty, or a different event id, places the object.
 * Entry: as StreamSpawnIfFree. Exit: placed, as StreamSpawnIfFree. Same event: A the
 * event id, N and Z from X, C 1. X and Y kept. An inner entry at $6004 places directly.
 */
Alu StreamSpawnIfOther(void)
{
    uint8_t slot = rd8(zTemp0B);
    uint8_t type = rd8((uint16_t)(OBJ_2750 + slot));
    uint8_t mine;
    uint8_t theirs;
    Alu r = { 0 };

    if (type == 0)
        return place_new_object(slot);
    mine = rd8((uint16_t)(OBJ_EVENT_ID + slot));
    theirs = rd8(zTemp0F);
    if (mine != theirs)
        return place_new_object(slot);
    r.a = mine;
    r.c = true;
    return r;
}

void abi_StreamSpawnIfOther(void)
{
    abi_exit_handler(StreamSpawnIfOther(), cpu.x);
}

/*
 * @routine 0047:0DB StreamCallOverlay
 * @run 6A:60DB
 * WHAT IT DOES: Record handler 6: calls the code the record points to.
 * HOW: JMP through the pointer in zTemp09-0A; then RecordIfMarked.
 * Entry: zTemp09-0A code. Exit: as RecordIfMarked.
 */
StreamExit StreamCallOverlay(uint8_t y)
{
    uint8_t x = cpu.x;
    uint8_t low = rd8(zTemp09);
    uint8_t high = rd8(zTemp0A);

    cpu.y = y;
    call_original((uint16_t)(low | (high << 8)));
    cpu.x = x;
    return record_if_marked(cpu.y);
}

void abi_StreamCallOverlay(void)
{
    abi_exit_stream(StreamCallOverlay(cpu.y));
}

/*
 * @routine 0047:0E3 JumpThroughRecord
 * @run 6A:60E3
 * WHAT IT DOES: The jump StreamCallOverlay reaches a record's code by.
 * HOW: JMP [zTemp09].
 * Entry: zTemp09-0A code. Exit: as that code leaves it.
 */
void abi_JumpThroughRecord(void)
{
    uint8_t low = rd8(zTemp09);
    uint8_t high = rd8(zTemp0A);

    call_original((uint16_t)(low | (high << 8)));
}

/*
 * @routine 0046:7A5 RunStreamRecord
 * @run 69:5FA5
 * WHAT IT DOES: Acts on the record the walk stands on, unless its event has already
 * happened.
 * HOW: Copies its eight bytes to zero page, the slot with $10 added past the player's
 * own slots. Tests the event's bit in EventFlags, and goes through the handler table,
 * whose entry it keeps in zTemp00-01.
 * Entry: zTemp10-11 the record. Exit: event done, A the bit found, N and Z from it, Y
 * its byte, C as BitOfIndex. Else as the handler, with Y its table index if the
 * handler leaves it. X kept. Clobbers zTemp00-01, zTemp08-0F.
 */
StreamExit RunStreamRecord(uint8_t y)
{
    static const uint16_t fields[] = {
        zTemp08, zTemp09, zTemp0A, zTemp0B, zTemp0C, zTemp0D, zTemp0E, zTemp0F
    };
    StreamExit e = { { 0 }, y, false };
    uint8_t byte = 0;
    uint8_t mask;
    uint8_t handler;
    uint8_t entry_lo;
    uint8_t entry_hi;

    for (unsigned i = 0; i < sizeof fields / sizeof fields[0]; i++) {
        byte = rd8_zp_y(zTemp10, (uint8_t)(2 + i));
        if (fields[i] == zTemp0B) {
            Alu slot = adc8(byte, SLOT_RESERVED, false);

            e.r.v = slot.v;
            e.r.v_set = true;
            byte = slot.a;
        }
        wr8(fields[i], byte);
    }
    mask = BitOfIndex(byte, &e.y);
    e.r.a = mask & rd8((uint16_t)(EventFlags + e.y));
    e.r.nz = e.r.a;
    e.r.c = (byte & 7) == 0;
    if (e.r.a != 0)
        return e;
    handler = rd8(zTemp08);
    e.y = (uint8_t)(handler << 1);
    entry_lo = rd8((uint16_t)(StreamHandlers + e.y));
    wr8(zTemp00, entry_lo);
    entry_hi = rd8((uint16_t)(StreamHandlers + 1 + e.y));
    wr8(zTemp01, entry_hi);
    /* JMP [zTemp00] reads the pointer back. */
    entry_lo = rd8(zTemp00);
    entry_hi = rd8(zTemp01);
    switch (handler) {
    case 0:
        e.r = StreamSpawnIfFree();
        e.nz_from_x = true;
        return e;
    case 1:
        return StreamStartScript();
    case 2:
        return StreamStartListKind2(e.y);
    case 3:
        return StreamStartListKind3(e.y);
    case 4:
        return StreamClearObject(e.y);
    case 5:
        e.r = StreamSpawnIfOther();
        e.nz_from_x = true;
        return e;
    case 6:
        return StreamCallOverlay(e.y);
    default:
        /* Past the table: its bytes are the entry, as on the original. */
        cpu.y = e.y;
        call_original((uint16_t)(entry_lo | (entry_hi << 8)));
        e.y = cpu.y;
        e.r.c_kept = true;
        e.r.v_set = false;
        e.r.a = cpu.a;
        e.r.nz = cpu.a;
        return e;
    }
}

void abi_RunStreamRecord(void)
{
    abi_exit_stream(RunStreamRecord(cpu.y));
}

/* A step of the walk: the record pointer moved, kept, and the next read. */
static void step_forward(void)
{
    uint8_t low = rd8(zTemp10);
    Alu r = adc8(low, STREAM_RECORD_BYTES, false);

    wr8(zTemp10, r.a);
    wr8(LevelStreamLo, r.a);
    low = rd8(zTemp11);
    r = adc8(low, 0, r.c);
    wr8(zTemp11, r.a);
    wr8(LevelStreamHi, r.a);
    read_record_position();
}

/* A compare of A with a byte, as the exit flags take it. */
static void note_compare(Alu *r, uint8_t a, uint8_t b)
{
    r->a = a;
    r->nz = (uint8_t)(a - b);
    r->c = a >= b;
    r->c_kept = false;
}

/*
 * @routine 0046:6CC StreamWalkForward
 * @run 69:5ECC
 * WHAT IT DOES: Walks the stream forward from the record it stands on: acts on each
 * record in range, skips those behind, stops at the first too far ahead or at the
 * end mark, where the stream position stays on the last real record.
 * HOW: StreamDistance against the range in zTemp06, then the position compared
 * with the walk's position.
 * Entry: zTemp10-11 the record, zTemp12, zProduct0 its position, zTemp04-05 the
 * position, zTemp06 the range. Exit: A, Y and flags as the last step left them: Y 1
 * once a record has been stepped to. X kept. Clobbers what the handlers do.
 */
StreamExit StreamWalkForward(uint8_t y)
{
    StreamExit e = { { 0 }, y, false };

    for (;;) {
        Alu distance;

        if (record_in_range(&distance)) {
            e = RunStreamRecord(e.y);
            e.nz_from_x = false;
        } else {
            uint8_t high = rd8(zProduct0);
            uint8_t low = rd8(zTemp12);
            uint8_t wanted;

            e.r = distance;
            if ((uint8_t)(high & low) == STREAM_END_MARK) {
                low = rd8(zTemp10);
                e.r = sbc8(low, STREAM_RECORD_BYTES, true);
                wr8(LevelStreamLo, e.r.a);
                low = rd8(zTemp11);
                e.r = sbc8(low, 0, e.r.c);
                wr8(LevelStreamHi, e.r.a);
                return e;
            }
            high = rd8(zProduct0);
            wanted = rd8(zTemp05);
            note_compare(&e.r, high, wanted);
            if (high > wanted)
                return e;
            if (high == wanted) {
                low = rd8(zTemp12);
                wanted = rd8(zTemp04);
                note_compare(&e.r, low, wanted);
                if (low >= wanted)
                    return e;
            }
        }
        step_forward();
        e.y = 1;
    }
}

void abi_StreamWalkForward(void)
{
    abi_exit_stream(StreamWalkForward(cpu.y));
}

/*
 * @routine 0046:744 StreamWalkBack
 * @run 69:5F44
 * WHAT IT DOES: Walks the stream backward from the record it stands on, as
 * StreamWalkForward does forward: stops at the first record behind the position, at
 * the end mark, or before the stream's start, where a record's position has its top
 * bit set.
 * HOW: The same tests the other way. The stream position moves only onto a record
 * that passes the top-bit test.
 * Entry and exit: as StreamWalkForward.
 */
StreamExit StreamWalkBack(uint8_t y)
{
    StreamExit e = { { 0 }, y, false };

    for (;;) {
        Alu distance;
        uint8_t byte;

        if (record_in_range(&distance)) {
            e = RunStreamRecord(e.y);
            e.nz_from_x = false;
        } else {
            uint8_t high = rd8(zProduct0);
            uint8_t low = rd8(zTemp12);
            uint8_t wanted;

            e.r = distance;
            if ((uint8_t)(high & low) == STREAM_END_MARK) {
                low = rd8(zTemp10);
                e.r = adc8(low, STREAM_RECORD_BYTES, false);
                wr8(LevelStreamLo, e.r.a);
                low = rd8(zTemp11);
                e.r = adc8(low, 0, e.r.c);
                wr8(LevelStreamHi, e.r.a);
                return e;
            }
            high = rd8(zProduct0);
            wanted = rd8(zTemp05);
            note_compare(&e.r, high, wanted);
            if (high < wanted)
                return e;
            if (high == wanted) {
                low = rd8(zTemp12);
                wanted = rd8(zTemp04);
                note_compare(&e.r, low, wanted);
                if (low < wanted)
                    return e;
            }
        }
        byte = rd8(zTemp10);
        e.r = sbc8(byte, STREAM_RECORD_BYTES, true);
        wr8(zTemp10, e.r.a);
        byte = rd8(zTemp11);
        e.r = sbc8(byte, 0, e.r.c);
        wr8(zTemp11, e.r.a);
        e.y = 1;
        byte = rd8_zp_y(zTemp10, 1);
        e.r.a = byte;
        e.r.nz = byte;
        if (byte & 0x80)
            return e;
        byte = rd8(zTemp10);
        wr8(LevelStreamLo, byte);
        byte = rd8(zTemp11);
        wr8(LevelStreamHi, byte);
        read_record_position();
    }
}

void abi_StreamWalkBack(void)
{
    abi_exit_stream(StreamWalkBack(cpu.y));
}

/* The record pointer, and its position: every walk starts here. Leaves Y 1. */
static void walk_start(void)
{
    uint8_t byte = rd8(LevelStreamLo);

    wr8(zTemp10, byte);
    byte = rd8(LevelStreamHi);
    wr8(zTemp11, byte);
    read_record_position();
}

/*
 * @routine 0046:5E7 StreamWalkLines
 * @run 69:5DE7
 * WHAT IT DOES: Walks the stream line by line across a span, a walk at every step.
 * HOW: From the stream position, StreamWalkForward or Back by the sign of the step;
 * then adds the step to the position and counts the word in WorkByteA and B down.
 * Entry: zTemp04-05 position, zTemp06 range, StreamStep the step, WorkByteA-B the
 * count. Exit: A 0, N 0, Z 1, C from the count's borrow, V from the last subtract, Y
 * as the last walk left it. X kept.
 */
StreamExit StreamWalkLines(void)
{
    for (;;) {
        StreamExit e;
        uint8_t byte;
        uint8_t other;

        walk_start();
        byte = rd8(StreamStepLo);
        e = (byte & 0x80) ? StreamWalkBack(1) : StreamWalkForward(1);
        byte = rd8(zTemp04);
        other = rd8(StreamStepLo);
        e.r = adc8(byte, other, false);
        wr8(zTemp04, e.r.a);
        byte = rd8(zTemp05);
        other = rd8(StreamStepHi);
        e.r = adc8(byte, other, e.r.c);
        wr8(zTemp05, e.r.a);
        byte = rd8(WorkByteB);
        e.r = sbc8(byte, 1, true);
        wr8(WorkByteB, e.r.a);
        byte = rd8(WorkByteA);
        e.r = sbc8(byte, 0, e.r.c);
        wr8(WorkByteA, e.r.a);
        other = rd8(WorkByteB);
        e.r.a |= other;
        e.r.nz = e.r.a;
        e.nz_from_x = false;
        if (e.r.a == 0)
            return e;
    }
}

void abi_StreamWalkLines(void)
{
    abi_exit_stream(StreamWalkLines());
}

/* One walk of a full pass, in the bus order the assembly stores its arguments. */
static StreamExit walk_full_span(uint8_t step_lo, uint8_t step_hi, uint16_t scroll_hi,
                                 uint16_t scroll_lo, uint8_t count_hi)
{
    uint8_t byte;

    wr8(StreamStepLo, step_lo);
    wr8(StreamStepHi, step_hi);
    byte = rd8(scroll_hi);
    wr8(zTemp05, byte);
    byte = rd8(scroll_lo);
    wr8(zTemp04, byte);
    wr8(WorkByteA, count_hi);
    wr8(WorkByteB, 0x41);
    wr8(zTemp06, 1);
    return StreamWalkLines();
}

/*
 * @routine 0046:53E StreamFullWalk
 * @run 69:5D3E
 * WHAT IT DOES: Walks the stream across the screen and back when a level section is
 * entered, so everything that belongs on screen comes into being, with the full-walk
 * flag up for handlers that act only then.
 * HOW: A forward walk of $141 lines and a backward walk of $41 from the scroll on the
 * axis the level runs along.
 * Entry: LevelAxis. Exit: as StreamWalkLines; the flag down. Clobbers zTemp04-06.
 */
StreamExit StreamFullWalk(void)
{
    uint8_t axis = rd8(LevelAxis);
    uint16_t scroll_hi = axis == AXIS_VERTICAL ? ScrollYHiScreen : ScrollXHiScreen;
    uint16_t scroll_lo = axis == AXIS_VERTICAL ? ScrollYLo : ScrollXLo;
    StreamExit e;

    wr8(LevelFullWalk, 1);
    walk_full_span(1, 0, scroll_hi, scroll_lo, 1);
    e = walk_full_span(0xFF, 0xFF, scroll_hi, scroll_lo, 0);
    wr8(LevelFullWalk, 0);
    return e;
}

void abi_StreamFullWalk(void)
{
    abi_exit_stream(StreamFullWalk());
}

/* The start of a walk one edge past the screen, with the speed as its range. */
static StreamExit edge_walk_forward(uint16_t scroll_lo, uint16_t scroll_hi, uint16_t speed,
                                    uint8_t y)
{
    uint8_t byte = rd8(scroll_lo);
    Alu r = adc8(byte, 0x40, false);

    wr8(zTemp04, r.a);
    byte = rd8(scroll_hi);
    r = adc8(byte, 1, r.c);
    wr8(zTemp05, r.a);
    byte = rd8(speed);
    wr8(zTemp06, byte);
    return StreamWalkForward(y);
}

static StreamExit edge_walk_back(uint16_t scroll_lo, uint16_t scroll_hi, uint16_t speed,
                                 uint8_t y)
{
    uint8_t byte = rd8(scroll_lo);
    Alu r = adc8(byte, 0xC0, false);

    wr8(zTemp04, r.a);
    byte = rd8(scroll_hi);
    r = adc8(byte, 0xFF, r.c);
    if (r.a & 0x80) {
        StreamExit before_start = { r, y, false };

        return before_start;
    }
    wr8(zTemp05, r.a);
    byte = (uint8_t)(~rd8(speed) + 1);
    wr8(zTemp06, byte);
    return StreamWalkBack(y);
}

/*
 * @routine 0046:664 StreamEdgeRight
 * @run 69:5E64
 * WHAT IT DOES: Scrolling right: walks the stream at the screen's right edge.
 * HOW: Scroll plus $140 as the position, the scroll speed as the range.
 * Entry: zTemp10-11 and the record position set. Exit: as StreamWalkForward.
 */
void abi_StreamEdgeRight(void)
{
    abi_exit_stream(edge_walk_forward(ScrollXLo, ScrollXHiScreen, ScrollSpeedX, cpu.y));
}

/*
 * @routine 0046:67B StreamEdgeLeft
 * @run 69:5E7B
 * WHAT IT DOES: Scrolling left: walks the stream back at the screen's left edge,
 * unless that lies before the level's start.
 * HOW: Scroll less $40 as the position, the scroll speed negated as the range.
 * Entry: as StreamEdgeRight. Exit: as StreamWalkBack; before the start, A the
 * position's high byte, N 1, Z 0, C and V from its add.
 */
void abi_StreamEdgeLeft(void)
{
    abi_exit_stream(edge_walk_back(ScrollXLo, ScrollXHiScreen, ScrollSpeedX, cpu.y));
}

/*
 * @routine 0046:698 StreamEdgeDown
 * @run 69:5E98
 * WHAT IT DOES: Scrolling down: walks the stream at the screen's bottom edge.
 * HOW: As StreamEdgeRight on Y.
 * Entry and exit: as StreamEdgeRight.
 */
void abi_StreamEdgeDown(void)
{
    abi_exit_stream(edge_walk_forward(ScrollYLo, ScrollYHiScreen, ScrollSpeedY, cpu.y));
}

/*
 * @routine 0046:6AF StreamEdgeUp
 * @run 69:5EAF
 * WHAT IT DOES: Scrolling up: walks the stream back at the screen's top edge, unless
 * that lies before the level's start.
 * HOW: As StreamEdgeLeft on Y.
 * Entry and exit: as StreamEdgeLeft.
 */
void abi_StreamEdgeUp(void)
{
    abi_exit_stream(edge_walk_back(ScrollYLo, ScrollYHiScreen, ScrollSpeedY, cpu.y));
}

/*
 * @routine 0046:62E StreamScrollStep
 * @run 69:5E2E
 * WHAT IT DOES: Each frame, walks the stream at whichever screen edge the scroll moved
 * toward.
 * HOW: Picks the axis the level runs along and goes by the sign of its scroll speed;
 * no speed returns.
 * Entry: nothing. Exit: as the walk; no speed, A 0, N 0, Z 1, Y 1. C and V from the
 * compare and what came before.
 */
StreamExit StreamScrollStep(void)
{
    uint8_t axis;
    uint8_t speed;
    StreamExit none = { { 0 }, 1, false };

    walk_start();
    axis = rd8(LevelAxis);
    none.r.c = axis >= AXIS_VERTICAL;
    if (axis == AXIS_VERTICAL) {
        speed = rd8(ScrollSpeedY);
        if (speed == 0)
            return none;
        if (speed & 0x80)
            return edge_walk_back(ScrollYLo, ScrollYHiScreen, ScrollSpeedY, 1);
        return edge_walk_forward(ScrollYLo, ScrollYHiScreen, ScrollSpeedY, 1);
    }
    speed = rd8(ScrollSpeedX);
    if (speed == 0)
        return none;
    if (speed & 0x80)
        return edge_walk_back(ScrollXLo, ScrollXHiScreen, ScrollSpeedX, 1);
    return edge_walk_forward(ScrollXLo, ScrollXHiScreen, ScrollSpeedX, 1);
}

void abi_StreamScrollStep(void)
{
    abi_exit_stream(StreamScrollStep());
}
