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

/* ApplyGravity and ApplyGravityUnlimited's exit: A, N, Z, C and V as gravity_exit_shim leaves them. */
static Ended ended_of_gravity(GravityResult g)
{
    Ended e;

    if (g.gained) {
        e = ended_with_a(ended_nz(g.carried ? g.high : g.low), g.low);
        return ended_with_overflow(ended_with_carry(e, g.carried), g.overflow);
    }
    e = ended_with_a(ended_nz((uint8_t)(g.high - 7)), g.high);
    return ended_with_carry(e, true);
}

/* A wait's exit: what came before, the callee's exit when it ran, then the rest. */
void wait_exit_shim(WaitEnd r)
{
    abi_ended(r.before);
    if (r.picked) {
        abi_random_end(r.timers);
    }
    abi_ended(r.after);
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

/*
 * @routine 0A9E:4C3 HopAndSettle
 * @run 71:CCC3
 * WHAT IT DOES: An object with a state and on the first screen row is lifted by sixteen times its next kind, moved by its Y speed, given gravity and asked for
 * the floor (request 9, probe height 10): on floor its Y speed is stopped; then it is put back down by the same lift.
 * HOW: Two 16-bit adds of the lift, ApplyVelocityY, ApplyGravity, ProbeTerrain.
 * WAITS: no.
 * Entry: X object, Y the probe's column, C passed to the lookup. Exit: see the wrapper.
 */
HopEnd HopAndSettle(uint8_t slot, uint8_t column, bool carry_in)
{
    HopEnd r = { { 0 }, false, { 0 }, { 0 } };
    uint8_t state, y_high, kind, lift, low, high, hit;
    Sum sum;
    AxisMove move;
    GravityResult gravity;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    r.before = e;
    if (state == 0) {
        return r;
    }
    y_high = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_load(y_high));
    r.before = e;
    if (y_high != 0) {
        return r;
    }
    kind = RD8(OBJ->next_kind[slot]);
    lift = (uint8_t)(kind << 4);
    WR8(ZP->temp00, lift);
    low = RD8(OBJ->y_lo[slot]);
    lift = RD8(ZP->temp00);
    sum = add_bytes(low, lift, false);
    WR8(OBJ->y_lo[slot], sum.value);
    high = RD8(OBJ->y_hi[slot]);
    sum = add_bytes(high, 0, sum.carry);
    WR8(OBJ->y_hi[slot], sum.value);
    e = then(e, of_sum(sum));
    jsr_push(0xCCEA);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    jsr_push(0xCCED);
    gravity = ApplyGravity(slot);
    jsr_pop();
    e = then(e, ended_of_gravity(gravity));
    WR8(ZP->product.mid, 0x0A);
    jsr_push(0xCCF6);
    r.probe = ProbeTerrain(9, slot, column, carry_of(e, carry_in));
    jsr_pop();
    r.probed = true;
    hit = (uint8_t)(r.probe.answer & 0x01);
    r.after = ended_load(hit);
    if (hit != 0) {
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0);
    }
    kind = RD8(OBJ->next_kind[slot]);
    lift = (uint8_t)(kind << 4);
    WR8(ZP->temp00, lift);
    low = RD8(OBJ->y_lo[slot]);
    lift = RD8(ZP->temp00);
    sum = sub_bytes(low, lift, true);
    WR8(OBJ->y_lo[slot], sum.value);
    high = RD8(OBJ->y_hi[slot]);
    sum = sub_bytes(high, 0, sum.carry);
    WR8(OBJ->y_hi[slot], sum.value);
    r.after = then(r.after, of_sum(sum));
    return r;
}

void abi_HopAndSettle(void)
{
    HopEnd r = HopAndSettle(cpu.x, cpu.y, (cpu.p & P_C) != 0);

    abi_ended(r.before);
    if (r.probed) {
        probe_exit_shim(r.probe);
    }
    abi_ended(r.after);
}

/*
 * @routine 0A9E:51B SetHitboxByFrame
 * @run 71:CD1B
 * WHAT IT DOES: Turns the object's hitbox on and sets its X offset from a table at $CD39 by its frame less $80; from frame $86 on, the hitbox goes off and the
 * offset 0.
 * HOW: A compare of the frame and a table read (bank 71).
 * Untraced: frames $86 and over (CD32).
 * WAITS: no.
 * Entry: X object. Exit: frame under $86: A the table byte, Y the index, C and V from the subtract; else A the frame, the compare's flags.
 */
Ended SetHitboxByFrame(uint8_t slot)
{
    uint8_t frame, byte;
    Sum index;
    Ended e;

    WR8(OBJ->hitbox_on[slot], 2);
    frame = RD8(OBJ->frame[slot]);
    e = ended_compare(frame, 0x86);
    if (frame >= 0x86) {
        WR8(OBJ->hitbox_on[slot], 0);
        WR8(OBJ->hitbox_dx[slot], 0);
        return e;
    }
    index = sub_bytes(frame, 0x80, true);
    byte = rd8((uint16_t)(0xCD39 + index.value));
    WR8(OBJ->hitbox_dx[slot], byte);
    return then(of_sum(index), ended_with_y(ended_load(byte), index.value));
}

void abi_SetHitboxByFrame(void)
{
    abi_ended(SetHitboxByFrame(cpu.x));
}

/*
 * @routine 0A9E:5F5 CountDownThenAdvance
 * @run 71:CDF5
 * WHAT IT DOES: A 16-bit wait (OBJ_3510 low, the life timer high): while it is not zero it counts down; at zero it picks new timers (PickTimersAtRandomB),
 * stops the animation and enters state 2.
 * HOW: An ORA of the two bytes, a 16-bit subtract.
 * Untraced: the arm at zero (CDFD to CE0B).
 * WAITS: no.
 * Entry: X object. Exit: see the wrapper.
 */
WaitEnd CountDownThenAdvance(uint8_t slot)
{
    WaitEnd r = { { 0 }, false, { 0 }, { 0 } };
    uint8_t high, low, merged, state;
    Sum sum;

    high = RD8(OBJ->life_timer[slot]);
    low = RD8(OBJ->f3510[slot]);
    merged = (uint8_t)(high | low);
    r.before = ended_load(merged);
    if (merged == 0) {
        jsr_push(0xCE00);
        r.timers = PickTimersAtRandomB(slot);
        jsr_pop();
        r.picked = true;
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 2);
        r.after = ended_load(2);
        return r;
    }
    low = RD8(OBJ->f3510[slot]);
    sum = sub_bytes(low, 1, true);
    WR8(OBJ->f3510[slot], sum.value);
    high = RD8(OBJ->life_timer[slot]);
    sum = sub_bytes(high, 0, sum.carry);
    WR8(OBJ->life_timer[slot], sum.value);
    (void)state;
    r.after = of_sum(sum);
    return r;
}

void abi_CountDownThenAdvance(void)
{
    wait_exit_shim(CountDownThenAdvance(cpu.x));
}

/*
 * @routine 0A9E:61E CountDownAngleThenAdvance
 * @run 71:CE1E
 * WHAT IT DOES: With the object's X high byte 0, a 16-bit wait (OBJ_3568 high, OBJ_35C0 low) counts down; at zero it picks a new angle
 * (PickAngleAtRandomB), stops the animation, clears OBJ_2800 and enters state 3.
 * HOW: An ORA of the two bytes, a 16-bit subtract.
 * Untraced: everything past the X high byte test.
 * WAITS: no.
 * Entry: X object. Exit: see the wrapper.
 */
WaitEnd CountDownAngleThenAdvance(uint8_t slot)
{
    WaitEnd r = { { 0 }, false, { 0 }, { 0 } };
    uint8_t x_high, high, low, merged;
    Sum sum;

    x_high = RD8(OBJ->x_hi[slot]);
    r.before = ended_load(x_high);
    if (x_high != 0) {
        return r;
    }
    high = RD8(OBJ->angle_hi[slot]);
    low = RD8(OBJ->angle_lo[slot]);
    merged = (uint8_t)(high | low);
    r.before = ended_load(merged);
    if (merged == 0) {
        jsr_push(0xCE2E);
        r.timers = PickAngleAtRandomB(slot);
        jsr_pop();
        r.picked = true;
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->f2800[slot], 0);
        WR8(OBJ->state[slot], 3);
        r.after = ended_load(3);
        return r;
    }
    low = RD8(OBJ->angle_lo[slot]);
    sum = sub_bytes(low, 1, true);
    WR8(OBJ->angle_lo[slot], sum.value);
    high = RD8(OBJ->angle_hi[slot]);
    sum = sub_bytes(high, 0, sum.carry);
    WR8(OBJ->angle_hi[slot], sum.value);
    r.after = of_sum(sum);
    return r;
}

void abi_CountDownAngleThenAdvance(void)
{
    wait_exit_shim(CountDownAngleThenAdvance(cpu.x));
}

/*
 * @routine 0A9E:5EE CountBothWaits
 * @run 71:CDEE
 * WHAT IT DOES: Runs both waits, CountDownThenAdvance and CountDownAngleThenAdvance.
 * HOW: Two calls.
 * WAITS: no.
 * Entry: X object. Exit: as the second.
 */
WaitPair CountBothWaits(uint8_t slot)
{
    WaitPair pair;

    jsr_push(0xCDF1);
    pair.first = CountDownThenAdvance(slot);
    jsr_pop();
    jsr_push(0xCDF4);
    pair.second = CountDownAngleThenAdvance(slot);
    jsr_pop();
    return pair;
}

void abi_CountBothWaits(void)
{
    WaitPair pair = CountBothWaits(cpu.x);

    wait_exit_shim(pair.first);
    wait_exit_shim(pair.second);
}

/*
 * @routine 0A9E:53F StartKnifeThrower
 * @run 71:CD3F
 * WHAT IT DOES: Loads the object's record. If its next kind is a count (bit 7 clear) it starts that many kind $30 objects after it, each numbered from $81 up, each
 * remembering this object's slot and its slot plus count; then the count loses bit 7, its low bit decides the facing, and the throw, the timers and the angle are
 * picked (ThrowByNextKindB, PickTimersAtRandomB, PickAngleAtRandomB).
 * HOW: LoadObjectRecord, a loop of ClearObjectInY and StartObjectAtPlace.
 * Untraced: one byte.
 * WAITS: no.
 * Entry: X object. Exit: see the wrapper (the three picks, one after the other).
 */
ThrowChain StartKnifeThrower(uint8_t slot)
{
    ThrowChain r = { { 0 }, { 0 }, { { 0 }, false, { 0 } }, { { 0 }, false, { 0 } } };
    uint8_t nz, last, kind, count, to, number, shared, bits, remaining;
    bool carry;
    Sum sum;
    Ended e;

    jsr_push(0xCD42);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    kind = RD8(OBJ->next_kind[slot]);
    e = then(e, ended_load(kind));
    if ((kind & 0x80) == 0) {
        WR8(OBJ->f37d0[slot], slot);
        kind = RD8(OBJ->next_kind[slot]);
        sum = add_bytes(slot, kind, false);
        WR8(OBJ->f3828[slot], sum.value);
        e = then(e, of_sum(sum));
        to = (uint8_t)(slot + 1);
        number = 0x81;
        WR8(ZP->temp00, number);
        do {
            jsr_push(0xCD5C);
            ClearObjectInY(to);
            jsr_pop();
            jsr_push(0xCD61);
            StartObjectAtPlace(0x30, slot, to);
            jsr_pop();
            number = RD8(ZP->temp00);
            WR8(OBJ->next_kind[to], number);
            WR8(OBJ->f37d0[to], slot);
            shared = RD8(OBJ->f3828[slot]);
            WR8(OBJ->f3828[to], shared);
            INC8(ZP->temp00);
            to++;
            remaining = DEC8(OBJ->next_kind[slot]);
        } while (remaining != 0);
        e = then(e, ended_with_y(ended_nz(remaining), to));
    }
    count = RD8(OBJ->next_kind[slot]);
    count = (uint8_t)(count & 0x7F);
    WR8(OBJ->next_kind[slot], count);
    bits = (uint8_t)(count & 0x01);
    bits = (uint8_t)(bits ^ 0x01);
    WR8(OBJ->facing_left[slot], bits);
    r.before = then(e, ended_load(bits));
    jsr_push(0xCD8A);
    r.thrown = ThrowByNextKindB(slot);
    jsr_pop();
    jsr_push(0xCD8D);
    r.timers = PickTimersAtRandomB(slot);
    jsr_pop();
    jsr_push(0xCD90);
    r.angle = PickAngleAtRandomB(slot);
    jsr_pop();
    return r;
}

void abi_StartKnifeThrower(void)
{
    ThrowChain r = StartKnifeThrower(cpu.x);

    abi_ended(r.before);
    abi_ended(r.thrown);
    abi_random_end(r.timers);
    abi_random_end(r.angle);
}

/*
 * @routine 0A9A:33A BounceAtWalls
 * @run 70:AB3A
 * WHAT IT DOES: Moves the object along X and asks the floor (request 9); when the floor under the side it is moving to has no wall bit, or the wall
 * ahead (request 2) has none, it goes on; otherwise it turns round (NegateVelocityX) and moves once more.
 * HOW: ApplyVelocityX, ProbeTerrain twice, NegateVelocityX; the answer is C: set when it turned.
 * WAITS: no.
 * Entry: X object, Y the probe's column. Exit: C set when the object turned round; A and the flags as the last instructions left them.
 */
TurnEnd BounceAtWalls(uint8_t slot, uint8_t column)
{
    TurnEnd r = { { 0 }, { 0 }, false, { 0 }, { 0 }, false, { 0 } };
    AxisMove move;
    uint8_t speed, side, flags, flags2;
    Ended e;

    jsr_push(0xAB3D);
    move = ApplyVelocityX(slot);
    jsr_pop();
    r.e0 = ended_of_axis_move(move);
    jsr_push(0xAB42);
    r.probe1 = ProbeTerrain(9, slot, column, r.e0.c);
    jsr_pop();
    speed = RD8(OBJ->vx_hi[slot]);
    side = (uint8_t)((speed >> 7) ^ 0x01);
    e = ended_with_carry(ended_with_y(ended_load(side), side), (speed & 0x40) != 0);
    flags = rd8((uint16_t)(TileHit + 1 + side));
    flags = (uint8_t)(flags & 0x01);
    e = then(e, ended_load(flags));
    r.e1 = e;
    if (flags != 0) {
        jsr_push(0xAB58);
        r.probe2 = ProbeTerrain(2, slot, side, e.c);
        jsr_pop();
        r.second = true;
        speed = RD8(OBJ->vx_hi[slot]);
        side = (uint8_t)(((speed >> 7) ^ 0x01) << 1);
        e = ended_with_carry(ended_with_y(ended_load(side), side), false);
        flags = rd8((uint16_t)(TileHit + side));
        flags2 = rd8((uint16_t)(TileHit + 1 + side));
        flags = (uint8_t)((flags | flags2) & 0x02);
        e = then(e, ended_load(flags));
        r.e2 = e;
        if (flags == 0) {
            r.e2 = then(e, ended_with_carry(ended_kept(), false));
            return r;
        }
    }
    jsr_push(0xAB72);
    NegateVelocityX(slot);
    jsr_pop();
    jsr_push(0xAB75);
    move = ApplyVelocityX(slot);
    jsr_pop();
    r.e3 = then(ended_of_axis_move(move), ended_with_carry(ended_kept(), true));
    r.turned = true;
    return r;
}

/* BounceAtWalls's exit, in the order its instructions leave it. */
void bounce_exit_shim(TurnEnd r)
{
    abi_ended(r.e0);
    probe_exit_shim(r.probe1);
    abi_ended(r.e1);
    if (r.second) {
        probe_exit_shim(r.probe2);
        abi_ended(r.e2);
    }
    if (r.turned) {
        abi_ended(r.e3);
    }
}

/* The carry BounceAtWalls leaves: set when it turned round. */
static bool turn_carry(TurnEnd r)
{
    return r.turned;
}

void abi_BounceAtWalls(void)
{
    bounce_exit_shim(BounceAtWalls(cpu.x, cpu.y));
}

/*
 * @routine 0A9D:12B BounceWithHeight4
 * @run 71:C12B
 * WHAT IT DOES: BounceAtWalls with a probe height of 4.
 * HOW: A store into zProduct1, then the call.
 * WAITS: no.
 * Entry: X object, Y the probe's column. Exit: as BounceAtWalls.
 */
TurnEnd BounceWithHeight4(uint8_t slot, uint8_t column)
{
    TurnEnd r;

    WR8(ZP->product.mid, 4);
    jsr_push(0xC132);
    r = BounceAtWalls(slot, column);
    jsr_pop();
    return r;
}

void abi_BounceWithHeight4(void)
{
    bounce_exit_shim(BounceWithHeight4(cpu.x, cpu.y));
}

/*
 * @routine 0A9D:0C0 WalkByPhase
 * @run 71:C0C0
 * WHAT IT DOES: A walking step for the phase in A (0 to 3): the phase times four picks an animation list pointer and a speed word from the table at $C0ED;
 * the list is stepped; when a new frame comes the object slows toward the player (SlowToPlayer), turns its speed round when its direction byte OBJ_3618 says,
 * and moves with BounceWithHeight4.
 * HOW: StepObjectFrames, SlowToPlayer, NegateVelocityX, BounceWithHeight4.
 * WAITS: no.
 * Entry: X object, A the phase, Y the probe's column. Exit: the step's flags when no new frame; else as BounceAtWalls.
 */
WalkEnd WalkByPhase(uint8_t slot, uint8_t phase, uint8_t column)
{
    WalkEnd r = { { 0 }, false, { 0 } };
    FrameStep step;
    uint8_t index, byte, y, direction;
    Ended e;

    index = (uint8_t)(phase << 2);
    byte = rd8((uint16_t)(0xC0ED + index));
    WR8(ZP->temp10, byte);
    byte = rd8((uint16_t)(0xC0EE + index));
    WR8(ZP->temp11, byte);
    byte = rd8((uint16_t)(0xC0EF + index));
    WR8(OBJ->vx_lo[slot], byte);
    byte = rd8((uint16_t)(0xC0F0 + index));
    WR8(OBJ->vx_hi[slot], byte);
    y = index;
    jsr_push(0xC0DC);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.pre = ended_of_frame_step(step, true, y);
    if (step != FRAME_CHANGED) {
        return r;
    }
    e = r.pre;
    jsr_push(0xC0E1);
    e = then(e, SlowToPlayer(slot));
    jsr_pop();
    direction = RD8(OBJ->f3618[slot]);
    e = then(e, ended_load(direction));
    r.pre = e;
    if (direction != 0) {
        jsr_push(0xC0E9);
        NegateVelocityX(slot);
        jsr_pop();
        r.pre = then(e, ended_with_carry(ended_kept(), false));
    }
    jsr_push(0xC0EC);
    r.turn = BounceWithHeight4(slot, column);
    jsr_pop();
    r.walked = true;
    return r;
}

/* A walk's exit: what came before, then BounceAtWalls's. */
void walk_phase_exit_shim(WalkEnd r)
{
    abi_ended(r.pre);
    if (r.walked) {
        bounce_exit_shim(r.turn);
    }
}

void abi_WalkByPhase(void)
{
    walk_phase_exit_shim(WalkByPhase(cpu.x, cpu.a, cpu.y));
}

/* The two attack walkers share their tail: choose an attack, copy the direction to the facing, count down and flip the direction at zero. */
static AttackWalk attack_walk(uint8_t slot, uint8_t phase, uint8_t column, uint16_t ret_walk, uint16_t ret_choose)
{
    AttackWalk r;
    uint8_t direction, left;
    Ended e;

    jsr_push(ret_walk);
    r.walk = WalkByPhase(slot, phase, column);
    jsr_pop();
    jsr_push(ret_choose);
    e = ChooseAttackByDistance(slot);
    jsr_pop();
    direction = RD8(OBJ->f3618[slot]);
    WR8(OBJ->facing_left[slot], direction);
    e = then(e, ended_load(direction));
    left = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(left));
    if (left == 0) {
        WR8(OBJ->f3510[slot], 0xB0);
        direction = RD8(OBJ->f3618[slot]);
        direction = (uint8_t)(direction ^ 0x01);
        WR8(OBJ->f3618[slot], direction);
        e = then(e, ended_load(direction));
    }
    r.rest = e;
    return r;
}

void attack_walk_exit_shim(AttackWalk r)
{
    walk_phase_exit_shim(r.walk);
    abi_ended(r.rest);
}

/*
 * @routine 0A9D:0A0 WalkAndAttackA
 * @run 71:C0A0
 * WHAT IT DOES: WalkByPhase 0 and ChooseAttackByDistance; the object faces its walking direction, counts OBJ_3510 down and at zero resets it to $B0
 * and turns the direction round.
 * HOW: attack_walk with phase 0.
 * WAITS: no.
 * Entry: X object, Y the probe's column. Exit: see attack_walk_exit_shim.
 */
AttackWalk WalkAndAttackA(uint8_t slot, uint8_t column)
{
    return attack_walk(slot, 0, column, 0xC0A4, 0xC0A7);
}

void abi_WalkAndAttackA(void)
{
    attack_walk_exit_shim(WalkAndAttackA(cpu.x, cpu.y));
}

/*
 * @routine 0A9D:163 WalkAndAttackB
 * @run 71:C163
 * WHAT IT DOES: As WalkAndAttackA with phase 3.
 * HOW: attack_walk with phase 3.
 * WAITS: no.
 * Entry and exit: as WalkAndAttackA.
 */
AttackWalk WalkAndAttackB(uint8_t slot, uint8_t column)
{
    return attack_walk(slot, 3, column, 0xC168, 0xC16B);
}

void abi_WalkAndAttackB(void)
{
    attack_walk_exit_shim(WalkAndAttackB(cpu.x, cpu.y));
}

/* The part of the chasers the second one branches into: the object is sent to state 3 when the player is $40 or more away vertically. */
static ChaseEnd chase_tail(uint8_t slot)
{
    ChaseEnd r = { { 0 }, { 0 }, { 0 } };
    PlayerDistance distance;
    Ended e;

    jsr_push(0xC1A1);
    distance = DistanceYToPlayer(slot);
    jsr_pop();
    e = then(ended_of_distance(distance), ended_compare(distance.distance, 0x40));
    if (distance.distance < 0x40) {
        r.tail = e;
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 3);
    r.tail = then(e, ended_load(3));
    return r;
}

/* The chasers' exit: the distance check, the walk, the move choice, then the tail. */
void chase_exit_shim(ChaseEnd r)
{
    abi_ended(r.head);
    walk_phase_exit_shim(r.walk);
    abi_ended(r.mid);
    abi_ended(r.tail);
}

/* Both chasers: set a state when the player is near, walk, choose the next move, then the shared tail. */
static ChaseEnd chase(uint8_t slot, uint8_t column, bool carry_in, bool far_when_ge, uint8_t limit, uint8_t near_state, bool invert,
                      uint16_t ret_distance, uint16_t ret_walk, uint16_t ret_move)
{
    ChaseEnd r;
    PlayerDistance distance;
    uint8_t facing;
    Ended e;
    ChaseEnd tail;

    jsr_push(ret_distance);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = then(ended_of_distance(distance), ended_compare(distance.distance, limit));
    if (far_when_ge ? distance.distance >= limit : distance.distance < limit) {
        WR8(OBJ->state[slot], near_state);
        e = then(e, ended_load(near_state));
    }
    r.head = e;
    jsr_push(ret_walk);
    r.walk = WalkByPhase(slot, 1, column);
    jsr_pop();
    facing = RD8(OBJ->facing_left[slot]);
    if (invert) {
        facing = (uint8_t)(facing ^ 0x01);
    }
    WR8(OBJ->f3618[slot], facing);
    r.mid = ended_load(facing);
    jsr_push(ret_move);
    r.mid = then(r.mid, ChooseNextMove(slot, carry_in));
    jsr_pop();
    tail = chase_tail(slot);
    r.tail = tail.tail;
    return r;
}

/*
 * @routine 0A9D:184 ChasePlayerA
 * @run 71:C184
 * WHAT IT DOES: Enters state 5 when the player is within $64 along X, walks (phase 1), takes its facing as its direction, chooses its next move, and goes to state 3
 * when the player is $40 or more away vertically.
 * HOW: DistanceXToPlayer, WalkByPhase, ChooseNextMove, DistanceYToPlayer.
 * WAITS: no.
 * Entry: X object, Y the probe's column, C passed on. Exit: see chase_exit_shim.
 */
ChaseEnd ChasePlayerA(uint8_t slot, uint8_t column, bool carry_in)
{
    return chase(slot, column, carry_in, false, 0x64, 5, false, 0xC187, 0xC195, 0xC19E);
}

void abi_ChasePlayerA(void)
{
    chase_exit_shim(ChasePlayerA(cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/*
 * @routine 0A9D:1B1 ChasePlayerB
 * @run 71:C1B1
 * WHAT IT DOES: As ChasePlayerA, but it enters state 4 when the player is $78 or more away along X and its direction is its facing turned round.
 * HOW: As ChasePlayerA.
 * WAITS: no.
 * Entry and exit: as ChasePlayerA.
 */
ChaseEnd ChasePlayerB(uint8_t slot, uint8_t column, bool carry_in)
{
    return chase(slot, column, carry_in, true, 0x78, 4, true, 0xC1B4, 0xC1C2, 0xC1CD);
}

void abi_ChasePlayerB(void)
{
    chase_exit_shim(ChasePlayerB(cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/*
 * @routine 0A9D:22A StepAndLunge
 * @run 71:C22A
 * WHAT IT DOES: Steps the animation list at $C260; when it ends the object resets its animation and enters state 4; on the new frame 7 it plays sound $1F
 * (on the first screen), takes an X speed from a table at $C25E by its facing and moves with BounceWithHeight4.
 * HOW: StepObjectFrames, SoundIfOnFirstScreen, BounceWithHeight4.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
 */
StepBounce StepAndLunge(uint8_t slot, uint8_t y)
{
    StepBounce r = { { 0 }, false, { 0 } };
    FrameStep step;
    uint8_t frame, facing, byte;

    step = step_list(slot, 0xC260, 0xC235, &y, true);
    r.pre = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step == FRAME_LIST_ENDED) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 4);
        r.pre = then(r.pre, ended_load(4));
        return r;
    }
    if (step != FRAME_CHANGED) {
        return r;
    }
    frame = RD8(OBJ->frame[slot]);
    r.pre = then(r.pre, ended_compare(frame, 7));
    if (frame != 7) {
        return r;
    }
    jsr_push(0xC245);
    SoundIfOnFirstScreen(0x1F, slot, y);
    jsr_pop();
    facing = RD8(OBJ->facing_left[slot]);
    byte = rd8((uint16_t)(0xC25E + facing));
    WR8(OBJ->vx_hi[slot], byte);
    jsr_push(0xC251);
    r.turn = BounceWithHeight4(slot, facing);
    jsr_pop();
    r.bounced = true;
    return r;
}

/* A stepping routine that may end in the bounce: what came before, then the bounce's exit. */
void step_bounce_exit_shim(StepBounce r)
{
    abi_ended(r.pre);
    if (r.bounced) {
        bounce_exit_shim(r.turn);
    }
}

void abi_StepAndLunge(void)
{
    step_bounce_exit_shim(StepAndLunge(cpu.x, cpu.y));
}

/*
 * @routine 0A9D:366 StepAndPounce
 * @run 71:C366
 * WHAT IT DOES: Sets OBJ_3359 3 and steps the animation list at $C3C5; when it ends OBJ_3359 becomes 2, the animation resets and the object enters state 4; on
 * the new frame $0E it plays sound $1A; on frames $0C and $0D it takes an X speed from a table at $C3BD by the frame and its facing and moves with
 * BounceWithHeight4.
 * HOW: StepObjectFrames, SoundIfOnFirstScreen, BounceWithHeight4.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
 */
StepBounce StepAndPounce(uint8_t slot, uint8_t y)
{
    StepBounce r = { { 0 }, false, { 0 } };
    FrameStep step;
    uint8_t frame, facing, index, byte;
    Sum diff;
    Ended e;

    WR8_IDX((uint16_t)(COL(OBJ->f3358) + 1), slot, 3);
    step = step_list(slot, 0xC3C5, 0xC376, &y, true);
    r.pre = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step == FRAME_LIST_ENDED) {
        WR8_IDX((uint16_t)(COL(OBJ->f3358) + 1), slot, 2);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 4);
        r.pre = then(r.pre, ended_load(4));
        return r;
    }
    if (step != FRAME_CHANGED) {
        return r;
    }
    frame = RD8(OBJ->frame[slot]);
    e = then(r.pre, ended_compare(frame, 0x0E));
    if (frame == 0x0E) {
        jsr_push(0xC386);
        SoundIfOnFirstScreen(0x1A, slot, y);
        jsr_pop();
        e = then(e, ended_load(0x1A));
    }
    frame = RD8(OBJ->frame[slot]);
    diff = sub_bytes(frame, 0x0C, true);
    e = then(e, of_sum(diff));
    e = then(e, ended_compare(diff.value, 2));
    r.pre = e;
    if (diff.value >= 2) {
        return r;
    }
    index = (uint8_t)(diff.value << 2);
    WR8(ZP->temp00, index);
    e = then(e, ended_with_carry(ended_load(index), false));
    facing = RD8(OBJ->facing_left[slot]);
    index = (uint8_t)((facing << 1) + RD8(ZP->temp00));
    byte = rd8((uint16_t)(0xC3BE + index));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xC3BD + index));
    WR8(OBJ->vx_lo[slot], byte);
    jsr_push(0xC3AB);
    r.turn = BounceWithHeight4(slot, index);
    jsr_pop();
    r.bounced = true;
    return r;
}

void abi_StepAndPounce(void)
{
    step_bounce_exit_shim(StepAndPounce(cpu.x, cpu.y));
}

/*
 * @routine 0A9D:269 WalkThenTurn
 * @run 71:C269
 * WHAT IT DOES: Walks by phase 2, takes its facing as its direction, runs TurnWhenWaitEnds, and when the player is $48 or more away along X resets its
 * animation and enters state 3.
 * HOW: WalkByPhase, TurnWhenWaitEnds, DistanceXToPlayer.
 * WAITS: no.
 * Entry: X object, Y the probe's column. Exit: the walk's, then the last compare or load.
 */
AttackWalk WalkThenTurn(uint8_t slot, uint8_t column)
{
    AttackWalk r;
    PlayerDistance distance;
    uint8_t facing;
    Ended e;

    jsr_push(0xC26E);
    r.walk = WalkByPhase(slot, 2, column);
    jsr_pop();
    facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->f3618[slot], facing);
    e = ended_load(facing);
    jsr_push(0xC277);
    e = then(e, TurnWhenWaitEnds(slot));
    jsr_pop();
    jsr_push(0xC27A);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_distance(distance));
    e = then(e, ended_compare(distance.distance, 0x48));
    if (distance.distance >= 0x48) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 3);
        e = then(e, ended_load(3));
    }
    r.rest = e;
    return r;
}

void abi_WalkThenTurn(void)
{
    attack_walk_exit_shim(WalkThenTurn(cpu.x, cpu.y));
}

/*
 * @routine 0A9D:453 FollowPartner
 * @run 71:C453
 * WHAT IT DOES: In state 0 loads the object's record; later, when it has taken a hit of 2 or more it plays sound $12 and sets its timer to 5; it takes the place of the
 * object before it, sets its hitbox from a four-byte row of the table at $C4AA by that object's frame less 1, copies its facing and hitbox flag, and is cleared
 * if the object before it is not kind $24.
 * HOW: LoadObjectRecord (a tail jump), SoundIfOnFirstScreen, CopyObjectPlaceBack, table reads (bank 71), ClearObject.
 * Untraced: the clear (C4A6).
 * Curated cases: tools/rondo/curated/follow_partner.txt (the traced arms with the partner of kind $24).
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
 */
FollowEnd FollowPartner(uint8_t slot, uint8_t y)
{
    FollowEnd r = { { 0 }, { 0 }, false, { 0 }, { 0 } };
    uint8_t state, nz, last, hit, before, frame, index, byte, kind, placed;
    bool carry;
    Sum diff;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        r.end = ended_of_record(last, nz, carry);
        return r;
    }
    hit = RD8(OBJ->hit_taken[slot]);
    e = ended_compare(hit, 2);
    if (hit >= 2) {
        e = then(e, ended_load(0x12));
        r.before = e;
        jsr_push(0xC467);
        r.sound = SoundIfOnFirstScreen(0x12, slot, y);
        jsr_pop();
        r.called = true;
        WR8(OBJ->timer[slot], 5);
        e = ended_kept();
    }
    before = (uint8_t)(slot - 1);
    jsr_push(0xC472);
    placed = CopyObjectPlaceBack(before, slot);
    jsr_pop();
    e = then(e, ended_with_y(ended_load(placed), before));
    frame = RD8_IDX((uint16_t)(COL(OBJ->frame) - 1), slot);
    diff = sub_bytes(frame, 1, true);
    index = (uint8_t)(diff.value << 2);
    e = then(e, ended_with_y(ended_with_carry(of_sum(diff), (diff.value & 0x40) != 0), index));
    byte = rd8((uint16_t)(0xC4AA + index));
    WR8(OBJ->hitbox_dx[slot], byte);
    byte = rd8((uint16_t)(0xC4AB + index));
    WR8(OBJ->hitbox_dy[slot], byte);
    byte = rd8((uint16_t)(0xC4AC + index));
    WR8(OBJ->hitbox_half_w[slot], byte);
    byte = rd8((uint16_t)(0xC4AD + index));
    WR8(OBJ->hitbox_half_h[slot], byte);
    byte = RD8_IDX((uint16_t)(COL(OBJ->facing_left) - 1), slot);
    WR8(OBJ->facing_left[slot], byte);
    byte = RD8_IDX((uint16_t)(COL(OBJ->hitbox_on) - 1), slot);
    WR8(OBJ->hitbox_on[slot], byte);
    kind = RD8_IDX((uint16_t)(COL(OBJ->kind) - 1), slot);
    e = then(e, ended_compare(kind, 0x24));
    if (kind != 0x24) {
        ClearObject(slot);
    }
    r.end = e;
    return r;
}

void abi_FollowPartner(void)
{
    FollowEnd r = FollowPartner(cpu.x, cpu.y);

    abi_ended(r.before);
    if (r.called) {
        sound_if_on_first_screen_exit_shim(r.sound);
    }
    abi_ended(r.end);
}

/*
 * @routine 0A9D:046 JumpByStateTableC
 * @run 71:C046
 * WHAT IT DOES: Goes to the code a table at $C058 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTableC(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xC058 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xC059 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTableC(void)
{
    Jump jump = JumpByStateTableC(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0A9C:7F2 FaceBreakAndRun
 * @run 70:BFF2
 * WHAT IT DOES: One frame of the cave object: its life timer goes up, it faces the player in some states (FaceThePlayerInSomeStatesD), may break (BreakWhenHit) and then the code its state names runs.
 * HOW: INC, two calls, a tail jump through the state table (JumpByStateTableC).
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to.
 */
Jump FaceBreakAndRun(uint8_t slot, Ended *before)
{
    SideEnd side;
    Ended e;
    Jump jump;

    INC8(OBJ->life_timer[slot]);
    jsr_push(0xBFF8);
    side = FaceThePlayerInSomeStatesD(slot);
    jsr_pop();
    e = side.side_used ? ended_of_side(side.side) : ended_kept();
    e = then(e, side.end);
    jsr_push(0xBFFB);
    e = then(e, BreakWhenHit(slot));
    jsr_pop();
    jump = JumpByStateTableC(slot);
    *before = e;
    return jump;
}

void abi_FaceBreakAndRun(void)
{
    Ended before;
    Jump jump = FaceBreakAndRun(cpu.x, &before);

    abi_ended(before);
    jump_shim(jump.end, jump.target);
}
