/*
 * TEMPORARY: the stage object routines of lot L8 for the overlays at sectors 0B4D to 0EBF (enemy kinds not yet identified), by routine. Bodies return the
 * arm they ended on and the values behind it (the types in stage_enemies_l8.h); each routine's exit shim builds A, Y and the flags from them, and the
 * abi_ wrappers set the registers.
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

enum { EV_OTHER_STATE, EV_RECORDED };
enum { FACE_NOT_EIGHTH, FACE_UNMARKED, FACE_FACED };
enum { DROP_FEW_HITS, DROP_NO_BITS, DROP_NOT_DRAWN, DROP_NO_SLOT, DROP_PLACED };
enum { NEAR_FAR, NEAR_CLOSE };
enum { LEAP_FAR, LEAP_NEAR };
enum { HOP_START, HOP_SAME_SIGN, HOP_STOPPED, HOP_COUNTING, HOP_RELAUNCH };
enum {
    SPAWN_LOADED, SPAWN_FRAME4, SPAWN_WAITING, SPAWN_SCREEN_LOW, SPAWN_SCREEN_3, SPAWN_SCREEN_HIGH, SPAWN_PAIRS_FULL, SPAWN_TYPE_2C,
    SPAWN_NO_SLOT, SPAWN_TYPE_CB, SPAWN_AIM_HIGH, SPAWN_GROUPS_FULL, SPAWN_GROUP
};

/*
 * @routine 0B4D:0A9 RecordEventWhenSlot10InState0F
 * @run 71:C0A9
 * WHAT IT DOES: When the state of slot $10 is $0F records the object's event, takes OBJ_34B8 from the table at $C0C0 by OBJ_3040 and goes to the next state.
 * HOW: A compare, RecordObjectEvent, a table read, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_record_event_exit_shim). The arm past the compare ran in no script; the draft stops there.
 */
EventArm RecordEventWhenSlot10InState0F(uint8_t slot)
{
    EventArm r = { 0 };

    r.state = RD8(OBJ->state[0x10]);
    if (r.state != 0x0F) {
        r.arm = EV_OTHER_STATE;
        return r;
    }
    jsr_push(0xC0B3);
    r.event.marked = RecordObjectEvent(slot, &r.event.stored, &r.event.event, &r.event.byte);
    jsr_pop();
    r.kind = RD8(OBJ->f3040[slot]);
    r.value = rd8((uint16_t)(0xC0C0 + r.kind));
    WR8(OBJ->f34b8[slot], r.value);
    r.counted = INC8(OBJ->state[slot]);
    r.arm = EV_RECORDED;
    return r;
}

static Ended l8_record_event_exit_shim(EventArm r)
{
    Ended e = ended_compare(r.state, 0x0F);

    if (r.arm == EV_OTHER_STATE) {
        return e;
    }
    e = then(e, l8_of_event(r.event.marked, r.event.stored, r.event.event, r.event.byte));
    e = then(e, ended_with_y(ended_load(r.kind), r.kind));
    e = then(e, ended_load(r.value));
    return then(e, ended_nz(r.counted));
}

void abi_RecordEventWhenSlot10InState0F(void)
{
    abi_ended(l8_record_event_exit_shim(RecordEventWhenSlot10InState0F(cpu.x)));
}

/*
 * @routine 0E0B:071 FaceThePlayerByStateTableB
 * @run 70:B071
 * WHAT IT DOES: Every eighth frame of OBJ_3568, in the states the table at $B089 marks, the object takes the facing of the player's side.
 * HOW: An AND, a table read by the state, SideOfPlayer, a store.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_face_by_table_b_exit_shim).
 */
FaceArm FaceThePlayerByStateTableB(uint8_t slot)
{
    FaceArm r = { 0 };

    r.bits = (uint8_t)(RD8(OBJ->f3568[slot]) & 0x07);
    if (r.bits != 0) {
        r.arm = FACE_NOT_EIGHTH;
        return r;
    }
    r.state = RD8(OBJ->state[slot]);
    r.marked = rd8((uint16_t)(0xB089 + r.state));
    if (r.marked == 0) {
        r.arm = FACE_UNMARKED;
        return r;
    }
    jsr_push(0xB083);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.facing = r.side.sides & 0x01;
    WR8(OBJ->facing_left[slot], r.facing);
    r.arm = FACE_FACED;
    return r;
}

static Ended l8_face_by_table_b_exit_shim(FaceArm r)
{
    Ended e = ended_load(r.bits);

    if (r.arm == FACE_NOT_EIGHTH) {
        return e;
    }
    e = then(e, ended_with_y(ended_nz(r.state), r.state));
    e = then(e, ended_load(r.marked));
    if (r.arm == FACE_UNMARKED) {
        return e;
    }
    e = then(e, ended_of_side(r.side));
    return then(e, ended_load(r.facing));
}

void abi_FaceThePlayerByStateTableB(void)
{
    abi_ended(l8_face_by_table_b_exit_shim(FaceThePlayerByStateTableB(cpu.x)));
}

/*
 * @routine 0EB9:2C5 MaybeDropItemByPlayerDistance
 * @run 70:A2C5
 * WHAT IT DOES: An object with 5 or more hits whose OBJ_HIT_REACTION bits 2 to 4 are not 0 draws a random number against the table at $AC70 by those bits; when the number is below it, it picks an item from the table at $AC78 by the player's distance and starts it in a free slot of $30 to $3F at its place.
 * HOW: NextRandom, PlayerDistanceSum, a table read, FindFreeSlotFrom30, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_drop_item_exit_shim). The drop arm ran in no script; the draft stops there.
 */
DropArm MaybeDropItemByPlayerDistance(uint8_t slot)
{
    DropArm r = { 0 };
    uint8_t raw;

    r.hits = RD8(OBJ->hit_taken[slot]);
    if (r.hits < 5) {
        r.arm = DROP_FEW_HITS;
        return r;
    }
    raw = RD8(OBJ->hit_reaction[slot]);
    r.bits = (uint8_t)((raw >> 2) & 0x07);
    r.bit1 = ((raw >> 1) & 0x01) != 0;
    if (r.bits == 0) {
        r.arm = DROP_NO_BITS;
        return r;
    }
    r.level = (uint8_t)(r.bits - 1);
    r.row = (uint8_t)(r.level << 4);
    WR8(ZP->temp00, r.row);
    jsr_push(0xA2E0);
    r.random = NextRandom();
    jsr_pop();
    r.threshold = rd8((uint16_t)(0xAC70 + r.level));
    if (r.random.value >= r.threshold) {
        r.arm = DROP_NOT_DRAWN;
        return r;
    }
    jsr_push(0xA2E8);
    r.sum = PlayerDistanceSum(slot, false);
    jsr_pop();
    r.value = (uint8_t)(r.sum.value & 0x0F);
    r.joined = add_bytes(r.value, RD8(ZP->temp00), false);
    r.item = rd8((uint16_t)(0xAC78 + r.joined.value));
    WR8(ZP->temp00, r.item);
    jsr_push(0xA2F6);
    r.found = l8_free_slot30();
    jsr_pop();
    if (!r.found.found) {
        r.arm = DROP_NO_SLOT;
        return r;
    }
    r.item2 = RD8(ZP->temp00);
    jsr_push(0xA2FD);
    r.placed = StartObjectAtPlace(r.item2, slot, r.found.slot);
    jsr_pop();
    r.arm = DROP_PLACED;
    return r;
}

Ended l8_drop_item_exit_shim(DropArm r)
{
    Ended e = ended_compare(r.hits, 5);

    if (r.arm == DROP_FEW_HITS) {
        return e;
    }
    e = then(e, ended_with_carry(ended_load(r.bits), r.bit1));
    if (r.arm == DROP_NO_BITS) {
        return e;
    }
    e = then(e, ended_with_y(ended_load(r.level), r.level));
    e = then(e, ended_with_carry(ended_load(r.row), false));
    e = then(e, ended_of_random(r.random));
    e = then(e, ended_compare(r.random.value, r.threshold));
    if (r.arm == DROP_NOT_DRAWN) {
        return e;
    }
    e = then(e, ended_of_sum(r.sum));
    e = then(e, ended_load(r.value));
    e = then(e, ended_of_sum(r.joined));
    e = then(e, ended_with_y(ended_kept(), r.joined.value));
    e = then(e, ended_load(r.item));
    e = then(e, l8_slot_ended(r.found));
    if (r.arm == DROP_NO_SLOT) {
        return e;
    }
    return then(then(e, ended_load(r.item2)), ended_load(r.placed));
}

void abi_MaybeDropItemByPlayerDistance(void)
{
    abi_ended(l8_drop_item_exit_shim(MaybeDropItemByPlayerDistance(cpu.x)));
}

/*
 * @routine 0EBE:66D HitBoxByFrameTableCE8F
 * @run 71:CE6D
 * WHAT IT DOES: Takes the hit box (OBJ_HITBOX_DX, OBJ_HITBOX_DY, OBJ_HITBOX_HALF_W, OBJ_HITBOX_HALF_H) from a four byte row of the table at $CE8F picked by the frame less $45.
 * HOW: A subtract, two shifts, four table reads, stores.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte, Y the row, C from the shift (l8_hit_box_ce8f_exit_shim). The body returns the subtract (sum), the row (first)
 * and the last byte (second).
 */
L8Arm HitBoxByFrameTableCE8F(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t frame, value;

    frame = RD8(OBJ->frame[slot]);
    r.sum = sub_bytes(frame, 0x45, true);
    r.first = (uint8_t)(r.sum.value << 2);
    value = rd8((uint16_t)(0xCE8F + r.first));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(0xCE90 + r.first));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(0xCE91 + r.first));
    WR8(OBJ->hitbox_half_w[slot], value);
    value = rd8((uint16_t)(0xCE92 + r.first));
    WR8(OBJ->hitbox_half_h[slot], value);
    r.second = value;
    return r;
}

static Ended l8_hit_box_ce8f_exit_shim(L8Arm r)
{
    Ended e = ended_with_y(ended_with_overflow(ended_with_carry(ended_load(r.first), (r.sum.value & 0x40) != 0), r.sum.overflow), r.first);

    return then(e, ended_load(r.second));
}

void abi_HitBoxByFrameTableCE8F(void)
{
    abi_ended(l8_hit_box_ce8f_exit_shim(HitBoxByFrameTableCE8F(cpu.x)));
}

/*
 * @routine 0EBE:6AB StepListFacePlayerUntilNear
 * @run 71:CEAB
 * WHAT IT DOES: Steps the animation list at $CECF while it faces the player; once the player is within $60 in X it clears the animation and goes to the next state.
 * HOW: StepObjectFrames, SideOfPlayer, DistanceXToPlayer, a compare, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_face_until_near_exit_shim).
 */
NearArm StepListFacePlayerUntilNear(uint8_t slot, uint8_t y_entry)
{
    NearArm r = { 0 };

    r.step = l8_list_step(slot, 0xCECF, 0xCEB6, true, y_entry);
    jsr_push(0xCEB9);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.facing = r.side.sides & 0x01;
    WR8(OBJ->facing_left[slot], r.facing);
    jsr_push(0xCEC1);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    if (r.distance.distance >= 0x60) {
        r.arm = NEAR_FAR;
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.counted = INC8(OBJ->state[slot]);
    r.arm = NEAR_CLOSE;
    return r;
}

static Ended l8_face_until_near_exit_shim(NearArm r)
{
    Ended e = l8_step_ended(r.step);

    e = then(e, ended_of_side(r.side));
    e = then(e, ended_load(r.facing));
    e = then(e, ended_of_distance(r.distance));
    e = then(e, ended_compare(r.distance.distance, 0x60));
    if (r.arm == NEAR_FAR) {
        return e;
    }
    return then(e, ended_nz(r.counted));
}

void abi_StepListFacePlayerUntilNear(void)
{
    abi_ended(l8_face_until_near_exit_shim(StepListFacePlayerUntilNear(cpu.x, cpu.y)));
}

/*
 * @routine 0EBE:6F6 FacePlayerThenPickPhase
 * @run 71:CEF6
 * WHAT IT DOES: Takes the facing of the player's side and sets OBJ_SUBSTATE from the table at $CF0E by the low four bits of OBJ_34B8, which then counts up.
 * HOW: SideOfPlayer, a table read, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: A the table byte, Y the index, N and Z from the new OBJ_34B8 (l8_pick_phase_exit_shim).
 */
PhaseArm FacePlayerThenPickPhase(uint8_t slot)
{
    PhaseArm r = { 0 };

    jsr_push(0xCEF9);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.facing = r.side.sides & 0x01;
    WR8(OBJ->facing_left[slot], r.facing);
    r.index = (uint8_t)(RD8(OBJ->f34b8[slot]) & 0x0F);
    r.value = rd8((uint16_t)(0xCF0E + r.index));
    WR8(OBJ->substate[slot], r.value);
    r.counted = INC8(OBJ->f34b8[slot]);
    return r;
}

static Ended l8_pick_phase_exit_shim(PhaseArm r)
{
    Ended e = ended_of_side(r.side);

    e = then(e, ended_load(r.facing));
    e = then(e, ended_with_y(ended_load(r.index), r.index));
    e = then(e, ended_load(r.value));
    return then(e, ended_nz(r.counted));
}

void abi_FacePlayerThenPickPhase(void)
{
    abi_ended(l8_pick_phase_exit_shim(FacePlayerThenPickPhase(cpu.x)));
}

/*
 * @routine 0EBE:71E LeapByPlayerDistance
 * @run 71:CF1E
 * WHAT IT DOES: When the player is $40 or more away in X clears OBJ_3568 and gives the object the speed X -$01C0, Y -$00C0; when nearer plays sound $C6, sets OBJ_3568 1 and the speed X -$00E0, Y -$0400. Goes to the next OBJ_SUBSTATE and, facing left, reverses the X speed.
 * HOW: DistanceXToPlayer, SoundCommand, stores, an INC, NegateVelocityX.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_leap_by_distance_exit_shim). The near arm and the reverse ran in no
 * script; the draft stops there.
 */
LeapArm LeapByPlayerDistance(uint8_t slot, uint8_t y_entry)
{
    LeapArm r = { 0 };

    jsr_push(0xCF21);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    if (r.distance.distance >= 0x40) {
        WR8(OBJ->f3568[slot], 0);
        WR8(OBJ->vx_hi[slot], 0xFE);
        WR8(OBJ->vx_lo[slot], 0x40);
        WR8(OBJ->vy_hi[slot], 0xFF);
        WR8(OBJ->vy_lo[slot], 0x40);
        r.arm = LEAP_FAR;
    } else {
        jsr_push(0xCF43);
        r.call = SoundCommand(0xC6, slot, y_entry);
        jsr_pop();
        WR8(OBJ->f3568[slot], 1);
        WR8(OBJ->vx_hi[slot], 0xFF);
        WR8(OBJ->vx_lo[slot], 0x20);
        WR8(OBJ->vy_hi[slot], 0xFC);
        WR8(OBJ->vy_lo[slot], 0x00);
        r.arm = LEAP_NEAR;
    }
    r.counted = INC8(OBJ->substate[slot]);
    r.facing = RD8(OBJ->facing_left[slot]);
    if (r.facing != 0) {
        jsr_push(0xCF67);
        r.negated = NegateVelocityX(slot);
        jsr_pop();
        r.negated_used = true;
    }
    return r;
}

static SoundEnd l8_leap_by_distance_exit_shim(LeapArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_of_distance(r.distance);

    e = then(e, ended_compare(r.distance.distance, 0x40));
    if (r.arm == LEAP_FAR) {
        e = then(e, ended_load(0x40));
    } else {
        s.sounded = true;
        s.command = 0xC6;
        s.before = then(e, ended_load(0xC6));
        s.call = r.call;
        e = ended_load(0x00);
    }
    e = then(e, ended_nz(r.counted));
    e = then(e, ended_load(r.facing));
    if (r.negated_used) {
        e = then(e, ended_of_sum(r.negated));
    }
    s.end = e;
    return s;
}

void abi_LeapByPlayerDistance(void)
{
    abi_sound_end(l8_leap_by_distance_exit_shim(LeapByPlayerDistance(cpu.x, cpu.y)));
}

/*
 * @routine 0EBE:7B9 CountDownThenClearPhase
 * @run 71:CFB9
 * WHAT IT DOES: Counts OBJ_3510 down; at zero clears OBJ_SUBSTATE.
 * HOW: A memory decrement and a store.
 * WAITS: no.
 * Entry: X object. Exit: the decrement's N and Z (the body returns the count). The store ran in no script; the draft stops there.
 */
uint8_t CountDownThenClearPhase(uint8_t slot)
{
    uint8_t timer = DEC8(OBJ->f3510[slot]);

    if (timer == 0) {
        WR8(OBJ->substate[slot], 0);
    }
    return timer;
}

static Ended l8_clear_phase_exit_shim(uint8_t timer)
{
    return ended_nz(timer);
}

void abi_CountDownThenClearPhase(void)
{
    abi_ended(l8_clear_phase_exit_shim(CountDownThenClearPhase(cpu.x)));
}

/* The launch the state 0 and the state $02 arms of 0EBF:006 share: the speed X -$0170, Y -$0120, reversed in X when facing left. */
static HopLaunch launch_up_and_aside(uint8_t slot)
{
    HopLaunch r = { 0 };

    WR8(OBJ->vx_hi[slot], 0xFE);
    WR8(OBJ->vx_lo[slot], 0x90);
    WR8(OBJ->vy_hi[slot], 0xFF);
    WR8(OBJ->vy_lo[slot], 0xE0);
    r.facing = RD8(OBJ->facing_left[slot]);
    if (r.facing == 0) {
        return r;
    }
    r.negated = NegateVelocityX(slot);
    return r;
}

static Ended launch_ended(HopLaunch r)
{
    if (r.facing == 0) {
        return then(ended_load(0xE0), ended_load(r.facing));
    }
    return then(ended_load(r.facing), ended_of_sum(r.negated));
}

/*
 * @routine 0EBF:006 HopBetweenSidesWithBox
 * @run 71:D006
 * WHAT IT DOES: Sets the hit box by the frame (HitBoxByFrameTableCE8F). In state 0 loads the record, picks a side at random, places the object from the table at $D0BD (X) and a random Y near $00DC, and launches it up and aside. In state 1 it steps the list at $D0B8 and moves; its X speed grows by the table at $D0C1 for its side until it changes sign, then it stops, takes the Y speed $FFC0, OBJ_34B8 $10 and goes to state 2. In later states it counts OBJ_34B8 down while it moves in Y and at zero goes back to state 1 and launches again.
 * HOW: HitBoxByFrameTableCE8F, LoadObjectRecord, NextRandom twice, StepObjectFrames, ApplyVelocityY and X, 16-bit adds, NegateVelocityX.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_hop_sides_exit_shim).
 */
HopArm HopBetweenSidesWithBox(uint8_t slot, uint8_t y_entry)
{
    HopArm r = { 0 };
    uint8_t side, value, old;
    Sum sum;

    jsr_push(0xD009);
    r.box = HitBoxByFrameTableCE8F(slot);
    jsr_pop();
    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        jsr_push(0xD011);
        r.record = l8_record(slot);
        jsr_pop();
        jsr_push(0xD014);
        r.random = NextRandom();
        jsr_pop();
        side = r.random.value & 0x01;
        WR8(OBJ->facing_left[slot], side);
        r.y = (uint8_t)(side << 1);
        value = rd8((uint16_t)(0xD0BD + r.y));
        WR8(OBJ->x_lo[slot], value);
        value = rd8((uint16_t)(0xD0BE + r.y));
        WR8(OBJ->x_hi[slot], value);
        jsr_push(0xD02A);
        r.random2 = NextRandom();
        jsr_pop();
        sum = sub_bytes((uint8_t)(r.random2.value & 0x1F), 0x10, true);
        r.sum = add_bytes(sum.value, 0xDC, false);
        WR8(OBJ->y_lo[slot], r.sum.value);
        r.launch = launch_up_and_aside(slot);
        r.arm = HOP_START;
        return r;
    }
    if (r.state == 1) {
        r.step = l8_list_step(slot, 0xD0B8, 0xD060, false, y_entry);
        jsr_push(0xD063);
        (void)ApplyVelocityY(slot);
        jsr_pop();
        jsr_push(0xD066);
        r.move = ApplyVelocityX(slot);
        jsr_pop();
        old = RD8(OBJ->vx_hi[slot]);
        WR8(ZP->temp00, old);
        r.facing = RD8(OBJ->facing_left[slot]);
        r.y = (uint8_t)(r.facing << 1);
        value = RD8(OBJ->vx_lo[slot]);
        sum = add_bytes(value, rd8((uint16_t)(0xD0C1 + r.y)), false);
        WR8(OBJ->vx_lo[slot], sum.value);
        value = RD8(OBJ->vx_hi[slot]);
        r.sum = add_bytes(value, rd8((uint16_t)(0xD0C2 + r.y)), sum.carry);
        WR8(OBJ->vx_hi[slot], r.sum.value);
        r.sign = (uint8_t)((r.sum.value ^ RD8(ZP->temp00)) & 0x80);
        if (r.sign == 0) {
            r.arm = HOP_SAME_SIGN;
            return r;
        }
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0);
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0xC0);
        WR8(OBJ->f34b8[slot], 0x10);
        r.counted = INC8(OBJ->state[slot]);
        r.arm = HOP_STOPPED;
        return r;
    }
    r.timer = DEC8(OBJ->f34b8[slot]);
    if (r.timer != 0) {
        r.move = ApplyVelocityY(slot);
        r.arm = HOP_COUNTING;
        return r;
    }
    WR8(OBJ->state[slot], 1);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    r.launch = launch_up_and_aside(slot);
    r.arm = HOP_RELAUNCH;
    return r;
}

static Ended l8_hop_sides_exit_shim(HopArm r)
{
    Ended e = then(l8_hit_box_ce8f_exit_shim(r.box), ended_load(r.state));

    if (r.arm == HOP_START) {
        e = then(e, l8_record_ended(r.record));
        e = then(e, ended_of_random(r.random));
        e = then(e, ended_with_y(ended_with_carry(ended_load(r.y), false), r.y));
        e = then(e, ended_of_random(r.random2));
        e = then(e, ended_of_sum(r.sum));
        return then(e, launch_ended(r.launch));
    }
    e = then(e, ended_load((uint8_t)(r.state - 1)));
    if (r.arm == HOP_SAME_SIGN || r.arm == HOP_STOPPED) {
        e = then(e, l8_step_ended(r.step));
        e = then(e, ended_of_axis_move(r.move));
        e = then(e, ended_with_y(ended_with_carry(ended_load(r.y), r.facing >= 0x80), r.y));
        e = then(e, ended_of_sum(r.sum));
        e = then(e, ended_load(r.sign));
        if (r.arm == HOP_SAME_SIGN) {
            return e;
        }
        e = then(e, ended_load(0x10));
        return then(e, ended_nz(r.counted));
    }
    e = then(e, ended_nz(r.timer));
    if (r.arm == HOP_COUNTING) {
        return then(e, ended_of_axis_move(r.move));
    }
    return then(then(e, ended_load(1)), launch_ended(r.launch));
}

void abi_HopBetweenSidesWithBox(void)
{
    abi_ended(l8_hop_sides_exit_shim(HopBetweenSidesWithBox(cpu.x, cpu.y)));
}

/*
 * @routine 0EBD:2A7 SpawnerByKindAndScreen
 * @run 71:C2A7
 * WHAT IT DOES: In state 0 loads the record. Unless frame $54's byte is 4 it waits OBJ_34B8 down; then by OBJ_3040: 0 starts an object of type $2C in a free slot (every second one from $1A) when the scroll's screen byte is 2 or 4, and waits $50 to $8F; 2 starts one of type $CB in a free slot of $30 to $3F and waits $80; otherwise, with OBJ_3568 below $80, it finds a group of four empty slots from $20 and starts four objects of type $49 there (each cleared first) numbered 3, 2, 1, 0 in OBJ_35C0 with one random number to 3 in OBJ_3670, and waits $F0.
 * HOW: LoadObjectRecord, searches of the slot kinds, StartObjectAtPlace, ClearObjectInY, NextRandom, FindFreeSlotFrom30.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_spawner_exit_shim). Everything past state 0 ran in no script; the draft stops there and the
 * arms are written from the decoded bytes.
 */
SpawnerArm SpawnerByKindAndScreen(uint8_t slot)
{
    SpawnerArm r = { 0 };
    uint8_t y, kind, count, random_bits, joined, placed;

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        r.record = l8_record(slot);
        r.arm = SPAWN_LOADED;
        return r;
    }
    r.frame = RD8(OBJ->frame[0x54]);
    if (r.frame == 4) {
        r.arm = SPAWN_FRAME4;
        return r;
    }
    r.timer = RD8(OBJ->f34b8[slot]);
    if (r.timer != 0) {
        r.counted = DEC8(OBJ->f34b8[slot]);
        r.arm = SPAWN_WAITING;
        return r;
    }
    r.kind = RD8(OBJ->f3040[slot]);
    if (r.kind == 0) {
        r.screen = RD8(SCROLLE->x_hi_screen[0]);
        if (r.screen < 2) {
            r.arm = SPAWN_SCREEN_LOW;
            return r;
        }
        if (r.screen == 3) {
            r.arm = SPAWN_SCREEN_3;
            return r;
        }
        if (r.screen >= 5) {
            r.arm = SPAWN_SCREEN_HIGH;
            return r;
        }
        for (y = 0x1A;;) {
            kind = RD8(OBJ->kind[y]);
            r.scan.y = y;
            r.scan.kind = kind;
            if (kind == 0) {
                break;
            }
            y = (uint8_t)(y + 2);
            r.scan.stepped = true;
            if (y >= 0x20) {
                r.scan.y = y;
                r.arm = SPAWN_PAIRS_FULL;
                return r;
            }
        }
        jsr_push(0xC2E6);
        r.placed = StartObjectAtPlace(0x2C, slot, y);
        jsr_pop();
        jsr_push(0xC2E9);
        r.random = NextRandom();
        jsr_pop();
        r.sum = add_bytes((uint8_t)(r.random.value & 0x3F), 0x50, false);
        WR8(OBJ->f34b8[slot], r.sum.value);
        r.arm = SPAWN_TYPE_2C;
        return r;
    }
    if (r.kind == 2) {
        jsr_push(0xC344);
        r.found = l8_free_slot30();
        jsr_pop();
        if (!r.found.found) {
            r.arm = SPAWN_NO_SLOT;
            return r;
        }
        jsr_push(0xC34B);
        r.placed = StartObjectAtPlace(0xCB, slot, r.found.slot);
        jsr_pop();
        WR8(OBJ->f34b8[slot], 0x80);
        r.arm = SPAWN_TYPE_CB;
        return r;
    }
    r.aim = RD8(OBJ->f3568[slot]);
    if (r.aim >= 0x80) {
        r.arm = SPAWN_AIM_HIGH;
        return r;
    }
    for (y = 0x20;;) {
        joined = RD8(OBJ->kind[y]);
        joined = (uint8_t)(joined | RD8(OBJ->kind[y + 1]));
        joined = (uint8_t)(joined | RD8(OBJ->kind[y + 2]));
        joined = (uint8_t)(joined | RD8(OBJ->kind[y + 3]));
        r.scan.y = y;
        r.scan.kind = joined;
        if (joined == 0) {
            break;
        }
        y = (uint8_t)(y + 4);
        r.scan.stepped = true;
        if (y >= 0x40) {
            r.scan.y = y;
            r.arm = SPAWN_GROUPS_FULL;
            return r;
        }
    }
    push8(y);
    jsr_push(0xC31A);
    r.random = NextRandom();
    jsr_pop();
    random_bits = r.random.value & 0x03;
    WR8(ZP->temp01, random_bits);
    y = pull8();
    WR8(ZP->temp00, 4);
    do {
        jsr_push(0xC326);
        ClearObjectInY(y);
        jsr_pop();
        jsr_push(0xC32B);
        placed = StartObjectAtPlace(0x49, slot, y);
        jsr_pop();
        count = RD8(ZP->temp00);
        WR8(OBJ->f35c0[y], (uint8_t)(count - 1));
        random_bits = RD8(ZP->temp01);
        WR8(OBJ->f3670[y], random_bits);
        y++;
        count = DEC8(ZP->temp00);
        (void)placed;
    } while (count != 0);
    r.left = count;
    r.y_end = y;
    WR8(OBJ->f34b8[slot], 0xF0);
    r.arm = SPAWN_GROUP;
    return r;
}

static Ended l8_spawner_exit_shim(SpawnerArm r)
{
    Ended e = ended_load(r.state);

    if (r.arm == SPAWN_LOADED) {
        return then(e, l8_record_ended(r.record));
    }
    e = then(e, ended_compare(r.frame, 4));
    if (r.arm == SPAWN_FRAME4) {
        return e;
    }
    e = then(e, ended_load(r.timer));
    if (r.arm == SPAWN_WAITING) {
        return then(e, ended_nz(r.counted));
    }
    e = then(e, ended_load(r.kind));
    if (r.kind == 0) {
        e = then(e, ended_compare(r.screen, 2));
        if (r.arm == SPAWN_SCREEN_LOW) {
            return e;
        }
        e = then(e, ended_compare(r.screen, 3));
        if (r.arm == SPAWN_SCREEN_3) {
            return e;
        }
        e = then(e, ended_compare(r.screen, 5));
        if (r.arm == SPAWN_SCREEN_HIGH) {
            return e;
        }
        e = l8_scan_ended(e, r.scan, 2, 0x20, r.arm == SPAWN_PAIRS_FULL);
        if (r.arm == SPAWN_PAIRS_FULL) {
            return e;
        }
        e = then(e, ended_load(r.placed));
        e = then(e, ended_of_random(r.random));
        return then(e, ended_of_sum(r.sum));
    }
    e = then(e, ended_compare(r.kind, 2));
    if (r.kind == 2) {
        e = then(e, l8_slot_ended(r.found));
        if (r.arm == SPAWN_NO_SLOT) {
            return e;
        }
        return then(then(e, ended_load(r.placed)), ended_load(0x80));
    }
    e = then(e, ended_compare(r.aim, 0x80));
    if (r.arm == SPAWN_AIM_HIGH) {
        return e;
    }
    e = l8_scan_ended(e, r.scan, 4, 0x40, r.arm == SPAWN_GROUPS_FULL);
    if (r.arm == SPAWN_GROUPS_FULL) {
        return e;
    }
    e = then(e, ended_of_random(r.random));
    e = then(e, ended_with_y(ended_nz(r.left), r.y_end));
    return then(e, ended_load(0xF0));
}

void abi_SpawnerByKindAndScreen(void)
{
    abi_ended(l8_spawner_exit_shim(SpawnerByKindAndScreen(cpu.x)));
}
