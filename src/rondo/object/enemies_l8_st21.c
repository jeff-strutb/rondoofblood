/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 0F6B (enemy kinds not yet identified), by routine. Bodies return the
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

enum { FS_START, FS_FOLLOW };
enum { ST21_NO_SLOT, ST21_STARTED };
enum { FLIP_FALL, FLIP_NONE, FLIP_FLIPPED };
enum { CS_LOADED, CS_OTHER_STEP, CS_CLEARED };
enum { HT_FLAGGED, HT_FEW_HITS, HT_OTHER_HIGH, HT_SET };
enum { S6_IN_STATE6, S6_NOT_FLAGGED, S6_FLAGGED };
enum { CF_OTHER_MODE, CF_OTHER_SCREEN, CF_CLEARED };
enum { MU_HIGH, MU_DONE };
enum { CC_COUNTING, CC_COPIED };
enum { BR_LOADED, BR_FEW_HITS, BR_NO_SLOT, BR_BROKEN };

/*
 * @routine 0F6B:07D LoadRecordSplitIntoFourAndClear
 * @run 70:B07D
 * WHAT IT DOES: Sets OBJ_SPRITE_FLIP of slot 0 to $C0, loads the record and records the event, starts four objects of type $A6 in the slots after this one at its place (the second and fourth with bit 7 of OBJ_X_LO set, the third and fourth one screen on in X) and clears itself.
 * HOW: LoadObjectRecord, RecordObjectEvent, four StartObjectAtPlace calls, INCs and ORs with X and Y swapped, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_split_four_exit_shim).
 */
SplitArm LoadRecordSplitIntoFourAndClear(uint8_t slot)
{
    SplitArm r = { 0 };
    uint8_t y, value, placed;

    WR8(OBJ->sprite_flip[0], 0xC0);
    jsr_push(0xB085);
    r.record = l8_record(slot);
    jsr_pop();
    jsr_push(0xB088);
    r.event.marked = RecordObjectEvent(slot, &r.event.stored, &r.event.event, &r.event.byte);
    jsr_pop();
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
    r.value = value;
    r.y = y;
    return r;
}

static Ended l8_split_four_exit_shim(SplitArm r)
{
    Ended e = then(l8_record_ended(r.record), l8_of_event(r.event.marked, r.event.stored, r.event.event, r.event.byte));

    return then(e, ended_with_y(ended_load(r.value), r.y));
}

void abi_LoadRecordSplitIntoFourAndClear(void)
{
    abi_ended(l8_split_four_exit_shim(LoadRecordSplitIntoFourAndClear(cpu.x)));
}

/*
 * @routine 0F6B:0BF LoadRecordThenFollowScroll
 * @run 70:B0BF
 * WHAT IT DOES: In state 0 loads the record and keeps the object's X in OBJ_34B8 and OBJ_3510. Later it sets its screen X to that spot less the scroll plus a quarter of the scroll (the scroll's pair of bytes shifted right 3 times), so it moves slowly against the scroll.
 * HOW: LoadObjectRecord, two shifts through carry, two 16-bit adds and a 16-bit subtract in memory.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_follow_scroll_exit_shim).
 */
FollowArm LoadRecordThenFollowScroll(uint8_t slot)
{
    FollowArm r = { 0 };
    uint8_t high, low, value, base_lo, base_hi;
    bool shifted;
    Sum sum, diff;

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        jsr_push(0xB0C7);
        r.record = l8_record(slot);
        jsr_pop();
        value = RD8(OBJ->x_lo[slot]);
        WR8(OBJ->f34b8[slot], value);
        r.value = RD8(OBJ->x_hi[slot]);
        WR8(OBJ->f3510[slot], r.value);
        r.arm = FS_START;
        return r;
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
    r.diff = sub_bytes(value, RD8(SCROLLE->x_hi_screen[0]), diff.carry);
    WR8(OBJ->x_hi[slot], r.diff.value);
    r.arm = FS_FOLLOW;
    return r;
}

static Ended l8_follow_scroll_exit_shim(FollowArm r)
{
    if (r.arm == FS_START) {
        return then(then(ended_load(r.state), l8_record_ended(r.record)), ended_load(r.value));
    }
    return ended_of_sum(r.diff);
}

void abi_LoadRecordThenFollowScroll(void)
{
    abi_ended(l8_follow_scroll_exit_shim(LoadRecordThenFollowScroll(cpu.x)));
}

/*
 * @routine 0F6B:150 StartPieceWithSpeedFromTable
 * @run 70:B150
 * WHAT IT DOES: Starts an object of type $A4 in a free slot of $20 to $3F at the object's place, $28 lower, with OBJ_3040 the number in zTemp00 plus 3 and the speed from the table at $B18A by twice that number.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace, table reads, stores between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object, zTemp00 the number. Exit: as the last instruction that ran (l8_piece_speed_exit_shim). The body returns the slot search (found), the X pulled (first), the number (second), the last table byte (third) and the last add (sum).
 */
L8Arm StartPieceWithSpeedFromTable(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t placed, value, number, index;
    Sum sum;

    jsr_push(0xB153);
    r.found.slot = FindFreeSlotFrom20(&r.found.last, &r.found.found);
    jsr_pop();
    if (!r.found.found) {
        r.arm = ST21_NO_SLOT;
        return r;
    }
    jsr_push(0xB15A);
    placed = StartObjectAtPlace(0xA4, slot, r.found.slot);
    jsr_pop();
    push8(slot);
    value = RD8(OBJ->y_lo[r.found.slot]);
    sum = add_bytes(value, 0x28, false);
    WR8(OBJ->y_lo[r.found.slot], sum.value);
    number = RD8(ZP->temp00);
    r.sum = add_bytes(number, 3, false);
    WR8(OBJ->f3040[r.found.slot], r.sum.value);
    number = RD8(ZP->temp00);
    index = (uint8_t)(number << 1);
    value = rd8((uint16_t)(0xB18A + index));
    WR8(OBJ->vx_lo[r.found.slot], value);
    value = rd8((uint16_t)(0xB18B + index));
    WR8(OBJ->vx_hi[r.found.slot], value);
    value = rd8((uint16_t)(0xB192 + index));
    WR8(OBJ->vy_lo[r.found.slot], value);
    value = rd8((uint16_t)(0xB193 + index));
    WR8(OBJ->vy_hi[r.found.slot], value);
    r.first = pull8();
    r.second = number;
    r.third = value;
    (void)placed;
    r.arm = ST21_STARTED;
    return r;
}

static Ended l8_piece_speed_exit_shim(L8Arm r)
{
    Ended e = l8_slot_ended(r.found);

    if (r.arm == ST21_NO_SLOT) {
        return e;
    }
    return then(e, ended_with_a(ended_with_overflow(ended_with_carry(ended_nz(r.first), r.second >= 0x80), r.sum.overflow), r.third));
}

void abi_StartPieceWithSpeedFromTable(void)
{
    abi_ended(l8_piece_speed_exit_shim(StartPieceWithSpeedFromTable(cpu.x)));
}

/*
 * @routine 0F6B:19A LoadRecordFlipByPlayerThenFall
 * @run 70:B19A
 * WHAT IT DOES: In state 0 loads the record, takes OBJ_3040 as its frame and, when the player is on the left side, faces left and negates (bytes inverted) its speed. Later it falls (ApplyGravity) and moves.
 * HOW: LoadObjectRecord, SideOfPlayer, EORs, ApplyGravity, a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_flip_then_fall_exit_shim).
 */
FollowArm LoadRecordFlipByPlayerThenFall(uint8_t slot)
{
    FollowArm r = { 0 };
    SideResult side;
    GravityResult gravity;

    r.state = RD8(OBJ->state[slot]);
    if (r.state != 0) {
        jsr_push(0xB1C7);
        gravity = ApplyGravity(slot);
        jsr_pop();
        (void)gravity;
        r.move = ApplyVelocity(slot);
        r.arm = FLIP_FALL;
        return r;
    }
    jsr_push(0xB1A2);
    r.record = l8_record(slot);
    jsr_pop();
    r.kind = RD8(OBJ->f3040[slot]);
    WR8(OBJ->frame[slot], r.kind);
    jsr_push(0xB1AB);
    side = SideOfPlayer(slot);
    jsr_pop();
    r.sides = side.sides;
    if ((side.sides & 0x01) == 0) {
        r.arm = FLIP_NONE;
        return r;
    }
    WR8(OBJ->facing_left[slot], 1);
    r.value = (uint8_t)(RD8(OBJ->vx_lo[slot]) ^ 0xFF);
    WR8(OBJ->vx_lo[slot], r.value);
    r.value = (uint8_t)(RD8(OBJ->vx_hi[slot]) ^ 0xFF);
    WR8(OBJ->vx_hi[slot], r.value);
    r.arm = FLIP_FLIPPED;
    return r;
}

static Ended l8_flip_then_fall_exit_shim(FollowArm r)
{
    Ended e;

    if (r.arm == FLIP_FALL) {
        return ended_of_axis_move(r.move);
    }
    e = then(ended_load(r.state), l8_record_ended(r.record));
    e = then(e, ended_load(r.kind));
    e = then(e, ended_with_carry(ended_load((uint8_t)(r.sides >> 1)), (r.sides & 0x01) != 0));
    if (r.arm == FLIP_NONE) {
        return e;
    }
    return then(e, ended_load(r.value));
}

void abi_LoadRecordFlipByPlayerThenFall(void)
{
    abi_ended(l8_flip_then_fall_exit_shim(LoadRecordFlipByPlayerThenFall(cpu.x)));
}

/*
 * @routine 0F6B:1CA LoadRecordThenClearAtSubStep3
 * @run 70:B1CA
 * WHAT IT DOES: In state 0 loads the record. Later, when the sub-mode's step is 3, clears OBJ_SPRITE_FLIP of slot 0 and clears the object.
 * HOW: LoadObjectRecord, a compare, a store, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_clear_at_step3_exit_shim). The body returns the state (first) and the sub-mode step (second).
 */
L8Arm LoadRecordThenClearAtSubStep3(uint8_t slot)
{
    L8Arm r = { 0 };

    r.first = RD8(OBJ->state[slot]);
    if (r.first == 0) {
        r.record = l8_record(slot);
        r.arm = CS_LOADED;
        return r;
    }
    r.second = RD8(ZP->sub_step);
    if (r.second != 3) {
        r.arm = CS_OTHER_STEP;
        return r;
    }
    WR8(OBJ->sprite_flip[0], 0);
    ClearObject(slot);
    r.arm = CS_CLEARED;
    return r;
}

static Ended l8_clear_at_step3_exit_shim(L8Arm r)
{
    if (r.arm == CS_LOADED) {
        return then(ended_load(r.first), l8_record_ended(r.record));
    }
    return ended_compare(r.second, 3);
}

void abi_LoadRecordThenClearAtSubStep3(void)
{
    abi_ended(l8_clear_at_step3_exit_shim(LoadRecordThenClearAtSubStep3(cpu.x)));
}

/*
 * @routine 0F6B:395 SoundWhenHurtThenFlag
 * @run 70:B395
 * WHAT IT DOES: When bit 7 of OBJ_31A0 is clear and the object has taken 5 hits plays sound $D2 and, when OBJ_32A8 of slot $10 is below 8, sets it to 5, sets OBJ_3618 to 5 and sets bit 7 of OBJ_31A0.
 * HOW: Tests, SoundCommand, stores.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_hurt_then_flag_exit_shim). The stores past the last compare ran in no script; the draft stops there.
 */
HurtSoundArm SoundWhenHurtThenFlag(uint8_t slot, uint8_t y_entry)
{
    HurtSoundArm r = { 0 };
    uint8_t flags, value;

    flags = RD8(OBJ->status_flags[slot]);
    r.first = (uint8_t)(flags & 0x80);
    if (r.first != 0) {
        r.arm = HT_FLAGGED;
        return r;
    }
    r.second = RD8(OBJ->hit_taken[slot]);
    if (r.second < 5) {
        r.arm = HT_FEW_HITS;
        return r;
    }
    jsr_push(0xB3A8);
    r.call = SoundCommand(0xD2, slot, y_entry);
    jsr_pop();
    r.value = RD8(OBJ->f32a8[0x10]);
    if (r.value >= 8) {
        r.arm = HT_OTHER_HIGH;
        return r;
    }
    WR8(OBJ->f32a8[0x10], 5);
    WR8(OBJ->f3618[slot], 5);
    value = RD8(OBJ->status_flags[slot]);
    r.value = (uint8_t)(0x80 | value);
    WR8(OBJ->status_flags[slot], r.value);
    r.arm = HT_SET;
    return r;
}

SoundEnd l8_hurt_then_flag_exit_shim(HurtSoundArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_load(r.first);

    if (r.arm == HT_FLAGGED) {
        s.end = e;
        return s;
    }
    e = then(e, ended_compare(r.second, 5));
    if (r.arm == HT_FEW_HITS) {
        s.end = e;
        return s;
    }
    s.sounded = true;
    s.command = 0xD2;
    s.before = then(e, ended_load(0xD2));
    s.call = r.call;
    s.end = r.arm == HT_OTHER_HIGH ? ended_compare(r.value, 8) : ended_load(r.value);
    return s;
}

void abi_SoundWhenHurtThenFlag(void)
{
    abi_sound_end(l8_hurt_then_flag_exit_shim(SoundWhenHurtThenFlag(cpu.x, cpu.y)));
}

/*
 * @routine 0F6B:3C0 SoundAndState6WhenFlagged
 * @run 70:B3C0
 * WHAT IT DOES: Outside state 6: when bit 7 of OBJ_31A0 is set plays sound $D3, goes to state 6 and copies OBJ_3618 to OBJ_32A8; otherwise copies OBJ_32A8 to OBJ_3618.
 * HOW: Compares, SoundCommand, copies.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_state6_when_flagged_exit_shim). The flagged arm ran in no script; the draft stops there.
 */
HurtSoundArm SoundAndState6WhenFlagged(uint8_t slot, uint8_t y_entry)
{
    HurtSoundArm r = { 0 };
    uint8_t flags;

    r.first = RD8(OBJ->state[slot]);
    if (r.first == 6) {
        r.arm = S6_IN_STATE6;
        return r;
    }
    flags = RD8(OBJ->status_flags[slot]);
    r.second = (uint8_t)(flags & 0x80);
    if (r.second == 0) {
        r.value = RD8(OBJ->f32a8[slot]);
        WR8(OBJ->f3618[slot], r.value);
        r.arm = S6_NOT_FLAGGED;
        return r;
    }
    jsr_push(0xB3D3);
    r.call = SoundCommand(0xD3, slot, y_entry);
    jsr_pop();
    WR8(OBJ->state[slot], 6);
    WR8(OBJ->substate[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    r.value = RD8(OBJ->f3618[slot]);
    WR8(OBJ->f32a8[slot], r.value);
    r.arm = S6_FLAGGED;
    return r;
}

SoundEnd l8_state6_when_flagged_exit_shim(HurtSoundArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_compare(r.first, 6);

    if (r.arm == S6_IN_STATE6) {
        s.end = e;
        return s;
    }
    e = then(e, ended_load(r.second));
    if (r.arm == S6_NOT_FLAGGED) {
        s.end = then(e, ended_load(r.value));
        return s;
    }
    s.sounded = true;
    s.command = 0xD3;
    s.before = then(e, ended_load(0xD3));
    s.call = r.call;
    s.end = ended_load(r.value);
    return s;
}

void abi_SoundAndState6WhenFlagged(void)
{
    abi_sound_end(l8_state6_when_flagged_exit_shim(SoundAndState6WhenFlagged(cpu.x, cpu.y)));
}

/*
 * @routine 0F6B:40C LoadRecordFrame0D
 * @run 70:B40C
 * WHAT IT DOES: Loads the object's record and sets its frame $0D.
 * HOW: LoadObjectRecord and a store.
 * WAITS: no.
 * Entry: X object. Exit: A $0D, Y 11, C from the record (l8_frame_0d_exit_shim; the body returns the record's result).
 */
L8Record LoadRecordFrame0D(uint8_t slot)
{
    L8Record r;

    jsr_push(0xB40F);
    r = l8_record(slot);
    jsr_pop();
    WR8(OBJ->frame[slot], 0x0D);
    return r;
}

static Ended l8_frame_0d_exit_shim(L8Record r)
{
    return then(l8_record_ended(r), ended_load(0x0D));
}

void abi_LoadRecordFrame0D(void)
{
    abi_ended(l8_frame_0d_exit_shim(LoadRecordFrame0D(cpu.x)));
}

/*
 * @routine 0F6B:415 ClearFourAtScreen2
 * @run 70:B415
 * WHAT IT DOES: When the sub-mode is 1 and the scroll's screen byte is 2, sets the scroll's X limit low byte $80 and zRasterStage 3, clears slots $40 to $43, gives the object the Y speed -$0080 and goes to the next state.
 * HOW: Two compares, ClearObjectInY four times, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_clear_four_exit_shim). The body returns the sub-mode (first), the screen byte (second) and the new state (third).
 */
L8Arm ClearFourAtScreen2(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t y;

    r.first = RD8(ZP->game_sub_mode);
    if (r.first != 1) {
        r.arm = CF_OTHER_MODE;
        return r;
    }
    r.second = RD8(SCROLLE->x_hi_screen[0]);
    if (r.second != 2) {
        r.arm = CF_OTHER_SCREEN;
        return r;
    }
    wr8(ScrollXMaxLo, 0x80);
    WR8(ZP->raster_stage, 3);
    for (y = 0x40; y < 0x44; y++) {
        jsr_push((uint16_t)(0xB430 + 4 * (y - 0x40)));
        ClearObjectInY(y);
        jsr_pop();
    }
    WR8(OBJ->vy_lo[slot], 0x80);
    WR8(OBJ->vy_hi[slot], 0xFF);
    r.third = INC8(OBJ->state[slot]);
    r.arm = CF_CLEARED;
    return r;
}

static Ended l8_clear_four_exit_shim(L8Arm r)
{
    Ended e;

    if (r.arm == CF_OTHER_MODE) {
        return ended_compare(r.first, 1);
    }
    if (r.arm == CF_OTHER_SCREEN) {
        return ended_compare(r.second, 2);
    }
    e = ended_compare(r.second, 2);
    e = then(e, ended_load(3));
    e = then(e, ended_with_y(ended_nz(0x43), 0x43));
    e = then(e, ended_load(0xFF));
    return then(e, ended_nz(r.third));
}

void abi_ClearFourAtScreen2(void)
{
    abi_ended(l8_clear_four_exit_shim(ClearFourAtScreen2(cpu.x)));
}

/*
 * @routine 0F6B:44A MoveUpUntilY40
 * @run 70:B44A
 * WHAT IT DOES: Moves the object along Y; once its Y low byte is below $40 sets OBJ_34B8 2 and goes to the next state.
 * HOW: ApplyVelocityY, a compare, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_move_up_exit_shim). The body returns the move, the Y low byte (first) and the new state (second).
 */
L8Arm MoveUpUntilY40(uint8_t slot)
{
    L8Arm r = { 0 };

    jsr_push(0xB44D);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.first = RD8(OBJ->y_lo[slot]);
    if (r.first >= 0x40) {
        r.arm = MU_HIGH;
        return r;
    }
    WR8(OBJ->f34b8[slot], 2);
    r.second = INC8(OBJ->state[slot]);
    r.arm = MU_DONE;
    return r;
}

static Ended l8_move_up_exit_shim(L8Arm r)
{
    Ended e = then(ended_of_axis_move(r.move), ended_compare(r.first, 0x40));

    if (r.arm == MU_HIGH) {
        return e;
    }
    e = then(e, ended_load(2));
    return then(e, ended_nz(r.second));
}

void abi_MoveUpUntilY40(void)
{
    abi_ended(l8_move_up_exit_shim(MoveUpUntilY40(cpu.x)));
}

/*
 * @routine 0F6B:50B CountDownThenCopyPhase
 * @run 70:B50B
 * WHAT IT DOES: Counts OBJ_3510 down; when it is 0 (or reaches 0) sets OBJ_SUBSTATE from OBJ_38D8.
 * HOW: A test, a memory decrement, a copy.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_copy_phase_exit_shim). The body returns the count read (first), the count after the decrement (second) and the byte copied (third).
 */
L8Arm CountDownThenCopyPhase(uint8_t slot)
{
    L8Arm r = { 0 };

    r.first = RD8(OBJ->f3510[slot]);
    if (r.first != 0) {
        r.second = DEC8(OBJ->f3510[slot]);
        if (r.second != 0) {
            r.arm = CC_COUNTING;
            return r;
        }
    }
    r.third = RD8(OBJ->f38d8[slot]);
    WR8(OBJ->substate[slot], r.third);
    r.arm = CC_COPIED;
    return r;
}

static Ended l8_copy_phase_exit_shim(L8Arm r)
{
    Ended e = ended_load(r.first);

    if (r.first != 0) {
        e = then(e, ended_nz(r.second));
        if (r.arm == CC_COUNTING) {
            return e;
        }
    }
    return then(e, ended_load(r.third));
}

void abi_CountDownThenCopyPhase(void)
{
    abi_ended(l8_copy_phase_exit_shim(CountDownThenCopyPhase(cpu.x)));
}

/*
 * @routine 0F6B:51C ResetPieceAndAdvance
 * @run 70:B51C
 * WHAT IT DOES: Clears OBJ_3778, sets OBJ_37D0 $80 and OBJ_35C0 $E0 and goes to the next OBJ_SUBSTATE.
 * HOW: Stores and an INC.
 * WAITS: no.
 * Entry: X object. Exit: A $E0, N and Z from the new OBJ_SUBSTATE (l8_reset_piece_exit_shim; the body returns it).
 */
uint8_t ResetPieceAndAdvance(uint8_t slot)
{
    WR8(OBJ->f3778[slot], 0);
    WR8(OBJ->f37d0[slot], 0x80);
    WR8(OBJ->f35c0[slot], 0xE0);
    return INC8(OBJ->substate[slot]);
}

static Ended l8_reset_piece_exit_shim(uint8_t phase)
{
    return then(ended_load(0xE0), ended_nz(phase));
}

void abi_ResetPieceAndAdvance(void)
{
    abi_ended(l8_reset_piece_exit_shim(ResetPieceAndAdvance(cpu.x)));
}

/*
 * @routine 0F6B:100 BreakIntoPiecesAtFifthHit
 * @run 70:B100
 * WHAT IT DOES: In state 0 loads the record. Once the object has taken 5 hits it records its event, plays sound $12, takes the kind of OBJ_3040, sets OBJ_3A90 $10, goes to state 0, drops $28 in Y, clears its frame and starts an object of type $9B in a free slot of $50 to $53, then four pieces (StartPieceWithSpeedFromTable for the numbers 0 to 3).
 * HOW: LoadObjectRecord, RecordObjectEvent, SoundCommand, FindFreeSlotFrom50, StartObjectAtPlace, four calls.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_break_pieces_exit_shim).
 */
BreakArm BreakIntoPiecesAtFifthHit(uint8_t slot, uint8_t y_entry)
{
    BreakArm r = { 0 };
    uint8_t kind, y, low;
    unsigned piece;

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        r.record = l8_record(slot);
        r.arm = BR_LOADED;
        return r;
    }
    r.hits = RD8(OBJ->hit_taken[slot]);
    if (r.hits < 5) {
        r.arm = BR_FEW_HITS;
        return r;
    }
    jsr_push(0xB112);
    r.event.marked = RecordObjectEvent(slot, &r.event.stored, &r.event.event, &r.event.byte);
    jsr_pop();
    y = r.event.marked ? r.event.byte : y_entry;          /* RecordObjectEvent leaves Y the byte index when it marked */
    jsr_push(0xB117);
    r.call = SoundCommand(0x12, slot, y);
    jsr_pop();
    kind = RD8(OBJ->f3040[slot]);
    WR8(OBJ->kind[slot], kind);
    WR8(OBJ->timer[slot], 0x10);
    WR8(OBJ->state[slot], 0);
    low = RD8(OBJ->y_lo[slot]);
    r.sum = add_bytes(low, 0xD8, false);
    WR8(OBJ->y_lo[slot], r.sum.value);
    WR8(OBJ->frame[slot], 0);
    jsr_push(0xB134);
    r.found.slot = FindFreeSlotFrom50(&r.found.last, &r.found.found);
    jsr_pop();
    if (!r.found.found) {
        r.arm = BR_NO_SLOT;
        return r;
    }
    jsr_push(0xB13B);
    r.placed = StartObjectAtPlace(0x9B, slot, r.found.slot);
    jsr_pop();
    WR8(ZP->temp00, 0);
    for (piece = 0; piece < 4; piece++) {
        jsr_push((uint16_t)(0xB140 + 5 * piece));
        r.piece[piece] = StartPieceWithSpeedFromTable(slot);
        jsr_pop();
        if (piece < 3) {
            r.counted[piece] = INC8(ZP->temp00);
        }
    }
    r.arm = BR_BROKEN;
    return r;
}

static SoundEnd l8_break_pieces_exit_shim(BreakArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_load(r.state);
    unsigned piece;

    if (r.arm == BR_LOADED) {
        s.end = then(e, l8_record_ended(r.record));
        return s;
    }
    e = then(e, ended_compare(r.hits, 5));
    if (r.arm == BR_FEW_HITS) {
        s.end = e;
        return s;
    }
    e = then(e, l8_of_event(r.event.marked, r.event.stored, r.event.event, r.event.byte));
    s.sounded = true;
    s.command = 0x12;
    s.before = then(e, ended_load(0x12));
    s.call = r.call;
    e = ended_of_sum(r.sum);
    e = then(e, l8_slot_ended(r.found));
    if (r.arm == BR_NO_SLOT) {
        s.end = e;
        return s;
    }
    e = then(e, ended_load(r.placed));
    for (piece = 0; piece < 4; piece++) {
        e = then(e, l8_piece_speed_exit_shim(r.piece[piece]));
        if (piece < 3) {
            e = then(e, ended_nz(r.counted[piece]));
        }
    }
    s.end = e;
    return s;
}

void abi_BreakIntoPiecesAtFifthHit(void)
{
    abi_sound_end(l8_break_pieces_exit_shim(BreakIntoPiecesAtFifthHit(cpu.x, cpu.y)));
}
