/*
 * TEMPORARY: the stage misc object routines of lot L6 (enemy kinds not yet identified), by routine. Bodies return how the routine's last
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
 * @routine 072C:5A7 LoadRecordA06
 * @run 70:BDA7
 * WHAT IT DOES: Loads the object's record.
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: as LoadObjectRecord.
 */
Ended LoadRecordA06(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    jsr_push(0xBDAA);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    return ended_of_record(last, nz, carry);
}

void abi_LoadRecordA06(void)
{
    abi_ended(LoadRecordA06(cpu.x));
}

/*
 * @routine 092E:2D1 ClearHitsTaken
 * @run 6B:8AD1
 * WHAT IT DOES: Sets the object's hits taken (OBJ_33B0) to 0.
 * HOW: A store.
 * WAITS: no.
 * Entry: X object. Exit: all kept.
 */
Ended ClearHitsTaken(uint8_t slot)
{
    WR8(OBJ->hit_taken[slot], 0);
    return ended_kept();
}

void abi_ClearHitsTaken(void)
{
    abi_ended(ClearHitsTaken(cpu.x));
}

/*
 * @routine 12DF:038 RandomTimerC0
 * @run 71:D038
 * WHAT IT DOES: Sets the object's timer OBJ_34B8 to a random number with bits 6 and 7 set.
 * HOW: NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: A the timer, N set, Z clear; C and V from the random number.
 */
Ended RandomTimerC0(uint8_t slot)
{
    RandomResult random;
    uint8_t value;

    jsr_push(0xD03B);
    random = NextRandom();
    jsr_pop();
    value = (uint8_t)(random.value | 0xC0);
    WR8(OBJ->f34b8[slot], value);
    return then(ended_of_random(random), ended_load(value));
}

void abi_RandomTimerC0(void)
{
    abi_ended(RandomTimerC0(cpu.x));
}

/*
 * @routine 0E0B:5DD NextPhaseUnlessKind25
 * @run 70:B5DD
 * WHAT IT DOES: Goes on to the next OBJ_2800 unless the object in slot $50 is of kind $25.
 * HOW: A compare and an INC.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the new OBJ_2800's N and Z.
 */
Ended NextPhaseUnlessKind25(uint8_t slot)
{
    uint8_t kind, phase;
    Ended e;

    kind = RD8(OBJ->kind[0x50]);
    e = ended_compare(kind, 0x25);
    if (kind == 0x25) {
        return e;
    }
    phase = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(phase));
}

void abi_NextPhaseUnlessKind25(void)
{
    abi_ended(NextPhaseUnlessKind25(cpu.x));
}
