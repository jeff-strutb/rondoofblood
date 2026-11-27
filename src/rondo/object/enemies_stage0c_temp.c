/*
 * TEMPORARY: the stage 0C61 enemies and their helpers (disc sectors 0CA9 to 0CAD), by routine until each enemy kind is
 * identified. Bodies return what they ended on (ObjArm, enemy_objects.h); each routine's exit shim builds A, Y and the
 * flags from it, as in 441de03, and its abi_ wrapper applies them.
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

/* RecordObjectEvent's exit: unmarked, A 0 and Z; else A the byte stored, Y its index, C whether the event is a multiple of 8. */
static Ended ended_of_object_event_stage(bool marked, uint8_t stored, uint8_t event, uint8_t byte)
{
    if (!marked) {
        return ended_load(0);
    }
    return ended_with_carry(ended_with_y(ended_load(stored), byte), (event & 7) == 0);
}

/* The exit of the angle scales (scale_exit_shim): A, N and Z, C, V when an add ran, and Y 0. */
static Ended ended_of_scale(ScaleResult r)
{
    Ended e = ended_with_y(ended_with_carry(ended_with_a(ended_nz(r.nz), r.a), r.carry), 0);

    return r.v_set ? ended_with_overflow(e, r.overflow) : e;
}

/* AngleToPlayer's exit: A, N, Z, C, V and Y. */
static Ended ended_of_angle(AngleExit r)
{
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_load(r.a), r.c), r.v), r.y);
}

/* CPY: N, Z and C from the register less the operand; A kept. */
static Ended cpy(uint8_t reg, uint8_t operand)
{
    return ended_with_carry(ended_nz((uint8_t)(reg - operand)), reg >= operand);
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

/* The arms the routines below end on. */
enum { SH_NOT_THREE, SH_MILD, SH_STARTED };
enum { E7_NO_FLAG, E7_LATE, E7_ENTERED };
enum { SD_SLIDING, SD_ADVANCED };
enum { SU_SLIDING, SU_ADVANCED };
enum { IB_HIGH, IB_LEFT, IB_RIGHT, IB_TOP, IB_BOTTOM, IB_INSIDE };
enum { TS_RUNNING, TS_SPAWNED };
enum { TR_RUNNING, TR_RESTED };
enum { TW_RUNNING, TW_WAITING };
enum { TP_BOTH, TP_NO_FIRST, TP_NO_SECOND };
enum { SL_BLOCKED, SL_SANK };
enum { PN_PRESENT, PN_GONE };
enum { DS_SKIPPED, DS_FULL, DS_DROPPED };
enum { SR_COUNTING, SR_RESTARTED };
enum { EF_PHASE, EF_LOW, EF_NO_SLOT, EF_SHOT };
enum { SA_COUNTING, SA_RESTARTED };
enum { FK_ENDED, FK_RUNNING, FK_MOVED };
enum { SO_LOAD, SO_NO_SLOT, SO_STARTED, SO_OTHER_STATE, SO_PART };
enum { WV_KEPT, WV_CLEARED };
enum { SM_ENDED, SM_MOVING };
enum { CK_CLEARED, CK_RUNNING };
enum { SU2_PRESENT, SU2_STARTED };
enum { FF_RECORD, FF_NO_FLAG, FF_FACED };
enum { PI_STEP, PI_START };
enum { TH_STRAIGHT, TH_REVERSED };
enum { SC_STEP, SC_START };
enum { DF_STEP, DF_STRAIGHT, DF_REVERSED };
enum { CQ_COUNTING, CQ_CUED };
enum { AF_FAR, AF_COMPARED };
enum { TL_LANDED, TL_TRACKED };
enum { TT_COUNTING, TT_AIMED };
enum { OL_ORBITING, OL_LEFT };
enum { MA_OUTSIDE, MA_SLOWING, MA_NEXT };
enum { LF_START, LF_HIT, LF_STEPPING, LF_RESTED };
enum { TK_OTHER, TK_DRAWN };
enum { CS_COUNTING, CS_DRAWN };
enum { SG_NO_LIST, SG_LIST };
enum { PP_COUNTING, PP_REPICKED };

/*
 * @routine 0CAB:175 FallingWithGravity
 * @run 70:B175
 * WHAT IT DOES: Steps the animation list at $B187, applies unlimited gravity and moves the object.
 * HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as ApplyVelocity, Y as the step left it.
 */
ObjArm FallingWithGravity(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    r.step = step_list(slot, 0xB187, 0xB180, &y, false);
    r.y = y;
    jsr_push(0xB183);
    ApplyGravityUnlimited(slot);
    jsr_pop();
    jsr_push(0xB186);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    return r;
}

static Ended falling_with_gravity_exit_shim(ObjArm r)
{
    return then(frame_step_exit_shim(r), ended_of_axis_move(r.move));
}

void abi_FallingWithGravity(void)
{
    abi_ended(falling_with_gravity_exit_shim(FallingWithGravity(cpu.x)));
}

/*
 * @routine 0CAB:203 StartWhenHurt
 * @run 70:B203
 * WHAT IT DOES: An object in state 3 that has taken a hit of 5 or more resets its animation, enters state 6 and clears OBJ_2800.
 * HOW: Two compares.
 * WAITS: no.
 * Entry: X object. Exit: the last compare or load.
 */
ObjArm StartWhenHurt(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->state[slot]);
    r.arm = SH_NOT_THREE;
    if (r.first != 3) {
        return r;
    }
    r.second = RD8(OBJ->hit_taken[slot]);
    r.arm = SH_MILD;
    if (r.second < 5) {
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 6);
    WR8(OBJ->f2800[slot], 0);
    r.arm = SH_STARTED;
    return r;
}

static Ended start_when_hurt_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 3);

    if (r.arm == SH_NOT_THREE) {
        return e;
    }
    e = then(e, ended_compare(r.second, 5));
    if (r.arm == SH_MILD) {
        return e;
    }
    return then(e, ended_load(6));
}

void abi_StartWhenHurt(void)
{
    abi_ended(start_when_hurt_exit_shim(StartWhenHurt(cpu.x)));
}

/*
 * @routine 0CAB:220 EnterStateSevenOnFlag
 * @run 70:B220
 * WHAT IT DOES: When bit 7 of the object's status flags is set and its state is below 7, it faces right, resets its animation, enters state 7 and clears OBJ_2800.
 * HOW: Two compares.
 * Untraced: every arm past the flag test (B227 to B23E).
 * WAITS: no.
 * Entry: X object. Exit: the last AND, compare or load.
 */
ObjArm EnterStateSevenOnFlag(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->status_flags[slot]);
    r.first = (uint8_t)(r.first & 0x80);
    r.arm = E7_NO_FLAG;
    if (r.first == 0) {
        return r;
    }
    r.second = RD8(OBJ->state[slot]);
    r.arm = E7_LATE;
    if (r.second >= 7) {
        return r;
    }
    WR8(OBJ->facing_left[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 7);
    WR8(OBJ->f2800[slot], 0);
    r.arm = E7_ENTERED;
    return r;
}

static Ended enter_state_seven_on_flag_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == E7_NO_FLAG) {
        return e;
    }
    e = then(e, ended_compare(r.second, 7));
    if (r.arm == E7_LATE) {
        return e;
    }
    return then(e, ended_load(7));
}

void abi_EnterStateSevenOnFlag(void)
{
    abi_ended(enter_state_seven_on_flag_exit_shim(EnterStateSevenOnFlag(cpu.x)));
}

/*
 * @routine 0CAB:304 StartWithGuard
 * @run 70:B304
 * WHAT IT DOES: Loads the object's record and starts a kind $25 object at its place in slot $11; its timer ends as $80.
 * HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: A $80, Y $11, C from the record, N set.
 */
ObjArm StartWithGuard(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB307);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    jsr_push(0xB30C);
    ClearObjectInY(0x11);
    jsr_pop();
    jsr_push(0xB311);
    r.third = StartObjectAtPlace(0x25, slot, 0x11);
    jsr_pop();
    WR8(OBJ->life_timer[slot], 0xE0);
    WR8(OBJ->life_timer[slot], 0x80);
    return r;
}

static Ended start_with_guard_exit_shim(ObjArm r)
{
    Ended e = then(record_exit_shim(r), ended_with_y(ended_load(r.third), 0x11));

    return then(e, ended_load(0x80));
}

void abi_StartWithGuard(void)
{
    abi_ended(start_with_guard_exit_shim(StartWithGuard(cpu.x)));
}

/*
 * @routine 0CAB:389 SlideAngleDown
 * @run 70:B389
 * WHAT IT DOES: Takes one off the 16-bit angle word (OBJ_3568 high, OBJ_35C0 low, in steps of $0100 less 0 on the low byte as the code subtracts $0100); when the high byte
 * reaches 0 the object's OBJ_2800 goes up one.
 * HOW: A 16-bit subtract.
 * WAITS: no.
 * Entry: X object. Exit: the last subtract or INC.
 */
ObjArm SlideAngleDown(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t low, high;

    low = RD8(OBJ->angle_lo[slot]);
    r.sum = sub_bytes(low, 0, true);
    WR8(OBJ->angle_lo[slot], r.sum.value);
    high = RD8(OBJ->angle_hi[slot]);
    r.sum2 = sub_bytes(high, 1, r.sum.carry);
    WR8(OBJ->angle_hi[slot], r.sum2.value);
    r.arm = SD_SLIDING;
    if (r.sum2.value != 0) {
        return r;
    }
    r.first = INC8(OBJ->f2800[slot]);
    r.arm = SD_ADVANCED;
    return r;
}

static Ended slide_angle_down_exit_shim(ObjArm r)
{
    Ended e = then(of_sum(r.sum), of_sum(r.sum2));

    if (r.arm == SD_SLIDING) {
        return e;
    }
    return then(e, ended_nz(r.first));
}

void abi_SlideAngleDown(void)
{
    abi_ended(slide_angle_down_exit_shim(SlideAngleDown(cpu.x)));
}

/*
 * @routine 0CAB:3A0 SlideAngleUp
 * @run 70:B3A0
 * WHAT IT DOES: Adds $00C0 to the 16-bit angle word (OBJ_3568 high, OBJ_35C0 low); once the high byte reaches $10 the object's OBJ_2800 goes up one.
 * HOW: A 16-bit add and a compare.
 * WAITS: no.
 * Entry: X object. Exit: the compare or the INC.
 */
ObjArm SlideAngleUp(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t low, high;

    low = RD8(OBJ->angle_lo[slot]);
    r.sum = add_bytes(low, 0xC0, false);
    WR8(OBJ->angle_lo[slot], r.sum.value);
    high = RD8(OBJ->angle_hi[slot]);
    r.sum2 = add_bytes(high, 0, r.sum.carry);
    WR8(OBJ->angle_hi[slot], r.sum2.value);
    r.arm = SU_SLIDING;
    if (r.sum2.value < 0x10) {
        return r;
    }
    r.first = INC8(OBJ->f2800[slot]);
    r.arm = SU_ADVANCED;
    return r;
}

static Ended slide_angle_up_exit_shim(ObjArm r)
{
    Ended e = then(then(of_sum(r.sum), of_sum(r.sum2)), ended_compare(r.sum2.value, 0x10));

    if (r.arm == SU_SLIDING) {
        return e;
    }
    return then(e, ended_nz(r.first));
}

void abi_SlideAngleUp(void)
{
    abi_ended(slide_angle_up_exit_shim(SlideAngleUp(cpu.x)));
}

/*
 * @routine 0CAB:422 AimAtPlayerThenArm
 * @run 70:B422
 * WHAT IT DOES: Takes OBJ_3510 as the angle to the player (a random one on three counts out of four, by OBJ_3670 and 3), sets the angle word from a table at $B44F by
 * OBJ_3670 and 3, and moves OBJ_3670 and OBJ_2800 on.
 * HOW: AngleToPlayer, NextRandom, table reads (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: N and Z of the new OBJ_2800; A the table's low byte, Y the table index, C 0.
 */
ObjArm AimAtPlayerThenArm(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t phase, byte;

    jsr_push(0xB425);
    r.angle = AngleToPlayer(slot);
    jsr_pop();
    WR8(OBJ->f3510[slot], r.angle.a);
    phase = RD8(OBJ->f3670[slot]);
    r.first = (uint8_t)(phase & 0x03);
    if (r.first != 0) {
        jsr_push(0xB432);
        r.random = NextRandom();
        jsr_pop();
        r.flag = true;
        WR8(OBJ->f3510[slot], r.random.value);
    }
    phase = RD8(OBJ->f3670[slot]);
    phase = (uint8_t)(phase & 0x03);
    r.second = (uint8_t)(phase << 1);
    byte = rd8((uint16_t)(0xB450 + r.second));
    WR8(OBJ->angle_hi[slot], byte);
    r.third = rd8((uint16_t)(0xB44F + r.second));
    WR8(OBJ->angle_lo[slot], r.third);
    r.fourth = INC8(OBJ->f3670[slot]);
    r.fifth = INC8(OBJ->f2800[slot]);
    return r;
}

static Ended aim_at_player_then_arm_exit_shim(ObjArm r)
{
    Ended e = then(ended_of_angle(r.angle), ended_load(r.first));

    if (r.flag) {
        e = then(e, ended_of_random(r.random));
    }
    e = then(e, ended_with_y(ended_with_carry(ended_load(r.second), false), r.second));
    e = then(e, ended_load(r.third));
    e = then(e, ended_nz(r.fourth));
    return then(e, ended_nz(r.fifth));
}

void abi_AimAtPlayerThenArm(void)
{
    abi_ended(aim_at_player_then_arm_exit_shim(AimAtPlayerThenArm(cpu.x)));
}

/*
 * @routine 0CAB:4AC InsideScreenBox
 * @run 70:B4AC
 * WHAT IT DOES: Says whether the object is inside the box X $20 to $DF, Y $20 to $BF on the first screen: C clear when inside, set when not.
 * HOW: The position's high bytes ORed, then four compares.
 * Untraced: the outside answer (B4CC).
 * Curated cases: tools/rondo/curated/inside_screen_box.txt (150 inside cases; the draft reaches the untraced outside answer on the seeded ones).
 * WAITS: no.
 * Entry: X object. Exit: C clear when inside; A and the flags as the last compare or load left them.
 */
ObjArm InsideScreenBox(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->x_hi[slot]);
    r.first = (uint8_t)(r.first | RD8(OBJ->y_hi[slot]));
    r.arm = IB_HIGH;
    if (r.first != 0) {
        return r;
    }
    r.second = RD8(OBJ->x_lo[slot]);
    r.arm = IB_LEFT;
    if (r.second < 0x20) {
        return r;
    }
    r.arm = IB_RIGHT;
    if (r.second >= 0xE0) {
        return r;
    }
    r.third = RD8(OBJ->y_lo[slot]);
    r.arm = IB_TOP;
    if (r.third < 0x20) {
        return r;
    }
    r.arm = IB_BOTTOM;
    if (r.third >= 0xC0) {
        return r;
    }
    r.flag = true;
    r.arm = IB_INSIDE;
    return r;
}

/* Whether the object is inside the box, and what the routine left. */
static ViewTest inside_screen_box_exit_shim(ObjArm r)
{
    ViewTest v = { r.flag, { 0 } };
    Ended e = ended_load(r.first);

    if (r.arm != IB_HIGH) {
        e = then(e, ended_compare(r.second, 0x20));
        if (r.arm != IB_LEFT) {
            e = then(e, ended_compare(r.second, 0xE0));
            if (r.arm != IB_RIGHT) {
                e = then(e, ended_compare(r.third, 0x20));
                if (r.arm != IB_TOP) {
                    e = then(e, ended_compare(r.third, 0xC0));
                }
            }
        }
    }
    v.end = then(e, ended_with_carry(ended_kept(), r.arm != IB_INSIDE));
    return v;
}

void abi_InsideScreenBox(void)
{
    abi_ended(inside_screen_box_exit_shim(InsideScreenBox(cpu.x)).end);
}

/*
 * @routine 0CAB:4CE CountThenStop
 * @run 70:B4CE
 * WHAT IT DOES: Counts the object's timer down; at zero it clears OBJ_2800.
 * HOW: DEC and a store.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the count.
 */
ObjArm CountThenStop(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = DEC8(OBJ->life_timer[slot]);
    if (r.first == 0) {
        WR8(OBJ->f2800[slot], 0);
    }
    return r;
}

static Ended count_then_stop_exit_shim(ObjArm r)
{
    return ended_nz(r.first);
}

void abi_CountThenStop(void)
{
    abi_ended(count_then_stop_exit_shim(CountThenStop(cpu.x)));
}

/*
 * @routine 0CAB:509 PickNextMoveBySide
 * @run 70:B509
 * WHAT IT DOES: Faces the player (bit 0 of SideOfPlayer), picks OBJ_2800 from a table at $B52C by the object's hit points halved (the high nibble) and a counter OBJ_3720 (the low
 * nibble), and counts the counter up.
 * HOW: SideOfPlayer, table read (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: N and Z of the new counter.
 */
ObjArm PickNextMoveBySide(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t index, byte;

    jsr_push(0xB50C);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.first = (uint8_t)(r.side.sides & 0x01);
    WR8(OBJ->facing_left[slot], r.first);
    r.second = RD8(OBJ->f32a8[slot]);
    r.third = (uint8_t)((r.second >> 1) & 0xF0);
    WR8(ZP->temp00, r.third);
    r.fourth = RD8(OBJ->f3720[slot]);
    r.fourth = (uint8_t)(r.fourth & 0x0F);
    index = RD8(ZP->temp00);
    r.sum = add_bytes(r.fourth, index, false);
    byte = rd8((uint16_t)(0xB52C + r.sum.value));
    WR8(OBJ->f2800[slot], byte);
    r.fifth = byte;
    r.sixth = INC8(OBJ->f3720[slot]);
    return r;
}

static Ended pick_next_move_by_side_exit_shim(ObjArm r)
{
    Ended e = then(ended_of_side(r.side), ended_load(r.first));

    e = then(e, ended_with_carry(ended_load((uint8_t)(r.second >> 1)), (r.second & 0x01) != 0));
    e = then(e, ended_load(r.third));
    e = then(e, ended_load(r.fourth));
    e = then(e, of_sum(r.sum));
    e = then(e, ended_with_y(ended_load(r.fifth), r.sum.value));
    return then(e, ended_nz(r.sixth));
}

void abi_PickNextMoveBySide(void)
{
    abi_ended(pick_next_move_by_side_exit_shim(PickNextMoveBySide(cpu.x)));
}

/*
 * @routine 0CAB:55C StepThenSpawnShot
 * @run 70:B55C
 * WHAT IT DOES: Steps the animation list at $B582; when it ends, starts a kind $28 object at its place in slot $12 with a timer of 1 and its facing, and moves OBJ_2800 on.
 * HOW: StepObjectFrames, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: list running: as the step; ended: N and Z of the new OBJ_2800.
 */
ObjArm StepThenSpawnShot(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    r.step = step_list(slot, 0xB582, 0xB567, &y, false);
    r.y = y;
    r.arm = TS_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    jsr_push(0xB56E);
    ClearObjectInY(0x12);
    jsr_pop();
    jsr_push(0xB573);
    r.first = StartObjectAtPlace(0x28, slot, 0x12);
    jsr_pop();
    WR8(OBJ->life_timer[0x12], 1);
    r.second = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[0x12], r.second);
    r.third = INC8(OBJ->f2800[slot]);
    r.arm = TS_SPAWNED;
    return r;
}

static Ended step_then_spawn_shot_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == TS_RUNNING) {
        return e;
    }
    e = then(e, ended_with_y(ended_load(r.first), 0x12));
    e = then(e, ended_load(r.second));
    return then(e, ended_nz(r.third));
}

void abi_StepThenSpawnShot(void)
{
    abi_ended(step_then_spawn_shot_exit_shim(StepThenSpawnShot(cpu.x)));
}

/*
 * @routine 0CAB:5A0 ResetToStateThree
 * @run 70:B5A0
 * WHAT IT DOES: Sets the object's frame to 1, its state to 3 and clears OBJ_2800.
 * HOW: Three stores.
 * WAITS: no.
 * Entry: X object. Exit: A 3, N and Z from it.
 */
ObjArm ResetToStateThree(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(OBJ->frame[slot], 1);
    WR8(OBJ->state[slot], 3);
    WR8(OBJ->f2800[slot], 0);
    return r;
}

static Ended reset_to_state_three_exit_shim(ObjArm r)
{
    (void)r;
    return ended_load(3);
}

void abi_ResetToStateThree(void)
{
    abi_ended(reset_to_state_three_exit_shim(ResetToStateThree(cpu.x)));
}

/*
 * @routine 0CAB:5AE StartThreeShots
 * @run 70:B5AE
 * WHAT IT DOES: Starts three kind $26 objects at this object's place in slots $13 to $15, each with this object's facing and its number 0 to 2 as its timer.
 * HOW: A loop of ClearObjectInY and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: A the last number, Y $16, N, Z and C from the compare of Y with $16.
 */
ObjArm StartThreeShots(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t to, facing;

    WR8(ZP->temp00, 0);
    for (to = 0x13; to < 0x16; to++) {
        jsr_push(0xB5B5);
        ClearObjectInY(to);
        jsr_pop();
        jsr_push(0xB5BA);
        StartObjectAtPlace(0x26, slot, to);
        jsr_pop();
        facing = RD8(OBJ->facing_left[slot]);
        WR8(OBJ->facing_left[to], facing);
        r.first = RD8(ZP->temp00);
        WR8(OBJ->life_timer[to], r.first);
        INC8(ZP->temp00);
    }
    return r;
}

/* Only the last pass's loads and compare show: each pass replaces the one before. */
static Ended start_three_shots_exit_shim(ObjArm r)
{
    Ended e = ended_with_y(ended_with_a(ended_load(r.first), r.first), 0x15);

    return then(e, ended_with_y(cpy(0x16, 0x16), 0x16));
}

void abi_StartThreeShots(void)
{
    abi_ended(start_three_shots_exit_shim(StartThreeShots(cpu.x)));
}

/* The step all the shot throwers share when their list ends: a kind $28 object in slot $12 at this object's place. */
static ObjArm spawn_shot(uint8_t slot, uint16_t ret_clear, uint16_t ret_start, int timer, bool timer_first)
{
    ObjArm r = { 0 };

    jsr_push(ret_clear);
    ClearObjectInY(0x12);
    jsr_pop();
    jsr_push(ret_start);
    r.first = StartObjectAtPlace(0x28, slot, 0x12);
    jsr_pop();
    if (timer_first && timer >= 0) {
        WR8(OBJ->life_timer[0x12], (uint8_t)timer);
        r.flag = true;
        r.fourth = (uint8_t)timer;
    }
    r.second = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[0x12], r.second);
    r.third = INC8(OBJ->f2800[slot]);
    return r;
}

static Ended spawn_shot_exit_shim(ObjArm r)
{
    Ended e = ended_with_y(ended_load(r.first), 0x12);

    if (r.flag) {
        e = then(e, ended_load(r.fourth));
    }
    e = then(e, ended_load(r.second));
    return then(e, ended_nz(r.third));
}

/*
 * @routine 0CAB:666 StepThenThrowShot
 * @run 70:B666
 * WHAT IT DOES: Steps the animation list at $B582; when it ends, starts a kind $28 object at its place in slot $12 with its facing and moves OBJ_2800 on.
 * HOW: StepObjectFrames, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: list running: as the step; ended: N and Z of the new OBJ_2800.
 */
ObjArm StepThenThrowShot(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    r.step = step_list(slot, 0xB582, 0xB671, &y, false);
    r.y = y;
    r.arm = TS_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    r.sub = obj_sub(spawn_shot(slot, 0xB678, 0xB67D, -1, false));
    r.arm = TS_SPAWNED;
    return r;
}

static Ended step_then_throw_shot_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == TS_RUNNING) {
        return e;
    }
    return then(e, spawn_shot_exit_shim(obj_arm(r.sub)));
}

void abi_StepThenThrowShot(void)
{
    abi_ended(step_then_throw_shot_exit_shim(StepThenThrowShot(cpu.x)));
}

/*
 * @routine 0CAB:6F3 StepThenThrowSlowShot
 * @run 70:B6F3
 * WHAT IT DOES: As StepThenThrowShot, but the shot's timer is 2.
 * HOW: StepObjectFrames, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry and exit: as StepThenThrowShot.
 */
ObjArm StepThenThrowSlowShot(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    r.step = step_list(slot, 0xB582, 0xB6FE, &y, false);
    r.y = y;
    r.arm = TS_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    r.sub = obj_sub(spawn_shot(slot, 0xB705, 0xB70A, 2, true));
    r.arm = TS_SPAWNED;
    return r;
}

static Ended step_then_throw_slow_shot_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == TS_RUNNING) {
        return e;
    }
    return then(e, spawn_shot_exit_shim(obj_arm(r.sub)));
}

void abi_StepThenThrowSlowShot(void)
{
    abi_ended(step_then_throw_slow_shot_exit_shim(StepThenThrowSlowShot(cpu.x)));
}

/* The step several objects share when their list ends: frame 1, state 3, OBJ_2800 cleared. */
static ObjArm step_then_rest(uint8_t slot, uint16_t list, uint16_t ret, bool frame_one)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    r.step = step_list(slot, list, ret, &y, false);
    r.y = y;
    r.arm = TR_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->frame[slot], frame_one ? 1 : 0);
    WR8(OBJ->state[slot], 3);
    WR8(OBJ->f2800[slot], 0);
    r.arm = TR_RESTED;
    return r;
}

static Ended step_then_rest_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == TR_RUNNING) {
        return e;
    }
    return then(e, ended_load(3));
}

/*
 * @routine 0CAB:640 StepThenRest
 * @run 70:B640
 * WHAT IT DOES: Steps the animation list at $B65B; when it ends the object shows frame 1, enters state 3 and clears OBJ_2800.
 * HOW: StepObjectFrames and three stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: A 3.
 */
ObjArm StepThenRest(uint8_t slot)
{
    return step_then_rest(slot, 0xB65B, 0xB64B, true);
}

void abi_StepThenRest(void)
{
    abi_ended(step_then_rest_exit_shim(StepThenRest(cpu.x)));
}

/*
 * @routine 0CAB:7A9 StepThenRestB
 * @run 70:B7A9
 * WHAT IT DOES: As StepThenRest with the list at $B7C4.
 * HOW: As StepThenRest.
 * WAITS: no.
 * Entry and exit: as StepThenRest.
 */
ObjArm StepThenRestB(uint8_t slot)
{
    return step_then_rest(slot, 0xB7C4, 0xB7B4, true);
}

void abi_StepThenRestB(void)
{
    abi_ended(step_then_rest_exit_shim(StepThenRestB(cpu.x)));
}

/*
 * @routine 0CAB:5CD StepThenWaitLonger
 * @run 70:B5CD
 * WHAT IT DOES: Steps the animation list at $B5E6; when it ends the object's frame is 0, its timer $30 and OBJ_2800 goes on.
 * HOW: StepObjectFrames and stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: N and Z of the new OBJ_2800.
 */
ObjArm StepThenWaitLonger(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    r.step = step_list(slot, 0xB5E6, 0xB5D8, &y, false);
    r.y = y;
    r.arm = TW_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->frame[slot], 0);
    WR8(OBJ->life_timer[slot], 0x30);
    r.first = INC8(OBJ->f2800[slot]);
    r.arm = TW_WAITING;
    return r;
}

static Ended step_then_wait_longer_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == TW_RUNNING) {
        return e;
    }
    return then(e, ended_with_a(ended_nz(r.first), 0x30));
}

void abi_StepThenWaitLonger(void)
{
    abi_ended(step_then_wait_longer_exit_shim(StepThenWaitLonger(cpu.x)));
}

/*
 * @routine 0CAB:62B StartTwoPieces
 * @run 70:B62B
 * WHAT IT DOES: Starts up to two kind $29 objects at this object's place in free slots from $20 (each only if a slot is free).
 * HOW: FindFreeSlotFrom20 and StartObjectAtPlace, twice.
 * WAITS: no.
 * Entry: X object. Exit: as the last search or start.
 */
ObjArm StartTwoPieces(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t found_slot, round;
    bool found;

    r.arm = TP_BOTH;
    for (round = 0; round < 2; round++) {
        jsr_push(round == 0 ? 0xB62E : 0xB638);
        found_slot = FindFreeSlotFrom20(&r.first, &found);
        jsr_pop();
        r.second = found_slot;
        r.flag = found;
        if (!found) {
            r.arm = round == 0 ? TP_NO_FIRST : TP_NO_SECOND;
            return r;
        }
        jsr_push(round == 0 ? 0xB635 : 0xB63F);
        r.third = StartObjectAtPlace(0x29, slot, found_slot);
        jsr_pop();
    }
    return r;
}

/* Each round's search replaces the exit of the one before; the last start's load follows when both were made. */
static Ended start_two_pieces_exit_shim(ObjArm r)
{
    Ended e = ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), r.first), r.second), r.flag);

    if (r.arm != TP_BOTH) {
        return e;
    }
    return then(e, ended_load(r.third));
}

void abi_StartTwoPieces(void)
{
    abi_ended(start_two_pieces_exit_shim(StartTwoPieces(cpu.x)));
}

/*
 * @routine 0CAB:69A SinkToLimit
 * @run 70:B69A
 * WHAT IT DOES: Takes 3 off the object's Y low byte while it is $30 or more.
 * HOW: A compare and a subtract.
 * WAITS: no.
 * Entry: X object. Exit: under $30: the compare's flags; else the subtract's.
 */
ObjArm SinkToLimit(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t low;

    r.first = RD8(OBJ->y_lo[slot]);
    r.arm = SL_BLOCKED;
    if (r.first < 0x30) {
        return r;
    }
    low = RD8(OBJ->y_lo[slot]);
    r.sum = sub_bytes(low, 3, true);
    WR8(OBJ->y_lo[slot], r.sum.value);
    r.arm = SL_SANK;
    return r;
}

static Ended sink_to_limit_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 0x30);

    if (r.arm == SL_BLOCKED) {
        return e;
    }
    return then(e, of_sum(r.sum));
}

void abi_SinkToLimit(void)
{
    abi_ended(sink_to_limit_exit_shim(SinkToLimit(cpu.x)));
}

/*
 * @routine 0CAB:687 SinkWhenPartnerGone
 * @run 70:B687
 * WHAT IT DOES: When slot $12's kind is not $28 the object sinks (SinkToLimit), its timer becomes $30 and OBJ_2800 moves on.
 * HOW: A compare, SinkToLimit, two stores.
 * WAITS: no.
 * Entry: X object. Exit: the compare's flags, or N and Z of the new OBJ_2800.
 */
ObjArm SinkWhenPartnerGone(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->kind[0x12]);
    r.arm = PN_PRESENT;
    if (r.first == 0x28) {
        return r;
    }
    jsr_push(0xB691);
    r.sub = obj_sub(SinkToLimit(slot));
    jsr_pop();
    WR8(OBJ->life_timer[slot], 0x30);
    r.second = INC8(OBJ->f2800[slot]);
    r.arm = PN_GONE;
    return r;
}

static Ended sink_when_partner_gone_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 0x28);

    if (r.arm == PN_PRESENT) {
        return e;
    }
    e = then(e, sink_to_limit_exit_shim(obj_arm(r.sub)));
    e = then(e, ended_load(0x30));
    return then(e, ended_nz(r.second));
}

void abi_SinkWhenPartnerGone(void)
{
    abi_ended(sink_when_partner_gone_exit_shim(SinkWhenPartnerGone(cpu.x)));
}

/*
 * @routine 0CAB:719 SetLongWait
 * @run 70:B719
 * WHAT IT DOES: Sets the object's timer to $40 and moves OBJ_2800 on.
 * HOW: A store and an INC.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the new OBJ_2800, A $40.
 */
ObjArm SetLongWait(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(OBJ->life_timer[slot], 0x40);
    r.first = INC8(OBJ->f2800[slot]);
    return r;
}

static Ended set_long_wait_exit_shim(ObjArm r)
{
    return ended_with_a(ended_nz(r.first), 0x40);
}

void abi_SetLongWait(void)
{
    abi_ended(set_long_wait_exit_shim(SetLongWait(cpu.x)));
}

/*
 * @routine 0CAB:738 DropShotWhenSlotFree
 * @run 70:B738
 * WHAT IT DOES: Every 16th count of OBJ_3618, searches slots $30 to $35 for a free one and starts a kind $2A object there at its place.
 * HOW: A search by kind, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: the last compare or load.
 */
ObjArm DropShotWhenSlotFree(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t phase, to, kind;

    phase = RD8(OBJ->f3618[slot]);
    r.first = (uint8_t)(phase & 0x0F);
    r.arm = DS_SKIPPED;
    if (r.first != 0) {
        return r;
    }
    to = 0x30;
    for (;;) {
        kind = RD8(OBJ->kind[to]);
        r.bytes[r.second] = kind;
        r.second++;
        if (kind == 0) {
            break;
        }
        to++;
        if (to >= 0x36) {
            r.arm = DS_FULL;
            return r;
        }
    }
    jsr_push(0xB74F);
    ClearObjectInY(to);
    jsr_pop();
    jsr_push(0xB754);
    r.fourth = StartObjectAtPlace(0x2A, slot, to);
    jsr_pop();
    r.arm = DS_DROPPED;
    return r;
}

/* Each pass of the search: the kind's load, then INY and CPY #$36; the free slot's pass ends on its load. */
static Ended drop_shot_when_slot_free_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);
    uint8_t to = 0x30, i;

    if (r.arm == DS_SKIPPED) {
        return e;
    }
    e = then(e, ended_with_y(ended_nz(to), to));
    for (i = 0; i < r.second; i++) {
        e = then(e, ended_load(r.bytes[i]));
        if (r.arm == DS_DROPPED && i + 1 == r.second) {
            break;
        }
        to++;
        e = then(e, ended_with_y(ended_nz(to), to));
        e = then(e, cpy(to, 0x36));
    }
    if (r.arm == DS_FULL) {
        return e;
    }
    return then(e, ended_load(r.fourth));
}

void abi_DropShotWhenSlotFree(void)
{
    abi_ended(drop_shot_when_slot_free_exit_shim(DropShotWhenSlotFree(cpu.x)));
}

/*
 * @routine 0CAB:722 SinkThenRestart
 * @run 70:B722
 * WHAT IT DOES: Drops a shot when a slot is free (DropShotWhenSlotFree), counts its timer down and at zero shows frame 1, enters state 3 and clears OBJ_2800.
 * HOW: DropShotWhenSlotFree, DEC, three stores.
 * WAITS: no.
 * Entry: X object. Exit: the count's N and Z, or A 3 at zero.
 */
ObjArm SinkThenRestart(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB725);
    r.sub = obj_sub(DropShotWhenSlotFree(slot));
    jsr_pop();
    r.first = DEC8(OBJ->life_timer[slot]);
    r.arm = SR_COUNTING;
    if (r.first != 0) {
        return r;
    }
    WR8(OBJ->frame[slot], 1);
    WR8(OBJ->state[slot], 3);
    WR8(OBJ->f2800[slot], 0);
    r.arm = SR_RESTARTED;
    return r;
}

static Ended sink_then_restart_exit_shim(ObjArm r)
{
    Ended e = then(drop_shot_when_slot_free_exit_shim(obj_arm(r.sub)), ended_nz(r.first));

    if (r.arm == SR_COUNTING) {
        return e;
    }
    return then(e, ended_load(3));
}

void abi_SinkThenRestart(void)
{
    abi_ended(sink_then_restart_exit_shim(SinkThenRestart(cpu.x)));
}

/*
 * @routine 0CAB:6C4 ShootEveryFourth
 * @run 70:B6C4
 * WHAT IT DOES: Every fourth count of OBJ_3618, while Y is under $A0, starts a kind $27 object at its place in the first free slot from $20 (sound $26 first), with the
 * object's facing and its OBJ_3778 count (low three bits) as its next kind; the count goes up.
 * HOW: FindFreeSlotFrom20, SoundCommand, StartObjectAtPlace.
 * Untraced: two bytes.
 * WAITS: no.
 * Entry: X object, Y pushed by the sound driver. Exit: see the wrapper.
 */
ObjArm ShootEveryFourth(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t phase, count, found_slot;
    bool found;

    (void)y;
    phase = RD8(OBJ->f3618[slot]);
    r.first = (uint8_t)(phase & 0x03);
    r.arm = EF_PHASE;
    if (r.first != 0) {
        return r;
    }
    r.second = RD8(OBJ->y_lo[slot]);
    r.arm = EF_LOW;
    if (r.second >= 0xA0) {
        return r;
    }
    jsr_push(0xB6D5);
    found_slot = FindFreeSlotFrom20(&r.third, &found);
    jsr_pop();
    r.fourth = found_slot;
    r.flag = found;
    r.arm = EF_NO_SLOT;
    if (!found) {
        return r;
    }
    r.arm = EF_SHOT;
    r.sounded = true;
    r.command = 0x26;
    jsr_push(0xB6DC);
    r.call = SoundCommand(0x26, slot, found_slot);
    jsr_pop();
    jsr_push(0xB6E1);
    r.fifth = StartObjectAtPlace(0x27, slot, found_slot);
    jsr_pop();
    r.sixth = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[found_slot], r.sixth);
    count = RD8(OBJ->f3778[slot]);
    r.seventh = (uint8_t)(count & 0x07);
    WR8(OBJ->next_kind[found_slot], r.seventh);
    r.eighth = INC8(OBJ->f3778[slot]);
    return r;
}

static SoundEnd shoot_every_fourth_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = ended_load(r.first);

    s.end = e;
    if (r.arm == EF_PHASE) {
        return s;
    }
    e = then(e, ended_compare(r.second, 0xA0));
    s.end = e;
    if (r.arm == EF_LOW) {
        return s;
    }
    e = then(e, ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), r.third), r.fourth), r.flag));
    s.end = e;
    if (r.arm == EF_NO_SLOT) {
        return s;
    }
    s.before = then(e, ended_load(0x26));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    e = ended_load(r.fifth);
    e = then(e, ended_load(r.sixth));
    e = then(e, ended_load(r.seventh));
    s.end = then(e, ended_nz(r.eighth));
    return s;
}

void abi_ShootEveryFourth(void)
{
    abi_sound_end(shoot_every_fourth_exit_shim(ShootEveryFourth(cpu.x, cpu.y)));
}

/*
 * @routine 0CAB:6AB SinkAndShootThenRestart
 * @run 70:B6AB
 * WHAT IT DOES: ShootEveryFourth and SinkToLimit; the timer counts down and at zero the object shows frame 1, enters state 3 and clears OBJ_2800.
 * HOW: The two calls, a DEC and three stores.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: see the wrapper.
 */
ObjArm SinkAndShootThenRestart(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };

    jsr_push(0xB6AE);
    r.sub = obj_sub(ShootEveryFourth(slot, y));
    jsr_pop();
    jsr_push(0xB6B1);
    r.sub2 = obj_sub(SinkToLimit(slot));
    jsr_pop();
    r.first = DEC8(OBJ->life_timer[slot]);
    r.arm = SA_COUNTING;
    if (r.first == 0) {
        WR8(OBJ->frame[slot], 1);
        WR8(OBJ->state[slot], 3);
        WR8(OBJ->f2800[slot], 0);
        r.arm = SA_RESTARTED;
    }
    return r;
}

static ShootTail sink_and_shoot_then_restart_exit_shim(ObjArm r)
{
    ShootTail t;
    Ended e = then(sink_to_limit_exit_shim(obj_arm(r.sub2)), ended_nz(r.first));

    t.shoot = shoot_every_fourth_exit_shim(obj_arm(r.sub));
    if (r.arm == SA_RESTARTED) {
        e = then(e, ended_load(3));
    }
    t.tail = e;
    return t;
}

void abi_SinkAndShootThenRestart(void)
{
    ShootTail t = sink_and_shoot_then_restart_exit_shim(SinkAndShootThenRestart(cpu.x, cpu.y));

    abi_sound_end(t.shoot);
    abi_ended(t.tail);
}

/*
 * @routine 0CAB:5F1 FlickerAndStartPieces
 * @run 70:B5F1
 * WHAT IT DOES: Counts the object's timer down; while it runs it starts up to two pieces (StartTwoPieces) and, when the timer is $18, moves the object to a random
 * place (X and Y low bytes from a table at $B623, high bytes 0); at zero OBJ_2800 moves on.
 * HOW: DEC, StartTwoPieces, NextRandom twice, table reads (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: the last compare, load or INC.
 */
ObjArm FlickerAndStartPieces(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t byte;

    r.first = DEC8(OBJ->life_timer[slot]);
    r.arm = FK_ENDED;
    if (r.first == 0) {
        r.second = INC8(OBJ->f2800[slot]);
        return r;
    }
    jsr_push(0xB5F9);
    r.sub = obj_sub(StartTwoPieces(slot));
    jsr_pop();
    r.third = RD8(OBJ->life_timer[slot]);
    r.arm = FK_RUNNING;
    if (r.third != 0x18) {
        return r;
    }
    WR8(OBJ->x_hi[slot], 0);
    jsr_push(0xB606);
    r.random = NextRandom();
    jsr_pop();
    r.fourth = (uint8_t)(r.random.value & 0x07);
    byte = rd8((uint16_t)(0xB623 + r.fourth));
    WR8(OBJ->x_lo[slot], byte);
    r.fifth = byte;
    WR8(OBJ->y_hi[slot], 0);
    jsr_push(0xB615);
    r.random2 = NextRandom();
    jsr_pop();
    r.sixth = (uint8_t)(r.random2.value & 0x07);
    byte = rd8((uint16_t)(0xB623 + r.sixth));
    WR8(OBJ->y_lo[slot], byte);
    r.seventh = byte;
    r.arm = FK_MOVED;
    return r;
}

static Ended flicker_and_start_pieces_exit_shim(ObjArm r)
{
    Ended e = ended_nz(r.first);

    if (r.arm == FK_ENDED) {
        return then(e, ended_nz(r.second));
    }
    e = then(e, start_two_pieces_exit_shim(obj_arm(r.sub)));
    e = then(e, ended_compare(r.third, 0x18));
    if (r.arm == FK_RUNNING) {
        return e;
    }
    e = then(e, ended_of_random(r.random));
    e = then(e, ended_with_y(ended_load(r.fourth), r.fourth));
    e = then(e, ended_load(r.fifth));
    e = then(e, ended_of_random(r.random2));
    e = then(e, ended_with_y(ended_load(r.sixth), r.sixth));
    return then(e, ended_load(r.seventh));
}

void abi_FlickerAndStartPieces(void)
{
    abi_ended(flicker_and_start_pieces_exit_shim(FlickerAndStartPieces(cpu.x)));
}

/*
 * @routine 0CAD:12A StepPartList
 * @run 71:C12A
 * WHAT IT DOES: Steps the animation list at $C196.
 * HOW: The list's address goes to zTemp10-11; a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as StepObjectFrames.
 */
ObjArm StepPartList(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    WR8(ZP->temp10, 0x96);
    WR8(ZP->temp11, 0xC1);
    r.step = StepObjectFrames(slot, &y);
    r.y = y;
    return r;
}

void abi_StepPartList(void)
{
    abi_ended(frame_step_exit_shim(StepPartList(cpu.x)));
}

/*
 * @routine 0CAD:0BC ShatterOnHits
 * @run 71:C0BC
 * @stop C11F
 * WHAT IT DOES: An object that breaks into a piece when hit often. State 0 loads its record and sets sprite set 6, frame 1 and X speed high 4. Once its hits taken reach 5 it
 * records its event, takes a kind from the table at $C160 by the player's stock (at most 4), clears its state, speeds and parts, sets Y speed high $FB and its timer
 * to $16 and starts a kind $9B object at its place from a free slot from $50. Before that it steps the part list (StepPartList) in state 1 and
 * goes on at $C11F once the scroll X low is $80 or more; other states go on at $C135 (neither ever ran in a script).
 * HOW: LoadObjectRecord, RecordObjectEvent, FindFreeSlotFrom50, StartObjectAtPlace, StepPartList.
 * WAITS: no.
 * Entry: X object. Exit: see the wrapper.
 */
ObjArm ShatterOnHits(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y, byte, found_slot;
    bool marked, found;

    r.first = RD8(OBJ->state[slot]);
    if (r.first == 0) {
        r.arm = SO_LOAD;
        jsr_push(0xC0C4);
        r.second = 0;
        r.third = LoadObjectRecord(slot, &r.fourth, &r.flag);
        jsr_pop();
        WR8(OBJ->sprite_set[slot], 6);
        WR8(OBJ->frame[slot], 1);
        WR8(OBJ->vx_hi[slot], 4);
        return r;
    }
    r.second = RD8(OBJ->hit_taken[slot]);
    if (r.second >= 5) {
        jsr_push(0xC0DE);
        marked = RecordObjectEvent(slot, &r.third, &r.fourth, &r.fifth);
        jsr_pop();
        r.flag = marked;
        y = RD8(ZP->stock);
        r.sixth = y;
        if (y >= 4) {
            y = 4;
        }
        r.seventh = rd8((uint16_t)(0xC160 + y));
        WR8(OBJ->kind[slot], r.seventh);
        WR8(OBJ->state[slot], 0);
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->vy_hi[slot], 0xFB);
        WR8(OBJ->vy_lo[slot], 0);
        WR8(OBJ->timer[slot], 0x16);
        jsr_push(0xC10B);
        found_slot = FindFreeSlotFrom50(&r.ninth, &found);
        jsr_pop();
        r.eighth = found_slot;
        r.other = found;
        r.arm = SO_NO_SLOT;
        if (!found) {
            r.target = 0xC15F;
            return r;
        }
        r.tenth = StartObjectAtPlace(0x9B, slot, found_slot);
        r.arm = SO_STARTED;
        return r;
    }
    r.bytes[0] = RD8(OBJ->state[slot]);
    r.bytes[1] = (uint8_t)(r.bytes[0] - 1);
    r.arm = SO_OTHER_STATE;
    if (r.bytes[1] != 0) {
        r.target = 0xC135;
        return r;
    }
    jsr_push(0xC11A);
    r.sub = obj_sub(StepPartList(slot));
    jsr_pop();
    r.bytes[2] = rd8(ScrollXLo);
    r.target = (r.bytes[2] & 0x80) == 0 ? 0xC15F : 0xC11F;
    r.arm = SO_PART;
    return r;
}

static Jump shatter_on_hits_exit_shim(ObjArm r)
{
    Jump jump = { ended_kept(), 0, false };
    ObjArm rec = { 0 };
    Ended e;
    uint8_t y;

    if (r.arm == SO_LOAD) {
        rec.first = r.third;
        rec.second = r.fourth;
        rec.flag = r.flag;
        jump.end = then(record_exit_shim(rec), ended_load(4));
        return jump;
    }
    e = then(ended_load(r.first), then(ended_load(r.second), ended_compare(r.second, 5)));
    if (r.arm == SO_NO_SLOT || r.arm == SO_STARTED) {
        e = then(e, ended_of_object_event_stage(r.flag, r.third, r.fourth, r.fifth));
        y = r.sixth;
        e = then(e, ended_with_y(ended_nz(y), y));
        e = then(e, cpy(y, 4));
        if (y >= 4) {
            e = then(e, ended_with_y(ended_nz(4), 4));
        }
        e = then(e, ended_load(r.seventh));
        e = then(e, ended_load(0x16));
        e = then(e, ended_of_find_slot(r.eighth, r.ninth, r.other));
        if (r.arm == SO_NO_SLOT) {
            jump.end = e;
            jump.target = r.target;
            jump.jumps = true;
            return jump;
        }
        e = then(e, ended_load(0x9B));
        jump.end = then(e, ended_with_y(ended_load(r.tenth), r.eighth));
        return jump;
    }
    e = then(e, ended_load(r.bytes[0]));
    e = then(e, ended_load(r.bytes[1]));
    if (r.arm == SO_OTHER_STATE) {
        jump.end = e;
        jump.target = r.target;
        jump.jumps = true;
        return jump;
    }
    e = then(e, frame_step_exit_shim(obj_arm(r.sub)));
    e = then(e, ended_load(r.bytes[2]));
    jump.end = e;
    jump.target = r.target;
    jump.jumps = true;
    return jump;
}

void abi_ShatterOnHits(void)
{
    Jump jump = shatter_on_hits_exit_shim(ShatterOnHits(cpu.x));

    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
}

/*
 * @routine 0CAD:15F DoNothing
 * @run 71:C15F
 * WHAT IT DOES: Returns at once (a state handler that has nothing to do).
 * HOW: RTS.
 * WAITS: no.
 * Entry: nothing. Exit: every register and flag kept.
 */
void DoNothing(void)
{
}

void abi_DoNothing(void)
{
    DoNothing();
}

/*
 * @routine 0CAD:165 WatchVramForMark
 * @run 71:C165
 * WHAT IT DOES: In state 0 loads the object's record and sets its sprite set to 6 and its frame to 7. Each frame it reads the VDC's video RAM at $05FB and, unless the
 * word is a $43 byte, records the object's event and clears the object.
 * HOW: LoadObjectRecord, SelectVramReadAddress, two data writes, SelectVramData, two reads of the VDC's data port, RecordObjectEvent, ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: see the wrapper.
 */
ObjArm WatchVramForMark(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state;
    bool marked;

    r.arm = WV_KEPT;
    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(0xC16D);
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        jsr_pop();
        WR8(OBJ->sprite_set[slot], 6);
        WR8(OBJ->frame[slot], 7);
        r.other = true;
    }
    jsr_push(0xC17A);
    SelectVramReadAddress();
    jsr_pop();
    st1(0xFB);
    st2(0x05);
    jsr_push(0xC181);
    SelectVramData();
    jsr_pop();
    r.third = RD8(VDC->data_lo);
    r.fourth = RD8(VDC->data_hi);
    if (r.third == 0x43) {
        return r;
    }
    jsr_push(0xC18E);
    marked = RecordObjectEvent(slot, &r.fifth, &r.sixth, &r.seventh);
    jsr_pop();
    r.eighth = marked;
    ClearObject(slot);
    r.arm = WV_CLEARED;
    return r;
}

static VramWatch watch_vram_for_mark_exit_shim(ObjArm r)
{
    VramWatch w = { { 0 }, false, 0, 0, { 0 }, false, 0, 0 };

    if (r.other) {
        w.start = then(record_exit_shim(r), ended_load(7));
    }
    w.low = r.third;
    w.high = r.fourth;
    w.read = then(ended_load(r.third), ended_with_y(ended_kept(), r.fourth));
    w.read = then(w.read, ended_compare(r.third, 0x43));
    if (r.arm == WV_CLEARED) {
        w.event = ended_of_object_event_stage(r.eighth != 0, r.fifth, r.sixth, r.seventh);
        w.cleared = true;
    }
    return w;
}

void abi_WatchVramForMark(void)
{
    VramWatch r = watch_vram_for_mark_exit_shim(WatchVramForMark(cpu.x));

    abi_ended(r.start);
    abi_ended(r.read);
    if (r.cleared) {
        abi_ended(r.event);
    }
}

/*
 * @routine 0CAC:45C StepMoveOrClear
 * @run 70:BC5C
 * WHAT IT DOES: Steps the animation list at $BC6F; while it runs the object moves by its velocity, and when it ends the object is cleared.
 * HOW: StepObjectFrames, then a tail jump into ApplyVelocity or ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: running: as ApplyVelocity; ended: as the step.
 */
ObjArm StepMoveOrClear(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    r.step = step_list(slot, 0xBC6F, 0xBC67, &y, true);
    r.y = y;
    r.arm = SM_ENDED;
    if (r.step == FRAME_LIST_ENDED) {
        ClearObject(slot);
        return r;
    }
    r.move = ApplyVelocity(slot);
    r.arm = SM_MOVING;
    return r;
}

static Ended step_move_or_clear_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == SM_ENDED) {
        return e;
    }
    return then(e, ended_of_axis_move(r.move));
}

void abi_StepMoveOrClear(void)
{
    abi_ended(step_move_or_clear_exit_shim(StepMoveOrClear(cpu.x)));
}

/*
 * @routine 0CAC:57C StepCountClear
 * @run 70:BD7C
 * WHAT IT DOES: Steps the animation list at $BD51; counts the object's timer down and clears it at zero; until then its hitbox half height comes from a table at $BD78 by the
 * frame less $17.
 * HOW: StepObjectFrames, DEC, a table read (bank 70).
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: cleared: N and Z of the count; else A the table byte, Y the index, C and V from the subtract.
 */
ObjArm StepCountClear(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0, frame;

    r.step = step_list(slot, 0xBD51, 0xBD87, &y, false);
    r.y = y;
    r.first = DEC8(OBJ->life_timer[slot]);
    r.arm = CK_CLEARED;
    if (r.first == 0) {
        ClearObject(slot);
        return r;
    }
    frame = RD8(OBJ->frame[slot]);
    r.sum = sub_bytes(frame, 0x17, true);
    r.second = rd8((uint16_t)(0xBD78 + r.sum.value));
    WR8(OBJ->hitbox_half_h[slot], r.second);
    r.arm = CK_RUNNING;
    return r;
}

static Ended step_count_clear_exit_shim(ObjArm r)
{
    Ended e = then(frame_step_exit_shim(r), ended_nz(r.first));

    if (r.arm == CK_CLEARED) {
        return e;
    }
    return then(then(e, of_sum(r.sum)), ended_with_y(ended_load(r.second), r.sum.value));
}

void abi_StepCountClear(void)
{
    abi_ended(step_count_clear_exit_shim(StepCountClear(cpu.x)));
}

/*
 * @routine 0CAC:5B5 StartFallingSpark
 * @run 70:BDB5
 * WHAT IT DOES: Loads the object's record, gives OBJ_3510 a random byte, its angle low byte one of four from a table at $BDE1, a random X low byte, X high 0, Y $0100 and a
 * timer of $C0.
 * HOW: LoadObjectRecord, NextRandom three times, a table read (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: A $C0, N set, Z clear; Y the table index, C and V as the last random left them.
 */
ObjArm StartFallingSpark(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t byte;

    jsr_push(0xBDB8);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    jsr_push(0xBDBB);
    r.random = NextRandom();
    jsr_pop();
    WR8(OBJ->f3510[slot], r.random.value);
    jsr_push(0xBDC1);
    r.random2 = NextRandom();
    jsr_pop();
    r.third = (uint8_t)(r.random2.value & 0x03);
    byte = rd8((uint16_t)(0xBDE1 + r.third));
    WR8(OBJ->angle_lo[slot], byte);
    WR8(OBJ->x_hi[slot], 0);
    jsr_push(0xBDD0);
    r.random3 = NextRandom();
    jsr_pop();
    WR8(OBJ->x_lo[slot], r.random3.value);
    WR8(OBJ->y_hi[slot], 1);
    WR8(OBJ->y_lo[slot], 0);
    WR8(OBJ->life_timer[slot], 0xC0);
    return r;
}

static Ended start_falling_spark_exit_shim(ObjArm r)
{
    Ended e = then(record_exit_shim(r), ended_of_random(r.random));

    e = then(e, ended_of_random(r.random2));
    e = then(e, ended_with_y(ended_load(r.third), r.third));
    e = then(e, ended_of_random(r.random3));
    return then(e, ended_load(0xC0));
}

void abi_StartFallingSpark(void)
{
    abi_ended(start_falling_spark_exit_shim(StartFallingSpark(cpu.x)));
}

/*
 * @routine 0CAC:6DD StartFromTable
 * @run 70:BEDD
 * WHAT IT DOES: Loads the object's record, sets its angle low byte to $80 and its angle high byte from a table at $BEF2 by its next kind, less $A0.
 * HOW: LoadObjectRecord and a subtract.
 * WAITS: no.
 * Entry: X object. Exit: A the angle high byte, Y the next kind, C and V from the subtract.
 */
ObjArm StartFromTable(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t byte;

    jsr_push(0xBEE0);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    WR8(OBJ->angle_lo[slot], 0x80);
    r.third = RD8(OBJ->next_kind[slot]);
    byte = rd8((uint16_t)(0xBEF2 + r.third));
    r.sum = sub_bytes(byte, 0xA0, true);
    WR8(OBJ->angle_hi[slot], r.sum.value);
    return r;
}

static Ended start_from_table_exit_shim(ObjArm r)
{
    Ended e = then(record_exit_shim(r), ended_with_y(ended_nz(r.third), r.third));

    return then(e, of_sum(r.sum));
}

void abi_StartFromTable(void)
{
    abi_ended(start_from_table_exit_shim(StartFromTable(cpu.x)));
}

/*
 * @routine 0CAB:591 StartShotsUnlessThere
 * @run 70:B591
 * WHAT IT DOES: Unless slot $12's kind is $28, starts the three shots (StartThreeShots) and plays sound $2E; then goes on as ResetToStateThree.
 * HOW: A compare, StartThreeShots, SoundCommand, the next routine's bytes.
 * WAITS: no.
 * Entry: X object. Exit: as ResetToStateThree, or the compare's flags when the shots are already there.
 */
ObjArm StartShotsUnlessThere(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->kind[0x12]);
    r.arm = SU2_PRESENT;
    if (r.first == 0x28) {
        return r;
    }
    jsr_push(0xB59B);
    r.sub = obj_sub(StartThreeShots(slot));
    jsr_pop();
    r.sounded = true;
    r.command = 0x2E;
    jsr_push(0xB5A0);
    r.call = SoundCommand(0x2E, slot, 0x16);
    jsr_pop();
    r.sub2 = obj_sub(ResetToStateThree(slot));
    r.arm = SU2_STARTED;
    return r;
}

static SoundEnd start_shots_unless_there_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = ended_compare(r.first, 0x28);

    s.end = e;
    if (r.arm == SU2_PRESENT) {
        return s;
    }
    e = then(e, start_three_shots_exit_shim(obj_arm(r.sub)));
    s.before = then(e, ended_load(0x2E));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.end = reset_to_state_three_exit_shim(obj_arm(r.sub2));
    return s;
}

void abi_StartShotsUnlessThere(void)
{
    abi_sound_end(start_shots_unless_there_exit_shim(StartShotsUnlessThere(cpu.x)));
}

/*
 * @routine 0CAC:282 FollowAndFacePlayer
 * @run 70:BA82
 * WHAT IT DOES: In state 0 loads the object's record; later it sets its frame to 0 and, when a table at $BAAF by slot $10's frame says so, copies slot $10's flash flag, takes
 * its place and shows the frame $29 plus the angle to the player in thirty-seconds.
 * HOW: LoadObjectRecord (a tail jump), CopyObjectPlaceBack, AngleToPlayer, table read (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: state 0: as LoadObjectRecord; else the last load or add.
 */
ObjArm FollowAndFacePlayer(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state, flash;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        r.arm = FF_RECORD;
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        return r;
    }
    WR8(OBJ->frame[slot], 0);
    r.third = RD8(OBJ->frame[0x10]);
    r.fourth = rd8((uint16_t)(0xBAAF + r.third));
    r.arm = FF_NO_FLAG;
    if (r.fourth == 0) {
        return r;
    }
    flash = RD8(OBJ->flash_flags[0x10]);
    WR8(OBJ->flash_flags[slot], flash);
    r.fifth = flash;
    jsr_push(0xBAA0);
    r.sixth = CopyObjectPlaceBack(0x10, slot);
    jsr_pop();
    jsr_push(0xBAA3);
    r.angle = AngleToPlayer(slot);
    jsr_pop();
    r.seventh = (uint8_t)(r.angle.a >> 5);
    r.sum = add_bytes(r.seventh, 0x29, false);
    WR8(OBJ->frame[slot], r.sum.value);
    r.arm = FF_FACED;
    return r;
}

static Ended follow_and_face_player_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == FF_RECORD) {
        return record_exit_shim(r);
    }
    e = ended_with_y(ended_load(r.fourth), r.third);
    if (r.arm == FF_NO_FLAG) {
        return e;
    }
    e = then(e, ended_load(r.fifth));
    e = then(e, ended_with_y(ended_load(r.sixth), 0x10));
    e = then(e, ended_of_angle(r.angle));
    e = then(e, ended_with_carry(ended_load(r.seventh), (r.angle.a & 0x10) != 0));
    return then(e, of_sum(r.sum));
}

void abi_FollowAndFacePlayer(void)
{
    abi_ended(follow_and_face_player_exit_shim(FollowAndFacePlayer(cpu.x)));
}

/*
 * @routine 0CAC:315 StepOrClearByKind
 * @run 70:BB15
 * WHAT IT DOES: Steps the animation list that a table at $BB2D names for the object's timer-index (OBJ_34B8 doubled); when it ends the object is cleared.
 * HOW: A table read (bank 70), StepObjectFrames, a tail jump into ClearObject. (The part of the routine at $BAEF that comes after its table.)
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step.
 */
ObjArm StepOrClearByKind(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t phase, index, y;

    phase = RD8(OBJ->life_timer[slot]);
    index = (uint8_t)(phase << 1);
    y = index;
    WR8(ZP->temp10, rd8((uint16_t)(0xBB2D + index)));
    WR8(ZP->temp11, rd8((uint16_t)(0xBB2E + index)));
    jsr_push(0xBB27);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    if (r.step == FRAME_LIST_ENDED) {
        ClearObject(slot);
    }
    return r;
}

/* The step's exit, Y always taken as set (the list index is Y's entry here). */
static Ended step_or_clear_by_kind_exit_shim(ObjArm r)
{
    return ended_of_frame_step(r.step, true, r.y);
}

void abi_StepOrClearByKind(void)
{
    abi_ended(step_or_clear_by_kind_exit_shim(StepOrClearByKind(cpu.x)));
}

/*
 * @routine 0CAC:2EF StartSoundedPiece
 * @run 70:BAEF
 * @stop BB10
 * WHAT IT DOES: In state 0 loads the object's record, plays the sound a table at $BB12 names for its timer-index, takes an X speed from a table at $BB10 by its facing, moves
 * along X once and faces right; in any other state it goes on as StepOrClearByKind (the code after the table).
 * HOW: LoadObjectRecord, SoundCommand, ApplyVelocityX.
 * Untraced: the tables at $BB10 are data the run only read.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: see the wrapper.
 */
ObjArm StartSoundedPiece(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t state, byte;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        r.arm = PI_STEP;
        r.sub = obj_sub(StepOrClearByKind(slot));
        return r;
    }
    r.arm = PI_START;
    jsr_push(0xBAF7);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    r.third = RD8(OBJ->life_timer[slot]);
    r.command = rd8((uint16_t)(0xBB12 + r.third));
    r.sounded = true;
    jsr_push(0xBB00);
    r.call = SoundCommand(r.command, slot, r.third);
    jsr_pop();
    r.fourth = RD8(OBJ->facing_left[slot]);
    byte = rd8((uint16_t)(0xBB10 + r.fourth));
    WR8(OBJ->vx_hi[slot], byte);
    jsr_push(0xBB0C);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    WR8(OBJ->facing_left[slot], 0);
    (void)y;
    return r;
}

static SoundEnd start_sounded_piece_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };

    if (r.arm == PI_STEP) {
        s.end = step_or_clear_by_kind_exit_shim(obj_arm(r.sub));
        return s;
    }
    s.before = then(record_exit_shim(r), ended_with_y(ended_load(r.command), r.third));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.end = then(ended_with_y(ended_kept(), r.fourth), ended_of_axis_move(r.move));
    return s;
}

void abi_StartSoundedPiece(void)
{
    abi_sound_end(start_sounded_piece_exit_shim(StartSoundedPiece(cpu.x, cpu.y)));
}

/*
 * @routine 0CAC:492 StartThrownPiece
 * @run 70:BC92
 * WHAT IT DOES: Loads the object's record, turns on bit 7 of its hitbox flag, gives it an X speed from a table at $BCD1 by its facing and a Y speed of $18, moves it, then picks
 * a random length from a table at $BCD3 and sets its speed from the angle a table at $BCE3 names for its next kind (SetVelocityFromAngle); a left-facing object is reversed.
 * HOW: LoadObjectRecord, ApplyVelocity, NextRandom, SetVelocityFromAngle.
 * Untraced: the reversal (BCCD).
 * Curated cases: tools/rondo/curated/start_thrown_piece.txt (right-facing objects, the traced arm).
 * WAITS: no.
 * Entry: X object. Exit: the last load; C, V and Y as the angle scale left them.
 */
ObjArm StartThrownPiece(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t flags, facing, byte, pick, kind, angle;

    jsr_push(0xBC95);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    flags = RD8(OBJ->hitbox_on[slot]);
    r.third = (uint8_t)(flags | 0x80);
    WR8(OBJ->hitbox_on[slot], r.third);
    facing = RD8(OBJ->facing_left[slot]);
    byte = rd8((uint16_t)(0xBCD1 + facing));
    WR8(OBJ->vx_hi[slot], byte);
    WR8(OBJ->vy_hi[slot], 0x18);
    jsr_push(0xBCAE);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    jsr_push(0xBCB1);
    r.random = NextRandom();
    jsr_pop();
    pick = (uint8_t)(r.random.value & 0x07);
    r.fourth = (uint8_t)(pick << 1);
    byte = rd8((uint16_t)(0xBCD3 + r.fourth));
    WR8(ZP->temp00, byte);
    byte = rd8((uint16_t)(0xBCD4 + r.fourth));
    WR8(ZP->temp01, byte);
    kind = RD8(OBJ->next_kind[slot]);
    angle = rd8((uint16_t)(0xBCE3 + kind));
    jsr_push(0xBCC8);
    r.scale = SetVelocityFromAngle(angle, slot);
    jsr_pop();
    r.fifth = RD8(OBJ->facing_left[slot]);
    r.arm = TH_STRAIGHT;
    if (r.fifth != 0) {
        jsr_push(0xBCD0);
        r.sum = NegateVelocityX(slot);
        jsr_pop();
        r.arm = TH_REVERSED;
    }
    return r;
}

static Ended start_thrown_piece_exit_shim(ObjArm r)
{
    Ended e = then(record_exit_shim(r), ended_load(r.third));

    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_of_random(r.random));
    e = then(e, ended_with_y(ended_with_carry(ended_load(r.fourth), false), r.fourth));
    e = then(e, ended_of_scale(r.scale));
    e = then(e, ended_load(r.fifth));
    if (r.arm == TH_STRAIGHT) {
        return e;
    }
    return then(e, of_sum(r.sum));
}

void abi_StartThrownPiece(void)
{
    abi_ended(start_thrown_piece_exit_shim(StartThrownPiece(cpu.x)));
}

/* A random pick of a two-byte pair: the table's high byte at +1 and its low byte at +0 of the doubled index. */
static ObjArm pick_speed_pair(uint16_t table, uint16_t ret, uint16_t high_column, uint16_t low_column, uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t pick, byte;

    jsr_push(ret);
    r.random = NextRandom();
    jsr_pop();
    pick = (uint8_t)(r.random.value & 0x07);
    r.first = (uint8_t)(pick << 1);
    byte = rd8((uint16_t)(table + 1 + r.first));
    WR8_IDX(high_column, slot, byte);
    r.second = rd8((uint16_t)(table + r.first));
    WR8_IDX(low_column, slot, r.second);
    return r;
}

static Ended pick_speed_pair_exit_shim(ObjArm r)
{
    Ended e = then(ended_of_random(r.random), ended_with_y(ended_with_carry(ended_load(r.first), false), r.first));

    return then(e, ended_load(r.second));
}

/*
 * @routine 0CAC:3C1 StartScatteringPiece
 * @run 70:BBC1
 * WHAT IT DOES: In any state but 0 it goes on as StepMoveOrClear. In state 0 it loads its record, takes a random X and Y speed from a table at $BC1C (sixteen bytes: eight
 * words), moves once, then takes a second random X speed from $BC2C and Y speed from $BC3C.
 * HOW: LoadObjectRecord, NextRandom four times, ApplyVelocity, table reads (bank 70).
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: the last random's pick and table read.
 */
ObjArm StartScatteringPiece(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        r.arm = SC_STEP;
        r.sub = obj_sub(StepMoveOrClear(slot));
        return r;
    }
    r.arm = SC_START;
    jsr_push(0xBBCC);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    r.sub = obj_sub(pick_speed_pair(0xBC1C, 0xBBCF, COL(OBJ->vx_hi), COL(OBJ->vx_lo), slot));
    r.sub2 = obj_sub(pick_speed_pair(0xBC1C, 0xBBE2, COL(OBJ->vy_hi), COL(OBJ->vy_lo), slot));
    jsr_push(0xBBF5);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.sub3 = obj_sub(pick_speed_pair(0xBC2C, 0xBBF8, COL(OBJ->vx_hi), COL(OBJ->vx_lo), slot));
    r.sub4 = obj_sub(pick_speed_pair(0xBC3C, 0xBC0B, COL(OBJ->vy_hi), COL(OBJ->vy_lo), slot));
    return r;
}

static Ended start_scattering_piece_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == SC_STEP) {
        return step_move_or_clear_exit_shim(obj_arm(r.sub));
    }
    e = then(record_exit_shim(r), pick_speed_pair_exit_shim(obj_arm(r.sub)));
    e = then(e, pick_speed_pair_exit_shim(obj_arm(r.sub2)));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, pick_speed_pair_exit_shim(obj_arm(r.sub3)));
    return then(e, pick_speed_pair_exit_shim(obj_arm(r.sub4)));
}

void abi_StartScatteringPiece(void)
{
    abi_ended(start_scattering_piece_exit_shim(StartScatteringPiece(cpu.x)));
}

/*
 * @routine 0CAC:399 StepListAndMove
 * @run 70:BB99
 * WHAT IT DOES: Steps the animation list that a table at $BBAC names for the object's timer-index (doubled) and moves the object (the part of the routine at $BB4E after its table).
 * HOW: A table read (bank 70), StepObjectFrames, a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as ApplyVelocity, Y from the step.
 */
ObjArm StepListAndMove(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t phase, index, y;

    phase = RD8(OBJ->life_timer[slot]);
    index = (uint8_t)(phase << 1);
    y = index;
    WR8(ZP->temp10, rd8((uint16_t)(0xBBAC + index)));
    WR8(ZP->temp11, rd8((uint16_t)(0xBBAD + index)));
    jsr_push(0xBBA9);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.move = ApplyVelocity(slot);
    return r;
}

static Ended step_list_and_move_exit_shim(ObjArm r)
{
    return then(ended_of_frame_step(r.step, true, r.y), ended_of_axis_move(r.move));
}

void abi_StepListAndMove(void)
{
    abi_ended(step_list_and_move_exit_shim(StepListAndMove(cpu.x)));
}

/*
 * @routine 0CAC:34E StartSideDrifter
 * @run 70:BB4E
 * @stop BB89
 * WHAT IT DOES: In state 0 loads the object's record, takes an X speed from a table at $BB89 by its facing, moves along X once, takes a four-byte speed row from the table at $BB8B by its
 * timer-index and reverses its X speed when it faces left; in any other state it goes on as StepListAndMove.
 * HOW: LoadObjectRecord, ApplyVelocityX, NegateVelocityX, table reads (bank 70).
 * Untraced: the tables at $BB89 and $BB8B are data the run only read.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: state 0: the last load or negate; else as StepListAndMove.
 */
ObjArm StartSideDrifter(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state, byte, phase;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        r.arm = DF_STEP;
        r.sub = obj_sub(StepListAndMove(slot));
        return r;
    }
    jsr_push(0xBB56);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    r.third = RD8(OBJ->facing_left[slot]);
    r.fourth = rd8((uint16_t)(0xBB89 + r.third));
    WR8(OBJ->vx_hi[slot], r.fourth);
    jsr_push(0xBB62);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    phase = RD8(OBJ->life_timer[slot]);
    r.fifth = phase;
    r.sixth = (uint8_t)(phase << 2);
    byte = rd8((uint16_t)(0xBB8C + r.sixth));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xBB8B + r.sixth));
    WR8(OBJ->vx_lo[slot], byte);
    byte = rd8((uint16_t)(0xBB8E + r.sixth));
    WR8(OBJ->vy_hi[slot], byte);
    r.seventh = rd8((uint16_t)(0xBB8D + r.sixth));
    WR8(OBJ->vy_lo[slot], r.seventh);
    r.eighth = RD8(OBJ->facing_left[slot]);
    r.arm = DF_STRAIGHT;
    if (r.eighth != 0) {
        jsr_push(0xBB88);
        r.sum = NegateVelocityX(slot);
        jsr_pop();
        r.arm = DF_REVERSED;
    }
    return r;
}

static Ended start_side_drifter_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == DF_STEP) {
        return step_list_and_move_exit_shim(obj_arm(r.sub));
    }
    e = then(record_exit_shim(r), ended_with_y(ended_load(r.fourth), r.third));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_with_y(ended_with_carry(ended_load(r.sixth), (r.fifth & 0x40) != 0), r.sixth));
    e = then(e, ended_load(r.seventh));
    e = then(e, ended_load(r.eighth));
    if (r.arm == DF_STRAIGHT) {
        return e;
    }
    return then(e, of_sum(r.sum));
}

void abi_StartSideDrifter(void)
{
    abi_ended(start_side_drifter_exit_shim(StartSideDrifter(cpu.x)));
}
/*
 * @routine 0CAB:31C CountThenCue
 * @run 70:B31C
 * WHAT IT DOES: Counts the object's timer down; at zero it clears bits 3 and 4 of zField8E, plays the music again (CdPlayAgain), sends sound $BF, sets the angle word to
 * $8000 (OBJ_3568 $80, OBJ_35C0 0) and enters its next state.
 * HOW: DEC, two memory bit clears, CdPlayAgain, SoundCommand, INC.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
 */
ObjArm CountThenCue(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t flags;

    r.first = DEC8(OBJ->life_timer[slot]);
    r.arm = CQ_COUNTING;
    if (r.first != 0) {
        return r;
    }
    flags = RD8(ZP->field8_e);
    flags = (uint8_t)(flags & 0xF7);
    WR8(ZP->field8_e, flags);
    flags = RD8(ZP->field8_e);
    flags = (uint8_t)(flags & 0xEF);
    WR8(ZP->field8_e, flags);
    jsr_push(0xB328);
    r.track = CdPlayAgain(slot, y);
    jsr_pop();
    r.command = 0xBF;
    jsr_push(0xB32D);
    r.call = SoundCommand(0xBF, slot, y);
    jsr_pop();
    WR8(OBJ->angle_hi[slot], 0x80);
    WR8(OBJ->angle_lo[slot], 0);
    r.second = INC8(OBJ->state[slot]);
    r.arm = CQ_CUED;
    return r;
}

static CueEnd count_then_cue_exit_shim(ObjArm r)
{
    CueEnd c = { { 0 }, false, { 0 }, false, 0, { 0 }, { 0 } };

    c.pre = ended_nz(r.first);
    if (r.arm == CQ_COUNTING) {
        return c;
    }
    c.played = true;
    c.track = r.track;
    c.command = r.command;
    c.call = r.call;
    c.sounded = true;
    c.end = ended_nz(r.second);
    return c;
}

void abi_CountThenCue(void)
{
    CueEnd r = count_then_cue_exit_shim(CountThenCue(cpu.x, cpu.y));

    abi_ended(r.pre);
    if (r.played) {
        play_track_exit_shim(r.track);
    }
    if (r.sounded) {
        sound_command_exit_shim(r.command, r.call);
    }
    abi_ended(r.end);
}


/*
 * @routine 0CAC:4EB BounceSparkOffWalls
 * @run 70:BCEB
 * WHAT IT DOES: Steps the animation list at $BD51; an object not on the first Y page is cleared; its hitbox half height comes from a table at $BD78 by the frame less $17; it moves, $0008 is added to
 * its Y speed (gravity), and it asks the terrain: a wall (request $22, height 4) halves its X speed and reverses it (the shift of the 16-bit speed through the carry, then NegateVelocityX); else a floor
 * (request 9) stops its X speed, sets its timer to $40 and moves OBJ_2800 on.
 * HOW: StepObjectFrames, ApplyVelocity, ProbeTerrain twice, table read (bank 70), a tail jump into NegateVelocityX or ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see abi_chain.
 */
ExitChain BounceSparkOffWalls(uint8_t slot, bool carry_in)
{
    ExitChain c = { { { STEP_ENDED, { 0 }, { { 0 }, { 0 }, false, 0, { 0 } }, { { 0 }, { 0 }, false, { 0 } }, { 0 } } }, 0 };
    FrameStep step;
    AxisMove move;
    Probe probe;
    uint8_t y = 0, y_high, frame, byte, low, high, hit, speed;
    bool carry;
    Sum index, sum;
    Ended e;

    step = step_list(slot, 0xBD51, 0xBCF6, &y, false);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    y_high = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_load(y_high));
    if (y_high != 0) {
        ClearObject(slot);
        chain_ended(&c, e);
        return c;
    }
    frame = RD8(OBJ->frame[slot]);
    index = sub_bytes(frame, 0x17, true);
    y = index.value;
    e = then(then(e, of_sum(index)), ended_with_y(ended_kept(), y));
    byte = rd8((uint16_t)(0xBD78 + y));
    WR8(OBJ->hitbox_half_h[slot], byte);
    e = then(e, ended_load(byte));
    jsr_push(0xBD0E);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    low = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(low, 8, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    e = then(e, of_sum(sum));
    high = RD8(OBJ->vy_hi[slot]);
    sum = add_bytes(high, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    e = then(e, of_sum(sum));
    WR8(ZP->product.mid, 4);
    carry = e.c_set ? e.c : carry_in;
    chain_ended(&c, e);
    jsr_push(0xBD28);
    probe = ProbeTerrain(0x22, slot, y, carry);
    jsr_pop();
    chain_probe(&c, probe);
    carry = probe_carry_out(probe, carry);
    hit = (uint8_t)(probe.answer & 0x02);
    e = ended_load(hit);
    if (hit != 0) {
        speed = RD8(OBJ->vx_hi[slot]);
        e = then(e, ended_with_carry(ended_load((uint8_t)(speed << 1)), (speed & 0x80) != 0));
        carry = ROR8(OBJ->vx_hi[slot], (speed & 0x80) != 0);
        e = then(e, ended_with_carry(ended_kept(), carry));
        carry = ROR8(OBJ->vx_lo[slot], carry);
        e = then(e, ended_with_carry(ended_kept(), carry));
        chain_ended(&c, e);
        chain_ended(&c, of_sum(NegateVelocityX(slot)));
        return c;
    }
    chain_ended(&c, e);
    jsr_push(0xBD3E);
    probe = ProbeTerrain(9, slot, probe.column, carry);
    jsr_pop();
    chain_probe(&c, probe);
    hit = (uint8_t)(probe.answer & 0x01);
    e = ended_load(hit);
    if (hit != 0) {
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0);
        WR8(OBJ->life_timer[slot], 0x40);
        e = then(e, ended_load(0x40));
        hit = INC8(OBJ->state[slot]);
        e = then(e, ended_nz(hit));
    }
    chain_ended(&c, e);
    return c;
}

void abi_BounceSparkOffWalls(void)
{
    ExitChain c = BounceSparkOffWalls(cpu.x, (cpu.p & P_C) != 0);

    abi_chain(&c);
}


/* One axis of 0CAC:60A: the distance from object $16 plus 8, as a 16-bit subtract and add; far when it is $10 or more. */
static ObjArm axis_far(uint16_t lo_column, uint16_t hi_column, uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t mine, theirs, low, high;

    mine = RD8_IDX(lo_column, slot);
    theirs = RD8_IDX(lo_column, 0x16);
    r.sum = sub_bytes(mine, theirs, true);
    WR8(ZP->temp00, r.sum.value);
    mine = RD8_IDX(hi_column, slot);
    theirs = RD8_IDX(hi_column, 0x16);
    r.sum2 = sub_bytes(mine, theirs, r.sum.carry);
    WR8(ZP->temp01, r.sum2.value);
    low = RD8(ZP->temp00);
    r.sum3 = add_bytes(low, 8, false);
    WR8(ZP->temp00, r.sum3.value);
    high = RD8(ZP->temp01);
    r.sum4 = add_bytes(high, 0, r.sum3.carry);
    r.arm = AF_FAR;
    r.flag = true;
    if (r.sum4.value != 0) {
        return r;
    }
    r.first = RD8(ZP->temp00);
    r.arm = AF_COMPARED;
    r.flag = r.first >= 0x10;
    return r;
}

static Ended axis_far_exit_shim(ObjArm r)
{
    Ended e = then(then(of_sum(r.sum), of_sum(r.sum2)), then(of_sum(r.sum3), of_sum(r.sum4)));

    if (r.arm == AF_FAR) {
        return e;
    }
    return then(e, ended_compare(r.first, 0x10));
}

/*
 * @routine 0CAC:60A TrackPointOrLand
 * @run 70:BE0A
 * WHAT IT DOES: Puts object $16 at the place of object $10, one length $30 along the object's angle OBJ_3510, and moves it. If the object has landed
 * (OBJ_3618 set), or is within 8 of object $16 on both axes, it lands: frame 2, OBJ_3300 $12, flags OBJ_2EE0 and OBJ_3618 set, and object $16
 * is placed and moved again along the angle. Otherwise it takes the angle to object $16 and moves at the speed OBJ_35C0 along it.
 * HOW: CopyObjectPlaceBack, SetVelocityFromAngle, ApplyVelocity, AngleToPoint, 16-bit subtract and add.
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocity.
 */
ObjArm TrackPointOrLand(uint8_t slot)
{
    ObjArm r = { 0 };
    ObjArm x_axis, y_axis;
    uint8_t byte;
    bool far;

    r.first = RD8(OBJ->f3510[slot]);
    WR8(ZP->temp02, r.first);
    push8(slot);
    jsr_push(0xBE17);
    r.second = CopyObjectPlaceBack(0x10, 0x16);
    jsr_pop();
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 0x30);
    r.third = RD8(ZP->temp02);
    jsr_push(0xBE22);
    r.scale = SetVelocityFromAngle(r.third, 0x16);
    jsr_pop();
    jsr_push(0xBE25);
    r.move = ApplyVelocity(0x16);
    jsr_pop();
    slot = pull8();
    r.fourth = slot;
    r.fifth = RD8(OBJ->f3618[slot]);
    far = false;
    if (r.fifth == 0) {
        x_axis = axis_far(COL(OBJ->x_lo), COL(OBJ->x_hi), slot);
        r.sub = obj_sub(x_axis);
        far = x_axis.flag;
        if (!far) {
            y_axis = axis_far(COL(OBJ->y_lo), COL(OBJ->y_hi), slot);
            r.sub2 = obj_sub(y_axis);
            far = y_axis.flag;
        }
    }
    if (!far) {
        r.arm = TL_LANDED;
        WR8(OBJ->f3358[slot], 2);
        WR8(OBJ->f3300[slot], 0x12);
        WR8(OBJ->fields_2ee0[slot], 1);
        WR8(OBJ->f3618[slot], 1);
        jsr_push(0xBE8C);
        r.sixth = CopyObjectPlaceBack(0x10, slot);
        jsr_pop();
        WR8(ZP->temp00, 0);
        WR8(ZP->temp01, 0x30);
        r.seventh = RD8(OBJ->f3510[slot]);
        jsr_push(0xBE98);
        r.scale2 = SetVelocityFromAngle(r.seventh, slot);
        jsr_pop();
        jsr_push(0xBE9B);
        r.move2 = ApplyVelocity(slot);
        jsr_pop();
        return r;
    }
    r.arm = TL_TRACKED;
    byte = RD8(OBJ->x_hi[0x16]);
    WR8(ZP->temp09, byte);
    byte = RD8(OBJ->x_lo[0x16]);
    WR8(ZP->temp08, byte);
    byte = RD8(OBJ->y_hi[0x16]);
    WR8(ZP->temp0b, byte);
    r.sixth = RD8(OBJ->y_lo[0x16]);
    WR8(ZP->temp0a, r.sixth);
    jsr_push(0xBEB3);
    r.angle = AngleToPoint(slot);
    jsr_pop();
    push8(r.angle.a);
    WR8(ZP->temp00, 0);
    r.seventh = RD8(OBJ->angle_lo[slot]);
    WR8(ZP->temp01, r.seventh);
    r.eighth = pull8();
    jsr_push(0xBEBF);
    r.scale2 = SetVelocityFromAngle(r.eighth, slot);
    jsr_pop();
    jsr_push(0xBEC2);
    r.move2 = ApplyVelocity(slot);
    jsr_pop();
    return r;
}

static Ended track_point_or_land_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    e = then(e, ended_with_y(ended_load(r.second), 0x10));
    e = then(e, ended_load(r.third));
    e = then(e, ended_of_scale(r.scale));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_nz(r.fourth));
    e = then(e, ended_load(r.fifth));
    if (r.fifth == 0) {
        e = then(e, axis_far_exit_shim(obj_arm(r.sub)));
        if (!r.sub.flag) {
            e = then(e, axis_far_exit_shim(obj_arm(r.sub2)));
        }
    }
    if (r.arm == TL_LANDED) {
        e = then(e, ended_with_y(ended_load(r.sixth), 0x10));
        e = then(e, ended_load(r.seventh));
        e = then(e, ended_of_scale(r.scale2));
        return then(e, ended_of_axis_move(r.move2));
    }
    e = then(e, ended_load(r.sixth));
    e = then(e, ended_of_angle(r.angle));
    e = then(e, ended_load(r.seventh));
    e = then(e, ended_load(r.eighth));
    e = then(e, ended_of_scale(r.scale2));
    return then(e, ended_of_axis_move(r.move2));
}

void abi_TrackPointOrLand(void)
{
    abi_ended(track_point_or_land_exit_shim(TrackPointOrLand(cpu.x)));
}

/*
 * @routine 0CAC:5E5 TrackThenAimAtPlayer
 * @run 70:BDE5
 * WHAT IT DOES: Runs TrackPointOrLand, adds the speed OBJ_35C0 to the angle OBJ_3510 and counts the timer down; at zero it takes the angle to the player and
 * the speed OBJ_35C0 along it, and goes on to the next state.
 * HOW: TrackPointOrLand, AngleToPlayer, SetVelocityFromAngle.
 * WAITS: no.
 * Entry: X object. Exit: the timer's count, or the state's count after the step.
 */
ObjArm TrackThenAimAtPlayer(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xBDE8);
    r.sub = obj_sub(TrackPointOrLand(slot));
    jsr_pop();
    r.first = RD8(OBJ->f3510[slot]);
    r.sum = add_bytes(r.first, RD8(OBJ->angle_lo[slot]), false);
    WR8(OBJ->f3510[slot], r.sum.value);
    r.second = DEC8(OBJ->life_timer[slot]);
    r.arm = TT_COUNTING;
    if (r.second != 0) {
        return r;
    }
    jsr_push(0xBDFA);
    r.angle = AngleToPlayer(slot);
    jsr_pop();
    push8(r.angle.a);
    WR8(ZP->temp01, 0);
    r.third = RD8(OBJ->angle_lo[slot]);
    WR8(ZP->temp01, r.third);
    r.fourth = pull8();
    jsr_push(0xBE06);
    r.scale = SetVelocityFromAngle(r.fourth, slot);
    jsr_pop();
    r.fifth = INC8(OBJ->state[slot]);
    r.arm = TT_AIMED;
    return r;
}

static Ended track_then_aim_at_player_exit_shim(ObjArm r)
{
    Ended e = track_point_or_land_exit_shim(obj_arm(r.sub));

    e = then(e, ended_load(r.first));
    e = then(e, of_sum(r.sum));
    e = then(e, ended_nz(r.second));
    if (r.arm == TT_COUNTING) {
        return e;
    }
    e = then(e, ended_of_angle(r.angle));
    e = then(e, ended_load(r.third));
    e = then(e, ended_load(r.fourth));
    e = then(e, ended_of_scale(r.scale));
    return then(e, ended_nz(r.fifth));
}

void abi_TrackThenAimAtPlayer(void)
{
    abi_ended(track_then_aim_at_player_exit_shim(TrackThenAimAtPlayer(cpu.x)));
}

/*
 * @routine 0CAC:703 OrbitThenLeave
 * @run 70:BF03
 * WHAT IT DOES: Steps the animation list that a table at $BF66 names for the object's next kind, puts the object at X $0080, Y $0070 relative
 * to its screen and moves it along its angle OBJ_3568 by the length held in OBJ_3510 and the timer; then adds $80 to that 16-bit length,
 * turns the angle by one and, once the high byte of the length reaches $50, takes the speed from the turned-about angle with length $0400 and goes on to the next state.
 * HOW: A table read (bank 70), StepObjectFrames, SetVelocityFromAngle, ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: the state's count when it goes on, else the compare.
 */
ObjArm OrbitThenLeave(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t kind, y, byte;

    kind = RD8(OBJ->next_kind[slot]);
    y = (uint8_t)(kind << 1);
    r.first = y;
    r.flag = (kind & 0x80) != 0;
    byte = rd8((uint16_t)(0xBF66 + y));
    WR8(ZP->temp10, byte);
    byte = rd8((uint16_t)(0xBF67 + y));
    WR8(ZP->temp11, byte);
    jsr_push(0xBF15);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->x_lo[slot], 0x80);
    WR8(OBJ->y_lo[slot], 0x70);
    byte = RD8(OBJ->f3510[slot]);
    WR8(ZP->temp00, byte);
    byte = RD8(OBJ->life_timer[slot]);
    WR8(ZP->temp01, byte);
    r.second = RD8(OBJ->angle_hi[slot]);
    jsr_push(0xBF35);
    r.scale = SetVelocityFromAngle(r.second, slot);
    jsr_pop();
    jsr_push(0xBF38);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    byte = RD8(OBJ->f3510[slot]);
    r.sum = add_bytes(byte, 0x80, false);
    WR8(OBJ->f3510[slot], r.sum.value);
    byte = RD8(OBJ->life_timer[slot]);
    r.sum2 = add_bytes(byte, 0, r.sum.carry);
    WR8(OBJ->life_timer[slot], r.sum2.value);
    r.third = INC8(OBJ->angle_hi[slot]);
    r.fourth = RD8(OBJ->life_timer[slot]);
    r.arm = OL_ORBITING;
    if (r.fourth < 0x50) {
        return r;
    }
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 4);
    byte = RD8(OBJ->angle_hi[slot]);
    r.sum3 = add_bytes(byte, 0x80, false);
    jsr_push(0xBF62);
    r.scale2 = SetVelocityFromAngle(r.sum3.value, slot);
    jsr_pop();
    r.fifth = INC8(OBJ->state[slot]);
    r.arm = OL_LEFT;
    return r;
}

static Ended orbit_then_leave_exit_shim(ObjArm r)
{
    Ended e = ended_with_carry(ended_with_y(ended_load(r.first), r.first), r.flag);

    e = then(e, ended_of_frame_step(r.step, true, r.y));
    e = then(e, ended_load(r.second));
    e = then(e, ended_of_scale(r.scale));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, of_sum(r.sum));
    e = then(e, of_sum(r.sum2));
    e = then(e, ended_nz(r.third));
    e = then(e, ended_compare(r.fourth, 0x50));
    if (r.arm == OL_ORBITING) {
        return e;
    }
    e = then(e, of_sum(r.sum3));
    e = then(e, ended_of_scale(r.scale2));
    return then(e, ended_nz(r.fifth));
}

void abi_OrbitThenLeave(void)
{
    abi_ended(orbit_then_leave_exit_shim(OrbitThenLeave(cpu.x)));
}

/*
 * @routine 0CAB:339 SlideTurnByTable
 * @run 70:B339
 * WHAT IT DOES: Steps the animation list at $B3E0, sets the object's Y to $0070 and its Y speed to the cosine of the angle OBJ_3510 scaled by the 16-bit
 * length OBJ_3568:OBJ_35C0, moves it in Y, turns the angle half a circle, and runs the code a table at $B383 names for OBJ_2800 (SlideAngleDown,
 * SlideAngleUp, TurnAngleBack).
 * HOW: StepObjectFrames, ScaleByCosine, ApplyVelocityY, then a jump through the table.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; Y the doubled OBJ_2800.
 */
ObjArm SlideTurnByTable(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y, byte, low;

    r.step = step_list(slot, 0xB3E0, 0xB344, &y, false);
    r.y = y;
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0x70);
    byte = RD8(OBJ->angle_lo[slot]);
    WR8(ZP->temp00, byte);
    byte = RD8(OBJ->angle_hi[slot]);
    WR8(ZP->temp01, byte);
    r.first = RD8(OBJ->f3510[slot]);
    jsr_push(0xB35C);
    r.scale = ScaleByCosine(r.first);
    jsr_pop();
    byte = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], byte);
    r.second = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], r.second);
    jsr_push(0xB369);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    byte = RD8(OBJ->f3510[slot]);
    r.third = (uint8_t)(byte ^ 0x80);
    WR8(OBJ->f3510[slot], r.third);
    r.fourth = RD8(OBJ->f2800[slot]);
    r.fifth = (uint8_t)(r.fourth << 1);
    low = rd8((uint16_t)(0xB383 + r.fifth));
    WR8(ZP->temp00, low);
    r.sixth = rd8((uint16_t)(0xB384 + r.fifth));
    WR8(ZP->temp01, r.sixth);
    r.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return r;
}

/* What the step, the cosine, the move, the turn and the index left before the jump. */
static Ended slide_turn_by_table_before_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    e = then(e, ended_load(r.first));
    e = then(e, ended_of_scale(r.scale));
    e = then(e, ended_load(r.second));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_load(r.third));
    return then(e, ended_with_carry(ended_with_y(ended_load(r.fifth), r.fifth), (r.fourth & 0x80) != 0));
}

static Jump slide_turn_by_table_exit_shim(ObjArm r)
{
    Jump jump = { { 0 }, r.target, true };

    jump.end = ended_load(r.sixth);
    return jump;
}

void abi_SlideTurnByTable(void)
{
    ObjArm r = SlideTurnByTable(cpu.x);
    Jump jump = slide_turn_by_table_exit_shim(r);

    abi_ended(slide_turn_by_table_before_shim(r));
    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0CAB:457 MoveAlongAndSlow
 * @run 70:B457
 * WHAT IT DOES: Moves the object along its angle OBJ_3510 by the 16-bit length OBJ_3568:OBJ_35C0. Outside the screen box (InsideScreenBox) it clears OBJ_3670
 * and OBJ_2800 and stops. Inside, it takes a byte of a table at $B4A0 (picked by OBJ_3670 and 7) off the length, and when the length runs out
 * it sets the timer from a table at $B4A8 picked at random and goes on to the next OBJ_2800.
 * HOW: SetVelocityFromAngle, ApplyVelocity, InsideScreenBox, a 16-bit subtract, NextRandom, table reads (bank 70).
 * Untraced: the outside arm (B46F).
 * WAITS: no.
 * Entry: X object. Exit: the last count, subtract or load.
 */
ObjArm MoveAlongAndSlow(uint8_t slot)
{
    ObjArm r = { 0 };
    ObjArm view;
    uint8_t byte, pick;

    byte = RD8(OBJ->angle_lo[slot]);
    WR8(ZP->temp00, byte);
    byte = RD8(OBJ->angle_hi[slot]);
    WR8(ZP->temp01, byte);
    r.first = RD8(OBJ->f3510[slot]);
    jsr_push(0xB467);
    r.scale = SetVelocityFromAngle(r.first, slot);
    jsr_pop();
    jsr_push(0xB46A);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    jsr_push(0xB46D);
    view = InsideScreenBox(slot);
    jsr_pop();
    r.sub = obj_sub(view);
    r.arm = MA_OUTSIDE;
    if (!view.flag) {
        WR8(OBJ->f3670[slot], 0);
        WR8(OBJ->f2800[slot], 0);
        return r;
    }
    byte = RD8(OBJ->f3670[slot]);
    r.second = (uint8_t)(byte & 0x07);
    byte = RD8(OBJ->angle_lo[slot]);
    r.sum = sub_bytes(byte, rd8((uint16_t)(0xB4A0 + r.second)), true);
    WR8(OBJ->angle_lo[slot], r.sum.value);
    byte = RD8(OBJ->angle_hi[slot]);
    r.sum2 = sub_bytes(byte, 0, r.sum.carry);
    WR8(OBJ->angle_hi[slot], r.sum2.value);
    r.arm = MA_SLOWING;
    if (r.sum2.carry) {
        return r;
    }
    jsr_push(0xB493);
    r.random = NextRandom();
    jsr_pop();
    pick = (uint8_t)(r.random.value & 0x03);
    r.third = pick;
    r.fourth = rd8((uint16_t)(0xB4A8 + pick));
    WR8(OBJ->life_timer[slot], r.fourth);
    r.fifth = INC8(OBJ->f2800[slot]);
    r.arm = MA_NEXT;
    return r;
}

static Ended move_along_and_slow_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    e = then(e, ended_of_scale(r.scale));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, inside_screen_box_exit_shim(obj_arm(r.sub)).end);
    if (r.arm == MA_OUTSIDE) {
        return e;
    }
    e = then(e, ended_with_y(ended_load(r.second), r.second));
    e = then(e, of_sum(r.sum));
    e = then(e, of_sum(r.sum2));
    if (r.arm == MA_SLOWING) {
        return e;
    }
    e = then(e, ended_of_random(r.random));
    e = then(e, ended_with_y(ended_load(r.third), r.third));
    e = then(e, ended_load(r.fourth));
    return then(e, ended_nz(r.fifth));
}

void abi_MoveAlongAndSlow(void)
{
    abi_ended(move_along_and_slow_exit_shim(MoveAlongAndSlow(cpu.x)));
}

/*
 * @routine 0CAB:7CD LaunchThenFall
 * @run 70:B7CD
 * WHAT IT DOES: State 0: plays sound $C0, quarters OBJ_36C8, takes the angle to the player plus $80 as the object's angle with the length $0080 and goes on to the next
 * OBJ_2800. Later: when it has been hit five times (OBJ_33B0) it sets OBJ_2EE0 to 3, its state to 4 and OBJ_2800 to 9; else it moves and steps the animation list at $B829, and when the
 * list ends it sets its frame to 1, its state to 3 and OBJ_2800 to 0.
 * HOW: SoundCommand, LSR (twice), AngleToPlayer, SetVelocityFromAngle, ApplyVelocity, StepObjectFrames.
 * Untraced: the hit arm (B7FB to B80A).
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see abi_sound_end.
 */
ObjArm LaunchThenFall(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y, byte;

    r.first = RD8(OBJ->f2800[slot]);
    if (r.first == 0) {
        r.arm = LF_START;
        r.sounded = true;
        r.command = 0xC0;
        jsr_push(0xB7D7);
        r.call = SoundCommand(0xC0, slot, y_entry);
        jsr_pop();
        r.second = LSR8_VALUE(OBJ->f36c8[slot], r.flag);
        r.second = LSR8_VALUE(OBJ->f36c8[slot], r.flag);
        jsr_push(0xB7E0);
        r.angle = AngleToPlayer(slot);
        jsr_pop();
        r.sum = add_bytes(r.angle.a, 0x80, false);
        push8(r.sum.value);
        WR8(ZP->temp00, 0x80);
        WR8(ZP->temp01, 0);
        r.third = pull8();
        jsr_push(0xB7F0);
        r.scale = SetVelocityFromAngle(r.third, slot);
        jsr_pop();
        r.fourth = INC8(OBJ->f2800[slot]);
        return r;
    }
    r.second = RD8(OBJ->hit_taken[slot]);
    if (r.second >= 5) {
        r.arm = LF_HIT;
        WR8(OBJ->fields_2ee0[slot], 3);
        WR8(OBJ->state[slot], 4);
        WR8(OBJ->f2800[slot], 9);
        return r;
    }
    jsr_push(0xB80E);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.step = step_list(slot, 0xB829, 0xB819, &y, false);
    r.y = y;
    r.arm = LF_STEPPING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->frame[slot], 1);
    WR8(OBJ->state[slot], 3);
    WR8(OBJ->f2800[slot], 0);
    r.arm = LF_RESTED;
    (void)byte;
    return r;
}

static SoundEnd launch_then_fall_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = ended_load(r.first);

    if (r.arm == LF_START) {
        s.before = then(e, ended_load(0xC0));
        s.sounded = true;
        s.command = r.command;
        s.call = r.call;
        e = ended_with_carry(ended_nz(r.second), r.flag);
        e = then(e, ended_of_angle(r.angle));
        e = then(e, of_sum(r.sum));
        e = then(e, ended_load(r.third));
        e = then(e, ended_of_scale(r.scale));
        s.end = then(e, ended_nz(r.fourth));
        return s;
    }
    e = then(e, ended_compare(r.second, 5));
    if (r.arm == LF_HIT) {
        s.end = then(e, ended_load(9));
        return s;
    }
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, frame_step_exit_shim(r));
    if (r.arm == LF_STEPPING) {
        s.end = e;
        return s;
    }
    s.end = then(e, ended_load(3));
    return s;
}

void abi_LaunchThenFall(void)
{
    abi_sound_end(launch_then_fall_exit_shim(LaunchThenFall(cpu.x, cpu.y)));
}

/*
 * @routine 0CAB:147 StartTileScriptByKind
 * @run 70:B147
 * WHAT IT DOES: Loads the object's record and sets its timer from a table at $B15D by its next kind; for kind 0 it draws the tile script 6 of group 0 (DrawTileScript).
 * HOW: LoadObjectRecord, a table read (bank 70), DrawTileScript.
 * WAITS: no.
 * Entry: X object. Exit: kind 0: as DrawTileScript's; else the compare with 0.
 */
ObjArm StartTileScriptByKind(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB14A);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    r.third = RD8(OBJ->next_kind[slot]);
    r.fourth = rd8((uint16_t)(0xB15D + r.third));
    WR8(OBJ->life_timer[slot], r.fourth);
    r.arm = TK_OTHER;
    if (r.third != 0) {
        return r;
    }
    jsr_push(0xB15C);
    r.run = DrawTileScript(6, 0);
    jsr_pop();
    r.arm = TK_DRAWN;
    return r;
}

static Ended start_tile_script_by_kind_exit_shim(ObjArm r)
{
    Ended e = then(record_exit_shim(r), ended_with_y(ended_nz(r.third), r.third));

    e = then(e, ended_load(r.fourth));
    e = then(e, cpy(r.third, 0));
    if (r.arm == TK_OTHER) {
        return e;
    }
    e = then(e, ended_load(6));
    return then(e, ended_of_tile_run(r.run, 6));
}

void abi_StartTileScriptByKind(void)
{
    abi_ended(start_tile_script_by_kind_exit_shim(StartTileScriptByKind(cpu.x)));
}

/*
 * @routine 0CAB:161 CountThenTileScript
 * @run 70:B161
 * WHAT IT DOES: Counts the object's timer down; at zero it draws the tile script of group 0 numbered the next kind plus 7 (DrawTileScript) and goes on to the next state.
 * HOW: DEC, DrawTileScript, INC.
 * WAITS: no.
 * Entry: X object. Exit: the count; at zero, the state's count.
 */
ObjArm CountThenTileScript(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = DEC8(OBJ->life_timer[slot]);
    r.arm = CS_COUNTING;
    if (r.first != 0) {
        return r;
    }
    r.second = RD8(OBJ->next_kind[slot]);
    r.sum = add_bytes(r.second, 6, false);
    r.third = (uint8_t)(r.sum.value + 1);
    jsr_push(0xB171);
    r.run = DrawTileScript(r.third, 0);
    jsr_pop();
    r.fourth = INC8(OBJ->state[slot]);
    r.arm = CS_DRAWN;
    return r;
}

static Ended count_then_tile_script_exit_shim(ObjArm r)
{
    Ended e = ended_nz(r.first);

    if (r.arm == CS_COUNTING) {
        return e;
    }
    e = then(e, ended_load(r.second));
    e = then(e, of_sum(r.sum));
    e = then(e, ended_nz(r.third));
    e = then(e, ended_with_y(ended_kept(), 0));
    e = then(e, ended_of_tile_run(r.run, r.third));
    return then(e, ended_nz(r.fourth));
}

void abi_CountThenTileScript(void)
{
    abi_ended(count_then_tile_script_exit_shim(CountThenTileScript(cpu.x)));
}

/* The jump of a table handler: A the table's high byte, Y the doubled selector, C its bit 7. */
static Jump jump_table_exit_shim(ObjArm r)
{
    Jump jump = { { 0 }, r.target, true };

    jump.end = ended_with_carry(ended_with_y(ended_load(r.first), r.second), (r.third & 0x80) != 0);
    return jump;
}

/*
 * @routine 0CAB:2E2 JumpByStateTableD
 * @run 70:B2E2
 * WHAT IT DOES: Goes to the code a table at $B2F4 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
ObjArm JumpByStateTableD(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t low;

    r.third = RD8(OBJ->state[slot]);
    r.second = (uint8_t)(r.third << 1);
    low = rd8((uint16_t)(0xB2F4 + r.second));
    WR8(ZP->temp00, low);
    r.first = rd8((uint16_t)(0xB2F5 + r.second));
    WR8(ZP->temp01, r.first);
    r.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return r;
}

static Jump jump_by_state_table_d_exit_shim(ObjArm r)
{
    return jump_table_exit_shim(r);
}

void abi_JumpByStateTableD(void)
{
    Jump jump = jump_by_state_table_d_exit_shim(JumpByStateTableD(cpu.x));

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0CAB:40A JumpByPhaseTable
 * @run 70:B40A
 * WHAT IT DOES: Goes to the code a table at $B41C names for OBJ_2800.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
 */
ObjArm JumpByPhaseTable(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t low;

    r.third = RD8(OBJ->f2800[slot]);
    r.second = (uint8_t)(r.third << 1);
    low = rd8((uint16_t)(0xB41C + r.second));
    WR8(ZP->temp00, low);
    r.first = rd8((uint16_t)(0xB41D + r.second));
    WR8(ZP->temp01, r.first);
    r.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return r;
}

static Jump jump_by_phase_table_exit_shim(ObjArm r)
{
    return jump_table_exit_shim(r);
}

void abi_JumpByPhaseTable(void)
{
    Jump jump = jump_by_phase_table_exit_shim(JumpByPhaseTable(cpu.x));

    jump_shim(jump.end, jump.target);
}

/* SetHitboxFromFrame's exit, as its wrapper leaves it. */
static Ended ended_of_frame_lookup(FrameLookup r)
{
    Ended e;

    if (r.none) {
        e = ended_load(0);
        if (!r.frame_zero) {
            e = ended_with_overflow(ended_with_carry(ended_with_y(e, r.frame_index), true), r.overflow);
        }
        return e;
    }
    e = ended_with_carry(ended_with_y(ended_load(r.last), r.table_offset), r.carry);
    return ended_with_overflow(e, r.overflow);
}

/* EnterStateFiveOnPlayerState9's exit, as its wrapper leaves it. */
static Ended ended_of_five(FiveResult r)
{
    switch (r.end) {
    case FIVE_PLAYER_NOT_9:
        return ended_with_carry(ended_with_a(ended_nz((uint8_t)(r.value - 9)), r.value), r.value >= 9);
    case FIVE_NOT_STATE_3:
        return ended_with_carry(ended_with_a(ended_nz((uint8_t)(r.value - 3)), r.value), r.value >= 3);
    default:
        return ended_with_carry(ended_load(5), true);
    }
}

/* What StepWithGuardTable's instructions have left once the jump's flags are in. */
static Ended step_with_guard_table_chain(GuardArm g)
{
    ObjArm r = g.arm;
    Ended e = ended_nz(r.first);

    e = then(e, start_when_hurt_exit_shim(obj_arm(r.sub)));
    e = then(e, enter_state_seven_on_flag_exit_shim(obj_arm(r.sub2)));
    e = then(e, ended_of_five(g.five));
    e = then(e, ended_of_frame_lookup(g.lookup));
    return then(e, jump_by_state_table_d_exit_shim(obj_arm(r.sub3)).end);
}

/* The registers the handler is entered with. */
static void step_with_guard_table_enter_shim(GuardArm g)
{
    abi_ended(step_with_guard_table_chain(g));
}

/*
 * @routine 0CAB:18E StepWithGuardTable
 * @run 70:B18E
 * WHAT IT DOES: One frame of an object with the guard states: OBJ_3618 goes up, StartWhenHurt, EnterStateSevenOnFlag, EnterStateFiveOnPlayerState9 and SetHitboxFromFrame run, then
 * the code the state table at $B2F4 names (JumpByStateTableD); afterwards a sound list from a table at $B1B3 by the frame, when not 0, starts on channel 5 (StartListChannelKind2, list plus $81).
 * HOW: INC, five calls, a call through the state table, a table read (bank 70), StartListChannelKind2.
 * WAITS: no.
 * Entry: X object. Exit: the list start's, or the table load's.
 */
GuardArm StepWithGuardTable(uint8_t slot)
{
    GuardArm g = { { 0 } };
    ObjArm *r = &g.arm;

    r->first = INC8(OBJ->f3618[slot]);
    jsr_push(0xB194);
    r->sub = obj_sub(StartWhenHurt(slot));
    jsr_pop();
    jsr_push(0xB197);
    r->sub2 = obj_sub(EnterStateSevenOnFlag(slot));
    jsr_pop();
    jsr_push(0xB19A);
    g.five = EnterStateFiveOnPlayerState9(slot);
    jsr_pop();
    jsr_push(0xB19D);
    g.lookup = SetHitboxFromFrame(slot);
    jsr_pop();
    jsr_push(0xB1A0);
    r->sub3 = obj_sub(JumpByStateTableD(slot));
    step_with_guard_table_enter_shim(g);
    slot = enter_handler(slot, jump_by_state_table_d_exit_shim(obj_arm(r->sub3)));
    jsr_pop();
    r->second = RD8(OBJ->frame[slot]);
    r->third = rd8((uint16_t)(0xB1B3 + r->second));
    r->arm = SG_NO_LIST;
    if (r->third == 0) {
        return g;
    }
    push8(slot);
    r->sum = add_bytes(r->third, 0x81, false);
    jsr_push(0xB1B1);
    r->fourth = StartListChannelKind2(r->sum.value, 5);
    jsr_pop();
    r->fifth = pull8();
    r->arm = SG_LIST;
    return g;
}

/* The flags went to the handler and came back: what the typed code adds after it. */
static Ended step_with_guard_table_exit_shim(GuardArm g)
{
    ObjArm r = g.arm;
    Ended e = ended_with_y(ended_load(r.third), r.second);

    if (r.arm == SG_NO_LIST) {
        return e;
    }
    e = then(e, of_sum(r.sum));
    e = then(e, ended_load(r.fourth));
    return then(e, ended_nz(r.fifth));
}

void abi_StepWithGuardTable(void)
{
    abi_ended(step_with_guard_table_exit_shim(StepWithGuardTable(cpu.x)));
}

/*
 * @routine 0CAB:3E5 StepPhaseThenPause
 * @run 70:B3E5
 * WHAT IT DOES: Runs the code OBJ_2800 names (JumpByPhaseTable); afterwards a pause counter OBJ_36C8 counts down, and at zero it is set from a table at $B406 picked at random, the state is 4
 * and OBJ_2800 is 0.
 * HOW: JumpByPhaseTable, NextRandom, a table read (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: the count; at zero, the OBJ_2800 clear.
 */
ObjArm StepPhaseThenPause(uint8_t slot)
{
    ObjArm r = { 0 };
    ObjArm table;

    jsr_push(0xB3E8);
    table = JumpByPhaseTable(slot);
    slot = enter_handler(slot, jump_by_phase_table_exit_shim(table));
    jsr_pop();
    r.first = RD8(OBJ->f36c8[slot]);
    r.arm = PP_COUNTING;
    if (r.first != 0) {
        r.second = DEC8(OBJ->f36c8[slot]);
        return r;
    }
    jsr_push(0xB3F4);
    r.random = NextRandom();
    jsr_pop();
    r.third = (uint8_t)(r.random.value & 0x03);
    r.fourth = rd8((uint16_t)(0xB406 + r.third));
    WR8(OBJ->f36c8[slot], r.fourth);
    WR8(OBJ->state[slot], 4);
    WR8(OBJ->f2800[slot], 0);
    r.arm = PP_REPICKED;
    return r;
}

static Ended step_phase_then_pause_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == PP_COUNTING) {
        return then(e, ended_nz(r.second));
    }
    e = then(e, ended_of_random(r.random));
    e = then(e, ended_with_y(ended_load(r.third), r.third));
    e = then(e, ended_load(r.fourth));
    return then(e, ended_load(4));
}

void abi_StepPhaseThenPause(void)
{
    abi_ended(step_phase_then_pause_exit_shim(StepPhaseThenPause(cpu.x)));
}
