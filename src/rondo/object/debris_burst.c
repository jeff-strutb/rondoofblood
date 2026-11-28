/*
 * The common burst of debris the stage overlays start when something breaks: several kind $9A objects
 * at one object's place, each remembering its number. The same bytes lie in Richter's overlay and in
 * the stages' at several run addresses; each copy's JSRs push its own return addresses, so the body
 * takes them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

/* The arms StartDebrisBurst ends on. */
enum { DBU_NO_SLOT, DBU_STARTED };

/* The body at any of its places: the returns of its two JSRs. */
static ObjArm debris_burst(uint8_t slot, uint8_t timer, uint8_t y_entry, uint16_t find_ret, uint16_t start_ret)
{
    ObjArm r = { 0 };
    uint8_t number, found_slot;
    bool found;

    WR8(ZP->temp00, timer);
    push8(y_entry);
    number = 0;
    for (;;) {
        WR8(ZP->hit_slot, number);
        jsr_push(find_ret);
        found_slot = FindFreeSlotFrom30(&r.first, &found);
        jsr_pop();
        r.second = found_slot;
        r.arm = DBU_NO_SLOT;
        if (!found) {
            break;
        }
        timer = RD8(ZP->temp00);
        WR8(OBJ->life_timer[found_slot], timer);
        jsr_push(start_ret);
        StartObjectAtPlace(0x9A, slot, found_slot);
        jsr_pop();
        number = RD8(ZP->hit_slot);
        WR8(OBJ->work_e[found_slot], number);
        r.third = number;
        number = RD8(ZP->hit_slot);
        number++;
        r.fourth = number;
        r.arm = DBU_STARTED;
        if (number >= 6) {
            break;
        }
    }
    r.fifth = pull8();
    return r;
}

/*
 * @routine 025A:10B StartDebrisBurst
 * @run 70:A90B
 * WHAT IT DOES: Starts up to six kind $9A objects at this object's place, each in the first free slot from $30, each given the
 * timer in A and its own number 0 to 5 in OBJ_WORK_E; stops at the first search that finds no slot.
 * HOW: Y is pushed and pulled back; zHitSlot keeps the count across the calls; FindFreeSlotFrom30, StartObjectAtPlace.
 * WAITS: no.
 * Entry: A the timer, X object, Y pushed and pulled back. Exit: Y as it came, N and Z from it; C 1 when all six were
 * started, 0 when a search found no slot; A the last byte loaded.
 */
ObjArm StartDebrisBurst(uint8_t slot, uint8_t timer, uint8_t y_entry)
{
    return debris_burst(slot, timer, y_entry, 0xA914, 0xA920);
}

/*
 * @routine 01BA:6D0 RichterStartDebrisBurst
 * @run 6D:AED0
 * WHAT IT DOES: StartDebrisBurst in Richter's overlay, the same bytes at his address.
 * HOW: StartDebrisBurst's body, with this copy's returns.
 * WAITS: no.
 * Entry, exit: as StartDebrisBurst.
 */
ObjArm RichterStartDebrisBurst(uint8_t slot, uint8_t timer, uint8_t y_entry)
{
    return debris_burst(slot, timer, y_entry, 0xAED9, 0xAEE5);
}

/* The last pass decides the exit: a search that found no slot, or the sixth start's loads and its count. */
Ended start_debris_burst_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == DBU_NO_SLOT) {
        e = ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), r.first), r.second), false);
    } else {
        e = ended_with_y(ended_with_a(ended_nz(r.third), r.third), r.second);
        e = ended_then(e, ended_with_carry(ended_with_y(ended_nz(r.fourth), r.fourth), r.fourth >= 6));
    }
    return ended_then(e, ended_with_y(ended_nz(r.fifth), r.fifth));
}

void abi_StartDebrisBurst(void)
{
    abi_ended(start_debris_burst_exit_shim(StartDebrisBurst(cpu.x, cpu.a, cpu.y)));
}

void abi_RichterStartDebrisBurst(void)
{
    abi_ended(start_debris_burst_exit_shim(RichterStartDebrisBurst(cpu.x, cpu.a, cpu.y)));
}
