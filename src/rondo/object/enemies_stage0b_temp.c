/*
 * TEMPORARY: the stage 0B overlay's object routines whose enemy kind is not yet identified, kept
 * together here until each kind has a file of its own.
 * The disc tables the routines read are named here by their overlay address until the typed data
 * layer reads them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"

/*
 * @routine 117C:0EA CountdownThenState4B
 * @run 70:B8EA
 * WHAT IT DOES: The same as CountdownThenState4, at this overlay's copy of it.
 * HOW: The shared body; the f3510 timer is counted down and at zero the object enters state 4.
 * WAITS: no.
 * Entry, exit: as CountdownThenState4.
 */
Ended CountdownThenState4B(uint8_t slot)
{
    return countdown_then_state(slot, COL(OBJ->f3510), 4);
}

void abi_CountdownThenState4B(void)
{
    abi_ended(CountdownThenState4B(cpu.x));
}

/*
 * @routine 117C:154 ChangeToType9BB
 * @run 70:B954
 * WHAT IT DOES: The same as ChangeToType9B, at this overlay's copy of it.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as ChangeToType9B.
 */
Ended ChangeToType9BB(uint8_t slot)
{
    return become_type(slot, 0x9B);
}

void abi_ChangeToType9BB(void)
{
    abi_ended(ChangeToType9BB(cpu.x));
}

/*
 * @routine 117C:1C7 SixteenBitTimerToState8
 * @run 70:B9C7
 * WHAT IT DOES: While the object is in state 5, counts its sixteen bit timer (f35c0 low, f3568
 * high) down by one a frame; when the whole timer reaches zero the object enters state 8.
 * HOW: A subtract of one from the low byte with the borrow carried into the high byte; the two
 * are then ORed (the low byte read again) to test for zero.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the last compare, OR or load; C and V from the high
 * byte's subtract when it ran; X and Y kept.
 */
Ended SixteenBitTimerToState8(uint8_t slot)
{
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t low, high, both, new_low, new_high;
    bool borrow;
    Ended e;

    if (state != 5) {
        return ended_compare(state, 5);
    }
    low = RD8(OBJ->f35c0[slot]);
    new_low = (uint8_t)(low - 1);
    borrow = low == 0;
    WR8(OBJ->f35c0[slot], new_low);
    high = RD8(OBJ->f3568[slot]);
    new_high = (uint8_t)(high - (borrow ? 1 : 0));
    WR8(OBJ->f3568[slot], new_high);
    both = RD8(OBJ->f35c0[slot]);
    both = (uint8_t)(new_high | both);
    e = ended_with_overflow(ended_with_carry(ended_load(both), !(high == 0 && borrow)),
                            sub_overflow(high, 0, new_high));
    if (both != 0) {
        return e;
    }
    WR8(OBJ->state[slot], 8);
    return ended_with_overflow(ended_with_carry(ended_load(8), e.c), e.v);
}

void abi_SixteenBitTimerToState8(void)
{
    abi_ended(SixteenBitTimerToState8(cpu.x));
}

/*
 * @routine 117C:46E NoOpStateA
 * @run 70:BC6E
 * WHAT IT DOES: A state handler that does nothing.
 * HOW: Returns at once.
 * WAITS: no.
 * Entry: nothing. Exit: everything kept.
 */
Ended NoOpStateA(void)
{
    return ended_kept();
}

void abi_NoOpStateA(void)
{
    abi_ended(NoOpStateA());
}

/*
 * @routine 117C:4ED SetXSpeedFromFacing
 * @run 70:BCED
 * WHAT IT DOES: Sets the object's X speed from a two byte entry of a table by which way it faces
 * (the facing byte times two).
 * HOW: The table is at overlay address $BD0F: the low byte first, then the high byte.
 * WAITS: no.
 * Entry: X object. Exit: A the low speed byte; N, Z from it; C bit 7 of the facing byte; Y the
 * entry's offset; X kept.
 */
Ended SetXSpeedFromFacing(uint8_t slot)
{
    enum { TABLE = 0xBD0F };
    uint8_t facing = RD8(OBJ->facing_left[slot]);
    uint8_t y = (uint8_t)(facing << 1);
    uint8_t value;

    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(TABLE + 1 + y)));
    value = rd8((uint16_t)(TABLE + y));
    WR8(OBJ->vx_lo[slot], value);
    return ended_with_y(ended_with_carry(ended_load(value), (facing & 0x80) != 0), y);
}

void abi_SetXSpeedFromFacing(void)
{
    abi_ended(SetXSpeedFromFacing(cpu.x));
}

/*
 * @routine 117C:69D EnterState6WhenHitHard
 * @run 70:BE9D
 * WHAT IT DOES: Sends the object to state 6, step 0, when its f3670 is set, its state is under 6
 * and the hit it has taken is 5 or more.
 * HOW: The three tests in that order, each stopping at the first that fails. The last arm ran in
 * no script: the draft stops there; it waits for coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z, C from the last load or compare; X and Y kept.
 */
Ended EnterState6WhenHitHard(uint8_t slot)
{
    uint8_t flag = RD8(OBJ->f3670[slot]);
    uint8_t state, hit;

    if (flag == 0) {
        return ended_load(0);
    }
    state = RD8(OBJ->state[slot]);
    if (state >= 6) {
        return ended_compare(state, 6);
    }
    hit = RD8(OBJ->hit_taken[slot]);
    if (hit < 5) {
        return ended_compare(hit, 5);
    }
    WR8(OBJ->state[slot], 6);
    WR8(OBJ->f2800[slot], 0);
    return ended_load(6);
}

void abi_EnterState6WhenHitHard(void)
{
    abi_ended(EnterState6WhenHitHard(cpu.x));
}

/*
 * @routine 117C:6B9 EnterState7IfYHighOrFlagged
 * @run 70:BEB9
 * WHAT IT DOES: Sends the object to state 7 when its Y position's high byte is not zero or bit 7
 * of its status flags is set.
 * HOW: Tests the high byte, then the flag. The arm that enters the state ran in no script: the
 * draft stops there; it waits for coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object. Exit: A the last value loaded; N, Z from it; X, Y and C kept.
 */
Ended EnterState7IfYHighOrFlagged(uint8_t slot)
{
    uint8_t high = RD8(OBJ->y_hi[slot]);
    uint8_t flags, bit;

    if (high == 0) {
        flags = RD8(OBJ->status_flags[slot]);
        bit = (uint8_t)(flags & 0x80);
        if (bit == 0) {
            return ended_load(0);
        }
    }
    WR8(OBJ->state[slot], 7);
    return ended_load(7);
}

void abi_EnterState7IfYHighOrFlagged(void)
{
    abi_ended(EnterState7IfYHighOrFlagged(cpu.x));
}

/*
 * @routine 117C:7C5 NoOpStateB
 * @run 70:BFC5
 * WHAT IT DOES: A state handler that does nothing.
 * HOW: Returns at once.
 * WAITS: no.
 * Entry: nothing. Exit: everything kept.
 */
Ended NoOpStateB(void)
{
    return ended_kept();
}

void abi_NoOpStateB(void)
{
    abi_ended(NoOpStateB());
}

/*
 * @routine 117C:7FC LaunchHopByFacing
 * @run 70:BFFC
 * WHAT IT DOES: Launches the object in a hop like LaunchHopByF3510, with frame $38, this
 * overlay's table of X speeds at $C01E, indexed by which way the object faces.
 * HOW: The shared hop body.
 * WAITS: no.
 * Entry: X object. Exit: A 8; N, Z from the step byte's new value; Y the facing byte; X and C
 * kept.
 */
Ended LaunchHopByFacing(uint8_t slot)
{
    enum { TABLE = 0xC01E };
    uint8_t facing = RD8(OBJ->facing_left[slot]);

    return hop_from_index(slot, facing, TABLE, 0x38);
}

void abi_LaunchHopByFacing(void)
{
    abi_ended(LaunchHopByFacing(cpu.x));
}

/*
 * @routine 117D:041 CountdownStopThenState2
 * @run 71:C041
 * WHAT IT DOES: Counts the object's f35c0 timer down and, when it reaches zero, stops its
 * animation and sends it to state 2.
 * HOW: One decrement of the timer; the rest only at zero.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the timer, or A 2 at zero; X, Y and C kept.
 */
Ended CountdownStopThenState2(uint8_t slot)
{
    return countdown_stop_then_state(slot, COL(OBJ->f35c0), 2);
}

void abi_CountdownStopThenState2(void)
{
    abi_ended(CountdownStopThenState2(cpu.x));
}
