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

/* Plays the sound the object's f31f8 asks for (its bits 5 to 7, minus one, index the table), if it is on the first screen. */
static ScreenSoundEnd sound_from_hit_code(uint8_t slot, uint16_t table)
{
    ScreenSoundEnd result = { ended_kept(), ended_kept(), false, { 0 } };
    uint8_t code = RD8(OBJ->f31f8[slot]);
    uint8_t bits = (uint8_t)((code >> 5) & 7);
    uint8_t y, command;
    bool carry = (code & 0x10) != 0;

    result.before = ended_with_carry(ended_load(bits), carry);
    if (bits == 0) {
        return result;
    }
    y = (uint8_t)(bits - 1);
    command = rd8((uint16_t)(table + y));
    result.before = ended_with_y(ended_with_carry(ended_load(command), carry), y);
    jsr_push(0xA312);
    result.sound = SoundIfOnFirstScreen(command, slot, y);
    jsr_pop();
    result.called = true;
    return result;
}

/*
 * @routine 0259:2FE PlayHitSoundA
 * @run 70:A2FE
 * WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
 * zero and the object is on the first screen: a table of sounds at overlay address $ABEE, indexed
 * by those bits minus one. Every stage overlay has its own copy with its own table.
 * HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
 * SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
 * played: as SoundIfOnFirstScreen leaves it (Y the table index).
 */
ScreenSoundEnd PlayHitSoundA(uint8_t slot)
{
    enum { TABLE = 0xABEE };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundA(void)
{
    abi_screen_sound_end(PlayHitSoundA(cpu.x));
}

/*
 * @routine 0519:2FE PlayHitSoundB
 * @run 70:A2FE
 * WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
 * zero and the object is on the first screen: a table of sounds at overlay address $AC23, indexed
 * by those bits minus one. Every stage overlay has its own copy with its own table.
 * HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
 * SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
 * played: as SoundIfOnFirstScreen leaves it (Y the table index).
 */
ScreenSoundEnd PlayHitSoundB(uint8_t slot)
{
    enum { TABLE = 0xAC23 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundB(void)
{
    abi_screen_sound_end(PlayHitSoundB(cpu.x));
}

/*
 * @routine 07D9:2FE PlayHitSoundC
 * @run 70:A2FE
 * WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
 * zero and the object is on the first screen: a table of sounds at overlay address $AB75, indexed
 * by those bits minus one. Every stage overlay has its own copy with its own table.
 * HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
 * SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
 * played: as SoundIfOnFirstScreen leaves it (Y the table index).
 */
ScreenSoundEnd PlayHitSoundC(uint8_t slot)
{
    enum { TABLE = 0xAB75 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundC(void)
{
    abi_screen_sound_end(PlayHitSoundC(cpu.x));
}

/*
 * @routine 0939:2FE PlayHitSoundD
 * @run 70:A2FE
 * WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
 * zero and the object is on the first screen: a table of sounds at overlay address $AC35, indexed
 * by those bits minus one. Every stage overlay has its own copy with its own table.
 * HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
 * SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
 * played: as SoundIfOnFirstScreen leaves it (Y the table index).
 */
ScreenSoundEnd PlayHitSoundD(uint8_t slot)
{
    enum { TABLE = 0xAC35 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundD(void)
{
    abi_screen_sound_end(PlayHitSoundD(cpu.x));
}

/*
 * @routine 09E9:2FE PlayHitSoundE
 * @run 70:A2FE
 * WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
 * zero and the object is on the first screen: a table of sounds at overlay address $AC17, indexed
 * by those bits minus one. Every stage overlay has its own copy with its own table.
 * HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
 * SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
 * played: as SoundIfOnFirstScreen leaves it (Y the table index).
 */
ScreenSoundEnd PlayHitSoundE(uint8_t slot)
{
    enum { TABLE = 0xAC17 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundE(void)
{
    abi_screen_sound_end(PlayHitSoundE(cpu.x));
}

/*
 * @routine 0BF9:2FE PlayHitSoundF
 * @run 70:A2FE
 * WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
 * zero and the object is on the first screen: a table of sounds at overlay address $AC40, indexed
 * by those bits minus one. Every stage overlay has its own copy with its own table.
 * HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
 * SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
 * played: as SoundIfOnFirstScreen leaves it (Y the table index).
 */
ScreenSoundEnd PlayHitSoundF(uint8_t slot)
{
    enum { TABLE = 0xAC40 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundF(void)
{
    abi_screen_sound_end(PlayHitSoundF(cpu.x));
}

/*
 * @routine 0CA9:2FE PlayHitSoundG
 * @run 70:A2FE
 * WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
 * zero and the object is on the first screen: a table of sounds at overlay address $AC64, indexed
 * by those bits minus one. Every stage overlay has its own copy with its own table.
 * HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
 * SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
 * played: as SoundIfOnFirstScreen leaves it (Y the table index).
 */
ScreenSoundEnd PlayHitSoundG(uint8_t slot)
{
    enum { TABLE = 0xAC64 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundG(void)
{
    abi_screen_sound_end(PlayHitSoundG(cpu.x));
}

/*
 * @routine 0D59:2FE PlayHitSoundH
 * @run 70:A2FE
 * WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
 * zero and the object is on the first screen: a table of sounds at overlay address $AD0A, indexed
 * by those bits minus one. Every stage overlay has its own copy with its own table.
 * HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
 * SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
 * played: as SoundIfOnFirstScreen leaves it (Y the table index).
 */
ScreenSoundEnd PlayHitSoundH(uint8_t slot)
{
    enum { TABLE = 0xAD0A };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundH(void)
{
    abi_screen_sound_end(PlayHitSoundH(cpu.x));
}

/*
 * @routine 0EB9:2FE PlayHitSoundI
 * @run 70:A2FE
 * WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
 * zero and the object is on the first screen: a table of sounds at overlay address $AC98, indexed
 * by those bits minus one. Every stage overlay has its own copy with its own table.
 * HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
 * SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
 * played: as SoundIfOnFirstScreen leaves it (Y the table index).
 */
ScreenSoundEnd PlayHitSoundI(uint8_t slot)
{
    enum { TABLE = 0xAC98 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundI(void)
{
    abi_screen_sound_end(PlayHitSoundI(cpu.x));
}

/*
 * @routine 1019:2FE PlayHitSoundJ
 * @run 70:A2FE
 * WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
 * zero and the object is on the first screen: a table of sounds at overlay address $AC74, indexed
 * by those bits minus one. Every stage overlay has its own copy with its own table.
 * HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
 * SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
 * played: as SoundIfOnFirstScreen leaves it (Y the table index).
 */
ScreenSoundEnd PlayHitSoundJ(uint8_t slot)
{
    enum { TABLE = 0xAC74 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundJ(void)
{
    abi_screen_sound_end(PlayHitSoundJ(cpu.x));
}

/*
 * @routine 1179:2FE PlayHitSoundK
 * @run 70:A2FE
 * WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
 * zero and the object is on the first screen: a table of sounds at overlay address $AC8E, indexed
 * by those bits minus one. Every stage overlay has its own copy with its own table.
 * HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
 * SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
 * played: as SoundIfOnFirstScreen leaves it (Y the table index).
 */
ScreenSoundEnd PlayHitSoundK(uint8_t slot)
{
    enum { TABLE = 0xAC8E };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundK(void)
{
    abi_screen_sound_end(PlayHitSoundK(cpu.x));
}

/*
 * @routine 16C9:2FE PlayHitSoundL
 * @run 70:A2FE
 * WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
 * zero and the object is on the first screen: a table of sounds at overlay address $ABBE, indexed
 * by those bits minus one. Every stage overlay has its own copy with its own table.
 * HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
 * SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
 * played: as SoundIfOnFirstScreen leaves it (Y the table index).
 */
ScreenSoundEnd PlayHitSoundL(uint8_t slot)
{
    enum { TABLE = 0xABBE };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundL(void)
{
    abi_screen_sound_end(PlayHitSoundL(cpu.x));
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

/*
 * @routine 0259:2C5 DropItemWhenHitF
 * @run 70:A2C5
 * WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
 * zero, may drop an item: a random number under the limit in a table at overlay address $ABC6
 * (indexed by those bits minus one) picks the chance; the item comes from a table at $ABCE,
 * indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
 * a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
 * HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
 * reaches as untraced bytes (most of the cases here) are NOT CHECKED; they wait for coverage
 * (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
 * free slot (or the table index); X kept. Clobbers zTemp00.
 */
Ended DropItemWhenHitF(uint8_t slot)
{
    enum { LIMITS = 0xABC6, ITEMS = 0xABCE };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitF(void)
{
    abi_ended(DropItemWhenHitF(cpu.x));
}

/*
 * @routine 09E9:2C5 DropItemWhenHitG
 * @run 70:A2C5
 * WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
 * zero, may drop an item: a random number under the limit in a table at overlay address $ABEF
 * (indexed by those bits minus one) picks the chance; the item comes from a table at $ABF7,
 * indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
 * a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
 * HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
 * reaches as untraced bytes (most of the cases here) are NOT CHECKED; they wait for coverage
 * (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
 * free slot (or the table index); X kept. Clobbers zTemp00.
 */
Ended DropItemWhenHitG(uint8_t slot)
{
    enum { LIMITS = 0xABEF, ITEMS = 0xABF7 };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitG(void)
{
    abi_ended(DropItemWhenHitG(cpu.x));
}

/*
 * @routine 0A99:2C5 DropItemWhenHitH
 * @run 70:A2C5
 * WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
 * zero, may drop an item: a random number under the limit in a table at overlay address $AC44
 * (indexed by those bits minus one) picks the chance; the item comes from a table at $AC4C,
 * indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
 * a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
 * HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
 * reaches as untraced bytes (most of the cases here) are NOT CHECKED; they wait for coverage
 * (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
 * free slot (or the table index); X kept. Clobbers zTemp00.
 */
Ended DropItemWhenHitH(uint8_t slot)
{
    enum { LIMITS = 0xAC44, ITEMS = 0xAC4C };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitH(void)
{
    abi_ended(DropItemWhenHitH(cpu.x));
}

/*
 * @routine 0CA9:2C5 DropItemWhenHitI
 * @run 70:A2C5
 * WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
 * zero, may drop an item: a random number under the limit in a table at overlay address $AC3C
 * (indexed by those bits minus one) picks the chance; the item comes from a table at $AC44,
 * indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
 * a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
 * HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
 * reaches as untraced bytes (most of the cases here) are NOT CHECKED; they wait for coverage
 * (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
 * free slot (or the table index); X kept. Clobbers zTemp00.
 */
Ended DropItemWhenHitI(uint8_t slot)
{
    enum { LIMITS = 0xAC3C, ITEMS = 0xAC44 };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitI(void)
{
    abi_ended(DropItemWhenHitI(cpu.x));
}

/*
 * @routine 0A99:2FE PlayHitSoundM
 * @run 70:A2FE
 * WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
 * zero and the object is on the first screen: a table of sounds at overlay address $AC7C, indexed
 * by those bits minus one. Every stage overlay has its own copy with its own table.
 * HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
 * SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
 * played: as SoundIfOnFirstScreen leaves it (Y the table index).
 */
ScreenSoundEnd PlayHitSoundM(uint8_t slot)
{
    enum { TABLE = 0xAC7C };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundM(void)
{
    abi_screen_sound_end(PlayHitSoundM(cpu.x));
}

/*
 * @routine 088A:5AF FaceThePlayerInSomeStatesC
 * @run 70:ADAF
 * WHAT IT DOES: FaceThePlayerInSomeStates in the overlay at sector 088A, with its table at $ADC0.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as FaceThePlayerInSomeStates.
 */
ObjArm FaceThePlayerInSomeStatesC(uint8_t slot)
{
    return face_player_in_states(slot, 0xADC0, 0xADBA);
}

void abi_FaceThePlayerInSomeStatesC(void)
{
    abi_side_end(face_player_in_states_exit_shim(FaceThePlayerInSomeStatesC(cpu.x)));
}

/*
 * @routine 0A9C:7FE FaceThePlayerInSomeStatesD
 * @run 71:BFFE
 * WHAT IT DOES: FaceThePlayerInSomeStates in the overlay at sector 0A9C, with its table at $C00F.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as FaceThePlayerInSomeStates.
 */
ObjArm FaceThePlayerInSomeStatesD(uint8_t slot)
{
    return face_player_in_states(slot, 0xC00F, 0xC009);
}

void abi_FaceThePlayerInSomeStatesD(void)
{
    abi_side_end(face_player_in_states_exit_shim(FaceThePlayerInSomeStatesD(cpu.x)));
}

/*
 * @routine 0A9E:591 ThrowByNextKindB
 * @run 71:CD91
 * WHAT IT DOES: ThrowByNextKind in the overlay at sector 0A9E, with its X speed table at $CDB4.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as ThrowByNextKind.
 */
ObjArm ThrowByNextKindB(uint8_t slot)
{
    return throw_by_next_kind(slot, 0xCDB4, 0xCDAD);
}

void abi_ThrowByNextKindB(void)
{
    abi_ended(throw_by_next_kind_exit_shim(ThrowByNextKindB(cpu.x)));
}

/*
 * @routine 0A9E:5B6 PickTimersAtRandomB
 * @run 71:CDB6
 * WHAT IT DOES: PickTimersAtRandom in the overlay at sector 0A9E, with its table at $CDCA.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as PickTimersAtRandom.
 */
ObjArm PickTimersAtRandomB(uint8_t slot)
{
    return pick_pair_at_random(slot, 0xCDCA, 0xCDB9, false);
}

void abi_PickTimersAtRandomB(void)
{
    abi_random_end(pick_pair_at_random_exit_shim(PickTimersAtRandomB(cpu.x)));
}

/*
 * @routine 0A9E:5D2 PickAngleAtRandomB
 * @run 71:CDD2
 * WHAT IT DOES: PickAngleAtRandom in the overlay at sector 0A9E, with its table at $CDE6.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as PickAngleAtRandom.
 */
ObjArm PickAngleAtRandomB(uint8_t slot)
{
    return pick_pair_at_random(slot, 0xCDE6, 0xCDD5, true);
}

void abi_PickAngleAtRandomB(void)
{
    abi_random_end(pick_pair_at_random_exit_shim(PickAngleAtRandomB(cpu.x)));
}

/*
 * @routine 117C:73A WalkWhileFrameChangesD
 * @run 70:BF3A
 * WHAT IT DOES: WalkWhileFrameChangesB in the overlay at sector 117C: lists at $BF56 plus Y, X
 * speeds at $BF5A by f3510.
 * HOW: StepObjectFrames, then ApplyVelocityX.
 * WAITS: no.
 * Entry, exit: as WalkWhileFrameChangesB.
 */
Ended WalkWhileFrameChangesD(uint8_t slot, uint8_t index)
{
    enum { LISTS = 0xBF56, SPEEDS = 0xBF5A };
    uint8_t exit_y = index, phase;
    FrameStep step;
    AxisMove move;
    Ended e;

    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    jsr_push(0xBF47);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, exit_y);
    if (step != FRAME_CHANGED) {
        return e;
    }
    phase = RD8(OBJ->f3510[slot]);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(SPEEDS + phase)));
    jsr_push(0xBF55);
    move = ApplyVelocityX(slot);
    jsr_pop();
    return ended_then(ended_then(e, ended_with_y(ended_kept(), phase)), ended_of_axis_move(move));
}

void abi_WalkWhileFrameChangesD(void)
{
    abi_ended(WalkWhileFrameChangesD(cpu.x, cpu.y));
}
