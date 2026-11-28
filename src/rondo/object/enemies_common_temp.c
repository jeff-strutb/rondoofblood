/*
 * TEMPORARY: object routines every stage overlay carries its own copy of (they differ only in the
 * disc tables they read), kept together here until each has a home.
 * The disc tables are named here by their overlay address until the typed data layer reads them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"

/* The arms the shared bodies end on. */
enum { CT_DI_MILD, CT_DI_NO_BITS, CT_DI_ROLL, CT_DI_NO_SLOT, CT_DI_DROPPED };
enum { CT_HS_NONE, CT_HS_PLAYED };
enum { CT_WD_STEPPED, CT_WD_MOVED };

/* Whether an item drops when the object is hit hard enough: see DropItemWhenHitA. */
ObjArm drop_item_when_hit(uint8_t slot, uint16_t limits, uint16_t items)
{
    ObjArm r = { 0 };
    uint8_t found_slot;
    bool found;

    r.first = RD8(OBJ->hit_taken[slot]);
    r.arm = CT_DI_MILD;
    if (r.first < 5) {
        return r;
    }
    r.second = RD8(OBJ->hit_reaction[slot]);
    r.third = (uint8_t)((r.second >> 2) & 7);
    r.arm = CT_DI_NO_BITS;
    if (r.third == 0) {
        return r;
    }
    r.fourth = (uint8_t)(r.third - 1);
    WR8(ZP->temp00, (uint8_t)(r.fourth << 4));
    jsr_push(0xA2E0);
    r.random = NextRandom();
    jsr_pop();
    r.fifth = rd8((uint16_t)(limits + r.fourth));
    r.arm = CT_DI_ROLL;
    if (r.random.value >= r.fifth) {
        return r;
    }
    jsr_push(0xA2E8);
    r.sum = PlayerDistanceSum(slot, false);
    jsr_pop();
    r.sum2 = add_bytes((uint8_t)(r.sum.value & 0x0F), RD8(ZP->temp00), false);
    r.sixth = rd8((uint16_t)(items + r.sum2.value));
    WR8(ZP->temp00, r.sixth);
    jsr_push(0xA2F6);
    found_slot = FindFreeSlotFrom30(&r.eighth, &found);
    jsr_pop();
    r.seventh = found_slot;
    r.flag = found;
    r.arm = CT_DI_NO_SLOT;
    if (!found) {
        return r;
    }
    r.ninth = RD8(ZP->temp00);
    jsr_push(0xA2FD);
    r.tenth = StartObjectAtPlace(r.ninth, slot, found_slot);
    jsr_pop();
    r.arm = CT_DI_DROPPED;
    return r;
}

/* The exit of every copy of DropItemWhenHit. */
Ended drop_item_when_hit_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 5);

    if (r.arm == CT_DI_MILD) {
        return e;
    }
    e = ended_then(e, ended_with_carry(ended_load(r.third), (r.second & 2) != 0));
    if (r.arm == CT_DI_NO_BITS) {
        return e;
    }
    e = ended_then(e, ended_with_y(ended_kept(), r.fourth));
    e = ended_then(ended_then(e, ended_of_random(r.random)), ended_compare(r.random.value, r.fifth));
    if (r.arm == CT_DI_ROLL) {
        return e;
    }
    e = ended_then(e, ended_of_sum(r.sum));
    e = ended_then(e, ended_with_y(ended_with_overflow(ended_with_carry(ended_load(r.sixth), r.sum2.carry), r.sum2.overflow), r.sum2.value));
    e = ended_then(e, ended_with_carry(ended_with_y(ended_load(r.eighth), r.seventh), r.flag));
    e = ended_then(e, ended_nz(0));
    if (r.arm == CT_DI_NO_SLOT) {
        return e;
    }
    e = ended_then(e, ended_load(r.ninth));
    return ended_then(e, ended_load(r.tenth));
}

/* Plays the sound the object's f31f8 asks for (its bits 5 to 7, minus one, index the table), if it is on the first screen. */
ObjArm sound_from_hit_code(uint8_t slot, uint16_t table)
{
    ObjArm r = { 0 };
    uint8_t code = RD8(OBJ->hit_reaction[slot]);

    r.first = (uint8_t)((code >> 5) & 7);
    r.flag = (code & 0x10) != 0;
    r.arm = CT_HS_NONE;
    if (r.first == 0) {
        return r;
    }
    r.second = (uint8_t)(r.first - 1);
    r.third = rd8((uint16_t)(table + r.second));
    jsr_push(0xA312);
    r.screen = SoundIfOnFirstScreen(r.third, slot, r.second);
    jsr_pop();
    r.screen_called = true;
    r.arm = CT_HS_PLAYED;
    return r;
}

/* The exit of every copy of PlayHitSound: the bits' load, then the table's command. */
ScreenSoundEnd play_hit_sound_exit_shim(ObjArm r)
{
    ScreenSoundEnd s = { ended_kept(), ended_kept(), false, { 0 } };

    s.before = ended_with_carry(ended_load(r.first), r.flag);
    if (r.arm == CT_HS_NONE) {
        return s;
    }
    s.before = ended_with_y(ended_with_carry(ended_load(r.third), r.flag), r.second);
    s.sound = r.screen;
    s.called = true;
    return s;
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
ObjArm PlayHitSoundA(uint8_t slot)
{
    enum { TABLE = 0xABEE };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundA(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSoundA(cpu.x)));
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
ObjArm PlayHitSoundB(uint8_t slot)
{
    enum { TABLE = 0xAC23 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundB(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSoundB(cpu.x)));
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
ObjArm PlayHitSoundC(uint8_t slot)
{
    enum { TABLE = 0xAB75 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundC(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSoundC(cpu.x)));
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
ObjArm PlayHitSoundD(uint8_t slot)
{
    enum { TABLE = 0xAC35 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundD(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSoundD(cpu.x)));
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
ObjArm PlayHitSoundE(uint8_t slot)
{
    enum { TABLE = 0xAC17 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundE(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSoundE(cpu.x)));
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
ObjArm PlayHitSoundF(uint8_t slot)
{
    enum { TABLE = 0xAC40 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundF(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSoundF(cpu.x)));
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
ObjArm PlayHitSoundG(uint8_t slot)
{
    enum { TABLE = 0xAC64 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundG(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSoundG(cpu.x)));
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
ObjArm PlayHitSoundH(uint8_t slot)
{
    enum { TABLE = 0xAD0A };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundH(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSoundH(cpu.x)));
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
ObjArm PlayHitSoundI(uint8_t slot)
{
    enum { TABLE = 0xAC98 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundI(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSoundI(cpu.x)));
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
ObjArm PlayHitSoundJ(uint8_t slot)
{
    enum { TABLE = 0xAC74 };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundJ(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSoundJ(cpu.x)));
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
ObjArm PlayHitSoundK(uint8_t slot)
{
    enum { TABLE = 0xAC8E };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundK(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSoundK(cpu.x)));
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
ObjArm PlayHitSoundL(uint8_t slot)
{
    enum { TABLE = 0xABBE };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundL(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSoundL(cpu.x)));
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
ObjArm DropItemWhenHitA(uint8_t slot)
{
    enum { LIMITS = 0xAB3D, ITEMS = 0xAB45 };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitA(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHitA(cpu.x)));
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
ObjArm DropItemWhenHitB(uint8_t slot)
{
    enum { LIMITS = 0xABFD, ITEMS = 0xAC05 };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitB(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHitB(cpu.x)));
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
ObjArm DropItemWhenHitC(uint8_t slot)
{
    enum { LIMITS = 0xAC08, ITEMS = 0xAC10 };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitC(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHitC(cpu.x)));
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
ObjArm DropItemWhenHitD(uint8_t slot)
{
    enum { LIMITS = 0xACD2, ITEMS = 0xACDA };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitD(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHitD(cpu.x)));
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
ObjArm DropItemWhenHitE(uint8_t slot)
{
    enum { LIMITS = 0xAC56, ITEMS = 0xAC5E };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitE(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHitE(cpu.x)));
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
ObjArm DropItemWhenHitF(uint8_t slot)
{
    enum { LIMITS = 0xABC6, ITEMS = 0xABCE };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitF(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHitF(cpu.x)));
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
ObjArm DropItemWhenHitG(uint8_t slot)
{
    enum { LIMITS = 0xABEF, ITEMS = 0xABF7 };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitG(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHitG(cpu.x)));
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
ObjArm DropItemWhenHitH(uint8_t slot)
{
    enum { LIMITS = 0xAC44, ITEMS = 0xAC4C };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitH(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHitH(cpu.x)));
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
ObjArm DropItemWhenHitI(uint8_t slot)
{
    enum { LIMITS = 0xAC3C, ITEMS = 0xAC44 };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitI(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHitI(cpu.x)));
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
ObjArm PlayHitSoundM(uint8_t slot)
{
    enum { TABLE = 0xAC7C };

    return sound_from_hit_code(slot, TABLE);
}

void abi_PlayHitSoundM(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSoundM(cpu.x)));
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
ObjArm WalkWhileFrameChangesD(uint8_t slot, uint8_t index)
{
    enum { LISTS = 0xBF56, SPEEDS = 0xBF5A };
    ObjArm r = { 0 };
    uint8_t exit_y = index;

    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    jsr_push(0xBF47);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    r.arm = CT_WD_STEPPED;
    if (r.step != FRAME_CHANGED) {
        return r;
    }
    r.first = RD8(OBJ->work_e[slot]);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(SPEEDS + r.first)));
    jsr_push(0xBF55);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    r.arm = CT_WD_MOVED;
    return r;
}

static Ended walk_while_frame_changes_d_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == CT_WD_STEPPED) {
        return e;
    }
    return ended_then(ended_then(e, ended_with_y(ended_kept(), r.first)), ended_of_axis_move(r.move));
}

void abi_WalkWhileFrameChangesD(void)
{
    abi_ended(walk_while_frame_changes_d_exit_shim(WalkWhileFrameChangesD(cpu.x, cpu.y)));
}
