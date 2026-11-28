/*
 * Object dispatchers of stages $09 to $10 and the secret stages that count a column and call checks before going to
 * the handler a table names.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "checked_jump_b2.h"

/*
 * A dispatcher with checks: when step is not 0 that object column is counted on first; then the routines the loaded
 * overlay holds at each check's address, in turn; then the handler the table names for the object column `by`.
 */
typedef struct {
    uint16_t step, by, table;
    uint8_t checks;
    struct { uint16_t run, ret; } check[4];
} CheckedJump;

static Jump checked_jump(uint8_t slot, const CheckedJump *c)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t value, index, low, high;
    unsigned i;

    if (c->step != 0) {
        value = (uint8_t)(RD8_IDX(c->step, slot) + 1);
        WR8_IDX(c->step, slot, value);
        abi_nz(value);
    }
    for (i = 0; i < c->checks; i++) {
        call_routine(c->check[i].ret, c->check[i].run);
        slot = cpu.x;
    }
    value = RD8_IDX(c->by, slot);
    index = (uint8_t)(value << 1);
    low = rd8((uint16_t)(c->table + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(c->table + 1 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (value & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

/*
 * @routine 13FC:1C2 CheckedJump13FC1C2
 * @run 70:B9C2
 * WHAT IT DOES: An object's dispatcher (Secret stage 0C): WORK_K is counted on, the loaded overlay's routines at $B9EA, $BA18, $BA2F are run,
 * then the handler of its state from the table at $B9E0.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump13FC1C2(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_k), COL(OBJ->state), 0xB9E0, 3, { { 0xB9EA, 0xB9C8 }, { 0xBA18, 0xB9CB }, { 0xBA2F, 0xB9CE } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump13FC1C2(void)
{
    Jump jump = CheckedJump13FC1C2(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 13FD:11C CheckedJump13FD11C
 * @run 71:C11C
 * WHAT IT DOES: An object's dispatcher (Secret stage 0C): WORK_K is counted on, the loaded overlay's routines at $B9EA, $BA18, $BA2F are run,
 * then the handler of its state from the table at $C13A.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump13FD11C(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_k), COL(OBJ->state), 0xC13A, 3, { { 0xB9EA, 0xC122 }, { 0xBA18, 0xC125 }, { 0xBA2F, 0xC128 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump13FD11C(void)
{
    Jump jump = CheckedJump13FD11C(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161D:5A4 CheckedJump161D5A4
 * @run 71:C5A4
 * WHAT IT DOES: An object's dispatcher (Stage 0E): WORK_F is counted on, the loaded overlay's routines at $C5CE, $C5F8, $C60A are run,
 * then the handler of its state from the table at $C5C2.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump161D5A4(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_f), COL(OBJ->state), 0xC5C2, 3, { { 0xC5CE, 0xC5AA }, { 0xC5F8, 0xC5AD }, { 0xC60A, 0xC5B0 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump161D5A4(void)
{
    Jump jump = CheckedJump161D5A4(cpu.x);

    jump_shim(jump.end, jump.target);
}
