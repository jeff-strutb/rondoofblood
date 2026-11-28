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
#include "checked_jump_b.h"

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
 * @routine 072C:58B CheckedJump072C58B
 * @run 70:BD8B
 * WHAT IT DOES: An object's dispatcher (Secret stage 03): the loaded overlay's routines at $B3A4, $B3DB are run,
 * then the handler of its state from the table at $BDA3.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump072C58B(uint8_t slot)
{
    CheckedJump jumps = { 0, COL(OBJ->state), 0xBDA3, 2, { { 0xB3A4, 0xBD8E }, { 0xB3DB, 0xBD91 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump072C58B(void)
{
    Jump jump = CheckedJump072C58B(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0E0B:040 CheckedJump0E0B040
 * @run 70:B040
 * WHAT IT DOES: An object's dispatcher (Secret stage 08): WORK_F is counted on, the loaded overlay's routines at $B071, $B099, $B0AB, $B0C2 are run,
 * then the handler of its state from the table at $B061.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump0E0B040(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_f), COL(OBJ->state), 0xB061, 4, { { 0xB071, 0xB046 }, { 0xB099, 0xB049 }, { 0xB0AB, 0xB04C }, { 0xB0C2, 0xB04F } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump0E0B040(void)
{
    Jump jump = CheckedJump0E0B040(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0E0C:4A8 CheckedJump0E0C4A8
 * @run 70:BCA8
 * WHAT IT DOES: An object's dispatcher (Secret stage 08): WORK_D is counted on, no check is run,
 * then the handler of its state from the table at $BCBD.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump0E0C4A8(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_d), COL(OBJ->state), 0xBCBD, 0, { { 0, 0 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump0E0C4A8(void)
{
    Jump jump = CheckedJump0E0C4A8(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBD:428 CheckedJump0EBD428
 * @run 71:C428
 * WHAT IT DOES: An object's dispatcher (Stage 09): WORK_E is counted on, the loaded overlay's routines at $C44D, $C45F are run,
 * then the handler of its state from the table at $C443.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump0EBD428(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_e), COL(OBJ->state), 0xC443, 2, { { 0xC44D, 0xC42E }, { 0xC45F, 0xC431 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump0EBD428(void)
{
    Jump jump = CheckedJump0EBD428(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBD:599 CheckedJump0EBD599
 * @run 71:C599
 * WHAT IT DOES: An object's dispatcher (Stage 09): WORK_E is counted on, the loaded overlay's routines at $C45F are run,
 * then the handler of its state from the table at $C5B1.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump0EBD599(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_e), COL(OBJ->state), 0xC5B1, 1, { { 0xC45F, 0xC59F } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump0EBD599(void)
{
    Jump jump = CheckedJump0EBD599(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBF:0C5 CheckedJump0EBF0C5
 * @run 71:D0C5
 * WHAT IT DOES: An object's dispatcher (Stage 09): WORK_H is counted on, the loaded overlay's routines at $D0EB are run,
 * then the handler of its state from the table at $D0DD.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump0EBF0C5(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_h), COL(OBJ->state), 0xD0DD, 1, { { 0xD0EB, 0xD0CB } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump0EBF0C5(void)
{
    Jump jump = CheckedJump0EBF0C5(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBF:592 CheckedJump0EBF592
 * @run 71:D592
 * WHAT IT DOES: An object's dispatcher (Stage 09): the loaded overlay's routines at $D5BE, $D5E1 are run,
 * then the handler of its state from the table at $D5AA.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump0EBF592(uint8_t slot)
{
    CheckedJump jumps = { 0, COL(OBJ->state), 0xD5AA, 2, { { 0xD5BE, 0xD595 }, { 0xD5E1, 0xD598 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump0EBF592(void)
{
    Jump jump = CheckedJump0EBF592(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101B:066 CheckedJump101B066
 * @run 70:B066
 * WHAT IT DOES: An object's dispatcher (Stage 0A): WORK_G is counted on, the loaded overlay's routines at $B088 are run,
 * then the handler of its state from the table at $B07E.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump101B066(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_g), COL(OBJ->state), 0xB07E, 1, { { 0xB088, 0xB06C } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump101B066(void)
{
    Jump jump = CheckedJump101B066(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101B:43C CheckedJump101B43C
 * @run 70:B43C
 * WHAT IT DOES: An object's dispatcher (Stage 0A): WORK_D is counted on, the loaded overlay's routines at $B46C are run,
 * then the handler of its state from the table at $B454.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump101B43C(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_d), COL(OBJ->state), 0xB454, 1, { { 0xB46C, 0xB442 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump101B43C(void)
{
    Jump jump = CheckedJump101B43C(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101C:289 CheckedJump101C289
 * @run 70:BA89
 * WHAT IT DOES: An object's dispatcher (Stage 0A): WORK_D is counted on, the loaded overlay's routines at $BAAE, $BAC0 are run,
 * then the handler of its state from the table at $BAA4.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump101C289(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_d), COL(OBJ->state), 0xBAA4, 2, { { 0xBAAE, 0xBA8F }, { 0xBAC0, 0xBA92 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump101C289(void)
{
    Jump jump = CheckedJump101C289(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101C:64B CheckedJump101C64B
 * @run 70:BE4B
 * WHAT IT DOES: An object's dispatcher (Stage 0A): WORK_D is counted on, the loaded overlay's routines at $BAAE, $BAC0 are run,
 * then the handler of its state from the table at $BE66.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump101C64B(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_d), COL(OBJ->state), 0xBE66, 2, { { 0xBAAE, 0xBE51 }, { 0xBAC0, 0xBE54 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump101C64B(void)
{
    Jump jump = CheckedJump101C64B(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101C:76E CheckedJump101C76E
 * @run 70:BF6E
 * WHAT IT DOES: An object's dispatcher (Stage 0A): the loaded overlay's routines at $BF90, $BFE0 are run,
 * then the handler of its state from the table at $BF86.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump101C76E(uint8_t slot)
{
    CheckedJump jumps = { 0, COL(OBJ->state), 0xBF86, 2, { { 0xBF90, 0xBF71 }, { 0xBFE0, 0xBF74 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump101C76E(void)
{
    Jump jump = CheckedJump101C76E(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101D:3EB CheckedJump101D3EB
 * @run 71:C3EB
 * WHAT IT DOES: An object's dispatcher (Stage 0A): the loaded overlay's routines at $C40F, $C438 are run,
 * then the handler of its state from the table at $C403.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump101D3EB(uint8_t slot)
{
    CheckedJump jumps = { 0, COL(OBJ->state), 0xC403, 2, { { 0xC40F, 0xC3EE }, { 0xC438, 0xC3F1 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump101D3EB(void)
{
    Jump jump = CheckedJump101D3EB(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101E:671 CheckedJump101E671
 * @run 71:CE71
 * WHAT IT DOES: An object's dispatcher (Stage 0A): WORK_E is counted on, the loaded overlay's routines at $CE8F are run,
 * then the handler of its state from the table at $CE89.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump101E671(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_e), COL(OBJ->state), 0xCE89, 1, { { 0xCE8F, 0xCE77 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump101E671(void)
{
    Jump jump = CheckedJump101E671(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117B:515 CheckedJump117B515
 * @run 70:B515
 * WHAT IT DOES: An object's dispatcher (Stage 0B): WORK_H is counted on, the loaded overlay's routines at $B53B are run,
 * then the handler of its state from the table at $B52D.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump117B515(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_h), COL(OBJ->state), 0xB52D, 1, { { 0xB53B, 0xB51B } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump117B515(void)
{
    Jump jump = CheckedJump117B515(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117C:19B CheckedJump117C19B
 * @run 70:B99B
 * WHAT IT DOES: An object's dispatcher (Stage 0B): the loaded overlay's routines at $B9C7, $B9EA are run,
 * then the handler of its state from the table at $B9B3.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump117C19B(uint8_t slot)
{
    CheckedJump jumps = { 0, COL(OBJ->state), 0xB9B3, 2, { { 0xB9C7, 0xB99E }, { 0xB9EA, 0xB9A1 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump117C19B(void)
{
    Jump jump = CheckedJump117C19B(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117C:653 CheckedJump117C653
 * @run 70:BE53
 * WHAT IT DOES: An object's dispatcher (Stage 0B): WORK_H is counted on, the loaded overlay's routines at $BE84, $BE9D, $BEB9, $BECB are run,
 * then the handler of its state from the table at $BE74.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump117C653(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_h), COL(OBJ->state), 0xBE74, 4, { { 0xBE84, 0xBE59 }, { 0xBE9D, 0xBE5C }, { 0xBEB9, 0xBE5F }, { 0xBECB, 0xBE62 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump117C653(void)
{
    Jump jump = CheckedJump117C653(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DB:418 CheckedJump12DB418
 * @run 70:B418
 * WHAT IT DOES: An object's dispatcher (Stage 0C): WORK_F is counted on, the loaded overlay's routines at $B43A are run,
 * then the handler of its state from the table at $B430.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump12DB418(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_f), COL(OBJ->state), 0xB430, 1, { { 0xB43A, 0xB41E } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump12DB418(void)
{
    Jump jump = CheckedJump12DB418(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DC:0C8 CheckedJump12DC0C8
 * @run 70:B8C8
 * WHAT IT DOES: An object's dispatcher (Stage 0C): WORK_D is counted on, no check is run,
 * then the handler of its state from the table at $B8DD.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump12DC0C8(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_d), COL(OBJ->state), 0xB8DD, 0, { { 0, 0 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump12DC0C8(void)
{
    Jump jump = CheckedJump12DC0C8(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DD:358 CheckedJump12DD358
 * @run 71:C358
 * WHAT IT DOES: An object's dispatcher (Stage 0C): WORK_D is counted on, the loaded overlay's routines at $C37F, $C3AF are run,
 * then the handler of its state from the table at $C373.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump12DD358(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_d), COL(OBJ->state), 0xC373, 2, { { 0xC37F, 0xC35E }, { 0xC3AF, 0xC361 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump12DD358(void)
{
    Jump jump = CheckedJump12DD358(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DE:24C CheckedJump12DE24C
 * @run 71:CA4C
 * WHAT IT DOES: An object's dispatcher (Stage 0C): WORK_D is counted on, the loaded overlay's routines at $CA73, $CA85 are run,
 * then the handler of its state from the table at $CA67.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump12DE24C(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_d), COL(OBJ->state), 0xCA67, 2, { { 0xCA73, 0xCA52 }, { 0xCA85, 0xCA55 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump12DE24C(void)
{
    Jump jump = CheckedJump12DE24C(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161B:2E2 CheckedJump161B2E2
 * @run 70:B2E2
 * WHAT IT DOES: An object's dispatcher (Stage 0E): no check is run,
 * then the handler of its state from the table at $B2F4.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump161B2E2(uint8_t slot)
{
    CheckedJump jumps = { 0, COL(OBJ->state), 0xB2F4, 0, { { 0, 0 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump161B2E2(void)
{
    Jump jump = CheckedJump161B2E2(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161B:31F CheckedJump161B31F
 * @run 70:B31F
 * WHAT IT DOES: An object's dispatcher (Stage 0E): WORK_E is counted on, the loaded overlay's routines at $B341 are run,
 * then the handler of its state from the table at $B337.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump161B31F(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_e), COL(OBJ->state), 0xB337, 1, { { 0xB341, 0xB325 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump161B31F(void)
{
    Jump jump = CheckedJump161B31F(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161B:605 CheckedJump161B605
 * @run 70:B605
 * WHAT IT DOES: An object's dispatcher (Stage 0E): WORK_I is counted on, the loaded overlay's routines at $B625 are run,
 * then the handler of its state from the table at $B61D.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump161B605(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_i), COL(OBJ->state), 0xB61D, 1, { { 0xB625, 0xB60B } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump161B605(void)
{
    Jump jump = CheckedJump161B605(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161C:2C4 CheckedJump161C2C4
 * @run 70:BAC4
 * WHAT IT DOES: An object's dispatcher (Stage 0E): WORK_H is counted on, the loaded overlay's routines at $BAEF, $BB1A are run,
 * then the handler of its state from the table at $BADF.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump161C2C4(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_h), COL(OBJ->state), 0xBADF, 2, { { 0xBAEF, 0xBACA }, { 0xBB1A, 0xBACD } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump161C2C4(void)
{
    Jump jump = CheckedJump161C2C4(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 18DC:011 CheckedJump18DC011
 * @run 70:B811
 * WHAT IT DOES: An object's dispatcher (Stage 10): the loaded overlay's routines at $B6FE, $B71B are run,
 * then the handler of its substate from the table at $B829.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump18DC011(uint8_t slot)
{
    CheckedJump jumps = { 0, COL(OBJ->substate), 0xB829, 2, { { 0xB6FE, 0xB814 }, { 0xB71B, 0xB817 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump18DC011(void)
{
    Jump jump = CheckedJump18DC011(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 18DC:2D0 CheckedJump18DC2D0
 * @run 70:BAD0
 * WHAT IT DOES: An object's dispatcher (Stage 10): WORK_D is counted on, no check is run,
 * then the handler of its state from the table at $BAE5.
 * HOW: JSRs to the routines loaded at those addresses, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckedJump18DC2D0(uint8_t slot)
{
    CheckedJump jumps = { COL(OBJ->work_d), COL(OBJ->state), 0xBAE5, 0, { { 0, 0 } } };

    return checked_jump(slot, &jumps);
}

void abi_CheckedJump18DC2D0(void)
{
    Jump jump = CheckedJump18DC2D0(cpu.x);

    jump_shim(jump.end, jump.target);
}
