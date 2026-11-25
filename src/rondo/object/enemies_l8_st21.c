/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 0F6B (enemy kinds not yet identified), by routine. Bodies return how the
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
 * @routine 0F6B:07D LoadRecordSplitIntoFourAndClear
 * @run 70:B07D
 * WHAT IT DOES: Sets OBJ_2FE8 of slot 0 to $C0, loads the record and records the event, starts four objects of type $A6 in the slots after this one at its place (the second and fourth with bit 7 of OBJ_X_LO set, the third and fourth one screen on in X) and clears itself.
 * HOW: LoadObjectRecord, RecordObjectEvent, four StartObjectAtPlace calls, INCs and ORs with X and Y swapped, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended LoadRecordSplitIntoFourAndClear(uint8_t slot)
{
    uint8_t last, nz, stored, event, byte, y, value, placed;
    bool carry, marked;
    Ended e;

    WR8(OBJ->sprite_flip[0], 0xC0);
    jsr_push(0xB085);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    jsr_push(0xB088);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    jsr_pop();
    e = then(e, l8_of_event(marked, stored, event, byte));
    y = (uint8_t)(slot + 1);
    jsr_push(0xB090);
    placed = StartObjectAtPlace(0xA6, slot, y);
    jsr_pop();
    y++;
    jsr_push(0xB096);
    placed = StartObjectAtPlace(0xA6, slot, y);
    jsr_pop();
    value = RD8(OBJ->x_lo[y]);
    WR8(OBJ->x_lo[y], (uint8_t)(value | 0x80));
    y++;
    jsr_push(0xB0A4);
    placed = StartObjectAtPlace(0xA6, slot, y);
    jsr_pop();
    INC8(OBJ->x_hi[y]);
    y++;
    jsr_push(0xB0AF);
    placed = StartObjectAtPlace(0xA6, slot, y);
    jsr_pop();
    INC8(OBJ->x_hi[y]);
    value = RD8(OBJ->x_lo[y]);
    value = (uint8_t)(value | 0x80);
    WR8(OBJ->x_lo[y], value);
    (void)placed;
    ClearObject(slot);
    return then(e, ended_with_y(ended_load(value), y));
}

void abi_LoadRecordSplitIntoFourAndClear(void)
{
    abi_ended(LoadRecordSplitIntoFourAndClear(cpu.x));
}

/*
 * @routine 0F6B:0BF LoadRecordThenFollowScroll
 * @run 70:B0BF
 * WHAT IT DOES: In state 0 loads the record and keeps the object's X in OBJ_34B8 and OBJ_3510. Later it sets its screen X to that spot less the scroll plus a quarter of the scroll (the scroll's pair of bytes shifted right 3 times), so it moves slowly against the scroll.
 * HOW: LoadObjectRecord, two shifts through carry, two 16-bit adds and a 16-bit subtract in memory.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended LoadRecordThenFollowScroll(uint8_t slot)
{
    uint8_t state, last, nz, high, low, value, base_lo, base_hi;
    bool carry, shifted;
    Sum sum, diff;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(0xB0C7);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        e = then(ended_load(state), ended_of_record(last, nz, carry));
        value = RD8(OBJ->x_lo[slot]);
        WR8(OBJ->f34b8[slot], value);
        value = RD8(OBJ->x_hi[slot]);
        WR8(OBJ->f3510[slot], value);
        return then(e, ended_load(value));
    }
    high = RD8(SCROLLE->x_hi_screen[0]);
    shifted = (high & 0x01) != 0;
    low = RD8(SCROLL->x_lo[0]);
    low = (uint8_t)((low >> 1) | (shifted ? 0x80 : 0));
    low = (uint8_t)(low >> 2);
    base_lo = RD8(OBJ->f34b8[slot]);
    sum = add_bytes(low, base_lo, false);
    WR8(OBJ->x_lo[slot], sum.value);
    base_hi = RD8(OBJ->f3510[slot]);
    sum = add_bytes(0, base_hi, sum.carry);
    WR8(OBJ->x_hi[slot], sum.value);
    value = RD8(OBJ->x_lo[slot]);
    diff = sub_bytes(value, RD8(SCROLL->x_lo[0]), true);
    WR8(OBJ->x_lo[slot], diff.value);
    value = RD8(OBJ->x_hi[slot]);
    diff = sub_bytes(value, RD8(SCROLLE->x_hi_screen[0]), diff.carry);
    WR8(OBJ->x_hi[slot], diff.value);
    return ended_of_sum(diff);
}

void abi_LoadRecordThenFollowScroll(void)
{
    abi_ended(LoadRecordThenFollowScroll(cpu.x));
}

/*
 * @routine 0F6B:150 StartPieceWithSpeedFromTable
 * @run 70:B150
 * WHAT IT DOES: Starts an object of type $A4 in a free slot of $20 to $3F at the object's place, $28 lower, with OBJ_3040 the number in zTemp00 plus 3 and the speed from the table at $B18A by twice that number.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace, table reads, stores between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object, zTemp00 the number. Exit: as the last instruction that ran.
 */
Ended StartPieceWithSpeedFromTable(uint8_t slot)
{
    uint8_t found_slot, last, placed, value, number, index;
    bool found;
    Sum sum;
    Ended e;

    jsr_push(0xB153);
    found_slot = FindFreeSlotFrom20(&last, &found);
    jsr_pop();
    e = ended_of_find_slot(found_slot, last, found);
    if (!found) {
        return e;
    }
    jsr_push(0xB15A);
    placed = StartObjectAtPlace(0xA4, slot, found_slot);
    jsr_pop();
    push8(slot);
    value = RD8(OBJ->y_lo[found_slot]);
    sum = add_bytes(value, 0x28, false);
    WR8(OBJ->y_lo[found_slot], sum.value);
    number = RD8(ZP->temp00);
    sum = add_bytes(number, 3, false);
    WR8(OBJ->f3040[found_slot], sum.value);
    number = RD8(ZP->temp00);
    index = (uint8_t)(number << 1);
    value = rd8((uint16_t)(0xB18A + index));
    WR8(OBJ->vx_lo[found_slot], value);
    value = rd8((uint16_t)(0xB18B + index));
    WR8(OBJ->vx_hi[found_slot], value);
    value = rd8((uint16_t)(0xB192 + index));
    WR8(OBJ->vy_lo[found_slot], value);
    value = rd8((uint16_t)(0xB193 + index));
    WR8(OBJ->vy_hi[found_slot], value);
    slot = pull8();
    (void)placed;
    return then(e, ended_with_a(ended_with_overflow(ended_with_carry(ended_nz(slot), number >= 0x80), sum.overflow), value));
}

void abi_StartPieceWithSpeedFromTable(void)
{
    abi_ended(StartPieceWithSpeedFromTable(cpu.x));
}

/*
 * @routine 0F6B:19A LoadRecordFlipByPlayerThenFall
 * @run 70:B19A
 * WHAT IT DOES: In state 0 loads the record, takes OBJ_3040 as its frame and, when the player is on the left side, faces left and negates (bytes inverted) its speed. Later it falls (ApplyGravity) and moves.
 * HOW: LoadObjectRecord, SideOfPlayer, EORs, ApplyGravity, a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended LoadRecordFlipByPlayerThenFall(uint8_t slot)
{
    uint8_t state, last, nz, kind, value;
    bool carry;
    SideResult side;
    GravityResult gravity;
    AxisMove move;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        jsr_push(0xB1C7);
        gravity = ApplyGravity(slot);
        jsr_pop();
        (void)gravity;
        move = ApplyVelocity(slot);
        return ended_of_axis_move(move);
    }
    jsr_push(0xB1A2);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = then(ended_load(state), ended_of_record(last, nz, carry));
    kind = RD8(OBJ->f3040[slot]);
    WR8(OBJ->frame[slot], kind);
    e = then(e, ended_load(kind));
    jsr_push(0xB1AB);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = then(e, ended_with_carry(ended_load((uint8_t)(side.sides >> 1)), (side.sides & 0x01) != 0));
    if ((side.sides & 0x01) == 0) {
        return e;
    }
    WR8(OBJ->facing_left[slot], 1);
    value = (uint8_t)(RD8(OBJ->vx_lo[slot]) ^ 0xFF);
    WR8(OBJ->vx_lo[slot], value);
    value = (uint8_t)(RD8(OBJ->vx_hi[slot]) ^ 0xFF);
    WR8(OBJ->vx_hi[slot], value);
    return then(e, ended_load(value));
}

void abi_LoadRecordFlipByPlayerThenFall(void)
{
    abi_ended(LoadRecordFlipByPlayerThenFall(cpu.x));
}

/*
 * @routine 0F6B:1CA LoadRecordThenClearAtSubStep3
 * @run 70:B1CA
 * WHAT IT DOES: In state 0 loads the record. Later, when the sub-mode's step is 3, clears OBJ_2FE8 of slot 0 and clears the object.
 * HOW: LoadObjectRecord, a compare, a store, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended LoadRecordThenClearAtSubStep3(uint8_t slot)
{
    uint8_t state, last, nz, step;
    bool carry;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        return then(e, ended_of_record(last, nz, carry));
    }
    step = RD8(ZP->sub_step);
    e = ended_compare(step, 3);
    if (step != 3) {
        return e;
    }
    WR8(OBJ->sprite_flip[0], 0);
    ClearObject(slot);
    return e;
}

void abi_LoadRecordThenClearAtSubStep3(void)
{
    abi_ended(LoadRecordThenClearAtSubStep3(cpu.x));
}

/*
 * @routine 0F6B:395 SoundWhenHurtThenFlag
 * @run 70:B395
 * WHAT IT DOES: When bit 7 of OBJ_31A0 is clear and the object has taken 5 hits plays sound $D2 and, when OBJ_32A8 of slot $10 is below 8, sets it to 5, sets OBJ_3618 to 5 and sets bit 7 of OBJ_31A0.
 * HOW: Tests, SoundCommand, stores.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The stores past the last compare ran in no script; the draft stops there.
 */
SoundEnd SoundWhenHurtThenFlag(uint8_t slot, uint8_t y_entry)
{
    uint8_t flags, hits, other, value;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    flags = RD8(OBJ->status_flags[slot]);
    e = ended_load((uint8_t)(flags & 0x80));
    if ((flags & 0x80) != 0) {
        r.end = e;
        return r;
    }
    hits = RD8(OBJ->hit_taken[slot]);
    e = then(e, ended_compare(hits, 5));
    if (hits < 5) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0xD2;
    r.before = then(e, ended_load(0xD2));
    jsr_push(0xB3A8);
    r.call = SoundCommand(0xD2, slot, y_entry);
    jsr_pop();
    other = RD8(OBJ->f32a8[0x10]);
    e = ended_compare(other, 8);
    if (other >= 8) {
        r.end = e;
        return r;
    }
    WR8(OBJ->f32a8[0x10], 5);
    WR8(OBJ->f3618[slot], 5);
    value = RD8(OBJ->status_flags[slot]);
    value = (uint8_t)(0x80 | value);
    WR8(OBJ->status_flags[slot], value);
    r.end = ended_load(value);
    return r;
}

void abi_SoundWhenHurtThenFlag(void)
{
    abi_sound_end(SoundWhenHurtThenFlag(cpu.x, cpu.y));
}

/*
 * @routine 0F6B:3C0 SoundAndState6WhenFlagged
 * @run 70:B3C0
 * WHAT IT DOES: Outside state 6: when bit 7 of OBJ_31A0 is set plays sound $D3, goes to state 6 and copies OBJ_3618 to OBJ_32A8; otherwise copies OBJ_32A8 to OBJ_3618.
 * HOW: Compares, SoundCommand, copies.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The flagged arm ran in no script; the draft stops there.
 */
SoundEnd SoundAndState6WhenFlagged(uint8_t slot, uint8_t y_entry)
{
    uint8_t state, flags, value;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_compare(state, 6);
    if (state == 6) {
        r.end = e;
        return r;
    }
    flags = RD8(OBJ->status_flags[slot]);
    e = then(e, ended_load((uint8_t)(flags & 0x80)));
    if ((flags & 0x80) == 0) {
        value = RD8(OBJ->f32a8[slot]);
        WR8(OBJ->f3618[slot], value);
        r.end = then(e, ended_load(value));
        return r;
    }
    r.sounded = true;
    r.command = 0xD3;
    r.before = then(e, ended_load(0xD3));
    jsr_push(0xB3D3);
    r.call = SoundCommand(0xD3, slot, y_entry);
    jsr_pop();
    WR8(OBJ->state[slot], 6);
    WR8(OBJ->f2800[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    value = RD8(OBJ->f3618[slot]);
    WR8(OBJ->f32a8[slot], value);
    r.end = ended_load(value);
    return r;
}

void abi_SoundAndState6WhenFlagged(void)
{
    abi_sound_end(SoundAndState6WhenFlagged(cpu.x, cpu.y));
}

/*
 * @routine 0F6B:40C LoadRecordFrame0D
 * @run 70:B40C
 * WHAT IT DOES: Loads the object's record and sets its frame $0D.
 * HOW: LoadObjectRecord and a store.
 * WAITS: no.
 * Entry: X object. Exit: A $0D, Y 11, C from the record.
 */
Ended LoadRecordFrame0D(uint8_t slot)
{
    uint8_t last, nz;
    bool carry;
    Ended e;

    jsr_push(0xB40F);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->frame[slot], 0x0D);
    return then(e, ended_load(0x0D));
}

void abi_LoadRecordFrame0D(void)
{
    abi_ended(LoadRecordFrame0D(cpu.x));
}

/*
 * @routine 0F6B:415 ClearFourAtScreen2
 * @run 70:B415
 * WHAT IT DOES: When the sub-mode is 1 and the scroll's screen byte is 2, sets the scroll's X limit low byte $80 and zRasterStage 3, clears slots $40 to $43, gives the object the Y speed -$0080 and goes to the next state.
 * HOW: Two compares, ClearObjectInY four times, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended ClearFourAtScreen2(uint8_t slot)
{
    uint8_t mode, screen, y;
    Ended e;

    mode = RD8(ZP->game_sub_mode);
    e = ended_compare(mode, 1);
    if (mode != 1) {
        return e;
    }
    screen = RD8(SCROLLE->x_hi_screen[0]);
    e = ended_compare(screen, 2);
    if (screen != 2) {
        return e;
    }
    wr8(ScrollXMaxLo, 0x80);
    WR8(ZP->raster_stage, 3);
    e = ended_load(3);
    for (y = 0x40; y < 0x44; y++) {
        jsr_push((uint16_t)(0xB430 + 4 * (y - 0x40)));
        ClearObjectInY(y);
        jsr_pop();
    }
    e = then(e, ended_with_y(ended_nz(0x43), 0x43));
    WR8(OBJ->vy_lo[slot], 0x80);
    WR8(OBJ->vy_hi[slot], 0xFF);
    e = then(e, ended_load(0xFF));
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_ClearFourAtScreen2(void)
{
    abi_ended(ClearFourAtScreen2(cpu.x));
}

/*
 * @routine 0F6B:44A MoveUpUntilY40
 * @run 70:B44A
 * WHAT IT DOES: Moves the object along Y; once its Y low byte is below $40 sets OBJ_34B8 2 and goes to the next state.
 * HOW: ApplyVelocityY, a compare, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended MoveUpUntilY40(uint8_t slot)
{
    uint8_t low;
    AxisMove move;
    Ended e;

    jsr_push(0xB44D);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = ended_of_axis_move(move);
    low = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(low, 0x40));
    if (low >= 0x40) {
        return e;
    }
    WR8(OBJ->f34b8[slot], 2);
    e = then(e, ended_load(2));
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_MoveUpUntilY40(void)
{
    abi_ended(MoveUpUntilY40(cpu.x));
}

/*
 * @routine 0F6B:50B CountDownThenCopyPhase
 * @run 70:B50B
 * WHAT IT DOES: Counts OBJ_3510 down; when it is 0 (or reaches 0) sets OBJ_2800 from OBJ_38D8.
 * HOW: A test, a memory decrement, a copy.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended CountDownThenCopyPhase(uint8_t slot)
{
    uint8_t timer, now, value;
    Ended e;

    timer = RD8(OBJ->f3510[slot]);
    e = ended_load(timer);
    if (timer != 0) {
        now = DEC8(OBJ->f3510[slot]);
        e = then(e, ended_nz(now));
        if (now != 0) {
            return e;
        }
    }
    value = RD8(OBJ->f38d8[slot]);
    WR8(OBJ->f2800[slot], value);
    return then(e, ended_load(value));
}

void abi_CountDownThenCopyPhase(void)
{
    abi_ended(CountDownThenCopyPhase(cpu.x));
}

/*
 * @routine 0F6B:51C ResetPieceAndAdvance
 * @run 70:B51C
 * WHAT IT DOES: Clears OBJ_3778, sets OBJ_37D0 $80 and OBJ_35C0 $E0 and goes to the next OBJ_2800.
 * HOW: Stores and an INC.
 * WAITS: no.
 * Entry: X object. Exit: A $E0, N and Z from the new OBJ_2800.
 */
Ended ResetPieceAndAdvance(uint8_t slot)
{
    WR8(OBJ->f3778[slot], 0);
    WR8(OBJ->f37d0[slot], 0x80);
    WR8(OBJ->f35c0[slot], 0xE0);
    return then(ended_load(0xE0), ended_nz(INC8(OBJ->f2800[slot])));
}

void abi_ResetPieceAndAdvance(void)
{
    abi_ended(ResetPieceAndAdvance(cpu.x));
}

/*
 * @routine 0F6B:100 BreakIntoPiecesAtFifthHit
 * @run 70:B100
 * WHAT IT DOES: In state 0 loads the record. Once the object has taken 5 hits it records its event, plays sound $12, takes the kind of OBJ_3040, sets OBJ_3A90 $10, goes to state 0, drops $28 in Y, clears its frame and starts an object of type $9B in a free slot of $50 to $53, then four pieces (StartPieceWithSpeedFromTable for the numbers 0 to 3).
 * HOW: LoadObjectRecord, RecordObjectEvent, SoundCommand, FindFreeSlotFrom50, StartObjectAtPlace, four calls.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
SoundEnd BreakIntoPiecesAtFifthHit(uint8_t slot, uint8_t y_entry)
{
    uint8_t state, last, nz, hits, stored, event, byte, kind, found_slot, placed, y, low;
    bool carry, marked, found;
    Sum sum;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;
    unsigned piece;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        r.end = then(e, ended_of_record(last, nz, carry));
        return r;
    }
    hits = RD8(OBJ->hit_taken[slot]);
    e = then(e, ended_compare(hits, 5));
    if (hits < 5) {
        r.end = e;
        return r;
    }
    jsr_push(0xB112);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    jsr_pop();
    e = then(e, l8_of_event(marked, stored, event, byte));
    y = e.y_set ? e.y : y_entry;
    r.sounded = true;
    r.command = 0x12;
    r.before = then(e, ended_load(0x12));
    jsr_push(0xB117);
    r.call = SoundCommand(0x12, slot, y);
    jsr_pop();
    kind = RD8(OBJ->f3040[slot]);
    WR8(OBJ->kind[slot], kind);
    WR8(OBJ->timer[slot], 0x10);
    WR8(OBJ->state[slot], 0);
    low = RD8(OBJ->y_lo[slot]);
    sum = add_bytes(low, 0xD8, false);
    WR8(OBJ->y_lo[slot], sum.value);
    WR8(OBJ->frame[slot], 0);
    e = ended_of_sum(sum);
    jsr_push(0xB134);
    found_slot = FindFreeSlotFrom50(&last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, last, found));
    if (!found) {
        r.end = e;
        return r;
    }
    jsr_push(0xB13B);
    placed = StartObjectAtPlace(0x9B, slot, found_slot);
    jsr_pop();
    e = then(e, ended_load(placed));
    WR8(ZP->temp00, 0);
    for (piece = 0; piece < 4; piece++) {
        jsr_push((uint16_t)(0xB140 + 5 * piece));
        e = then(e, StartPieceWithSpeedFromTable(slot));
        jsr_pop();
        if (piece < 3) {
            e = then(e, ended_nz(INC8(ZP->temp00)));
        }
    }
    r.end = e;
    return r;
}

void abi_BreakIntoPiecesAtFifthHit(void)
{
    abi_sound_end(BreakIntoPiecesAtFifthHit(cpu.x, cpu.y));
}
