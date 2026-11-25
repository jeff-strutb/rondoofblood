/*
 * TEMPORARY: the state and phase jump tables of lot L6 (the object code of enemy kinds not yet identified), by routine. Each reads the object's state (or
 * OBJ_2800), doubles it, takes the code address from a table in the overlay and jumps there.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

/*
 * @routine 072C:004 JumpByPhaseTable072C004
 * @run 70:B804
 * WHAT IT DOES: Goes to the code a table at $B816 names for the object's OBJ_2800.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
 */
Jump JumpByPhaseTable072C004(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t phase, index, low, high;

    phase = RD8(OBJ->f2800[slot]);
    index = (uint8_t)(phase << 1);
    low = rd8((uint16_t)(0xB816 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB817 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (phase & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByPhaseTable072C004(void)
{
    Jump jump = JumpByPhaseTable072C004(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 072C:763 JumpByStateTable072C763
 * @run 70:BF63
 * WHAT IT DOES: Goes to the code a table at $BF75 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable072C763(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xBF75 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xBF76 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable072C763(void)
{
    Jump jump = JumpByStateTable072C763(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 093C:3FF JumpByStateTable093C3FF
 * @run 70:BBFF
 * WHAT IT DOES: Goes to the code a table at $BC11 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable093C3FF(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xBC11 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xBC12 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable093C3FF(void)
{
    Jump jump = JumpByStateTable093C3FF(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 093C:4E5 JumpByStateTable093C4E5
 * @run 70:BCE5
 * WHAT IT DOES: Goes to the code a table at $BCF7 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable093C4E5(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xBCF7 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xBCF8 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable093C4E5(void)
{
    Jump jump = JumpByStateTable093C4E5(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 09EB:7D3 JumpByStateTable09EB7D3
 * @run 70:B7D3
 * WHAT IT DOES: Goes to the code a table at $B7E5 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable09EB7D3(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xB7E5 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB7E6 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable09EB7D3(void)
{
    Jump jump = JumpByStateTable09EB7D3(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 09EC:18D JumpByStateTable09EC18D
 * @run 70:B98D
 * WHAT IT DOES: Goes to the code a table at $B99F names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable09EC18D(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xB99F + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB9A0 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable09EC18D(void)
{
    Jump jump = JumpByStateTable09EC18D(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0BFC:12B JumpByStateTable0BFC12B
 * @run 70:B92B
 * WHAT IT DOES: Goes to the code a table at $B93D names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable0BFC12B(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xB93D + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB93E + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable0BFC12B(void)
{
    Jump jump = JumpByStateTable0BFC12B(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0CAB:12F JumpByStateTable0CAB12F
 * @run 70:B12F
 * WHAT IT DOES: Goes to the code a table at $B141 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable0CAB12F(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xB141 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB142 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable0CAB12F(void)
{
    Jump jump = JumpByStateTable0CAB12F(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0CAB:4D7 JumpByPhaseTable0CAB4D7
 * @run 70:B4D7
 * WHAT IT DOES: Goes to the code a table at $B4E9 names for the object's OBJ_2800.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
 */
Jump JumpByPhaseTable0CAB4D7(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t phase, index, low, high;

    phase = RD8(OBJ->f2800[slot]);
    index = (uint8_t)(phase << 1);
    low = rd8((uint16_t)(0xB4E9 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB4EA + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (phase & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByPhaseTable0CAB4D7(void)
{
    Jump jump = JumpByPhaseTable0CAB4D7(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0CAC:47A JumpByStateTable0CAC47A
 * @run 70:BC7A
 * WHAT IT DOES: Goes to the code a table at $BC8C names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable0CAC47A(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xBC8C + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xBC8D + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable0CAC47A(void)
{
    Jump jump = JumpByStateTable0CAC47A(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0CAC:59D JumpByStateTable0CAC59D
 * @run 70:BD9D
 * WHAT IT DOES: Goes to the code a table at $BDAF names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable0CAC59D(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xBDAF + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xBDB0 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable0CAC59D(void)
{
    Jump jump = JumpByStateTable0CAC59D(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0CAC:6C3 JumpByStateTable0CAC6C3
 * @run 70:BEC3
 * WHAT IT DOES: Goes to the code a table at $BED5 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable0CAC6C3(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xBED5 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xBED6 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable0CAC6C3(void)
{
    Jump jump = JumpByStateTable0CAC6C3(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 051A:190 JumpByStateTable051A190
 * @run 70:A990
 * WHAT IT DOES: Goes to the code a table at $A9A2 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable051A190(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xA9A2 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xA9A3 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable051A190(void)
{
    Jump jump = JumpByStateTable051A190(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 072B:1B1 JumpByStateTable072B1B1
 * @run 70:B1B1
 * WHAT IT DOES: Goes to the code a table at $B1C3 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable072B1B1(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xB1C3 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB1C4 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable072B1B1(void)
{
    Jump jump = JumpByStateTable072B1B1(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 072B:24C JumpByStateTable072B24C
 * @run 70:B24C
 * WHAT IT DOES: Goes to the code a table at $B25E names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable072B24C(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xB25E + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB25F + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable072B24C(void)
{
    Jump jump = JumpByStateTable072B24C(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 051B:350 JumpByPhaseTable051B350
 * @run 70:B350
 * WHAT IT DOES: Goes to the code a table at $B362 names for the object's OBJ_2800.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
 */
Jump JumpByPhaseTable051B350(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t phase, index, low, high;

    phase = RD8(OBJ->f2800[slot]);
    index = (uint8_t)(phase << 1);
    low = rd8((uint16_t)(0xB362 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB363 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (phase & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByPhaseTable051B350(void)
{
    Jump jump = JumpByPhaseTable051B350(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 051B:5B5 JumpByPhaseTable051B5B5
 * @run 70:B5B5
 * WHAT IT DOES: Goes to the code a table at $B5C7 names for the object's OBJ_2800.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
 */
Jump JumpByPhaseTable051B5B5(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t phase, index, low, high;

    phase = RD8(OBJ->f2800[slot]);
    index = (uint8_t)(phase << 1);
    low = rd8((uint16_t)(0xB5C7 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB5C8 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (phase & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByPhaseTable051B5B5(void)
{
    Jump jump = JumpByPhaseTable051B5B5(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 088A:4E4 JumpByStateTable088A4E4
 * @run 70:ACE4
 * WHAT IT DOES: Goes to the code a table at $ACF6 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable088A4E4(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xACF6 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xACF7 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable088A4E4(void)
{
    Jump jump = JumpByStateTable088A4E4(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0E0B:16C JumpByPhaseTable0E0B16C
 * @run 70:B16C
 * WHAT IT DOES: Goes to the code a table at $B17E names for the object's OBJ_2800.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
 */
Jump JumpByPhaseTable0E0B16C(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t phase, index, low, high;

    phase = RD8(OBJ->f2800[slot]);
    index = (uint8_t)(phase << 1);
    low = rd8((uint16_t)(0xB17E + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB17F + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (phase & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByPhaseTable0E0B16C(void)
{
    Jump jump = JumpByPhaseTable0E0B16C(cpu.x);

    jump_shim(jump.end, jump.target);
}
