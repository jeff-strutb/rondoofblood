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

/* CountDownThenRestartMusicEnd: the exit before the track call, the call, and the sound call with what follows it (built by l8_restart_music_exit_shim). */
typedef struct {
    Ended pre;
    bool played;
    TrackPlay track;
    SoundEnd tail;
} RestartEnd;

/* CountDownThenRestartMusicAndScript: the timer, and when it ran out the state count, the track call, the bytes ORed in, the sound call, the script start and the X pulled. */
typedef struct {
    bool restarted;
    uint8_t timer, count, flags, hit, pulled;
    TrackPlay track;
    CommandCall call;
    SoleSlot sole;
} L8RestartArm;

/* The arms ThrownPieceThenScatterAndLeave ends on. */
enum { ST23_THROWN_LOADED, ST23_THROWN_FALLING, ST23_THROWN_LANDED, ST23_THROWN_LATER };

/*
 * @routine 0F6B:488 CountDownThenRestartMusicAndScript
 * @run 70:B488
 * WHAT IT DOES: Counts OBJ_3510 down; at zero clears OBJ_34B8, sets OBJ_38D8 1, OBJ_35C0 and OBJ_3568 $E0 and OBJ_3510 $40, goes to the next state, clears bits 3 and 4 of zField8E, plays the track again (CdPlayAgain), sets bit 2 of zField8E and bit 1 of OBJ_3250, plays sound $D1 and starts script 1 in the only free script slot 3.
 * HOW: A memory decrement, stores, RMWs of zField8E, CdPlayAgain, SoundCommand, StartSoleScriptSlot between a PHX and a PLX.
 * WAITS: no. (CdPlayAgain starts a disc command; it does not wait.)
 * Entry: X object, Y kept for the track and the sound. Exit: as the last instruction that ran (l8_restart_music_exit_shim).
 */
static L8RestartArm count_down_then_restart_music(uint8_t slot, uint8_t y_entry)
{
    L8RestartArm r = { 0 };

    r.timer = DEC8(OBJ->f3510[slot]);
    if (r.timer != 0) {
        r.restarted = false;
        return r;
    }
    WR8(OBJ->f34b8[slot], 0);
    WR8(OBJ->f38d8[slot], 1);
    WR8(OBJ->f35c0[slot], 0xE0);
    WR8(OBJ->f3568[slot], 0xE0);
    WR8(OBJ->f3510[slot], 0x40);
    r.count = INC8(OBJ->state[slot]);
    RMW8(ZP->field8e, old, old & 0xF7);
    RMW8(ZP->field8e, old, old & 0xEF);
    jsr_push(0xB4AC);
    r.track = CdPlayAgain(slot, y_entry);
    jsr_pop();
    r.flags = RD8(ZP->field8e);
    r.flags = (uint8_t)(r.flags | 0x04);
    WR8(ZP->field8e, r.flags);
    r.hit = RD8(OBJ->hitbox_on[slot]);
    r.hit = (uint8_t)(0x02 | r.hit);
    WR8(OBJ->hitbox_on[slot], r.hit);
    jsr_push(0xB4BF);
    r.call = SoundCommand(0xD1, slot, y_entry);
    jsr_pop();
    push8(slot);
    jsr_push(0xB4C7);
    r.sole = StartSoleScriptSlot(1, 3);
    jsr_pop();
    r.pulled = pull8();
    r.restarted = true;
    return r;
}

/* What CountDownThenRestartMusicAndScript leaves, the track and sound calls included, from the arm it ended on. */
static RestartEnd l8_restart_music_exit_shim(L8RestartArm a)
{
    RestartEnd r;
    Ended e;

    r.played = false;
    r.tail = l8_sound_none(ended_kept());
    r.pre = ended_nz(a.timer);
    if (!a.restarted) {
        return r;
    }
    r.pre = then(r.pre, ended_load(0x40));
    r.pre = then(r.pre, ended_nz(a.count));
    r.track = a.track;
    r.played = true;
    e = ended_load(a.flags);
    e = then(e, ended_load(a.hit));
    r.tail.sounded = true;
    r.tail.command = 0xD1;
    r.tail.before = then(e, ended_load(0xD1));
    r.tail.call = a.call;
    e = ended_with_y(ended_with_carry(ended_with_a(ended_nz(a.pulled), a.sole.started ? a.sole.script : a.sole.busy_kind), a.sole.started), a.sole.script);
    r.tail.end = e;
    return r;
}

void abi_CountDownThenRestartMusicAndScript(void)
{
    RestartEnd r = l8_restart_music_exit_shim(count_down_then_restart_music(cpu.x, cpu.y));

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
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_thrown_piece_exit_shim).
 */
L8ThrownArm ThrownPieceThenScatterAndLeave(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, number, value, kind;
    AxisMove move;
    RandomResult random;
    L8ThrownArm r = { 0 };

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        jsr_push(0xB77E);
        r.record = l8_record(slot);
        jsr_pop();
        WR8(OBJ->vx_hi[slot], 8);
        jsr_push(0xB786);
        r.move1 = ApplyVelocityX(slot);
        jsr_pop();
        WR8(OBJ->vy_hi[slot], 0xF8);
        jsr_push(0xB78E);
        r.move2 = ApplyVelocityY(slot);
        jsr_pop();
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->f34b8[slot], 3);
        r.arm = ST23_THROWN_LOADED;
        return r;
    }
    if (r.state == 1) {
        jsr_push(0xB79D);
        r.move1 = ApplyVelocityY(slot);
        jsr_pop();
        jsr_push(0xB7A0);
        r.gravity = ApplyGravity(slot);
        jsr_pop();
        r.ground = RD8(OBJ->y_lo[slot]);
        if (r.ground < 0xD0) {
            r.step = StepListByNumberTable(slot, y);
            r.arm = ST23_THROWN_FALLING;
            return r;
        }
        WR8(OBJ->y_lo[slot], 0xD0);
        WR8(OBJ->hitbox_half_w[slot], 8);
        WR8(OBJ->hitbox_half_h[slot], 8);
        WR8(ZP->temp00, 0);
        for (;;) {
            jsr_push(0xB7BE);
            r.finds[r.passes] = l8_free_slot30();
            jsr_pop();
            r.passes++;
            if (!r.finds[r.passes - 1].found) {
                break;
            }
            jsr_push(0xB7C5);
            r.placed[r.passes - 1] = StartObjectAtPlace(0xAC, slot, r.finds[r.passes - 1].slot);
            jsr_pop();
            number = RD8(ZP->temp00);
            WR8(OBJ->f3040[r.finds[r.passes - 1].slot], number);
            number = (uint8_t)(number + 1);
            r.next[r.passes - 1] = number;
            if (number == 4) {
                break;
            }
            WR8(ZP->temp00, number);
        }
        y = r.finds[r.passes - 1].slot;
        jsr_push(0xB7D8);
        r.call = SoundCommand(0x26, slot, y);
        jsr_pop();
        r.count1 = INC8(OBJ->state[slot]);
        r.count2 = INC8(OBJ->f34b8[slot]);
        jsr_push(0xB7E1);
        r.step = ClearAnimationThenStepListByNumber(slot, y);
        jsr_pop();
        r.arm = ST23_THROWN_LANDED;
        return r;
    }
    r.wave = RD8(ZP->wave_counter);
    if ((r.wave & 0x03) == 0) {
        for (y = 0x50; y != 0x55; y++) {
            kind = RD8(OBJ->kind[y]);
            r.kinds[r.passes] = kind;
            r.passes++;
            if (kind != 0) {
                continue;
            }
            jsr_push(0xB7F1);
            ClearObjectInY(y);
            jsr_pop();
            jsr_push(0xB7F6);
            StartObjectAtPlace(0x9B, slot, y);
            jsr_pop();
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
            r.move1 = ApplyVelocityY(y);
            jsr_pop();
            (void)move;
            r.spawned = true;
            r.spawn_y = y;
            break;
        }
    }
    jsr_push(0xB81E);
    r.step = StepListByNumberTable(slot, y);
    jsr_pop();
    if (r.step.step == FRAME_LIST_ENDED) {
        ClearObject(slot);
    }
    r.arm = ST23_THROWN_LATER;
    return r;
}

/* What ThrownPieceThenScatterAndLeave leaves, the sound call's included, from the arm it ended on (each pass of its loops replayed). */
static SoundEnd l8_thrown_piece_exit_shim(L8ThrownArm a)
{
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e = ended_load(a.state);
    uint8_t i, y;

    if (a.arm == ST23_THROWN_LOADED) {
        e = then(e, l8_record_ended(a.record));
        e = then(e, ended_load(8));
        e = then(e, ended_of_axis_move(a.move1));
        e = then(e, ended_load(0xF8));
        e = then(e, ended_of_axis_move(a.move2));
        r.end = then(e, ended_load(3));
        return r;
    }
    e = then(e, ended_load((uint8_t)(a.state - 1)));
    if (a.arm == ST23_THROWN_FALLING || a.arm == ST23_THROWN_LANDED) {
        e = then(e, ended_of_axis_move(a.move1));
        e = then(e, l8_of_gravity(a.gravity));
        e = then(e, ended_compare(a.ground, 0xD0));
        if (a.arm == ST23_THROWN_FALLING) {
            r.end = then(e, l8_number_step_exit_shim(a.step));
            return r;
        }
        e = then(e, ended_load(0));
        for (i = 0; i < a.passes; i++) {
            e = then(e, l8_slot_ended(a.finds[i]));
            if (!a.finds[i].found) {
                break;
            }
            e = then(then(e, ended_load(a.placed[i])), ended_compare(a.next[i], 4));
        }
        r.sounded = true;
        r.command = 0x26;
        r.before = then(e, ended_load(0x26));
        r.call = a.call;
        e = ended_nz(a.count1);
        e = then(e, ended_nz(a.count2));
        e = then(e, l8_number_step_exit_shim(a.step));
        r.end = e;
        return r;
    }
    e = then(e, ended_with_overflow(ended_nz((uint8_t)((a.wave & 0x80) | ((a.wave & 0x03) ? 1 : 0))), (a.wave & 0x40) != 0));
    for (i = 0; i < a.passes; i++) {
        y = (uint8_t)(0x50 + i);
        e = then(e, ended_load(a.kinds[i]));
        if (a.kinds[i] != 0) {
            e = then(e, ended_with_y(ended_nz(0), y));
            e = then(e, l8_compare_y((uint8_t)(y + 1), 0x55));
            continue;
        }
        e = then(e, ended_of_axis_move(a.move1));
        e = ended_with_y(e, a.spawn_y);
    }
    e = then(e, l8_number_step_exit_shim(a.step));
    r.end = e;
    return r;
}

void abi_ThrownPieceThenScatterAndLeave(void)
{
    abi_sound_end(l8_thrown_piece_exit_shim(ThrownPieceThenScatterAndLeave(cpu.x, cpu.y)));
}
