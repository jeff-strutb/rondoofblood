/*
 * TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
 * together here until each kind has a file of its own.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"

/*
 * @routine 07DB:5FD ClampSpeedXToTwo
 * @run 70:B5FD
 * WHAT IT DOES: Keeps an object's X speed from passing 2 pixels a frame either way: a speed over
 * +2 becomes +2.00 and one under -2 becomes -2.00.
 * HOW: Reads the speed's high byte (twice, as the sign test and the compare each load it) and,
 * past the limit, stores the limit in the high byte and zero in the low byte.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z, C from the last compare or load (C the compare's, also when the
 * speed is clamped); X and Y kept.
 */
Ended ClampSpeedXToTwo(uint8_t slot)
{
    uint8_t speed = RD8(OBJ->vx_hi[slot]);

    if ((speed & 0x80) != 0) {
        speed = RD8(OBJ->vx_hi[slot]);
        if (speed >= 0xFE) {
            return ended_compare(speed, 0xFE);
        }
        WR8(OBJ->vx_hi[slot], 0xFE);
        WR8(OBJ->vx_lo[slot], 0);
        return ended_with_carry(ended_load(0xFE), false);
    }
    speed = RD8(OBJ->vx_hi[slot]);
    if (speed < 2) {
        return ended_compare(speed, 2);
    }
    WR8(OBJ->vx_hi[slot], 2);
    WR8(OBJ->vx_lo[slot], 0);
    return ended_with_carry(ended_load(2), true);
}

void abi_ClampSpeedXToTwo(void)
{
    abi_ended(ClampSpeedXToTwo(cpu.x));
}

/*
 * @routine 07DB:622 ThrowUpward
 * @run 70:B622
 * WHAT IT DOES: Throws an object upward at 3 pixels a frame, unless it is already rising slowly
 * (its Y speed between -2.00 and 0 counted as a high byte of $FE or $FF).
 * HOW: Reads the Y speed's high byte; below $FE it stores $FD there and zero in the low byte.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z, C from the last compare or load; X and Y kept.
 */
Ended ThrowUpward(uint8_t slot)
{
    uint8_t speed = RD8(OBJ->vy_hi[slot]);

    if (speed >= 0xFE) {
        return ended_compare(speed, 0xFE);
    }
    WR8(OBJ->vy_hi[slot], 0xFD);
    WR8(OBJ->vy_lo[slot], 0);
    return ended_with_carry(ended_load(0xFD), false);
}

void abi_ThrowUpward(void)
{
    abi_ended(ThrowUpward(cpu.x));
}

/*
 * @routine 07DB:71B CountdownThenState4
 * @run 70:B71B
 * WHAT IT DOES: Counts the object's f3510 timer down and, when it reaches zero, sends the
 * object to state 4.
 * HOW: One decrement of the timer; the state byte is written only at zero.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the timer, or A 4 at zero; X, Y and C kept.
 */
Ended CountdownThenState4(uint8_t slot)
{
    return countdown_then_state(slot, COL(OBJ->f3510), 4);
}

void abi_CountdownThenState4(void)
{
    abi_ended(CountdownThenState4(cpu.x));
}

/*
 * @routine 07DB:785 ChangeToType9B
 * @run 70:B785
 * WHAT IT DOES: Turns the object into one of type $9B, in state 0.
 * HOW: Stores $9B in its type byte (OBJ_2750) and zero in its state.
 * WAITS: no.
 * Entry: X object. Exit: A $9B, N set, Z clear; X, Y and C kept.
 */
Ended ChangeToType9B(uint8_t slot)
{
    return become_type(slot, 0x9B);
}

void abi_ChangeToType9B(void)
{
    abi_ended(ChangeToType9B(cpu.x));
}

/*
 * @routine 07DB:7B1 EnterState6IfFlagged
 * @run 70:B7B1
 * WHAT IT DOES: Sends the object to state 6 when bit 7 of its f31a0 is set.
 * HOW: Reads f31a0 once and masks bit 7; writes the state only when it is set.
 * WAITS: no.
 * Entry: X object. Exit: A the masked bit, or 6 when entered; N, Z from it; X, Y and C kept.
 */
Ended EnterState6IfFlagged(uint8_t slot)
{
    return enter_state_if_flagged(slot, 6);
}

void abi_EnterState6IfFlagged(void)
{
    abi_ended(EnterState6IfFlagged(cpu.x));
}

/*
 * @routine 07DC:09D CountdownStopThenState4
 * @run 70:B89D
 * WHAT IT DOES: Counts the object's f34b8 timer down and, when it reaches zero, clears its f3408
 * and f3460 and sends it to state 4.
 * HOW: One decrement of the timer; the rest only at zero.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the timer, or A 4 at zero; X, Y and C kept.
 */
Ended CountdownStopThenState4(uint8_t slot)
{
    return countdown_stop_then_state(slot, COL(OBJ->f34b8), 4);
}

void abi_CountdownStopThenState4(void)
{
    abi_ended(CountdownStopThenState4(cpu.x));
}

/*
 * @routine 07DF:349 RightOfObject4DWithin
 * @run 71:D349
 * WHAT IT DOES: Says, in the carry, whether the object stands to the right of the object in slot
 * $4D by less than the distance in A, when slot $4D holds an object of type $AC.
 * HOW: Stores A as the limit in zTemp00; checks slot $4D's type; subtracts the two X positions
 * (low bytes into zTemp01, then high bytes); the answer is yes only when the high bytes are
 * equal and the low difference is under the limit.
 * WAITS: no.
 * Entry: X object, A limit. Exit: C set for yes; A the last value compared; N, Z from it. Clobbers
 * zTemp00-01. The arms after the type check ran in no script, so the draft stops there and
 * liftcmp judges only the type check; the rest waits for coverage (PROCEDURE section 6).
 */
Ended RightOfObject4DWithin(uint8_t slot, uint8_t limit)
{
    enum { OTHER = 0x4D, OTHER_TYPE = 0xAC };
    uint8_t type, low, other_low, high, other_high, diff_low, diff_high, kept;
    bool borrow, overflow;

    WR8(ZP->temp00, limit);
    type = RD8(OBJ->kind[OTHER]);
    if (type != OTHER_TYPE) {
        return ended_with_carry(ended_compare(type, OTHER_TYPE), false);
    }
    low = RD8(OBJ->x_lo[slot]);
    other_low = RD8(OBJ->x_lo[OTHER]);
    diff_low = (uint8_t)(low - other_low);
    borrow = low < other_low;
    WR8(ZP->temp01, diff_low);
    high = RD8(OBJ->x_hi[slot]);
    other_high = RD8(OBJ->x_hi[OTHER]);
    diff_high = (uint8_t)(high - other_high - (borrow ? 1 : 0));
    overflow = (((high ^ other_high) & (high ^ diff_high)) & 0x80) != 0;
    if (diff_high != 0) {
        return ended_with_overflow(ended_with_carry(ended_load(diff_high), false), overflow);
    }
    kept = RD8(ZP->temp01);
    limit = RD8(ZP->temp00);
    if (kept >= limit) {
        return ended_with_overflow(ended_with_carry(ended_compare(kept, limit), false), overflow);
    }
    return ended_with_overflow(ended_with_carry(ended_compare(kept, limit), true), overflow);
}

void abi_RightOfObject4DWithin(void)
{
    abi_ended(RightOfObject4DWithin(cpu.x, cpu.a));
}
