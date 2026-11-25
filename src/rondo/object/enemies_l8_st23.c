/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 0F6B, third part (enemy kinds not yet identified), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/* CountDownThenRestartMusicEnd: the exit before the track call, the call, and the sound call with what follows it. */
typedef struct {
    Ended pre;
    bool played;
    TrackPlay track;
    SoundEnd tail;
} RestartEnd;

/*
 * @routine 0F6B:488 CountDownThenRestartMusicAndScript
 * @run 70:B488
 * WHAT IT DOES: Counts OBJ_3510 down; at zero clears OBJ_34B8, sets OBJ_38D8 1, OBJ_35C0 and OBJ_3568 $E0 and OBJ_3510 $40, goes to the next state, clears bits 3 and 4 of zField8E, plays the track again (CdPlayAgain), sets bit 2 of zField8E and bit 1 of OBJ_3250, plays sound $D1 and starts script 1 in the only free script slot 3.
 * HOW: A memory decrement, stores, RMWs of zField8E, CdPlayAgain, SoundCommand, StartSoleScriptSlot between a PHX and a PLX.
 * WAITS: no. (CdPlayAgain starts a disc command; it does not wait.)
 * Entry: X object, Y kept for the track and the sound. Exit: as the last instruction that ran.
 */
static RestartEnd count_down_then_restart_music(uint8_t slot, uint8_t y_entry)
{
    uint8_t timer, flags, hit;
    SoleSlot sole;
    RestartEnd r;
    Ended e;

    r.played = false;
    r.tail = l8_sound_none(ended_kept());
    timer = DEC8(OBJ->f3510[slot]);
    r.pre = ended_nz(timer);
    if (timer != 0) {
        return r;
    }
    WR8(OBJ->f34b8[slot], 0);
    WR8(OBJ->f38d8[slot], 1);
    WR8(OBJ->f35c0[slot], 0xE0);
    WR8(OBJ->f3568[slot], 0xE0);
    WR8(OBJ->f3510[slot], 0x40);
    r.pre = then(r.pre, ended_load(0x40));
    r.pre = then(r.pre, ended_nz(INC8(OBJ->state[slot])));
    RMW8(ZP->field8e, old, old & 0xF7);
    RMW8(ZP->field8e, old, old & 0xEF);
    jsr_push(0xB4AC);
    r.track = CdPlayAgain(slot, y_entry);
    jsr_pop();
    r.played = true;
    flags = RD8(ZP->field8e);
    flags = (uint8_t)(flags | 0x04);
    WR8(ZP->field8e, flags);
    e = ended_load(flags);
    hit = RD8(OBJ->hitbox_on[slot]);
    hit = (uint8_t)(0x02 | hit);
    WR8(OBJ->hitbox_on[slot], hit);
    e = then(e, ended_load(hit));
    r.tail.sounded = true;
    r.tail.command = 0xD1;
    r.tail.before = then(e, ended_load(0xD1));
    jsr_push(0xB4BF);
    r.tail.call = SoundCommand(0xD1, slot, y_entry);
    jsr_pop();
    push8(slot);
    jsr_push(0xB4C7);
    sole = StartSoleScriptSlot(1, 3);
    jsr_pop();
    slot = pull8();
    e = ended_with_y(ended_with_carry(ended_with_a(ended_nz(slot), sole.started ? sole.script : sole.busy_kind), sole.started), sole.script);
    r.tail.end = e;
    return r;
}

void abi_CountDownThenRestartMusicAndScript(void)
{
    RestartEnd r = count_down_then_restart_music(cpu.x, cpu.y);

    abi_ended(r.pre);
    if (r.played) {
        play_track_exit_shim(r.track);
    }
    abi_sound_end(r.tail);
}

/*
 * @routine 0F6B:776 ThrownPieceThenScatterAndLeave
 * @run 70:B776
 * WHAT IT DOES: In state 0 loads the record, sets the speed X $0800 and Y -$0800 (moved once each) and OBJ_34B8 3. In state 1 it falls (ApplyVelocityY, ApplyGravity) and steps the list by OBJ_34B8 until it reaches Y $D0; there it stops, shrinks its hit box to 8 by 8, starts four objects of type $AC numbered 0 to 3 in free slots of $30 to $3F, plays sound $26, goes to the next state and restarts the animation. In later states, every fourth frame, it starts an object of type $9B in the first empty slot of $50 to $54 with a random speed, steps the list and clears itself when the list ends.
 * HOW: LoadObjectRecord, ApplyVelocityX and Y, ApplyGravity, FindFreeSlotFrom30, StartObjectAtPlace, SoundCommand, NextRandom, StepListByNumberTable, ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
SoundEnd ThrownPieceThenScatterAndLeave(uint8_t slot, uint8_t y_entry)
{
    uint8_t state, last, nz, y = y_entry, ground, number, found_slot, placed, value, wave, kind, sub_y;
    bool carry, found;
    AxisMove move;
    GravityResult gravity;
    RandomResult random;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        jsr_push(0xB77E);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        e = then(e, ended_of_record(last, nz, carry));
        WR8(OBJ->vx_hi[slot], 8);
        e = then(e, ended_load(8));
        jsr_push(0xB786);
        move = ApplyVelocityX(slot);
        jsr_pop();
        e = then(e, ended_of_axis_move(move));
        WR8(OBJ->vy_hi[slot], 0xF8);
        e = then(e, ended_load(0xF8));
        jsr_push(0xB78E);
        move = ApplyVelocityY(slot);
        jsr_pop();
        e = then(e, ended_of_axis_move(move));
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->f34b8[slot], 3);
        r.end = then(e, ended_load(3));
        return r;
    }
    e = then(e, ended_load((uint8_t)(state - 1)));
    if (state == 1) {
        jsr_push(0xB79D);
        move = ApplyVelocityY(slot);
        jsr_pop();
        e = then(e, ended_of_axis_move(move));
        jsr_push(0xB7A0);
        gravity = ApplyGravity(slot);
        jsr_pop();
        e = then(e, l8_of_gravity(gravity));
        ground = RD8(OBJ->y_lo[slot]);
        e = then(e, ended_compare(ground, 0xD0));
        if (ground < 0xD0) {
            r.end = then(e, StepListByNumberTable(slot, y));
            return r;
        }
        WR8(OBJ->y_lo[slot], 0xD0);
        WR8(OBJ->hitbox_half_w[slot], 8);
        WR8(OBJ->hitbox_half_h[slot], 8);
        WR8(ZP->temp00, 0);
        e = then(e, ended_load(0));
        for (;;) {
            jsr_push(0xB7BE);
            found_slot = FindFreeSlotFrom30(&last, &found);
            jsr_pop();
            e = then(e, ended_of_find_slot(found_slot, last, found));
            if (!found) {
                break;
            }
            jsr_push(0xB7C5);
            placed = StartObjectAtPlace(0xAC, slot, found_slot);
            jsr_pop();
            number = RD8(ZP->temp00);
            WR8(OBJ->f3040[found_slot], number);
            number = (uint8_t)(number + 1);
            e = then(then(e, ended_load(placed)), ended_compare(number, 4));
            if (number == 4) {
                break;
            }
            WR8(ZP->temp00, number);
        }
        y = e.y_set ? e.y : y;
        r.sounded = true;
        r.command = 0x26;
        r.before = then(e, ended_load(0x26));
        jsr_push(0xB7D8);
        r.call = SoundCommand(0x26, slot, y);
        jsr_pop();
        e = ended_nz(INC8(OBJ->state[slot]));
        e = then(e, ended_nz(INC8(OBJ->f34b8[slot])));
        jsr_push(0xB7E1);
        e = then(e, ClearAnimationThenStepListByNumber(slot, y));
        jsr_pop();
        r.end = e;
        return r;
    }
    wave = RD8(ZP->wave_counter);
    e = then(e, ended_with_overflow(ended_nz((uint8_t)((wave & 0x80) | ((wave & 0x03) ? 1 : 0))), (wave & 0x40) != 0));
    if ((wave & 0x03) == 0) {
        for (y = 0x50; y != 0x55; y++) {
            kind = RD8(OBJ->kind[y]);
            e = then(e, ended_load(kind));
            if (kind != 0) {
                e = then(e, ended_with_y(ended_nz(0), y));
                e = then(e, l8_compare_y((uint8_t)(y + 1), 0x55));
                continue;
            }
            jsr_push(0xB7F1);
            ClearObjectInY(y);
            jsr_pop();
            jsr_push(0xB7F6);
            placed = StartObjectAtPlace(0x9B, slot, y);
            jsr_pop();
            sub_y = slot;
            jsr_push(0xB7FA);
            random = NextRandom();
            jsr_pop();
            value = (uint8_t)((random.value & 0x0F) - 7);
            WR8(OBJ->vx_hi[y], value);
            jsr_push(0xB805);
            move = ApplyVelocityX(y);
            jsr_pop();
            jsr_push(0xB808);
            random = NextRandom();
            jsr_pop();
            value = (uint8_t)((random.value & 0x0F) - 0x0F);
            WR8(OBJ->vy_hi[y], value);
            jsr_push(0xB813);
            move = ApplyVelocityY(y);
            jsr_pop();
            e = then(e, ended_of_axis_move(move));
            e = ended_with_y(e, y);
            (void)placed;
            (void)sub_y;
            break;
        }
    }
    jsr_push(0xB81E);
    e = then(e, StepListByNumberTable(slot, y));
    jsr_pop();
    if (e.a == 0) {
        ClearObject(slot);
    }
    r.end = e;
    return r;
}

void abi_ThrownPieceThenScatterAndLeave(void)
{
    abi_sound_end(ThrownPieceThenScatterAndLeave(cpu.x, cpu.y));
}
