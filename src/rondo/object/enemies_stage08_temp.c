/*
 * TEMPORARY: the stage 08 overlay's object routines whose enemy kind is not yet identified, kept
 * together here until each kind has a file of its own.
 * The disc tables the routines read are named here by their overlay address until the typed data
 * layer reads them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"

/* Stores a constant in the object's f34b8 (here the phase index of the speed table below). */
static Ended set_phase(uint8_t slot, uint8_t phase)
{
    WR8(OBJ->life_timer[slot], phase);
    return ended_load(phase);
}

/*
 * @routine 0D5B:200 SetPhase2
 * @run 70:B200
 * WHAT IT DOES: Sets the object's f34b8 to 2, the phase SetSpeedFromPhaseTable reads.
 * HOW: One store.
 * WAITS: no.
 * Entry: X object. Exit: A 2, N and Z clear; X, Y, C and V kept.
 */
Ended SetPhase2(uint8_t slot)
{
    return set_phase(slot, 2);
}

void abi_SetPhase2(void)
{
    abi_ended(SetPhase2(cpu.x));
}

/*
 * @routine 0D5B:206 SetPhase3
 * @run 70:B206
 * WHAT IT DOES: Sets the object's f34b8 to 3, the phase SetSpeedFromPhaseTable reads.
 * HOW: One store.
 * WAITS: no.
 * Entry: X object. Exit: A 3, N and Z clear; X, Y, C and V kept.
 */
Ended SetPhase3(uint8_t slot)
{
    return set_phase(slot, 3);
}

void abi_SetPhase3(void)
{
    abi_ended(SetPhase3(cpu.x));
}

/*
 * @routine 0D5B:20C SetPhase4
 * @run 70:B20C
 * WHAT IT DOES: Sets the object's f34b8 to 4, the phase SetSpeedFromPhaseTable reads.
 * HOW: One store.
 * WAITS: no.
 * Entry: X object. Exit: A 4, N and Z clear; X, Y, C and V kept.
 */
Ended SetPhase4(uint8_t slot)
{
    return set_phase(slot, 4);
}

void abi_SetPhase4(void)
{
    abi_ended(SetPhase4(cpu.x));
}

/*
 * @routine 0D5B:212 SetSpeedFromPhaseTable
 * @run 70:B212
 * WHAT IT DOES: Sets the object's X and Y speeds from a table of four byte entries, the entry
 * picked by the object's phase (f34b8) from a block chosen by the way it faces, and moves the
 * object on to its next state.
 * HOW: A table at overlay address $B23F gives each facing's block start; the entry's bytes are
 * at $B241 to $B244 from there (X low, X high, Y low, Y high; the loads run high byte first).
 * The block start passes through zTemp00.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte loaded; N, Z from the new state; C and V from the add
 * of the block start; Y the entry's offset; X kept. Clobbers zTemp00.
 */
Ended SetSpeedFromPhaseTable(uint8_t slot)
{
    enum { BLOCKS = 0xB23F, ENTRIES = 0xB241 };
    uint8_t facing = RD8(OBJ->facing_left[slot]);
    uint8_t phase, start, y, value, state, scaled;
    unsigned total;

    WR8(ZP->temp00, rd8((uint16_t)(BLOCKS + facing)));
    phase = RD8(OBJ->life_timer[slot]);
    scaled = (uint8_t)(phase << 2);
    start = RD8(ZP->temp00);
    total = (unsigned)scaled + start;
    y = (uint8_t)total;
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(ENTRIES + 1 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(ENTRIES + y)));
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(ENTRIES + 3 + y)));
    value = rd8((uint16_t)(ENTRIES + 2 + y));
    WR8(OBJ->vy_lo[slot], value);
    state = RD8(OBJ->state[slot]);
    state = (uint8_t)(state + 1);
    WR8(OBJ->state[slot], state);
    return ended_with_overflow(
        ended_with_y(ended_with_carry(ended_with_a(ended_nz(state), value), total > 0xFF), y),
        add_overflow(scaled, start, y));
}

void abi_SetSpeedFromPhaseTable(void)
{
    abi_ended(SetSpeedFromPhaseTable(cpu.x));
}

/*
 * @routine 0D5E:14F EnterState6IfFlaggedB
 * @run 71:C94F
 * WHAT IT DOES: Sends the object to state 6 when bit 7 of its status flags is set (the same as
 * EnterState6IfFlagged, in this stage's overlay).
 * HOW: Reads the flags once and masks bit 7; writes the state only when it is set.
 * WAITS: no.
 * Entry: X object. Exit: A the masked bit, or 6 when entered; N, Z from it; X, Y and C kept.
 */
Ended EnterState6IfFlaggedB(uint8_t slot)
{
    return enter_state_if_flagged(slot, 6);
}

void abi_EnterState6IfFlaggedB(void)
{
    abi_ended(EnterState6IfFlaggedB(cpu.x));
}

/*
 * @routine 0D5E:171 AdvanceState
 * @run 71:C971
 * WHAT IT DOES: Moves the object on to its next state.
 * HOW: One increment of the state byte.
 * WAITS: no.
 * Entry: X object. Exit: N, Z from the new state; A, X, Y, C and V kept.
 */
Ended AdvanceState(uint8_t slot)
{
    uint8_t state = RD8(OBJ->state[slot]);

    state = (uint8_t)(state + 1);
    WR8(OBJ->state[slot], state);
    return ended_nz(state);
}

void abi_AdvanceState(void)
{
    abi_ended(AdvanceState(cpu.x));
}

/*
 * @routine 0D5E:6A6 EnterState3WithTallHitbox
 * @run 71:CEA6
 * WHAT IT DOES: Puts the object in state 3 with a narrow, tall hit rectangle (half width 6, half
 * height $18, no offsets), f3358 set to 2 and its step byte cleared.
 * HOW: Constant stores.
 * WAITS: no.
 * Entry: X object. Exit: A 3, N and Z clear; X, Y, C and V kept.
 */
Ended EnterState3WithTallHitbox(uint8_t slot)
{
    WR8(OBJ->f3358[slot], 2);
    WR8(OBJ->hitbox_half_w[slot], 6);
    WR8(OBJ->hitbox_half_h[slot], 0x18);
    WR8(OBJ->hitbox_dx[slot], 0);
    WR8(OBJ->hitbox_dy[slot], 0);
    WR8(OBJ->state[slot], 3);
    WR8(OBJ->f2800[slot], 0);
    return ended_load(3);
}

void abi_EnterState3WithTallHitbox(void)
{
    abi_ended(EnterState3WithTallHitbox(cpu.x));
}

/*
 * @routine 0D5E:6EE SetHitboxFromFrameRowsB
 * @run 71:CEEE
 * WHAT IT DOES: Sets the object's whole hit rectangle from a four byte row of a table, the row
 * chosen by the frame minus $5A; frames under $5A leave it alone.
 * HOW: The table is at overlay address $CF12.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte loaded (or the frame minus $5A when under); N, Z from
 * it; C clear when under, else bit 6 of the row number; V from the subtract; Y the row's offset
 * when read; X kept.
 */
Ended SetHitboxFromFrameRowsB(uint8_t slot)
{
    enum { TABLE = 0xCF12 };
    uint8_t frame = RD8(OBJ->frame[slot]);
    uint8_t row = (uint8_t)(frame - 0x5A);
    uint8_t y, value;
    bool overflow = sub_overflow(frame, 0x5A, row);

    if (frame < 0x5A) {
        return ended_with_overflow(ended_with_carry(ended_load(row), false), overflow);
    }
    y = (uint8_t)(row << 2);
    value = rd8((uint16_t)(TABLE + y));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(TABLE + 1 + y));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(TABLE + 2 + y));
    WR8(OBJ->hitbox_half_w[slot], value);
    value = rd8((uint16_t)(TABLE + 3 + y));
    WR8(OBJ->hitbox_half_h[slot], value);
    return ended_with_overflow(ended_with_y(ended_with_carry(ended_load(value), (row & 0x40) != 0), y),
                               overflow);
}

void abi_SetHitboxFromFrameRowsB(void)
{
    abi_ended(SetHitboxFromFrameRowsB(cpu.x));
}
