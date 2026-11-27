/*
 * What every stage overlay's objects do when the player hits them: the damage a hit does, the score it earns and the handler that
 * runs a hit through all of that. (The overlay at sector 0259 holds the copy these are filed from.)
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

/*
 * @routine 0259:28D HitDamage
 * @run 70:A28D
 * WHAT IT DOES: Looks up how much damage the object takes from the hit it has taken: a table at $ABA6 indexed by the object's OBJ_2EE0 times sixteen plus its hit; the damage goes to zTemp00.
 * HOW: Four shifts, an add, a table read (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
 */
/* HitDamage's body with the table of the overlay that holds it (the copies differ only in the table's address). */
static ObjArm hit_damage_at(uint8_t slot, uint16_t table)
{
    ObjArm r = { 0 };
    uint8_t row, hit;

    row = RD8(OBJ->fields_2ee0[slot]);
    r.first = (uint8_t)(row << 4);
    WR8(ZP->temp00, r.first);
    hit = RD8(OBJ->hit_taken[slot]);
    r.sum = add_bytes(hit, RD8(ZP->temp00), false);
    r.second = rd8((uint16_t)(table + r.sum.value));
    WR8(ZP->temp00, r.second);
    return r;
}

ObjArm HitDamage(uint8_t slot)
{
    return hit_damage_at(slot, 0xABA6);
}

/*
 * The JSR $A28D goes to the HitDamage of the overlay loaded there: the table of the copy resident at $A28D (this
 * file's and the 17 of enemies_l8_hit_damage.c), 0 when the copy there has no C.
 */
static uint16_t hit_damage_table_resident(void)
{
    static const struct { uint16_t sector; uint16_t table; } copies[] = {
        { 0x0259, 0xABA6 }, { 0x0519, 0xABDB }, { 0x05C9, 0xAB8F }, { 0x0729, 0xAB97 }, { 0x07D9, 0xAB0D },
        { 0x0889, 0xAA94 }, { 0x0939, 0xABCD }, { 0x0A99, 0xAC14 }, { 0x0B49, 0xAA98 }, { 0x0BF9, 0xABE8 },
        { 0x0CA9, 0xABFC }, { 0x0D59, 0xACA2 }, { 0x0E09, 0xAB01 }, { 0x0EB9, 0xAC50 }, { 0x1019, 0xAC0C },
        { 0x10C9, 0xABAB }, { 0x1179, 0xAC16 }, { 0x16C9, 0xAB66 },
    };
    uint32_t at = resident_at(0xA28D);

    for (unsigned i = 0; i < sizeof copies / sizeof copies[0]; i++)
        if (at == DISC(copies[i].sector, 0x28D))
            return copies[i].table;
    return 0;
}

/* A copy with no C: its code run as the original's (the frame already pushed), its exit as the arm the shim reads. */
static ObjArm hit_damage_original_shim(uint8_t slot)
{
    ObjArm r = { 0 };

    cpu.x = slot;
    call_orig_framed(0xA1E8, 0xA28D);
    r.second = cpu.a;
    r.sum.value = cpu.y;
    r.sum.carry = (cpu.p & P_C) != 0;
    r.sum.overflow = (cpu.p & P_V) != 0;
    return r;
}

/* The shifted row's load with C clear, the table index's sum, the damage's load with Y the index. */
static Ended hit_damage_exit_shim(ObjArm r)
{
    Ended e = ended_with_carry(ended_load(r.first), false);

    e = ended_then(e, ended_of_sum(r.sum));
    return ended_then(e, ended_with_y(ended_load(r.second), r.sum.value));
}

void abi_HitDamage(void)
{
    abi_ended(hit_damage_exit_shim(HitDamage(cpu.x)));
}

/*
 * @routine 0259:2A3 ScoreForKind
 * @run 70:A2A3
 * WHAT IT DOES: When the byte HitAttackFlags has bit 0 clear, adds the score a table at $ABF5 gives for the object's kind less $20 (two bytes of BCD, to zTemp00-01); when that carries over
 * the mark the score tests for it, the player gets an extra life (GiveExtraLife).
 * HOW: AddScoreWord, a table read (bank 70).
 * Untraced: the extra life (A2C1).
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: see the wrapper.
 */
enum { HH_SK_FLAGGED, HH_SK_NO_CHANGE, HH_SK_LIFE };

ObjArm ScoreForKind(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t kind;

    r.first = rd8(HitAttackFlags);
    r.first = (uint8_t)(r.first & 0x01);
    r.arm = HH_SK_FLAGGED;
    if (r.first != 0) {
        return r;
    }
    kind = RD8(OBJ->kind[slot]);
    r.sum = sub_bytes(kind, 0x20, true);
    r.y = (uint8_t)(r.sum.value << 1);
    WR8(ZP->temp00, rd8((uint16_t)(0xABF5 + r.y)));
    WR8(ZP->temp01, rd8((uint16_t)(0xABF6 + r.y)));
    jsr_push(0xA2BF);
    r.second = AddScoreWord();
    jsr_pop();
    r.arm = HH_SK_NO_CHANGE;
    if (r.second == 0) {
        return r;
    }
    jsr_push(0xA2C4);
    r.sub = obj_sub(GiveExtraLife(slot, y));
    jsr_pop();
    r.arm = HH_SK_LIFE;
    return r;
}

/* The attack flags' load; else the kind's subtraction, the score table's load with Y and C, and the add's result; when it changed,
   the extra life's exit (its sound before, its end after these). */
SoundEnd score_for_kind_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e;

    if (r.arm == HH_SK_FLAGGED) {
        s.end = ended_load(r.first);
        return s;
    }
    e = ended_then(ended_of_sum(r.sum), ended_with_y(ended_with_carry(ended_load(r.y), (r.sum.value & 0x80) != 0), r.y));
    e = ended_then(e, ended_with_carry(ended_load(r.second), r.second != 0));
    s.end = e;
    if (r.arm == HH_SK_NO_CHANGE) {
        return s;
    }
    s = give_extra_life_exit_shim(obj_arm(r.sub));
    s.end = ended_then(e, s.end);
    return s;
}

void abi_ScoreForKind(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind(cpu.x, cpu.y)));
}

/*
 * @routine 0259:186 ObjectHit
 * @run 70:A186
 * WHAT IT DOES: What an object does with the hits it has taken this frame (the handler every enemy's state runs): its two attack marks are cleared; an object already marked dead (status bit 7) is left;
 * the player's attacks (HitAttackFlags bits 0 and 1) mark it and decide whether it is hit; a hit with damage (HitDamage) shortens its health OBJ_32A8, flashes it ($86) and gives it a hurt timer;
 * when the health runs out or an attack kills it outright it records its event, scores (ScoreForKind), may drop an item (DropItemWhenHitF) and plays its hit sound (PlayHitSoundA), then turns into
 * the kind $9B plus its death number (or is cleared when that number is 7), or, with no number, is marked dead and its hit state cleared.
 * HOW: Bit tests and stores in the order of the assembly; the steps are returned as an exit chain.
 * Untraced: two arms of the hit and the death are shared with the chain; the draft reaches them as stubs (see liftcmp).
 * WAITS: no.
 * Entry: X object, Y the registers' Y for the sound calls. Exit: see abi_chain.
 */
/* Where in r.bytes ObjectHit keeps the values it read and made, in the order of the assembly. */
enum {
    OH_STATUS1, OH_BITS, OH_BITS2, OH_BITS3, OH_FLASH, OH_STATUS2, OH_ATTACK1, OH_STATUS_A, OH_FLAGS_A, OH_ATTACK2, OH_STATUS_B, OH_FLAGS_B,
    OH_HP, OH_HIT, OH_DAMAGE, OH_HIT_Y, OH_BYTE1, OH_BYTE2, OH_FLAGS3, OH_STATUS_C, OH_SHOUT_BITS, OH_COMMAND, OH_NUMBER, OH_FLAGS_N,
    OH_KIND1, OH_KIND2, OH_STATUS_N, OH_STORED, OH_EVENT, OH_RECORD_BYTE
};
/* Arms: the point it returned from. */
enum { OH_DEAD, OH_HP_NEGATIVE, OH_NO_HIT, OH_NO_DAMAGE, OH_SHOUT, OH_TAIL_NO_NUMBER, OH_TAIL_CLEARED, OH_TAIL_KIND };

HitArm ObjectHit(uint8_t slot, uint8_t y)
{
    HitArm h = { { 0 }, { 0 } };
    ObjArm *r = &h.arm;
    uint8_t status, bits, flags, hit, command, kind, number, hp, damage;
    uint8_t stored, event, byte;
    bool marked;
    bool record_now = false;

    status = RD8(OBJ->status_flags[slot]);
    status = (uint8_t)(status & 0x9F);
    WR8(OBJ->status_flags[slot], status);
    r->bytes[OH_STATUS1] = status;
    bits = RD8(OBJ->f31f8[slot]);
    bits = (uint8_t)(bits & 0x03);
    r->bytes[OH_BITS] = bits;
    if (bits != 0) {
        r->bytes[OH_BITS2] = DEC8(OBJ->f31f8[slot]);
        bits = RD8(OBJ->f31f8[slot]);
        bits = (uint8_t)(bits & 0x03);
        r->bytes[OH_BITS3] = bits;
        if (bits == 0) {
            r->bytes[OH_FLASH] = RD8_IDX(0x3AE8, slot);
            WR8(OBJ->flash_flags[slot], r->bytes[OH_FLASH]);
        }
    }
    status = RD8(OBJ->status_flags[slot]);
    status = (uint8_t)(status & 0x80);
    r->bytes[OH_STATUS2] = status;
    if (status != 0) {
        r->arm = OH_DEAD;
        return h;
    }
    bits = (uint8_t)(rd8(HitAttackFlags) & 0x01);
    r->bytes[OH_ATTACK1] = bits;
    if (bits != 0) {
        status = RD8(OBJ->status_flags[slot]);
        status = (uint8_t)(status | 0x20);
        WR8(OBJ->status_flags[slot], status);
        r->bytes[OH_STATUS_A] = status;
        flags = RD8(OBJ->f30f0[slot]);
        flags = (uint8_t)(flags & 0x20);
        r->bytes[OH_FLAGS_A] = flags;
        record_now = flags != 0;
    }
    if (!record_now) {
        bits = (uint8_t)(rd8(HitAttackFlags) & 0x02);
        r->bytes[OH_ATTACK2] = bits;
        if (bits != 0) {
            status = RD8(OBJ->status_flags[slot]);
            status = (uint8_t)(status | 0x40);
            WR8(OBJ->status_flags[slot], status);
            r->bytes[OH_STATUS_B] = status;
            flags = RD8(OBJ->f30f0[slot]);
            flags = (uint8_t)(flags & 0x40);
            r->bytes[OH_FLAGS_B] = flags;
            record_now = flags != 0;
        }
    }
    if (!record_now) {
        r->bytes[OH_HP] = RD8(OBJ->f32a8[slot]);
        if ((r->bytes[OH_HP] & 0x80) != 0) {
            r->arm = OH_HP_NEGATIVE;
            return h;
        }
        hit = RD8(OBJ->hit_taken[slot]);
        r->bytes[OH_HIT] = hit;
        if (hit == 0) {
            r->arm = OH_NO_HIT;
            return h;
        }
        jsr_push(0xA1E8);
        {
            uint16_t table = hit_damage_table_resident();

            r->sub3 = obj_sub(table ? hit_damage_at(slot, table) : hit_damage_original_shim(slot));
        }
        jsr_pop();
        r->bytes[OH_DAMAGE] = RD8(ZP->temp00);
        if (r->bytes[OH_DAMAGE] == 0) {
            r->arm = OH_NO_DAMAGE;
            return h;
        }
        y = RD8(OBJ->hit_taken[slot]);
        r->bytes[OH_HIT_Y] = y;
        byte = rd8((uint16_t)(0xA26D + y));
        WR8(OBJ->timer[slot], byte);
        r->bytes[OH_BYTE1] = byte;
        byte = rd8((uint16_t)(0xA27D + y));
        WR8(OBJ->f2f90[slot], byte);
        r->bytes[OH_BYTE2] = byte;
        r->bytes[OH_FLASH] = RD8(OBJ->flash_flags[slot]);
        WR8_IDX(0x3AE8, slot, r->bytes[OH_FLASH]);
        WR8(OBJ->flash_flags[slot], 0x86);
        flags = RD8(OBJ->f31f8[slot]);
        flags = (uint8_t)(flags | 0x03);
        WR8(OBJ->f31f8[slot], flags);
        r->bytes[OH_FLAGS3] = flags;
        hp = RD8(OBJ->f32a8[slot]);
        damage = RD8(ZP->temp00);
        r->sum = sub_bytes(hp, damage, true);
        WR8(OBJ->f32a8[slot], r->sum.value);
        if (r->sum.carry) {
            status = RD8(OBJ->status_flags[slot]);
            r->bytes[OH_STATUS_C] = status;
            bits = (uint8_t)((status >> 3) & 0x03);
            r->bytes[OH_SHOUT_BITS] = bits;
            y = bits;
            command = rd8((uint16_t)(0xA269 + y));
            r->bytes[OH_COMMAND] = command;
            r->screen = SoundIfOnFirstScreen(command, slot, y);
            r->screen_called = true;
            r->arm = OH_SHOUT;
            return h;
        }
    }
    jsr_push(0xA22C);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    jsr_pop();
    r->other = marked;
    if (marked) {
        y = byte;
        r->bytes[OH_STORED] = stored;
        r->bytes[OH_EVENT] = event;
        r->bytes[OH_RECORD_BYTE] = byte;
    }
    jsr_push(0xA22F);
    h.score = ScoreForKind(slot, y);
    jsr_pop();
    jsr_push(0xA232);
    r->sub = obj_sub(DropItemWhenHitF(slot));
    jsr_pop();
    jsr_push(0xA235);
    r->sub2 = obj_sub(PlayHitSoundA(slot));
    jsr_pop();
    status = RD8(OBJ->status_flags[slot]);
    number = (uint8_t)(status & 0x07);
    r->bytes[OH_NUMBER] = number;
    if (number == 0) {
        flags = RD8(OBJ->f30f0[slot]);
        flags = (uint8_t)(flags & 0xFB);
        WR8(OBJ->f30f0[slot], flags);
        r->bytes[OH_FLAGS_N] = flags;
        kind = RD8(OBJ->f3148[slot]);
        kind = (uint8_t)(kind & 0x87);
        r->bytes[OH_KIND1] = kind;
        kind = (uint8_t)(kind | 0x04);
        r->bytes[OH_KIND2] = kind;
        WR8(OBJ->f3148[slot], kind);
        status = RD8(OBJ->status_flags[slot]);
        status = (uint8_t)(status | 0x80);
        WR8(OBJ->status_flags[slot], status);
        r->bytes[OH_STATUS_N] = status;
        r->arm = OH_TAIL_NO_NUMBER;
        return h;
    }
    if (number == 7) {
        ClearObject(slot);
        r->arm = OH_TAIL_CLEARED;
        return h;
    }
    r->sum2 = add_bytes((uint8_t)(number - 1), 0x9B, false);
    WR8(OBJ->kind[slot], r->sum2.value);
    WR8(OBJ->state[slot], 0);
    r->arm = OH_TAIL_KIND;
    return h;
}

/* The exit steps in order, as the old chain applied them: the loads and compares along the walk, HitDamage's exit, the sound
   the first screen gets, then the event record, the score, the item drop, the hit sound and the last loads of the death. */
void object_hit_exit_shim(HitArm h)
{
    const ObjArm *r = &h.arm;
    const uint8_t *v = r->bytes;
    ScreenSoundEnd shout = { { 0 }, { 0 }, true, { 0 } };
    Ended e = ended_load(v[OH_STATUS1]);
    bool record_now;

    e = ended_then(e, ended_load(v[OH_BITS]));
    if (v[OH_BITS] != 0) {
        e = ended_then(e, ended_nz(v[OH_BITS2]));
        e = ended_then(e, ended_load(v[OH_BITS3]));
        if (v[OH_BITS3] == 0) {
            e = ended_then(e, ended_load(v[OH_FLASH]));
        }
    }
    e = ended_then(e, ended_load(v[OH_STATUS2]));
    if (r->arm == OH_DEAD) {
        abi_ended(e);
        return;
    }
    e = ended_then(e, ended_load(v[OH_ATTACK1]));
    record_now = false;
    if (v[OH_ATTACK1] != 0) {
        e = ended_then(e, ended_load(v[OH_STATUS_A]));
        e = ended_then(e, ended_load(v[OH_FLAGS_A]));
        record_now = v[OH_FLAGS_A] != 0;
    }
    if (!record_now) {
        e = ended_then(e, ended_load(v[OH_ATTACK2]));
        if (v[OH_ATTACK2] != 0) {
            e = ended_then(e, ended_load(v[OH_STATUS_B]));
            e = ended_then(e, ended_load(v[OH_FLAGS_B]));
            record_now = v[OH_FLAGS_B] != 0;
        }
    }
    if (!record_now) {
        e = ended_then(e, ended_load(v[OH_HP]));
        if (r->arm == OH_HP_NEGATIVE) {
            abi_ended(e);
            return;
        }
        e = ended_then(e, ended_load(v[OH_HIT]));
        if (r->arm == OH_NO_HIT) {
            abi_ended(e);
            return;
        }
        abi_ended(e);
        abi_ended(hit_damage_exit_shim(obj_arm(r->sub3)));
        e = ended_then(ended_kept(), ended_load(v[OH_DAMAGE]));
        if (r->arm == OH_NO_DAMAGE) {
            abi_ended(e);
            return;
        }
        e = ended_then(e, ended_with_y(ended_nz(v[OH_HIT_Y]), v[OH_HIT_Y]));
        e = ended_then(e, ended_load(v[OH_BYTE1]));
        e = ended_then(e, ended_load(v[OH_BYTE2]));
        e = ended_then(e, ended_load(0x86));
        e = ended_then(e, ended_load(v[OH_FLAGS3]));
        e = ended_then(e, ended_of_sum(r->sum));
        if (r->arm == OH_SHOUT) {
            e = ended_then(e, ended_with_y(ended_with_carry(ended_load(v[OH_SHOUT_BITS]), (v[OH_STATUS_C] & 0x04) != 0), v[OH_SHOUT_BITS]));
            e = ended_then(e, ended_load(v[OH_COMMAND]));
            abi_ended(e);
            shout.sound = r->screen;
            abi_screen_sound_end(shout);
            return;
        }
    }
    abi_ended(e);
    if (r->other) {
        abi_ended(ended_with_carry(ended_with_y(ended_load(v[OH_STORED]), v[OH_RECORD_BYTE]), (v[OH_EVENT] & 7) == 0));
    } else {
        abi_ended(ended_load(0));
    }
    abi_sound_end(score_for_kind_exit_shim(h.score));
    abi_ended(drop_item_when_hit_exit_shim(obj_arm(r->sub)));
    abi_screen_sound_end(play_hit_sound_exit_shim(obj_arm(r->sub2)));
    e = ended_load(v[OH_NUMBER]);
    if (r->arm == OH_TAIL_NO_NUMBER) {
        e = ended_then(e, ended_load(v[OH_FLAGS_N]));
        e = ended_then(e, ended_load(v[OH_KIND1]));
        e = ended_then(e, ended_load(v[OH_KIND2]));
        e = ended_then(e, ended_load(v[OH_STATUS_N]));
        abi_ended(e);
        return;
    }
    e = ended_then(e, ended_compare(v[OH_NUMBER], 7));
    if (r->arm == OH_TAIL_KIND) {
        e = ended_then(e, ended_of_sum(r->sum2));
    }
    abi_ended(e);
}

void abi_ObjectHit(void)
{
    object_hit_exit_shim(ObjectHit(cpu.x, cpu.y));
}
