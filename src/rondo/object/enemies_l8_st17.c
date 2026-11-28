/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 0B4A (enemy kinds not yet identified), by routine. Bodies return the
 * arm they ended on and the values behind it; each routine's exit shim builds how its last instructions left A, Y and the flags (Ended,
 * enemy_objects.h), and the abi_ wrappers show it.
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

/* The arms the routines end on (L8Arm, or the routine's own struct). */
enum { ST17_RING_NONE_FREE, ST17_RING_DONE };
enum { ST17_FLING_FALLING, ST17_FLING_FLUNG };
enum { ST17_HURT_NONE, ST17_HURT_MANY, ST17_HURT_CLEARED, ST17_HURT_WAIT };
enum { ST17_STEPS_FAR, ST17_STEPS_WAITING, ST17_STEPS_MORE, ST17_STEPS_LAST };
enum { ST17_PULSE_COUNTING, ST17_PULSE_STEPPED, ST17_PULSE_DONE };
enum {
    ST17_WATCH_LOADED, ST17_WATCH_NO_KIND, ST17_WATCH_FRAME, ST17_WATCH_X_LOW, ST17_WATCH_X_HIGH, ST17_WATCH_PLAYER, ST17_WATCH_SEEN,
    ST17_WATCH_FALLING, ST17_WATCH_LANDED, ST17_WATCH_IDLE, ST17_WATCH_DROP_AIR, ST17_WATCH_DROP_DONE
};
enum {
    ST17_BOSS_LOADED, ST17_BOSS_NO_HITS, ST17_BOSS_HURT, ST17_BOSS_RIGHT, ST17_BOSS_CHARGE, ST17_BOSS_STEPPED, ST17_BOSS_LISTED,
    ST17_BOSS_WAITING, ST17_BOSS_TILES_SET, ST17_BOSS_CLEARED, ST17_BOSS_GAP, ST17_BOSS_TILES, ST17_BOSS_FALLING, ST17_BOSS_RECOVERED
};

/* RecordObjectEvent's result. */
typedef struct {
    bool marked;
    uint8_t stored, event, byte;
} L8St17Event;

static Ended l8_st17_event_ended(L8St17Event r)
{
    return l8_of_event(r.marked, r.stored, r.event, r.byte);
}

/*
 * @routine 0B4A:39F StartRingOfPieces
 * @run 70:AB9F
 * WHAT IT DOES: For the numbers 0 to $17 starts an object of type $28 in a free slot of $20 to $3F at the object's place and gives it the seven bytes of the eight byte row of the table at $ABE8: OBJ_WORK_D, X speed high, Y speed high, OBJ_WORK_F, OBJ_WORK_G, OBJ_WORK_H and OBJ_WORK_I; it stops when no slot is free.
 * HOW: A loop of FindFreeSlotFrom20, StartObjectAtPlace, a table read by the number times 8, stores. zD9 and zDA keep their address form until they have names.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_ring_of_pieces_exit_shim).
 */
L8St17RingArm StartRingOfPieces(uint8_t slot)
{
    uint8_t number = 0, row, placed, value = 0;
    L8St17RingArm r = { 0 };

    for (;;) {
        wr8(0x20DA, number);
        jsr_push(0xABA5);
        r.found.slot = FindFreeSlotFrom20(&r.found.last, &r.found.found);
        jsr_pop();
        if (!r.found.found) {
            r.arm = ST17_RING_NONE_FREE;
            return r;
        }
        jsr_push(0xABAC);
        placed = StartObjectAtPlace(0x28, slot, r.found.slot);
        jsr_pop();
        wr8(0x20D9, slot);
        row = (uint8_t)(rd8(0x20DA) << 3);
        value = rd8((uint16_t)(0xABE8 + row));
        WR8(OBJ->f34b8[r.found.slot], value);
        value = rd8((uint16_t)(0xABE9 + row));
        WR8(OBJ->vx_hi[r.found.slot], value);
        value = rd8((uint16_t)(0xABEA + row));
        WR8(OBJ->vy_hi[r.found.slot], value);
        value = rd8((uint16_t)(0xABEB + row));
        WR8(OBJ->f3568[r.found.slot], value);
        value = rd8((uint16_t)(0xABEC + row));
        WR8(OBJ->f35c0[r.found.slot], value);
        value = rd8((uint16_t)(0xABED + row));
        WR8(OBJ->work_h[r.found.slot], value);
        value = rd8((uint16_t)(0xABEE + row));
        WR8(OBJ->work_i[r.found.slot], value);
        slot = rd8(0x20D9);
        number = rd8(0x20DA);
        number++;
        (void)placed;
        if (number >= 0x18) {
            r.number = number;
            r.value = value;
            r.arm = ST17_RING_DONE;
            return r;
        }
    }
}

/* What StartRingOfPieces leaves in A, Y and the flags, from the arm it ended on (the last pass of the loop). */
Ended l8_ring_of_pieces_exit_shim(L8St17RingArm r)
{
    if (r.arm == ST17_RING_NONE_FREE) {
        return l8_slot_ended(r.found);
    }
    return ended_with_y(ended_with_a(l8_compare_y(r.number, 0x18), r.value), r.number);
}

void abi_StartRingOfPieces(void)
{
    abi_ended(l8_ring_of_pieces_exit_shim(StartRingOfPieces(cpu.x)));
}

/*
 * @routine 0B4A:4A8 LoadRecordFlingThenFall
 * @run 70:ACA8
 * WHAT IT DOES: In state 0 loads the record, moves the object once and takes its speed from OBJ_WORK_F, OBJ_WORK_G, OBJ_WORK_H and OBJ_WORK_I, and its frame from OBJ_WORK_D plus $12. Later it falls (ApplyGravityUnlimited) and moves.
 * HOW: LoadObjectRecord, ApplyVelocity, four copies, an add, ApplyGravityUnlimited, a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_fling_exit_shim).
 */
L8Arm LoadRecordFlingThenFall(uint8_t slot)
{
    uint8_t value;
    GravityResult gravity;
    L8Arm r = { 0 };

    r.load = l8_load_new(slot, 0xACB0);
    if (r.load.state != 0) {
        jsr_push(0xACD8);
        gravity = ApplyGravityUnlimited(slot);
        jsr_pop();
        (void)gravity;
        r.move = ApplyVelocity(slot);
        r.arm = ST17_FLING_FALLING;
        return r;
    }
    jsr_push(0xACB3);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    value = RD8(OBJ->f3568[slot]);
    WR8(OBJ->vx_hi[slot], value);
    value = RD8(OBJ->f35c0[slot]);
    WR8(OBJ->vx_lo[slot], value);
    value = RD8(OBJ->work_h[slot]);
    WR8(OBJ->vy_hi[slot], value);
    value = RD8(OBJ->work_i[slot]);
    WR8(OBJ->vy_lo[slot], value);
    value = RD8(OBJ->f34b8[slot]);
    r.sum = add_bytes(value, 0x12, false);
    WR8(OBJ->frame[slot], r.sum.value);
    r.arm = ST17_FLING_FLUNG;
    return r;
}

/* What LoadRecordFlingThenFall leaves in A, Y and the flags, from the arm it ended on (load the state and record, move the move, sum the frame add). */
static Ended l8_fling_exit_shim(L8Arm r)
{
    Ended e = l8_load_ended(r.load);

    if (r.arm == ST17_FLING_FALLING) {
        return ended_of_axis_move(r.move);
    }
    e = then(e, ended_of_axis_move(r.move));
    return then(e, ended_of_sum(r.sum));
}

void abi_LoadRecordFlingThenFall(void)
{
    abi_ended(l8_fling_exit_shim(LoadRecordFlingThenFall(cpu.x)));
}

/*
 * @routine 0B4A:4F4 LoadRecordFacingLeftRising
 * @run 70:ACF4
 * WHAT IT DOES: Loads the object's record, sets its facing 1, frame $15 and Y speed high $F0.
 * HOW: LoadObjectRecord and three stores.
 * WAITS: no.
 * Entry: X object. Exit: A $F0, Y 11, C from the record (l8_facing_rising_exit_shim).
 */
L8Record LoadRecordFacingLeftRising(uint8_t slot)
{
    L8Record r;

    jsr_push(0xACF7);
    r = l8_record(slot);
    jsr_pop();
    WR8(OBJ->facing_left[slot], 1);
    WR8(OBJ->frame[slot], 0x15);
    WR8(OBJ->vy_hi[slot], 0xF0);
    return r;
}

/* What LoadRecordFacingLeftRising leaves: the record's exit and A $F0. */
static Ended l8_facing_rising_exit_shim(L8Record r)
{
    return then(l8_record_ended(r), ended_load(0xF0));
}

void abi_LoadRecordFacingLeftRising(void)
{
    abi_ended(l8_facing_rising_exit_shim(LoadRecordFacingLeftRising(cpu.x)));
}

/*
 * @routine 0B4A:507 SoundBAAtScreen2
 * @run 70:AD07
 * WHAT IT DOES: When the scroll's screen byte is 2 plays sound $BA and goes to the next state.
 * HOW: A compare, SoundCommand, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_sound_ba_exit_shim).
 */
L8St17ScreenArm SoundBAAtScreen2(uint8_t slot, uint8_t y_entry)
{
    L8St17ScreenArm r = { 0 };

    r.screen = RD8(SCROLLE->x_hi_screen[0]);
    if (r.screen != 2) {
        r.sounded = false;
        return r;
    }
    jsr_push(0xAD13);
    r.call = SoundCommand(0xBA, slot, y_entry);
    jsr_pop();
    r.count = INC8(OBJ->state[slot]);
    r.sounded = true;
    return r;
}

/* What SoundBAAtScreen2 leaves, the sound call's included, from the arm it ended on. */
static SoundEnd l8_sound_ba_exit_shim(L8St17ScreenArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_compare(r.screen, 2);

    if (!r.sounded) {
        s.end = e;
        return s;
    }
    s.sounded = true;
    s.command = 0xBA;
    s.before = then(e, ended_load(0xBA));
    s.call = r.call;
    s.end = ended_nz(r.count);
    return s;
}

void abi_SoundBAAtScreen2(void)
{
    abi_sound_end(l8_sound_ba_exit_shim(SoundBAAtScreen2(cpu.x, cpu.y)));
}

/*
 * @routine 0B4A:58B LoadRecordThenDrawTileScript2
 * @run 70:AD8B
 * WHAT IT DOES: When bit 0 of zField92 is set the object is cleared; otherwise it loads its record, sets its hit offset OBJ_HITBOX_DY $E8 and draws tile script 2.
 * HOW: A test, a tail jump into ClearObject, LoadObjectRecord, DrawTileScript.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTileScript, or the test when the object is cleared (l8_draw_tile2_exit_shim). The jump into ClearObject ran in no script.
 */
L8St17DrawArm LoadRecordThenDrawTileScript2(uint8_t slot)
{
    L8St17DrawArm r = { 0 };

    r.flag = RD8(ZP->field92);
    if ((r.flag & 0x01) != 0) {
        ClearObject(slot);
        r.cleared = true;
        return r;
    }
    jsr_push(0xAD97);
    r.record = l8_record(slot);
    jsr_pop();
    WR8(OBJ->hitbox_dy[slot], 0xE8);
    jsr_push(0xADA2);
    r.run = DrawTileScript(2, 0);
    jsr_pop();
    return r;
}

/* What LoadRecordThenDrawTileScript2 leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_draw_tile2_exit_shim(L8St17DrawArm r)
{
    if (r.cleared) {
        return ended_load((uint8_t)(r.flag & 0x01));
    }
    return then(l8_record_ended(r.record), ended_of_tile_run(r.run, 2));
}

void abi_LoadRecordThenDrawTileScript2(void)
{
    abi_ended(l8_draw_tile2_exit_shim(LoadRecordThenDrawTileScript2(cpu.x)));
}

/*
 * @routine 0B4A:608 ScatterPiecesDownward
 * @run 70:AE08
 * WHAT IT DOES: Moves the object down by its hit offset OBJ_HITBOX_DY, starts up to 8 objects of type $9A in free slots of $30 to $3F at its place, each with an X speed from the table at $AE4E by the parity of the count, moved once, OBJ_WORK_D 1 and OBJ_WORK_E the count less 1, then moves the object back.
 * HOW: An add and a subtract of OBJ_HITBOX_DY on OBJ_Y_LO, a loop of FindFreeSlotFrom30, StartObjectAtPlace and ApplyVelocityX with X and Y swapped, a counter in zTemp00. zD9 keeps its address form until it has a name.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_scatter_exit_shim).
 */
L8St17ScatterArm ScatterPiecesDownward(uint8_t slot)
{
    uint8_t value, offset, count;
    L8St17ScatterPass *pass;
    L8St17ScatterArm r = { 0 };

    value = RD8(OBJ->y_lo[slot]);
    offset = RD8(OBJ->hitbox_dy[slot]);
    r.first = add_bytes(value, offset, false);
    WR8(OBJ->y_lo[slot], r.first.value);
    WR8(ZP->temp00, 8);
    for (;;) {
        pass = &r.pass[r.passes++];
        jsr_push(0xAE19);
        pass->found.slot = FindFreeSlotFrom30(&pass->found.last, &pass->found.found);
        jsr_pop();
        if (!pass->found.found) {
            break;
        }
        jsr_push(0xAE20);
        pass->placed = StartObjectAtPlace(0x9A, slot, pass->found.slot);
        jsr_pop();
        wr8(0x20D9, slot);
        count = RD8(ZP->temp00);
        pass->parity = count & 0x01;
        pass->speed = rd8((uint16_t)(0xAE4E + pass->parity));
        WR8(OBJ->vx_hi[pass->found.slot], pass->speed);
        jsr_push(0xAE31);
        pass->move = ApplyVelocityX(pass->found.slot);
        jsr_pop();
        slot = rd8(0x20D9);
        pass->slot = slot;
        WR8(OBJ->f34b8[pass->found.slot], 1);
        value = RD8(ZP->temp00);
        value--;
        WR8(OBJ->work_e[pass->found.slot], value);
        pass->left = value;
        pass->dec = DEC8(ZP->temp00);
        if (pass->dec == 0) {
            break;
        }
    }
    value = RD8(OBJ->y_lo[slot]);
    offset = RD8(OBJ->hitbox_dy[slot]);
    r.last = sub_bytes(value, offset, true);
    WR8(OBJ->y_lo[slot], r.last.value);
    return r;
}

/* What ScatterPiecesDownward leaves in A, Y and the flags (each pass of the loop replayed; the last may have stopped at its slot search). */
static Ended l8_scatter_exit_shim(L8St17ScatterArm r)
{
    Ended e = ended_of_sum(r.first);
    unsigned i;

    e = then(e, ended_load(8));
    for (i = 0; i < r.passes; i++) {
        const L8St17ScatterPass *pass = &r.pass[i];

        e = then(e, l8_slot_ended(pass->found));
        if (!pass->found.found) {
            break;
        }
        e = then(e, ended_load(pass->placed));
        e = then(e, ended_nz(pass->parity));
        e = then(e, ended_load(pass->speed));
        e = then(e, ended_of_axis_move(pass->move));
        e = then(e, ended_nz(pass->slot));
        e = then(e, ended_load(pass->left));
        e = then(e, ended_nz(pass->dec));
    }
    return then(e, ended_of_sum(r.last));
}

void abi_ScatterPiecesDownward(void)
{
    abi_ended(l8_scatter_exit_shim(ScatterPiecesDownward(cpu.x)));
}

/*
 * @routine 0B4A:650 ClearWhenHurtThenWait
 * @run 70:AE50
 * WHAT IT DOES: When the object has taken 1 to 4 hits: if the player's health zPlayerHealth is $5C plays sound $0B, becomes kind $9F with OBJ_WORK_D 3 and state 0; otherwise sets the health $5C and clears itself.
 * HOW: Two compares, a compare of the health, SoundCommand, stores, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_clear_hurt_exit_shim). The arm for health $5C ran in no script; the draft stops there.
 */
L8St17HurtArm ClearWhenHurtThenWait(uint8_t slot, uint8_t y_entry)
{
    L8St17HurtArm r = { 0 };

    r.hits = RD8(OBJ->hit_taken[slot]);
    if (r.hits == 0) {
        r.arm = ST17_HURT_NONE;
        return r;
    }
    if (r.hits >= 5) {
        r.arm = ST17_HURT_MANY;
        return r;
    }
    r.health = RD8(ZP->player_health);
    if (r.health != 0x5C) {
        WR8(ZP->player_health, 0x5C);
        ClearObject(slot);
        r.arm = ST17_HURT_CLEARED;
        return r;
    }
    jsr_push(0xAE64);
    r.call = SoundCommand(0x0B, slot, y_entry);
    jsr_pop();
    WR8(OBJ->kind[slot], 0x9F);
    WR8(OBJ->f34b8[slot], 3);
    WR8(OBJ->state[slot], 0);
    r.arm = ST17_HURT_WAIT;
    return r;
}

/* What ClearWhenHurtThenWait leaves, the sound call's included, from the arm it ended on. */
static SoundEnd l8_clear_hurt_exit_shim(L8St17HurtArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_load(r.hits);

    if (r.arm == ST17_HURT_NONE) {
        s.end = e;
        return s;
    }
    e = then(e, ended_compare(r.hits, 5));
    if (r.arm == ST17_HURT_MANY) {
        s.end = e;
        return s;
    }
    e = then(e, ended_compare(r.health, 0x5C));
    if (r.arm == ST17_HURT_CLEARED) {
        s.end = then(e, ended_load(0x5C));
        return s;
    }
    s.sounded = true;
    s.command = 0x0B;
    s.before = then(e, ended_load(0x0B));
    s.call = r.call;
    s.end = ended_load(3);
    return s;
}

void abi_ClearWhenHurtThenWait(void)
{
    abi_sound_end(l8_clear_hurt_exit_shim(ClearWhenHurtThenWait(cpu.x, cpu.y)));
}

/*
 * @routine 0B4A:5A3 DrawTileScriptStepsThenScatter
 * @run 70:ADA3
 * WHAT IT DOES: When the object's X high byte is not 0 draws tile script $02 plus OBJ_WORK_D. Otherwise, at 5 hits taken, it sets OBJ_INVULN_TIMER $18, draws the next tile script, plays sound $19, scatters pieces (ScatterPiecesDownward) and advances OBJ_WORK_D; below 3 steps it takes OBJ_HITBOX_DY and the frame from the table at $AE02; at 3 it sets zField92 1, clears OBJ_HITBOX_DY, sets the hit box $10 by $10 and frame $17 and goes to the next state.
 * HOW: DrawTileScript, SoundCommand, ScatterPiecesDownward, table reads, stores.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_steps_scatter_exit_shim).
 */
L8St17StepsArm DrawTileScriptStepsThenScatter(uint8_t slot, uint8_t y_entry)
{
    uint8_t value;
    L8St17StepsArm r = { 0 };

    (void)y_entry;
    r.high = RD8(OBJ->x_hi[slot]);
    if (r.high != 0) {
        r.steps = RD8(OBJ->f34b8[slot]);
        r.sum = add_bytes(2, r.steps, false);
        r.item = r.sum.value;
        r.run = DrawTileScript(r.item, 0);
        r.arm = ST17_STEPS_FAR;
        return r;
    }
    r.hits = RD8(OBJ->hit_taken[slot]);
    if (r.hits != 5) {
        r.arm = ST17_STEPS_WAITING;
        return r;
    }
    WR8(OBJ->invuln_timer[slot], 0x18);
    r.steps = RD8(OBJ->f34b8[slot]);
    r.sum = add_bytes(2, r.steps, false);
    r.item = (uint8_t)(r.sum.value + 1);
    jsr_push(0xADC9);
    r.run = DrawTileScript(r.item, 0);
    jsr_pop();
    jsr_push(0xADCE);
    r.call = SoundCommand(0x19, slot, 0);
    jsr_pop();
    jsr_push(0xADD1);
    r.scatter = ScatterPiecesDownward(slot);
    jsr_pop();
    r.count = INC8(OBJ->f34b8[slot]);
    r.after = RD8(OBJ->f34b8[slot]);
    if (r.after >= 3) {
        WR8(ZP->field92, 1);
        WR8(OBJ->hitbox_dy[slot], 0);
        WR8(OBJ->hitbox_half_w[slot], 0x10);
        WR8(OBJ->hitbox_half_h[slot], 0x10);
        WR8(OBJ->frame[slot], 0x17);
        r.state_inc = INC8(OBJ->state[slot]);
        r.arm = ST17_STEPS_LAST;
        return r;
    }
    r.index = (uint8_t)(r.after << 1);
    value = rd8((uint16_t)(0xAE02 + r.index));
    WR8(OBJ->hitbox_dy[slot], value);
    r.value = rd8((uint16_t)(0xAE03 + r.index));
    WR8(OBJ->frame[slot], r.value);
    r.arm = ST17_STEPS_MORE;
    return r;
}

/* What DrawTileScriptStepsThenScatter leaves, the sound call's included, from the arm it ended on. */
static SoundEnd l8_steps_scatter_exit_shim(L8St17StepsArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_load(r.high);

    if (r.arm == ST17_STEPS_FAR) {
        e = then(e, ended_of_sum(r.sum));
        s.end = then(e, ended_of_tile_run(r.run, r.item));
        return s;
    }
    e = then(e, ended_compare(r.hits, 5));
    if (r.arm == ST17_STEPS_WAITING) {
        s.end = e;
        return s;
    }
    e = then(then(e, ended_of_sum(r.sum)), ended_nz(r.item));
    e = then(e, ended_of_tile_run(r.run, r.item));
    s.sounded = true;
    s.command = 0x19;
    s.before = then(e, ended_load(0x19));
    s.call = r.call;
    e = l8_scatter_exit_shim(r.scatter);
    e = then(e, ended_nz(r.count));
    e = then(e, ended_load(r.after));
    e = then(e, ended_compare(r.after, 3));
    if (r.arm == ST17_STEPS_LAST) {
        e = then(e, ended_load(0x17));
        s.end = then(e, ended_nz(r.state_inc));
        return s;
    }
    e = then(e, ended_with_y(ended_with_carry(ended_load(r.index), false), r.index));
    s.end = then(e, ended_load(r.value));
    return s;
}

void abi_DrawTileScriptStepsThenScatter(void)
{
    abi_sound_end(l8_steps_scatter_exit_shim(DrawTileScriptStepsThenScatter(cpu.x, cpu.y)));
}

/* The leaks of two sound calls in one routine: the first (with what follows it up to the second), the second's call, and the rest. */
typedef struct {
    SoundEnd first;
    bool second;
    CommandCall call2;
    Ended end;
} DoubleSound;

static void abi_double_sound(DoubleSound t)
{
    abi_sound_end(t.first);
    if (t.second) {
        sound_command_exit_shim(0x19, t.call2);
    }
    abi_ended(t.end);
}

/* PulseWaterLevelAndScatter: the number handed on and what MaybeSound19AndSpawnPiece returned, the count after the DEC, the second sound call, the move and the counts of the wrap, the tile run and event of the end, the wave bits and table bytes. */
typedef struct {
    int arm;
    uint8_t number, count, inc, steps, wave, index, value_a, value_b;
    L8SpawnPieceArm piece;
    CommandCall call2;
    AxisMove move;
    TileRun run;
    L8St17Event event;
} L8St17PulseArm;

/*
 * @routine 0B4A:517 PulseWaterLevelAndScatter
 * @run 70:AD17
 * WHAT IT DOES: Sets slot 0's OBJ_WORK_E 3, OBJ_WORK_F 0 and the byte at $3BF0 to 2; hands OBJ_WORK_E of this object to MaybeSound19AndSpawnPiece and counts it down. When it goes below 0 it sets it $10, plays sound $19, moves in Y and advances OBJ_WORK_D; at 4 it draws tile script 1, clears the water level, records the object's event and clears itself. Otherwise the water level takes a pair of bytes from the table at $AD6B by the wave counter.
 * HOW: MaybeSound19AndSpawnPiece, SoundCommand, ApplyVelocityY, DrawTileScript, RecordObjectEvent, ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept for the sounds. Exit: as the last instruction that ran (l8_pulse_exit_shim). The byte at $3BF0 keeps its address form until it has a name.
 */
static L8St17PulseArm pulse_water_level_and_scatter(uint8_t slot, uint8_t y_entry)
{
    uint8_t y;
    L8St17PulseArm r = { 0 };

    wr8(0x3BF0, 0x02);
    WR8(OBJ->work_e[0], 3);
    WR8(OBJ->f3568[0], 0);
    r.number = RD8(OBJ->work_e[slot]);
    jsr_push(0xAD2A);
    r.piece = MaybeSound19AndSpawnPiece(slot, r.number, y_entry);
    jsr_pop();
    /* The piece's exit sets Y only when it spawned (the slot search's slot). */
    y = r.piece.spawned ? r.piece.found.slot : y_entry;
    r.count = DEC8(OBJ->work_e[slot]);
    if ((r.count & 0x80) != 0) {
        WR8(OBJ->work_e[slot], 0x10);
        jsr_push(0xAD39);
        r.call2 = SoundCommand(0x19, slot, y);
        jsr_pop();
        jsr_push(0xAD3C);
        r.move = ApplyVelocityY(slot);
        jsr_pop();
        r.inc = INC8(OBJ->f34b8[slot]);
        r.steps = RD8(OBJ->f34b8[slot]);
        if (r.steps >= 4) {
            jsr_push(0xAD4C);
            r.run = DrawTileScript(1, 0);
            jsr_pop();
            wr8(LevelYHi, 0);
            wr8(RasterWaterWanted, 0);
            jsr_push(0xAD55);
            r.event.marked = RecordObjectEvent(slot, &r.event.stored, &r.event.event, &r.event.byte);
            jsr_pop();
            ClearObject(slot);
            r.arm = ST17_PULSE_DONE;
            return r;
        }
        r.arm = ST17_PULSE_STEPPED;
    } else {
        r.arm = ST17_PULSE_COUNTING;
    }
    r.wave = (uint8_t)(RD8(ZP->wave_counter) & 0x03);
    r.index = (uint8_t)(r.wave << 1);
    r.value_a = rd8((uint16_t)(0xAD6B + r.index));
    wr8(RasterWaterWanted, r.value_a);
    r.value_b = rd8((uint16_t)(0xAD6C + r.index));
    wr8(LevelYHi, r.value_b);
    return r;
}

/* What PulseWaterLevelAndScatter leaves, both sound calls included, from the arm it ended on. */
static DoubleSound l8_pulse_exit_shim(L8St17PulseArm r)
{
    SoundEnd first = sound_end_after(ended_load(r.number), l8_sound19_piece_exit_shim(r.piece));
    DoubleSound t;
    Ended e;

    t.second = false;
    t.end = ended_kept();
    t.first = first;
    e = first.end;
    e = then(e, ended_nz(r.count));
    if (r.arm != ST17_PULSE_COUNTING) {
        e = then(e, ended_load(0x10));
        e = then(e, ended_load(0x19));
        t.first.end = e;
        t.second = true;
        t.call2 = r.call2;
        e = ended_of_axis_move(r.move);
        e = then(e, ended_nz(r.inc));
        e = then(e, ended_load(r.steps));
        e = then(e, ended_compare(r.steps, 4));
        if (r.arm == ST17_PULSE_DONE) {
            e = then(e, ended_of_tile_run(r.run, 1));
            e = then(e, l8_st17_event_ended(r.event));
            t.end = e;
            return t;
        }
    } else {
        t.first.end = e;
        e = ended_kept();
    }
    e = then(e, ended_load(r.wave));
    e = then(e, ended_with_y(ended_with_carry(ended_load(r.index), false), r.index));
    e = then(e, ended_load(r.value_a));
    t.end = then(e, ended_load(r.value_b));
    return t;
}

void abi_PulseWaterLevelAndScatter(void)
{
    abi_double_sound(l8_pulse_exit_shim(pulse_water_level_and_scatter(cpu.x, cpu.y)));
}

/* LoadRecordThenWatchThenDropTwice: the state, the record, the scan of slots $10 on (the last kind and slot read), the byte reads of the watch, and the fall's add, count, move and ground. */
typedef struct {
    int arm;
    uint8_t state, kind, scan_y, frame, x_low, x_high, player_x, ground, high, count, carried;
    L8Record record;
    AxisMove move;
    Sum sum;
    CommandCall call;
} L8St17WatchArm;

/*
 * @routine 0B4A:79D LoadRecordThenWatchThenDropTwice
 * @run 70:AF9D
 * WHAT IT DOES: In state 0 loads the record. State 1 looks for an object of kind $24 in slots $10 to $2E; when it is in frame $11, at X low $B8 or more on the first screen, and the player's X low is $C0 or more, the object goes to state 2. State 2: its Y speed gains $0020 a frame and it moves; at Y low $BC it stops, takes Y speed high $FE, plays sound $2C and goes to state 3. State 3 does the same fall until its Y high byte is not 0, then goes to state 4.
 * HOW: LoadObjectRecord, a search, compares, 16-bit adds in memory, ApplyVelocity, SoundCommand.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_watch_drop_exit_shim). Everything past state 0 ran in no script; the draft stops there and the arms are written from the decoded bytes.
 */
L8St17WatchArm LoadRecordThenWatchThenDropTwice(uint8_t slot, uint8_t y_entry)
{
    uint8_t y, value;
    L8St17WatchArm r = { 0 };

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        r.record = l8_record(slot);
        r.arm = ST17_WATCH_LOADED;
        return r;
    }
    if (r.state == 1) {
        for (y = 0x10;;) {
            r.kind = RD8(OBJ->kind[y]);
            r.scan_y = y;
            if (r.kind == 0x24) {
                break;
            }
            y++;
            if (y == 0x2F) {
                r.arm = ST17_WATCH_NO_KIND;
                return r;
            }
        }
        r.frame = RD8(OBJ->frame[y]);
        if (r.frame != 0x11) {
            r.arm = ST17_WATCH_FRAME;
            return r;
        }
        r.x_low = RD8(OBJ->x_lo[y]);
        if (r.x_low < 0xB8) {
            r.arm = ST17_WATCH_X_LOW;
            return r;
        }
        r.x_high = RD8(OBJ->x_hi[y]);
        if (r.x_high != 0) {
            r.arm = ST17_WATCH_X_HIGH;
            return r;
        }
        r.player_x = RD8(OBJ->x_lo[0]);
        if (r.player_x < 0xC0) {
            r.arm = ST17_WATCH_PLAYER;
            return r;
        }
        r.count = INC8(OBJ->state[slot]);
        r.arm = ST17_WATCH_SEEN;
        return r;
    }
    if (r.state == 2) {
        value = RD8(OBJ->vy_lo[slot]);
        r.sum = add_bytes(value, 0x20, false);
        WR8(OBJ->vy_lo[slot], r.sum.value);
        if (r.sum.carry) {
            r.carried = INC8(OBJ->vy_hi[slot]);
        }
        jsr_push(0xAFCB);
        r.move = ApplyVelocity(slot);
        jsr_pop();
        r.ground = RD8(OBJ->y_lo[slot]);
        if (r.ground < 0xBC) {
            r.arm = ST17_WATCH_FALLING;
            return r;
        }
        WR8(OBJ->vy_lo[slot], 0);
        WR8(OBJ->vy_hi[slot], 0xFE);
        jsr_push(0xAFDF);
        r.call = SoundCommand(0x2C, slot, y_entry);
        jsr_pop();
        r.count = INC8(OBJ->state[slot]);
        r.arm = ST17_WATCH_LANDED;
        return r;
    }
    if (r.state != 3) {
        r.arm = ST17_WATCH_IDLE;
        return r;
    }
    value = RD8(OBJ->vy_lo[slot]);
    r.sum = add_bytes(value, 0x20, false);
    WR8(OBJ->vy_lo[slot], r.sum.value);
    if (r.sum.carry) {
        r.carried = INC8(OBJ->vy_hi[slot]);
    }
    jsr_push(0xAFF7);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.high = RD8(OBJ->y_hi[slot]);
    if (r.high == 0) {
        r.arm = ST17_WATCH_DROP_AIR;
        return r;
    }
    r.count = INC8(OBJ->state[slot]);
    r.arm = ST17_WATCH_DROP_DONE;
    return r;
}

/* What LoadRecordThenWatchThenDropTwice leaves, the sound call's included, from the arm it ended on. */
static SoundEnd l8_watch_drop_exit_shim(L8St17WatchArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_load(r.state);

    if (r.arm == ST17_WATCH_LOADED) {
        s.end = then(e, l8_record_ended(r.record));
        return s;
    }
    e = then(e, ended_load((uint8_t)(r.state - 1)));
    if (r.arm <= ST17_WATCH_SEEN) {
        e = then(e, ended_with_y(ended_compare(r.kind, 0x24), r.scan_y));
        if (r.arm == ST17_WATCH_NO_KIND) {
            uint8_t y = (uint8_t)(r.scan_y + 1);

            s.end = then(e, ended_with_y(l8_compare_y(y, 0x2F), y));
            return s;
        }
        if (r.arm == ST17_WATCH_FRAME) {
            s.end = then(e, ended_compare(r.frame, 0x11));
            return s;
        }
        e = then(e, ended_compare(r.frame, 0x11));
        e = then(e, ended_compare(r.x_low, 0xB8));
        if (r.arm == ST17_WATCH_X_LOW) {
            s.end = e;
            return s;
        }
        e = then(e, ended_load(r.x_high));
        if (r.arm == ST17_WATCH_X_HIGH) {
            s.end = e;
            return s;
        }
        e = then(e, ended_compare(r.player_x, 0xC0));
        if (r.arm == ST17_WATCH_PLAYER) {
            s.end = e;
            return s;
        }
        s.end = then(e, ended_nz(r.count));
        return s;
    }
    e = then(e, ended_load((uint8_t)(r.state - 2)));
    if (r.arm == ST17_WATCH_FALLING || r.arm == ST17_WATCH_LANDED) {
        e = then(e, ended_of_sum(r.sum));
        if (r.sum.carry) {
            e = then(e, ended_nz(r.carried));
        }
        e = then(e, ended_of_axis_move(r.move));
        e = then(e, ended_compare(r.ground, 0xBC));
        if (r.arm == ST17_WATCH_FALLING) {
            s.end = e;
            return s;
        }
        e = then(e, ended_load(0xFE));
        s.sounded = true;
        s.command = 0x2C;
        s.before = then(e, ended_load(0x2C));
        s.call = r.call;
        s.end = ended_nz(r.count);
        return s;
    }
    e = then(e, ended_load((uint8_t)(r.state - 3)));
    if (r.arm == ST17_WATCH_IDLE) {
        s.end = e;
        return s;
    }
    e = then(e, ended_of_sum(r.sum));
    if (r.sum.carry) {
        e = then(e, ended_nz(r.carried));
    }
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_load(r.high));
    if (r.arm == ST17_WATCH_DROP_AIR) {
        s.end = e;
        return s;
    }
    s.end = then(e, ended_nz(r.count));
    return s;
}

void abi_LoadRecordThenWatchThenDropTwice(void)
{
    abi_sound_end(l8_watch_drop_exit_shim(LoadRecordThenWatchThenDropTwice(cpu.x, cpu.y)));
}

/* The four VDC word writes of 0B4A:6B6: the select, the low byte (the object's OBJ_WORK_E, which then grows by $10), the high byte $0F, then the data select and a zero word. Returns the add of the grown byte. */
static Sum write_tile_pair(uint8_t slot)
{
    uint8_t low;
    Sum sum;

    jsr_push(0xAF55);
    SelectVramWriteAddress();
    jsr_pop();
    low = RD8(OBJ->work_e[slot]);
    WR8(VDC->data_lo, low);
    sum = add_bytes(low, 0x10, false);
    WR8(OBJ->work_e[slot], sum.value);
    WR8(VDC->data_hi, 0x0F);
    jsr_push(0xAF66);
    SelectVramData();
    jsr_pop();
    WR8(VDC->data_lo, 0);
    WR8(VDC->data_hi, 0);
    return sum;
}

/* BossPhasesHurtFallAndTileWrites: the state, the reads and counts of each arm, the two events, the side, the record, the list step and channel call, the move and subtract, and the four tile pair adds. */
typedef struct {
    int arm;
    uint8_t state, hits, timer, flag, bits, value, count, pulled;
    L8St17Event event1, event2;
    SideResult side;
    L8Record record;
    L8Step step;
    AxisMove move;
    Sum diff;
    Sum pairs[4];
} L8St17BossArm;

/*
 * @routine 0B4A:6B6 BossPhasesHurtFallAndTileWrites
 * @run 70:AEB6
 * WHAT IT DOES: A boss in phases. State 0 loads the record. State 1: at 1 to 4 hits records the event and goes to state 2; at 5 hits, with the player on the right, takes the X speed 4, OBJ_WORK_D $10, records the event, counts OBJ_WORK_F up and goes to state 5. State 2 steps the list at $AF8A and, when it ends, goes to state 3 with OBJ_WORK_D $10 and starts list 0 on channel 5. State 3 waits OBJ_WORK_D down, then state 4 with OBJ_WORK_D $40 and frame $0F. State 4 counts down, drops one row every eighth frame and writes four tile pairs; at the end it clears the object. States 5 on fall: the X speed loses $0034 a frame until OBJ_WORK_D runs out, then state 2.
 * HOW: LoadObjectRecord, RecordObjectEvent, SideOfPlayer, StepObjectFrames, StartListChannelKind3, VDC writes, ApplyVelocity, ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_boss_phases_exit_shim). Everything past state 0 ran in no script; the draft stops there and the arms are written from the decoded bytes.
 */
L8St17BossArm BossPhasesHurtFallAndTileWrites(uint8_t slot, uint8_t y_entry)
{
    uint8_t bits, value;
    int round;
    L8St17BossArm r = { 0 };

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        r.record = l8_record(slot);
        r.arm = ST17_BOSS_LOADED;
        return r;
    }
    if (r.state == 1) {
        r.hits = RD8(OBJ->hit_taken[slot]);
        if (r.hits == 0) {
            r.arm = ST17_BOSS_NO_HITS;
            return r;
        }
        if (r.hits < 5) {
            jsr_push(0xAECD);
            r.event1.marked = RecordObjectEvent(slot, &r.event1.stored, &r.event1.event, &r.event1.byte);
            jsr_pop();
            r.count = INC8(OBJ->state[slot]);
            r.arm = ST17_BOSS_HURT;
            return r;
        }
        jsr_push(0xAED4);
        r.side = SideOfPlayer(slot);
        jsr_pop();
        if ((r.side.sides & 1) != 0) {
            r.count = INC8(OBJ->state[slot]);
            r.arm = ST17_BOSS_RIGHT;
            return r;
        }
        WR8(OBJ->vx_hi[slot], 4);
        WR8(OBJ->f34b8[slot], 0x10);
        jsr_push(0xAEE4);
        r.event2.marked = RecordObjectEvent(slot, &r.event2.stored, &r.event2.event, &r.event2.byte);
        jsr_pop();
        r.count = INC8(OBJ->f3568[slot]);
        WR8(OBJ->state[slot], 5);
        r.arm = ST17_BOSS_CHARGE;
        return r;
    }
    if (r.state == 2) {
        r.step = l8_list_step(slot, 0xAF8A, 0xAEFB, true, y_entry);
        if (r.step.step != FRAME_LIST_ENDED) {
            r.arm = ST17_BOSS_STEPPED;
            return r;
        }
        r.count = INC8(OBJ->state[slot]);
        WR8(OBJ->f34b8[slot], 0x10);
        push8(slot);
        jsr_push(0xAF0C);
        StartListChannelKind3(0, 5);
        jsr_pop();
        slot = pull8();
        r.pulled = slot;
        r.arm = ST17_BOSS_LISTED;
        return r;
    }
    if (r.state == 3) {
        r.timer = DEC8(OBJ->f34b8[slot]);
        if (r.timer != 0) {
            r.arm = ST17_BOSS_WAITING;
            return r;
        }
        r.count = INC8(OBJ->state[slot]);
        WR8(OBJ->f34b8[slot], 0x40);
        WR8(OBJ->frame[slot], 0x0F);
        r.arm = ST17_BOSS_TILES_SET;
        return r;
    }
    if (r.state == 4) {
        r.timer = DEC8(OBJ->f34b8[slot]);
        if (r.timer == 0) {
            r.flag = RD8(OBJ->f3568[slot]);
            ClearObject(slot);
            r.arm = ST17_BOSS_CLEARED;
            return r;
        }
        bits = RD8(OBJ->f34b8[slot]);
        r.value = (uint8_t)(bits & 0x07);
        if (r.value != 0) {
            r.arm = ST17_BOSS_GAP;
            return r;
        }
        r.count = INC8(OBJ->y_lo[slot]);
        bits = RD8(OBJ->f34b8[slot]);
        value = (uint8_t)((bits >> 3) + 0xC8);
        WR8(OBJ->work_e[slot], value);
        for (round = 0; round < 4; round++) {
            r.pairs[round] = write_tile_pair(slot);
        }
        r.arm = ST17_BOSS_TILES;
        return r;
    }
    jsr_push(0xAF6E);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    value = RD8(OBJ->vx_lo[slot]);
    r.diff = sub_bytes(value, 0x34, true);
    WR8(OBJ->vx_lo[slot], r.diff.value);
    value = RD8(OBJ->vx_hi[slot]);
    r.diff = sub_bytes(value, 0, r.diff.carry);
    WR8(OBJ->vx_hi[slot], r.diff.value);
    r.timer = DEC8(OBJ->f34b8[slot]);
    if (r.timer != 0) {
        r.arm = ST17_BOSS_FALLING;
        return r;
    }
    WR8(OBJ->state[slot], 2);
    r.arm = ST17_BOSS_RECOVERED;
    return r;
}

/* What BossPhasesHurtFallAndTileWrites leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_boss_phases_exit_shim(L8St17BossArm r)
{
    Ended e = ended_load(r.state);
    int round;

    if (r.arm == ST17_BOSS_LOADED) {
        return then(e, l8_record_ended(r.record));
    }
    e = then(e, ended_load((uint8_t)(r.state - 1)));
    if (r.arm <= ST17_BOSS_CHARGE) {
        e = then(e, ended_load(r.hits));
        if (r.arm == ST17_BOSS_NO_HITS) {
            return e;
        }
        e = then(e, ended_compare(r.hits, 5));
        if (r.arm == ST17_BOSS_HURT) {
            e = then(e, l8_st17_event_ended(r.event1));
            return then(e, ended_nz(r.count));
        }
        e = then(e, ended_with_carry(ended_load((uint8_t)(r.side.sides >> 1)), (r.side.sides & 1) != 0));
        if (r.arm == ST17_BOSS_RIGHT) {
            return then(e, ended_nz(r.count));
        }
        e = then(e, ended_load(0x10));
        e = then(e, l8_st17_event_ended(r.event2));
        e = then(e, ended_nz(r.count));
        return then(e, ended_load(5));
    }
    e = then(e, ended_load((uint8_t)(r.state - 2)));
    if (r.arm <= ST17_BOSS_LISTED) {
        e = then(e, l8_step_ended(r.step));
        if (r.arm == ST17_BOSS_STEPPED) {
            return e;
        }
        e = then(e, ended_nz(r.count));
        return then(e, ended_with_a(ended_nz(r.pulled), 3));
    }
    e = then(e, ended_load((uint8_t)(r.state - 3)));
    if (r.arm <= ST17_BOSS_TILES_SET) {
        e = then(e, ended_nz(r.timer));
        if (r.arm == ST17_BOSS_WAITING) {
            return e;
        }
        e = then(e, ended_nz(r.count));
        return then(e, ended_load(0x0F));
    }
    e = then(e, ended_load((uint8_t)(r.state - 4)));
    if (r.arm <= ST17_BOSS_TILES) {
        e = then(e, ended_nz(r.timer));
        if (r.arm == ST17_BOSS_CLEARED) {
            return then(e, ended_load(r.flag));
        }
        e = then(e, ended_load(r.value));
        if (r.arm == ST17_BOSS_GAP) {
            return e;
        }
        e = then(e, ended_nz(r.count));
        for (round = 0; round < 4; round++) {
            e = then(e, ended_of_sum(r.pairs[round]));
        }
        return e;
    }
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_of_sum(r.diff));
    e = then(e, ended_nz(r.timer));
    if (r.arm == ST17_BOSS_FALLING) {
        return e;
    }
    return then(e, ended_load(2));
}

void abi_BossPhasesHurtFallAndTileWrites(void)
{
    abi_ended(l8_boss_phases_exit_shim(BossPhasesHurtFallAndTileWrites(cpu.x, cpu.y)));
}
