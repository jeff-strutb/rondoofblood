/*
 * TEMPORARY: the stage 0A51 enemies and their helpers (disc sectors 0A99 to 0A9E), by routine until each enemy kind is
 * identified. Bodies return how the routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the
 * abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

static Ended of_sum(Sum sum)
{
    return ended_of_sum(sum);
}

/* CPY and CPX: N, Z and C from the register less the operand; A kept. */
static Ended cpy(uint8_t reg, uint8_t operand)
{
    return ended_with_carry(ended_nz((uint8_t)(reg - operand)), reg >= operand);
}

/* The carry as it stands: what the exit state set, else the entry's. */
static bool carry_of(Ended e, bool entry)
{
    return e.c_set ? e.c : entry;
}

/* The steps of an animation list at `list` (zTemp10 first or zTemp11 first, as the assembly stores them). */
static FrameStep step_list(uint8_t slot, uint16_t list, uint16_t ret, uint8_t *y, bool high_first)
{
    FrameStep step;

    if (high_first) {
        WR8(ZP->temp11, (uint8_t)(list >> 8));
        WR8(ZP->temp10, (uint8_t)list);
    } else {
        WR8(ZP->temp10, (uint8_t)list);
        WR8(ZP->temp11, (uint8_t)(list >> 8));
    }
    jsr_push(ret);
    step = StepObjectFrames(slot, y);
    jsr_pop();
    return step;
}

/*
 * @routine 0A9D:01F BreakWhenHit
 * @run 71:C01F
 * WHAT IT DOES: An object in a state below $0B whose status bit 7 is set clears the object after it, stops its hitbox and animation,
 * sets OBJ_3778 to $40 and enters state $0B.
 * HOW: Reads the state and the status flags; ClearObject on slot plus one.
 * WAITS: no.
 * Entry: X object. Exit: A $0B, N and Z from it, C 0.
 */
Ended BreakWhenHit(uint8_t slot)
{
    uint8_t state, flags;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_compare(state, 0x0B);
    if (state >= 0x0B) {
        return e;
    }
    flags = RD8(OBJ->status_flags[slot]);
    flags = (uint8_t)(flags & 0x80);
    e = then(e, ended_load(flags));
    if (flags == 0) {
        return e;
    }
    jsr_push(0xC031);
    ClearObject((uint8_t)(slot + 1));
    jsr_pop();
    e = then(e, ended_nz(slot));
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f3778[slot], 0x40);
    WR8(OBJ->state[slot], 0x0B);
    return then(e, ended_load(0x0B));
}

void abi_BreakWhenHit(void)
{
    abi_ended(BreakWhenHit(cpu.x));
}

/*
 * @routine 0A9D:070 StartWithPartner
 * @run 71:C070
 * WHAT IT DOES: Loads the object's record, sets OBJ_3510 to $B0 and its angle byte to $40, and starts a kind $25 object at its place in the slot
 * after it.
 * HOW: LoadObjectRecord, ClearObjectInY, a tail jump into StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte copied, Y the slot plus one, C from the record.
 */
Ended StartWithPartner(uint8_t slot)
{
    uint8_t nz, last, to, placed;
    bool carry;

    jsr_push(0xC073);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    WR8(OBJ->f3510[slot], 0xB0);
    WR8(OBJ->angle_hi[slot], 0x40);
    to = (uint8_t)(slot + 1);
    jsr_push(0xC083);
    ClearObjectInY(to);
    jsr_pop();
    placed = StartObjectAtPlace(0x25, slot, to);
    return then(ended_of_record(last, nz, carry), ended_with_y(ended_load(placed), to));
}

void abi_StartWithPartner(void)
{
    abi_ended(StartWithPartner(cpu.x));
}

/*
 * @routine 0A9D:088 StopWhenLanded
 * @run 71:C088
 * WHAT IT DOES: Unless slot $54's frame is 4, asks StopOnLanding with a probe height of 4; on landing the object's animation is reset and it enters
 * its next state.
 * HOW: StopOnLanding, two stores, an INC.
 * Untraced: one byte.
 * WAITS: no.
 * Entry: X object, Y the probe's column, C passed on. Exit: the mover's registers, then N and Z of the new state when landed.
 */
LandWait StopWhenLanded(uint8_t slot, uint8_t column, bool carry_in)
{
    LandWait wait = { false, { 0 }, { 0 } };
    uint8_t frame, state;

    frame = RD8(OBJ->frame[0x54]);
    wait.end = ended_compare(frame, 4);
    if (frame == 4) {
        return wait;
    }
    wait.end = ended_kept();
    jsr_push(0xC094);
    wait.mover = StopOnLanding(4, slot, column, carry_in);
    jsr_pop();
    wait.asked = true;
    if (!mover_landed(wait.mover)) {
        return wait;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    state = INC8(OBJ->state[slot]);
    wait.end = ended_nz(state);
    return wait;
}

void abi_StopWhenLanded(void)
{
    land_wait_exit_shim(StopWhenLanded(cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/*
 * @routine 0A9D:113 SlowToPlayer
 * @run 71:C113
 * WHAT IT DOES: When the player is within 5 along X, the object's X speed becomes that distance and its direction byte OBJ_3618 the
 * side the player is on (inverted).
 * HOW: DistanceXToPlayer, SideOfPlayer.
 * WAITS: no.
 * Entry: X object. Exit: the compare with 5, or A the flipped side bit.
 */
Ended SlowToPlayer(uint8_t slot)
{
    PlayerDistance distance;
    SideResult side;
    uint8_t bits;
    Ended e;

    jsr_push(0xC116);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = then(ended_of_distance(distance), ended_compare(distance.distance, 5));
    if (distance.distance >= 5) {
        return e;
    }
    WR8(OBJ->vx_hi[slot], distance.distance);
    WR8(OBJ->vx_lo[slot], 0);
    jsr_push(0xC123);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_side(side));
    bits = (uint8_t)(side.sides & 0x01);
    e = then(e, ended_load(bits));
    bits = (uint8_t)(bits ^ 0x01);
    WR8(OBJ->f3618[slot], bits);
    return then(e, ended_load(bits));
}

void abi_SlowToPlayer(void)
{
    abi_ended(SlowToPlayer(cpu.x));
}

/*
 * @routine 0A9D:133 ChooseAttackByDistance
 * @run 71:C133
 * WHAT IT DOES: When the player is within $20 vertically the object resets its animation, sets OBJ_3720 1 and enters state 4; else when within
 * $18 along X it resets, sets OBJ_3720 2 and enters state 7.
 * HOW: DistanceYToPlayer, DistanceXToPlayer.
 * WAITS: no.
 * Entry: X object. Exit: the last compare or load.
 */
Ended ChooseAttackByDistance(uint8_t slot)
{
    PlayerDistance distance;
    Ended e;

    jsr_push(0xC136);
    distance = DistanceYToPlayer(slot);
    jsr_pop();
    e = then(ended_of_distance(distance), ended_compare(distance.distance, 0x20));
    if (distance.distance < 0x20) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->f3720[slot], 1);
        WR8(OBJ->state[slot], 4);
        return then(e, ended_load(4));
    }
    jsr_push(0xC14E);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_distance(distance));
    e = then(e, ended_compare(distance.distance, 0x18));
    if (distance.distance >= 0x18) {
        return e;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f3720[slot], 2);
    WR8(OBJ->state[slot], 7);
    return then(e, ended_load(7));
}

void abi_ChooseAttackByDistance(void)
{
    abi_ended(ChooseAttackByDistance(cpu.x));
}

/*
 * @routine 0A9D:28A TurnWhenWaitEnds
 * @run 71:C28A
 * WHAT IT DOES: Counts the angle byte down; at zero resets the animation, takes OBJ_3670 from the player's side (bit 1, vertical) and enters state 8.
 * HOW: DEC, SideOfPlayer and a shift.
 * WAITS: no.
 * Entry: X object. Exit: wait running: N and Z from the count; at zero A $08, C bit 0 of the sides.
 */
Ended TurnWhenWaitEnds(uint8_t slot)
{
    SideResult side;
    uint8_t left, shifted;
    Ended e;

    left = DEC8(OBJ->angle_hi[slot]);
    e = ended_nz(left);
    if (left != 0) {
        return e;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    jsr_push(0xC298);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_side(side));
    shifted = (uint8_t)(side.sides >> 1);
    e = then(e, ended_with_carry(ended_load(shifted), (side.sides & 0x01) != 0));
    WR8(OBJ->f3670[slot], shifted);
    WR8(OBJ->state[slot], 8);
    return then(e, ended_load(8));
}

void abi_TurnWhenWaitEnds(void)
{
    abi_ended(TurnWhenWaitEnds(cpu.x));
}

/*
 * @routine 0A9D:31E StepWithArmor
 * @run 71:C31E
 * WHAT IT DOES: Sets bits 0 and 1 of the object's OBJ_3148 while it steps the animation list at $C347; when the list ends it clears them, resets
 * the animation and enters state 5.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step; at the end A 5.
 */
Ended StepWithArmor(uint8_t slot, uint8_t y)
{
    FrameStep step;
    uint8_t bits;
    Ended e;

    bits = RD8(OBJ->f3148[slot]);
    bits = (uint8_t)(bits | 0x03);
    WR8(OBJ->f3148[slot], bits);
    step = step_list(slot, 0xC347, 0xC331, &y, true);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    bits = RD8(OBJ->f3148[slot]);
    bits = (uint8_t)(bits & 0xFC);
    WR8(OBJ->f3148[slot], bits);
    e = then(e, ended_load(bits));
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 5);
    return then(e, ended_load(5));
}

void abi_StepWithArmor(void)
{
    abi_ended(StepWithArmor(cpu.x, cpu.y));
}

/*
 * @routine 0A9D:1CF ChooseNextMove
 * @run 71:C1CF
 * WHAT IT DOES: While the object in slot 0 is in OBJ_2800 state 1 and the player is $50 or more away along X and the distance sum is even, the
 * object resets its animation and enters state 9; otherwise its wait OBJ_3568 counts down and at zero it resets, picks a new wait from the table
 * at $C226 by the distance sum and its next state from the table at $C223 by a counter OBJ_37D0, which goes up one.
 * HOW: DistanceXToPlayer, PlayerDistanceSum (twice, with the carry the compares left), table reads (bank 71).
 * WAITS: no.
 * Entry: X object, C entry (it reaches PlayerDistanceSum when a compare did not set it). Exit: the last compare, AND, DEC or INC.
 */
Ended ChooseNextMove(uint8_t slot, bool carry_in)
{
    PlayerDistance distance;
    Sum sum;
    uint8_t lead, bit, left, picked, wait, counter, index, next;
    Ended e;

    lead = RD8(OBJ->f2800[0]);
    e = ended_compare(lead, 1);
    if (lead == 1) {
        jsr_push(0xC1D9);
        distance = DistanceXToPlayer(slot);
        jsr_pop();
        e = then(e, ended_of_distance(distance));
        e = then(e, ended_compare(distance.distance, 0x50));
        if (distance.distance >= 0x50) {
            jsr_push(0xC1E0);
            sum = PlayerDistanceSum(slot, carry_of(e, carry_in));
            jsr_pop();
            e = then(e, of_sum(sum));
            bit = (uint8_t)(sum.value & 0x01);
            e = then(e, ended_load(bit));
            if (bit == 0) {
                WR8(OBJ->anim_step[slot], 0);
                WR8(OBJ->anim_timer[slot], 0);
                WR8(OBJ->state[slot], 9);
                return then(e, ended_load(9));
            }
        }
    }
    left = DEC8(OBJ->angle_hi[slot]);
    e = then(e, ended_nz(left));
    if (left != 0) {
        return e;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    jsr_push(0xC1FE);
    sum = PlayerDistanceSum(slot, carry_of(e, carry_in));
    jsr_pop();
    e = then(e, of_sum(sum));
    picked = (uint8_t)(sum.value & 0x03);
    e = then(e, ended_with_y(ended_load(picked), picked));
    wait = rd8((uint16_t)(0xC226 + picked));
    WR8(OBJ->angle_hi[slot], wait);
    e = then(e, ended_load(wait));
    counter = RD8(OBJ->f37d0[slot]);
    counter = (uint8_t)(counter & 0x07);
    e = then(e, ended_with_y(ended_load(counter), counter));
    index = rd8((uint16_t)(0xC21B + counter));
    e = then(e, ended_with_y(ended_load(index), index));
    next = rd8((uint16_t)(0xC223 + index));
    WR8(OBJ->state[slot], next);
    e = then(e, ended_load(next));
    counter = INC8(OBJ->f37d0[slot]);
    return then(e, ended_nz(counter));
}

void abi_ChooseNextMove(void)
{
    abi_ended(ChooseNextMove(cpu.x, (cpu.p & P_C) != 0));
}

/*
 * @routine 0A9D:2A2 StepAndShout
 * @run 71:C2A2
 * WHAT IT DOES: Steps the animation list a table at $C2E4 names for OBJ_3670; when the list ends it resets, enters state 7 and picks a wait
 * from the table at $C2E0 by the distance sum; on a new frame $26 or $2D it plays sound $1F if the object is on the first screen.
 * HOW: StepObjectFrames, PlayerDistanceSum, a tail jump into SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
 */
ScreenSoundEnd StepAndShout(uint8_t slot)
{
    ScreenSoundEnd r = { { 0 }, { 0 }, false, { 0 } };
    FrameStep step;
    Sum sum;
    uint8_t phase, index, y, frame, picked, wait, list_low, list_high;
    Ended e;

    phase = RD8(OBJ->f3670[slot]);
    index = (uint8_t)(phase << 1);
    y = index;
    list_low = rd8((uint16_t)(0xC2E4 + index));
    WR8(ZP->temp10, list_low);
    list_high = rd8((uint16_t)(0xC2E5 + index));
    WR8(ZP->temp11, list_high);
    e = ended_with_carry(ended_with_y(ended_load(list_high), index), (phase & 0x80) != 0);
    jsr_push(0xC2B4);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = then(e, ended_of_frame_step(step, true, y));
    r.end = e;
    if (step == FRAME_LIST_ENDED) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 7);
        e = then(e, ended_load(7));
        jsr_push(0xC2D6);
        sum = PlayerDistanceSum(slot, false);
        jsr_pop();
        e = then(e, of_sum(sum));
        picked = (uint8_t)(sum.value & 0x03);
        wait = rd8((uint16_t)(0xC2E0 + picked));
        WR8(OBJ->angle_hi[slot], wait);
        r.end = then(e, ended_with_y(ended_load(wait), picked));
        return r;
    }
    if (step != FRAME_CHANGED) {
        return r;
    }
    frame = RD8(OBJ->frame[slot]);
    e = then(e, ended_compare(frame, 0x26));
    if (frame != 0x26) {
        e = then(e, ended_compare(frame, 0x2D));
        if (frame != 0x2D) {
            r.end = e;
            return r;
        }
    }
    r.before = then(e, ended_load(0x1F));
    r.end = ended_kept();
    r.called = true;
    r.sound = SoundIfOnFirstScreen(0x1F, slot, y);
    return r;
}

void abi_StepAndShout(void)
{
    abi_screen_sound_end(StepAndShout(cpu.x));
}

/*
 * @routine 0A9D:410 ShootArrow
 * @run 71:C410
 * WHAT IT DOES: Starts a kind $9C object at this object's place in the first free slot from $30, plays sound $15 if it is on the first screen,
 * gives the new object an X and Y speed from a table at $C443 (two bytes by the cue's index doubled) and moves it once.
 * HOW: The slot and the index are kept in zRichterObject and zHitSlot; FindFreeSlotFrom30, StartObjectAtPlace, SoundIfOnFirstScreen,
 * ApplyVelocity.
 * Untraced: one byte.
 * WAITS: no.
 * Entry: X object, Y the cue's index. Exit: see the wrapper.
 */
ShotEnd ShootArrow(uint8_t slot, uint8_t cue)
{
    ShotEnd r = { { 0 }, { 0 }, false, { 0 }, { 0 } };
    uint8_t last, found_slot, placed, index, byte, saved, restored;
    bool found;
    Ended e;

    WR8(ZP->richter_object, slot);
    WR8(ZP->hit_slot, cue);
    jsr_push(0xC417);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), last), found_slot), found);
    if (!found) {
        r.end = e;
        return r;
    }
    jsr_push(0xC41E);
    placed = StartObjectAtPlace(0x9C, slot, found_slot);
    jsr_pop();
    e = then(e, ended_load(placed));
    r.before = then(e, ended_load(0x15));
    jsr_push(0xC423);
    r.sound = SoundIfOnFirstScreen(0x15, slot, found_slot);
    jsr_pop();
    r.called = true;
    saved = RD8(ZP->hit_slot);
    index = (uint8_t)(saved << 1);
    byte = rd8((uint16_t)(0xC443 + index));
    WR8(OBJ->vx_hi[found_slot], byte);
    byte = rd8((uint16_t)(0xC444 + index));
    WR8(OBJ->vy_hi[found_slot], byte);
    r.mid = ended_with_carry(ended_with_y(ended_with_a(ended_nz(byte), byte), found_slot), (saved & 0x80) != 0);
    jsr_push(0xC437);
    r.move = ApplyVelocity(found_slot);
    jsr_pop();
    restored = RD8(ZP->richter_object);
    r.end = then(ended_with_y(ended_of_axis_move(r.move), found_slot), ended_nz(restored));
    return r;
}

void abi_ShootArrow(void)
{
    ShotEnd r = ShootArrow(cpu.x, cpu.y);

    abi_ended(r.before);
    if (r.called) {
        sound_if_on_first_screen_exit_shim(r.sound);
        abi_ended(r.mid);
    }
    abi_ended(r.end);
}

/*
 * @routine 0A9D:3DA FlashAndCount
 * @run 71:C3DA
 * @stop C401
 * WHAT IT DOES: Flashes the object ($8A), steps the animation list at $C401, counts OBJ_3778 down and clears the object at zero; while it runs, when
 * the count equals one of eight cue values (table at $C43B) it shoots an arrow with that cue's index.
 * HOW: StepObjectFrames, DEC, a scan of the table, ShootArrow (reached by a branch).
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
 */
ShotEnd FlashAndCount(uint8_t slot, uint8_t y)
{
    ShotEnd r = { { 0 }, { 0 }, false, { 0 }, { 0 } };
    FrameStep step;
    uint8_t left, count, cue, index;
    Ended e;

    WR8(OBJ->flash_flags[slot], 0x8A);
    step = step_list(slot, 0xC401, 0xC3EA, &y, false);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    left = DEC8(OBJ->f3778[slot]);
    e = then(e, ended_nz(left));
    if (left == 0) {
        ClearObject(slot);
        r.end = e;
        return r;
    }
    index = 0;
    e = then(e, ended_with_y(ended_kept(), 0));
    do {
        count = RD8(OBJ->f3778[slot]);
        cue = rd8((uint16_t)(0xC43B + index));
        e = then(e, ended_compare(count, cue));
        if (count == cue) {
            r = ShootArrow(slot, index);
            r.before = then(e, r.before);
            return r;
        }
        index++;
        e = then(e, ended_with_y(ended_nz(index), index));
        e = then(e, cpy(index, 8));
    } while (index < 8);
    r.end = e;
    return r;
}

void abi_FlashAndCount(void)
{
    ShotEnd r = FlashAndCount(cpu.x, cpu.y);

    abi_ended(r.before);
    if (r.called) {
        sound_if_on_first_screen_exit_shim(r.sound);
        abi_ended(r.mid);
    }
    abi_ended(r.end);
}
