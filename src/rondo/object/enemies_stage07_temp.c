/*
 * TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
 * together here until each kind has a file of its own.
 * The disc tables the routines read are named here by their overlay address until the typed data
 * layer reads them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"

/*
 * @routine 0BFB:3DC EnterState4IfFlagged
 * @run 70:B3DC
 * WHAT IT DOES: Sends the object to state 4 when bit 7 of its status flags is set.
 * HOW: Reads the flags once and masks bit 7; writes the state only when it is set.
 * WAITS: no.
 * Entry: X object. Exit: A the masked bit, or 4 when entered; N, Z from it; X, Y and C kept.
 */
Ended EnterState4IfFlagged(uint8_t slot)
{
    return enter_state_if_flagged(slot, 4);
}

void abi_EnterState4IfFlagged(void)
{
    abi_ended(EnterState4IfFlagged(cpu.x));
}

/*
 * @routine 0BFC:141 ScrollNotAtRightEnd
 * @run 70:B941
 * WHAT IT DOES: Says, in the carry, that the scroll has not come to the right end of the level:
 * carry set when zField82 is set, when two or more screens remain, or when one screen remains
 * and the scroll's low byte is under $E0; clear otherwise.
 * HOW: Subtracts the first scroll entry's high screen byte from the scroll's limit
 * (ScrollXMaxHi) and tests the difference.
 * WAITS: no.
 * Entry: nothing. Exit: C the answer; A and N, Z from the last load or compare; V from the
 * subtract. Y and X kept. The arms past the screen difference of 0 or 1 ran in no script: the
 * draft stops there and liftcmp judges only the first tests; the rest waits for coverage
 * (PROCEDURE section 6).
 */
Ended ScrollNotAtRightEnd(void)
{
    uint8_t stop = RD8(ZP->field82);
    uint8_t limit, screen, remaining, low;
    bool overflow;

    if (stop != 0) {
        return ended_with_carry(ended_load(stop), true);
    }
    limit = rd8(ScrollXMaxHi);
    screen = RD8(SCROLLE->x_hi_screen[0]);
    remaining = (uint8_t)(limit - screen);
    overflow = sub_overflow(limit, screen, remaining);
    if (remaining >= 2) {
        return ended_with_overflow(ended_compare(remaining, 2), overflow);
    }
    if (remaining == 0) {
        return ended_with_overflow(ended_with_carry(ended_compare(remaining, 0), false), overflow);
    }
    low = RD8(SCROLL->x_lo[0]);
    if (low >= 0xE0) {
        return ended_with_overflow(ended_with_carry(ended_compare(low, 0xE0), false), overflow);
    }
    return ended_with_overflow(ended_with_carry(ended_compare(low, 0xE0), true), overflow);
}

void abi_ScrollNotAtRightEnd(void)
{
    abi_ended(ScrollNotAtRightEnd());
}

/*
 * @routine 0BFC:2AB SetHitboxSizeFromFrame
 * @run 70:BAAB
 * WHAT IT DOES: Sets the object's hit rectangle's half width and half height from a two byte
 * row of a table by its frame minus $2A; frame 0 clears both.
 * HOW: The table is at overlay address $BAC9.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte loaded; N, Z from it; C bit 7 of the frame minus $2A
 * (the shift); V from the subtract; Y the row's offset; X kept.
 */
Ended SetHitboxSizeFromFrame(uint8_t slot)
{
    enum { TABLE = 0xBAC9 };
    uint8_t frame = RD8(OBJ->frame[slot]);
    uint8_t diff, y, value;
    bool overflow;

    if (frame == 0) {
        WR8(OBJ->hitbox_half_w[slot], 0);
        WR8(OBJ->hitbox_half_h[slot], 0);
        return ended_load(0);
    }
    diff = (uint8_t)(frame - 0x2A);
    overflow = sub_overflow(frame, 0x2A, diff);
    y = (uint8_t)(diff << 1);
    value = rd8((uint16_t)(TABLE + y));
    WR8(OBJ->hitbox_half_w[slot], value);
    value = rd8((uint16_t)(TABLE + 1 + y));
    WR8(OBJ->hitbox_half_h[slot], value);
    return ended_with_overflow(ended_with_y(ended_with_carry(ended_load(value), (diff & 0x80) != 0), y),
                               overflow);
}

void abi_SetHitboxSizeFromFrame(void)
{
    abi_ended(SetHitboxSizeFromFrame(cpu.x));
}

/*
 * @routine 0BFC:3B3 ChangeToType9C
 * @run 70:BBB3
 * WHAT IT DOES: Turns the object into one of type $9C, in state 0.
 * HOW: Stores $9C in its kind byte and zero in its state.
 * WAITS: no.
 * Entry: X object. Exit: A $9C, N set, Z clear; X, Y and C kept.
 */
Ended ChangeToType9C(uint8_t slot)
{
    return become_type(slot, 0x9C);
}

void abi_ChangeToType9C(void)
{
    abi_ended(ChangeToType9C(cpu.x));
}
