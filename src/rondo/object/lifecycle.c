/* Emptying object slots. */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"

/* Fields of one object lie $58 apart, one parallel array to a field. */
enum { OBJ_STRIDE = 0x58, OBJ_FIRST_ENEMY_SLOT = 0x10, OBJ_SLOT_END = 0x54 };

/*
 * @routine 0046:20B ClearObjectExceptPlace
 * @run 69:5A0B
 * WHAT IT DOES: Empties an object slot but leaves where it is: its position and
 * the fields ClearObject clears before this one stay.
 * HOW: Stores zero to each field in turn: three fields, then the velocities and
 * every field from there up to the last (an unnamed column one past the timer), $58 apart.
 * WAITS: no.
 * Entry: X object. Exit: A, X, Y and flags kept.
 */
void ClearObjectExceptPlace(uint8_t object)
{
    uint16_t column = COL(OBJ->vx_hi);
    uint16_t last = (uint16_t)(COL(OBJ->timer) + OBJ_STRIDE);

    WR8(OBJ->facing_left[object], 0);
    WR8(OBJ->flash_flags[object], 0);
    WR8(OBJ->f2800[object], 0);
    for (; column <= last; column += OBJ_STRIDE) {
        WR8_IDX(column, object, 0);
    }
}

void abi_ClearObjectExceptPlace(void)
{
    ClearObjectExceptPlace(cpu.x);
}

/*
 * @routine 0046:1ED ClearObject
 * @run 69:59ED
 * WHAT IT DOES: Empties an object slot altogether.
 * HOW: Zeroes the identity and position fields, then runs into
 * ClearObjectExceptPlace.
 * WAITS: no.
 * Entry: X object. Exit: A, X, Y and flags kept.
 */
void ClearObject(uint8_t object)
{
    WR8(OBJ->kind[object], 0);
    WR8(OBJ->state[object], 0);
    WR8(OBJ->f2a10[object], 0);
    WR8(OBJ->frame[object], 0);
    WR8(OBJ->x_hi[object], 0);
    WR8(OBJ->x_lo[object], 0);
    WR8(OBJ->x_frac[object], 0);
    WR8(OBJ->y_hi[object], 0);
    WR8(OBJ->y_lo[object], 0);
    WR8(OBJ->y_frac[object], 0);
    ClearObjectExceptPlace(object);
}

void abi_ClearObject(void)
{
    ClearObject(cpu.x);
}

/*
 * @routine 0046:1E7 ClearObjectInY
 * @run 69:59E7
 * WHAT IT DOES: ClearObject for a caller holding the slot in Y.
 * HOW: Swaps X and Y, calls ClearObject, swaps them back.
 * WAITS: no.
 * Entry: Y object. Exit: A, X, Y and flags kept.
 */
void ClearObjectInY(uint8_t object)
{
    jsr_push(0x59EB);
    ClearObject(object);
    jsr_pop();
}

void abi_ClearObjectInY(void)
{
    ClearObjectInY(cpu.y);
}

/*
 * @routine 0046:28D ClearEnemySlots
 * @run 69:5A8D
 * WHAT IT DOES: Empties every object slot from $10 to $53, the slots below $10
 * staying: the player and what travels with it.
 * HOW: Keeps X on the stack and calls ClearObject in a loop.
 * WAITS: no.
 * Entry: X (kept on the stack). Exit: X kept, N and Z from X, C 1. A and Y kept.
 */
void ClearEnemySlots(uint8_t entry_x)
{
    push8(entry_x);
    for (unsigned slot = OBJ_FIRST_ENEMY_SLOT; slot < OBJ_SLOT_END; slot++) {
        jsr_push(0x5A93);
        ClearObject((uint8_t)slot);
        jsr_pop();
    }
    (void)pull8();
}

void abi_ClearEnemySlots(void)
{
    ClearEnemySlots(cpu.x);
    abi_nz(cpu.x);
    abi_flag(P_C, true);
}
