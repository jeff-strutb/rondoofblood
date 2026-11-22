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

    state = RD8(OBJ->f27a8[slot]);
    result.above = state > 4;
    if (state == 4) {
        return result;
    }
    flags = RD8(OBJ->f31a0[slot]);
    if ((flags & 0x80) == 0) {
        result.end = STATE4_NO_FLAG;
        return result;
    }
    WR8(OBJ->f3408[slot], 0);
    WR8(OBJ->f3460[slot], 0);
    WR8(OBJ->f27a8[slot], 4);
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
 * @routine 0CAB:260 SetHitOffsetsFromFrame
 * @run 70:B260
 * WHAT IT DOES: Loads an object's four hit offsets (OBJ_3930, OBJ_39E0, OBJ_3988, OBJ_3A38)
 * from a four-byte row of a table, picked by the object's animation frame through a second table
 * of row numbers; no frame, or a row number 0, clears the last two.
 * HOW: Reads the frame (OBJ_2A68), the row number at $B292 + frame - 1, then the four bytes at
 * $BA2E + 4 * (row - 1). Tables read at bare addresses until the typed data layer lands.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte loaded (0 when cleared), N and Z from it, Y the
 * table offset, C the last shift's carry. X kept. D clear.
 */
FrameLookup SetHitOffsetsFromFrame(uint8_t slot)
{
    FrameLookup result = { true, 0, false, 0, 0, false, false };
    uint8_t frame, row, offset, value, shifted;

    frame = RD8(OBJ->f2a68[slot]);
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
            WR8(OBJ->f3930[slot], value);
            value = rd8((uint16_t)(0xBA2F + offset));
            WR8(OBJ->f39e0[slot], value);
            value = rd8((uint16_t)(0xBA30 + offset));
            WR8(OBJ->f3988[slot], value);
            value = rd8((uint16_t)(0xBA31 + offset));
            WR8(OBJ->f3a38[slot], value);
            result.last = value;
            result.none = false;
            return result;
        }
    }
    WR8(OBJ->f3988[slot], 0);
    WR8(OBJ->f3a38[slot], 0);
    return result;
}

void abi_SetHitOffsetsFromFrame(void)
{
    FrameLookup result = SetHitOffsetsFromFrame(cpu.x);

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
