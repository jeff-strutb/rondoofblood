/*
 * TEMPORARY: the stage object routines of lot L8 for the overlays at sectors 0B4D to 0EBF (enemy kinds not yet identified), by routine. Bodies return how the
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
 * @routine 0B4D:0A9 RecordEventWhenSlot10InState0F
 * @run 71:C0A9
 * WHAT IT DOES: When the state of slot $10 is $0F records the object's event, takes OBJ_34B8 from the table at $C0C0 by OBJ_3040 and goes to the next state.
 * HOW: A compare, RecordObjectEvent, a table read, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. The arm past the compare ran in no script; the draft stops there.
 */
Ended RecordEventWhenSlot10InState0F(uint8_t slot)
{
    uint8_t state, stored, event, byte, kind, value;
    bool marked;
    Ended e;

    state = RD8(OBJ->state[0x10]);
    e = ended_compare(state, 0x0F);
    if (state != 0x0F) {
        return e;
    }
    jsr_push(0xC0B3);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    jsr_pop();
    e = then(e, l8_of_event(marked, stored, event, byte));
    kind = RD8(OBJ->f3040[slot]);
    e = then(e, ended_with_y(ended_load(kind), kind));
    value = rd8((uint16_t)(0xC0C0 + kind));
    WR8(OBJ->f34b8[slot], value);
    e = then(e, ended_load(value));
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_RecordEventWhenSlot10InState0F(void)
{
    abi_ended(RecordEventWhenSlot10InState0F(cpu.x));
}

/*
 * @routine 0E0B:071 FaceThePlayerByStateTableB
 * @run 70:B071
 * WHAT IT DOES: Every eighth frame of OBJ_3568, in the states the table at $B089 marks, the object takes the facing of the player's side.
 * HOW: An AND, a table read by the state, SideOfPlayer, a store.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended FaceThePlayerByStateTableB(uint8_t slot)
{
    uint8_t bits, y, marked, facing;
    SideResult side;
    Ended e;

    bits = (uint8_t)(RD8(OBJ->f3568[slot]) & 0x07);
    e = ended_load(bits);
    if (bits != 0) {
        return e;
    }
    y = RD8(OBJ->state[slot]);
    e = then(e, ended_with_y(ended_nz(y), y));
    marked = rd8((uint16_t)(0xB089 + y));
    e = then(e, ended_load(marked));
    if (marked == 0) {
        return e;
    }
    jsr_push(0xB083);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_side(side));
    facing = side.sides & 0x01;
    WR8(OBJ->facing_left[slot], facing);
    return then(e, ended_load(facing));
}

void abi_FaceThePlayerByStateTableB(void)
{
    abi_ended(FaceThePlayerByStateTableB(cpu.x));
}

/*
 * @routine 0EB9:2C5 MaybeDropItemByPlayerDistance
 * @run 70:A2C5
 * WHAT IT DOES: An object with 5 or more hits whose OBJ_31F8 bits 2 to 4 are not 0 draws a random number against the table at $AC70 by those bits; when the number is below it, it picks an item from the table at $AC78 by the player's distance and starts it in a free slot of $30 to $3F at its place.
 * HOW: NextRandom, PlayerDistanceSum, a table read, FindFreeSlotFrom30, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. The drop arm ran in no script; the draft stops there.
 */
Ended MaybeDropItemByPlayerDistance(uint8_t slot)
{
    uint8_t hits, raw, bits, level, row, threshold, item, found_slot, last, placed, value;
    bool found;
    RandomResult random;
    Sum sum, joined;
    Ended e;

    hits = RD8(OBJ->hit_taken[slot]);
    e = ended_compare(hits, 5);
    if (hits < 5) {
        return e;
    }
    raw = RD8(OBJ->f31f8[slot]);
    bits = (uint8_t)((raw >> 2) & 0x07);
    e = then(e, ended_with_carry(ended_load(bits), ((raw >> 1) & 0x01) != 0));
    if (bits == 0) {
        return e;
    }
    level = (uint8_t)(bits - 1);
    e = then(e, ended_with_y(ended_load(level), level));
    row = (uint8_t)(level << 4);
    e = then(e, ended_with_carry(ended_load(row), false));
    WR8(ZP->temp00, row);
    jsr_push(0xA2E0);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    threshold = rd8((uint16_t)(0xAC70 + level));
    e = then(e, ended_compare(random.value, threshold));
    if (random.value >= threshold) {
        return e;
    }
    jsr_push(0xA2E8);
    sum = PlayerDistanceSum(slot, false);
    jsr_pop();
    e = then(e, ended_of_sum(sum));
    value = (uint8_t)(sum.value & 0x0F);
    e = then(e, ended_load(value));
    joined = add_bytes(value, RD8(ZP->temp00), false);
    e = then(e, ended_of_sum(joined));
    level = joined.value;
    e = then(e, ended_with_y(ended_kept(), level));
    item = rd8((uint16_t)(0xAC78 + level));
    WR8(ZP->temp00, item);
    e = then(e, ended_load(item));
    jsr_push(0xA2F6);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, last, found));
    if (!found) {
        return e;
    }
    item = RD8(ZP->temp00);
    jsr_push(0xA2FD);
    placed = StartObjectAtPlace(item, slot, found_slot);
    jsr_pop();
    return then(then(e, ended_load(item)), ended_load(placed));
}

void abi_MaybeDropItemByPlayerDistance(void)
{
    abi_ended(MaybeDropItemByPlayerDistance(cpu.x));
}

/*
 * @routine 0EBE:66D HitBoxByFrameTableCE8F
 * @run 71:CE6D
 * WHAT IT DOES: Takes the hit box (OBJ_3930, OBJ_39E0, OBJ_3988, OBJ_3A38) from a four byte row of the table at $CE8F picked by the frame less $45.
 * HOW: A subtract, two shifts, four table reads, stores.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte, Y the row, C from the shift.
 */
Ended HitBoxByFrameTableCE8F(uint8_t slot)
{
    uint8_t frame, row, value;
    Sum diff;
    Ended e;

    frame = RD8(OBJ->frame[slot]);
    diff = sub_bytes(frame, 0x45, true);
    row = (uint8_t)(diff.value << 2);
    e = ended_with_y(ended_with_overflow(ended_with_carry(ended_load(row), (diff.value & 0x40) != 0), diff.overflow), row);
    value = rd8((uint16_t)(0xCE8F + row));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(0xCE90 + row));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(0xCE91 + row));
    WR8(OBJ->hitbox_half_w[slot], value);
    value = rd8((uint16_t)(0xCE92 + row));
    WR8(OBJ->hitbox_half_h[slot], value);
    return then(e, ended_load(value));
}

void abi_HitBoxByFrameTableCE8F(void)
{
    abi_ended(HitBoxByFrameTableCE8F(cpu.x));
}

/*
 * @routine 0EBE:6AB StepListFacePlayerUntilNear
 * @run 71:CEAB
 * WHAT IT DOES: Steps the animation list at $CECF while it faces the player; once the player is within $60 in X it clears the animation and goes to the next state.
 * HOW: StepObjectFrames, SideOfPlayer, DistanceXToPlayer, a compare, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListFacePlayerUntilNear(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, facing;
    FrameStep step;
    SideResult side;
    PlayerDistance distance;
    Ended e = l8_step_list(slot, 0xCECF, 0xCEB6, true, &y, &step);

    jsr_push(0xCEB9);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_side(side));
    facing = side.sides & 0x01;
    e = then(e, ended_load(facing));
    WR8(OBJ->facing_left[slot], facing);
    jsr_push(0xCEC1);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_distance(distance));
    e = then(e, ended_compare(distance.distance, 0x60));
    if (distance.distance >= 0x60) {
        return e;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_StepListFacePlayerUntilNear(void)
{
    abi_ended(StepListFacePlayerUntilNear(cpu.x, cpu.y));
}

/*
 * @routine 0EBE:6F6 FacePlayerThenPickPhase
 * @run 71:CEF6
 * WHAT IT DOES: Takes the facing of the player's side and sets OBJ_2800 from the table at $CF0E by the low four bits of OBJ_34B8, which then counts up.
 * HOW: SideOfPlayer, a table read, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: A the table byte, Y the index, N and Z from the new OBJ_34B8.
 */
Ended FacePlayerThenPickPhase(uint8_t slot)
{
    uint8_t facing, index, value;
    SideResult side;
    Ended e;

    jsr_push(0xCEF9);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = ended_of_side(side);
    facing = side.sides & 0x01;
    e = then(e, ended_load(facing));
    WR8(OBJ->facing_left[slot], facing);
    index = (uint8_t)(RD8(OBJ->f34b8[slot]) & 0x0F);
    e = then(e, ended_with_y(ended_load(index), index));
    value = rd8((uint16_t)(0xCF0E + index));
    WR8(OBJ->f2800[slot], value);
    e = then(e, ended_load(value));
    return then(e, ended_nz(INC8(OBJ->f34b8[slot])));
}

void abi_FacePlayerThenPickPhase(void)
{
    abi_ended(FacePlayerThenPickPhase(cpu.x));
}

/*
 * @routine 0EBE:71E LeapByPlayerDistance
 * @run 71:CF1E
 * WHAT IT DOES: When the player is $40 or more away in X clears OBJ_3568 and gives the object the speed X -$01C0, Y -$00C0; when nearer plays sound $C6, sets OBJ_3568 1 and the speed X -$00E0, Y -$0400. Goes to the next OBJ_2800 and, facing left, reverses the X speed.
 * HOW: DistanceXToPlayer, SoundCommand, stores, an INC, NegateVelocityX.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The near arm and the reverse ran in no script; the draft stops there.
 */
SoundEnd LeapByPlayerDistance(uint8_t slot, uint8_t y_entry)
{
    uint8_t facing;
    PlayerDistance distance;
    Sum negated;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    jsr_push(0xCF21);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = ended_of_distance(distance);
    e = then(e, ended_compare(distance.distance, 0x40));
    if (distance.distance >= 0x40) {
        WR8(OBJ->f3568[slot], 0);
        WR8(OBJ->vx_hi[slot], 0xFE);
        WR8(OBJ->vx_lo[slot], 0x40);
        WR8(OBJ->vy_hi[slot], 0xFF);
        WR8(OBJ->vy_lo[slot], 0x40);
        e = then(e, ended_load(0x40));
    } else {
        r.sounded = true;
        r.command = 0xC6;
        r.before = then(e, ended_load(0xC6));
        jsr_push(0xCF43);
        r.call = SoundCommand(0xC6, slot, y_entry);
        jsr_pop();
        WR8(OBJ->f3568[slot], 1);
        WR8(OBJ->vx_hi[slot], 0xFF);
        WR8(OBJ->vx_lo[slot], 0x20);
        WR8(OBJ->vy_hi[slot], 0xFC);
        WR8(OBJ->vy_lo[slot], 0x00);
        e = ended_load(0x00);
    }
    e = then(e, ended_nz(INC8(OBJ->f2800[slot])));
    facing = RD8(OBJ->facing_left[slot]);
    e = then(e, ended_load(facing));
    if (facing != 0) {
        jsr_push(0xCF67);
        negated = NegateVelocityX(slot);
        jsr_pop();
        e = then(e, ended_of_sum(negated));
    }
    r.end = e;
    return r;
}

void abi_LeapByPlayerDistance(void)
{
    abi_sound_end(LeapByPlayerDistance(cpu.x, cpu.y));
}

/*
 * @routine 0EBE:7B9 CountDownThenClearPhase
 * @run 71:CFB9
 * WHAT IT DOES: Counts OBJ_3510 down; at zero clears OBJ_2800.
 * HOW: A memory decrement and a store.
 * WAITS: no.
 * Entry: X object. Exit: the decrement's N and Z. The store ran in no script; the draft stops there.
 */
Ended CountDownThenClearPhase(uint8_t slot)
{
    uint8_t timer = DEC8(OBJ->f3510[slot]);

    if (timer == 0) {
        WR8(OBJ->f2800[slot], 0);
    }
    return ended_nz(timer);
}

void abi_CountDownThenClearPhase(void)
{
    abi_ended(CountDownThenClearPhase(cpu.x));
}

/* The launch the state 0 and the state $02 arms of 0EBF:006 share: the speed X -$0170, Y -$0120, reversed in X when facing left. */
static Ended launch_up_and_aside(uint8_t slot)
{
    uint8_t facing;
    Sum negated;
    Ended e;

    WR8(OBJ->vx_hi[slot], 0xFE);
    WR8(OBJ->vx_lo[slot], 0x90);
    WR8(OBJ->vy_hi[slot], 0xFF);
    WR8(OBJ->vy_lo[slot], 0xE0);
    facing = RD8(OBJ->facing_left[slot]);
    e = ended_load(facing);
    if (facing == 0) {
        return then(ended_load(0xE0), e);
    }
    negated = NegateVelocityX(slot);
    return then(e, ended_of_sum(negated));
}

/*
 * @routine 0EBF:006 HopBetweenSidesWithBox
 * @run 71:D006
 * WHAT IT DOES: Sets the hit box by the frame (HitBoxByFrameTableCE8F). In state 0 loads the record, picks a side at random, places the object from the table at $D0BD (X) and a random Y near $00DC, and launches it up and aside. In state 1 it steps the list at $D0B8 and moves; its X speed grows by the table at $D0C1 for its side until it changes sign, then it stops, takes the Y speed $FFC0, OBJ_34B8 $10 and goes to state 2. In later states it counts OBJ_34B8 down while it moves in Y and at zero goes back to state 1 and launches again.
 * HOW: HitBoxByFrameTableCE8F, LoadObjectRecord, NextRandom twice, StepObjectFrames, ApplyVelocityY and X, 16-bit adds, NegateVelocityX.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended HopBetweenSidesWithBox(uint8_t slot, uint8_t y_entry)
{
    uint8_t state, last, nz, side, y = y_entry, value, old, timer;
    bool carry;
    FrameStep step;
    RandomResult random;
    AxisMove move;
    Sum sum;
    Ended e;

    jsr_push(0xD009);
    e = HitBoxByFrameTableCE8F(slot);
    jsr_pop();
    state = RD8(OBJ->state[slot]);
    e = then(e, ended_load(state));
    if (state == 0) {
        jsr_push(0xD011);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        e = then(e, ended_of_record(last, nz, carry));
        jsr_push(0xD014);
        random = NextRandom();
        jsr_pop();
        e = then(e, ended_of_random(random));
        side = random.value & 0x01;
        WR8(OBJ->facing_left[slot], side);
        y = (uint8_t)(side << 1);
        value = rd8((uint16_t)(0xD0BD + y));
        WR8(OBJ->x_lo[slot], value);
        value = rd8((uint16_t)(0xD0BE + y));
        WR8(OBJ->x_hi[slot], value);
        jsr_push(0xD02A);
        random = NextRandom();
        jsr_pop();
        value = (uint8_t)((random.value & 0x1F) - 0x10 + 0xDC);
        WR8(OBJ->y_lo[slot], value);
        return then(e, launch_up_and_aside(slot));
    }
    e = then(e, ended_load((uint8_t)(state - 1)));
    if (state == 1) {
        e = then(e, l8_step_list(slot, 0xD0B8, 0xD060, false, &y, &step));
        jsr_push(0xD063);
        move = ApplyVelocityY(slot);
        jsr_pop();
        jsr_push(0xD066);
        move = ApplyVelocityX(slot);
        jsr_pop();
        e = then(e, ended_of_axis_move(move));
        old = RD8(OBJ->vx_hi[slot]);
        WR8(ZP->temp00, old);
        value = RD8(OBJ->facing_left[slot]);
        y = (uint8_t)(value << 1);
        value = RD8(OBJ->vx_lo[slot]);
        sum = add_bytes(value, rd8((uint16_t)(0xD0C1 + y)), false);
        WR8(OBJ->vx_lo[slot], sum.value);
        value = RD8(OBJ->vx_hi[slot]);
        sum = add_bytes(value, rd8((uint16_t)(0xD0C2 + y)), sum.carry);
        WR8(OBJ->vx_hi[slot], sum.value);
        e = then(e, ended_of_sum(sum));
        value = (uint8_t)((sum.value ^ RD8(ZP->temp00)) & 0x80);
        e = then(e, ended_load(value));
        if (value == 0) {
            return e;
        }
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0);
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0xC0);
        WR8(OBJ->f34b8[slot], 0x10);
        e = then(e, ended_load(0x10));
        return then(e, ended_nz(INC8(OBJ->state[slot])));
    }
    timer = DEC8(OBJ->f34b8[slot]);
    e = then(e, ended_nz(timer));
    if (timer != 0) {
        move = ApplyVelocityY(slot);
        return then(e, ended_of_axis_move(move));
    }
    WR8(OBJ->state[slot], 1);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    return then(then(e, ended_load(1)), launch_up_and_aside(slot));
}

void abi_HopBetweenSidesWithBox(void)
{
    abi_ended(HopBetweenSidesWithBox(cpu.x, cpu.y));
}
