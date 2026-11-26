/*
 * TEMPORARY: the stage 0841 object routines of lot L6 (enemy kinds not yet identified), by routine. Bodies return what they ended on
 * (ObjArm, enemy_objects.h); each routine's exit shim builds A, Y and the flags from it and its abi_ wrapper applies them.
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

/* The arms the routines below end on. */
enum { S8_NS_WAITING, S8_NS_NEXT };
enum { S8_CN_COUNTING, S8_CN_NEXT };
enum { S8_SF_COUNTING, S8_SF_NEXT };
enum { S8_FL_RECORD, S8_FL_FALLING };
enum { S8_FB_WAITING, S8_FB_NEXT };
enum { S8_AC_HIGH, S8_AC_LOW, S8_AC_NEXT };
enum { S8_WF_COUNTING, S8_WF_WAITING, S8_WF_STARTED };
enum { S8_FT_FALLING, S8_FT_PEAK };
enum { S8_FS_RECORD, S8_FS_FOLLOWED };
enum { S8_DC_SKIP, S8_DC_DONE };
enum { S8_SP_ENDED, S8_SP_HELD, S8_SP_FRAME, S8_SP_NO_SLOT, S8_SP_SPAWNED };
enum { S8_CP_NEAR, S8_CP_FAR, S8_CP_STEPPING, S8_CP_ENDED };
enum { S8_NP_STATE11_X, S8_NP_STATE11_Y, S8_NP_PLACED };

/*
 * @routine 088B:6C5 ClearFrame
 * @run 70:B6C5
 * WHAT IT DOES: Sets the object's frame to 0.
 * HOW: A store.
 * WAITS: no.
 * Entry: X object. Exit: all kept.
 */
ObjArm ClearFrame(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(OBJ->frame[slot], 0);
    return r;
}

static Ended clear_frame_exit_shim(ObjArm r)
{
    (void)r;
    return ended_kept();
}

void abi_ClearFrame(void)
{
    abi_ended(clear_frame_exit_shim(ClearFrame(cpu.x)));
}

/*
 * @routine 088B:21D TurnAroundInState9
 * @run 70:B21D
 * WHAT IT DOES: Sets the object's state to 9 and reverses its X speed (a tail jump into NegateVelocityX).
 * HOW: A store, NegateVelocityX.
 * WAITS: no.
 * Entry: X object. Exit: as NegateVelocityX.
 */
ObjArm TurnAroundInState9(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(OBJ->state[slot], 9);
    r.sum = NegateVelocityX(slot);
    return r;
}

static Ended turn_around_in_state9_exit_shim(ObjArm r)
{
    return then(ended_load(9), ended_of_sum(r.sum));
}

void abi_TurnAroundInState9(void)
{
    abi_ended(turn_around_in_state9_exit_shim(TurnAroundInState9(cpu.x)));
}

/*
 * @routine 088B:225 FallBackInState9
 * @run 70:B225
 * WHAT IT DOES: Sets the object's state to 9 and reverses its Y speed (a tail jump into NegateVelocityY).
 * HOW: A store, NegateVelocityY.
 * WAITS: no.
 * Entry: X object. Exit: as NegateVelocityY.
 */
ObjArm FallBackInState9(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(OBJ->state[slot], 9);
    r.sum = NegateVelocityY(slot);
    return r;
}

static Ended fall_back_in_state9_exit_shim(ObjArm r)
{
    return then(ended_load(9), ended_of_sum(r.sum));
}

void abi_FallBackInState9(void)
{
    abi_ended(fall_back_in_state9_exit_shim(FallBackInState9(cpu.x)));
}

/*
 * @routine 088A:71E PlaySoundTwo
 * @run 70:AF1E
 * WHAT IT DOES: Plays sound 2.
 * HOW: SoundCommand.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: see abi_sound_end.
 */
ObjArm PlaySoundTwo(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };

    r.sounded = true;
    r.command = 2;
    jsr_push(0xAF23);
    r.call = SoundCommand(2, slot, y_entry);
    jsr_pop();
    return r;
}

static SoundEnd play_sound_two_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, true, 2, { 0 } };

    s.before = ended_load(2);
    s.call = r.call;
    s.end = ended_kept();
    return s;
}

void abi_PlaySoundTwo(void)
{
    abi_sound_end(play_sound_two_exit_shim(PlaySoundTwo(cpu.x, cpu.y)));
}

/*
 * @routine 088A:31C LoadThenSpeedF0
 * @run 70:AB1C
 * WHAT IT DOES: Loads the object's record and sets its X speed high to $F0.
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: A $F0, N set, Z clear; C and Y from the record.
 */
ObjArm LoadThenSpeedF0(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xAB1F);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    WR8(OBJ->vx_hi[slot], 0xF0);
    return r;
}

static Ended load_then_speed_f0_exit_shim(ObjArm r)
{
    return then(record_exit_shim(r), ended_load(0xF0));
}

void abi_LoadThenSpeedF0(void)
{
    abi_ended(load_then_speed_f0_exit_shim(LoadThenSpeedF0(cpu.x)));
}

/*
 * @routine 088A:325 NextStateOnScrollY
 * @run 70:AB25
 * WHAT IT DOES: Goes on to the next state when the scroll's Y high byte is not zero.
 * HOW: A read and an INC.
 * WAITS: no.
 * Entry: X object. Exit: the load, or the new state's N and Z.
 */
ObjArm NextStateOnScrollY(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(SCROLL->y_hi[0]);
    r.arm = S8_NS_WAITING;
    if (r.first == 0) {
        return r;
    }
    r.second = INC8(OBJ->state[slot]);
    r.arm = S8_NS_NEXT;
    return r;
}

static Ended next_state_on_scroll_y_exit_shim(ObjArm r)
{
    if (r.arm == S8_NS_WAITING) {
        return ended_load(r.first);
    }
    return then(ended_load(r.first), ended_nz(r.second));
}

void abi_NextStateOnScrollY(void)
{
    abi_ended(next_state_on_scroll_y_exit_shim(NextStateOnScrollY(cpu.x)));
}

/*
 * @routine 088A:765 StepListByIndex088A
 * @run 70:AF65
 * WHAT IT DOES: Steps the animation list whose address the table at $AF72 gives for the index in Y.
 * HOW: Two table reads (bank 70), a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y the table index. Exit: as StepObjectFrames.
 */
ObjArm StepListByIndex088A(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, rd8((uint16_t)(0xAF72 + y)));
    WR8(ZP->temp11, rd8((uint16_t)(0xAF73 + y)));
    r.step = StepObjectFrames(slot, &y);
    r.y = y;
    return r;
}

void abi_StepListByIndex088A(void)
{
    abi_ended(frame_step_exit_shim(StepListByIndex088A(cpu.x, cpu.y)));
}

/*
 * @routine 088B:777 SetTimersOf12To15
 * @run 70:B777
 * WHAT IT DOES: Sets OBJ_34B8 of the objects 18 to 21 to 1.
 * HOW: A loop of Y from 18.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, Y 22, the compare of Y with 22.
 */
ObjArm SetTimersOf12To15(void)
{
    ObjArm r = { 0 };
    uint8_t y;

    for (y = 0x12; y < 0x16; y++) {
        WR8(OBJ->f34b8[y], 1);
    }
    return r;
}

static Ended set_timers_of12_to15_exit_shim(ObjArm r)
{
    (void)r;
    return then(ended_load(1), ended_with_y(ended_with_carry(ended_nz(0), true), 0x16));
}

void abi_SetTimersOf12To15(void)
{
    abi_ended(set_timers_of12_to15_exit_shim(SetTimersOf12To15()));
}

/*
 * @routine 088A:52D CountThenNextStateF3510
 * @run 70:AD2D
 * WHAT IT DOES: Counts OBJ_3510 down; at zero it sets it to $30 and goes on to the next state.
 * HOW: DEC, a store, INC.
 * WAITS: no.
 * Entry: X object. Exit: the count, or the new state's N and Z (A $30).
 */
ObjArm CountThenNextStateF3510(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = DEC8(OBJ->f3510[slot]);
    r.arm = S8_CN_COUNTING;
    if (r.first != 0) {
        return r;
    }
    WR8(OBJ->f3510[slot], 0x30);
    r.second = INC8(OBJ->state[slot]);
    r.arm = S8_CN_NEXT;
    return r;
}

static Ended count_then_next_state_f3510_exit_shim(ObjArm r)
{
    Ended e = ended_nz(r.first);

    if (r.arm == S8_CN_COUNTING) {
        return e;
    }
    return then(then(e, ended_load(0x30)), ended_nz(r.second));
}

void abi_CountThenNextStateF3510(void)
{
    abi_ended(count_then_next_state_f3510_exit_shim(CountThenNextStateF3510(cpu.x)));
}

/*
 * @routine 088A:724 SetFrameThenCountToNext
 * @run 70:AF24
 * WHAT IT DOES: Sets the object's frame (OBJ_2A68) to $0C and counts OBJ_3568 down; at zero it sets it to $10 and goes on to the next state.
 * HOW: A store, DEC, a store, INC.
 * WAITS: no.
 * Entry: X object. Exit: the count, or the new state's N and Z (A $10).
 */
ObjArm SetFrameThenCountToNext(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(OBJ->frame[slot], 0x0C);
    r.first = DEC8(OBJ->f3568[slot]);
    r.arm = S8_SF_COUNTING;
    if (r.first != 0) {
        return r;
    }
    WR8(OBJ->f3568[slot], 0x10);
    r.second = INC8(OBJ->state[slot]);
    r.arm = S8_SF_NEXT;
    return r;
}

static Ended set_frame_then_count_to_next_exit_shim(ObjArm r)
{
    Ended e = then(ended_load(0x0C), ended_nz(r.first));

    if (r.arm == S8_SF_COUNTING) {
        return e;
    }
    return then(then(e, ended_load(0x10)), ended_nz(r.second));
}

void abi_SetFrameThenCountToNext(void)
{
    abi_ended(set_frame_then_count_to_next_exit_shim(SetFrameThenCountToNext(cpu.x)));
}

/*
 * @routine 088A:41E StartListSoundThenSpawn20
 * @run 70:AC1E
 * WHAT IT DOES: Starts the script list $83 on channel 4, starts a kind $20 object in slot $10 at this object's place and clears this object.
 * HOW: StartListChannelKind2, StartObjectAtPlace, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: as ClearObject.
 */
ObjArm StartListSoundThenSpawn20(uint8_t slot)
{
    ObjArm r = { 0 };

    push8(slot);
    jsr_push(0xAC26);
    StartListChannelKind2(0x83, 4);
    jsr_pop();
    slot = pull8();
    jsr_push(0xAC2E);
    r.first = StartObjectAtPlace(0x20, slot, 0x10);
    jsr_pop();
    ClearObject(slot);
    return r;
}

static Ended start_list_sound_then_spawn20_exit_shim(ObjArm r)
{
    return ended_with_y(ended_load(r.first), 0x10);
}

void abi_StartListSoundThenSpawn20(void)
{
    abi_ended(start_list_sound_then_spawn20_exit_shim(StartListSoundThenSpawn20(cpu.x)));
}

/*
 * @routine 088B:084 StartFallWithFrame21
 * @run 70:B084
 * WHAT IT DOES: Zeroes the object's X speed, sets its Y speed high to $FD and its frame to $21 and goes on to the next OBJ_2800.
 * HOW: Stores and an INC.
 * WAITS: no.
 * Entry: X object. Exit: A $21, N and Z from the new OBJ_2800.
 */
ObjArm StartFallWithFrame21(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0xFD);
    WR8(OBJ->frame[slot], 0x21);
    r.first = INC8(OBJ->f2800[slot]);
    return r;
}

static Ended start_fall_with_frame21_exit_shim(ObjArm r)
{
    return then(ended_load(0x21), ended_nz(r.first));
}

void abi_StartFallWithFrame21(void)
{
    abi_ended(start_fall_with_frame21_exit_shim(StartFallWithFrame21(cpu.x)));
}

/*
 * @routine 088B:240 SpeedBySideOfScreen
 * @run 70:B240
 * WHAT IT DOES: Sets OBJ_3510 to 2, takes the X speed high from the two-byte table at $B259 by bit 7 of the object's X low and clears the X speed low, then moves the object.
 * HOW: Two ROLs, a table read (bank 70), a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object, C passed into the first ROL. Exit: as ApplyVelocity, Y the table index.
 */
ObjArm SpeedBySideOfScreen(uint8_t slot, bool carry_in)
{
    ObjArm r = { 0 };
    uint8_t low, first, second, speed;
    bool carry_one;

    WR8(OBJ->f3510[slot], 2);
    low = RD8(OBJ->x_lo[slot]);
    first = (uint8_t)((low << 1) | (carry_in ? 1 : 0));
    carry_one = (low & 0x80) != 0;
    second = (uint8_t)((first << 1) | (carry_one ? 1 : 0));
    r.first = (uint8_t)(second & 0x01);
    speed = rd8((uint16_t)(0xB259 + r.first));
    WR8(OBJ->vx_hi[slot], speed);
    WR8(OBJ->vx_lo[slot], 0);
    r.move = ApplyVelocity(slot);
    return r;
}

static Ended speed_by_side_of_screen_exit_shim(ObjArm r)
{
    return ended_with_y(ended_of_axis_move(r.move), r.first);
}

void abi_SpeedBySideOfScreen(void)
{
    abi_ended(speed_by_side_of_screen_exit_shim(SpeedBySideOfScreen(cpu.x, (cpu.p & P_C) != 0)));
}

/*
 * @routine 088A:359 StartThenFallAlongList
 * @run 70:AB59
 * WHAT IT DOES: In state 0 loads the object's record. Later it steps the animation list at $AB72, applies gravity without a limit and moves the object.
 * HOW: LoadObjectRecord, StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: state 0: as LoadObjectRecord; else as ApplyVelocity.
 */
ObjArm StartThenFallAlongList(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    r.third = RD8(OBJ->state[slot]);
    if (r.third == 0) {
        r.arm = S8_FL_RECORD;
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        return r;
    }
    WR8(ZP->temp11, 0xAB);
    WR8(ZP->temp10, 0x72);
    jsr_push(0xAB6C);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    jsr_push(0xAB6F);
    ApplyGravityUnlimited(slot);
    jsr_pop();
    r.move = ApplyVelocity(slot);
    r.y = y;
    r.arm = S8_FL_FALLING;
    return r;
}

static Ended start_then_fall_along_list_exit_shim(ObjArm r)
{
    if (r.arm == S8_FL_RECORD) {
        return then(ended_load(r.third), record_exit_shim(r));
    }
    return then(then(ended_load(r.third), frame_step_exit_shim(r)), ended_with_y(ended_of_axis_move(r.move), r.y));
}

void abi_StartThenFallAlongList(void)
{
    abi_ended(start_then_fall_along_list_exit_shim(StartThenFallAlongList(cpu.x, cpu.y)));
}

/*
 * @routine 088A:39B FrameBySideThenTimerOnScroll
 * @run 70:AB9B
 * WHAT IT DOES: Sets the object's frame to $25 when its X low is under $C0, else 0; once the scroll's Y high byte is not zero it sets OBJ_34B8 to $C0 and goes on to the next state.
 * HOW: Stores, compares, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the scroll's load, or the new state's N and Z (A $C0).
 */
ObjArm FrameBySideThenTimerOnScroll(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(OBJ->frame[slot], 0);
    r.first = RD8(OBJ->x_lo[slot]);
    if (r.first < 0xC0) {
        WR8(OBJ->frame[slot], 0x25);
    }
    r.second = RD8(SCROLL->y_hi[0]);
    r.arm = S8_FB_WAITING;
    if (r.second == 0) {
        return r;
    }
    WR8(OBJ->f34b8[slot], 0xC0);
    r.third = INC8(OBJ->state[slot]);
    r.arm = S8_FB_NEXT;
    return r;
}

static Ended frame_by_side_then_timer_on_scroll_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 0xC0);

    if (r.first < 0xC0) {
        e = then(e, ended_load(0x25));
    }
    e = then(e, ended_load(r.second));
    if (r.arm == S8_FB_WAITING) {
        return e;
    }
    return then(then(e, ended_load(0xC0)), ended_nz(r.third));
}

void abi_FrameBySideThenTimerOnScroll(void)
{
    abi_ended(frame_by_side_then_timer_on_scroll_exit_shim(FrameBySideThenTimerOnScroll(cpu.x)));
}

/*
 * @routine 088A:400 StepListACB2ThenNextOnScreen
 * @run 70:AC00
 * WHAT IT DOES: Steps the animation list at $ACB2, moves the object along Y and goes on to the next state once its Y position is on the first screen (high byte not zero and low byte under $C0).
 * HOW: StepObjectFrames, ApplyVelocityY, compares, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the last compare or load, or the new state's N and Z.
 */
ObjArm StepListACB2ThenNextOnScreen(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0xB2);
    WR8(ZP->temp11, 0xAC);
    jsr_push(0xAC0B);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    jsr_push(0xAC0E);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.first = RD8(OBJ->y_hi[slot]);
    r.arm = S8_AC_HIGH;
    if (r.first == 0) {
        return r;
    }
    r.second = RD8(OBJ->y_lo[slot]);
    r.arm = S8_AC_LOW;
    if (r.second >= 0xC0) {
        return r;
    }
    r.third = INC8(OBJ->state[slot]);
    r.arm = S8_AC_NEXT;
    return r;
}

static Ended step_list_acb2_then_next_on_screen_exit_shim(ObjArm r)
{
    Ended e = then(frame_step_exit_shim(r), ended_of_axis_move(r.move));

    e = then(e, ended_load(r.first));
    if (r.arm == S8_AC_HIGH) {
        return e;
    }
    e = then(e, ended_compare(r.second, 0xC0));
    if (r.arm == S8_AC_LOW) {
        return e;
    }
    return then(e, ended_nz(r.third));
}

void abi_StepListACB2ThenNextOnScreen(void)
{
    abi_ended(step_list_acb2_then_next_on_screen_exit_shim(StepListACB2ThenNextOnScreen(cpu.x, cpu.y)));
}

/*
 * @routine 088A:53B FallAndBlinkThenClear
 * @run 70:AD3B
 * WHAT IT DOES: Adds $0008 to the object's Y speed and moves the object. Its frame is 0, or on every other frame of the wave counter OBJ_34B8 plus $47. Counts OBJ_3510 down and clears the object at zero.
 * HOW: A 16-bit add, ApplyVelocity, a test of zTemp wave bit 0, an add, DEC, ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: the count, or as ClearObject.
 */
ObjArm FallAndBlinkThenClear(uint8_t slot)
{
    ObjArm r = { 0 };

    r.sum = add_bytes(RD8(OBJ->vy_lo[slot]), 0x08, false);
    WR8(OBJ->vy_lo[slot], r.sum.value);
    r.sum2 = add_bytes(RD8(OBJ->vy_hi[slot]), 0x00, r.sum.carry);
    WR8(OBJ->vy_hi[slot], r.sum2.value);
    jsr_push(0xAD4F);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    WR8(OBJ->frame[slot], 0);
    r.first = (uint8_t)(RD8(ZP->wave_counter) & 0x01);
    if (r.first != 0) {
        r.sum3 = add_bytes(RD8(OBJ->f34b8[slot]), 0x47, false);
        WR8(OBJ->frame[slot], r.sum3.value);
    }
    r.second = DEC8(OBJ->f3510[slot]);
    if (r.second == 0) {
        jsr_push(0xAD69);
        ClearObject(slot);
        jsr_pop();
    }
    return r;
}

static Ended fall_and_blink_then_clear_exit_shim(ObjArm r)
{
    Ended e = ended_of_sum(r.sum2);

    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_load(r.first));
    if (r.first != 0) {
        e = then(e, ended_of_sum(r.sum3));
    }
    return then(e, ended_nz(r.second));
}

void abi_FallAndBlinkThenClear(void)
{
    abi_ended(fall_and_blink_then_clear_exit_shim(FallAndBlinkThenClear(cpu.x)));
}

/*
 * @routine 088A:4FC LoadThenSpeedFromTable
 * @run 70:ACFC
 * WHAT IT DOES: Loads the object's record, takes its X speed from the two-byte entries of the table at $AD21 by OBJ_34B8, its timer from the table at $AD29 and its frame as OBJ_34B8 plus $47.
 * HOW: LoadObjectRecord, table reads (bank 70), an add.
 * WAITS: no.
 * Entry: X object. Exit: A the frame, N, Z, C and V from the add; Y OBJ_34B8.
 */
ObjArm LoadThenSpeedFromTable(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xACFF);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    r.third = RD8(OBJ->f34b8[slot]);
    r.fourth = (uint8_t)(r.third << 1);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xAD22 + r.fourth)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xAD21 + r.fourth)));
    r.fifth = RD8(OBJ->f34b8[slot]);
    WR8(OBJ->f3510[slot], rd8((uint16_t)(0xAD29 + r.fifth)));
    r.sum = add_bytes(r.fifth, 0x47, false);
    WR8(OBJ->frame[slot], r.sum.value);
    return r;
}

static Ended load_then_speed_from_table_exit_shim(ObjArm r)
{
    Ended e = then(record_exit_shim(r), ended_with_carry(ended_with_y(ended_load(r.fourth), r.fourth), (r.third & 0x80) != 0));

    e = then(e, ended_with_y(ended_nz(r.fifth), r.fifth));
    return then(e, ended_of_sum(r.sum));
}

void abi_LoadThenSpeedFromTable(void)
{
    abi_ended(load_then_speed_from_table_exit_shim(LoadThenSpeedFromTable(cpu.x)));
}

/*
 * @routine 088B:6C9 PlaceFromTableCopySlot10
 * @run 70:B6C9
 * WHAT IT DOES: Puts the object at the X and Y low bytes the tables at $BB66 and $BBA6 give for the row the table at $B736 names by OBJ_3510, copies the sprite set, frame and facing of the object in slot $10 and sets its flash flags to $10.
 * HOW: Table reads (bank 70), copies.
 * WAITS: no.
 * Entry: X object. Exit: A $10, N and Z from it; Y the row.
 */
ObjArm PlaceFromTableCopySlot10(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->f3510[slot]);
    r.second = rd8((uint16_t)(0xB736 + r.first));
    WR8(OBJ->x_lo[slot], rd8((uint16_t)(0xBB66 + r.second)));
    WR8(OBJ->y_lo[slot], rd8((uint16_t)(0xBBA6 + r.second)));
    WR8(OBJ->sprite_set[slot], RD8(OBJ->sprite_set[0x10]));
    WR8(OBJ->frame[slot], RD8(OBJ->frame[0x10]));
    WR8(OBJ->facing_left[slot], RD8(OBJ->facing_left[0x10]));
    WR8(OBJ->flash_flags[slot], 0x10);
    return r;
}

static Ended place_from_table_copy_slot10_exit_shim(ObjArm r)
{
    Ended e = then(ended_with_y(ended_nz(r.first), r.first), ended_with_y(ended_load(r.second), r.second));

    return then(e, ended_load(0x10));
}

void abi_PlaceFromTableCopySlot10(void)
{
    abi_ended(place_from_table_copy_slot10_exit_shim(PlaceFromTableCopySlot10(cpu.x)));
}

/*
 * @routine 088A:4B9 StartThreeNumberedObjects
 * @run 70:ACB9
 * WHAT IT DOES: Clears the objects in slots $16 to $18, starts a kind $28 object in each at the place of the object in slot $3E, gives each sprite set 7, its number (0 to 2) as OBJ_34B8 and the frame number plus $47, and then clears the object in slot $3E.
 * HOW: A loop of ClearObjectInY and StartObjectAtPlace, ClearObject.
 * WAITS: no.
 * Entry: X object (kept). Exit: the last compare of Y with $19; A the last frame.
 */
ObjArm StartThreeNumberedObjects(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y, number;

    push8(slot);
    WR8(ZP->temp00, 0);
    for (y = 0x16; y < 0x19; y++) {
        jsr_push(0xACC3);
        ClearObjectInY(y);
        jsr_pop();
        jsr_push(0xACC8);
        StartObjectAtPlace(0x28, 0x3E, y);
        jsr_pop();
        WR8(OBJ->sprite_set[y], 7);
        number = RD8(ZP->temp00);
        WR8(OBJ->f34b8[y], number);
        r.sum = add_bytes(number, 0x47, false);
        WR8(OBJ->frame[y], r.sum.value);
        INC8(ZP->temp00);
    }
    jsr_push(0xACE2);
    ClearObject(0x3E);
    jsr_pop();
    r.first = pull8();
    return r;
}

/* Each pass replaces the exit of the one before: the last frame's add, then INY and CPY #$19. */
static Ended start_three_numbered_objects_exit_shim(ObjArm r)
{
    Ended e = then(ended_of_sum(r.sum), ended_with_y(ended_with_carry(ended_nz(0), true), 0x19));

    return then(e, ended_nz(r.first));
}

void abi_StartThreeNumberedObjects(void)
{
    abi_ended(start_three_numbered_objects_exit_shim(StartThreeNumberedObjects(cpu.x)));
}

/*
 * @routine 088B:705 PlaceFromTableThenStepList
 * @run 70:B705
 * WHAT IT DOES: Puts the object at the X and Y low bytes the tables at $BB66 and $BBA6 give for the row the table at $B73A names by OBJ_3510, sets sprite set 7, copies the facing of the object in slot $10 and steps the animation list the table at $B73E gives for OBJ_3510.
 * HOW: Table reads (bank 70), StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y overwritten. Exit: as StepObjectFrames.
 */
ObjArm PlaceFromTableThenStepList(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y;

    r.first = RD8(OBJ->f3510[slot]);
    r.second = rd8((uint16_t)(0xB73A + r.first));
    WR8(OBJ->x_lo[slot], rd8((uint16_t)(0xBB66 + r.second)));
    WR8(OBJ->y_lo[slot], rd8((uint16_t)(0xBBA6 + r.second)));
    WR8(OBJ->sprite_set[slot], 7);
    WR8(OBJ->facing_left[slot], RD8(OBJ->facing_left[0x10]));
    r.third = RD8(OBJ->f3510[slot]);
    r.fourth = (uint8_t)(r.third << 1);
    y = r.fourth;
    WR8(ZP->temp10, rd8((uint16_t)(0xB73E + y)));
    WR8(ZP->temp11, rd8((uint16_t)(0xB73F + y)));
    jsr_push(0xB735);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    return r;
}

static Ended place_from_table_then_step_list_exit_shim(ObjArm r)
{
    Ended e = then(ended_with_y(ended_nz(r.first), r.first), ended_with_y(ended_load(r.second), r.second));

    e = then(e, ended_with_carry(ended_with_y(ended_load(r.fourth), r.fourth), (r.third & 0x80) != 0));
    return then(e, frame_step_exit_shim(r));
}

void abi_PlaceFromTableThenStepList(void)
{
    abi_ended(place_from_table_then_step_list_exit_shim(PlaceFromTableThenStepList(cpu.x)));
}

/*
 * @routine 088C:43C StartThenWaitForField83
 * @run 70:BC3C
 * WHAT IT DOES: In state 0 loads the object's record and sets frame 2 and sprite set 6. In state 1, once zField83 is not zero, it sets the scroll Y high to 1, frame $46, sprite set 7, X low $80, Y low $58 and goes on to the next state.
 * HOW: LoadObjectRecord, a DEC, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the DEC, the load of zField83, or the new state's N and Z (A $58).
 */
ObjArm StartThenWaitForField83(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t count;

    r.sixth = RD8(OBJ->state[slot]);
    count = r.sixth;
    if (r.sixth == 0) {
        r.flag = true;
        jsr_push(0xBC44);
        r.first = LoadObjectRecord(slot, &r.second, &r.other);
        jsr_pop();
        WR8(OBJ->frame[slot], 2);
        WR8(OBJ->sprite_set[slot], 6);
        count = 6;
    }
    count--;
    r.third = count;
    r.arm = S8_WF_COUNTING;
    if (count != 0) {
        return r;
    }
    r.fourth = RD8(ZP->field83);
    r.arm = S8_WF_WAITING;
    if (r.fourth == 0) {
        return r;
    }
    WR8(SCROLL->y_hi[0], 1);
    WR8(OBJ->frame[slot], 0x46);
    WR8(OBJ->sprite_set[slot], 7);
    WR8(OBJ->x_lo[slot], 0x80);
    WR8(OBJ->y_lo[slot], 0x58);
    r.fifth = INC8(OBJ->state[slot]);
    r.arm = S8_WF_STARTED;
    return r;
}

/* r.flag: the record was loaded (state 0); r.other its carry. */
static Ended start_then_wait_for_field83_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.sixth);

    if (r.flag) {
        e = then(e, ended_of_record(r.first, r.second, r.other));
        e = then(then(e, ended_load(2)), ended_load(6));
    }
    e = then(e, ended_load(r.third));
    if (r.arm == S8_WF_COUNTING) {
        return e;
    }
    e = then(e, ended_load(r.fourth));
    if (r.arm == S8_WF_WAITING) {
        return e;
    }
    return then(then(e, ended_load(0x58)), ended_nz(r.fifth));
}

void abi_StartThenWaitForField83(void)
{
    abi_ended(start_then_wait_for_field83_exit_shim(StartThenWaitForField83(cpu.x)));
}

/*
 * @routine 088A:699 LoadThenStartFiveFromTable
 * @run 70:AE99
 * WHAT IT DOES: Loads the object's record, clears OBJ_3778 and its Y speed, sets OBJ_34B8 to $30 and bit 2 of zField8E, copies its X and Y low bytes into all 64 entries of the two tables at $BB66 and $BBA6, then clears the objects in slots $11 to $15 and starts in each an object whose kind the table at $AEDA gives, at this object's place.
 * HOW: LoadObjectRecord, two loops, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: the last compare of Y with $16.
 */
ObjArm LoadThenStartFiveFromTable(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y, kind;

    jsr_push(0xAE9C);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    WR8(OBJ->f3778[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->f34b8[slot], 0x30);
    r.third = (uint8_t)(RD8(ZP->field8e) | 0x04);
    WR8(ZP->field8e, r.third);
    for (y = 0; y < 0x40; y++) {
        wr8((uint16_t)(0xBB66 + y), RD8(OBJ->x_lo[slot]));
        wr8((uint16_t)(0xBBA6 + y), RD8(OBJ->y_lo[slot]));
    }
    for (y = 0x11; y < 0x16; y++) {
        jsr_push(0xAEC7);
        ClearObjectInY(y);
        jsr_pop();
        push8(y);
        kind = rd8((uint16_t)(0xAEDA + (y - 0x11)));
        y = pull8();
        jsr_push(0xAED4);
        r.fourth = StartObjectAtPlace(kind, slot, y);
        jsr_pop();
    }
    return r;
}

static Ended load_then_start_five_from_table_exit_shim(ObjArm r)
{
    Ended e = then(record_exit_shim(r), ended_load(r.third));

    return then(e, ended_with_overflow(ended_with_y(ended_with_a(ended_with_carry(ended_nz(0), true), r.fourth), 0x16), false));
}

void abi_LoadThenStartFiveFromTable(void)
{
    abi_ended(load_then_start_five_from_table_exit_shim(LoadThenStartFiveFromTable(cpu.x)));
}

/*
 * @routine 088B:098 FallThenSoundAtPeak
 * @run 70:B098
 * WHAT IT DOES: Applies gravity and moves the object; once its Y speed high byte is not negative it sends sound $9A and goes on to the next OBJ_2800.
 * HOW: ApplyGravity, ApplyVelocity, SoundCommand, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound's push. Exit: see the wrapper.
 */
ObjArm FallThenSoundAtPeak(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };

    jsr_push(0xB09B);
    ApplyGravity(slot);
    jsr_pop();
    jsr_push(0xB09E);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.first = RD8(OBJ->vy_hi[slot]);
    r.arm = S8_FT_FALLING;
    if ((r.first & 0x80) != 0) {
        return r;
    }
    r.sounded = true;
    r.command = 0x9A;
    jsr_push(0xB0A8);
    r.call = SoundCommand(0x9A, slot, y_entry);
    jsr_pop();
    r.second = INC8(OBJ->f2800[slot]);
    r.arm = S8_FT_PEAK;
    return r;
}

static SoundEnd fall_then_sound_at_peak_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = then(then(ended_of_axis_move(r.move), ended_load(r.first)), ended_kept());

    if (r.arm == S8_FT_FALLING) {
        s.end = e;
        return s;
    }
    s.before = then(e, ended_load(0x9A));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.end = ended_nz(r.second);
    return s;
}

void abi_FallThenSoundAtPeak(void)
{
    abi_sound_end(fall_then_sound_at_peak_exit_shim(FallThenSoundAtPeak(cpu.x, cpu.y)));
}

/*
 * @routine 088B:0E8 SpeedFromRandomRow
 * @run 70:B0E8
 * WHAT IT DOES: Takes the object's two speeds (four bytes: X low and high, Y low and high) from the row of the table at $B109 that a random number from 0 to 7 picks.
 * HOW: NextRandom, table reads (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: A the last byte read, Y the row offset, C clear.
 */
ObjArm SpeedFromRandomRow(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB0EB);
    r.random = NextRandom();
    jsr_pop();
    r.first = (uint8_t)((r.random.value & 0x07) << 2);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB10A + r.first)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB109 + r.first)));
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(0xB10C + r.first)));
    r.second = rd8((uint16_t)(0xB10B + r.first));
    WR8(OBJ->vy_lo[slot], r.second);
    return r;
}

static Ended speed_from_random_row_exit_shim(ObjArm r)
{
    Ended e = then(ended_of_random(r.random),
                   ended_with_carry(ended_with_y(ended_load((uint8_t)(r.random.value & 0x07)), r.first), false));

    return then(e, ended_load(r.second));
}

void abi_SpeedFromRandomRow(void)
{
    abi_ended(speed_from_random_row_exit_shim(SpeedFromRandomRow(cpu.x)));
}

/*
 * @routine 088A:63F SetHitBoxFromFrameTable
 * @run 70:AE3F
 * WHAT IT DOES: Takes the object's hit box (OBJ_3930, OBJ_39E0, OBJ_3988, OBJ_3A38) from the four-byte row of the table at $B8F7 that its frame less one names.
 * HOW: Table reads (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: A the last byte read, Y the row offset.
 */
ObjArm SetHitBoxFromFrameTable(uint8_t slot)
{
    ObjArm r = { 0 };

    r.sum = sub_bytes(RD8(OBJ->frame[slot]), 1, true);
    r.first = (uint8_t)(r.sum.value << 2);
    WR8(OBJ->hitbox_dx[slot], rd8((uint16_t)(0xB8F7 + r.first)));
    WR8(OBJ->hitbox_dy[slot], rd8((uint16_t)(0xB8F8 + r.first)));
    WR8(OBJ->hitbox_half_w[slot], rd8((uint16_t)(0xB8F9 + r.first)));
    r.second = rd8((uint16_t)(0xB8FA + r.first));
    WR8(OBJ->hitbox_half_h[slot], r.second);
    return r;
}

static Ended set_hit_box_from_frame_table_exit_shim(ObjArm r)
{
    Ended e = then(ended_of_sum(r.sum), ended_with_carry(ended_with_y(ended_load(r.first), r.first), (r.sum.value & 0x40) != 0));

    return then(e, ended_load(r.second));
}

void abi_SetHitBoxFromFrameTable(void)
{
    abi_ended(set_hit_box_from_frame_table_exit_shim(SetHitBoxFromFrameTable(cpu.x)));
}

/*
 * @routine 088B:7AF FollowSlot10WithHitBox
 * @run 70:B7AF
 * WHAT IT DOES: In state 0 loads the object's record. Later it takes the place, facing and OBJ_3250 of the object in slot $10 and a hit box from the four-byte row of the table at $B9DF that slot's frame less one names; with a zero half width and half height it clears OBJ_3250. When the object in slot $10 is no longer of kind $20 this object is cleared.
 * HOW: LoadObjectRecord, CopyObjectPlaceBack, table reads (bank 70), ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: the compare with $20, or as ClearObject.
 */
ObjArm FollowSlot10WithHitBox(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t value;

    r.tenth = RD8(OBJ->state[slot]);
    if (r.tenth == 0) {
        r.arm = S8_FS_RECORD;
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        return r;
    }
    jsr_push(0xB7BC);
    r.third = CopyObjectPlaceBack(0x10, slot);
    jsr_pop();
    r.sum = sub_bytes(RD8(OBJ->frame[0x10]), 1, true);
    r.fourth = (uint8_t)(r.sum.value << 2);
    WR8(OBJ->hitbox_dx[slot], rd8((uint16_t)(0xB9DF + r.fourth)));
    WR8(OBJ->hitbox_dy[slot], rd8((uint16_t)(0xB9E0 + r.fourth)));
    WR8(OBJ->hitbox_half_w[slot], rd8((uint16_t)(0xB9E1 + r.fourth)));
    r.fifth = rd8((uint16_t)(0xB9E2 + r.fourth));
    WR8(OBJ->hitbox_half_h[slot], r.fifth);
    r.sixth = RD8(OBJ->facing_left[0x10]);
    WR8(OBJ->facing_left[slot], r.sixth);
    r.seventh = RD8(OBJ->hitbox_on[0x10]);
    WR8(OBJ->hitbox_on[slot], r.seventh);
    value = RD8(OBJ->hitbox_half_w[slot]);
    r.eighth = (uint8_t)(value | RD8(OBJ->hitbox_half_h[slot]));
    if (r.eighth == 0) {
        WR8(OBJ->hitbox_on[slot], 0);
    }
    r.ninth = RD8(OBJ->kind[0x10]);
    if (r.ninth != 0x20) {
        ClearObject(slot);
    }
    r.arm = S8_FS_FOLLOWED;
    return r;
}

static Ended follow_slot10_with_hit_box_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == S8_FS_RECORD) {
        return then(ended_load(r.tenth), record_exit_shim(r));
    }
    e = then(then(ended_load(r.tenth), ended_with_y(ended_load(r.third), 0x10)), ended_nz(r.third));
    e = then(then(e, ended_of_sum(r.sum)), ended_with_carry(ended_with_y(ended_load(r.fourth), r.fourth), (r.sum.value & 0x40) != 0));
    e = then(e, ended_load(r.fifth));
    e = then(e, ended_load(r.sixth));
    e = then(e, ended_load(r.seventh));
    e = then(e, ended_load(r.eighth));
    return then(e, ended_compare(r.ninth, 0x20));
}

void abi_FollowSlot10WithHitBox(void)
{
    abi_ended(follow_slot10_with_hit_box_exit_shim(FollowSlot10WithHitBox(cpu.x)));
}

/*
 * @routine 088B:285 SoundThenAimRandomLength
 * @run 70:B285
 * WHAT IT DOES: Sends sound $2B, then sets the object's speed along the angle to the player with a length from a table at $B2A7 by a random 0 to 3, and goes on to the next OBJ_2800.
 * HOW: SoundCommand, AngleToPlayer, NextRandom, SetVelocityFromAngle, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound's push. Exit: see the wrapper.
 */
ObjArm SoundThenAimRandomLength(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };

    r.sounded = true;
    r.command = 0x2B;
    jsr_push(0xB28A);
    r.call = SoundCommand(0x2B, slot, y_entry);
    jsr_pop();
    jsr_push(0xB28D);
    r.angle = AngleToPlayer(slot);
    jsr_pop();
    push8(r.angle.a);
    jsr_push(0xB291);
    r.random = NextRandom();
    jsr_pop();
    r.second = (uint8_t)(r.random.value & 0x03);
    r.first = (uint8_t)(r.second << 1);
    WR8(ZP->temp00, rd8((uint16_t)(0xB2A7 + r.first)));
    WR8(ZP->temp01, rd8((uint16_t)(0xB2A8 + r.first)));
    r.third = pull8();
    jsr_push(0xB2A3);
    r.scale = SetVelocityFromAngle(r.third, slot);
    jsr_pop();
    r.fourth = INC8(OBJ->f2800[slot]);
    return r;
}

static SoundEnd sound_then_aim_random_length_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = ended_of_angle(r.angle);

    s.before = ended_load(0x2B);
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    e = then(then(e, ended_of_random(r.random)), ended_with_carry(ended_with_y(ended_load(r.second), r.first), false));
    e = then(e, ended_load(r.third));
    e = then(e, ended_of_scale(r.scale));
    s.end = then(e, ended_nz(r.fourth));
    return s;
}

void abi_SoundThenAimRandomLength(void)
{
    abi_sound_end(sound_then_aim_random_length_exit_shim(SoundThenAimRandomLength(cpu.x, cpu.y)));
}

/*
 * @routine 088A:32E DrawStepCountThenClear
 * @run 70:AB2E
 * WHAT IT DOES: On every 32nd frame of the wave counter it draws the tile script of group 0 numbered OBJ_3040, starts a kind $25 object in a free slot from $30 at its place and moves; it counts OBJ_3040 up and clears the object when it reaches 4.
 * HOW: DrawTileScript, FindFreeSlotFrom30, StartObjectAtPlace, ApplyVelocity, an INC, ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or as ClearObject.
 */
ObjArm DrawStepCountThenClear(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t found_slot;
    bool found;

    r.first = (uint8_t)(RD8(ZP->wave_counter) & 0x1F);
    r.arm = S8_DC_SKIP;
    if (r.first != 0) {
        return r;
    }
    r.second = RD8(OBJ->f3040[slot]);
    r.sum = add_bytes(r.second, 0, false);
    jsr_push(0xAB3E);
    r.run = DrawTileScript(r.sum.value, 0);
    jsr_pop();
    jsr_push(0xAB41);
    found_slot = FindFreeSlotFrom30(&r.fourth, &found);
    jsr_pop();
    r.third = found_slot;
    r.flag = found;
    if (found) {
        jsr_push(0xAB48);
        r.fifth = StartObjectAtPlace(0x25, slot, found_slot);
        jsr_pop();
    }
    jsr_push(0xAB4B);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    INC8(OBJ->f3040[slot]);
    r.sixth = RD8(OBJ->f3040[slot]);
    if (r.sixth >= 4) {
        ClearObject(slot);
    }
    r.arm = S8_DC_DONE;
    return r;
}

static Ended draw_step_count_then_clear_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == S8_DC_SKIP) {
        return e;
    }
    e = then(then(e, ended_load(r.second)), ended_of_sum(r.sum));
    e = then(e, ended_of_tile_run(r.run, r.sum.value));
    e = then(e, ended_of_find_slot(r.third, r.fourth, r.flag));
    if (r.flag) {
        e = then(then(e, ended_load(0x25)), ended_with_y(ended_load(r.fifth), r.third));
    }
    e = then(e, ended_of_axis_move(r.move));
    e = then(then(e, ended_nz(r.sixth)), ended_load(r.sixth));
    return then(e, ended_compare(r.sixth, 4));
}

void abi_DrawStepCountThenClear(void)
{
    abi_ended(draw_step_count_then_clear_exit_shim(DrawStepCountThenClear(cpu.x)));
}

/*
 * @routine 088B:0AC StepListIndex0AThenSpawn
 * @run 70:B0AC
 * WHAT IT DOES: Steps the animation list StepListByIndex088A gives for index $0A. When the list ends it clears its animation step and timer and goes to state 2. While it runs, on a changed frame $24 it starts a kind $23 object in a free slot from $30 at its place.
 * HOW: StepListByIndex088A, FindFreeSlotFrom30, StartObjectAtPlace, stores.
 * WAITS: no.
 * Entry: X object. Exit: the step's, the compare, or the start's registers.
 */
ObjArm StepListIndex0AThenSpawn(uint8_t slot)
{
    ObjArm r = { 0 };
    ObjArm step;
    uint8_t found_slot;
    bool found;

    jsr_push(0xB0B1);
    step = StepListByIndex088A(slot, 0x0A);
    jsr_pop();
    r.sub = obj_sub(step);
    r.arm = S8_SP_ENDED;
    if (step.step == FRAME_LIST_ENDED) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 2);
        return r;
    }
    r.arm = S8_SP_HELD;
    if (step.step != FRAME_CHANGED) {
        return r;
    }
    r.first = RD8(OBJ->frame[slot]);
    r.arm = S8_SP_FRAME;
    if (r.first != 0x24) {
        return r;
    }
    jsr_push(0xB0BF);
    found_slot = FindFreeSlotFrom30(&r.third, &found);
    jsr_pop();
    r.second = found_slot;
    r.flag = found;
    if (!found) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 2);
        r.arm = S8_SP_NO_SLOT;
        return r;
    }
    jsr_push(0xB0C6);
    r.fourth = StartObjectAtPlace(0x23, slot, found_slot);
    jsr_pop();
    r.arm = S8_SP_SPAWNED;
    return r;
}

static Ended step_list_index0a_then_spawn_exit_shim(ObjArm r)
{
    Ended e = then(ended_with_y(ended_nz(0x0A), 0x0A), frame_step_exit_shim(obj_arm(r.sub)));

    if (r.arm == S8_SP_ENDED) {
        return then(e, ended_load(2));
    }
    if (r.arm == S8_SP_HELD) {
        return e;
    }
    e = then(then(e, ended_load(r.first)), ended_compare(r.first, 0x24));
    if (r.arm == S8_SP_FRAME) {
        return e;
    }
    e = then(e, ended_of_find_slot(r.second, r.third, r.flag));
    if (r.arm == S8_SP_NO_SLOT) {
        return then(e, ended_load(2));
    }
    return then(then(e, ended_load(0x23)), ended_with_y(ended_load(r.fourth), r.second));
}

void abi_StepListIndex0AThenSpawn(void)
{
    abi_ended(step_list_index0a_then_spawn_exit_shim(StepListIndex0AThenSpawn(cpu.x)));
}

/*
 * @routine 088B:044 CheckPlayerThenStepListIndex08
 * @run 70:B044
 * WHAT IT DOES: With OBJ_2800 0 it goes on to OBJ_2800 1 and, when the player is $50 or more away along X, goes to state 6. Later it steps the animation list StepListByIndex088A gives for index 8 and when that ends clears its animation step and timer and goes to state 5. (The state 6 arm ran in no script; it is written from the decoded bytes.)
 * HOW: DistanceXToPlayer, StepListByIndex088A, stores.
 * WAITS: no.
 * Entry: X object. Exit: the compare, the step's, or A 5 / 6.
 */
ObjArm CheckPlayerThenStepListIndex08(uint8_t slot)
{
    ObjArm r = { 0 };
    ObjArm step;

    r.first = RD8(OBJ->f2800[slot]);
    if (r.first == 0) {
        r.second = INC8(OBJ->f2800[slot]);
        jsr_push(0xB04F);
        r.distance = DistanceXToPlayer(slot);
        jsr_pop();
        r.arm = S8_CP_NEAR;
        if (r.distance.distance < 0x50) {
            return r;
        }
        WR8(OBJ->state[slot], 6);
        r.arm = S8_CP_FAR;
        return r;
    }
    jsr_push(0xB05E);
    step = StepListByIndex088A(slot, 8);
    jsr_pop();
    r.sub = obj_sub(step);
    r.arm = S8_CP_STEPPING;
    if (step.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 5);
    r.arm = S8_CP_ENDED;
    return r;
}

static Ended check_player_then_step_list_index08_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == S8_CP_NEAR || r.arm == S8_CP_FAR) {
        e = then(then(e, ended_nz(r.second)), ended_of_distance(r.distance));
        e = then(e, ended_compare(r.distance.distance, 0x50));
        if (r.arm == S8_CP_NEAR) {
            return e;
        }
        return then(e, ended_load(6));
    }
    e = then(then(e, ended_with_y(ended_nz(8), 8)), frame_step_exit_shim(obj_arm(r.sub)));
    if (r.arm == S8_CP_STEPPING) {
        return e;
    }
    return then(e, ended_load(5));
}

void abi_CheckPlayerThenStepListIndex08(void)
{
    abi_ended(check_player_then_step_list_index08_exit_shim(CheckPlayerThenStepListIndex08(cpu.x)));
}

/*
 * @routine 088B:25B NearPlayerThenPlaceOrState11
 * @run 70:B25B
 * WHAT IT DOES: When the player is within $60 along X and $A0 along Y the object goes on as SpeedBySideOfScreen; otherwise it goes to state $0B.
 * HOW: DistanceXToPlayer, DistanceYToPlayer, a tail jump into SpeedBySideOfScreen, a store.
 * WAITS: no.
 * Entry: X object. Exit: as SpeedBySideOfScreen, or A $0B.
 */
ObjArm NearPlayerThenPlaceOrState11(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB25E);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.arm = S8_NP_STATE11_X;
    if (r.distance.distance < 0x60) {
        jsr_push(0xB265);
        r.distance2 = DistanceYToPlayer(slot);
        jsr_pop();
        r.arm = S8_NP_STATE11_Y;
        if (r.distance2.distance < 0xA0) {
            r.sub = obj_sub(SpeedBySideOfScreen(slot, false));
            r.arm = S8_NP_PLACED;
            return r;
        }
    }
    WR8(OBJ->state[slot], 0x0B);
    return r;
}

static Ended near_player_then_place_or_state11_exit_shim(ObjArm r)
{
    Ended e = then(ended_of_distance(r.distance), ended_compare(r.distance.distance, 0x60));

    if (r.arm == S8_NP_STATE11_X) {
        return then(e, ended_load(0x0B));
    }
    e = then(then(e, ended_of_distance(r.distance2)), ended_compare(r.distance2.distance, 0xA0));
    if (r.arm == S8_NP_STATE11_Y) {
        return then(e, ended_load(0x0B));
    }
    return then(e, speed_by_side_of_screen_exit_shim(obj_arm(r.sub)));
}

void abi_NearPlayerThenPlaceOrState11(void)
{
    abi_ended(near_player_then_place_or_state11_exit_shim(NearPlayerThenPlaceOrState11(cpu.x)));
}
