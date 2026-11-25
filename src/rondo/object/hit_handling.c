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
Ended HitDamage(uint8_t slot)
{
    uint8_t row, hit, damage;
    Sum index;
    Ended e;

    row = RD8(OBJ->fields_2ee0[slot]);
    row = (uint8_t)(row << 4);
    e = ended_with_carry(ended_load(row), false);
    WR8(ZP->temp00, row);
    hit = RD8(OBJ->hit_taken[slot]);
    index = add_bytes(hit, RD8(ZP->temp00), false);
    e = ended_then(e, ended_of_sum(index));
    damage = rd8((uint16_t)(0xABA6 + index.value));
    WR8(ZP->temp00, damage);
    return ended_then(e, ended_with_y(ended_load(damage), index.value));
}

void abi_HitDamage(void)
{
    abi_ended(HitDamage(cpu.x));
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
SoundEnd ScoreForKind(uint8_t slot, uint8_t y)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    uint8_t flags, kind, index, changed;
    Sum diff;
    Ended e;

    flags = rd8(HitAttackFlags);
    flags = (uint8_t)(flags & 0x01);
    e = ended_load(flags);
    r.end = e;
    if (flags != 0) {
        return r;
    }
    kind = RD8(OBJ->kind[slot]);
    diff = sub_bytes(kind, 0x20, true);
    index = (uint8_t)(diff.value << 1);
    e = ended_then(ended_of_sum(diff), ended_with_y(ended_with_carry(ended_load(index), (diff.value & 0x80) != 0), index));
    WR8(ZP->temp00, rd8((uint16_t)(0xABF5 + index)));
    WR8(ZP->temp01, rd8((uint16_t)(0xABF6 + index)));
    jsr_push(0xA2BF);
    changed = AddScoreWord();
    jsr_pop();
    e = ended_then(e, ended_with_carry(ended_load(changed), changed != 0));
    r.end = e;
    if (changed == 0) {
        return r;
    }
    jsr_push(0xA2C4);
    r = GiveExtraLife(slot, y);
    jsr_pop();
    r.end = ended_then(e, r.end);
    return r;
}

void abi_ScoreForKind(void)
{
    abi_sound_end(ScoreForKind(cpu.x, cpu.y));
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
ExitChain ObjectHit(uint8_t slot, uint8_t y)
{
    ExitChain c = { { { STEP_ENDED, { 0 }, { { 0 }, { 0 }, false, 0, { 0 } }, { { 0 }, { 0 }, false, { 0 } }, { 0 } } }, 0 };
    uint8_t status, bits, flash, t, attack, flags, hp, hit, damage, state, command, kind;
    uint8_t stored, event, byte, last;
    bool marked, found;
    Sum rest;
    Ended e;
    SoundOnScreen shout;
    ScreenSoundEnd screen;
    SoundEnd score;
    Ended drop;
    uint8_t number;
    bool record_now = false;
    ExitStep st = { STEP_ENDED, { 0 }, { { 0 }, { 0 }, false, 0, { 0 } }, { { 0 }, { 0 }, false, { 0 } }, { 0 } };

    (void)last;
    (void)found;
    status = RD8(OBJ->status_flags[slot]);
    status = (uint8_t)(status & 0x9F);
    WR8(OBJ->status_flags[slot], status);
    e = ended_load(status);
    bits = RD8(OBJ->f31f8[slot]);
    bits = (uint8_t)(bits & 0x03);
    e = ended_then(e, ended_load(bits));
    if (bits != 0) {
        bits = DEC8(OBJ->f31f8[slot]);
        e = ended_then(e, ended_nz(bits));
        bits = RD8(OBJ->f31f8[slot]);
        bits = (uint8_t)(bits & 0x03);
        e = ended_then(e, ended_load(bits));
        if (bits == 0) {
            t = RD8_IDX(0x3AE8, slot);
            WR8(OBJ->flash_flags[slot], t);
            e = ended_then(e, ended_load(t));
        }
    }
    status = RD8(OBJ->status_flags[slot]);
    status = (uint8_t)(status & 0x80);
    e = ended_then(e, ended_load(status));
    if (status != 0) {
        chain_ended(&c, e);
        return c;
    }
    attack = rd8(HitAttackFlags);
    bits = (uint8_t)(attack & 0x01);
    e = ended_then(e, ended_load(bits));
    if (bits != 0) {
        status = RD8(OBJ->status_flags[slot]);
        status = (uint8_t)(status | 0x20);
        WR8(OBJ->status_flags[slot], status);
        e = ended_then(e, ended_load(status));
        flags = RD8(OBJ->f30f0[slot]);
        flags = (uint8_t)(flags & 0x20);
        e = ended_then(e, ended_load(flags));
        record_now = flags != 0;
    }
    if (!record_now) {
        attack = rd8(HitAttackFlags);
        bits = (uint8_t)(attack & 0x02);
        e = ended_then(e, ended_load(bits));
        if (bits != 0) {
            status = RD8(OBJ->status_flags[slot]);
            status = (uint8_t)(status | 0x40);
            WR8(OBJ->status_flags[slot], status);
            e = ended_then(e, ended_load(status));
            flags = RD8(OBJ->f30f0[slot]);
            flags = (uint8_t)(flags & 0x40);
            e = ended_then(e, ended_load(flags));
            record_now = flags != 0;
        }
    }
    if (!record_now) {
        hp = RD8(OBJ->f32a8[slot]);
        e = ended_then(e, ended_load(hp));
        if ((hp & 0x80) != 0) {
            chain_ended(&c, e);
            return c;
        }
        hit = RD8(OBJ->hit_taken[slot]);
        e = ended_then(e, ended_load(hit));
        if (hit == 0) {
            chain_ended(&c, e);
            return c;
        }
        chain_ended(&c, e);
        jsr_push(0xA1E8);
        chain_ended(&c, HitDamage(slot));
        jsr_pop();
        e = ended_kept();
        damage = RD8(ZP->temp00);
        e = ended_then(e, ended_load(damage));
        if (damage == 0) {
            chain_ended(&c, e);
            return c;
        }
        y = RD8(OBJ->hit_taken[slot]);
        e = ended_then(e, ended_with_y(ended_nz(y), y));
        byte = rd8((uint16_t)(0xA26D + y));
        WR8(OBJ->timer[slot], byte);
        e = ended_then(e, ended_load(byte));
        byte = rd8((uint16_t)(0xA27D + y));
        WR8(OBJ->f2f90[slot], byte);
        e = ended_then(e, ended_load(byte));
        t = RD8(OBJ->flash_flags[slot]);
        WR8_IDX(0x3AE8, slot, t);
        WR8(OBJ->flash_flags[slot], 0x86);
        e = ended_then(e, ended_load(0x86));
        flags = RD8(OBJ->f31f8[slot]);
        flags = (uint8_t)(flags | 0x03);
        WR8(OBJ->f31f8[slot], flags);
        e = ended_then(e, ended_load(flags));
        hp = RD8(OBJ->f32a8[slot]);
        damage = RD8(ZP->temp00);
        rest = sub_bytes(hp, damage, true);
        WR8(OBJ->f32a8[slot], rest.value);
        e = ended_then(e, ended_of_sum(rest));
        if (rest.carry) {
            status = RD8(OBJ->status_flags[slot]);
            bits = (uint8_t)((status >> 3) & 0x03);
            e = ended_then(e, ended_with_y(ended_with_carry(ended_load(bits), (status & 0x04) != 0), bits));
            y = bits;
            command = rd8((uint16_t)(0xA269 + y));
            e = ended_then(e, ended_load(command));
            chain_ended(&c, e);
            shout = SoundIfOnFirstScreen(command, slot, y);
            st.c.called = true;
            st.c.sound = shout;
            st.kind = STEP_SCREEN_END;
            c.step[c.count++] = st;
            return c;
        }
    }
    chain_ended(&c, e);
    jsr_push(0xA22C);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    jsr_pop();
    if (marked) {
        y = byte;
        chain_ended(&c, ended_with_carry(ended_with_y(ended_load(stored), byte), (event & 7) == 0));
    } else {
        chain_ended(&c, ended_load(0));
    }
    jsr_push(0xA22F);
    score = ScoreForKind(slot, y);
    jsr_pop();
    chain_sound(&c, score);
    jsr_push(0xA232);
    drop = DropItemWhenHitF(slot);
    jsr_pop();
    chain_ended(&c, drop);
    jsr_push(0xA235);
    screen = PlayHitSoundA(slot);
    jsr_pop();
    chain_screen(&c, screen);
    status = RD8(OBJ->status_flags[slot]);
    number = (uint8_t)(status & 0x07);
    e = ended_load(number);
    if (number == 0) {
        flags = RD8(OBJ->f30f0[slot]);
        flags = (uint8_t)(flags & 0xFB);
        WR8(OBJ->f30f0[slot], flags);
        e = ended_then(e, ended_load(flags));
        kind = RD8(OBJ->f3148[slot]);
        kind = (uint8_t)(kind & 0x87);
        e = ended_then(e, ended_load(kind));
        kind = (uint8_t)(kind | 0x04);
        e = ended_then(e, ended_load(kind));
        WR8(OBJ->f3148[slot], kind);
        status = RD8(OBJ->status_flags[slot]);
        status = (uint8_t)(status | 0x80);
        WR8(OBJ->status_flags[slot], status);
        e = ended_then(e, ended_load(status));
        chain_ended(&c, e);
        return c;
    }
    e = ended_then(e, ended_compare(number, 7));
    if (number == 7) {
        ClearObject(slot);
        chain_ended(&c, e);
        return c;
    }
    rest = add_bytes((uint8_t)(number - 1), 0x9B, false);
    e = ended_then(e, ended_of_sum(rest));
    WR8(OBJ->kind[slot], rest.value);
    WR8(OBJ->state[slot], 0);
    chain_ended(&c, e);
    return c;
}

void abi_ObjectHit(void)
{
    ExitChain c = ObjectHit(cpu.x, cpu.y);

    abi_chain(&c);
}
