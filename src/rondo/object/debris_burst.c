/*
 * The common burst of debris the stage overlays start when something breaks: several kind $9A objects
 * at one object's place, each remembering its number.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

/*
 * @routine 01BA:6D0 StartDebrisBurst
 * @run 70:A90B
 * WHAT IT DOES: Starts up to six kind $9A objects at this object's place, each in the first free slot from $30, each given the
 * timer in A and its own number 0 to 5 in OBJ_3510; stops at the first search that finds no slot.
 * HOW: Y is pushed and pulled back; zHitSlot keeps the count across the calls; FindFreeSlotFrom30, StartObjectAtPlace.
 * WAITS: no.
 * Entry: A the timer, X object, Y pushed and pulled back. Exit: Y as it came, N and Z from it; C 1 when all six were
 * started, 0 when a search found no slot; A the last byte loaded.
 */
Ended StartDebrisBurst(uint8_t slot, uint8_t timer, uint8_t y_entry)
{
    uint8_t number, last, found_slot, placed, again;
    bool found;
    Ended e = ended_load(timer);

    WR8(ZP->temp00, timer);
    push8(y_entry);
    number = 0;
    for (;;) {
        WR8(ZP->hit_slot, number);
        jsr_push(0xA914);
        found_slot = FindFreeSlotFrom30(&last, &found);
        jsr_pop();
        e = ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), last), found_slot), found);
        if (!found) {
            break;
        }
        timer = RD8(ZP->temp00);
        WR8(OBJ->life_timer[found_slot], timer);
        jsr_push(0xA920);
        placed = StartObjectAtPlace(0x9A, slot, found_slot);
        jsr_pop();
        number = RD8(ZP->hit_slot);
        WR8(OBJ->f3510[found_slot], number);
        e = ended_with_y(ended_with_a(ended_nz(number), number), found_slot);
        number = RD8(ZP->hit_slot);
        number++;
        e = ended_then(e, ended_with_carry(ended_with_y(ended_nz(number), number), number >= 6));
        if (number >= 6) {
            break;
        }
    }
    again = pull8();
    (void)placed;
    return ended_then(e, ended_with_y(ended_nz(again), again));
}

void abi_StartDebrisBurst(void)
{
    abi_ended(StartDebrisBurst(cpu.x, cpu.a, cpu.y));
}
