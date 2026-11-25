/*
 * TEMPORARY: the stage 04D1 object routines of lot L6 (enemy kinds not yet identified), by routine. Bodies return how the routine's last
 * instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/*
 * @routine 051B:23D LoadRecordA04
 * @run 70:B23D
 * WHAT IT DOES: Loads the object's record.
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: as LoadObjectRecord.
 */
Ended LoadRecordA04(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    jsr_push(0xB240);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    return ended_of_record(last, nz, carry);
}

void abi_LoadRecordA04(void)
{
    abi_ended(LoadRecordA04(cpu.x));
}

/*
 * @routine 051B:76A LoadRecordB04
 * @run 70:B76A
 * WHAT IT DOES: Loads the object's record.
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: as LoadObjectRecord.
 */
Ended LoadRecordB04(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    jsr_push(0xB76D);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    return ended_of_record(last, nz, carry);
}

void abi_LoadRecordB04(void)
{
    abi_ended(LoadRecordB04(cpu.x));
}
