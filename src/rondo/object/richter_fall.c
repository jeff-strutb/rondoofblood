/*
 * Richter in the air, from his overlay at $A000 (sector $1B9): the jump and the fall (states 3, 4, 11
 * and 12), and the landing that ends them.
 *
 * A body returns plain values; its abi_ wrapper gives the registers and flags the assembly leaves,
 * replaying the exits of the callees whose registers and flags survive to the end.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"
#include "access.h"
#include "enemy_objects.h"
#include "player_objects.h"
#include "richter.h"

enum {
    COLUMN_DEBRIS_BIT = 0x08, SOUND_DEBRIS = 0x23, SLOT_LAST_EXTRA = 2, KIND_CLEARED_BY_DEBRIS = 1,
    SCRIPT_DEBRIS = 0x08, DEBRIS_BELOW = 0x10, DEBRIS_TIMER = 0, SCRIPT_SLOT = 7
};

/*
 * @routine 01BA:68B RichterDebrisOnMarkedColumns
 * @run 6D:AE8B
 * WHAT IT DOES: When a column read under him has bit 3: sound $23; each of slots 2 down to 1 holding
 * kind 1 emptied, with script 8; a burst of debris 16 pixels below him.
 * HOW: RichterColumnHit's three bytes ORed; SoundCommand and ClearObject through their kernel
 * vectors, RichterStartHitScript. The script leaves X 7, so the count goes on down from 6 after it.
 * His Y is pushed, moved down for RichterStartDebrisBurst with timer 0, and pulled back.
 * WAITS: no.
 * Entry: X and Y for the sound driver. Exit: no bit 3: A 0, N clear, Z set; X, Y, C and V kept.
 * Otherwise A his Y low byte, N and Z from it; X 0; Y 8 if a slot was emptied, else kept; C as
 * RichterStartDebrisBurst; V from adding the carry to his Y high byte.
 */
DebrisColumns RichterDebrisOnMarkedColumns(uint8_t x, uint8_t y)
{
    DebrisColumns r = { 0 };
    Sum low, high;
    uint8_t value, slot;

    value = RD8(PVARS->column_hit[0]);
    value |= RD8(PVARS->column_hit[1]);
    value |= RD8(PVARS->column_hit[2]);
    if (!(value & COLUMN_DEBRIS_BIT)) {
        return r;
    }
    r.burst_started = true;
    jsr_push(0xAE9E);
    SoundCommand(SOUND_DEBRIS, x, y);
    jsr_pop();
    for (slot = SLOT_LAST_EXTRA; slot != 0; slot--) {
        if (RD8(OBJ->kind[slot]) != KIND_CLEARED_BY_DEBRIS) {
            continue;
        }
        jsr_push(0xAEA9);
        ClearObject(slot);
        jsr_pop();
        jsr_push(0xAEAE);
        RichterStartHitScript(SCRIPT_DEBRIS);
        jsr_pop();
        slot = SCRIPT_SLOT;
        y = SCRIPT_DEBRIS;
    }
    value = RD8(OBJ->y_lo[0]);
    push8(value);
    low = add_bytes(value, DEBRIS_BELOW, false);
    WR8(OBJ->y_lo[0], low.value);
    value = RD8(OBJ->y_hi[0]);
    push8(value);
    high = add_bytes(value, 0, low.carry);
    WR8(OBJ->y_hi[0], high.value);
    r.overflow = high.overflow;
    jsr_push(0xAEC7);
    r.burst = RichterStartDebrisBurst(0, DEBRIS_TIMER, y);
    jsr_pop();
    value = pull8();
    WR8(OBJ->y_hi[0], value);
    value = pull8();
    WR8(OBJ->y_lo[0], value);
    r.y_low = value;
    return r;
}

void debris_columns_exit_shim(DebrisColumns r)
{
    if (!r.burst_started) {
        abi_ended(ended_load(0));
        return;
    }
    cpu.x = 0;
    abi_flag(P_V, r.overflow);
    abi_ended(start_debris_burst_exit_shim(r.burst));
    abi_ended(ended_load(r.y_low));
}

void abi_RichterDebrisOnMarkedColumns(void)
{
    debris_columns_exit_shim(RichterDebrisOnMarkedColumns(cpu.x, cpu.y));
}
