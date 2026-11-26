/*
 * The level's object stream: records sorted by position along the level, walked
 * forward and back as the screen scrolls, each acted on as it comes in range. A
 * record is ten bytes: position (word), handler, four arguments, the event's flags
 * and its id, as zTemp12 and zProduct0 and zTemp08-0F hold them while it is read.
 *
 * The bodies make the assembly's accesses and calls in its order. What it leaves in
 * A, Y and the flags when a routine ends is worked out by the exit shims the wrappers
 * call, from values the body kept, never from RAM read again.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"

enum {
    STREAM_RECORD_BYTES = 0x0A, STREAM_END_MARK = 0xFF, SLOT_RESERVED = 0x10,
    AXIS_HORIZONTAL_LOCKED = 0x02, AXIS_VERTICAL = 0x01, OBJECT_MARKED = 0x80,
    EDGE_AHEAD = 0x0140, EDGE_BEHIND = 0xFFC0, FULL_WALK_AHEAD = 0x0141, FULL_WALK_BEHIND = 0x0041,
    RECORD_HANDLERS = 7, RECORD_ARGUMENTS = 2, RECORD_SLOT_BYTE = 5,
    HANDLER_SPAWN_IF_FREE = 0x5FF9, HANDLER_START_SCRIPT = 0x608C, HANDLER_START_LIST_KIND2 = 0x609F,
    HANDLER_START_LIST_KIND3 = 0x60B0, HANDLER_CLEAR_OBJECT = 0x60BB, HANDLER_SPAWN_IF_OTHER = 0x60C4,
    HANDLER_CALL_OVERLAY = 0x60DB
};

/* The stream's walk position and the record's two position bytes, as every walk starts. */
static void walk_start(void)
{
    uint8_t byte;
    uint8_t y = 0;

    byte = RD8(LEVEL->level_stream.lo);
    WR8(ZP->temp10, byte);
    byte = RD8(LEVEL->level_stream.hi);
    WR8(ZP->temp11, byte);
    byte = RD8_PTR_Y(ZP->temp10, y);
    WR8(ZP->temp12, byte);
    y++;
    byte = RD8_PTR_Y(ZP->temp10, y);
    WR8(ZP->product.lo, byte);
}

/* The record's position into zTemp12 and zProduct0 (Y 0 and 1 of the record). */
static void read_record_position(void)
{
    uint8_t byte;
    uint8_t y = 0;

    byte = RD8_PTR_Y(ZP->temp10, y);
    WR8(ZP->temp12, byte);
    y++;
    byte = RD8_PTR_Y(ZP->temp10, y);
    WR8(ZP->product.lo, byte);
}

/* A compare leaves A, N, Z and C; V stands. */
static void compare_exit_shim(uint8_t a, uint8_t b)
{
    cpu.a = a;
    abi_nz((uint8_t)(a - b));
    abi_flag(P_C, a >= b);
}

/* StreamDistance's result and what it leaves: A the high byte, C and V from its last add or subtract. */
typedef struct { uint16_t value; uint8_t a; bool c; bool v; } DistanceExit;

static DistanceExit stream_distance(void)
{
    DistanceExit out;
    uint8_t record_low = RD8(ZP->temp12);
    uint8_t walk_low = RD8(ZP->temp04);
    Sum low = sub_bytes(record_low, walk_low, true);
    uint8_t record_high;
    uint8_t walk_high;
    Sum high;
    uint8_t mid;

    WR8(ZP->product.mid, low.value);
    record_high = RD8(ZP->product.lo);
    walk_high = RD8(ZP->temp05);
    high = sub_bytes(record_high, walk_high, low.carry);
    WR8(ZP->product.hi, high.value);
    if (high.carry) {
        out.value = (uint16_t)(low.value | (high.value << 8));
        out.a = high.value;
        out.c = true;
        out.v = high.overflow;
        return out;
    }
    mid = RD8(ZP->product.mid);
    low = add_bytes((uint8_t)(mid ^ 0xFF), 1, false);
    WR8(ZP->product.mid, low.value);
    record_high = RD8(ZP->product.hi);
    high = add_bytes((uint8_t)(record_high ^ 0xFF), 0, low.carry);
    WR8(ZP->product.hi, high.value);
    out.value = (uint16_t)(low.value | (high.value << 8));
    out.a = high.value;
    out.c = high.carry;
    out.v = high.overflow;
    return out;
}

/*
 * @routine 0046:723 StreamDistance
 * @run 69:5F23
 * WHAT IT DOES: Gives how far the record is from the walk's position, as a word.
 * HOW: Subtracts the walk's position, and negates a negative result.
 * WAITS: no.
 * Entry: zTemp12, zProduct0 record position, zTemp04-05 walk position. Exit:
 * zProduct1-2 the distance; A its high byte, N, Z, C and V from the last add or
 * subtract. X and Y kept.
 */
uint16_t StreamDistance(void)
{
    return stream_distance().value;
}

void abi_StreamDistance(void)
{
    DistanceExit out = stream_distance();

    cpu.a = out.a;
    abi_nz(out.a);
    abi_flag(P_C, out.c);
    abi_flag(P_V, out.v);
}

/* What the record handlers' common end did: whether it marked the event, with the pieces its exit needs. */
typedef struct {
    bool recorded;
    uint8_t flags;      /* zTemp0E as read */
    uint8_t event;      /* zTemp0F as read, when recorded */
    uint8_t mask;
    uint8_t byte;
} MarkedEvent;

/*
 * @routine 0047:095 RecordIfMarked
 * @run 6A:6095
 * WHAT IT DOES: The end of the record handlers: if this record's flags ask for it,
 * marks its event.
 * HOW: Bit 7 of zTemp0E, event in zTemp0F.
 * WAITS: no.
 * Entry: zTemp0E, zTemp0F. Exit: unmarked, A zTemp0E, N and Z from it, C, Y kept.
 * Else as RecordEvent. X and V kept.
 */
static MarkedEvent record_if_marked(uint8_t x)
{
    MarkedEvent e = { false, 0, 0, 0, 0 };

    e.flags = RD8(ZP->temp0e);
    if ((e.flags & OBJECT_MARKED) == 0) {
        return e;
    }
    e.event = RD8(ZP->temp0f);
    e.recorded = true;
    jsr_push(0x609E);
    e.mask = RecordEvent(e.event, x, &e.byte);
    jsr_pop();
    return e;
}

void RecordIfMarked(uint8_t x)
{
    record_if_marked(x);
}

static void marked_exit_shim(MarkedEvent e)
{
    if (e.recorded) {
        cpu.a = e.mask;
        cpu.y = e.byte;
        abi_flag(P_C, (e.event & 7) == 0);
    } else {
        cpu.a = e.flags;
    }
    abi_nz(cpu.a);
}

void abi_RecordIfMarked(void)
{
    marked_exit_shim(record_if_marked(cpu.x));
}

/* How a spawn left its registers: what ClearObject and the place's last subtract left. */
typedef struct {
    uint8_t event;      /* zTemp0F: A at the end */
    Sum last;           /* the last subtract: C and V */
    bool locked;        /* the axis compare set C */
} PlaceExit;

/*
 * Fills a slot from the record: the slot is emptied, given its kind, its place less the scroll
 * on the axis the level runs along, and the record's next kind, flags and event id. The slot
 * is in X, and the caller's X is on the stack: it comes back at the end.
 */
static PlaceExit place_new_object(uint8_t slot)
{
    PlaceExit out = { 0, { 0, false, false }, false };
    uint8_t axis;
    uint8_t byte;
    uint8_t scroll;
    Sum low;
    Sum high;

    jsr_push(0x6007);
    ClearObject(slot);
    jsr_pop();
    byte = RD8(ZP->temp0c);
    WR8(OBJ->kind[slot], byte);
    axis = RD8(LEVEL->level_axis);
    if (axis != AXIS_VERTICAL) {
        byte = RD8(ZP->temp12);
        scroll = RD8(SCROLL->x_lo[0]);
        low = sub_bytes(byte, scroll, true);
        WR8(OBJ->x_lo[slot], low.value);
        byte = RD8(ZP->product.lo);
        scroll = RD8(SCROLLE->x_hi_screen[0]);
        high = sub_bytes(byte, scroll, low.carry);
        WR8(OBJ->x_hi[slot], high.value);
        byte = RD8(ZP->temp09);
        scroll = RD8(SCROLLLIM->scroll_y_offset);
        low = sub_bytes(byte, scroll, true);
        WR8(OBJ->y_lo[slot], low.value);
        byte = RD8(ZP->temp0a);
        scroll = RD8(SCROLLE->y_hi_screen[0]);
        high = sub_bytes(byte, scroll, low.carry);
        WR8(OBJ->y_hi[slot], high.value);
        out.last = high;
        axis = RD8(LEVEL->level_axis);
        if (axis == AXIS_HORIZONTAL_LOCKED) {
            out.locked = true;
        } else {
            WR8(ZP->temp09, 0);
            scroll = RD8(SCROLL->y_lo[0]);
            if (scroll & 0x80) {
                DEC8(ZP->temp09);
            }
            byte = RD8(OBJ->y_lo[slot]);
            scroll = RD8(SCROLL->y_lo[0]);
            low = sub_bytes(byte, scroll, true);
            WR8(OBJ->y_lo[slot], low.value);
            byte = RD8(OBJ->y_hi[slot]);
            scroll = RD8(ZP->temp09);
            high = sub_bytes(byte, scroll, low.carry);
            WR8(OBJ->y_hi[slot], high.value);
            out.last = high;
        }
    } else {
        byte = RD8(ZP->temp12);
        scroll = RD8(SCROLL->y_lo[0]);
        low = sub_bytes(byte, scroll, true);
        WR8(OBJ->y_lo[slot], low.value);
        byte = RD8(ZP->product.lo);
        scroll = RD8(SCROLLE->y_hi_screen[0]);
        high = sub_bytes(byte, scroll, low.carry);
        WR8(OBJ->y_hi[slot], high.value);
        byte = RD8(ZP->temp09);
        scroll = RD8(SCROLL->x_lo[0]);
        low = sub_bytes(byte, scroll, true);
        WR8(OBJ->x_lo[slot], low.value);
        byte = RD8(ZP->temp0a);
        scroll = RD8(SCROLLE->x_hi_screen[0]);
        high = sub_bytes(byte, scroll, low.carry);
        WR8(OBJ->x_hi[slot], high.value);
        out.last = high;
    }
    byte = RD8(ZP->temp0d);
    WR8(OBJ->next_kind[slot], byte);
    byte = RD8(ZP->temp0e);
    WR8(OBJ->flags[slot], byte);
    out.event = RD8(ZP->temp0f);
    WR8(OBJ->event_id[slot], out.event);
    return out;
}

static void placed_exit_shim(PlaceExit out)
{
    cpu.a = out.event;
    abi_flag(P_C, out.locked ? true : out.last.carry);
    abi_flag(P_V, out.last.overflow);
}

/*
 * @routine 0047:08A StreamSpawnEnd
 * @run 6A:608A
 * WHAT IT DOES: The end of the spawning record handlers: gives back the caller's X.
 * HOW: Pulls X, which the handler kept on the stack.
 * WAITS: no.
 * Entry: the caller's X on the stack. Exit: X that, N and Z from it. A, Y, C and V kept.
 */
uint8_t StreamSpawnEnd(void)
{
    return pull8();
}

void abi_StreamSpawnEnd(void)
{
    cpu.x = StreamSpawnEnd();
    abi_nz(cpu.x);
}

/* Whether a spawn placed its object, and what the exit then needs. */
typedef struct {
    bool placed;
    uint8_t kind;       /* the slot's kind, when it was taken */
    uint8_t x;          /* the caller's X, as pulled */
    PlaceExit place;
} Spawn;

/*
 * @routine 0046:7F9 StreamSpawnIfFree
 * @run 69:5FF9
 * WHAT IT DOES: Record handler 0: starts an object in the record's slot, if that slot
 * is empty.
 * HOW: The slot's type must be zero. The object is placed at the record's position
 * less the scroll, on the axis the level runs along.
 * WAITS: no.
 * Entry: the record in zTemp09-0F, zTemp12, zProduct0; X kept on the stack. Exit: A zTemp0F (or
 * the slot's type, if taken), X and Y kept, N and Z from X, C and V from the last step.
 */
Spawn StreamSpawnIfFree(uint8_t x)
{
    uint8_t slot;
    Spawn spawn = { false, 0, 0, { 0, { 0, false, false }, false } };

    push8(x);
    slot = RD8(ZP->temp0b);
    spawn.kind = RD8(OBJ->kind[slot]);
    if (spawn.kind == 0) {
        spawn.placed = true;
        spawn.place = place_new_object(slot);
    }
    spawn.x = StreamSpawnEnd();
    return spawn;
}

void abi_StreamSpawnIfFree(void)
{
    Spawn spawn = StreamSpawnIfFree(cpu.x);

    if (spawn.placed) {
        placed_exit_shim(spawn.place);
    } else {
        cpu.a = spawn.kind;
    }
    abi_nz(spawn.x);
}

/*
 * @routine 0047:004 StreamPlaceObject
 * @run 6A:6004
 * WHAT IT DOES: The body of the spawning record handlers, entered with the slot in X.
 * HOW: As StreamSpawnIfFree, past its check.
 * WAITS: no.
 * Entry: X slot, on the stack the caller's X. Exit: as StreamSpawnIfFree.
 */
Spawn StreamPlaceObject(uint8_t slot)
{
    Spawn spawn = { true, 0, 0, { 0, { 0, false, false }, false } };

    spawn.place = place_new_object(slot);
    spawn.x = StreamSpawnEnd();
    return spawn;
}

void abi_StreamPlaceObject(void)
{
    Spawn spawn = StreamPlaceObject(cpu.x);

    placed_exit_shim(spawn.place);
    cpu.x = spawn.x;
    abi_nz(spawn.x);
}

/*
 * @routine 0047:08C StreamStartScript
 * @run 6A:608C
 * WHAT IT DOES: Record handler 1: starts a script slot, kind 1.
 * HOW: Slot and script from the record; then RecordIfMarked.
 * WAITS: no.
 * Entry: zTemp09 script, zTemp0A slot. Exit: as RecordIfMarked, with Y the script
 * if nothing else set it.
 */
static MarkedEvent stream_start_script(uint8_t x, uint8_t *script_out)
{
    uint8_t script;
    uint8_t slot;

    push8(x);
    script = RD8(ZP->temp09);
    slot = RD8(ZP->temp0a);
    jsr_push(0x6094);
    StartScriptSlotKind1(script, slot);
    jsr_pop();
    x = pull8();
    *script_out = script;
    return record_if_marked(x);
}

void StreamStartScript(uint8_t x)
{
    uint8_t script;

    stream_start_script(x, &script);
}

void abi_StreamStartScript(void)
{
    uint8_t script;
    MarkedEvent e = stream_start_script(cpu.x, &script);

    cpu.y = script;
    marked_exit_shim(e);
}

/*
 * @routine 0047:09F StreamStartListKind2
 * @run 6A:609F
 * WHAT IT DOES: Record handler 2: starts a list channel as kind 2, only while the
 * full-walk flag is up: that is, when a level section is first entered.
 * HOW: Tests LevelFullWalk; then list and channel from the record, and RecordIfMarked.
 * WAITS: no.
 * Entry: zTemp09 list, zTemp0A channel. Exit: flag down, A 0, N 0, Z 1. Else as
 * RecordIfMarked.
 */
static bool stream_start_list2(uint8_t x, MarkedEvent *e)
{
    uint8_t list;
    uint8_t channel;
    uint8_t walking = RD8(LEVEL->full_walk);

    if (walking == 0) {
        return false;
    }
    push8(x);
    list = RD8(ZP->temp09);
    channel = RD8(ZP->temp0a);
    jsr_push(0x60AC);
    StartListChannelKind2(list, channel);
    jsr_pop();
    x = pull8();
    *e = record_if_marked(x);
    return true;
}

void StreamStartListKind2(uint8_t x)
{
    MarkedEvent e;

    stream_start_list2(x, &e);
}

void abi_StreamStartListKind2(void)
{
    MarkedEvent e;

    if (stream_start_list2(cpu.x, &e)) {
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
 * WAITS: no.
 * Entry: zTemp09 list, zTemp0A channel. Exit: as RecordIfMarked.
 */
static MarkedEvent stream_start_list3(uint8_t x)
{
    uint8_t list;
    uint8_t channel;

    push8(x);
    list = RD8(ZP->temp09);
    channel = RD8(ZP->temp0a);
    jsr_push(0x60B8);
    StartListChannelKind3(list, channel);
    jsr_pop();
    x = pull8();
    return record_if_marked(x);
}

void StreamStartListKind3(uint8_t x)
{
    stream_start_list3(x);
}

void abi_StreamStartListKind3(void)
{
    marked_exit_shim(stream_start_list3(cpu.x));
}

/*
 * @routine 0047:0BB StreamClearObject
 * @run 6A:60BB
 * WHAT IT DOES: Record handler 4: empties an object slot.
 * HOW: ClearObject on the record's slot; then RecordIfMarked.
 * WAITS: no.
 * Entry: zTemp09 slot. Exit: as RecordIfMarked.
 */
static MarkedEvent stream_clear_object(uint8_t x)
{
    uint8_t slot;

    push8(x);
    slot = RD8(ZP->temp09);
    jsr_push(0x60C1);
    ClearObject(slot);
    jsr_pop();
    x = pull8();
    return record_if_marked(x);
}

void StreamClearObject(uint8_t x)
{
    stream_clear_object(x);
}

void abi_StreamClearObject(void)
{
    marked_exit_shim(stream_clear_object(cpu.x));
}

/*
 * @routine 0047:0C4 StreamSpawnIfOther
 * @run 6A:60C4
 * WHAT IT DOES: Record handler 5: starts an object in the record's slot unless that
 * slot already holds the object for this same event, so one spawned earlier stays.
 * HOW: Empty, or a different event id, places the object.
 * WAITS: no.
 * Entry: as StreamSpawnIfFree. Exit: placed, as StreamSpawnIfFree. Same event: A the
 * event id, N and Z from X, C 1. X and Y kept. An inner entry at $6004 places directly.
 */
Spawn StreamSpawnIfOther(uint8_t x)
{
    uint8_t slot;
    uint8_t wanted;
    Spawn spawn = { false, 0, 0, { 0, { 0, false, false }, false } };

    push8(x);
    slot = RD8(ZP->temp0b);
    spawn.kind = RD8(OBJ->kind[slot]);
    if (spawn.kind == 0) {
        spawn.placed = true;
    } else {
        spawn.kind = RD8(OBJ->event_id[slot]);
        wanted = RD8(ZP->temp0f);
        spawn.placed = spawn.kind != wanted;
    }
    if (spawn.placed) {
        spawn.place = place_new_object(slot);
        spawn.x = StreamSpawnEnd();
    } else {
        spawn.x = pull8();
    }
    return spawn;
}

void abi_StreamSpawnIfOther(void)
{
    Spawn spawn = StreamSpawnIfOther(cpu.x);

    if (spawn.placed) {
        placed_exit_shim(spawn.place);
    } else {
        cpu.a = spawn.kind;
        abi_flag(P_C, true);
    }
    abi_nz(spawn.x);
}

/* One run of the code a record names, which leaves X as it likes: the X it left. */
static uint8_t record_code_shim(uint16_t code)
{
    call_original(code);
    return cpu.x;
}

/*
 * @routine 0047:0E3 JumpThroughRecord
 * @run 6A:60E3
 * WHAT IT DOES: The jump StreamCallOverlay reaches a record's code by.
 * HOW: JMP [zTemp09].
 * WAITS: no.
 * Entry: zTemp09-0A code. Exit: as that code leaves it.
 */
void JumpThroughRecord(void)
{
    uint16_t code = RD16_LH2(ZP->temp09, ZP->temp0a);

    record_code_shim(code);
}

void abi_JumpThroughRecord(void)
{
    uint16_t code = RD16_LH2(ZP->temp09, ZP->temp0a);

    call_original(code);
}

/*
 * @routine 0047:0DB StreamCallOverlay
 * @run 6A:60DB
 * WHAT IT DOES: Record handler 6: calls the code the record points to.
 * HOW: JMP through the pointer in zTemp09-0A, as a call; then RecordIfMarked.
 * WAITS: it can wait where the record's code does.
 * Entry: zTemp09-0A code. Exit: as RecordIfMarked.
 */
static MarkedEvent stream_call_overlay(uint8_t x, uint8_t *pulled)
{
    push8(x);
    jsr_push(0x60DF);
    JumpThroughRecord();
    jsr_pop();
    *pulled = pull8();
    return record_if_marked(*pulled);
}

void StreamCallOverlay(uint8_t x)
{
    uint8_t pulled;

    stream_call_overlay(x, &pulled);
}

void abi_StreamCallOverlay(void)
{
    uint8_t pulled;
    MarkedEvent e = stream_call_overlay(cpu.x, &pulled);

    cpu.x = pulled;
    marked_exit_shim(e);
}

/* What RunStreamRecord found before it dispatches. */
typedef struct {
    uint8_t done;       /* the event's bit in EventFlags: nonzero, the record is not run */
    uint8_t byte;       /* BitOfIndex's byte index (Y) */
    uint8_t event;      /* the record's last byte */
    bool slot_overflow; /* V of the slot byte's add of $10 */
    uint8_t handler;
    uint16_t code;
} RecordRun;

/* Copies the record to zero page, finds whether its event is done and, if not, which code it names. */
static RecordRun load_record(uint8_t x)
{
    RecordRun run = { 0, 0, 0, false, 0, 0 };
    uint8_t y = RECORD_ARGUMENTS;
    uint8_t byte;
    uint8_t mask;
    uint8_t flags;
    Sum slot;
    uint8_t lo;
    uint8_t hi;

    byte = RD8_PTR_Y(ZP->temp10, y);
    WR8(ZP->temp08, byte);
    y++;
    byte = RD8_PTR_Y(ZP->temp10, y);
    WR8(ZP->temp09, byte);
    y++;
    byte = RD8_PTR_Y(ZP->temp10, y);
    WR8(ZP->temp0a, byte);
    y++;
    byte = RD8_PTR_Y(ZP->temp10, y);
    slot = add_bytes(byte, SLOT_RESERVED, false);
    run.slot_overflow = slot.overflow;
    WR8(ZP->temp0b, slot.value);
    y++;
    byte = RD8_PTR_Y(ZP->temp10, y);
    WR8(ZP->temp0c, byte);
    y++;
    byte = RD8_PTR_Y(ZP->temp10, y);
    WR8(ZP->temp0d, byte);
    y++;
    byte = RD8_PTR_Y(ZP->temp10, y);
    WR8(ZP->temp0e, byte);
    y++;
    run.event = RD8_PTR_Y(ZP->temp10, y);
    WR8(ZP->temp0f, run.event);
    jsr_push(0x5FD4);
    mask = BitOfIndex(run.event, x, &run.byte);
    jsr_pop();
    flags = RD8(LEVEL->event_flags[run.byte]);
    run.done = (uint8_t)(mask & flags);
    if (run.done != 0) {
        return run;
    }
    run.handler = RD8(ZP->temp08);
    y = (uint8_t)(run.handler << 1);
    lo = rd8((uint16_t)(StreamHandlers + y));
    WR8(ZP->temp00, lo);
    hi = rd8((uint16_t)(StreamHandlers + 1 + y));
    WR8(ZP->temp01, hi);
    run.code = RD16_LH2(ZP->temp00, ZP->temp01);
    return run;
}

static void handler_entry_shim(RecordRun run, uint8_t x);

/*
 * @routine 0046:7A5 RunStreamRecord
 * @run 69:5FA5
 * WHAT IT DOES: Acts on the record the walk stands on, unless its event has already
 * happened.
 * HOW: Copies its eight bytes to zero page, the slot with $10 added past the player's
 * own slots. Tests the event's bit in EventFlags, and goes through the handler table,
 * whose entry it keeps in zTemp00-01.
 * WAITS: it can wait where a handler's code does.
 * Entry: zTemp10-11 the record, X the caller's, which the handlers keep on the stack.
 * Exit: event done, A the bit found, N and Z from it, Y its byte, C as BitOfIndex. Else as the
 * handler, with Y its table index if the handler leaves it. X kept. Clobbers zTemp00-01,
 * zTemp08-0F.
 */
uint8_t RunStreamRecord(uint8_t x)
{
    RecordRun run = load_record(x);

    if (run.done != 0) {
        return x;
    }
    /* The jump goes by the address the table holds, as the original's does. */
    switch (run.code) {
    case HANDLER_SPAWN_IF_FREE:
        StreamSpawnIfFree(x);
        break;
    case HANDLER_START_SCRIPT:
        StreamStartScript(x);
        break;
    case HANDLER_START_LIST_KIND2:
        StreamStartListKind2(x);
        break;
    case HANDLER_START_LIST_KIND3:
        StreamStartListKind3(x);
        break;
    case HANDLER_CLEAR_OBJECT:
        StreamClearObject(x);
        break;
    case HANDLER_SPAWN_IF_OTHER:
        StreamSpawnIfOther(x);
        break;
    case HANDLER_CALL_OVERLAY:
        handler_entry_shim(run, x);   /* the registers the code the record names is jumped to with */
        StreamCallOverlay(x);
        break;
    default:
        /* Another entry, or past the table: the bytes there are the code to run. */
        handler_entry_shim(run, x);
        x = record_code_shim(run.code);
        break;
    }
    return x;
}

/* What the assembly has in A, Y and the flags as it jumps to a handler. */
static void handler_entry_shim(RecordRun run, uint8_t x)
{
    cpu.x = x;
    cpu.a = (uint8_t)(run.code >> 8);
    cpu.y = (uint8_t)(run.handler << 1);
    abi_nz(cpu.a);
    abi_flag(P_C, (run.handler & 0x80) != 0);
    abi_flag(P_V, run.slot_overflow);
}

void abi_RunStreamRecord(void)
{
    RecordRun run = load_record(cpu.x);

    if (run.done != 0) {
        cpu.a = run.done;
        cpu.y = run.byte;
        abi_nz(run.done);
        abi_flag(P_C, (run.event & 7) == 0);
        abi_flag(P_V, run.slot_overflow);
        return;
    }
    handler_entry_shim(run, cpu.x);
    switch (run.code) {
    case HANDLER_SPAWN_IF_FREE:
        abi_StreamSpawnIfFree();
        break;
    case HANDLER_START_SCRIPT:
        abi_StreamStartScript();
        break;
    case HANDLER_START_LIST_KIND2:
        abi_StreamStartListKind2();
        break;
    case HANDLER_START_LIST_KIND3:
        abi_StreamStartListKind3();
        break;
    case HANDLER_CLEAR_OBJECT:
        abi_StreamClearObject();
        break;
    case HANDLER_SPAWN_IF_OTHER:
        abi_StreamSpawnIfOther();
        break;
    case HANDLER_CALL_OVERLAY:
        abi_StreamCallOverlay();
        break;
    default:
        call_original(run.code);
        break;
    }
}

/* How a walk ended. */
/* StreamWalkEnd and StreamWalk are in routines.h: what a stream walk ends on. */

/* The record pointer moves on one record, the stream position and the record's position follow. */
static void step_forward(void)
{
    uint8_t low = RD8(ZP->temp10);
    Sum lo = add_bytes(low, STREAM_RECORD_BYTES, false);
    uint8_t high;
    Sum hi;

    WR8(ZP->temp10, lo.value);
    WR8(LEVEL->level_stream.lo, lo.value);
    high = RD8(ZP->temp11);
    hi = add_bytes(high, 0, lo.carry);
    WR8(ZP->temp11, hi.value);
    WR8(LEVEL->level_stream.hi, hi.value);
    read_record_position();
}

/* Back from the end mark: the stream position is the record after the pointer's. Returns the add's high byte. */
static Sum end_back(void)
{
    uint8_t low = RD8(ZP->temp10);
    Sum lo = add_bytes(low, STREAM_RECORD_BYTES, false);
    uint8_t high;
    Sum hi;

    WR8(LEVEL->level_stream.lo, lo.value);
    high = RD8(ZP->temp11);
    hi = add_bytes(high, 0, lo.carry);
    WR8(LEVEL->level_stream.hi, hi.value);
    return hi;
}

static Sum end_forward(void)
{
    uint8_t low = RD8(ZP->temp10);
    Sum lo = sub_bytes(low, STREAM_RECORD_BYTES, true);
    uint8_t high;
    Sum hi;

    WR8(LEVEL->level_stream.lo, lo.value);
    high = RD8(ZP->temp11);
    hi = sub_bytes(high, 0, lo.carry);
    WR8(LEVEL->level_stream.hi, hi.value);
    return hi;
}

/*
 * @routine 0046:6CC StreamWalkForward
 * @run 69:5ECC
 * WHAT IT DOES: Walks the stream forward from the record it stands on: acts on each
 * record in range, skips those behind, stops at the first too far ahead or at the
 * end mark, where the stream position stays on the last real record.
 * HOW: StreamDistance against the range in zTemp06, then the position compared
 * with the walk's position.
 * WAITS: it can wait where a handler's code does.
 * Entry: zTemp10-11 the record, zTemp12, zProduct0 its position, zTemp04-05 the
 * position, zTemp06 the range, X the caller's. Exit: A, Y and flags as the last step left
 * them: Y 1 once a record has been stepped to. X kept. Clobbers what the handlers do.
 */
StreamWalk StreamWalkForward(uint8_t x)
{
    StreamWalk walk = { WALK_STOPPED_HIGH, false, false, 0, 0, { 0, false, false }, 0 };

    walk.x = x;

    for (;;) {
        DistanceExit dist;
        uint8_t high;
        uint8_t range;

        jsr_push(0x5ECF);
        dist = stream_distance();
        jsr_pop();
        walk.distance_v = dist.v;
        high = RD8(ZP->product.hi);
        if (high == 0) {
            uint8_t mid = RD8(ZP->product.mid);

            range = RD8(ZP->temp06);
            if (mid < range) {
                jsr_push(0x5EDC);
                x = RunStreamRecord(x);
                walk.x = x;
                jsr_pop();
                step_forward();
                walk.stepped = true;
                continue;
            }
        }
        {
            uint8_t position = RD8(ZP->product.lo);
            uint8_t record_low = RD8(ZP->temp12);
            uint8_t wanted;

            if ((uint8_t)(position & record_low) == STREAM_END_MARK) {
                walk.last = end_forward();
                walk.how = WALK_END_MARK;
                return walk;
            }
            position = RD8(ZP->product.lo);
            wanted = RD8(ZP->temp05);
            if (position >= wanted) {
                if (position != wanted) {
                    walk.how = WALK_STOPPED_HIGH;
                    walk.cmp_a = position;
                    walk.cmp_b = wanted;
                    return walk;
                }
                record_low = RD8(ZP->temp12);
                wanted = RD8(ZP->temp04);
                if (record_low >= wanted) {
                    walk.how = WALK_STOPPED_LOW;
                    walk.cmp_a = record_low;
                    walk.cmp_b = wanted;
                    return walk;
                }
            }
            step_forward();
            walk.stepped = true;
        }
    }
}

/* A, the flags and Y as a walk leaves them. V is StreamDistance's until an end changes it. */
void stream_walk_exit_shim(StreamWalk walk)
{
    if (walk.stepped) {
        cpu.y = 1;
    }
    switch (walk.how) {
    case WALK_STOPPED_HIGH:
    case WALK_STOPPED_LOW:
        abi_flag(P_V, walk.distance_v);
        compare_exit_shim(walk.cmp_a, walk.cmp_b);
        break;
    case WALK_END_MARK:
    case EDGE_BEFORE_START:
        cpu.a = walk.last.value;
        abi_nz(cpu.a);
        abi_flag(P_C, walk.last.carry);
        abi_flag(P_V, walk.last.overflow);
        break;
    case WALK_BEFORE_START:
        cpu.a = walk.cmp_a;
        abi_nz(cpu.a);
        abi_flag(P_C, walk.last.carry);
        abi_flag(P_V, walk.last.overflow);
        break;
    case STEP_NO_SPEED:
        cpu.y = 1;
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, walk.cmp_a >= AXIS_VERTICAL);
        break;
    }
}

void abi_StreamWalkForward(void)
{
    stream_walk_exit_shim(StreamWalkForward(cpu.x));
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
 * WAITS: it can wait where a handler's code does.
 * Entry and exit: as StreamWalkForward.
 */
StreamWalk StreamWalkBack(uint8_t x)
{
    StreamWalk walk = { WALK_STOPPED_HIGH, false, false, 0, 0, { 0, false, false }, 0 };

    walk.x = x;

    for (;;) {
        DistanceExit dist;
        uint8_t high;
        uint8_t range;
        bool step = false;

        jsr_push(0x5F47);
        dist = stream_distance();
        jsr_pop();
        walk.distance_v = dist.v;
        high = RD8(ZP->product.hi);
        if (high == 0) {
            uint8_t mid = RD8(ZP->product.mid);

            range = RD8(ZP->temp06);
            if (mid < range) {
                jsr_push(0x5F54);
                x = RunStreamRecord(x);
                walk.x = x;
                jsr_pop();
                step = true;
            }
        }
        if (!step) {
            uint8_t position = RD8(ZP->product.lo);
            uint8_t record_low = RD8(ZP->temp12);
            uint8_t wanted;

            if ((uint8_t)(position & record_low) == STREAM_END_MARK) {
                walk.last = end_back();
                walk.how = WALK_END_MARK;
                return walk;
            }
            position = RD8(ZP->product.lo);
            wanted = RD8(ZP->temp05);
            if (position < wanted) {
                walk.how = WALK_STOPPED_HIGH;
                walk.cmp_a = position;
                walk.cmp_b = wanted;
                return walk;
            }
            if (position == wanted) {
                record_low = RD8(ZP->temp12);
                wanted = RD8(ZP->temp04);
                if (record_low < wanted) {
                    walk.how = WALK_STOPPED_LOW;
                    walk.cmp_a = record_low;
                    walk.cmp_b = wanted;
                    return walk;
                }
            }
        }
        {
            uint8_t low = RD8(ZP->temp10);
            Sum lo = sub_bytes(low, STREAM_RECORD_BYTES, true);
            uint8_t flag_byte;
            uint8_t y = 1;
            Sum hi;

            WR8(ZP->temp10, lo.value);
            high = RD8(ZP->temp11);
            hi = sub_bytes(high, 0, lo.carry);
            WR8(ZP->temp11, hi.value);
            walk.stepped = true;
            flag_byte = RD8_PTR_Y(ZP->temp10, y);
            if (flag_byte & 0x80) {
                walk.how = WALK_BEFORE_START;
                walk.cmp_a = flag_byte;
                walk.last = hi;
                return walk;
            }
            low = RD8(ZP->temp10);
            WR8(LEVEL->level_stream.lo, low);
            high = RD8(ZP->temp11);
            WR8(LEVEL->level_stream.hi, high);
            read_record_position();
        }
    }
}

void abi_StreamWalkBack(void)
{
    stream_walk_exit_shim(StreamWalkBack(cpu.x));
}

/*
 * @routine 0046:5E7 StreamWalkLines
 * @run 69:5DE7
 * WHAT IT DOES: Walks the stream line by line across a span, a walk at every step.
 * HOW: From the stream position, StreamWalkForward or Back by the sign of the step;
 * then adds the step to the position and counts the word in WorkByteA and B down.
 * WAITS: it can wait where a handler's code does.
 * Entry: zTemp04-05 position, zTemp06 range, StreamStep the step, WorkByteA-B the
 * count, X the caller's. Exit: A 0, N 0, Z 1, C from the count's borrow, V from the last
 * subtract, Y as the last walk left it. X kept.
 */
static Sum stream_walk_lines(uint8_t *xp)
{
    uint8_t x = *xp;
    Sum count_hi;

    do {
        uint8_t step_lo;
        uint8_t byte;
        Sum sum;
        Sum count_lo;
        uint8_t other;

        walk_start();
        step_lo = RD8(LEVEL->stream_step.lo);
        if (step_lo & 0x80) {
            StreamWalk walk;

            jsr_push(0x5E08);
            walk = StreamWalkBack(x);
            jsr_pop();
            x = walk.x;
        } else {
            StreamWalk walk;

            jsr_push(0x5E03);
            walk = StreamWalkForward(x);
            jsr_pop();
            x = walk.x;
        }
        byte = RD8(ZP->temp04);
        step_lo = RD8(LEVEL->stream_step.lo);
        sum = add_bytes(byte, step_lo, false);
        WR8(ZP->temp04, sum.value);
        byte = RD8(ZP->temp05);
        other = RD8(LEVEL->stream_step.hi);
        sum = add_bytes(byte, other, sum.carry);
        WR8(ZP->temp05, sum.value);
        byte = RD8(LEVEL->stream_count.lo);
        count_lo = sub_bytes(byte, 1, true);
        WR8(LEVEL->stream_count.lo, count_lo.value);
        byte = RD8(LEVEL->stream_count.hi);
        count_hi = sub_bytes(byte, 0, count_lo.carry);
        WR8(LEVEL->stream_count.hi, count_hi.value);
        other = RD8(LEVEL->stream_count.lo);
        byte = (uint8_t)(count_hi.value | other);
        if (byte == 0) {
            break;
        }
    } while (1);
    *xp = x;
    return count_hi;
}

void StreamWalkLines(uint8_t x)
{
    stream_walk_lines(&x);
}

/* The count ended by going from 1 to 0: A is the ORA's result, 0; every walk leaves Y 1. */
static void lines_exit_shim(Sum count)
{
    cpu.a = 0;
    cpu.y = 1;
    abi_nz(0);
    abi_flag(P_C, count.carry);
    abi_flag(P_V, count.overflow);
}

void abi_StreamWalkLines(void)
{
    {
        uint8_t x = cpu.x;

        lines_exit_shim(stream_walk_lines(&x));
    }
}

/* One walk of a full pass, from the scroll, one line in range at a time. */
static Sum walk_full_span(uint8_t *x, uint8_t step_lo, uint8_t step_hi, bool vertical, uint8_t count_hi,
                          uint16_t ret)
{
    uint8_t byte;
    Sum count;

    WR8(LEVEL->stream_step.lo, step_lo);
    WR8(LEVEL->stream_step.hi, step_hi);
    byte = vertical ? RD8(SCROLLE->y_hi_screen[0]) : RD8(SCROLLE->x_hi_screen[0]);
    WR8(ZP->temp05, byte);
    byte = vertical ? RD8(SCROLL->y_lo[0]) : RD8(SCROLL->x_lo[0]);
    WR8(ZP->temp04, byte);
    WR8(LEVEL->stream_count.hi, count_hi);
    WR8(LEVEL->stream_count.lo, 0x41);
    WR8(ZP->temp06, 1);
    jsr_push(ret);
    count = stream_walk_lines(x);
    jsr_pop();
    return count;
}

/*
 * @routine 0046:53E StreamFullWalk
 * @run 69:5D3E
 * WHAT IT DOES: Walks the stream across the screen and back when a level section is
 * entered, so everything that belongs on screen comes into being, with the full-walk
 * flag up for handlers that act only then.
 * HOW: A forward walk of $141 lines and a backward walk of $41 from the scroll on the
 * axis the level runs along.
 * WAITS: it can wait where a handler's code does.
 * Entry: LevelAxis, X the caller's. Exit: as StreamWalkLines; the flag down.
 * Clobbers zTemp04-06.
 */
static Sum stream_full_walk(uint8_t *x)
{
    uint8_t axis = RD8(LEVEL->level_axis);
    bool vertical = axis == AXIS_VERTICAL;
    Sum count;

    WR8(LEVEL->full_walk, 1);
    count = walk_full_span(x, 0x01, 0x00, vertical, 0x01, vertical ? 0x5DBE : 0x5D6D);
    count = walk_full_span(x, 0xFF, 0xFF, vertical, 0x00, vertical ? 0x5DE3 : 0x5D92);
    WR8(LEVEL->full_walk, 0);
    return count;
}

void StreamFullWalk(uint8_t x)
{
    stream_full_walk(&x);
}

void abi_StreamFullWalk(void)
{
    {
        uint8_t x = cpu.x;

        lines_exit_shim(stream_full_walk(&x));
    }
}

/* The scroll's position and speed on one axis, read where the edge walks read them. */
static uint8_t scroll_low(bool vertical)
{
    return vertical ? RD8(SCROLL->y_lo[0]) : RD8(SCROLL->x_lo[0]);
}

static uint8_t scroll_high(bool vertical)
{
    return vertical ? RD8(SCROLLE->y_hi_screen[0]) : RD8(SCROLLE->x_hi_screen[0]);
}

static uint8_t scroll_speed(bool vertical)
{
    return vertical ? RD8(SCROLLE->speed_y[0]) : RD8(SCROLLE->speed_x[0]);
}

/* A walk one edge past the screen, with the speed as its range. */
static StreamWalk edge_walk_forward(uint8_t x, bool vertical)
{
    uint8_t low = scroll_low(vertical);
    Sum lo = add_bytes(low, 0x40, false);
    uint8_t high;
    Sum hi;
    uint8_t speed;

    WR8(ZP->temp04, lo.value);
    high = scroll_high(vertical);
    hi = add_bytes(high, 0x01, lo.carry);
    WR8(ZP->temp05, hi.value);
    speed = scroll_speed(vertical);
    WR8(ZP->temp06, speed);
    return StreamWalkForward(x);
}

static StreamWalk edge_walk_back(uint8_t x, bool vertical)
{
    uint8_t low = scroll_low(vertical);
    Sum lo = add_bytes(low, 0xC0, false);
    uint8_t high;
    Sum hi;
    uint8_t speed;

    WR8(ZP->temp04, lo.value);
    high = scroll_high(vertical);
    hi = add_bytes(high, 0xFF, lo.carry);
    if (hi.value & 0x80) {
        StreamWalk before_start = { EDGE_BEFORE_START, false, false, 0, 0, hi, x };

        return before_start;
    }
    WR8(ZP->temp05, hi.value);
    speed = scroll_speed(vertical);
    speed = (uint8_t)((uint8_t)(speed ^ 0xFF) + 1);
    WR8(ZP->temp06, speed);
    return StreamWalkBack(x);
}

/*
 * @routine 0046:664 StreamEdgeRight
 * @run 69:5E64
 * WHAT IT DOES: Scrolling right: walks the stream at the screen's right edge.
 * HOW: Scroll plus $140 as the position, the scroll speed as the range.
 * WAITS: it can wait where a handler's code does.
 * Entry: zTemp10-11 and the record position set, X the caller's. Exit: as StreamWalkForward.
 */
StreamWalk StreamEdgeRight(uint8_t x)
{
    return edge_walk_forward(x, false);
}

void abi_StreamEdgeRight(void)
{
    stream_walk_exit_shim(StreamEdgeRight(cpu.x));
}

/*
 * @routine 0046:67B StreamEdgeLeft
 * @run 69:5E7B
 * WHAT IT DOES: Scrolling left: walks the stream back at the screen's left edge,
 * unless that lies before the level's start.
 * HOW: Scroll less $40 as the position, the scroll speed negated as the range.
 * WAITS: it can wait where a handler's code does.
 * Entry: as StreamEdgeRight. Exit: as StreamWalkBack; before the start, A the
 * position's high byte, N 1, Z 0, C and V from its add.
 */
StreamWalk StreamEdgeLeft(uint8_t x)
{
    return edge_walk_back(x, false);
}

void abi_StreamEdgeLeft(void)
{
    stream_walk_exit_shim(StreamEdgeLeft(cpu.x));
}

/*
 * @routine 0046:698 StreamEdgeDown
 * @run 69:5E98
 * WHAT IT DOES: Scrolling down: walks the stream at the screen's bottom edge.
 * HOW: As StreamEdgeRight on Y.
 * WAITS: it can wait where a handler's code does.
 * Entry and exit: as StreamEdgeRight.
 */
StreamWalk StreamEdgeDown(uint8_t x)
{
    return edge_walk_forward(x, true);
}

void abi_StreamEdgeDown(void)
{
    stream_walk_exit_shim(StreamEdgeDown(cpu.x));
}

/*
 * @routine 0046:6AF StreamEdgeUp
 * @run 69:5EAF
 * WHAT IT DOES: Scrolling up: walks the stream back at the screen's top edge, unless
 * that lies before the level's start.
 * HOW: As StreamEdgeLeft on Y.
 * WAITS: it can wait where a handler's code does.
 * Entry and exit: as StreamEdgeLeft.
 */
StreamWalk StreamEdgeUp(uint8_t x)
{
    return edge_walk_back(x, true);
}

void abi_StreamEdgeUp(void)
{
    stream_walk_exit_shim(StreamEdgeUp(cpu.x));
}

/*
 * @routine 0046:62E StreamScrollStep
 * @run 69:5E2E
 * WHAT IT DOES: Each frame, walks the stream at whichever screen edge the scroll moved
 * toward.
 * HOW: Picks the axis the level runs along and goes by the sign of its scroll speed;
 * no speed returns.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as the walk; no speed, A 0, N 0, Z 1, Y 1. C and V from the
 * compare and what came before.
 */
StreamWalk StreamScrollStep(uint8_t x)
{
    uint8_t axis;
    bool vertical;
    uint8_t speed;
    StreamWalk none = { STEP_NO_SPEED, false, false, 0, 0, { 0, false, false }, x };

    walk_start();
    axis = RD8(LEVEL->level_axis);
    vertical = axis == AXIS_VERTICAL;
    none.cmp_a = axis;
    speed = scroll_speed(vertical);
    if (speed == 0) {
        return none;
    }
    if (vertical) {
        return (speed & 0x80) ? StreamEdgeUp(x) : StreamEdgeDown(x);
    }
    return (speed & 0x80) ? StreamEdgeLeft(x) : StreamEdgeRight(x);
}

void abi_StreamScrollStep(void)
{
    uint8_t x = cpu.x;

    cpu.y = 1;
    stream_walk_exit_shim(StreamScrollStep(x));
}
