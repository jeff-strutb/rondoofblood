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
 * list restarted, once bit 7 of OBJ_HIT_STATUS is set.
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
    flags = RD8(OBJ->hit_status[slot]);
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
 * WHAT IT DOES: Loads an object's hitbox (dx, dy, half width, half height: OBJ_HITBOX_DX, OBJ_HITBOX_DY, OBJ_HITBOX_HALF_W, OBJ_HITBOX_HALF_H)
 * from a four-byte row of a table, picked by the object's animation frame through a second table
 * of row numbers; no frame, or a row number 0, clears the last two.
 * HOW: Reads the frame (OBJ_FRAME), the row number at $B292 + frame - 1, then the four bytes at
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
    kind = RD8(OBJ->start_arg[slot]);
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

/*
 * @routine 09EB:578 LaunchOnTimer
 * @run 70:B578
 * WHAT IT DOES: Counts an object's life timer down; when it runs out the object shows frame
 * next kind + $12, takes its two high speed bytes (X and Y) from a two-byte table row of its
 * next kind, and moves to its next state.
 * HOW: DEC of the timer in memory; on zero, the add, the row at $B59B + 2 * next kind. Table
 * read at a bare address until the typed data layer lands.
 * WAITS: no.
 * Entry: X object. Exit: timer left: A kept, N and Z from the new timer. Moved on: A the second
 * table byte, Y the row offset, N and Z from the new state, C bit 7 of the next kind, V from
 * the add of $12. X kept. D clear.
 */
Launch LaunchOnTimer(uint8_t slot)
{
    Launch result = { true, 0, 0, 0, false, false, 0 };
    uint8_t kind, offset, value;
    Sum sum;

    result.timer = DEC8(OBJ->life_timer[slot]);
    if (result.timer != 0) {
        return result;
    }
    kind = RD8(OBJ->start_arg[slot]);
    sum = add_bytes(kind, 0x12, false);
    WR8(OBJ->frame[slot], sum.value);
    kind = RD8(OBJ->start_arg[slot]);
    result.carry = (kind & 0x80) != 0;
    offset = (uint8_t)(kind << 1);
    value = rd8((uint16_t)(0xB59B + offset));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xB59C + offset));
    WR8(OBJ->vy_hi[slot], value);
    result.new_state = INC8(OBJ->state[slot]);
    result.waiting = false;
    result.loaded = value;
    result.row_offset = offset;
    result.overflow = sum.overflow;
    return result;
}

void abi_LaunchOnTimer(void)
{
    Launch result = LaunchOnTimer(cpu.x);

    if (result.waiting) {
        abi_nz(result.timer);
    } else {
        cpu.a = result.loaded;
        cpu.y = result.row_offset;
        abi_nz(result.new_state);
        abi_flag(P_C, result.carry);
        abi_flag(P_V, result.overflow);
    }
}

/*
 * @routine 09EA:60E SetHitboxFromFrameFive
 * @run 70:AE0E
 * WHAT IT DOES: Loads an object's hitbox (OBJ_HITBOX_DX, OBJ_HITBOX_DY, OBJ_HITBOX_HALF_W, OBJ_HITBOX_HALF_H) from the
 * four-byte row at $BC13 picked by its frame less 5.
 * HOW: Four loads from the row. Table read at a bare address until the typed data layer lands.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte, N and Z from it, Y the row offset, C the last shift's
 * carry, V from the frame subtract. X kept. D clear.
 */
HitboxRow SetHitboxFromFrameFive(uint8_t slot)
{
    HitboxRow result;
    uint8_t frame, row, value;
    Sum sum;

    frame = RD8(OBJ->frame[slot]);
    sum = sub_bytes(frame, 5, true);
    row = sum.value;
    result.carry = (row & 0x40) != 0;
    result.overflow = sum.overflow;
    result.offset = (uint8_t)(row << 2);
    value = rd8((uint16_t)(0xBC13 + result.offset));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(0xBC14 + result.offset));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(0xBC15 + result.offset));
    WR8(OBJ->hitbox_half_w[slot], value);
    value = rd8((uint16_t)(0xBC16 + result.offset));
    WR8(OBJ->hitbox_half_h[slot], value);
    result.last = value;
    return result;
}

void abi_SetHitboxFromFrameFive(void)
{
    HitboxRow result = SetHitboxFromFrameFive(cpu.x);

    cpu.a = result.last;
    cpu.y = result.offset;
    abi_nz(result.last);
    abi_flag(P_C, result.carry);
    abi_flag(P_V, result.overflow);
}

/*
 * @routine 0CAB:240 EnterStateFiveOnPlayerState9
 * @run 70:B240
 * WHAT IT DOES: When the player (slot 0) is in state 9 and the object is in state 3, turns the
 * object to face right, restarts its frame list, clears OBJ_SUBSTATE and moves it to state 5.
 * HOW: Two compares, then the stores. Random cases never reach the stores: the curated cases
 * of tools/rondo/curated/enemy_state_five.txt do (liftcmp --curated).
 * WAITS: no.
 * Entry: X object. Exit: A and flags as the last compare or load left them (see the wrapper).
 * X and Y kept. D clear.
 */
FiveResult EnterStateFiveOnPlayerState9(uint8_t slot)
{
    FiveResult result = { FIVE_PLAYER_NOT_9, 0 };
    uint8_t state;

    state = RD8(OBJ->state[0]);
    result.value = state;
    if (state != 9) {
        return result;
    }
    state = RD8(OBJ->state[slot]);
    result.value = state;
    if (state != 3) {
        result.end = FIVE_NOT_STATE_3;
        return result;
    }
    WR8(OBJ->facing_left[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 5);
    WR8(OBJ->substate[slot], 0);
    result.end = FIVE_ENTERED;
    return result;
}

void abi_EnterStateFiveOnPlayerState9(void)
{
    FiveResult result = EnterStateFiveOnPlayerState9(cpu.x);

    switch (result.end) {
    case FIVE_PLAYER_NOT_9:
        cpu.a = result.value;
        abi_nz((uint8_t)(result.value - 9));
        abi_flag(P_C, result.value >= 9);
        break;
    case FIVE_NOT_STATE_3:
        cpu.a = result.value;
        abi_nz((uint8_t)(result.value - 3));
        abi_flag(P_C, result.value >= 3);
        break;
    case FIVE_ENTERED:
        cpu.a = 5;
        abi_nz(5);
        abi_flag(P_C, true);
        break;
    }
}

/*
 * @routine 072C:02C StopAndPickPhase
 * @run 70:B82C
 * WHAT IT DOES: Stops an object (clears OBJ_3670 and its four speed bytes), then sets OBJ_SUBSTATE
 * from a table of eight bytes at $B84B picked by the low three bits of a counter, OBJ_3720,
 * which it then advances.
 * HOW: Five clears, a table read, an INC in memory. Table read at a bare address until the typed
 * data layer lands.
 * WAITS: no.
 * Entry: X object. Exit: A the table byte, Y its index, N and Z from the counter's new value.
 * X kept. D clear.
 */
PhasePick StopAndPickPhase(uint8_t slot)
{
    PhasePick result;
    uint8_t counter, value;

    WR8(OBJ->f3670[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    counter = RD8(OBJ->f3720[slot]);
    result.index = (uint8_t)(counter & 0x07);
    value = rd8((uint16_t)(0xB84B + result.index));
    WR8(OBJ->substate[slot], value);
    result.new_phase = INC8(OBJ->f3720[slot]);
    result.loaded = value;
    return result;
}

void abi_StopAndPickPhase(void)
{
    PhasePick result = StopAndPickPhase(cpu.x);

    cpu.a = result.loaded;
    cpu.y = result.index;
    abi_nz(result.new_phase);
}
