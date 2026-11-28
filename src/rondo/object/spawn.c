/* Finding a slot for a new object and starting it from its record. */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"

/*
 * @routine 0047:0E6 LoadObjectRecord
 * @run 6A:60E6
 * WHAT IT DOES: Starts an object from the stage's record for its type: copies the
 * record's twelve bytes into the object's fields and moves it on to its next
 * state.
 * HOW: The overlay's pointer at OverlayObjectRecords points to a table of records
 * by type minus one. The fourth byte packs two fields, a nibble each, and is read twice.
 * WAITS: no.
 * Entry: X object, its type set, stage overlay mapped, D clear. Exit: A the record's last
 * byte, Y 11, C bit 3 of the fourth byte, N and Z from the state field's new value.
 * X and V kept. Clobbers zTemp10-11, zMultiplierLo-Hi.
 */
uint8_t LoadObjectRecord(uint8_t object, uint8_t *exit_nz, bool *exit_c)
{
    uint8_t byte, index, state;

    byte = rd8(OverlayObjectRecords);
    WR8(ZP->multiplier.lo, byte);
    byte = rd8(OverlayObjectRecords + 1);
    WR8(ZP->multiplier.hi, byte);
    byte = RD8(OBJ->kind[object]);
    index = (uint8_t)((uint8_t)(byte - 1) << 1);
    byte = RD8_PTR_Y(ZP->multiplier.lo, index);
    WR8(ZP->temp10, byte);
    index++;
    byte = RD8_PTR_Y(ZP->multiplier.lo, index);
    WR8(ZP->temp11, byte);
    byte = RD8_PTR_Y(ZP->temp10, 0);
    WR8(OBJ->f3358[object], byte);
    byte = RD8_PTR_Y(ZP->temp10, 1);
    WR8(OBJ->f3300[object], byte);
    byte = RD8_PTR_Y(ZP->temp10, 2);
    WR8(OBJ->health[object], byte);
    byte = RD8_PTR_Y(ZP->temp10, 3);
    WR8(OBJ->hitbox_on[object], (uint8_t)(byte & 0x0F));
    byte = RD8_PTR_Y(ZP->temp10, 3);
    *exit_c = (byte & 0x08) != 0;
    WR8(OBJ->fields_2ee0[object], (uint8_t)(byte >> 4));
    byte = RD8_PTR_Y(ZP->temp10, 4);
    WR8(OBJ->hitbox_half_w[object], byte);
    byte = RD8_PTR_Y(ZP->temp10, 5);
    WR8(OBJ->hitbox_half_h[object], byte);
    byte = RD8_PTR_Y(ZP->temp10, 6);
    WR8(OBJ->f2a10[object], byte);
    byte = RD8_PTR_Y(ZP->temp10, 7);
    WR8(OBJ->frame[object], byte);
    byte = RD8_PTR_Y(ZP->temp10, 8);
    WR8(OBJ->loop_flags[object], byte);
    byte = RD8_PTR_Y(ZP->temp10, 9);
    WR8(OBJ->pace[object], byte);
    byte = RD8_PTR_Y(ZP->temp10, 10);
    WR8(OBJ->hit_status[object], byte);
    byte = RD8_PTR_Y(ZP->temp10, 11);
    WR8(OBJ->hit_reaction[object], byte);
    WR8(OBJ->hitbox_dx[object], 0);
    WR8(OBJ->hitbox_dy[object], 0);
    state = INC8(OBJ->state[object]);
    *exit_nz = state;
    return byte;
}

/* A the record's last byte, Y 11, N and Z from the new state, C bit 3 of the packed byte. */
void load_record_exit_shim(uint8_t last, uint8_t state, bool carry)
{
    cpu.a = last;
    cpu.y = 11;
    abi_nz(state);
    abi_flag(P_C, carry);
}

void abi_LoadObjectRecord(void)
{
    uint8_t nz;
    bool carry;
    uint8_t last = LoadObjectRecord(cpu.x, &nz, &carry);

    load_record_exit_shim(last, nz, carry);
}

/*
 * @routine 0047:15F CopyObjectPlace
 * @run 6A:615F
 * WHAT IT DOES: Puts one object where another is.
 * HOW: Copies the six position bytes field by field, each read and then stored.
 * WAITS: no.
 * Entry: X source, Y destination. Exit: A the last byte, N and Z from it. X, Y, C
 * and V kept.
 */
uint8_t CopyObjectPlace(uint8_t from, uint8_t to)
{
    uint8_t byte;

    byte = RD8(OBJ->x_hi[from]);
    WR8(OBJ->x_hi[to], byte);
    byte = RD8(OBJ->x_lo[from]);
    WR8(OBJ->x_lo[to], byte);
    byte = RD8(OBJ->x_frac[from]);
    WR8(OBJ->x_frac[to], byte);
    byte = RD8(OBJ->y_hi[from]);
    WR8(OBJ->y_hi[to], byte);
    byte = RD8(OBJ->y_lo[from]);
    WR8(OBJ->y_lo[to], byte);
    byte = RD8(OBJ->y_frac[from]);
    WR8(OBJ->y_frac[to], byte);
    return byte;
}

/* A the last byte copied, N and Z from it. */
void copy_place_exit_shim(uint8_t last)
{
    cpu.a = last;
    abi_nz(last);
}

void abi_CopyObjectPlace(void)
{
    copy_place_exit_shim(CopyObjectPlace(cpu.x, cpu.y));
}

/*
 * @routine 0047:15C StartObjectAtPlace
 * @run 6A:615C
 * WHAT IT DOES: Starts a new object of a type where another is.
 * HOW: Stores the type, then runs into CopyObjectPlace.
 * WAITS: no.
 * Entry: A type, X source, Y new object. Exit: as CopyObjectPlace.
 */
uint8_t StartObjectAtPlace(uint8_t type, uint8_t from, uint8_t to)
{
    WR8(OBJ->kind[to], type);
    return CopyObjectPlace(from, to);
}

void abi_StartObjectAtPlace(void)
{
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 0047:184 CopyObjectPlaceBack
 * @run 6A:6184
 * WHAT IT DOES: CopyObjectPlace in the other direction, from Y to X.
 * HOW: Swaps X and Y, calls CopyObjectPlace, swaps them back.
 * WAITS: no.
 * Entry: Y source, X destination. Exit: as CopyObjectPlace (X and Y as they were).
 */
uint8_t CopyObjectPlaceBack(uint8_t from, uint8_t to)
{
    uint8_t last;

    jsr_push(0x6188);
    last = CopyObjectPlace(from, to);
    jsr_pop();
    return last;
}

void abi_CopyObjectPlaceBack(void)
{
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, cpu.x));
}

/*
 * @routine 0047:18A StepObjectFrames
 * @run 6A:618A
 * WHAT IT DOES: Steps an object's animation list one frame. A list is pairs of
 * frames to hold (0 ends it) and a graphic; the list starts over from nothing after
 * its end. Says whether the graphic changed, and whether the list ended.
 * HOW: Holds while the timer runs; at zero, reads the next pair by the object's
 * position in the list, setting the timer one short for the frame just used.
 * WAITS: no.
 * Entry: X object, zTemp10-11 the list, D clear. Exit: changed: A $FF, C 1, N 1, Z 0, Y the
 * pair's second byte's index. Held: A $FF, C 0, N 1, Z 0, Y kept. Ended: A 0, C 0,
 * N 0, Z 1, Y the pair's index. X and V kept.
 */
FrameStep StepObjectFrames(uint8_t object, uint8_t *exit_y)
{
    uint8_t timer, index, byte;

    timer = RD8(OBJ->anim_timer[object]);
    if (timer != 0) {
        DEC8(OBJ->anim_timer[object]);
        return FRAME_HELD;
    }
    byte = RD8(OBJ->anim_step[object]);
    index = (uint8_t)(byte << 1);
    timer = RD8_PTR_Y(ZP->temp10, index);
    *exit_y = index;
    if (timer == 0) {
        WR8(OBJ->anim_step[object], 0);
        WR8(OBJ->anim_timer[object], 0);
        return FRAME_LIST_ENDED;
    }
    WR8(OBJ->anim_timer[object], timer);
    index++;
    *exit_y = index;
    byte = RD8_PTR_Y(ZP->temp10, index);
    WR8(OBJ->frame[object], byte);
    INC8(OBJ->anim_step[object]);
    DEC8(OBJ->anim_timer[object]);
    return FRAME_CHANGED;
}

/* Changed: A $FF, C 1; held: A $FF, C 0; ended: A 0, C 0. N and Z from A. Y as the body left it. */
void step_frames_exit_shim(FrameStep step, uint8_t y)
{
    cpu.y = y;
    cpu.a = step == FRAME_LIST_ENDED ? 0x00 : 0xFF;
    abi_nz(cpu.a);
    abi_flag(P_C, step == FRAME_CHANGED);
}

void abi_StepObjectFrames(void)
{
    uint8_t y = cpu.y;
    FrameStep step = StepObjectFrames(cpu.x, &y);

    step_frames_exit_shim(step, y);
}

/* The first empty slot in a range, emptied; found says whether there was one. */
static uint8_t find_free_slot(uint8_t slot, uint8_t end, uint8_t *exit_a, bool *found)
{
    uint8_t type;

    do {
        type = RD8(OBJ->kind[slot]);
        *exit_a = type;
        if (type == 0) {
            jsr_push(0x63D9);
            ClearObjectInY(slot);
            jsr_pop();
            *found = true;
            return slot;
        }
        slot++;
    } while (slot < end);
    *found = false;
    return slot;
}

/* Y the slot (or the end of the range), A the last type read, N 0, Z 1, C found. */
void find_slot_exit_shim(uint8_t slot, uint8_t last_type, bool found)
{
    cpu.y = slot;
    cpu.a = last_type;
    abi_nz(0);
    abi_flag(P_C, found);
}

/*
 * @routine 0047:3C8 FindFreeSlotFrom30
 * @run 6A:63C8
 * WHAT IT DOES: Finds an empty object slot from $30 to $3F and empties it fully.
 * HOW: The first with a zero type gets ClearObjectInY (through its vector).
 * WAITS: no.
 * Entry: nothing, D clear. Exit: found, C 1, Y the slot, A 0, N 0, Z 1. None, C 0, Y $40, A
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

    find_slot_exit_shim(slot, a, found);
}

/*
 * @routine 0047:3DB FindFreeSlotFrom20
 * @run 6A:63DB
 * WHAT IT DOES: Finds an empty object slot from $20 to $3F and empties it fully.
 * HOW: As FindFreeSlotFrom30, starting lower (it jumps into its loop).
 * WAITS: no.
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

    find_slot_exit_shim(slot, a, found);
}

/*
 * @routine 0047:3DF FindFreeSlotFrom50
 * @run 6A:63DF
 * WHAT IT DOES: Finds an empty object slot from $50 to $53 and empties it fully.
 * HOW: As FindFreeSlotFrom30, in its own loop that shares the found tail. The loop's later
 * rounds and the none-found exit ran in no script: the draft stops there, so liftcmp judges only
 * the slots it reaches; the rest waits for coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: nothing, D clear. Exit: as FindFreeSlotFrom30, with Y $54 for none.
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

    find_slot_exit_shim(slot, a, found);
}
