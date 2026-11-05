/* Emptying object slots. */
#include "machine.h"
#include "ram.h"

/* Fields of one object lie $58 apart, one parallel array to a field. */
enum { OBJ_STRIDE = 0x58, OBJ_FIRST_ENEMY_SLOT = 0x10, OBJ_SLOT_END = 0x54 };

/*
 * @routine 0046:20B ClearObjectExceptPlace
 * @run 69:5A0B
 * WHAT IT DOES: Empties an object slot but leaves where it is: its position and
 * the fields ClearObject clears before this one stay.
 * HOW: Stores zero to each field in turn: three fields, then the velocities and
 * every field from there up to the last at $3AE8, $58 apart.
 * Entry: X object. Exit: A, X, Y and flags kept.
 */
void ClearObjectExceptPlace(uint8_t object)
{
    wr8((uint16_t)(OBJ_2AC0 + object), 0);
    wr8((uint16_t)(OBJ_2B18 + object), 0);
    wr8((uint16_t)(OBJ_2800 + object), 0);
    for (uint16_t field = OBJ_VX_HI; field <= 0x3AE8; field += OBJ_STRIDE)
        wr8((uint16_t)(field + object), 0);
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
 * Entry: X object. Exit: A, X, Y and flags kept.
 */
void ClearObject(uint8_t object)
{
    static const uint16_t fields[] = {
        OBJ_2750, OBJ_27A8, OBJ_2A10, OBJ_2A68, OBJ_X_HI,
        OBJ_X_LO, OBJ_X_FRAC, OBJ_Y_HI, OBJ_Y_LO, OBJ_Y_FRAC
    };

    for (unsigned i = 0; i < sizeof fields / sizeof fields[0]; i++)
        wr8((uint16_t)(fields[i] + object), 0);
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
 * HOW: Swaps X and Y around the call.
 * Entry: Y object. Exit: A, X, Y and flags kept.
 */
void abi_ClearObjectInY(void)
{
    ClearObject(cpu.y);
}

/*
 * @routine 0046:28D ClearEnemySlots
 * @run 69:5A8D
 * WHAT IT DOES: Empties every object slot from $10 to $53, the slots below $10
 * staying: the player and what travels with it.
 * HOW: ClearObject in a loop.
 * Entry: nothing. Exit: X kept, N and Z from X, C 1. A and Y kept.
 */
void ClearEnemySlots(void)
{
    for (unsigned slot = OBJ_FIRST_ENEMY_SLOT; slot < OBJ_SLOT_END; slot++)
        ClearObject((uint8_t)slot);
}

void abi_ClearEnemySlots(void)
{
    ClearEnemySlots();
    abi_nz(cpu.x);
    abi_flag(P_C, true);
}
