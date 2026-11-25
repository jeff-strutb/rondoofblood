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

/*
 * @routine 025C:068 JumpByPhaseTable025C068
 * @run 70:B868
 * WHAT IT DOES: Goes to the code a table at $B87A names for the object's OBJ_2800.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
 */
Jump JumpByPhaseTable025C068(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t phase, index, low, high;

    phase = RD8(OBJ->f2800[slot]);
    index = (uint8_t)(phase << 1);
    low = rd8((uint16_t)(0xB87A + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB87B + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (phase & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByPhaseTable025C068(void)
{
    Jump jump = JumpByPhaseTable025C068(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 025C:40D JumpByPhaseTable025C40D
 * @run 70:BC0D
 * WHAT IT DOES: Goes to the code a table at $BC1F names for the object's OBJ_2800.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
 */
Jump JumpByPhaseTable025C40D(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t phase, index, low, high;

    phase = RD8(OBJ->f2800[slot]);
    index = (uint8_t)(phase << 1);
    low = rd8((uint16_t)(0xBC1F + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xBC20 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (phase & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByPhaseTable025C40D(void)
{
    Jump jump = JumpByPhaseTable025C40D(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 025C:51C JumpByStateTable025C51C
 * @run 70:BD1C
 * WHAT IT DOES: Goes to the code a table at $BD2E names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable025C51C(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xBD2E + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xBD2F + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable025C51C(void)
{
    Jump jump = JumpByStateTable025C51C(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 051B:225 JumpByStateTable051B225
 * @run 70:B225
 * WHAT IT DOES: Goes to the code a table at $B237 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable051B225(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xB237 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB238 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable051B225(void)
{
    Jump jump = JumpByStateTable051B225(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 05CB:212 JumpByPhaseTable05CB212
 * @run 70:B212
 * WHAT IT DOES: Goes to the code a table at $B224 names for the object's OBJ_2800.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
 */
Jump JumpByPhaseTable05CB212(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t phase, index, low, high;

    phase = RD8(OBJ->f2800[slot]);
    index = (uint8_t)(phase << 1);
    low = rd8((uint16_t)(0xB224 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB225 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (phase & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByPhaseTable05CB212(void)
{
    Jump jump = JumpByPhaseTable05CB212(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 05CB:4B8 JumpByPhaseTable05CB4B8
 * @run 70:B4B8
 * WHAT IT DOES: Goes to the code a table at $B4CA names for the object's OBJ_2800.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
 */
Jump JumpByPhaseTable05CB4B8(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t phase, index, low, high;

    phase = RD8(OBJ->f2800[slot]);
    index = (uint8_t)(phase << 1);
    low = rd8((uint16_t)(0xB4CA + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB4CB + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (phase & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByPhaseTable05CB4B8(void)
{
    Jump jump = JumpByPhaseTable05CB4B8(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 05CC:0E9 JumpByStateTable05CC0E9
 * @run 70:B8E9
 * WHAT IT DOES: Goes to the code a table at $B8FB names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable05CC0E9(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xB8FB + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB8FC + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable05CC0E9(void)
{
    Jump jump = JumpByStateTable05CC0E9(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 088A:304 JumpByStateTable088A304
 * @run 70:AB04
 * WHAT IT DOES: Goes to the code a table at $AB16 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable088A304(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xAB16 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xAB17 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable088A304(void)
{
    Jump jump = JumpByStateTable088A304(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0B4A:4DC JumpByStateTable0B4A4DC
 * @run 70:ACDC
 * WHAT IT DOES: Goes to the code a table at $ACEE names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable0B4A4DC(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xACEE + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xACEF + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable0B4A4DC(void)
{
    Jump jump = JumpByStateTable0B4A4DC(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0B4A:573 JumpByStateTable0B4A573
 * @run 70:AD73
 * WHAT IT DOES: Goes to the code a table at $AD85 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable0B4A573(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xAD85 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xAD86 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable0B4A573(void)
{
    Jump jump = JumpByStateTable0B4A573(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0B4C:0D7 JumpByPhaseTable0B4C0D7
 * @run 70:B8D7
 * WHAT IT DOES: Goes to the code a table at $B8E9 names for the object's OBJ_2800.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
 */
Jump JumpByPhaseTable0B4C0D7(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t phase, index, low, high;

    phase = RD8(OBJ->f2800[slot]);
    index = (uint8_t)(phase << 1);
    low = rd8((uint16_t)(0xB8E9 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB8EA + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (phase & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByPhaseTable0B4C0D7(void)
{
    Jump jump = JumpByPhaseTable0B4C0D7(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0B4C:42F JumpByPhaseTable0B4C42F
 * @run 70:BC2F
 * WHAT IT DOES: Goes to the code a table at $BC41 names for the object's OBJ_2800.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
 */
Jump JumpByPhaseTable0B4C42F(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t phase, index, low, high;

    phase = RD8(OBJ->f2800[slot]);
    index = (uint8_t)(phase << 1);
    low = rd8((uint16_t)(0xBC41 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xBC42 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (phase & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByPhaseTable0B4C42F(void)
{
    Jump jump = JumpByPhaseTable0B4C42F(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0B4D:08F JumpByStateTable0B4D08F
 * @run 70:C08F
 * WHAT IT DOES: Goes to the code a table at $C0A1 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable0B4D08F(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xC0A1 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xC0A2 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable0B4D08F(void)
{
    Jump jump = JumpByStateTable0B4D08F(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 088B:06C JumpByPhaseTable088B06C
 * @run 70:B06C
 * WHAT IT DOES: Goes to the code a table at $B07E names for the object's OBJ_2800.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
 */
Jump JumpByPhaseTable088B06C(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t phase, index, low, high;

    phase = RD8(OBJ->f2800[slot]);
    index = (uint8_t)(phase << 1);
    low = rd8((uint16_t)(0xB07E + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB07F + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (phase & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByPhaseTable088B06C(void)
{
    Jump jump = JumpByPhaseTable088B06C(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 025B:545 JumpByStateTable025B545
 * @run 70:B545
 * WHAT IT DOES: Goes to the code a table at $B557 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable025B545(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xB557 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB558 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable025B545(void)
{
    Jump jump = JumpByStateTable025B545(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0B4B:64C JumpByStateTable0B4B64C
 * @run 70:B64C
 * WHAT IT DOES: Goes to the code a table at $B65E names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable0B4B64C(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xB65E + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB65F + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable0B4B64C(void)
{
    Jump jump = JumpByStateTable0B4B64C(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0F6B:3EC JumpByStateTable0F6B3EC
 * @run 70:B3EC
 * WHAT IT DOES: Goes to the code a table at $B3FE names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable0F6B3EC(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xB3FE + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB3FF + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTable0F6B3EC(void)
{
    Jump jump = JumpByStateTable0F6B3EC(cpu.x);

    jump_shim(jump.end, jump.target);
}
