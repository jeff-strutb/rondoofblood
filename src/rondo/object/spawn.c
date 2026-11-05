/* Finding a slot for a new object and starting it from its record. */
#include "machine.h"
#include "ram.h"
#include "routines.h"

/* Fields of an object record, in the order it stores them after the first four. */
static const uint16_t record_tail_fields[] = {
    OBJ_3988, OBJ_3A38, OBJ_2A10, OBJ_2A68, OBJ_30F0, OBJ_3148, OBJ_31A0, OBJ_31F8
};

/*
 * @routine 0047:0E6 LoadObjectRecord
 * @run 6A:60E6
 * WHAT IT DOES: Starts an object from the stage's record for its type: copies the
 * record's twelve bytes into the object's fields and moves it on to its next
 * state.
 * HOW: The overlay's pointer at OverlayObjectRecords points to a table of records
 * by type minus one. The fourth byte packs two fields, a nibble each.
 * Entry: X object, its type set, stage overlay mapped. Exit: A the record's last
 * byte, Y 11, C bit 3 of the fourth byte, N and Z from the state field's new value.
 * X and V kept. Clobbers zTemp10-11, zMultiplierLo-Hi.
 */
uint8_t LoadObjectRecord(uint8_t object, uint8_t *exit_nz, bool *exit_c)
{
    uint8_t byte;
    uint8_t index;

    byte = rd8(OverlayObjectRecords);
    wr8(zMultiplierLo, byte);
    byte = rd8(OverlayObjectRecords + 1);
    wr8(zMultiplierHi, byte);
    byte = rd8((uint16_t)(OBJ_2750 + object));
    index = (uint8_t)((uint8_t)(byte - 1) << 1);
    byte = rd8_zp_y(zMultiplierLo, index);
    wr8(zTemp10, byte);
    byte = rd8_zp_y(zMultiplierLo, (uint8_t)(index + 1));
    wr8(zTemp11, byte);
    byte = rd8_zp_y(zTemp10, 0);
    wr8((uint16_t)(OBJ_3358 + object), byte);
    byte = rd8_zp_y(zTemp10, 1);
    wr8((uint16_t)(OBJ_3300 + object), byte);
    byte = rd8_zp_y(zTemp10, 2);
    wr8((uint16_t)(OBJ_32A8 + object), byte);
    byte = rd8_zp_y(zTemp10, 3);
    wr8((uint16_t)(OBJ_3250 + object), byte & 0x0F);
    byte = rd8_zp_y(zTemp10, 3);
    *exit_c = (byte & 0x08) != 0;
    wr8((uint16_t)(OBJ_FIELDS_2EE0 + object), byte >> 4);
    for (unsigned i = 0; i < sizeof record_tail_fields / sizeof record_tail_fields[0]; i++) {
        byte = rd8_zp_y(zTemp10, (uint8_t)(4 + i));
        wr8((uint16_t)(record_tail_fields[i] + object), byte);
    }
    wr8((uint16_t)(OBJ_3930 + object), 0);
    wr8((uint16_t)(OBJ_39E0 + object), 0);
    *exit_nz = (uint8_t)(rd8((uint16_t)(OBJ_27A8 + object)) + 1);
    wr8((uint16_t)(OBJ_27A8 + object), *exit_nz);
    return byte;
}

void abi_LoadObjectRecord(void)
{
    uint8_t nz;
    bool carry;

    cpu.a = LoadObjectRecord(cpu.x, &nz, &carry);
    cpu.y = 11;
    abi_nz(nz);
    abi_flag(P_C, carry);
}

/*
 * @routine 0047:15F CopyObjectPlace
 * @run 6A:615F
 * WHAT IT DOES: Puts one object where another is.
 * HOW: Copies the six position bytes field by field.
 * Entry: X source, Y destination. Exit: A the last byte, N and Z from it. X, Y, C
 * and V kept.
 */
uint8_t CopyObjectPlace(uint8_t from, uint8_t to)
{
    static const uint16_t fields[] = {
        OBJ_X_HI, OBJ_X_LO, OBJ_X_FRAC, OBJ_Y_HI, OBJ_Y_LO, OBJ_Y_FRAC
    };
    uint8_t byte = 0;

    for (unsigned i = 0; i < sizeof fields / sizeof fields[0]; i++) {
        byte = rd8((uint16_t)(fields[i] + from));
        wr8((uint16_t)(fields[i] + to), byte);
    }
    return byte;
}

void abi_CopyObjectPlace(void)
{
    cpu.a = CopyObjectPlace(cpu.x, cpu.y);
    abi_nz(cpu.a);
}

/*
 * @routine 0047:15C StartObjectAtPlace
 * @run 6A:615C
 * WHAT IT DOES: Starts a new object of a type where another is.
 * HOW: Stores the type, then runs into CopyObjectPlace.
 * Entry: A type, X source, Y new object. Exit: as CopyObjectPlace.
 */
uint8_t StartObjectAtPlace(uint8_t type, uint8_t from, uint8_t to)
{
    wr8((uint16_t)(OBJ_2750 + to), type);
    return CopyObjectPlace(from, to);
}

void abi_StartObjectAtPlace(void)
{
    cpu.a = StartObjectAtPlace(cpu.a, cpu.x, cpu.y);
    abi_nz(cpu.a);
}

/*
 * @routine 0047:184 CopyObjectPlaceBack
 * @run 6A:6184
 * WHAT IT DOES: CopyObjectPlace in the other direction, from Y to X.
 * HOW: Swaps X and Y around the call.
 * Entry: Y source, X destination. Exit: as CopyObjectPlace.
 */
void abi_CopyObjectPlaceBack(void)
{
    cpu.a = CopyObjectPlace(cpu.y, cpu.x);
    abi_nz(cpu.a);
}

typedef enum { FRAME_HELD, FRAME_CHANGED, FRAME_LIST_ENDED } FrameStep;

/*
 * @routine 0047:18A StepObjectFrames
 * @run 6A:618A
 * WHAT IT DOES: Steps an object's animation list one frame. A list is pairs of
 * frames to hold (0 ends it) and a graphic; the list starts over from nothing after
 * its end. Says whether the graphic changed, and whether the list ended.
 * HOW: Holds while the timer runs; at zero, reads the next pair by the object's
 * position in the list, setting the timer one short for the frame just used.
 * Entry: X object, zTemp10-11 the list. Exit: changed: A $FF, C 1, N 1, Z 0, Y the
 * pair's second byte's index. Held: A $FF, C 0, N 1, Z 0, Y kept. Ended: A 0, C 0,
 * N 0, Z 1, Y the pair's index. X and V kept.
 */
FrameStep StepObjectFrames(uint8_t object, uint8_t *exit_y)
{
    uint8_t timer = rd8((uint16_t)(OBJ_3460 + object));
    uint8_t index;
    uint8_t byte;

    if (timer != 0) {
        byte = rd8((uint16_t)(OBJ_3460 + object));
        wr8((uint16_t)(OBJ_3460 + object), (uint8_t)(byte - 1));
        return FRAME_HELD;
    }
    index = (uint8_t)(rd8((uint16_t)(OBJ_3408 + object)) << 1);
    timer = rd8_zp_y(zTemp10, index);
    *exit_y = index;
    if (timer == 0) {
        wr8((uint16_t)(OBJ_3408 + object), 0);
        wr8((uint16_t)(OBJ_3460 + object), 0);
        return FRAME_LIST_ENDED;
    }
    wr8((uint16_t)(OBJ_3460 + object), timer);
    index++;
    *exit_y = index;
    byte = rd8_zp_y(zTemp10, index);
    wr8((uint16_t)(OBJ_2A68 + object), byte);
    byte = (uint8_t)(rd8((uint16_t)(OBJ_3408 + object)) + 1);
    wr8((uint16_t)(OBJ_3408 + object), byte);
    byte = rd8((uint16_t)(OBJ_3460 + object));
    wr8((uint16_t)(OBJ_3460 + object), (uint8_t)(byte - 1));
    return FRAME_CHANGED;
}

void abi_StepObjectFrames(void)
{
    uint8_t y = cpu.y;
    FrameStep step = StepObjectFrames(cpu.x, &y);

    cpu.y = y;
    cpu.a = step == FRAME_LIST_ENDED ? 0x00 : 0xFF;
    abi_nz(cpu.a);
    abi_flag(P_C, step == FRAME_CHANGED);
}

/* The first empty slot in a range, emptied; found says whether there was one. */
static uint8_t find_free_slot(uint8_t slot, uint8_t end, uint8_t *exit_a, bool *found)
{
    uint8_t type;

    do {
        type = rd8((uint16_t)(OBJ_2750 + slot));
        *exit_a = type;
        if (type == 0) {
            ClearObject(slot);
            *found = true;
            return slot;
        }
        slot++;
    } while (slot < end);
    *found = false;
    return slot;
}

static void abi_exit_slot(uint8_t slot, uint8_t a, bool found)
{
    cpu.y = slot;
    cpu.a = a;
    abi_flag(P_C, found);
    abi_nz(found ? a : 0);
}

/*
 * @routine 0047:3C8 FindFreeSlotFrom30
 * @run 6A:63C8
 * WHAT IT DOES: Finds an empty object slot from $30 to $3F and empties it fully.
 * HOW: The first with a zero type gets ClearObject.
 * Entry: nothing. Exit: found, C 1, Y the slot, A 0, N 0, Z 1. None, C 0, Y $40, A
 * the last type, N 0, Z 1. X and V kept.
 */
uint8_t FindFreeSlotFrom30(uint8_t *exit_a, bool *found)
{
    return find_free_slot(0x30, 0x40, exit_a, found);
}

void abi_FindFreeSlotFrom30(void)
{
    uint8_t a;
    bool found;
    uint8_t slot = FindFreeSlotFrom30(&a, &found);

    abi_exit_slot(slot, a, found);
}

/*
 * @routine 0047:3DB FindFreeSlotFrom20
 * @run 6A:63DB
 * WHAT IT DOES: Finds an empty object slot from $20 to $3F and empties it fully.
 * HOW: As FindFreeSlotFrom30, starting lower.
 * Entry and exit: as FindFreeSlotFrom30.
 */
uint8_t FindFreeSlotFrom20(uint8_t *exit_a, bool *found)
{
    return find_free_slot(0x20, 0x40, exit_a, found);
}

void abi_FindFreeSlotFrom20(void)
{
    uint8_t a;
    bool found;
    uint8_t slot = FindFreeSlotFrom20(&a, &found);

    abi_exit_slot(slot, a, found);
}

/*
 * @routine 0047:3DF FindFreeSlotFrom50
 * @run 6A:63DF
 * WHAT IT DOES: Finds an empty object slot from $50 to $53 and empties it fully.
 * HOW: As FindFreeSlotFrom30, in its own loop.
 * Entry: nothing. Exit: as FindFreeSlotFrom30, with Y $54 for none.
 */
uint8_t FindFreeSlotFrom50(uint8_t *exit_a, bool *found)
{
    return find_free_slot(0x50, 0x54, exit_a, found);
}

void abi_FindFreeSlotFrom50(void)
{
    uint8_t a;
    bool found;
    uint8_t slot = FindFreeSlotFrom50(&a, &found);

    abi_exit_slot(slot, a, found);
}
