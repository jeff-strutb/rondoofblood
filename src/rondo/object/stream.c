/*
 * The level's object stream: records sorted by position along the level, walked
 * forward and back as the screen scrolls, each acted on as it comes in range. A
 * record is ten bytes: position (word), handler, four arguments, the event's flags
 * and its id, as zTemp12-13 and zTemp08-0F hold them while it is read.
 *
 * The bodies deal in values. What the assembly leaves in A and the flags when a routine
 * ends is worked out in the *_exit_shim functions the wrappers call, from how the routine
 * ended and the RAM it left.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

enum {
    STREAM_RECORD_BYTES = 0x0A, STREAM_END_MARK = 0xFF, SLOT_RESERVED = 0x10,
    AXIS_HORIZONTAL_LOCKED = 0x02, AXIS_VERTICAL = 0x01, OBJECT_MARKED = 0x80,
    EDGE_AHEAD = 0x0140, EDGE_BEHIND = 0xFFC0, FULL_WALK_AHEAD = 0x0141, FULL_WALK_BEHIND = 0x0041,
    RECORD_HANDLERS = 7, RECORD_ARGUMENTS = 2, RECORD_SLOT_BYTE = 5
};

static uint16_t read_word(uint16_t lo, uint16_t hi)
{
    return (uint16_t)(rd8(lo) | (rd8(hi) << 8));
}

static void write_word(uint16_t lo, uint16_t hi, uint16_t value)
{
    wr8(lo, (uint8_t)value);
    wr8(hi, (uint8_t)(value >> 8));
}

static uint16_t stream_record_at(void)
{
    return read_word(zTemp10, zTemp11);
}

/* The record the walk stands on: its position, into zTemp12 and zProduct0. */
static void read_record_position(void)
{
    uint16_t record = stream_record_at();

    wr8(zTemp12, rd8(record));
    wr8(zProduct0, rd8((uint16_t)(record + 1)));
}

static void word_add_flags_shim(uint16_t before, uint16_t amount)
{
    uint16_t sum = (uint16_t)(before + amount);

    abi_flag(P_C, sum < before);
    abi_flag(P_V, (~(before ^ amount) & (before ^ sum) & 0x8000) != 0);
}

static void word_subtract_flags_shim(uint16_t before, uint16_t amount)
{
    uint16_t difference = (uint16_t)(before - amount);

    abi_flag(P_C, before >= amount);
    abi_flag(P_V, ((before ^ amount) & (before ^ difference) & 0x8000) != 0);
}

/* A compare leaves A, N, Z and C; V stands. */
static void compare_exit_shim(uint8_t a, uint8_t b)
{
    cpu.a = a;
    abi_nz((uint8_t)(a - b));
    abi_flag(P_C, a >= b);
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
uint16_t StreamDistance(void)
{
    uint16_t record = read_word(zTemp12, zProduct0);
    uint16_t walk = read_word(zTemp04, zTemp05);
    uint16_t distance = record >= walk ? (uint16_t)(record - walk) : (uint16_t)(walk - record);

    write_word(zProduct1, zProduct2, distance);
    return distance;
}

/*
 * C and V as StreamDistance leaves them. A negated word carries out only from zero, which
 * a borrow rules out, and overflows only on $8000.
 */
static void distance_flags_shim(void)
{
    uint16_t record = read_word(zTemp12, zProduct0);
    uint16_t walk = read_word(zTemp04, zTemp05);

    if (record >= walk) {
        word_subtract_flags_shim(record, walk);
    } else {
        abi_flag(P_C, false);
        abi_flag(P_V, (uint16_t)(walk - record) == 0x8000);
    }
}

void abi_StreamDistance(void)
{
    uint16_t distance = StreamDistance();

    cpu.a = (uint8_t)(distance >> 8);
    abi_nz(cpu.a);
    distance_flags_shim();
}

/* Is the record close enough to act on? zTemp06 is how close. */
static bool record_in_range(void)
{
    return StreamDistance() < rd8(zTemp06);
}

/* Whether a handler recorded its event, and the bit and byte that event is in. */
typedef struct {
    bool recorded;
    uint8_t mask;
    uint8_t byte;
} MarkedEvent;

/*
 * @routine 0047:095 RecordIfMarked
 * @run 6A:6095
 * WHAT IT DOES: The end of the record handlers: if this record's flags ask for it,
 * marks its event.
 * HOW: Bit 7 of zTemp0E, event in zTemp0F.
 * Entry: zTemp0E, zTemp0F. Exit: unmarked, A zTemp0E, N and Z from it, C, Y kept.
 * Else as RecordEvent. X and V kept.
 */
MarkedEvent RecordIfMarked(void)
{
    MarkedEvent e = { false, 0, 0 };

    if (rd8(zTemp0E) & OBJECT_MARKED) {
        e.recorded = true;
        e.mask = RecordEvent(rd8(zTemp0F), &e.byte);
    }
    return e;
}

static void marked_exit_shim(MarkedEvent e)
{
    if (e.recorded) {
        cpu.a = e.mask;
        cpu.y = e.byte;
        abi_flag(P_C, (rd8(zTemp0F) & 7) == 0);
    } else {
        cpu.a = rd8(zTemp0E);
    }
    abi_nz(cpu.a);
}

void abi_RecordIfMarked(void)
{
    marked_exit_shim(RecordIfMarked());
}

/* What a place on the map is, less the scroll: a word subtract, kept for the exit's flags. */
typedef struct {
    uint16_t before;
    uint16_t amount;
} PlaceStep;

static PlaceStep place_axis(uint16_t place, uint16_t scroll, uint16_t field_lo, uint16_t field_hi)
{
    PlaceStep step = { place, scroll };

    write_word(field_lo, field_hi, (uint16_t)(place - scroll));
    return step;
}

/* Fills a slot from the record; the slot is already known usable. */
static PlaceStep place_new_object(uint8_t slot)
{
    uint16_t x_lo = (uint16_t)(OBJ_X_LO + slot);
    uint16_t x_hi = (uint16_t)(OBJ_X_HI + slot);
    uint16_t y_lo = (uint16_t)(OBJ_Y_LO + slot);
    uint16_t y_hi = (uint16_t)(OBJ_Y_HI + slot);
    uint16_t place_along;
    uint16_t place_across;
    uint8_t axis;
    PlaceStep step;

    ClearObject(slot);
    place_along = read_word(zTemp12, zProduct0);
    place_across = read_word(zTemp09, zTemp0A);
    wr8((uint16_t)(OBJ_2750 + slot), rd8(zTemp0C));
    axis = rd8(LevelAxis);
    if (axis == AXIS_VERTICAL) {
        place_axis(place_along, read_word(ScrollYLo, ScrollYHiScreen), y_lo, y_hi);
        step = place_axis(place_across, read_word(ScrollXLo, ScrollXHiScreen), x_lo, x_hi);
    } else {
        place_axis(place_along, read_word(ScrollXLo, ScrollXHiScreen), x_lo, x_hi);
        step = place_axis(place_across, read_word(ScrollYOffset, ScrollYHiScreen), y_lo, y_hi);
        if (axis != AXIS_HORIZONTAL_LOCKED) {
            int8_t scroll = (int8_t)rd8(ScrollYLo);

            wr8(zTemp09, scroll < 0 ? 0xFF : 0x00);
            step = place_axis(read_word(y_lo, y_hi), (uint16_t)(int16_t)scroll, y_lo, y_hi);
        }
    }
    wr8((uint16_t)(OBJ_3040 + slot), rd8(zTemp0D));
    wr8((uint16_t)(OBJ_FLAGS_2F38 + slot), rd8(zTemp0E));
    wr8((uint16_t)(OBJ_EVENT_ID + slot), rd8(zTemp0F));
    return step;
}

/* A zTemp0F; C and V from the last subtract, but the locked axis's compare sets C. */
static void placed_exit_shim(PlaceStep step)
{
    word_subtract_flags_shim(step.before, step.amount);
    if (rd8(LevelAxis) == AXIS_HORIZONTAL_LOCKED)
        abi_flag(P_C, true);
    cpu.a = rd8(zTemp0F);
}

/* Whether a spawn placed its object, and how. */
typedef struct {
    bool placed;
    PlaceStep step;
} Spawn;

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
Spawn StreamSpawnIfFree(void)
{
    uint8_t slot = rd8(zTemp0B);
    Spawn spawn = { false, { 0, 0 } };

    if (rd8((uint16_t)(OBJ_2750 + slot)) == 0) {
        spawn.placed = true;
        spawn.step = place_new_object(slot);
    }
    return spawn;
}

void abi_StreamSpawnIfFree(void)
{
    Spawn spawn = StreamSpawnIfFree();

    if (spawn.placed)
        placed_exit_shim(spawn.step);
    else
        cpu.a = rd8((uint16_t)(OBJ_2750 + rd8(zTemp0B)));
    abi_nz(cpu.x);
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
    placed_exit_shim(place_new_object(cpu.x));
    abi_nz(cpu.a);
}

/*
 * @routine 0047:08C StreamStartScript
 * @run 6A:608C
 * WHAT IT DOES: Record handler 1: starts a script slot, kind 1.
 * HOW: Slot and script from the record; then RecordIfMarked.
 * Entry: zTemp09 script, zTemp0A slot. Exit: as RecordIfMarked, with Y the script
 * if nothing else set it.
 */
MarkedEvent StreamStartScript(void)
{
    StartScriptSlotKind1(rd8(zTemp09), rd8(zTemp0A));
    return RecordIfMarked();
}

void abi_StreamStartScript(void)
{
    uint8_t script = rd8(zTemp09);
    MarkedEvent e = StreamStartScript();

    cpu.y = script;
    marked_exit_shim(e);
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
MarkedEvent StreamStartListKind2(void)
{
    MarkedEvent none = { false, 0, 0 };

    if (rd8(LevelFullWalk) == 0)
        return none;
    StartListChannelKind2(rd8(zTemp09), rd8(zTemp0A));
    return RecordIfMarked();
}

void abi_StreamStartListKind2(void)
{
    bool walking = rd8(LevelFullWalk) != 0;
    MarkedEvent e = StreamStartListKind2();

    if (walking) {
        marked_exit_shim(e);
    } else {
        cpu.a = 0;
        abi_nz(0);
    }
}

/*
 * @routine 0047:0B0 StreamStartListKind3
 * @run 6A:60B0
 * WHAT IT DOES: Record handler 3: starts a list channel as kind 3.
 * HOW: List and channel from the record; then RecordIfMarked.
 * Entry: zTemp09 list, zTemp0A channel. Exit: as RecordIfMarked.
 */
MarkedEvent StreamStartListKind3(void)
{
    StartListChannelKind3(rd8(zTemp09), rd8(zTemp0A));
    return RecordIfMarked();
}

void abi_StreamStartListKind3(void)
{
    marked_exit_shim(StreamStartListKind3());
}

/*
 * @routine 0047:0BB StreamClearObject
 * @run 6A:60BB
 * WHAT IT DOES: Record handler 4: empties an object slot.
 * HOW: ClearObject on the record's slot; then RecordIfMarked.
 * Entry: zTemp09 slot. Exit: as RecordIfMarked.
 */
MarkedEvent StreamClearObject(void)
{
    ClearObject(rd8(zTemp09));
    return RecordIfMarked();
}

void abi_StreamClearObject(void)
{
    marked_exit_shim(StreamClearObject());
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
Spawn StreamSpawnIfOther(void)
{
    uint8_t slot = rd8(zTemp0B);
    Spawn spawn = { false, { 0, 0 } };

    if (rd8((uint16_t)(OBJ_2750 + slot)) == 0 ||
        rd8((uint16_t)(OBJ_EVENT_ID + slot)) != rd8(zTemp0F)) {
        spawn.placed = true;
        spawn.step = place_new_object(slot);
    }
    return spawn;
}

void abi_StreamSpawnIfOther(void)
{
    Spawn spawn = StreamSpawnIfOther();

    if (spawn.placed) {
        placed_exit_shim(spawn.step);
    } else {
        cpu.a = rd8(zTemp0F);
        abi_flag(P_C, true);
    }
    abi_nz(cpu.x);
}

/* One call to the code a record names, with X kept round it. */
static void record_code_shim(uint16_t code)
{
    uint8_t x = cpu.x;

    call_original(code);
    cpu.x = x;
}

/*
 * @routine 0047:0DB StreamCallOverlay
 * @run 6A:60DB
 * WHAT IT DOES: Record handler 6: calls the code the record points to.
 * HOW: JMP through the pointer in zTemp09-0A; then RecordIfMarked.
 * Entry: zTemp09-0A code. Exit: as RecordIfMarked.
 */
MarkedEvent StreamCallOverlay(void)
{
    record_code_shim(read_word(zTemp09, zTemp0A));
    return RecordIfMarked();
}

void abi_StreamCallOverlay(void)
{
    marked_exit_shim(StreamCallOverlay());
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
    call_original(read_word(zTemp09, zTemp0A));
}

/* Copies the record to zero page. Returns the mask of its event's bit if already done. */
static uint8_t load_record(uint8_t *byte)
{
    uint16_t record = stream_record_at();
    uint8_t mask;

    wr8(zTemp08, rd8((uint16_t)(record + RECORD_ARGUMENTS)));
    wr8(zTemp09, rd8((uint16_t)(record + 3)));
    wr8(zTemp0A, rd8((uint16_t)(record + 4)));
    wr8(zTemp0B, (uint8_t)(rd8((uint16_t)(record + RECORD_SLOT_BYTE)) + SLOT_RESERVED));
    wr8(zTemp0C, rd8((uint16_t)(record + 6)));
    wr8(zTemp0D, rd8((uint16_t)(record + 7)));
    wr8(zTemp0E, rd8((uint16_t)(record + 8)));
    wr8(zTemp0F, rd8((uint16_t)(record + 9)));
    mask = BitOfIndex(rd8(zTemp0F), byte);
    return (uint8_t)(mask & rd8((uint16_t)(EventFlags + *byte)));
}

/* The handler's table entry, as the assembly keeps it in zTemp00-01. */
static uint16_t handler_entry(uint8_t handler)
{
    uint16_t entry = (uint16_t)(StreamHandlers + (uint8_t)(handler << 1));

    wr8(zTemp00, rd8(entry));
    wr8(zTemp01, rd8((uint16_t)(entry + 1)));
    return read_word(zTemp00, zTemp01);
}

/* What the assembly has in A, Y and the flags as it jumps to a handler. */
static void handler_entry_shim(uint8_t handler, uint16_t code)
{
    uint8_t slot = rd8((uint16_t)(stream_record_at() + RECORD_SLOT_BYTE));
    uint8_t slot_sum = (uint8_t)(slot + SLOT_RESERVED);

    cpu.a = (uint8_t)(code >> 8);
    cpu.y = (uint8_t)(handler << 1);
    abi_nz(cpu.a);
    abi_flag(P_C, (handler & 0x80) != 0);
    abi_flag(P_V, (~(slot ^ SLOT_RESERVED) & (slot ^ slot_sum) & 0x80) != 0);
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
void RunStreamRecord(void)
{
    uint8_t byte;
    uint8_t handler;
    uint16_t code;

    if (load_record(&byte) != 0)
        return;
    handler = rd8(zTemp08);
    code = handler_entry(handler);
    switch (handler) {
    case 0:
        StreamSpawnIfFree();
        break;
    case 1:
        StreamStartScript();
        break;
    case 2:
        StreamStartListKind2();
        break;
    case 3:
        StreamStartListKind3();
        break;
    case 4:
        StreamClearObject();
        break;
    case 5:
        StreamSpawnIfOther();
        break;
    case 6:
        handler_entry_shim(handler, code);
        StreamCallOverlay();
        break;
    default:
        /* Past the table: its bytes are the entry, as on the original. */
        handler_entry_shim(handler, code);
        record_code_shim(code);
        break;
    }
}

void abi_RunStreamRecord(void)
{
    static void (*const handlers[RECORD_HANDLERS])(void) = {
        abi_StreamSpawnIfFree, abi_StreamStartScript, abi_StreamStartListKind2,
        abi_StreamStartListKind3, abi_StreamClearObject, abi_StreamSpawnIfOther,
        abi_StreamCallOverlay
    };
    uint8_t byte;
    uint8_t done = load_record(&byte);
    uint8_t handler = rd8(zTemp08);
    uint16_t code;

    if (done != 0) {
        handler_entry_shim(0, 0);
        cpu.a = done;
        cpu.y = byte;
        abi_nz(done);
        abi_flag(P_C, (rd8(zTemp0F) & 7) == 0);
        return;
    }
    code = handler_entry(handler);
    handler_entry_shim(handler, code);
    if (handler < RECORD_HANDLERS)
        handlers[handler]();
    else
        call_original(code);
}

/* How a walk ended. */
typedef enum {
    WALK_STOPPED_HIGH,     /* the record's high byte differs from the position's */
    WALK_STOPPED_LOW,      /* the high bytes matched; the low bytes decided */
    WALK_END_MARK,
    WALK_BEFORE_START,     /* backward: the record stepped to has its top bit set */
    EDGE_BEFORE_START,     /* the edge lies before the level's start: no walk made */
    STEP_NO_SPEED          /* the scroll did not move: no walk made */
} WalkEnd;

typedef struct {
    WalkEnd how;
    bool back;
    bool stepped;          /* a record was stepped to, which leaves Y 1 */
    uint16_t scroll_lo, scroll_hi;
} StreamWalk;

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
StreamWalk StreamWalkForward(void)
{
    StreamWalk walk = { WALK_STOPPED_HIGH, false, false, 0, 0 };

    for (;;) {
        uint16_t record;

        if (record_in_range()) {
            RunStreamRecord();
        } else {
            uint8_t high = rd8(zProduct0);
            uint8_t low = rd8(zTemp12);
            uint8_t wanted = rd8(zTemp05);

            if ((high & low) == STREAM_END_MARK) {
                write_word(LevelStreamLo, LevelStreamHi,
                           (uint16_t)(stream_record_at() - STREAM_RECORD_BYTES));
                walk.how = WALK_END_MARK;
                return walk;
            }
            if (high > wanted) {
                walk.how = WALK_STOPPED_HIGH;
                return walk;
            }
            if (high == wanted && low >= rd8(zTemp04)) {
                walk.how = WALK_STOPPED_LOW;
                return walk;
            }
        }
        record = (uint16_t)(stream_record_at() + STREAM_RECORD_BYTES);
        write_word(zTemp10, zTemp11, record);
        write_word(LevelStreamLo, LevelStreamHi, record);
        read_record_position();
        walk.stepped = true;
    }
}

/* A, the flags and Y as a walk leaves them. V is StreamDistance's until an end changes it. */
static void walk_exit_shim(StreamWalk walk)
{
    uint16_t record = stream_record_at();

    if (walk.stepped)
        cpu.y = 1;
    switch (walk.how) {
    case WALK_STOPPED_HIGH:
        distance_flags_shim();
        compare_exit_shim(rd8(zProduct0), rd8(zTemp05));
        break;
    case WALK_STOPPED_LOW:
        distance_flags_shim();
        compare_exit_shim(rd8(zTemp12), rd8(zTemp04));
        break;
    case WALK_END_MARK:
        if (walk.back) {
            word_add_flags_shim(record, STREAM_RECORD_BYTES);
            cpu.a = (uint8_t)((uint16_t)(record + STREAM_RECORD_BYTES) >> 8);
        } else {
            word_subtract_flags_shim(record, STREAM_RECORD_BYTES);
            cpu.a = (uint8_t)((uint16_t)(record - STREAM_RECORD_BYTES) >> 8);
        }
        abi_nz(cpu.a);
        break;
    case WALK_BEFORE_START:
        word_subtract_flags_shim((uint16_t)(record + STREAM_RECORD_BYTES), STREAM_RECORD_BYTES);
        cpu.a = rd8((uint16_t)(record + 1));
        abi_nz(cpu.a);
        break;
    case EDGE_BEFORE_START: {
        uint16_t scroll = read_word(walk.scroll_lo, walk.scroll_hi);

        word_add_flags_shim(scroll, EDGE_BEHIND);
        cpu.a = (uint8_t)((uint16_t)(scroll + EDGE_BEHIND) >> 8);
        abi_nz(cpu.a);
        break;
    }
    case STEP_NO_SPEED:
        cpu.y = 1;
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, rd8(LevelAxis) >= AXIS_VERTICAL);
        break;
    }
}

void abi_StreamWalkForward(void)
{
    walk_exit_shim(StreamWalkForward());
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
StreamWalk StreamWalkBack(void)
{
    StreamWalk walk = { WALK_STOPPED_HIGH, true, false, 0, 0 };

    for (;;) {
        uint16_t record;

        if (record_in_range()) {
            RunStreamRecord();
        } else {
            uint8_t high = rd8(zProduct0);
            uint8_t low = rd8(zTemp12);
            uint8_t wanted = rd8(zTemp05);

            if ((high & low) == STREAM_END_MARK) {
                write_word(LevelStreamLo, LevelStreamHi,
                           (uint16_t)(stream_record_at() + STREAM_RECORD_BYTES));
                walk.how = WALK_END_MARK;
                return walk;
            }
            if (high < wanted) {
                walk.how = WALK_STOPPED_HIGH;
                return walk;
            }
            if (high == wanted && low < rd8(zTemp04)) {
                walk.how = WALK_STOPPED_LOW;
                return walk;
            }
        }
        record = (uint16_t)(stream_record_at() - STREAM_RECORD_BYTES);
        write_word(zTemp10, zTemp11, record);
        walk.stepped = true;
        if (rd8((uint16_t)(record + 1)) & 0x80) {
            walk.how = WALK_BEFORE_START;
            return walk;
        }
        write_word(LevelStreamLo, LevelStreamHi, record);
        read_record_position();
    }
}

void abi_StreamWalkBack(void)
{
    walk_exit_shim(StreamWalkBack());
}

/* The record pointer, and its position: every walk starts here. */
static void walk_start(void)
{
    write_word(zTemp10, zTemp11, read_word(LevelStreamLo, LevelStreamHi));
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
void StreamWalkLines(void)
{
    uint16_t count;

    do {
        walk_start();
        if (rd8(StreamStepLo) & 0x80)
            StreamWalkBack();
        else
            StreamWalkForward();
        write_word(zTemp04, zTemp05,
                   (uint16_t)(read_word(zTemp04, zTemp05) + read_word(StreamStepLo, StreamStepHi)));
        count = (uint16_t)(read_word(WorkByteB, WorkByteA) - 1);
        write_word(WorkByteB, WorkByteA, count);
    } while (count != 0);
}

/* The count ended by going from 1 to 0, with no borrow and no overflow. Every walk has Y 1. */
static void lines_exit_shim(void)
{
    cpu.a = 0;
    cpu.y = 1;
    abi_nz(0);
    abi_flag(P_C, true);
    abi_flag(P_V, false);
}

void abi_StreamWalkLines(void)
{
    StreamWalkLines();
    lines_exit_shim();
}

/* One walk of a full pass, from the scroll, one line in range at a time. */
static void walk_full_span(uint16_t step, uint16_t scroll, uint16_t lines)
{
    write_word(StreamStepLo, StreamStepHi, step);
    write_word(zTemp04, zTemp05, scroll);
    write_word(WorkByteB, WorkByteA, lines);
    wr8(zTemp06, 1);
    StreamWalkLines();
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
void StreamFullWalk(void)
{
    bool vertical = rd8(LevelAxis) == AXIS_VERTICAL;
    uint16_t scroll_lo = vertical ? ScrollYLo : ScrollXLo;
    uint16_t scroll_hi = vertical ? ScrollYHiScreen : ScrollXHiScreen;

    wr8(LevelFullWalk, 1);
    walk_full_span(0x0001, read_word(scroll_lo, scroll_hi), FULL_WALK_AHEAD);
    walk_full_span(0xFFFF, read_word(scroll_lo, scroll_hi), FULL_WALK_BEHIND);
    wr8(LevelFullWalk, 0);
}

void abi_StreamFullWalk(void)
{
    StreamFullWalk();
    lines_exit_shim();
}

/* A walk one edge past the screen, with the speed as its range. */
static StreamWalk edge_walk_forward(uint16_t scroll_lo, uint16_t scroll_hi, uint16_t speed)
{
    write_word(zTemp04, zTemp05, (uint16_t)(read_word(scroll_lo, scroll_hi) + EDGE_AHEAD));
    wr8(zTemp06, rd8(speed));
    return StreamWalkForward();
}

static StreamWalk edge_walk_back(uint16_t scroll_lo, uint16_t scroll_hi, uint16_t speed)
{
    uint16_t edge = (uint16_t)(read_word(scroll_lo, scroll_hi) + EDGE_BEHIND);

    wr8(zTemp04, (uint8_t)edge);
    if (edge & 0x8000) {
        StreamWalk before_start = { EDGE_BEFORE_START, true, false, scroll_lo, scroll_hi };

        return before_start;
    }
    wr8(zTemp05, (uint8_t)(edge >> 8));
    wr8(zTemp06, (uint8_t)(0u - rd8(speed)));
    return StreamWalkBack();
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
    walk_exit_shim(edge_walk_forward(ScrollXLo, ScrollXHiScreen, ScrollSpeedX));
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
    walk_exit_shim(edge_walk_back(ScrollXLo, ScrollXHiScreen, ScrollSpeedX));
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
    walk_exit_shim(edge_walk_forward(ScrollYLo, ScrollYHiScreen, ScrollSpeedY));
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
    walk_exit_shim(edge_walk_back(ScrollYLo, ScrollYHiScreen, ScrollSpeedY));
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
StreamWalk StreamScrollStep(void)
{
    bool vertical = rd8(LevelAxis) == AXIS_VERTICAL;
    uint16_t scroll_lo = vertical ? ScrollYLo : ScrollXLo;
    uint16_t scroll_hi = vertical ? ScrollYHiScreen : ScrollXHiScreen;
    uint16_t speed_field = vertical ? ScrollSpeedY : ScrollSpeedX;
    int8_t speed = (int8_t)rd8(speed_field);
    StreamWalk none = { STEP_NO_SPEED, false, false, 0, 0 };

    walk_start();
    if (speed == 0)
        return none;
    if (speed < 0)
        return edge_walk_back(scroll_lo, scroll_hi, speed_field);
    return edge_walk_forward(scroll_lo, scroll_hi, speed_field);
}

void abi_StreamScrollStep(void)
{
    StreamWalk walk = StreamScrollStep();

    cpu.y = 1;
    walk_exit_shim(walk);
}
