/*
 * TEMPORARY: routines of the four stages' enemies whose kind is not yet identified, kept here
 * until each kind has a file named by what it is.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"

/*
 * @routine 093C:068 EnterStateFourOnFlag
 * @run 70:B868
 * WHAT IT DOES: Switches an object that is not yet in state 4 into state 4, with its animation
 * list restarted, once bit 7 of OBJ_31A0 is set.
 * HOW: Reads the state and the flag once each; clears the two animation counters and stores 4.
 * WAITS: no.
 * Entry: X object. Exit: A and flags as the last compare, AND or load left them (see the
 * wrapper). X and Y kept. D clear. The bank is that of the other stage copies' dumps.
 */
State4Result EnterStateFourOnFlag(uint8_t slot)
{
    State4Result result = { STATE4_ALREADY, false };
    uint8_t state, flags;

    state = RD8(OBJ->state[slot]);
    result.above = state > 4;
    if (state == 4) {
        return result;
    }
    flags = RD8(OBJ->status_flags[slot]);
    if ((flags & 0x80) == 0) {
        result.end = STATE4_NO_FLAG;
        return result;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 4);
    result.end = STATE4_ENTERED;
    return result;
}

void abi_EnterStateFourOnFlag(void)
{
    State4Result result = EnterStateFourOnFlag(cpu.x);

    switch (result.end) {
    case STATE4_ALREADY:
        cpu.a = 4;
        abi_nz(0);
        abi_flag(P_C, true);
        break;
    case STATE4_NO_FLAG:
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, result.above);
        break;
    case STATE4_ENTERED:
        cpu.a = 4;
        abi_nz(4);
        abi_flag(P_C, result.above);
        break;
    }
}

/*
 * @routine 0CAB:260 SetHitboxFromFrame
 * @run 70:B260
 * WHAT IT DOES: Loads an object's hitbox (dx, dy, half width, half height: OBJ_3930, OBJ_39E0, OBJ_3988, OBJ_3A38)
 * from a four-byte row of a table, picked by the object's animation frame through a second table
 * of row numbers; no frame, or a row number 0, clears the last two.
 * HOW: Reads the frame (OBJ_2A68), the row number at $B292 + frame - 1, then the four bytes at
 * $BA2E + 4 * (row - 1). Tables read at bare addresses until the typed data layer lands.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte loaded (0 when cleared), N and Z from it, Y the
 * table offset, C the last shift's carry. X kept. D clear.
 */
FrameLookup SetHitboxFromFrame(uint8_t slot)
{
    FrameLookup result = { true, 0, false, 0, 0, false, false };
    uint8_t frame, row, offset, value, shifted;

    frame = RD8(OBJ->frame[slot]);
    if (frame == 0) {
        result.frame_zero = true;
    } else {
        result.frame_index = (uint8_t)(frame - 1);
        result.overflow = sub_bytes(frame, 1, true).overflow;
        row = rd8((uint16_t)(0xB292 + result.frame_index));
        result.last = row;
        if (row != 0) {
            shifted = (uint8_t)(row - 1);
            result.carry = (shifted & 0x40) != 0;
            offset = (uint8_t)(shifted << 2);
            result.table_offset = offset;
            value = rd8((uint16_t)(0xBA2E + offset));
            WR8(OBJ->hitbox_dx[slot], value);
            value = rd8((uint16_t)(0xBA2F + offset));
            WR8(OBJ->hitbox_dy[slot], value);
            value = rd8((uint16_t)(0xBA30 + offset));
            WR8(OBJ->hitbox_half_w[slot], value);
            value = rd8((uint16_t)(0xBA31 + offset));
            WR8(OBJ->hitbox_half_h[slot], value);
            result.last = value;
            result.none = false;
            return result;
        }
    }
    WR8(OBJ->hitbox_half_w[slot], 0);
    WR8(OBJ->hitbox_half_h[slot], 0);
    return result;
}

void abi_SetHitboxFromFrame(void)
{
    FrameLookup result = SetHitboxFromFrame(cpu.x);

    if (result.none) {
        cpu.a = 0;
        abi_nz(0);
        if (!result.frame_zero) {
            cpu.y = result.frame_index;
            abi_flag(P_C, true);
            abi_flag(P_V, result.overflow);
        }
    } else {
        abi_flag(P_V, result.overflow);
        cpu.a = result.last;
        cpu.y = result.table_offset;
        abi_nz(result.last);
        abi_flag(P_C, result.carry);
    }
}

/*
 * @routine 0CAB:3B9 TurnAngleBack
 * @run 70:B3B9
 * WHAT IT DOES: Turns an object's 16-bit angle (OBJ_3568 high, OBJ_35C0 low) back by $0080;
 * once it passes below zero, sets bit 2 of the stage flag byte zField8E, shows frame 1,
 * switches the hitbox on (2) and moves the object to its next state.
 * HOW: A two-byte subtract, low byte first; the borrow decides.
 * WAITS: no.
 * Entry: X object. Exit: plain: A the new high byte, N Z C V from that subtract; borrowed: A 2,
 * N and Z from the new state, C 0, V from the subtract. X and Y kept. D clear.
 */
TurnBack TurnAngleBack(uint8_t slot)
{
    TurnBack result = { false, 0, false, false, 0 };
    uint8_t low, high, flags;
    Sum sum;

    low = RD8(OBJ->angle_lo[slot]);
    sum = sub_bytes(low, 0x80, true);
    WR8(OBJ->angle_lo[slot], sum.value);
    high = RD8(OBJ->angle_hi[slot]);
    sum = sub_bytes(high, 0, sum.carry);
    WR8(OBJ->angle_hi[slot], sum.value);
    result.high = sum.value;
    result.carry = sum.carry;
    result.overflow = sum.overflow;
    if (sum.carry) {
        return result;
    }
    flags = RD8(ZP->field8_e);
    WR8(ZP->field8_e, (uint8_t)(flags | 0x04));
    WR8(OBJ->frame[slot], 1);
    WR8(OBJ->hitbox_on[slot], 2);
    result.new_state = INC8(OBJ->state[slot]);
    result.borrowed = true;
    return result;
}

void abi_TurnAngleBack(void)
{
    TurnBack result = TurnAngleBack(cpu.x);

    if (result.borrowed) {
        cpu.a = 2;
        abi_nz(result.new_state);
        abi_flag(P_C, false);
        abi_flag(P_V, result.overflow);
    } else {
        cpu.a = result.high;
        abi_nz(result.high);
        abi_flag(P_C, result.carry);
        abi_flag(P_V, result.overflow);
    }
}

/*
 * @routine 09EB:5ED AdvanceOnTimer
 * @run 70:B5ED
 * WHAT IT DOES: Counts an object's life timer down; when it reaches zero, loads OBJ_3510, the
 * angle pair and OBJ_3618 from a four-byte row of a table picked by its next kind, and moves it
 * to its next state.
 * HOW: DEC of the timer in memory; on zero, the row at $B614 + 4 * next kind. Table read at a
 * bare address until the typed data layer lands.
 * WAITS: no.
 * Entry: X object. Exit: timer left: A kept, N and Z from the new timer. Moved on: A the last
 * byte loaded, Y the row offset, N and Z from the new state, C bit 6 of the next kind. X kept.
 * D clear.
 */
TimerAdvance AdvanceOnTimer(uint8_t slot)
{
    TimerAdvance result = { true, 0, 0, 0, false, 0 };
    uint8_t kind, offset, value;

    result.timer = DEC8(OBJ->life_timer[slot]);
    if (result.timer != 0) {
        return result;
    }
    kind = RD8(OBJ->next_kind[slot]);
    result.carry = (kind & 0x40) != 0;
    offset = (uint8_t)(kind << 2);
    value = rd8((uint16_t)(0xB614 + offset));
    WR8(OBJ->f3510[slot], value);
    value = rd8((uint16_t)(0xB615 + offset));
    WR8(OBJ->angle_hi[slot], value);
    value = rd8((uint16_t)(0xB616 + offset));
    WR8(OBJ->angle_lo[slot], value);
    value = rd8((uint16_t)(0xB617 + offset));
    WR8(OBJ->f3618[slot], value);
    result.new_state = INC8(OBJ->state[slot]);
    result.waiting = false;
    result.loaded = value;
    result.row_offset = offset;
    return result;
}

void abi_AdvanceOnTimer(void)
{
    TimerAdvance result = AdvanceOnTimer(cpu.x);

    if (result.waiting) {
        abi_nz(result.timer);
    } else {
        cpu.a = result.loaded;
        cpu.y = result.row_offset;
        abi_nz(result.new_state);
        abi_flag(P_C, result.carry);
    }
}
