/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 025B (enemy kinds not yet identified), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l8.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/* CPY: N, Z and C from the compare; A kept. */
static Ended ended_compare_y(uint8_t y, uint8_t operand)
{
    return ended_with_carry(ended_nz((uint8_t)(y - operand)), y >= operand);
}

/*
 * @routine 025B:0F0 StepVerticalScrollTable
 * @run 70:B0F0
 * WHAT IT DOES: While OBJ_3568 is not 0, steps through the table at $B129 and $B133: OBJ_3510 takes the entry and the scroll's Y (ScrollYLo+1, ScrollYHi+1) falls by it; OBJ_3568 counts up and wraps to 0 at 9. When it is 0 and the scroll byte ScrollXLo+5 is 0, a random number with its top bit set starts the table (OBJ_3568 1).
 * HOW: A table read by OBJ_3568 as the index, a 16-bit subtract, and NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: the last instruction that ran.
 */
Ended StepVerticalScrollTable(uint8_t slot)
{
    uint8_t index, scroll, value, low, high, shifted, step;
    Sum diff;
    RandomResult r;
    Ended e;

    index = RD8(OBJ->f3568[slot]);
    e = ended_load(index);
    if (index == 0) {
        scroll = RD8(SCROLL->x_lo[5]);
        e = then(e, ended_load(scroll));
        if (scroll != 0) {
            return e;
        }
        jsr_push(0xB122);
        r = NextRandom();
        jsr_pop();
        e = then(e, ended_of_random(r));
        shifted = (uint8_t)(r.value << 1);
        e = then(e, ended_with_carry(ended_load(shifted), (r.value & 0x80) != 0));
        if ((r.value & 0x80) == 0) {
            return e;
        }
        step = INC8(OBJ->f3568[slot]);
        return then(e, ended_nz(step));
    }
    e = ended_with_y(e, index);
    value = rd8((uint16_t)(0xB129 + index));
    WR8(OBJ->f3510[slot], value);
    e = then(e, ended_load(value));
    low = RD8(SCROLL->y_lo[1]);
    value = rd8((uint16_t)(0xB129 + index));
    diff = sub_bytes(low, value, true);
    WR8(SCROLL->y_lo[1], diff.value);
    high = RD8(SCROLL->y_hi[1]);
    value = rd8((uint16_t)(0xB133 + index));
    diff = sub_bytes(high, value, diff.carry);
    WR8(SCROLL->y_hi[1], diff.value);
    e = then(e, ended_with_overflow(ended_with_carry(ended_load(diff.value), diff.carry), diff.overflow));
    step = INC8(OBJ->f3568[slot]);
    e = then(e, ended_nz(step));
    e = then(e, ended_compare_y(index, 9));
    if (index != 9) {
        return e;
    }
    WR8(OBJ->f3568[slot], 0);
    return e;
}

void abi_StepVerticalScrollTable(void)
{
    abi_ended(StepVerticalScrollTable(cpu.x));
}
