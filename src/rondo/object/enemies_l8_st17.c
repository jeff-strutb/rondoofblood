/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 0B4A (enemy kinds not yet identified), by routine. Bodies return how the
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

/*
 * @routine 0B4A:39F StartRingOfPieces
 * @run 70:AB9F
 * WHAT IT DOES: For the numbers 0 to $17 starts an object of type $28 in a free slot of $20 to $3F at the object's place and gives it the seven bytes of the eight byte row of the table at $ABE8: OBJ_34B8, X speed high, Y speed high, OBJ_3568, OBJ_35C0, OBJ_3618 and OBJ_3670; it stops when no slot is free.
 * HOW: A loop of FindFreeSlotFrom20, StartObjectAtPlace, a table read by the number times 8, stores. zD9 and zDA keep their address form until they have names.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended StartRingOfPieces(uint8_t slot)
{
    uint8_t number = 0, found_slot, last, placed, row, value = 0;
    bool found;
    Ended e;

    for (;;) {
        wr8(0x20DA, number);
        jsr_push(0xABA5);
        found_slot = FindFreeSlotFrom20(&last, &found);
        jsr_pop();
        e = ended_of_find_slot(found_slot, last, found);
        if (!found) {
            return e;
        }
        jsr_push(0xABAC);
        placed = StartObjectAtPlace(0x28, slot, found_slot);
        jsr_pop();
        wr8(0x20D9, slot);
        row = (uint8_t)(rd8(0x20DA) << 3);
        value = rd8((uint16_t)(0xABE8 + row));
        WR8(OBJ->f34b8[found_slot], value);
        value = rd8((uint16_t)(0xABE9 + row));
        WR8(OBJ->vx_hi[found_slot], value);
        value = rd8((uint16_t)(0xABEA + row));
        WR8(OBJ->vy_hi[found_slot], value);
        value = rd8((uint16_t)(0xABEB + row));
        WR8(OBJ->f3568[found_slot], value);
        value = rd8((uint16_t)(0xABEC + row));
        WR8(OBJ->f35c0[found_slot], value);
        value = rd8((uint16_t)(0xABED + row));
        WR8(OBJ->f3618[found_slot], value);
        value = rd8((uint16_t)(0xABEE + row));
        WR8(OBJ->f3670[found_slot], value);
        slot = rd8(0x20D9);
        number = rd8(0x20DA);
        number++;
        e = ended_with_y(ended_with_a(l8_compare_y(number, 0x18), value), number);
        (void)placed;
        if (number >= 0x18) {
            return e;
        }
    }
}

void abi_StartRingOfPieces(void)
{
    abi_ended(StartRingOfPieces(cpu.x));
}

/*
 * @routine 0B4A:4A8 LoadRecordFlingThenFall
 * @run 70:ACA8
 * WHAT IT DOES: In state 0 loads the record, moves the object once and takes its speed from OBJ_3568, OBJ_35C0, OBJ_3618 and OBJ_3670, and its frame from OBJ_34B8 plus $12. Later it falls (ApplyGravityUnlimited) and moves.
 * HOW: LoadObjectRecord, ApplyVelocity, four copies, an add, ApplyGravityUnlimited, a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended LoadRecordFlingThenFall(uint8_t slot)
{
    uint8_t value;
    AxisMove move;
    GravityResult gravity;
    Sum sum;
    uint8_t first;
    Ended e = l8_load_if_new(slot, 0xACB0, &first);

    if (first != 0) {
        jsr_push(0xACD8);
        gravity = ApplyGravityUnlimited(slot);
        jsr_pop();
        (void)gravity;
        move = ApplyVelocity(slot);
        return ended_of_axis_move(move);
    }
    jsr_push(0xACB3);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    value = RD8(OBJ->f3568[slot]);
    WR8(OBJ->vx_hi[slot], value);
    value = RD8(OBJ->f35c0[slot]);
    WR8(OBJ->vx_lo[slot], value);
    value = RD8(OBJ->f3618[slot]);
    WR8(OBJ->vy_hi[slot], value);
    value = RD8(OBJ->f3670[slot]);
    WR8(OBJ->vy_lo[slot], value);
    value = RD8(OBJ->f34b8[slot]);
    sum = add_bytes(value, 0x12, false);
    WR8(OBJ->frame[slot], sum.value);
    return then(e, ended_of_sum(sum));
}

void abi_LoadRecordFlingThenFall(void)
{
    abi_ended(LoadRecordFlingThenFall(cpu.x));
}

/*
 * @routine 0B4A:4F4 LoadRecordFacingLeftRising
 * @run 70:ACF4
 * WHAT IT DOES: Loads the object's record, sets its facing 1, frame $15 and Y speed high $F0.
 * HOW: LoadObjectRecord and three stores.
 * WAITS: no.
 * Entry: X object. Exit: A $F0, Y 11, C from the record.
 */
Ended LoadRecordFacingLeftRising(uint8_t slot)
{
    uint8_t last, nz;
    bool carry;
    Ended e;

    jsr_push(0xACF7);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->facing_left[slot], 1);
    WR8(OBJ->frame[slot], 0x15);
    WR8(OBJ->vy_hi[slot], 0xF0);
    return then(e, ended_load(0xF0));
}

void abi_LoadRecordFacingLeftRising(void)
{
    abi_ended(LoadRecordFacingLeftRising(cpu.x));
}

/*
 * @routine 0B4A:507 SoundBAAtScreen2
 * @run 70:AD07
 * WHAT IT DOES: When the scroll's screen byte is 2 plays sound $BA and goes to the next state.
 * HOW: A compare, SoundCommand, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
SoundEnd SoundBAAtScreen2(uint8_t slot, uint8_t y_entry)
{
    uint8_t screen;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    screen = RD8(SCROLLE->x_hi_screen[0]);
    e = ended_compare(screen, 2);
    if (screen != 2) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0xBA;
    r.before = then(e, ended_load(0xBA));
    jsr_push(0xAD13);
    r.call = SoundCommand(0xBA, slot, y_entry);
    jsr_pop();
    r.end = ended_nz(INC8(OBJ->state[slot]));
    return r;
}

void abi_SoundBAAtScreen2(void)
{
    abi_sound_end(SoundBAAtScreen2(cpu.x, cpu.y));
}

/*
 * @routine 0B4A:58B LoadRecordThenDrawTileScript2
 * @run 70:AD8B
 * WHAT IT DOES: When bit 0 of zField92 is set the object is cleared; otherwise it loads its record, sets its hit offset OBJ_39E0 $E8 and draws tile script 2.
 * HOW: A test, a tail jump into ClearObject, LoadObjectRecord, DrawTileScript.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTileScript, or the test when the object is cleared. The jump into ClearObject ran in no script.
 */
Ended LoadRecordThenDrawTileScript2(uint8_t slot)
{
    uint8_t flag, last, nz;
    bool carry;
    TileRun run;
    Ended e;

    flag = RD8(ZP->field92);
    e = ended_load((uint8_t)(flag & 0x01));
    if ((flag & 0x01) != 0) {
        ClearObject(slot);
        return e;
    }
    jsr_push(0xAD97);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->hitbox_dy[slot], 0xE8);
    jsr_push(0xADA2);
    run = DrawTileScript(2, 0);
    jsr_pop();
    return then(e, ended_of_tile_run(run, 2));
}

void abi_LoadRecordThenDrawTileScript2(void)
{
    abi_ended(LoadRecordThenDrawTileScript2(cpu.x));
}

/*
 * @routine 0B4A:608 ScatterPiecesDownward
 * @run 70:AE08
 * WHAT IT DOES: Moves the object down by its hit offset OBJ_39E0, starts up to 8 objects of type $9A in free slots of $30 to $3F at its place, each with an X speed from the table at $AE4E by the parity of the count, moved once, OBJ_34B8 1 and OBJ_3510 the count less 1, then moves the object back.
 * HOW: An add and a subtract of OBJ_39E0 on OBJ_Y_LO, a loop of FindFreeSlotFrom30, StartObjectAtPlace and ApplyVelocityX with X and Y swapped, a counter in zTemp00. zD9 keeps its address form until it has a name.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended ScatterPiecesDownward(uint8_t slot)
{
    uint8_t value, offset, count, found_slot, last, placed, parity, speed;
    bool found;
    AxisMove move;
    Sum sum;
    Ended e;

    value = RD8(OBJ->y_lo[slot]);
    offset = RD8(OBJ->hitbox_dy[slot]);
    sum = add_bytes(value, offset, false);
    WR8(OBJ->y_lo[slot], sum.value);
    e = ended_of_sum(sum);
    WR8(ZP->temp00, 8);
    e = then(e, ended_load(8));
    for (;;) {
        jsr_push(0xAE19);
        found_slot = FindFreeSlotFrom30(&last, &found);
        jsr_pop();
        e = then(e, ended_of_find_slot(found_slot, last, found));
        if (!found) {
            break;
        }
        jsr_push(0xAE20);
        placed = StartObjectAtPlace(0x9A, slot, found_slot);
        jsr_pop();
        e = then(e, ended_load(placed));
        wr8(0x20D9, slot);
        count = RD8(ZP->temp00);
        parity = count & 0x01;
        e = then(e, ended_nz(parity));
        speed = rd8((uint16_t)(0xAE4E + parity));
        WR8(OBJ->vx_hi[found_slot], speed);
        e = then(e, ended_load(speed));
        jsr_push(0xAE31);
        move = ApplyVelocityX(found_slot);
        jsr_pop();
        e = then(e, ended_of_axis_move(move));
        slot = rd8(0x20D9);
        e = then(e, ended_nz(slot));
        WR8(OBJ->f34b8[found_slot], 1);
        count = RD8(ZP->temp00);
        count--;
        WR8(OBJ->f3510[found_slot], count);
        e = then(e, ended_load(count));
        count = DEC8(ZP->temp00);
        e = then(e, ended_nz(count));
        if (count == 0) {
            break;
        }
    }
    value = RD8(OBJ->y_lo[slot]);
    offset = RD8(OBJ->hitbox_dy[slot]);
    sum = sub_bytes(value, offset, true);
    WR8(OBJ->y_lo[slot], sum.value);
    return then(e, ended_of_sum(sum));
}

void abi_ScatterPiecesDownward(void)
{
    abi_ended(ScatterPiecesDownward(cpu.x));
}

/*
 * @routine 0B4A:650 ClearWhenHurtThenWait
 * @run 70:AE50
 * WHAT IT DOES: When the object has taken 1 to 4 hits: if the player's health zPlayerHealth is $5C plays sound $0B, becomes kind $9F with OBJ_34B8 3 and state 0; otherwise sets the health $5C and clears itself.
 * HOW: Two compares, a compare of the health, SoundCommand, stores, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The arm for health $5C ran in no script; the draft stops there.
 */
SoundEnd ClearWhenHurtThenWait(uint8_t slot, uint8_t y_entry)
{
    uint8_t hits, health;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    hits = RD8(OBJ->hit_taken[slot]);
    e = ended_load(hits);
    if (hits == 0) {
        r.end = e;
        return r;
    }
    e = then(e, ended_compare(hits, 5));
    if (hits >= 5) {
        r.end = e;
        return r;
    }
    health = RD8(ZP->player_health);
    e = then(e, ended_compare(health, 0x5C));
    if (health != 0x5C) {
        WR8(ZP->player_health, 0x5C);
        ClearObject(slot);
        r.end = then(e, ended_load(0x5C));
        return r;
    }
    r.sounded = true;
    r.command = 0x0B;
    r.before = then(e, ended_load(0x0B));
    jsr_push(0xAE64);
    r.call = SoundCommand(0x0B, slot, y_entry);
    jsr_pop();
    WR8(OBJ->kind[slot], 0x9F);
    WR8(OBJ->f34b8[slot], 3);
    WR8(OBJ->state[slot], 0);
    r.end = ended_load(3);
    return r;
}

void abi_ClearWhenHurtThenWait(void)
{
    abi_sound_end(ClearWhenHurtThenWait(cpu.x, cpu.y));
}

/*
 * @routine 0B4A:5A3 DrawTileScriptStepsThenScatter
 * @run 70:ADA3
 * WHAT IT DOES: When the object's X high byte is not 0 draws tile script $02 plus OBJ_34B8. Otherwise, at 5 hits taken, it sets OBJ_3A90 $18, draws the next tile script, plays sound $19, scatters pieces (ScatterPiecesDownward) and advances OBJ_34B8; below 3 steps it takes OBJ_39E0 and the frame from the table at $AE02; at 3 it sets zField92 1, clears OBJ_39E0, sets the hit box $10 by $10 and frame $17 and goes to the next state.
 * HOW: DrawTileScript, SoundCommand, ScatterPiecesDownward, table reads, stores.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
SoundEnd DrawTileScriptStepsThenScatter(uint8_t slot, uint8_t y_entry)
{
    uint8_t high, step, hits, item, index, value;
    TileRun run;
    Sum sum;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    high = RD8(OBJ->x_hi[slot]);
    e = ended_load(high);
    if (high != 0) {
        step = RD8(OBJ->f34b8[slot]);
        sum = add_bytes(2, step, false);
        e = then(e, ended_of_sum(sum));
        item = sum.value;
        run = DrawTileScript(item, 0);
        r.end = then(e, ended_of_tile_run(run, item));
        return r;
    }
    hits = RD8(OBJ->hit_taken[slot]);
    e = then(e, ended_compare(hits, 5));
    if (hits != 5) {
        r.end = e;
        return r;
    }
    WR8(OBJ->timer[slot], 0x18);
    step = RD8(OBJ->f34b8[slot]);
    sum = add_bytes(2, step, false);
    item = (uint8_t)(sum.value + 1);
    e = then(then(e, ended_of_sum(sum)), ended_nz(item));
    jsr_push(0xADC9);
    run = DrawTileScript(item, 0);
    jsr_pop();
    e = then(e, ended_of_tile_run(run, item));
    r.sounded = true;
    r.command = 0x19;
    r.before = then(e, ended_load(0x19));
    jsr_push(0xADCE);
    r.call = SoundCommand(0x19, slot, 0);
    jsr_pop();
    jsr_push(0xADD1);
    e = ScatterPiecesDownward(slot);
    jsr_pop();
    e = then(e, ended_nz(INC8(OBJ->f34b8[slot])));
    step = RD8(OBJ->f34b8[slot]);
    e = then(e, ended_load(step));
    e = then(e, ended_compare(step, 3));
    if (step >= 3) {
        WR8(ZP->field92, 1);
        WR8(OBJ->hitbox_dy[slot], 0);
        WR8(OBJ->hitbox_half_w[slot], 0x10);
        WR8(OBJ->hitbox_half_h[slot], 0x10);
        WR8(OBJ->frame[slot], 0x17);
        e = then(e, ended_load(0x17));
        r.end = then(e, ended_nz(INC8(OBJ->state[slot])));
        return r;
    }
    index = (uint8_t)(step << 1);
    e = then(e, ended_with_y(ended_with_carry(ended_load(index), false), index));
    value = rd8((uint16_t)(0xAE02 + index));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(0xAE03 + index));
    WR8(OBJ->frame[slot], value);
    r.end = then(e, ended_load(value));
    return r;
}

void abi_DrawTileScriptStepsThenScatter(void)
{
    abi_sound_end(DrawTileScriptStepsThenScatter(cpu.x, cpu.y));
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

/*
 * @routine 0B4A:517 PulseWaterLevelAndScatter
 * @run 70:AD17
 * WHAT IT DOES: Sets slot 0's OBJ_3510 3, OBJ_3568 0 and the byte at $3BF0 to 2; hands OBJ_3510 of this object to MaybeSound19AndSpawnPiece and counts it down. When it goes below 0 it sets it $10, plays sound $19, moves in Y and advances OBJ_34B8; at 4 it draws tile script 1, clears the water level, records the object's event and clears itself. Otherwise the water level takes a pair of bytes from the table at $AD6B by the wave counter.
 * HOW: MaybeSound19AndSpawnPiece, SoundCommand, ApplyVelocityY, DrawTileScript, RecordObjectEvent, ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept for the sounds. Exit: as the last instruction that ran. The byte at $3BF0 keeps its address form until it has a name.
 */
static DoubleSound pulse_water_level_and_scatter(uint8_t slot, uint8_t y_entry)
{
    uint8_t value, y, index, steps, stored, event, byte;
    SoundEnd first;
    AxisMove move;
    TileRun run;
    bool marked;
    DoubleSound t;
    Ended e;

    t.second = false;
    t.end = ended_kept();
    wr8(0x3BF0, 0x02);
    WR8(OBJ->f3510[0], 3);
    WR8(OBJ->f3568[0], 0);
    value = RD8(OBJ->f3510[slot]);
    jsr_push(0xAD2A);
    first = MaybeSound19AndSpawnPiece(slot, value, y_entry);
    jsr_pop();
    first = sound_end_after(ended_load(value), first);
    t.first = first;
    y = first.end.y_set ? first.end.y : y_entry;
    e = first.end;
    value = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(value));
    if ((value & 0x80) != 0) {
        WR8(OBJ->f3510[slot], 0x10);
        e = then(e, ended_load(0x10));
        e = then(e, ended_load(0x19));
        t.first.end = e;
        t.second = true;
        jsr_push(0xAD39);
        t.call2 = SoundCommand(0x19, slot, y);
        jsr_pop();
        jsr_push(0xAD3C);
        move = ApplyVelocityY(slot);
        jsr_pop();
        e = ended_of_axis_move(move);
        e = then(e, ended_nz(INC8(OBJ->f34b8[slot])));
        steps = RD8(OBJ->f34b8[slot]);
        e = then(e, ended_load(steps));
        e = then(e, ended_compare(steps, 4));
        if (steps >= 4) {
            jsr_push(0xAD4C);
            run = DrawTileScript(1, 0);
            jsr_pop();
            e = then(e, ended_of_tile_run(run, 1));
            wr8(LevelYHi, 0);
            wr8(RasterWaterWanted, 0);
            jsr_push(0xAD55);
            marked = RecordObjectEvent(slot, &stored, &event, &byte);
            jsr_pop();
            e = then(e, l8_of_event(marked, stored, event, byte));
            ClearObject(slot);
            t.end = e;
            return t;
        }
    } else {
        t.first.end = e;
        e = ended_kept();
    }
    value = (uint8_t)(RD8(ZP->wave_counter) & 0x03);
    e = then(e, ended_load(value));
    index = (uint8_t)(value << 1);
    e = then(e, ended_with_y(ended_with_carry(ended_load(index), false), index));
    value = rd8((uint16_t)(0xAD6B + index));
    wr8(RasterWaterWanted, value);
    e = then(e, ended_load(value));
    value = rd8((uint16_t)(0xAD6C + index));
    wr8(LevelYHi, value);
    t.end = then(e, ended_load(value));
    return t;
}

void abi_PulseWaterLevelAndScatter(void)
{
    abi_double_sound(pulse_water_level_and_scatter(cpu.x, cpu.y));
}

/*
 * @routine 0B4A:79D LoadRecordThenWatchThenDropTwice
 * @run 70:AF9D
 * WHAT IT DOES: In state 0 loads the record. State 1 looks for an object of kind $24 in slots $10 to $2E; when it is in frame $11, at X low $B8 or more on the first screen, and the player's X low is $C0 or more, the object goes to state 2. State 2: its Y speed gains $0020 a frame and it moves; at Y low $BC it stops, takes Y speed high $FE, plays sound $2C and goes to state 3. State 3 does the same fall until its Y high byte is not 0, then goes to state 4.
 * HOW: LoadObjectRecord, a search, compares, 16-bit adds in memory, ApplyVelocity, SoundCommand.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. Everything past state 0 ran in no script; the draft stops there and the arms are written from the decoded bytes.
 */
SoundEnd LoadRecordThenWatchThenDropTwice(uint8_t slot, uint8_t y_entry)
{
    uint8_t state, last, nz, y, kind, value, ground, high;
    bool carry;
    AxisMove move;
    Sum sum;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        r.end = then(e, ended_of_record(last, nz, carry));
        return r;
    }
    e = then(e, ended_load((uint8_t)(state - 1)));
    if (state == 1) {
        for (y = 0x10;;) {
            kind = RD8(OBJ->kind[y]);
            e = then(e, ended_with_y(ended_compare(kind, 0x24), y));
            if (kind == 0x24) {
                break;
            }
            y++;
            e = then(e, ended_with_y(l8_compare_y(y, 0x2F), y));
            if (y == 0x2F) {
                r.end = e;
                return r;
            }
        }
        value = RD8(OBJ->frame[y]);
        e = then(e, ended_compare(value, 0x11));
        if (value != 0x11) {
            r.end = e;
            return r;
        }
        value = RD8(OBJ->x_lo[y]);
        e = then(e, ended_compare(value, 0xB8));
        if (value < 0xB8) {
            r.end = e;
            return r;
        }
        high = RD8(OBJ->x_hi[y]);
        e = then(e, ended_load(high));
        if (high != 0) {
            r.end = e;
            return r;
        }
        value = RD8(OBJ->x_lo[0]);
        e = then(e, ended_compare(value, 0xC0));
        if (value < 0xC0) {
            r.end = e;
            return r;
        }
        r.end = then(e, ended_nz(INC8(OBJ->state[slot])));
        return r;
    }
    e = then(e, ended_load((uint8_t)(state - 2)));
    if (state == 2) {
        value = RD8(OBJ->vy_lo[slot]);
        sum = add_bytes(value, 0x20, false);
        WR8(OBJ->vy_lo[slot], sum.value);
        e = then(e, ended_of_sum(sum));
        if (sum.carry) {
            e = then(e, ended_nz(INC8(OBJ->vy_hi[slot])));
        }
        jsr_push(0xAFCB);
        move = ApplyVelocity(slot);
        jsr_pop();
        e = then(e, ended_of_axis_move(move));
        ground = RD8(OBJ->y_lo[slot]);
        e = then(e, ended_compare(ground, 0xBC));
        if (ground < 0xBC) {
            r.end = e;
            return r;
        }
        WR8(OBJ->vy_lo[slot], 0);
        WR8(OBJ->vy_hi[slot], 0xFE);
        e = then(e, ended_load(0xFE));
        r.sounded = true;
        r.command = 0x2C;
        r.before = then(e, ended_load(0x2C));
        jsr_push(0xAFDF);
        r.call = SoundCommand(0x2C, slot, y_entry);
        jsr_pop();
        r.end = ended_nz(INC8(OBJ->state[slot]));
        return r;
    }
    e = then(e, ended_load((uint8_t)(state - 3)));
    if (state != 3) {
        r.end = e;
        return r;
    }
    value = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(value, 0x20, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    e = then(e, ended_of_sum(sum));
    if (sum.carry) {
        e = then(e, ended_nz(INC8(OBJ->vy_hi[slot])));
    }
    jsr_push(0xAFF7);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    high = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_load(high));
    if (high == 0) {
        r.end = e;
        return r;
    }
    r.end = then(e, ended_nz(INC8(OBJ->state[slot])));
    return r;
}

void abi_LoadRecordThenWatchThenDropTwice(void)
{
    abi_sound_end(LoadRecordThenWatchThenDropTwice(cpu.x, cpu.y));
}

/* The four VDC word writes of 0B4A:6B6: the select, the low byte (the object's OBJ_3510, which then grows by $10), the high byte $0F, then the data select and a zero word. */
static Ended write_tile_pair(uint8_t slot)
{
    uint8_t low, grown;
    Sum sum;

    jsr_push(0xAF55);
    SelectVramWriteAddress();
    jsr_pop();
    low = RD8(OBJ->f3510[slot]);
    WR8(VDC->data_lo, low);
    sum = add_bytes(low, 0x10, false);
    WR8(OBJ->f3510[slot], sum.value);
    WR8(VDC->data_hi, 0x0F);
    jsr_push(0xAF66);
    SelectVramData();
    jsr_pop();
    WR8(VDC->data_lo, 0);
    WR8(VDC->data_hi, 0);
    grown = sum.value;
    (void)grown;
    return ended_of_sum(sum);
}

/*
 * @routine 0B4A:6B6 BossPhasesHurtFallAndTileWrites
 * @run 70:AEB6
 * WHAT IT DOES: A boss in phases. State 0 loads the record. State 1: at 1 to 4 hits records the event and goes to state 2; at 5 hits, with the player on the right, takes the X speed 4, OBJ_34B8 $10, records the event, counts OBJ_3568 up and goes to state 5. State 2 steps the list at $AF8A and, when it ends, goes to state 3 with OBJ_34B8 $10 and starts list 0 on channel 5. State 3 waits OBJ_34B8 down, then state 4 with OBJ_34B8 $40 and frame $0F. State 4 counts down, drops one row every eighth frame and writes four tile pairs; at the end it clears the object. States 5 on fall: the X speed loses $0034 a frame until OBJ_34B8 runs out, then state 2.
 * HOW: LoadObjectRecord, RecordObjectEvent, SideOfPlayer, StepObjectFrames, StartListChannelKind3, VDC writes, ApplyVelocity, ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran. Everything past state 0 ran in no script; the draft stops there and the arms are written from the decoded bytes.
 */
Ended BossPhasesHurtFallAndTileWrites(uint8_t slot, uint8_t y_entry)
{
    uint8_t state, last, nz, hits, stored, event, byte, y = y_entry, timer, bits, value, flag;
    bool carry, marked;
    FrameStep step;
    SideResult side;
    AxisMove move;
    Sum diff;
    int round;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        return then(e, ended_of_record(last, nz, carry));
    }
    e = then(e, ended_load((uint8_t)(state - 1)));
    if (state == 1) {
        hits = RD8(OBJ->hit_taken[slot]);
        e = then(e, ended_load(hits));
        if (hits == 0) {
            return e;
        }
        e = then(e, ended_compare(hits, 5));
        if (hits < 5) {
            jsr_push(0xAECD);
            marked = RecordObjectEvent(slot, &stored, &event, &byte);
            jsr_pop();
            e = then(e, l8_of_event(marked, stored, event, byte));
            return then(e, ended_nz(INC8(OBJ->state[slot])));
        }
        jsr_push(0xAED4);
        side = SideOfPlayer(slot);
        jsr_pop();
        e = then(e, ended_with_carry(ended_load((uint8_t)(side.sides >> 1)), (side.sides & 1) != 0));
        if ((side.sides & 1) != 0) {
            return then(e, ended_nz(INC8(OBJ->state[slot])));
        }
        WR8(OBJ->vx_hi[slot], 4);
        WR8(OBJ->f34b8[slot], 0x10);
        e = then(e, ended_load(0x10));
        jsr_push(0xAEE4);
        marked = RecordObjectEvent(slot, &stored, &event, &byte);
        jsr_pop();
        e = then(e, l8_of_event(marked, stored, event, byte));
        e = then(e, ended_nz(INC8(OBJ->f3568[slot])));
        WR8(OBJ->state[slot], 5);
        return then(e, ended_load(5));
    }
    e = then(e, ended_load((uint8_t)(state - 2)));
    if (state == 2) {
        e = then(e, l8_step_list(slot, 0xAF8A, 0xAEFB, true, &y, &step));
        if (step != FRAME_LIST_ENDED) {
            return e;
        }
        e = then(e, ended_nz(INC8(OBJ->state[slot])));
        WR8(OBJ->f34b8[slot], 0x10);
        push8(slot);
        jsr_push(0xAF0C);
        StartListChannelKind3(0, 5);
        jsr_pop();
        slot = pull8();
        return then(e, ended_with_a(ended_nz(slot), 3));
    }
    e = then(e, ended_load((uint8_t)(state - 3)));
    if (state == 3) {
        timer = DEC8(OBJ->f34b8[slot]);
        e = then(e, ended_nz(timer));
        if (timer != 0) {
            return e;
        }
        e = then(e, ended_nz(INC8(OBJ->state[slot])));
        WR8(OBJ->f34b8[slot], 0x40);
        WR8(OBJ->frame[slot], 0x0F);
        return then(e, ended_load(0x0F));
    }
    e = then(e, ended_load((uint8_t)(state - 4)));
    if (state == 4) {
        timer = DEC8(OBJ->f34b8[slot]);
        e = then(e, ended_nz(timer));
        if (timer == 0) {
            flag = RD8(OBJ->f3568[slot]);
            e = then(e, ended_load(flag));
            ClearObject(slot);
            return e;
        }
        bits = RD8(OBJ->f34b8[slot]);
        value = (uint8_t)(bits & 0x07);
        e = then(e, ended_load(value));
        if (value != 0) {
            return e;
        }
        e = then(e, ended_nz(INC8(OBJ->y_lo[slot])));
        bits = RD8(OBJ->f34b8[slot]);
        value = (uint8_t)((bits >> 3) + 0xC8);
        WR8(OBJ->f3510[slot], value);
        for (round = 0; round < 4; round++) {
            e = then(e, write_tile_pair(slot));
        }
        return e;
    }
    jsr_push(0xAF6E);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    value = RD8(OBJ->vx_lo[slot]);
    diff = sub_bytes(value, 0x34, true);
    WR8(OBJ->vx_lo[slot], diff.value);
    value = RD8(OBJ->vx_hi[slot]);
    diff = sub_bytes(value, 0, diff.carry);
    WR8(OBJ->vx_hi[slot], diff.value);
    e = then(e, ended_of_sum(diff));
    timer = DEC8(OBJ->f34b8[slot]);
    e = then(e, ended_nz(timer));
    if (timer != 0) {
        return e;
    }
    WR8(OBJ->state[slot], 2);
    return then(e, ended_load(2));
}

void abi_BossPhasesHurtFallAndTileWrites(void)
{
    abi_ended(BossPhasesHurtFallAndTileWrites(cpu.x, cpu.y));
}
