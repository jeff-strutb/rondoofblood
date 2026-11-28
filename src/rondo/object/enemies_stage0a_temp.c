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
/* The carry of a move along one axis, as the move's exit leaves it (what the next terrain probe is entered with). */
static bool axis_move_carry_shim(AxisMove move)
{
    return ended_of_axis_move(move).c;
}

/* Arms: one enum per routine that has more than one way out. */
enum { S0A_BW_HIGH_STATE, S0A_BW_UNFLAGGED, S0A_BW_BROKEN };
enum { S0A_SL_FRAME_4, S0A_SL_FLYING, S0A_SL_LANDED };
enum { S0A_SP_FAR, S0A_SP_SLOWED };
enum { S0A_CA_NEAR_Y, S0A_CA_FAR_X, S0A_CA_NEAR_X };
enum { S0A_TW_WAITING, S0A_TW_TURNED };
enum { S0A_AR_STEPPING, S0A_AR_ENDED };
enum { S0A_CN_ENTER_9, S0A_CN_WAIT, S0A_CN_PICKED };
enum { S0A_SH_HELD, S0A_SH_ENDED, S0A_SH_QUIET, S0A_SH_SOUND_26, S0A_SH_SOUND_2D };
enum { S0A_SA_NONE, S0A_SA_SHOT };
enum { S0A_FC_CLEARED, S0A_FC_NONE, S0A_FC_SHOT };
enum { S0A_HP_STATE_0, S0A_HP_HIGH, S0A_HP_RAN };
enum { S0A_HB_OFF, S0A_HB_SET };
enum { S0A_CD_COUNTING, S0A_CD_PICKED, S0A_CD_OFFSCREEN };
enum { S0A_BO_TURN_FIRST, S0A_BO_STAY_SECOND, S0A_BO_TURN_SECOND };
enum { S0A_WP_STEPPED, S0A_WP_STRAIGHT, S0A_WP_TURNED };
enum { S0A_AW_COUNTING, S0A_AW_FLIPPED };
enum { S0A_CT_NEAR, S0A_CT_FAR };
enum { S0A_SB_ENDED, S0A_SB_OTHER, S0A_SB_FRAME, S0A_SB_BOUNCE };
enum { S0A_PO_ENDED, S0A_PO_OTHER, S0A_PO_NOT_FRAMES, S0A_PO_BOUNCE };
enum { S0A_WT_NEAR, S0A_WT_FAR };
enum { S0A_FP_IDLE, S0A_FP_PLAIN, S0A_FP_HIT };

/* A wait's exit: what came before, the callee's exit when it ran, then the rest. */
void wait_exit_shim(WaitEnd r)
{
    abi_ended(r.before);
    if (r.picked) {
        abi_random_end(pick_pair_at_random_exit_shim(r.timers));
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
ObjArm BreakWhenHit(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t flags;

    r.first = RD8(OBJ->state[slot]);
    r.arm = S0A_BW_HIGH_STATE;
    if (r.first >= 0x0B) {
        return r;
    }
    flags = RD8(OBJ->hit_status[slot]);
    r.second = (uint8_t)(flags & 0x80);
    r.arm = S0A_BW_UNFLAGGED;
    if (r.second == 0) {
        return r;
    }
    jsr_push(0xC031);
    ClearObject((uint8_t)(slot + 1));
    jsr_pop();
    r.third = slot;
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f3778[slot], 0x40);
    WR8(OBJ->state[slot], 0x0B);
    r.arm = S0A_BW_BROKEN;
    return r;
}

/* The compare with $0B, then the flag byte's load, then (when broken) the slot's N and Z and the new state. */
static Ended break_when_hit_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 0x0B);

    if (r.arm == S0A_BW_HIGH_STATE) {
        return e;
    }
    e = then(e, ended_load(r.second));
    if (r.arm == S0A_BW_UNFLAGGED) {
        return e;
    }
    e = then(e, ended_nz(r.third));
    return then(e, ended_load(0x0B));
}

void abi_BreakWhenHit(void)
{
    abi_ended(break_when_hit_exit_shim(BreakWhenHit(cpu.x)));
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
ObjArm StartWithPartner(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xC073);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    WR8(OBJ->f3510[slot], 0xB0);
    WR8(OBJ->angle_hi[slot], 0x40);
    r.y = (uint8_t)(slot + 1);
    jsr_push(0xC083);
    ClearObjectInY(r.y);
    jsr_pop();
    r.third = StartObjectAtPlace(0x25, slot, r.y);
    return r;
}

/* The record's exit, then A the placed object's type with Y the partner's slot. */
static Ended start_with_partner_exit_shim(ObjArm r)
{
    return then(record_exit_shim(r), ended_with_y(ended_load(r.third), r.y));
}

void abi_StartWithPartner(void)
{
    abi_ended(start_with_partner_exit_shim(StartWithPartner(cpu.x)));
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
MoverArm StopWhenLanded(uint8_t slot, uint8_t column, bool carry_in)
{
    MoverArm w = { false, { 0 }, { 0 } };

    w.arm.first = RD8(OBJ->frame[0x54]);
    w.arm.arm = S0A_SL_FRAME_4;
    if (w.arm.first == 4) {
        return w;
    }
    jsr_push(0xC094);
    w.mover = StopOnLanding(4, slot, column, carry_in);
    jsr_pop();
    w.asked = true;
    w.arm.arm = S0A_SL_FLYING;
    if (!mover_landed(w.mover)) {
        return w;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    w.arm.second = INC8(OBJ->state[slot]);
    w.arm.arm = S0A_SL_LANDED;
    return w;
}

/* The frame's compare with 4, or nothing while it flies, or the new state's N and Z once it lands. */
static LandWait stop_when_landed_exit_shim(MoverArm w)
{
    LandWait l = { w.asked, w.mover, { 0 } };

    if (w.arm.arm == S0A_SL_FRAME_4) {
        l.end = ended_compare(w.arm.first, 4);
    } else if (w.arm.arm == S0A_SL_LANDED) {
        l.end = ended_nz(w.arm.second);
    } else {
        l.end = ended_kept();
    }
    return l;
}

void abi_StopWhenLanded(void)
{
    land_wait_exit_shim(stop_when_landed_exit_shim(StopWhenLanded(cpu.x, cpu.y, (cpu.p & P_C) != 0)));
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
ObjArm SlowToPlayer(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xC116);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.arm = S0A_SP_FAR;
    if (r.distance.distance >= 5) {
        return r;
    }
    WR8(OBJ->vx_hi[slot], r.distance.distance);
    WR8(OBJ->vx_lo[slot], 0);
    jsr_push(0xC123);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.first = (uint8_t)(r.side.sides & 0x01);
    r.second = (uint8_t)(r.first ^ 0x01);
    WR8(OBJ->f3618[slot], r.second);
    r.arm = S0A_SP_SLOWED;
    return r;
}

/* The distance and its compare with 5; when it was less, the side's exit and the two loads of the bit and its flip. */
static Ended slow_to_player_exit_shim(ObjArm r)
{
    Ended e = then(ended_of_distance(r.distance), ended_compare(r.distance.distance, 5));

    if (r.arm == S0A_SP_FAR) {
        return e;
    }
    e = then(e, ended_of_side(r.side));
    e = then(e, ended_load(r.first));
    return then(e, ended_load(r.second));
}

void abi_SlowToPlayer(void)
{
    abi_ended(slow_to_player_exit_shim(SlowToPlayer(cpu.x)));
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
ObjArm ChooseAttackByDistance(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xC136);
    r.distance = DistanceYToPlayer(slot);
    jsr_pop();
    if (r.distance.distance < 0x20) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->f3720[slot], 1);
        WR8(OBJ->state[slot], 4);
        r.arm = S0A_CA_NEAR_Y;
        return r;
    }
    jsr_push(0xC14E);
    r.distance2 = DistanceXToPlayer(slot);
    jsr_pop();
    r.arm = S0A_CA_FAR_X;
    if (r.distance2.distance >= 0x18) {
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f3720[slot], 2);
    WR8(OBJ->state[slot], 7);
    r.arm = S0A_CA_NEAR_X;
    return r;
}

/* The Y distance and its compare with $20; past it the X distance and its compare with $18; the state loaded when one was near. */
static Ended choose_attack_by_distance_exit_shim(ObjArm r)
{
    Ended e = then(ended_of_distance(r.distance), ended_compare(r.distance.distance, 0x20));

    if (r.arm == S0A_CA_NEAR_Y) {
        return then(e, ended_load(4));
    }
    e = then(e, ended_of_distance(r.distance2));
    e = then(e, ended_compare(r.distance2.distance, 0x18));
    if (r.arm == S0A_CA_FAR_X) {
        return e;
    }
    return then(e, ended_load(7));
}

void abi_ChooseAttackByDistance(void)
{
    abi_ended(choose_attack_by_distance_exit_shim(ChooseAttackByDistance(cpu.x)));
}

/*
 * @routine 0A9D:28A TurnWhenWaitEnds
 * @run 71:C28A
 * WHAT IT DOES: Counts the angle byte down; at zero resets the animation, takes OBJ_3670 from the player's side (bit 1, vertical) and enters state 8.
 * HOW: DEC, SideOfPlayer and a shift.
 * WAITS: no.
 * Entry: X object. Exit: wait running: N and Z from the count; at zero A $08, C bit 0 of the sides.
 */
ObjArm TurnWhenWaitEnds(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = DEC8(OBJ->angle_hi[slot]);
    r.arm = S0A_TW_WAITING;
    if (r.first != 0) {
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    jsr_push(0xC298);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.second = (uint8_t)(r.side.sides >> 1);
    r.flag = (r.side.sides & 0x01) != 0;
    WR8(OBJ->f3670[slot], r.second);
    WR8(OBJ->state[slot], 8);
    r.arm = S0A_TW_TURNED;
    return r;
}

/* The count's N and Z; once it is out, the side's exit, the side shifted right with its low bit in C, and the state 8. */
static Ended turn_when_wait_ends_exit_shim(ObjArm r)
{
    Ended e = ended_nz(r.first);

    if (r.arm == S0A_TW_WAITING) {
        return e;
    }
    e = then(e, ended_of_side(r.side));
    e = then(e, ended_with_carry(ended_load(r.second), r.flag));
    return then(e, ended_load(8));
}

void abi_TurnWhenWaitEnds(void)
{
    abi_ended(turn_when_wait_ends_exit_shim(TurnWhenWaitEnds(cpu.x)));
}

/*
 * @routine 0A9D:31E StepWithArmor
 * @run 71:C31E
 * WHAT IT DOES: Sets bits 0 and 1 of the object's OBJ_PACE while it steps the animation list at $C347; when the list ends it clears them, resets
 * the animation and enters state 5.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step; at the end A 5.
 */
ObjArm StepWithArmor(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t bits;

    bits = RD8(OBJ->pace[slot]);
    bits = (uint8_t)(bits | 0x03);
    WR8(OBJ->pace[slot], bits);
    r.step = step_list(slot, 0xC347, 0xC331, &y, true);
    r.y = y;
    r.arm = S0A_AR_STEPPING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    bits = RD8(OBJ->pace[slot]);
    r.first = (uint8_t)(bits & 0xFC);
    WR8(OBJ->pace[slot], r.first);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 5);
    r.arm = S0A_AR_ENDED;
    return r;
}

/* The step's exit; when the list ended, the cleared bits and the state 5 loaded after it. */
static Ended step_with_armor_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == S0A_AR_STEPPING) {
        return e;
    }
    e = then(e, ended_load(r.first));
    return then(e, ended_load(5));
}

void abi_StepWithArmor(void)
{
    abi_ended(step_with_armor_exit_shim(StepWithArmor(cpu.x, cpu.y)));
}

/*
 * @routine 0A9D:1CF ChooseNextMove
 * @run 71:C1CF
 * WHAT IT DOES: While the object in slot 0 is in OBJ_SUBSTATE state 1 and the player is $50 or more away along X and the distance sum is even, the
 * object resets its animation and enters state 9; otherwise its wait OBJ_3568 counts down and at zero it resets, picks a new wait from the table
 * at $C226 by the distance sum and its next state from the table at $C223 by a counter OBJ_37D0, which goes up one.
 * HOW: DistanceXToPlayer, PlayerDistanceSum (twice, with the carry the compares left), table reads (bank 71).
 * WAITS: no.
 * Entry: X object, C entry (it reaches PlayerDistanceSum when a compare did not set it). Exit: the last compare, AND, DEC or INC.
 */
ObjArm ChooseNextMove(uint8_t slot, bool carry_in)
{
    ObjArm r = { 0 };
    bool carry;

    (void)carry_in;   /* the compare of the first byte sets C before anything reads it */
    r.first = RD8(OBJ->substate[0]);
    carry = r.first >= 1;
    if (r.first == 1) {
        r.flag = true;
        jsr_push(0xC1D9);
        r.distance = DistanceXToPlayer(slot);
        jsr_pop();
        carry = r.distance.distance >= 0x50;
        if (r.distance.distance >= 0x50) {
            r.other = true;
            jsr_push(0xC1E0);
            r.sum = PlayerDistanceSum(slot, carry);
            jsr_pop();
            carry = r.sum.carry;
            r.second = (uint8_t)(r.sum.value & 0x01);
            if (r.second == 0) {
                WR8(OBJ->anim_step[slot], 0);
                WR8(OBJ->anim_timer[slot], 0);
                WR8(OBJ->state[slot], 9);
                r.arm = S0A_CN_ENTER_9;
                return r;
            }
        }
    }
    r.third = DEC8(OBJ->angle_hi[slot]);
    r.arm = S0A_CN_WAIT;
    if (r.third != 0) {
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    jsr_push(0xC1FE);
    r.sum2 = PlayerDistanceSum(slot, carry);
    jsr_pop();
    r.fourth = (uint8_t)(r.sum2.value & 0x03);
    r.fifth = rd8((uint16_t)(0xC226 + r.fourth));
    WR8(OBJ->angle_hi[slot], r.fifth);
    r.sixth = RD8(OBJ->f37d0[slot]);
    r.sixth = (uint8_t)(r.sixth & 0x07);
    r.seventh = rd8((uint16_t)(0xC21B + r.sixth));
    r.eighth = rd8((uint16_t)(0xC223 + r.seventh));
    WR8(OBJ->state[slot], r.eighth);
    r.ninth = INC8(OBJ->f37d0[slot]);
    r.arm = S0A_CN_PICKED;
    return r;
}

/* The compare of the lead byte with 1 and, when it was 1, the X distance, its compare with $50 and (when far) the sum's exit and
   the bit's load; then the count's N and Z; when it ran out, the second sum and the table loads in order. */
static Ended choose_next_move_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 1);

    if (r.flag) {
        e = then(e, ended_of_distance(r.distance));
        e = then(e, ended_compare(r.distance.distance, 0x50));
        if (r.other) {
            e = then(e, of_sum(r.sum));
            e = then(e, ended_load(r.second));
            if (r.arm == S0A_CN_ENTER_9) {
                return then(e, ended_load(9));
            }
        }
    }
    e = then(e, ended_nz(r.third));
    if (r.arm == S0A_CN_WAIT) {
        return e;
    }
    e = then(e, of_sum(r.sum2));
    e = then(e, ended_with_y(ended_load(r.fourth), r.fourth));
    e = then(e, ended_load(r.fifth));
    e = then(e, ended_with_y(ended_load(r.sixth), r.sixth));
    e = then(e, ended_with_y(ended_load(r.seventh), r.seventh));
    e = then(e, ended_load(r.eighth));
    return then(e, ended_nz(r.ninth));
}

void abi_ChooseNextMove(void)
{
    abi_ended(choose_next_move_exit_shim(ChooseNextMove(cpu.x, (cpu.p & P_C) != 0)));
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
ObjArm StepAndShout(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t phase, y, picked, wait;
    Sum sum;

    phase = RD8(OBJ->f3670[slot]);
    r.second = (uint8_t)(phase << 1);
    y = r.second;
    WR8(ZP->temp10, rd8((uint16_t)(0xC2E4 + r.second)));
    r.first = rd8((uint16_t)(0xC2E5 + r.second));
    WR8(ZP->temp11, r.first);
    r.flag = (phase & 0x80) != 0;
    jsr_push(0xC2B4);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = S0A_SH_HELD;
    if (r.step == FRAME_LIST_ENDED) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 7);
        jsr_push(0xC2D6);
        r.sum = PlayerDistanceSum(slot, false);
        jsr_pop();
        sum = r.sum;
        picked = (uint8_t)(sum.value & 0x03);
        wait = rd8((uint16_t)(0xC2E0 + picked));
        WR8(OBJ->angle_hi[slot], wait);
        r.third = picked;
        r.fourth = wait;
        r.arm = S0A_SH_ENDED;
        return r;
    }
    if (r.step != FRAME_CHANGED) {
        return r;
    }
    r.fifth = RD8(OBJ->frame[slot]);
    r.arm = S0A_SH_SOUND_26;
    if (r.fifth != 0x26) {
        r.arm = S0A_SH_SOUND_2D;
        if (r.fifth != 0x2D) {
            r.arm = S0A_SH_QUIET;
            return r;
        }
    }
    r.screen_called = true;
    r.screen = SoundIfOnFirstScreen(0x1F, slot, y);
    return r;
}

/* The list address's load with Y and C, then the step's exit; the list-ended path adds the state, the sum and the wait's load;
   the frame path adds its compares and the call's A. */
static ScreenSoundEnd step_and_shout_exit_shim(ObjArm r)
{
    ScreenSoundEnd s = { { 0 }, { 0 }, false, { 0 } };
    Ended e = ended_with_carry(ended_with_y(ended_load(r.first), r.second), r.flag);

    e = then(e, ended_of_frame_step(r.step, true, r.y));
    s.end = e;
    if (r.arm == S0A_SH_HELD) {
        return s;
    }
    if (r.arm == S0A_SH_ENDED) {
        e = then(e, ended_load(7));
        e = then(e, of_sum(r.sum));
        s.end = then(e, ended_with_y(ended_load(r.fourth), r.third));
        return s;
    }
    e = then(e, ended_compare(r.fifth, 0x26));
    if (r.arm != S0A_SH_SOUND_26) {
        e = then(e, ended_compare(r.fifth, 0x2D));
        if (r.arm == S0A_SH_QUIET) {
            s.end = e;
            return s;
        }
    }
    s.before = then(e, ended_load(0x1F));
    s.end = ended_kept();
    s.called = r.screen_called;
    s.sound = r.screen;
    return s;
}

void abi_StepAndShout(void)
{
    abi_screen_sound_end(step_and_shout_exit_shim(StepAndShout(cpu.x)));
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
ObjArm ShootArrow(uint8_t slot, uint8_t cue)
{
    ObjArm r = { 0 };
    uint8_t saved, index;

    WR8(ZP->richter_object, slot);
    WR8(ZP->hit_slot, cue);
    jsr_push(0xC417);
    r.y = FindFreeSlotFrom30(&r.first, &r.flag);
    jsr_pop();
    r.arm = S0A_SA_NONE;
    if (!r.flag) {
        return r;
    }
    jsr_push(0xC41E);
    r.third = StartObjectAtPlace(0x9C, slot, r.y);
    jsr_pop();
    jsr_push(0xC423);
    r.screen = SoundIfOnFirstScreen(0x15, slot, r.y);
    jsr_pop();
    r.screen_called = true;
    saved = RD8(ZP->hit_slot);
    index = (uint8_t)(saved << 1);
    r.fourth = rd8((uint16_t)(0xC443 + index));
    WR8(OBJ->vx_hi[r.y], r.fourth);
    r.fourth = rd8((uint16_t)(0xC444 + index));
    WR8(OBJ->vy_hi[r.y], r.fourth);
    r.other = (saved & 0x80) != 0;
    jsr_push(0xC437);
    r.move = ApplyVelocity(r.y);
    jsr_pop();
    r.fifth = RD8(ZP->richter_object);
    r.arm = S0A_SA_SHOT;
    return r;
}

/* The slot search's exit; when found, the placed type and the cue's A, the call, the velocity byte's load with C from the saved
   byte, the move's exit with Y and the slot restored's N and Z. */
static ShotEnd shoot_arrow_exit_shim(ObjArm r)
{
    ShotEnd s = { { 0 }, { 0 }, false, { 0 }, { 0 } };
    Ended e = ended_of_find_slot(r.y, r.first, r.flag);

    if (r.arm == S0A_SA_NONE) {
        s.end = e;
        return s;
    }
    e = then(e, ended_load(r.third));
    s.before = then(e, ended_load(0x15));
    s.sound = r.screen;
    s.called = true;
    s.mid = ended_with_carry(ended_with_y(ended_with_a(ended_nz(r.fourth), r.fourth), r.y), r.other);
    s.end = then(ended_with_y(ended_of_axis_move(r.move), r.y), ended_nz(r.fifth));
    return s;
}

static void shot_end_abi(ShotEnd r)
{
    abi_ended(r.before);
    if (r.called) {
        sound_if_on_first_screen_exit_shim(r.sound);
        abi_ended(r.mid);
    }
    abi_ended(r.end);
}

void abi_ShootArrow(void)
{
    shot_end_abi(shoot_arrow_exit_shim(ShootArrow(cpu.x, cpu.y)));
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
ObjArm FlashAndCount(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t index;

    WR8(OBJ->sprite_attr[slot], 0x8A);
    r.step = step_list(slot, 0xC401, 0xC3EA, &y, false);
    r.y = y;
    r.first = DEC8(OBJ->f3778[slot]);
    if (r.first == 0) {
        ClearObject(slot);
        r.arm = S0A_FC_CLEARED;
        return r;
    }
    index = 0;
    do {
        r.second = RD8(OBJ->f3778[slot]);
        r.third = rd8((uint16_t)(0xC43B + index));
        if (r.second == r.third) {
            r.fourth = index;
            r.sub = obj_sub(ShootArrow(slot, index));
            r.arm = S0A_FC_SHOT;
            return r;
        }
        index++;
    } while (index < 8);
    r.fourth = index;
    r.arm = S0A_FC_NONE;
    return r;
}

/* The step's exit and the count's N and Z; then the cue scan's last pass: Y the pass's index, the compare of the count with the
   cue (and, after a pass with no shot, the index's N and Z with Y and its compare with 8); a shot's own exit follows. */
static ShotEnd flash_and_count_exit_shim(ObjArm r)
{
    ShotEnd s = { { 0 }, { 0 }, false, { 0 }, { 0 } };
    Ended e = then(frame_step_exit_shim(r), ended_nz(r.first));

    if (r.arm == S0A_FC_CLEARED) {
        s.end = e;
        return s;
    }
    e = then(e, ended_with_y(ended_kept(), 0));
    if (r.arm == S0A_FC_SHOT) {
        if (r.fourth != 0) {
            e = then(e, ended_with_y(ended_nz(r.fourth), r.fourth));
            e = then(e, cpy(r.fourth, 8));
        }
        e = then(e, ended_compare(r.second, r.third));
        s = shoot_arrow_exit_shim(obj_arm(r.sub));
        s.before = then(e, s.before);
        return s;
    }
    e = then(e, ended_compare(r.second, r.third));
    e = then(e, ended_with_y(ended_nz(r.fourth), r.fourth));
    s.end = then(e, cpy(r.fourth, 8));
    return s;
}

void abi_FlashAndCount(void)
{
    shot_end_abi(flash_and_count_exit_shim(FlashAndCount(cpu.x, cpu.y)));
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
L7HopArm HopAndSettle(uint8_t slot, uint8_t column, bool carry_in)
{
    L7HopArm r = { { 0 }, { 0 } };
    uint8_t kind, lift, low, high;
    Sum sum;
    AxisMove move;
    GravityResult gravity;
    bool carry;

    (void)carry_in;   /* the gravity's exit sets C before the probe reads it */
    r.arm.first = RD8(OBJ->state[slot]);
    r.arm.arm = S0A_HP_STATE_0;
    if (r.arm.first == 0) {
        return r;
    }
    r.arm.second = RD8(OBJ->y_hi[slot]);
    r.arm.arm = S0A_HP_HIGH;
    if (r.arm.second != 0) {
        return r;
    }
    kind = RD8(OBJ->start_arg[slot]);
    lift = (uint8_t)(kind << 4);
    WR8(ZP->temp00, lift);
    low = RD8(OBJ->y_lo[slot]);
    lift = RD8(ZP->temp00);
    sum = add_bytes(low, lift, false);
    WR8(OBJ->y_lo[slot], sum.value);
    high = RD8(OBJ->y_hi[slot]);
    sum = add_bytes(high, 0, sum.carry);
    WR8(OBJ->y_hi[slot], sum.value);
    jsr_push(0xCCEA);
    move = ApplyVelocityY(slot);
    jsr_pop();
    jsr_push(0xCCED);
    gravity = ApplyGravity(slot);
    jsr_pop();
    carry = gravity.gained ? gravity.carried : true;
    WR8(ZP->product.mid, 0x0A);
    jsr_push(0xCCF6);
    r.probe = ProbeTerrain(9, slot, column, carry);
    jsr_pop();
    r.arm.third = (uint8_t)(r.probe.answer & 0x01);
    if (r.arm.third != 0) {
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0);
    }
    kind = RD8(OBJ->start_arg[slot]);
    lift = (uint8_t)(kind << 4);
    WR8(ZP->temp00, lift);
    low = RD8(OBJ->y_lo[slot]);
    lift = RD8(ZP->temp00);
    sum = sub_bytes(low, lift, true);
    WR8(OBJ->y_lo[slot], sum.value);
    high = RD8(OBJ->y_hi[slot]);
    r.arm.sum = sub_bytes(high, 0, sum.carry);
    WR8(OBJ->y_hi[slot], r.arm.sum.value);
    (void)move;
    r.arm.arm = S0A_HP_RAN;
    return r;
}

/* The state's load and, past it, the high byte's; after the probe, the hit bit's load and the high byte's subtraction. */
static HopEnd hop_and_settle_exit_shim(L7HopArm r)
{
    HopEnd h = { { 0 }, false, { 0 }, { 0 } };

    h.before = ended_load(r.arm.first);
    if (r.arm.arm == S0A_HP_STATE_0) {
        return h;
    }
    h.before = then(h.before, ended_load(r.arm.second));
    if (r.arm.arm == S0A_HP_HIGH) {
        return h;
    }
    h.probed = true;
    h.probe = r.probe;
    h.after = then(ended_load(r.arm.third), of_sum(r.arm.sum));
    return h;
}

void abi_HopAndSettle(void)
{
    HopEnd r = hop_and_settle_exit_shim(HopAndSettle(cpu.x, cpu.y, (cpu.p & P_C) != 0));

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
ObjArm SetHitboxByFrame(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(OBJ->hitbox_on[slot], 2);
    r.first = RD8(OBJ->frame[slot]);
    r.arm = S0A_HB_OFF;
    if (r.first >= 0x86) {
        WR8(OBJ->hitbox_on[slot], 0);
        WR8(OBJ->hitbox_dx[slot], 0);
        return r;
    }
    r.sum = sub_bytes(r.first, 0x80, true);
    r.second = rd8((uint16_t)(0xCD39 + r.sum.value));
    WR8(OBJ->hitbox_dx[slot], r.second);
    r.arm = S0A_HB_SET;
    return r;
}

/* The frame's compare with $86, or the index's subtraction and the byte's load with Y the index. */
static Ended set_hitbox_by_frame_exit_shim(ObjArm r)
{
    if (r.arm == S0A_HB_OFF) {
        return ended_compare(r.first, 0x86);
    }
    return then(of_sum(r.sum), ended_with_y(ended_load(r.second), r.sum.value));
}

void abi_SetHitboxByFrame(void)
{
    abi_ended(set_hitbox_by_frame_exit_shim(SetHitboxByFrame(cpu.x)));
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
ObjArm CountDownThenAdvance(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t high, low;
    Sum sum;

    high = RD8(OBJ->life_timer[slot]);
    low = RD8(OBJ->f3510[slot]);
    r.first = (uint8_t)(high | low);
    if (r.first == 0) {
        jsr_push(0xCE00);
        r.sub = obj_sub(PickTimersAtRandomB(slot));
        jsr_pop();
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 2);
        r.arm = S0A_CD_PICKED;
        return r;
    }
    low = RD8(OBJ->f3510[slot]);
    sum = sub_bytes(low, 1, true);
    WR8(OBJ->f3510[slot], sum.value);
    high = RD8(OBJ->life_timer[slot]);
    r.sum = sub_bytes(high, 0, sum.carry);
    WR8(OBJ->life_timer[slot], r.sum.value);
    r.arm = S0A_CD_COUNTING;
    return r;
}

/* The merged timer's load, then either the picker (its exit applied between) and the state's load, or the count's subtraction. */
static WaitEnd count_down_then_advance_exit_shim(ObjArm r)
{
    WaitEnd w = { { 0 }, false, { 0 }, { 0 } };

    w.before = ended_load(r.first);
    if (r.arm == S0A_CD_PICKED) {
        w.picked = true;
        w.timers = obj_arm(r.sub);
        w.after = ended_load(2);
        return w;
    }
    w.after = of_sum(r.sum);
    return w;
}

void abi_CountDownThenAdvance(void)
{
    wait_exit_shim(count_down_then_advance_exit_shim(CountDownThenAdvance(cpu.x)));
}

/*
 * @routine 0A9E:61E CountDownAngleThenAdvance
 * @run 71:CE1E
 * WHAT IT DOES: With the object's X high byte 0, a 16-bit wait (OBJ_3568 high, OBJ_35C0 low) counts down; at zero it picks a new angle
 * (PickAngleAtRandomB), stops the animation, clears OBJ_SUBSTATE and enters state 3.
 * HOW: An ORA of the two bytes, a 16-bit subtract.
 * Untraced: everything past the X high byte test.
 * WAITS: no.
 * Entry: X object. Exit: see the wrapper.
 */
ObjArm CountDownAngleThenAdvance(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t high, low;
    Sum sum;

    r.first = RD8(OBJ->x_hi[slot]);
    r.arm = S0A_CD_OFFSCREEN;
    if (r.first != 0) {
        return r;
    }
    high = RD8(OBJ->angle_hi[slot]);
    low = RD8(OBJ->angle_lo[slot]);
    r.second = (uint8_t)(high | low);
    if (r.second == 0) {
        jsr_push(0xCE2E);
        r.sub = obj_sub(PickAngleAtRandomB(slot));
        jsr_pop();
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->substate[slot], 0);
        WR8(OBJ->state[slot], 3);
        r.arm = S0A_CD_PICKED;
        return r;
    }
    low = RD8(OBJ->angle_lo[slot]);
    sum = sub_bytes(low, 1, true);
    WR8(OBJ->angle_lo[slot], sum.value);
    high = RD8(OBJ->angle_hi[slot]);
    r.sum = sub_bytes(high, 0, sum.carry);
    WR8(OBJ->angle_hi[slot], r.sum.value);
    r.arm = S0A_CD_COUNTING;
    return r;
}

/* The X high byte's load, or past it the merged angle's; then the picker and the state's load, or the count's subtraction. */
static WaitEnd count_down_angle_then_advance_exit_shim(ObjArm r)
{
    WaitEnd w = { { 0 }, false, { 0 }, { 0 } };

    if (r.arm == S0A_CD_OFFSCREEN) {
        w.before = ended_load(r.first);
        return w;
    }
    w.before = ended_load(r.second);
    if (r.arm == S0A_CD_PICKED) {
        w.picked = true;
        w.timers = obj_arm(r.sub);
        w.after = ended_load(3);
        return w;
    }
    w.after = of_sum(r.sum);
    return w;
}

void abi_CountDownAngleThenAdvance(void)
{
    wait_exit_shim(count_down_angle_then_advance_exit_shim(CountDownAngleThenAdvance(cpu.x)));
}

/*
 * @routine 0A9E:5EE CountBothWaits
 * @run 71:CDEE
 * WHAT IT DOES: Runs both waits, CountDownThenAdvance and CountDownAngleThenAdvance.
 * HOW: Two calls.
 * WAITS: no.
 * Entry: X object. Exit: as the second.
 */
WaitArms CountBothWaits(uint8_t slot)
{
    WaitArms pair;

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
    WaitArms pair = CountBothWaits(cpu.x);

    wait_exit_shim(count_down_then_advance_exit_shim(pair.first));
    wait_exit_shim(count_down_angle_then_advance_exit_shim(pair.second));
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
ObjArm StartKnifeThrower(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t kind, count, to, number, shared, remaining;
    Sum sum;

    jsr_push(0xCD42);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    r.third = RD8(OBJ->start_arg[slot]);
    if ((r.third & 0x80) == 0) {
        r.other = true;
        WR8(OBJ->f37d0[slot], slot);
        kind = RD8(OBJ->start_arg[slot]);
        r.sum = add_bytes(slot, kind, false);
        WR8(OBJ->f3828[slot], r.sum.value);
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
            WR8(OBJ->start_arg[to], number);
            WR8(OBJ->f37d0[to], slot);
            shared = RD8(OBJ->f3828[slot]);
            WR8(OBJ->f3828[to], shared);
            INC8(ZP->temp00);
            to++;
            remaining = DEC8(OBJ->start_arg[slot]);
        } while (remaining != 0);
        r.fourth = remaining;
        r.y = to;
    }
    count = RD8(OBJ->start_arg[slot]);
    count = (uint8_t)(count & 0x7F);
    WR8(OBJ->start_arg[slot], count);
    r.fifth = (uint8_t)((count & 0x01) ^ 0x01);
    WR8(OBJ->facing_left[slot], r.fifth);
    jsr_push(0xCD8A);
    r.sub = obj_sub(ThrowByNextKindB(slot));
    jsr_pop();
    jsr_push(0xCD8D);
    r.sub2 = obj_sub(PickTimersAtRandomB(slot));
    jsr_pop();
    jsr_push(0xCD90);
    r.sub3 = obj_sub(PickAngleAtRandomB(slot));
    jsr_pop();
    return r;
}

/* The record's exit and the kind's load, the offsets' sum and the slot count's N and Z with Y when pieces were started, the facing bit's
   load; then the three callees' exits in order. */
static void start_knife_thrower_exit_shim(ObjArm r)
{
    Ended e = then(record_exit_shim(r), ended_load(r.third));

    if (r.other) {
        e = then(e, of_sum(r.sum));
        e = then(e, ended_with_y(ended_nz(r.fourth), r.y));
    }
    abi_ended(then(e, ended_load(r.fifth)));
    abi_ended(throw_by_next_kind_exit_shim(obj_arm(r.sub)));
    abi_random_end(pick_pair_at_random_exit_shim(obj_arm(r.sub2)));
    abi_random_end(pick_pair_at_random_exit_shim(obj_arm(r.sub3)));
}

void abi_StartKnifeThrower(void)
{
    start_knife_thrower_exit_shim(StartKnifeThrower(cpu.x));
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
BounceArm BounceAtWalls(uint8_t slot, uint8_t column)
{
    BounceArm r = { { 0 }, { 0 }, { 0 } };
    uint8_t side, flags, flags2;

    jsr_push(0xAB3D);
    r.arm.move = ApplyVelocityX(slot);
    jsr_pop();
    jsr_push(0xAB42);
    r.probe1 = ProbeTerrain(9, slot, column, axis_move_carry_shim(r.arm.move));
    jsr_pop();
    r.arm.first = RD8(OBJ->vx_hi[slot]);
    side = (uint8_t)((r.arm.first >> 7) ^ 0x01);
    flags = rd8((uint16_t)(TileHit + 1 + side));
    r.arm.second = (uint8_t)(flags & 0x01);
    r.arm.arm = S0A_BO_TURN_FIRST;
    if (r.arm.second != 0) {
        jsr_push(0xAB58);
        r.probe2 = ProbeTerrain(2, slot, side, (r.arm.first & 0x40) != 0);
        jsr_pop();
        r.arm.third = RD8(OBJ->vx_hi[slot]);
        side = (uint8_t)(((r.arm.third >> 7) ^ 0x01) << 1);
        flags = rd8((uint16_t)(TileHit + side));
        flags2 = rd8((uint16_t)(TileHit + 1 + side));
        r.arm.fourth = (uint8_t)((flags | flags2) & 0x02);
        r.arm.arm = S0A_BO_STAY_SECOND;
        if (r.arm.fourth == 0) {
            return r;
        }
        r.arm.arm = S0A_BO_TURN_SECOND;
    }
    jsr_push(0xAB72);
    NegateVelocityX(slot);
    jsr_pop();
    jsr_push(0xAB75);
    r.arm.move2 = ApplyVelocityX(slot);
    jsr_pop();
    return r;
}

/* The first move's exit, the probe, the side's load with Y and C from the speed's bit 6 and the wall flag's load; when that was set,
   the second probe and the second side's load and flags (C 0 when it stays); when it turns, the second move's exit with C 1. */
static void bounce_exit_shim(BounceArm r)
{
    uint8_t side = (uint8_t)((r.arm.first >> 7) ^ 0x01);
    Ended e;

    abi_ended(ended_of_axis_move(r.arm.move));
    probe_exit_shim(r.probe1);
    e = ended_with_carry(ended_with_y(ended_load(side), side), (r.arm.first & 0x40) != 0);
    abi_ended(then(e, ended_load(r.arm.second)));
    if (r.arm.arm != S0A_BO_TURN_FIRST) {
        probe_exit_shim(r.probe2);
        side = (uint8_t)(((r.arm.third >> 7) ^ 0x01) << 1);
        e = ended_with_carry(ended_with_y(ended_load(side), side), false);
        e = then(e, ended_load(r.arm.fourth));
        if (r.arm.arm == S0A_BO_STAY_SECOND) {
            e = then(e, ended_with_carry(ended_kept(), false));
        }
        abi_ended(e);
    }
    if (r.arm.arm != S0A_BO_STAY_SECOND) {
        abi_ended(then(ended_of_axis_move(r.arm.move2), ended_with_carry(ended_kept(), true)));
    }
}

/* BounceAtWalls's exit, in the order its instructions leave it. */
/* The carry BounceAtWalls leaves: set when it turned round. */

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
BounceArm BounceWithHeight4(uint8_t slot, uint8_t column)
{
    BounceArm r;

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
WalkArm WalkByPhase(uint8_t slot, uint8_t phase, uint8_t column)
{
    WalkArm r = { { 0 }, { { 0 }, { 0 }, { 0 } } };
    uint8_t index, byte, y;

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
    r.arm.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.arm.y = y;
    r.arm.arm = S0A_WP_STEPPED;
    if (r.arm.step != FRAME_CHANGED) {
        return r;
    }
    jsr_push(0xC0E1);
    r.arm.sub = obj_sub(SlowToPlayer(slot));
    jsr_pop();
    r.arm.first = RD8(OBJ->f3618[slot]);
    r.arm.arm = S0A_WP_STRAIGHT;
    if (r.arm.first != 0) {
        jsr_push(0xC0E9);
        NegateVelocityX(slot);
        jsr_pop();
        r.arm.arm = S0A_WP_TURNED;
    }
    jsr_push(0xC0EC);
    r.turn = BounceWithHeight4(slot, column);
    jsr_pop();
    return r;
}

/* The step's exit with Y and C; once it changed frames, SlowToPlayer's exit and the direction's load (C 0 when it was negated),
   then the bounce's registers. */
static void walk_phase_exit_shim(WalkArm r)
{
    Ended e = ended_of_frame_step(r.arm.step, true, r.arm.y);

    if (r.arm.arm != S0A_WP_STEPPED) {
        e = then(e, slow_to_player_exit_shim(obj_arm(r.arm.sub)));
        e = then(e, ended_load(r.arm.first));
        if (r.arm.arm == S0A_WP_TURNED) {
            e = then(e, ended_with_carry(ended_kept(), false));
        }
    }
    abi_ended(e);
    if (r.arm.arm != S0A_WP_STEPPED) {
        bounce_exit_shim(r.turn);
    }
}

/* A walk's exit: what came before, then BounceAtWalls's. */
void abi_WalkByPhase(void)
{
    walk_phase_exit_shim(WalkByPhase(cpu.x, cpu.a, cpu.y));
}

/* The two attack walkers share their tail: choose an attack, copy the direction to the facing, count down and flip the direction at zero. */
static AttackArm attack_walk(uint8_t slot, uint8_t phase, uint8_t column, uint16_t ret_walk, uint16_t ret_choose)
{
    AttackArm r = { { { 0 }, { { 0 }, { 0 }, { 0 } } }, { 0 } };

    jsr_push(ret_walk);
    r.walk = WalkByPhase(slot, phase, column);
    jsr_pop();
    jsr_push(ret_choose);
    r.rest.sub = obj_sub(ChooseAttackByDistance(slot));
    jsr_pop();
    r.rest.first = RD8(OBJ->f3618[slot]);
    WR8(OBJ->facing_left[slot], r.rest.first);
    r.rest.second = DEC8(OBJ->f3510[slot]);
    r.rest.arm = S0A_AW_COUNTING;
    if (r.rest.second == 0) {
        WR8(OBJ->f3510[slot], 0xB0);
        r.rest.third = RD8(OBJ->f3618[slot]);
        r.rest.third = (uint8_t)(r.rest.third ^ 0x01);
        WR8(OBJ->f3618[slot], r.rest.third);
        r.rest.arm = S0A_AW_FLIPPED;
    }
    return r;
}

/* The walk's registers, then what choosing the attack, the direction's load, the count and (when flipped) the new direction left. */
static void attack_walk_exit_shim(AttackArm r)
{
    Ended e;

    walk_phase_exit_shim(r.walk);
    e = then(choose_attack_by_distance_exit_shim(obj_arm(r.rest.sub)), ended_load(r.rest.first));
    e = then(e, ended_nz(r.rest.second));
    if (r.rest.arm == S0A_AW_FLIPPED) {
        e = then(e, ended_load(r.rest.third));
    }
    abi_ended(e);
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
AttackArm WalkAndAttackA(uint8_t slot, uint8_t column)
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
AttackArm WalkAndAttackB(uint8_t slot, uint8_t column)
{
    return attack_walk(slot, 3, column, 0xC168, 0xC16B);
}

void abi_WalkAndAttackB(void)
{
    attack_walk_exit_shim(WalkAndAttackB(cpu.x, cpu.y));
}

/* The part of the chasers the second one branches into: the object is sent to state 3 when the player is $40 or more away vertically. */

/* The chasers' exit: the distance check, the walk, the move choice, then the tail. */

/* Both chasers: set a state when the player is near, walk, choose the next move, then the shared tail. */
static ObjArm chase_tail(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xC1A1);
    r.distance = DistanceYToPlayer(slot);
    jsr_pop();
    r.arm = S0A_CT_NEAR;
    if (r.distance.distance < 0x40) {
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 3);
    r.arm = S0A_CT_FAR;
    return r;
}

/* The four exits of the chase in order: the distance and what it did to the state, the walk, the facing and the move choice, the tail. */
static void chase_exit_shim(L7ChaseArm r)
{
    Ended e = then(ended_of_distance(r.head.distance), ended_compare(r.head.distance.distance, r.head.first));

    if (r.head.flag) {
        e = then(e, ended_load(r.head.second));
    }
    abi_ended(e);
    walk_phase_exit_shim(r.walk);
    abi_ended(then(ended_load(r.mid.first), choose_next_move_exit_shim(obj_arm(r.mid.sub))));
    e = then(ended_of_distance(r.tail.distance), ended_compare(r.tail.distance.distance, 0x40));
    if (r.tail.arm == S0A_CT_FAR) {
        e = then(e, ended_load(3));
    }
    abi_ended(e);
}

static L7ChaseArm chase(uint8_t slot, uint8_t column, bool carry_in, bool far_when_ge, uint8_t limit, uint8_t near_state, bool invert,
                      uint16_t ret_distance, uint16_t ret_walk, uint16_t ret_move)
{
    L7ChaseArm r = { { 0 }, { { 0 }, { { 0 }, { 0 }, { 0 } } }, { 0 }, { 0 } };
    uint8_t facing;

    jsr_push(ret_distance);
    r.head.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.head.first = limit;
    r.head.second = near_state;
    if (far_when_ge ? r.head.distance.distance >= limit : r.head.distance.distance < limit) {
        WR8(OBJ->state[slot], near_state);
        r.head.flag = true;
    }
    jsr_push(ret_walk);
    r.walk = WalkByPhase(slot, 1, column);
    jsr_pop();
    facing = RD8(OBJ->facing_left[slot]);
    if (invert) {
        facing = (uint8_t)(facing ^ 0x01);
    }
    WR8(OBJ->f3618[slot], facing);
    r.mid.first = facing;
    jsr_push(ret_move);
    r.mid.sub = obj_sub(ChooseNextMove(slot, carry_in));
    jsr_pop();
    r.tail = chase_tail(slot);
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
L7ChaseArm ChasePlayerA(uint8_t slot, uint8_t column, bool carry_in)
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
L7ChaseArm ChasePlayerB(uint8_t slot, uint8_t column, bool carry_in)
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
StepBounceArm StepAndLunge(uint8_t slot, uint8_t y)
{
    StepBounceArm r = { { 0 }, { { 0 }, { 0 }, { 0 } } };
    uint8_t facing, byte;

    r.arm.step = step_list(slot, 0xC260, 0xC235, &y, true);
    r.arm.y = y;
    if (r.arm.step == FRAME_LIST_ENDED) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 4);
        r.arm.arm = S0A_SB_ENDED;
        return r;
    }
    r.arm.arm = S0A_SB_OTHER;
    if (r.arm.step != FRAME_CHANGED) {
        return r;
    }
    r.arm.first = RD8(OBJ->frame[slot]);
    r.arm.arm = S0A_SB_FRAME;
    if (r.arm.first != 7) {
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
    r.arm.arm = S0A_SB_BOUNCE;
    return r;
}

/* The step's exit; the state's load when the list ended; the frame's compare with 7; the bounce's registers after it. */
static void step_and_lunge_exit_shim(StepBounceArm r)
{
    Ended e = frame_step_exit_shim(r.arm);

    if (r.arm.arm == S0A_SB_ENDED) {
        e = then(e, ended_load(4));
    } else if (r.arm.arm >= S0A_SB_FRAME) {
        e = then(e, ended_compare(r.arm.first, 7));
    }
    abi_ended(e);
    if (r.arm.arm == S0A_SB_BOUNCE) {
        bounce_exit_shim(r.turn);
    }
}

/* A stepping routine that may end in the bounce: what came before, then the bounce's exit. */

void abi_StepAndLunge(void)
{
    step_and_lunge_exit_shim(StepAndLunge(cpu.x, cpu.y));
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
StepBounceArm StepAndPounce(uint8_t slot, uint8_t y)
{
    StepBounceArm r = { { 0 }, { { 0 }, { 0 }, { 0 } } };
    uint8_t index, facing, byte;
    Sum diff;

    WR8_IDX((uint16_t)(COL(OBJ->f3358) + 1), slot, 3);
    r.arm.step = step_list(slot, 0xC3C5, 0xC376, &y, true);
    r.arm.y = y;
    if (r.arm.step == FRAME_LIST_ENDED) {
        WR8_IDX((uint16_t)(COL(OBJ->f3358) + 1), slot, 2);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 4);
        r.arm.arm = S0A_PO_ENDED;
        return r;
    }
    r.arm.arm = S0A_PO_OTHER;
    if (r.arm.step != FRAME_CHANGED) {
        return r;
    }
    r.arm.first = RD8(OBJ->frame[slot]);
    if (r.arm.first == 0x0E) {
        jsr_push(0xC386);
        SoundIfOnFirstScreen(0x1A, slot, y);
        jsr_pop();
        r.arm.flag = true;
    }
    r.arm.second = RD8(OBJ->frame[slot]);
    diff = sub_bytes(r.arm.second, 0x0C, true);
    r.arm.sum = diff;
    r.arm.arm = S0A_PO_NOT_FRAMES;
    if (diff.value >= 2) {
        return r;
    }
    index = (uint8_t)(diff.value << 2);
    WR8(ZP->temp00, index);
    facing = RD8(OBJ->facing_left[slot]);
    index = (uint8_t)((facing << 1) + RD8(ZP->temp00));
    byte = rd8((uint16_t)(0xC3BE + index));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xC3BD + index));
    WR8(OBJ->vx_lo[slot], byte);
    jsr_push(0xC3AB);
    r.turn = BounceWithHeight4(slot, index);
    jsr_pop();
    r.arm.arm = S0A_PO_BOUNCE;
    return r;
}

/* The step's exit; the state's load when the list ended; the frame's compare with $0E (and the cue's A), the offset's subtraction
   and its compare with 2; the bounce's registers after it. */
static void step_and_pounce_exit_shim(StepBounceArm r)
{
    Ended e = frame_step_exit_shim(r.arm);

    if (r.arm.arm == S0A_PO_ENDED) {
        e = then(e, ended_load(4));
    } else if (r.arm.arm >= S0A_PO_NOT_FRAMES) {
        e = then(e, ended_compare(r.arm.first, 0x0E));
        if (r.arm.flag) {
            e = then(e, ended_load(0x1A));
        }
        e = then(e, of_sum(r.arm.sum));
        e = then(e, ended_compare(r.arm.sum.value, 2));
    }
    abi_ended(e);
    if (r.arm.arm == S0A_PO_BOUNCE) {
        bounce_exit_shim(r.turn);
    }
}

void abi_StepAndPounce(void)
{
    step_and_pounce_exit_shim(StepAndPounce(cpu.x, cpu.y));
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
AttackArm WalkThenTurn(uint8_t slot, uint8_t column)
{
    AttackArm r = { { { 0 }, { { 0 }, { 0 }, { 0 } } }, { 0 } };

    jsr_push(0xC26E);
    r.walk = WalkByPhase(slot, 2, column);
    jsr_pop();
    r.rest.first = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->f3618[slot], r.rest.first);
    jsr_push(0xC277);
    r.rest.sub = obj_sub(TurnWhenWaitEnds(slot));
    jsr_pop();
    jsr_push(0xC27A);
    r.rest.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.rest.arm = S0A_WT_NEAR;
    if (r.rest.distance.distance >= 0x48) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 3);
        r.rest.arm = S0A_WT_FAR;
    }
    return r;
}

/* The walk's registers, then the facing's load, the turn's exit, the X distance and its compare with $48 (and the state 3 loaded past it). */
static void walk_then_turn_exit_shim(AttackArm r)
{
    Ended e = then(ended_load(r.rest.first), turn_when_wait_ends_exit_shim(obj_arm(r.rest.sub)));

    walk_phase_exit_shim(r.walk);
    e = then(e, ended_of_distance(r.rest.distance));
    e = then(e, ended_compare(r.rest.distance.distance, 0x48));
    if (r.rest.arm == S0A_WT_FAR) {
        e = then(e, ended_load(3));
    }
    abi_ended(e);
}

void abi_WalkThenTurn(void)
{
    walk_then_turn_exit_shim(WalkThenTurn(cpu.x, cpu.y));
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
ObjArm FollowPartner(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t before, frame;

    r.arm = S0A_FP_IDLE;
    if (RD8(OBJ->state[slot]) == 0) {
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        return r;
    }
    r.third = RD8(OBJ->hit_taken[slot]);
    r.arm = S0A_FP_PLAIN;
    if (r.third >= 2) {
        jsr_push(0xC467);
        r.screen = SoundIfOnFirstScreen(0x12, slot, y);
        jsr_pop();
        r.screen_called = true;
        WR8(OBJ->invuln_timer[slot], 5);
        r.arm = S0A_FP_HIT;
    }
    before = (uint8_t)(slot - 1);
    r.y = before;
    jsr_push(0xC472);
    r.fourth = CopyObjectPlaceBack(before, slot);
    jsr_pop();
    frame = RD8_IDX((uint16_t)(COL(OBJ->frame) - 1), slot);
    r.sum = sub_bytes(frame, 1, true);
    r.fifth = (uint8_t)(r.sum.value << 2);
    r.sixth = rd8((uint16_t)(0xC4AA + r.fifth));
    WR8(OBJ->hitbox_dx[slot], r.sixth);
    r.sixth = rd8((uint16_t)(0xC4AB + r.fifth));
    WR8(OBJ->hitbox_dy[slot], r.sixth);
    r.sixth = rd8((uint16_t)(0xC4AC + r.fifth));
    WR8(OBJ->hitbox_half_w[slot], r.sixth);
    r.sixth = rd8((uint16_t)(0xC4AD + r.fifth));
    WR8(OBJ->hitbox_half_h[slot], r.sixth);
    r.sixth = RD8_IDX((uint16_t)(COL(OBJ->facing_left) - 1), slot);
    WR8(OBJ->facing_left[slot], r.sixth);
    r.sixth = RD8_IDX((uint16_t)(COL(OBJ->hitbox_on) - 1), slot);
    WR8(OBJ->hitbox_on[slot], r.sixth);
    r.seventh = RD8_IDX((uint16_t)(COL(OBJ->kind) - 1), slot);
    if (r.seventh != 0x24) {
        ClearObject(slot);
    }
    return r;
}

/* The record's exit for an idle slot; else the hit count's compare with 2 (past it the cue's A, the call, and a cleared exit), then the
   placed object's type with Y the previous slot, the frame offset's subtraction with C from its bit 6 and Y, and the kind's compare. */
static FollowEnd follow_partner_exit_shim(ObjArm r)
{
    FollowEnd s = { { 0 }, { 0 }, false, { 0 }, { 0 } };
    Ended e;

    if (r.arm == S0A_FP_IDLE) {
        s.end = record_exit_shim(r);
        return s;
    }
    e = ended_compare(r.third, 2);
    if (r.arm == S0A_FP_HIT) {
        e = then(e, ended_load(0x12));
        s.before = e;
        s.called = r.screen_called;
        s.sound = r.screen;
        e = ended_kept();
    }
    e = then(e, ended_with_y(ended_load(r.fourth), r.y));
    e = then(e, ended_with_y(ended_with_carry(of_sum(r.sum), (r.sum.value & 0x40) != 0), r.fifth));
    s.end = then(e, ended_compare(r.seventh, 0x24));
    return s;
}

void abi_FollowPartner(void)
{
    FollowEnd r = follow_partner_exit_shim(FollowPartner(cpu.x, cpu.y));

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
ObjArm JumpByStateTableC(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state, low;

    state = RD8(OBJ->state[slot]);
    r.y = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xC058 + r.y));
    WR8(ZP->temp00, low);
    r.first = rd8((uint16_t)(0xC059 + r.y));
    WR8(ZP->temp01, r.first);
    r.flag = (state & 0x80) != 0;
    r.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return r;
}

/* The high byte's load with Y the offset and C the state's bit 7, and the table's address as the jump's target. */
static Jump jump_by_state_table_c_exit_shim(ObjArm r)
{
    Jump jump = { { 0 }, 0, true };

    jump.end = ended_with_carry(ended_with_y(ended_load(r.first), r.y), r.flag);
    jump.target = r.target;
    return jump;
}

void abi_JumpByStateTableC(void)
{
    Jump jump = jump_by_state_table_c_exit_shim(JumpByStateTableC(cpu.x));

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
ObjArm FaceBreakAndRun(uint8_t slot)
{
    ObjArm r = { 0 };

    INC8(OBJ->life_timer[slot]);
    jsr_push(0xBFF8);
    r.sub = obj_sub(FaceThePlayerInSomeStatesD(slot));
    jsr_pop();
    jsr_push(0xBFFB);
    r.sub2 = obj_sub(BreakWhenHit(slot));
    jsr_pop();
    r.sub3 = obj_sub(JumpByStateTableC(slot));
    return r;
}

/* The face's exit (its side's when used), the break's exit, then the table's jump. */
static void face_break_and_run_exit_shim(ObjArm r)
{
    SideEnd side = face_player_in_states_exit_shim(obj_arm(r.sub));
    Jump jump;
    Ended e;

    e = side.side_used ? ended_of_side(side.side) : ended_kept();
    e = then(e, side.end);
    e = then(e, break_when_hit_exit_shim(obj_arm(r.sub2)));
    abi_ended(e);
    jump = jump_by_state_table_c_exit_shim(obj_arm(r.sub3));
    jump_shim(jump.end, jump.target);
}

void abi_FaceBreakAndRun(void)
{
    face_break_and_run_exit_shim(FaceBreakAndRun(cpu.x));
}
