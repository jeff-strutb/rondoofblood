/*
 * TEMPORARY: object routines every stage overlay carries its own copy of (they differ only in the
 * disc tables they read), kept together here until each has a home.
 * The disc tables are named here by their overlay address until the typed data layer reads them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"

/* Whether an item drops when the object is hit hard enough: see DropItemWhenHitA. */
static Ended drop_item_when_hit(uint8_t slot, uint16_t limits, uint16_t items)
{
    uint8_t hit = RD8(OBJ->hit_taken[slot]);
    uint8_t code, bits, y, limit, item, index, type, last, last2;
    bool found;
    uint8_t found_slot;
    RandomResult random;
    Sum sum, added;
    Ended e;

    e = ended_compare(hit, 5);
    if (hit < 5) {
        return e;
    }
    code = RD8(OBJ->f31f8[slot]);
    bits = (uint8_t)((code >> 2) & 7);
    e = ended_then(e, ended_with_carry(ended_load(bits), (code & 2) != 0));
    if (bits == 0) {
        return e;
    }
    y = (uint8_t)(bits - 1);
    e = ended_then(e, ended_with_y(ended_kept(), y));
    WR8(ZP->temp00, (uint8_t)(y << 4));
    jsr_push(0xA2E0);
    random = NextRandom();
    jsr_pop();
    limit = rd8((uint16_t)(limits + y));
    e = ended_then(ended_then(e, ended_of_random(random)), ended_compare(random.value, limit));
    if (random.value >= limit) {
        return e;
    }
    jsr_push(0xA2E8);
    sum = PlayerDistanceSum(slot, false);
    jsr_pop();
    e = ended_then(e, ended_of_sum(sum));
    index = (uint8_t)(sum.value & 0x0F);
    added = add_bytes(index, RD8(ZP->temp00), false);
    y = added.value;
    item = rd8((uint16_t)(items + y));
    WR8(ZP->temp00, item);
    e = ended_then(e, ended_with_y(ended_with_overflow(ended_with_carry(ended_load(item), added.carry),
                                                          added.overflow), y));
    jsr_push(0xA2F6);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = ended_then(e, ended_with_carry(ended_with_y(ended_load(last), found_slot), found));
    e = ended_then(e, ended_nz(0));
    if (!found) {
        return e;
    }
    type = RD8(ZP->temp00);
    e = ended_then(e, ended_load(type));
    jsr_push(0xA2FD);
    last2 = StartObjectAtPlace(type, slot, found_slot);
    jsr_pop();
    return ended_then(e, ended_load(last2));
}

/*
 * @routine 07D9:2C5 DropItemWhenHitA
 * @run 70:A2C5
 * WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
 * zero, may drop an item: a random number under the limit in a table at overlay address $AB3D
 * (indexed by those bits minus one) picks the chance; the item comes from a table at $AB45,
 * indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
 * a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
 * HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
 * reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
 * coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
 * free slot (or the table index); X kept. Clobbers zTemp00.
 */
Ended DropItemWhenHitA(uint8_t slot)
{
    enum { LIMITS = 0xAB3D, ITEMS = 0xAB45 };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitA(void)
{
    abi_ended(DropItemWhenHitA(cpu.x));
}

/*
 * @routine 0939:2C5 DropItemWhenHitB
 * @run 70:A2C5
 * WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
 * zero, may drop an item: a random number under the limit in a table at overlay address $ABFD
 * (indexed by those bits minus one) picks the chance; the item comes from a table at $AC05,
 * indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
 * a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
 * HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
 * reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
 * coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
 * free slot (or the table index); X kept. Clobbers zTemp00.
 */
Ended DropItemWhenHitB(uint8_t slot)
{
    enum { LIMITS = 0xABFD, ITEMS = 0xAC05 };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitB(void)
{
    abi_ended(DropItemWhenHitB(cpu.x));
}

/*
 * @routine 0BF9:2C5 DropItemWhenHitC
 * @run 70:A2C5
 * WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
 * zero, may drop an item: a random number under the limit in a table at overlay address $AC08
 * (indexed by those bits minus one) picks the chance; the item comes from a table at $AC10,
 * indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
 * a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
 * HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
 * reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
 * coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
 * free slot (or the table index); X kept. Clobbers zTemp00.
 */
Ended DropItemWhenHitC(uint8_t slot)
{
    enum { LIMITS = 0xAC08, ITEMS = 0xAC10 };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitC(void)
{
    abi_ended(DropItemWhenHitC(cpu.x));
}

/*
 * @routine 0D59:2C5 DropItemWhenHitD
 * @run 70:A2C5
 * WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
 * zero, may drop an item: a random number under the limit in a table at overlay address $ACD2
 * (indexed by those bits minus one) picks the chance; the item comes from a table at $ACDA,
 * indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
 * a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
 * HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
 * reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
 * coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
 * free slot (or the table index); X kept. Clobbers zTemp00.
 */
Ended DropItemWhenHitD(uint8_t slot)
{
    enum { LIMITS = 0xACD2, ITEMS = 0xACDA };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitD(void)
{
    abi_ended(DropItemWhenHitD(cpu.x));
}

/*
 * @routine 1179:2C5 DropItemWhenHitE
 * @run 70:A2C5
 * WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
 * zero, may drop an item: a random number under the limit in a table at overlay address $AC56
 * (indexed by those bits minus one) picks the chance; the item comes from a table at $AC5E,
 * indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
 * a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
 * HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
 * reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
 * coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
 * free slot (or the table index); X kept. Clobbers zTemp00.
 */
Ended DropItemWhenHitE(uint8_t slot)
{
    enum { LIMITS = 0xAC56, ITEMS = 0xAC5E };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitE(void)
{
    abi_ended(DropItemWhenHitE(cpu.x));
}
